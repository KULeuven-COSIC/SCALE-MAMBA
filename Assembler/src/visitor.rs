// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use crate::asm::{Block, Body, Jump, JumpCondition, JumpMode, Statement, Terminator};
use crate::{lexer::Register, Compiler};

pub trait Visitor<'a> {
    fn cx(&self) -> &'a Compiler;
    fn visit_body(&mut self, body: &mut Body<'a>) {
        self.walk_body(body)
    }
    fn walk_body(&mut self, body: &mut Body<'a>) {
        let Body { blocks } = body;
        for block in blocks {
            self.visit_block(block);
        }
    }

    fn visit_block(&mut self, block: &mut Block<'a>) {
        self.walk_block(block)
    }
    fn walk_block(&mut self, block: &mut Block<'a>) {
        let Block { stmts, terminator } = block;
        for stmt in stmts {
            self.visit_stmt(stmt);
        }
        self.visit_terminator(&mut terminator.elem);
    }

    fn visit_stmt(&mut self, stmt: &mut Statement<'a>) {
        self.walk_stmt(stmt)
    }
    fn walk_stmt(&mut self, stmt: &mut Statement<'a>) {
        let Statement {
            instr,
            vectorized,
            comment,
            span,
        } = stmt;
        let _ = vectorized;
        let _ = comment;
        let _ = span;
        let _ = instr;

        let cx = self.cx();
        stmt.replace_registers(cx, |mut reg| {
            self.visit_register(&mut reg);
            reg
        });
    }
    fn visit_terminator(&mut self, terminator: &mut Terminator) {
        self.walk_terminator(terminator)
    }
    fn walk_terminator(&mut self, terminator: &mut Terminator) {
        match terminator {
            Terminator::Jump(jmp) => {
                let Jump {
                    target_block,
                    comment,
                    mode,
                } = jmp;
                let _ = comment;
                self.visit_block_id(target_block);
                match mode {
                    JumpMode::Goto => {}
                    JumpMode::Conditional(cnd) => {
                        let JumpCondition {
                            fallthrough_block,
                            jump_if_equal,
                            constant,
                            register,
                        } = cnd;
                        let _ = jump_if_equal;
                        let _ = constant;
                        self.visit_block_id(fallthrough_block);
                        let mut reg = register.map(Register::from);
                        self.visit_register(&mut reg.elem);
                        *register = reg.require(self.cx());
                    }
                    JumpMode::Call { fallthrough_block } => self.visit_block_id(fallthrough_block),
                }
            }
            Terminator::Restart { comment }
            | Terminator::Crash { comment }
            | Terminator::Return { comment } => {
                let _ = comment;
            }
            Terminator::Done => {}
        }
    }
    fn visit_block_id(&mut self, _id: &mut usize) {}
    fn visit_register(&mut self, _reg: &mut Register) {}
}
