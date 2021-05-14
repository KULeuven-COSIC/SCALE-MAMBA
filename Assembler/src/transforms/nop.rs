// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

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
