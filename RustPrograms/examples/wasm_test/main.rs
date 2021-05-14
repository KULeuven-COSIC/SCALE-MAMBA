// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![no_std]
#![no_main]

#[scale::main(KAPPA = 40)]
#[inline(never)]
fn main() {
    let mut row_nb = i32::max_value() - 4_i32;
    for j in 0..black_box(10) {
        black_box(j);
        print!(row_nb as i64, "\n");
        row_nb += 1;
    }
    if row_nb > 0 {
        scale::panic!("overflow not working for 32 bit integers\n");
    }
}
