//! Replace
//!
//! ```asm
//! addint r1, r2, r3
//! mov r4, r1
//! ```
//!
//! with
//!
//! ```asm
//! addint r4, r2, r3
//! ```

use std::collections::{hash_map::Entry, HashMap};

use crate::{asm::Instruction, Compiler};
use crate::{
    asm::{Body, Terminator},
    lexer::{Operand, Register},
};

#[derive(Default)]
pub struct Pass;

impl super::Pass for Pass {
    fn apply<'a>(&mut self, cx: &'a Compiler, body: &mut Body<'a>) {
        struct MultipleReads;
        let mut single_read_registers: HashMap<Register, Result<(), MultipleReads>> =
            HashMap::new();
        for block in &body.blocks {
            for stmt in &block.stmts {
                if let Instruction::Assign { value, .. } = stmt.instr {
                    if let Operand::Register(value) = value.elem {
                        match single_read_registers.entry(value) {
                            Entry::Occupied(mut entry) => {
                                let _ = entry.insert(Err(MultipleReads));
                            }
                            Entry::Vacant(entry) => {
                                entry.insert(Ok(()));
                            }
                        }
                    }
                } else {
                    for reg in stmt.read_registers(cx) {
                        let _ = single_read_registers.insert(reg, Err(MultipleReads));
                    }
                }
            }
            match &block.terminator.elem {
                Terminator::Jump(jmp) => match &jmp.mode {
                    crate::asm::JumpMode::Goto => {}
                    crate::asm::JumpMode::Conditional(cond) => {
                        single_read_registers.insert(cond.register.elem.into(), Err(MultipleReads));
                    }
                    crate::asm::JumpMode::Call { .. } => {}
                },
                Terminator::Restart { .. } => {}
                Terminator::Crash { .. } => {}
                Terminator::Return { .. } => {}
                Terminator::Done => {}
            }
        }

        // The actual optimization runs only within a block for now, handling multi-block
        // is not easy to do.
        for block in body.blocks.iter_mut() {
            // We look for an operation that outputs to a single-use register and remember that
            // in `candidates`. If there is a read or write to the register that we would use as
            // a replacement, we cancel the optimization. This is to prevent
            // ```rust
            // addint r1, r2, r3
            // mov r5, r4
            // mov r4, r1
            // ```
            // which would end up having the wrong value for `r5` if we assigned to `r4` during
            // the addition operation.
            let mut candidates: HashMap<Register, usize> = HashMap::new();
            let mut optimizations = Vec::new();
            for (stmt_id, stmt) in block.stmts.iter_mut().enumerate() {
                let span = trace_span!("statement", stmt = %stmt.relex(cx).0.display(cx));
                let _guard = span.enter();
                if let Instruction::Assign { destination, value } = stmt.instr {
                    if let Operand::Register(reg) = value.elem {
                        if let Some(pos) = candidates.remove(&reg) {
                            // A candidate is now upgraded to be an optimization.
                            optimizations.push((pos, stmt_id, destination.elem, reg));
                        }
                    }
                }
                // Clear anything touching a current candidate.
                for reg in stmt
                    .read_registers(cx)
                    .into_iter()
                    .chain(stmt.write_registers(cx))
                {
                    candidates.remove(&reg);
                }
                for reg in stmt.write_registers(cx) {
                    if let Some(Ok(())) = single_read_registers.get(&reg) {
                        candidates.insert(reg, stmt_id);
                    }
                }
            }
            for (opt, remove, new, old) in optimizations {
                block.stmts[remove].instr = Instruction::Nop;
                block.stmts[opt].replace_registers(cx, |r| if r == old { new } else { r });
            }
            block.stmts.retain(|s| !matches!(s.instr, Instruction::Nop));
        }
    }
    fn name(&self) -> &'static str {
        "destination_propagation"
    }
}
