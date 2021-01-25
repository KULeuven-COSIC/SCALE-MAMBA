use crate::testing_emulated::{mod_inverse, P};
use crate::{ClearModp, SecretI64, SecretModp};
use num_bigint::BigInt;
use std::convert::{TryFrom, TryInto};

#[no_mangle]
extern "C" fn __mulc(a: ClearModp, b: ClearModp) -> ClearModp {
    let a: BigInt = a.into();
    let b: BigInt = b.into();
    (a * b).into()
}

#[no_mangle]
extern "C" fn __mulm(a: SecretModp, b: ClearModp) -> SecretModp {
    let a: BigInt = a.into();
    let b: BigInt = b.into();
    (a * b).into()
}

#[no_mangle]
extern "C" fn __mulci(a: ClearModp, b: i32) -> ClearModp {
    let a: BigInt = a.into();
    let b: BigInt = b.into();
    (a * b).into()
}

#[no_mangle]
extern "C" fn __mulsi(a: SecretModp, b: i32) -> SecretModp {
    let a: BigInt = a.into();
    let b: BigInt = b.into();
    (a * b).into()
}

#[no_mangle]
extern "C" fn __divc(a: ClearModp, b: ClearModp) -> ClearModp {
    let a: BigInt = a.into();
    let b: BigInt = b.into();
    let x = mod_inverse(&b, &*P).unwrap();
    (a * x).into()
}

#[no_mangle]
extern "C" fn __divci(a: ClearModp, b: i32) -> ClearModp {
    let a: BigInt = a.into();
    let b: BigInt = b.into();
    let x = mod_inverse(&b, &*P).unwrap();
    (a * x).into()
}

#[no_mangle]
extern "C" fn __modc(a: ClearModp, b: ClearModp) -> ClearModp {
    let a: BigInt = a.into();
    let b: BigInt = b.into();
    (a % b).into()
}

#[no_mangle]
extern "C" fn __modci(a: ClearModp, b: i32) -> ClearModp {
    let a: BigInt = a.into();
    let b: BigInt = b.into();
    (a % b).into()
}

#[no_mangle]
pub fn __mul2sint(a: SecretI64, b: SecretI64) -> (SecretI64, SecretI64) {
    let aa: BigInt = a.0.into();
    let bb: BigInt = b.0.into();
    let c = aa * bb;
    let high: BigInt = &c >> 64;
    let low: BigInt = c - (&high << 64);
    (
        SecretI64(high.try_into().unwrap()),
        SecretI64(u64::try_from(low).unwrap() as i64),
    )
}
