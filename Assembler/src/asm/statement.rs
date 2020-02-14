use super::{Instruction, IoInstruction, Statement};
use crate::binary::instructions::RegisterMode;
use crate::compiler::Compiler;
use crate::lexer::{MapAllValues, Register};
use crate::span::Spanned;
use crate::transforms::vectorize;
use std::num::NonZeroU16;

// Some helper structures and traits to deduplicate the implementation of `read_registers` and `write_registers`
struct RegisterModeRead;
struct RegisterModeWrite;

trait RegisterModeTrait {
    const MODE: RegisterMode;
}

impl RegisterModeTrait for RegisterModeRead {
    const MODE: RegisterMode = RegisterMode::Read;
}

impl RegisterModeTrait for RegisterModeWrite {
    const MODE: RegisterMode = RegisterMode::Write;
}

#[derive(Clone, Debug)]
pub struct RegisterIterator {
    v: NonZeroU16,
    registers: Vec<Register>,
}

impl RegisterIterator {
    pub fn is_empty(&self) -> bool {
        self.registers.is_empty()
    }
}

impl<'a> IntoIterator for &'a RegisterIterator {
    type Item = Register;
    type IntoIter = Box<dyn Iterator<Item = Register> + 'a>;
    fn into_iter(self) -> Self::IntoIter {
        let v = self.v;
        Box::new(self.registers.iter().flat_map(move |&r| vectorize(v, r)))
    }
}

impl IntoIterator for RegisterIterator {
    type Item = Register;
    type IntoIter = Box<dyn Iterator<Item = Register>>;
    fn into_iter(self) -> Self::IntoIter {
        let v = self.v;
        Box::new(
            self.registers
                .into_iter()
                .flat_map(move |r| vectorize(v, r)),
        )
    }
}

impl<'a> Statement<'a> {
    /// All registers this instruction writes to. So for vectorized instructions all
    /// elements of the vector, not just the base.
    pub fn write_registers(&self, cx: &Compiler) -> RegisterIterator {
        RegisterIterator {
            v: self.vectorized.elem,
            registers: self.write_registers_base(cx),
        }
    }

    /// All registers this instruction reads from. So for vectorized instructions all
    /// elements of the vector, not just the base.
    pub fn read_registers(&self, cx: &Compiler) -> RegisterIterator {
        RegisterIterator {
            v: self.vectorized.elem,
            registers: self.read_registers_base(cx),
        }
    }

    /// The base registers (so for vector ops, just the first register of the vector)
    pub fn write_registers_base(&self, cx: &Compiler) -> Vec<Register> {
        self.read_registers_generic::<RegisterModeWrite>(cx)
    }

    /// The base registers (so for vector ops, just the first register of the vector)
    pub fn read_registers_base(&self, cx: &Compiler) -> Vec<Register> {
        self.read_registers_generic::<RegisterModeRead>(cx)
    }

    fn read_registers_generic<T: RegisterModeTrait>(&self, cx: &Compiler) -> Vec<Register> {
        use crate::binary::instructions::ArgTy;
        let relexed = self.relex(cx);
        let (instr, args) = match relexed.fetch_instr(cx) {
            Some(i) => i,
            None => return Vec::new(),
        };
        let mut list = Vec::new();
        let mut args_iter = args.iter();
        for decl_arg in instr.args {
            match decl_arg.ty {
                ArgTy::Register(_, mode) if mode == T::MODE => {
                    list.push(args_iter.next().unwrap().require(cx).elem)
                }
                ArgTy::List {
                    element_type,
                    len_arg,
                } => {
                    let arg_pos = instr
                        .args
                        .iter()
                        .position(|arg| arg.name == len_arg)
                        .unwrap();
                    let len: Spanned<'_, i32> = args[arg_pos].require(cx);
                    let len = match &instr.args[arg_pos].ty {
                        ArgTy::Int {
                            signed: false,
                            offset,
                        } => len.elem - offset,
                        ty => panic!("invalid array length type {:?}", ty),
                    };
                    match element_type {
                        ArgTy::List { .. } => unimplemented!("nested lists"),
                        ArgTy::Register(_, mode) if *mode == T::MODE => {
                            for _ in 0..len {
                                list.push(args_iter.next().unwrap().require(cx).elem);
                            }
                        }
                        _ => {
                            for _ in 0..len {
                                // eat elements not relevant for us
                                args_iter.next().unwrap();
                            }
                        }
                    }
                }
                _ => {
                    args_iter.next().unwrap();
                }
            }
        }
        assert_eq!(args_iter.next(), None);
        list
    }

    pub fn memory_read(&self, cx: &Compiler) -> bool {
        let relexed = self.relex(cx);
        let (instr, _) = match relexed.fetch_instr(cx) {
            Some(i) => i,
            None => return false,
        };
        instr.mem_read
    }

    pub fn memory_write(&self, cx: &Compiler) -> bool {
        let relexed = self.relex(cx);
        let (instr, _) = match relexed.fetch_instr(cx) {
            Some(i) => i,
            None => return false,
        };
        instr.mem_write
    }

    pub fn replace_registers(&mut self, cx: &Compiler, f: impl Fn(Register) -> Register) {
        let f = &f;
        match &mut self.instr {
            Instruction::StoreToMemory {
                destination, value, ..
            } => {
                destination.map_all_values(cx, f);
                value.map_all_values(cx, f);
            }
            Instruction::LoadFromMemory {
                source,
                destination,
                ..
            } => {
                source.map_all_values(cx, f);
                destination.map_all_values(cx, f);
            }
            Instruction::BinaryOperation {
                a, b, destination, ..
            } => {
                a.map_all_values(cx, f);
                b.map_all_values(cx, f);
                destination.map_all_values(cx, f);
            }
            Instruction::Mul2Sint {
                values,
                destinations,
            } => {
                for value in values {
                    value.map_all_values(cx, f);
                }
                for dest in destinations {
                    dest.map_all_values(cx, f);
                }
            }
            Instruction::PopR { destination } => destination.map_all_values(cx, f),
            Instruction::PushR { value } => value.map_all_values(cx, f),
            Instruction::PeekR { destination, value } => {
                destination.map_all_values(cx, f);
                value.map_all_values(cx, f);
            }
            Instruction::PokeR { value1, value2 } => {
                value1.map_all_values(cx, f);
                value2.map_all_values(cx, f);
            }
            Instruction::GetSP { destination, .. } => destination.map_all_values(cx, f),
            Instruction::StArg { source } => source.map_all_values(cx, f),
            Instruction::OpenSbit {
                source,
                destination,
            } => {
                source.map_all_values(cx, f);
                destination.map_all_values(cx, f);
            }
            Instruction::OpenSint {
                source,
                destination,
            } => {
                source.map_all_values(cx, f);
                destination.map_all_values(cx, f);
            }
            Instruction::Assign { destination, value }
            | Instruction::UnaryOperation {
                a: value,
                destination,
                ..
            } => {
                destination.map_all_values(cx, f);
                value.map_all_values(cx, f);
            }
            Instruction::Convert {
                value, destination, ..
            } => {
                destination.map_all_values(cx, f);
                value.map_all_values(cx, f);
            }
            Instruction::PrintR { arg, .. } => arg.map_all_values(cx, f),
            Instruction::SintBit { args, destination } => {
                for arg in args {
                    arg.map_all_values(cx, f);
                }
                destination.map_all_values(cx, f);
            }
            Instruction::PrintFL { args } => {
                for arg in args {
                    arg.map_all_values(cx, f);
                }
            }
            Instruction::PrintFix { args, .. } => {
                for arg in args {
                    arg.map_all_values(cx, f);
                }
            }
            Instruction::Io {
                instr: IoInstruction::OutputInt { value },
                ..
            } => value.map_all_values(cx, f),
            Instruction::Io {
                instr: IoInstruction::PrivateOutput { value, .. },
                ..
            } => value.map_all_values(cx, f),
            Instruction::Io {
                instr: IoInstruction::OutputClear { value },
                ..
            } => value.map_all_values(cx, f),
            Instruction::Io {
                instr: IoInstruction::OutputShares { registers },
                ..
            } => {
                for register in registers {
                    register.map_all_values(cx, f);
                }
            }
            Instruction::StartOpen { registers } => {
                for register in registers {
                    register.map_all_values(cx, f);
                }
            }
            Instruction::DaBit { dest1, dest2 } => {
                dest1.map_all_values(cx, f);
                dest2.map_all_values(cx, f);
            }
            Instruction::Clock { .. } => {}
            Instruction::StopOpen { registers } => {
                for reg in registers {
                    reg.map_all_values(cx, f);
                }
            }
            Instruction::GenerateBit { destination } => destination.map_all_values(cx, f),
            Instruction::GenerateTriple { destinations } => {
                for destination in destinations {
                    destination.map_all_values(cx, f)
                }
            }
            Instruction::GenerateSquare { destinations } => {
                for destination in destinations {
                    destination.map_all_values(cx, f)
                }
            }
            Instruction::RequiredBitLength(_) => {}
            Instruction::JoinTape(_) => {}
            Instruction::LocalFunction(_) => {}
            Instruction::GarbledCircuit(_) => {}
            Instruction::RunTape { .. } => {}
            Instruction::VMControl(_) => {}
            Instruction::LdArg { destination } => destination.map_all_values(cx, f),
            Instruction::LdTn { destination } => destination.map_all_values(cx, f),
            Instruction::PrintI { .. } => {}
            Instruction::Nop => {}
            Instruction::Io { instr, .. } => instr.replace_registers(cx, f),
        }
    }

    pub fn is_barrier(&self, cx: &Compiler) -> bool {
        let relexed = self.relex(cx);
        let (instr, _) = match relexed.fetch_instr(cx) {
            Some(i) => i,
            None => return false,
        };
        instr.barrier
    }

    pub fn is_startopen(&self) -> bool {
        match self.instr {
            Instruction::StartOpen { .. } => true,
            _ => false,
        }
    }
    pub fn is_stopopen(&self) -> bool {
        match self.instr {
            Instruction::StopOpen { .. } => true,
            _ => false,
        }
    }
}

impl IoInstruction<'_> {
    pub fn replace_registers(&mut self, cx: &Compiler, f: impl Fn(Register) -> Register) {
        let f = &f;
        match self {
            IoInstruction::OpenChan { destination } => destination.map_all_values(cx, f),
            IoInstruction::CloseChan => {}
            IoInstruction::PrivateInput { destination, .. } => {
                destination.map_all_values(cx, f);
            }
            IoInstruction::PrivateOutput { value, .. } => value.map_all_values(cx, f),
            IoInstruction::InputShares { registers } => {
                for reg in registers {
                    reg.map_all_values(cx, f);
                }
            }
            IoInstruction::OutputShares { registers } => {
                for reg in registers {
                    reg.map_all_values(cx, f);
                }
            }
            IoInstruction::OutputClear { value } => value.map_all_values(cx, f),
            IoInstruction::InputClear { destination } => destination.map_all_values(cx, f),
            IoInstruction::OutputInt { value } => value.map_all_values(cx, f),
            IoInstruction::InputInt { destination } => destination.map_all_values(cx, f),
        }
    }
}
