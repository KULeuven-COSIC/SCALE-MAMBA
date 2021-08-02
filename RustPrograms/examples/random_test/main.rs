// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![no_std]
#![no_main]
#![feature(const_evaluatable_checked)]

use scale_std::ieee::*;
use scale_std::fixed_point::*;
use scale_std::integer::*;

/* Tests the random number generators */

#[scale::main(KAPPA = 40)]
#[inline(always)]
fn main() {
    let x: i64 = 100;
    for _i in 0..10 {
        let a : i64 = x.rand();
        println!(" i64 rand mod ", x, " : ",a);

        let a = i64::randomize();
        println!(" i64 rand : ",a);

        let a = SecretI64::randomize();
        println!(" SecretI64 rand : ",a.reveal());

        let a = ClearModp::randomize();
        println!(" ClearModp rand : ",a);

        let a = SecretModp::randomize();
        println!(" SecretModp rand : ",a.reveal());

        let a = SecretBit::randomize();
        println!(" SecretBit rand : ",a.reveal());

        let a = ClearIEEE::randomize();
        println!(" ClearIEEE rand : ",a);

        let a = SecretIEEE::randomize();
        println!(" SecretIEEE rand : ",a.reveal());

        let a: ClearInteger<10> = ClearInteger::randomize();
        println!(" ClearInteger<10> rand : ",a);

        let a: SecretInteger<10> = SecretInteger::randomize();
        println!(" SecretInteger<10> rand : ",a.reveal());

        let a: ClearFixed<20,10> = ClearFixed::randomize();
        println!(" ClearFixed<20,10> rand : ",a);

        let a: SecretFixed<20,10> = SecretFixed::randomize();
        println!(" SecretFixed<20,10> rand : ",a.reveal());

        let a = SecretModp::get_random_bit();
        println!(" SecretModp rand bit : ",a.reveal());

        let (a,b) = SecretModp::get_random_square();
        println!(" SecretModp rand square : (",a.reveal(),",",b.reveal(),")");

        let (a,b,c) = SecretModp::get_random_triple();
        println!(" SecretModp rand triple : (",a.reveal(),",",b.reveal(),",",c.reveal(),")");

        println!("-----------------------");

    }
}
