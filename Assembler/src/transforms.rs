//! Optimizations and other transformations of the basic block datastructures

use crate::asm::Body;
use crate::lexer::Register;
use crate::Compiler;
use std::io::Write;
use std::num::NonZeroU32;
use std::path::{Path, PathBuf};

mod nop;
mod nop_removal;
mod optimizer_step1;
mod print;
mod register_reuse;

/// The interface of a single transformation
pub trait Pass {
    fn apply<'a>(&mut self, cx: &'a Compiler, body: &mut Body<'a>);
    fn name(&self) -> &'static str;
}

fn mk_pass<T: Pass + Default + 'static>() -> Box<dyn Pass> {
    Box::new(T::default())
}

const PASSES: &[fn() -> Box<dyn Pass>] = &[
    mk_pass::<nop::Pass>,
    mk_pass::<print::Pass>,
    mk_pass::<optimizer_step1::Pass>,
    mk_pass::<nop_removal::Pass>,
    mk_pass::<register_reuse::Pass>,
];

pub fn apply_default_optimization_pipeline<'a>(
    cx: &'a Compiler,
    body: &mut Body<'a>,
    dump: Option<PathBuf>,
) {
    for pass in PASSES {
        let mut pass = pass();
        info!("opt pass: {}", pass.name());
        pass.apply(cx, body);
        if let Some(dump) = &dump {
            dump_pass(cx, body, dump, pass.name());
        }
    }
}

fn dump_pass<'a>(cx: &'a Compiler, body: &Body<'a>, dump: &Path, pass_name: &str) {
    std::fs::create_dir_all(dump).unwrap();
    let relexed = body.relex(&cx);
    // FIXME: get rid of these unwraps
    let mut file = std::fs::File::create(dump.join(pass_name)).unwrap();
    for lex in relexed {
        writeln!(file, "{}", lex).unwrap();
    }
}

pub fn list_optimizations() {
    for pass in PASSES {
        println!("{}", pass().name());
    }
}

pub fn run_optimizations<'a>(
    cx: &'a Compiler,
    body: &mut Body<'a>,
    dump: Option<PathBuf>,
    optimizations: &[String],
) {
    let passes: std::collections::HashMap<_, _> = PASSES
        .iter()
        .map(|p| {
            let pass = p();
            (pass.name(), p)
        })
        .collect();
    for pass in optimizations {
        let mut pass = (passes.get(pass.as_str()).expect("unknown pass"))();
        info!("opt pass: {}", pass.name());
        pass.apply(cx, body);
        if let Some(dump) = &dump {
            dump_pass(cx, body, dump, pass.name());
        }
    }
}

/// Applies the given function to the register and all its contiguous vectorized registers
pub fn vectorize(
    v: NonZeroU32,
    reg: Register,
) -> impl Iterator<Item = Register> + Clone + ExactSizeIterator {
    (0..v.get()).map(move |i| reg.map(|j| i + j))
}
