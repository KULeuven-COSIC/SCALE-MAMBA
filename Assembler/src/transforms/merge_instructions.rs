// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use crate::asm::Body;
use crate::asm::Instruction;
use crate::Compiler;

#[derive(Default)]
pub struct Pass;

// This is the transform which merges instructions into bigger multi-instructions
// where possible
impl super::Pass for Pass {
    fn name(&self) -> &'static str {
        "merge_instructions"
    }

    #[allow(clippy::cognitive_complexity)]
    #[allow(clippy::nonminimal_bool)]
    #[allow(clippy::needless_range_loop)]
    fn apply<'a>(&mut self, cx: &'a Compiler, body: &mut Body<'a>) {
        if cx.optimization_level < 2 {
            return;
        }
        for (block_id, block) in body.blocks.iter_mut().enumerate() {
            // don't panic on subnormal assembly
            if block.stmts.is_empty() {
                continue;
            }
            debug!("\n# Block {}", block_id);
            // We go through the merge the operations which we can
            debug!("\tMerging instructions");
            for i in 0..block.stmts.len() {
                if let Some((i_stmt, trailing)) = block.stmts[i..].split_first_mut() {
                    // Merge DataInstructions
                    if let Instruction::DataInstr {
                        instruction: i_instr,
                        registers: i_regs,
                    } = &mut i_stmt.instr
                    {
                        // Go through rest of instructions in this block looking for a match
                        for j in 0..trailing.len() {
                            if let Instruction::DataInstr {
                                instruction: j_instr,
                                registers: j_regs,
                            } = &trailing[j].instr
                            {
                                // Need them to be have the same name and also the same
                                // vectorization value [for when compiling Mamba programs]
                                if i_instr == j_instr
                                    && trailing[j].vectorized.elem.get()
                                        == i_stmt.vectorized.elem.get()
                                {
                                    // Need to merge instructions i with j
                                    i_regs.extend(j_regs);
                                    trailing[j].instr = Instruction::Nop;
                                }
                            }
                        }
                    }
                }
            }
            debug!("\t\tFinished optimizing block");
        }
    }
}
