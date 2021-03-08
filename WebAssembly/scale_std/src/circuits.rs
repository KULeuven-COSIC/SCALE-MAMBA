// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use crate::array::Array;
use scale::*;

/**************************
 * Cryptographic circuits *
 *      e.g. AES/SHA      *
 **************************/

const AES_128: u32 = 100;
const AES_192: u32 = 101;
const AES_256: u32 = 102;
const SHA_3: u32 = 103;
const SHA_256: u32 = 104;
const SHA_512: u32 = 105;

#[inline(always)]
#[allow(non_snake_case)]
pub fn AES128(key128: [SecretI64; 2], mess: [SecretI64; 2]) -> [SecretI64; 2] {
    let ciph = unsafe { execute_garbled_circuit!(AES_128(key128, mess) -> [SecretI64; 2]) };
    ciph
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn AES192(key192: [SecretI64; 3], mess: [SecretI64; 2]) -> [SecretI64; 2] {
    let ciph = unsafe { execute_garbled_circuit!(AES_192(key192, mess) -> [SecretI64; 2]) };
    ciph
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn AES256(key256: [SecretI64; 4], mess: [SecretI64; 2]) -> [SecretI64; 2] {
    let ciph = unsafe { execute_garbled_circuit!(AES_256(key256, mess) -> [SecretI64; 2]) };
    ciph
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn SHA3(istate: Array<SecretI64, 25>) -> Array<SecretI64, 25> {
    let ostate = unsafe { execute_garbled_circuit!(SHA_3(istate) -> Array<SecretI64,25>) };
    ostate
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn SHA256(mess: Array<SecretI64, 8>, state: Array<SecretI64, 4>) -> Array<SecretI64, 4> {
    let ostate = unsafe { execute_garbled_circuit!(SHA_256(mess,state) -> Array<SecretI64,4>) };
    ostate
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn SHA512(mess: Array<SecretI64, 16>, state: Array<SecretI64, 8>) -> Array<SecretI64, 8> {
    let ostate = unsafe { execute_garbled_circuit!(SHA_512(mess,state) -> Array<SecretI64,8>) };
    ostate
}

/**************************
 *   IEEE FP Operations   *
 **************************/

const FP_ADD: u32 = 120;
const FP_MUL: u32 = 121;
const FP_DIV: u32 = 122;
const FP_EQ: u32 = 123;
const FP_F2I: u32 = 124;
const FP_I2F: u32 = 125;
const FP_SQRT: u32 = 126;
const FP_LT: u32 = 127;
const FP_FLOOR: u32 = 128;
const FP_CEIL: u32 = 129;

#[inline(always)]
#[allow(non_snake_case)]
pub fn IEEE_add(input: [SecretI64; 2]) -> SecretI64 {
    let ans = unsafe { execute_garbled_circuit!(FP_ADD(input) -> SecretI64) };
    ans
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn IEEE_mul(input: [SecretI64; 2]) -> SecretI64 {
    let ans = unsafe { execute_garbled_circuit!(FP_MUL(input) -> SecretI64) };
    ans
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn IEEE_div(input: [SecretI64; 2]) -> SecretI64 {
    let ans = unsafe { execute_garbled_circuit!(FP_DIV(input) -> SecretI64) };
    ans
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn IEEE_eq(input: [SecretI64; 2]) -> SecretI64 {
    let ans = unsafe { execute_garbled_circuit!(FP_EQ(input) -> SecretI64) };
    ans
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn IEEE_f2i(input: SecretI64) -> SecretI64 {
    let ans = unsafe { execute_garbled_circuit!(FP_F2I(input) -> SecretI64) };
    ans
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn IEEE_i2f(input: SecretI64) -> SecretI64 {
    let ans = unsafe { execute_garbled_circuit!(FP_I2F(input) -> SecretI64) };
    ans
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn IEEE_sqrt(input: SecretI64) -> SecretI64 {
    let ans = unsafe { execute_garbled_circuit!(FP_SQRT(input) -> SecretI64) };
    ans
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn IEEE_lt(input: [SecretI64; 2]) -> SecretI64 {
    let ans = unsafe { execute_garbled_circuit!(FP_LT(input) -> SecretI64) };
    ans
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn IEEE_floor(input: SecretI64) -> SecretI64 {
    let ans = unsafe { execute_garbled_circuit!(FP_FLOOR(input) -> SecretI64) };
    ans
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn IEEE_ceil(input: SecretI64) -> SecretI64 {
    let ans = unsafe { execute_garbled_circuit!(FP_CEIL(input) -> SecretI64) };
    ans
}
