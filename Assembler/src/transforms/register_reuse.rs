// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

//! A simple register reuse optimization pass.
//! Since we have infinite registers, using a general register coloring pass
//! would be wasteful.
//!
//! This is essentially what is known as `Linear scan global register allocation`

use super::vectorize;
use crate::asm::{Body, JumpMode, Terminator};
use crate::errors::DeadWrite;
use crate::errors::UninitializedRead;
use crate::lexer::RegisterStruct;
use crate::span::Span;
use crate::Compiler;
use std::num::NonZeroU32;
// Using tree sets to make the optimizations deterministic
use std::collections::BTreeMap;
use std::collections::BTreeSet;

#[derive(Default)]
pub struct Pass;

#[derive(Copy, Clone, Debug)]
struct Lifetime {
    start: usize,
    end: usize,
}

#[derive(Default)]
struct BlockInfo {
    lifetimes: RegisterStruct<BTreeMap<u32, Lifetime>>,
    /// Registers seen anywhere and whether their vector length
    seen: RegisterStruct<BTreeMap<u32, NonZeroU32>>,
    // Registers read before written (may have been written in a prev block)
    uninit: RegisterStruct<BTreeMap<u32, Vec<Span>>>,
    // Registers written but never read (may get read in a different block)
    dead_write: RegisterStruct<BTreeMap<u32, Vec<Span>>>,
}

#[derive(Default)]
struct BodyInfo {
    blocks: Vec<BlockInfo>,
}

impl BodyInfo {
    fn new(cx: &Compiler, body: &Body<'_>) -> Self {
        let mut info = Self::default();
        info.init(cx, body);
        info
    }

    fn init(&mut self, cx: &Compiler, body: &Body<'_>) {
        let span = debug_span!("init");
        let _enter = span.enter();
        for (id, block) in body.blocks.iter().enumerate() {
            let span = debug_span!("block", id);
            let _enter = span.enter();
            let mut lifetimes: RegisterStruct<BTreeMap<u32, Lifetime>> = Default::default();
            let mut seen: RegisterStruct<BTreeMap<u32, NonZeroU32>> = Default::default();
            let mut uninit: RegisterStruct<BTreeMap<u32, Vec<Span>>> = Default::default();
            let mut dead_write: RegisterStruct<BTreeMap<u32, Vec<Span>>> = Default::default();
            // We can't insert into `seen` directly as that would mean later uses overwrite the vectorization
            let mut mark_seen =
                |reg, v| seen.get_or_insert(reg, || Some(v), |old| *old = std::cmp::max(v, *old));
            for (stmt_id, stmt) in block.stmts.iter().enumerate() {
                let relexed = stmt.relex(cx);
                let relexed = relexed.0.display(cx);
                let span = debug_span!("stmt", %relexed);
                let _enter = span.enter();
                // reads happen before writes, because any instruction will first read values
                // and then write values.
                for base_reg in stmt.read_registers_base(cx) {
                    trace!(%base_reg, v = stmt.vectorized.elem.get(), "read");
                    mark_seen(base_reg, stmt.vectorized.elem);
                    for reg in vectorize(stmt.vectorized.elem, base_reg) {
                        trace!(%reg);
                        // reading from a register makes it not dead code
                        let _ = dead_write.remove(reg);
                        // reading from an uninitialized register
                        if lifetimes.get(reg).is_none() {
                            // FIXME: make `read_registers` return spanned registers
                            uninit.get_or_insert(
                                reg,
                                || Some(vec![stmt.span]),
                                |old| old.push(stmt.span),
                            );
                        }
                        lifetimes.get_or_insert(
                            reg,
                            // Reads of registers not initialized in this block can be ignored,
                            // because they will be caught by `cross_block`/`seen`.
                            || None,
                            // Reads terminate a register.
                            |lt| lt.end = stmt_id,
                        );
                    }
                }
                for base_reg in stmt.write_registers_base(cx) {
                    trace!(%base_reg, v = stmt.vectorized.elem.get(), "write");
                    mark_seen(base_reg, stmt.vectorized.elem);
                    for reg in vectorize(stmt.vectorized.elem, base_reg) {
                        trace!(%reg);
                        // Insert into dead write map, reads remove the entry from the map
                        dead_write.get_or_insert(
                            reg,
                            || Some(vec![stmt.span]),
                            |old| old.push(stmt.span),
                        );
                        lifetimes.get_or_insert(
                            reg,
                            // Writes initialize a register.
                            // It may still be used outside the current block, but we catch this via
                            // `cross_block`/`seen`.
                            || {
                                Some(Lifetime {
                                    start: stmt_id,
                                    end: stmt_id,
                                })
                            },
                            // Overwriting a register, there'll probably be another read down the road.
                            // FIXME: should we detect dead code where writes are never read?
                            |lt| lt.end = stmt_id,
                        );
                    }
                }
            }
            match &block.terminator.elem {
                Terminator::Jump(jmp) => match &jmp.mode {
                    JumpMode::Conditional(cnd) => {
                        // FIXME: deduplicate with read logic above
                        let reg = cnd.register.elem.into();
                        mark_seen(reg, NonZeroU32::new(1).unwrap());
                        trace!(%reg);
                        // reading from a register makes it not dead code
                        let _ = dead_write.remove(reg);
                        // reading from an uninitialized register
                        if lifetimes.get(reg).is_none() {
                            // FIXME: make `read_registers` return spanned registers
                            uninit.get_or_insert(
                                reg,
                                || Some(vec![block.terminator.span]),
                                |old| old.push(block.terminator.span),
                            );
                        }
                        lifetimes.get_or_insert(
                            reg,
                            // Reads of registers not initialized in this block can be ignored,
                            // because they will be caught by `cross_block`/`seen`.
                            || None,
                            // Reads terminate a register.
                            |lt| lt.end = block.stmts.len(),
                        );
                    }
                    JumpMode::Call { .. } | JumpMode::Goto => {}
                },
                Terminator::Restart { .. }
                | Terminator::Crash { .. }
                | Terminator::Return { .. }
                | Terminator::Done => {}
            }
            trace!(?lifetimes);
            trace!(?dead_write);
            trace!(?uninit);
            trace!("seen = {:#?}", seen);
            seen.as_mut().map(clean);
            trace!("cleaned seen = {:#?}", seen);
            self.blocks.push(BlockInfo {
                seen,
                lifetimes,
                uninit,
                dead_write,
            });
        }
    }
}

/// Merges register lifetimes of registers of the same vector
fn clean_lifetimes(map: &mut BTreeMap<u32, Lifetime>, base_map: &BTreeMap<u32, u32>) {
    debug!("clean lifetimes: {:#?}", base_map);
    // Convert to a (sorted by register, because of the BTreeMap) vector
    let mut all: Vec<_> = map
        .iter()
        .map(|(&r, &v)| {
            (
                *base_map.get(&r).unwrap_or_else(|| {
                    panic!("register {} not found in {:?}", r, base_map);
                }),
                v,
            )
        })
        .collect();
    all.dedup_by(|(r, v), (a_r, a_v)| {
        if a_r == r {
            // update the previous element if it has a later start
            if a_v.start > v.start {
                trace!(r = *r, v.start, a_v.start, "update");
                a_v.start = v.start;
            }
            // update the previous element if it has an earlier end
            if a_v.end < v.end {
                trace!(r = *r, v.end, a_v.end, "update");
                a_v.end = v.end;
            }
            trace!(r = *r, v.start, v.end, "drop");
            // remove the current element now that it has been merged with the previous
            true
        } else {
            false
        }
    });
    *map = all.into_iter().collect();
}

fn clean(map: &mut BTreeMap<u32, NonZeroU32>) {
    if let Some((&cur_r, &cur_v)) = map.iter().next() {
        let mut cur_r = cur_r;
        let mut cur_v = cur_v;
        let mut all: Vec<_> = map.iter().map(|(&r, &v)| (r, v)).collect();
        let mut first = true;
        all.retain(|&(r, v)| {
            if first {
                first = false;
                return true;
            }
            // Is the register part of the current registers vectorized elements?
            if r < cur_r + cur_v.get() {
                // Is the register *also* vectorized and goes beyond the current vector register?
                let end = r + v.get();
                let cur_end = cur_r + cur_v.get();
                if end > cur_end {
                    // expand register
                    cur_v = NonZeroU32::new(cur_v.get() + end - cur_end).unwrap();
                }
                // do not keep this around, it's covered by the current register
                false
            } else {
                cur_r = r;
                cur_v = v;
                true
            }
        });
        *map = all.into_iter().collect();
    }
}

impl super::Pass for Pass {
    fn name(&self) -> &'static str {
        "register_coloring"
    }
    fn apply<'a>(&mut self, cx: &'a Compiler, body: &mut Body<'a>) {
        if cx.optimization_level == 0 {
            return;
        }
        let BodyInfo { mut blocks } = BodyInfo::new(cx, body);

        if blocks.is_empty() {
            return;
        }
        // Find all accesses that occur in multiple blocks
        let mut cross_block: RegisterStruct<BTreeSet<u32>> = Default::default();
        let mut seen_bases: RegisterStruct<BTreeSet<u32>> = Default::default();
        let mut base_map: RegisterStruct<BTreeMap<u32, u32>> = Default::default();
        let mut seen: RegisterStruct<BTreeMap<u32, NonZeroU32>> = Default::default();
        for (id, block) in blocks.iter().enumerate() {
            let span = debug_span!("block", id);
            let _enter = span.enter();
            trace!(?block.seen);
            seen.as_mut()
                .join(cross_block.as_mut())
                .join(seen_bases.as_mut().join(base_map.as_mut()))
                .join(block.seen.as_ref())
                .map(
                    |(((seen, cross_block), (seen_bases, base_map)), block_seen)| {
                        seen.extend(block_seen);
                        let mut block_seen_bases: BTreeSet<u32> = Default::default();
                        // Get the vectorized register if this is a vector element access
                        // FIXME: this is a bit inefficient (O(n^2)), maybe return a element register -> base register map from `clean`
                        // instead of computing `base_map` on the fly here
                        for (&base_r, base_v) in block_seen {
                            for r in base_r..base_r + base_v.get() {
                                base_map.insert(r, base_r);
                            }
                            block_seen_bases.insert(base_r);
                        }
                        cross_block.extend(block_seen_bases.intersection(seen_bases));
                        seen_bases.extend(block_seen_bases);
                    },
                );
        }
        trace!(?cross_block);
        // Clean them up so sub-vector accesses just count as vector accesses
        seen.as_mut().map(clean);
        trace!(?seen, "cleaned");
        drop(seen_bases);
        trace!(?base_map);

        debug!("merge_reg_lifetimes");
        // Merge register lifetimes of the same vector
        for (id, block) in blocks.iter_mut().enumerate() {
            let span = debug_span!("block", id);
            let _enter = span.enter();
            trace!(?block.lifetimes);
            block
                .lifetimes
                .as_mut()
                .join(base_map.as_ref())
                .map(|(lt, base_map)| clean_lifetimes(lt, base_map));
            trace!(?block.lifetimes);
        }

        // mark all uninitialized locals as global
        for (id, block) in blocks.iter().enumerate() {
            let span = debug_span!("block", id);
            let _enter = span.enter();
            trace!(?base_map);
            trace!(?cross_block);
            trace!(?block.uninit);
            cross_block
                .as_mut()
                .join(block.uninit.as_ref())
                .join(block.dead_write.as_ref().join(base_map.as_ref()))
                .map(|((cross_block, uninit), (dead_write, base_map))| {
                    for (&reg, spans) in uninit {
                        let base = *base_map.get(&reg).unwrap();
                        if cross_block.insert(base) {
                            cx.report(UninitializedRead {
                                spans: spans.clone(),
                                reg,
                            });
                        }
                    }
                    for (&reg, spans) in dead_write {
                        let base = *base_map.get(&reg).unwrap();
                        if cross_block.insert(base) {
                            cx.report(DeadWrite {
                                spans: spans.clone(),
                                reg,
                            });
                        }
                    }
                });
        }
        trace!(?cross_block);

        let mut block_local_maps: Vec<RegisterStruct<BTreeMap<u32, u32>>> = Vec::new();
        // The starting register index of each block starts after all the cross block registers
        let block_start_idx = cross_block
            .as_ref()
            .join(seen.as_ref())
            .map(|(cb, seen)| -> u32 {
                cb.iter()
                    // reserve multiple registers for all vectorized registers
                    .map(|r| seen[r].get())
                    .sum()
            });

        debug!("compute_rename_map");
        // Compute the renaming maps for each block
        for (id, block) in blocks.iter().enumerate() {
            let span = debug_span!("block", id);
            let _enter = span.enter();
            trace!(?block.lifetimes);
            let block_local_map = cross_block
                .as_ref()
                .join(block.lifetimes.as_ref())
                .join(block_start_idx.join(seen.as_ref()))
                .map(|((cross_block, v), (block_start_idx, seen))| {
                    let lifetimes: BTreeSet<(usize, (usize, u32))> = v
                        .iter()
                        // ignore registers from the global set
                        .filter(|(i, _)| !cross_block.contains(i))
                        // sort registers by their start point
                        .map(|(&i, r)| (r.start, (r.end, i)))
                        .collect();
                    trace!(?lifetimes);
                    // Map from old register id to new register id.
                    let mut block_local_map = BTreeMap::new();
                    let mut lifetimes = lifetimes.into_iter();
                    // The first register that is used
                    if let Some((_start, (end, reg))) = lifetimes.next() {
                        // List of free register allocations to pick the next allocation from.
                        // The value is the size of the hole.
                        let mut free: BTreeMap<u32, u32> = BTreeMap::new();

                        // The first index we may use starts after the first cross block
                        // register. This is true for every block by itself.
                        free.insert(block_start_idx, u32::max_value() - block_start_idx);

                        // FIXME: use an allocator crate
                        let mut allocate = |size: NonZeroU32, dealloc: Vec<(u32, NonZeroU32)>| {
                            trace!(?free, ?size, "allocate");
                            for (dealloc, size) in dealloc {
                                let size = size.get();
                                trace!(dealloc);
                                // See if we fit right behind a hole, before a hole or in the middle of two.
                                let mut iter = free.iter();
                                loop {
                                    if let Some((&r, &s)) = iter.next() {
                                        if r + s == dealloc {
                                            if let Some((&r2, &s2)) = iter.next() {
                                                if r2 == dealloc + size {
                                                    // Merge prev and next elements.
                                                    free.remove(&r2);
                                                    *free.get_mut(&r).unwrap() += size + s2;
                                                    break;
                                                }
                                            }
                                            // Merge with prev.
                                            *free.get_mut(&r).unwrap() += size;
                                            break;
                                        } else if r > dealloc {
                                            // New hole
                                            free.insert(dealloc, size);
                                            break;
                                        }
                                    } else {
                                        free.insert(dealloc, size);
                                        break;
                                    }
                                }
                            }
                            let size = size.get();
                            trace!(?free, ?size, "allocate");
                            // Naive "smallest fit" algorithm
                            let smallest = free
                                .iter()
                                .find(|(_, &s)| s >= size)
                                .expect("no more registers available");
                            let mut smallest_size = *smallest.1;
                            let mut smallest = *smallest.0;
                            for (&r, &s) in free.range(smallest..) {
                                if s < smallest_size && s >= size {
                                    smallest = r;
                                    smallest_size = s;
                                }
                            }
                            free.remove(&smallest).unwrap();
                            if smallest_size != size {
                                assert!(smallest_size > size);
                                // Reinsert the leftover hole
                                free.insert(smallest + size, smallest_size - size);
                            }
                            trace!(smallest, ?free, "allocated");
                            smallest
                        };

                        // End location to index map so we know what index ends next.
                        // This is used to deduplicate registers, because if we can remove
                        // an entry from this list, we get the id back for reusing it.
                        let mut ends: BTreeMap<usize, Vec<(u32, NonZeroU32)>> = BTreeMap::new();

                        let idx = allocate(seen[&reg], vec![]);
                        ends.insert(end, vec![(idx, seen[&reg])]);
                        block_local_map.insert(reg, idx);

                        for (start, (end, reg)) in lifetimes {
                            let span = debug_span!("register alloc", ?reg, start, end);
                            let _enter = span.enter();
                            let size = seen[&reg];
                            trace!(?ends);
                            let &next_end = ends
                                .keys()
                                .next()
                                .expect("there's always at least one element");
                            // If the register is used the last time in this statement or a previous
                            // one, reuse the register. This allows a command to read from a register
                            // and write to it in the same instruction.
                            let idx = if next_end <= start {
                                let indices = ends.remove(&next_end).unwrap();
                                allocate(size, indices)
                            } else {
                                // no free block local register available, reserve a new one
                                allocate(size, vec![])
                            };
                            trace!(idx);
                            // If two registers are used the last time in the same instruction put them all
                            // into a list so we free all of them at the same time.
                            ends.entry(end).or_default().push((idx, size));
                            // We need to collect all remappings first, so we can do all renamings at once
                            // without conflicting with the cross block register renamings
                            assert!(block_local_map.insert(reg, idx).is_none());
                        }
                    }
                    block_local_map
                });
            trace!(?block_local_map);
            block_local_maps.push(block_local_map);
        }

        trace!(?base_map);

        // Compute the renaming map for the global registers
        let global_register_map: RegisterStruct<BTreeMap<_, _>> = seen
            .as_ref()
            .join(cross_block.join(base_map.as_ref()))
            .map(|(seen, (cb, base_map))| {
                let mut i = 0;
                let mut glob: BTreeMap<u32, u32> = Default::default();
                for r in cb {
                    let base = base_map.get(&r).unwrap_or_else(|| {
                        panic!("register {} not found in {:#?}", r, base_map);
                    });
                    if let Some(&renamed) = glob.get(&base) {
                        // Existing entry for the vectorized register exists, so we use the same renaming
                        let offset = r - base;
                        glob.insert(r, renamed + offset);
                    } else {
                        let new = i;
                        // Reserve space for the register
                        i += seen
                            .get(&base)
                            .unwrap_or_else(|| {
                                panic!("register {} not found in {:#?}", r, seen);
                            })
                            .get();
                        glob.insert(r, new);
                    }
                }
                glob
            });

        trace!(?global_register_map);

        let span = debug_span!("rewrite_registers");
        let _enter = span.enter();

        // Mutate all registers
        for (id, (block, local_map)) in body.blocks.iter_mut().zip(block_local_maps).enumerate() {
            let span = debug_span!("block", id);
            let _enter = span.enter();
            trace!("{:#?}", local_map);
            let map = |reg| {
                local_map
                    .as_ref()
                    .join(global_register_map.as_ref().join(base_map.as_ref()))
                    .map_reg(reg, |reg, (local_map, (global_register_map, base_map))| {
                        let span = trace_span!("map id", reg);
                        let _enter = span.enter();
                        // Find the vector base register.
                        let base_reg = base_map[&reg];
                        // The index into the vector register
                        let offset = reg - base_reg;
                        let new_id = local_map
                            .get(&base_reg)
                            .or_else(|| global_register_map.get(&base_reg))
                            .cloned();
                        trace!(?new_id);
                        let new_id = new_id.expect("register must be either global or block local");
                        trace!(new_id);
                        // Readd the index so we get back the element register
                        new_id + offset
                    })
            };
            for stmt in &mut block.stmts {
                trace!("replace reg in {}", stmt.relex(cx).0.display(cx));
                stmt.replace_registers(cx, map);
            }
            match &mut block.terminator.elem {
                Terminator::Jump(jmp) => match &mut jmp.mode {
                    JumpMode::Conditional(jc) => {
                        jc.register = jc
                            .register
                            .span
                            .with(map(jc.register.elem.into()))
                            .require(cx)
                    }
                    JumpMode::Goto | JumpMode::Call { .. } => {}
                },
                Terminator::Restart { .. }
                | Terminator::Crash { .. }
                | Terminator::Return { .. }
                | Terminator::Done => {}
            }
        }
    }
}
