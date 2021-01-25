//! Flip the branches on a conditional branch if the target block is the next block.
//! This generates nicer assemby. While we could also adjust the assembly generator to
//! handle this, it's much simpler to do here.

use crate::asm::{Body, JumpMode, Terminator};
use crate::Compiler;

#[derive(Default)]
pub struct Pass;

impl super::Pass for Pass {
    fn apply<'a>(&mut self, cx: &'a Compiler, body: &mut Body<'a>) {
        if cx.optimization_level < 4 {
            return;
        }
        for (i, block) in body.blocks.iter_mut().enumerate() {
            if let Terminator::Jump(jmp) = &mut block.terminator.elem {
                if let JumpMode::Conditional(cnd) = &mut jmp.mode {
                    if jmp.target_block == i + 1 {
                        std::mem::swap(&mut jmp.target_block, &mut cnd.fallthrough_block);
                        cnd.jump_if_equal = !cnd.jump_if_equal;
                    }
                }
            }
        }
    }
    fn name(&self) -> &'static str {
        "cond_flip"
    }
}
