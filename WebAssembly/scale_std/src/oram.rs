// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use crate::array::*;
use crate::bit_protocols::*;
use crate::slice::*;
use scale::*;

/*
####################################
#### BASIC SIMPLE ORAM ROUTINES ####
####################################
*/

/* We execute this totally via pointers, and not arrays/slices directly
 * to avoid a lot of memory thrashing. Thus we process the values
 * `in-place'
 * The input "array" is assumed to be already bit-decomposed, and the 
 * output array is assumed to be of size 
 *    2^len+2^{len/2}
 * The extra bit of space is to allow some extra processing space
 *  - Protocol 2 from Keller+Scholl
 */
#[inline(always)]
#[allow(non_snake_case)]
pub fn Demux_Array(output_addr: i64, input_addr: i64, t: u64, len: i64) {
    if len <= 1 {
        let s = SecretModp::load_from_mem(input_addr);
        let ns = ClearModp::from(1) - s;
        unsafe { ns.store_in_mem(output_addr) };
        unsafe { s.store_in_mem(output_addr + 1) };
    } else {
        let nlen = two_power((len / 2) as u64) as i64;
        let tlen = two_power(len as u64) as i64;
        // Compute b
        Demux_Array(output_addr, input_addr, t, len / 2);
        // Compute c
        Demux_Array(
            output_addr + tlen,
            input_addr + len / 2,
            t-tlen as u64, 
            len - len / 2,
        );
        // Process multiplication by c0 at the end to avoid issues
        // re over-writing data which is needed for other things.
        for i in nlen..tlen {
            let j = i % nlen;
            let k = i / nlen;
            let bj = SecretModp::load_from_mem(output_addr + j);
            let ck = SecretModp::load_from_mem(output_addr + tlen + k);
            let a = bj * ck;
            unsafe { a.store_in_mem(output_addr + i) };
        }
        let c0 = SecretModp::load_from_mem(output_addr + tlen);
        for j in 0..nlen {
            let bj = SecretModp::load_from_mem(output_addr + j);
            let a = bj * c0;
            unsafe { a.store_in_mem(output_addr + j) };
        }
    }
}

/* The assumption here is that s is less than 2^K 
 *  - Outputs the Demux data, plus some extra garbage to avoid
 *    memory trashing
 */
#[inline(always)]
#[allow(non_snake_case)]
pub fn Demux<const K: u64, const KAPPA: u64>(s: SecretModp) -> Slice<SecretModp> {
    let bit_decomp = BitDec::<K, K, KAPPA>(s);
    let nlen = two_power(K);
    // Need 2*nlen here as we need some space to avoid memory trashing
    let t = 2*nlen;
    let b: Slice<SecretModp> = Slice::uninitialized(t);
    Demux_Array(b.addr(0), bit_decomp.addr(0), t, K as i64);
    b
}

/* Same but now K is a run time value */
#[inline(always)]
#[allow(non_snake_case)]
pub fn demux<const KAPPA: u64>(s: SecretModp, k: i64) -> Slice<SecretModp> {
    let bit_decomp = bitdec::<KAPPA>(s, k);
    let nlen = two_power(k as u64);
    // Need 2*nlen here as we need some space to avoid memory trashing
    let t = 2*nlen;
    let b: Slice<SecretModp> = Slice::uninitialized(t);
    Demux_Array(b.addr(0), bit_decomp.addr(0), t, k);
    b
}



// oram read/write a SecretModp slice
impl<'a> Slice<SecretModp> {
    #[inline(always)]
    pub fn read_oram<const K: u64, const KAPPA: u64>(&'a self, i: SecretModp) -> SecretModp {
        let demux = Demux::<K, KAPPA>(i);
        let mut ans = SecretModp::from(0);
        for i in 0..self.len() {
            ans = ans + *demux.get_unchecked(i) * *self.get_unchecked(i);
        }
        ans
    }

    #[inline(always)]
    pub fn write_oram<const K: u64, const KAPPA: u64>(&mut self, i: SecretModp, val: SecretModp) {
        let demux = Demux::<K, KAPPA>(i);
        for i in 0..self.len() {
            let d = *demux.get_unchecked(i);
            let a = *self.get_unchecked(i);
            let v = a * (ClearModp::from(1) - d) + val * d;
            self.set(i, &v);
        }
    }

    #[inline(always)]
    pub fn read_with_demux(&'a self, demux: &'a Slice<SecretModp>) -> SecretModp {
        let mut ans = SecretModp::from(0);
        for i in 0..self.len() {
            ans = ans + *demux.get_unchecked(i) * *self.get_unchecked(i);
        }
        ans
    }

    #[inline(always)]
    pub fn write_with_demux(&mut self, demux: &'a Slice<SecretModp>, val: SecretModp) {
        for i in 0..self.len() {
            let d = *demux.get_unchecked(i);
            let a = *self.get_unchecked(i);
            let v = a * (ClearModp::from(1) - d) + val * d;
            self.set(i, &v);
        }
    }

    #[inline(always)]
    pub fn read_with_demux_and_offset(&'a self, offset: i64, demux: &'a Slice<SecretModp>) -> SecretModp {
        let mut ans = SecretModp::from(0);
        for i in 0..demux.len() {
            ans = ans + *demux.get_unchecked(i) * *self.get_unchecked(offset as u64 + i);
        }
        ans
    }

    #[inline(always)]
    pub fn write_with_demux_and_offset(&mut self, offset: i64, demux: &'a Slice<SecretModp>, val: SecretModp) {
        for i in 0..demux.len() {
            let d = *demux.get_unchecked(i);
            let a = *self.get_unchecked(offset as u64 + i);
            let v = a * (ClearModp::from(1) - d) + val * d;
            self.set(offset as u64 + i, &v);
        }
    }

}

// oram read/write a SecretModp array
impl<'a, const N: u64> Array<SecretModp, N> {
    #[inline(always)]
    pub fn read_oram<const K: u64, const KAPPA: u64>(&'a self, i: SecretModp) -> SecretModp {
        let demux = Demux::<K, KAPPA>(i);
        let mut ans = SecretModp::from(0);
        for i in 0..N {
            ans = ans + *demux.get_unchecked(i) * *self.get_unchecked(i);
        }
        ans
    }

    #[inline(always)]
    pub fn write_oram<const K: u64, const KAPPA: u64>(&mut self, i: SecretModp, val: SecretModp) {
        let demux = Demux::<K, KAPPA>(i);
        for i in 0..N {
            let d = *demux.get_unchecked(i);
            let a = *self.get_unchecked(i);
            let v = a * (ClearModp::from(1) - d) + val * d;
            self.set(i, &v);
        }
    }

    #[inline(always)]
    pub fn read_with_demux(&'a self, demux: &'a Slice<SecretModp>) -> SecretModp {
        let mut ans = SecretModp::from(0);
        for i in 0..N {
            ans = ans + *demux.get_unchecked(i) * *self.get_unchecked(i);
        }
        ans
    }

    #[inline(always)]
    pub fn write_with_demux(&mut self, demux: &'a Slice<SecretModp>, val: SecretModp) {
        for i in 0..N {
            let d = *demux.get_unchecked(i);
            let a = *self.get_unchecked(i);
            let v = a * (ClearModp::from(1) - d) + val * d;
            self.set(i, &v);
        }
    }

        #[inline(always)]
    pub fn read_with_demux_and_offset(&'a self, offset: i64, demux: &'a Slice<SecretModp>) -> SecretModp {
        let mut ans = SecretModp::from(0);
        for i in 0..demux.len() {
            ans = ans + *demux.get_unchecked(i) * *self.get_unchecked(offset as u64 + i);
        }
        ans
    }

    #[inline(always)]
    pub fn write_with_demux_and_offset(&mut self, offset: i64, demux: &'a Slice<SecretModp>, val: SecretModp) {
        for i in 0..demux.len() {
            let d = *demux.get_unchecked(i);
            let a = *self.get_unchecked(offset as u64 + i);
            let v = a * (ClearModp::from(1) - d) + val * d;
            self.set(offset as u64 + i, &v);
        }
    }

}
