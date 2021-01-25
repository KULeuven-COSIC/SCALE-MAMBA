#![no_std]
#![no_main]

scale::main! {
    I64_MEMORY = 0;
    SECRET_I64_MEMORY = 0;
    SECRET_MODP_MEMORY = 0;
    CLEAR_MODP_MEMORY = 0;
}

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
    return a + b - 2 * a * b;
}

#[inline(always)]
pub fn carry(b: [SecretModp; 2], a: [SecretModp; 2]) -> [SecretModp; 2] {
    let mut ans: [SecretModp; 2] = [SecretModp::from(0), SecretModp::from(0)];
    ans[0] = a[0] * b[0];
    ans[1] = a[1] + a[0] * b[1];
    ans
}

#[inline(always)]
pub fn reg_carry(
    b: Option<[SecretModp; 2]>,
    a: Option<[SecretModp; 2]>,
) -> Option<[SecretModp; 2]> {
    match a {
        None => return None,
        Some(aa) => match b {
            None => return None,
            Some(bb) => {
                let mut ans: [SecretModp; 2] = [SecretModp::from(0), SecretModp::from(0)];
                ans[0] = aa[0] * bb[0];
                ans[1] = aa[1] + aa[0] * bb[1];
                return Some(ans);
            }
        },
    }
}

// XXXX Not sure if this works, got it from internet forum
// You need to use `core` instead.
const fn num_bits<T>() -> usize {
    core::mem::size_of::<T>() * 8
}

#[inline(always)]
pub fn ceil_log_2(a: u32) {
    num_bits::<i32>() as u32 - a.leading_zeros();
}
#[inline(always)]
pub fn two_power(n: u32) -> ClearModp {
    let two: i32 = 2;
    if n < 30 {
        // If n known at run time this next bit can be precomputed
        let a = two.pow(n);
        return ClearModp::from(a);
    }
    let b:i32 = two.pow(30);
    let max = ClearModp::from(b);
    let a = two.pow(n%30);
    // XXXX How get this loops to unroll if n known at compile time?
    let mut res = ClearModp::from(a);
    for i in 0..n/30 {
        res *= max;
    }
    return res;
}

/* ######################################
 * ##### SECTION 14.1 OF THE MANUAL #####   
   ###################################### */
#[inline(always)]
pub fn Inv(a: SecretModp) -> SecretModp {
    let (t0, t1) = __square();  // What ever function needed for a pre-processed square
    let s = t0*a;
    let c = s.reveal();
    let c = 1/c;
    return c*t0;
}


/* ######################################
 * ##### SECTION 14.2 OF THE MANUAL #####   
   ###################################### */


/*#[inline(always)]
pub fn KOpL(op: fn(SecretModp, SecretModp) -> SecretModp, a: Vec<SecretModp>) {
    let k = a.len();
    if k==1 {
        return a[0];
    }
    let t1 = KOpL(op, a[0..k/2]);
    let t2 = KOpL(op, a[k/2..k]);
    return op(t1,t2);
}

/* Takes a vector of SecretModpModp things, which are
 * known to be bits and forms the sum
 */
#[inline(always)]
pub fn SumBits( xB : Vec<SecretModp> ) {
    let k = xB.len();
    let mut v = Vec::with_capacity(k);
    let mut twop=1;
    // Again next should be unrolled as k is known
    for i in 0..k {
        v[i]=xB[i]*twop;
        twop=twop*2;
    }
    let sum = KOpL(addition_op, v);
    return sum;
}*/


#[inline(always)]
fn main() {
    let a = 0;
    let b = 1;

    let ca = ClearModp::from(a);
    let cb = ClearModp::from(b);
    let sa = SecretModp::from(a);
    let sb = SecretModp::from(b);
    let scc = [SecretModp::from(a), SecretModp::from(a)];
    let scd = [SecretModp::from(b), SecretModp::from(a)];
    let sce = [SecretModp::from(a), SecretModp::from(b)];
    let scf = [SecretModp::from(b), SecretModp::from(b)];

    or_op(sa, sa).test();
    or_op(sa, sb).test();
    or_op(sb, sb).test();
    /*
        or_op(ca,sb).test();
        or_op(sa,cb).test();
        or_op(ca,cb).test();
        ... and the same for all other operations.
    */
    mul_op(sa, sa).test();
    mul_op(sa, sb).test();
    mul_op(sb, sb).test();

    addition_op(sa, sa).test();
    addition_op(sa, sb).test();
    addition_op(sb, sb).test();

    xor_op(sa, sa).test();
    xor_op(sa, sb).test();
    xor_op(sb, sb).test();

    carry(scc, scd)[0].test();
    carry(scc, scd)[1].test();
    carry(scd, sce)[0].test();
    carry(scd, sce)[1].test();
    carry(sce, scf)[0].test();
    carry(sce, scf)[1].test();

    two_power(29).test();
    two_power(37).test();

    Inv(SecretModp::from(5)).test();
//    let V = vec![SecretModp::from(4),SecretModp::from(29),SecretModp::from(14),SecretModp::from(7)];
//    SumBits(V).test(); 
}
