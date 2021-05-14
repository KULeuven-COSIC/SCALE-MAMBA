// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

//! Optimizations and other transformations of the basic block datastructures

use crate::asm::Body;
use crate::lexer::Register;
use crate::Compiler;
use std::io::Write;
use std::num::NonZeroU32;
use std::path::{Path, PathBuf};

mod assignment_chain;
mod cmp;
mod cond_flip;
mod dce;
mod dead_block_removal;
mod destination_propagation;
mod goto_chain_collapse;
mod merge_instructions;
mod move_done_block_to_end;
mod nop;
mod nop_removal;
mod optimizer_step1;
mod print;
mod print_merge;
mod register_reuse;
pub mod validate;

/// The interface of a single transformation
pub trait Pass {
    fn apply<'a>(&mut self, cx: &'a Compiler, body: &mut Body<'a>);
    fn name(&self) -> &'static str;
}

fn mk_pass<T: Pass + Default + 'static>() -> Box<dyn Pass> {
    Box::new(T::default())
}

const PASSES: &[fn() -> Box<dyn Pass>] = &[
    mk_pass::<validate::Pass>,
    mk_pass::<nop::Pass>,
    mk_pass::<print::Pass>,
    mk_pass::<print_merge::Pass>,
    mk_pass::<optimizer_step1::Pass>,
    mk_pass::<merge_instructions::Pass>,
    mk_pass::<nop_removal::Pass>,
    mk_pass::<destination_propagation::Pass>,
    mk_pass::<assignment_chain::Pass>,
    mk_pass::<cmp::Pass>,
    mk_pass::<dce::Pass>,
    mk_pass::<nop_removal::Pass>,
    mk_pass::<goto_chain_collapse::Pass>,
    mk_pass::<dead_block_removal::Pass>,
    mk_pass::<move_done_block_to_end::Pass>,
    mk_pass::<register_reuse::Pass>,
    mk_pass::<nop_removal::Pass>,
    mk_pass::<cond_flip::Pass>,
    mk_pass::<validate::Pass>,
];

pub fn apply_default_optimization_pipeline<'a>(
    cx: &'a Compiler,
    body: &mut Body<'a>,
    dump: Option<PathBuf>,
) {
    for pass in PASSES {
        let mut pass = pass();
        info!("opt pass: {}", pass.name());
        println!("apply_default_optimization_pipeline: {}", pass.name());
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
        writeln!(file, "{}", lex.display(cx)).unwrap();
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
        println!("run_optimizations: {}", pass.name());
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
