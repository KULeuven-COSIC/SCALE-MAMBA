
// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![no_std]
#![no_main]
#![feature(const_evaluatable_checked)]

use scale_std::floating_point::*;

#[scale::main(KAPPA = 40)]
#[inline(always)]
fn main() {
    let cf: ClearFloat<40, 10> = ClearFloat::from(3.14532);
    println!(cf.clone());
    let sf: SecretFloat<40, 10> = SecretFloat::from(cf);
    let csf: ClearFloat<40, 10> = sf.reveal();
    let ca: ClearFloat<40, 10> = ClearFloat::from(-123456789_i64);
    println!(ca);
    let sa: SecretFloat<40, 10> = SecretFloat::from(123.456789_f64);
    println!(sa.reveal());
    let sa: SecretFloat<40, 10> = SecretFloat::from(-123456789_i64);
    println!(sa.reveal());
}
