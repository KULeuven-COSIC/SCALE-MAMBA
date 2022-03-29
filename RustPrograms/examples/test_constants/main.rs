#![no_std]
#![no_main]

#[scale::main(KAPPA = 40)]
#[inline(never)]
fn main() {
    // A positive constant at the boundary of i32 range
    (i32::MAX as i64).test();

    // A positive constant just outside the boundary of i32 range
    (i32::MAX as i64 + 1).test();

    // A positive constant at the boundary of i64 range
    i64::MAX.test();

    // A negative constant at the boundary of i32 range
    (i32::MIN as i64).test();

    // A positive constant just outside the boundary of i32 range
    (i32::MIN as i64 - 1).test();

    // A negative constant at the boundary of i64 range
    i64::MIN.test();

    // A negative constant out of i32 range with `i >> 32 != i / 2_i64.pow(32)`.
    (-0x0000_0001_0000_0001_i64).test();

    // A negative constant out of i32 range with `i >> 32 == i / 2_i64.pow(32)`.
    (-0x0000_0001_0000_0000_i64).test();

    // A negative constant out of i32 range with `lower < -2^31`.
    (-0x0000_0000_ffff_ffff_i64).test();

    // A positive constant out of i32 range with `lower >= 2^31`.
    0x0000_0000_ffff_ffff_i64.test();
}
