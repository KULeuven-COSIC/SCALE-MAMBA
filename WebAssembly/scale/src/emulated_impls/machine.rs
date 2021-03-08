
// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use crate::ClearModp;
use crate::ConstI32;
use crate::Stack;

#[no_mangle]
extern "C" fn __crash() -> ! {
    println!("CRASH requested");
    std::process::exit(-1);
}

#[no_mangle]
extern "C" fn __restart() -> ! {
    println!("RESTART requested");
    std::process::exit(0);
}

#[no_mangle]
extern "C" fn __reqbl(_: u32) {}

#[no_mangle]
extern "C" fn __start_clock(_: u32) {}

#[no_mangle]
extern "C" fn __stop_clock(_: u32) {}

#[no_mangle]
extern "C" fn __gc(_: u32) {}

#[no_mangle]
extern "C" fn __lf(a: u32) {
    // Add in fake push's to avoid the emulator crashing
    if a == 500 {
        unsafe { ClearModp::push(&ClearModp::from(ConstI32::<0>)) };
    }
    if a == 502 {
        unsafe { ClearModp::push(&ClearModp::from(ConstI32::<0>)) };
        unsafe { ClearModp::push(&ClearModp::from(ConstI32::<0>)) };
        unsafe { ClearModp::push(&ClearModp::from(ConstI32::<0>)) };
        unsafe { ClearModp::push(&ClearModp::from(ConstI32::<0>)) };
        unsafe { ClearModp::push(&ClearModp::from(ConstI32::<0>)) };
    }
    if a == 503 {
        unsafe { i64::push(&i64::from(ConstI32::<0>)) };
    }
}
