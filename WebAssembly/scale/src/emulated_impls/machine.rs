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
extern "C" fn __lf(_: u32) {}
