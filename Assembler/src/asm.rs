// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

mod augmented_block;
mod graphviz;
mod instructions;
mod parser;
mod relexer;
mod statement;

pub use instructions::*;

use std::num::NonZeroU32;

use crate::lexer::register;
use crate::span::{Span, Spanned};

#[derive(Debug, Default)]
/// Represents an entire file
pub struct Body<'a> {
    pub blocks: Vec<Block<'a>>,
}

#[derive(Debug)]
pub struct Block<'a> {
    pub stmts: Vec<Statement<'a>>,
    pub terminator: Spanned<Terminator>,
}

impl<'a> Default for Block<'a> {
    fn default() -> Self {
        Self {
            stmts: Default::default(),
            terminator: Terminator::Done.into(),
        }
    }
}

#[derive(Debug, Clone)]
pub struct Statement<'a> {
    pub instr: Instruction<'a>,
    // FIXME: what's the maximum value for `vectorized`?
    // XXXXX: Answer : 2^23
    pub vectorized: Spanned<NonZeroU32>,
    pub comment: Span,
    pub span: Span,
}

impl<'a> From<Instruction<'a>> for Statement<'a> {
    fn from(instr: Instruction<'a>) -> Self {
        Self::from_instr_with_comment(instr, Span::DUMMY)
    }
}

impl<'a> Statement<'a> {
    pub fn from_instr_with_comment(instr: Instruction<'a>, comment: Span) -> Self {
        Self {
            instr,
            vectorized: NonZeroU32::new(1).unwrap().into(),
            comment,
            span: Span::DUMMY,
        }
    }
}

#[derive(Debug, Clone)]
pub struct AugmentedBlock<'a> {
    pub stmts: Vec<AugmentedStatement<'a>>,
}

#[derive(Debug, Clone)]
pub struct AugmentedStatement<'a> {
    pub instruction_depth: i32,
    pub round_depth: i32,
    pub blck: i32,
    pub stmt: Statement<'a>,
    pub processed: bool,
}

check_type_size!(STATEMENT: Statement<'static>, 128);

#[derive(Debug)]
pub enum Terminator {
    Jump(Jump),
    Restart { comment: Span },
    Crash { comment: Span },
    Return { comment: Span },
    Done,
}

#[derive(Debug)]
pub struct Jump {
    pub target_block: usize,
    pub mode: JumpMode,
    pub comment: Span,
}

#[derive(Debug)]
pub enum JumpMode {
    Goto,
    Conditional(JumpCondition),
    /// Calls in SCALE are just jumps that push their address onto a stack.
    /// `return` will pop the last item from the stack and jump there.
    Call {
        fallthrough_block: usize,
    },
}

#[derive(Debug)]
pub struct JumpCondition {
    pub fallthrough_block: usize,
    /// If true, jumps to `target_block` on equality and to
    /// `fallthrough_block` on inequality.
    /// If false, jumps to `target_block` on inequality and to
    /// `fallthrough_block` on equality.
    pub jump_if_equal: bool,
    /// The constant to compare against
    pub constant: Spanned<i32>,
    /// Register to be checked
    pub register: Spanned<register::Regint>,
}
