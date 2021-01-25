#![no_std]
#![no_main]

use scale::*;

#[cfg_attr(not(feature = "emulate"), no_mangle)]
pub fn main() {
    let a = unsafe { __ldsi(42) };
    let a = unsafe { __convsintsreg(a) };
    let b = unsafe { __private_input(1, 20) };
    unsafe { __private_output(b, 2, 69) };
    let b = unsafe { __convsintsreg(b) };
    let mut d = b;
    let n = unsafe { __ldsi(3) };
    let n = unsafe { __convsintsreg(n) };
    while unsafe { __opensbit(__ltzsint(__subsint(d, n))) } {
        let e = unsafe { __convsregsint(d) };
        unsafe { __private_output(e, 2, 420) };
        d += a;
    }
}
