// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use std::convert::{TryFrom, TryInto};

use anyhow::Context as _;
use scasm::{
    asm::Instruction,
    asm::Jump,
    asm::JumpCondition,
    asm::JumpMode,
    asm::Statement,
    asm::Terminator,
    lexer::{Const, Operand, Register, RegisterKind},
    span::Span,
};
use tracing::{instrument, trace};
use walrus::{
    ir::ExtendedLoad,
    ir::{
        BinaryOp, Binop, Block, Br, BrIf, BrTable, Drop, GlobalGet, GlobalSet, IfElse, Instr,
        InstrSeqType, Load, LoadKind, LocalGet, LocalSet, LocalTee, Loop, Select, Store, StoreKind,
        UnaryOp, Unop, Value,
    },
    FunctionKind, GlobalId,
};

use crate::{stack::AsRegisterKind, stack::PersistentKind, CurrentBlockHandler};

impl<'a, 'bh, 'cx, 'wasm> CurrentBlockHandler<'a, 'bh, 'cx, 'wasm> {
    pub(crate) fn global_set(&mut self, val: Operand, global: GlobalId) -> anyhow::Result<()> {
        let reg = self.stack.global(global)?;
        let instr = Instruction::Assign {
            destination: reg.into(),
            value: val.into(),
        };
        let comment = self
            .bh
            .comment(|f| write!(f, "read from global {}: {}", global.index(), reg))?;
        self.push_stmt(Statement::from_instr_with_comment(instr, comment));
        Ok(())
    }

    pub fn wasm_val_to_op(&mut self, val: Value) -> anyhow::Result<Operand> {
        Ok(match val {
            Value::I64(i) if i32::try_from(i).is_ok() => Operand::Value(Const::Int(i as i32)),
            Value::I64(i) => {
                let mut upper = i / 2_i64.pow(32);
                let mut lower = i % 2_i64.pow(32);
                if lower >= 2_i64.pow(31) {
                    lower -= 2_i64.pow(32);
                    upper += 1;
                } else if lower < -2_i64.pow(31) {
                    lower += 2_i64.pow(32);
                    upper -= 1;
                }
                let lower = self.val_to_reg(Operand::Value(Const::Int(lower as i32)));
                let upper = self.val_to_reg(Operand::Value(Const::Int(upper as i32)));
                // 2.pow(16)
                let upper_shift = self.val_to_reg(Operand::Value(Const::Int(2_i32.pow(16))));
                // 2.pow(16).pow(2)
                let upper_shift = self.binop(
                    upper_shift,
                    upper_shift,
                    RegisterKind::Regint,
                    "mulint",
                    Span::DUMMY,
                );
                // FIXME: use `shlint` now that we have it.
                // upper * 2.pow(16).pow(2)
                let upper_shift = self.binop(
                    upper,
                    upper_shift,
                    RegisterKind::Regint,
                    "mulint",
                    Span::DUMMY,
                );
                let result = self.binop(
                    upper_shift,
                    lower,
                    RegisterKind::Regint,
                    "addint",
                    Span::DUMMY,
                );
                result.into()
            }
            Value::I32(i) => Operand::Value(Const::Int(i)),
            Value::F32(f) if f == 0.0 => Operand::Value(Const::Int(0)),
            other => anyhow::bail!("unimplemented {:?}", other),
        })
    }

    #[instrument(skip(self), level = "info")]
    pub fn process_instr(&mut self, instr: &Instr) -> anyhow::Result<()> {
        match instr {
            Instr::Const(c) => {
                let op = self.wasm_val_to_op(c.value)?;
                self.stack.push(op);
            }
            Instr::Call(call) => {
                let func = self.module.funcs.get(call.func);
                let name = &func.name;
                match &func.kind {
                    FunctionKind::Import(import) => {
                        let fn_ty = self.module.types.get(import.ty);
                        let import = self.module.imports.get(import.import);
                        if import.module != "env" {
                            anyhow::bail!("Instruction Call, import: module is not env")
                        }
                        trace!(%import.name);

                        self.handle_extern_call(&import.name, fn_ty)?;
                    }
                    FunctionKind::Local(func) => self.handle_call(name.as_deref(), func)?,
                    FunctionKind::Uninitialized(_) => unreachable!(),
                }
            }
            Instr::Unop(Unop {
                op: UnaryOp::I64ExtendSI32,
            })
            | Instr::Unop(Unop {
                op: UnaryOp::I64ExtendUI32,
            }) => {
                // Do nothing, we'd just pop and push back again anyway
            }
            Instr::Unop(Unop {
                op: UnaryOp::I32x4Splat,
            }) => {
                self.stack.pop()?;
                self.stack.push_temp(RegisterKind::Clear);
            }
            Instr::Unop(Unop {
                op: UnaryOp::I32WrapI64,
            }) => {
                let val = self.stack.pop()?;
                let comment = self.comment(|f| write!(f, "wrap i64 to be a valid i32"))?;
                let mask = self.val_to_reg(Operand::from(-1));
                let result = self.binop(val, mask, RegisterKind::Regint, "andint", comment);
                self.stack.push(result);
            }
            Instr::Unop(Unop {
                op: UnaryOp::I32Eqz,
            })
            | Instr::Unop(Unop {
                op: UnaryOp::I64Eqz,
            }) => {
                let val = self.stack.pop()?;
                let result = self.unop(val, "eqzint");
                self.stack.push(result.elem);
            }
            Instr::Binop(Binop {
                op: op @ BinaryOp::I64Mul,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I32Mul,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I32DivU,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I64DivU,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I64DivS,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I64Sub,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I32Sub,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I64Add,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I32Add,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I64Xor,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I64And,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I32And,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I64Shl,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I32Shl,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I64ShrS,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I32ShrU,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I64ShrU,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I32Or,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I64Or,
            }) => {
                let b = self.stack.pop()?;
                let a = self.stack.pop()?;
                let instruction = match op {
                    BinaryOp::I32Add | BinaryOp::I64Add => "addint",
                    BinaryOp::I32Sub | BinaryOp::I64Sub => "subint",
                    BinaryOp::I32Mul | BinaryOp::I64Mul => "mulint",
                    BinaryOp::I64DivS | BinaryOp::I64DivU | BinaryOp::I32DivU => "divint",
                    BinaryOp::I64Xor => {
                        if let (Operand::Value(Const::Int(-1)), other)
                        | (other, Operand::Value(Const::Int(-1))) = (a, b)
                        {
                            let result = self.unop(other, "invint");
                            self.stack.push(result.elem);
                            return Ok(());
                        }
                        "xorint"
                    }
                    BinaryOp::I64And | BinaryOp::I32And => "andint",
                    BinaryOp::I64Or | BinaryOp::I32Or => "orint",
                    BinaryOp::I64Shl | BinaryOp::I32Shl => "shlint",
                    BinaryOp::I64ShrS | BinaryOp::I32ShrU | BinaryOp::I64ShrU => "shrint",
                    _ => unreachable!(),
                };
                let a = self.val_to_reg(a);
                let b = self.val_to_reg(b);
                let result = self.binop(a, b, RegisterKind::Regint, instruction, Span::DUMMY);
                let result = self.wrap_if_32_bit(result, op);
                self.stack.push(result);
            }
            Instr::Binop(Binop {
                op: op @ BinaryOp::I32RemS,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I64RemS,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I32RemU,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I64RemU,
            }) => {
                let b = self.stack.pop()?;
                let a = self.stack.pop()?;
                let a = self.val_to_reg(a);
                let b = self.val_to_reg(b);
                let result = self.binop(a, b, RegisterKind::Regint, "modint", Span::DUMMY);
                let result = self.wrap_if_32_bit(result, op);
                self.stack.push(result);
            }
            Instr::Binop(Binop {
                op: op @ BinaryOp::I32GtU,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I64GtU,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I32LeS,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I64LeS,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I32LeU,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I64LeU,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I32Ne,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I64Ne,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I32Eq,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I64Eq,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I32LtU,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I64LtU,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I32GeU,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I64GeU,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I32LtS,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I64LtS,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I32GtS,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I64GtS,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I32GeS,
            })
            | Instr::Binop(Binop {
                op: op @ BinaryOp::I64GeS,
            }) => {
                let b = self.stack.pop()?;
                let a = self.stack.pop()?;
                let a = self.val_to_reg(a);
                let b = self.val_to_reg(b);
                let (check_for_unsigned, op) = match op {
                    BinaryOp::I32LtU | BinaryOp::I64LtU => (true, &BinaryOp::I64LtS),
                    BinaryOp::I32GtU | BinaryOp::I64GtU => (true, &BinaryOp::I64GtS),
                    BinaryOp::I32GeU | BinaryOp::I64GeU => (true, &BinaryOp::I64GeS),
                    BinaryOp::I32LeU | BinaryOp::I64LeU => (true, &BinaryOp::I64LeS),
                    _ => (false, op),
                };
                // tmp = a < b
                let (invert, instruction) = match op {
                    BinaryOp::I64Ne | BinaryOp::I32Ne => (true, "eqint"),
                    BinaryOp::I64Eq | BinaryOp::I32Eq => (false, "eqint"),
                    // `>=` becomes `<` and then `!`
                    BinaryOp::I64GeS | BinaryOp::I32GeS => (true, "ltint"),
                    // `<` stays `<`
                    BinaryOp::I64LtS | BinaryOp::I32LtS => (false, "ltint"),
                    // `<=` becomes `>` and then `!`
                    BinaryOp::I64LeS | BinaryOp::I32LeS => (true, "gtint"),
                    // `>` stays `>`
                    BinaryOp::I64GtS | BinaryOp::I32GtS => (false, "gtint"),
                    _ => unreachable!(),
                };
                let result = self.binop(a, b, RegisterKind::Regint, instruction, Span::DUMMY);
                let result = if invert {
                    self.invert_bit(result)?
                } else {
                    result
                };
                let result = if check_for_unsigned {
                    // a_ltz = a < 0
                    // b_ltz = b < 0
                    // Less than op
                    // https://play.rust-lang.org/?version=stable&mode=debug&edition=2018&gist=7306ccee550d803411a661fba17aed9f
                    // Greater than op
                    // https://play.rust-lang.org/?version=stable&mode=debug&edition=2018&gist=4b69ea9b5fd3d55628ec53fad2f2e879

                    // xneg yneg xy res
                    // 0    0    1  1   5 < 10  5 < 10
                    // 0    0    0  0   10 < 5  10 < 5
                    // 0    1    0  1   5 < -5  5 < 251
                    // 0    1    0  ~
                    // 1    0    1  0   -5 < 5   251 < 5
                    // 1    0    0  ~
                    // 1    1    0  0   -5 < -10 251 < 246
                    // 1    1    1  1   -10 < -5 246 < 251

                    // !xneg & !yneg & xy | !xneg & yneg & !xy | xneg & yneg & xy
                    // can be simplified as
                    // xy & (!xneg & !yneg | xneg & yneg) | !xneg & yneg & !xy
                    // which can further be simplified as
                    // xy & (xneg == yneg) | !xneg & yneg & !xy
                    // which (if we had a select operation) would be
                    // if xy { xneg == yneg } | { !xneg & yneg }
                    // so instead we do
                    // xy * (xneg == yneg) + (!xneg & yneg) * !xy
                    let a_ltz = self.ltz(a);
                    let b_ltz = self.ltz(b);
                    // xneg == yneg
                    let sign_eq =
                        self.binop(a_ltz, b_ltz, RegisterKind::Regint, "eqint", Span::DUMMY);
                    // !xy
                    let not_result = self.invert_bit(result)?;
                    // !xneg & yneg for "lt" and xneg & !yneg for "gt"
                    let discriminant = if instruction == "ltint" {
                        // !xneg
                        let not_a_ltz = self.invert_bit(a_ltz)?;
                        self.binop(
                            not_a_ltz,
                            b_ltz,
                            RegisterKind::Regint,
                            "andint",
                            Span::DUMMY,
                        )
                    } else {
                        // !yneg
                        let not_b_ltz = self.invert_bit(b_ltz)?;
                        self.binop(
                            a_ltz,
                            not_b_ltz,
                            RegisterKind::Regint,
                            "andint",
                            Span::DUMMY,
                        )
                    };
                    // xy * (xneg == yneg)
                    let left = self.binop(
                        result.map(Into::into),
                        sign_eq.map(Into::into),
                        RegisterKind::Regint,
                        "mulint",
                        Span::DUMMY,
                    );
                    // !xy * (!xneg & yneg)
                    let right = self.binop(
                        not_result.map(Into::into),
                        discriminant.map(Into::into),
                        RegisterKind::Regint,
                        "mulint",
                        Span::DUMMY,
                    );
                    self.binop(
                        left.map(Into::into),
                        right.map(Into::into),
                        RegisterKind::Regint,
                        "addint",
                        Span::DUMMY,
                    )
                } else {
                    result
                };
                self.stack.push(result);
            }
            Instr::LocalSet(LocalSet { local }) => {
                let val = self.stack.pop()?;

                self.assign_to_persistent(
                    PersistentKind::Local(*local),
                    val,
                    format_args!("write to local from local.set {}", local.index()),
                )?;
            }
            Instr::LocalTee(LocalTee { local }) => {
                let val = self.stack.pop()?;

                let reg = self.assign_to_persistent(
                    PersistentKind::Local(*local),
                    val,
                    format_args!("write to local from local.tee {}", local.index()),
                )?;

                self.stack.push(reg);
            }
            Instr::LocalGet(LocalGet { local }) => {
                // WARNING: Subtle semantics
                // We set no instruction here, but later overwrite it with a register to register copy.
                // We do this later, because at this point in time we don't know the register kind that
                // we're working with. This is because wasm reuses local variable ids if they are similar
                // enough, which applies to our dummy structs for the integer types. So we delay the
                // naming of this instruction until we try to read from the "register".
                self.read_from_persistent(
                    PersistentKind::Local(*local),
                    self.module.locals.get(*local).ty().as_register_kind(),
                    format_args!("read from local {}", local.index()),
                )?
            }
            Instr::IfElse(IfElse {
                consequent,
                alternative,
            }) => {
                let cond = self.stack.pop()?;
                let cond = Span::DUMMY.with(cond).require(self.cx);
                // We allocate the `follow` branch first, then the `else` branch,
                // because that's the order that wasm orders its
                // block ids, leading to a similar processing. Additionally this leads to simpler assembly
                // in SCALE.
                let follow = self.allocate_new_block();
                let alternative_id = self.allocate_block(*alternative)?.scale_block_id;
                // jump back to current block afterwards
                self.body.blocks[alternative_id].terminator = Terminator::Jump(Jump {
                    target_block: follow,
                    comment: Span::DUMMY,
                    mode: JumpMode::Goto,
                })
                .into();
                let consequent_id = self.allocate_block(*consequent)?.scale_block_id;
                self.body.blocks[consequent_id].terminator = Terminator::Jump(Jump {
                    target_block: follow,
                    comment: Span::DUMMY,
                    mode: JumpMode::Goto,
                })
                .into();
                let terminator = std::mem::replace(self.terminator_mut(), Terminator::Done);
                self.body.blocks[follow].terminator.elem = terminator;
                let terminator = Terminator::Jump(Jump {
                    target_block: consequent_id,
                    comment: Span::DUMMY,
                    mode: JumpMode::Conditional(JumpCondition {
                        fallthrough_block: alternative_id,
                        jump_if_equal: false,
                        constant: 0.into(),
                        register: cond,
                    }),
                });
                self.set_terminator(terminator)?;
                // for statements after the if/else
                self.scasm_block_id = follow;
            }
            Instr::BrIf(BrIf { block }) => {
                let cond = self.stack.pop()?;
                let cond = match cond {
                    Operand::Register(reg) => reg,
                    Operand::Value(val) => anyhow::bail!("BrIf operand: {:?}, not supported", val),
                };
                self.br_if(cond, *block, 0, false, "br_if".into())?;
            }
            Instr::BrTable(BrTable { blocks, default }) => {
                trace!(?blocks, ?default, "br_table");
                let index = self.stack.pop()?;
                // The wasm optimizer is sometimes a bit weird and creates `br_table` with
                // constant conditions. The `scasm` optimizations will clean that up, so
                // do not try to be smart here and just translate everything the same.
                let index = self.val_to_reg(index);
                for (i, &block) in blocks.iter().enumerate() {
                    let i = i32::try_from(i).context("too many br_table entries")?;
                    self.br_if(index, block, i, true, "br_table".into())?;
                }
                self.br(*default)?;
            }
            Instr::Loop(Loop { seq }) => {
                let loop_id = self.allocate_block(*seq)?.scale_block_id;
                let fallthrough = self.allocate_new_block();
                // The terminator of the loop block is to jump to the next block. Any actual looping will
                // require a `br` instruction to jump to the loop block
                let comment = self.comment(|f| write!(f, "end loop {:?}", seq))?;
                let terminator = Terminator::Jump(Jump {
                    target_block: fallthrough,
                    comment,
                    mode: JumpMode::Goto,
                });
                self.body.blocks[loop_id].terminator = terminator.into();
                let terminator = std::mem::replace(self.terminator_mut(), Terminator::Done);
                self.body.blocks[fallthrough].terminator.elem = terminator;
                let comment = self.comment(|f| write!(f, "start loop {:?}", seq))?;

                self.set_terminator(Terminator::Jump(Jump {
                    target_block: loop_id,
                    comment,
                    mode: JumpMode::Goto,
                }))?;
                self.scasm_block_id = fallthrough;
            }
            Instr::Block(Block { seq }) => {
                // Any `goto block_id`s in wasm will want to jump to the end of the block,
                // so basically to the start of the fallthrough block.
                let fallthrough = self.allocate_new_block();
                let block = self.allocate_block(*seq)?;
                block.jump_target_block = fallthrough;
                let block_id = block.scale_block_id;
                // Blocks can require args and leave results on the stack
                let has_return_value = match self.func.block(*seq).ty {
                    InstrSeqType::Simple(ty) => ty,
                    InstrSeqType::MultiValue(_) => {
                        anyhow::bail!("unimplemented multi-value extension to wasm")
                    }
                };
                // The terminator of the new block is to jump to the end of the block, and thus
                // to the fallthrough block.
                let terminator = Terminator::Jump(Jump {
                    target_block: fallthrough,
                    comment: Span::DUMMY,
                    mode: JumpMode::Goto,
                });
                self.body.blocks[block_id].terminator = terminator.into();
                // The fallthrough block is the block we're going to be working on from now on.
                let terminator = std::mem::replace(self.terminator_mut(), Terminator::Done);
                self.body.blocks[fallthrough].terminator.elem = terminator;
                // Our current blocks terminator jumps to the new block.
                self.set_terminator(Terminator::Jump(Jump {
                    target_block: block_id,
                    comment: Span::DUMMY,
                    mode: JumpMode::Goto,
                }))?;
                // We continue processing the current instructions in the fallthrough, the block
                // itself will be processed via `self.to_process`.
                self.scasm_block_id = fallthrough;
                if let Some(ty) = has_return_value {
                    self.read_from_persistent(
                        PersistentKind::BlockReturn(*seq),
                        ty.as_register_kind(),
                        format_args!("obtain previous block's return value"),
                    )?;
                }
            }
            Instr::Drop(Drop {}) => {
                let to_drop = self.stack.pop();
                trace!(?to_drop);
            }
            Instr::Select(Select { ty: _ }) => {
                let condition = self.stack.pop()?;
                let condition = Span::DUMMY.with(condition).require(self.cx);
                let value_if_cond_is_zero = self.stack.pop()?;
                let value_if_cond_is_not_zero = self.stack.pop()?;

                let kind = Register::try_from(value_if_cond_is_not_zero)
                    .or_else(|_| Register::try_from(value_if_cond_is_zero))
                    .map(|r| r.kind())
                    .unwrap_or(RegisterKind::Regint);

                let destination = self.stack.push_temp(kind);
                let cond_is_zero = self.allocate_new_block();
                let follow = self.allocate_new_block();
                self.body.blocks[cond_is_zero].stmts.push(
                    Instruction::Assign {
                        destination: destination.into(),
                        value: value_if_cond_is_zero.into(),
                    }
                    .into(),
                );
                self.body.blocks[cond_is_zero].terminator = Terminator::Jump(Jump {
                    target_block: follow,
                    comment: Span::DUMMY,
                    mode: JumpMode::Goto,
                })
                .into();
                let cond_is_not_zero = self.allocate_new_block();
                self.body.blocks[cond_is_not_zero].stmts.push(
                    Instruction::Assign {
                        destination: destination.into(),
                        value: value_if_cond_is_not_zero.into(),
                    }
                    .into(),
                );
                self.body.blocks[cond_is_not_zero].terminator = Terminator::Jump(Jump {
                    target_block: follow,
                    comment: Span::DUMMY,
                    mode: JumpMode::Goto,
                })
                .into();
                let terminator = std::mem::replace(self.terminator_mut(), Terminator::Done);
                self.body.blocks[follow].terminator.elem = terminator;
                let terminator = Terminator::Jump(Jump {
                    target_block: cond_is_not_zero,
                    comment: Span::DUMMY,
                    mode: JumpMode::Conditional(JumpCondition {
                        fallthrough_block: cond_is_zero,
                        jump_if_equal: false,
                        constant: 0.into(),
                        register: condition,
                    }),
                });
                self.set_terminator(terminator)?;
                self.scasm_block_id = follow;
            }
            Instr::GlobalSet(GlobalSet { global }) => {
                let val = self.stack.pop()?;
                self.global_set(val, *global)?;
            }
            Instr::GlobalGet(GlobalGet { global }) => {
                let reg = self.stack.global(*global)?;
                let dest = self.stack.push_temp(RegisterKind::Regint);
                let instr = Instruction::Assign {
                    destination: dest.into(),
                    value: Operand::from(reg).into(),
                };
                let comment = self
                    .bh
                    .comment(|f| write!(f, "read from global {}: {}", global.index(), reg))?;
                self.push_stmt(Statement::from_instr_with_comment(instr, comment));
            }
            Instr::Store(Store { memory, kind, arg }) => {
                let value = self.stack.pop()?;
                match self.stack.pop()? {
                    Operand::Value(Const::Int(address)) => {
                        if memory.index() != 0 {
                            anyhow::bail!("Instruction Store, Value: index is not zero")
                        }
                        match kind {
                            StoreKind::I64 { .. } => {}
                            _ => anyhow::bail!("unimplemented memory store for {:?}", kind),
                        }
                        let offset = arg.offset + address as u32;
                        let offset = offset / 8;
                        let offset = offset
                            + self
                                .offset_addr_val
                                .context("must have an offset address set")?;

                        // FIXME: get rid of this hack with https://github.com/rust-lang/rust/blob/38030ffb4e735b26260848b744c0910a5641e1db/compiler/rustc_target/src/spec/wasm32_base.rs#L17
                        let instr = Instruction::General {
                            instruction: "stmint",
                            destinations: vec![],
                            values: vec![
                                Span::DUMMY.with(self.val_to_reg(value).into()),
                                Span::DUMMY
                                    .with(Operand::Value(Const::Int(offset.try_into().unwrap()))),
                            ],
                        };
                        self.push_stmt(instr);
                    }
                    Operand::Register(reg) => {
                        // ((reg - arg.offset) / 8) + heap + 1000
                        let offset = self.val_to_reg(Operand::Value(Const::Int(arg.offset as i32)));
                        let real_offset = self.binop(
                            Operand::Register(reg),
                            offset,
                            RegisterKind::Regint,
                            "subint",
                            Span::DUMMY,
                        );

                        let eight = self.val_to_reg(Operand::Value(Const::Int(8)));
                        let offset = self.binop(
                            Operand::Register(real_offset),
                            eight,
                            RegisterKind::Regint,
                            "divint",
                            Span::DUMMY,
                        );

                        let wasm_heap_offset_reg = self.val_to_reg(Operand::Value(Const::Int(
                            self.offset_addr_val.unwrap() as i32,
                        )));

                        let offset = self.binop(
                            Operand::Register(offset),
                            Operand::Register(wasm_heap_offset_reg),
                            RegisterKind::Regint,
                            "addint",
                            Span::DUMMY,
                        );

                        let instruction = match kind {
                            StoreKind::I64 { .. } => "stminti",
                            StoreKind::I32_8 { .. }
                            | StoreKind::I32_16 { .. }
                            | StoreKind::I64_8 { .. }
                            | StoreKind::I64_16 { .. }
                            | StoreKind::I64_32 { .. } => "stminti",
                            StoreKind::F32 => "stmsi",
                            StoreKind::I32 { .. } => {
                                anyhow::bail!("unimplemented Store Kind: I32")
                            }
                            StoreKind::F64 => {
                                anyhow::bail!("unimplemented Store Kind: F64")
                            }
                            StoreKind::V128 => {
                                anyhow::bail!("unimplemented Store Kind: V128")
                            }
                        };

                        let instr = Instruction::General {
                            instruction,
                            destinations: vec![],
                            values: vec![
                                Span::DUMMY.with(self.val_to_reg(value).into()),
                                Span::DUMMY.with(Operand::Register(offset)),
                            ],
                        };
                        self.push_stmt(instr);
                    }
                    other => anyhow::bail!("this operand {:?} is not implemented", other),
                };
            }
            Instr::Load(Load { memory, kind, arg }) => {
                if memory.index() != 0 {
                    anyhow::bail!("Load: memory index should be 0")
                }
                let eight = self.val_to_reg(Operand::Value(Const::Int(8)));
                let (instruction, address, sub_offset, dest_register_kind) = match self
                    .stack
                    .pop()?
                {
                    Operand::Value(Const::Int(i)) => {
                        if arg.offset != 0 {
                            anyhow::bail!("wasm should have optimized away any static offsets to memory accesses")
                        }
                        let offset = i as u32;

                        // The address may not be a multiple of `8`, so we need to do a partial load.
                        // This mainly happens for `i32.load`, the LoadKind match below handles that.
                        let sub_offset = offset % 8;
                        let offset = offset / 8;
                        let offset = offset
                            + self
                                .offset_addr_val
                                .context("must have an offset address set")?;

                        trace!(?offset);
                        (
                            "ldmint",
                            Operand::Value(Const::Int(offset.try_into().unwrap())),
                            Operand::Value(Const::Int(sub_offset as i32)),
                            RegisterKind::Regint,
                        )
                    }
                    Operand::Register(reg) => {
                        let arg_offset =
                            self.val_to_reg(Operand::Value(Const::Int(arg.offset as i32)));
                        let real_offset = self.binop(
                            Operand::Register(reg),
                            arg_offset,
                            RegisterKind::Regint,
                            "subint",
                            Span::DUMMY,
                        );
                        let offset = self.binop(
                            Operand::Register(real_offset),
                            eight,
                            RegisterKind::Regint,
                            "divint",
                            Span::DUMMY,
                        );

                        let wasm_heap_offset_reg = self.val_to_reg(Operand::Value(Const::Int(
                            self.offset_addr_val.unwrap() as i32,
                        )));

                        let offset = self.binop(
                            Operand::Register(offset),
                            Operand::Register(wasm_heap_offset_reg),
                            RegisterKind::Regint,
                            "addint",
                            Span::DUMMY,
                        );

                        // offset % 8 operation, look at the const case above.
                        // We don't have a `modint` operation, so we multiply the divison
                        // result by the divisor and subtract it from the original value.
                        // So basically `x % y == x - ((x / y) * y)
                        let sub_offset_helper = self.binop(
                            Operand::Register(offset),
                            eight,
                            RegisterKind::Regint,
                            "mulint",
                            Span::DUMMY,
                        );
                        let sub_offset = self.binop(
                            Operand::Register(real_offset),
                            Operand::Register(sub_offset_helper),
                            RegisterKind::Regint,
                            "subint",
                            Span::DUMMY,
                        );

                        let (instruction, dest_register_kind) = match kind {
                            LoadKind::I64 { .. } => ("ldminti", RegisterKind::Regint),
                            LoadKind::I32_8 { .. }
                            | LoadKind::I32_16 { .. }
                            | LoadKind::I64_8 { .. }
                            | LoadKind::I64_16 { .. }
                            | LoadKind::I64_32 { .. } => ("ldminti", RegisterKind::Regint),
                            LoadKind::F32 => ("ldmsi", RegisterKind::Secret),
                            LoadKind::I32 { .. } => {
                                // TODO: need to implement this part quickly
                                anyhow::bail!("Operand Register I32 is unimplemented")
                            }
                            LoadKind::F64 => {
                                anyhow::bail!("Operand Register F64 is unimplemented")
                            }
                            LoadKind::V128 => {
                                anyhow::bail!("Operand Register V128 is unimplemented")
                            }
                        };

                        (
                            instruction,
                            Operand::Register(offset),
                            Operand::Register(sub_offset),
                            dest_register_kind,
                        )
                    }
                    op => anyhow::bail!("Operand: {} not implemented", op),
                };
                let destination = self.stack.push_temp(dest_register_kind);
                let instr = Instruction::General {
                    instruction,
                    destinations: vec![destination.into()],
                    values: vec![address.into()],
                };
                self.push_stmt(instr);
                match kind {
                    // Nothing to do, value loaded correctly
                    LoadKind::I64 { .. } => {}
                    // if the `sub_offset` is 0, then we need to right shift by 32 to get the upper 32 bits as the value
                    // if the `sub_offset` is 4, then we need to mask away the upper 32 bits
                    LoadKind::I32 { .. }
                    | LoadKind::I32_16 { .. }
                    | LoadKind::I32_8 { .. }
                    | LoadKind::I64_16 { .. }
                    | LoadKind::I64_8 { .. }
                    | LoadKind::I64_32 { .. } => {
                        let mask = match kind {
                            LoadKind::I32 { .. } | LoadKind::I64_32 { .. } => 0xFFFF_FFFF,
                            LoadKind::I32_16 { .. } | LoadKind::I64_16 { .. } => 0xFFFF,
                            LoadKind::I32_8 { .. } | LoadKind::I64_8 { .. } => 0xFF,
                            lk => anyhow::bail!("Load Kind: {:?} not implemented", lk),
                        };
                        // FIXME: implement sign extension
                        match kind {
                            LoadKind::I64_32 { kind }
                            | LoadKind::I32_16 { kind }
                            | LoadKind::I64_16 { kind }
                            | LoadKind::I32_8 { kind }
                            | LoadKind::I64_8 { kind } => match kind {
                                ExtendedLoad::SignExtend => {
                                    anyhow::bail!("ExtendedLoad::SignExtend not implemented")
                                }
                                ExtendedLoad::ZeroExtend | ExtendedLoad::ZeroExtendAtomic => {}
                            },
                            LoadKind::I32 { .. }
                            | LoadKind::I64 { .. }
                            | LoadKind::F32
                            | LoadKind::F64
                            | LoadKind::V128 => {}
                        }
                        let mask = |val,
                                    this: &mut CurrentBlockHandler<'_, '_, '_, '_>|
                         -> anyhow::Result<Register> {
                            let mask = this.wasm_val_to_op(Value::I64(mask))?;
                            let mask = this.val_to_reg(mask);
                            Ok(this.binop(val, mask, RegisterKind::Regint, "andint", Span::DUMMY))
                        };
                        let val = self.stack.pop()?;
                        let dest = match sub_offset {
                            Operand::Value(Const::Int(4)) => {
                                let shift = self.val_to_reg(Operand::Value(Const::Int(32)));
                                self.binop(val, shift, RegisterKind::Regint, "shrint", Span::DUMMY)
                            }
                            Operand::Value(Const::Int(0)) => mask(val, self)?,
                            Operand::Value(other) => {
                                anyhow::bail!("unimplemented sub-offset {}", other)
                            }
                            Operand::Register(_) => {
                                // Branchless extraction of the correct 32 bit sub-component of the 64 bit value.
                                let shift = self.binop(
                                    sub_offset,
                                    eight,
                                    RegisterKind::Regint,
                                    "mulint",
                                    Span::DUMMY,
                                );
                                let val = self.binop(
                                    val,
                                    shift,
                                    RegisterKind::Regint,
                                    "shrint",
                                    Span::DUMMY,
                                );
                                mask(val.into(), self)?
                            }
                        };
                        self.stack.push(Operand::Register(dest));
                    }
                    LoadKind::F32 => {}
                    LoadKind::F64 | LoadKind::V128 => {
                        anyhow::bail!("unimplemented memory load for {:?}", kind)
                    }
                }
            }
            // We do not emit any traps here, as wasm also generates this at the end of blocks just after
            // an unconditional `br`.
            Instr::Unreachable(_) => {}
            Instr::Return(_) => {
                // We are not using `set_terminator` here, as this block was scheduled to fall through to another
                // block at creation time, but we now want to overwrite this.
                if self.is_main_function {
                    *self.terminator_mut() = Terminator::Done;
                } else {
                    let fn_ty = self.func.ty();
                    for (id, _) in self.module.types.results(fn_ty).iter().enumerate() {
                        let val = self.stack.pop()?;

                        self.assign_to_persistent(
                            PersistentKind::FunctionReturn(self.func.entry_block(), id),
                            val,
                            "return value for function ret",
                        )?;
                    }
                    *self.terminator_mut() = Terminator::Return {
                        comment: self.comment(|f| write!(f, "return for func {:?}", fn_ty))?,
                    };
                }
            }
            Instr::Br(Br { block }) => {
                self.br(*block)?;
            }
            other => anyhow::bail!("unimplemented {:?}", other),
        }
        Ok(())
    }
}
