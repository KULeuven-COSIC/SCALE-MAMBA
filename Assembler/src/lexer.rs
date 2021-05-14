// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

mod binary;
mod parser;
mod printer;

use crate::binary::instructions::name2instr;
use crate::compiler::Expected;
use crate::errors::UnknownInstruction;
use crate::span::{Span, Spanned};
use crate::Compiler;
pub use documentation::RegisterKind;
pub use parser::parse;
use std::collections::BTreeSet;
use std::convert::TryFrom;
use std::convert::TryInto;
use std::{collections::BTreeMap, hash::Hash};

macro_rules! gen_register {
    ($($name:ident = $e:ident,)*) => {
        #[derive(Copy, Clone, Debug, Eq, PartialEq, Hash, Ord, PartialOrd)]
        pub enum Register {
            $($name(register::$name),)*
        }

        impl Register {
            pub fn with_id(self, new_id: u32) -> Self {
                match self {
                    $(Register::$name(_) => Register::$name(register::$name(new_id)),)*
                }
            }

            pub fn kind(&self) -> RegisterKind {
                match self {
                    $(Register::$name(_) => RegisterKind::$name,)*
                }
            }

            pub fn map(self, f: impl FnOnce(u32) -> u32) -> Self {
                match self {
                    $(Register::$name(i) => Register::$name(register::$name(f(i.0))),)*
                }
            }

            pub fn new(kind: RegisterKind, id: u32) -> Self {
                match kind {
                    RegisterKind::Bit => Register::Regint(register::Regint(id)),
                    $(
                        RegisterKind::$name => Register::$name(register::$name(id)),
                    )*
                }
            }
        }

        impl std::fmt::Display for Register {
            fn fmt(&self, fmt: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
                match self {
                    $(Register::$name(x) => x.fmt(fmt),)*
                }
            }
        }
        impl Default for Register {
            fn default() -> Register {
                Register::Secret(Default::default())
            }
        }
        pub mod register {
            use crate::span::Spanned;
            $(
                #[derive(Copy, Clone, Debug, Eq, PartialEq, Default, Hash, Ord, PartialOrd)]
                pub struct $name(pub u32);
                impl From<$name> for super::Register {
                    fn from(r: $name) -> super::Register {
                        super::Register::$name(r)
                    }
                }
                impl From<$name> for super::Operand {
                    fn from(r: $name) -> super::Operand {
                        super::Operand::Register(r.into())
                    }
                }
                impl From<Spanned<$name>> for Spanned<super::Register> {
                    fn from(r: Spanned<$name>) -> Self {
                        r.map(Into::into)
                    }
                }
                impl From<Spanned<$name>> for Spanned<super::Operand> {
                    fn from(r: Spanned<$name>) -> Self {
                        r.map(Into::into)
                    }
                }
                impl std::convert::TryFrom<super::Register> for $name {
                    type Error = ();
                    fn try_from(reg: super::Register) -> Result<$name, ()> {
                        match reg {
                            super::Register::$name(me) => Ok(me),
                            _ => Err(()),
                        }
                    }
                }
                impl std::convert::TryFrom<super::Operand> for $name {
                    type Error = ();
                    fn try_from(op: super::Operand) -> Result<$name, ()> {
                        match op {
                            super::Operand::Register(super::Register::$name(me)) => Ok(me),
                            _ => Err(()),
                        }
                    }
                }
                impl std::fmt::Display for $name {
                    fn fmt(&self, fmt: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
                        write!(fmt, "{}{}", stringify!($e), self.0)
                    }
                }
                impl crate::compiler::Expected for $name {
                    const EXPECTED: &'static str = stringify!($name);
                }
                impl From<Vec<Spanned<$name>>> for super::RegisterVec {
                    fn from(args: Vec<Spanned<$name>>) -> Self {
                        super::RegisterVec::$name(args)
                    }
                }
            )*
        }
        #[derive(Clone, Debug)]
        pub enum RegisterVec {
            $($name(Vec<Spanned<register::$name>>),)*
        }
        impl RegisterVec {
            pub fn iter(&self) -> Vec<Spanned<Register>> {
                match self {
                    $(RegisterVec::$name(v) => v.iter().map(|e| e.map(|r| r.into())).collect(),)*
                }
            }
        }

        impl MapAllValues<Register> for RegisterVec {
            fn map_all_values(&mut self, cx: &Compiler, mut f: impl FnMut(Register) -> Register)
            {
                match self {
                    $(RegisterVec::$name(v) => for reg in v {
                        reg.map_all_values(cx, &mut f);
                    },)*
                }
            }
        }

        #[derive(Copy, Clone, Debug, Default)]
        pub struct RegisterStruct<T> {
            $($e: T,)*
        }

        impl<T: Clone + Default> RegisterStruct<Vec<T>> {
            pub fn get(&mut self, register: Register) -> &mut T {
                match register {
                    $(Register::$name(reg) => {
                        let id = reg.0 as usize;
                        if id >= self.$e.len() {
                            self.$e.resize(id + 1, Default::default());
                        }
                        &mut self.$e[id]
                    },)*
                }
            }
        }

        impl RegisterStruct<BTreeSet<u32>> {
            #[must_use]
            pub fn insert(&mut self, register: Register) -> bool {
                match register {
                    $(Register::$name(reg) => self.$e.insert(reg.0),)*
                }
            }
            #[must_use]
            pub fn remove(&mut self, register: Register) -> bool {
                match register {
                    $(Register::$name(reg) => self.$e.remove(&reg.0),)*
                }
            }
        }

        impl<T> RegisterStruct<BTreeMap<u32, T>> {
            pub fn get_or_insert(&mut self, register: Register, init: impl Fn() -> Option<T>, modify: impl Fn(&mut T)) {
                match register {
                    $(Register::$name(reg) => match self.$e.entry(reg.0) {
                        std::collections::btree_map::Entry::Occupied(mut entry) => modify(entry.get_mut()),
                        std::collections::btree_map::Entry::Vacant(entry) => if let Some(new) = init() {
                            entry.insert(new);
                        },
                    },)*
                }
            }

            #[must_use]
            pub fn get(&self, register: Register) -> Option<&T> {
                match register {
                    $(Register::$name(reg) => self.$e.get(&reg.0),)*
                }
            }

            #[must_use]
            pub fn insert(&mut self, register: Register, t: T) -> Option<T> {
                match register {
                    $(Register::$name(reg) => self.$e.insert(reg.0, t),)*
                }
            }

            #[must_use]
            pub fn remove(&mut self, register: Register) -> Option<T> {
                match register {
                    $(Register::$name(reg) => self.$e.remove(&reg.0),)*
                }
            }
        }

        impl<T> RegisterStruct<T> {
            /// Access just a single register bank's entry
            pub fn access(&self, kind: RegisterKind) -> &T {
                match kind {
                    RegisterKind::Bit => &self.r,
                    $(RegisterKind::$name => &self.$e,)*
                }
            }

            /// Access just a single register bank's entry
            pub fn access_mut(&mut self, kind: RegisterKind) -> &mut T {
                match kind {
                    RegisterKind::Bit => &mut self.r,
                    $(RegisterKind::$name => &mut self.$e,)*
                }
            }

            /// Transform all register banks' value
            pub fn map<U>(self, mut f: impl FnMut(T) -> U) -> RegisterStruct<U> {
                RegisterStruct {
                    $($e: f(self.$e),)*
                }
            }

            /// Transform a register while accessing the appropriate bank.
            #[must_use]
            pub fn map_reg(self, register: Register, mut f: impl FnMut(u32, T) -> u32) -> Register {
                match register {
                    $(Register::$name(reg) => Register::$name(register::$name(f(reg.0, self.$e))),)*
                }
            }

            #[must_use]
            /// Create a `RegisterStruct` that takes ownership of the data and tuples it.
            /// When you use `map` next, you get tuples of the elements.
            pub fn join<U>(self, other: RegisterStruct<U>) -> RegisterStruct<(T, U)> {
                RegisterStruct {
                    $($e: (self.$e, other.$e),)*
                }
            }

            #[must_use]
            /// Create a `RegisterStruct` of references so `map` does not consume the original.
            pub fn as_ref(&self) -> RegisterStruct<&T> {
                RegisterStruct {
                    $($e: &self.$e,)*
                }
            }

            #[must_use]
            /// Create a `RegisterStruct` of references so `map` does not consume the original.
            pub fn as_mut(&mut self) -> RegisterStruct<&mut T> {
                RegisterStruct {
                    $($e: &mut self.$e,)*
                }
            }
        }
    }
}

gen_register! {
    Secret = s,
    Clear = c,
    Regint = r,
    SecretRegint = sr,
    SecretBit = sb,
}

#[derive(Debug, Eq, PartialEq, Clone)]
pub struct Lexical<'a> {
    pub instruction: &'a str,
    pub args: Vec<Spanned<Operand>>,
    pub comment: Span,
    pub span: Span,
}

impl<'a> Lexical<'a> {
    pub fn nop() -> Self {
        Self {
            instruction: "nop",
            args: Vec::new(),
            comment: Span::DUMMY,
            span: Span::DUMMY,
        }
    }
    pub fn fetch_instr<'b>(
        &'b self,
        cx: &Compiler,
    ) -> Option<(
        &'static crate::binary::instructions::Instruction,
        &'b [Spanned<Operand>],
    )> {
        let (instr, args) = if self.instruction.starts_with('v') {
            (&self.instruction[1..], &self.args[1..])
        } else {
            (self.instruction, &self.args[..])
        };
        let instr = name2instr(instr);
        if instr.is_none() {
            // nops aren't real instructions, don't report errors on them
            if self.instruction != "nop" && self.instruction != "vnop" {
                cx.report(UnknownInstruction {
                    span: self.span,
                    instruction: self.instruction,
                });
            }
        }
        instr.map(|i| (i, args))
    }
}

check_type_size!(LEXICAL: Lexical<'static>, 72);

impl<'a> Lexical<'a> {
    pub fn args(&self) -> Spanned<&[Spanned<Operand>]> {
        self.span.with(&self.args)
    }
}

#[derive(Copy, Clone, Debug, Eq, PartialEq, Hash)]
pub enum Operand {
    Register(Register),
    Value(Const),
}

#[derive(Copy, Clone, Debug)]
pub enum Const {
    // FIXME: also need unsigned integer
    Int(i32),
    Bool(bool),
}

impl Hash for Const {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        self.as_int().hash(state)
    }
}

impl std::cmp::Eq for Const {}
// Comparison ignores the type of the constant and just looks at the value.
// false == 0 and true == 1
impl std::cmp::PartialEq for Const {
    fn eq(&self, other: &Const) -> bool {
        self.as_int() == other.as_int()
    }
}

impl Const {
    pub fn as_int(self) -> i32 {
        match self {
            Const::Int(i) => i,
            Const::Bool(i) => i as _,
        }
    }
}

impl Default for Operand {
    fn default() -> Self {
        Operand::Value(Const::Int(i32::max_value()))
    }
}

impl From<Register> for Operand {
    fn from(reg: Register) -> Self {
        Operand::Register(reg)
    }
}

impl crate::compiler::Expected for Register {
    const EXPECTED: &'static str = "register";
}

impl crate::compiler::Expected for i32 {
    const EXPECTED: &'static str = "integer";
}

impl crate::compiler::Expected for u32 {
    const EXPECTED: &'static str = "positive integer";
}

impl crate::compiler::Expected for Operand {
    const EXPECTED: &'static str = "operand";
}

impl From<i32> for Operand {
    fn from(i: i32) -> Self {
        Operand::Value(Const::Int(i))
    }
}

impl TryFrom<u32> for Operand {
    type Error = ();
    fn try_from(u: u32) -> Result<Self, ()> {
        i32::try_from(u).map_err(drop).map(Into::into)
    }
}

impl TryFrom<usize> for Operand {
    type Error = ();
    fn try_from(u: usize) -> Result<Self, ()> {
        i32::try_from(u).map_err(drop).map(Into::into)
    }
}

impl TryFrom<Operand> for i32 {
    type Error = ();
    fn try_from(op: Operand) -> Result<Self, ()> {
        match op {
            Operand::Value(Const::Int(i)) => Ok(i),
            Operand::Value(_) | Operand::Register(_) => Err(()),
        }
    }
}

impl TryFrom<Operand> for u32 {
    type Error = ();
    fn try_from(op: Operand) -> Result<Self, ()> {
        match op {
            Operand::Value(Const::Int(i)) => u32::try_from(i).map_err(drop),
            Operand::Value(_) | Operand::Register(_) => Err(()),
        }
    }
}

impl TryFrom<Operand> for Register {
    type Error = ();
    fn try_from(op: Operand) -> Result<Register, ()> {
        match op {
            Operand::Register(reg) => Ok(reg),
            Operand::Value(_) => Err(()),
        }
    }
}

impl Operand {
    pub fn try_as_register(self) -> Result<Register, ()> {
        self.try_into()
    }
    pub fn try_as_int(self) -> Result<i32, ()> {
        self.try_into()
    }
}

pub trait MapAllValues<T> {
    fn map_all_values(&mut self, cx: &Compiler, f: impl FnMut(T) -> T);
}

impl<
        T: std::fmt::Display + std::fmt::Debug + Copy,
        U: Expected + Default + TryFrom<T> + Into<T> + Copy + std::fmt::Display,
    > MapAllValues<T> for Spanned<U>
{
    fn map_all_values(&mut self, cx: &Compiler, mut f: impl FnMut(T) -> T) {
        *self = self.map(|this| f(this.into())).require(cx);
    }
}

impl MapAllValues<Register> for Spanned<Operand> {
    fn map_all_values(&mut self, _cx: &Compiler, mut f: impl FnMut(Register) -> Register) {
        match self.elem {
            Operand::Value(_) => {}
            Operand::Register(reg) => self.elem = Operand::Register(f(reg)),
        }
    }
}

impl<U: std::fmt::Display + std::fmt::Debug + Copy> Spanned<U> {
    pub fn require<T: Expected + Default + TryFrom<U>>(self, cx: &Compiler) -> Spanned<T> {
        cx.checked_from(self)
    }
}

impl Spanned<Operand> {
    pub fn require_uint(self, cx: &Compiler) -> Spanned<u32> {
        // FIXME: error, not panic
        cx.checked_from(self).map(|i: i32| i.try_into().unwrap())
    }
}

impl<T: TryInto<Operand> + Copy + std::fmt::Display + std::fmt::Debug> Spanned<T> {
    pub fn into_operand(self, cx: &Compiler) -> Spanned<Operand> {
        cx.checked_from(self)
    }
}
