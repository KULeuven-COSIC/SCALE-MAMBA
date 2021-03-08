
// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use super::*;

impl Output for i64 {
    #[inline(always)]
    fn output<const C: u32>(self, _: Channel<C>) {
        unsafe {
            __output_int(self, C);
        }
    }
}

impl Input for i64 {
    #[inline(always)]
    fn input<const C: u32>(_: Channel<C>) -> Self {
        unsafe { __input_int(C) }
    }
}

impl Rand for i64 {
    #[inline(always)]
    fn rand(self) -> i64 {
        unsafe { __rand(self) }
    }
}

impl RevealIfSecret for i64 {
    type Output = i64;
    fn reveal_if_secret(&self) -> i64 {
        *self
    }
}
