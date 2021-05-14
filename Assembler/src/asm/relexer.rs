// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

//! Takes an asm `Body` and generates `Vec<Lexical>`s from it.

use super::{Body, Instruction, IoInstruction, JumpMode, Statement, Terminator};
use crate::lexer::{Const, Lexical, Operand, Register};
use crate::span::Spanned;
use crate::Compiler;

use std::collections::BTreeMap;
use std::convert::{TryFrom, TryInto};

impl<'a> Body<'a> {
    pub fn relex(&'a self, cx: &'a Compiler) -> Vec<Lexical<'a>> {
        let mut lexicals = Vec::new();
        let mut block_starts: Vec<i32> = Vec::new();
        let mut forward_jumps = BTreeMap::new();
        debug!(num_blocks = self.blocks.len());
        for (i, block) in self.blocks.iter().enumerate() {
            trace!(num_blocks = block_starts.len(), num_lexicals = lexicals.len(), ?block.terminator.elem);
            block_starts.push(lexicals.len().try_into().unwrap());
            for statement in &block.stmts {
                let (lexical, call_dest) = statement.relex(cx);
                if let Some(call_dest) = call_dest {
                    forward_jumps.insert(lexicals.len(), call_dest);
                }
                lexicals.push(lexical);
            }
            match &block.terminator.elem {
                Terminator::Done => {
                    if i != self.blocks.len() - 1 {
                        forward_jumps.insert(lexicals.len(), self.blocks.len());
                        // We are not at the end of the last block, so we jump to the end.
                        // We do this by using an invalid instruction and replacing it with a
                        // jmp to the end later.
                        lexicals.push(Lexical {
                            instruction: "jmp",
                            args: vec![block.terminator.span.with(Operand::Value(Const::Int(0)))],
                            comment: block.terminator.span.end(),
                            span: block.terminator.span,
                        });
                    }
                }
                Terminator::Restart { comment } => lexicals.push(Lexical {
                    instruction: "restart",
                    args: vec![],
                    comment: *comment,
                    span: block.terminator.span,
                }),
                Terminator::Return { comment } => lexicals.push(Lexical {
                    instruction: "return",
                    args: vec![],
                    comment: *comment,
                    span: block.terminator.span,
                }),
                Terminator::Crash { comment } => lexicals.push(Lexical {
                    instruction: "crash",
                    args: vec![],
                    comment: *comment,
                    span: block.terminator.span,
                }),
                Terminator::Jump(jump) => {
                    let mut jump_offset = |block_id, pos| match block_starts.get(block_id) {
                        None => {
                            forward_jumps.insert(pos, block_id);
                            // use an explicitly wrong value for now, it will be replaced later
                            0
                        }
                        Some(destination) => destination - i32::try_from(pos).unwrap() - 1,
                    };
                    let (instruction, fallthrough) = match &jump.mode {
                        JumpMode::Call { fallthrough_block } => {
                            let offset = jump_offset(jump.target_block, lexicals.len());
                            lexicals.push(Lexical {
                                instruction: "call",
                                args: vec![block.terminator.span.with(offset.into())],
                                comment: jump.comment,
                                span: block.terminator.span,
                            });
                            ("jmp", *fallthrough_block)
                        }
                        JumpMode::Goto => ("jmp", jump.target_block),
                        JumpMode::Conditional(cond) => {
                            let instruction = if cond.jump_if_equal { "jmpeq" } else { "jmpne" };
                            let offset = jump_offset(jump.target_block, lexicals.len());
                            lexicals.push(Lexical {
                                instruction,
                                args: vec![
                                    cond.register.map(Operand::from),
                                    cond.constant.map(Operand::from),
                                    block.terminator.span.with(offset.into()),
                                ],
                                comment: jump.comment,
                                span: block.terminator.span,
                            });
                            ("jmp", cond.fallthrough_block)
                        }
                    };
                    // in case we jump to the next block, we don't need to emit a jump instruction.
                    if fallthrough != i + 1 {
                        lexicals.push(Lexical {
                            instruction,
                            args: vec![block
                                .terminator
                                .span
                                .with(jump_offset(fallthrough, lexicals.len()).into())],
                            comment: jump.comment,
                            span: block.terminator.span,
                        });
                    }
                }
            }
        }
        for (pos, block_id) in forward_jumps {
            let arg_idx = match &*lexicals[pos].instruction {
                "call" | "jmp" => 0,
                // other jumps have a register and offset first
                _ => 2,
            };
            let block_pos = if block_id == block_starts.len() {
                lexicals.len().try_into().unwrap()
            } else {
                block_starts[block_id]
            };
            lexicals[pos].args[arg_idx].elem =
                Operand::Value(Const::Int(block_pos - i32::try_from(pos).unwrap() - 1));
        }
        lexicals
    }
}

impl<'a> Statement<'a> {
    pub fn relex(&'a self, cx: &'a Compiler) -> (Lexical<'a>, Option<usize>) {
        let call_dest = None;
        let (instruction, args): (_, Vec<Spanned<Operand>>) = match &self.instr {
            Instruction::General {
                instruction,
                destinations,
                values,
            } => (
                *instruction,
                destinations
                    .iter()
                    .map(|x| x.map(Into::into))
                    .chain(values.iter().map(|x| x.map(Into::into)))
                    .collect(),
            ),
            // ld*
            Instruction::Assign { destination, value } => (
                match (destination.elem, value.elem) {
                    (Register::Clear(_), Operand::Value(_)) => "ldi",
                    (Register::Regint(_), Operand::Value(_)) => "ldint",
                    (Register::Secret(_), Operand::Value(_)) => "ldsi",
                    (Register::SecretRegint(_), Operand::Value(_)) => "ldsint",
                    (Register::SecretBit(_), Operand::Value(_)) => "ldsbit",
                    (Register::Clear(_), Operand::Register(_)) => "movc",
                    (Register::Regint(_), Operand::Register(_)) => "movint",
                    (Register::Secret(_), Operand::Register(_)) => "movs",
                    (Register::SecretRegint(_), Operand::Register(_)) => "movsint",
                    (Register::SecretBit(_), Operand::Register(_)) => "movsb",
                },
                vec![destination.clone().map(Into::into), *value],
            ),
            Instruction::StartOpen { registers } => (
                "startopen",
                std::iter::once(self.span.with(Operand::Value(Const::Int(
                    registers.len().try_into().unwrap(),
                ))))
                .chain(registers.iter().map(|x| x.map(Into::into)))
                .collect(),
            ),
            Instruction::StopOpen { registers } => (
                "stopopen",
                std::iter::once(self.span.with(Operand::Value(Const::Int(
                    registers.len().try_into().unwrap(),
                ))))
                .chain(registers.iter().map(|x| x.map(Into::into)))
                .collect(),
            ),
            Instruction::DataInstr {
                instruction,
                registers,
            } => (
                *instruction,
                std::iter::once(self.span.with(Operand::Value(Const::Int(
                    registers.len().try_into().unwrap(),
                ))))
                .chain(registers.iter().map(|x| x.map(Into::into)))
                .collect(),
            ),
            Instruction::Io { instr, channel } => match instr {
                IoInstruction::InputShares { registers } => (
                    "input_shares",
                    std::iter::once(self.span.with(registers.len() + 1).into_operand(cx))
                        .chain(std::iter::once(channel.into_operand(cx)))
                        .chain(registers.iter().copied().map(Into::into))
                        .collect(),
                ),
                IoInstruction::OutputShares { registers } => (
                    "output_shares",
                    std::iter::once(self.span.with(registers.len() + 1).into_operand(cx))
                        .chain(std::iter::once(channel.into_operand(cx)))
                        .chain(registers.iter().copied().map(Into::into))
                        .collect(),
                ),
            },
            Instruction::Nop => ("nop", vec![]),
        };
        let (instruction, args) = if self.vectorized.elem.get() > 1 {
            let mut args = args;
            args.insert(
                0,
                self.span.with(Operand::Value(Const::Int(
                    self.vectorized.elem.get().try_into().unwrap(),
                ))),
            );
            (cx.strings.push_get(format!("v{}", instruction)), args)
        } else {
            (instruction, args)
        };
        (
            Lexical {
                span: self.span,
                instruction,
                args,
                comment: self.comment,
            },
            call_dest,
        )
    }
}
