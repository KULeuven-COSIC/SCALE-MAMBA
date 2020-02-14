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

impl<'a> std::fmt::Display for Lexical<'a> {
    fn fmt(&self, fmt: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(fmt, "{}", self.instruction)?;
        let mut args = self.args.iter().map(|a| &a.elem);
        if let Some(arg) = args.next() {
            write!(fmt, " {}", arg)?;
            for arg in args {
                write!(fmt, ", {}", arg)?;
            }
        }
        if !self.comment.is_empty() {
            if !self.instruction.is_empty() {
                write!(fmt, " ")?;
            }
            write!(fmt, "# {}", self.comment.snippet())?;
        }
        Ok(())
    }
}
