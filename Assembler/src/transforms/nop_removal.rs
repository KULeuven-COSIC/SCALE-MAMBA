//! Remove all `nop` instructions

use crate::Compiler;
use crate::{
    asm::{Body, Instruction},
    lexer::Operand,
};

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
                // Assignments from a register to itself are useless
                Instruction::Assign { destination, value } => {
                    Operand::Register(destination.elem) != value.elem
                }
                _ => true,
            });
        }
    }
    fn name(&self) -> &'static str {
        "nop_removal"
    }
}
