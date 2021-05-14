// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

mod addition;
mod array_arith;
mod bitwise;
mod conversion;
mod debug_printing;
mod int;
mod io;
mod load_store;
mod logical_instructions;
mod machine;
mod memory;
mod multiplication_division;
mod numb_th;
mod open;
mod secret_instructions;
mod sregint_cmp;
mod stacks;

use num_bigint::BigInt;
use std::collections::HashMap;
use std::sync::Mutex;

pub use multiplication_division::__mul2sint;

lazy_static::lazy_static! {
    static ref NUMBER_TABLE: Mutex<HashMap<i64, BigInt>> = Default::default();
}

impl From<crate::ClearModp> for BigInt {
    fn from(val: crate::ClearModp) -> BigInt {
        NUMBER_TABLE.lock().unwrap().get(&val.0).unwrap().clone()
    }
}

impl From<BigInt> for crate::ClearModp {
    fn from(int: BigInt) -> Self {
        let mut int = int % &*crate::testing_emulated::P;
        if int < BigInt::from(0) {
            int += &*crate::testing_emulated::P;
        }
        let mut number_table = NUMBER_TABLE.lock().unwrap();
        let id = number_table.len() as u64 as i64;
        number_table.insert(id, int);
        Self(id)
    }
}

impl From<crate::SecretModp> for BigInt {
    fn from(val: crate::SecretModp) -> BigInt {
        val.0.into()
    }
}

impl From<BigInt> for crate::SecretModp {
    fn from(int: BigInt) -> Self {
        Self(int.into())
    }
}

impl From<crate::SecretBit> for BigInt {
    fn from(val: crate::SecretBit) -> BigInt {
        val.0.into()
    }
}
