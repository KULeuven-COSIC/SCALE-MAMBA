// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use scasm::{
    asm::Instruction, asm::Statement, asm::Terminator, binary::instructions::name2instr,
    binary::instructions::ArgTy, binary::instructions::RegisterMode, lexer::Const, lexer::Lexical,
    lexer::Operand, lexer::RegisterKind, span::Span,
};
use tracing::{debug, instrument, trace};
use walrus::{ir::Value, ActiveDataLocation, DataKind, ExportItem, GlobalKind, InitExpr, Type};

use crate::{error::Error, CurrentBlockHandler};

const TEST_MEMORY_OFFSET: u32 = 1000;

impl<'a, 'bh, 'cx, 'wasm> CurrentBlockHandler<'a, 'bh, 'cx, 'wasm> {
    #[instrument(skip(self))]
    pub fn handle_extern_call(&mut self, name: &str, fn_ty: &Type) -> Result<(), Error> {
        match name {
            "init_wasm_heap_memory" => {
                assert_eq!(fn_ty.params().len(), 0);
                assert_eq!(fn_ty.results().len(), 0);

                let global_heap_base_id = self
                    .module
                    .exports
                    .iter()
                    .find_map(|export| {
                        if let ExportItem::Global(global_id) = export.item {
                            if export.name == "__heap_base" {
                                return Some(global_id);
                            }
                        }

                        None
                    })
                    .expect("cannot find the heap memory start address");
                let mut wasm_heap_start_address =
                    match self.module.globals.get(global_heap_base_id).kind {
                        GlobalKind::Import(_) => {
                            unimplemented!();
                        }
                        GlobalKind::Local(local) => match local {
                            InitExpr::Value(value) => self.wasm_val_to_op(value),
                            _ => unimplemented!(),
                        },
                    };
                if let Operand::Value(Const::Int(wasm_heap_address)) = &mut wasm_heap_start_address
                {
                    // heap_base is in bytes and regint can store 8 bytes
                    *wasm_heap_address /= 8;
                    self.offset_addr_val = Some(*wasm_heap_address as u32 + TEST_MEMORY_OFFSET);
                }

                let wasm_heap_start_address = self.val_to_reg(wasm_heap_start_address);
                let base_wasmp_heap_reg = self.stack.temp(RegisterKind::Regint);
                self.push_stmt(Instruction::General {
                    instruction: "newint",
                    destinations: vec![base_wasmp_heap_reg.into()],
                    values: vec![Span::DUMMY.with(wasm_heap_start_address.into())],
                });
                self.heap_dest_reg = Some(base_wasmp_heap_reg);

                // Write the initial memory state
                if let Some(memory) = self.module.memories.iter().next() {
                    assert!(
                        self.module.memories.iter().nth(1).is_none(),
                        "we don't support multiple memories yet"
                    );
                    let mut last_segment_end = None;
                    for &segment in &memory.data_segments {
                        let segment = self.module.data.get(segment);
                        let mut start = match &segment.kind {
                            DataKind::Active(act) => match act.location {
                                ActiveDataLocation::Absolute(start) => start,
                                ActiveDataLocation::Relative(_) => unimplemented!(),
                            },
                            DataKind::Passive => unimplemented!(),
                        };
                        // FIXME: look for data from previous segments that may overlap here instead of just panicking
                        if let Some(last_segment_end) = last_segment_end {
                            assert!(
                                start / 8 > last_segment_end / 8,
                                "{} > {}",
                                start,
                                last_segment_end
                            );
                        }
                        last_segment_end = Some(start + segment.value.len() as u32);
                        trace!(start);
                        let mut alignment = start as usize % 8;
                        if alignment == 0 {
                            alignment = 8;
                        } else {
                            start += 8;
                        }
                        let start = start / 8;
                        let start = start + self.offset_addr_val.unwrap();

                        trace!(start, alignment, "adjusted start");
                        trace!(n = segment.value.len());
                        let mut data = [0; 8];
                        // The first bytes may not exist, so we leave them as zero
                        for (dest, src) in data[alignment..].iter_mut().zip(&segment.value) {
                            *dest = *src;
                        }

                        let mut write_val = |idx: i32, data| {
                            trace!(?idx, ?data, "write val");
                            let data = u64::from_le_bytes(data);
                            if data == 0 {
                                return;
                            }
                            let data = self.wasm_val_to_op(Value::I64(data as i64));
                            let data = self.val_to_reg(data);
                            let addr = Operand::Value(Const::Int(idx + start as i32));
                            self.push_stmt(Instruction::General {
                                instruction: "stmint",
                                destinations: vec![],
                                values: vec![Span::DUMMY.with(data.into()), Span::DUMMY.with(addr)],
                            });
                        };

                        write_val(-1, data);

                        if let Some(rest) = segment.value.get((8 - alignment)..) {
                            // We iterate over up to 8 bytes at a time and write them to memory as a `u64`
                            for (idx, slice) in rest.chunks(8).enumerate() {
                                let mut data = [0; 8];
                                // The last bytes may not exist, so we leave them as zero.
                                data[..slice.len()].copy_from_slice(slice);
                                write_val(idx as i32, data)
                            }
                        }
                    }
                }

                Ok(())
            }
            // Special handling for some things
            "__black_box" => {
                assert_eq!(fn_ty.params().len(), 1);
                assert_eq!(fn_ty.results().len(), 1);
                let comment = self.comment(|f| write!(f, "function call: {}", name));
                let instr = Instruction::Assign {
                    value: Span::DUMMY.with(self.stack.pop()),
                    destination: Span::DUMMY.with(self.stack.push_temp(RegisterKind::Regint)),
                };
                self.push_stmt(Statement::from_instr_with_comment(instr, comment));
                Ok(())
            }
            "__reqbl" => {
                assert_eq!(fn_ty.params().len(), 1);
                let reqbl = self.stack.pop();
                match reqbl {
                    Operand::Register(_) => panic!("reqbl must take a constant argument"),
                    Operand::Value(Const::Bool(_)) => panic!("reqbl must take an integer argument"),
                    Operand::Value(Const::Int(i)) => match &mut self.reqbl {
                        reqbl @ None => *reqbl = Some(i),
                        Some(reqbl) => *reqbl = std::cmp::max(*reqbl, i),
                    },
                }
                Ok(())
            }
            "__bit" => {
                assert_eq!(fn_ty.params().len(), 0);
                assert_eq!(fn_ty.results().len(), 1);
                let comment = self.comment(|f| write!(f, "function call: {}", name));
                let instr = Instruction::DataInstr {
                    instruction: "bit",
                    registers: vec![Span::DUMMY
                        .with(self.stack.push_temp(RegisterKind::Secret))
                        .require(self.cx)],
                };
                self.push_stmt(Statement::from_instr_with_comment(instr, comment));
                Ok(())
            }
            "__square" => {
                assert_eq!(fn_ty.params().len(), 0);
                assert_eq!(fn_ty.results().len(), 0); // results are handled via the pop functions
                let comment = self.comment(|f| write!(f, "function call: {}", name));

                // Reverse order pushing so that the popping in Rust works in the right order.
                let b = self.stack.push_temp(RegisterKind::Secret);
                let a = self.stack.push_temp(RegisterKind::Secret);

                let instr = Instruction::DataInstr {
                    instruction: "square",
                    registers: vec![
                        Span::DUMMY.with(a).require(self.cx),
                        Span::DUMMY.with(b).require(self.cx),
                    ],
                };
                self.push_stmt(Statement::from_instr_with_comment(instr, comment));
                Ok(())
            }
            "__triple" => {
                assert_eq!(fn_ty.params().len(), 0);
                assert_eq!(fn_ty.results().len(), 0); // results are handled via the pop functions
                let comment = self.comment(|f| write!(f, "function call: {}", name));

                // Reverse order pushing so that the popping in Rust works in the right order.
                let c = self.stack.push_temp(RegisterKind::Secret);
                let b = self.stack.push_temp(RegisterKind::Secret);
                let a = self.stack.push_temp(RegisterKind::Secret);

                let instr = Instruction::DataInstr {
                    instruction: "triple",
                    registers: vec![
                        Span::DUMMY.with(a).require(self.cx),
                        Span::DUMMY.with(b).require(self.cx),
                        Span::DUMMY.with(c).require(self.cx),
                    ],
                };
                self.push_stmt(Statement::from_instr_with_comment(instr, comment));
                Ok(())
            }
            "reveal" => {
                assert_eq!(fn_ty.params().len(), 1);
                assert_eq!(fn_ty.results().len(), 1);
                let comment = self.comment(|f| write!(f, "function call: {}", name));
                let instr = Instruction::StartOpen {
                    registers: vec![Span::DUMMY.with(self.stack.pop()).require(self.cx)],
                };
                self.push_stmt(Statement::from_instr_with_comment(instr, comment));
                let instr = Instruction::StopOpen {
                    registers: vec![Span::DUMMY
                        .with(self.stack.push_temp(RegisterKind::Clear))
                        .require(self.cx)],
                };
                self.push_stmt(Statement::from_instr_with_comment(instr, comment));
                Ok(())
            }
            "__maybe_optimized_secret_modp_multiplication" => {
                // FIXME: implement squaring if the registers are the same.
                assert_eq!(fn_ty.params().len(), 2);
                assert_eq!(fn_ty.results().len(), 1);
                let comment = self.comment(|f| write!(f, "secret modp multiplication"));
                let a = self.stack.pop();
                let b = self.stack.pop();
                // triple s3, s4, s5 # 2
                let x = self.r(RegisterKind::Secret);
                let y = self.r(RegisterKind::Secret);
                let z = self.r(RegisterKind::Secret);
                let instr = Instruction::DataInstr {
                    instruction: "triple",
                    registers: [x, y, z]
                        .iter()
                        .copied()
                        .map(|reg| Span::DUMMY.with(reg).require(self.cx))
                        .collect(),
                };
                self.push_stmt(Statement::from_instr_with_comment(instr, comment));
                // subs s6, s0, s3
                let a = self.binop(a, x, RegisterKind::Secret, "subs", comment);
                // subs s7, s1, s4
                let b = self.binop(b, y, RegisterKind::Secret, "subs", comment);
                // startopen 2, s6, s7
                let instr = Instruction::StartOpen {
                    registers: [a, b]
                        .iter()
                        .copied()
                        .map(|reg| Span::DUMMY.with(reg).require(self.cx))
                        .collect(),
                };
                self.push_stmt(Statement::from_instr_with_comment(instr, comment));
                // stopopen 2, c0, c1
                let a = self.r(RegisterKind::Clear);
                let b = self.r(RegisterKind::Clear);
                let instr = Instruction::StopOpen {
                    registers: [a, b]
                        .iter()
                        .copied()
                        .map(|reg| Span::DUMMY.with(reg).require(self.cx))
                        .collect(),
                };
                self.push_stmt(Statement::from_instr_with_comment(instr, comment));
                let c = self.binop(y, a, RegisterKind::Secret, "mulm", comment);
                let d = self.binop(x, b, RegisterKind::Secret, "mulm", comment);
                let e = self.binop(a, b, RegisterKind::Clear, "mulc", comment);
                let sum = self.binop(z, c, RegisterKind::Secret, "adds", comment);
                let sum = self.binop(sum, d, RegisterKind::Secret, "adds", comment);
                let sum = self.binop(sum, e, RegisterKind::Secret, "addm", comment);
                self.stack.push(sum);
                Ok(())
            }
            "pop_secret_bit" | "pop_secret_i64" | "pop_secret_modp" => {
                assert_eq!(fn_ty.params().len(), 0);
                assert_eq!(fn_ty.results().len(), 1);
                // Value already on stack
                Ok(())
            }
            // Everything else is translated 1:1 into assembly or
            // actually implemented as a function call.
            _ => {
                let params = fn_ty.params().len();
                let results = fn_ty.results().len();

                let (adjusted_name, results) = if name.starts_with("push_multi_arg_") {
                    assert_eq!(results, 0);
                    (name.trim_start_matches("push_multi_arg_"), None)
                } else {
                    (name.trim_start_matches('_'), Some(results))
                };

                let instr = match name2instr(adjusted_name) {
                    Some(instr) => instr,
                    None => unimplemented!("unknown extern function: {}, {:?}", name, fn_ty),
                };
                if instr.terminator {
                    match adjusted_name {
                        "crash" => {
                            *self.terminator_mut() = Terminator::Crash {
                                comment: Span::DUMMY,
                            }
                        }
                        "restart" => {
                            // Clean the heap memory
                            self.push_stmt(Instruction::General {
                                instruction: "deleteint",
                                destinations: vec![],
                                values: vec![Span::DUMMY.with(self.heap_dest_reg.unwrap().into())],
                            });
                            // Clean the test memory
                            let zero = self.val_to_reg(Operand::Value(Const::Int(0)));
                            self.push_stmt(Instruction::General {
                                instruction: "deleteint",
                                destinations: vec![],
                                values: vec![Span::DUMMY.with(zero.into())],
                            });
                            self.push_stmt(Instruction::General {
                                instruction: "deletec",
                                destinations: vec![],
                                values: vec![Span::DUMMY.with(zero.into())],
                            });

                            *self.terminator_mut() = Terminator::Restart {
                                comment: Span::DUMMY,
                            }
                        }
                        _ => todo!(),
                    }
                    return Ok(());
                }
                let results = results.unwrap_or_else(|| instr.args.len() - params);
                trace!(?instr.args);
                assert_eq!(
                    params + results,
                    instr.args.len(),
                    "{} params and {} results, but instruction has {} args",
                    params,
                    results,
                    instr.args.len()
                );

                let params: Vec<_> = instr.args[results..]
                    .iter()
                    .rev()
                    .inspect(|&arg| trace!(?arg))
                    .map(|arg| match arg.ty {
                        ArgTy::Register(_, mode) => {
                            assert_eq!(mode, RegisterMode::Read);
                            let val = self.stack.pop();
                            // We need to lift constants to a register in case the
                            // assembly instruction requires a register arg.
                            self.val_to_reg(val).into()
                        }
                        ArgTy::Int { .. } => self.stack.pop(),
                        ArgTy::Player => self.stack.pop(),
                        ArgTy::Channel => self.stack.pop(),
                        _ => panic!("invalid input register: {:?}", arg),
                    })
                    .collect();
                let results: Vec<_> = instr.args[0..results]
                    .iter()
                    // Using reverse order so that the `rev` in the result printing
                    // gives the right order again. We need the reverse order in
                    // pushing the return values to the stack, so when popping the
                    // elements we get the first return value on the first pop.
                    .rev()
                    .map(|arg| match arg.ty {
                        ArgTy::Register(kind, mode) => {
                            assert_eq!(mode, RegisterMode::Write);
                            self.stack.push_temp(kind)
                        }
                        _ => panic!("invalid output register: {:?}", arg),
                    })
                    .collect();
                // Generate the asm
                let span = self.comment(|f| {
                    write!(f, "{}", adjusted_name)?;
                    let mut first = true;
                    for result in results.into_iter().rev() {
                        if first {
                            first = false;
                        } else {
                            write!(f, ",")?;
                        }
                        write!(f, " {}", result)?;
                    }
                    for param in params.iter().rev() {
                        if first {
                            first = false;
                        } else {
                            write!(f, ",")?;
                        }
                        write!(f, " {}", param)?;
                    }
                    write!(f, "# converted function call directly to asm")?;
                    Ok(())
                });
                let snip = span.snippet(self.cx);
                debug!(?snip);
                // Parse the asm back
                let lex = Lexical::parse(self.cx, span).unwrap();
                let stmt = Statement::parse(self.cx, &lex);
                assert!(scasm::transforms::validate::validate(self.cx, &stmt));
                self.push_stmt(stmt);
                Ok(())
            }
        }
    }
}
