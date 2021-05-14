// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use crate::array::*;
use crate::fixed_point::*;
use crate::ieee::*;
use crate::integer::*;
use crate::math::*;
use core::ops::{Add, Div, Mul, Sub};
use scale::*;

/**************************************
 *  Generic Routines Used by math.rs  *
 **************************************/

/**************************************
 *             Trigonometry           *
 **************************************/

/* Reduces the input to [0,90) and returns whether the  reduced value is
* greater than \Pi and greater than Pi over 2
* Input x: value of any type to be reduced to the [0,90) interval

* Return (w, s1, s2)
*   w: reduced angle
*   s1 \in \{ -1, 1 } : -1 when reduction to 2*pi is greater than pi
*   s2 \in \{ -1, 1 } : -1 when reduction to pi is greater than pi/2
*/

#[allow(non_snake_case)]
pub fn TrigSubroutine<S, C, O>(x: S) -> Array<S, 3>
where
    S: Float,
    S: From<O>,
    S: Add<S, Output = S>,
    S: Sub<S, Output = S>,
    S: Sub<C, Output = S>,
    S: Mul<S, Output = S>,
    S: Mul<C, Output = S>,
    S: Div<C, Output = S>,
    S: ScaleCmpZ<O>,
    S: Reveal<Output = C>, // For testing
    C: Float,
    C: Sub<C, Output = C>,
    C: Sub<S, Output = S>,
    C: Mul<S, Output = S>,
    C: Print, // For testing
{
    let two_pi: C = C::two_pi();
    let pi: C = C::pi();
    let half_pi: C = C::half_pi();
    let f = (x / two_pi).floor();
    let y = x - f * two_pi;
    let t = y - pi;
    let b1 = t.gtz();
    let mut s1: S = S::from(b1);
    let mut w = s1 * (two_pi - y - y) + y;
    let t = w - half_pi;
    let b2 = t.gtz();
    let mut s2: S = S::from(b2);
    w = s2 * (pi - w - w) + w;
    s1 = S::from(1_i64) - s1 - s1;
    s2 = S::from(1_i64) - s2 - s2;
    let mut ans: Array<S, 3> = Array::uninitialized();
    ans.set(0, &w);
    ans.set(1, &s1);
    ans.set(2, &s2);
    ans
}

// The Kernel Sin Routine
pub fn kernel_sin<S, C>(w: S, s: S) -> S
where
    S: Float,
    S: Add<C, Output = S>,
    S: Mul<S, Output = S>,
    S: Div<C, Output = S>,
    S: Reveal<Output = C>, // For testing
    C: Float,
    C: Print, // Testing
{
    let mut p_3307: Array<C, 11> = Array::uninitialized();
    p_3307.set(0, &C::from(1.57079632679489000000000));
    p_3307.set(1, &C::from(-0.64596409750624600000000));
    p_3307.set(2, &C::from(0.07969262624616700000000));
    p_3307.set(3, &C::from(-0.00468175413531868000000));
    p_3307.set(4, &C::from(0.00016044118478735800000));
    p_3307.set(5, &C::from(-0.00000359884323520707000));
    p_3307.set(6, &C::from(0.00000005692172920657320));
    p_3307.set(7, &C::from(-0.00000000066880348849204));
    p_3307.set(8, &C::from(0.00000000000606691056085));
    p_3307.set(9, &C::from(-0.00000000000004375295071));
    p_3307.set(10, &C::from(0.00000000000000025002854));
    let v = w / C::half_pi();
    let v2 = v * v;
    let pol = poly_eval(p_3307, v2);
    let ans = s * v * pol;
    ans
}

// The Kernel Cos Routine
#[allow(non_snake_case)]
pub fn kernel_cos<S, C>(w: S, s: S) -> S
where
    S: Float,
    S: Add<C, Output = S>,
    S: Mul<S, Output = S>,
    S: Div<C, Output = S>,
    C: Float,
{
    let mut p_3508: Array<C, 11> = Array::uninitialized();
    p_3508.set(0, &C::from(1.00000000000000000000));
    p_3508.set(1, &C::from(-0.50000000000000000000));
    p_3508.set(2, &C::from(0.04166666666666667129));
    p_3508.set(3, &C::from(-0.00138888888888888873));
    p_3508.set(4, &C::from(0.00002480158730158702));
    p_3508.set(5, &C::from(-0.00000027557319223933));
    p_3508.set(6, &C::from(0.00000000208767569817));
    p_3508.set(7, &C::from(-0.00000000001147074513));
    p_3508.set(8, &C::from(0.00000000000004779454));
    p_3508.set(9, &C::from(-0.00000000000000015612));
    p_3508.set(10, &C::from(0.00000000000000000040));
    let v = w;
    let v2 = v * v;
    let ans = s * poly_eval(p_3508, v2);
    ans
}

/* Code for generic atan, asin and acos */

// The Kernel atan Routine
#[inline(never)]
pub fn kernel_atan<S, C, O>(x: S) -> S
where
    S: Float,
    S: From<C>,
    S: From<O>,
    S: Add<C, Output = S>,
    S: Add<S, Output = S>,
    S: Sub<C, Output = S>,
    S: Sub<S, Output = S>,
    S: Mul<S, Output = S>,
    S: Mul<C, Output = S>,
    S: Div<C, Output = S>,
    S: Div<S, Output = S>,
    S: ScaleCmpZ<O>,
    S: Reveal<Output = C>, // For testing
    C: Float,
    C: Sub<S, Output = S>,
    C: Div<S, Output = S>,
    C: Print, // Testing
{
    let one: C = C::from(1_i64);
    let s_t = x.ltz();
    let s: S = S::from(s_t);

    let x_abs = x.fabs();
    let b_t = (x_abs - one).gtz();
    let b: S = S::from(b_t);
    let oneb = one - b;
    let mut v = one / x_abs;
    v = oneb * (x_abs - v) + v;
    let v2 = v * v;

    let mut p_5102: Array<C, 9> = Array::uninitialized();
    p_5102.set(0, &C::from(21514.05962602441933193254468));
    p_5102.set(1, &C::from(73597.43380288444240814980706));
    p_5102.set(2, &C::from(100272.5618306302784970511863));
    p_5102.set(3, &C::from(69439.29750032252337059765503));
    p_5102.set(4, &C::from(25858.09739719099025716567793));
    p_5102.set(5, &C::from(5038.63918550126655793779119));
    p_5102.set(6, &C::from(460.1588804635351471161727227));
    p_5102.set(7, &C::from(15.08767735870030987717455528));
    p_5102.set(8, &C::from(0.07523052818757628444510729539));
    /* Print P Poly */
    let mut q_5102: Array<C, 9> = Array::uninitialized();
    q_5102.set(0, &C::from(21514.05962602441933193298234));
    q_5102.set(1, &C::from(80768.78701155924885176713209));
    q_5102.set(2, &C::from(122892.6789092784776298743322));
    q_5102.set(3, &C::from(97323.20349053555680260434387));
    q_5102.set(4, &C::from(42868.57652046408093184006664));
    q_5102.set(5, &C::from(10401.13491566890057005103878));
    q_5102.set(6, &C::from(1289.75056911611097141145955));
    q_5102.set(7, &C::from(68.51937831018968013114024294));
    q_5102.set(8, &C::from(1_i64));

    let mut y = v * Pade(p_5102, q_5102, v2);
    let y_ptwo = C::half_pi() - y;
    y = oneb * (y - y_ptwo) + y_ptwo;
    y = (one - s) * (y + y) - y;

    y
}

// The Kernel asin Routine
#[inline(always)]
pub fn kernel_asin<S, C, O>(x: S) -> S
where
    S: Float,
    S: From<C>,
    S: From<O>,
    S: Add<C, Output = S>,
    S: Add<S, Output = S>,
    S: Sub<C, Output = S>,
    S: Sub<S, Output = S>,
    S: Mul<S, Output = S>,
    S: Mul<C, Output = S>,
    S: Div<C, Output = S>,
    S: Div<S, Output = S>,
    S: ScaleCmpZ<O>,
    S: Reveal<Output = C>, // For testing
    C: Float,
    C: Sub<S, Output = S>,
    C: Div<S, Output = S>,
    C: Print, // Testing
{
    let x2 = x * x;
    let sqrt1 = (C::from(1_i64) - x2).sqrt();
    let isqrt = x / sqrt1;
    let ans = kernel_atan::<S, C, O>(isqrt);
    ans
}

// The Kernel acos Routine
#[inline(always)]
pub fn kernel_acos<S, C, O>(x: S) -> S
where
    S: Float,
    S: From<C>,
    S: From<O>,
    S: Add<C, Output = S>,
    S: Add<S, Output = S>,
    S: Sub<C, Output = S>,
    S: Sub<S, Output = S>,
    S: Mul<S, Output = S>,
    S: Mul<C, Output = S>,
    S: Div<C, Output = S>,
    S: Div<S, Output = S>,
    S: ScaleCmpZ<O>,
    S: Reveal<Output = C>, // For testing
    C: Float,
    C: Sub<S, Output = S>,
    C: Div<S, Output = S>,
    C: Print, // Testing
{
    let y = kernel_asin::<S, C, O>(x);
    let ans = C::half_pi() - y;
    ans
}

pub fn atan_sfix<const K: u64, const F: u64, const KAPPA: u64>(
    x: SecretFixed<K, F, KAPPA>,
) -> SecretFixed<K, F, KAPPA>
where
    ConstU64<{ K - F }>: ,
    ConstU64<{ 2 * K }>: ,
    ConstU64<{ 2 * F }>: ,
    ConstU64<{ 2 * (K - F) }>: ,
    ConstU64<{ K - 1 }>: ,
    ConstU64<{ K + 1 }>: ,
    ConstU64<{ F + 1 }>: ,
    ConstI32<{ f_as_i32(F) }>: ,
    ConstI32<{ f_as_i32(K) }>: ,
    ConstU64<{ ClearFixed::<K, F>::THETA }>: ,
    ConstU64<{ SecretFixed::<K, F, KAPPA>::THETA }>: ,
{
    let ans = kernel_atan::<SecretFixed<K, F, KAPPA>, ClearFixed<K, F>, SecretModp>(x);
    ans
}

/***********************************
 *       Exponenentiation          *
 ***********************************/

// This trait captures a function which produces 2^self
// assuming
//   - self is a fixed point value which represents an integer.
//       - If x has a fractional part the value will be wrong
//   - self is positive
pub trait Pow2 {
    fn pow2(self) -> Self;
}

// Implementations of the Pow2 trait
impl<const K: u64, const F: u64> Pow2 for ClearFixed<K, F>
where
    ConstI32<{ f_as_i32(F) }>: ,
{
    fn pow2(self) -> ClearFixed<K, F> {
        let v = self.rep().rep() >> ConstI32::<{ f_as_i32(F) }>;
        let pow = v + ClearModp::from(f_as_i32(F) as i64);
        let y = ClearModp::from(1) << pow;
        let z: ClearInteger<K> = ClearInteger::from(y);
        ClearFixed::set(z)
    }
}

impl<const K: u64, const F: u64, const KAPPA: u64> Pow2 for SecretFixed<K, F, KAPPA>
where
    ConstI32<{ f_as_i32(F) }>: ,
{
    fn pow2(self) -> SecretFixed<K, F, KAPPA> {
        let v = self.rep().Trunc(F, false).rep();
        let pow = v + ClearModp::from(f_as_i32(F) as i64);
        let y = Pow2::<K, KAPPA>(pow);
        let z: SecretInteger<K, KAPPA> = SecretInteger::from(y);
        SecretFixed::set(z)
    }
}

impl Pow2 for SecretIEEE {
    // Here we use a trick related to IEEE encoding
    fn pow2(self) -> SecretIEEE {
        let y = SecretI64::from(self);
        let e = y + 1023_i64;
        let t = e << ConstU32::<52>;
        SecretIEEE::set(t)
    }
}

// The Kernel Exp2 Routines: 2^x

// Computes 2^{|x|} for a fixed point type
//   - Here x can have a fractional part
pub fn kernel_exp2<S, C, O>(x: S) -> S
where
    S: Float,
    S: Pow2,
    S: From<O>,
    S: Add<C, Output = S>,
    S: Sub<S, Output = S>,
    S: Mul<S, Output = S>,
    S: ScaleCmpZ<O>,
    S: Reveal<Output = C>, // For testing
    C: Float,
    C: Print, // Testing
{
    let a = x.fabs();
    let b = a.floor();
    let c = a - b;
    let d = b.pow2();

    let mut p_1045: Array<C, 10> = Array::uninitialized();
    p_1045.set(0, &C::from(0.99999999999998151058451));
    p_1045.set(1, &C::from(0.69314718056364205693851));
    p_1045.set(2, &C::from(0.24022650683729748257646));
    p_1045.set(3, &C::from(0.0555041102193305250618));
    p_1045.set(4, &C::from(0.0096181190501642860210497));
    p_1045.set(5, &C::from(0.0013333931011014250476911));
    p_1045.set(6, &C::from(0.00015395144945146697380844));
    p_1045.set(7, &C::from(0.000015368748541192116946474));
    p_1045.set(8, &C::from(0.0000012256971722926501833228));
    p_1045.set(9, &C::from(0.00000014433329807023165258784));
    let e = poly_eval(p_1045, c);
    let g = d * e;
    g
}

/* Kernel log_2 routine
 * Input must be in the range [1/2,..1]
 */
pub fn kernel_log2<S, C>(x: S) -> S
where
    S: Float,
    S: Add<S, Output = S>,
    S: Mul<S, Output = S>,
    S: Div<S, Output = S>,
    S: Mul<C, Output = S>,
    S: From<C>,
    C: Float,
{
    let mut p_2524: Array<C, 4> = Array::uninitialized();
    p_2524.set(0, &C::from(-2.05466671951));
    p_2524.set(1, &C::from(-8.8626599391));
    p_2524.set(2, &C::from(6.10585199015));
    p_2524.set(3, &C::from(4.81147460989));
    /* Print P Poly */
    let mut q_2524: Array<C, 4> = Array::uninitialized();
    q_2524.set(0, &C::from(0.353553425277));
    q_2524.set(1, &C::from(4.54517087629));
    q_2524.set(2, &C::from(6.42784209029));
    q_2524.set(3, &C::from(1_i64));

    let ans = Pade(p_2524, q_2524, x);
    ans
}
