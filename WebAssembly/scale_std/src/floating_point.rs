// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use crate::array::*;
use crate::bit_protocols::*;
use crate::fixed_point::*;
use crate::float_subroutines::*;
use crate::ieee::*;
use crate::integer::*;
use crate::local_functions::*;
use crate::slice::*;
use core::ops::{Add, Div, Mul, Neg, Sub};
use scale::alloc::*;
use scale::*;

/* This gives floating point arithmetic
 *
 * It uses the global statistical security parameter kappa
 * Follows the algorithms in Section 14.5 of the main Scale
 * manual
 *
 */

#[derive(Clone)]
pub struct ClearFloat<const V: u64, const P: u64, const DETECT_OVERFLOW: bool> {
    param: Array<ClearModp, 5>, // v, p, z, s, err
}

#[derive(Clone)]
pub struct SecretFloat<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool> {
    param: Array<SecretModp, 5>, // v, p, z, s, err
}

/*
 * Stuff to enable usage in arrays etc
 *
 */

impl<const V: u64, const P: u64, const DETECT_OVERFLOW: bool> GetAllocator
    for ClearFloat<V, P, DETECT_OVERFLOW>
{
    type Allocator = &'static Allocator<ClearModp>;
    fn get_allocator() -> &'static Allocator<ClearModp> {
        ClearModp::get_allocator()
    }
    fn size_type() -> u64 {
        5
    }
}

impl<const V: u64, const P: u64, const DETECT_OVERFLOW: bool> LoadFromMem<i64>
    for ClearFloat<V, P, DETECT_OVERFLOW>
{
    #[allow(non_snake_case)]
    fn load_from_mem(idx: i64) -> Self {
        let mut Atemp: Array<ClearModp, 5> = Array::uninitialized();
        Atemp.set(0, &ClearModp::load_from_mem(idx));
        Atemp.set(1, &ClearModp::load_from_mem(idx + 1));
        Atemp.set(2, &ClearModp::load_from_mem(idx + 2));
        Atemp.set(3, &ClearModp::load_from_mem(idx + 3));
        Atemp.set(4, &ClearModp::load_from_mem(idx + 4));
        let temp: ClearFloat<V, P, DETECT_OVERFLOW> = ClearFloat::set(Atemp);
        temp
    }
}

impl<const V: u64, const P: u64, const DETECT_OVERFLOW: bool> StoreInMem<i64>
    for ClearFloat<V, P, DETECT_OVERFLOW>
{
    unsafe fn store_in_mem(&self, idx: i64) {
        self.v().store_in_mem(idx);
        self.p().store_in_mem(idx + 1);
        self.z().store_in_mem(idx + 2);
        self.s().store_in_mem(idx + 3);
        self.err().store_in_mem(idx + 4);
    }
}

impl<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool> GetAllocator
    for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
{
    type Allocator = &'static Allocator<SecretModp>;
    fn get_allocator() -> &'static Allocator<SecretModp> {
        SecretModp::get_allocator()
    }
    fn size_type() -> u64 {
        5
    }
}

impl<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool> LoadFromMem<i64>
    for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
{
    #[allow(non_snake_case)]
    fn load_from_mem(idx: i64) -> Self {
        let mut Atemp: Array<SecretModp, 5> = Array::uninitialized();
        Atemp.set(0, &SecretModp::load_from_mem(idx));
        Atemp.set(1, &SecretModp::load_from_mem(idx + 1));
        Atemp.set(2, &SecretModp::load_from_mem(idx + 2));
        Atemp.set(3, &SecretModp::load_from_mem(idx + 3));
        Atemp.set(4, &SecretModp::load_from_mem(idx + 4));
        let temp: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW> = SecretFloat::set(Atemp);
        temp
    }
}

impl<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool> StoreInMem<i64>
    for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
{
    unsafe fn store_in_mem(&self, idx: i64) {
        self.v().store_in_mem(idx);
        self.p().store_in_mem(idx + 1);
        self.z().store_in_mem(idx + 2);
        self.s().store_in_mem(idx + 3);
        self.err().store_in_mem(idx + 4);
    }
}

/* Prints a clear float */
impl<const V: u64, const P: u64, const DETECT_OVERFLOW: bool> Print
    for ClearFloat<V, P, DETECT_OVERFLOW>
{
    #[inline(always)]
    fn print(self) {
        unsafe {
            __print_float(
                *self.param.get_unchecked(0),
                *self.param.get_unchecked(1),
                *self.param.get_unchecked(2),
                *self.param.get_unchecked(3),
                *self.param.get_unchecked(4),
            )
        }
    }
}

impl<'a, const V: u64, const P: u64, const DETECT_OVERFLOW: bool> Print
    for &'a ClearFloat<V, P, DETECT_OVERFLOW>
{
    #[inline(always)]
    fn print(self) {
        unsafe {
            __print_float(
                *self.param.get_unchecked(0),
                *self.param.get_unchecked(1),
                *self.param.get_unchecked(2),
                *self.param.get_unchecked(3),
                *self.param.get_unchecked(4),
            )
        }
    }
}

/* over or underflow detection for SecretFloats */
pub fn flow_detect<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>(
    p: SecretModp,
) -> SecretModp
where
    ConstU64<{ P + 1 }>: ,
{
    if DETECT_OVERFLOW {
        let p_int: SecretInteger<P, KAPPA> = SecretInteger::from(p);
        let s = p_int.ltz() * ClearModp::from(-2) + ClearModp::from(1);
        let sp_int: SecretInteger<P, KAPPA> = SecretInteger::from(s * p);
        let max: SecretInteger<P, KAPPA> = SecretInteger::from(two_power(P - 1) as i64);
        sp_int.ge(max)
    } else {
        SecretModp::from(0)
    }
}

/* over or underflow detection for ClearFloats */
pub fn flow_detect_c<const V: u64, const P: u64, const DETECT_OVERFLOW: bool>(
    p: ClearModp,
) -> ClearModp
where
    ConstU64<{ P + 1 }>: ,
{
    if DETECT_OVERFLOW {
        let p_int: ClearInteger<P> = ClearInteger::from(p);
        let s = p_int.ltz() * ClearModp::from(-2) + ClearModp::from(1);
        let sp_int: ClearInteger<P> = ClearInteger::from(s * p);
        let max: ClearInteger<P> = ClearInteger::from(two_power(P - 1) as i64);
        sp_int.ge(max)
    } else {
        ClearModp::from(0)
    }
}

/* Basic Constructors */

impl<const V: u64, const P: u64, const DETECT_OVERFLOW: bool> From<ClearIEEE>
    for ClearFloat<V, P, DETECT_OVERFLOW>
{
    #[inline(always)]
    fn from(a: ClearIEEE) -> Self {
        let x = to_float(a.rep(), V as i64, P as i64);
        ClearFloat::set(x)
    }
}

impl<const V: u64, const P: u64, const DETECT_OVERFLOW: bool> From<f64>
    for ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ V - 1 }>: ,
    ConstU64<{ V + 1 }>: ,
{
    #[inline(always)]
    fn from(f: f64) -> Self {
        let a: ClearIEEE = ClearIEEE::from(f);
        Self::from(a)
    }
}

impl<const V: u64, const P: u64, const DETECT_OVERFLOW: bool> From<i64>
    for ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ V - 1 }>: ,
    ConstU64<{ V + 1 }>: ,
{
    #[inline(always)]
    fn from(a: i64) -> Self {
        let b: ClearIEEE = ClearIEEE::from(a);
        Self::from(b)
    }
}

impl<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool> From<f64>
    for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ V - 1 }>: ,
    ConstU64<{ V + 1 }>: ,
{
    #[inline(always)]
    fn from(f: f64) -> Self {
        let a: ClearIEEE = ClearIEEE::from(f);
        Self::from(ClearFloat::from(a))
    }
}

impl<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool> From<i64>
    for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ V - 1 }>: ,
    ConstU64<{ V + 1 }>: ,
{
    #[inline(always)]
    fn from(a: i64) -> Self {
        let b: ClearIEEE = ClearIEEE::from(a);
        Self::from(ClearFloat::from(b))
    }
}

impl<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    From<ClearFloat<V, P, DETECT_OVERFLOW>> for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
{
    #[inline(always)]
    fn from(a: ClearFloat<V, P, DETECT_OVERFLOW>) -> Self {
        let mut ans: Array<SecretModp, 5> = Array::uninitialized();
        ans.set(0, &SecretModp::from(*a.param.get_unchecked(0)));
        ans.set(1, &SecretModp::from(*a.param.get_unchecked(1)));
        ans.set(2, &SecretModp::from(*a.param.get_unchecked(2)));
        ans.set(3, &SecretModp::from(*a.param.get_unchecked(3)));
        ans.set(4, &SecretModp::from(*a.param.get_unchecked(4)));
        Self { param: ans }
    }
}

impl<const V: u64, const P: u64, const K: u64, const DETECT_OVERFLOW: bool> From<ClearInteger<K>>
    for ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ K - 1 }>: ,
{
    #[inline(always)]
    fn from(a: ClearInteger<K>) -> Self {
        let x = Clear_Int2Fl(a, ConstU64::<V>);
        ClearFloat::set(x)
    }
}

impl<const V: u64, const P: u64, const KAPPA: u64, const K: u64, const DETECT_OVERFLOW: bool>
    From<SecretInteger<K, KAPPA>> for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ K - 1 }>: ,
{
    #[inline(always)]
    fn from(a: SecretInteger<K, KAPPA>) -> Self {
        let x = Secret_Int2Fl(a, ConstU64::<V>);
        SecretFloat::set(x)
    }
}

impl<const K: u64, const F: u64, const V: u64, const P: u64, const DETECT_OVERFLOW: bool>
    From<ClearFixed<K, F>> for ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ K - 1 }>: ,
{
    #[inline(always)]
    fn from(a: ClearFixed<K, F>) -> Self {
        let mut x = Clear_Int2Fl(a.rep(), ConstU64::<V>);
        x.set(
            1,
            &((*x.get_unchecked(1) - ClearModp::from(F as i64))
                * (ClearModp::from(1) - *x.get_unchecked(3))),
        );
        ClearFloat::set(x)
    }
}

impl<
        const K: u64,
        const F: u64,
        const V: u64,
        const P: u64,
        const KAPPA: u64,
        const DETECT_OVERFLOW: bool,
    > From<SecretFixed<K, F, KAPPA>> for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ K - 1 }>: ,
{
    #[inline(always)]
    fn from(a: SecretFixed<K, F, KAPPA>) -> Self {
        let mut x = Secret_Int2Fl(a.rep(), ConstU64::<V>);
        x.set(
            1,
            &((*x.get_unchecked(1) - ClearModp::from(F as i64))
                * (ClearModp::from(1) - *x.get_unchecked(3))),
        );
        SecretFloat::set(x)
    }
}

/* Set the underlying representation to something */

impl<const V: u64, const P: u64, const DETECT_OVERFLOW: bool> ClearFloat<V, P, DETECT_OVERFLOW> {
    #[inline(always)]
    pub fn set(x: Array<ClearModp, 5>) -> ClearFloat<V, P, DETECT_OVERFLOW> {
        Self { param: x }
    }
}

impl<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
{
    #[inline(always)]
    pub fn set(x: Array<SecretModp, 5>) -> SecretFloat<V, P, KAPPA, DETECT_OVERFLOW> {
        Self { param: x }
    }
}

/* Get the underlying representation */

impl<'a, const V: u64, const P: u64, const DETECT_OVERFLOW: bool>
    ClearFloat<V, P, DETECT_OVERFLOW>
{
    #[inline(always)]
    pub fn v(&'a self) -> ClearModp {
        *self.param.get_unchecked(0)
    }
    #[inline(always)]
    pub fn p(&'a self) -> ClearModp {
        *self.param.get_unchecked(1)
    }
    #[inline(always)]
    pub fn z(&'a self) -> ClearModp {
        *self.param.get_unchecked(2)
    }
    #[inline(always)]
    pub fn s(&'a self) -> ClearModp {
        *self.param.get_unchecked(3)
    }
    #[inline(always)]
    pub fn err(&'a self) -> ClearModp {
        *self.param.get_unchecked(4)
    }
}

impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
{
    #[inline(always)]
    pub fn v(&'a self) -> SecretModp {
        *self.param.get_unchecked(0)
    }
    #[inline(always)]
    pub fn p(&'a self) -> SecretModp {
        *self.param.get_unchecked(1)
    }
    #[inline(always)]
    pub fn z(&'a self) -> SecretModp {
        *self.param.get_unchecked(2)
    }
    #[inline(always)]
    pub fn s(&'a self) -> SecretModp {
        *self.param.get_unchecked(3)
    }
    #[inline(always)]
    pub fn err(&'a self) -> SecretModp {
        *self.param.get_unchecked(4)
    }
}

/* Reveal Operation */

impl<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool> Reveal
    for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
{
    type Output = ClearFloat<V, P, DETECT_OVERFLOW>;
    #[inline(always)]
    fn reveal(&self) -> ClearFloat<V, P, DETECT_OVERFLOW> {
        let err_int: SecretInteger<V, KAPPA> = SecretInteger::from(self.err());
        let signal = err_int.eqz();

        let mut clear_param: Array<ClearModp, 5> = Array::uninitialized();
        clear_param.set(0, &(self.v() * signal).reveal());
        clear_param.set(1, &(self.p() * signal).reveal());
        clear_param.set(2, &(self.z() * signal).reveal());
        clear_param.set(3, &(self.s() * signal).reveal());
        clear_param.set(4, &(ClearModp::from(1) - signal).reveal());

        ClearFloat { param: clear_param }
    }
}

/* Addition and Subtraction */
// SecretFloat + SecretFloat (all 4 clone/reference combinations)
pub fn ssfloatadd<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>(
    v1: SecretModp,
    p1: SecretModp,
    z1: SecretModp,
    s1: SecretModp,
    err1: SecretModp,
    v2: SecretModp,
    p2: SecretModp,
    z2: SecretModp,
    s2: SecretModp,
    err2: SecretModp,
) -> Array<SecretModp, 5>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    let v1_int: SecretInteger<V, KAPPA> = SecretInteger::from(v1);
    let v2_int: SecretInteger<V, KAPPA> = SecretInteger::from(v2);
    let p1_int: SecretInteger<P, KAPPA> = SecretInteger::from(p1);
    let p2_int: SecretInteger<P, KAPPA> = SecretInteger::from(p2);

    let a: SecretModp = p1_int.lt(p2_int);
    let diff_p: SecretInteger<P, KAPPA> = SecretInteger::from(p1 - p2);
    let b: SecretModp = diff_p.eqz();
    let c: SecretModp = v1_int.lt(v2_int);
    let ap1: SecretModp = a * p1;
    let ap2: SecretModp = a * p2;
    let aneg = ClearModp::from(1) - a;
    let bneg = ClearModp::from(1) - b;
    let cneg = ClearModp::from(1) - c;
    let av1: SecretModp = a * v1;
    let av2: SecretModp = a * v2;
    let cv1: SecretModp = c * v1;
    let cv2: SecretModp = c * v2;

    let pmax: SecretModp = ap2 + p1 - ap1;
    let pmin: SecretModp = p2 - ap2 + ap1;
    let vmax: SecretModp = bneg * (av2 + v1 - av1) + b * (cv2 + v1 - cv1);
    let vmin: SecretModp = bneg * (av1 + v2 - av2) + b * (cv1 + v2 - cv2);
    let s3 = s1 + s2 - ClearModp::from(2) * s1 * s2;
    let ltz_in: SecretInteger<P, KAPPA> =
        SecretInteger::from(ClearModp::from(V as i64) + pmin - pmax);
    let d: SecretModp = ltz_in.ltz();
    let e: SecretModp = (ClearModp::from(1) - d) * (pmax - pmin);
    let pow_delta = Pow2::<V, KAPPA>(e);

    let v4 = vmax * pow_delta + (ClearModp::from(1) - ClearModp::from(2) * s3) * vmin;
    let v: SecretInteger<{ 2 * V }, KAPPA> = SecretInteger::from(
        (d * vmax + (ClearModp::from(1) - d) * v4)
            * modp_two_power(V)
            * Inv(SecretModp::from(pow_delta)),
    );

    let u = BitDec::<{ V + 1 }, { V + 1 }, KAPPA>(v.Trunc(V, true).rep());
    let u_reverse = u.reverse();
    let h = u_reverse.clone().PreOr();
    let p0 = ClearModp::from((V + 1) as i64) - h.evaluate(ClearModp::from(1));
    let mut p0_slice: Slice<SecretModp> = Slice::uninitialized(h.len());
    for i in 0..h.len() {
        p0_slice.set(
            i,
            &(modp_two_power(i) * (ClearModp::from(1) - *h.get(i).unwrap())),
        );
    }
    let pow_p0 = ClearModp::from(1) + p0_slice.evaluate(ClearModp::from(1));
    let mut v_trunc = SecretModp::from(0);
    for i in 0..u.len() {
        v_trunc = v_trunc + modp_two_power(i + 1) * *u.get(i).unwrap();
    }
    let t2: SecretInteger<{ V + 2 }, KAPPA> = SecretInteger::from(pow_p0 * v_trunc);
    let t2 = t2.Trunc(2, true);
    let p = pmax - p0 + ClearModp::from(1);
    let zz = z1 * z2;
    let zprod = ClearModp::from(1) - z1 - z2 + zz;
    let v = zprod * t2.rep() + z1 * v2 + z2 * v1;
    let vi: SecretInteger<V, KAPPA> = SecretInteger::from(v);
    let z = vi.eqz();
    let p = (zprod * p + z1 * p2 + z2 * p1) * (ClearModp::from(1) - z);
    let s = bneg * (a * s2 + aneg * s1) + b * (c * s2 + cneg * s1);
    let mut s = zprod * s + (z2 - zz) * s1 + (z1 - zz) * s2;
    // Normalize s
    s = s * (ClearModp::from(1) - z);
    let err = err1 + err2 + flow_detect::<V, P, KAPPA, DETECT_OVERFLOW>(p);

    let mut result: Array<SecretModp, 5> = Array::uninitialized();
    result.set(0, &v);
    result.set(1, &p);
    result.set(2, &z);
    result.set(3, &s);
    result.set(4, &err);

    result
}

impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Add<&'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>> for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn add(self, other: &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatadd::<V, P, KAPPA, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );
        SecretFloat { param: result }
    }
}
impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Add<&'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>>
    for &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn add(self, other: &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatadd::<V, P, KAPPA, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );
        SecretFloat { param: result }
    }
}
impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Add<SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>> for &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn add(self, other: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatadd::<V, P, KAPPA, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );
        SecretFloat { param: result }
    }
}
impl<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Add<SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>> for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn add(self, other: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatadd::<V, P, KAPPA, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );
        SecretFloat { param: result }
    }
}

// CLEAR FLOAT + CLEAR FLOAT (all 4 clone/reference combinations)
pub fn ccfloatadd<const V: u64, const P: u64, const DETECT_OVERFLOW: bool>(
    v1: ClearModp,
    p1: ClearModp,
    z1: ClearModp,
    s1: ClearModp,
    err1: ClearModp,
    v2: ClearModp,
    p2: ClearModp,
    z2: ClearModp,
    s2: ClearModp,
    err2: ClearModp,
) -> Array<ClearModp, 5>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ V + 2 }>: ,
{
    let v1_int: ClearInteger<V> = ClearInteger::from(v1);
    let v2_int: ClearInteger<V> = ClearInteger::from(v2);
    let p1_int: ClearInteger<P> = ClearInteger::from(p1);
    let p2_int: ClearInteger<P> = ClearInteger::from(p2);
    let a: ClearModp = p1_int.lt(p2_int);
    let diff_p: ClearInteger<P> = ClearInteger::from(p1 - p2);
    let b: ClearModp = (diff_p).eqz();
    let c: ClearModp = v1_int.lt(v2_int);
    let ap1: ClearModp = a * p1;
    let ap2: ClearModp = a * p2;
    let aneg = ClearModp::from(1) - a;
    let bneg = ClearModp::from(1) - b;
    let cneg = ClearModp::from(1) - c;
    let av1: ClearModp = a * v1;
    let av2: ClearModp = a * v2;
    let cv1: ClearModp = c * v1;
    let cv2: ClearModp = c * v2;
    let pmax: ClearModp = ap2 + p1 - ap1;
    let pmin: ClearModp = p2 - ap2 + ap1;
    let vmax: ClearModp = bneg * (av2 + v1 - av1) + b * (cv2 + v1 - cv1);
    let vmin: ClearModp = bneg * (av1 + v2 - av2) + b * (cv1 + v2 - cv2);
    let s3 = s1 + s2 - ClearModp::from(2) * s1 * s2;
    let ltz_in: ClearInteger<P> = ClearInteger::from(ClearModp::from(V as i64) + pmin - pmax);
    let d: ClearModp = ltz_in.ltz();
    let e: ClearModp = (ClearModp::from(1) - d) * (pmax - pmin);
    let pow_delta = ClearModp::from(1) << e;
    let v4 = vmax * pow_delta + (ClearModp::from(1) - ClearModp::from(2) * s3) * vmin;
    let v = (d * vmax + (ClearModp::from(1) - d) * v4)
        * modp_two_power(V)
        * Inv(SecretModp::from(pow_delta)).reveal();
    let u = Slice::bit_decomposition_ClearModp_Signed(v, 2 * V + 1);
    let u_reverse = u.reverse().slice(..V + 1);
    let h = u_reverse.PreOr();
    let p0 = ClearModp::from((V + 1) as i64) - h.evaluate(ClearModp::from(1));
    let mut p0_slice: Slice<ClearModp> = Slice::uninitialized(h.len());
    for i in 0..h.len() {
        p0_slice.set(
            i,
            &(modp_two_power(i) * (ClearModp::from(1) - *h.get(i).unwrap())),
        );
    }
    let mut v_trunc = ClearModp::from(0);
    for i in 0..(V + 1) {
        v_trunc = v_trunc + modp_two_power(i + 1) * *u.get(V + i).unwrap();
    }

    let pow_p0 = ClearModp::from(1) + p0_slice.evaluate(ClearModp::from(1));
    let t2: SecretInteger<{ V + 2 }, 40> = SecretInteger::from(pow_p0 * v_trunc);
    let t2 = t2.Trunc(2, true).reveal();
    let p = pmax - p0 + ClearModp::from(1);
    let zz = z1 * z2;
    let zprod = ClearModp::from(1) - z1 - z2 + zz;
    let v = zprod * t2.rep() + z1 * v2 + z2 * v1;
    let vi: ClearInteger<V> = ClearInteger::from(v);
    let z = vi.eqz();
    let p = (zprod * p + z1 * p2 + z2 * p1) * (ClearModp::from(1) - z);
    let s = bneg * (a * s2 + aneg * s1) + b * (c * s2 + cneg * s1);
    let mut s = zprod * s + (z2 - zz) * s1 + (z1 - zz) * s2;
    // Normalize s
    s = s * (ClearModp::from(1) - z);
    let err = err1 + err2 + flow_detect_c::<V, P, DETECT_OVERFLOW>(p);

    let mut result: Array<ClearModp, 5> = Array::uninitialized();
    result.set(0, &v);
    result.set(1, &p);
    result.set(2, &z);
    result.set(3, &s);
    result.set(4, &err);

    result
}

impl<'a, const V: u64, const P: u64, const DETECT_OVERFLOW: bool>
    Add<&'a ClearFloat<V, P, DETECT_OVERFLOW>> for ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = ClearFloat<V, P, DETECT_OVERFLOW>;
    #[inline(always)]
    fn add(self, other: &'a ClearFloat<V, P, DETECT_OVERFLOW>) -> Self::Output {
        let result = ccfloatadd::<V, P, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );
        ClearFloat { param: result }
    }
}
impl<'a, const V: u64, const P: u64, const DETECT_OVERFLOW: bool>
    Add<&'a ClearFloat<V, P, DETECT_OVERFLOW>> for &'a ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = ClearFloat<V, P, DETECT_OVERFLOW>;
    #[inline(always)]
    fn add(self, other: &'a ClearFloat<V, P, DETECT_OVERFLOW>) -> Self::Output {
        let result = ccfloatadd::<V, P, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );
        ClearFloat { param: result }
    }
}
impl<'a, const V: u64, const P: u64, const DETECT_OVERFLOW: bool>
    Add<ClearFloat<V, P, DETECT_OVERFLOW>> for &'a ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = ClearFloat<V, P, DETECT_OVERFLOW>;
    #[inline(always)]
    fn add(self, other: ClearFloat<V, P, DETECT_OVERFLOW>) -> Self::Output {
        let result = ccfloatadd::<V, P, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );
        ClearFloat { param: result }
    }
}
impl<const V: u64, const P: u64, const DETECT_OVERFLOW: bool> Add<ClearFloat<V, P, DETECT_OVERFLOW>>
    for ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = ClearFloat<V, P, DETECT_OVERFLOW>;
    #[inline(always)]
    fn add(self, other: ClearFloat<V, P, DETECT_OVERFLOW>) -> Self::Output {
        let result = ccfloatadd::<V, P, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );
        ClearFloat { param: result }
    }
}

// ClearFloat + SecretFloat (all 8 clone/reference combinations)
impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Add<&'a ClearFloat<V, P, DETECT_OVERFLOW>> for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn add(self, other: &'a ClearFloat<V, P, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatadd::<V, P, KAPPA, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            SecretModp::from(other.v()),
            SecretModp::from(other.p()),
            SecretModp::from(other.z()),
            SecretModp::from(other.s()),
            SecretModp::from(other.err()),
        );
        SecretFloat { param: result }
    }
}
impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Add<&'a ClearFloat<V, P, DETECT_OVERFLOW>> for &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn add(self, other: &'a ClearFloat<V, P, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatadd::<V, P, KAPPA, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            SecretModp::from(other.v()),
            SecretModp::from(other.p()),
            SecretModp::from(other.z()),
            SecretModp::from(other.s()),
            SecretModp::from(other.err()),
        );
        SecretFloat { param: result }
    }
}
impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Add<ClearFloat<V, P, DETECT_OVERFLOW>> for &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn add(self, other: ClearFloat<V, P, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatadd::<V, P, KAPPA, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            SecretModp::from(other.v()),
            SecretModp::from(other.p()),
            SecretModp::from(other.z()),
            SecretModp::from(other.s()),
            SecretModp::from(other.err()),
        );
        SecretFloat { param: result }
    }
}
impl<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Add<ClearFloat<V, P, DETECT_OVERFLOW>> for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn add(self, other: ClearFloat<V, P, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatadd::<V, P, KAPPA, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            SecretModp::from(other.v()),
            SecretModp::from(other.p()),
            SecretModp::from(other.z()),
            SecretModp::from(other.s()),
            SecretModp::from(other.err()),
        );
        SecretFloat { param: result }
    }
}

impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Add<&'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>> for ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn add(self, other: &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatadd::<V, P, KAPPA, DETECT_OVERFLOW>(
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
            SecretModp::from(self.v()),
            SecretModp::from(self.p()),
            SecretModp::from(self.z()),
            SecretModp::from(self.s()),
            SecretModp::from(self.err()),
        );
        SecretFloat { param: result }
    }
}
impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Add<&'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>> for &'a ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn add(self, other: &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatadd::<V, P, KAPPA, DETECT_OVERFLOW>(
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
            SecretModp::from(self.v()),
            SecretModp::from(self.p()),
            SecretModp::from(self.z()),
            SecretModp::from(self.s()),
            SecretModp::from(self.err()),
        );
        SecretFloat { param: result }
    }
}
impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Add<SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>> for &'a ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn add(self, other: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatadd::<V, P, KAPPA, DETECT_OVERFLOW>(
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
            SecretModp::from(self.v()),
            SecretModp::from(self.p()),
            SecretModp::from(self.z()),
            SecretModp::from(self.s()),
            SecretModp::from(self.err()),
        );
        SecretFloat { param: result }
    }
}
impl<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Add<SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>> for ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn add(self, other: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatadd::<V, P, KAPPA, DETECT_OVERFLOW>(
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
            SecretModp::from(self.v()),
            SecretModp::from(self.p()),
            SecretModp::from(self.z()),
            SecretModp::from(self.s()),
            SecretModp::from(self.err()),
        );
        SecretFloat { param: result }
    }
}

impl<const V: u64, const P: u64, const DETECT_OVERFLOW: bool> Neg
    for ClearFloat<V, P, DETECT_OVERFLOW>
{
    type Output = ClearFloat<V, P, DETECT_OVERFLOW>;
    #[inline(always)]
    fn neg(self) -> ClearFloat<V, P, DETECT_OVERFLOW> {
        let mut param_new: Array<ClearModp, 5> = Array::uninitialized();
        let mut s = ClearModp::from(1) - self.s();
        s = s * (ClearModp::from(1) - self.z());
        param_new.set(0, &self.v());
        param_new.set(1, &self.p());
        param_new.set(2, &self.z());
        param_new.set(3, &s);
        param_new.set(4, &self.err());
        ClearFloat { param: param_new }
    }
}
impl<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool> Neg
    for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn neg(self) -> SecretFloat<V, P, KAPPA, DETECT_OVERFLOW> {
        let mut param_new: Array<SecretModp, 5> = Array::uninitialized();
        let mut s = ClearModp::from(1) - self.s();
        s = s * (ClearModp::from(1) - self.z());
        param_new.set(0, &self.v());
        param_new.set(1, &self.p());
        param_new.set(2, &self.z());
        param_new.set(3, &s);
        param_new.set(4, &self.err());
        SecretFloat { param: param_new }
    }
}
impl<'a, const V: u64, const P: u64, const DETECT_OVERFLOW: bool> Neg
    for &'a ClearFloat<V, P, DETECT_OVERFLOW>
{
    type Output = ClearFloat<V, P, DETECT_OVERFLOW>;
    #[inline(always)]
    fn neg(self) -> ClearFloat<V, P, DETECT_OVERFLOW> {
        let mut param_new: Array<ClearModp, 5> = Array::uninitialized();
        param_new.set(0, &self.v());
        param_new.set(1, &self.p());
        param_new.set(2, &self.z());
        param_new.set(3, &(ClearModp::from(1) - self.s()));
        param_new.set(4, &self.err());
        ClearFloat { param: param_new }
    }
}
impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool> Neg
    for &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn neg(self) -> SecretFloat<V, P, KAPPA, DETECT_OVERFLOW> {
        let mut param_new: Array<SecretModp, 5> = Array::uninitialized();
        param_new.set(0, &self.v());
        param_new.set(1, &self.p());
        param_new.set(2, &self.z());
        param_new.set(3, &(ClearModp::from(1) - self.s()));
        param_new.set(4, &self.err());
        SecretFloat { param: param_new }
    }
}

impl<'a, const V: u64, const P: u64, const DETECT_OVERFLOW: bool>
    Sub<&'a ClearFloat<V, P, DETECT_OVERFLOW>> for &'a ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = ClearFloat<V, P, DETECT_OVERFLOW>;
    #[inline(always)]
    fn sub(
        self,
        other: &'a ClearFloat<V, P, DETECT_OVERFLOW>,
    ) -> ClearFloat<V, P, DETECT_OVERFLOW> {
        self + (-other)
    }
}
impl<'a, const V: u64, const P: u64, const DETECT_OVERFLOW: bool>
    Sub<&'a ClearFloat<V, P, DETECT_OVERFLOW>> for ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = ClearFloat<V, P, DETECT_OVERFLOW>;
    #[inline(always)]
    fn sub(
        self,
        other: &'a ClearFloat<V, P, DETECT_OVERFLOW>,
    ) -> ClearFloat<V, P, DETECT_OVERFLOW> {
        self + (-other)
    }
}

impl<'a, const V: u64, const P: u64, const DETECT_OVERFLOW: bool>
    Sub<ClearFloat<V, P, DETECT_OVERFLOW>> for &'a ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = ClearFloat<V, P, DETECT_OVERFLOW>;
    #[inline(always)]
    fn sub(self, other: ClearFloat<V, P, DETECT_OVERFLOW>) -> ClearFloat<V, P, DETECT_OVERFLOW> {
        self + (-other)
    }
}

impl<const V: u64, const P: u64, const DETECT_OVERFLOW: bool> Sub<ClearFloat<V, P, DETECT_OVERFLOW>>
    for ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = ClearFloat<V, P, DETECT_OVERFLOW>;
    #[inline(always)]
    fn sub(self, other: ClearFloat<V, P, DETECT_OVERFLOW>) -> ClearFloat<V, P, DETECT_OVERFLOW> {
        self + (-other)
    }
}

impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Sub<&'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>>
    for &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn sub(
        self,
        other: &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>,
    ) -> SecretFloat<V, P, KAPPA, DETECT_OVERFLOW> {
        self + (-other)
    }
}

impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Sub<&'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>> for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn sub(
        self,
        other: &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>,
    ) -> SecretFloat<V, P, KAPPA, DETECT_OVERFLOW> {
        self + (-other)
    }
}

impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Sub<SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>> for &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn sub(
        self,
        other: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>,
    ) -> SecretFloat<V, P, KAPPA, DETECT_OVERFLOW> {
        self + (-other)
    }
}

impl<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Sub<SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>> for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn sub(
        self,
        other: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>,
    ) -> SecretFloat<V, P, KAPPA, DETECT_OVERFLOW> {
        self + (-other)
    }
}

impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Sub<&'a ClearFloat<V, P, DETECT_OVERFLOW>> for &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn sub(
        self,
        other: &'a ClearFloat<V, P, DETECT_OVERFLOW>,
    ) -> SecretFloat<V, P, KAPPA, DETECT_OVERFLOW> {
        self + (-other)
    }
}

impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Sub<&'a ClearFloat<V, P, DETECT_OVERFLOW>> for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn sub(
        self,
        other: &'a ClearFloat<V, P, DETECT_OVERFLOW>,
    ) -> SecretFloat<V, P, KAPPA, DETECT_OVERFLOW> {
        self + (-other)
    }
}

impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Sub<ClearFloat<V, P, DETECT_OVERFLOW>> for &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn sub(
        self,
        other: ClearFloat<V, P, DETECT_OVERFLOW>,
    ) -> SecretFloat<V, P, KAPPA, DETECT_OVERFLOW> {
        self + (-other)
    }
}

impl<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Sub<ClearFloat<V, P, DETECT_OVERFLOW>> for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn sub(
        self,
        other: ClearFloat<V, P, DETECT_OVERFLOW>,
    ) -> SecretFloat<V, P, KAPPA, DETECT_OVERFLOW> {
        self + (-other)
    }
}

impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Sub<&'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>> for &'a ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn sub(
        self,
        other: &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>,
    ) -> SecretFloat<V, P, KAPPA, DETECT_OVERFLOW> {
        self + (-other)
    }
}

impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Sub<&'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>> for ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn sub(
        self,
        other: &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>,
    ) -> SecretFloat<V, P, KAPPA, DETECT_OVERFLOW> {
        self + (-other)
    }
}

impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Sub<SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>> for &'a ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn sub(
        self,
        other: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>,
    ) -> SecretFloat<V, P, KAPPA, DETECT_OVERFLOW> {
        self + (-other)
    }
}

impl<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Sub<SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>> for ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn sub(
        self,
        other: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>,
    ) -> SecretFloat<V, P, KAPPA, DETECT_OVERFLOW> {
        self + (-other)
    }
}

/* Multiplication */
// SECRETFLOAT * SECRETFLOAT
pub fn ssfloatmul<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>(
    v1: SecretModp,
    p1: SecretModp,
    z1: SecretModp,
    s1: SecretModp,
    err1: SecretModp,
    v2: SecretModp,
    p2: SecretModp,
    z2: SecretModp,
    s2: SecretModp,
    err2: SecretModp,
) -> Array<SecretModp, 5>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
{
    let c2expl = modp_two_power(V);
    let u1: SecretInteger<{ 2 * V }, KAPPA> = SecretInteger::from(v1 * v2);
    let u1 = u1.Trunc(V - 1, true).rep();
    let t: SecretInteger<{ V + 1 }, KAPPA> = SecretInteger::from(u1 - c2expl);
    let b = t.ltz();

    let u2: SecretInteger<{ V + 1 }, KAPPA> = SecretInteger::from(u1 + b * u1);
    let u2 = u2.Trunc(1, true).rep();

    let mut res: Array<SecretModp, 5> = Array::uninitialized();
    let z = z1 + z2 - z1 * z2;
    let p = (p1 + p2 - b + ClearModp::from(V as i64)) * (ClearModp::from(1) - z);
    let mut s = s1 + s2 - ClearModp::from(2) * s1 * s2;
    // Normalize s
    s = s * (ClearModp::from(1) - z);
    res.set(0, &u2);
    res.set(1, &p);
    res.set(2, &z);
    res.set(3, &s);
    res.set(
        4,
        &(err1 + err2 + flow_detect::<V, P, KAPPA, DETECT_OVERFLOW>(p)),
    );

    res
}

impl<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Mul<SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>> for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn mul(self, other: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatmul::<V, P, KAPPA, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );
        SecretFloat { param: result }
    }
}

impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Mul<&'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>> for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn mul(self, other: &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatmul::<V, P, KAPPA, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );
        SecretFloat { param: result }
    }
}

impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Mul<SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>> for &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn mul(self, other: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatmul::<V, P, KAPPA, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );
        SecretFloat { param: result }
    }
}

impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Mul<&'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>>
    for &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn mul(self, other: &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatmul::<V, P, KAPPA, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );
        SecretFloat { param: result }
    }
}

// CLEARFLOAT * CLEARFLOAT
pub fn ccfloatmul<const V: u64, const P: u64, const DETECT_OVERFLOW: bool>(
    v1: ClearModp,
    p1: ClearModp,
    z1: ClearModp,
    s1: ClearModp,
    err1: ClearModp,
    v2: ClearModp,
    p2: ClearModp,
    z2: ClearModp,
    s2: ClearModp,
    err2: ClearModp,
) -> Array<ClearModp, 5>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
{
    let c2expl = modp_two_power(V);
    let u1: ClearInteger<{ 2 * V }> = ClearInteger::from(v1 * v2);
    let u1 = u1.Trunc(V - 1, true).rep();
    let t: ClearInteger<{ V + 1 }> = ClearInteger::from(u1 - c2expl);
    let b = t.ltz();

    let u2: ClearInteger<{ V + 1 }> = ClearInteger::from(u1 + b * u1);
    let u2 = u2.Trunc(1, true).rep();

    let mut res: Array<ClearModp, 5> = Array::uninitialized();
    let z = z1 + z2 - z1 * z2;
    let p = (p1 + p2 - b + ClearModp::from(V as i64)) * (ClearModp::from(1) - z);
    let mut s = s1 + s2 - ClearModp::from(2) * s1 * s2;
    // Normalize s
    s = s * (ClearModp::from(1) - z);
    res.set(0, &u2);
    res.set(1, &p);
    res.set(2, &z);
    res.set(3, &s);
    res.set(
        4,
        &(err1 + err2 + flow_detect_c::<V, P, DETECT_OVERFLOW>(p)),
    );

    res
}

impl<const V: u64, const P: u64, const DETECT_OVERFLOW: bool> Mul<ClearFloat<V, P, DETECT_OVERFLOW>>
    for ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
{
    type Output = ClearFloat<V, P, DETECT_OVERFLOW>;
    #[inline(always)]
    fn mul(self, other: ClearFloat<V, P, DETECT_OVERFLOW>) -> Self::Output {
        let result = ccfloatmul::<V, P, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );
        ClearFloat { param: result }
    }
}

impl<'a, const V: u64, const P: u64, const DETECT_OVERFLOW: bool>
    Mul<&'a ClearFloat<V, P, DETECT_OVERFLOW>> for ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
{
    type Output = ClearFloat<V, P, DETECT_OVERFLOW>;
    #[inline(always)]
    fn mul(self, other: &'a ClearFloat<V, P, DETECT_OVERFLOW>) -> Self::Output {
        let result = ccfloatmul::<V, P, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );
        ClearFloat { param: result }
    }
}

impl<'a, const V: u64, const P: u64, const DETECT_OVERFLOW: bool>
    Mul<&'a ClearFloat<V, P, DETECT_OVERFLOW>> for &'a ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
{
    type Output = ClearFloat<V, P, DETECT_OVERFLOW>;
    #[inline(always)]
    fn mul(self, other: &'a ClearFloat<V, P, DETECT_OVERFLOW>) -> Self::Output {
        let result = ccfloatmul::<V, P, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );
        ClearFloat { param: result }
    }
}
impl<'a, const V: u64, const P: u64, const DETECT_OVERFLOW: bool>
    Mul<ClearFloat<V, P, DETECT_OVERFLOW>> for &'a ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
{
    type Output = ClearFloat<V, P, DETECT_OVERFLOW>;
    #[inline(always)]
    fn mul(self, other: ClearFloat<V, P, DETECT_OVERFLOW>) -> Self::Output {
        let result = ccfloatmul::<V, P, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );
        ClearFloat { param: result }
    }
}

impl<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Mul<SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>> for ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn mul(self, other: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatmul::<V, P, KAPPA, DETECT_OVERFLOW>(
            SecretModp::from(self.v()),
            SecretModp::from(self.p()),
            SecretModp::from(self.z()),
            SecretModp::from(self.s()),
            SecretModp::from(self.err()),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );
        SecretFloat { param: result }
    }
}

impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Mul<&'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>> for ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn mul(self, other: &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatmul::<V, P, KAPPA, DETECT_OVERFLOW>(
            SecretModp::from(self.v()),
            SecretModp::from(self.p()),
            SecretModp::from(self.z()),
            SecretModp::from(self.s()),
            SecretModp::from(self.err()),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );
        SecretFloat { param: result }
    }
}

impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Mul<SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>> for &'a ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn mul(self, other: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatmul::<V, P, KAPPA, DETECT_OVERFLOW>(
            SecretModp::from(self.v()),
            SecretModp::from(self.p()),
            SecretModp::from(self.z()),
            SecretModp::from(self.s()),
            SecretModp::from(self.err()),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );
        SecretFloat { param: result }
    }
}
impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Mul<&'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>> for &'a ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn mul(self, other: &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatmul::<V, P, KAPPA, DETECT_OVERFLOW>(
            SecretModp::from(self.v()),
            SecretModp::from(self.p()),
            SecretModp::from(self.z()),
            SecretModp::from(self.s()),
            SecretModp::from(self.err()),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );
        SecretFloat { param: result }
    }
}

impl<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Mul<ClearFloat<V, P, DETECT_OVERFLOW>> for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn mul(self, other: ClearFloat<V, P, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatmul::<V, P, KAPPA, DETECT_OVERFLOW>(
            SecretModp::from(other.v()),
            SecretModp::from(other.p()),
            SecretModp::from(other.z()),
            SecretModp::from(other.s()),
            SecretModp::from(other.err()),
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
        );
        SecretFloat { param: result }
    }
}
impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Mul<&'a ClearFloat<V, P, DETECT_OVERFLOW>> for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn mul(self, other: &'a ClearFloat<V, P, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatmul::<V, P, KAPPA, DETECT_OVERFLOW>(
            SecretModp::from(other.v()),
            SecretModp::from(other.p()),
            SecretModp::from(other.z()),
            SecretModp::from(other.s()),
            SecretModp::from(other.err()),
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
        );
        SecretFloat { param: result }
    }
}

impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Mul<ClearFloat<V, P, DETECT_OVERFLOW>> for &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn mul(self, other: ClearFloat<V, P, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatmul::<V, P, KAPPA, DETECT_OVERFLOW>(
            SecretModp::from(other.v()),
            SecretModp::from(other.p()),
            SecretModp::from(other.z()),
            SecretModp::from(other.s()),
            SecretModp::from(other.err()),
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
        );
        SecretFloat { param: result }
    }
}
impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Mul<&'a ClearFloat<V, P, DETECT_OVERFLOW>> for &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn mul(self, other: &'a ClearFloat<V, P, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatmul::<V, P, KAPPA, DETECT_OVERFLOW>(
            SecretModp::from(other.v()),
            SecretModp::from(other.p()),
            SecretModp::from(other.z()),
            SecretModp::from(other.s()),
            SecretModp::from(other.err()),
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
        );
        SecretFloat { param: result }
    }
}

#[allow(non_snake_case)]
pub fn SDiv_ABZS12<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>(
    a: SecretModp,
    b: SecretModp,
) -> SecretModp
where
    ConstU64<{ 2 * V + 1 }>: ,
{
    let theta: u64 = ceil_log_2(V as u32).into();
    let mut x = b;
    let mut y = a;
    let two_pow_v = ClearModp::from(1) << ClearModp::from((V + 1) as i64);
    for _i in 0..(theta - 1) {
        let y_int: SecretInteger<{ 2 * V + 1 }, KAPPA> = SecretInteger::from(y * (two_pow_v - x));
        y = y_int.TruncPr(V, true).rep();
        let x_int: SecretInteger<{ 2 * V + 1 }, KAPPA> = SecretInteger::from(x * (two_pow_v - x));
        x = x_int.TruncPr(V, true).rep();
    }
    let y_int: SecretInteger<{ 2 * V + 1 }, KAPPA> = SecretInteger::from(y * (two_pow_v - x));
    let y: SecretModp = y_int.TruncPr(V, true).rep();

    y
}

#[allow(non_snake_case)]
pub fn ssfloatdiv<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>(
    v1: SecretModp,
    p1: SecretModp,
    z1: SecretModp,
    s1: SecretModp,
    err1: SecretModp,
    v2: SecretModp,
    p2: SecretModp,
    z2: SecretModp,
    s2: SecretModp,
    err2: SecretModp,
) -> Array<SecretModp, 5>
// ~ local_division_ABSZ12() in mamba
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V + 1 }>: ,
{
    let v = SDiv_ABZS12::<V, P, KAPPA, DETECT_OVERFLOW>(v1, v2 + z2);
    let max = modp_two_power(V);
    let v_int: SecretInteger<{ V + 1 }, KAPPA> = SecretInteger::from(v - max);
    // let b = v_int.ltz(); Causes a problem so expanded it here...
    let b = -v_int.Trunc(V, true).rep();
    let t = v * b + v;

    let t_int: SecretInteger<{ V + 1 }, KAPPA> = SecretInteger::from(t);
    let ObTr = t_int.ObliviousTrunc(SecretModp::from(1));
    let v = *ObTr.get_unchecked(0);
    let p =
        (ClearModp::from(1) - z1) * (p1 - p2 - ClearModp::from(V as i64) + ClearModp::from(1) - b);
    let s = s1 + s2 - ClearModp::from(2) * s1 * s2;
    let err = err1 + err2 + flow_detect::<V, P, KAPPA, DETECT_OVERFLOW>(p) + z2;

    let mut result: Array<SecretModp, 5> = Array::uninitialized();
    result.set(0, &v);
    result.set(1, &p);
    result.set(2, &z1);
    result.set(3, &s);
    result.set(4, &err);

    result
}

impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Div<&'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>>
    for &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V + 1 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn div(self, other: &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatdiv::<V, P, KAPPA, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );

        SecretFloat { param: result }
    }
}
impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Div<&'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>> for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V + 1 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn div(self, other: &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatdiv::<V, P, KAPPA, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );

        SecretFloat { param: result }
    }
}
impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Div<SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>> for &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V + 1 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn div(self, other: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatdiv::<V, P, KAPPA, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );

        SecretFloat { param: result }
    }
}
impl<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Div<SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>> for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V + 1 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn div(self, other: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatdiv::<V, P, KAPPA, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );

        SecretFloat { param: result }
    }
}

impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Div<&'a ClearFloat<V, P, DETECT_OVERFLOW>> for &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V + 1 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn div(self, other: &'a ClearFloat<V, P, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatdiv::<V, P, KAPPA, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            SecretModp::from(other.v()),
            SecretModp::from(other.p()),
            SecretModp::from(other.z()),
            SecretModp::from(other.s()),
            SecretModp::from(other.err()),
        );

        SecretFloat { param: result }
    }
}
impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Div<&'a ClearFloat<V, P, DETECT_OVERFLOW>> for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V + 1 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn div(self, other: &'a ClearFloat<V, P, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatdiv::<V, P, KAPPA, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            SecretModp::from(other.v()),
            SecretModp::from(other.p()),
            SecretModp::from(other.z()),
            SecretModp::from(other.s()),
            SecretModp::from(other.err()),
        );

        SecretFloat { param: result }
    }
}
impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Div<ClearFloat<V, P, DETECT_OVERFLOW>> for &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V + 1 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn div(self, other: ClearFloat<V, P, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatdiv::<V, P, KAPPA, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            SecretModp::from(other.v()),
            SecretModp::from(other.p()),
            SecretModp::from(other.z()),
            SecretModp::from(other.s()),
            SecretModp::from(other.err()),
        );

        SecretFloat { param: result }
    }
}
impl<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Div<ClearFloat<V, P, DETECT_OVERFLOW>> for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V + 1 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn div(self, other: ClearFloat<V, P, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatdiv::<V, P, KAPPA, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            SecretModp::from(other.v()),
            SecretModp::from(other.p()),
            SecretModp::from(other.z()),
            SecretModp::from(other.s()),
            SecretModp::from(other.err()),
        );

        SecretFloat { param: result }
    }
}
impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Div<&'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>> for &'a ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V + 1 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn div(self, other: &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatdiv::<V, P, KAPPA, DETECT_OVERFLOW>(
            SecretModp::from(self.v()),
            SecretModp::from(self.p()),
            SecretModp::from(self.z()),
            SecretModp::from(self.s()),
            SecretModp::from(self.err()),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );

        SecretFloat { param: result }
    }
}
impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Div<&'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>> for ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V + 1 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn div(self, other: &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatdiv::<V, P, KAPPA, DETECT_OVERFLOW>(
            SecretModp::from(self.v()),
            SecretModp::from(self.p()),
            SecretModp::from(self.z()),
            SecretModp::from(self.s()),
            SecretModp::from(self.err()),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );

        SecretFloat { param: result }
    }
}
impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Div<SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>> for &'a ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V + 1 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn div(self, other: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatdiv::<V, P, KAPPA, DETECT_OVERFLOW>(
            SecretModp::from(self.v()),
            SecretModp::from(self.p()),
            SecretModp::from(self.z()),
            SecretModp::from(self.s()),
            SecretModp::from(self.err()),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );

        SecretFloat { param: result }
    }
}
impl<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    Div<SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>> for ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V + 1 }>: ,
{
    type Output = SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>;
    #[inline(always)]
    fn div(self, other: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> Self::Output {
        let result = ssfloatdiv::<V, P, KAPPA, DETECT_OVERFLOW>(
            SecretModp::from(self.v()),
            SecretModp::from(self.p()),
            SecretModp::from(self.z()),
            SecretModp::from(self.s()),
            SecretModp::from(self.err()),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );

        SecretFloat { param: result }
    }
}

#[allow(non_snake_case)]
pub fn CDiv_ABZS12<const V: u64, const P: u64, const DETECT_OVERFLOW: bool>(
    a: ClearModp,
    b: ClearModp,
) -> ClearModp
where
    ConstU64<{ 2 * V + 1 }>: ,
{
    let theta: u64 = ceil_log_2(V as u32).into();
    let mut x = b;
    let mut y = a;
    let two_pow_v = ClearModp::from(1) << ClearModp::from((V + 1) as i64);
    for _i in 0..(theta - 1) {
        let y_int: ClearInteger<{ 2 * V + 1 }> = ClearInteger::from(y * (two_pow_v - x));
        y = y_int.Trunc(V, true).rep();
        let x_int: ClearInteger<{ 2 * V + 1 }> = ClearInteger::from(x * (two_pow_v - x));
        x = x_int.Trunc(V, true).rep();
    }
    let y_int: ClearInteger<{ 2 * V + 1 }> = ClearInteger::from(y * (two_pow_v - x));
    let y: ClearModp = y_int.Trunc(V, true).rep();

    y
}
pub fn ccfloatdiv<const V: u64, const P: u64, const DETECT_OVERFLOW: bool>(
    v1: ClearModp,
    p1: ClearModp,
    z1: ClearModp,
    s1: ClearModp,
    err1: ClearModp,
    v2: ClearModp,
    p2: ClearModp,
    z2: ClearModp,
    s2: ClearModp,
    err2: ClearModp,
) -> Array<ClearModp, 5>
// ~ local_division_ABSZ12() in mamba
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V + 1 }>: ,
{
    let v = CDiv_ABZS12::<V, P, DETECT_OVERFLOW>(v1, v2 + z2);
    let max = modp_two_power(V);
    let v_int: ClearInteger<{ V + 1 }> = ClearInteger::from(v - max);
    // let b = v_int.ltz(); Causes a problem so expanded it here...
    let b = -v_int.Trunc(V, true).rep();
    let t = v * b + v;

    let t_int: ClearInteger<{ V + 1 }> = ClearInteger::from(t);
    let v: ClearModp = t_int.Trunc(1, true).rep();
    let p =
        (ClearModp::from(1) - z1) * (p1 - p2 - ClearModp::from(V as i64) + ClearModp::from(1) - b);
    let s = s1 + s2 - ClearModp::from(2) * s1 * s2;
    let err = err1 + err2 + flow_detect_c::<V, P, DETECT_OVERFLOW>(p) + z2;

    let mut result: Array<ClearModp, 5> = Array::uninitialized();
    result.set(0, &v);
    result.set(1, &p);
    result.set(2, &z1);
    result.set(3, &s);
    result.set(4, &err);

    result
}

impl<'a, const V: u64, const P: u64, const DETECT_OVERFLOW: bool>
    Div<&'a ClearFloat<V, P, DETECT_OVERFLOW>> for &'a ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V + 1 }>: ,
{
    type Output = ClearFloat<V, P, DETECT_OVERFLOW>;
    #[inline(always)]
    fn div(self, other: &'a ClearFloat<V, P, DETECT_OVERFLOW>) -> Self::Output {
        let result = ccfloatdiv::<V, P, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );

        ClearFloat { param: result }
    }
}
impl<'a, const V: u64, const P: u64, const DETECT_OVERFLOW: bool>
    Div<ClearFloat<V, P, DETECT_OVERFLOW>> for &'a ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V + 1 }>: ,
{
    type Output = ClearFloat<V, P, DETECT_OVERFLOW>;
    #[inline(always)]
    fn div(self, other: ClearFloat<V, P, DETECT_OVERFLOW>) -> Self::Output {
        let result = ccfloatdiv::<V, P, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );

        ClearFloat { param: result }
    }
}
impl<'a, const V: u64, const P: u64, const DETECT_OVERFLOW: bool>
    Div<&'a ClearFloat<V, P, DETECT_OVERFLOW>> for ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V + 1 }>: ,
{
    type Output = ClearFloat<V, P, DETECT_OVERFLOW>;
    #[inline(always)]
    fn div(self, other: &'a ClearFloat<V, P, DETECT_OVERFLOW>) -> Self::Output {
        let result = ccfloatdiv::<V, P, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );

        ClearFloat { param: result }
    }
}
impl<const V: u64, const P: u64, const DETECT_OVERFLOW: bool> Div<ClearFloat<V, P, DETECT_OVERFLOW>>
    for ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V + 1 }>: ,
{
    type Output = ClearFloat<V, P, DETECT_OVERFLOW>;
    #[inline(always)]
    fn div(self, other: ClearFloat<V, P, DETECT_OVERFLOW>) -> Self::Output {
        let result = ccfloatdiv::<V, P, DETECT_OVERFLOW>(
            self.v(),
            self.p(),
            self.z(),
            self.s(),
            self.err(),
            other.v(),
            other.p(),
            other.z(),
            other.s(),
            other.err(),
        );

        ClearFloat { param: result }
    }
}

// ScaleCmpZ and ScaleCmp:
impl<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    ScaleCmpZ<SecretModp> for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
{
    #[inline(always)]
    fn ltz(self) -> SecretModp {
        self.s()
    }
    #[inline(always)]
    fn lez(self) -> SecretModp {
        self.s() + self.z()
    }
    #[inline(always)]
    fn gtz(self) -> SecretModp {
        ClearModp::from(1) - self.s() - self.z()
    }
    #[inline(always)]
    fn gez(self) -> SecretModp {
        ClearModp::from(1) - self.s()
    }
    #[inline(always)]
    fn eqz(self) -> SecretModp {
        self.z()
    }
    #[inline(always)]
    fn nez(self) -> SecretModp {
        ClearModp::from(1) - self.z()
    }
}

impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    ScaleCmpZ<SecretModp> for &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
{
    #[inline(always)]
    fn ltz(self) -> SecretModp {
        self.s()
    }
    #[inline(always)]
    fn lez(self) -> SecretModp {
        self.s() + self.z()
    }
    #[inline(always)]
    fn gtz(self) -> SecretModp {
        ClearModp::from(1) - self.s() - self.z()
    }
    #[inline(always)]
    fn gez(self) -> SecretModp {
        ClearModp::from(1) - self.s()
    }
    #[inline(always)]
    fn eqz(self) -> SecretModp {
        self.z()
    }
    #[inline(always)]
    fn nez(self) -> SecretModp {
        ClearModp::from(1) - self.z()
    }
}

impl<const V: u64, const P: u64, const DETECT_OVERFLOW: bool> ScaleCmpZ<ClearModp>
    for ClearFloat<V, P, DETECT_OVERFLOW>
{
    #[inline(always)]
    fn ltz(self) -> ClearModp {
        self.s()
    }
    #[inline(always)]
    fn lez(self) -> ClearModp {
        self.s() + self.z()
    }
    #[inline(always)]
    fn gtz(self) -> ClearModp {
        ClearModp::from(1) - self.s() - self.z()
    }
    #[inline(always)]
    fn gez(self) -> ClearModp {
        ClearModp::from(1) - self.s()
    }
    #[inline(always)]
    fn eqz(self) -> ClearModp {
        self.z()
    }
    #[inline(always)]
    fn nez(self) -> ClearModp {
        ClearModp::from(1) - self.z()
    }
}

impl<'a, const V: u64, const P: u64, const DETECT_OVERFLOW: bool> ScaleCmpZ<ClearModp>
    for &'a ClearFloat<V, P, DETECT_OVERFLOW>
{
    #[inline(always)]
    fn ltz(self) -> ClearModp {
        self.s()
    }
    #[inline(always)]
    fn lez(self) -> ClearModp {
        self.s() + self.z()
    }
    #[inline(always)]
    fn gtz(self) -> ClearModp {
        ClearModp::from(1) - self.s() - self.z()
    }
    #[inline(always)]
    fn gez(self) -> ClearModp {
        ClearModp::from(1) - self.s()
    }
    #[inline(always)]
    fn eqz(self) -> ClearModp {
        self.z()
    }
    #[inline(always)]
    fn nez(self) -> ClearModp {
        ClearModp::from(1) - self.z()
    }
}

// SecretFloat cmp SecretFloat (4 options with ref / clone)
impl<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    ScaleCmp<SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>, SecretModp>
    for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    #[inline(always)]
    fn lt(self, other: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> SecretModp {
        (self - other).ltz()
    }
    #[inline(always)]
    fn le(self, other: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> SecretModp {
        (self - other).lez()
    }
    #[inline(always)]
    fn gt(self, other: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> SecretModp {
        (self - other).gtz()
    }
    #[inline(always)]
    fn ge(self, other: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> SecretModp {
        (self - other).gez()
    }
    #[inline(always)]
    fn eq(self, other: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> SecretModp {
        (self - other).eqz()
    }
    #[inline(always)]
    fn ne(self, other: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> SecretModp {
        (self - other).nez()
    }
}

impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    ScaleCmp<SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>, SecretModp>
    for &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    #[inline(always)]
    fn lt(self, other: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> SecretModp {
        (self - other).ltz()
    }
    #[inline(always)]
    fn le(self, other: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> SecretModp {
        (self - other).lez()
    }
    #[inline(always)]
    fn gt(self, other: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> SecretModp {
        (self - other).gtz()
    }
    #[inline(always)]
    fn ge(self, other: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> SecretModp {
        (self - other).gez()
    }
    #[inline(always)]
    fn eq(self, other: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> SecretModp {
        (self - other).eqz()
    }
    #[inline(always)]
    fn ne(self, other: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> SecretModp {
        (self - other).nez()
    }
}

impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    ScaleCmp<&'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>, SecretModp>
    for &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    #[inline(always)]
    fn lt(self, other: &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> SecretModp {
        (self - other).ltz()
    }
    #[inline(always)]
    fn le(self, other: &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> SecretModp {
        (self - other).lez()
    }
    #[inline(always)]
    fn gt(self, other: &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> SecretModp {
        (self - other).gtz()
    }
    #[inline(always)]
    fn ge(self, other: &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> SecretModp {
        (self - other).gez()
    }
    #[inline(always)]
    fn eq(self, other: &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> SecretModp {
        (self - other).eqz()
    }
    #[inline(always)]
    fn ne(self, other: &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> SecretModp {
        (self - other).nez()
    }
}

impl<'a, const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    ScaleCmp<&'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>, SecretModp>
    for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    #[inline(always)]
    fn lt(self, other: &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> SecretModp {
        (self - other).ltz()
    }
    #[inline(always)]
    fn le(self, other: &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> SecretModp {
        (self - other).lez()
    }
    #[inline(always)]
    fn gt(self, other: &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> SecretModp {
        (self - other).gtz()
    }
    #[inline(always)]
    fn ge(self, other: &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> SecretModp {
        (self - other).gez()
    }
    #[inline(always)]
    fn eq(self, other: &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> SecretModp {
        (self - other).eqz()
    }
    #[inline(always)]
    fn ne(self, other: &'a SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>) -> SecretModp {
        (self - other).nez()
    }
}

// ClearFloat Cmp ClearFloat
impl<const V: u64, const P: u64, const DETECT_OVERFLOW: bool>
    ScaleCmp<ClearFloat<V, P, DETECT_OVERFLOW>, ClearModp> for ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    #[inline(always)]
    fn lt(self, other: ClearFloat<V, P, DETECT_OVERFLOW>) -> ClearModp {
        (self - other).ltz()
    }
    #[inline(always)]
    fn le(self, other: ClearFloat<V, P, DETECT_OVERFLOW>) -> ClearModp {
        (self - other).lez()
    }
    #[inline(always)]
    fn gt(self, other: ClearFloat<V, P, DETECT_OVERFLOW>) -> ClearModp {
        (self - other).gtz()
    }
    #[inline(always)]
    fn ge(self, other: ClearFloat<V, P, DETECT_OVERFLOW>) -> ClearModp {
        (self - other).gez()
    }
    #[inline(always)]
    fn eq(self, other: ClearFloat<V, P, DETECT_OVERFLOW>) -> ClearModp {
        (self - other).eqz()
    }
    #[inline(always)]
    fn ne(self, other: ClearFloat<V, P, DETECT_OVERFLOW>) -> ClearModp {
        (self - other).nez()
    }
}
impl<'a, const V: u64, const P: u64, const DETECT_OVERFLOW: bool>
    ScaleCmp<ClearFloat<V, P, DETECT_OVERFLOW>, ClearModp> for &'a ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    #[inline(always)]
    fn lt(self, other: ClearFloat<V, P, DETECT_OVERFLOW>) -> ClearModp {
        (self - other).ltz()
    }
    #[inline(always)]
    fn le(self, other: ClearFloat<V, P, DETECT_OVERFLOW>) -> ClearModp {
        (self - other).lez()
    }
    #[inline(always)]
    fn gt(self, other: ClearFloat<V, P, DETECT_OVERFLOW>) -> ClearModp {
        (self - other).gtz()
    }
    #[inline(always)]
    fn ge(self, other: ClearFloat<V, P, DETECT_OVERFLOW>) -> ClearModp {
        (self - other).gez()
    }
    #[inline(always)]
    fn eq(self, other: ClearFloat<V, P, DETECT_OVERFLOW>) -> ClearModp {
        (self - other).eqz()
    }
    #[inline(always)]
    fn ne(self, other: ClearFloat<V, P, DETECT_OVERFLOW>) -> ClearModp {
        (self - other).nez()
    }
}

impl<'a, const V: u64, const P: u64, const DETECT_OVERFLOW: bool>
    ScaleCmp<&'a ClearFloat<V, P, DETECT_OVERFLOW>, ClearModp>
    for &'a ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    #[inline(always)]
    fn lt(self, other: &'a ClearFloat<V, P, DETECT_OVERFLOW>) -> ClearModp {
        (self - other).ltz()
    }
    #[inline(always)]
    fn le(self, other: &'a ClearFloat<V, P, DETECT_OVERFLOW>) -> ClearModp {
        (self - other).lez()
    }
    #[inline(always)]
    fn gt(self, other: &'a ClearFloat<V, P, DETECT_OVERFLOW>) -> ClearModp {
        (self - other).gtz()
    }
    #[inline(always)]
    fn ge(self, other: &'a ClearFloat<V, P, DETECT_OVERFLOW>) -> ClearModp {
        (self - other).gez()
    }
    #[inline(always)]
    fn eq(self, other: &'a ClearFloat<V, P, DETECT_OVERFLOW>) -> ClearModp {
        (self - other).eqz()
    }
    #[inline(always)]
    fn ne(self, other: &'a ClearFloat<V, P, DETECT_OVERFLOW>) -> ClearModp {
        (self - other).nez()
    }
}

impl<'a, const V: u64, const P: u64, const DETECT_OVERFLOW: bool>
    ScaleCmp<&'a ClearFloat<V, P, DETECT_OVERFLOW>, ClearModp> for ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ P + 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ V + 2 }>: ,
{
    #[inline(always)]
    fn lt(self, other: &'a ClearFloat<V, P, DETECT_OVERFLOW>) -> ClearModp {
        (self - other).ltz()
    }
    #[inline(always)]
    fn le(self, other: &'a ClearFloat<V, P, DETECT_OVERFLOW>) -> ClearModp {
        (self - other).lez()
    }
    #[inline(always)]
    fn gt(self, other: &'a ClearFloat<V, P, DETECT_OVERFLOW>) -> ClearModp {
        (self - other).gtz()
    }
    #[inline(always)]
    fn ge(self, other: &'a ClearFloat<V, P, DETECT_OVERFLOW>) -> ClearModp {
        (self - other).gez()
    }
    #[inline(always)]
    fn eq(self, other: &'a ClearFloat<V, P, DETECT_OVERFLOW>) -> ClearModp {
        (self - other).eqz()
    }
    #[inline(always)]
    fn ne(self, other: &'a ClearFloat<V, P, DETECT_OVERFLOW>) -> ClearModp {
        (self - other).nez()
    }
}
