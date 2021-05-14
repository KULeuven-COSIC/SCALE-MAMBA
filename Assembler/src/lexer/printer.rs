// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use super::*;

impl std::fmt::Display for Operand {
    fn fmt(&self, fmt: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Operand::Register(reg) => reg.fmt(fmt),
            Operand::Value(i) => i.fmt(fmt),
        }
    }
}

impl std::fmt::Display for Const {
    fn fmt(&self, fmt: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Const::Bool(b) => b.fmt(fmt),
            Const::Int(i) => i.fmt(fmt),
        }
    }
}

impl<'a> Lexical<'a> {
    pub fn display(&'a self, cx: &'a Compiler) -> LexicalDisplay<'a> {
        LexicalDisplay(self, cx)
    }
}

pub struct LexicalDisplay<'a>(&'a Lexical<'a>, &'a Compiler);

impl<'a> std::fmt::Display for LexicalDisplay<'a> {
    fn fmt(&self, fmt: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(fmt, "{}", self.0.instruction)?;
        // FIXME: check that all arguments have the correct type by checking against the
        // documentation in binary/instructions.rs
        let mut args = self.0.args.iter().map(|a| &a.elem);
        if let Some(arg) = args.next() {
            write!(fmt, " {}", arg)?;
            for arg in args {
                write!(fmt, ", {}", arg)?;
            }
        }
        if !self.0.comment.is_empty() {
            if !self.0.instruction.is_empty() {
                write!(fmt, " ")?;
            }
            write!(fmt, "# {}", self.0.comment.snippet(self.1))?;
        }
        Ok(())
    }
}
