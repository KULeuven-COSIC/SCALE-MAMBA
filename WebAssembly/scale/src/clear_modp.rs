// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use super::*;

impl From<ClearModp> for i64 {
    #[inline(always)]
    fn from(s: ClearModp) -> i64 {
        unsafe { __convmodp(s, 64) }
    }
}

impl<const I: i32> core::ops::Sub<ClearModp> for ConstI32<I> {
    type Output = ClearModp;
    #[inline(always)]
    fn sub(self, c: ClearModp) -> ClearModp {
        unsafe { __subcfi(c, I) }
    }
}

impl<const I: i32> core::ops::Div<ClearModp> for ConstI32<I> {
    type Output = ClearModp;
    #[inline(always)]
    fn div(self, c: ClearModp) -> ClearModp {
        ClearModp::from(self) / c
    }
}

impl<const I: i32> core::ops::Rem<ClearModp> for ConstI32<I> {
    type Output = ClearModp;
    #[inline(always)]
    fn rem(self, c: ClearModp) -> ClearModp {
        ClearModp::from(self) % c
    }
}

impl Output for ClearModp {
    #[inline(always)]
    fn output(self, channel: i64) {
        unsafe {
            __output_clear(self, channel);
        }
    }
}

impl Input for ClearModp {
    #[inline(always)]
    fn input(channel: i64) -> Self {
        unsafe { __input_clear(channel) }
    }
}

impl Print for ClearModp {
    #[inline(always)]
    fn print(self) {
        unsafe { __print_reg(self) }
    }
}

impl ClearModp {
    #[inline(always)]
    pub fn legendre(self) -> ClearModp {
        unsafe { __legendrec(self) }
    }
}

impl ClearModp {
    #[inline(always)]
    pub fn digest(self) -> ClearModp {
        unsafe { __digestc(self) }
    }
}

impl RevealIfSecret for ClearModp {
    type Output = Self;
    fn reveal_if_secret(&self) -> Self {
        *self
    }
}

impl core::ops::Neg for ClearModp {
    type Output = ClearModp;
    #[inline]
    fn neg(self) -> ClearModp {
        ClearModp::from(0) - self
    }
}

impl Randomize for ClearModp {
    #[inline(always)]
    fn randomize() -> ClearModp {
        unsafe { __randc() }
    }
}
