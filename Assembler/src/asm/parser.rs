use super::{BinaryOperation, Block, Body, JumpMode, MemoryBank, Statement, Terminator};
use super::{Instruction, IoInstruction, Jump, JumpCondition, UnaryOperation};
use crate::lexer::{Const, Lexical, Operand, RegisterKind};
use crate::span::{Span, Spanned};
use crate::{errors, Compiler};
use std::collections::HashMap;
use std::convert::{TryFrom, TryInto};
use std::num::NonZeroU16;

impl<'a> Statement<'a> {
    pub fn parse(cx: &'a Compiler, lex: &Lexical<'a>) -> Self {
        let (instruction, vectorized, args) = if lex.instruction.starts_with('v') {
            let (vectorized, rest) = lex.args().split_first_or_err(cx);
            let vectorized = vectorized.require::<i32>(cx);
            let vectorized = match vectorized.elem.try_into() {
                Ok(val) if val > 1 => Ok(vectorized.span.with(NonZeroU16::new(val).unwrap())),
                _ => Err(cx.report(errors::InvalidVectorSize { n: vectorized })),
            };
            (&lex.instruction[1..], vectorized, rest)
        } else {
            (&*lex.instruction, Err(lex.span), lex.args())
        };
        Self::parse_inner(
            cx,
            instruction,
            vectorized.unwrap_or_else(|span| span.with(NonZeroU16::new(1).unwrap())),
            args,
            lex.comment,
        )
    }

    fn parse_inner(
        cx: &'a Compiler,
        instruction: &'a str,
        vectorized: Spanned<'a, NonZeroU16>,
        args: Spanned<'a, &[Spanned<'a, Operand>]>,
        comment: Span<'a>,
    ) -> Self {
        let instr: Instruction<'a> = match instruction {
            "jmp" => {
                let offset: Spanned<'a, u32> = args.index_or_err(cx, 0).require(cx);
                assert_eq!(offset.elem, 0);
                Instruction::Nop
            }
            "" => Instruction::Nop,
            "ldsi" | "ldi" | "ldint" | "movs" | "movc" | "movint" | "movsint" | "ldsint" => {
                Instruction::Assign {
                    destination: args.index_or_err(cx, 0).require(cx),
                    value: args.index_or_err(cx, 1),
                }
            }
            "ldmc" | "ldms" | "ldmci" | "ldmsi" | "ldmint" | "ldminti" | "ldmsint" | "ldmsinti" => {
                Instruction::LoadFromMemory {
                    destination: args.index_or_err(cx, 0).require(cx),
                    source: args.index_or_err(cx, 1),
                    memory_bank: match &instruction[3..] {
                        "s" | "si" => MemoryBank::Secret,
                        "sint" | "sinti" => MemoryBank::SecretRegint,
                        "c" | "ci" => MemoryBank::Clear,
                        "int" | "inti" => MemoryBank::Regint,
                        _ => unreachable!(),
                    },
                }
            }
            "stmc" | "stms" | "stmci" | "stmsi" | "stmint" | "stminti" | "stmsint" | "stmsinti" => {
                Instruction::StoreToMemory {
                    value: args.index_or_err(cx, 0).require(cx),
                    destination: args.index_or_err(cx, 1),
                    memory_bank: match &instruction[3..] {
                        "s" | "si" => MemoryBank::Secret,
                        "sint" | "sinti" => MemoryBank::SecretRegint,
                        "c" | "ci" => MemoryBank::Clear,
                        "int" | "inti" => MemoryBank::Regint,
                        _ => unreachable!(),
                    },
                }
            }
            "addc" | "adds" | "addm" | "addci" | "addsi" | "addsint" | "addsintc" | "addint"
            | "subc" | "subs" | "subml" | "submr" | "subci" | "subsi" | "subcfi" | "subsfi"
            | "subint" | "subsint" | "subsintc" | "subcints" | "mulc" | "mulm" | "mulci"
            | "mulsi" | "mulsint" | "mulsintc" | "mulint" | "divc" | "divci" | "divsint"
            | "divint" | "modc" | "modci" | "shlsint" | "shrsint" | "shlc" | "shrc" | "shlci"
            | "shrci" | "ltint" | "gtint" | "eqint" | "andc" | "xorc" | "orc" | "andci"
            | "orci" | "xorci" | "sand" | "xorsb" | "andsb" | "orsb" | "andsint" | "andsintc"
            | "orsint" | "xorsint" | "notc" | "orsintc" | "xorsintc" | "bitsint" => {
                Instruction::BinaryOperation {
                    op: match &instruction[..3] {
                        "add" => BinaryOperation::Add,
                        "sub" => BinaryOperation::Sub,
                        "mul" => BinaryOperation::Mul,
                        "mod" => BinaryOperation::Mod,
                        "div" => BinaryOperation::Div,
                        "shl" => BinaryOperation::Shl,
                        "shr" => BinaryOperation::Shr,
                        "and" => BinaryOperation::And,
                        "san" => BinaryOperation::And,
                        "xor" => BinaryOperation::Xor,
                        "not" => BinaryOperation::Not,
                        "bit" => BinaryOperation::Bit,
                        _ => match &instruction[..2] {
                            "lt" => BinaryOperation::Lt,
                            "gt" => BinaryOperation::Gt,
                            "or" => BinaryOperation::Or,
                            "eq" => BinaryOperation::Eq,
                            _ => unreachable!(),
                        },
                    },
                    a: args.index_or_err(cx, 1),
                    b: args.index_or_err(cx, 2),
                    destination: args.index_or_err(cx, 0).require(cx),
                    instruction,
                }
            }
            "legendrec" | "digestc" | "neg" | "negb" | "invsint" | "eqzint" | "ltzint"
            | "eqzsint" | "ltzsint" | "rand" => Instruction::UnaryOperation {
                op: match &instruction[..3] {
                    "leg" => UnaryOperation::Legendre,
                    "dig" => UnaryOperation::Digestc,
                    "neg" => UnaryOperation::Neg,
                    "inv" => UnaryOperation::Inv,
                    "ltz" => UnaryOperation::Lt,
                    "eqz" => UnaryOperation::Eq,
                    "ran" => UnaryOperation::Rand,
                    _ => unreachable!(),
                },
                a: args.index_or_err(cx, 1),
                destination: args.index_or_err(cx, 0).require(cx),
            },
            "triple" => Instruction::GenerateTriple {
                destinations: [
                    args.index_or_err(cx, 0).require(cx),
                    args.index_or_err(cx, 1).require(cx),
                    args.index_or_err(cx, 2).require(cx),
                ],
            },
            "bit" => Instruction::GenerateBit {
                destination: args.index_or_err(cx, 0).require(cx),
            },
            "square" => Instruction::GenerateSquare {
                destinations: [
                    args.index_or_err(cx, 0).require(cx),
                    args.index_or_err(cx, 1).require(cx),
                ],
            },
            "mul2sint" => Instruction::Mul2Sint {
                destinations: [
                    args.index_or_err(cx, 0).require(cx),
                    args.index_or_err(cx, 1).require(cx),
                ],
                values: [
                    args.index_or_err(cx, 2).require(cx),
                    args.index_or_err(cx, 3).require(cx),
                ],
            },
            "sintbit" => Instruction::SintBit {
                destination: args.index_or_err(cx, 0).require(cx),
                args: [
                    args.index_or_err(cx, 1),
                    args.index_or_err(cx, 2),
                    args.index_or_err(cx, 3),
                ],
            },
            "pushint" | "pushc" | "pushs" | "pushsint" | "pushsbit" => Instruction::PushR {
                value: args.index_or_err(cx, 0).require(cx),
            },
            "popint" | "popc" | "pops" | "popsint" | "popsbit" => Instruction::PopR {
                destination: args.index_or_err(cx, 0).require(cx),
            },
            "peekint" | "peekc" | "peeks" | "peeksint" | "peeksbit" => Instruction::PeekR {
                destination: args.index_or_err(cx, 0).require(cx),
                value: args.index_or_err(cx, 1).require(cx),
            },
            "pokeint" | "pokec" | "pokes" | "pokesint" | "pokesbit" => Instruction::PokeR {
                value1: args.index_or_err(cx, 0).require(cx),
                value2: args.index_or_err(cx, 1).require(cx),
            },
            "getspc" => Instruction::GetSP {
                destination: args.index_or_err(cx, 0).require(cx),
                itype: RegisterKind::Clear,
            },
            "getsps" => Instruction::GetSP {
                destination: args.index_or_err(cx, 0).require(cx),
                itype: RegisterKind::Secret,
            },
            "getspint" => Instruction::GetSP {
                destination: args.index_or_err(cx, 0).require(cx),
                itype: RegisterKind::Regint,
            },
            "getspsint" => Instruction::GetSP {
                destination: args.index_or_err(cx, 0).require(cx),
                itype: RegisterKind::SecretRegint,
            },
            "getspsbit" => Instruction::GetSP {
                destination: args.index_or_err(cx, 0).require(cx),
                itype: RegisterKind::SecretBit,
            },
            "convint" | "convmodp" | "convregsreg" | "convsregsint" | "convsintsreg" => {
                Instruction::Convert {
                    destination: args.index_or_err(cx, 0).require(cx),
                    value: args.index_or_err(cx, 1).require(cx),
                    modp: args.get(2).map(|arg| arg.require(cx)),
                    unsignedint: false,
                }
            }
            "convsuregsint" => Instruction::Convert {
                destination: args.index_or_err(cx, 0).require(cx),
                value: args.index_or_err(cx, 1).require(cx),
                modp: args.get(2).map(|arg| arg.require(cx)),
                unsignedint: true,
            },
            "startopen" => {
                let registers = args.elem.iter().skip(1).map(|op| op.require(cx)).collect();
                assert_eq!(
                    args.index_or_err(cx, 0).elem,
                    Operand::Value(Const::Int(args.elem.len() as u32 as i32 - 1))
                );
                Instruction::StartOpen { registers }
            }
            "stopopen" => {
                let registers = args.elem.iter().skip(1).map(|op| op.require(cx)).collect();
                assert_eq!(
                    args.index_or_err(cx, 0).elem,
                    Operand::Value(Const::Int(args.elem.len() as u32 as i32 - 1))
                );
                Instruction::StopOpen { registers }
            }

            "reqbl" => Instruction::RequiredBitLength(args.index_or_err(cx, 0).require(cx)),

            "join_tape" => Instruction::JoinTape(args.index_or_err(cx, 0).require(cx)),

            "run_tape" => Instruction::RunTape {
                source: [
                    args.index_or_err(cx, 0).require(cx),
                    args.index_or_err(cx, 1).require(cx),
                    args.index_or_err(cx, 2).require(cx),
                ],
            },

            "start_clock" | "stop_clock" => Instruction::Clock {
                stop: instruction == "stop_clock",
                id: args.index_or_err(cx, 0).require(cx),
            },

            "clear_memory" | "clear_registers" => Instruction::VMControl(instruction),

            "ldarg" => Instruction::LdArg {
                destination: args.index_or_err(cx, 0).require(cx),
            },

            "ldtn" => Instruction::LdTn {
                destination: args.index_or_err(cx, 0).require(cx),
            },

            "print_mem" | "print_char" | "print_char4" => Instruction::PrintI {
                instruction,
                arg: args.index_or_err(cx, 0).require(cx),
            },
            "print_reg" | "print_char_regint" | "print_char4_regint" | "print_int"
            | "print_fix_plain" => Instruction::PrintR {
                instruction,
                arg: args.index_or_err(cx, 0).require(cx),
            },
            "print_float" => Instruction::PrintFL {
                args: [
                    args.index_or_err(cx, 0).require(cx),
                    args.index_or_err(cx, 1).require(cx),
                    args.index_or_err(cx, 2).require(cx),
                    args.index_or_err(cx, 3).require(cx),
                ],
            },
            "print_fix" => Instruction::PrintFix {
                args: [
                    args.index_or_err(cx, 0),
                    args.index_or_err(cx, 1),
                    args.index_or_err(cx, 2),
                ],
            },
            "starg" => Instruction::StArg {
                source: args.index_or_err(cx, 0).require(cx),
            },
            "opensint" => Instruction::OpenSint {
                destination: args.index_or_err(cx, 0).require(cx),
                source: args.index_or_err(cx, 1).require(cx),
            },
            "opensbit" => Instruction::OpenSbit {
                destination: args.index_or_err(cx, 0).require(cx),
                source: args.index_or_err(cx, 1).require(cx),
            },
            "dabit" => Instruction::DaBit {
                dest1: args.index_or_err(cx, 0).require(cx),
                dest2: args.index_or_err(cx, 1).require(cx),
            },
            "open_channel" | "close_channel" | "private_input" | "private_output"
            | "input_shares" | "output_shares" | "output_clear" | "input_clear" | "output_int"
            | "input_int" => {
                let (instr, channel) = match instruction {
                    "open_channel" => (
                        IoInstruction::OpenChan {
                            destination: args.index_or_err(cx, 0).require(cx),
                        },
                        args.index_or_err(cx, 1).require_uint(cx),
                    ),
                    "close_channel" => (
                        IoInstruction::CloseChan,
                        args.index_or_err(cx, 0).require_uint(cx),
                    ),
                    "private_input" => (
                        IoInstruction::PrivateInput {
                            destination: args.index_or_err(cx, 0).require(cx),
                            player: args.index_or_err(cx, 1).require_uint(cx),
                        },
                        args.index_or_err(cx, 2).require_uint(cx),
                    ),
                    "private_output" => (
                        IoInstruction::PrivateOutput {
                            value: args.index_or_err(cx, 0).require(cx),
                            player: args.index_or_err(cx, 1).require_uint(cx),
                        },
                        args.index_or_err(cx, 2).require_uint(cx),
                    ),
                    "input_shares" | "output_shares" => {
                        let channel = args.index_or_err(cx, 1).require_uint(cx);
                        let registers: Vec<_> =
                            args.elem.iter().skip(2).map(|op| op.require(cx)).collect();
                        assert_eq!(
                            args.index_or_err(cx, 0).require_uint(cx).elem,
                            registers.len() as u32 + 1,
                        );
                        match instruction {
                            "input_shares" => (IoInstruction::InputShares { registers }, channel),
                            "output_shares" => (IoInstruction::OutputShares { registers }, channel),
                            _ => unreachable!(),
                        }
                    }
                    "output_clear" => (
                        IoInstruction::OutputClear {
                            value: args.index_or_err(cx, 0).require(cx),
                        },
                        args.index_or_err(cx, 1).require_uint(cx),
                    ),
                    "input_clear" => (
                        IoInstruction::InputClear {
                            destination: args.index_or_err(cx, 0).require(cx),
                        },
                        args.index_or_err(cx, 1).require_uint(cx),
                    ),
                    "output_int" => (
                        IoInstruction::OutputInt {
                            value: args.index_or_err(cx, 0).require(cx),
                        },
                        args.index_or_err(cx, 1).require_uint(cx),
                    ),
                    "input_int" => (
                        IoInstruction::InputInt {
                            destination: args.index_or_err(cx, 0).require(cx),
                        },
                        args.index_or_err(cx, 1).require_uint(cx),
                    ),
                    _ => unreachable!(),
                };
                Instruction::Io { instr, channel }
            }

            "gc" => Instruction::GarbledCircuit(args.index_or_err(cx, 0).require(cx)),
            "lf" => Instruction::LocalFunction(args.index_or_err(cx, 0).require(cx)),
            _ => {
                cx.report(errors::UnknownInstruction {
                    span: args.span,
                    instruction,
                });
                Instruction::Nop
            }
        };
        Self {
            instr,
            vectorized,
            comment,
            span: args.span,
        }
    }
}

impl<'a> Body<'a> {
    pub fn parse(cx: &'a Compiler, lexicals: &[Lexical<'a>]) -> Self {
        // first we compute all jump labels

        use std::collections::{btree_map::Entry, BTreeMap};
        // counts the number of blocks, so we know what id to allocate next
        let mut num_blocks = 1;
        let mut block_starts = BTreeMap::new();
        let mut jump_labels = BTreeMap::new();
        for (i, lexical) in lexicals.iter().enumerate() {
            // generate a new block at `start` if there's not already a block starting there
            // returns the id of the new block
            let mut mk_block = |start| match block_starts.entry(start) {
                Entry::Vacant(entry) => {
                    trace!(
                        "creating new block {} at {} for {:?}",
                        num_blocks,
                        start,
                        lexical.span
                    );
                    entry.insert((num_blocks, vec![lexical.span]));
                    let old = num_blocks;
                    num_blocks += 1;
                    old
                }
                Entry::Occupied(mut val) => {
                    let val = val.get_mut();
                    trace!(
                        "encountering previously created block at {} for jump from {:?}: {:?}",
                        start,
                        lexical.span,
                        val
                    );
                    val.1.push(lexical.span);
                    val.0
                }
            };
            let (mut mode, offset) = match &*lexical.instruction {
                "restart" => {
                    mk_block(i + 1);
                    assert!(jump_labels
                        .insert(
                            i,
                            lexical.span.with(Terminator::Restart {
                                comment: lexical.comment
                            })
                        )
                        .is_none());
                    continue;
                }
                "return" => {
                    mk_block(i + 1);
                    assert!(jump_labels
                        .insert(
                            i,
                            lexical.span.with(Terminator::Return {
                                comment: lexical.comment
                            })
                        )
                        .is_none());
                    continue;
                }
                "crash" => {
                    mk_block(i + 1);
                    assert!(jump_labels
                        .insert(
                            i,
                            lexical.span.with(Terminator::Crash {
                                comment: lexical.comment
                            })
                        )
                        .is_none());
                    continue;
                }
                "jmpnz" | "jmpeqz" => (
                    JumpMode::Conditional(JumpCondition {
                        fallthrough_block: usize::max_value(),
                        jump_if_zero: lexical.instruction == "jmpeqz",
                        register: lexical.args().index_or_err(cx, 0).require(cx),
                    }),
                    lexical.args().index_or_err(cx, 1),
                ),
                "jmp" => (JumpMode::Goto, lexical.args().index_or_err(cx, 0)),
                "call" => (JumpMode::Call, lexical.args().index_or_err(cx, 0)),
                _ => continue,
            };
            // +1 because that's what the docs say. an offset of `0` is just a `nop`
            let offset = offset.require::<i32>(cx).elem + 1;
            // nops don't cause new blocks
            if offset == 1 {
                if let JumpMode::Goto = mode {
                    continue;
                }
            }
            let destination = i64::try_from(i).unwrap() + i64::from(offset);
            let destination = match usize::try_from(destination) {
                Ok(dest) => dest,
                Err(_) => {
                    cx.report(errors::JumpOutOfBounds {
                        spans: vec![lexical.span],
                    });
                    // just loop here, we already errored
                    i
                }
            };
            trace!("destination: {}", destination);

            let target_block;
            let fallthrough_block;

            // for roundtripping we need to preserve block order, so we decide which block to create first
            // by looking at whether we are jumping backwards

            if destination < i + 1 {
                // a new block starts at the jump target
                target_block = mk_block(destination);
                // after each terminator a new block starts
                fallthrough_block = mk_block(i + 1);
            } else {
                // after each terminator a new block starts
                fallthrough_block = mk_block(i + 1);
                // a new block starts at the jump target
                target_block = mk_block(destination);
            };

            trace!("target_block: {}", target_block);

            if let JumpMode::Conditional(JumpCondition {
                fallthrough_block: block,
                ..
            }) = &mut mode
            {
                *block = fallthrough_block;
            }
            let jump = Jump {
                target_block,
                mode,
                comment: lexical.comment,
            };
            trace!("create jump label at {}: {:?}", i, jump);
            assert!(jump_labels
                .insert(i, lexical.span.with(Terminator::Jump(jump)))
                .is_none());
        }

        // then we parse the statements in each basic block

        // A map converting block ids into the original source order to make sure
        // we can roundtrip asm files. Otherwise we'd be changing block orders by
        // fairly arbitrary rules.
        let block_map: HashMap<usize, usize> = block_starts
            .iter()
            .enumerate()
            .map(|(i, (_, (v, _)))| (*v, i + 1))
            .collect();
        // This closure fixes all the block ids in a terminator
        let fix_jl = |mut jl: Spanned<'a, Terminator<'a>>| {
            // adjust jump targets
            if let Terminator::Jump(jmp) = &mut jl.elem {
                jmp.target_block = block_map[&jmp.target_block];
                match &mut jmp.mode {
                    JumpMode::Conditional(cnd) => {
                        cnd.fallthrough_block = block_map[&cnd.fallthrough_block]
                    }
                    JumpMode::Call | JumpMode::Goto => {}
                }
            }
            jl
        };

        let mut blocks = Vec::new();
        let mut stmts = Vec::new();
        for (i, lexical) in lexicals.iter().enumerate() {
            trace!("{}: {}", i, lexical);
            // finalize the current block when we encounter a new one
            if let Some((next_block_id, _)) = block_starts.remove(&i) {
                let next_block_id = block_map[&next_block_id];
                // Some blocks are preceded by a jump, fetch that.
                // If there's no jump, default to just jumping directly to the next block
                let terminator = i
                    .checked_sub(1)
                    .and_then(|i| jump_labels.remove(&i))
                    .map(fix_jl)
                    .unwrap_or_else(|| {
                        Span::generated("autogenerated").with(Terminator::Jump(Jump {
                            target_block: next_block_id,
                            mode: JumpMode::Goto,
                            // FIXME: store a span in the jump labels
                            comment: Span::generated("autogenerated for block continuation"),
                        }))
                    });
                blocks.push(Block {
                    stmts: std::mem::replace(&mut stmts, Vec::new()),
                    terminator,
                });
            }
            if !jump_labels.contains_key(&i) {
                stmts.push(Statement::parse(cx, lexical));
            }
        }
        let final_terminator = lexicals
            .len()
            .checked_sub(1)
            .and_then(|idx| jump_labels.remove(&idx))
            .map(fix_jl)
            .unwrap_or_else(|| {
                stmts
                    .last()
                    .map(|stmt| stmt.span.end())
                    .unwrap_or(Span::DUMMY)
                    .with(Terminator::Done)
            });
        debug!("final_terminator: {:?}", final_terminator);
        assert!(jump_labels.is_empty());
        // we've had some code jump beyond the program boundaries
        for (i, (_, spans)) in block_starts {
            // jumping just beyond the end is essentially creating a `Done` terminator
            if i != lexicals.len() {
                cx.report(errors::JumpOutOfBounds { spans });
            }
        }
        // push the last block's statements
        blocks.push(Block {
            terminator: final_terminator,
            stmts,
        });
        Body { blocks }
    }
}
