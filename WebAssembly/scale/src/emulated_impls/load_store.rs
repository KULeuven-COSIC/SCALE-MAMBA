// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use crate::{ClearModp, SecretI64, SecretModp};
use num_bigint::ToBigInt;
use std::sync::Mutex;

#[no_mangle]
extern "C" fn __ldi(value: i32) -> ClearModp {
    value.to_bigint().unwrap().into()
}

#[no_mangle]
extern "C" fn __ldsi(value: i32) -> SecretModp {
    value.to_bigint().unwrap().into()
}

#[no_mangle]
extern "C" fn __ldmc(address: u32) -> ClearModp {
    let address = address as usize;
    let mut vec = MEMORY_CLEAR.lock().unwrap();
    if vec.len() <= address {
        vec.resize(address + 1, ClearModp::from(0_i64));
    }
    vec[address]
}

#[no_mangle]
extern "C" fn __ldmci(address: i64) -> ClearModp {
    let address = address as usize;
    let mut vec = MEMORY_CLEAR.lock().unwrap();
    if vec.len() <= address {
        vec.resize(address + 1, ClearModp::from(0_i64));
    }
    vec[address]
}

#[no_mangle]
extern "C" fn __ldms(address: u32) -> SecretModp {
    let address = address as usize;
    let mut vec = MEMORY_SECRET.lock().unwrap();
    if vec.len() <= address {
        vec.resize(address + 1, SecretModp::from(SecretI64::from(0_i64)));
    }
    vec[address]
}

#[no_mangle]
extern "C" fn __ldmsi(address: i64) -> SecretModp {
    let address = address as usize;
    let mut vec = MEMORY_SECRET.lock().unwrap();
    if vec.len() <= address {
        vec.resize(address + 1, SecretModp::from(SecretI64::from(0_i64)));
    }
    vec[address]
}

#[no_mangle]
extern "C" fn __ldmsint(address: u32) -> SecretI64 {
    let address = address as usize;
    let mut vec = MEMORY_SECRET_I64.lock().unwrap();
    if vec.len() <= address {
        vec.resize(address + 1, SecretI64::from(0_i64));
    }
    vec[address]
}

#[no_mangle]
extern "C" fn __ldmsinti(address: i64) -> SecretI64 {
    let address = address as usize;
    let mut vec = MEMORY_SECRET_I64.lock().unwrap();
    if vec.len() <= address {
        vec.resize(address + 1, SecretI64::from(0_i64));
    }
    vec[address]
}

#[no_mangle]
extern "C" fn __ldmint(address: u32) -> i64 {
    let address = address as usize;
    let mut vec = MEMORY_I64.lock().unwrap();
    if vec.len() <= address {
        vec.resize(address + 1, 0);
    }
    vec[address]
}

#[no_mangle]
extern "C" fn __ldminti(address: i64) -> i64 {
    let address = address as usize;
    let mut vec = MEMORY_I64.lock().unwrap();
    if vec.len() <= address {
        vec.resize(address + 1, 0);
    }
    vec[address]
}

#[no_mangle]
extern "C" fn __stmc(value: ClearModp, address: u32) {
    let address = address as usize;
    let mut vec = MEMORY_CLEAR.lock().unwrap();
    if vec.len() <= address {
        vec.resize(address + 1, ClearModp::from(0_i64));
    }
    vec[address] = value;
}

#[no_mangle]
extern "C" fn __stmci(value: ClearModp, address: i64) {
    let address = address as usize;
    let mut vec = MEMORY_CLEAR.lock().unwrap();
    if vec.len() <= address {
        vec.resize(address + 1, ClearModp::from(0_i64));
    }
    vec[address] = value;
}

#[no_mangle]
extern "C" fn __stms(value: SecretModp, address: u32) {
    let address = address as usize;
    let mut vec = MEMORY_SECRET.lock().unwrap();
    if vec.len() <= address {
        vec.resize(address + 1, SecretModp::from(SecretI64::from(0_i64)));
    }
    vec[address] = value;
}

#[no_mangle]
extern "C" fn __stmsi(value: SecretModp, address: i64) {
    let address = address as usize;
    let mut vec = MEMORY_SECRET.lock().unwrap();
    if vec.len() <= address {
        vec.resize(address + 1, SecretModp::from(SecretI64::from(0_i64)));
    }
    vec[address] = value;
}

#[no_mangle]
extern "C" fn __stmint(value: i64, address: u32) {
    let address = address as usize;
    let mut vec = MEMORY_I64.lock().unwrap();
    if vec.len() <= address {
        vec.resize(address + 1, 0);
    }
    vec[address] = value;
}

#[no_mangle]
extern "C" fn __stminti(value: i64, address: i64) {
    let address = address as usize;
    let mut vec = MEMORY_I64.lock().unwrap();
    if vec.len() <= address {
        vec.resize(address + 1, 0);
    }
    vec[address] = value;
}

#[no_mangle]
extern "C" fn __stmsint(value: SecretI64, address: u32) {
    let address = address as usize;
    let mut vec = MEMORY_SECRET_I64.lock().unwrap();
    if vec.len() <= address {
        vec.resize(address + 1, SecretI64::from(0_i64));
    }
    vec[address] = value;
}

#[no_mangle]
extern "C" fn __stmsinti(value: SecretI64, address: i64) {
    let address = address as usize;
    let mut vec = MEMORY_SECRET_I64.lock().unwrap();
    if vec.len() <= address {
        vec.resize(address + 1, SecretI64::from(0_i64));
    }
    vec[address] = value;
}

/*
#[no_mangle]
extern "C" fn __movc(value: ClearModp) -> (ClearModp) {}

#[no_mangle]
extern "C" fn __movs(value: SecretModp) -> (SecretModp) {}

#[no_mangle]
extern "C" fn __movint(value: i64) -> (i64) {}

*/

#[no_mangle]
extern "C" fn __clear_memory() {
    MEMORY_I64.lock().unwrap().clear();
    MEMORY_CLEAR.lock().unwrap().clear();
    MEMORY_SECRET.lock().unwrap().clear();
    MEMORY_SECRET_I64.lock().unwrap().clear();
}

#[no_mangle]
// Doing nothing, at runtime we have no concept of registers
extern "C" fn __clear_registers() {}

lazy_static::lazy_static! {
    pub static ref MEMORY_I64: Mutex<Vec<i64>> = Default::default();
    pub static ref MEMORY_CLEAR: Mutex<Vec<crate::ClearModp>> = Default::default();
    pub static ref MEMORY_SECRET: Mutex<Vec<crate::SecretModp>> = Default::default();
    pub static ref MEMORY_SECRET_I64: Mutex<Vec<crate::SecretI64>> = Default::default();
}
