// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use crate::array::*;
use crate::bit_protocols::*;
use crate::circuits::*;
use crate::fixed_point::*;
use crate::float_subroutines::*;
use crate::floating_point::*;
use crate::ieee::*;
use crate::integer::*;
use crate::local_functions::*;
use crate::math_generic::*;
use crate::slice::*;
use core::ops::{Add, Div, Mul};
use scale::alloc::GetAllocator;
use scale::*;

/********************************/
/*             Traits           */
/********************************/

pub trait Floor {
    fn floor(self) -> Self;
}

pub trait FAbs {
    fn fabs(self) -> Self;
}

pub trait Sqrt {
    fn sqrt(self) -> Self;
}

pub trait Constants
where
    Self: From<f64>,
{
    #[inline(always)]
    fn two_pi() -> Self {
        Self::from(6.2831853071795864769252867665590057684)
    }
    #[inline(always)]
    fn pi() -> Self {
        Self::from(3.1415926535897932384626433832795028842)
    }
    #[inline(always)]
    fn half_pi() -> Self {
        Self::from(1.5707963267948966192313216916397514421)
    }
    #[inline(always)]
    fn ln2() -> Self {
        Self::from(0.69314718055994530941723212145817656807)
    }
    #[inline(always)]
    fn e() -> Self {
        Self::from(2.7182818284590452353602874713526624978)
    }
}

impl Constants for ClearIEEE {}
impl Constants for SecretIEEE {}
impl<const K: u64, const F: u64> Constants for ClearFixed<K, F> {}
impl<const K: u64, const F: u64, const KAPPA: u64> Constants for SecretFixed<K, F, KAPPA> {}

impl<const V: u64, const P: u64, const DETECT_OVERFLOW: bool> Constants
    for ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ V - 1 }>: ,
{
}

impl<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool> Constants
    for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ V - 1 }>: ,
{
}

/* Make some supertraits to help in where clauses */

pub trait Float:
    Copy
    + From<i64>
    + From<f64>
    + Floor
    + FAbs
    + Sqrt
    + Constants
    + LoadFromMem<i64>
    + StoreInMem<i64>
    + GetAllocator
{
}

impl Float for ClearIEEE {}

impl Float for SecretIEEE {}

impl<const K: u64, const F: u64> Float for ClearFixed<K, F>
where
    ConstU64<{ K + 1 }>: ,
    ConstU64<{ K - 1 }>: ,
    ConstU64<{ 2 * F }>: ,
    ConstU64<{ 2 * K }>: ,
    ConstI32<{ f_as_i32(F) }>: ,
    ConstI32<{ f_as_i32(K) }>: ,
    ConstU64<{ ClearFixed::<K, F>::THETA }>: ,
{
}

impl<const K: u64, const F: u64, const KAPPA: u64> Float for SecretFixed<K, F, KAPPA>
where
    ConstU64<{ K + 1 }>: ,
    ConstU64<{ K - 1 }>: ,
    ConstU64<{ 2 * K }>: ,
    ConstU64<{ 2 * F }>: ,
    ConstI32<{ f_as_i32(F) }>: ,
    ConstI32<{ f_as_i32(K) }>: ,
    ConstU64<{ SecretFixed::<K, F, KAPPA>::THETA }>: ,
{
}

/* Trait with No Copy for use with Clear/Secret Float types */
pub trait FloatNC:
    From<i64>
    + From<f64>
    + Floor
    + FAbs
    + Sqrt
    + Constants
    + LoadFromMem<i64>
    + StoreInMem<i64>
    + GetAllocator
{
}

impl<const V: u64, const P: u64, const DETECT_OVERFLOW: bool> FloatNC
    for ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ V - 1 }>: ,
    ConstU64<{ V + 2 }>: ,
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ 2 * V + 1 }>: ,
    ConstU64<{ ClearFloat::<V, P, DETECT_OVERFLOW>::THETA }>: ,
{
}

impl<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool> FloatNC
    for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ V - 1 }>: ,
    ConstU64<{ V + 2 }>: ,
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ 2 * V + 1 }>: ,
    ConstU64<{ SecretFloat::<V, P, KAPPA, DETECT_OVERFLOW>::THETA }>: ,
{
}

/* Define NaN for IEEE types */

impl ClearIEEE {
    #[allow(non_snake_case)]
    pub fn NaN() -> Self {
        ClearIEEE::set(9218868437227405312_i64)
    }
}

impl SecretIEEE {
    #[allow(non_snake_case)]
    pub fn NaN() -> Self {
        SecretIEEE::set(SecretI64::from(9218868437227405312_i64))
    }
}

/* Polynomial/Rational Function Evaluation */

// Evaluate polynomial using Horner's Rule
pub fn poly_eval<S, C, const N: u64>(poly: Array<C, N>, x: S) -> S
where
    S: Float,
    S: Add<C, Output = S>,
    S: Mul<S, Output = S>,
    C: Float,
{
    let mut sum: S = S::from(0_i64);
    for i in 0..N {
        sum = sum * x + *poly.get_unchecked(N - 1 - i);
    }
    sum
}


/*
// Evaluate polynomial using Horner's Rule
pub fn poly_eval_NC<'a, S, C, const N: u64>(poly: Array<C, N>, x: &'a S) -> S
where
    S: FloatNC,
    S: Add<&'a C, Output = S>,
    S: Mul<&'a S, Output = S>,
    C: FloatNC,
{
    let mut sum: S = S::from(0_i64);
    for i in 0..N {
        sum = (sum * &x) + &*poly.get_unchecked(N - 1 - i);
    }
    sum
}
*/


// Evaluate Pade approximation
//   P and Q must have same degree
#[allow(non_snake_case)]
pub fn Pade<S, C, const N: u64>(poly_p: Array<C, N>, poly_q: Array<C, N>, x: S) -> S
where
    S: Float,
    S: Add<S, Output = S>,
    S: Mul<S, Output = S>,
    S: Div<S, Output = S>,
    S: Mul<C, Output = S>,
    S: From<C>,
    C: Float,
{
    let mut num: S = S::from(*poly_p.get_unchecked(0));
    let mut den: S = S::from(*poly_q.get_unchecked(0));
    let mut x_pow = x;
    for i in 1..N {
        num = num + x_pow * *poly_p.get_unchecked(i);
        den = den + x_pow * *poly_q.get_unchecked(i);
        x_pow = x_pow * x;
    }
    num / den
}

/********************************/
/*           ClearIEEE          */
/********************************/

impl ClearIEEE {
    #[inline(always)]
    pub fn acos(self) -> ClearIEEE {
        ClearIEEE::set(FP_acos(self.rep()))
    }
    #[inline(always)]
    pub fn asin(self) -> ClearIEEE {
        ClearIEEE::set(FP_asin(self.rep()))
    }
    #[inline(always)]
    pub fn atan(self) -> ClearIEEE {
        ClearIEEE::set(FP_atan(self.rep()))
    }
    #[inline(always)]
    pub fn cos(self) -> ClearIEEE {
        ClearIEEE::set(FP_cos(self.rep()))
    }
    #[inline(always)]
    pub fn cosh(self) -> ClearIEEE {
        ClearIEEE::set(FP_cosh(self.rep()))
    }
    #[inline(always)]
    pub fn sin(self) -> ClearIEEE {
        ClearIEEE::set(FP_sin(self.rep()))
    }
    #[inline(always)]
    pub fn sinh(self) -> ClearIEEE {
        ClearIEEE::set(FP_sinh(self.rep()))
    }
    #[inline(always)]
    pub fn tan(self) -> ClearIEEE {
        let num = self.sin();
        let cos = self.cos();
        num / cos
    }
    #[inline(always)]
    pub fn tanh(self) -> ClearIEEE {
        ClearIEEE::set(FP_tanh(self.rep()))
    }
    #[inline(always)]
    pub fn exp(self) -> ClearIEEE {
        ClearIEEE::set(FP_exp(self.rep()))
    }
    #[inline(always)]
    pub fn log(self) -> ClearIEEE {
        ClearIEEE::set(FP_log(self.rep()))
    }
    #[inline(always)]
    pub fn log2(self) -> ClearIEEE {
        let v = ClearIEEE::set(FP_log(self.rep()));
        v / ClearIEEE::ln2()
    }
    #[inline(always)]
    pub fn log10(self) -> ClearIEEE {
        ClearIEEE::set(FP_log10(self.rep()))
    }
    #[inline(always)]
    pub fn ceil(self) -> ClearIEEE {
        ClearIEEE::set(FP_ceil(self.rep()))
    }
    #[inline(always)]
    pub fn exp2(self) -> ClearIEEE {
        let y = self * ClearIEEE::ln2();
        y.exp()
    }
}

impl Floor for ClearIEEE {
    #[inline(always)]
    fn floor(self) -> Self {
        ClearIEEE::set(FP_floor(self.rep()))
    }
}

impl FAbs for ClearIEEE {
    #[inline(always)]
    fn fabs(self) -> Self {
        ClearIEEE::set(FP_fabs(self.rep()))
    }
}

impl Sqrt for ClearIEEE {
    #[inline(always)]
    fn sqrt(self) -> ClearIEEE {
        ClearIEEE::set(FP_sqrt(self.rep()))
    }
}

/********************************/
/*           SecretIEEE         */
/********************************/

impl FAbs for SecretIEEE {
    #[inline(always)]
    fn fabs(self) -> SecretIEEE {
        let a = self.rep().set_bit(SecretBit::from(false), ConstU32::<63>);
        SecretIEEE::set(a)
    }
}

impl Floor for SecretIEEE {
    #[inline(always)]
    fn floor(self) -> Self {
        SecretIEEE::set(IEEE_floor(self.rep()))
    }
}

impl Sqrt for SecretIEEE {
    #[inline(always)]
    fn sqrt(self) -> SecretIEEE {
        SecretIEEE::set(IEEE_sqrt(self.rep()))
    }
}

impl SecretIEEE {
    #[inline(always)]
    pub fn acos(self) -> SecretIEEE {
        kernel_acos::<SecretIEEE, ClearIEEE, SecretBit>(self)
    }
    #[inline(always)]
    pub fn asin(self) -> SecretIEEE {
        kernel_asin::<SecretIEEE, ClearIEEE, SecretBit>(self)
    }
    #[inline(always)]
    pub fn atan(self) -> SecretIEEE {
        kernel_atan::<SecretIEEE, ClearIEEE, SecretBit>(self)
    }
    #[inline(always)]
    pub fn cos(self) -> SecretIEEE {
        let ts = TrigSubroutine::<SecretIEEE, ClearIEEE, SecretBit>(self);
        kernel_cos::<SecretIEEE, ClearIEEE>(*ts.get_unchecked(0), *ts.get_unchecked(2))
    }
    #[inline(always)]
    pub fn cosh(self) -> SecretIEEE {
        let e2 = self.exp_2();
        let ans = (*e2.get_unchecked(0) + *e2.get_unchecked(1)) * ClearIEEE::from(0.5);
        ans
    }
    #[inline(always)]
    pub fn sin(self) -> SecretIEEE {
        let ts = TrigSubroutine::<SecretIEEE, ClearIEEE, SecretBit>(self);
        kernel_sin::<SecretIEEE, ClearIEEE>(*ts.get_unchecked(0), *ts.get_unchecked(1))
    }
    #[inline(always)]
    pub fn sinh(self) -> SecretIEEE {
        let e2 = self.exp_2();
        let ans = (*e2.get_unchecked(0) - *e2.get_unchecked(1)) * ClearIEEE::from(0.5);
        ans
    }
    #[inline(always)]
    pub fn tan(self) -> SecretIEEE {
        let ts = TrigSubroutine::<SecretIEEE, ClearIEEE, SecretBit>(self);
        let ans1 = kernel_sin::<SecretIEEE, ClearIEEE>(*ts.get_unchecked(0), *ts.get_unchecked(1));
        let ans2 = kernel_cos::<SecretIEEE, ClearIEEE>(*ts.get_unchecked(0), *ts.get_unchecked(2));
        ans1 / ans2
    }
    #[inline(always)]
    pub fn tanh(self) -> SecretIEEE {
        let e2 = self.exp_2();
        let ans = (*e2.get_unchecked(0) - *e2.get_unchecked(1))
            / (*e2.get_unchecked(0) + *e2.get_unchecked(1));
        ans
    }
    #[inline(always)]
    pub fn exp(self) -> SecretIEEE {
        let log2 = ClearIEEE::ln2();
        let ans = (self / log2).exp2();
        ans
    }
    #[inline(always)]
    pub fn log(self) -> SecretIEEE {
        let z = self.log2();
        z * ClearIEEE::ln2()
    }
    #[inline(always)]
    pub fn log10(self) -> SecretIEEE {
        let z = self.log2();
        z * ClearIEEE::from(0.30102999566398119521373889472449302677)
    }
    #[inline(always)]
    pub fn log2(self) -> SecretIEEE {
        let mut y = self.rep();
        // Extract exponent and normalize it
        let ex = (y >> ConstU32::<52>) - i64::from(1022);
        // Set the exponent in y to 1022
        let one = SecretBit::from(true);
        let zero = SecretBit::from(false);
        y = y.set_bit(zero, ConstU32::<62>);
        y = y.set_bit(one, ConstU32::<61>);
        y = y.set_bit(one, ConstU32::<60>);
        y = y.set_bit(one, ConstU32::<59>);
        y = y.set_bit(one, ConstU32::<58>);
        y = y.set_bit(one, ConstU32::<57>);
        y = y.set_bit(one, ConstU32::<56>);
        y = y.set_bit(one, ConstU32::<55>);
        y = y.set_bit(one, ConstU32::<54>);
        y = y.set_bit(one, ConstU32::<53>);
        y = y.set_bit(zero, ConstU32::<52>);
        // Now form the log2
        let z = SecretIEEE::set(y);
        let mut ans = kernel_log2::<SecretIEEE, ClearIEEE>(z);
        ans = ans + SecretIEEE::from(ex);
        ans
    }
    #[inline(always)]
    pub fn ceil(self) -> SecretIEEE {
        SecretIEEE::set(IEEE_ceil(self.rep()))
    }
    #[inline(always)]
    pub fn exp2(self) -> SecretIEEE {
        let s_t = self.ltz();
        let s: SecretIEEE = SecretIEEE::from(s_t);
        let g = kernel_exp2::<SecretIEEE, ClearIEEE, SecretBit>(self);
        let ans = (ClearIEEE::from(1_i64) - s) * g + s / g;
        ans
    }
    #[inline(always)]
    pub fn exp_2(self) -> Array<SecretIEEE, 2> {
        let log2 = ClearIEEE::ln2();
        let ans1 = (self / log2).exp2();
        let ans2 = ClearIEEE::from(1_i64) / ans1;
        let mut ans: Array<SecretIEEE, 2> = Array::uninitialized();
        ans.set(0, &ans1);
        ans.set(1, &ans2);
        ans
    }
}

/********************************/
/*           ClearFixed         */
/********************************/

impl<const K: u64, const F: u64> FAbs for ClearFixed<K, F> {
    fn fabs(self) -> ClearFixed<K, F> {
        let s = self.rep().ltz();
        let v = (ClearModp::from(1) - s - s) * self.rep().rep();
        let u: ClearInteger<K> = ClearInteger::from(v);
        ClearFixed::set(u)
    }
}

impl<const K: u64, const F: u64> Floor for ClearFixed<K, F> {
    fn floor(self) -> Self {
        let v = self.rep().Trunc(K - F, true);
        ClearFixed::from(v)
    }
}

impl<const K: u64, const F: u64> ClearFixed<K, F>
where
    ConstU64<{ F + 1 }>: ,
    ConstU64<{ F - 1 }>: ,
    ConstU64<{ 2 * F }>: ,
    ConstU64<{ K + 1 }>: ,
    ConstU64<{ K - 1 }>: ,
    ConstU64<{ 2 * K }>: ,
    ConstU64<{ 2 * (K - F) }>: ,
    ConstI32<{ f_as_i32(F) }>: ,
    ConstI32<{ f_as_i32(K) }>: ,
    ConstU64<{ ClearFixed::<K, F>::THETA }>: ,
    ConstU64<{ K - F - 1 }>: ,
{
    pub fn ceil(self) -> Self {
        let f = self.floor();
        let a = self - f;
        let b = a.gtz();
        let ans = f + ClearFixed::from(1_i64) * ClearFixed::from(b);
        ans
    }
    pub fn sin(self) -> Self {
        let ts = TrigSubroutine::<ClearFixed<K, F>, ClearFixed<K, F>, ClearModp>(self);
        let ans = kernel_sin::<ClearFixed<K, F>, ClearFixed<K, F>>(
            *ts.get_unchecked(0),
            *ts.get_unchecked(1),
        );
        ans
    }
    pub fn cos(self) -> Self {
        let ts = TrigSubroutine::<ClearFixed<K, F>, ClearFixed<K, F>, ClearModp>(self);
        let ans = kernel_cos::<ClearFixed<K, F>, ClearFixed<K, F>>(
            *ts.get_unchecked(0),
            *ts.get_unchecked(2),
        );
        ans
    }
    pub fn tan(self) -> Self {
        let ts = TrigSubroutine::<ClearFixed<K, F>, ClearFixed<K, F>, ClearModp>(self);
        let ans1 = kernel_sin::<ClearFixed<K, F>, ClearFixed<K, F>>(
            *ts.get_unchecked(0),
            *ts.get_unchecked(1),
        );
        let ans2 = kernel_cos::<ClearFixed<K, F>, ClearFixed<K, F>>(
            *ts.get_unchecked(0),
            *ts.get_unchecked(2),
        );
        ans1 / ans2
    }
    pub fn asin(self) -> Self {
        let ans = kernel_asin::<ClearFixed<K, F>, ClearFixed<K, F>, ClearModp>(self);
        ans
    }
    pub fn acos(self) -> Self {
        let ans = kernel_acos::<ClearFixed<K, F>, ClearFixed<K, F>, ClearModp>(self);
        ans
    }
    pub fn atan(self) -> Self {
        let ans = kernel_atan::<ClearFixed<K, F>, ClearFixed<K, F>, ClearModp>(self);
        ans
    }

    // Computes 2^x
    pub fn exp2(self) -> Self {
        let s_t = self.ltz();
        let s: ClearFixed<K, F> = ClearFixed::from(s_t);
        let g = kernel_exp2::<ClearFixed<K, F>, ClearFixed<K, F>, ClearModp>(self);
        let ans = (ClearFixed::from(1_i64) - s) * g + s / g;
        ans
    }

    // Computes exp(x)
    pub fn exp(self) -> Self {
        let log2 = ClearFixed::ln2();
        let ans = (self / log2).exp2();
        ans
    }

    // Computes exp(x) and exp(-x)
    fn exp_2(self) -> Array<Self, 2> {
        let log2 = ClearFixed::ln2();
        let ans1 = (self / log2).exp2();
        let ans2 = ClearFixed::from(1_i64) / ans1;
        let mut ans: Array<Self, 2> = Array::uninitialized();
        ans.set(0, &ans1);
        ans.set(1, &ans2);
        ans
    }

    // Computes sinh(x)
    pub fn sinh(self) -> Self {
        let e2 = self.exp_2();
        let ans = (*e2.get_unchecked(0) - *e2.get_unchecked(1)) * ClearFixed::from(0.5);
        ans
    }

    // Computes cosh(x)
    pub fn cosh(self) -> Self {
        let e2 = self.exp_2();
        let ans = (*e2.get_unchecked(0) + *e2.get_unchecked(1)) * ClearFixed::from(0.5);
        ans
    }

    // Computes tanh(x)
    pub fn tanh(self) -> Self {
        let e2 = self.exp_2();
        let ans = (*e2.get_unchecked(0) - *e2.get_unchecked(1))
            / (*e2.get_unchecked(0) + *e2.get_unchecked(1));
        ans
    }

    // Computes log2(x)
    pub fn log2(self) -> Self {
        let fl: ClearFloat<F, F, true> = ClearFloat::from(self);
        let vv: ClearFixed<K, F> = ClearFixed::set(ClearInteger::set(fl.v()));
        let mut a = kernel_log2::<ClearFixed<K, F>, ClearFixed<K, F>>(vv);
        let pp: ClearFixed<K, F> = ClearFixed::from(fl.p() + ClearModp::from(F as i64));
        a = a + pp;
        let mask = (ClearModp::from(1) - fl.z()) * (ClearModp::from(1) - fl.s());
        let t = a.rep().rep() * mask;
        ClearFixed::set(ClearInteger::set(t))
    }

    // Computes log(x)
    pub fn log(self) -> Self {
        let v = self.log2() * ClearFixed::ln2();
        v
    }

    // Computes log10(x)
    pub fn log10(self) -> Self {
        let v = self.log2() * ClearFixed::from(0.30102999566398119521373889472449302677);
        v
    }
}

/********************************/
/*           SecretFixed        */
/********************************/

impl<const K: u64, const F: u64, const KAPPA: u64> FAbs for SecretFixed<K, F, KAPPA>
where
    ConstU64<{ K + 1 }>: ,
    ConstU64<{ K - 1 }>: ,
{
    fn fabs(self) -> SecretFixed<K, F, KAPPA> {
        let s = self.rep().ltz();
        let v = (ClearModp::from(1) - s - s) * self.rep().rep();
        let u: SecretInteger<K, KAPPA> = SecretInteger::from(v);
        SecretFixed::set(u)
    }
}

impl<const K: u64, const F: u64, const KAPPA: u64> Floor for SecretFixed<K, F, KAPPA> {
    fn floor(self) -> Self {
        let v = self.rep().Trunc(K - F, true);
        SecretFixed::from(v)
    }
}

impl<const K: u64, const F: u64, const KAPPA: u64> SecretFixed<K, F, KAPPA>
where
    ConstU64<{ 2 * K }>: ,
    ConstU64<{ 2 * F }>: ,
    ConstU64<{ 2 * (K - F) }>: ,
    ConstU64<{ K - 1 }>: ,
    ConstU64<{ K + 1 }>: ,
    ConstU64<{ F + 1 }>: ,
    ConstU64<{ F - 1 }>: ,
    ConstI32<{ f_as_i32(F) }>: ,
    ConstI32<{ f_as_i32(K) }>: ,
    ConstU64<{ ClearFixed::<K, F>::THETA }>: ,
    ConstU64<{ SecretFixed::<K, F, KAPPA>::THETA }>: ,
    ConstU64<{ K - F - 1 }>: ,
{
    pub fn ceil(self) -> Self {
        let f = self.floor();
        let a = self - f;
        let b = a.gtz();
        let ans = f + ClearFixed::from(1_i64) * SecretFixed::from(b);
        ans
    }
    pub fn sin(self) -> Self {
        let ts = TrigSubroutine::<SecretFixed<K, F, KAPPA>, ClearFixed<K, F>, SecretModp>(self);
        let ans = kernel_sin::<SecretFixed<K, F, KAPPA>, ClearFixed<K, F>>(
            *ts.get_unchecked(0),
            *ts.get_unchecked(1),
        );
        ans
    }
    pub fn cos(self) -> Self {
        let ts = TrigSubroutine::<SecretFixed<K, F, KAPPA>, ClearFixed<K, F>, SecretModp>(self);
        let ans = kernel_cos::<SecretFixed<K, F, KAPPA>, ClearFixed<K, F>>(
            *ts.get_unchecked(0),
            *ts.get_unchecked(2),
        );
        ans
    }
    pub fn tan(self) -> Self {
        let ts = TrigSubroutine::<SecretFixed<K, F, KAPPA>, ClearFixed<K, F>, SecretModp>(self);
        let ans1 = kernel_sin::<SecretFixed<K, F, KAPPA>, ClearFixed<K, F>>(
            *ts.get_unchecked(0),
            *ts.get_unchecked(1),
        );
        let ans2 = kernel_cos::<SecretFixed<K, F, KAPPA>, ClearFixed<K, F>>(
            *ts.get_unchecked(0),
            *ts.get_unchecked(2),
        );
        ans1 / ans2
    }
    // For some reason there is a bug if this is not inline(always)
    #[inline(always)]
    pub fn asin(self) -> Self {
        let ans = kernel_asin::<SecretFixed<K, F, KAPPA>, ClearFixed<K, F>, SecretModp>(self);
        ans
    }
    // For some reason there is a bug if this is not inline(always)
    #[inline(always)]
    pub fn acos(self) -> Self {
        let ans = kernel_acos::<SecretFixed<K, F, KAPPA>, ClearFixed<K, F>, SecretModp>(self);
        ans
    }
    pub fn atan(self) -> Self {
        let ans = kernel_atan::<SecretFixed<K, F, KAPPA>, ClearFixed<K, F>, SecretModp>(self);
        ans
    }

    // Computes 2^x
    pub fn exp2(self) -> Self {
        let s_t = self.ltz();
        let s: SecretFixed<K, F, KAPPA> = SecretFixed::from(s_t);
        let g = kernel_exp2::<SecretFixed<K, F, KAPPA>, ClearFixed<K, F>, SecretModp>(self);
        let ans = (ClearFixed::from(1_i64) - s) * g + s / g;
        ans
    }

    // Computes exp(x)
    pub fn exp(self) -> Self {
        let log2 = ClearFixed::ln2();
        let ans = (self / log2).exp2();
        ans
    }

    // Computes exp(x) and exp(-x)
    fn exp_2(self) -> Array<Self, 2> {
        let log2 = ClearFixed::ln2();
        let ans1 = (self / log2).exp2();
        let ans2 = ClearFixed::from(1_i64) / ans1;
        let mut ans: Array<Self, 2> = Array::uninitialized();
        ans.set(0, &ans1);
        ans.set(1, &ans2);
        ans
    }

    // Computes sinh(x)
    pub fn sinh(self) -> Self {
        let e2 = self.exp_2();
        let ans = (*e2.get_unchecked(0) - *e2.get_unchecked(1)) * ClearFixed::from(0.5);
        ans
    }

    // Computes cosh(x)
    pub fn cosh(self) -> Self {
        let e2 = self.exp_2();
        let ans = (*e2.get_unchecked(0) + *e2.get_unchecked(1)) * ClearFixed::from(0.5);
        ans
    }

    // Computes tanh(x)
    pub fn tanh(self) -> SecretFixed<K, F, KAPPA> {
        let e2 = self.exp_2();
        let ans = (*e2.get_unchecked(0) - *e2.get_unchecked(1))
            / (*e2.get_unchecked(0) + *e2.get_unchecked(1));
        ans
    }

    // Computes log2(x)
    pub fn log2(self) -> Self {
        // Here we can set DETECT_OVERFLOW to false, as we only use fl for
        // access, and no arithmetic.
        let fl: SecretFloat<F, F, KAPPA, false> = SecretFloat::from(self);
        let vv: SecretFixed<K, F, KAPPA> = SecretFixed::set(SecretInteger::set(fl.v()));
        let mut a = kernel_log2::<SecretFixed<K, F, KAPPA>, ClearFixed<K, F>>(vv);
        let pp: SecretFixed<K, F, KAPPA> = SecretFixed::from(fl.p() + ClearModp::from(F as i64));
        a = a + pp;
        let mask = (ClearModp::from(1) - fl.z()) * (ClearModp::from(1) - fl.s());
        let t = a.rep().rep() * mask;
        SecretFixed::set(SecretInteger::set(t))
    }

    // Computes log(x)
    pub fn log(self) -> Self {
        let v = self.log2() * ClearFixed::ln2();
        v
    }

    // Computes log10(x)
    pub fn log10(self) -> Self {
        let v = self.log2() * ClearFixed::from(0.30102999566398119521373889472449302677);
        v
    }
}

/********************************/
/*           ClearFloat        */
/********************************/

impl<const V: u64, const P: u64, const DETECT_OVERFLOW: bool> FAbs
    for ClearFloat<V, P, DETECT_OVERFLOW>
{
    #[inline(always)]
    fn fabs(self) -> ClearFloat<V, P, DETECT_OVERFLOW> {
        let mut param_new: Array<ClearModp, 5> = Array::uninitialized();
        param_new.set(0, &self.v());
        param_new.set(1, &self.p());
        param_new.set(2, &self.z());
        param_new.set(3, &(ClearModp::from(0)));
        param_new.set(4, &self.err());
        ClearFloat::set(param_new)
    }
}

impl<const V: u64, const P: u64, const DETECT_OVERFLOW: bool> ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ V - 1 }>: ,
    ConstU64<{ V + 2 }>: ,
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ 2 * V + 1 }>: ,
{
    // Rounding with FP output
    // 0 -> floor, 1 -> ceil, -1 -> trunc
    #[allow(non_snake_case)]
    fn FLRound(self, mode: i64) -> Self {
        let p_int: ClearInteger<V> = ClearInteger::from(self.p());
        // let a = p_int.ltz(); Causes a problem so expanded it here...
        let a = -p_int.Trunc(V - 1, true).rep();
        let p2_int: ClearInteger<V> = ClearInteger::from(self.p() + ClearModp::from(V as i64 - 1));
        let b = -p2_int.Trunc(V - 1, true).rep();
        let v_int: ClearInteger<V> = ClearInteger::from(self.v());
        let m = -a * (ClearModp::from(1) - b) * self.p();

        let mi64: i64 = i64::from(m);
        let inv_2pow_p1 = modp_two_power(mi64 as u64);
        let v2_int_trunc = v_int.Trunc(mi64 as u64, true);
        let v2_int = self.v() - v2_int_trunc.rep() * inv_2pow_p1;
        let v2_int: ClearInteger<V> = ClearInteger::from(v2_int);
        let c = v2_int.eqz();
        let mut away_from_zero = ClearModp::from(0);
        let mut modec = ClearModp::from(mode);
        if mode == -1 {
            modec = self.s();
        } else {
            away_from_zero = ClearModp::from(mode) + self.s() * ClearModp::from(1 - 2 * mode);
        }
        let mut v =
            self.v() - v2_int.rep() + (ClearModp::from(1) - c) * inv_2pow_p1 * away_from_zero;
        let twopower1 = modp_two_power(V - 1);
        let twopower = twopower1 + twopower1;
        let v_int: ClearInteger<{ V + 1 }> = ClearInteger::from(v - twopower);
        let d = v_int.eqz();
        v = d * twopower1 + (ClearModp::from(1) - d) * v;
        v = a * ((ClearModp::from(1) - b) * v + b * away_from_zero * twopower1)
            + (ClearModp::from(1) - a) * self.v();
        let s = (ClearModp::from(1) - b * modec) * self.s();
        let v_int: ClearInteger<V> = ClearInteger::from(v);
        let v_zero = v_int.eqz();
        let z = v_zero + self.z() - v_zero * self.z(); // z = Or(v_zero, z1)
        v = v * (ClearModp::from(1) - z);
        let p = ((self.p() + d * a) * (ClearModp::from(1) - b)
            + b * away_from_zero * ClearModp::from(1 - V as i64))
            * (ClearModp::from(1) - z);
        let mut ans: Array<ClearModp, 5> = Array::uninitialized();
        ans.set(0, &v);
        ans.set(1, &p);
        ans.set(2, &z);
        ans.set(3, &s);
        ans.set(4, &self.err());

        let answer: ClearFloat<V, P, DETECT_OVERFLOW> = ClearFloat::set(ans);
        answer
    }

    #[allow(non_snake_case)]
    pub fn log2(self) -> Self {
        let zero = ClearModp::from(0);
        let one = ClearModp::from(1);
        let one_float: ClearFloat<V, P, DETECT_OVERFLOW> = ClearFloat::from(1.0);

        let mut temp: Array<ClearModp, 5> = Array::uninitialized();
        temp.set(0, &self.v());
        temp.set(1, &ClearModp::from(-(V as i64)));
        temp.set(2, &zero);
        temp.set(3, &zero);
        temp.set(4, &zero);
        let vv: ClearFloat<V, P, DETECT_OVERFLOW> = ClearFloat::set(temp);

        // 2*log_2 e
        let tlog2e_ieee: ClearIEEE = ClearIEEE::from(2.8853900817779268147198493620037842749);
        let tlog2e: ClearFloat<V, P, DETECT_OVERFLOW> = ClearFloat::from(tlog2e_ieee);

        let mut v2 = &one_float - &vv;
        let v3 = &one_float + &vv;
        let mut vy = &v2 / &v3;
        let vy2 = &vy * &vy;
        let mut vv = &vy * &tlog2e;

        // Here we approximate M = ceil(V/(2*log2(3)) - 1/2) by V/3
        for i in 1..V / 3 {
            let den: ClearIEEE = ClearIEEE::from(2 * i as i64 + 1);
            let temp = tlog2e_ieee / den;
            let temp: ClearFloat<V, P, DETECT_OVERFLOW> = ClearFloat::from(temp);
            vy = &vy * &vy2;
            v2 = &vy * &temp;
            vv = &vv + &v2;
        }

        let temp: ClearInteger<V> = ClearInteger::from(ClearModp::from(V as i64) + vv.p());
        let v2 = Clear_Int2Fl(temp, ConstU64::<V>);
        let v2: ClearFloat<V, P, DETECT_OVERFLOW> = ClearFloat::set(v2);

        vv = &v2 - &vv;

        let t_int: ClearInteger<P> = ClearInteger::from(self.p() + ClearModp::from(V as i64 - 1));
        let a = t_int.eqz();

        let ctLm1 = modp_two_power(V - 1);
        let t_int: ClearInteger<V> = ClearInteger::from(self.v() - ctLm1);
        let b = t_int.eqz();

        let z = a * b;
        let v = vv.v() * (one - z);
        let p = vv.p() * (one - z);
        let err = self.err() + self.z() + self.s();

        let mut temp: Array<ClearModp, 5> = Array::uninitialized();
        temp.set(0, &v);
        temp.set(1, &p);
        temp.set(2, &z);
        temp.set(3, &vv.s());
        temp.set(4, &err);

        let ans: ClearFloat<V, P, DETECT_OVERFLOW> = ClearFloat::set(temp);
        ans
    }

    #[inline(always)]
    pub fn log(self) -> Self {
        let z = self.log2();
        z * ClearFloat::<V, P, DETECT_OVERFLOW>::ln2()
    }
    #[inline(always)]
    pub fn log10(self) -> Self {
        let z = self.log2();
        z * ClearFloat::<V, P, DETECT_OVERFLOW>::from(0.30102999566398119521373889472449302677)
    }

    #[inline(always)]
    pub fn ceil(self) -> Self {
        self.FLRound(1)
    }

    // Assumes V < 2^{P-1}
    #[allow(non_snake_case)]
    pub fn exp2(self) -> Self {
        // If V < 2^{P-1} then we have
        // max = ceil( log_2(2^{P-1}-1+V) - V + 1)
        //     = P-V+1
        let max = ClearModp::from(P as i64 - V as i64 + 1);
        let one = ClearModp::from(1);
        let zero = ClearModp::from(0);
        let twokm1 = modp_two_power(P - 1);
        let twolm1 = modp_two_power(V - 1);
        let vm1 = ClearModp::from(V as i64 - 1);
        let twol = twolm1 + twolm1;
        let a_int: ClearInteger<P> = ClearInteger::from(self.p() - max);
        let b_int: ClearInteger<P> = ClearInteger::from(self.p() + vm1);
        let c_int: ClearInteger<P> =
            ClearInteger::from(self.p() + ClearModp::from(2 * V as i64 - 1));
        let a = a_int.ltz();
        let b = b_int.ltz();
        let c = c_int.ltz();
        let p2 =
            -a * (one - c) * (b * (self.p() + ClearModp::from(V as i64)) + (one - b) * self.p());
        let v_int: ClearInteger<V> = ClearInteger::from(self.v());
        let p2_i64: i64 = i64::from(p2);
        let tpow = modp_two_power(p2_i64 as u64);
        let x_int = v_int.Trunc(p2_i64 as u64, true);
        let x = x_int.rep();
        let y = self.v() - tpow * x;
        let y_int: ClearInteger<V> = ClearInteger::from(y);
        let d = y_int.eqz();
        let x = (one - b * self.s()) * (x - (one - d) * self.s())
            + b * self.s() * (twol - ClearModp::from(1) + d - x);
        let y = (one - d) * self.s() * (tpow - y) + (one - self.s()) * y;
        let w = a * (one - c) * ((one - b) * x + b * self.s()) * (one - self.s() - self.s())
            - c * self.s();
        let u =
            a * (one - c) * (b * x + (one - b) * (twol / tpow) * y) + (twol - one) * c * self.s();
        let u_slice = Slice::bit_decomposition_ClearModp_Signed(u, V);
        let mut vu: ClearFloat<V, P, DETECT_OVERFLOW> = ClearFloat::from(1.0);
        /* This loop could be done more efficiently by doing the final floating point
         * multiplication via a binary tree
         */
        for i in 0..V {
            let t_t_mi1 = ClearIEEE::from(-(i as i64 + 1)).exp2().exp2();
            let temp: ClearFloat<V, P, DETECT_OVERFLOW> = ClearFloat::from(t_t_mi1);
            let ui = *u_slice.get_unchecked(V - 1 - i);
            let ai = twolm1 * (one - ui) + temp.v() * ui;
            let bi = -vm1 * (one - ui) + temp.p() * ui;

            let mut Atemp: Array<ClearModp, 5> = Array::uninitialized();
            Atemp.set(0, &ai);
            Atemp.set(1, &bi);
            Atemp.set(2, &zero);
            Atemp.set(3, &zero);
            Atemp.set(4, &zero);
            let temp: ClearFloat<V, P, DETECT_OVERFLOW> = ClearFloat::set(Atemp);

            vu = &vu * &temp;
        }
        let p = a * (w + vu.p()) + twokm1 * (one - a) * (one - self.s() - self.s());
        let v = twolm1 * self.z() + (one - self.z()) * vu.v();
        let p = -self.z() * vm1 + (one - self.z()) * p;
        let mut Atemp: Array<ClearModp, 5> = Array::uninitialized();
        Atemp.set(0, &v);
        Atemp.set(1, &p);
        Atemp.set(2, &zero);
        Atemp.set(3, &zero);
        Atemp.set(4, &self.err());
        let temp: ClearFloat<V, P, DETECT_OVERFLOW> = ClearFloat::set(Atemp);
        temp
    }

    #[inline(always)]
    pub fn exp(self) -> Self {
        let log2 = ClearFloat::ln2();
        let ans = (self / log2).exp2();
        ans
    }

    // Computes exp(x) and exp(-x)
    fn exp_2(self) -> Array<Self, 2> {
        let log2 = ClearFloat::ln2();
        let ans1 = (self / log2).exp2();
        let ans2 = ClearFloat::from(1_i64) / &ans1;
        let mut ans: Array<Self, 2> = Array::uninitialized();
        ans.set(0, &ans1);
        ans.set(1, &ans2);
        ans
    }

    #[inline(always)]
    pub fn cosh(self) -> Self {
        let e2 = self.exp_2();
        let ans = (&*e2.get_unchecked(0) + &*e2.get_unchecked(1)) * ClearFloat::from(0.5);
        ans
    }
    #[inline(always)]
    pub fn sinh(self) -> Self {
        let e2 = self.exp_2();
        let ans = (&*e2.get_unchecked(0) - &*e2.get_unchecked(1)) * ClearFloat::from(0.5);
        ans
    }
    #[inline(always)]
    pub fn tanh(self) -> Self {
        let e2 = self.exp_2();
        let ans = (&*e2.get_unchecked(0) - &*e2.get_unchecked(1))
            / (&*e2.get_unchecked(0) + &*e2.get_unchecked(1));
        ans
    }
}

impl<const V: u64, const P: u64, const DETECT_OVERFLOW: bool> Floor
    for ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ V - 1 }>: ,
    ConstU64<{ V + 2 }>: ,
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ 2 * V + 1 }>: ,
{
    #[inline(always)]
    fn floor(self) -> Self {
        self.FLRound(0)
    }
}

/********************************/
/*           SecretFloat        */
/********************************/

impl<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool> FAbs
    for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
{
    #[inline(always)]
    fn fabs(self) -> SecretFloat<V, P, KAPPA, DETECT_OVERFLOW> {
        let mut param_new: Array<SecretModp, 5> = Array::uninitialized();
        param_new.set(0, &self.v());
        param_new.set(1, &self.p());
        param_new.set(2, &self.z());
        param_new.set(3, &(SecretModp::from(0)));
        param_new.set(4, &self.err());
        SecretFloat::set(param_new)
    }
}

impl<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ V - 1 }>: ,
    ConstU64<{ V + 2 }>: ,
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ 2 * V + 1 }>: ,
{
    // Rounding with FP output
    // 0 -> floor, 1 -> ceil, -1 -> trunc
    #[allow(non_snake_case)]
    fn FLRound(self, mode: i64) -> Self {
        let p_int: SecretInteger<V, KAPPA> = SecretInteger::from(self.p());
        // let a = p_int.ltz(); Causes a problem so expanded it here...
        let a = -p_int.Trunc(V - 1, true).rep();
        let p2_int: SecretInteger<V, KAPPA> =
            SecretInteger::from(self.p() + ClearModp::from(V as i64 - 1));
        let b = -p2_int.Trunc(V - 1, true).rep();
        let v_int: SecretInteger<V, KAPPA> = SecretInteger::from(self.v());
        let m = -a * (ClearModp::from(1) - b) * self.p();

        let ObTr = v_int.ObliviousTrunc(m);

        let v2_int: SecretInteger<V, KAPPA> = SecretInteger::from(*ObTr.get_unchecked(1));
        let inv_2pow_p1 = *ObTr.get_unchecked(2);

        let c = v2_int.eqz();
        let mut away_from_zero = SecretModp::from(0);
        let mut modes = SecretModp::from(mode);
        if mode == -1 {
            modes = self.s();
        } else {
            away_from_zero = ClearModp::from(mode) + self.s() * ClearModp::from(1 - 2 * mode);
        }
        let mut v =
            self.v() - v2_int.rep() + (ClearModp::from(1) - c) * inv_2pow_p1 * away_from_zero;
        let twopower1 = modp_two_power(V - 1);
        let twopower = twopower1 + twopower1;
        let v_int: SecretInteger<{ V + 1 }, KAPPA> = SecretInteger::from(v - twopower);
        let d = v_int.eqz();
        v = d * twopower1 + (ClearModp::from(1) - d) * v;
        v = a * ((ClearModp::from(1) - b) * v + b * away_from_zero * twopower1)
            + (ClearModp::from(1) - a) * self.v();
        let s = (ClearModp::from(1) - b * modes) * self.s();
        let v_int: SecretInteger<V, KAPPA> = SecretInteger::from(v);
        let v_zero = v_int.eqz();
        let z = v_zero + self.z() - v_zero * self.z(); // z = Or(v_zero, z1)
        v = v * (ClearModp::from(1) - z);
        let p = ((self.p() + d * a) * (ClearModp::from(1) - b)
            + b * away_from_zero * ClearModp::from(1 - V as i64))
            * (ClearModp::from(1) - z);
        let mut ans: Array<SecretModp, 5> = Array::uninitialized();
        ans.set(0, &v);
        ans.set(1, &p);
        ans.set(2, &z);
        ans.set(3, &s);
        ans.set(4, &self.err());

        let answer: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW> = SecretFloat::set(ans);
        answer
    }

    #[inline(always)]
    pub fn ceil(self) -> Self {
        self.FLRound(1)
    }

    #[allow(non_snake_case)]
    pub fn log2(self) -> Self {
        let szero = SecretModp::from(0);
        let one = ClearModp::from(1);
        let one_float: ClearFloat<V, P, DETECT_OVERFLOW> = ClearFloat::from(1.0);

        let mut temp: Array<SecretModp, 5> = Array::uninitialized();
        temp.set(0, &self.v());
        temp.set(1, &SecretModp::from(-(V as i64)));
        temp.set(2, &szero);
        temp.set(3, &szero);
        temp.set(4, &szero);
        let vv: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW> = SecretFloat::set(temp);

        // 2*log_2 e
        let tlog2e_ieee: ClearIEEE = ClearIEEE::from(2.8853900817779268147198493620037842749);
        let tlog2e: ClearFloat<V, P, DETECT_OVERFLOW> = ClearFloat::from(tlog2e_ieee);

        let mut v2 = &one_float - &vv;
        let v3 = &one_float + &vv;
        let mut vy = &v2 / &v3;
        let vy2 = &vy * &vy;
        let mut vv = &vy * &tlog2e;

        // Here we approximate M = ceil(V/(2*log2(3)) - 1/2) by V/3
        for i in 1..V / 3 {
            let den: ClearIEEE = ClearIEEE::from(2 * i as i64 + 1);
            let temp = tlog2e_ieee / den;
            let temp: ClearFloat<V, P, DETECT_OVERFLOW> = ClearFloat::from(temp);
            vy = &vy * &vy2;
            v2 = &vy * &temp;
            vv = &vv + &v2;
        }

        let temp: SecretInteger<V, KAPPA> = SecretInteger::from(ClearModp::from(V as i64) + vv.p());
        let v2 = Secret_Int2Fl(temp, ConstU64::<V>);
        let v2: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW> = SecretFloat::set(v2);

        vv = &v2 - &vv;

        let t_int: SecretInteger<P, KAPPA> =
            SecretInteger::from(self.p() + ClearModp::from(V as i64 - 1));
        let a = t_int.eqz();

        let ctLm1 = modp_two_power(V - 1);
        let t_int: SecretInteger<V, KAPPA> = SecretInteger::from(self.v() - ctLm1);
        let b = t_int.eqz();

        let z = a * b;
        let v = vv.v() * (one - z);
        let p = vv.p() * (one - z);
        let err = self.err() + self.z() + self.s();

        let mut temp: Array<SecretModp, 5> = Array::uninitialized();
        temp.set(0, &v);
        temp.set(1, &p);
        temp.set(2, &z);
        temp.set(3, &vv.s());
        temp.set(4, &err);

        let ans: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW> = SecretFloat::set(temp);
        ans
    }

    #[inline(always)]
    pub fn log(self) -> Self {
        let z = self.log2();
        z * ClearFloat::<V, P, DETECT_OVERFLOW>::ln2()
    }
    #[inline(always)]
    pub fn log10(self) -> Self {
        let z = self.log2();
        z * ClearFloat::<V, P, DETECT_OVERFLOW>::from(0.30102999566398119521373889472449302677)
    }

    // Assumes V < 2^{P-1}
    #[allow(non_snake_case)]
    pub fn exp2(self) -> Self {
        // If V < 2^{P-1} then we have
        // max = ceil( log_2(2^{P-1}-1+V) - V + 1)
        //     = P-V+1
        let max = ClearModp::from(P as i64 - V as i64 + 1);
        let one = ClearModp::from(1);
        let szero = SecretModp::from(0);
        let twokm1 = modp_two_power(P - 1);
        let twolm1 = modp_two_power(V - 1);
        let vm1 = ClearModp::from(V as i64 - 1);
        let twol = twolm1 + twolm1;
        let a_int: SecretInteger<P, KAPPA> = SecretInteger::from(self.p() - max);
        let b_int: SecretInteger<P, KAPPA> = SecretInteger::from(self.p() + vm1);
        let c_int: SecretInteger<P, KAPPA> =
            SecretInteger::from(self.p() + ClearModp::from(2 * V as i64 - 1));
        let a = a_int.ltz();
        let b = b_int.ltz();
        let c = c_int.ltz();
        let p2 =
            -a * (one - c) * (b * (self.p() + ClearModp::from(V as i64)) + (one - b) * self.p());
        let v_int: SecretInteger<V, KAPPA> = SecretInteger::from(self.v());

        let ObTr = v_int.ObliviousTrunc(p2);
        let x = *ObTr.get_unchecked(0);
        let y = *ObTr.get_unchecked(1);
        let tpow = *ObTr.get_unchecked(2);

        let y_int: SecretInteger<V, KAPPA> = SecretInteger::from(y);
        let d = y_int.eqz();
        let x = (one - b * self.s()) * (x - (one - d) * self.s())
            + b * self.s() * (twol - ClearModp::from(1) + d - x);
        let y = (one - d) * self.s() * (tpow - y) + (one - self.s()) * y;
        let w = a * (one - c) * ((one - b) * x + b * self.s()) * (one - self.s() - self.s())
            - c * self.s();
        let u = a * (one - c) * (b * x + (one - b) * (twol * Inv(tpow)) * y)
            + (twol - one) * c * self.s();
        let u_slice = BitDec::<V, V, KAPPA>(u);
        let mut vu: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW> = SecretFloat::from(1.0);
        /* This loop could be done more efficiently by doing the final floating point
         * multiplication via a binary tree
         */
        for i in 0..V {
            let t_t_mi1 = ClearIEEE::from(-(i as i64 + 1)).exp2().exp2();
            let temp: ClearFloat<V, P, DETECT_OVERFLOW> = ClearFloat::from(t_t_mi1);
            let ui = *u_slice.get_unchecked(V - 1 - i);
            let ai = twolm1 * (one - ui) + temp.v() * ui;
            let bi = -vm1 * (one - ui) + temp.p() * ui;

            let mut Atemp: Array<SecretModp, 5> = Array::uninitialized();
            Atemp.set(0, &ai);
            Atemp.set(1, &bi);
            Atemp.set(2, &szero);
            Atemp.set(3, &szero);
            Atemp.set(4, &szero);
            let temp: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW> = SecretFloat::set(Atemp);

            vu = &vu * &temp;
        }
        let p = a * (w + vu.p()) + twokm1 * (one - a) * (one - self.s() - self.s());
        let v = twolm1 * self.z() + (one - self.z()) * vu.v();
        let p = -self.z() * vm1 + (one - self.z()) * p;
        let mut Atemp: Array<SecretModp, 5> = Array::uninitialized();
        Atemp.set(0, &v);
        Atemp.set(1, &p);
        Atemp.set(2, &szero);
        Atemp.set(3, &szero);
        Atemp.set(4, &self.err());
        let temp: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW> = SecretFloat::set(Atemp);
        temp
    }

    #[inline(always)]
    pub fn exp(self) -> Self {
        let log2 = ClearFloat::ln2();
        let ans = (self / log2).exp2();
        ans
    }

    // Computes exp(x) and exp(-x)
    fn exp_2(self) -> Array<Self, 2> {
        let log2 = ClearFloat::ln2();
        let ans1 = (self / log2).exp2();
        let ans2 = ClearFloat::from(1_i64) / &ans1;
        let mut ans: Array<Self, 2> = Array::uninitialized();
        ans.set(0, &ans1);
        ans.set(1, &ans2);
        ans
    }

    #[inline(always)]
    pub fn cosh(self) -> Self {
        let e2 = self.exp_2();
        let ans = (&*e2.get_unchecked(0) + &*e2.get_unchecked(1)) * ClearFloat::from(0.5);
        ans
    }
    #[inline(always)]
    pub fn sinh(self) -> Self {
        let e2 = self.exp_2();
        let ans = (&*e2.get_unchecked(0) - &*e2.get_unchecked(1)) * ClearFloat::from(0.5);
        ans
    }
    #[inline(always)]
    pub fn tanh(self) -> Self {
        let e2 = self.exp_2();
        let ans = (&*e2.get_unchecked(0) - &*e2.get_unchecked(1))
            / (&*e2.get_unchecked(0) + &*e2.get_unchecked(1));
        ans
    }
}

impl<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool> Floor
    for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ V - 1 }>: ,
    ConstU64<{ V + 2 }>: ,
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ 2 * V + 1 }>: ,
{
    #[inline(always)]
    fn floor(self) -> Self {
        self.FLRound(0)
    }
}
