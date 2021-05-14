// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use crate::{ClearModp, RawSecretBit, SecretI64, SecretModp};
use std::sync::Mutex;

#[no_mangle]
extern "C" fn __peekint(address: i64) -> i64 {
    let address = address as usize;
    let vec = STACK_I64.lock().unwrap();
    vec[address]
}

#[no_mangle]
extern "C" fn __pokeint(address: i64, value: i64) {
    let address = address as usize;
    let mut vec = STACK_I64.lock().unwrap();
    vec[address] = value;
}

#[no_mangle]
extern "C" fn __pushint(value: i64) {
    let mut vec = STACK_I64.lock().unwrap();
    vec.push(value);
}

#[no_mangle]
extern "C" fn __popint() -> i64 {
    let mut vec = STACK_I64.lock().unwrap();
    vec.pop().unwrap()
}

#[no_mangle]
extern "C" fn __getspint() -> i64 {
    let vec = STACK_I64.lock().unwrap();
    (vec.len() - 1) as i64
}

#[no_mangle]
extern "C" fn __peekc(address: i64) -> ClearModp {
    let address = address as usize;
    let vec = STACK_CLEAR.lock().unwrap();
    vec[address]
}

#[no_mangle]
extern "C" fn __pokec(address: i64, value: ClearModp) {
    let address = address as usize;
    let mut vec = STACK_CLEAR.lock().unwrap();
    vec[address] = value;
}

#[no_mangle]
extern "C" fn __pushc(value: ClearModp) {
    let mut vec = STACK_CLEAR.lock().unwrap();
    vec.push(value);
}

#[no_mangle]
extern "C" fn __popc() -> ClearModp {
    let mut vec = STACK_CLEAR.lock().unwrap();
    vec.pop().unwrap()
}

#[no_mangle]
extern "C" fn __getspc() -> i64 {
    let vec = STACK_CLEAR.lock().unwrap();
    (vec.len() - 1) as i64
}

#[no_mangle]
extern "C" fn __peeks(address: i64) -> SecretModp {
    let address = address as usize;
    let vec = STACK_SECRET.lock().unwrap();
    vec[address]
}

#[no_mangle]
extern "C" fn __pokes(address: i64, value: SecretModp) {
    let address = address as usize;
    let mut vec = STACK_SECRET.lock().unwrap();
    vec[address] = value;
}

#[no_mangle]
extern "C" fn __pushs(value: SecretModp) {
    let mut vec = STACK_SECRET.lock().unwrap();
    vec.push(value);
}

#[no_mangle]
extern "C" fn __pops() -> SecretModp {
    let mut vec = STACK_SECRET.lock().unwrap();
    vec.pop().unwrap()
}

#[no_mangle]
extern "C" fn __getsps() -> i64 {
    let vec = STACK_SECRET.lock().unwrap();
    (vec.len() - 1) as i64
}

#[no_mangle]
extern "C" fn __peeksint(address: i64) -> SecretI64 {
    let address = address as usize;
    let vec = STACK_SECRET_I64.lock().unwrap();
    vec[address]
}

#[no_mangle]
extern "C" fn __pokesint(address: i64, value: SecretI64) {
    let address = address as usize;
    let mut vec = STACK_SECRET_I64.lock().unwrap();
    vec[address] = value;
}

#[no_mangle]
extern "C" fn __pushsint(value: SecretI64) {
    let mut vec = STACK_SECRET_I64.lock().unwrap();
    vec.push(value);
}

#[no_mangle]
extern "C" fn __popsint() -> SecretI64 {
    let mut vec = STACK_SECRET_I64.lock().unwrap();
    vec.pop().unwrap()
}

#[no_mangle]
extern "C" fn __getspsint() -> i64 {
    let vec = STACK_SECRET_I64.lock().unwrap();
    (vec.len() - 1) as i64
}

#[no_mangle]
extern "C" fn __peeksbit(address: i64) -> RawSecretBit {
    let address = address as usize;
    let vec = STACK_SECRET_BIT.lock().unwrap();
    vec[address]
}

#[no_mangle]
extern "C" fn __pokesbit(address: i64, value: RawSecretBit) {
    let address = address as usize;
    let mut vec = STACK_SECRET_BIT.lock().unwrap();
    vec[address] = value;
}

#[no_mangle]
extern "C" fn __pushsbit(value: RawSecretBit) {
    let mut vec = STACK_SECRET_BIT.lock().unwrap();
    vec.push(value);
}

#[no_mangle]
extern "C" fn __popsbit() -> RawSecretBit {
    let mut vec = STACK_SECRET_BIT.lock().unwrap();
    vec.pop().unwrap()
}

#[no_mangle]
extern "C" fn __getspsbit() -> i64 {
    let vec = STACK_SECRET_BIT.lock().unwrap();
    (vec.len() - 1) as i64
}

#[no_mangle]
extern "C" fn __rpeekint(address: i64) -> i64 {
    let address = address as usize;
    let vec = STACK_I64.lock().unwrap();
    vec[vec.len() - 1 - address]
}

#[no_mangle]
extern "C" fn __rpokeint(address: i64, value: i64) {
    let address = address as usize;
    let mut vec = STACK_I64.lock().unwrap();
    let vv = vec.len();
    vec[vv - 1 - address] = value;
}

#[no_mangle]
extern "C" fn __rpeekc(address: i64) -> ClearModp {
    let address = address as usize;
    let vec = STACK_CLEAR.lock().unwrap();
    vec[vec.len() - 1 - address]
}

#[no_mangle]
extern "C" fn __rpokec(address: i64, value: ClearModp) {
    let address = address as usize;
    let mut vec = STACK_CLEAR.lock().unwrap();
    let vv = vec.len();
    vec[vv - 1 - address] = value;
}

#[no_mangle]
extern "C" fn __rpeeks(address: i64) -> SecretModp {
    let address = address as usize;
    let vec = STACK_SECRET.lock().unwrap();
    vec[vec.len() - 1 - address]
}

#[no_mangle]
extern "C" fn __rpokes(address: i64, value: SecretModp) {
    let address = address as usize;
    let mut vec = STACK_SECRET.lock().unwrap();
    let vv = vec.len();
    vec[vv - 1 - address] = value;
}

#[no_mangle]
extern "C" fn __rpeeksint(address: i64) -> SecretI64 {
    let address = address as usize;
    let vec = STACK_SECRET_I64.lock().unwrap();
    vec[vec.len() - 1 - address]
}

#[no_mangle]
extern "C" fn __rpokesint(address: i64, value: SecretI64) {
    let address = address as usize;
    let mut vec = STACK_SECRET_I64.lock().unwrap();
    let vv = vec.len();
    vec[vv - 1 - address] = value;
}

#[no_mangle]
extern "C" fn __rpeeksbit(address: i64) -> RawSecretBit {
    let address = address as usize;
    let vec = STACK_SECRET_BIT.lock().unwrap();
    vec[vec.len() - 1 - address]
}

#[no_mangle]
extern "C" fn __rpokesbit(address: i64, value: RawSecretBit) {
    let address = address as usize;
    let mut vec = STACK_SECRET_BIT.lock().unwrap();
    let vv = vec.len();
    vec[vv - 1 - address] = value;
}

lazy_static::lazy_static! {
    static ref STACK_I64: Mutex<Vec<i64>> = Default::default();
    static ref STACK_CLEAR: Mutex<Vec<crate::ClearModp>> = Default::default();
    static ref STACK_SECRET: Mutex<Vec<crate::SecretModp>> = Default::default();
    static ref STACK_SECRET_I64: Mutex<Vec<crate::SecretI64>> = Default::default();
    static ref STACK_SECRET_BIT: Mutex<Vec<crate::RawSecretBit>> = Default::default();
}
