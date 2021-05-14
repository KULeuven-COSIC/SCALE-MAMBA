// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use super::*;
use core::ops::Mul;
use core::ops::Not;

impl From<SecretBit> for SecretI64 {
    #[inline(always)]
    fn from(b: SecretBit) -> SecretI64 {
        unsafe {
            let a = SecretI64::from(ConstI32::<0>);
            __sintbit(a, b.to_raw(), 0)
        }
    }
}

impl Reveal for SecretI64 {
    type Output = i64;
    #[inline(always)]
    fn reveal(&self) -> i64 {
        unsafe { __opensint(*self) }
    }
}

impl RevealIfSecret for SecretI64 {
    type Output = i64;
    fn reveal_if_secret(&self) -> i64 {
        self.reveal()
    }
}

impl Randomize for SecretI64 {
    #[inline(always)]
    fn randomize() -> SecretI64 {
        unsafe { __randsint() }
    }
}

impl Not for SecretI64 {
    type Output = SecretI64;
    #[inline(always)]
    fn not(self) -> Self {
        unsafe { __invsint(self) }
    }
}

impl core::ops::Div<i64> for SecretI64 {
    type Output = SecretI64;
    #[inline(always)]
    fn div(self, y: i64) -> SecretI64 {
        let secrety = SecretI64::from(y);
        self / secrety
    }
}
impl core::ops::Div<SecretI64> for i64 {
    type Output = SecretI64;
    #[inline(always)]
    fn div(self, y: SecretI64) -> SecretI64 {
        let secretself = SecretI64::from(self);
        secretself / y
    }
}

impl ScaleCmp<Self, SecretBit> for SecretI64 {
    #[inline(always)]
    fn lt(self, other: Self) -> SecretBit {
        unsafe { __ltzsint(self - other) }.to_real()
    }
    #[inline(always)]
    fn le(self, other: Self) -> SecretBit {
        unsafe { (__ltzsint(self - other)).to_real() | (__eqzsint(other - self)).to_real() }
    }
    #[inline(always)]
    fn gt(self, other: Self) -> SecretBit {
        unsafe { __ltzsint(other - self) }.to_real()
    }
    #[inline(always)]
    fn ge(self, other: Self) -> SecretBit {
        unsafe { (__ltzsint(other - self)).to_real() | (__eqzsint(other - self)).to_real() }
    }
    #[inline(always)]
    fn eq(self, other: Self) -> SecretBit {
        unsafe { __eqzsint(other - self) }.to_real()
    }
    #[inline(always)]
    fn ne(self, other: Self) -> SecretBit {
        !self.eq(other)
    }
}

impl ScaleCmpZ<SecretBit> for SecretI64 {
    #[inline(always)]
    fn ltz(self) -> SecretBit {
        unsafe { __ltzsint(self).to_real() }
    }
    #[inline(always)]
    fn lez(self) -> SecretBit {
        unsafe { (__ltzsint(self)).to_real() | (__eqzsint(self)).to_real() }
    }
    #[inline(always)]
    fn gtz(self) -> SecretBit {
        !self.lez()
    }
    #[inline(always)]
    fn gez(self) -> SecretBit {
        !self.ltz()
    }
    #[inline(always)]
    fn eqz(self) -> SecretBit {
        unsafe { __eqzsint(self).to_real() }
    }
    #[inline(always)]
    fn nez(self) -> SecretBit {
        !self.eqz()
    }
}

impl ScaleCmp<i64, SecretBit> for SecretI64 {
    #[inline(always)]
    fn lt(self, other: i64) -> SecretBit {
        unsafe { __ltzsint(self - other).to_real() }
    }
    #[inline(always)]
    fn le(self, other: i64) -> SecretBit {
        unsafe { (__ltzsint(self - other)).to_real() | (__eqzsint(other - self)).to_real() }
    }
    #[inline(always)]
    fn gt(self, other: i64) -> SecretBit {
        unsafe { __ltzsint(other - self).to_real() }
    }
    #[inline(always)]
    fn ge(self, other: i64) -> SecretBit {
        unsafe { (__ltzsint(other - self)).to_real() | (__eqzsint(other - self)).to_real() }
    }
    #[inline(always)]
    fn eq(self, other: i64) -> SecretBit {
        unsafe { __eqzsint(other - self).to_real() }
    }
    #[inline(always)]
    fn ne(self, other: i64) -> SecretBit {
        !self.eq(other)
    }
}

impl ScaleCmp<SecretI64, SecretBit> for i64 {
    #[inline(always)]
    fn lt(self, other: SecretI64) -> SecretBit {
        other.lt(self)
    }
    #[inline(always)]
    fn le(self, other: SecretI64) -> SecretBit {
        other.le(self)
    }
    #[inline(always)]
    fn gt(self, other: SecretI64) -> SecretBit {
        other.gt(self)
    }
    #[inline(always)]
    fn ge(self, other: SecretI64) -> SecretBit {
        other.ge(self)
    }
    #[inline(always)]
    fn eq(self, other: SecretI64) -> SecretBit {
        other.eq(self)
    }
    #[inline(always)]
    fn ne(self, other: SecretI64) -> SecretBit {
        other.ne(self)
    }
}

impl SecretI64 {
    #[inline(always)]
    pub fn set_bit<const I: u32>(self, b: SecretBit, _: ConstU32<I>) -> SecretI64 {
        unsafe { __sintbit(self, b.to_raw(), I) }
    }
}

impl SecretI64 {
    #[inline(always)]
    pub fn get_bit<const I: u32>(self, _: ConstU32<I>) -> SecretBit {
        unsafe { __bitsint(self, I).to_real() }
    }
}

impl Mul<SecretBit> for SecretI64 {
    type Output = Self;
    #[inline(always)]
    fn mul(self, bit: SecretBit) -> Self::Output {
        unsafe { __sand(self, bit.to_raw()) }
    }
}

impl Mul<SecretI64> for SecretBit {
    type Output = SecretI64;
    #[inline(always)]
    fn mul(self, val: SecretI64) -> Self::Output {
        unsafe { __sand(val, self.to_raw()) }
    }
}

impl SecretI64 {
    #[inline(always)]
    pub fn mult2(a: SecretI64, b: SecretI64) -> (Self, Self) {
        __mul2sint(a, b)
    }
}
