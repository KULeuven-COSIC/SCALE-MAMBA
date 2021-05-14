// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use crate::{ClearModp, SecretModp};
use num_bigint::BigInt;

// ADDITIONS
#[no_mangle]
extern "C" fn __addc(a: ClearModp, b: ClearModp) -> ClearModp {
    let a: BigInt = a.into();
    let b: BigInt = b.into();
    (a + b).into()
}

#[no_mangle]
extern "C" fn __adds(a: SecretModp, b: SecretModp) -> SecretModp {
    SecretModp(__addc(a.0, b.0))
}

#[no_mangle]
extern "C" fn __addm(a: SecretModp, b: ClearModp) -> SecretModp {
    SecretModp(__addc(a.0, b))
}

#[no_mangle]
extern "C" fn __addci(a: ClearModp, b: i32) -> ClearModp {
    let a: BigInt = a.into();
    let b: BigInt = b.into();
    (a + b).into()
}

#[no_mangle]
extern "C" fn __addsi(a: SecretModp, b: i32) -> SecretModp {
    SecretModp(__addci(a.0, b))
}

//SUBTRACTIONS
#[no_mangle]
extern "C" fn __subc(a: ClearModp, b: ClearModp) -> ClearModp {
    let a: BigInt = a.into();
    let b: BigInt = b.into();
    (a - b).into()
}

#[no_mangle]
extern "C" fn __subci(a: ClearModp, b: i32) -> ClearModp {
    let a: BigInt = a.into();
    let b: BigInt = b.into();
    (a - b).into()
}

#[no_mangle]
extern "C" fn __subcfi(a: ClearModp, b: i32) -> ClearModp {
    let a: BigInt = a.into();
    let b: BigInt = b.into();
    (b - a).into()
}

#[no_mangle]
extern "C" fn __subs(a: SecretModp, b: SecretModp) -> SecretModp {
    SecretModp(__subc(a.0, b.0))
}

#[no_mangle]
extern "C" fn __subsi(a: SecretModp, b: i32) -> SecretModp {
    SecretModp(__subci(a.0, b))
}

#[no_mangle]
extern "C" fn __subsfi(a: SecretModp, b: i32) -> SecretModp {
    SecretModp(__subcfi(a.0, b))
}

#[no_mangle]
extern "C" fn __subml(a: SecretModp, b: ClearModp) -> SecretModp {
    SecretModp(__subc(a.0, b))
}

#[no_mangle]
extern "C" fn __submr(a: ClearModp, b: SecretModp) -> SecretModp {
    SecretModp(__subc(a, b.0))
}
