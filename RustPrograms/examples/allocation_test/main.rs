// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![no_std]
#![no_main]

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

    let first_a = a.get(0).unwrap().clone();
    let test = first_a.get(0);
    let array = a.get(0).unwrap().clone();
    drop(a);

    println!("a --- ", *test.unwrap());
    println!("array --- ", *array.get(1).unwrap());

    let mut a: Slice<i64> = Slice::uninitialized(5);
    for i in 0..5 {
        a.set(i, &(i as i64));
    }
    for i in 0..5 {
        a.get(i).unwrap();
        println!("value === ", *a.get(i).unwrap());
    }
    let test = a.get(1).unwrap().clone();

    let cloned_a = a.clone();
    drop(a);

    scale::assert!(cloned_a.get(10).is_none());
    println!("----", test);

    for elt in cloned_a {
        println!("element cloned --- ", elt);
    }
}
