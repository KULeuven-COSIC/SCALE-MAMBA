// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

//! Remove blocks that are unconditionally jumped into from a single other block by moving all instructions
//! from to the single other block.

use crate::Compiler;
use crate::{
    asm::{Body, Jump, JumpMode, Terminator},
    span::Span,
};
use std::collections::{hash_map::Entry, HashMap};

#[derive(Default)]
pub struct Pass;

impl super::Pass for Pass {
    fn apply<'a>(&mut self, cx: &'a Compiler, body: &mut Body<'a>) {
        if cx.optimization_level < 4 {
            return;
        }
        // Find all blocks with a single caller.
        // The map contains `None` if a second caller was found.
        let mut callers: HashMap<usize, Option<usize>> = HashMap::new();
        // The first block always has a caller and further callers should not act on it.
        callers.insert(0, None);
        for (i, block) in body.blocks.iter().enumerate() {
            match &block.terminator.elem {
                Terminator::Jump(jump) => match &jump.mode {
                    JumpMode::Goto => match callers.entry(jump.target_block) {
                        Entry::Vacant(vac) => {
                            vac.insert(Some(i));
                        }
                        Entry::Occupied(mut occ) => {
                            occ.insert(None);
                        }
                    },
                    JumpMode::Conditional(cond) => {
                        callers.insert(cond.fallthrough_block, None);
                        callers.insert(jump.target_block, None);
                    }
                    JumpMode::Call { fallthrough_block } => {
                        callers.insert(jump.target_block, None);
                        callers.insert(*fallthrough_block, None);
                    }
                },
                Terminator::Restart { .. }
                | Terminator::Crash { .. }
                | Terminator::Return { .. }
                | Terminator::Done => {}
            }
        }

        trace!(?callers);

        while let Some(mut block) = callers.keys().next().copied() {
            if let Some(caller) = callers.remove(&block).unwrap() {
                loop {
                    trace!(block, caller);
                    let stmts = std::mem::take(&mut body.blocks[block].stmts);
                    body.blocks[caller].stmts.extend(stmts);
                    let terminator = std::mem::replace(
                        &mut body.blocks[block].terminator,
                        Terminator::Crash {
                            comment: Span::DUMMY,
                        }
                        .into(),
                    );
                    // If the next block is also being collapsed, do this immediately and remove it from the
                    // list of blocks to be processed.
                    if let Terminator::Jump(Jump {
                        mode: JumpMode::Goto,
                        target_block,
                        ..
                    }) = terminator.elem
                    {
                        if let Some(Some(_)) = callers.remove(&target_block) {
                            block = target_block;
                            continue;
                        }
                    }
                    body.blocks[caller].terminator = terminator;
                    break;
                }
            }
        }
    }
    fn name(&self) -> &'static str {
        "goto_chain_collapse"
    }
}
