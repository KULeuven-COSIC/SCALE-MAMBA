// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![no_std]
#![no_main]
#![feature(const_evaluatable_checked, const_generics)]
#![allow(non_snake_case, incomplete_features)]

use scale_std::bit_protocols::{KOpL, KOr};
use scale_std::fixed_point::*;
use scale_std::integer::*;
use scale_std::math::*;
use scale_std::slice::Slice;

#[scale::main(KAPPA = 40)]
#[inline(always)]
fn main() {
    let test_1 = SecretInteger::<64>::from(10);
    let test_2 = SecretInteger::<64>::from(4343234);

    // println!("value1 -- ", test_1.reveal());
    // println!("value2 -- ", test_2.reveal());
    // println!("reveal -- ", test_1.eq(test_2).reveal());
    let mut a: Slice<SecretModp> = Slice::uninitialized(64);
    let mut i = 0;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(1));
    i += 1;
    a.set(i, &SecretModp::from(1));
    i += 1;
    a.set(i, &SecretModp::from(1));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(1));
    i += 1;
    a.set(i, &SecretModp::from(1));
    i += 1;
    a.set(i, &SecretModp::from(1));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(1));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(1));
    i += 1;
    a.set(i, &SecretModp::from(1));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(1));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(1));
    i += 1;
    a.set(i, &SecretModp::from(1));
    i += 1;
    a.set(i, &SecretModp::from(1));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    i += 1;
    a.set(i, &SecretModp::from(0));
    println!("OK");
    let resp = KOr(&a);
    println!("resp---", resp.reveal()); // Should be 1

    if i64::from(test_1.eq(test_2).reveal()) == 1 {
        println!("match");
    } else {
        println!("notmatch");
    }
    let test_1 = SecretInteger::<64>::from(10);
    let test_2 = SecretInteger::<64>::from(10);

    if i64::from(test_1.eq(test_2).reveal()) == 1 {
        println!("match");
    } else {
        println!("notmatch");
    }
}
