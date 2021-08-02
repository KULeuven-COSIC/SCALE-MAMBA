// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

//! A dummy optimization not changing any code, but just
//! checking various invariants.

use documentation::ArgTy;

use crate::{
    asm::{Body, Statement},
    lexer::{Operand, RegisterKind},
};
use crate::{errors::ExpectedGot, Compiler};

#[derive(Default)]
pub struct Pass;

/// Returns `true` if the statement is valid
pub fn validate(cx: &Compiler, stmt: &Statement<'_>) -> bool {
    let relexed = stmt.relex(cx);
    let (instr, args) = match relexed.0.fetch_instr(cx) {
        Some(i) => i,
        None => return true,
    };
    for (arg_index, (should, &is)) in instr.args.iter().zip(args).enumerate() {
        match should.ty {
            ArgTy::Int { .. } => match is.elem {
                Operand::Register(_) => {
                    cx.report(ExpectedGot {
                        expected: format!(
                            "constant as argument {} to {}",
                            arg_index,
                            relexed.0.display(cx)
                        ),
                        got: is,
                    });
                    return false;
                }
                Operand::Value(_) => {}
            },
            ArgTy::Register(kind, _) => match is.elem {
                Operand::Register(reg)
                    if reg.kind() == RegisterKind::Bit && kind == RegisterKind::Regint => {}
                Operand::Register(reg)
                    if reg.kind() == RegisterKind::Regint && kind == RegisterKind::Bit => {}
                Operand::Register(reg) if reg.kind() == kind => {}
                _ => {
                    cx.report(ExpectedGot {
                        expected: format!(
                            "Register of type {:?} as argument {} to {}",
                            kind,
                            arg_index,
                            relexed.0.display(cx)
                        ),
                        got: is,
                    });
                    return false;
                }
            },
            ArgTy::List { .. } => continue,
        }
    }
    true
}

impl super::Pass for Pass {
    fn apply<'a>(&mut self, cx: &'a Compiler, body: &mut Body<'a>) {
        for block in &body.blocks {
            for stmt in &block.stmts {
                validate(cx, stmt);
            }
        }
    }
    fn name(&self) -> &'static str {
        "validate"
    }
}
