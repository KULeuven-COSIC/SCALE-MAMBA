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
