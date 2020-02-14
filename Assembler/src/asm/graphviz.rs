//! This module contains the graphviz renderer for our basic block structure

use super::{Body, JumpMode, Statement, Terminator};
use crate::Compiler;
use std::io::{self, Write};

impl<'a> Body<'a> {
    pub fn print_dot_file<F: Write>(&self, cx: &'a Compiler, mut f: F) -> io::Result<()> {
        writeln!(f, "digraph G {{")?;
        writeln!(f, "start [shape=Mdiamond];")?;
        writeln!(f, "start -> b0;")?;
        for (i, block) in self.blocks.iter().enumerate() {
            match &block.terminator.elem {
                Terminator::Return { .. } => writeln!(f, "b{} -> return;", i)?,
                Terminator::Done => writeln!(f, "b{} -> end;", i)?,
                Terminator::Crash { .. } => writeln!(f, "b{} -> crash", i)?,
                Terminator::Restart { .. } => writeln!(f, "b{} -> restart", i)?,
                Terminator::Jump(jump) => {
                    let target_block = match &jump.mode {
                        JumpMode::Conditional(cond) => {
                            let instr = if cond.jump_if_zero { "jmpeqz" } else { "jmpnz" };
                            writeln!(
                                f,
                                "b{} -> b{} [label = \"{} {}\"]",
                                i, jump.target_block, instr, cond.register.elem
                            )?;
                            cond.fallthrough_block
                        }
                        JumpMode::Call | JumpMode::Goto => jump.target_block,
                    };
                    writeln!(f, "b{} -> b{}", i, target_block)?;
                }
            }
            write!(f, "b{} [shape=\"none\" label=<", i)?;
            write!(f, r#"<table border="0" cellborder="1" cellspacing="0">"#)?;
            write!(f, r#"<tr><td align="left" balign="left">"#)?;
            let (end, start2) = if block.stmts.len() > 10 {
                (5, block.stmts.len() - 5)
            } else {
                (block.stmts.len(), block.stmts.len())
            };
            let print_stmt = |f: &mut F, stmt: &Statement<'_>| {
                write!(
                    f,
                    "{}<br/>",
                    v_htmlescape::escape(&stmt.relex(cx).to_string())
                )
            };
            for stmt in &block.stmts[..end] {
                print_stmt(&mut f, stmt)?
            }
            if block.stmts.len() > 10 {
                write!(
                    f,
                    "... {} elements skipped ...<br/>",
                    block.stmts.len() - 10
                )?;
            }
            for stmt in &block.stmts[start2..] {
                print_stmt(&mut f, stmt)?;
            }
            write!(f, "</td></tr>")?;
            writeln!(f, "</table>")?;
            writeln!(f, ">];")?;
        }
        writeln!(f, "end [shape=Msquare];")?;
        writeln!(f, "}}")?;
        Ok(())
    }
}
