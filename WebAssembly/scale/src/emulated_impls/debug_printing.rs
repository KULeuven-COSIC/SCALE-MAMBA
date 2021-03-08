
// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use crate::ClearModp;
use core::mem;
use num_bigint::BigInt;

#[no_mangle]
extern "C" fn __print_reg(value: ClearModp) {
    let value: BigInt = value.into();
    print!("{}", value);
}

#[no_mangle]
extern "C" fn __print_int(value: i64) {
    print!("{}", value);
}

#[no_mangle]
extern "C" fn __print_char_regint(value: i64) {
    print!("{}", value as u8 as char);
}

#[no_mangle]
extern "C" fn __print_ieee_float(value: i64) {
    let float: f64 = unsafe { mem::transmute(value) };
    print!("{}", float);
}


#[no_mangle]
extern "C" fn __print_fix(value: i64, f: i32, _k: i32) {
    let mut float = value as f64;
    float = float / ((1_i64 << f) as f64);
    print!("{}", float);
}

#[no_mangle]
extern "C" fn __print_float(_v: i64, _p: i64, _z: i64, _s: i64, _e: i64) {
    print!(" <- Some Float Value -> ");
}
