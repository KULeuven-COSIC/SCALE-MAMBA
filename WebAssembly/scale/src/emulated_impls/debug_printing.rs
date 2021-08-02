// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use crate::ClearModp;
use core::mem;
use num_bigint::BigInt;
use num_traits::ToPrimitive;

use crate::testing_emulated::P;

#[no_mangle]
extern "C" fn __print_reg(value: ClearModp) {
    let mut value: BigInt = value.into();
    if value > (&*P / 2) {
        value -= &*P;
    }
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
extern "C" fn __print_fix(value: ClearModp, f: i32, _k: i32) {
    let mut val_big : BigInt = value.into();
    if val_big > (&*P / 2) {
        val_big -= &*P;
    }
    let val64 : i64 = val_big.to_i64().unwrap();
    let mut float = val64 as f64;
    float = float / ((1_i64 << f) as f64);
    print!("{}", float);
}

#[no_mangle]
extern "C" fn __print_float(v: ClearModp, p: ClearModp, z: ClearModp, s: ClearModp, e: ClearModp) {
    let v_i: BigInt = v.into();
    let mut p_i: BigInt = p.into();
    if p_i > (&*P / 2) {
        p_i -= &*P;
    }
    let z_i: BigInt = z.into();
    let s_i: BigInt = s.into();
    let e_i: BigInt = e.into();
    let v_f: f64 = v_i.to_f64().unwrap();
    let p_f: f64 = p_i.to_f64().unwrap();
    let z_f: f64 = z_i.to_f64().unwrap();
    let s_f: f64 = s_i.to_f64().unwrap();
    let mut vv = (1.0-z_f)*(1.0-2.0*s_f)*(v_f)*(p_f.exp2());
    if (e_i.to_i64().unwrap())!=0 {
        vv = f64::NAN;
    }
    print!("{}",vv);
}
