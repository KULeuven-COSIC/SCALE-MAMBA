// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use crate::circuits::*;
use crate::fixed_point::*;
use crate::local_functions::*;
use core::cmp::Ordering;
use core::cmp::Ordering::*;
use core::mem;
use core::ops::{Add, Div, Mul, Neg, Sub};
use scale::alloc::*;
use scale::*;

// The secure IEEE Type
#[derive(Copy, Clone)]
pub struct SecretIEEE {
    val: SecretI64,
}

// The holding "insecure" IEEE Type
#[derive(Copy, Clone, Eq, PartialEq)]
pub struct ClearIEEE {
    val: i64,
}

/*
 * Stuff to enable usage in arrays etc
 *
 */

impl GetAllocator for ClearIEEE {
    type Allocator = &'static Allocator<i64>;
    fn get_allocator() -> &'static Allocator<i64> {
        i64::get_allocator()
    }
    fn size_type() -> u64 {
        1
    }
}

impl LoadFromMem<i64> for ClearIEEE {
    fn load_from_mem(idx: i64) -> Self {
        let a = i64::load_from_mem(idx);
        ClearIEEE { val: a }
    }
}

impl StoreInMem<i64> for ClearIEEE {
    unsafe fn store_in_mem(&self, idx: i64) {
        self.val.store_in_mem(idx);
    }
}

impl GetAllocator for SecretIEEE {
    type Allocator = &'static Allocator<SecretI64>;
    fn get_allocator() -> &'static Allocator<SecretI64> {
        SecretI64::get_allocator()
    }
    fn size_type() -> u64 {
        1
    }
}

impl LoadFromMem<i64> for SecretIEEE {
    fn load_from_mem(idx: i64) -> Self {
        let a = SecretI64::load_from_mem(idx);
        SecretIEEE { val: a }
    }
}

impl StoreInMem<i64> for SecretIEEE {
    unsafe fn store_in_mem(&self, idx: i64) {
        self.val.store_in_mem(idx);
    }
}

// Get the bit representation in an i64
impl ClearIEEE {
    #[inline(always)]
    pub fn rep(self) -> i64 {
        self.val
    }
}

// Get the bit representation in an SecretI64
impl SecretIEEE {
    #[inline(always)]
    pub fn rep(self) -> SecretI64 {
        self.val
    }
}

/* Set the underlying representation to something */

impl ClearIEEE {
    #[inline(always)]
    pub fn set(v: i64) -> Self {
        Self { val: v }
    }
}

impl SecretIEEE {
    #[inline(always)]
    pub fn set(v: SecretI64) -> Self {
        Self { val: v }
    }
}

/* Print a ClearIEEE */

impl Print for ClearIEEE {
    #[inline(always)]
    fn print(self) {
        unsafe { __print_ieee_float(self.val) }
    }
}

/* The conversion routines */

impl From<ClearIEEE> for SecretIEEE {
    #[inline(always)]
    fn from(c: ClearIEEE) -> SecretIEEE {
        SecretIEEE {
            val: unsafe { __convregsreg(c.val) },
        }
    }
}

impl From<f64> for ClearIEEE {
    #[inline(always)]
    fn from(f: f64) -> ClearIEEE {
        unsafe { mem::transmute(f) }
    }
}

impl From<f64> for SecretIEEE {
    #[inline(always)]
    fn from(f: f64) -> SecretIEEE {
        let t = unsafe { mem::transmute(f) };
        SecretIEEE {
            val: unsafe { __convregsreg(t) },
        }
    }
}

impl From<SecretI64> for SecretIEEE {
    #[inline(always)]
    fn from(s: SecretI64) -> SecretIEEE {
        SecretIEEE { val: IEEE_i2f(s) }
    }
}

impl From<SecretBit> for SecretIEEE {
    #[inline(always)]
    fn from(b: SecretBit) -> SecretIEEE {
        let s: SecretI64 = SecretI64::from(b);
        SecretIEEE { val: IEEE_i2f(s) }
    }
}

impl From<i64> for ClearIEEE {
    #[inline(always)]
    fn from(i: i64) -> ClearIEEE {
        ClearIEEE { val: FP_i2f(i) }
    }
}

impl From<i64> for SecretIEEE {
    #[inline(always)]
    fn from(i: i64) -> SecretIEEE {
        let s = SecretI64::from(i);
        SecretIEEE { val: IEEE_i2f(s) }
    }
}

impl From<SecretIEEE> for SecretI64 {
    #[inline(always)]
    fn from(f: SecretIEEE) -> SecretI64 {
        IEEE_f2i(f.val)
    }
}

impl From<ClearIEEE> for i64 {
    #[inline(always)]
    fn from(f: ClearIEEE) -> i64 {
        FP_f2i(f.val)
    }
}

impl<const K: u64, const F: u64> From<ClearFixed<K, F>> for ClearIEEE {
    #[inline(always)]
    fn from(a: ClearFixed<K, F>) -> Self {
        let v = from_fix(a.rep().rep(), K as i64, F as i64);
        Self { val: v }
    }
}

impl Randomize for SecretIEEE {
    #[inline(always)]
    fn randomize() -> SecretIEEE {
        let mut v = unsafe { __randsint() };
        let zero: SecretBit = SecretBit::from(false);
        let one: SecretBit = SecretBit::from(true);
        v = v.set_bit(zero, ConstU32::<63>);
        v = v.set_bit(zero, ConstU32::<62>);
        v = v.set_bit(one, ConstU32::<61>);
        v = v.set_bit(one, ConstU32::<60>);
        v = v.set_bit(one, ConstU32::<59>);
        v = v.set_bit(one, ConstU32::<58>);
        v = v.set_bit(one, ConstU32::<57>);
        v = v.set_bit(one, ConstU32::<56>);
        v = v.set_bit(one, ConstU32::<55>);
        v = v.set_bit(one, ConstU32::<54>);
        v = v.set_bit(one, ConstU32::<53>);
        v = v.set_bit(zero, ConstU32::<52>);
        Self { val: v }
    }
}

impl Randomize for ClearIEEE {
    #[inline(always)]
    fn randomize() -> ClearIEEE {
        let v = unsafe { __randfloat() };
        Self { val: v }
    }
}

/* Reveal */

// This is a NOP to make certain generic operations work
impl Reveal for ClearIEEE {
    type Output = ClearIEEE;
    #[inline(always)]
    fn reveal(&self) -> ClearIEEE {
        ClearIEEE { val: self.val }
    }
}

impl Reveal for SecretIEEE {
    type Output = ClearIEEE;
    #[inline(always)]
    fn reveal(&self) -> ClearIEEE {
        ClearIEEE {
            val: self.val.reveal(),
        }
    }
}

/* Comparison Routines */

impl PartialOrd for ClearIEEE {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        if self == other {
            return Some(Equal);
        }
        let a = FP_lt(self.rep(), other.rep());
        if a == 1 {
            return Some(Less);
        }
        let b = FP_lt(other.rep(), self.rep());
        if b == 1 {
            return Some(Greater);
        }

        None
    }
}

/* This is purely for usage in generic routines */
impl ScaleCmp<Self, i64> for ClearIEEE {
    #[inline(always)]
    fn lt(self, other: Self) -> i64 {
        let ans: i64 = i64::from(self < other);
        ans
    }
    #[inline(always)]
    fn le(self, other: Self) -> i64 {
        let ans: i64 = i64::from(self <= other);
        ans
    }
    #[inline(always)]
    fn gt(self, other: Self) -> i64 {
        let ans: i64 = i64::from(self > other);
        ans
    }
    #[inline(always)]
    fn ge(self, other: Self) -> i64 {
        let ans: i64 = i64::from(self >= other);
        ans
    }
    #[inline(always)]
    fn eq(self, other: Self) -> i64 {
        let ans: i64 = i64::from(self == other);
        ans
    }
    #[inline(always)]
    fn ne(self, other: Self) -> i64 {
        let ans: i64 = i64::from(self != other);
        ans
    }
}

impl ScaleCmpZ<i64> for ClearIEEE {
    #[inline(always)]
    fn ltz(self) -> i64 {
        let ans: i64 = i64::from(self < ClearIEEE::from(0.0));
        ans
    }
    #[inline(always)]
    fn lez(self) -> i64 {
        let ans: i64 = i64::from(self <= ClearIEEE::from(0.0));
        ans
    }
    #[inline(always)]
    fn gtz(self) -> i64 {
        let ans: i64 = i64::from(self > ClearIEEE::from(0.0));
        ans
    }
    #[inline(always)]
    fn gez(self) -> i64 {
        let ans: i64 = i64::from(self >= ClearIEEE::from(0.0));
        ans
    }
    #[inline(always)]
    fn eqz(self) -> i64 {
        let ans: i64 = i64::from(self == ClearIEEE::from(0.0));
        ans
    }
    #[inline(always)]
    fn nez(self) -> i64 {
        let ans: i64 = i64::from(self != ClearIEEE::from(0.0));
        ans
    }
}

impl ScaleCmp<Self, SecretBit> for SecretIEEE {
    #[inline(always)]
    fn lt(self, other: Self) -> SecretBit {
        let ans = IEEE_lt([self.val, other.val]);
        unsafe { __bitsint(ans, 0).to_real() }
    }
    #[inline(always)]
    fn le(self, other: Self) -> SecretBit {
        self.lt(other) | self.eq(other)
    }
    #[inline(always)]
    fn gt(self, other: Self) -> SecretBit {
        let ans = IEEE_lt([other.val, self.val]);
        unsafe { __bitsint(ans, 0).to_real() }
    }
    #[inline(always)]
    fn ge(self, other: Self) -> SecretBit {
        self.gt(other) | self.eq(other)
    }
    #[inline(always)]
    fn eq(self, other: Self) -> SecretBit {
        let ans = IEEE_eq([other.val, self.val]);
        unsafe { __bitsint(ans, 0).to_real() }
    }
    #[inline(always)]
    fn ne(self, other: Self) -> SecretBit {
        !self.eq(other)
    }
}

impl ScaleCmpZ<SecretBit> for SecretIEEE {
    #[inline(always)]
    fn ltz(self) -> SecretBit {
        let zero = SecretI64::from(0);
        let ans = IEEE_lt([self.val, zero]);
        unsafe { __bitsint(ans, 0).to_real() }
    }
    #[inline(always)]
    fn lez(self) -> SecretBit {
        self.ltz() | self.eqz()
    }
    #[inline(always)]
    fn gtz(self) -> SecretBit {
        let zero = SecretI64::from(0);
        let ans = IEEE_lt([zero, self.val]);
        unsafe { __bitsint(ans, 0).to_real() }
    }
    #[inline(always)]
    fn gez(self) -> SecretBit {
        self.gtz() | self.eqz()
    }
    #[inline(always)]
    fn eqz(self) -> SecretBit {
        let zero = SecretI64::from(0);
        let ans = IEEE_eq([self.val, zero]);
        unsafe { __bitsint(ans, 0).to_real() }
    }
    #[inline(always)]
    fn nez(self) -> SecretBit {
        !self.eqz()
    }
}

/* Arithmetic */

impl Mul<SecretIEEE> for SecretIEEE {
    type Output = Self;
    #[inline(always)]
    fn mul(self, other: SecretIEEE) -> Self::Output {
        SecretIEEE {
            val: IEEE_mul([self.val, other.val]),
        }
    }
}

impl Mul<SecretIEEE> for ClearIEEE {
    type Output = SecretIEEE;
    #[inline(always)]
    fn mul(self, other: SecretIEEE) -> Self::Output {
        let s: SecretIEEE = SecretIEEE::from(self);
        SecretIEEE {
            val: IEEE_mul([s.val, other.val]),
        }
    }
}

impl Mul<ClearIEEE> for SecretIEEE {
    type Output = SecretIEEE;
    #[inline(always)]
    fn mul(self, other: ClearIEEE) -> Self::Output {
        let o: SecretIEEE = SecretIEEE::from(other);
        SecretIEEE {
            val: IEEE_mul([self.val, o.val]),
        }
    }
}

impl Mul<ClearIEEE> for ClearIEEE {
    type Output = ClearIEEE;
    #[inline(always)]
    fn mul(self, other: ClearIEEE) -> Self::Output {
        ClearIEEE {
            val: FP_mul(self.val, other.val),
        }
    }
}

impl Add<SecretIEEE> for SecretIEEE {
    type Output = Self;
    #[inline(always)]
    fn add(self, other: SecretIEEE) -> Self::Output {
        SecretIEEE {
            val: IEEE_add([self.val, other.val]),
        }
    }
}

impl Add<SecretIEEE> for ClearIEEE {
    type Output = SecretIEEE;
    #[inline(always)]
    fn add(self, other: SecretIEEE) -> Self::Output {
        let s: SecretIEEE = SecretIEEE::from(self);
        SecretIEEE {
            val: IEEE_add([s.val, other.val]),
        }
    }
}

impl Add<ClearIEEE> for SecretIEEE {
    type Output = SecretIEEE;
    #[inline(always)]
    fn add(self, other: ClearIEEE) -> Self::Output {
        let o: SecretIEEE = SecretIEEE::from(other);
        SecretIEEE {
            val: IEEE_add([self.val, o.val]),
        }
    }
}

impl Add<ClearIEEE> for ClearIEEE {
    type Output = ClearIEEE;
    #[inline(always)]
    fn add(self, other: ClearIEEE) -> Self::Output {
        ClearIEEE {
            val: FP_add(self.val, other.val),
        }
    }
}

impl Div<SecretIEEE> for SecretIEEE {
    type Output = Self;
    #[inline(always)]
    fn div(self, other: SecretIEEE) -> Self::Output {
        SecretIEEE {
            val: IEEE_div([self.val, other.val]),
        }
    }
}

impl Div<SecretIEEE> for ClearIEEE {
    type Output = SecretIEEE;
    #[inline(always)]
    fn div(self, other: SecretIEEE) -> Self::Output {
        let s: SecretIEEE = SecretIEEE::from(self);
        SecretIEEE {
            val: IEEE_div([s.val, other.val]),
        }
    }
}

impl Div<ClearIEEE> for SecretIEEE {
    type Output = SecretIEEE;
    #[inline(always)]
    fn div(self, other: ClearIEEE) -> Self::Output {
        let o: SecretIEEE = SecretIEEE::from(other);
        SecretIEEE {
            val: IEEE_div([self.val, o.val]),
        }
    }
}

impl Div<ClearIEEE> for ClearIEEE {
    type Output = ClearIEEE;
    #[inline(always)]
    fn div(self, other: ClearIEEE) -> Self::Output {
        ClearIEEE {
            val: FP_div(self.val, other.val),
        }
    }
}

impl Neg for SecretIEEE {
    type Output = Self;
    #[inline(always)]
    fn neg(self) -> Self::Output {
        SecretIEEE {
            val: unsafe { __xorsintc(self.val, 1_i64 << 63) },
        }
    }
}

impl Neg for ClearIEEE {
    type Output = Self;
    #[inline(always)]
    fn neg(self) -> Self::Output {
        ClearIEEE {
            val: self.val ^ (1_i64 << 63),
        }
    }
}

impl Sub<SecretIEEE> for SecretIEEE {
    type Output = Self;
    #[inline(always)]
    fn sub(self, other: SecretIEEE) -> Self::Output {
        SecretIEEE {
            val: IEEE_add([self.val, (-other).val]),
        }
    }
}

impl Sub<SecretIEEE> for ClearIEEE {
    type Output = SecretIEEE;
    #[inline(always)]
    fn sub(self, other: SecretIEEE) -> Self::Output {
        let s: SecretIEEE = SecretIEEE::from(self);
        SecretIEEE {
            val: IEEE_add([s.val, (-other).val]),
        }
    }
}

impl Sub<ClearIEEE> for SecretIEEE {
    type Output = SecretIEEE;
    #[inline(always)]
    fn sub(self, other: ClearIEEE) -> Self::Output {
        let o: SecretIEEE = SecretIEEE::from(-other);
        SecretIEEE {
            val: IEEE_add([self.val, o.val]),
        }
    }
}

impl Sub<ClearIEEE> for ClearIEEE {
    type Output = ClearIEEE;
    #[inline(always)]
    fn sub(self, other: ClearIEEE) -> Self::Output {
        ClearIEEE {
            val: FP_add(self.val, (-other).val),
        }
    }
}
