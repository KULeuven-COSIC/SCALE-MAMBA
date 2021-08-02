// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use num_bigint::BigInt;
use std::convert::TryFrom;

use crate::testing_emulated::P;

#[no_mangle]
extern "C" fn __andsint(left: crate::SecretI64, right: crate::SecretI64) -> crate::SecretI64 {
    crate::SecretI64(left.0 & right.0)
}
#[no_mangle]
extern "C" fn __andsintc(left: crate::SecretI64, right: i64) -> crate::SecretI64 {
    crate::SecretI64(left.0 & right)
}
#[no_mangle]
extern "C" fn __orsint(left: crate::SecretI64, right: crate::SecretI64) -> crate::SecretI64 {
    crate::SecretI64(left.0 | right.0)
}
#[no_mangle]
extern "C" fn __orsintc(left: crate::SecretI64, right: i64) -> crate::SecretI64 {
    crate::SecretI64(left.0 | right)
}
#[no_mangle]
extern "C" fn __xorsint(left: crate::SecretI64, right: crate::SecretI64) -> crate::SecretI64 {
    crate::SecretI64(left.0 ^ right.0)
}
#[no_mangle]
extern "C" fn __xorsintc(left: crate::SecretI64, right: i64) -> crate::SecretI64 {
    crate::SecretI64(left.0 ^ right)
}

#[no_mangle]
extern "C" fn __andsb(
    left: crate::RawSecretBit,
    right: crate::RawSecretBit,
) -> crate::RawSecretBit {
    crate::RawSecretBit(left.0 & right.0)
}
#[no_mangle]
extern "C" fn __orsb(left: crate::RawSecretBit, right: crate::RawSecretBit) -> crate::RawSecretBit {
    crate::RawSecretBit(left.0 | right.0)
}
#[no_mangle]
extern "C" fn __xorsb(
    left: crate::RawSecretBit,
    right: crate::RawSecretBit,
) -> crate::RawSecretBit {
    crate::RawSecretBit(left.0 ^ right.0)
}

#[no_mangle]
extern "C" fn __andc(left: crate::ClearModp, right: crate::ClearModp) -> crate::ClearModp {
    (BigInt::from(left) & BigInt::from(right)).into()
}
#[no_mangle]
extern "C" fn __orc(left: crate::ClearModp, right: crate::ClearModp) -> crate::ClearModp {
    (BigInt::from(left) | BigInt::from(right)).into()
}
#[no_mangle]
extern "C" fn __xorc(left: crate::ClearModp, right: crate::ClearModp) -> crate::ClearModp {
    (BigInt::from(left) ^ BigInt::from(right)).into()
}

#[no_mangle]
extern "C" fn __andci(left: crate::ClearModp, right: i32) -> crate::ClearModp {
    (BigInt::from(left) & BigInt::from(right)).into()
}
#[no_mangle]
extern "C" fn __orci(left: crate::ClearModp, right: i32) -> crate::ClearModp {
    (BigInt::from(left) | BigInt::from(right)).into()
}
#[no_mangle]
extern "C" fn __xorci(left: crate::ClearModp, right: i32) -> crate::ClearModp {
    (BigInt::from(left) ^ BigInt::from(right)).into()
}

#[no_mangle]
extern "C" fn __shlsint(left: crate::SecretI64, right: i32) -> crate::SecretI64 {
    crate::SecretI64(left.0 << right)
}
#[no_mangle]
extern "C" fn __shrsint(left: crate::SecretI64, right: i32) -> crate::SecretI64 {
    crate::SecretI64(left.0 >> right)
}

#[no_mangle]
extern "C" fn __shlc(left: crate::ClearModp, right: crate::ClearModp) -> crate::ClearModp {
    (BigInt::from(left) << u128::try_from(BigInt::from(right)).unwrap()).into()
}
#[no_mangle]
extern "C" fn __shlci(left: crate::ClearModp, right: i32) -> crate::ClearModp {
    (BigInt::from(left) << right).into()
}
#[no_mangle]
extern "C" fn __shrc(left: crate::ClearModp, right: crate::ClearModp) -> crate::ClearModp {
    (BigInt::from(left) >> u128::try_from(BigInt::from(right)).unwrap()).into()
}
#[no_mangle]
extern "C" fn __shrci(left: crate::ClearModp, right: i32) -> crate::ClearModp {
    (BigInt::from(left) >> right).into()
}

#[no_mangle]
extern "C" fn __eqzc(value: crate::ClearModp) -> crate::ClearModp {
    let mut ans: crate::ClearModp = crate::ClearModp::from(0);
    if BigInt::from(value)==BigInt::from(0)
       { ans = crate::ClearModp::from(1); }
    ans
}

#[no_mangle]
extern "C" fn __ltzc(value: crate::ClearModp) -> crate::ClearModp {
    let val_big : BigInt = value.into();
    let mut ans: BigInt =BigInt::from(0);
    if val_big > (&*P / 2) {
        ans=BigInt::from(1);
    }
    ans.into()
}
