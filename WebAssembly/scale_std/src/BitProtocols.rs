// This is definitely not real Rust yet. But is
// a place holder for algorithms we will need.
// This file corresponds to the Basic and BitOriented
// operations of the SCALE Manual (Sections 14.1
// and 14.2)


/* Note in this file all array lengths etc are known
 * at compile time, so a lot of stuff can be evaluated
 * at compile time. This is what Mamba does, which means
 * the resulting bytecode is already pretty much optimized
 * before it even goes into scasm.
 *
 * Can we do this same here?
 */


/* At the start of every tape we should output a REQBL 
 * command. We need to compute this as the max which
 * is signalled at any point
 *
 * This is a hack to store this at the moment
 */

let max_reqbl=0;

pub fn update_max_reqbl(a: u32) {
    max_reqbl = max(max_reqbl, a);
}

/*
####################################
######### BASIC OPERATORS  #########
####################################
*/

#[inline(always)]
pub fn or_op(a: SecretModp, b: SecretModp) {
  return a+b-a*b;
}

#[inline(always)]
pub fn mul_op(a: SecretModp, b: SecretModp) {
  return a*b;
}

#[inline(always)]
pub fn addition_op(a: SecretModp, b: SecretModp) {
  return a+b;
}

#[inline(always)]
pub fn xor_op(a: SecretModp, b: SecretModp) {
  return a+b-2*a*b;
}

/* Carry propogation:
        (p,g) = (p_2, g_2)o(p_1, g_1) -> (p_1 & p_2, g_2 | (p_2 & g_1))
*/
#[inline(always)]
pub fn carry(b: [SecretModp; 2], a: [SecretModp; 2]) {
    let ans: [SecretModp, 2];
    ans[0]=a[0]*b[0];
    ans[1]=a[1]+a[0]*b[1];
    return ans;
}

#[inline(always)]
pub fn reg_carry(b: Option<[SecretModp; 2]>, a: Option<[SecretModp; 2]>) {
    match a {
        None => return b;
        Some(aa) => {
            match b {
               None => return a;
               Some(bb) => {
                      let ans: [SecretModp, 2];
                      ans[0]=aa[0]*bb[0];
                      ans[1]=aa[1]+aa[0]*bb[1];
                      return ans;
               }
            }
        }
    }
}


// XXXX Not sure if this works, got it from internet forum
// Need to check ;-)
#[inline(always)]
pub fn ceil_log_2(a: u32) {
  return (num_bits::<i32>() as u32 - x.leading_zeros() - 1);
}


/*
####################################
######### HELPER FUNCTIONS #########
####################################
*/
#[inline(always)]
def two_power(n: u32) -> ClearModp {
    if (n<31) {
        // If n known at run time this next bit can be precomputed
        let a=u32(2).pow(n);
        let ans=ClearModp::from(a);
        return ans;
    }
    let b=u32(2).pow(31);
    let mx=ClearModp::from(m);
    let a=u32(2).pow(n%31);
    let ans=ClearModp::from(a);
    // XXXX How get this loops to unroll if n known at compile time?
    for i in 0..n/31 {
        res *= max;
    }
    return ans;
}





/*
####################################
#### SECTION 14.1 OF THE MANUAL ####
####################################
*/
#[inline(always)]
pub fn Inv(a: SecretModp) {
    t0, t1 = Square();  // What ever function needed for a pre-processed square
    s = t0*a;
    c = s.reveal();
    c = 1/c;
    return c*t;
}


/*
####################################
#### SECTION 14.2 OF THE MANUAL ####
####################################
*/


#[inline(always)]
/* XXXX Is this the correct way to do function passing
 * in that I want this to inline fully?
 */
pub fn KOpL(op: fn(SecretModp, SecretModp) -> SecretModp, a: Vec<SecretModp>) {
    let k = a.len();
    if k==1 {
        return a[0];
    }
    let t1 = KOpL(op, a[0..k/2]);
    let t2 = KOpL(op, a[k/2..k]);
    return op(t1,t2);
}

// KOR is used so much we have a short cut
#[inline(always)]
def KOR(a: Vec<SecretModp>) {
  return KOpL(or_op, a);
}


/* Uses algorithm from SecureSCM WP9 deliverable */
#[inline(always)]
pub fn PreOpL(op: fn(SecretModp, SecretModp) -> SecretModp, a: Vec<SecretModp>) {
    let k = a.len();
    let logk = ceil_log_2(k);
    let kmax = i32(2).pow(logk);
    let output = Vec::with_capacity(logk);
    // XXXX How get these loops to unroll if k known at compile time?
    for i in 0..logk {
        let twoi=i32(2).pow(i);
        let lim = kmax/(2*twoi);
        for j in 0..lim {
            let y = two_power(i) + j*two_power(i+1) - 1;
            for z in 1..(twoi+1) {
                if (y+z)<k {
                    output[y+z] = op(output[y], output[y+z]);
                }
            }
        }
    }
    return output;
}


// PreOR is used so much we have a short cut
#[inline(always)]
def PreOR(a: Vec<SecretModp>) {
  return PreOpL(or_op, a);
}

/* Takes a vector of SecretModpModp things, which are
 * known to be bits and forms the sum
 */
#[inline(always)]
pub fn SumBits( xB : Vec<SecretModp> ) {
    let k = xB.len();
    let v = Vec::with_capacity(k);
    let twop=1;
    // Again next should be unrolled as k is known
    for i in 0..k {
        v[i]=xB[i]*twop;
        twop=twop*2;
    }
    let sum = KOpL(addition_op, v);
    return sum;
}


// Random secret integer in range [0, 2^k - 1]
#[inline(always)]
pub fn PRandInt( k: i64 ) {
    let mut xB = Vec::with_capacity(k);
    // XXXX Again next should be unrolled as k is known
    for i in 0..k {
        xB[i]= Bit(); // What ever function to get a pre-processed bit here
    }
    let x = SumBits(xB);
    return x;
}


#[inline(always)]
pub fn PRandM( k: i64, m: i64, kappa: i64 ) {
    update_max_reqbl(k+ kappa);
    let mut rB = Vec::with_capacity(k);
    // XXXX Again next should be unrolled as k is known
    for i in 0..k {
        rB[i]= Bit(); // What ever function to get a pre-processed bit here
    }
    let r = SumBits(rB);
    let rdash = PRandInt(k+kappa-m);
    return rdash, r, rB;
}


pub fn CarryOutAux(aB: Vec< Option<[SecretModp; 2]> >) {
    let mut k=aB.len();
    // Force a to have even length
    if k>1 && k%2==1 {
      aB.append(None);
      k = k+1;
    }
    let mut u = Vec::with_capacity(k/2);
    aB.reverse();
    if k==1 {
        return a[0][1];
    }
    // XXXX Again next should be unrolled as k is known
    for i in 0..k/2 {
        u[k/2-i-1]=reg_carry(a[2*i+1], a[2*i]);
    }
    return CarryOutAux(u);
}


/* carry out with carry-in bit c */
pub fn CarryOut(aB: Vec<ClearModp>, bB: Vec<SecretModp>, c: SecretModp) {
    let k = aB.len();
    let mut d = Vec::with_capacity(k);
    // XXXX Again next should be unrolled as k is known
    for i in 0..k {
        let t0=a[i]+b[i]-2*t0;
        let t1=a[i]*b[i];
        d[k-i-1]=[t0, t1];
    }
    d[0][1]+=d[0][1]+d[0][0]*c;
    
    return CarryOutAux(d);
}

/* Add the bits a[k-1], ..., a[0] and b[k-1], ..., b[0], return k+1
   bits s[0], ... , s[k]
*/
pub fn BitAdd(aB: Vec<SecretModp>, bB: Vec<SecretModp>, 
              bits_to_compute: Option<u32>) {
    let k=aB.len();
    let mut b_to_c=k;
    match bits_to_compute {
        None     => { ; }
        Some(aa) => { b_to_c=aa; }
    }


}
