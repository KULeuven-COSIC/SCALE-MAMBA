#![no_std]
#![no_main]
#![feature(const_evaluatable_checked)]

use scale_std::slice::Slice;
use scale_std::array::Array;

scale::main! {
    I64_MEMORY = 10;
    SECRET_I64_MEMORY = 10;
    SECRET_MODP_MEMORY = 10;
    CLEAR_MODP_MEMORY = 10;
    KAPPA = 40;
}

#[inline(always)]
fn main() {
    let a: Slice<Array<i64, 2>> = Slice::uninitialized(5);
    for i in 0..5 {
        for j in 0..2 {
            a.get(i).set(j, &((i * 2 + j) as i64));
        }
    }
    let mut problem = false;
    for i in 0..5 {
        for j in 0..2 {
            print!(i, ",", j, ": ", a.get(i as u64).get(j as u64), "\n");
            if a.get(i as u64).get(j as u64) != i * 2 + j {
                print!("ERROR ^^^^^^^^^^^^^^^^\n");
                problem = true;
            }
        }
    }
    if problem {
        scale::panic!("There were errors above");
    }
}
