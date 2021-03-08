
// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

//! This program is NOT meant to be run in the test environment
//! You should compile it and run it with three players to see
//! what happens (also you can check the asm prodcued)
#![no_std]
#![no_main]
#![feature(const_evaluatable_checked)]

#[scale::main(KAPPA = 40)]
#[inline(always)]
fn main() {
    let v: i64 = 1;

    let _ans = open_channel(Channel::<10>);
    v.output(Channel::<10>);

    let b = ConstI32::<3543>;

    let sa = SecretModp::private_input(Player::<1>, Channel::<10>);
    let sb = SecretModp::from(b);
    let sc = sa * sb;
    let ca = sc.reveal();
    ca.output(Channel::<10>);
    sa.private_output(Player::<0>, Channel::<10>);
    sb.private_output(Player::<1>, Channel::<10>);
    sc.private_output(Player::<2>, Channel::<10>);

    let ca = ClearModp::input(Channel::<10>);
    let a = i64::input(Channel::<10>);
    ca.output(Channel::<10>);
    a.output(Channel::<10>);
    close_channel(Channel::<10>);
}
