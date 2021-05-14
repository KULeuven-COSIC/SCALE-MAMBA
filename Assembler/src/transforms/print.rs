// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

//! A dummy optimization printing the basic block data structures

use crate::asm::Body;
use crate::Compiler;

#[derive(Default)]
pub struct Pass;

impl super::Pass for Pass {
    fn name(&self) -> &'static str {
        "print_structure"
    }

    #[allow(clippy::cognitive_complexity)]
    fn apply<'a>(&mut self, cx: &'a Compiler, body: &mut Body<'a>) {
        if trace_span!("").is_disabled() {
            return;
        }
        for (block_id, block) in body.blocks.iter().enumerate() {
            trace!("# Block {}", block_id);
            for stmt in block.stmts.iter() {
                let relexed = stmt.relex(cx);
                trace!("    # {}", relexed.0.display(cx));
                trace!("        # write: {:?}", stmt.write_registers(cx));
                trace!("        #  read: {:?}", stmt.read_registers(cx));
                trace!("        #  barrier: {:?}", stmt.is_barrier(cx));
                trace!(
                    "        #  memory (R/W): {:?}/{:?}",
                    stmt.memory_read(cx),
                    stmt.memory_write(cx)
                );
            }
            trace!("    # {:?}", block.terminator.elem);
        }
    }
}
