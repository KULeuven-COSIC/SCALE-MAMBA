// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![no_std]
#![no_main]
#![feature(const_evaluatable_checked)]
#![allow(non_snake_case, incomplete_features)]

use scale_std::integer::*;

// This program is to test between the Mamba and Rust pipelines

#[scale::main(KAPPA = 40)]
#[inline(always)]
fn main() {
    let a: ClearInteger<6> = ClearInteger::from(23);
    let b: SecretInteger<6> = SecretInteger::from(a);

    // Answer should be 2 or 3
    let t1 = b.TruncPr(3, true);

    print!("\t ", t1.reveal(), "\n");

    // Answer should be 2
    let t2 = b.Trunc(3, true);

    print!("\t ", t2.reveal(), "\n");
}
