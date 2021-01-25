use crate::StackAddress;
use core::ops::{Add, Sub};

impl Add<i64> for StackAddress {
    type Output = Self;
    #[inline(always)]
    fn add(self, i: i64) -> Self {
        Self(self.0 + i)
    }
}

impl Sub<i64> for StackAddress {
    type Output = Self;
    #[inline(always)]
    fn sub(self, i: i64) -> Self {
        Self(self.0 - i)
    }
}

impl Sub<StackAddress> for StackAddress {
    type Output = i64;
    #[inline(always)]
    fn sub(self, other: Self) -> i64 {
        self.0 - other.0
    }
}
