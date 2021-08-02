// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![no_std]
#![no_main]
#![feature(const_evaluatable_checked)]
#![allow(non_snake_case, incomplete_features)]

use scale_std::bit_protocols::*;

#[scale::main(KAPPA = 40)]
#[inline(always)]
fn main() {
    /* We test BitDecFull in a separate program as we want
     * to also test this for small primes. And the
     * other things in test_bitprotocols will not work
     * for small primes
     */

    let ss1 = SecretModp::from(5);

    let v3 = BitDecFull(ss1);
    for i in 0..10 {
        print!(" i = ", i, "   ", v3.get_unchecked(i as u64).reveal(), "\n");
    }
    v3.get_unchecked(0).reveal().test_value(ClearModp::from(1));
    v3.get_unchecked(1).reveal().test_value(ClearModp::from(0));
    v3.get_unchecked(2).reveal().test_value(ClearModp::from(1));
    v3.get_unchecked(3).reveal().test_value(ClearModp::from(0));
    v3.get_unchecked(4).reveal().test_value(ClearModp::from(0));
    v3.get_unchecked(5).reveal().test_value(ClearModp::from(0));
    v3.get_unchecked(6).reveal().test_value(ClearModp::from(0));
    v3.get_unchecked(7).reveal().test_value(ClearModp::from(0));
    v3.get_unchecked(8).reveal().test_value(ClearModp::from(0));
    v3.get_unchecked(9).reveal().test_value(ClearModp::from(0));
}
