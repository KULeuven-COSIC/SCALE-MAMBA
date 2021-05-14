// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

//! Remove instructions that write to a register which is never read.

use std::collections::HashSet;

use documentation::name2instr;

use crate::asm::{Body, Instruction, Jump, JumpCondition, JumpMode, Terminator};
use crate::Compiler;

#[derive(Default)]
pub struct Pass;

impl super::Pass for Pass {
    fn apply<'a>(&mut self, cx: &'a Compiler, body: &mut Body<'a>) {
        let mut i = 0;
        loop {
            let span = debug_span!("dce", i);
            let _guard = span.enter();
            i += 1;
            let mut reads: HashSet<_> = HashSet::default();
            for block in body.blocks.iter() {
                for stmt in block.stmts.iter() {
                    reads.extend(stmt.read_registers(cx));
                }
                match &block.terminator.elem {
                    Terminator::Jump(Jump {
                        target_block: _,
                        mode,
                        comment: _,
                    }) => match mode {
                        JumpMode::Goto => {}
                        JumpMode::Conditional(JumpCondition {
                            fallthrough_block: _,
                            jump_if_equal: _,
                            constant: _,
                            register,
                        }) => {
                            reads.insert(register.elem.into());
                        }
                        JumpMode::Call { .. } => {}
                    },
                    Terminator::Restart { .. } => {}
                    Terminator::Crash { .. } => {}
                    Terminator::Return { .. } => {}
                    Terminator::Done => {}
                }
            }
            let mut changed = false;
            for block in body.blocks.iter_mut() {
                block.stmts.retain(|stmt| {
                    match &stmt.instr {
                        Instruction::General { instruction, .. } => match name2instr(instruction) {
                            None => return true,
                            // Barrier instructions have side effects, do not optimize them away
                            Some(instr) if instr.barrier || instruction.starts_with("new") => {
                                return true
                            }
                            Some(_) => {}
                        },
                        Instruction::Assign { .. } => {}
                        // Can't handle these yet
                        Instruction::StartOpen { .. } => return true,
                        Instruction::StopOpen { .. } => return true,
                        Instruction::DataInstr { .. } => return true,
                        Instruction::Io { .. } => return true,
                        Instruction::Nop => {}
                    }
                    let writes = stmt.write_registers(cx);
                    // If there are no write registers, this instruction exists just for its side effects.
                    // Otherwise we check wether any of the write registers are read again.
                    let retain =
                        writes.is_empty() || writes.into_iter().any(|r| reads.contains(&r));
                    if !retain && !cx.check_fuel_left() {
                        return true;
                    }
                    changed |= !retain;
                    if !retain {
                        trace!(instr = %stmt.relex(cx).0.display(cx), "dropping");
                    }
                    retain
                });
            }
            if !changed {
                break;
            }
        }
    }

    fn name(&self) -> &'static str {
        "dce"
    }
}
