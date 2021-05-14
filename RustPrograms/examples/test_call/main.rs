// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![no_std]
#![no_main]
#![feature(const_evaluatable_checked)]

#[scale::main(KAPPA = 40)]
#[inline(never)]
fn bar(x: i64) -> i64 {
    print!(
        "asfdasdfasfasdfasdfasdfasdfasdfasdfasdfasdfadsfasdfasdfasdfasdfasdfadsfasdfasdf",
        x, "\n"
    );
    x * 2
}

#[inline(never)]
fn main() {
    print!("foo");
    let y = bar(42);
    bar(43 + y).test();
}
