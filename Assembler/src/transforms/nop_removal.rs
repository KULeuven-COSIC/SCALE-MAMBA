//! Remove all `nop` instructions

use crate::asm::{Body, Instruction};
use crate::Compiler;

#[derive(Default)]
pub struct Pass;

impl super::Pass for Pass {
    fn apply<'a>(&mut self, cx: &'a Compiler, body: &mut Body<'a>) {
        if cx.optimization_level == 0 {
            return;
        }
        for block in &mut body.blocks {
            block.stmts.retain(|elem| match elem.instr {
                Instruction::Nop => false,
                _ => true,
            });
        }
    }
    fn name(&self) -> &'static str {
        "nop_removal"
    }
}
