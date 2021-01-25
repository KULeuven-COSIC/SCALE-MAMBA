#![no_std]
#![no_main]

use scale::*;

#[cfg_attr(not(feature = "emulate"), no_mangle)]
pub fn main() {
    let y = black_box(5);
    for i in 0..y {
        unsafe { __print_reg(__convint(i)) };
    }
    unsafe { __print_reg(__ldi(42)) };
}
