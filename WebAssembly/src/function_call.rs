// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use scasm::{
    asm::{Instruction, Jump, JumpMode, Statement, Terminator},
    lexer::RegisterKind,
    span::Span,
};
use std::collections::BTreeSet;
use tracing::trace;
use walrus::LocalFunction;
use walrus::{
    ir::{dfs_in_order, Visitor},
    LocalId,
};

use crate::{
    error::Error,
    stack::{AsRegisterKind, PersistentKind},
    CurrentBlockHandler,
};

impl<'a, 'bh, 'cx, 'wasm> CurrentBlockHandler<'a, 'bh, 'cx, 'wasm> {
    pub fn handle_call(
        &mut self,
        name: Option<&str>,
        func: &'wasm LocalFunction,
    ) -> Result<(), Error> {
        trace!(name = %name.as_ref().unwrap(), "call");
        let function_start_block = self.add(func);
        trace!(function_start_block);

        // Stash the *current* function's local variables
        let mut locals = Used::default();
        dfs_in_order(&mut locals, func, func.entry_block());
        let locals = locals.locals;

        #[derive(Default)]
        struct Used {
            locals: BTreeSet<LocalId>,
        }

        impl<'a> Visitor<'a> for Used {
            fn visit_local_id(&mut self, id: &LocalId) {
                self.locals.insert(*id);
            }
        }

        // FIXME: bug on the push (we are sometimes calling a push on a register which is not set)
        for local in locals.iter().copied() {
            let val = self.stack.expect_local(local);
            let instruction = match val.kind() {
                RegisterKind::Secret => "pushs",
                RegisterKind::Clear => "pushc",
                RegisterKind::Bit | RegisterKind::Regint => "pushint",
                RegisterKind::SecretRegint => "pushsint",
                RegisterKind::SecretBit => "pushsbit",
            };
            let instr = Instruction::General {
                instruction,
                destinations: vec![],
                values: vec![Span::DUMMY.with(val.into())],
            };
            let comment = self.comment(|f| write!(f, "push for local {}", local.index()))?;
            self.push_stmt(Statement::from_instr_with_comment(instr, comment));
        }

        // Write function arguments
        for &local in func.args.iter().rev() {
            let val = self.stack.pop()?;
            self.assign_to_persistent(PersistentKind::Local(local), val, "function argument")?;
        }

        // Actually perform the call
        let results = self.module.types.results(func.ty());
        let comment = self.comment(|f| write!(f, "call {}", name.as_ref().unwrap()))?;

        let fallthrough_block = self.allocate_new_block();
        let terminator = std::mem::replace(
            self.terminator_mut(),
            Terminator::Jump(Jump {
                target_block: function_start_block,
                comment,
                mode: JumpMode::Call { fallthrough_block },
            }),
        );
        // Continue after the call
        self.scasm_block_id = fallthrough_block;
        self.set_terminator(terminator)?;

        // Restore local variables
        for local in locals.iter().copied().rev() {
            let val = self.stack.expect_local(local);
            let instruction = match val.kind() {
                RegisterKind::Secret => "pops",
                RegisterKind::Clear => "popc",
                RegisterKind::Bit | RegisterKind::Regint => "popint",
                RegisterKind::SecretRegint => "popsint",
                RegisterKind::SecretBit => "popsbit",
            };

            let instr = Instruction::General {
                instruction,
                destinations: vec![comment.with(val)],
                values: vec![],
            };
            let comment = self.comment(|f| write!(f, "pop for local {}", local.index()))?;
            self.push_stmt(Statement::from_instr_with_comment(instr, comment));
        }

        // Read function return values
        for (i, result) in results.iter().enumerate() {
            self.read_from_persistent(
                PersistentKind::FunctionReturn(func.entry_block(), i),
                result.as_register_kind(),
                "read return value passing via stack",
            )?;
        }
        Ok(())
    }
}
