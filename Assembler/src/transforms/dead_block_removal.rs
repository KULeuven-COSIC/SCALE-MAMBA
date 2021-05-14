// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

//! Remove blocks that are never jumped into.

use crate::asm::Body;
use crate::visitor::Visitor;
use crate::Compiler;
use std::collections::{BTreeSet, HashMap, HashSet};

#[derive(Default)]
pub struct Pass;

impl super::Pass for Pass {
    fn apply<'a>(&mut self, cx: &'a Compiler, body: &mut Body<'a>) {
        if cx.optimization_level < 4 {
            return;
        }
        let mut referenced = HashMap::new();
        for (i, block) in body.blocks.iter_mut().enumerate() {
            trace!(i, ?block.terminator);
            struct DestinationFinder<'a, 'b> {
                cx: &'a Compiler,
                referenced: &'b mut HashMap<usize, HashSet<usize>>,
                current: usize,
            }
            impl<'a, 'b> Visitor<'a> for DestinationFinder<'a, 'b> {
                fn cx(&self) -> &'a Compiler {
                    &self.cx
                }
                fn visit_block_id(&mut self, id: &mut usize) {
                    self.referenced.entry(*id).or_default().insert(self.current);
                }
            }
            DestinationFinder {
                cx,
                referenced: &mut referenced,
                current: i,
            }
            .visit_block(block);
        }

        let mut to_remove = BTreeSet::new();
        // Repeat the inner loop until a fixed point.
        loop {
            trace!(?referenced, ?to_remove);
            let mut changed = false;
            // Find all blocks which have zero inputs and mark them for removal.
            // But do not look at block 0 which is the entry point
            for i in 1..body.blocks.len() {
                if let Some(sources) = referenced.get(&i) {
                    if !sources.is_empty() {
                        continue;
                    }
                }
                if !to_remove.insert(i) {
                    // Already marked this for removal
                    continue;
                }
                // Remove this block from all other blocks' list of incoming blocks.
                // If the list length drops to zero, future iterations will remove that
                // block, too.
                for sources in referenced.values_mut() {
                    sources.remove(&i);
                }
                changed = true;
            }
            if !changed {
                break;
            }
        }
        trace!(?to_remove);

        let mut remappings = Vec::new();
        let mut n = 0;
        let mut last_i = 0;
        for &i in &to_remove {
            trace!(i, last_i, n, ?remappings);
            let iter = (n..).take(i - last_i);
            remappings.extend(iter);
            // Insert dummy remapping for blocks that will be removed
            remappings.push(usize::max_value());
            n += i - last_i;
            last_i = i + 1;
        }
        trace!(last_i, n, ?remappings);
        remappings.extend((n..).take(body.blocks.len() - remappings.len()));
        trace!(?remappings);

        // Remove blocks.
        for &i in to_remove.iter().rev() {
            body.blocks.remove(i);
        }

        BlockRemapper { cx, remappings }.visit_body(body);
    }
    fn name(&self) -> &'static str {
        "dead_block_removal"
    }
}

// Remap ids since the blocks have been shifted.
pub struct BlockRemapper<'a> {
    pub cx: &'a Compiler,
    pub remappings: Vec<usize>,
}

impl<'a, 'b> Visitor<'a> for BlockRemapper<'a> {
    fn cx(&self) -> &'a Compiler {
        &self.cx
    }
    fn visit_block_id(&mut self, id: &mut usize) {
        *id = self.remappings[*id];
    }
}
