use crate::asm::Instruction;
use crate::asm::MemoryBank;
use crate::asm::{BinaryOperation, Body, JumpMode, Terminator};
use crate::compiler::Compiler;
use crate::lexer::Register;
use crate::lexer::{Const, Operand};
use crate::span::Spanned;
use cranelift_codegen::ir::condcodes::IntCC;
use cranelift_codegen::ir::types::*;
use cranelift_codegen::ir::ExtFuncData;
use cranelift_codegen::ir::FuncRef;
use cranelift_codegen::ir::Inst;
use cranelift_codegen::ir::SigRef;
use cranelift_codegen::ir::{AbiParam, ExternalName, Function, InstBuilder, MemFlags, Signature};
use cranelift_codegen::ir::{TrapCode, Value};
use cranelift_codegen::isa::{self, CallConv};
use cranelift_codegen::{settings, Context};
use cranelift_frontend::{FunctionBuilder, FunctionBuilderContext, Variable};
use std::collections::HashMap;
use std::str::FromStr;
use target_lexicon::triple;

impl Compiler {
    pub fn generate_ir<'a>(&'a self, body: Body<'a>) -> String {
        let sig = Signature::new(CallConv::SystemV);
        let mut fn_builder_ctx = FunctionBuilderContext::new();
        let mut func = Function::with_name_signature(ExternalName::user(0, 0), sig);
        {
            let mut builder = FunctionBuilder::new(&mut func, &mut fn_builder_ctx);
            let blocks: Vec<_> = body.blocks.iter().map(|_| builder.create_ebb()).collect();
            let mut builder = Builder {
                fx: builder,
                cx: self,
                next_id: 0,
                vars: Default::default(),
                intrinsics: Default::default(),
            };

            // our code starts at block 0
            builder.fx.switch_to_block(blocks[0]);
            builder.fx.seal_block(blocks[0]);

            for (block_id, block) in body.blocks.into_iter().enumerate() {
                builder.fx.switch_to_block(blocks[block_id]);
                for stmt in block.stmts {
                    match stmt.instr {
                        Instruction::Assign { destination, value } => {
                            let destination = builder.var(destination.elem);
                            let value = builder.read_operand(value);
                            builder.fx.def_var(destination, value);
                        }
                        Instruction::LoadFromMemory {
                            source,
                            memory_bank,
                            destination,
                        } => {
                            let source = builder.read_operand(source);
                            let destination = builder.var(destination.elem);
                            let val = builder.fx.ins().load(
                                I32,
                                MemFlags::new(),
                                source,
                                memory_bank.offset(),
                            );
                            builder.fx.def_var(destination, val);
                        }
                        Instruction::StoreToMemory {
                            destination,
                            memory_bank,
                            value,
                        } => {
                            let value = builder.var(value.elem);
                            let value = builder.fx.use_var(value);
                            let destination = builder.read_operand(destination);
                            builder.fx.ins().store(
                                MemFlags::new(),
                                value,
                                destination,
                                memory_bank.offset(),
                            );
                        }
                        Instruction::BinaryOperation {
                            op,
                            a,
                            b,
                            destination,
                            ..
                        } => {
                            let a = builder.read_operand(a);
                            let b = builder.read_operand(b);
                            let destination = builder.var(destination.elem);
                            let result = match op {
                                BinaryOperation::Add => builder.fx.ins().iadd(a, b),
                                BinaryOperation::Mul => builder.fx.ins().imul(a, b),
                                BinaryOperation::Sub => builder.fx.ins().isub(a, b),
                                BinaryOperation::Div => builder.fx.ins().sdiv(a, b),
                                BinaryOperation::Mod => builder.fx.ins().srem(a, b),
                                BinaryOperation::And => builder.fx.ins().band(a, b),
                                BinaryOperation::Or => builder.fx.ins().bor(a, b),
                                BinaryOperation::Xor => builder.fx.ins().bxor(a, b),
                                BinaryOperation::Not => {
                                    let ret = builder.call_intrinsic(Intrinsic::Notc, &[a, b]);
                                    let ret = builder.fx.inst_results(ret);
                                    assert_eq!(ret.len(), 1);
                                    ret[0]
                                }
                                BinaryOperation::Lt => {
                                    builder.fx.ins().icmp(IntCC::SignedLessThan, a, b)
                                }
                                BinaryOperation::Gt => {
                                    builder.fx.ins().icmp(IntCC::SignedGreaterThan, a, b)
                                }
                                BinaryOperation::Eq => builder.fx.ins().icmp(IntCC::Equal, a, b),
                                BinaryOperation::Shl => builder.fx.ins().ishl(a, b),
                                BinaryOperation::Shr => builder.fx.ins().sshr(a, b),
                                BinaryOperation::Bit => {
                                    let ret = builder.call_intrinsic(Intrinsic::Bitsint, &[a, b]);
                                    let ret = builder.fx.inst_results(ret);
                                    assert_eq!(ret.len(), 1);
                                    ret[0]
                                }
                            };
                            builder.fx.def_var(destination, result);
                        }
                        Instruction::RequiredBitLength(num) => {
                            let num = builder.fx.ins().iconst(I32, i64::from(num.elem));
                            builder.call_intrinsic(Intrinsic::RequiredBitLength, &[num]);
                        }
                        Instruction::StartOpen { registers } => {
                            let registers: Vec<_> = registers
                                .iter()
                                .map(|reg| builder.fx.ins().iconst(I32, i64::from(reg.elem.0)))
                                .collect();
                            builder.call_intrinsic(Intrinsic::StartOpen, &registers);
                        }
                        Instruction::StopOpen { registers } => {
                            let registers: Vec<_> = registers
                                .iter()
                                .map(|reg| builder.fx.ins().iconst(I32, i64::from(reg.elem.0)))
                                .collect();
                            builder.call_intrinsic(Intrinsic::StopOpen, &registers);
                        }
                        Instruction::Convert {
                            value,
                            destination,
                            modp,
                            ..
                        } => {
                            let value = builder.read_register(value);
                            let modp = builder
                                .fx
                                .ins()
                                .iconst(I32, i64::from(modp.map(|modp| modp.elem).unwrap_or(0)));
                            let destination = builder.var(destination.elem);
                            let result = builder.call_intrinsic(Intrinsic::Convert, &[value, modp]);
                            let result = builder.fx.inst_results(result);
                            assert_eq!(result.len(), 1);
                            let result = result[0];
                            builder.fx.def_var(destination, result);
                        }
                        Instruction::GenerateTriple { destinations } => {
                            let result = builder.call_intrinsic(Intrinsic::GenerateTriple, &[]);
                            let result = builder.fx.inst_results(result).to_owned();
                            assert_eq!(result.len(), 3);
                            for (dest, result) in destinations.iter().zip(result) {
                                let dest = builder.var(dest.elem.into());
                                builder.fx.def_var(dest, result);
                            }
                        }
                        Instruction::GenerateSquare { destinations } => {
                            let result = builder.call_intrinsic(Intrinsic::GenerateSquare, &[]);
                            let result = builder.fx.inst_results(result).to_owned();
                            assert_eq!(result.len(), 2);
                            for (dest, result) in destinations.iter().zip(result) {
                                let dest = builder.var(dest.elem.into());
                                builder.fx.def_var(dest, result);
                            }
                        }
                        Instruction::GenerateBit { destination } => {
                            let result = builder.call_intrinsic(Intrinsic::GenerateBit, &[]);
                            let result = builder.fx.inst_results(result).to_owned();
                            assert_eq!(result.len(), 1);
                            let result = result[0];
                            let dest = builder.var(destination.elem.into());
                            builder.fx.def_var(dest, result);
                        }
                        Instruction::LdArg { destination } => {
                            let inst = builder.call_intrinsic(Intrinsic::Ldarg, &[]);
                            let destination = builder.var(destination.elem.into());
                            let val = builder.fx.inst_results(inst);
                            assert_eq!(val.len(), 1);
                            let val = val[0];
                            builder.fx.def_var(destination, val);
                        }
                        Instruction::LdTn { destination } => {
                            let inst = builder.call_intrinsic(Intrinsic::Ldtn, &[]);
                            let destination = builder.var(destination.elem.into());
                            let val = builder.fx.inst_results(inst);
                            assert_eq!(val.len(), 1);
                            let val = val[0];
                            builder.fx.def_var(destination, val);
                        }
                        Instruction::Clock { stop, id } => {
                            let intrinsic = if stop {
                                Intrinsic::StopClock
                            } else {
                                Intrinsic::StartClock
                            };
                            let id = builder.fx.ins().iconst(I32, i64::from(id.elem));
                            builder.call_intrinsic(intrinsic, &[id]);
                        }
                        _ => {
                            builder.cx.report(crate::errors::UnimplementedInstruction {
                                span: stmt.span,
                                name: "",
                            });
                            panic!("{:#?}", stmt.instr);
                        }
                    }
                }
                match block.terminator.elem {
                    Terminator::Jump(jmp) => match jmp.mode {
                        JumpMode::Conditional(cond) => {
                            let val = builder.read_register(cond.register);
                            if cond.jump_if_zero {
                                builder.fx.ins().brz(val, blocks[jmp.target_block], &[]);
                            } else {
                                builder.fx.ins().brnz(val, blocks[jmp.target_block], &[]);
                            }
                            builder.fx.ins().jump(blocks[cond.fallthrough_block], &[]);
                        }
                        // FIXME: we can actually create functions in cranelift
                        JumpMode::Call | JumpMode::Goto => {
                            builder.fx.ins().jump(blocks[jmp.target_block], &[]);
                        }
                    },
                    Terminator::Return { .. } => {
                        builder.call_intrinsic(Intrinsic::Return, &[]);
                    }
                    Terminator::Restart { .. } => {
                        builder.call_intrinsic(Intrinsic::Restart, &[]);
                        builder.fx.ins().trap(TrapCode::UnreachableCodeReached);
                    }
                    Terminator::Crash { .. } => {
                        builder.fx.ins().trap(TrapCode::UnreachableCodeReached);
                    }
                    Terminator::Done => {
                        builder.fx.ins().return_(&[]);
                    }
                }
            }
            for &block in &blocks[1..] {
                builder.fx.seal_block(block);
            }
        }

        let flags = settings::Flags::new(settings::builder());
        let mut cx = Context::for_function(func);
        // FIXME: create our own ISA for SCALE and upstream it
        let isa = isa::lookup(triple!("x86_64")).unwrap();
        let isa = isa.finish(flags);
        let isa = &*isa;

        let mut compile = || {
            cx.verify_if(isa)?;
            cx.compute_cfg();
            cx.preopt(isa)?;
            cx.legalize(isa)?;
            cx.postopt(isa)?;
            cx.compute_domtree();
            cx.compute_loop_analysis();
            cx.licm(isa)?;
            cx.simple_gvn(isa)?;
            cx.compute_domtree();
            cx.eliminate_unreachable_code(isa)?;
            cx.dce(isa)?;
            cx.regalloc(isa)?;
            cx.prologue_epilogue(isa)?;
            cx.redundant_reload_remover(isa)?;
            cx.shrink_instructions(isa)?;
            cx.relax_branches(isa)
        };

        if let Err(err) = compile() {
            panic!("{}", err);
        }
        cx.func.display(None).to_string()
    }
}

struct Builder<'a, 'b> {
    fx: FunctionBuilder<'b>,
    cx: &'a Compiler,
    next_id: u32,
    vars: HashMap<Register, Variable>,
    intrinsics: HashMap<(Intrinsic, usize), FuncRef>,
}

#[repr(u32)]
#[derive(Copy, Clone, Debug, Eq, PartialEq, Hash)]
enum Intrinsic {
    RequiredBitLength,
    Restart,
    Ldarg,
    Ldtn,
    Notc,
    Bitsint,
    StartOpen,
    StopOpen,
    Convert,
    GenerateTriple,
    GenerateSquare,
    GenerateBit,
    StartClock,
    StopClock,
    Return,
}

fn intrinsic_signature(fx: &mut FunctionBuilder<'_>, name: Intrinsic, nargs: usize) -> SigRef {
    let mut sig = Signature::new(CallConv::SystemV);
    match name {
        Intrinsic::StartClock | Intrinsic::StopClock | Intrinsic::RequiredBitLength => {
            sig.params.push(AbiParam::new(I32));
        }
        Intrinsic::Return | Intrinsic::Restart => {}
        Intrinsic::GenerateBit | Intrinsic::Ldarg | Intrinsic::Ldtn => {
            sig.returns.push(AbiParam::new(I32));
        }
        Intrinsic::Convert | Intrinsic::Bitsint | Intrinsic::Notc => {
            sig.params.push(AbiParam::new(I32));
            sig.params.push(AbiParam::new(I32));
            sig.returns.push(AbiParam::new(I32));
        }
        Intrinsic::StartOpen | Intrinsic::StopOpen => {
            sig.params.extend((0..nargs).map(|_| AbiParam::new(I32)));
        }
        Intrinsic::GenerateTriple => {
            sig.returns.extend((0..3).map(|_| AbiParam::new(I32)));
        }
        Intrinsic::GenerateSquare => {
            sig.returns.extend((0..2).map(|_| AbiParam::new(I32)));
        }
    }
    assert_eq!(nargs, sig.params.len());
    fx.import_signature(sig)
}

impl<'a, 'b> Builder<'a, 'b> {
    fn call_intrinsic(&mut self, name: Intrinsic, args: &[Value]) -> Inst {
        let fx = &mut self.fx;
        let func = *self
            .intrinsics
            .entry((name, args.len()))
            .or_insert_with(|| {
                let func = ExtFuncData {
                    colocated: false,
                    // this is slightly hacky, but it's the easiest way to support intrinsics with an arbitrary number of arguments
                    // the first id identifies the intrinsic, the second one the variant used
                    name: ExternalName::user(name as u32, args.len() as u32),
                    signature: intrinsic_signature(fx, name, args.len()),
                };
                fx.import_function(func)
            });
        self.fx.ins().call(func, args)
    }

    fn read_register(&mut self, reg: Spanned<'a, Register>) -> Value {
        let var = self.var(reg.elem);
        self.fx.use_var(var)
    }

    fn read_operand(&mut self, op: Spanned<'a, Operand>) -> Value {
        match op.elem {
            Operand::Register(reg) => self.read_register(op.span.with(reg)),
            Operand::Value(Const::Int(i)) => self.fx.ins().iconst(I32, i64::from(i)),
            Operand::Value(Const::Bool(b)) => self.fx.ins().bconst(B1, b),
        }
    }

    fn var(&mut self, register: Register) -> Variable {
        let next_id = &mut self.next_id;
        let fx = &mut self.fx;
        *self.vars.entry(register).or_insert_with(|| {
            let var = Variable::with_u32(*next_id);
            *next_id += 1;
            // FIXME: variables can have what value range?
            fx.declare_var(var, register.ty());
            var
        })
    }
}

impl Register {
    fn ty(self) -> Type {
        match self {
            Register::Secret(_)
            | Register::Clear(_)
            | Register::Regint(_)
            | Register::SecretRegint(_) => I32,
            Register::SecretBit(_) => B1,
        }
    }
}

impl MemoryBank {
    const SECRET_END: i32 = Self::CLEAR_START - 1;
    const CLEAR_START: i32 = 1 << 8;
    const CLEAR_END: i32 = Self::REGULAR_START - 1;
    const REGULAR_START: i32 = 1 << 16;
    const REGULAR_END: i32 = Self::SECRET_REGULAR_START - 1;
    const SECRET_REGULAR_START: i32 = 1 << 24;
    const SECRET_REGULAR_END: i32 = i32::max_value();
    /// The starting offset of each memory bank. This is quite a hack, but cranelift
    /// does not know about different memory banks.
    fn offset(self) -> i32 {
        match self {
            MemoryBank::Secret => 0,
            MemoryBank::Clear => Self::CLEAR_START,
            MemoryBank::Regint => Self::REGULAR_START,
            MemoryBank::SecretRegint => Self::SECRET_REGULAR_START,
        }
    }
    /// Inverse operation of `offset`, yielding a `MemoryBank` and the id inside the bank
    pub fn for_address(u: i32) -> (Self, i32) {
        match u {
            0..=Self::SECRET_END => (MemoryBank::Secret, u),
            Self::CLEAR_START..=Self::CLEAR_END => (MemoryBank::Clear, u - Self::CLEAR_START),
            Self::REGULAR_START..=Self::REGULAR_END => (MemoryBank::Clear, u - Self::REGULAR_END),
            Self::SECRET_REGULAR_START..=Self::SECRET_REGULAR_END => {
                (MemoryBank::Clear, u - Self::SECRET_REGULAR_START)
            }
            _ => panic!("got negative address: {}", u),
        }
    }
}
