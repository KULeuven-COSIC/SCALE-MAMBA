// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use anyhow::Context as _;
use scasm::lexer::{Operand, Register, RegisterKind, RegisterStruct};
use std::collections::HashMap;
use tracing::{instrument, trace};
use walrus::{ir::InstrSeqId, GlobalId, LocalId, ValType};

#[derive(Default)]
pub struct Stack {
    stack: Vec<Operand>,
    regs: RegisterStruct<u32>,
    globals: HashMap<GlobalId, Register>,
    /// Locals, arguments and return types.
    persistent: HashMap<PersistentKind, Register>,
}

impl Stack {
    pub fn from_locals<'a>(module_locals: impl Iterator<Item = &'a walrus::Local>) -> Self {
        let mut stack = Self::default();
        for local in module_locals {
            let kind = local.ty().as_register_kind();
            stack.local(PersistentKind::Local(local.id()), kind);
        }
        stack
    }
}

#[derive(Debug, Copy, Clone, Eq, PartialEq, Hash)]
pub enum PersistentKind {
    Local(LocalId),
    BlockReturn(InstrSeqId),
    /// The id is the id of the entry block
    FunctionReturn(InstrSeqId, usize),
}

pub trait AsRegisterKind {
    fn as_register_kind(&self) -> RegisterKind;
}

impl AsRegisterKind for ValType {
    fn as_register_kind(&self) -> RegisterKind {
        match self {
            ValType::I32 | ValType::I64 => RegisterKind::Regint,
            ValType::F32 => RegisterKind::Secret,
            ValType::F64 => RegisterKind::SecretRegint,
            ValType::V128 => RegisterKind::Clear,
            ValType::Externref => unreachable!(),
            ValType::Funcref => unreachable!(),
        }
    }
}

impl Stack {
    pub fn dump_stack(&self) {
        trace!("{:#?}", self.stack);
    }

    pub fn expect_stack_empty(&self) -> anyhow::Result<()> {
        if !self.stack.is_empty() {
            anyhow::bail!("stack is {:#?}", self.stack)
        }
        Ok(())
    }

    /// Creates a register that is not on the stack.
    #[instrument(skip(self))]
    pub fn temp(&mut self, kind: RegisterKind) -> Register {
        let cregs = self.regs.access_mut(kind);
        let reg = Register::new(kind, *cregs);
        *cregs += 1;
        reg
    }

    pub fn push_temp(&mut self, kind: RegisterKind) -> Register {
        let reg = self.temp(kind);
        self.push(reg);
        reg
    }

    pub fn push(&mut self, op: impl Into<Operand>) {
        let op = op.into();
        trace!(?op, "push");
        self.stack.push(op);
    }

    pub fn pop(&mut self) -> anyhow::Result<Operand> {
        let op = self
            .stack
            .pop()
            .context("invalid wasm: stack did not contain any element on pop")?;
        trace!(?op, "pop");
        Ok(op)
    }

    pub fn read_top(&self) -> anyhow::Result<Operand> {
        let op = self
            .stack
            .last()
            .copied()
            .context("invalid wasm: stack did not contain any element on read_top")?;
        trace!(?op, "read_top");
        Ok(op)
    }

    #[instrument(skip(self))]
    pub fn local(&mut self, delay: PersistentKind, register_kind: RegisterKind) -> Register {
        if let Some(&r) = self.persistent.get(&delay) {
            return r;
        }
        let persistent_reg = self.temp(register_kind);
        self.persistent.insert(delay, persistent_reg);
        persistent_reg
    }

    #[instrument(skip(self))]
    #[track_caller]
    pub fn expect_local(&mut self, local: LocalId) -> Register {
        self.persistent[&PersistentKind::Local(local)]
    }

    #[instrument(skip(self))]
    pub fn global(&mut self, global: GlobalId) -> anyhow::Result<Register> {
        if let Some(&r) = self.globals.get(&global) {
            if r.kind() != RegisterKind::Regint {
                anyhow::bail!("the register should be a RegInt")
            }
            return Ok(r);
        }
        let persistent_reg = self.temp(RegisterKind::Regint);
        self.globals.insert(global, persistent_reg);
        Ok(persistent_reg)
    }
}
