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

    Io {
        instr: IoInstruction,
        channel: Spanned<u32>,
    },

    /// An instruction that doesn't exist in SCALE, will get optimized out
    /// during assembly.
    Nop,
}
