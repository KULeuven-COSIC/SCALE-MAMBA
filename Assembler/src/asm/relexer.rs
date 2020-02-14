//! Takes an asm `Body` and generates `Vec<Lexical>`s from it.

use super::{
    Body, Instruction, IoInstruction, JumpMode, MemoryBank, Statement, Terminator, UnaryOperation,
};
use crate::lexer::{Const, Lexical, Operand, Register, RegisterKind};
use crate::span::Spanned;
use crate::Compiler;

use std::collections::BTreeMap;
use std::convert::{TryFrom, TryInto};

impl<'a> Body<'a> {
    pub fn relex(&'a self, cx: &'a Compiler) -> Vec<Lexical<'a>> {
        let mut lexicals = Vec::new();
        let mut block_starts: Vec<i32> = Vec::new();
        let mut forward_jumps = BTreeMap::new();
        for (i, block) in self.blocks.iter().enumerate() {
            //println!("#{:?} {:?}", lexicals.len(), block.terminator.elem);
            block_starts.push(lexicals.len().try_into().unwrap());
            for statement in &block.stmts {
                lexicals.push(statement.relex(cx));
            }
            match &block.terminator.elem {
                // FIXME: generate a jump to the end in case this is not the final block
                Terminator::Done => assert_eq!(i, self.blocks.len() - 1),
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
                        JumpMode::Call => ("call", jump.target_block),
                        JumpMode::Goto => ("jmp", jump.target_block),
                        JumpMode::Conditional(cond) => {
                            let instruction = if cond.jump_if_zero { "jmpeqz" } else { "jmpnz" };
                            let offset = jump_offset(jump.target_block, lexicals.len());
                            lexicals.push(Lexical {
                                instruction,
                                args: vec![
                                    cond.register.map(Operand::Register),
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
                "jmp" => 0,
                // other jumps have a register first
                _ => 1,
            };
            lexicals[pos].args[arg_idx].elem = Operand::Value(Const::Int(
                block_starts[block_id] - i32::try_from(pos).unwrap() - 1,
            ));
        }
        lexicals
    }
}

impl<'a> Statement<'a> {
    pub fn relex(&'a self, cx: &'a Compiler) -> Lexical<'a> {
        let (instruction, args): (_, Vec<Spanned<'a, Operand>>) = match &self.instr {
            Instruction::GenerateTriple { destinations } => (
                "triple",
                destinations.iter().map(|x| x.map(Into::into)).collect(),
            ),
            Instruction::GenerateBit { destination } => {
                ("bit", vec![destination.clone().map(Into::into)])
            }
            Instruction::GenerateSquare { destinations } => (
                "square",
                destinations.iter().map(|x| x.map(Into::into)).collect(),
            ),
            Instruction::Mul2Sint {
                destinations,
                values,
            } => (
                "mul2sint",
                destinations
                    .iter()
                    .map(|x| x.map(Into::into))
                    .chain(values.iter().map(|x| x.map(Into::into)))
                    .collect(),
            ),
            Instruction::SintBit { destination, args } => (
                "sintbit",
                vec![
                    destination.clone().map(Into::into),
                    args[0],
                    args[1],
                    args[2],
                ],
            ),
            // ld*
            Instruction::Assign { destination, value } => (
                match (destination.elem, value.elem) {
                    (Register::Clear(_), Operand::Value(_)) => "ldi",
                    (Register::Regint(_), Operand::Value(_)) => "ldint",
                    (Register::Secret(_), Operand::Value(_)) => "ldsi",
                    (Register::SecretRegint(_), Operand::Value(_)) => "ldsint",
                    (Register::Clear(_), Operand::Register(_)) => "movc",
                    (Register::Regint(_), Operand::Register(_)) => "movint",
                    (Register::Secret(_), Operand::Register(_)) => "movs",
                    (Register::SecretRegint(_), Operand::Register(_)) => "movsint",
                    (Register::SecretBit(_), _) => unimplemented!(),
                },
                vec![destination.clone().map(Into::into), *value],
            ),
            Instruction::LoadFromMemory {
                destination,
                memory_bank,
                source,
            } => (
                match (destination.elem, memory_bank, source.elem) {
                    (Register::Clear(_), MemoryBank::Clear, Operand::Value(_)) => "ldmc",
                    (Register::Secret(_), MemoryBank::Secret, Operand::Value(_)) => "ldms",
                    (Register::Regint(_), MemoryBank::Regint, Operand::Value(_)) => "ldmint",
                    (Register::SecretRegint(_), MemoryBank::SecretRegint, Operand::Value(_)) => {
                        "ldmsint"
                    }

                    (Register::Clear(_), MemoryBank::Regint, Operand::Register(_)) => "ldmci",
                    (Register::Regint(_), MemoryBank::Regint, Operand::Register(_)) => "ldminti",
                    (Register::Secret(_), MemoryBank::Secret, Operand::Register(_))
                    | (Register::Secret(_), MemoryBank::SecretRegint, Operand::Register(_)) => {
                        "ldmsi"
                    }
                    (Register::SecretRegint(_), MemoryBank::SecretRegint, Operand::Register(_)) => {
                        "ldmsinti"
                    }
                    (Register::Clear(_), MemoryBank::Clear, Operand::Register(_)) => "ldmci",
                    _ => unimplemented!("no instruction for {:#?}", self),
                },
                vec![destination.clone().map(Into::into), *source],
            ),
            Instruction::StoreToMemory {
                destination,
                memory_bank,
                value,
            } => (
                match (memory_bank, value.elem, destination.elem) {
                    (MemoryBank::Clear, Register::Clear(_), Operand::Value(_)) => "stmc",
                    (MemoryBank::Secret, Register::Secret(_), Operand::Value(_)) => "stms",
                    (MemoryBank::Regint, Register::Regint(_), Operand::Value(_)) => "stmint",
                    (MemoryBank::SecretRegint, Register::SecretRegint(_), Operand::Value(_)) => {
                        "stmsint"
                    }

                    (MemoryBank::Clear, Register::Clear(_), Operand::Register(_))
                    | (MemoryBank::Clear, Register::Regint(_), Operand::Register(_)) => "stmci",
                    (MemoryBank::Regint, Register::Regint(_), Operand::Register(_)) => "stminti",
                    (MemoryBank::Secret, Register::Secret(_), Operand::Register(_)) => "stmsi",
                    (MemoryBank::SecretRegint, Register::SecretRegint(_), Operand::Register(_)) => {
                        "stmsinti"
                    }
                    _ => unimplemented!("no instruction for {:#?}", self),
                },
                vec![value.clone().map(Into::into), *destination],
            ),
            Instruction::BinaryOperation {
                instruction,
                a,
                b,
                destination,
                ..
            } => (
                instruction,
                vec![destination.clone().map(Into::into), *a, *b],
            ),
            Instruction::UnaryOperation { a, destination, op } => (
                match (op, destination.elem) {
                    (UnaryOperation::Legendre, Register::Clear(_)) => "legendrec",
                    (UnaryOperation::Digestc, Register::Clear(_)) => "digestc",
                    (UnaryOperation::Neg, Register::SecretRegint(_)) => "neg",
                    (UnaryOperation::Neg, Register::SecretBit(_)) => "negb",
                    (UnaryOperation::Inv, Register::SecretRegint(_)) => "invsint",
                    (UnaryOperation::Lt, Register::Regint(_)) => "ltzint",
                    (UnaryOperation::Lt, Register::SecretBit(_)) => "ltzsint",
                    (UnaryOperation::Eq, Register::Regint(_)) => "eqzint",
                    (UnaryOperation::Eq, Register::SecretBit(_)) => "eqzsint",
                    (UnaryOperation::Rand, Register::Regint(_)) => "rand",
                    _ => unreachable!("{:?}, {:?}", op, destination),
                },
                vec![destination.clone().map(Into::into), *a],
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
            Instruction::PrintI { instruction, arg } => {
                (instruction, vec![arg.clone().map(Into::into)])
            }
            Instruction::PrintR { instruction, arg } => {
                (instruction, vec![arg.clone().map(Into::into)])
            }
            Instruction::PrintFL { args } => (
                "print_float",
                args.iter().map(|x| x.map(Into::into)).collect(),
            ),
            Instruction::PrintFix { args } => (
                "print_fix",
                args.iter().map(|x| x.map(Into::into)).collect(),
            ),
            Instruction::Io { instr, channel } => match instr {
                IoInstruction::OpenChan { destination } => (
                    "open_channel",
                    vec![destination.map(Into::into), channel.into_operand(cx)],
                ),
                IoInstruction::CloseChan => ("close_channel", vec![channel.into_operand(cx)]),
                IoInstruction::PrivateInput {
                    destination,
                    player,
                } => (
                    "private_input",
                    vec![
                        destination.into_operand(cx),
                        player.into_operand(cx),
                        channel.into_operand(cx),
                    ],
                ),
                IoInstruction::PrivateOutput { value, player } => (
                    "private_output",
                    vec![
                        value.into_operand(cx),
                        player.into_operand(cx),
                        channel.into_operand(cx),
                    ],
                ),
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
                IoInstruction::OutputClear { value } => (
                    "output_clear",
                    vec![value.map(Into::into), channel.into_operand(cx)],
                ),
                IoInstruction::InputClear { destination } => (
                    "input_clear",
                    vec![
                        destination.map(Into::into),
                        channel.clone().into_operand(cx),
                    ],
                ),
                IoInstruction::OutputInt { value } => (
                    "output_int",
                    vec![value.map(Into::into), channel.into_operand(cx)],
                ),
                IoInstruction::InputInt { destination } => (
                    "input_int",
                    vec![destination.map(Into::into), channel.into_operand(cx)],
                ),
            },
            Instruction::PushR { value } => (
                match value.elem {
                    Register::Regint(_) => "pushint",
                    Register::Clear(_) => "pushc",
                    Register::Secret(_) => "pushs",
                    Register::SecretRegint(_) => "pushsint",
                    Register::SecretBit(_) => "pushsbit",
                },
                vec![value.clone().map(Into::into)],
            ),
            Instruction::PopR { destination } => (
                match destination.elem {
                    Register::Regint(_) => "popint",
                    Register::Clear(_) => "popc",
                    Register::Secret(_) => "pops",
                    Register::SecretRegint(_) => "popsint",
                    Register::SecretBit(_) => "popsbit",
                },
                vec![destination.clone().map(Into::into)],
            ),
            Instruction::PokeR { value1, value2 } => (
                match value2.elem {
                    Register::Regint(_) => "pokeint",
                    Register::Clear(_) => "pokec",
                    Register::Secret(_) => "pokes",
                    Register::SecretRegint(_) => "pokesint",
                    Register::SecretBit(_) => "pokesbit",
                },
                vec![
                    value1.clone().map(Into::into),
                    value2.clone().map(Into::into),
                ],
            ),
            Instruction::PeekR { destination, value } => (
                match destination.elem {
                    Register::Regint(_) => "peekint",
                    Register::Clear(_) => "peekc",
                    Register::Secret(_) => "peeks",
                    Register::SecretRegint(_) => "peeksint",
                    Register::SecretBit(_) => "peeksbit",
                },
                vec![
                    destination.clone().map(Into::into),
                    value.clone().map(Into::into),
                ],
            ),
            Instruction::GetSP { destination, itype } => (
                match itype {
                    RegisterKind::Clear => "getspc",
                    RegisterKind::Secret => "getsps",
                    RegisterKind::Regint => "getspint",
                    RegisterKind::SecretRegint => "getspsint",
                    RegisterKind::SecretBit => "getspsbit",
                },
                vec![destination.clone().map(Into::into)],
            ),
            Instruction::Convert {
                value,
                destination,
                modp,
                unsignedint,
            } => (
                match (destination.elem, value.elem, unsignedint) {
                    (Register::Clear(_), Register::Regint(_), false) => "convint",
                    (Register::Regint(_), Register::Clear(_), false) => "convmodp",
                    (Register::Secret(_), Register::SecretRegint(_), false) => "convsregsint",
                    (Register::Secret(_), Register::SecretRegint(_), true) => "convsuregsint",
                    (Register::SecretRegint(_), Register::Regint(_), false) => "convregsreg",
                    (Register::SecretRegint(_), Register::Secret(_), false) => "convsintsreg",
                    _ => unimplemented!(),
                },
                [destination, value]
                    .iter()
                    .map(|reg| reg.map(Operand::Register))
                    .chain(modp.map(|modp| modp.map(Const::Int).map(Operand::Value)))
                    .collect(),
            ),
            Instruction::RequiredBitLength(i) => {
                ("reqbl", vec![i.map(Const::Int).map(Operand::Value)])
            }
            Instruction::JoinTape(i) => ("join_tape", vec![i.map(Const::Int).map(Operand::Value)]),
            Instruction::RunTape { source } => (
                "run_tape",
                source.iter().map(|x| x.map(Into::into)).collect(),
            ),
            Instruction::StArg { source } => ("starg", vec![source.clone().map(Into::into)]),
            Instruction::Clock { stop, id } => (
                if *stop { "stop_clock" } else { "start_clock" },
                vec![id.map(Const::Int).map(Operand::Value)],
            ),
            Instruction::VMControl(instr) => (instr, vec![]),
            Instruction::LdTn { destination } => {
                ("ldtn", vec![destination.clone().map(Into::into)])
            }
            Instruction::LdArg { destination } => {
                ("ldarg", vec![destination.clone().map(Into::into)])
            }
            Instruction::OpenSint {
                destination,
                source,
            } => (
                "opensint",
                vec![
                    destination.clone().map(Into::into),
                    source.clone().map(Into::into),
                ],
            ),
            Instruction::OpenSbit {
                destination,
                source,
            } => (
                "opensbit",
                vec![
                    destination.clone().map(Into::into),
                    source.clone().map(Into::into),
                ],
            ),
            Instruction::DaBit { dest1, dest2 } => (
                "dabit",
                vec![dest1.clone().map(Into::into), dest2.clone().map(Into::into)],
            ),
            Instruction::Nop => ("nop", vec![]),
            Instruction::GarbledCircuit(i) => ("gc", vec![i.map(Const::Int).map(Operand::Value)]),
            Instruction::LocalFunction(i) => ("lf", vec![i.map(Const::Int).map(Operand::Value)]),
        };
        let (instruction, args) = if self.vectorized.elem.get() > 1 {
            let mut args = args;
            args.insert(
                0,
                self.span.with(Operand::Value(Const::Int(
                    self.vectorized.elem.get().into(),
                ))),
            );
            (cx.strings.push_get(format!("v{}", instruction)), args)
        } else {
            (instruction, args)
        };
        Lexical {
            span: self.span,
            instruction,
            args,
            comment: self.comment,
        }
    }
}
