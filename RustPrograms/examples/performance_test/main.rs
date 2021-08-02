// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![no_std]
#![no_main]
#![feature(const_evaluatable_checked, const_generics)]
#![allow(non_snake_case, incomplete_features)]

use scale_std::fixed_point::*;

/* The purpose of this test is to perform a test
 * of a repeated operation. We can use this to
 * test performance improvements, or compare
 * the Rust pipelines performance with the
 * Mamba pipeline performance.
 */

#[scale::main(KAPPA = 40)]
#[inline(always)]
fn main() {
    let a: ClearFixed<40, 20> = ClearFixed::from(3.00);
    let mut b: SecretFixed<40, 20> = SecretFixed::from(a);

    start_clock(ConstI32::<1>);
    for _i in 0..100 {
        b = b.sin();
        b = b.asin();
    }
    stop_clock(ConstI32::<1>);
}
