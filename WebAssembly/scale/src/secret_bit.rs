// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use super::*;
#[cfg(feature = "emulate")]
use core::convert::TryFrom;

impl RawSecretBit {
    pub fn to_real(self) -> SecretBit {
        #[cfg(feature = "emulate")]
        {
            SecretBit(self.0)
        }
        #[cfg(not(feature = "emulate"))]
        unsafe {
            SecretBit(__setbit(self, 0))
        }
    }
}

impl SecretBit {
    pub fn to_raw(self) -> RawSecretBit {
        #[cfg(feature = "emulate")]
        {
            RawSecretBit(self.0)
        }
        #[cfg(not(feature = "emulate"))]
        unsafe {
            __bitsint(self.0, 0)
        }
    }
}

impl Reveal for SecretBit {
    type Output = bool;
    #[inline(always)]
    fn reveal(&self) -> bool {
        #[cfg(feature = "emulate")]
        {
            self.0 == 1
        }
        #[cfg(not(feature = "emulate"))]
        unsafe {
            __opensbit(__bitsint(self.0, 0))
        }
    }
}

impl core::ops::Not for SecretBit {
    type Output = Self;
    fn not(self) -> Self {
        #[cfg(feature = "emulate")]
        {
            if self.0 != 0 && self.0 != 1 {
                std::panic!()
            }
            Self(1 - self.0)
        }
        #[cfg(not(feature = "emulate"))]
        unsafe {
            SecretBit(__setbit(__negb(__bitsint(self.0, 0)),0))
        }
    }
}

impl core::ops::BitOr for SecretBit {
    type Output = Self;
    fn bitor(self, other: Self) -> Self {
        #[cfg(feature = "emulate")]
        {
            Self(self.0 | other.0)
        }
        #[cfg(not(feature = "emulate"))]
        unsafe {
            SecretBit(__setbit(__orsb(
                __bitsint(self.0,0),
                __bitsint(other.0,0),
                ),0))
        }
    }
}
impl core::ops::BitXor for SecretBit {
    type Output = Self;
    fn bitxor(self, other: Self) -> Self {
        #[cfg(feature = "emulate")]
        {
            Self(self.0 ^ other.0)
        }
        #[cfg(not(feature = "emulate"))]
        unsafe {
            SecretBit(__setbit(__xorsb(
                __bitsint(self.0,0),
                __bitsint(other.0,0),
            ),0))
        }
    }
}
impl core::ops::BitAnd for SecretBit {
    type Output = Self;
    fn bitand(self, other: Self) -> Self {
        #[cfg(feature = "emulate")]
        {
            Self(self.0 & other.0)
        }
        #[cfg(not(feature = "emulate"))]
        unsafe {
            SecretBit(__setbit(__andsb(
                __bitsint(self.0,0),
                __bitsint(other.0,0),
            ),0))
        }
    }
}

impl From<bool> for SecretBit {
    fn from(b: bool) -> Self {
        Self(if b {
            From::from(ConstI32::<1>)
        } else {
            From::from(ConstI32::<0>)
        })
    }
}

impl From<SecretModp> for SecretBit {
    fn from(val: SecretModp) -> Self {
        #[cfg(feature = "emulate")]
        {
            Self(i64::try_from(val.0).unwrap())
        }
        #[cfg(not(feature = "emulate"))]
        {
           unsafe {
               Self(__convsintsreg(val))
           }
        }
    }
}

impl From<SecretBit> for SecretModp {
    fn from(val: SecretBit) -> Self {
        #[cfg(feature = "emulate")]
        {
            Self::try_from(val.0).unwrap()
        }
        #[cfg(not(feature = "emulate"))]
        {
            unsafe { __convsregsint(val.0) }
        }
    }
}

impl Randomize for SecretBit {
    #[inline(always)]
    fn randomize() -> SecretBit {
        #[cfg(feature = "emulate")]
        {
            SecretBit(0)
        }
        #[cfg(not(feature = "emulate"))]
        {
           unsafe{ SecretBit(__setbit( __randsbit(), 0)) }
        }
    }
}
