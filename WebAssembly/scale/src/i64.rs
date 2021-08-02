// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use super::*;

impl Output for i64 {
    #[inline(always)]
    fn output(self, channel: i64) {
        unsafe {
            __output_int(self, channel);
        }
    }
}

impl Input for i64 {
    #[inline(always)]
    fn input(channel: i64) -> Self {
        unsafe { __input_int(channel) }
    }
}

impl Rand for i64 {
    #[inline(always)]
    fn rand(self) -> i64 {
        unsafe { __rand(self) }
    }
}

impl Randomize for i64 {
    #[inline(always)]
    fn randomize() -> i64 {
        unsafe { __randint() }
    }
}

impl RevealIfSecret for i64 {
    type Output = i64;
    fn reveal_if_secret(&self) -> i64 {
        *self
    }
}
