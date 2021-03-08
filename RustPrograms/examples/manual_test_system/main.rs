
// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

//! This program is NOT meant to be run in the test environment
//! You should just compile it with -A and then check the assembler
//! is correct
#![no_std]
#![no_main]
#![feature(const_evaluatable_checked)]

#[scale::main(KAPPA = 40)]
#[inline(always)]
fn main() {
    start_clock(ConstU32::<3>);
    stop_clock(ConstU32::<4>);
    require_bit_length(ConstU32::<10>);
    require_bit_length(ConstU32::<20>);
    crash();
    restart();
    clear_memory();
    clear_registers();

    let a = ClearModp::from(4);
    let c = a.digest();
    let m = black_box(10);
    let d = m.rand();
}
