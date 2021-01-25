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
        Self::from(SecretModp::from(a.x))
    }
}

impl<const K: u64> From<i64> for ClearInteger<K> {
    #[inline(always)]
    fn from(a: i64) -> Self {
        Self::from(ClearModp::from(a))
    }
}

impl<const K: u64> From<ClearModp> for ClearInteger<K> {
    #[inline(always)]
    fn from(x: ClearModp) -> Self {
        Self { x }
    }
}

impl<const K: u64, const KAPPA: u64> From<SecretModp> for SecretInteger<K, KAPPA> {
    #[inline(always)]
    fn from(x: SecretModp) -> Self {
        Self { x }
    }
}

/* Convert sizes
 * - Assumes user knows what they are doing
 */

impl<const K: u64> ClearInteger<K> {
    #[inline(always)]
    pub fn recast<const K2: u64>(self) -> ClearInteger<K2> {
        ClearInteger { x: self.x }
    }
}

impl<const K: u64, const KAPPA: u64> SecretInteger<K, KAPPA> {
    #[inline(always)]
    pub fn recast<const K2: u64>(self) -> SecretInteger<K2, KAPPA> {
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

/* Print a clear integer */

impl<const K: u64> Print for ClearInteger<K> {
    #[inline(always)]
    fn print(self) {
        self.x.print();
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
    pub fn Mod2m<const M: u64, const SIGNED: bool>(
        self,
        _: ConstU64<M>,
        _: ConstBool<SIGNED>,
    ) -> ClearInteger<K> {
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
    pub fn Mod2<const SIGNED: bool>(self, _: ConstBool<SIGNED>) -> ClearInteger<K> {
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
    pub fn Mod2m<const M: u64, const SIGNED: bool>(
        self,
        _: ConstU64<M>,
        _: ConstBool<SIGNED>,
    ) -> Self {
        scale::assert!(M < K);
        let (r_dprime, r, rb) = PRandM::<K, M, KAPPA>();
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
            let u = BitLT(c_prime, rb.iter());
            let t4 = u * c2m;
            let t5 = c_prime - r;
            ans = t5 + t4;
        } else {
            let tt = c_prime + c_prime;
            ans = c_prime + rb.get(0) - tt * rb.get(0);
        }
        Self { x: ans }
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    pub fn Mod2<const SIGNED: bool>(self, signed: ConstBool<SIGNED>) -> Self {
        self.Mod2m(ConstU64::<1>, signed)
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
    pub unsafe fn negate(self) -> ClearInteger<K> {
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
    pub unsafe fn negate(self) -> Self {
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

impl<const K: u64> Add<ClearInteger<K>> for ClearInteger<K>
where
    ConstU64<{ K + 1 }>: ,
{
    type Output = Self;
    #[inline(always)]
    fn add(self, other: ClearInteger<K>) -> Self::Output {
        let v = ClearInteger::<{ K + 1 }>::from(self.x + other.x);
        let w = v.Mod2m(ConstU64::<K>, ConstBool::<true>);
        Self::from(w.x)
    }
}

impl<const K: u64, const KAPPA: u64> Add<SecretInteger<K, KAPPA>> for ClearInteger<K>
where
    ConstU64<{ K + 1 }>: ,
{
    type Output = SecretInteger<K, KAPPA>;
    #[inline(always)]
    fn add(self, other: SecretInteger<K, KAPPA>) -> Self::Output {
        let v = SecretInteger::<{ K + 1 }, KAPPA>::from(self.x + other.x);
        let w = v.Mod2m(ConstU64::<K>, ConstBool::<true>);
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
        let v = SecretInteger::<{ K + 1 }, KAPPA>::from(self.x + other.x);
        let w = v.Mod2m(ConstU64::<K>, ConstBool::<true>);
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
        let v = ClearInteger::<{ K + 1 }>::from(self.x - other.x);
        let w = v.Mod2m(ConstU64::<K>, ConstBool::<true>);
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
        let v = SecretInteger::<{ K + 1 }, KAPPA>::from(self.x - other.x);
        let w = v.Mod2m(ConstU64::<K>, ConstBool::<true>);
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
        let v = SecretInteger::<{ K + 1 }, KAPPA>::from(self.x - other.x);
        let w = v.Mod2m(ConstU64::<K>, ConstBool::<true>);
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
        other - self
    }
}

impl<const K: u64> Mul<ClearInteger<K>> for ClearInteger<K>
where
    ConstU64<{ 2 * K }>: ,
{
    type Output = Self;
    #[inline(always)]
    fn mul(self, other: ClearInteger<K>) -> Self::Output {
        let v = ClearInteger::<{ 2 * K }>::from(self.x * other.x);
        let w = v.Mod2m(ConstU64::<K>, ConstBool::<true>);
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
        let v = SecretInteger::<{ 2 * K }, KAPPA>::from(self.x * other.x);
        let w = v.Mod2m(ConstU64::<K>, ConstBool::<true>);
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
        let v = SecretInteger::<{ 2 * K }, KAPPA>::from(self.x * other.x);
        let w = v.Mod2m(ConstU64::<K>, ConstBool::<true>);
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
pub fn Pow2<const K: u64, const KAPPA: u64>(a: SecretModp) -> SecretModp
where
    ConstU64<{ CeilLog2::<K>::RESULT }>: ,
{
    let mut d = BitDec::<{ CeilLog2::<K>::RESULT }, { CeilLog2::<K>::RESULT }, KAPPA>(a);
    for i in 0..CeilLog2::<K>::RESULT {
        d.set(
            i,
            &(modp_two_power(1 << i) * d.get(i) + ClearModp::from(1) - d.get(i)),
        );
    }
    KOpL(mul_op, &d)
}

/* For a in [0..k) this converts it into unary form */
#[inline(always)]
#[allow(non_snake_case)]
pub fn B2U<const K: u64, const KAPPA: u64>(a: SecretModp) -> (Slice<SecretModp>, SecretModp)
where
    ConstU64<{ CeilLog2::<K>::RESULT }>: ,
{
    let pow2a = Pow2::<K, KAPPA>(a);
    let random = PRandM::<K, K, KAPPA>();
    let v = pow2a + modp_two_power(K) * random.0 + random.1;
    let c = v.reveal();
    let cb: Slice<ClearModp> = bits(c, K);
    let mut x: Slice<SecretModp> = Slice::uninitialized(K);
    for i in 0..K {
        let v = cb.get(i) * random.2.get(i);
        x.set(i, &(cb.get(i) + random.2.get(i) - v - v));
    }
    let mut y = PreOr(&x);
    for i in 0..K {
        y.set(i, &(ClearModp::from(1) - y.get(i)));
    }
    (y, pow2a)
}

impl<const K: u64> ClearInteger<K> {
    #[inline(always)]
    #[allow(non_snake_case)]
    pub fn Trunc<const M: u64, const SIGNED: bool>(
        self,
        _: ConstU64<M>,
        _: ConstBool<SIGNED>,
    ) -> Self {
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
    #[inline(always)]
    #[allow(non_snake_case)]
    pub fn TruncPr<const M: u64, const SIGNED: bool>(
        self,
        _: ConstU64<M>,
        _: ConstBool<SIGNED>,
    ) -> Self {
        let (r_dprime, r, _rb) = PRandM::<K, M, KAPPA>();
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

    #[inline(always)]
    #[allow(non_snake_case)]
    pub fn Trunc<const M: u64, const SIGNED: bool>(
        self,
        _: ConstU64<M>,
        _: ConstBool<SIGNED>,
    ) -> Self {
        let a_dash = self.Mod2m(ConstU64::<M>, ConstBool::<SIGNED>);
        let c2m = modp_two_power(M);
        let d = (self.x - a_dash.x) / c2m;
        Self { x: d }
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    pub fn TruncRoundNearest<const M: u64, const SIGNED: bool>(
        self,
        _: ConstU64<M>,
        _: ConstBool<SIGNED>,
    ) -> Self
    where
        ConstU64<{ M - 1 }>: ,
    {
        let a: SecretModp;
        if M == 1 {
            let b = self.Mod2(ConstBool::<true>);
            a = (self.x + b.x) / ClearModp::from(2);
        } else {
            let (r_dprime, r_prime, r) = PRandM::<K, M, KAPPA>();
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
            let u = BitLT(c_prime, r.static_slice_from_start::<1>().iter());
            let bit = ((c - c_prime) / c2m1) % two;
            let xor = bit + u - two * bit * u;
            let prod = xor * r.get(M - 1);
            let u_prime = bit * u + u - two * bit * u + r.get(M - 1) - prod;
            let a_prime = (c % c2m) - r_prime + c2m * u_prime;
            let d = (self.x - a_prime) / c2m;
            let rounding = xor + r.get(M - 1) - two * prod;
            a = d + rounding;
        }
        Self { x: a }
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    pub fn ObliviousTrunc(self, m: SecretModp) -> Self
    where
        ConstU64<{ CeilLog2::<K>::RESULT }>: ,
        ConstU64<{ K + 1 }>: ,
        ConstU64<{ K - 1 }>: ,
    {
        if K == 1 {
            let ans = Self {
                x: self.x * (ClearModp::from(1) - m),
            };
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
            let t1 = twop * r.get(i);
            let t2 = t1 * x.get(i);
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
            c_dprime = c_dprime + ci * (x.get(i - 1) - x.get(i));
            twop = twop * two;
        }
        let lhs: SecretInteger<K, KAPPA> = SecretInteger::from(c_dprime);
        let rhs: SecretInteger<K, KAPPA> = SecretInteger::from(r_prime);
        let d = lhs.lt(rhs);
        let pow2inv = Inv(pow2m);
        let b = (self.x - c_dprime + r_prime) * pow2inv - d;
        Self { x: b }
    }
}

impl<const K: u64, const KAPPA: u64> ModpSecretCmpZ for SecretInteger<K, KAPPA>
where
    ConstU64<{ K - 1 }>: ,
    ConstU64<{ K + 1 }>: ,
{
    #[inline(always)]
    fn ltz(self) -> SecretModp {
        let t = self.Trunc(ConstU64::<{ K - 1 }>, ConstBool::<true>);
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
        let t = self.Trunc(ConstU64::<{ K - 1 }>, ConstBool::<true>);
        ClearModp::from(1) + t.x
    }
    #[inline(always)]
    fn eqz(self) -> SecretModp {
        let (r_dprime, r, rb) = PRandM::<K, K, KAPPA>();
        let twok = modp_two_power(K);
        let t = self.x + r_dprime * twok + self.x + r;
        let c = t.reveal();
        let cb: Slice<ClearModp> = bits(c, K);
        let mut d: Slice<SecretModp> = Slice::uninitialized(K);
        for i in 0..K {
            let v = cb.get(i) * rb.get(i);
            d.set(i, &(cb.get(i) + rb.get(i) - v - v));
        }
        ClearModp::from(1) - KOr(&d)
    }
    #[inline(always)]
    fn nez(self) -> SecretModp {
        ClearModp::from(1) - self.eqz()
    }
}

impl<const K: u64, const KAPPA: u64> ModpSecretCmp<Self> for SecretInteger<K, KAPPA>
where
    ConstU64<{ K + 1 }>: ,
    ConstU64<{ K - 1 }>: ,
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
