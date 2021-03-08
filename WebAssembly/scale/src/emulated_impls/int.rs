
// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#[no_mangle]
extern "C" fn __ldint(value: i32) -> i64 {
    value.into()
}
#[no_mangle]
extern "C" fn __addint(left: i64, right: i64) -> i64 {
    left + right
}
#[no_mangle]
extern "C" fn __subint(left: i64, right: i64) -> i64 {
    left - right
}
#[no_mangle]
extern "C" fn __mulint(left: i64, right: i64) -> i64 {
    left * right
}
#[no_mangle]
extern "C" fn __divint(left: i64, right: i64) -> i64 {
    left / right
}
