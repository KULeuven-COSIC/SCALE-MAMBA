// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![warn(clippy::all)]
#![allow(clippy::trivial_regex)]
#![deny(rust_2018_idioms)]

mod error;
mod extern_call;
mod function_call;
mod instruction;
mod stack;

use anyhow::{Context as _, Result};
pub use error::Error;

use std::io::Write as _;
use std::{
    collections::{hash_map::Entry, HashMap},
    convert::TryInto,
    fmt,
};
use std::{fs::File, io::Read, path::PathBuf};

use parse_display::{Display, FromStr};
use scasm::{
    asm::{Body, Instruction, Jump, JumpCondition, JumpMode, Statement, Terminator},
    lexer::{Const, Operand, Register, RegisterKind},
    span::{Expansion, Span, Spanned},
    Compiler,
};
use stack::{PersistentKind, Stack};
use structopt::StructOpt;
use tracing::{debug_span, trace};
use walrus::{
    ir::{BinaryOp, InstrSeq, InstrSeqId, InstrSeqType},
    ExportItem, FunctionKind,
};

#[derive(Display, FromStr, PartialEq, Debug)]
#[display(style = "snake_case")]
enum InputFormat {
    Assembly,
    Bytecode,
}

#[derive(Debug)]
struct BlockInfo {
    /// The actual id allocated for the wasm block
    scale_block_id: usize,
    /// The block we should jump to when branching to this block.
    /// This can differ from `scale_block_id`, because while for
    /// `loop` we jump to the loop block's start again, for `block`
    /// we jump to the *end* of the block, thus the following block.
    jump_target_block: usize,
    pub has_return: bool,
}

pub struct Context<'a, 'wasm> {
    pub body: Body<'a>,
    cx: &'a Compiler,
    module: &'wasm walrus::Module,
    /// The maximum reqbl seen during transpilation.
    reqbl: Option<i32>,
    /// This value are the address offset computed including the wasm heap
    offset_addr_val: Option<u32>,
    /// This register store the destination address where the heap memory is allocated
    heap_dest_reg: Option<Register>,
    #[allow(dead_code)]
    // FIXME: use this
    exports: HashMap<String, &'wasm ExportItem>,
    to_process: Vec<&'wasm walrus::LocalFunction>,
    /// Map from function start block ids to their scasm block
    seen: HashMap<InstrSeqId, usize>,
    /// Cross function type inference for arguments and return types
    stack: stack::Stack,
}

impl<'a, 'wasm> Context<'a, 'wasm> {
    fn new(
        cx: &'a Compiler,
        exports: HashMap<String, &'wasm ExportItem>,
        module: &'wasm walrus::Module,
    ) -> anyhow::Result<Self> {
        let mut this = Self {
            body: Body::default(),
            cx,
            reqbl: None,
            offset_addr_val: None,
            heap_dest_reg: None,
            exports,
            module,
            to_process: Default::default(),
            seen: Default::default(),
            stack: Stack::from_locals(module.locals.iter()),
        };
        let initial = this.allocate_new_block();
        if initial != 0 {
            anyhow::bail!("Initial block si not zero, it is : {}", initial)
        };

        this.body.blocks[0].terminator.elem = Terminator::Jump(Jump {
            target_block: 1,
            mode: JumpMode::Goto,
            comment: Span::DUMMY,
        });
        Ok(this)
    }

    // Create a new block that does not correspond to a wasm block
    fn allocate_new_block(&mut self) -> usize {
        let id = self.body.blocks.len();
        self.body.blocks.push(Default::default());
        id
    }
}

struct BlockHandler<'a, 'cx, 'wasm> {
    ctx: &'cx mut Context<'a, 'wasm>,
    seen: HashMap<InstrSeqId, BlockInfo>,
    to_process: Vec<(usize, InstrSeqId)>,
    func: &'wasm walrus::LocalFunction,
    is_main_function: bool,
}

impl<'a, 'cx, 'wasm> std::ops::Deref for BlockHandler<'a, 'cx, 'wasm> {
    type Target = Context<'a, 'wasm>;
    fn deref(&self) -> &Self::Target {
        self.ctx
    }
}

impl<'a, 'cx, 'wasm> std::ops::DerefMut for BlockHandler<'a, 'cx, 'wasm> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        self.ctx
    }
}

impl<'a, 'wasm> Context<'a, 'wasm> {
    fn add(&mut self, func: &'wasm walrus::LocalFunction) -> usize {
        if let Some(&already) = self.seen.get(&func.entry_block()) {
            return already;
        }
        let scale_block_id = self.allocate_new_block();
        self.seen.insert(func.entry_block(), scale_block_id);
        self.to_process.push(func);
        scale_block_id
    }

    fn process_next<'cx>(&'cx mut self) -> Option<BlockHandler<'a, 'cx, 'wasm>> {
        let func = self.to_process.pop()?;
        // Maps from wasm block ids to scale block ids
        let mut seen = HashMap::new();
        let scale_block_id = self.seen[&func.entry_block()];
        let to_process = vec![(scale_block_id, func.entry_block())];
        seen.insert(
            func.entry_block(),
            BlockInfo {
                scale_block_id,
                jump_target_block: scale_block_id,
                has_return: false,
            },
        );
        Some(BlockHandler {
            is_main_function: self.seen.len() == 1,
            seen,
            to_process,
            func,
            ctx: self,
        })
    }
}

impl<'a, 'cx, 'wasm> BlockHandler<'a, 'cx, 'wasm> {
    fn process_next<'bh>(&'bh mut self) -> Option<CurrentBlockHandler<'a, 'bh, 'cx, 'wasm>> {
        let (scasm_block_id, block_id) = self.to_process.pop()?;
        let block = self.func.block(block_id);
        Some(CurrentBlockHandler {
            scasm_block_id,
            block,
            bh: self,
        })
    }
    fn comment(
        &mut self,
        f: impl FnOnce(&mut dyn fmt::Write) -> fmt::Result,
    ) -> anyhow::Result<Span> {
        let mut val = String::new();
        f(&mut val).unwrap();
        if val.is_empty() {
            anyhow::bail!("nothing written");
        }
        let file = self.cx.files.len() as u32;
        let text = self.cx.files.push_get(val);
        Ok(Span::new(
            file,
            0,
            text.len().try_into().unwrap(),
            Expansion::Generated,
        ))
    }
}

struct CurrentBlockHandler<'a, 'bh, 'cx, 'wasm> {
    bh: &'bh mut BlockHandler<'a, 'cx, 'wasm>,
    scasm_block_id: usize,
    block: &'wasm InstrSeq,
}

impl<'a, 'bh, 'cx, 'wasm> std::ops::Deref for CurrentBlockHandler<'a, 'bh, 'cx, 'wasm> {
    type Target = BlockHandler<'a, 'cx, 'wasm>;
    fn deref(&self) -> &Self::Target {
        self.bh
    }
}

impl<'a, 'bh, 'cx, 'wasm> std::ops::DerefMut for CurrentBlockHandler<'a, 'bh, 'cx, 'wasm> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        self.bh
    }
}

impl<'a, 'bh, 'cx, 'wasm> CurrentBlockHandler<'a, 'bh, 'cx, 'wasm> {
    fn push_stmt_to_block(&mut self, block: usize, instr: impl Into<Statement<'a>>) -> usize {
        let stmt: Statement<'a> = instr.into();
        trace!(?stmt.instr);
        let stmts = &mut self.body.blocks[block].stmts;
        let id = stmts.len();
        stmts.push(stmt);
        id
    }
    fn push_stmt(&mut self, instr: impl Into<Statement<'a>>) -> usize {
        let block = self.scasm_block_id;
        self.push_stmt_to_block(block, instr)
    }

    fn set_terminator(&mut self, terminator: impl Into<Terminator>) -> Result<()> {
        let t = self.terminator_mut();
        if !matches!(t, Terminator::Done) {
            anyhow::bail!("Expected terminator to be Done, not: {:?}", t);
        }
        *t = terminator.into();
        Ok(())
    }
    fn terminator_mut(&mut self) -> &mut Terminator {
        &mut self.bh.body.blocks[self.scasm_block_id].terminator.elem
    }

    #[track_caller]
    fn allocate_block(&mut self, wasm_block_id: InstrSeqId) -> anyhow::Result<&mut BlockInfo> {
        let scale_block_id = self.body.blocks.len();
        self.to_process.push((scale_block_id, wasm_block_id));
        self.body.blocks.push(Default::default());
        // Blocks can require args and leave results on the stack
        let has_return = match self.func.block(wasm_block_id).ty {
            InstrSeqType::Simple(None) => false,
            InstrSeqType::MultiValue(_) => anyhow::bail!("multi-value extension to wasm"),
            InstrSeqType::Simple(Some(_)) => true,
        };
        Ok(match self.seen.entry(wasm_block_id) {
            Entry::Occupied(_) => unreachable!(),
            Entry::Vacant(vac) => vac.insert(BlockInfo {
                scale_block_id,
                jump_target_block: scale_block_id,
                has_return,
            }),
        })
    }

    #[track_caller]
    fn get_block(&mut self, wasm_block_id: InstrSeqId) -> &mut BlockInfo {
        self.seen.get_mut(&wasm_block_id).unwrap()
    }

    fn ltz(&mut self, a: impl Into<Operand>) -> Register {
        let destination = self.stack.temp(RegisterKind::Regint);
        let instr = Instruction::General {
            instruction: "ltzint",
            values: vec![a.into().into()],
            destinations: vec![destination.into()],
        };
        self.push_stmt(Statement::from_instr_with_comment(instr, Span::DUMMY));
        destination
    }

    fn assign(&mut self, dest: Register, value: Operand) {
        let instr = Instruction::Assign {
            destination: Span::DUMMY.with(dest),
            value: Span::DUMMY.with(value),
        };
        self.push_stmt(instr);
    }

    fn unop(&mut self, a: Operand, instruction: &'static str) -> Spanned<Register> {
        let destination = Span::DUMMY.with(self.stack.temp(RegisterKind::Regint));
        let instr = Instruction::General {
            instruction,
            destinations: vec![destination],
            values: vec![Span::DUMMY.with(a)],
        };
        self.push_stmt(instr);
        destination
    }

    fn binop(
        &mut self,
        a: impl Into<Operand>,
        b: impl Into<Operand>,
        kind: RegisterKind,
        instruction: &'static str,
        comment: Span,
    ) -> Register {
        let a = a.into().into();
        let b = b.into().into();
        let destination = self.stack.temp(kind);
        let instr = Instruction::General {
            instruction,
            destinations: vec![destination.into()],
            values: vec![a, b],
        };
        self.push_stmt(Statement::from_instr_with_comment(instr, comment));
        destination
    }

    fn wrap_if_32_bit(&mut self, a: Register, op: &BinaryOp) -> Register {
        match op {
            BinaryOp::I32Add
            | BinaryOp::I32Sub
            | BinaryOp::I32Mul
            | BinaryOp::I32DivS
            | BinaryOp::I32DivU
            | BinaryOp::I32RemS
            | BinaryOp::I32RemU
            | BinaryOp::I32And
            | BinaryOp::I32Or
            | BinaryOp::I32Xor
            | BinaryOp::I32Shl
            | BinaryOp::I32ShrS
            | BinaryOp::I32ShrU
            | BinaryOp::I32Rotl
            | BinaryOp::I32Rotr => {}
            // Skip 64 bit ops, they don't need adjusting.
            BinaryOp::I64Add
            | BinaryOp::I64Sub
            | BinaryOp::I64Mul
            | BinaryOp::I64DivS
            | BinaryOp::I64DivU
            | BinaryOp::I64RemS
            | BinaryOp::I64RemU
            | BinaryOp::I64And
            | BinaryOp::I64Or
            | BinaryOp::I64Xor
            | BinaryOp::I64Shl
            | BinaryOp::I64ShrS
            | BinaryOp::I64ShrU
            | BinaryOp::I64Rotl
            | BinaryOp::I64Rotr => return a,
            _ => unreachable!("wrap_if_i32 should not be used on {:?}", op),
        }

        let b = self.val_to_reg(Operand::Value(Const::Int(32)));
        // a << 32
        let c = self.binop(a, b, RegisterKind::Regint, "shlint", Span::DUMMY);
        // (a << 32) >> 32
        self.binop(c, b, RegisterKind::Regint, "shrint", Span::DUMMY)
    }

    fn r(&mut self, kind: RegisterKind) -> Register {
        self.stack.temp(kind)
    }

    /// Lift a value to a register if it's not already a register.
    /// Some assembly instructions can only take register arguments,
    /// so we create a new register and immediately add an assignment
    /// to that register assigning the constant.
    fn val_to_reg(&mut self, val: Operand) -> Register {
        if let Operand::Register(r) = val {
            return r;
        }
        let reg = self.stack.temp(RegisterKind::Regint);
        self.assign(reg, val);
        reg
    }

    /// destination = 1 - tmp
    /// Super hacky because there's no neg for `RegisterKind::Regint`, so
    /// we subtract the bit from `1`, which, since `tmp` can only be `1` or
    /// `0`, boils down to inverting the bit
    fn invert_bit(&mut self, val: impl Into<Operand>) -> anyhow::Result<Register> {
        let comment = self
            .bh
            .comment(|f| write!(f, "negation hack, sub from 1 is the same as negation"))?;
        let one = self.val_to_reg(Operand::Value(Const::Int(1)));
        Ok(self.binop(one, val, RegisterKind::Regint, "subint", comment))
    }

    fn read_from_persistent(
        &mut self,
        kind: PersistentKind,
        register_kind: RegisterKind,
        comment: impl std::fmt::Display,
    ) -> anyhow::Result<()> {
        let reg = self.stack.local(kind, register_kind);
        let dest = self.stack.push_temp(register_kind);
        let instr = Instruction::Assign {
            destination: Span::DUMMY.with(dest),
            value: Span::DUMMY.with(reg.into()),
        };
        let comment = self.comment(|f| write!(f, "{}", comment))?;
        self.push_stmt(Statement::from_instr_with_comment(instr, comment));
        Ok(())
    }

    fn assign_to_persistent(
        &mut self,
        kind: PersistentKind,
        val: Operand,
        comment: impl std::fmt::Display,
    ) -> anyhow::Result<Register> {
        let register_kind = match val {
            Operand::Value(_) => RegisterKind::Regint,
            Operand::Register(reg) => reg.kind(),
        };
        let reg = self.stack.local(kind, register_kind);
        let instr = Instruction::Assign {
            destination: Span::DUMMY.with(reg),
            value: Span::DUMMY.with(val),
        };
        let comment = self.comment(|f| write!(f, "{}", comment))?;
        self.push_stmt(Statement::from_instr_with_comment(instr, comment));
        Ok(reg)
    }

    fn br_if(
        &mut self,
        cond: Register,
        wasm_block_id: InstrSeqId,
        cmp_constant: i32,
        jump_if_equal: bool,
        comment: String,
    ) -> anyhow::Result<()> {
        let cond = Span::DUMMY.with(cond).require(self.cx);
        let fallthrough_block = self.allocate_new_block();
        let terminator = std::mem::replace(self.terminator_mut(), Terminator::Done);
        self.body.blocks[fallthrough_block].terminator.elem = terminator;
        let target_block = self.get_block(wasm_block_id);
        let has_return = target_block.has_return;
        let target_block = target_block.jump_target_block;

        self.create_block_return_value_assignment(
            wasm_block_id,
            has_return,
            |this| -> anyhow::Result<Operand> { this.stack.read_top() },
        )?;

        let comment = self.comment(|f| f.write_str(&comment))?;

        *self.terminator_mut() = Terminator::Jump(Jump {
            target_block,
            comment,
            mode: JumpMode::Conditional(JumpCondition {
                fallthrough_block,
                jump_if_equal,
                constant: Span::DUMMY.with(cmp_constant),
                register: cond,
            }),
        });
        // Continue adding instructions to the fallthrough block
        self.scasm_block_id = fallthrough_block;
        Ok(())
    }

    fn create_block_return_value_assignment(
        &mut self,
        wasm_block_id: InstrSeqId,
        has_return: bool,
        get_stack_element: impl FnOnce(&mut Self) -> anyhow::Result<Operand>,
    ) -> anyhow::Result<()> {
        if has_return {
            let val = get_stack_element(self)?;
            self.assign_to_persistent(
                PersistentKind::BlockReturn(wasm_block_id),
                val,
                format_args!("block return value"),
            )?;
        }
        Ok(())
    }

    fn br(&mut self, wasm_block_id: InstrSeqId) -> anyhow::Result<()> {
        let target_block = self.get_block(wasm_block_id);
        let has_return = target_block.has_return;
        let target_block = target_block.jump_target_block;

        self.create_block_return_value_assignment(wasm_block_id, has_return, |this| {
            this.stack.pop()
        })?;

        *self.terminator_mut() = Terminator::Jump(Jump {
            target_block,
            comment: Span::DUMMY,
            mode: JumpMode::Goto,
        });
        Ok(())
    }
}

pub fn transpile_wasm_function_body_to_scale<'a, 'wasm>(
    cx: &'a Compiler,
    module: &'wasm walrus::Module,
    func: &'wasm walrus::LocalFunction,
    exports: HashMap<String, &'wasm ExportItem>,
) -> anyhow::Result<Context<'a, 'wasm>> {
    let mut context = Context::new(cx, exports, module)?;
    context.add(func);

    let mut first_func = true;
    while let Some(mut blocks) = context.process_next() {
        if first_func != blocks.is_main_function {
            anyhow::bail!("The first function is not the main function")
        }
        // scasm_block_id is mutable, because things like `BrIf` introduce a new scasm block
        // without doing the same in wasm.
        let mut first_block = true;
        while let Some(mut cbh) = blocks.process_next() {
            if first_block && first_func {
                for global in module.globals.iter() {
                    match global.kind {
                        walrus::GlobalKind::Import(_) => {
                            anyhow::bail!("the global Import is not supported")
                        }
                        walrus::GlobalKind::Local(init_expr) => match init_expr {
                            walrus::InitExpr::Value(value) => {
                                let operand = cbh.wasm_val_to_op(value)?;
                                cbh.global_set(operand, global.id())?;
                            }
                            other => {
                                anyhow::bail!("do not support this kind of global '{:?}'", other)
                            }
                        },
                    }
                }
            }

            let span = debug_span!("block", cbh.scasm_block_id, ?cbh.block.ty);
            let _scope = span.enter();
            for (instr, _) in cbh.block.instrs.iter() {
                cbh.process_instr(instr)?;
                cbh.stack.dump_stack();
            }
            let ret_types = match cbh.block.ty {
                InstrSeqType::MultiValue(id) => module.types.get(id).results().to_owned(),
                InstrSeqType::Simple(None) => {
                    cbh.stack.expect_stack_empty()?;
                    vec![]
                }
                // FIXME: use the value in the Some to do something more concrete than `pop_unknown`.
                InstrSeqType::Simple(Some(ty)) => vec![ty],
            };
            if !ret_types.is_empty() {
                match cbh
                    .block
                    .instrs
                    .last()
                    .context("wasm doesn't produce empty blocks")?
                    .0
                {
                    // If the last instruction diverged there will be nothing left on the stack.
                    walrus::ir::Instr::Br(..) | walrus::ir::Instr::Unreachable(..) => {
                        cbh.stack.expect_stack_empty()?;
                    }
                    _ => {
                        for ret_type in ret_types {
                            trace!(?ret_type, "popping function return value");
                            // FIXME: is the iteration order correct here for the popping?
                            let val = cbh.stack.pop()?;
                            let persistent = if first_block {
                                PersistentKind::FunctionReturn(cbh.block.id(), 0)
                            } else {
                                PersistentKind::BlockReturn(cbh.block.id())
                            };
                            cbh.assign_to_persistent(
                                persistent,
                                val,
                                "assign trailing block return value",
                            )?;
                        }
                    }
                }
            }
            if !first_func && first_block {
                *cbh.terminator_mut() = Terminator::Return {
                    comment: Span::DUMMY,
                };
            }
            first_block = false;
        }
        first_func = false;
    }

    if let Some(reqbl) = context.reqbl {
        context.body.blocks[0].stmts.push(
            Instruction::General {
                destinations: vec![],
                instruction: "reqbl",
                values: vec![Span::DUMMY.with(Operand::Value(Const::Int(reqbl)))],
            }
            .into(),
        );
    }

    // Execute clear_register instruction [hack to avoid problems
    // due to weird DEBUG issue XXXX]
    context.body.blocks[0].stmts.push(
        Instruction::General {
            instruction: "clear_registers",
            destinations: vec![],
            values: vec![],
        }
        .into(),
    );

    // Allocate test memory for clear modp
    let input_reg = context.stack.temp(RegisterKind::Regint);
    let tmp_reg = context.stack.temp(RegisterKind::Regint);
    context.body.blocks[0].stmts.push(
        Instruction::Assign {
            destination: input_reg.into(),
            value: Span::DUMMY.with(Operand::Value(Const::Int(1000))),
        }
        .into(),
    );
    context.body.blocks[0].stmts.push(
        Instruction::General {
            destinations: vec![tmp_reg.into()],
            instruction: "newc",
            values: vec![Span::DUMMY.with(Operand::Register(input_reg))],
        }
        .into(),
    );

    // Allocate test memory for clear int
    let input_reg = context.stack.temp(RegisterKind::Regint);
    let tmp_reg = context.stack.temp(RegisterKind::Regint);
    context.body.blocks[0].stmts.push(
        Instruction::Assign {
            destination: input_reg.into(),
            value: Span::DUMMY.with(Operand::Value(Const::Int(1000))),
        }
        .into(),
    );
    context.body.blocks[0].stmts.push(
        Instruction::General {
            destinations: vec![tmp_reg.into()],
            instruction: "newint",
            values: vec![Span::DUMMY.with(Operand::Register(input_reg))],
        }
        .into(),
    );

    Ok(context)
}

/// Debug implementation
impl<'a, 'bh, 'cx, 'wasm> CurrentBlockHandler<'a, 'bh, 'cx, 'wasm> {
    /// Generate print instruction to debug and print a line
    #[allow(dead_code)]
    fn debug_print(&mut self, line: &str) {
        line.chars().for_each(|ch| {
            let comment = self
                .comment(|f| write!(f, "print char {}", ch))
                .expect("cannot format comment");
            let print_line = Instruction::General {
                instruction: "print_char",
                destinations: vec![],
                values: vec![Span::DUMMY.with(Operand::Value(Const::Int(ch as i32)))],
            };
            self.push_stmt(Statement::from_instr_with_comment(print_line, comment));
        })
    }

    /// Generate print instruction to debug and print a register
    #[allow(dead_code)]
    fn debug_print_reg(&mut self, reg: Register) {
        let print_reg = Instruction::General {
            instruction: match reg.kind() {
                RegisterKind::Regint => "print_int",
                RegisterKind::Clear => "print_reg",
                _ => "print_reg",
            },
            destinations: vec![],
            values: vec![Span::DUMMY.with(Operand::Register(reg))],
        };

        self.push_stmt(print_reg);
    }
}

#[derive(StructOpt)]
pub struct Args {
    /// The path to the file to process.
    /// The input format is figured out from the file extension and can be either
    /// `.asm` or `.bc`
    /// If this is missing, the input is read from `stdin` and optimized assembly is output to `stdout`
    #[structopt(parse(from_os_str))]
    pub input: Option<PathBuf>,
    /// The path to write the output to.
    /// The output format is figured out from the extension and can be one of
    /// * `.asm` (scasm assembly)
    /// * `.bc` (bytecode)
    /// * `.dot` (graphviz dot file showing the control flow graph of the input)
    ///
    /// If this is missing, the output is written to `stdout` in scale assembly format
    #[structopt(parse(from_os_str))]
    pub output: Option<PathBuf>,

    /// Optimization level, level 0 means no optimizations.
    #[structopt(short = "O", long, default_value = "4")]
    pub optimization_level: u8,
}

pub fn main(args: Args) -> anyhow::Result<()> {
    // Load wasm input
    let buf = if let Some(file) = args.input {
        std::fs::read(file)?
    } else {
        let stdin = std::io::stdin();
        let mut stdin = stdin.lock();
        let mut buf = Vec::new();
        stdin.read_to_end(&mut buf)?;
        buf
    };

    // Parse wasm input
    let config = walrus::ModuleConfig::new();
    let module = config.parse(&buf)?;
    let exports: HashMap<String, &ExportItem> = module
        .exports
        .iter()
        .map(|export| (export.name.to_owned(), &export.item))
        .collect();

    let func = if let ExportItem::Function(func) = exports["main"] {
        *func
    } else {
        anyhow::bail!("invalid item for main: {:?}", exports["main"]);
    };
    let func = module.funcs.get(func);
    let func = match &func.kind {
        FunctionKind::Import(_) => anyhow::bail!("main function is a function import"),
        FunctionKind::Local(func) => func,
        FunctionKind::Uninitialized(_) => unreachable!(),
    };
    let mut cx = Compiler::stderr();
    cx.optimization_level = args.optimization_level;
    cx.verbose = true;
    cx.show_warnings = false;
    let Context { mut body, .. } =
        transpile_wasm_function_body_to_scale(&cx, &module, func, exports)?;

    let opts = vec![
        "validate".to_string(),
        "nop_removal".to_string(),
        "print_merge".to_string(),
        "start_stop_open".to_string(),
        "merge_instructions".to_string(),
        "nop_removal".to_string(),
        "destination_propagation".to_string(),
        "assignment_chain".to_string(),
        "cmp".to_string(),
        "dce".to_string(),
        "nop_removal".to_string(),
        "goto_chain_collapse".to_string(),
        "dead_block_removal".to_string(),
        "move_done_block_to_end".to_string(),
        "register_coloring".to_string(),
        "cond_flip".to_string(),
        "nop_removal".to_string(),
        "validate".to_string(),
    ];
    scasm::transforms::run_optimizations(&cx, &mut body, None, &opts);
    // scasm::transforms::apply_default_optimization_pipeline(&cx, &mut body, None);
    cx.check_for_errors().unwrap();

    let path = args.output.unwrap();

    if path.extension().unwrap() == "dot" {
        body.print_dot_file(&cx, File::create(&path)?)?;
    } else if path.extension().unwrap() == "asm" {
        let mut out_file = File::create(&path)?;
        for lex in &body.relex(&cx) {
            writeln!(out_file, "{}", lex.display(&cx))?;
        }
    } else if path.extension().unwrap() == "bc" {
        // FIXME: implement thread handling and thus correct .sch generation
        let file_name = format!("{}-0", path.file_stem().unwrap().to_str().unwrap());
        let bc_file = path.with_file_name(file_name).with_extension("bc");
        scasm::binary::generate_bytecode(&cx, &body.relex(&cx), File::create(bc_file)?)?;
        let schedfile = path.with_extension("sch");
        let mut schedfile = File::create(schedfile)?;
        writeln!(schedfile, "1")?;
        writeln!(schedfile, "1")?;
        writeln!(
            schedfile,
            "{}-0",
            path.file_stem().unwrap().to_str().unwrap()
        )?;
        writeln!(schedfile, "1 0")?;
        writeln!(schedfile, "0")?;
        for arg in std::env::args() {
            write!(schedfile, "{} ", arg)?;
        }
        writeln!(schedfile)?;
    } else {
        anyhow::bail!(
            "unknown destination format: {}",
            path.extension().unwrap().to_str().unwrap()
        );
    }
    Ok(())
}
