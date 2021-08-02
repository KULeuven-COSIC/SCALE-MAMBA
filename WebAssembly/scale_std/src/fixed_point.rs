// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use crate::array::*;
use crate::bit_protocols::*;
use crate::ieee::*;
use crate::integer::*;
use crate::local_functions::*;
use crate::slice::*;
use core::ops::{Add, Div, Mul, Neg, Sub};
use scale::alloc::*;
use scale::*;

/* This fixed point arithmetic
 *
 * It uses the global statistical security parameter kappa
 * Follows the algorithms in Section 14.4 of the main Scale
 * manual
 *
 */

#[derive(Copy, Clone)]
pub struct ClearFixed<const K: u64, const F: u64> {
    x: ClearInteger<K>,
}

#[derive(Copy, Clone)]
pub struct SecretFixed<const K: u64, const F: u64, const KAPPA: u64> {
    x: SecretInteger<K, KAPPA>,
}

/*
 * Stuff to enable usage in arrays etc
 *
 */

impl<const K: u64, const F: u64> GetAllocator for ClearFixed<K, F> {
    type Allocator = &'static Allocator<ClearModp>;
    fn get_allocator() -> &'static Allocator<ClearModp> {
        ClearModp::get_allocator()
    }
    fn size_type() -> u64 {
        1
    }
}

impl<const K: u64, const F: u64> LoadFromMem<i64> for ClearFixed<K, F> {
    fn load_from_mem(idx: i64) -> Self {
        let a = ClearModp::load_from_mem(idx);
        let b: ClearInteger<K> = ClearInteger::set(a);
        ClearFixed { x: b }
    }
}

impl<const K: u64, const F: u64> StoreInMem<i64> for ClearFixed<K, F> {
    unsafe fn store_in_mem(&self, idx: i64) {
        self.x.rep().store_in_mem(idx);
    }
}

impl<const K: u64, const F: u64, const KAPPA: u64> GetAllocator for SecretFixed<K, F, KAPPA> {
    type Allocator = &'static Allocator<SecretModp>;
    fn get_allocator() -> &'static Allocator<SecretModp> {
        SecretModp::get_allocator()
    }
    fn size_type() -> u64 {
        1
    }
}

impl<const K: u64, const F: u64, const KAPPA: u64> LoadFromMem<i64> for SecretFixed<K, F, KAPPA> {
    fn load_from_mem(idx: i64) -> Self {
        let a = SecretModp::load_from_mem(idx);
        let b: SecretInteger<K, KAPPA> = SecretInteger::set(a);
        SecretFixed { x: b }
    }
}

impl<const K: u64, const F: u64, const KAPPA: u64> StoreInMem<i64> for SecretFixed<K, F, KAPPA> {
    unsafe fn store_in_mem(&self, idx: i64) {
        self.x.rep().store_in_mem(idx);
    }
}

/* Basic Constructors
 *
 * Assumes K>=F (unlike Mamba)
 */

impl<const K: u64, const F: u64, const KAPPA: u64> From<ClearFixed<K, F>>
    for SecretFixed<K, F, KAPPA>
{
    #[inline(always)]
    fn from(a: ClearFixed<K, F>) -> Self {
        let z = SecretInteger::from(a.x);
        Self { x: z }
    }
}

impl<const K: u64, const F: u64> From<ClearIEEE> for ClearFixed<K, F> {
    #[inline(always)]
    fn from(a: ClearIEEE) -> Self {
        let v = to_fix(a.rep(), K as i64, F as i64);
        let z: ClearInteger<K> = ClearInteger::from(v);
        Self { x: z }
    }
}

impl<const K: u64, const F: u64> From<f64> for ClearFixed<K, F> {
    #[inline(always)]
    fn from(f: f64) -> ClearFixed<K, F> {
        let a: ClearIEEE = ClearIEEE::from(f);
        Self::from(a)
    }
}

impl<const K: u64, const F: u64, const KAPPA: u64> From<f64> for SecretFixed<K, F, KAPPA> {
    #[inline(always)]
    fn from(f: f64) -> SecretFixed<K, F, KAPPA> {
        let b: ClearFixed<K, F> = ClearFixed::from(f);
        Self::from(b)
    }
}

impl<const K: u64, const F: u64> From<i64> for ClearFixed<K, F> {
    #[inline(always)]
    fn from(a: i64) -> Self {
        let y = ClearModp::from(a);
        let cpow = modp_two_power(F);
        let z: ClearInteger<K> = ClearInteger::from(y * cpow);
        Self { x: z }
    }
}

impl<const K: u64, const F: u64> From<ClearModp> for ClearFixed<K, F> {
    #[inline(always)]
    fn from(y: ClearModp) -> Self {
        let cpow = modp_two_power(F);
        let z: ClearInteger<K> = ClearInteger::from(y * cpow);
        Self { x: z }
    }
}

impl<const K: u64, const F: u64> From<ClearInteger<K>> for ClearFixed<K, F> {
    #[inline(always)]
    fn from(y: ClearInteger<K>) -> Self {
        let cpow = modp_two_power(F);
        let z: ClearInteger<K> = ClearInteger::from(y.rep() * cpow);
        Self { x: z }
    }
}

impl<const K: u64, const F: u64, const KAPPA: u64> From<i64> for SecretFixed<K, F, KAPPA> {
    #[inline(always)]
    fn from(a: i64) -> Self {
        let y = ClearModp::from(a);
        let cpow = modp_two_power(F);
        let z: SecretInteger<K, KAPPA> = SecretInteger::from(y * cpow);
        Self { x: z }
    }
}

impl<const K: u64, const F: u64, const KAPPA: u64> From<ClearModp> for SecretFixed<K, F, KAPPA> {
    #[inline(always)]
    fn from(y: ClearModp) -> Self {
        let cpow = modp_two_power(F);
        let z: SecretInteger<K, KAPPA> = SecretInteger::from(y * cpow);
        Self { x: z }
    }
}

impl<const K: u64, const F: u64, const KAPPA: u64> From<ClearInteger<K>>
    for SecretFixed<K, F, KAPPA>
{
    #[inline(always)]
    fn from(y: ClearInteger<K>) -> Self {
        let cpow = modp_two_power(F);
        let z: SecretInteger<K, KAPPA> = SecretInteger::from(y.rep() * cpow);
        Self { x: z }
    }
}

impl<const K: u64, const F: u64, const KAPPA: u64> From<SecretModp> for SecretFixed<K, F, KAPPA> {
    #[inline(always)]
    fn from(y: SecretModp) -> Self {
        let cpow = modp_two_power(F);
        let z: SecretInteger<K, KAPPA> = SecretInteger::from(y * cpow);
        Self { x: z }
    }
}

impl<const K: u64, const F: u64, const KAPPA: u64> From<SecretInteger<K, KAPPA>>
    for SecretFixed<K, F, KAPPA>
{
    #[inline(always)]
    fn from(y: SecretInteger<K, KAPPA>) -> Self {
        let cpow = modp_two_power(F);
        let z: SecretInteger<K, KAPPA> = SecretInteger::from(y.rep() * cpow);
        Self { x: z }
    }
}

#[inline(always)]
pub const fn abs_sub(a: u64, b: u64) -> u64 {
    if a > b {
        a - b
    } else {
        b - a
    }
}

/* Convert sizes
 * - Assumes user knows what they are doing
 */
impl<const K: u64, const F: u64> ClearFixed<K, F> {
    #[inline(always)]
    pub unsafe fn recast<const K2: u64, const F2: u64>(self) -> ClearFixed<K2, F2> {
        // First cast to the new size of integer
        let mut ans: ClearInteger<K2> = self.x.recast();
        // Now scale up/down
        if F2 > F {
            let cpow = modp_two_power(F2 - F);
            let cpow_i: ClearInteger<K2> = ClearInteger::from(cpow);
            ans = ans.mul(cpow_i);
        } else {
            ans = ans.Trunc(abs_sub(F, F2), true);
        }
        ClearFixed { x: ans }
    }
}

impl<const K: u64, const F: u64, const KAPPA: u64> SecretFixed<K, F, KAPPA> {
    #[inline(always)]
    pub unsafe fn recast<const K2: u64, const F2: u64>(self) -> SecretFixed<K2, F2, KAPPA> {
        // First cast to the new size of integer
        let mut ans: SecretInteger<K2, KAPPA> = self.x.recast();
        // Now scale up/down
        if F2 > F {
            let cpow = modp_two_power(F2 - F);
            let cpow_i: ClearInteger<K2> = ClearInteger::from(cpow);
            ans = ans.mul_clear(cpow_i);
        } else {
            ans = ans.Trunc(abs_sub(F, F2), true);
        }
        SecretFixed { x: ans }
    }
}

/* Access the underlying representation */

impl<const K: u64, const F: u64> ClearFixed<K, F> {
    #[inline(always)]
    pub fn rep(self) -> ClearInteger<K> {
        self.x
    }
}

impl<const K: u64, const F: u64, const KAPPA: u64> SecretFixed<K, F, KAPPA> {
    #[inline(always)]
    pub fn rep(self) -> SecretInteger<K, KAPPA> {
        self.x
    }
}

/* Get the underlying integer part only */
impl<const K: u64, const F: u64> ClearFixed<K, F>
where
    ConstI32<{ f_as_i32(F) }>: ,
    ConstU64<{ K - 1 }>: ,
    ConstU64<{ K + 1 }>: ,
{
    #[inline(always)]
    pub fn rep_integer(self) -> ClearModp {
        let vp = self.x.rep() >> ConstI32::<{ f_as_i32(F) }>;
        let vn = (-self.x.rep()) >> ConstI32::<{ f_as_i32(F) }>;
        let b = self.x.gtz();
        let v = b * vp - (ClearModp::from(1) - b) * vn;
        ClearModp::from(v)
    }
}

/* Set the underlying representation to something */

impl<const K: u64, const F: u64> ClearFixed<K, F> {
    #[inline(always)]
    pub fn set(v: ClearInteger<K>) -> Self {
        Self { x: v }
    }
}

impl<const K: u64, const F: u64> ClearFixed<K, F> {
    #[inline(always)]
    pub fn set_modp(v: ClearModp) -> Self {
        Self {
            x: ClearInteger::set(v),
        }
    }
}

impl<const K: u64, const F: u64, const KAPPA: u64> SecretFixed<K, F, KAPPA> {
    #[inline(always)]
    pub fn set(v: SecretInteger<K, KAPPA>) -> Self {
        Self { x: v }
    }
}

impl<const K: u64, const F: u64, const KAPPA: u64> SecretFixed<K, F, KAPPA> {
    #[inline(always)]
    pub fn set_modp(v: SecretModp) -> Self {
        Self {
            x: SecretInteger::set(v),
        }
    }
}

/* Randomization
 *  - Assumes K>=F
 */
impl<const K: u64, const F: u64> Randomize for ClearFixed<K, F> {
    #[inline(always)]
    fn randomize() -> ClearFixed<K, F> {
        let a = modp_two_power(F);
        let b = ClearModp::randomize() % a;
        Self {
            x: ClearInteger::set(b),
        }
    }
}

impl<const K: u64, const F: u64, const KAPPA: u64> Randomize for SecretFixed<K, F, KAPPA> {
    #[inline(always)]
    fn randomize() -> SecretFixed<K, F, KAPPA> {
        let a = PRandInt(F);
        Self {
            x: SecretInteger::set(a),
        }
    }
}

/* Print a clear fixed point number */

impl<const K: u64, const F: u64> Print for ClearFixed<K, F> {
    #[inline(always)]
    fn print(self) {
        unsafe { __print_fix(self.x.rep(), F as i32, K as i32) }
    }
}

/* Reveal Operation */

// This is a NOP, but needed to ensure some generic routines compile
impl<const K: u64, const F: u64> Reveal for ClearFixed<K, F> {
    type Output = ClearFixed<K, F>;
    #[inline(always)]
    fn reveal(&self) -> ClearFixed<K, F> {
        ClearFixed { x: self.x }
    }
}

impl<const K: u64, const F: u64, const KAPPA: u64> Reveal for SecretFixed<K, F, KAPPA> {
    type Output = ClearFixed<K, F>;
    #[inline(always)]
    fn reveal(&self) -> ClearFixed<K, F> {
        ClearFixed { x: self.x.reveal() }
    }
}

/* Arithmetic Routines
 *
 * We do not give functional versions here as for sfix
 * operations the operator versions do no truncation.
 * If the result is "out of bounds" then "undefined"
 * behaviour occurs. This is faster.
 *
 * So we actually convert back to the base representation
 * do the arithmetic there, and then convert into
 * Integer representation
 *
 */

/* Now the operator versions .... */
impl<const K: u64, const F: u64> Neg for ClearFixed<K, F> {
    type Output = Self;
    #[inline(always)]
    fn neg(self) -> Self::Output {
        ClearFixed::set(-self.x)
    }
}

impl<const K: u64, const F: u64, const KAPPA: u64> Neg for SecretFixed<K, F, KAPPA> {
    type Output = Self;
    #[inline(always)]
    fn neg(self) -> Self::Output {
        SecretFixed::set(-self.x)
    }
}

impl<const K: u64, const F: u64> Add<ClearFixed<K, F>> for ClearFixed<K, F> {
    type Output = Self;
    #[inline(always)]
    fn add(self, other: ClearFixed<K, F>) -> Self::Output {
        let v = self.x.rep() + other.x.rep();
        let ans: ClearInteger<K> = ClearInteger::set(v);
        ClearFixed::set(ans)
    }
}

impl<const K: u64, const F: u64, const KAPPA: u64> Add<SecretFixed<K, F, KAPPA>>
    for ClearFixed<K, F>
{
    type Output = SecretFixed<K, F, KAPPA>;
    #[inline(always)]
    fn add(self, other: SecretFixed<K, F, KAPPA>) -> Self::Output {
        let v = self.x.rep() + other.x.rep();
        let ans: SecretInteger<K, KAPPA> = SecretInteger::set(v);
        SecretFixed::set(ans)
    }
}

impl<const K: u64, const F: u64, const KAPPA: u64> Add<SecretFixed<K, F, KAPPA>>
    for SecretFixed<K, F, KAPPA>
{
    type Output = Self;
    #[inline(always)]
    fn add(self, other: SecretFixed<K, F, KAPPA>) -> Self::Output {
        let v = self.x.rep() + other.x.rep();
        let ans: SecretInteger<K, KAPPA> = SecretInteger::set(v);
        SecretFixed::set(ans)
    }
}

impl<const K: u64, const F: u64, const KAPPA: u64> Add<ClearFixed<K, F>>
    for SecretFixed<K, F, KAPPA>
{
    type Output = SecretFixed<K, F, KAPPA>;
    #[inline(always)]
    fn add(self, other: ClearFixed<K, F>) -> Self::Output {
        other + self
    }
}

impl<const K: u64, const F: u64> Sub<ClearFixed<K, F>> for ClearFixed<K, F> {
    type Output = Self;
    #[inline(always)]
    fn sub(self, other: ClearFixed<K, F>) -> Self::Output {
        let v = self.x.rep() - other.x.rep();
        let ans: ClearInteger<K> = ClearInteger::set(v);
        ClearFixed::set(ans)
    }
}

impl<const K: u64, const F: u64, const KAPPA: u64> Sub<SecretFixed<K, F, KAPPA>>
    for ClearFixed<K, F>
{
    type Output = SecretFixed<K, F, KAPPA>;
    #[inline(always)]
    fn sub(self, other: SecretFixed<K, F, KAPPA>) -> Self::Output {
        let v = self.x.rep() - other.x.rep();
        let ans: SecretInteger<K, KAPPA> = SecretInteger::set(v);
        SecretFixed::set(ans)
    }
}

impl<const K: u64, const F: u64, const KAPPA: u64> Sub<SecretFixed<K, F, KAPPA>>
    for SecretFixed<K, F, KAPPA>
{
    type Output = Self;
    #[inline(always)]
    fn sub(self, other: SecretFixed<K, F, KAPPA>) -> Self::Output {
        let v = self.x.rep() - other.x.rep();
        let ans: SecretInteger<K, KAPPA> = SecretInteger::set(v);
        SecretFixed::set(ans)
    }
}

impl<const K: u64, const F: u64, const KAPPA: u64> Sub<ClearFixed<K, F>>
    for SecretFixed<K, F, KAPPA>
{
    type Output = SecretFixed<K, F, KAPPA>;
    #[inline(always)]
    fn sub(self, other: ClearFixed<K, F>) -> Self::Output {
        self + (-other)
    }
}

impl<const K: u64, const F: u64> Mul<ClearFixed<K, F>> for ClearFixed<K, F>
where
    ConstU64<{ 2 * K }>: ,
{
    type Output = Self;
    fn mul(self, other: ClearFixed<K, F>) -> Self::Output {
        let v = self.x.rep() * other.x.rep();
        let u = ClearInteger::<{ 2 * K }>::from(v);
        let w = u.Trunc(F, true);
        let ans: ClearInteger<K> = ClearInteger::set(w.rep());
        ClearFixed::set(ans)
    }
}

impl<const K: u64, const F: u64, const KAPPA: u64> Mul<SecretFixed<K, F, KAPPA>>
    for ClearFixed<K, F>
where
    ConstU64<{ 2 * K }>: ,
{
    type Output = SecretFixed<K, F, KAPPA>;
    fn mul(self, other: SecretFixed<K, F, KAPPA>) -> Self::Output {
        let v = self.x.rep() * other.x.rep();
        let u = SecretInteger::<{ 2 * K }, KAPPA>::from(v);
        let w = u.TruncPr(F, true);
        let ans: SecretInteger<K, KAPPA> = SecretInteger::set(w.rep());
        SecretFixed::set(ans)
    }
}

impl<const K: u64, const F: u64, const KAPPA: u64> Mul<SecretFixed<K, F, KAPPA>>
    for SecretFixed<K, F, KAPPA>
where
    ConstU64<{ 2 * K }>: ,
{
    type Output = Self;
    fn mul(self, other: SecretFixed<K, F, KAPPA>) -> Self::Output {
        let v = self.x.rep() * other.x.rep();
        let u = SecretInteger::<{ 2 * K }, KAPPA>::from(v);
        let w = u.TruncPr(F, true);
        let ans: SecretInteger<K, KAPPA> = SecretInteger::set(w.rep());
        SecretFixed::set(ans)
    }
}

impl<const K: u64, const F: u64, const KAPPA: u64> Mul<ClearFixed<K, F>>
    for SecretFixed<K, F, KAPPA>
where
    ConstU64<{ 2 * K }>: ,
{
    type Output = SecretFixed<K, F, KAPPA>;
    fn mul(self, other: ClearFixed<K, F>) -> Self::Output {
        other * self
    }
}

impl<const K: u64, const F: u64, const KAPPA: u64> ScaleCmpZ<SecretModp>
    for SecretFixed<K, F, KAPPA>
where
    ConstU64<{ K + 1 }>: ,
    ConstU64<{ K - 1 }>: ,
{
    #[inline(always)]
    fn ltz(self) -> SecretModp {
        self.x.ltz()
    }
    #[inline(always)]
    fn lez(self) -> SecretModp {
        self.x.lez()
    }
    #[inline(always)]
    fn gtz(self) -> SecretModp {
        self.x.gtz()
    }
    #[inline(always)]
    fn gez(self) -> SecretModp {
        self.x.gez()
    }
    #[inline(always)]
    fn eqz(self) -> SecretModp {
        self.x.eqz()
    }
    #[inline(always)]
    fn nez(self) -> SecretModp {
        self.x.nez()
    }
}

impl<const K: u64, const F: u64> ScaleCmpZ<ClearModp> for ClearFixed<K, F>
where
    ConstU64<{ K + 1 }>: ,
    ConstU64<{ K - 1 }>: ,
{
    #[inline(always)]
    fn ltz(self) -> ClearModp {
        self.x.ltz()
    }
    #[inline(always)]
    fn lez(self) -> ClearModp {
        self.x.lez()
    }
    #[inline(always)]
    fn gtz(self) -> ClearModp {
        self.x.gtz()
    }
    #[inline(always)]
    fn gez(self) -> ClearModp {
        self.x.gez()
    }
    #[inline(always)]
    fn eqz(self) -> ClearModp {
        self.x.eqz()
    }
    #[inline(always)]
    fn nez(self) -> ClearModp {
        self.x.nez()
    }
}

impl<const K: u64, const F: u64, const KAPPA: u64> ScaleCmp<Self, SecretModp>
    for SecretFixed<K, F, KAPPA>
where
    ConstU64<{ K + 1 }>: ,
    ConstU64<{ K - 1 }>: ,
{
    #[inline(always)]
    fn lt(self, other: Self) -> SecretModp {
        self.x.lt(other.x)
    }
    #[inline(always)]
    fn le(self, other: Self) -> SecretModp {
        self.x.le(other.x)
    }
    #[inline(always)]
    fn gt(self, other: Self) -> SecretModp {
        self.x.gt(other.x)
    }
    #[inline(always)]
    fn ge(self, other: Self) -> SecretModp {
        self.x.ge(other.x)
    }
    #[inline(always)]
    fn eq(self, other: Self) -> SecretModp {
        self.x.eq(other.x)
    }
    #[inline(always)]
    fn ne(self, other: Self) -> SecretModp {
        self.x.ne(other.x)
    }
}

impl<const K: u64, const F: u64> ScaleCmp<Self, ClearModp> for ClearFixed<K, F>
where
    ConstU64<{ K + 1 }>: ,
    ConstU64<{ K - 1 }>: ,
{
    #[inline(always)]
    fn lt(self, other: Self) -> ClearModp {
        self.x.lt(other.x)
    }
    #[inline(always)]
    fn le(self, other: Self) -> ClearModp {
        self.x.le(other.x)
    }
    #[inline(always)]
    fn gt(self, other: Self) -> ClearModp {
        self.x.gt(other.x)
    }
    #[inline(always)]
    fn ge(self, other: Self) -> ClearModp {
        self.x.ge(other.x)
    }
    #[inline(always)]
    fn eq(self, other: Self) -> ClearModp {
        self.x.eq(other.x)
    }
    #[inline(always)]
    fn ne(self, other: Self) -> ClearModp {
        self.x.ne(other.x)
    }
}

#[inline(always)]
fn twos_complement<const K: u64>(x: ClearModp, _: ConstU64<K>) -> ClearModp {
    let bits: Slice<ClearModp> = Slice::bit_decomposition_ClearModp_Signed(x, K);
    let mut twos_result = ClearModp::from(0);
    for i in 0..K {
        twos_result =
            (twos_result + twos_result) + ClearModp::from(1) - *bits.get_unchecked(K - 1 - i);
    }
    twos_result + ClearModp::from(1)
}

#[inline(always)]
fn approximate_reciprocal<const K: u64, const F: u64, const THETA: u64>(
    divisor: ClearModp,
    _: ConstU64<K>,
    _: ConstU64<F>,
    _: ConstU64<THETA>,
) -> ClearModp
where
    ConstI32<{ f_as_i32(K) }>: ,
{
    let bits: Slice<ClearModp> = Slice::bit_decomposition_ClearModp_Signed(divisor, K);
    let mut cnt_leading_zeros = ClearModp::from(0);
    let mut flag: i64 = 0;
    let mut normalized_divisor = divisor;

    for i in 0..K {
        flag = flag | i64::from(*bits.get_unchecked(K - 1 - i));
        if flag == 0 {
            cnt_leading_zeros = cnt_leading_zeros + ClearModp::from(1);
            normalized_divisor = normalized_divisor + normalized_divisor;
        }
    }

    let mut q = modp_two_power(K);
    let mut e = twos_complement(normalized_divisor, ConstU64::<K>);
    for _i in 0..THETA {
        q = q + ((q * e) >> ConstI32::<{ f_as_i32(K) }>);
        e = (e * e) >> ConstI32::<{ f_as_i32(K) }>;
    }
    let res = q >> (ClearModp::from(2 * { K - F } as i64) - cnt_leading_zeros);
    res
}

impl<const K: u64, const F: u64> ClearFixed<K, F> {
    // theta = int(ceil(log(K/3.5) / log(2)));
    pub const THETA: u64 = {
        match K {
            0..4 => 0,
            4..8 => 1,
            8..15 => 2,
            15..29 => 3,
            29..57 => 4,
            57..113 => 5,
            113..225 => 6,
            225..449 => 7,
            449.. => core::panic!("K too large"),
        }
    };
}

impl<const K: u64, const F: u64, const KAPPA: u64> SecretFixed<K, F, KAPPA> {
    // theta = int(ceil(log(K/3.5) / log(2)));
    pub const THETA: u64 = {
        match K {
            0..4 => 0,
            4..8 => 1,
            8..15 => 2,
            15..29 => 3,
            29..57 => 4,
            57..113 => 5,
            113..225 => 6,
            225..449 => 7,
            449.. => core::panic!("K too large"),
        }
    };
}

pub const fn f_as_i32(f: u64) -> i32 {
    f as i32
}

/* Goldschmidt method implemented with SE aproximation:
    http://stackoverflow.com/questions/2661541/picking-good-first-estimates-for-goldschmidt-division
*/
impl<const K: u64, const F: u64> Div<ClearFixed<K, F>> for ClearFixed<K, F>
where
    ConstU64<{ K - 1 }>: ,
    ConstU64<{ K + 1 }>: ,
    ConstU64<{ Self::THETA }>: ,
    ConstI32<{ f_as_i32(F) }>: ,
    ConstI32<{ f_as_i32(K) }>: ,
{
    type Output = Self;
    #[inline(always)]
    fn div(self, other: ClearFixed<K, F>) -> Self::Output {
        let two = ClearModp::from(2) * modp_two_power(F);

        let t = self.ltz();
        let sign_a = ClearModp::from(1) - t - t;
        let t = other.ltz();
        let sign_b = ClearModp::from(1) - t - t;

        let mut a0 = self.x.rep() * sign_a;
        let mut b0 = other.x.rep() * sign_b;

        let mut w0 = approximate_reciprocal(
            b0,
            ConstU64::<K>,
            ConstU64::<F>,
            ConstU64::<{ Self::THETA }>,
        );

        for _i in 1..Self::THETA {
            a0 = (a0 * w0) >> ConstI32::<{ f_as_i32(F) }>;
            b0 = (b0 * w0) >> ConstI32::<{ f_as_i32(F) }>;
            w0 = two - b0;
        }
        let ans: ClearInteger<K> = ClearInteger::set(sign_a * sign_b * a0);
        ClearFixed { x: ans }
    }
}

/* Goldschmidt method implemented with SE aproximation:
     http://stackoverflow.com/questions/2661541/picking-good-first-estimates-for-goldschmidt-division
*/
impl<const K: u64, const F: u64, const KAPPA: u64> Div<ClearFixed<K, F>>
    for SecretFixed<K, F, KAPPA>
where
    ConstU64<{ K - 1 }>: ,
    ConstU64<{ K + 1 }>: ,
    ConstU64<{ 2 * K }>: ,
    ConstU64<{ Self::THETA }>: ,
    ConstI32<{ f_as_i32(F) }>: ,
    ConstI32<{ f_as_i32(K) }>: ,
{
    type Output = Self;
    #[inline(always)]
    fn div(self, other: ClearFixed<K, F>) -> Self::Output {
        let two = ClearModp::from(2) * modp_two_power(F);

        let t = self.ltz();
        let sign_a = SecretModp::from(1) - t - t;
        let t = other.ltz();
        let sign_b = ClearModp::from(1) - t - t;

        let mut a0 = self.x.rep() * sign_a;
        let mut b0 = other.x.rep() * sign_b;

        let mut w0 = approximate_reciprocal(
            b0,
            ConstU64::<K>,
            ConstU64::<F>,
            ConstU64::<{ Self::THETA }>,
        );

        for _i in 1..Self::THETA {
            let temp: SecretInteger<{ 2 * K }, KAPPA> = SecretInteger::from(a0 * w0);
            a0 = temp.TruncPr(F, true).rep();
            b0 = (b0 * w0) >> ConstI32::<{ f_as_i32(F) }>;
            w0 = two - b0;
        }
        let ans: SecretInteger<K, KAPPA> = SecretInteger::set(sign_a * sign_b * a0);
        SecretFixed { x: ans }
    }
}

/* Computes secret integer values [c] and [v_prime] st.
        2^{k-1} <= c < 2^k and c = b*v_prime
*/
#[allow(non_snake_case)]
#[inline(always)]
fn Norm<const K: u64, const KAPPA: u64>(
    b: SecretModp,
    _: ConstU64<K>,
    _: ConstU64<KAPPA>,
) -> Array<SecretModp, 2>
where
    ConstU64<{ K + 1 }>: ,
    ConstU64<{ K - 1 }>: ,
{
    let ib: SecretInteger<K, KAPPA> = SecretInteger::from(b);
    let ltz = ib.ltz();
    let sign = ClearModp::from(1) - ltz - ltz;
    let absolute_val = sign * b;

    let bits_order = BitDec::<K, K, KAPPA>(absolute_val);
    // Invert bits
    let bits = bits_order.reverse();
    let suffixes = bits.PreOr();

    let mut z: Array<SecretModp, K> = Array::uninitialized();
    for i in 0..(K - 1) {
        z.set(
            K - 1 - i,
            &(*suffixes.get_unchecked(K - 1 - i) - *suffixes.get_unchecked(K - 2 - i)),
        );
    }
    z.set(0, &*suffixes.get_unchecked(0));

    let acc = z.evaluate(ClearModp::from(2));

    let mut ans: Array<SecretModp, 2> = Array::uninitialized();
    ans.set(0, &(absolute_val * acc));
    ans.set(1, &(sign * acc));
    ans
}

#[allow(non_snake_case)]
#[inline(always)]
fn AppRcr<const K: u64, const F: u64, const KAPPA: u64>(
    b: SecretModp,
    _: ConstU64<K>,
    _: ConstU64<F>,
    _: ConstU64<KAPPA>,
) -> SecretModp
where
    ConstU64<{ 2 * K }>: ,
    ConstU64<{ K + 1 }>: ,
    ConstU64<{ K - 1 }>: ,
{
    let a: i64 = (2.9142 * ((1_i64 << K) as f64)) as i64;
    let alpha = ClearModp::from(a);

    let Nm = Norm(b, ConstU64::<K>, ConstU64::<KAPPA>);
    let c = *Nm.get_unchecked(0);
    let v = *Nm.get_unchecked(1);
    let d = alpha - c - c;
    let w_int: SecretInteger<{ 2 * K }, KAPPA> = SecretInteger::from(d * v);
    let w = w_int.TruncPr(2 * (K - F), true).rep();
    w
}

/* Produces the fixed point division of a by b.
   Uses the Goldschmidt method as presented in Catrina10
*/
impl<const K: u64, const F: u64, const KAPPA: u64> Div<SecretFixed<K, F, KAPPA>>
    for SecretFixed<K, F, KAPPA>
where
    ConstU64<{ 2 * K }>: ,
    ConstU64<{ Self::THETA }>: ,
    ConstU64<{ K + 1 }>: ,
    ConstU64<{ K - 1 }>: ,
{
    type Output = Self;
    #[inline(always)]
    fn div(self, other: SecretFixed<K, F, KAPPA>) -> Self::Output {
        let a = self.x.rep();
        let b = other.x.rep();
        let alpha: ClearInteger<{ 2 * K }> = ClearInteger::from(modp_two_power(2 * F));
        let w = AppRcr(b, ConstU64::<K>, ConstU64::<F>, ConstU64::<KAPPA>);
        let mut x: SecretInteger<{ 2 * K }, KAPPA> = SecretInteger::from(alpha.rep() - b * w);

        let mut y: SecretInteger<{ 2 * K }, KAPPA> = SecretInteger::from(a * w);
        y = y.TruncPr(F, true);
        for _i in 0..Self::THETA {
            y = unsafe { x.add_clear(alpha).mul(y) };
            x = unsafe { x.mul(x) };
            y = y.TruncPr(2 * F, true);
            x = x.TruncPr(2 * F, true);
        }
        y = unsafe { x.add_clear(alpha).mul(y) };
        y = y.TruncPr(2 * F, true);
        let ans: SecretInteger<K, KAPPA> = SecretInteger::set(y.rep());
        SecretFixed { x: ans }
    }
}

impl<const K: u64, const F: u64, const KAPPA: u64> Div<SecretFixed<K, F, KAPPA>>
    for ClearFixed<K, F>
where
    ConstU64<{ 2 * F }>: ,
    ConstU64<{ 2 * K }>: ,
    ConstU64<{ K + 1 }>: ,
    ConstU64<{ K - 1 }>: ,
    ConstU64<{ 2 * (K - F) }>: ,
    ConstI32<{ f_as_i32(F) }>: ,
    ConstI32<{ f_as_i32(K) }>: ,
    ConstU64<{ SecretFixed::<K, F, KAPPA>::THETA }>: ,
{
    type Output = SecretFixed<K, F, KAPPA>;
    #[inline(always)]
    fn div(self, other: SecretFixed<K, F, KAPPA>) -> Self::Output {
        let numerator = SecretFixed::from(self);
        let ans = numerator / other;
        ans
    }
}
