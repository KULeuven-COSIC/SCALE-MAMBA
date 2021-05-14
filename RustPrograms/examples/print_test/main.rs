// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![no_std]
#![no_main]

#[scale::main(KAPPA = 40)]
#[inline(never)]
fn main() {
    let val = 1_isize;
    print!(val);
    let val = 1_usize;
    print!(val);
    let val = 1_i128;
    print!(val);
    let val = 1_u128;
    print!(val);
    let val = 1_i64;
    print!(val);
    let val = 1_u64;
    print!(val);
    let val = 1_i32;
    print!(val);
    let val = 1_u32;
    print!(val);
    let val = 1_i16;
    print!(val);
    let val = 1_u16;
    print!(val);
    let val = 1_i8;
    print!(val);
    let val = 1_u8;
    print!(val);
    let val = true;
    print!(val);
}
