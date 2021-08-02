// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use crate::lexer::{register, Operand, Register};
use crate::span::Spanned;

#[derive(Clone, Debug)]
#[allow(clippy::large_enum_variant)]
pub enum IoInstruction {
    InputShares {
        registers: Vec<Spanned<register::Secret>>,
    },
    OutputShares {
        registers: Vec<Spanned<register::Secret>>,
    },
}

check_type_size!(INSTRUCTION: Instruction<'static>, 72);

#[derive(Debug, Clone)]
#[allow(clippy::large_enum_variant)]
pub enum Instruction<'a> {
    /// All instructions that are uninteresting to optimizations.
    /// The destinations come first and must be writable registers.
    /// The values come after that and must be readable registers.
    /// Assembly instructions that don't have this general pattern
    /// of results first then args, get their own variant.
    General {
        instruction: &'a str,
        destinations: Vec<Spanned<Register>>,
        values: Vec<Spanned<Operand>>,
    },

    /// `ld*` and `mov*` instructions
    Assign {
        destination: Spanned<Register>,
        value: Spanned<Operand>,
    },
    StartOpen {
        registers: Vec<Spanned<register::Secret>>,
    },
    StopOpen {
        registers: Vec<Spanned<register::Clear>>,
    },

    /// Data Instructions
    DataInstr {
        instruction: &'a str,
        registers: Vec<Spanned<register::Secret>>,
    },

    Io {
        instr: IoInstruction,
        channel: Spanned<Register>,
    },

    /// An instruction that doesn't exist in SCALE, will get optimized out
    /// during assembly.
    Nop,
}

impl<'a> Instruction<'a> {
    pub fn as_assign(&self) -> Option<(Register, Operand)> {
        match self {
            Self::Assign { destination, value } => Some((destination.elem, value.elem)),
            _ => None,
        }
    }

    #[allow(clippy::type_complexity)]
    pub fn as_general(&self, name: &str) -> Option<(&[Spanned<Register>], &[Spanned<Operand>])> {
        match self {
            Self::General {
                destinations,
                values,
                instruction,
            } if *instruction == name => Some((destinations, values)),
            _ => None,
        }
    }
    pub fn as_binop(&self, name: &str) -> Option<(Register, Register, Register)> {
        match self.as_general(name)? {
            ([result], [a, b]) => Some((
                result.elem,
                a.elem.try_as_register().ok()?,
                b.elem.try_as_register().ok()?,
            )),
            _ => None,
        }
    }
}
