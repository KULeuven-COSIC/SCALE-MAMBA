// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

//! This program is NOT meant to be run in the test environment
//! You should compile it and run it with three players to see
//! what happens (also you can check the asm prodcued)
#![no_std]
#![no_main]
#![feature(const_evaluatable_checked)]

use scale_std::array::Array;
use scale_std::slice::Slice;

#[scale::main(KAPPA = 40)]
#[inline(always)]
fn main() {
    let v: i64 = 1;

    let _ans = open_channel(10);
    v.output(1);

    let b = ConstI32::<3543>;

    let sa = SecretModp::private_input(1, 10);
    let sb = SecretModp::from(b);
    let sc = sa * sb;
    let ca = sc.reveal();
    ca.output(2);
    sa.private_output(0, 3);
    sb.private_output(1, 4);
    sc.private_output(2, 5);

    println!("Enter two values on each player. Must be same on each player");
    let ca = ClearModp::input(40);
    let a = i64::input(50);
    ca.output(6);
    a.output(7);

    println!("Now player 1 enter the array of five values");

    let a: Array<SecretModp, 5> = Array::private_input(1, 20);
    for i in 0..5 {
        println!("a.", i, " = ", a.get(i).unwrap().reveal());
    }
    a.private_output(2,8);

    println!("Now player 2 enter the slice of six values");

    let b: Slice<SecretModp> = Slice::private_input(6, 2, 30);
    for i in 0..6 {
        println!("b.", i, " = ", b.get(i).unwrap().reveal());
    }
    b.private_output(2, 9);

    println!("Now each player enter a value");
    for i in 0..3 {
        println!("Player ",i," enter a value:");
        let sa = SecretModp::private_input(i, 15);
        println!("Player ",i," enterred the value ",sa.reveal());
    }

    close_channel(10);
}
