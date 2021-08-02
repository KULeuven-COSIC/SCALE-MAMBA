// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use crate::array::*;
use crate::bit_protocols::*;
use crate::slice::*;
use core::ops::{Add, Mul, Neg, Sub};
use scale::*;

/* This implements arithmetic in Z_<K>
 *  It uses the global statistical security parameter kappa
 * Follows the algorithms in Section 14.3 of the main Scale
 * manual
 *
 * x is held mod p, but it is considered to
 *   be an integer in the range -2^{k-1},...,2^{k-1}-1
 *
 * Note k is always less than log 2p
 */

#[derive(Copy, Clone)]
pub struct ClearInteger<const K: u64> {
    x: ClearModp,
}

#[derive(Copy, Clone)]
pub struct SecretInteger<const K: u64, const KAPPA: u64> {
    x: SecretModp,
}

/* Basic Constructors */

impl<const K: u64, const KAPPA: u64> From<ClearInteger<K>> for SecretInteger<K, KAPPA> {
    #[inline(always)]
    fn from(a: ClearInteger<K>) -> Self {
        unsafe { __reqbl(K as u32) };
        Self::from(SecretModp::from(a.x))
    }
}

impl<const K: u64> From<i64> for ClearInteger<K> {
    #[inline(always)]
    fn from(a: i64) -> Self {
        unsafe { __reqbl(K as u32) };
        Self::from(ClearModp::from(a))
    }
}

impl<const K: u64> From<ClearModp> for ClearInteger<K> {
    #[inline(always)]
    fn from(x: ClearModp) -> Self {
        unsafe { __reqbl(K as u32) };
        Self { x }
    }
}

impl<const K: u64, const KAPPA: u64> From<i64> for SecretInteger<K, KAPPA> {
    #[inline(always)]
    fn from(a: i64) -> Self {
        unsafe { __reqbl(K as u32) };
        Self::from(SecretModp::from(a))
    }
}

impl<const K: u64, const KAPPA: u64> From<ClearModp> for SecretInteger<K, KAPPA> {
    #[inline(always)]
    fn from(x: ClearModp) -> Self {
        unsafe { __reqbl(K as u32) };
        Self::from(SecretModp::from(x))
    }
}

impl<const K: u64, const KAPPA: u64> From<SecretModp> for SecretInteger<K, KAPPA> {
    #[inline(always)]
    fn from(x: SecretModp) -> Self {
        unsafe { __reqbl(K as u32) };
        Self { x }
    }
}

/* Convert sizes
 * - Assumes user knows what they are doing
 */

impl<const K: u64> ClearInteger<K> {
    #[inline(always)]
    pub unsafe fn recast<const K2: u64>(self) -> ClearInteger<K2> {
        __reqbl(K2 as u32);
        ClearInteger { x: self.x }
    }
}

impl<const K: u64, const KAPPA: u64> SecretInteger<K, KAPPA> {
    #[inline(always)]
    pub unsafe fn recast<const K2: u64>(self) -> SecretInteger<K2, KAPPA> {
        __reqbl((K2 + KAPPA) as u32);
        SecretInteger { x: self.x }
    }
}

/* Access the underlying representation */

impl<const K: u64> ClearInteger<K> {
    #[inline(always)]
    pub fn rep(self) -> ClearModp {
        self.x
    }
}

impl<const K: u64, const KAPPA: u64> SecretInteger<K, KAPPA> {
    #[inline(always)]
    pub fn rep(self) -> SecretModp {
        self.x
    }
}

/* Set the underlying representation to something */

impl<const K: u64> ClearInteger<K> {
    #[inline(always)]
    pub fn set(v: ClearModp) -> Self {
        Self { x: v }
    }
}

impl<const K: u64, const KAPPA: u64> SecretInteger<K, KAPPA> {
    #[inline(always)]
    pub fn set(v: SecretModp) -> Self {
        Self { x: v }
    }
}

/* Print a clear integer */

impl<const K: u64> Print for ClearInteger<K> {
    #[inline(always)]
    fn print(self) {
        self.x.print();
    }
}

/* Randomization */
impl<const K: u64> Randomize for ClearInteger<K> {
    #[inline(always)]
    fn randomize() -> ClearInteger<K> {
        let adiv2 = modp_two_power(K - 1);
        let a = adiv2 + adiv2;
        let b = ClearModp::randomize() % a - adiv2;
        Self { x: b }
    }
}

impl<const K: u64, const KAPPA: u64> Randomize for SecretInteger<K, KAPPA> {
    #[inline(always)]
    fn randomize() -> SecretInteger<K, KAPPA> {
        let adiv2 = modp_two_power(K - 1);
        let a = PRandInt(K) - adiv2;
        Self { x: a }
    }
}

/* Reveal Operation */

impl<const K: u64, const KAPPA: u64> Reveal for SecretInteger<K, KAPPA> {
    type Output = ClearInteger<K>;
    #[inline(always)]
    fn reveal(&self) -> ClearInteger<K> {
        ClearInteger { x: self.x.reveal() }
    }
}

/* Modular Reduction Operations
 *  - These keep the data type size the same.
 *    i.e. Take in Integer<K> and does mod 2^M returning
 *    another Integer<K>
 */

impl<const K: u64> ClearInteger<K> {
    #[inline(always)]
    #[allow(non_snake_case)]
    pub fn Mod2m(self, M: u64, SIGNED: bool) -> ClearInteger<K> {
        scale::assert!(M < K);
        let twom = modp_two_power(M);
        let mut val = self.x;
        if SIGNED {
            let twok = modp_two_power(K - 1);
            val = val + twok;
        }
        ClearInteger { x: val % twom }
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    pub fn Mod2(self, SIGNED: bool) -> ClearInteger<K> {
        let two = ClearModp::from(2);
        let mut val = self.x;
        if SIGNED {
            val = val + ClearModp::from(1);
        }
        ClearInteger { x: val % two }
    }
}

impl<const K: u64, const KAPPA: u64> SecretInteger<K, KAPPA> {
    #[inline(always)]
    #[allow(non_snake_case)]
    pub fn Mod2m(self, M: u64, SIGNED: bool) -> Self {
        scale::assert!(M < K);
        let (r_dprime, r, rb) = PRandM_Slice(K, M, KAPPA);
        let c2m = modp_two_power(M);
        let t0 = r_dprime * c2m;
        let mut t1 = self.x;
        if SIGNED {
            let twok = modp_two_power(K - 1);
            t1 = t1 + twok;
        }
        let t2 = t0 + t1;
        let t3 = t2 + r;
        let c = t3.reveal();
        let c_prime = c % c2m;
        let ans: SecretModp;
        if M != 1 {
            let u = BitLT(c_prime, &rb, rb.len());
            let t4 = u * c2m;
            let t5 = c_prime - r;
            ans = t5 + t4;
        } else {
            let tt = c_prime + c_prime;
            ans = c_prime + *rb.get_unchecked(0) - tt * *rb.get_unchecked(0);
        }
        Self { x: ans }
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    pub fn Mod2(self, signed: bool) -> Self {
        self.Mod2m(1, signed)
    }
}

/* Arithmetic Routines
 *
 * First we give function call variants which assume that enough
 * the data will not overflow modulo 2^K.
 *
 * Then we give operator versions which ensure this does not happen
 * by performing a reduction operation.
 */

impl<const K: u64> ClearInteger<K> {
    #[inline(always)]
    pub fn negate(self) -> ClearInteger<K> {
        ClearInteger { x: -self.x }
    }

    #[inline(always)]
    pub unsafe fn add(self, other: ClearInteger<K>) -> ClearInteger<K> {
        ClearInteger {
            x: self.x + other.x,
        }
    }

    #[inline(always)]
    pub unsafe fn sub(self, other: ClearInteger<K>) -> ClearInteger<K> {
        ClearInteger {
            x: self.x - other.x,
        }
    }

    #[inline(always)]
    pub unsafe fn mul(self, other: ClearInteger<K>) -> ClearInteger<K> {
        ClearInteger {
            x: self.x * other.x,
        }
    }

    // Now mixed variants

    #[inline(always)]
    pub unsafe fn add_secret<const KAPPA: u64>(
        self,
        other: SecretInteger<K, KAPPA>,
    ) -> SecretInteger<K, KAPPA> {
        SecretInteger {
            x: self.x + other.x,
        }
    }

    #[inline(always)]
    pub unsafe fn sub_secret<const KAPPA: u64>(
        self,
        other: SecretInteger<K, KAPPA>,
    ) -> SecretInteger<K, KAPPA> {
        SecretInteger {
            x: self.x - other.x,
        }
    }

    #[inline(always)]
    pub unsafe fn mul_secret<const KAPPA: u64>(
        self,
        other: SecretInteger<K, KAPPA>,
    ) -> SecretInteger<K, KAPPA> {
        SecretInteger {
            x: self.x * other.x,
        }
    }
}

impl<const K: u64, const KAPPA: u64> SecretInteger<K, KAPPA> {
    #[inline(always)]
    pub fn negate(self) -> Self {
        SecretInteger { x: -self.x }
    }

    #[inline(always)]
    pub unsafe fn add(self, other: Self) -> Self {
        SecretInteger {
            x: self.x + other.x,
        }
    }

    #[inline(always)]
    pub unsafe fn sub(self, other: Self) -> Self {
        SecretInteger {
            x: self.x - other.x,
        }
    }

    #[inline(always)]
    pub unsafe fn mul(self, other: Self) -> Self {
        SecretInteger {
            x: self.x * other.x,
        }
    }

    // Now mixed variants

    #[inline(always)]
    pub unsafe fn add_clear(self, other: ClearInteger<K>) -> Self {
        SecretInteger {
            x: self.x + other.x,
        }
    }

    #[inline(always)]
    pub unsafe fn sub_clear(self, other: ClearInteger<K>) -> Self {
        SecretInteger {
            x: self.x - other.x,
        }
    }

    #[inline(always)]
    pub unsafe fn mul_clear(self, other: ClearInteger<K>) -> Self {
        SecretInteger {
            x: self.x * other.x,
        }
    }
}

/* Now the operator versions .... */
impl<const K: u64> Neg for ClearInteger<K> {
    type Output = Self;
    #[inline(always)]
    fn neg(self) -> Self::Output {
        Self::from(-self.x)
    }
}

impl<const K: u64, const KAPPA: u64> Neg for SecretInteger<K, KAPPA> {
    type Output = Self;
    #[inline(always)]
    fn neg(self) -> Self::Output {
        Self::from(-self.x)
    }
}

impl<const K: u64, const KAPPA: u64> ScaleCmpZ<SecretModp> for SecretInteger<K, KAPPA> {
    #[inline(always)]
    fn ltz(self) -> SecretModp {
        let t = self.Trunc(K - 1, true);
        -t.x
    }
    #[inline(always)]
    fn lez(self) -> SecretModp {
        let e1 = self.ltz();
        let e2 = self.eqz();
        e1 + e2 - e1 * e2
    }
    #[inline(always)]
    fn gtz(self) -> SecretModp {
        let e1 = self.ltz();
        let e2 = self.eqz();
        ClearModp::from(1) - e1 - e2 + e1 * e2
    }
    #[inline(always)]
    fn gez(self) -> SecretModp {
        let t = self.Trunc(K - 1, true);
        ClearModp::from(1) + t.x
    }
    #[inline(always)]
    fn eqz(self) -> SecretModp {
        let (r_dprime, r, rb) = PRandM_Slice(K, K, KAPPA);
        let twok = modp_two_power(K);
        let t = self.x + r_dprime * twok + r;
        let c = t.reveal();
        let cb: Slice<ClearModp> = Slice::bit_decomposition_ClearModp_Signed(c, K);
        let mut d: Slice<SecretModp> = Slice::uninitialized(K);
        for i in 0..K {
            let v = *cb.get_unchecked(i) * *rb.get_unchecked(i);
            d.set(i, &(*cb.get_unchecked(i) + *rb.get_unchecked(i) - v - v));
        }
        ClearModp::from(1) - KOr(&d)
    }
    #[inline(always)]
    fn nez(self) -> SecretModp {
        ClearModp::from(1) - self.eqz()
    }
}

impl<const K: u64> ScaleCmpZ<ClearModp> for ClearInteger<K> {
    #[inline(always)]
    fn ltz(self) -> ClearModp {
        unsafe { __ltzc(self.x) }
    }
    #[inline(always)]
    fn lez(self) -> ClearModp {
        let e1 = self.ltz();
        let e2 = self.eqz();
        e1 + e2 - e1 * e2
    }
    #[inline(always)]
    fn gtz(self) -> ClearModp {
        let e1 = self.ltz();
        let e2 = self.eqz();
        ClearModp::from(1) - e1 - e2 + e1 * e2
    }
    #[inline(always)]
    fn gez(self) -> ClearModp {
        let t = self.Trunc(K - 1, true);
        ClearModp::from(1) + t.x
    }
    #[inline(always)]
    fn eqz(self) -> ClearModp {
        unsafe { __eqzc(self.x) }
    }
    #[inline(always)]
    fn nez(self) -> ClearModp {
        ClearModp::from(1) - self.eqz()
    }
}

impl<const K: u64> Add<ClearInteger<K>> for ClearInteger<K>
where
    ConstU64<{ K + 1 }>: ,
{
    type Output = Self;
    #[inline(always)]
    fn add(self, other: ClearInteger<K>) -> Self::Output {
        let v = ClearInteger::<{ K + 1 }>::from(self.x + other.x);
        //let b = v.ltz();  XXXX FIX LATER XXXX
        let b = ClearModp::from(1);
        let w = v.Mod2m(K - 1, true).rep() - b * modp_two_power(K - 1);
        Self::from(w)
    }
}

impl<const K: u64, const KAPPA: u64> Add<SecretInteger<K, KAPPA>> for ClearInteger<K>
where
    ConstU64<{ K + 1 }>: ,
{
    type Output = SecretInteger<K, KAPPA>;
    #[inline(always)]
    fn add(self, other: SecretInteger<K, KAPPA>) -> Self::Output {
        // XXXX NEEDS FIXING
        let v = SecretInteger::<{ K + 1 }, KAPPA>::from(self.x + other.x);
        let w = v.Mod2m(K - 1, true);
        Self::Output::from(w.x)
    }
}

impl<const K: u64, const KAPPA: u64> Add<SecretInteger<K, KAPPA>> for SecretInteger<K, KAPPA>
where
    ConstU64<{ K + 1 }>: ,
{
    type Output = Self;
    #[inline(always)]
    fn add(self, other: SecretInteger<K, KAPPA>) -> Self::Output {
        // XXXX NEEDS FIXING
        let v = SecretInteger::<{ K + 1 }, KAPPA>::from(self.x + other.x);
        let w = v.Mod2m(K - 1, true);
        Self::from(w.x)
    }
}

impl<const K: u64, const KAPPA: u64> Add<ClearInteger<K>> for SecretInteger<K, KAPPA>
where
    ConstU64<{ K + 1 }>: ,
{
    type Output = SecretInteger<K, KAPPA>;
    #[inline(always)]
    fn add(self, other: ClearInteger<K>) -> Self::Output {
        other + self
    }
}

impl<const K: u64> Sub<ClearInteger<K>> for ClearInteger<K>
where
    ConstU64<{ K + 1 }>: ,
{
    type Output = Self;
    #[inline(always)]
    fn sub(self, other: ClearInteger<K>) -> Self::Output {
        // XXXX NEEDS FIXING
        let v = ClearInteger::<{ K + 1 }>::from(self.x - other.x);
        let w = v.Mod2m(K - 1, true);
        Self::from(w.x)
    }
}

impl<const K: u64, const KAPPA: u64> Sub<SecretInteger<K, KAPPA>> for ClearInteger<K>
where
    ConstU64<{ K + 1 }>: ,
{
    type Output = SecretInteger<K, KAPPA>;
    #[inline(always)]
    fn sub(self, other: SecretInteger<K, KAPPA>) -> Self::Output {
        // XXXX NEEDS FIXING
        let v = SecretInteger::<{ K + 1 }, KAPPA>::from(self.x - other.x);
        let w = v.Mod2m(K - 1, true);
        Self::Output::from(w.x)
    }
}

impl<const K: u64, const KAPPA: u64> Sub<SecretInteger<K, KAPPA>> for SecretInteger<K, KAPPA>
where
    ConstU64<{ K + 1 }>: ,
{
    type Output = Self;
    #[inline(always)]
    fn sub(self, other: SecretInteger<K, KAPPA>) -> Self::Output {
        // XXXX NEEDS FIXING
        let v = SecretInteger::<{ K + 1 }, KAPPA>::from(self.x - other.x);
        let w = v.Mod2m(K, true);
        Self::from(w.x)
    }
}

impl<const K: u64, const KAPPA: u64> Sub<ClearInteger<K>> for SecretInteger<K, KAPPA>
where
    ConstU64<{ K + 1 }>: ,
{
    type Output = SecretInteger<K, KAPPA>;
    #[inline(always)]
    fn sub(self, other: ClearInteger<K>) -> Self::Output {
        self + (-other)
    }
}

impl<const K: u64> Mul<ClearInteger<K>> for ClearInteger<K>
where
    ConstU64<{ 2 * K }>: ,
{
    type Output = Self;
    #[inline(always)]
    fn mul(self, other: ClearInteger<K>) -> Self::Output {
        // XXXX NEEDS CHECKING RE NEGATIVE NUMBERS
        let v = ClearInteger::<{ 2 * K }>::from(self.x * other.x);
        let w = v.Mod2m(K - 1, true);
        Self::from(w.x)
    }
}

impl<const K: u64, const KAPPA: u64> Mul<SecretInteger<K, KAPPA>> for ClearInteger<K>
where
    ConstU64<{ 2 * K }>: ,
{
    type Output = SecretInteger<K, KAPPA>;
    #[inline(always)]
    fn mul(self, other: SecretInteger<K, KAPPA>) -> Self::Output {
        // XXXX NEEDS CHECKING RE NEGATIVE NUMBERS
        let v = SecretInteger::<{ 2 * K }, KAPPA>::from(self.x * other.x);
        let w = v.Mod2m(K - 1, true);
        Self::Output::from(w.x)
    }
}

impl<const K: u64, const KAPPA: u64> Mul<SecretInteger<K, KAPPA>> for SecretInteger<K, KAPPA>
where
    ConstU64<{ 2 * K }>: ,
{
    type Output = Self;
    #[inline(always)]
    fn mul(self, other: SecretInteger<K, KAPPA>) -> Self::Output {
        // XXXX NEEDS CHECKING RE NEGATIVE NUMBERS
        let v = SecretInteger::<{ 2 * K }, KAPPA>::from(self.x * other.x);
        let w = v.Mod2m(K - 1, true);
        Self::from(w.x)
    }
}

impl<const K: u64, const KAPPA: u64> Mul<ClearInteger<K>> for SecretInteger<K, KAPPA>
where
    ConstU64<{ 2 * K }>: ,
{
    type Output = SecretInteger<K, KAPPA>;
    #[inline(always)]
    fn mul(self, other: ClearInteger<K>) -> Self::Output {
        other * self
    }
}

/* Compute 2^a when a \in [0...k) */
#[inline(always)]
#[allow(non_snake_case)]
pub fn Pow2<const K: u64, const KAPPA: u64>(a: SecretModp) -> SecretModp {
    if K >= 512 {
        core::panic!("K too large")
    }
    let len = ceil_log_2(K as u32) as u64;
    // We do a bit of overkill computation here, but it avoids a const issue in Rust
    let mut d = BitDec::<9, 9, KAPPA>(a);
    for i in 0..len {
        d.set(
            i,
            &(modp_two_power(1 << i) * *d.get_unchecked(i) + ClearModp::from(1)
                - *d.get_unchecked(i)),
        );
    }
    KOpL(mul_op, d.addr(0), len as i64)
}

/* As the next function should always be inlined the multiple
 * return values should work
 */
/* For a in [0..k) this converts it into unary form */
#[inline(always)]
#[allow(non_snake_case)]
pub fn B2U<const K: u64, const KAPPA: u64>(a: SecretModp) -> (Slice<SecretModp>, SecretModp) {
    let pow2a = Pow2::<K, KAPPA>(a);
    let random = PRandM_Slice(K, K, KAPPA);
    let v = pow2a + modp_two_power(K) * random.0 + random.1;
    let c = v.reveal();
    let cb: Slice<ClearModp> = Slice::bit_decomposition_ClearModp_Signed(c, K);
    let mut x: Slice<SecretModp> = Slice::uninitialized(K);
    for i in 0..K {
        let v = *cb.get_unchecked(i) * *random.2.get_unchecked(i);
        x.set(
            i,
            &(*cb.get_unchecked(i) + *random.2.get_unchecked(i) - v - v),
        );
    }
    let mut y = x.PreOr();
    for i in 0..K {
        y.set(i, &(ClearModp::from(1) - *y.get_unchecked(i)));
    }
    (y, pow2a)
}

impl<const K: u64> ClearInteger<K> {
    #[inline(always)]
    #[allow(non_snake_case)]
    pub fn Trunc(self, M: u64, SIGNED: bool) -> Self {
        let c2m = modp_two_power(M);
        let mut t = self.x;
        if SIGNED {
            let twok = modp_two_power(K - 1);
            t = t + twok;
        }
        t = t % c2m;
        let d = (self.x - t) / c2m;
        Self { x: d }
    }
}

impl<const K: u64, const KAPPA: u64> SecretInteger<K, KAPPA> {
    #[allow(non_snake_case)]
    pub fn TruncPr(self, M: u64, SIGNED: bool) -> Self {
        let (r_dprime, r, _rb) = PRandM_Slice(K, M, KAPPA);
        let c2m = modp_two_power(M);
        let mut t = r_dprime * c2m + self.x + r;
        if SIGNED {
            let twok = modp_two_power(K - 1);
            t = t + twok;
        }
        let c = t.reveal();
        let c_prime = c % c2m;
        let d = self.x - c_prime + r;
        let d = d / c2m;
        Self { x: d }
    }

    #[allow(non_snake_case)]
    pub fn Trunc(self, M: u64, SIGNED: bool) -> Self {
        let a_dash = self.Mod2m(M, SIGNED);
        let c2m = modp_two_power(M);
        let d = (self.x - a_dash.x) / c2m;
        Self { x: d }
    }

    #[allow(non_snake_case)]
    pub fn TruncRoundNearest(self, M: u64, SIGNED: bool) -> Self {
        let a: SecretModp;
        if M == 1 {
            let b = self.Mod2(true);
            a = (self.x + b.x) / ClearModp::from(2);
        } else {
            let (r_dprime, r_prime, r) = PRandM_Slice(K, M, KAPPA);
            let two = ClearModp::from(2);
            let c2m = modp_two_power(M);
            let c2m1 = modp_two_power(M - 1);
            let mut t = self.x + r_dprime * c2m + r_prime;
            if SIGNED {
                let twok = modp_two_power(K - 1);
                t = t + twok;
            }
            let c = t.reveal();
            let c_prime = c % c2m1;
            let u = BitLT(c_prime, &r, r.len() - 1);
            let bit = ((c - c_prime) / c2m1) % two;
            let xor = bit + u - two * bit * u;
            let prod = xor * *r.get_unchecked(M - 1);
            let u_prime = bit * u + u - two * bit * u + *r.get_unchecked(M - 1) - prod;
            let a_prime = (c % c2m) - r_prime + c2m * u_prime;
            let d = (self.x - a_prime) / c2m;
            let rounding = xor + *r.get_unchecked(M - 1) - two * prod;
            a = d + rounding;
        }
        Self { x: a }
    }

    //  This computes (t = self/2^m, self-pow2*t, pow2) where pow2 = 2^m
    #[allow(non_snake_case)]
    pub fn ObliviousTrunc(self, m: SecretModp) -> Array<SecretModp, 3> {
        let mut ans: Array<SecretModp, 3> = Array::uninitialized();
        if K == 1 {
            ans.set(0, &(self.x * (ClearModp::from(1) - m)));
            ans.set(1, &(self.x * m));
            ans.set(2, &(ClearModp::from(1) + m));
            return ans;
        }
        unsafe { __reqbl((K + KAPPA) as u32) };
        let (x, pow2m) = B2U::<K, KAPPA>(m);
        let rk: SecretModp = PRandInt(KAPPA);
        let mut r: Array<SecretModp, K> = Array::uninitialized();
        let mut r_prime = SecretModp::from(0);
        let mut r_dprime = SecretModp::from(0);
        let two = ClearModp::from(2);
        let mut twop = ClearModp::from(1);
        for i in 0..K {
            r.set(i, &SecretModp::get_random_bit());
            let t1 = twop * *r.get_unchecked(i);
            let t2 = t1 * *x.get_unchecked(i);
            r_prime = r_prime + t2;
            r_dprime = r_dprime + t1 - t2;
            twop = twop * two;
        }
        let c2k = modp_two_power(K);
        r_dprime = r_dprime + c2k * rk;
        let t = self.x + r_dprime + r_prime;
        let c = t.reveal();
        let mut c_dprime = SecretModp::from(0);
        twop = ClearModp::from(2);
        for i in 1..K {
            let ci = c % twop;
            c_dprime = c_dprime + ci * (*x.get_unchecked(i - 1) - *x.get_unchecked(i));
            twop = twop * two;
        }
        let test: SecretInteger<K, KAPPA> = SecretInteger::from(c_dprime - r_prime);
        let d = test.ltz();

        let pow2inv = Inv(pow2m);
        let b = (self.x - c_dprime + r_prime) * pow2inv - d;
        ans.set(0, &b);
        let b = c_dprime - r_prime + pow2m * d;
        ans.set(1, &b);
        ans.set(2, &pow2m);
        ans
    }
}

impl<const K: u64, const KAPPA: u64> ScaleCmp<Self, SecretModp> for SecretInteger<K, KAPPA>
where
    ConstU64<{ K + 1 }>: ,
{
    #[inline(always)]
    fn lt(self, other: Self) -> SecretModp {
        let temp = unsafe { self.sub(other) };
        temp.ltz()
    }
    #[inline(always)]
    fn le(self, other: Self) -> SecretModp {
        let temp = unsafe { self.sub(other) };
        temp.lez()
    }
    #[inline(always)]
    fn gt(self, other: Self) -> SecretModp {
        let temp = unsafe { self.sub(other) };
        temp.gtz()
    }
    #[inline(always)]
    fn ge(self, other: Self) -> SecretModp {
        let temp = unsafe { self.sub(other) };
        temp.gez()
    }
    #[inline(always)]
    fn eq(self, other: Self) -> SecretModp {
        let temp = unsafe { self.sub(other) };
        temp.eqz()
    }
    #[inline(always)]
    fn ne(self, other: Self) -> SecretModp {
        let temp = unsafe { self.sub(other) };
        temp.nez()
    }
}

impl<const K: u64> ScaleCmp<Self, ClearModp> for ClearInteger<K>
where
    ConstU64<{ K + 1 }>: ,
{
    #[inline(always)]
    fn lt(self, other: Self) -> ClearModp {
        let temp = unsafe { self.sub(other) };
        temp.ltz()
    }
    #[inline(always)]
    fn le(self, other: Self) -> ClearModp {
        let temp = unsafe { self.sub(other) };
        temp.lez()
    }
    #[inline(always)]
    fn gt(self, other: Self) -> ClearModp {
        let temp = unsafe { self.sub(other) };
        temp.gtz()
    }
    #[inline(always)]
    fn ge(self, other: Self) -> ClearModp {
        let temp = unsafe { self.sub(other) };
        temp.gez()
    }
    #[inline(always)]
    fn eq(self, other: Self) -> ClearModp {
        let temp = unsafe { self.sub(other) };
        temp.eqz()
    }
    #[inline(always)]
    fn ne(self, other: Self) -> ClearModp {
        let temp = unsafe { self.sub(other) };
        temp.nez()
    }
}
