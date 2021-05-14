// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

//! Replace
//!
//! ```asm
//! addint r1, r2, r3
//! mov r4, r1
//! ```
//!
//! with
//!
//! ```asm
//! addint r4, r2, r3
//! ```

use crate::span::*;
use crate::{
    asm::Body,
    lexer::{Operand, Register},
};
use crate::{asm::Instruction, Compiler};

#[derive(Default)]
pub struct Pass;

// Returns
//    false: if there is more than one instruction which reads this source register
//           and more than one instruction which writes this destination register
//    true: if only one which reads
fn is_removable<'a>(
    cx: &'a Compiler,
    body: &Body<'a>,
    source: Register,
    destination: Register,
) -> bool {
    let mut cnts = 0;
    let mut cntd = 0;
    for block in &body.blocks {
        for stmt in &block.stmts {
            for x in stmt.read_registers_base(cx) {
                if x == source {
                    if cnts == 0 {
                        cnts = 1
                    } else {
                        return false;
                    }
                }
            }
            for x in stmt.write_registers_base(cx) {
                if x == destination {
                    if cntd == 0 {
                        cntd = 1
                    } else {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

impl super::Pass for Pass {
    fn apply<'a>(&mut self, cx: &'a Compiler, body: &mut Body<'a>) {
        let mut count = 0;
        let mut total = 0;
        // First count the total number of instructions
        for block in &body.blocks {
            for _stmt in &block.stmts {
                total = total + 1;
            }
        }
        // If total too big then ignore this optimization
        //   - This is a pity, but is due to us not being able to just search within a block
        //     for a read once, as a register could be read later than in a block
        if total > 20000 {
            println!("\tIgnoring destination propagation optimization due to there being a total of  {} instructions",total);
            return;
        }

        // First remove unnessary conversions by converting them into mov instructions
        // Do this first, before we remove mov instructions
        for b in 0..body.blocks.len() {
            for i in 0..body.blocks[b].stmts.len() {
                let forward_instr = &body.blocks[b].stmts[i].instr;
                match forward_instr {
                    Instruction::General {
                        instruction,
                        destinations,
                        values,
                    } => {
                        let back_instr = match *instruction {
                            "convsintsreg" => "convsregsint", // s->sr
                            "convsregsint" => "convsintsreg", // sr->s
                            "convsintsbit" => "convsbitsint", // s->sb
                            "convsbitsint" => "convsintsbit", // sb->s
                            _ => "none",
                        };
                        if back_instr != "none" {
                            let destination = destinations[0].elem;
                            let mut destination2 = destinations[0].elem; // This will be the new destination reg
                            let source = values[0].elem;
                            // The conversion should have a desination register that is read only once
                            // and that should be by the back_instr
                            let mut cnt = 0;
                            let mut bj = 0;
                            let mut ij = 0;
                            let mut ok = true;
                            for b1 in 0..body.blocks.len() {
                                if !ok {
                                    break;
                                }
                                for i1 in 0..body.blocks[b1].stmts.len() {
                                    if !ok {
                                        break;
                                    }
                                    for x in body.blocks[b1].stmts[i1].read_registers_base(cx) {
                                        if x == destination {
                                            if cnt == 0 {
                                                cnt = 1;
                                                bj = b1;
                                                ij = i1;
                                                match body.blocks[b1].stmts[i1].instr {
                                                    Instruction::General {
                                                        instruction, ..
                                                    } => {
                                                        if back_instr != instruction {
                                                            ok = false;
                                                        } else {
                                                            destination2 = *body.blocks[b1].stmts
                                                                [i1]
                                                                .write_registers_base(cx)
                                                                .first()
                                                                .unwrap();
                                                        }
                                                    }
                                                    _ => {
                                                        ok = false;
                                                    }
                                                }
                                            } else {
                                                ok = false;
                                            }
                                        }
                                    }
                                }
                            }
                            if ok {
                                // Convert first function to a MOV and second to a NOP
                                let dest: Spanned<Register> = Spanned::from(destination2);
                                let src: Spanned<Operand> = Spanned::from(source);
                                body.blocks[b].stmts[i].instr = Instruction::Assign {
                                    destination: dest,
                                    value: src,
                                };
                                body.blocks[bj].stmts[ij].instr = Instruction::Nop;
                                count = count + 1;
                            }
                        }
                    }
                    _ => {}
                };
            }
        }

        for b in 0..body.blocks.len() {
            for i in 0..body.blocks[b].stmts.len() {
                if let Instruction::Assign {
                    destination, value, ..
                } = body.blocks[b].stmts[i].instr
                {
                    if let Operand::Register(value) = value.elem {
                        // These instructions should be mov* instructions
                        // Find out if more than one instruction reads this source register
                        // and if more than one writes this destingation register
                        //   - Should not need to do the latter as we should be in SSA form
                        //     But for some reason we are not
                        let flag = is_removable(cx, body, value, destination.elem);
                        // If only one instruction reads this register then we can remove it
                        if flag {
                            // Search through and replace the write to value with destination
                            // This should never be more than 1, again due to SSA form
                            // But sometimes it is (which is why we cannot early terminate this loop)
                            // This is because we have some instructions which write to source,
                            // whose value is never used (as we know source is only read once,
                            // [by this mov]
                            let mut cnt = 0;
                            for block1 in &mut body.blocks {
                                for stmt in &mut block1.stmts {
                                    let mut flag = false;
                                    for x in stmt.write_registers_base(cx) {
                                        if x == value {
                                            flag = true;
                                        }
                                    }
                                    if flag {
                                        cnt = cnt + 1;
                                        stmt.replace_registers(cx, |r| {
                                            if r == value {
                                                destination.elem
                                            } else {
                                                r
                                            }
                                        });
                                    }
                                }
                            }
                            body.blocks[b].stmts[i].instr = Instruction::Nop;
                            count = count + 1;
                        }
                    }
                }
            }
        }
        println!(
            "\tDestination propagation deleted {} instructions out of {}",
            count, total
        );
    }
    fn name(&self) -> &'static str {
        "destination_propagation"
    }
}
