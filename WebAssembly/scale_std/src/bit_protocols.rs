use crate::array::*;
use crate::iter::CompileTimeLengthIterator;
use crate::slice::*;
use scale::*;

/*
####################################
######### BASIC OPERATORS  #########
####################################
*/
#[inline(always)]
pub fn or_op(a: SecretModp, b: SecretModp) -> SecretModp {
    return a + b - a * b;
}
#[inline(always)]
pub fn mul_op(a: SecretModp, b: SecretModp) -> SecretModp {
    return a * b;
}
#[inline(always)]
pub fn addition_op(a: SecretModp, b: SecretModp) -> SecretModp {
    return a + b;
}
#[inline(always)]
pub fn xor_op(a: SecretModp, b: SecretModp) -> SecretModp {
    return a + b - ConstI32::<2> * a * b;
}

#[inline(always)]
pub fn reg_carry(
    b: Array<SecretModp, 2>,
    a: Array<SecretModp, 2>,
    compute_p: bool,
) -> Array<SecretModp, 2> {
    let mut ans: Array<SecretModp, 2> = Array::uninitialized();
    if compute_p {
        ans.set(0, &(a.get(0) * b.get(0)));
    }
    ans.set(1, &(a.get(1) + a.get(0) * b.get(1)));
    ans
}

#[inline(always)]
pub fn carry(b: Array<SecretModp, 2>, a: Array<SecretModp, 2>) -> Array<SecretModp, 2> {
    let mut ans: Array<SecretModp, 2> = Array::uninitialized();
    ans.set(0, &(a.get(0) * b.get(0)));
    ans.set(1, &(a.get(1) + a.get(0) * b.get(1)));
    ans
}

/* ######################################
 * #####      Helper functions      #####
 * ###################################### */
const fn num_bits<T>() -> usize {
    core::mem::size_of::<T>() * 8
}

#[inline(always)]
pub const fn ceil_log_2(a: u32) -> u32 {
    let mut check = 0;
    if a.count_ones() == 1 {
        check = 1;
    }
    num_bits::<i32>() as u32 - a.leading_zeros() - check
}

pub struct CeilLog2<const N: u64>;

impl<const N: u64> CeilLog2<N> {
    /// The same thing as calling `ceil_log_2`, but the computation
    /// is guaranteed to happen at compile-time, and does not rely on
    /// optimizations.
    pub const RESULT: u64 = ceil_log_2(N as u32) as u64;
}

#[inline(always)]
pub fn two_power(n: u64) -> u64 {
    const TWO: u64 = 2;
    if n < 30 {
        // If n known at run time this next bit can be precomputed
        let a = TWO.pow(n as u32);
        return a;
    }
    let b: u64 = TWO.pow(30);
    let a = TWO.pow((n % 30) as u32);
    let mut res: u64 = a;
    for _i in 0..n / 30 {
        res *= b;
    }
    return res;
}

pub struct TwoPower<const N: u64>;

impl<const N: u64> TwoPower<N> {
    /// The same thing as calling `i64_two_power`, but the computation
    /// is guaranteed to happen at compile-time, and does not rely on
    /// optimizations.
    pub const RESULT: i64 = i64_two_power(N);
    pub const RESULT_N_NEG_ONE: i64 = i64_two_power(N - 1);
}

// Now version producing an i64 result
#[inline(always)]
pub const fn i64_two_power(n: u64) -> i64 {
    const TWO: i64 = 2;
    if n < 30 {
        // If n known at run time this next bit can be precomputed
        let a = TWO.pow(n as u32);
        return a;
    }
    let b: i64 = TWO.pow(30);
    let a = TWO.pow((n % 30) as u32);
    let mut res: i64 = a;
    let mut i = 0;
    while i < n / 30 {
        res *= b;
        i += 1;
    }
    return res;
}

// Now version producing a ClearModp result and create
// a value bigger than 2^64
#[inline(always)]
pub fn modp_two_power(n: u64) -> ClearModp {
    const TWO: i64 = 2;
    if n < 30 {
        // If n known at run time this next bit can be precomputed
        let a = TWO.pow(n as u32);
        let b = ClearModp::from(a);
        return b;
    }
    let b: i64 = TWO.pow(30);
    let max = ClearModp::from(b);
    let a = TWO.pow((n % 30) as u32);
    let mut res = ClearModp::from(a);
    for _i in 0..n / 30 {
        res *= max;
    }
    return res;
}

#[inline(always)]
pub fn bits(a: ClearModp, m: u64) -> Slice<ClearModp> {
    let mut ab: Slice<ClearModp> = Slice::uninitialized(m);
    let mut a_helper: Slice<ClearModp> = Slice::uninitialized(m);
    ab.set(0, &(a % ClearModp::from(2)));
    a_helper.set(0, &a);
    for i in 1..m {
        a_helper.set(
            i,
            &((a_helper.get(i - 1) - ab.get(i - 1)) / ClearModp::from(2)),
        );
        ab.set(i, &(a_helper.get(i) % ClearModp::from(2)));
    }
    return ab;
}

#[inline(always)]
pub fn get_primecompl(bitlen: u64) -> Slice<ClearModp> {
    /* Uses P (in big endian) to output the two's complement (in little endian) */
    let mut pb: Slice<i64> = Slice::uninitialized(bitlen + 1);
    for i in 1..bitlen + 1 {
        pb.set(i - 1, &(!P[(bitlen - i) as usize] as i64));
    }
    let mut result: Slice<ClearModp> = Slice::uninitialized(bitlen);
    let mut carry: i64 = 1;
    for index in 0..bitlen {
        if carry != 1 {
            result.set(index, &ClearModp::from(pb.get(index)));
        } else {
            let bit = pb.get(index);
            if bit == 1 {
                result.set(index, &ClearModp::from(0));
            } else {
                result.set(index, &ClearModp::from(1));
                carry = 0;
            }
        }
    }
    return result;
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn Inv(a: SecretModp) -> SecretModp {
    let (t0, _t1) = __square(); // What ever function needed for a pre-processed square
    let s = t0 * a;
    let c = s.reveal();
    let c = ConstI32::<1> / c;
    return c * t0;
}

/*
####################################
#### SECTION 14.2 OF THE MANUAL ####
####################################
*/
#[inline(always)]
#[allow(non_snake_case)]
pub fn KOpL(
    op: impl Fn(SecretModp, SecretModp) -> SecretModp + Copy,
    s: &Slice<SecretModp>,
) -> SecretModp {
    let l: u64 = s.len();
    if l == 1 {
        return s.get(0);
    }
    let t1: SecretModp = KOpL(op, &s.slice(..l / 2));
    let t2: SecretModp = KOpL(op, &s.slice(l / 2..));
    return op(t1, t2);
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn KOr(a: &Slice<SecretModp>) -> SecretModp {
    return KOpL(or_op, a);
}

/* Uses algorithm from SecureSCM WP9 deliverable.
 * op must be a binary function that outputs a new register
*/
#[inline(always)]
#[allow(non_snake_case)]
pub fn PreOpL(
    op: impl Fn(SecretModp, SecretModp) -> SecretModp + Copy,
    items: &Slice<SecretModp>,
) -> Slice<SecretModp> {
    let k: u64 = items.len();
    let logk: u64 = ceil_log_2(k as u32).into();
    let kmax: u64 = two_power(logk);
    let mut output: Slice<SecretModp> = Slice::uninitialized(k);
    for i in 0..k {
        output.set(i, &items.get(i));
    }
    for i in 0u64..logk {
        for j in 0u64..(kmax / two_power(i + 1)) {
            let y: u64 = two_power(i) + j * two_power(i + 1) - 1;
            let zmax: u64 = two_power(i) + 1;
            for z in 1u64..zmax {
                if y + z < k {
                    output.set(y + z, &op(output.get(y), output.get(y + z)));
                }
            }
        }
    }
    output
}

/*
 * Uses algorithm from SecureSCM WP9 deliverable.
 * op must be a binary function that outputs a new register
 */
#[inline(always)]
#[allow(non_snake_case)]
pub fn PreOpL2(
    op: impl Fn(Array<SecretModp, 2>, Array<SecretModp, 2>, bool) -> Array<SecretModp, 2> + Copy,
    items: &Slice<Array<SecretModp, 2>>,
) -> Slice<Array<SecretModp, 2>> {
    let k: u64 = items.len();
    let logk: u64 = ceil_log_2(k as u32).into();
    let kmax: u64 = two_power(logk);
    let output: Slice<Array<SecretModp, 2>> = Slice::uninitialized(k);
    for i in 0..k {
        output.get(i).set(0, &items.get(i).get(0));
        output.get(i).set(1, &items.get(i).get(1));
    }
    for i in 0u64..logk {
        for j in 0u64..(kmax / two_power(i + 1)) {
            let y: u64 = two_power(i) + j * two_power(i + 1) - 1;
            let zmax: u64 = two_power(i) + 1;
            for z in 1u64..zmax {
                if y + z < k {
                    let res_op: Array<SecretModp, 2> = op(output.get(y), output.get(y + z), j != 0);
                    output.get(y + z).set(0, &res_op.get(0));
                    output.get(y + z).set(1, &res_op.get(1));
                }
            }
        }
    }
    output
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn PreOr(a: &Slice<SecretModp>) -> Slice<SecretModp> {
    return PreOpL(or_op, a);
}

/* Takes a vector of SecretModpModp things, which are
 * known to be bits and forms the sum
 */
#[inline(always)]
#[allow(non_snake_case)]
pub fn SumBits(xb: &Slice<SecretModp>) -> SecretModp {
    let k: u64 = xb.len();
    let mut v: Slice<SecretModp> = Slice::uninitialized(k);
    let mut twop = ClearModp::from(1);
    for i in 0..k {
        v.set(i, &(xb.get(i) * twop));
        twop = twop + twop;
    }
    return KOpL(addition_op, &v);
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn PRandInt(k: u64) -> SecretModp {
    if k == 0 {
        SecretModp::from(ConstI32::<0>)
    } else {
        let mut result = SecretModp::get_random_bit();
        for _ in 1..k {
            result = result + result + SecretModp::get_random_bit();
        }
        result
    }
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn PRandM<const K: u64, const M: u64, const KAPPA: u64>(
) -> (SecretModp, SecretModp, Array<SecretModp, M>) {
    // We are not using the `require_bit_length` wrapper here, as we can't do math in const generics yet.
    unsafe { __reqbl((K + KAPPA) as u32) };
    let mut rb: Array<SecretModp, M> = Array::uninitialized();
    for i in 0u64..M {
        rb.set(i, &SecretModp::get_random_bit());
    }
    let r: SecretModp = SumBits(&rb.slice(..));
    let r2: SecretModp = PRandInt(K + KAPPA - M);
    return (r2, r, rb);
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn CarryOutAux(a: &Slice<Array<SecretModp, 2>>) -> SecretModp {
    let mut k: u64 = a.len();
    let mut offset: u64 = 0;
    if k == 1 {
        return a.get(0).get(1);
    }
    if k % 2 == 1 {
        offset = 1;
        k += 1;
    }
    let u: Slice<Array<SecretModp, 2>> = Slice::uninitialized(k / 2);
    for i in offset..k / 2 {
        let arr: Array<SecretModp, 2> = reg_carry(
            a.get(2 * i + 1 - offset),
            a.get(2 * i - offset),
            i != k / 2 - 1,
        );
        u.get(i).set(0, &arr.get(0));
        u.get(i).set(1, &arr.get(1));
    }
    if offset == 1 {
        u.get(0).set(0, &a.get(0).get(0));
        u.get(0).set(1, &a.get(0).get(1));
    }
    return CarryOutAux(&u);
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn CarryOut(a: &Slice<ClearModp>, b: &Slice<SecretModp>, c: ClearModp) -> SecretModp {
    let k: u64 = a.len();
    let d: Slice<Array<SecretModp, 2>> = Slice::uninitialized(k);
    for i in 0u64..k {
        d.get(i).set(1, &(a.get(i) * b.get(i)));
        d.get(i)
            .set(0, &(a.get(i) + b.get(i) - ConstI32::<2> * d.get(i).get(1)));
    }
    d.get(k - 1)
        .set(1, &(d.get(k - 1).get(1) + c * d.get(k - 1).get(0)));
    return CarryOutAux(&d);
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn BitAdd<T: Modp<U>, U: Modp<T>>(ab: &Slice<T>, bb: &Slice<U>) -> Slice<SecretModp> {
    let k: u64 = ab.len();
    let d: Slice<Array<SecretModp, 2>> = Slice::uninitialized(k);
    for i in 0..k {
        d.get(i).set(1, &(ab.get(i) * bb.get(i)).into());
        d.get(i).set(
            0,
            &(ab.get(i) + bb.get(i) - d.get(i).get(1) - d.get(i).get(1)).into(),
        );
    }

    let c: Slice<Array<SecretModp, 2>> = PreOpL2(reg_carry, &d);
    let mut s: Slice<SecretModp> = Slice::uninitialized(k + 1);
    s.set(
        0,
        &(ab.get(0) + bb.get(0) - c.get(0).get(1) - c.get(0).get(1)),
    );
    for i in 1..k {
        s.set(
            i,
            &(SecretModp::from(
                ab.get(i) + bb.get(i) + c.get(i - 1).get(1) - c.get(i).get(1) - c.get(i).get(1),
            )),
        );
    }
    s.set(k, &c.get(k - 1).get(1));
    return s;
}

#[inline(always)]
pub fn bitincrement<const K: u64>(ab: &Array<SecretModp, K>) -> Slice<SecretModp> {
    let d: Slice<Array<SecretModp, 2>> = Slice::uninitialized(K);
    d.get(0).set(1, &(ab.get(0)));
    d.get(0).set(0, &(SecretModp::from(1) - ab.get(0)));
    for i in 1..K {
        d.get(i).set(1, &(SecretModp::from(0)));
        d.get(i).set(0, &(ab.get(i)));
    }
    let c: Slice<Array<SecretModp, 2>> = PreOpL2(reg_carry, &d);

    let mut s: Slice<SecretModp> = Slice::uninitialized(K + 1);
    s.set(
        0,
        &(ab.get(0) + SecretModp::from(1) - ConstI32::<2> * c.get(0).get(1)),
    );
    for i in 1..K {
        s.set(
            i,
            &(ab.get(i) + c.get(i - 1).get(1) - ConstI32::<2> * c.get(i).get(1)),
        );
    }
    s.set(K, &c.get(K - 1).get(1));
    return s;
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn BitLT<const K: u64>(
    a: ClearModp,
    bb: impl IntoIterator<Item = SecretModp> + CompileTimeLengthIterator<K>,
) -> SecretModp {
    let mut ab: Array<ClearModp, K> = Array::uninitialized();
    let mut a_helper: Array<ClearModp, K> = Array::uninitialized();
    let mut sb: Array<SecretModp, K> = Array::uninitialized();
    ab.set(K - 1, &(a % ClearModp::from(ConstI32::<2>)));
    a_helper.set(0, &a);
    for i in 1..K {
        a_helper.set(
            i,
            &((a_helper.get(i - 1) - ab.get(K - i)) / ClearModp::from(ConstI32::<2>)),
        );
        ab.set(
            K - i - 1,
            &(a_helper.get(i) % ClearModp::from(ConstI32::<2>)),
        );
    }
    for (i, v) in bb.into_iter().enumerate() {
        sb.set(K - 1 - i as u64, &(ConstI32::<1> - v));
    }
    let c: SecretModp = CarryOut(&ab.slice(..), &sb.slice(..), ClearModp::from(ConstI32::<1>));
    return ConstI32::<1> - c;
}

#[inline(always)]
pub fn bitltfull<T: Modp<U>, U: Modp<T>>(ab: &Slice<T>, bb: &Slice<U>) -> SecretModp {
    let k: u64 = ab.len();
    let mut e: Slice<SecretModp> = Slice::uninitialized(k);
    let mut g: Slice<SecretModp> = Slice::uninitialized(k);
    let mut h: Slice<SecretModp> = Slice::uninitialized(k);
    for i in 0..k {
        e.set(
            k - 1 - i,
            &((ab.get(i) + bb.get(i) - ab.get(i) * bb.get(i) * ConstI32::<2>).into()),
        );
    }
    let f: Slice<SecretModp> = PreOr(&e);
    g.set(k - 1, &f.get(0));
    for i in 0..k - 1 {
        g.set(i, &(f.get(k - 1 - i) - f.get(k - 2 - i)));
    }
    let mut ans = SecretModp::from(0);
    for i in 0..k {
        h.set(i, &((bb.get(i) * g.get(i)).into()));
        ans = ans + h.get(i);
    }
    return ans;
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn BitDec<const K: u64, const M: u64, const KAPPA: u64>(a: SecretModp) -> Slice<SecretModp> {
    let random = PRandM::<K, M, KAPPA>();
    let r_prime: SecretModp = random.0;
    let r: SecretModp = random.1;
    let rb: Array<SecretModp, M> = random.2;
    let cons: ClearModp = modp_two_power(K) + modp_two_power(K + KAPPA);
    let sc: SecretModp = a + cons - modp_two_power(M) * r_prime - r;
    let c = sc.reveal();
    let cb: Slice<ClearModp> = bits(c, M);
    return BitAdd(&cb, &rb.slice(..)).slice(..M);
}

#[inline(always)]
pub fn bitdecfullbig(a: SecretModp) -> Slice<SecretModp> {
    // Returns secret shared bit decomposition of
    let bitlen: u64 = P.len() as u64;
    let mut abits: Slice<SecretModp> = Slice::uninitialized(bitlen);
    let mut bbits: Slice<SecretModp> = Slice::uninitialized(bitlen);
    let mut pbits: Slice<ClearModp> = Slice::uninitialized(bitlen + 1);
    for i in 0u64..bitlen {
        pbits.set(i + 1, &ClearModp::from(P[(bitlen - 1 - i) as usize] as i64));
    }
    // Loop until we get some random integers less than p
    let mut cond: i64 = 0;
    while cond == 0 {
        for i in 0u64..bitlen {
            bbits.set(i, &SecretModp::get_random_bit());
        }
        cond = i64::from(bitltfull(&bbits, &pbits).reveal());
    }
    let b: SecretModp = SumBits(&bbits);
    let c: ClearModp = (a - b).reveal();
    let czero = SecretModp::from((i64::from(c) == 0) as i64);
    let d: Slice<SecretModp> = BitAdd(&bits(c, bitlen), &bbits);
    let q: SecretModp = bitltfull(&pbits, &d);
    let f: Slice<ClearModp> = get_primecompl(bitlen);
    let mut g: Slice<SecretModp> = Slice::uninitialized(bitlen + 1);
    for i in 0..bitlen {
        g.set(i, &(f.get(i) * q));
    }
    let h: Slice<SecretModp> = BitAdd(&d, &g);
    for i in 0..bitlen {
        abits.set(
            i,
            &(h.get(i) * (ConstI32::<1> - czero) + bbits.get(i) * czero),
        );
    }
    return abits;
}

#[inline(always)]
pub fn bitdecfull(a: SecretModp) -> Slice<SecretModp> {
    let bitlen: u64 = P.len() as u64;
    if bitlen > 63 {
        return bitdecfullbig(a);
    }
    let mut abits: Slice<SecretModp> = Slice::uninitialized(bitlen);
    let mut bbits: Slice<SecretModp> = Slice::uninitialized(bitlen);
    let mut pbits: Slice<SecretModp> = Slice::uninitialized(bitlen + 1);
    let mut p: i64 = 0;
    for i in 0u64..bitlen {
        pbits.set(
            i + 1,
            &SecretModp::from(P[(bitlen - 1 - i) as usize] as i64),
        );
        p += (two_power(i) * (P[(bitlen - 1 - i) as usize] as u64)) as i64;
    }
    // Loop until we get some random integers less than p
    let mut cond: i64 = 0;
    while cond == 0 {
        for i in 0u64..bitlen {
            bbits.set(i, &SecretModp::get_random_bit());
        }
        cond = i64::from(bitltfull(&bbits, &pbits).reveal());
    }
    let b: SecretModp = SumBits(&bbits);
    let mut c: i64 = i64::from((a - b).reveal());
    let bit: i64 = (c < 0) as i64;
    c = c + (p * bit) as i64;
    let czero = SecretModp::from((c == 0) as i64);
    let t: Slice<ClearModp> = bits(ClearModp::from(p - c), bitlen);
    //let mut ts: Slice<SecretModp> = Slice::uninitialized(bitlen);
    //for i in 0..bitlen {ts.set(i,&SecretModp::from(t.get(i)));}
    let q: SecretModp = ConstI32::<1> - bitltfull(&bbits, &t);
    let fbar: Slice<ClearModp> = bits(ClearModp::from((1 << bitlen) + c - p), bitlen);
    let fbard: Slice<ClearModp> = bits(ClearModp::from(c), bitlen);
    let mut g: Slice<SecretModp> = Slice::uninitialized(bitlen);
    for i in 0..bitlen {
        g.set(i, &((fbar.get(i) - fbard.get(i)) * q + fbard.get(i)));
    }
    let h: Slice<SecretModp> = BitAdd(&bbits, &g);
    for i in 0..bitlen {
        abits.set(
            i,
            &((ConstI32::<1> - czero) * h.get(i) + czero * bbits.get(i)),
        );
    }
    return abits;
}
