// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use documentation::name2instr;

use super::{Block, Body, JumpMode, Statement, Terminator};
use super::{Instruction, IoInstruction, Jump, JumpCondition};
use crate::lexer::{Const, Lexical, Operand};
use crate::span::{Span, Spanned};
use crate::{errors, Compiler};
use std::collections::HashMap;
use std::convert::{TryFrom, TryInto};
use std::iter;
use std::num::NonZeroU32;

impl<'a> Statement<'a> {
    pub fn parse(cx: &'a Compiler, lex: &Lexical<'a>) -> Self {
        let (instruction, vectorized, args) = if lex.instruction.starts_with('v') {
            let (vectorized, rest) = lex.args().split_first_or_err(cx);
            let vectorized = vectorized.require::<i32>(cx);
            let vectorized = match vectorized.elem.try_into() {
                Ok(val) if val > 1 => Ok(vectorized.span.with(NonZeroU32::new(val).unwrap())),
                _ => Err(cx.report(errors::InvalidVectorSize { n: vectorized })),
            };
            (&lex.instruction[1..], vectorized, rest)
        } else {
            (&*lex.instruction, Err(lex.span), lex.args())
        };
        Self::parse_inner(
            cx,
            instruction,
            vectorized.unwrap_or_else(|span| span.with(NonZeroU32::new(1).unwrap())),
            args,
            lex.comment,
        )
    }

    fn parse_inner(
        cx: &'a Compiler,
        instruction: &'a str,
        vectorized: Spanned<NonZeroU32>,
        args: Spanned<&[Spanned<Operand>]>,
        comment: Span,
    ) -> Self {
        let instr: Instruction<'a> = match instruction {
            "jmp" => {
                let offset: Spanned<u32> = args.index_or_err(cx, 0).require(cx);
                assert_eq!(offset.elem, 0);
                Instruction::Nop
            }
            "" => Instruction::Nop,
            "ldsi" | "ldi" | "ldint" | "movs" | "movc" | "movint" | "movsint" | "movsb"
            | "ldsint" | "ldsbit" => Instruction::Assign {
                destination: args.index_or_err(cx, 0).require(cx),
                value: args.index_or_err(cx, 1),
            },
            "call" | "addc" | "adds" | "addm" | "addci" | "addsi" | "addsint" | "addsintc"
            | "addint" | "subc" | "subs" | "subml" | "submr" | "subci" | "subsi" | "subcfi"
            | "subsfi" | "subint" | "subsint" | "subsintc" | "subcints" | "mulc" | "mulm"
            | "mulci" | "mulsi" | "mulsint" | "mulsintc" | "mulint" | "divc" | "divci"
            | "divsint" | "divint" | "modc" | "modci" | "modint" | "shlsint" | "shrsint"
            | "shlc" | "shrc" | "shlci" | "shrci" | "eqzc" | "ltzc" | "ltint" | "gtint"
            | "eqint" | "andc" | "xorc" | "orc" | "andci" | "orci" | "xorci" | "sand" | "xorsb"
            | "andsb" | "orsb" | "andsint" | "andsintc" | "orsint" | "xorsint" | "notc"
            | "orsintc" | "xorsintc" | "bitsint" | "andint" | "orint" | "xorint" | "shlint"
            | "shrint" | "legendrec" | "digestc" | "neg" | "negb" | "invsint" | "eqzint"
            | "ltzint" | "invint" | "eqzsint" | "ltzsint" | "rand" | "randc" | "randint"
            | "randsint" | "randfloat" | "randsbit" | "getspc" | "getsps" | "getspint"
            | "getspsint" | "getspsbit" | "ldmc" | "ldms" | "ldmci" | "ldmsi" | "ldmint"
            | "ldminti" | "ldmsint" | "ldmsinti" | "stmc" | "stms" | "stmci" | "stmsi"
            | "stmint" | "stminti" | "stmsint" | "stmsinti" | "open_channel" | "close_channel"
            | "private_input" | "private_output" | "mprivate_input" | "mprivate_output"
            | "output_clear" | "input_clear" | "output_int" | "input_int" | "reqbl"
            | "join_tape" | "run_tape" | "start_clock" | "stop_clock" | "clear_memory"
            | "clear_registers" | "starg" | "convsuregsint" | "convint" | "convmodp"
            | "convregsreg" | "convsregsint" | "convsintsreg" | "convsintsbit" | "convsbitsint"
            | "pokeint" | "pokec" | "pokes" | "pokesint" | "pokesbit" | "peekint" | "peekc"
            | "peeks" | "peeksint" | "peeksbit" | "rpokeint" | "rpokec" | "rpokes"
            | "rpokesint" | "rpokesbit" | "rpeekint" | "rpeekc" | "rpeeks" | "rpeeksint"
            | "rpeeksbit" | "popint" | "popc" | "pops" | "popsint" | "popsbit" | "pushint"
            | "pushc" | "pushs" | "pushsint" | "pushsbit" | "ldarg" | "ldtn" | "opensint"
            | "opensbit" | "dabit" | "mul2sint" | "sintbit" | "setbit" | "gc" | "lf"
            | "print_reg" | "print_char_regint" | "print_char4_regint" | "print_int"
            | "print_fix_plain" | "print_ieee_float" | "print_float" | "print_mem"
            | "print_fix" | "print_char" | "print_char4" | "newc" | "news" | "newint"
            | "newsint" | "deletec" | "deletes" | "deleteint" | "deletesint" | "maddc"
            | "madds" | "maddm" | "msubc" | "msubs" | "msubml" | "msubmr" | "mmulc" | "mmulm"
            | "mdivc" | "mmodc" | "mrevc" | "mrevs" | "mevalcc" | "mevalsc" | "mbitdecc"
            | "mbitdecint" | "mbitdeccs" => {
                let instr = name2instr(instruction).expect("unknown instruction");
                if instr.args.len() == args.elem.len() {
                    let num_destinations = instr
                        .args
                        .iter()
                        .position(|arg| !arg.ty.is_write())
                        .unwrap_or(instr.args.len());
                    let destinations = args.elem[..num_destinations]
                        .iter()
                        .map(|r| r.require(cx))
                        .collect();
                    let values = args.elem[num_destinations..].iter().copied().collect();
                    Instruction::General {
                        instruction,
                        destinations,
                        values,
                    }
                } else {
                    cx.report(crate::errors::TooManyArguments { span: args.span });
                    Instruction::Nop
                }
            }
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
            "triple" | "square" | "bit" => {
                let registers = args.elem.iter().skip(1).map(|op| op.require(cx)).collect();
                assert_eq!(
                    args.index_or_err(cx, 0).elem,
                    Operand::Value(Const::Int(args.elem.len() as u32 as i32 - 1))
                );
                Instruction::DataInstr {
                    instruction,
                    registers,
                }
            }
            "input_shares" | "output_shares" => {
                let channel = args.index_or_err(cx, 1).require(cx);
                let registers: Vec<_> = args.elem.iter().skip(2).map(|op| op.require(cx)).collect();
                assert_eq!(
                    args.index_or_err(cx, 0).require_uint(cx).elem,
                    registers.len() as u32 + 1,
                );
                let instr = match instruction {
                    "input_shares" => IoInstruction::InputShares { registers },
                    "output_shares" => IoInstruction::OutputShares { registers },
                    _ => unreachable!(),
                };
                Instruction::Io { instr, channel }
            }

            "nop" => Instruction::Nop,
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
                "jmpne" | "jmpeq" => (
                    JumpMode::Conditional(JumpCondition {
                        fallthrough_block: usize::max_value(),
                        jump_if_equal: lexical.instruction == "jmpeq",
                        register: lexical.args().index_or_err(cx, 0).require(cx),
                        constant: lexical.args().index_or_err(cx, 1).require(cx),
                    }),
                    lexical.args().index_or_err(cx, 2),
                ),
                "jmp" => (JumpMode::Goto, lexical.args().index_or_err(cx, 0)),
                "call" => (
                    JumpMode::Call {
                        fallthrough_block: usize::max_value(),
                    },
                    lexical.args().index_or_err(cx, 0),
                ),
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

            match &mut mode {
                JumpMode::Conditional(JumpCondition {
                    fallthrough_block: block,
                    ..
                })
                | JumpMode::Call {
                    fallthrough_block: block,
                } => *block = fallthrough_block,
                _ => {}
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
        trace!(num_blocks = block_starts.len());
        let block_map: HashMap<usize, usize> = block_starts
            .iter()
            .enumerate()
            .map(|(i, (_, (v, _)))| (*v, i + 1))
            .collect();
        // This closure fixes all the block ids in a terminator
        let fix_jl = |mut jl: Spanned<Terminator>| {
            // adjust jump targets
            if let Terminator::Jump(jmp) = &mut jl.elem {
                jmp.target_block = block_map[&jmp.target_block];
                match &mut jmp.mode {
                    JumpMode::Call { fallthrough_block }
                    | JumpMode::Conditional(JumpCondition {
                        fallthrough_block, ..
                    }) => *fallthrough_block = block_map[&*fallthrough_block],
                    JumpMode::Goto => {}
                }
            }
            jl
        };

        let mut blocks = Vec::new();
        let mut stmts = Vec::new();
        for (i, lexical) in lexicals
            .iter()
            .chain(iter::once(&Lexical::nop()))
            .enumerate()
        {
            trace!("{}: {}", i, lexical.display(cx));
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
                        Span::generated().with(Terminator::Jump(Jump {
                            target_block: next_block_id,
                            mode: JumpMode::Goto,
                            // FIXME: store a span in the jump labels
                            comment: Span::generated(),
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
