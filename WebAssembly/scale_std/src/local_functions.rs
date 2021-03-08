// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use crate::array::*;
use crate::matrix::Matrix;
use scale::*;

/**************************
 *  Local Linear Algebra  *
 **************************/

const C_MULT_C: u32 = 0;
const S_MULT_C: u32 = 1;
const C_MULT_S: u32 = 2;
const GAUSS_ELIM: u32 = 3;

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

const FP_ACOS: u32 = 200;
const FP_ASIN: u32 = 201;
const FP_ATAN: u32 = 202;
const FP_COS: u32 = 203;
const FP_COSH: u32 = 204;
const FP_SIN: u32 = 205;
const FP_SINH: u32 = 206;
const FP_TANH: u32 = 207;
const FP_EXP: u32 = 208;
const FP_LOG: u32 = 209;
const FP_LOG10: u32 = 210;
const FP_FABS: u32 = 211;

const TO_FIX: u32 = 500;
const FROM_FIX: u32 = 501;
const TO_FLOAT: u32 = 502;
const FROM_FLOAT: u32 = 503;

#[inline(always)]
#[allow(non_snake_case)]
#[allow(unused_variables)]
pub fn Matrix_Mul_CC<const N: u64, const L: u64, const M: u64>(
    A: &Matrix<ClearModp, N, L>,
    B: &Matrix<ClearModp, L, M>,
) -> Matrix<ClearModp, N, M> {
    let (col, row, C) = unsafe {
        execute_local_function!(C_MULT_C(
            N as i64,
            L as i64,
            *A,
            L as i64,
            M as i64,
            *B
        ) ->
            i64,
            i64,
            Matrix::<ClearModp, N, M>
        )
    };

    #[cfg(all(debug_assertions, not(feature = "emulate")))]
    if row != (N as i64) || col != (M as i64) {
        crash();
    }

    C
}

#[inline(always)]
#[allow(non_snake_case)]
#[allow(unused_variables)]
pub fn Matrix_Mul_SC<const N: u64, const L: u64, const M: u64>(
    A: &Matrix<SecretModp, N, L>,
    B: &Matrix<ClearModp, L, M>,
) -> Matrix<SecretModp, N, M> {
    let (col, row, C) = unsafe {
        execute_local_function!(S_MULT_C(
            N as i64,
            L as i64,
            *A,
            L as i64,
            M as i64,
            *B
        ) ->
            i64,
            i64,
            Matrix::<SecretModp, N, M>
        )
    };

    #[cfg(all(debug_assertions, not(feature = "emulate")))]
    if row != (N as i64) || col != (M as i64) {
        crash();
    }

    C
}

#[inline(always)]
#[allow(non_snake_case)]
#[allow(unused_variables)]
pub fn Matrix_Mul_CS<const N: u64, const L: u64, const M: u64>(
    A: &Matrix<ClearModp, N, L>,
    B: &Matrix<SecretModp, L, M>,
) -> Matrix<SecretModp, N, M> {
    let (col, row, C) = unsafe {
        execute_local_function!(C_MULT_S(
            N as i64,
            L as i64,
            *A,
            L as i64,
            M as i64,
            *B
        ) ->
            i64,
            i64,
            Matrix::<SecretModp, N, M>
        )
    };

    #[cfg(all(debug_assertions, not(feature = "emulate")))]
    if row != (N as i64) || col != (M as i64) {
        crash();
    }

    C
}

#[inline(always)]
#[allow(non_snake_case)]
#[allow(unused_variables)]
pub fn Gauss_Elim<const N: u64, const M: u64>(
    A: &Matrix<ClearModp, N, M>,
) -> Matrix<ClearModp, N, M> {
    let (col, row, B) = unsafe {
        execute_local_function!(GAUSS_ELIM(
            N as i64,
            M as i64,
            *A
        ) ->
            i64,
            i64,
            Matrix::<ClearModp, N, M>
        )
    };

    #[cfg(all(debug_assertions, not(feature = "emulate")))]
    if row != (N as i64) || col != (M as i64) {
        crash();
    }

    B
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn FP_add(a: i64, b: i64) -> i64 {
    let c = unsafe { execute_local_function!(FP_ADD(a, b) -> i64 ) };
    c
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn FP_mul(a: i64, b: i64) -> i64 {
    let c = unsafe { execute_local_function!(FP_MUL(a, b) -> i64 ) };
    c
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn FP_div(a: i64, b: i64) -> i64 {
    let c = unsafe { execute_local_function!(FP_DIV(a, b) -> i64 ) };
    c
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn FP_eq(a: i64, b: i64) -> i64 {
    let c = unsafe { execute_local_function!(FP_EQ(a, b) -> i64 ) };
    c
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn FP_f2i(a: i64) -> i64 {
    let c = unsafe { execute_local_function!(FP_F2I(a) -> i64 ) };
    c
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn FP_i2f(a: i64) -> i64 {
    let c = unsafe { execute_local_function!(FP_I2F(a) -> i64 ) };
    c
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn FP_sqrt(a: i64) -> i64 {
    let c = unsafe { execute_local_function!(FP_SQRT(a) -> i64 ) };
    c
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn FP_lt(a: i64, b: i64) -> i64 {
    let c = unsafe { execute_local_function!(FP_LT(a, b) -> i64 ) };
    c
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn FP_acos(a: i64) -> i64 {
    let c = unsafe { execute_local_function!(FP_ACOS(a) -> i64 ) };
    c
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn FP_asin(a: i64) -> i64 {
    let c = unsafe { execute_local_function!(FP_ASIN(a) -> i64 ) };
    c
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn FP_atan(a: i64) -> i64 {
    let c = unsafe { execute_local_function!(FP_ATAN(a) -> i64 ) };
    c
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn FP_cos(a: i64) -> i64 {
    let c = unsafe { execute_local_function!(FP_COS(a) -> i64 ) };
    c
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn FP_cosh(a: i64) -> i64 {
    let c = unsafe { execute_local_function!(FP_COSH(a) -> i64 ) };
    c
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn FP_sin(a: i64) -> i64 {
    let c = unsafe { execute_local_function!(FP_SIN(a) -> i64 ) };
    c
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn FP_sinh(a: i64) -> i64 {
    let c = unsafe { execute_local_function!(FP_SINH(a) -> i64 ) };
    c
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn FP_tanh(a: i64) -> i64 {
    let c = unsafe { execute_local_function!(FP_TANH(a) -> i64 ) };
    c
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn FP_exp(a: i64) -> i64 {
    let c = unsafe { execute_local_function!(FP_EXP(a) -> i64 ) };
    c
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn FP_log(a: i64) -> i64 {
    let c = unsafe { execute_local_function!(FP_LOG(a) -> i64 ) };
    c
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn FP_log10(a: i64) -> i64 {
    let c = unsafe { execute_local_function!(FP_LOG10(a) -> i64 ) };
    c
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn FP_ceil(a: i64) -> i64 {
    let c = unsafe { execute_local_function!(FP_CEIL(a) -> i64 ) };
    c
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn FP_fabs(a: i64) -> i64 {
    let c = unsafe { execute_local_function!(FP_FABS(a) -> i64 ) };
    c
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn FP_floor(a: i64) -> i64 {
    let c = unsafe { execute_local_function!(FP_FLOOR(a) -> i64 ) };
    c
}

// Conversion Routines

#[inline(always)]
#[allow(non_snake_case)]
pub fn to_fix(a: i64, k: i64, f: i64) -> ClearModp {
    let c = unsafe { execute_local_function!(TO_FIX(a, k, f) -> ClearModp ) };
    c
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn from_fix(x: ClearModp, k: i64, f: i64) -> i64 {
    let c = unsafe { execute_local_function!(FROM_FIX(x, k, f) -> i64 ) };
    c
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn to_float(a: i64, vlen: i64, plen: i64) -> Array<ClearModp, 5> {
    let x = unsafe { execute_local_function!(TO_FLOAT(a, vlen, plen) -> Array<ClearModp, 5>) };
    let mut ans: Array<ClearModp, 5> = Array::uninitialized();
    ans.set(0, &x.get_unchecked(4));
    ans.set(1, &x.get_unchecked(3));
    ans.set(2, &x.get_unchecked(2));
    ans.set(3, &x.get_unchecked(1));
    ans.set(4, &x.get_unchecked(0));
    ans
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn from_float(v: ClearModp, p: ClearModp, z: ClearModp, s: ClearModp, err: ClearModp) -> i64 {
    let c = unsafe { execute_local_function!(FROM_FLOAT(v, p, z, s, err) -> i64 ) };
    c
}
