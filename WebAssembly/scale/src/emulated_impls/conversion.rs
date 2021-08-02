// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use crate::{ClearModp, RawSecretBit, SecretI64, SecretModp};
use num_bigint::{BigInt, ToBigInt};
use std::convert::TryInto;

use crate::testing_emulated::P;

#[no_mangle]
extern "C" fn __convint(value: i64) -> ClearModp {
    value.to_bigint().unwrap().into()
}

#[no_mangle]
extern "C" fn __convmodp(val: ClearModp, bitlength: u32) -> i64 {
    let mut value: BigInt = val.into();
    let mut neg = 0;
    if value > (&*P / 2) {
        neg = 1;
        value = &*P - value;
    }
    value = value & ((BigInt::from(1) << (bitlength-1))-1);
    if neg == 1 {
       value=-value;
    }
    value.try_into().unwrap()
}

#[no_mangle]
extern "C" fn __convsintsreg(value: SecretModp) -> SecretI64 {
    let mut value: BigInt = value.into();
    if value > (&*P / 2) {
        value -= &*P;
    }
    SecretI64(value.try_into().unwrap())
}

#[no_mangle]
extern "C" fn __convregsreg(value: i64) -> SecretI64 {
    SecretI64(value)
}

#[no_mangle]
extern "C" fn __convsregsint(value: SecretI64) -> SecretModp {
    let value: BigInt = value.0.into();
    SecretModp(value.into())
}

#[no_mangle]
extern "C" fn __convsintsbit(value: SecretI64) -> RawSecretBit {
    RawSecretBit(value.0 & 1)
}

#[no_mangle]
extern "C" fn __convsbitsint(value: RawSecretBit) -> SecretI64 {
    SecretI64(value.0)
}

#[no_mangle]
extern "C" fn __convsuregsint(value: SecretI64) -> SecretModp {
    let mut value: BigInt = value.0.into();
    if value < BigInt::from(0) {
        value += BigInt::from(1) << 64;
    }
    SecretModp(value.into())
}
