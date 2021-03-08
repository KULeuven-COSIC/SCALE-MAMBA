// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![no_std]
#![no_main]
#![feature(const_evaluatable_checked)]

use scale_std::array::Array;
use scale_std::slice::Slice;

#[scale::main(KAPPA = 40)]
#[inline(always)]
fn main() {
    let mut a: Slice<Array<i64, 2>> = Slice::uninitialized(5);
    for i in 0..5 {
        for j in 0..2 {
            a.get_mut(i).unwrap().set(j, &((i * 2 + j) as i64));
        }
    }
    let mut problem = false;
    for i in 0..5 {
        for j in 0..2 {
            let arr = a.get(i as u64).unwrap();
            print!(i, ",", j, ": ", arr.get(j as u64).unwrap(), "\n");
            if *arr.get(j as u64).unwrap() != i * 2 + j {
                print!("ERROR ^^^^^^^^^^^^^^^^\n");
                problem = true;
            }
        }
    }
    if problem {
        scale::panic!("There were errors above");
    }
}
