// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![no_std]
#![no_main]
#![feature(const_evaluatable_checked)]

use scale::black_box;

#[inline(never)]
fn fib(x: u64) -> u64 {
    print!(x as i64, "\n");
    match x {
        0 => 0,
        1 | 2 => 1,
        _ => fib(x - 1) + fib(x - 2),
    }
}

#[inline(always)]
fn check(n: i64) -> i64 {
    fib(black_box(n) as u64) as i64
}

#[inline(never)]
#[scale::main(KAPPA = 40)]
fn main() {
    check(0).test();
    check(1).test();
    check(2).test();
    check(3).test();
    check(4).test();
}
