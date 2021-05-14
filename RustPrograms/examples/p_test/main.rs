// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![no_std]
#![no_main]

use scale::P;

#[scale::main(KAPPA = 40)]
#[inline(always)]
fn main() {
    const BITLEN: u64 = P.len() as u64;

    let mut found_nonzero = false;
    for i in 0u64..BITLEN {
        let b = P[(BITLEN - 1 - i) as usize];
        found_nonzero |= b;
        print!(b as i64, "\n");
        ClearModp::from(b as i64);
    }
    if !found_nonzero {
        scale::panic!()
    }
}
