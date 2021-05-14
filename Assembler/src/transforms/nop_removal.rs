// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

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
