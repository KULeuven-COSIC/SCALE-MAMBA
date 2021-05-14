// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

//! Move blocks with `Done` terminators to the end of the assembly.
//! This is just a convenience thing making the assembly less suprising.

// FIXME: make this pass a full reordering pass so the assembly and the
// linear control flow match up, reducing forward/backward jumping.

use super::dead_block_removal::BlockRemapper;
use crate::asm::{Body, Terminator};
use crate::visitor::Visitor;
use crate::Compiler;
use std::collections::BTreeMap;

#[derive(Default)]
pub struct Pass;

impl super::Pass for Pass {
    fn apply<'a>(&mut self, cx: &'a Compiler, body: &mut Body<'a>) {
        if cx.optimization_level < 4 {
            return;
        }

        let mut to_move = Vec::new();
        for (i, block) in body.blocks.drain(..).enumerate() {
            // Never move the start block, and only move blocks with done terminators
            let move_it = i != 0 && matches!(block.terminator.elem, Terminator::Done);
            to_move.push((i, move_it, block));
        }
        for (i, b, _) in &to_move {
            trace!(i, b);
        }
        to_move.sort_by_key(|&(_, b, _)| b);
        for (i, b, _) in &to_move {
            trace!(i, b);
        }
        let remappings: BTreeMap<_, _> = to_move
            .iter()
            .enumerate()
            .map(|(new, (old, _, _))| (old, new))
            .collect();
        let remappings: Vec<_> = remappings.values().copied().collect();
        trace!(?remappings);

        // Move blocks.
        body.blocks
            .extend(to_move.into_iter().map(|(_, _, block)| block));

        BlockRemapper { cx, remappings }.visit_body(body);
    }
    fn name(&self) -> &'static str {
        "move_done_block_to_end"
    }
}
