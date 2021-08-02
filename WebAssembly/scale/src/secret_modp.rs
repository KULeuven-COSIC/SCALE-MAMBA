// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use super::*;

impl SecretModp {
    #[inline(always)]
    pub fn from_unsigned(val: SecretI64) -> SecretModp {
        unsafe { __convsuregsint(val) }
    }
}

impl From<ClearModp> for SecretModp {
    #[inline(always)]
    fn from(a: ClearModp) -> SecretModp {
        unsafe {
            let s = __ldsi(0);
            __addm(s, a)
        }
    }
}

impl From<i64> for SecretModp {
    #[inline(always)]
    fn from(a: i64) -> SecretModp {
        Self::from(ClearModp::from(a))
    }
}

impl RevealIfSecret for SecretModp {
    type Output = ClearModp;
    fn reveal_if_secret(&self) -> ClearModp {
        self.reveal()
    }
}

impl Reveal for SecretModp {
    type Output = ClearModp;
    #[inline(always)]
    fn reveal(&self) -> ClearModp {
        unsafe { reveal(*self) }
    }
}

impl core::ops::Div<ClearModp> for SecretModp {
    type Output = SecretModp;
    #[inline(always)]
    fn div(self, s: ClearModp) -> SecretModp {
        self * (ConstI32::<1> / s)
    }
}

impl<const I: i32> core::ops::Div<ConstI32<I>> for SecretModp {
    type Output = SecretModp;
    #[inline(always)]
    fn div(self, s: ConstI32<I>) -> SecretModp {
        self * (ConstI32::<1> / ClearModp::from(s))
    }
}

impl<const I: i32> core::ops::Sub<SecretModp> for ConstI32<I> {
    type Output = SecretModp;
    #[inline(always)]
    fn sub(self, s: SecretModp) -> SecretModp {
        unsafe { __subsfi(s, I) }
    }
}

#[cfg(not(feature = "emulate"))]
impl core::ops::Mul for SecretModp {
    type Output = Self;
    #[inline(always)]
    fn mul(self, other: Self) -> Self {
        extern "C" {
            fn __maybe_optimized_secret_modp_multiplication(
                a: SecretModp,
                b: SecretModp,
            ) -> SecretModp;
        }
        unsafe { __maybe_optimized_secret_modp_multiplication(self, other) }
    }
}

#[cfg(feature = "emulate")]
impl core::ops::Mul for SecretModp {
    type Output = Self;
    #[inline(always)]
    fn mul(self, other: Self) -> Self {
        Self(self.0 * other.0)
    }
}

impl SecretModp {
    #[cfg(not(feature = "emulate"))]
    #[inline(always)]
    pub fn get_random_bit() -> Self {
        unsafe { __bit() }
    }

    #[cfg(feature = "emulate")]
    #[inline(always)]
    pub fn get_random_bit() -> Self {
        use rand::Rng;
        let mut rng = rand::thread_rng();
        SecretModp::from(rng.gen_range(0..2))
    }

    #[inline(always)]
    pub fn get_random_square() -> (Self, Self) {
        __square()
    }

    #[inline(always)]
    pub fn get_random_triple() -> (Self, Self, Self) {
        __triple()
    }
}

impl Randomize for SecretModp {
    #[inline(always)]
    fn randomize() -> SecretModp {
        let (a, _b) = SecretModp::get_random_square();
        a
    }
}

impl SecretModp {
    #[inline(always)]
    pub fn private_output(self, player: i64, channel: i64) {
        unsafe {
            __private_output(self, player, channel);
        }
    }
}

impl SecretModp {
    #[inline(always)]
    pub fn private_input(player: i64, channel: i64) -> Self {
        unsafe { __private_input(player, channel) }
    }
}

impl core::ops::Neg for SecretModp {
    type Output = SecretModp;
    #[inline]
    fn neg(self) -> SecretModp {
        SecretModp::from(0) - self
    }
}
