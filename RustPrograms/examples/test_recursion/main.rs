#![no_std]
#![no_main]
#![feature(const_evaluatable_checked)]

use scale::black_box;

scale::main! {
    I64_MEMORY = 0;
    SECRET_I64_MEMORY = 0;
    SECRET_MODP_MEMORY = 0;
    CLEAR_MODP_MEMORY = 0;
    KAPPA = 40;
}

#[inline(never)]
fn fib(x: u64) -> u64 {
    print!(x as i64, "\n");
    match x {
        0 => 0,
        1 | 2 => 1,
        _ => fib(x - 1) + fib(x - 2)
    }
}

#[inline(always)]
fn check(n: i64) -> i64 {
    fib(black_box(n) as u64) as i64
}

#[inline(never)]
fn main() {
    check(0).test();
    check(1).test();
    check(2).test();
    check(3).test();
    check(4).test();
}
