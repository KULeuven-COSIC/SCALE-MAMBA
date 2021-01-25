use super::*;

impl Test for i64 {
    #[track_caller]
    #[inline(always)]
    fn test(self) {
        let loc = core::panic::Location::caller();
        unsafe { self.store_in_mem(i64::from(loc.line()) + I64_MEMORY + TEST_MEMORY_OFFSET) }
    }
}

impl TestValue for i64 {
    #[track_caller]
    #[inline(always)]
    fn test_value(self, _: i64) {
        self.test();
    }
}

impl TestValue for bool {
    #[track_caller]
    #[inline(always)]
    fn test_value(self, _: bool) {
        (self as i64).test();
    }
}

impl TestValue for SecretI64 {
    #[track_caller]
    #[inline(always)]
    fn test_value(self, _: SecretI64) {
        self.test();
    }
}

impl TestValue for ClearModp {
    #[track_caller]
    #[inline(always)]
    fn test_value(self, _: ClearModp) {
        self.test();
    }
}

impl TestValue for SecretModp {
    #[track_caller]
    #[inline(always)]
    fn test_value(self, _: SecretModp) {
        self.test();
    }
}

impl TestValue for SecretBit {
    #[track_caller]
    #[inline(always)]
    fn test_value(self, _: SecretBit) {
        self.test();
    }
}

impl Test for bool {
    #[track_caller]
    #[inline(always)]
    fn test(self) {
        (self as i64).test()
    }
}

impl TestMem for i64 {
    #[track_caller]
    #[inline(always)]
    fn test_mem<const I: u32>(self, _t_location: ConstU32<I>) {}
}

impl Test for ClearModp {
    #[track_caller]
    #[inline(always)]
    fn test(self) {
        let loc = core::panic::Location::caller();
        unsafe { self.store_in_mem(i64::from(loc.line()) + CLEAR_MODP_MEMORY + TEST_MEMORY_OFFSET) }
    }
}

impl TestMem for ClearModp {
    #[track_caller]
    #[inline(always)]
    fn test_mem<const I: u32>(self, _t_location: ConstU32<I>) {}
}

impl Test for SecretI64 {
    #[track_caller]
    #[inline(always)]
    fn test(self) {
        self.reveal().test()
    }
}

impl TestMem for SecretI64 {
    #[track_caller]
    #[inline(always)]
    fn test_mem<const I: u32>(self, _t_location: ConstU32<I>) {}
}

impl Test for SecretModp {
    #[track_caller]
    #[inline(always)]
    fn test(self) {
        self.reveal().test()
    }
}

impl TestMem for SecretModp {
    #[track_caller]
    #[inline(always)]
    fn test_mem<const I: u32>(self, _t_location: ConstU32<I>) {}
}

impl Test for SecretBit {
    #[track_caller]
    #[inline(always)]
    fn test(self) {
        self.reveal().test()
    }
}

impl TestMem for SecretBit {
    #[track_caller]
    #[inline(always)]
    fn test_mem<const I: u32>(self, _t_location: ConstU32<I>) {}
}
