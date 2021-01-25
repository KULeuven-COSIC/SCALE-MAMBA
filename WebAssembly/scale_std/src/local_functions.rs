use crate::matrix::Matrix;
use scale::*;

/**************************
 *  Local Linear Algebra  *
 **************************/

const C_MULT_C: u32 = 0;
const S_MULT_C: u32 = 1;
const C_MULT_S: u32 = 2;
const GAUSS_ELIM: u32 = 3;

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
