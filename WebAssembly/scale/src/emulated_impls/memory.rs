// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use super::load_store::{MEMORY_CLEAR, MEMORY_I64, MEMORY_SECRET, MEMORY_SECRET_I64};
use crate::{ClearModp, SecretI64, SecretModp};

/// ClearModp
#[no_mangle]
extern "C" fn __newc(n: i64) -> i64 {
    let mut vec = MEMORY_CLEAR.lock().unwrap();
    let current_len = vec.len();

    vec.resize(current_len + n as usize, ClearModp::from(0_i64));

    current_len as i64
}

/// ClearModp
#[no_mangle]
extern "C" fn __deletec(_n: i64) {}

/// SecretModp
#[no_mangle]
extern "C" fn __news(n: i64) -> i64 {
    let mut vec = MEMORY_SECRET.lock().unwrap();
    let current_len = vec.len();

    vec.resize(current_len + n as usize, SecretModp::from(0_i64));

    current_len as i64
}
/// SecretModp
#[no_mangle]
extern "C" fn __deletes(_n: i64) {}

/// SecretI64
#[no_mangle]
extern "C" fn __newsint(n: i64) -> i64 {
    let mut vec = MEMORY_SECRET_I64.lock().unwrap();
    let current_len = vec.len();

    vec.resize(current_len + n as usize, SecretI64::from(0_i64));

    current_len as i64
}

/// SecretI64
#[no_mangle]
extern "C" fn __deletesint(_n: i64) {}

/// i64
#[no_mangle]
extern "C" fn __newint(n: i64) -> i64 {
    let mut vec = MEMORY_I64.lock().unwrap();
    let current_len = vec.len();

    vec.resize(current_len + n as usize, 0_i64);

    current_len as i64
}

/// i64
#[no_mangle]
extern "C" fn __deleteint(_n: i64) {}
