// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use crate::array::*;
use crate::slice::*;
use scale::*;

/*
####################################
######### BASIC OPERATORS  #########
####################################
*/
#[inline(always)]
pub fn secret_or_op(a: SecretModp, b: SecretModp) -> SecretModp {
    a + b - a * b
}

pub fn clear_or_op(a: ClearModp, b: ClearModp) -> ClearModp {
    a + b - a * b
}

#[inline(always)]
pub fn mul_op(a: SecretModp, b: SecretModp) -> SecretModp {
    a * b
}
#[inline(always)]
pub fn addition_op(a: SecretModp, b: SecretModp) -> SecretModp {
    a + b
}
#[inline(always)]
pub fn xor_op(a: SecretModp, b: SecretModp) -> SecretModp {
    a + b - ConstI32::<2> * a * b
}

#[inline(always)]
pub fn reg_carry(
    b: Array<SecretModp, 2>,
    a: Array<SecretModp, 2>,
    compute_p: bool,
) -> Array<SecretModp, 2> {
    let mut ans: Array<SecretModp, 2> = Array::uninitialized();
    if compute_p {
        ans.set(0, &(*a.get_unchecked(0) * *b.get_unchecked(0)));
    }
    ans.set(
        1,
        &(*a.get_unchecked(1) + *a.get_unchecked(0) * *b.get_unchecked(1)),
    );

    ans
}

#[inline(always)]
pub fn carry(b: Array<SecretModp, 2>, a: Array<SecretModp, 2>) -> Array<SecretModp, 2> {
    let mut ans: Array<SecretModp, 2> = Array::uninitialized();
    ans.set(0, &(*a.get_unchecked(0) * *b.get_unchecked(0)));
    ans.set(
        1,
        &(*a.get_unchecked(1) + *a.get_unchecked(0) * *b.get_unchecked(1)),
    );

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

    res
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

    res
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

    res
}

/* Produces an array expressing 2<<bitlen-p */
#[inline(always)]
pub fn get_primecompl(bitlen: u64) -> Slice<ClearModp> {
    /* Uses P (in big endian) to output the two's complement (in little endian) */
    let mut pb: Array<i64, BITLENP> = Array::uninitialized();
    for i in 0u64..BITLENP {
        pb.set(i, &i64::from(1 - P[(BITLEN - 1 - i) as usize] as i64));
    }
    let mut result: Slice<ClearModp> = Slice::uninitialized(bitlen);
    let mut carry: i64 = 1;
    for index in 0..bitlen {
        if carry != 1 {
            result.set(index, &ClearModp::from(*pb.get_unchecked(index)));
        } else {
            let bit = pb.get_unchecked(index);
            if *bit == 1 {
                result.set(index, &ClearModp::from(0));
            } else {
                result.set(index, &ClearModp::from(1));
                carry = 0;
            }
        }
    }

    result
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn Inv(a: SecretModp) -> SecretModp {
    let (t0, _t1) = SecretModp::get_random_square();
    let s = t0 * a;
    let c = s.reveal();
    let c = ConstI32::<1> / c;

    c * t0
}

/*
####################################
#### SECTION 14.2 OF THE MANUAL ####
####################################
*/

/* We execute this via pointers, and not slices directly
 * to avoid a lot of memory thrashing
 */
#[inline(always)]
#[allow(non_snake_case)]
pub fn KOpL(
    op: impl Fn(SecretModp, SecretModp) -> SecretModp + Copy,
    start_addr: i64,
    len: i64,
) -> SecretModp {
    if len <= 1 {
        let s = SecretModp::load_from_mem(start_addr);
        return s;
    }
    let t1: SecretModp = KOpL(op, start_addr, len / 2);
    let t2: SecretModp = KOpL(op, start_addr + len / 2, len - len / 2);

    op(t1, t2)
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn KOr(a: &Slice<SecretModp>) -> SecretModp {
    KOpL(secret_or_op, a.addr(0), a.len() as i64)
}

/* Uses algorithm from SecureSCM WP9 deliverable.
 * op must be a binary function that outputs a new register
*/
#[inline(always)]
#[allow(non_snake_case)]
pub fn PreOpL<T>(op: impl Fn(T, T) -> T + Copy, items: &Slice<T>) -> Slice<T>
where
    T: Modp<SecretModp> + Copy,
{
    let k: u64 = items.len();
    let logk: u64 = ceil_log_2(k as u32).into();
    let kmax: u64 = two_power(logk);
    let mut output: Slice<T> = Slice::uninitialized(k);
    for i in 0..k {
        output.set(i, &*items.get_unchecked(i));
    }
    for i in 0u64..logk {
        for j in 0u64..(kmax / two_power(i + 1)) {
            let y: u64 = two_power(i) + j * two_power(i + 1) - 1;
            let zmax: u64 = two_power(i) + 1;
            for z in 1u64..zmax {
                if y + z < k {
                    output.set(
                        y + z,
                        &op(*output.get_unchecked(y), *output.get_unchecked(y + z)),
                    );
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
    let mut output: Slice<Array<SecretModp, 2>> = Slice::uninitialized(k);
    for i in 0..k {
        output
            .get_mut_unchecked(i)
            .set(0, &*items.get_unchecked(i).get_unchecked(0));
        output
            .get_mut_unchecked(i)
            .set(1, &*items.get_unchecked(i).get_unchecked(1));
    }
    for i in 0u64..logk {
        for j in 0u64..(kmax / two_power(i + 1)) {
            let y: u64 = two_power(i) + j * two_power(i + 1) - 1;
            let zmax: u64 = two_power(i) + 1;
            for z in 1u64..zmax {
                if y + z < k {
                    let res_op: Array<SecretModp, 2> = op(
                        output.get_unchecked(y).clone(),
                        output.get_unchecked(y + z).clone(),
                        j != 0,
                    );
                    output
                        .get_mut_unchecked(y + z)
                        .set(0, &*res_op.get_unchecked(0));
                    output
                        .get_mut_unchecked(y + z)
                        .set(1, &*res_op.get_unchecked(1));
                }
            }
        }
    }

    output
}

pub trait PreOr {
    #[allow(non_snake_case)]
    fn PreOr(self) -> Self;
}

impl PreOr for Slice<SecretModp> {
    #[inline(always)]
    #[allow(non_snake_case)]
    fn PreOr(self) -> Slice<SecretModp> {
        PreOpL(secret_or_op, &self)
    }
}

impl PreOr for Slice<ClearModp> {
    #[inline(always)]
    #[allow(non_snake_case)]
    fn PreOr(self) -> Slice<ClearModp> {
        PreOpL(clear_or_op, &self)
    }
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

/* As the next function should always be inlined the multiple
 * return values should work
 */
#[inline(always)]
#[allow(non_snake_case)]
pub fn PRandM_Array<const K: u64, const M: u64, const KAPPA: u64>(
) -> (SecretModp, SecretModp, Array<SecretModp, M>) {
    unsafe { __reqbl((K + KAPPA) as u32) };
    let mut rb: Array<SecretModp, M> = Array::uninitialized();
    for i in 0u64..M {
        rb.set(i, &SecretModp::get_random_bit());
    }
    let r = rb.evaluate(ClearModp::from(2));
    let r2: SecretModp = PRandInt(K + KAPPA - M);

    (r2, r, rb)
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn PRandM_Slice(K: u64, M: u64, KAPPA: u64) -> (SecretModp, SecretModp, Slice<SecretModp>) {
    unsafe { __reqbl((K + KAPPA) as u32) };
    let mut rb: Slice<SecretModp> = Slice::uninitialized(M);
    for i in 0u64..M {
        rb.set(i, &SecretModp::get_random_bit());
    }
    let r = rb.evaluate(ClearModp::from(2));
    let r2: SecretModp = PRandInt(K + KAPPA - M);

    (r2, r, rb)
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn CarryOutAux(a: &Slice<Array<SecretModp, 2>>) -> SecretModp {
    let mut k: u64 = a.len();
    let mut offset: u64 = 0;
    if k == 1 {
        return *a.get_unchecked(0).get_unchecked(1);
    }
    if k % 2 == 1 {
        offset = 1;
        k += 1;
    }
    let mut u: Slice<Array<SecretModp, 2>> = Slice::uninitialized(k / 2);
    for i in offset..k / 2 {
        let arr: Array<SecretModp, 2> = reg_carry(
            a.get_unchecked(2 * i + 1 - offset).clone(),
            a.get_unchecked(2 * i - offset).clone(),
            i != k / 2 - 1,
        );
        u.get_mut_unchecked(i).set(0, &*arr.get_unchecked(0));
        u.get_mut_unchecked(i).set(1, &*arr.get_unchecked(1));
    }
    if offset == 1 {
        u.get_mut_unchecked(0)
            .set(0, &*a.get_unchecked(0).get_unchecked(0));
        u.get_mut_unchecked(0)
            .set(1, &*a.get_unchecked(0).get_unchecked(1));
    }

    CarryOutAux(&u)
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn CarryOut(a: &Slice<ClearModp>, b: &Slice<SecretModp>, c: ClearModp) -> SecretModp {
    let k: u64 = a.len();
    let mut d: Slice<Array<SecretModp, 2>> = Slice::uninitialized(k);
    for i in 0u64..k {
        d.get_mut_unchecked(i)
            .set(1, &(*a.get_unchecked(i) * *b.get_unchecked(i)));
        let resp = *a.get_unchecked(i) + *b.get_unchecked(i)
            - ConstI32::<2> * *d.get_unchecked(i).get_unchecked(1);
        d.get_mut_unchecked(i).set(0, &resp);
    }
    let resp =
        *d.get_unchecked(k - 1).get_unchecked(1) + c * *d.get_unchecked(k - 1).get_unchecked(0);
    d.get_mut_unchecked(k - 1).set(1, &resp);
    CarryOutAux(&d)
}

/* Here M is the number of bits we want to return
 *  - We must have 0 < M <=ab.len()+1
 */
#[inline(always)]
#[allow(non_snake_case)]
pub fn BitAdd<T, U>(ab: &Slice<T>, bb: &Slice<U>, M: u64) -> Slice<SecretModp>
where
    T: Modp<U> + Copy,
    U: Modp<T> + Copy,
{
    let k: u64 = ab.len();
    let mut d: Slice<Array<SecretModp, 2>> = Slice::uninitialized(k);
    for i in 0..k {
        d.get_mut_unchecked(i)
            .set(1, &(*ab.get_unchecked(i) * *bb.get_unchecked(i)).into());
        let resp = *ab.get_unchecked(i) + *bb.get_unchecked(i)
            - *d.get_unchecked(i).get_unchecked(1)
            - *d.get_unchecked(i).get_unchecked(1);
        d.get_mut_unchecked(i).set(0, &resp.into());
    }

    let c: Slice<Array<SecretModp, 2>> = PreOpL2(reg_carry, &d);
    let mut s: Slice<SecretModp> = Slice::uninitialized(M);
    s.set(
        0,
        &(*ab.get_unchecked(0) + *bb.get_unchecked(0)
            - *c.get_unchecked(0).get_unchecked(1)
            - *c.get_unchecked(0).get_unchecked(1)),
    );
    for i in 1..M {
        s.set(
            i,
            &(SecretModp::from(
                *ab.get_unchecked(i)
                    + *bb.get_unchecked(i)
                    + *c.get_unchecked(i - 1).get_unchecked(1)
                    - *c.get_unchecked(i).get_unchecked(1)
                    - *c.get_unchecked(i).get_unchecked(1),
            )),
        );
    }
    if M == (k + 1) {
        s.set(k, &*c.get_unchecked(k - 1).get_unchecked(1));
    }

    s
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn BitIncrement<const K: u64>(ab: &Array<SecretModp, K>) -> Slice<SecretModp> {
    let mut d: Slice<Array<SecretModp, 2>> = Slice::uninitialized(K);
    d.get_mut_unchecked(0).set(1, &(*ab.get_unchecked(0)));
    d.get_mut_unchecked(0)
        .set(0, &(SecretModp::from(1) - *ab.get_unchecked(0)));
    for i in 1..K {
        d.get_mut_unchecked(i).set(1, &(SecretModp::from(0)));
        d.get_mut_unchecked(i).set(0, &(*ab.get_unchecked(i)));
    }
    let c: Slice<Array<SecretModp, 2>> = PreOpL2(reg_carry, &d);

    let mut s: Slice<SecretModp> = Slice::uninitialized(K + 1);
    s.set(
        0,
        &(*ab.get_unchecked(0) + SecretModp::from(1)
            - ConstI32::<2> * *c.get_unchecked(0).get_unchecked(1)),
    );
    for i in 1..K {
        s.set(
            i,
            &(*ab.get_unchecked(i) + *c.get_unchecked(i - 1).get_unchecked(1)
                - ConstI32::<2> * *c.get_unchecked(i).get_unchecked(1)),
        );
    }
    s.set(K, &*c.get_unchecked(K - 1).get_unchecked(1));

    s
}

#[inline(always)]
#[allow(non_snake_case)]
// The K here allows for a smaller sub-slice to be used. Usually K=b.len()
pub fn BitLT(a: ClearModp, b: &Slice<SecretModp>, K: u64) -> SecretModp {
    let mut ab: Slice<ClearModp> = Slice::uninitialized(K);
    let mut sb: Slice<SecretModp> = Slice::uninitialized(K);
    ab.set(K - 1, &(a % ClearModp::from(ConstI32::<2>)));
    let mut temp = a;
    for i in 1..K {
        temp = (temp - *ab.get_unchecked(K - i)) / ClearModp::from(ConstI32::<2>);
        ab.set(K - i - 1, &(temp % ClearModp::from(ConstI32::<2>)));
    }
    for i in 0..K {
        sb.set(K - 1 - i as u64, &(ConstI32::<1> - *b.get_unchecked(i)));
    }
    let c: SecretModp = CarryOut(&ab, &sb, ClearModp::from(ConstI32::<1>));

    ConstI32::<1> - c
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn BitLTFull<T, U>(ab: &Slice<T>, bb: &Slice<U>) -> SecretModp
where
    T: Modp<U> + Copy,
    U: Modp<T> + Copy,
{
    let k: u64 = ab.len();
    let mut e: Slice<SecretModp> = Slice::uninitialized(k);
    let mut g: Slice<SecretModp> = Slice::uninitialized(k);
    for i in 0..k {
        e.set(
            k - 1 - i,
            &((*ab.get_unchecked(i) + *bb.get_unchecked(i)
                - *ab.get_unchecked(i) * *bb.get_unchecked(i) * ConstI32::<2>)
                .into()),
        );
    }
    let f: Slice<SecretModp> = e.PreOr();
    g.set(k - 1, &*f.get_unchecked(0));
    for i in 0..k - 1 {
        g.set(
            i,
            &(*f.get_unchecked(k - 1 - i) - *f.get_unchecked(k - 2 - i)),
        );
    }
    let mut ans = SecretModp::from(0);
    for i in 0..k {
        let temp = *bb.get_unchecked(i) * *g.get_unchecked(i);
        ans += temp;
    }

    ans
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn BitDec<const K: u64, const M: u64, const KAPPA: u64>(a: SecretModp) -> Slice<SecretModp> {
    let (r_prime, r, rb) = PRandM_Slice(K, M, KAPPA);
    let cons: ClearModp = modp_two_power(K) + modp_two_power(K + KAPPA);
    let sc: SecretModp = a + cons - modp_two_power(M) * r_prime - r;
    let c = sc.reveal();
    let cb: Slice<ClearModp> = Slice::bit_decomposition_ClearModp_Signed(c, M);
    BitAdd(&cb, &rb, M)
}

/* Now with K=M and runtime values */
#[inline(always)]
#[allow(non_snake_case)]
pub fn bitdec<const KAPPA: u64>(a: SecretModp, k: i64) -> Slice<SecretModp> {
    let (r_prime, r, rb) = PRandM_Slice(k as u64, k as u64, KAPPA);
    let twop = modp_two_power(k as u64);
    let cons: ClearModp = twop + modp_two_power(k as u64 + KAPPA);
    let sc: SecretModp = a + cons - twop * r_prime - r;
    let c = sc.reveal();
    let cb: Slice<ClearModp> = Slice::bit_decomposition_ClearModp_Signed(c, k as u64);
    BitAdd(&cb, &rb, k as u64)
}


const BITLEN: u64 = P.len() as u64;

const BITLENP: u64 = {
    let mut a = BITLEN;
    while !P[(BITLEN - a + 1) as usize - 1] {
        a = a - 1;
    }
    a
};

#[inline(always)]
#[allow(non_snake_case)]
pub fn BitDecFullBig(a: SecretModp) -> Array<SecretModp, BITLENP> {
    // Returns secret shared bit decomposition of
    println!("In BitDecFullBig");
    let mut abits: Array<SecretModp, BITLENP> = Array::uninitialized();
    let mut bbits: Slice<SecretModp> = Slice::uninitialized(BITLENP);
    let mut pbits: Slice<ClearModp> = Slice::uninitialized(BITLENP + 1);
    for i in 0u64..BITLENP {
        pbits.set(i, &ClearModp::from(P[(BITLEN - 1 - i) as usize] as i64));
    }
    // Loop until we get some random integers less than p
    let mut cond: i64 = 0;
    while cond == 0 {
        for i in 0u64..BITLENP {
            bbits.set(i, &SecretModp::get_random_bit());
        }
        let v = BitLTFull(&bbits, &pbits).reveal();
        cond = i64::from(v);
    }
    let b: SecretModp = bbits.evaluate(ClearModp::from(2));
    let c: ClearModp = (a - b).reveal();
    let czero = ClearModp::from((i64::from(c) == 0) as i64);
    let cb: Slice<ClearModp> = Slice::bit_decomposition_ClearModp(c, BITLENP);
    let d: Slice<SecretModp> = BitAdd(&cb, &bbits, cb.len() + 1);
    let q: SecretModp = BitLTFull(&pbits, &d);
    let f: Slice<ClearModp> = get_primecompl(BITLENP);
    let mut g: Slice<SecretModp> = Slice::uninitialized(BITLENP + 1);
    for i in 0..BITLENP {
        g.set(i, &(*f.get_unchecked(i) * q));
    }
    let h: Slice<SecretModp> = BitAdd(&d, &g, d.len() + 1);
    for i in 0..BITLENP {
        abits.set(
            i,
            &(*h.get_unchecked(i) * (ConstI32::<1> - czero) + *bbits.get_unchecked(i) * czero),
        );
    }

    abits
}

#[inline(always)]
#[allow(non_snake_case)]
pub fn BitDecFull(a: SecretModp) -> Array<SecretModp, BITLENP> {
    if BITLEN > 63 {
        return BitDecFullBig(a);
    }
    let mut abits: Array<SecretModp, BITLENP> = Array::uninitialized();
    let mut bbits: Slice<SecretModp> = Slice::uninitialized(BITLENP);
    let mut pbits: Slice<ClearModp> = Slice::uninitialized(BITLENP + 1);
    let mut p: i64 = 0;
    for i in 0u64..BITLENP {
        pbits.set(i, &ClearModp::from(P[(BITLEN - 1 - i) as usize] as i64));
        p += (two_power(i) * (P[(BITLEN - 1 - i) as usize] as u64)) as i64;
    }
    // Loop until we get some random integers less than p
    let mut cond: i64 = 0;
    while cond == 0 {
        for i in 0u64..BITLENP {
            bbits.set(i, &SecretModp::get_random_bit());
        }
        cond = i64::from(BitLTFull(&bbits, &pbits).reveal());
    }
    let b: SecretModp = bbits.evaluate(ClearModp::from(2));
    let mut c: i64 = i64::from((a - b).reveal());
    let bit: i64 = (c < 0) as i64;
    c = c + (p * bit);
    let czero = ClearModp::from((c == 0) as i64);
    let t: Slice<ClearModp> = Slice::bit_decomposition_ClearModp(ClearModp::from(p - c), BITLENP);
    //let mut ts: Slice<SecretModp> = Slice::uninitialized(BITLEN);
    //for i in 0..BITLENP {ts.set(i,&SecretModp::from(t.get(i)));}
    let q: SecretModp = ClearModp::from(1) - BitLTFull(&bbits, &t);
    // BITLENP > 63 is handled above
    #[allow(arithmetic_overflow)]
    let vv = i64_two_power(BITLENP) + c - p;
    let fbar: Slice<i64> = Slice::bit_decomposition_i64(vv, BITLENP);
    let fbard: Slice<i64> = Slice::bit_decomposition_i64(c, BITLENP);
    let mut g: Slice<SecretModp> = Slice::uninitialized(BITLENP);
    for i in 0..BITLENP {
        let temp1 = *fbar.get_unchecked(i) - *fbard.get_unchecked(i);
        let temp2 = ClearModp::from(temp1) * q + ClearModp::from(*fbard.get_unchecked(i));
        g.set(i, &temp2);
    }
    let h: Slice<SecretModp> = BitAdd(&bbits, &g, bbits.len() + 1);
    for i in 0..BITLENP {
        abits.set(
            i,
            &((ClearModp::from(1) - czero) * *h.get_unchecked(i) + czero * *bbits.get_unchecked(i)),
        );
    }

    abits
}
