// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

//! A peephole optimization for recognizing the `!=` pattern before jump conditions
//! that we can completely lower into the jump condition itself.

use crate::Compiler;
use crate::{
    asm::{Body, JumpMode, Statement, Terminator},
    lexer::Register,
    span::Span,
};

#[derive(Default)]
pub struct Pass;

impl super::Pass for Pass {
    fn apply<'a>(&mut self, cx: &'a Compiler, body: &mut Body<'a>) {
        for block in &mut body.blocks {
            if let Terminator::Jump(jmp) = &mut block.terminator.elem {
                if let JumpMode::Conditional(cnd) = &mut jmp.mode {
                    if cnd.constant.elem == 0 {
                        if let Some((dest, reg, val)) = find_elements(&block.stmts) {
                            if Register::from(cnd.register.elem) == dest {
                                cnd.register = Span::DUMMY.with(reg).require(cx);
                                cnd.constant.elem = val;
                            }
                        }
                    }
                }
            }
        }
    }

    fn name(&self) -> &'static str {
        "cmp"
    }
}

#[instrument(skip(stmts))]
fn find_elements(stmts: &[Statement<'_>]) -> Option<(Register, Register, i32)> {
    let mut iter = stmts.iter().rev();
    let neg = iter.next()?;
    let one = iter.next()?;
    let eq = iter.next()?;
    let constant = iter.next()?;

    let (constant, val) = constant.instr.as_assign()?;
    let eq_val = val.try_as_int().ok()?;

    let (eq_result, a, b) = eq.instr.as_binop("eqint")?;
    let eq_reg = match (a == constant, b == constant) {
        (true, true) => unreachable!(),
        (true, false) => b,
        (false, true) => a,
        (false, false) => return None,
    };

    let (one, val) = one.instr.as_assign()?;
    if val.try_as_int().ok()? != 1 {
        return None;
    }

    let (dest, a, b) = neg.instr.as_binop("subint")?;
    if a != one || b != eq_result {
        return None;
    }
    Some((dest, eq_reg, eq_val))
}
