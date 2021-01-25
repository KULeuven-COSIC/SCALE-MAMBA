#![no_std]
#![no_main]

use scale::*;

#[cfg_attr(not(feature = "emulate"), no_mangle)]
pub fn main() {
    let y = unsafe { __ldi(3) };
    let x = unsafe { __ldi(42) };
    let z = x + y;
    unsafe {
        __print_reg(z);
    }
}
