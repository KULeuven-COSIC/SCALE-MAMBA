// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

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
            table_start(&mut f, i)?;
            let (end, start2) = if block.stmts.len() > 11 && !cx.verbose {
                (5, block.stmts.len() - 5)
            } else {
                (block.stmts.len(), block.stmts.len())
            };
            let print_stmt = |f: &mut F, stmt: &Statement<'_>| -> io::Result<()> {
                let lex = stmt.relex(cx).0;
                write!(
                    f,
                    "{}<br/>",
                    v_htmlescape::escape(&lex.display(cx).to_string())
                )?;
                Ok(())
            };
            for stmt in &block.stmts[..end] {
                print_stmt(&mut f, stmt)?
            }
            if block.stmts.len() > 11 && !cx.verbose {
                write!(
                    f,
                    "... {} elements skipped ...<br/>",
                    block.stmts.len() - 11
                )?;
            }
            for stmt in &block.stmts[start2..] {
                print_stmt(&mut f, stmt)?;
            }
            table_end(&mut f)?;
            match &block.terminator.elem {
                Terminator::Return { .. } => writeln!(f, "b{} -> return_{};", i, i)?,
                Terminator::Done => writeln!(f, "b{} -> end;", i)?,
                Terminator::Crash { .. } => writeln!(f, "b{} -> crash", i)?,
                Terminator::Restart { .. } => writeln!(f, "b{} -> restart", i)?,
                Terminator::Jump(jump) => {
                    let target_block = match &jump.mode {
                        JumpMode::Conditional(cond) => {
                            let instr = if cond.jump_if_equal { "jmpeq" } else { "jmpne" };
                            writeln!(
                                f,
                                "b{} -> b{} [label = \"{} {} {}\"]",
                                i, jump.target_block, instr, cond.register.elem, cond.constant.elem,
                            )?;
                            cond.fallthrough_block
                        }
                        JumpMode::Call { fallthrough_block } => {
                            writeln!(
                                f,
                                "b{} -> b{} [label = \"call\", style = \"dotted\"]",
                                i, jump.target_block,
                            )?;
                            *fallthrough_block
                        }
                        JumpMode::Goto => jump.target_block,
                    };
                    writeln!(
                        f,
                        "b{} -> b{} [label = \"{}\"]",
                        i,
                        target_block,
                        jump.comment.snippet(cx)
                    )?;
                }
            }
        }
        writeln!(f, "end [shape=Msquare];")?;
        writeln!(f, "}}")?;
        Ok(())
    }
}

fn table_end<F: Write>(mut f: F) -> io::Result<()> {
    write!(f, "</td></tr>")?;
    writeln!(f, "</table>")?;
    writeln!(f, ">];")?;
    Ok(())
}

fn table_start<F: Write>(mut f: F, i: impl std::fmt::Display) -> io::Result<()> {
    write!(f, "b{} [shape=\"none\" label=<", i)?;
    write!(f, r#"<table border="0" cellborder="1" cellspacing="0">"#)?;
    write!(f, r#"<tr><td align="left" balign="left">"#)?;
    Ok(())
}
