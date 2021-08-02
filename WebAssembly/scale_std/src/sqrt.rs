// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use crate::array::*;
use crate::bit_protocols::*;
use crate::fixed_point::*;
use crate::floating_point::*;
use crate::integer::*;
use crate::math::*;
use crate::slice::*;
use core::cmp::max;
use scale::*;

/***************************************************
 * Helper Routines for Sqrt for Fixed Point Values *
 ***************************************************/

// Returns the index array of MSB of size K
#[allow(non_snake_case)]
fn Secret_MSB<const K: u64, const KAPPA: u64>(
    b: SecretModp,
    _: ConstU64<K>,
    _: ConstU64<KAPPA>,
) -> Array<SecretModp, K> {
    let x_order = BitDec::<K, K, KAPPA>(b);
    let x = x_order.reverse();
    let y = x.PreOr();
    let mut z: Array<SecretModp, K> = Array::uninitialized();
    for i in 0..(K - 1) {
        z.set(
            i,
            &(*y.get_unchecked(K - 1 - i) - *y.get_unchecked(K - 2 - i)),
        );
    }
    z.set(K - 1, &*y.get_unchecked(0));
    z
}

// XXXX Seems very wasteful to define the same function
// roughly the same again. Any way of fixing this with
// generics?
#[allow(non_snake_case)]
fn Clear_MSB<const K: u64>(b: ClearModp, _: ConstU64<K>) -> Array<ClearModp, K> {
    let x_order: Slice<ClearModp> = Slice::bit_decomposition_ClearModp_Signed(b, K);
    let x = x_order.reverse();
    let y = x.PreOr();
    let mut z: Array<ClearModp, K> = Array::uninitialized();
    for i in 0..(K - 1) {
        z.set(
            i,
            &(*y.get_unchecked(K - 1 - i) - *y.get_unchecked(K - 2 - i)),
        );
    }
    z.set(K - 1, &*y.get_unchecked(0));
    z
}

// Similar to norm_SQ [see Documentation],
// but saves rounds by not calculating m, v and c.
#[allow(non_snake_case)]
fn Secret_norm_simplified_SQ<const K: u64, const KAPPA: u64>(
    b: SecretModp,
    _: ConstU64<K>,
    _: ConstU64<KAPPA>,
) -> Array<SecretModp, 2> {
    let z = Secret_MSB(b, ConstU64::<K>, ConstU64::<KAPPA>);

    // Construct m
    let mut m_odd = SecretModp::from(0);
    for i in (0..K).step_by(2) {
        m_odd = m_odd + *z.get_unchecked(i);
    }

    // Construct w, changes from what is on the paper
    let mut w = SecretModp::from(0);
    for i in 1..(K / 2) {
        let wi = *z.get_unchecked(2 * i - 1) + *z.get_unchecked(2 * i);
        w = w + modp_two_power(i) * wi;
    }
    let mut ans: Array<SecretModp, 2> = Array::uninitialized();
    ans.set(0, &m_odd);
    ans.set(1, &w);
    ans
}

// XXXX Seems very wasteful to define the same function
// roughly the same again. Any way of fixing this with
// generics?
#[allow(non_snake_case)]
fn Clear_norm_simplified_SQ<const K: u64>(b: ClearModp, _: ConstU64<K>) -> Array<ClearModp, 2> {
    let z = Clear_MSB(b, ConstU64::<K>);

    // Construct m
    let mut m_odd = ClearModp::from(0);
    for i in (0..K).step_by(2) {
        m_odd = m_odd + *z.get_unchecked(i);
    }

    // Construct w, changes from what is on the paper
    let mut w = ClearModp::from(0);
    for i in 1..(K / 2 + 1) {
        let wi = *z.get_unchecked(2 * i - 1) + *z.get_unchecked(2 * i);
        w = w + modp_two_power(i) * wi;
    }
    let mut ans: Array<ClearModp, 2> = Array::uninitialized();
    ans.set(0, &m_odd);
    ans.set(1, &w);
    ans
}

/* For use when 3K>=3F, i.e. K>=F
 * Note in the Rust implementation we insist that K>=F
 */
impl<const K: u64, const F: u64, const KAPPA: u64> Sqrt for SecretFixed<K, F, KAPPA>
where
    ConstU64<{ 2 * K }>: ,
    ConstU64<{ 2 * F }>: ,
    ConstU64<{ K + 1 }>: ,
    ConstU64<{ K - 1 }>: ,
    ConstU64<{ SecretFixed::<K, F, KAPPA>::THETA }>: ,
{
    #[allow(non_snake_case)]
    fn sqrt(self) -> SecretFixed<K, F, KAPPA> {
        // THETA is log_2(K/3.5) we want
        //   theta = max(log_2 K, 6) = max(THETA+1.2,6) approx max(THETA+2,6)
        let theta = max(SecretFixed::<K, F, KAPPA>::THETA + 2, 6);
        let Fmod2 = ClearModp::from((F % 2) as i64);
        let SnsSQ = Secret_norm_simplified_SQ(self.rep().rep(), ConstU64::<K>, ConstU64::<KAPPA>);
        let mut m_odd = *SnsSQ.get_unchecked(0);
        let mut w = *SnsSQ.get_unchecked(1);
        m_odd = m_odd + (ClearModp::from(1) - m_odd - m_odd) * Fmod2;
        let m_odd_fx: SecretFixed<K, F, KAPPA> = SecretFixed::from(m_odd);
        w = w + w * (ClearModp::from(1) - m_odd) * Fmod2;
        let t = (F - F % 2) / 2;
        let vv: SecretInteger<K, KAPPA> = SecretInteger::from(w * modp_two_power(t));
        let mut ww: SecretFixed<K, F, KAPPA> = SecretFixed::set(vv);
        let sqrt2: ClearFixed<K, F> = ClearFixed::from(1.4142135623730950488016887242096980786);
        ww = (sqrt2 * ww - ww) * m_odd_fx + ww;
        let y_0: SecretFixed<K, F, KAPPA> = SecretFixed::from(1.0) / ww;

        let half: ClearFixed<K, F> = ClearFixed::from(0.5);
        let four: ClearFixed<K, F> = ClearFixed::from(4.0);
        let three: ClearFixed<K, F> = ClearFixed::from(3.0);
        let three_half: ClearFixed<K, F> = ClearFixed::from(1.5);
        let mut g = self * y_0;
        let mut h = y_0 * half;
        let mut gh = g * h;

        for _i in 1..(theta - 2) {
            let r = three_half - gh;
            g = g * r;
            h = h * r;
            gh = g * h;
        }

        // Final Newton Iteration
        let r = three_half - gh;
        h = h * r;
        let H = four * h * h;
        g = h * (three - H * self) * self;

        g
    }
}

/* For use when 3K>=3F, i.e. K>=F
 * Note in the Rust implementation we insist that K>=F
 */
impl<const K: u64, const F: u64> Sqrt for ClearFixed<K, F>
where
    ConstU64<{ 2 * K }>: ,
    ConstU64<{ 2 * F }>: ,
    ConstU64<{ K + 1 }>: ,
    ConstU64<{ K - 1 }>: ,
    ConstU64<{ ClearFixed::<K, F>::THETA }>: ,
    ConstI32<{ f_as_i32(F) }>: ,
    ConstI32<{ f_as_i32(K) }>: ,
{
    #[allow(non_snake_case)]
    fn sqrt(self) -> ClearFixed<K, F> {
        // THETA is log_2(K/3.5) we want
        //   theta = max(log_2 K, 6) = max(THETA+1.2,6) approx max(THETA+2,6)
        let theta = max(ClearFixed::<K, F>::THETA + 2, 6);
        let Fmod2 = ClearModp::from((F % 2) as i64);
        let SnsSQ = Clear_norm_simplified_SQ(self.rep().rep(), ConstU64::<K>);
        let mut m_odd = *SnsSQ.get_unchecked(0);
        let mut w = *SnsSQ.get_unchecked(1);
        m_odd = m_odd + (ClearModp::from(1) - m_odd - m_odd) * Fmod2;
        let m_odd_fx: ClearFixed<K, F> = ClearFixed::from(m_odd);
        w = w + w * (ClearModp::from(1) - m_odd) * Fmod2;
        let t = (F - F % 2) / 2;
        let vv: ClearInteger<K> = ClearInteger::from(w * modp_two_power(t));
        let mut ww: ClearFixed<K, F> = ClearFixed::set(vv);
        let sqrt2: ClearFixed<K, F> = ClearFixed::from(1.4142135623730950488016887242096980786);
        ww = (sqrt2 * ww - ww) * m_odd_fx + ww;
        let y_0: ClearFixed<K, F> = ClearFixed::from(1.0) / ww;

        let half: ClearFixed<K, F> = ClearFixed::from(0.5);
        let four: ClearFixed<K, F> = ClearFixed::from(4.0);
        let three: ClearFixed<K, F> = ClearFixed::from(3.0);
        let three_half: ClearFixed<K, F> = ClearFixed::from(1.5);
        let mut g = self * y_0;
        let mut h = y_0 * half;
        let mut gh = g * h;

        for _i in 1..(theta - 2) {
            let r = three_half - gh;
            g = g * r;
            h = h * r;
            gh = g * h;
        }

        // Final Newton Iteration
        let r = three_half - gh;
        h = h * r;
        let H = four * h * h;
        g = h * (three - H * self) * self;

        g
    }
}

/******************************************************
 * Helper Routines for Sqrt for Floating Point Values *
 ******************************************************/

impl<const V: u64, const P: u64, const DETECT_OVERFLOW: bool> ClearFloat<V, P, DETECT_OVERFLOW> {
    // theta = int(ceil(log(V/5.4) / log(2)));
    pub const THETA: u64 = {
        match V {
            0..6 => 0,
            6..11 => 0,
            11..22 => 2,
            22..44 => 3,
            44..87 => 4,
            87..173 => 5,
            173..346 => 6,
            346..692 => 7,
            692.. => core::panic!("V too large"),
        }
    };
}

impl<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool>
    SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
{
    // theta = int(ceil(log(V/5.4) / log(2)));
    pub const THETA: u64 = {
        match V {
            0..6 => 0,
            6..11 => 0,
            11..22 => 2,
            22..44 => 3,
            44..87 => 4,
            87..173 => 5,
            173..346 => 6,
            346..692 => 7,
            692.. => core::panic!("V too large"),
        }
    };
}

impl<const V: u64, const P: u64, const DETECT_OVERFLOW: bool> Sqrt
    for ClearFloat<V, P, DETECT_OVERFLOW>
where
    ConstU64<{ ClearFloat::<V, P, DETECT_OVERFLOW>::THETA }>: ,
    ConstU64<{ V - 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ V + 2 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ P + 1 }>: ,
{
    #[allow(non_snake_case)]
    fn sqrt(self) -> ClearFloat<V, P, DETECT_OVERFLOW> {
        let one = ClearModp::from(1);
        let zero = ClearModp::from(0);
        let b_slice: Slice<ClearModp> = Slice::bit_decomposition_ClearModp_Signed(self.p(), 1);
        let b = *b_slice.get_unchecked(0);
        // c = b XOR V%2, d = b OR V%2
        let mut c = b;
        let mut d = b;
        if V % 2 == 1 {
            d = d + one - b;
            c = d - b;
        }
        // p = (self.p() - b)/2 + [l/2] + d
        let p = (self.p() - b) / ClearModp::from(2) + ClearModp::from(((V - V % 2) / 2) as i64) + d;
        let alpha: ClearFloat<V, P, DETECT_OVERFLOW> = ClearFloat::from(-0.8099868542);
        let beta: ClearFloat<V, P, DETECT_OVERFLOW> = ClearFloat::from(1.787727479);

        let mut temp: Array<ClearModp, 5> = Array::uninitialized();
        temp.set(0, &self.v());
        temp.set(1, &ClearModp::from(-(V as i64)));
        temp.set(2, &zero);
        temp.set(3, &zero);
        temp.set(4, &zero);
        let vv: ClearFloat<V, P, DETECT_OVERFLOW> = ClearFloat::set(temp);

        let tLm2 = modp_two_power(V - 2);
        let tLm1 = tLm2 + tLm2;
        let tt = ClearModp::from(3) * tLm2;

        let mut temp: Array<ClearModp, 5> = Array::uninitialized();
        temp.set(0, &ClearModp::from(tt));
        temp.set(1, &ClearModp::from(-(V as i64) + 1));
        temp.set(2, &zero);
        temp.set(3, &zero);
        temp.set(4, &zero);
        let tt: ClearFloat<V, P, DETECT_OVERFLOW> = ClearFloat::set(temp);

        let mut v2 = &vv * &alpha;
        let v0 = &v2 + &beta;
        let mut vg = &vv * &v0;

        let mut temp: Array<ClearModp, 5> = Array::uninitialized();
        temp.set(0, &v0.v());
        temp.set(1, &(v0.p() - one));
        temp.set(2, &v0.z());
        temp.set(3, &v0.s());
        temp.set(4, &v0.err());
        let mut vh: ClearFloat<V, P, DETECT_OVERFLOW> = ClearFloat::set(temp);

        for _i in 0..ClearFloat::<V, P, DETECT_OVERFLOW>::THETA {
            v2 = &vg * &vh;
            v2 = &tt - &v2;
            vg = &vg * &v2;
            vh = &vh * &v2;
        }

        let vh2 = &vh * &vh;
        v2 = &vv * &vh2;

        let mut temp: Array<ClearModp, 5> = Array::uninitialized();
        temp.set(0, &v2.v());
        temp.set(1, &(v2.p() + one));
        temp.set(2, &v2.z());
        temp.set(3, &v2.s());
        temp.set(4, &v2.err());
        let gg: ClearFloat<V, P, DETECT_OVERFLOW> = ClearFloat::set(temp);

        v2 = &tt - &gg;
        vh = &vh * &v2;

        let mut temp: Array<ClearModp, 5> = Array::uninitialized();
        temp.set(0, &vh.v());
        temp.set(1, &(vh.p() + one));
        temp.set(2, &vh.z());
        temp.set(3, &vh.s());
        temp.set(4, &vh.err());
        let gg: ClearFloat<V, P, DETECT_OVERFLOW> = ClearFloat::set(temp);

        v2 = &vv * &gg;

        let sqrt2: ClearFloat<V, P, DETECT_OVERFLOW> =
            ClearFloat::from(1.4142135623730950488016887242096980786);
        let vv = tLm1 * (one - c) + sqrt2.v() * c;
        let pp = -(one - c) * (ClearModp::from(V as i64 - 1)) + sqrt2.p() * c;

        let mut temp: Array<ClearModp, 5> = Array::uninitialized();
        temp.set(0, &vv);
        temp.set(1, &pp);
        temp.set(2, &zero);
        temp.set(3, &zero);
        temp.set(4, &zero);
        let gg: ClearFloat<V, P, DETECT_OVERFLOW> = ClearFloat::set(temp);

        v2 = &v2 * &gg;

        let p = (v2.p() + p) * (one - self.z());
        let v = v2.v() * (one - self.z());
        let err = self.err() + self.s();

        let mut temp: Array<ClearModp, 5> = Array::uninitialized();
        temp.set(0, &v);
        temp.set(1, &p);
        temp.set(2, &self.z());
        temp.set(3, &self.s());
        temp.set(4, &err);
        let ans: ClearFloat<V, P, DETECT_OVERFLOW> = ClearFloat::set(temp);

        ans
    }
}

impl<const V: u64, const P: u64, const KAPPA: u64, const DETECT_OVERFLOW: bool> Sqrt
    for SecretFloat<V, P, KAPPA, DETECT_OVERFLOW>
where
    ConstU64<{ SecretFloat::<V, P, KAPPA, DETECT_OVERFLOW>::THETA }>: ,
    ConstU64<{ V - 1 }>: ,
    ConstU64<{ V + 1 }>: ,
    ConstU64<{ V + 2 }>: ,
    ConstU64<{ 2 * V }>: ,
    ConstU64<{ P + 1 }>: ,
{
    #[allow(non_snake_case)]
    fn sqrt(self) -> SecretFloat<V, P, KAPPA, DETECT_OVERFLOW> {
        let one = ClearModp::from(1);
        let zero = ClearModp::from(0);
        let szero = SecretModp::from(0);
        let b_slice = BitDec::<V, 1, KAPPA>(self.p());
        let b = *b_slice.get_unchecked(0);
        // c = b XOR V%2, d = b OR V%2
        let mut c = b;
        let mut d = b;
        if V % 2 == 1 {
            d = d + one - b;
            c = d - b;
        }
        // p = (self.p() - b)/2 + [l/2] + d
        let p = (self.p() - b) / ClearModp::from(2) + ClearModp::from(((V - V % 2) / 2) as i64) + d;
        let alpha: ClearFloat<V, P, DETECT_OVERFLOW> = ClearFloat::from(-0.8099868542);
        let beta: ClearFloat<V, P, DETECT_OVERFLOW> = ClearFloat::from(1.787727479);

        let mut temp: Array<SecretModp, 5> = Array::uninitialized();
        temp.set(0, &self.v());
        temp.set(1, &SecretModp::from(-(V as i64)));
        temp.set(2, &szero);
        temp.set(3, &szero);
        temp.set(4, &szero);
        let vv: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW> = SecretFloat::set(temp);

        let tLm2 = modp_two_power(V - 2);
        let tLm1 = tLm2 + tLm2;
        let tt = ClearModp::from(3) * tLm2;

        let mut temp: Array<ClearModp, 5> = Array::uninitialized();
        temp.set(0, &ClearModp::from(tt));
        temp.set(1, &ClearModp::from(-(V as i64) + 1));
        temp.set(2, &zero);
        temp.set(3, &zero);
        temp.set(4, &zero);
        let tt: ClearFloat<V, P, DETECT_OVERFLOW> = ClearFloat::set(temp);

        let mut v2 = &vv * &alpha;
        let v0 = &v2 + &beta;
        let mut vg = &vv * &v0;

        let mut temp: Array<SecretModp, 5> = Array::uninitialized();
        temp.set(0, &v0.v());
        temp.set(1, &(v0.p() - one));
        temp.set(2, &v0.z());
        temp.set(3, &v0.s());
        temp.set(4, &v0.err());
        let mut vh: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW> = SecretFloat::set(temp);

        for _i in 0..SecretFloat::<V, P, KAPPA, DETECT_OVERFLOW>::THETA {
            v2 = &vg * &vh;
            v2 = &tt - &v2;
            vg = &vg * &v2;
            vh = &vh * &v2;
        }

        let vh2 = &vh * &vh;
        v2 = &vv * &vh2;

        let mut temp: Array<SecretModp, 5> = Array::uninitialized();
        temp.set(0, &v2.v());
        temp.set(1, &(v2.p() + one));
        temp.set(2, &v2.z());
        temp.set(3, &v2.s());
        temp.set(4, &v2.err());
        let gg: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW> = SecretFloat::set(temp);

        v2 = &tt - &gg;
        vh = &vh * &v2;

        let mut temp: Array<SecretModp, 5> = Array::uninitialized();
        temp.set(0, &vh.v());
        temp.set(1, &(vh.p() + one));
        temp.set(2, &vh.z());
        temp.set(3, &vh.s());
        temp.set(4, &vh.err());
        let gg: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW> = SecretFloat::set(temp);

        v2 = &vv * &gg;

        let sqrt2: ClearFloat<V, P, DETECT_OVERFLOW> =
            ClearFloat::from(1.4142135623730950488016887242096980786);
        let vv = tLm1 * (one - c) + sqrt2.v() * c;
        let pp = -(one - c) * (ClearModp::from(V as i64 - 1)) + sqrt2.p() * c;

        let mut temp: Array<SecretModp, 5> = Array::uninitialized();
        temp.set(0, &vv);
        temp.set(1, &pp);
        temp.set(2, &szero);
        temp.set(3, &szero);
        temp.set(4, &szero);
        let gg: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW> = SecretFloat::set(temp);

        v2 = &v2 * &gg;

        let p = (v2.p() + p) * (one - self.z());
        let v = v2.v() * (one - self.z());
        let err = self.err() + self.s();

        let mut temp: Array<SecretModp, 5> = Array::uninitialized();
        temp.set(0, &v);
        temp.set(1, &p);
        temp.set(2, &self.z());
        temp.set(3, &self.s());
        temp.set(4, &err);
        let ans: SecretFloat<V, P, KAPPA, DETECT_OVERFLOW> = SecretFloat::set(temp);

        ans
    }
}
