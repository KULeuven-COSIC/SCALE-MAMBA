//! A dummy optimization not doing anything

use crate::asm::Body;
use crate::Compiler;

#[derive(Default)]
pub struct Pass;

impl super::Pass for Pass {
    fn apply<'a>(&mut self, _cx: &'a Compiler, _body: &mut Body<'a>) {
        // do nothing
    }
    fn name(&self) -> &'static str {
        "dummy"
    }
}
