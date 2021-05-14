// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use crate::asm::AugmentedBlock;
use crate::asm::AugmentedStatement;
use crate::asm::Block;
use crate::asm::Body;
use crate::asm::Instruction;
use crate::asm::Statement;
use crate::errors::ExpectedGot;
use crate::lexer::Register;
use crate::span::Span;
use crate::Compiler;

use std::cmp;
use std::collections::HashSet;
use std::collections::{BTreeMap, BTreeSet};

#[derive(Default)]
pub struct Pass;

use std::time::Instant;

#[allow(clippy::cognitive_complexity)]
pub fn augment_block<'a>(
    block: &Block<'a>,
    cx: &Compiler,
    init_instr_depth: &[Register],
    init_run: bool,
    check: bool,
) -> Result<(AugmentedBlock<'a>, Vec<Register>), ()> {
    debug!(
        "augment_block {:?}, {:?}, {:?}",
        init_instr_depth, init_run, check
    );
    let mut new_stmts = vec![
        AugmentedStatement {
            instruction_depth: 0,
            round_depth: 0,
            blck: 0,
            stmt: Statement {
                instr: Instruction::Nop,
                vectorized: std::num::NonZeroU32::new(1).unwrap().into(),
                span: Span::DUMMY,
                comment: Span::DUMMY,
            },
            processed: false,
        };
        block.stmts.len()
    ];
    // Copy stmts from block to new_stmts
    for (cnt, stmt) in block.stmts.iter().enumerate() {
        new_stmts[cnt].stmt = stmt.clone();
    }
    let mut register_instr_depth = BTreeMap::new();
    let mut register_round_depth = BTreeMap::new();
    // Load in the registers we know should be assigned -1 now, if none
    // we assume all unmet read registers have -1 depth
    let do_loop = !init_run && !check;
    for &x in init_instr_depth {
        register_instr_depth.insert(x, -1);
        register_round_depth.insert(x, -1);
    }
    let mut looping = true;
    let mut loop_count = 0;

    // Variable here from timing the main loop due tune things
    let now = Instant::now();
    let mut ncont = 0;
    while looping {
        loop_count += 1;
        let mut mem_i_depth = -1;
        let mut mem_r_depth = 0;
        let mut open_i_depth = -1;
        let mut open_r_depth = -1;
        let mut blck = 0;
        let mut startopen_vect_level = 1;
        looping = false;
        debug!(
            "\tStarting another loop : {} {} : {} {} {} : Time {}",
            loop_count,
            ncont,
            block.stmts.len(),
            register_instr_depth.len(),
            register_round_depth.len(),
            now.elapsed().as_secs()
        );
        for (cnt, stmt) in block.stmts.iter().enumerate() {
            let instruction_depth;
            let round_depth;
            let mut cont = true;

            if stmt.is_barrier(cx) {
                blck += 1;
            }
            // Divide also into blocks when we meet a different vectorized startopen from the
            // previous one
            if stmt.is_startopen() && stmt.vectorized.elem.get() != startopen_vect_level {
                blck += 1;
                startopen_vect_level = stmt.vectorized.elem.get();
            }

            if stmt.is_stopopen() && open_i_depth == -1 {
                continue;
            }
            if mem_i_depth == -10 && (stmt.memory_read(cx) || stmt.memory_write(cx)) {
                continue;
            }

            // Test for non SSA format
            let write_regs = stmt.write_registers(cx);
            if !new_stmts[cnt].processed {
                for x in write_regs.clone() {
                    if register_instr_depth.contains_key(&x) {
                        if do_loop {
                            // We should not really ever get here, as to get
                            // here we should have processed the instruction already
                            cx.report(ExpectedGot {
                                expected: "do_loop",
                                got: stmt.span.with("How did we get here?"),
                            });
                            trace!("    # {}", &x.to_string());
                            return Err(());
                        } else {
                            info!(%x, ?register_instr_depth, "skipping start/stop open optimization since code is not in ssa form");
                            return Err(());
                        }
                    }
                }
            }

            // Now the actual operations
            // First compute the max depths of the read instructions
            //   - Test for read before write here
            //   - We need to run this for memory store instructions to get
            //     the mem_i_depth correct. Otherwise if the instruction is
            //     already processed we can skip this step
            let mut max_instr_depth = -1;
            let mut max_rnd_depth = 0;
            let read_regs = stmt.read_registers(cx);
            if stmt.memory_write(cx) || !new_stmts[cnt].processed {
                for x in read_regs.clone() {
                    match register_instr_depth.get(&x) {
                        Some(depth) => {
                            max_instr_depth = cmp::max(max_instr_depth, *depth);
                        }
                        None => {
                            if do_loop {
                                looping = true;
                                if stmt.memory_write(cx) {
                                    mem_i_depth = -10; // Signal dont use memory instructions on this loop
                                }
                                cont = false;
                                continue;
                            } else {
                                register_instr_depth.insert(x, -1);
                            }
                        }
                    }
                    match register_round_depth.get(&x) {
                        Some(depth) => max_rnd_depth = cmp::max(max_rnd_depth, *depth),
                        None => {
                            if !do_loop {
                                register_round_depth.insert(x, -1);
                            }
                        }
                    }
                }
            }

            if !cont || new_stmts[cnt].processed {
                continue;
            }

            // If we get here we can deal with this instruction now
            //   - So we do so.

            // Instructions which both load or store
            if stmt.memory_write(cx) || stmt.memory_read(cx) {
                mem_i_depth = cmp::max(mem_i_depth, max_instr_depth) + 1;
                mem_r_depth = cmp::max(mem_r_depth, max_rnd_depth);
                for x in write_regs {
                    register_instr_depth.insert(x, mem_i_depth);
                    register_round_depth.insert(x, mem_r_depth);
                }
                instruction_depth = mem_i_depth;
                round_depth = mem_r_depth;
            }
            // Read only instruction
            else if write_regs.is_empty() && !read_regs.is_empty() && !stmt.is_startopen() {
                instruction_depth = max_instr_depth + 1;
                round_depth = max_rnd_depth;
            }
            // Write to register instruction
            else if !write_regs.is_empty() && !stmt.is_stopopen() {
                for x in write_regs {
                    register_instr_depth.insert(x, max_instr_depth + 1);
                    register_round_depth.insert(x, max_rnd_depth);
                }
                instruction_depth = max_instr_depth + 1;
                round_depth = max_rnd_depth;
            }
            // StartOpen
            else if stmt.is_startopen() {
                open_i_depth = max_instr_depth + 1;
                open_r_depth = max_rnd_depth;
                instruction_depth = max_instr_depth + 1;
                round_depth = max_rnd_depth;
            }
            // StopOpen
            else if stmt.is_stopopen() {
                for x in write_regs {
                    register_instr_depth.insert(x, open_i_depth + 1);
                    register_round_depth.insert(x, open_r_depth + 1);
                }
                instruction_depth = open_i_depth + 1;
                round_depth = open_r_depth + 1;
                open_i_depth = -1;
                open_r_depth = -1;
            } else {
                instruction_depth = 0;
                round_depth = 0;
            }
            new_stmts[cnt].instruction_depth = instruction_depth;
            new_stmts[cnt].round_depth = round_depth;
            new_stmts[cnt].blck = blck;
            new_stmts[cnt].processed = true;
            ncont += 1;
        }
    }
    debug!("\tFinished looping");

    let new_block = AugmentedBlock { stmts: new_stmts };
    let mut new_init = Vec::new();
    // Print the instructions and the register depths
    if !trace_span!("").is_disabled() {
        for i in 0..new_block.stmts.len() {
            let relexed = new_block.stmts[i].stmt.relex(cx);
            trace!("    # {}", relexed.0.display(cx));
            trace!(
                "    #  instruction_depth: {}",
                new_block.stmts[i].instruction_depth
            );
            trace!("    #  round_depth: {}", new_block.stmts[i].round_depth);
            trace!("    #  block: {:?}", new_block.stmts[i].blck);
        }
    }
    for (reg, depth) in &register_instr_depth {
        if !trace_span!("").is_disabled() {
            trace!("{} has {} instruction depth", reg, depth);
        }
        if *depth == -1 {
            new_init.push(*reg);
        }
    }
    if !trace_span!("").is_disabled() {
        for (reg, depth) in &register_round_depth {
            trace!("{} has {} round depth", reg, depth);
        }
    }
    Ok((new_block, new_init))
}

// We now run the first step of optimize [merge the start/stop opens]
impl super::Pass for Pass {
    fn name(&self) -> &'static str {
        "start_stop_open"
    }

    #[allow(clippy::cognitive_complexity)]
    #[allow(clippy::nonminimal_bool)]
    #[allow(clippy::needless_range_loop)]
    fn apply<'a>(&mut self, cx: &'a Compiler, body: &mut Body<'a>) {
        if cx.optimization_level < 2 {
            return;
        }
        'blocks: for (block_id, block) in body.blocks.iter_mut().enumerate() {
            // don't panic on subnormal assembly
            if block.stmts.is_empty() {
                continue;
            }
            debug!("\n# Block {}", block_id);
            let init = Vec::new();
            let (mut augmented_block, new_init) = match augment_block(block, cx, &init, true, false)
            {
                Ok(data) => data,
                Err(()) => continue 'blocks,
            };
            // We go through the AugmentedBlock and merge start/stop open instructions
            // which have the same round depth and are not seperated by a barrier
            // instruction.
            debug!("\tMerging stop/start open");
            for i in 0..augmented_block.stmts.len() {
                if let Some((a_stmt, trailing)) = augmented_block.stmts[i..].split_first_mut() {
                    // Merge startopens
                    if let Instruction::StartOpen { registers: i_regs } = &mut a_stmt.stmt.instr {
                        if !trailing[0].stmt.is_stopopen() {
                            println!(
                                "XXXX For input assembler a stopopen should follow a startopen"
                            )
                        }
                        // Go through rest of instructions in this block looking for a match
                        for j in 0..trailing.len() {
                            if let Instruction::StartOpen { registers: j_regs } =
                                &trailing[j].stmt.instr
                            {
                                if trailing[j].round_depth == a_stmt.round_depth
                                    && trailing[j].stmt.vectorized.elem.get()
                                        == a_stmt.stmt.vectorized.elem.get()
                                    && trailing[j].blck == a_stmt.blck
                                {
                                    if let Instruction::StopOpen { .. } =
                                        &trailing[j + 1].stmt.instr
                                    {
                                        // Need to merge instructions (i,i+1) with (j,j+1)
                                        i_regs.extend(j_regs);
                                        trailing[j].stmt.instr = Instruction::Nop;
                                    } else {
                                        println!("XXXX For input assembler a stopopen should follow a startopen")
                                    }
                                }
                            }
                        }
                    }
                    // Merge stopopens
                    if let Instruction::StopOpen { registers: i_regs } = &mut a_stmt.stmt.instr {
                        // Go through rest of instructions in this block looking for a match
                        for j in 0..trailing.len() {
                            if let Instruction::StopOpen { registers: j_regs } =
                                &trailing[j].stmt.instr
                            {
                                if trailing[j].round_depth == a_stmt.round_depth
                                    && trailing[j].stmt.vectorized.elem.get()
                                        == a_stmt.stmt.vectorized.elem.get()
                                    && trailing[j].blck == a_stmt.blck
                                {
                                    // Need to merge instructions (i,i+1) with (j,j+1)
                                    i_regs.extend(j_regs);
                                    trailing[j].stmt.instr = Instruction::Nop;
                                }
                            }
                        }
                    }
                }
            }
            debug!("\tDone merging stop/start open");

            // Map back to block for next call to augment_block
            for i in 0..augmented_block.stmts.len() {
                block.stmts[i] = augmented_block.stmts[i].stmt.clone();
            }

            // Now recompute the instruction depths
            let (mut augmented_block1, _) = match augment_block(block, cx, &new_init, false, false)
            {
                Ok(data) => data,
                Err(()) => continue 'blocks,
            };
            debug!("\tDone new augmentation: Now sorting");

            // We go now do the sort mentioned in the Documentation
            for blck in 0..=augmented_block1.max_blck_number() {
                let (mut startb, stopb) = augmented_block1.start_stop_blck(blck);
                // The first instruction could be a barrier so we skip it in the sort
                if augmented_block1.stmts[startb].stmt.is_barrier(cx) {
                    startb += 1;
                }

                //              Fast sort variant
                augmented_block1.stmts[startb..=stopb].sort();
            }

            debug!("\tFinished sorting");
            // Map back to block for next call to augment_block
            for i in 0..augmented_block1.stmts.len() {
                block.stmts[i] = augmented_block1.stmts[i].stmt.clone();
            }

            // Double check now
            let (augmented_block2, _) = match augment_block(block, cx, &new_init, false, true) {
                Ok(data) => data,
                Err(()) => continue 'blocks,
            };

            if cx.optimization_level < 3 {
                continue 'blocks;
            }

            let dummy = Statement {
                instr: Instruction::Nop,
                vectorized: std::num::NonZeroU32::new(1).unwrap().into(),
                span: Span::DUMMY,
                comment: Span::DUMMY,
            };

            // Now attempt to move instructions between a startopen and stopopen
            debug!("\tMoving statements");
            trace!("Max Round Depth {}", augmented_block2.max_round_depth());
            trace!("Max Blck Number {}", augmented_block2.max_blck_number());
            let mut cnt = 0;
            for blck in 0..=augmented_block2.max_blck_number() {
                trace!("blck {}", blck);
                for rnd_depth in 0..=augmented_block2.max_round_depth() {
                    cnt += 1;
                    //if (cnt==8) { return; }
                    let (start, stop) = augmented_block2.start_stop(rnd_depth, blck);
                    trace!(
                        "BLCK {}, RND {} : START {}, STOP {}, CNT {}",
                        blck,
                        rnd_depth,
                        start,
                        stop,
                        cnt,
                    );
                    // Check not two startopens with the same round depth and block
                    // and if one find all registers in the startopen.
                    // Keep the instructions which come before this startopen
                    let mut nstart = 0;
                    let mut startopen_pos = 0;
                    let mut active_registers = HashSet::new();
                    let mut instructions_keep = BTreeSet::new();
                    for i in start..=stop {
                        if augmented_block2.stmts[i].round_depth == rnd_depth
                            && augmented_block2.stmts[i].blck == blck
                        {
                            trace!(
                                "  {} # {} : {} {}",
                                i,
                                block.stmts[i].relex(cx).0.display(cx),
                                augmented_block2.stmts[i].round_depth,
                                augmented_block2.stmts[i].instruction_depth
                            );
                            if block.stmts[i].is_startopen() {
                                nstart += 1;
                                startopen_pos = i;
                                for x in block.stmts[i].read_registers(cx) {
                                    active_registers.insert(x.to_string());
                                }
                            }
                            if nstart == 0 {
                                instructions_keep.insert(i);
                            }
                        }
                    }
                    if nstart > 1 {
                        println!("Something wrongXXXXX");
                        continue 'blocks;
                    };
                    if nstart == 0 {
                        continue;
                    }
                    trace!("\trecursively find all instructions which touch the set");
                    trace!("\t{:?} ", active_registers);
                    // Now recursively find all instructions which touch these instructions
                    // and remove them from those under consideration, also remove any barrier
                    // instructions and stopopens
                    let mut repeat = true;
                    let mut mem_write = false;
                    while repeat {
                        trace!("\trepeat");
                        repeat = false;
                        let mut new_instructions_keep = instructions_keep.clone();
                        for i in instructions_keep.iter() {
                            let read_regs = block.stmts[*i].read_registers(cx);
                            let write_regs = block.stmts[*i].write_registers(cx);
                            let mut rr = HashSet::new();
                            let mut wr = HashSet::new();
                            for x in read_regs {
                                rr.insert(x.to_string());
                            }
                            for x in write_regs {
                                wr.insert(x.to_string());
                            }
                            // Deal with various instructions
                            if block.stmts[*i].memory_write(cx) {
                                mem_write = true;
                                repeat = true;
                                new_instructions_keep.remove(i);
                                active_registers.extend(rr.iter().cloned());
                            } else if block.stmts[*i].memory_read(cx) && mem_write {
                                repeat = true;
                                new_instructions_keep.remove(i);
                                active_registers.extend(wr.iter().cloned());
                                active_registers.extend(rr.iter().cloned());
                            } else if !active_registers.is_disjoint(&wr) {
                                repeat = true;
                                new_instructions_keep.remove(i);
                                active_registers.extend(rr.iter().cloned());
                            }
                            if block.stmts[*i].is_barrier(cx) {
                                repeat = true;
                                new_instructions_keep.remove(i);
                            }
                            if block.stmts[*i].is_stopopen() {
                                repeat = true;
                                new_instructions_keep.remove(i);
                            }
                        }
                        instructions_keep = new_instructions_keep;
                    }

                    trace!("\trepeat done");
                    trace!("\t{:?}", instructions_keep);
                    // Now move the kept instructions to after the startopen
                    let mut cache = Vec::with_capacity(instructions_keep.len());
                    let mut start_free = None;
                    for i in start..=startopen_pos {
                        if instructions_keep.contains(&i) {
                            trace!("\t\treserving {}", block.stmts[i].relex(cx).0.display(cx));
                            cache.push(std::mem::replace(&mut block.stmts[i], dummy.clone()));
                            if start_free.is_none() {
                                start_free = Some(i);
                            }
                        } else if let Some(start_free) = &mut start_free {
                            trace!("\t\tmoving {} -> {}", i, start_free);
                            block.stmts.swap(*start_free, i);
                            *start_free += 1;
                        }
                    }
                    if let Some(start_free) = start_free {
                        // put the `instructions_keep` elements back
                        block.stmts[start_free..=startopen_pos].swap_with_slice(&mut cache);
                    }
                    if !trace_span!("").is_disabled() {
                        for i in start..=startopen_pos {
                            if augmented_block2.stmts[i].round_depth == rnd_depth
                                && augmented_block2.stmts[i].blck == blck
                            {
                                let relexed = block.stmts[i].relex(cx);
                                trace!("  {} # {}", i, relexed.0.display(cx));
                            }
                        }
                    }
                }
            }
            debug!("\t\tFinished optimizing block");
        }
    }
}
