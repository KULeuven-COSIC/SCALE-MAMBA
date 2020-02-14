mod augmented_block;
mod graphviz;
mod instructions;
mod parser;
mod relexer;
mod statement;

pub use instructions::*;

use std::num::NonZeroU16;

use crate::lexer::Register;
use crate::span::{Span, Spanned};

#[derive(Debug, Clone)]
/// Represents an entire file
pub struct Body<'a> {
    pub blocks: Vec<Block<'a>>,
}

#[derive(Debug, Clone)]
pub struct Block<'a> {
    pub stmts: Vec<Statement<'a>>,
    pub terminator: Spanned<'a, Terminator<'a>>,
}

#[derive(Debug, Clone)]
pub struct Statement<'a> {
    pub instr: Instruction<'a>,
    // FIXME: what's the maximum value for `vectorized`?
    // XXXXX: Answer : 2^23
    pub vectorized: Spanned<'a, NonZeroU16>,
    pub comment: Span<'a>,
    pub span: Span<'a>,
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

check_type_size!(STATEMENT: Statement<'static>, 240);

#[derive(Copy, Clone, Debug, Eq, PartialEq)]
pub enum MemoryBank {
    Secret,
    Clear,
    Regint,
    SecretRegint,
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum BinaryOperation {
    Add,
    Mul,
    Sub,
    Div,
    Mod,
    And,
    Or,
    Xor,
    Not,
    /// `<` operation
    Lt,
    /// `>` operation
    Gt,
    /// `==` operation
    Eq,
    /// `<<` operation
    Shl,
    /// `>>` operation
    Shr,
    /// Extract b-th bit from a
    Bit,
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum UnaryOperation {
    Legendre,
    Digestc,
    Neg,
    Inv,
    Lt,
    Eq,
    Rand,
}

#[derive(Clone, Debug)]
pub enum Terminator<'a> {
    Jump(Jump<'a>),
    Restart { comment: Span<'a> },
    Crash { comment: Span<'a> },
    Return { comment: Span<'a> },
    Done,
}

#[derive(Clone, Debug)]
pub struct Jump<'a> {
    pub target_block: usize,
    pub mode: JumpMode<'a>,
    pub comment: Span<'a>,
}

#[derive(Clone, Debug)]
pub enum JumpMode<'a> {
    Goto,
    Conditional(JumpCondition<'a>),
    /// Calls in SCALE are just jumps that push their address onto a stack.
    /// `return` will pop the last item from the stack and jump there.
    Call,
}

#[derive(Clone, Debug)]
pub struct JumpCondition<'a> {
    pub fallthrough_block: usize,
    /// If true, jumps to `target_block` on zero and to
    /// `fallthrough_block` on nonzero
    pub jump_if_zero: bool,
    // FIXME: should only work on clear registers, right?
    // Indeed XXXX
    /// Register to be checked
    pub register: Spanned<'a, Register>,
}
