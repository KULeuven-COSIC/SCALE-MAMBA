// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

//! A dummy optimization not doing anything

use std::collections::HashMap;

use crate::Compiler;
use crate::{
    asm::{Body, Instruction},
    lexer::Operand,
};

#[derive(Default)]
pub struct Pass;

impl super::Pass for Pass {
    fn apply<'a>(&mut self, cx: &'a Compiler, body: &mut Body<'a>) {
        for block in &mut body.blocks {
            let mut constants = HashMap::new();
            for stmt in &mut block.stmts {
                match &mut stmt.instr {
                    Instruction::General {
                        instruction,
                        destinations,
                        values,
                    } if *instruction == "print_char_regint" => {
                        assert!(destinations.is_empty());
                        assert_eq!(values.len(), 1);
                        if let Operand::Register(reg) = values[0].elem {
                            trace!(?reg);
                            if let Some(&val) = constants.get(&reg) {
                                values[0].elem = Operand::Value(val);
                                *instruction = "print_char";
                            }
                        }
                    }
                    Instruction::Assign { destination, value } => {
                        if let Operand::Value(v) = value.elem {
                            trace!(?destination.elem, ?value.elem);
                            constants.insert(destination.elem, v);
                        } else {
                            let _ = constants.remove(&destination.elem);
                        }
                    }
                    _ => {
                        for write in stmt.write_registers(cx) {
                            let _ = constants.remove(&write);
                        }
                    }
                }
            }
        }
    }
    fn name(&self) -> &'static str {
        "print_merge"
    }
}
