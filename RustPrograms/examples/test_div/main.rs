// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![no_std]
#![no_main]
#![feature(const_evaluatable_checked, const_generics)]
#![allow(incomplete_features)]

use scale_std::fixed_point::*;

fn test_approx<const K: u64, const F: u64>(a: ClearFixed<K, F>, val: ClearFixed<K, F>) -> ClearModp
where
    ConstU64<{ K + 1 }>: ,
    ConstU64<{ K - 1 }>: ,
{
    let lower = val - ClearFixed::from(0.00001);
    let upper = val + ClearFixed::from(0.00001);
    let ok = a.gt(lower) & a.lt(upper);
    ok
}

#[scale::main(KAPPA = 40)]
#[inline(always)]
fn main() {
    print!("\n\n Test the math SecretFixed library \n");
    let c: SecretFixed<40, 20> = SecretFixed::from(0.5431);
    let t = c.acos().reveal();
    print!("acos(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(0.99667167));
    ok.test_value(ClearModp::from(1));

    let t = c.asin().reveal();
    print!("asin(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(0.5741246));
    ok.test_value(ClearModp::from(1));
}
