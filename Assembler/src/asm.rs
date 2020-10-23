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

#[derive(Debug, Clone, Default)]
/// Represents an entire file
pub struct Body<'a> {
    pub blocks: Vec<Block<'a>>,
}

#[derive(Debug, Clone)]
pub struct Block<'a> {
    pub stmts: Vec<Statement<'a>>,
    pub terminator: Spanned<Terminator>,
}

impl<'a> Default for Block<'a> {
    fn default() -> Self {
        Self {
            stmts: Default::default(),
            terminator: Span::DUMMY.with(Terminator::Done),
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
            vectorized: Span::DUMMY.with(NonZeroU32::new(1).unwrap()),
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

#[derive(Clone, Debug)]
pub enum Terminator {
    Jump(Jump),
    Restart { comment: Span },
    Crash { comment: Span },
    Return { comment: Span },
    Done,
}

#[derive(Clone, Debug)]
pub struct Jump {
    pub target_block: usize,
    pub mode: JumpMode,
    pub comment: Span,
}

#[derive(Clone, Debug)]
pub enum JumpMode {
    Goto,
    Conditional(JumpCondition),
    /// Calls in SCALE are just jumps that push their address onto a stack.
    /// `return` will pop the last item from the stack and jump there.
    Call,
}

#[derive(Clone, Debug)]
pub struct JumpCondition {
    pub fallthrough_block: usize,
    /// If true, jumps to `target_block` on zero and to
    /// `fallthrough_block` on nonzero
    pub jump_if_zero: bool,
    /// Register to be checked
    pub register: Spanned<register::Regint>,
}
