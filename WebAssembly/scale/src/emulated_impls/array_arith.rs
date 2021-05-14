// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boï¿½tie, Paris, France.

use crate::ClearModp;
use crate::SecretModp;

// Doing nothing, at runtime as we test these instructions in a different
// way in the test routines [at present]
#[no_mangle]
extern "C" fn __maddc(_dest: i64, _a: i64, _b: i64, _len: i64) {}

#[no_mangle]
extern "C" fn __madds(_dest: i64, _a: i64, _b: i64, _len: i64) {}

#[no_mangle]
extern "C" fn __maddm(_dest: i64, _a: i64, _b: i64, _len: i64) {}

#[no_mangle]
extern "C" fn __msubc(_dest: i64, _a: i64, _b: i64, _len: i64) {}

#[no_mangle]
extern "C" fn __msubs(_dest: i64, _a: i64, _b: i64, _len: i64) {}

#[no_mangle]
extern "C" fn __msubml(_dest: i64, _a: i64, _b: i64, _len: i64) {}

#[no_mangle]
extern "C" fn __msubmr(_dest: i64, _a: i64, _b: i64, _len: i64) {}

#[no_mangle]
extern "C" fn __mmulc(_dest: i64, _a: i64, _b: i64, _len: i64) {}

#[no_mangle]
extern "C" fn __mmulm(_dest: i64, _a: i64, _b: i64, _len: i64) {}

#[no_mangle]
extern "C" fn __mdivc(_dest: i64, _a: i64, _b: i64, _len: i64) {}

#[no_mangle]
extern "C" fn __mmodc(_dest: i64, _a: i64, _b: i64, _len: i64) {}

#[no_mangle]
extern "C" fn __mrevc(_dest: i64, _a: i64) {}

#[no_mangle]
extern "C" fn __mrevs(_dest: i64, _a: i64) {}

#[no_mangle]
extern "C" fn __mevalcc(_arr: i64, _len: i64, _val: ClearModp) -> ClearModp {
    ClearModp::from(1)
}

#[no_mangle]
extern "C" fn __mevalsc(_arr: i64, _len: i64, _val: ClearModp) -> SecretModp {
    SecretModp::from(1)
}

#[no_mangle]
extern "C" fn __mbitdecc(_dest: i64, _a: ClearModp, _len: i64) {}

#[no_mangle]
extern "C" fn __mbitdecint(_dest: i64, _a: i64, _len: i64) {}
