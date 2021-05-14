// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

//! Replace
//!
//! ```asm
//! mov r1, r2
//! some_instr r1
//! ```
//!
//! with
//!
//! ```asm
//! some_instr r2
//! ```

use std::collections::{HashMap, HashSet};

use crate::{
    asm::Body,
    asm::JumpMode,
    asm::Terminator,
    lexer::{Operand, Register},
};
use crate::{asm::Instruction, Compiler};

#[derive(Default)]
pub struct Pass;

impl super::Pass for Pass {
    fn apply<'a>(&mut self, cx: &'a Compiler, body: &mut Body<'a>) {
        #[derive(Copy, Clone)]
        struct MultipleAssignments;
        // Count how many times a register is written to
        let mut writes: HashMap<Register, Result<Operand, MultipleAssignments>> = HashMap::new();
        for block in &mut body.blocks {
            for stmt in &mut block.stmts {
                // Record all writes that we can eliminate if there are no further writes to the same register
                match stmt.instr {
                    Instruction::Assign { destination, value } => {
                        writes
                            .entry(destination.elem)
                            .and_modify(|entry| *entry = entry.and(Err(MultipleAssignments)))
                            .or_insert(Ok(value.elem));
                    }
                    _ => {
                        // Treat all other assignments as blocking this optimization for now.
                        for reg in stmt.write_registers(cx) {
                            writes.insert(reg, Err(MultipleAssignments));
                        }
                        // Treat all reads as blocking this optimization if they are part of a vector operation
                        if stmt.vectorized.elem.get() > 1 {
                            for reg in stmt.read_registers(cx) {
                                writes.insert(reg, Err(MultipleAssignments));
                            }
                        }
                    }
                }
            }
        }

        // In-block
        for block in &mut body.blocks {
            for stmt in &mut block.stmts {
                // If an instruction wrote to one of our inputs, we just replace our input
                // with whatever was read from.
                match &mut stmt.instr {
                    Instruction::General { values, .. } => {
                        for op in values {
                            if let Operand::Register(reg) = &mut op.elem {
                                if let Some(Ok(Operand::Register(val))) = writes.get(reg) {
                                    *reg = *val;
                                }
                            }
                        }
                    }
                    Instruction::StartOpen { registers } => {
                        for reg in registers {
                            if let Some(Ok(Operand::Register(val))) = writes.get(&reg.elem.into()) {
                                *reg = reg.span.with(*val).require(cx);
                            }
                        }
                    }
                    _ => {}
                }
            }
            match &mut block.terminator.elem {
                Terminator::Jump(jmp) => match &mut jmp.mode {
                    JumpMode::Goto => {}
                    JumpMode::Conditional(cnd) => {
                        let reg = &mut cnd.register;
                        if let Some(Ok(Operand::Register(val))) = writes.get(&reg.elem.into()) {
                            *reg = reg.span.with(*val).require(cx);
                        }
                    }
                    JumpMode::Call { .. } => {}
                },
                Terminator::Restart { .. } => {}
                Terminator::Crash { .. } => {}
                Terminator::Return { .. } => {}
                Terminator::Done => {}
            }
        }

        // Count how many times a register is read from
        let mut reads = HashSet::new();
        for block in &mut body.blocks {
            for stmt in &mut block.stmts {
                // Record all registers used by this instruction
                for reg in stmt.read_registers(cx) {
                    reads.insert(reg);
                }
            }
            // Record the register read by this branch
            match &block.terminator.elem {
                Terminator::Jump(jmp) => match &jmp.mode {
                    JumpMode::Goto => {}
                    JumpMode::Conditional(cnd) => {
                        reads.insert(cnd.register.elem.into());
                    }
                    JumpMode::Call { .. } => {}
                },
                Terminator::Restart { .. } => {}
                Terminator::Crash { .. } => {}
                Terminator::Return { .. } => {}
                Terminator::Done => {}
            }
        }

        // Remove all assignments to registers that are never read
        for block in &mut body.blocks {
            block.stmts.retain(|stmt| match stmt.instr {
                Instruction::Assign { destination, .. } => reads.contains(&destination.elem),
                _ => true,
            })
        }
    }
    fn name(&self) -> &'static str {
        "assignment_chain"
    }
}
