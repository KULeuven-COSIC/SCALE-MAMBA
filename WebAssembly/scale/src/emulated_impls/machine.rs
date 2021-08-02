// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use core::mem;
use crate::ClearModp;
use crate::SecretI64;
use crate::ConstI32;
use crate::Stack;
use crate::{
    __popint, __popsint,
};

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
extern "C" fn __gc(a: u32) {
    let mut done=false;
    if a == 120 {
        let y = unsafe { __popsint() };
        let x = unsafe { __popsint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let yfloat : f64 = unsafe { mem::transmute(y) };
        let result = xfloat + yfloat;
        let ans : SecretI64 = unsafe { mem::transmute(result) } ;
        unsafe { SecretI64::push(&ans) };
        done=true;
    }
    else if a == 121 {
        let y = unsafe { __popsint() };
        let x = unsafe { __popsint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let yfloat : f64 = unsafe { mem::transmute(y) };
        let result = xfloat * yfloat;
        let ans : SecretI64 = unsafe { mem::transmute(result) } ;
        unsafe { SecretI64::push(&ans) };
        done=true;
    }
    else if a == 122 {
        let y = unsafe { __popsint() };
        let x = unsafe { __popsint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let yfloat : f64 = unsafe { mem::transmute(y) };
        let result = xfloat / yfloat;
        let ans : SecretI64 = unsafe { mem::transmute(result) } ;
        unsafe { SecretI64::push(&ans) };
        done=true;
    }
    else if a == 123 {
        let x = unsafe { __popsint() };
        let y = unsafe { __popsint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let yfloat : f64 = unsafe { mem::transmute(y) };
        let result = (yfloat == xfloat) as i64;
        let ans : SecretI64 = SecretI64::from(result);
        unsafe { SecretI64::push(&ans) };
        done=true;
    }
    else if a == 124 {
        let x = unsafe { __popsint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let ans : SecretI64 = SecretI64::from(xfloat as i64);
        unsafe { SecretI64::push(&ans) };
        done=true;
    }
    else if a == 125 {
        let x = unsafe { __popsint() };
        let result : f64 = x.0 as f64;
        let ans1 : i64 = unsafe { mem::transmute(result) } ;
        let ans : SecretI64 = SecretI64::from(ans1);
        unsafe { SecretI64::push(&ans) };
        done=true;
    }
    else if a == 126 {
        let x = unsafe { __popsint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let result = xfloat.sqrt();
        let ans : SecretI64 = unsafe { mem::transmute(result) } ;
        unsafe { SecretI64::push(&ans) };
        done=true;
    }
    else if a == 127 {
        let x = unsafe { __popsint() };
        let y = unsafe { __popsint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let yfloat : f64 = unsafe { mem::transmute(y) };
        let result = (yfloat < xfloat) as i64;
        let ans : SecretI64 = SecretI64::from(result);
        unsafe { SecretI64::push(&ans) };
        done=true;
    }
    else if a == 128 {
        let x = unsafe { __popsint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let result = xfloat.floor();
        let ans : SecretI64 = unsafe { mem::transmute(result) } ;
        unsafe { SecretI64::push(&ans) };
        done=true;
    }
    else if a == 129 {
        let x = unsafe { __popsint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let result = xfloat.ceil();
        let ans : SecretI64 = unsafe { mem::transmute(result) } ;
        unsafe { SecretI64::push(&ans) };
        done=true;
    }

    // Warning if GC is really fake
    if !done {
       println!("Called GC routine number {}: This will not emulate correctly",a);
    }
}

#[no_mangle]
extern "C" fn __lf(a: u32) {
    let mut done=false;
    if a == 500 {
        let f = unsafe { __popint() } as f64;
        let _k = unsafe { __popint() };
        let x = unsafe { __popint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let xfixed = (xfloat*f.exp2()) as i64;
        let result = ClearModp::from(xfixed);
        unsafe { ClearModp::push(&result) };
        done=true;
    }
    else if a == 120 {
        let y = unsafe { __popint() };
        let x = unsafe { __popint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let yfloat : f64 = unsafe { mem::transmute(y) };
        let result = xfloat + yfloat;
        let ans : i64 = unsafe { mem::transmute(result) } ;
        unsafe { i64::push(&ans) };
        done=true;
    }
    else if a == 121 {
        let y = unsafe { __popint() };
        let x = unsafe { __popint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let yfloat : f64 = unsafe { mem::transmute(y) };
        let result = xfloat * yfloat;
        let ans : i64 = unsafe { mem::transmute(result) } ;
        unsafe { i64::push(&ans) };
        done=true;
    }
    else if a == 122 {
        let y = unsafe { __popint() };
        let x = unsafe { __popint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let yfloat : f64 = unsafe { mem::transmute(y) };
        let result = xfloat / yfloat;
        let ans : i64 = unsafe { mem::transmute(result) } ;
        unsafe { i64::push(&ans) };
        done=true;
    }
    else if a == 123 {
        let x = unsafe { __popint() };
        let y = unsafe { __popint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let yfloat : f64 = unsafe { mem::transmute(y) };
        let ans = (yfloat == xfloat) as i64;
        unsafe { i64::push(&ans) };
        done=true;
    }
    else if a == 124 {
        let x = unsafe { __popint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let ans : i64 = xfloat as i64;
        unsafe { i64::push(&ans) };
        done=true;
    }
    else if a == 125 {
        let x = unsafe { __popint() };
        let result : f64 = x as f64;
        let ans : i64 = unsafe { mem::transmute(result) } ;
        unsafe { i64::push(&ans) };
        done=true;
    }
    else if a == 126 {
        let x = unsafe { __popint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let result = xfloat.sqrt();
        let ans : i64 = unsafe { mem::transmute(result) } ;
        unsafe { i64::push(&ans) };
        done=true;
    }
    else if a == 127 {
        let x = unsafe { __popint() };
        let y = unsafe { __popint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let yfloat : f64 = unsafe { mem::transmute(y) };
        let ans = (yfloat < xfloat) as i64;
        unsafe { i64::push(&ans) };
        done=true;
    }
    else if a == 128 {
        let x = unsafe { __popint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let result = xfloat.floor();
        let ans : i64 = unsafe { mem::transmute(result) } ;
        unsafe { i64::push(&ans) };
        done=true;
    }
    else if a == 129 {
        let x = unsafe { __popint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let result = xfloat.ceil();
        let ans : i64 = unsafe { mem::transmute(result) } ;
        unsafe { i64::push(&ans) };
        done=true;
    }
    else if a == 200 {
        let x = unsafe { __popint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let result = xfloat.acos();
        let ans : i64 = unsafe { mem::transmute(result) } ;
        unsafe { i64::push(&ans) };
        done=true;
    }
    else if a == 201 {
        let x = unsafe { __popint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let result = xfloat.asin();
        let ans : i64 = unsafe { mem::transmute(result) } ;
        unsafe { i64::push(&ans) };
        done=true;
    }
    else if a == 202 {
        let x = unsafe { __popint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let result = xfloat.atan();
        let ans : i64 = unsafe { mem::transmute(result) } ;
        unsafe { i64::push(&ans) };
        done=true;
    }
    else if a == 203 {
        let x = unsafe { __popint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let result = xfloat.cos();
        let ans : i64 = unsafe { mem::transmute(result) } ;
        unsafe { i64::push(&ans) };
        done=true;
    }
    else if a == 204 {
        let x = unsafe { __popint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let result = xfloat.cosh();
        let ans : i64 = unsafe { mem::transmute(result) } ;
        unsafe { i64::push(&ans) };
        done=true;
    }
    else if a == 205 {
        let x = unsafe { __popint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let result = xfloat.sin();
        let ans : i64 = unsafe { mem::transmute(result) } ;
        unsafe { i64::push(&ans) };
        done=true;
    }
    else if a == 206 {
        let x = unsafe { __popint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let result = xfloat.sinh();
        let ans : i64 = unsafe { mem::transmute(result) } ;
        unsafe { i64::push(&ans) };
        done=true;
    }
    else if a == 207 {
        let x = unsafe { __popint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let result = xfloat.tanh();
        let ans : i64 = unsafe { mem::transmute(result) } ;
        unsafe { i64::push(&ans) };
        done=true;
    }
    else if a == 208 {
        let x = unsafe { __popint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let result = xfloat.exp();
        let ans : i64 = unsafe { mem::transmute(result) } ;
        unsafe { i64::push(&ans) };
        done=true;
    }
    else if a == 209 {
        let x = unsafe { __popint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let result = xfloat.ln();
        let ans : i64 = unsafe { mem::transmute(result) } ;
        unsafe { i64::push(&ans) };
        done=true;
    }
    else if a == 210 {
        let x = unsafe { __popint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let result = xfloat.log10();
        let ans : i64 = unsafe { mem::transmute(result) } ;
        unsafe { i64::push(&ans) };
        done=true;
    }
    else if a == 211 {
        let x = unsafe { __popint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let result = xfloat.abs();
        let ans : i64 = unsafe { mem::transmute(result) } ;
        unsafe { i64::push(&ans) };
        done=true;
    }
    else if a == 502 {
        // Note this routine is not really tested yet
        // Will certainly not work for big v_size and p_size
        let _p_size = unsafe { __popint() };
        let v_size = unsafe { __popint() };
        let x = unsafe { __popint() };
        let xfloat : f64 = unsafe { mem::transmute(x) };
        let mut err=ClearModp::from(0);
        let mut s=ClearModp::from(0);
        let mut z=ClearModp::from(0);
        let mut p=ClearModp::from(0);
        let mut v=ClearModp::from(0);
        if xfloat < 0.0 { 
            s = ClearModp::from(1); 
        }
        if xfloat.is_nan() {
            err=ClearModp::from(1);
        }
        else if xfloat ==0.0 { 
            z = ClearModp::from(1); 
        }
        else {
            let mut pp=(xfloat.abs().log2()-v_size as f64+1.0).floor();
            let mut vv = xfloat.abs()*((-pp).exp2());
            let ptwo=(v_size as f64).exp2();
            while vv>=ptwo {
                vv=vv/2.0;
                pp=pp+1.0;
            }
            v = ClearModp::from(vv as i64);
            p = ClearModp::from(pp as i64);
        }

        unsafe { ClearModp::push(&err) };
        unsafe { ClearModp::push(&s) };
        unsafe { ClearModp::push(&z) };
        unsafe { ClearModp::push(&p) };
        unsafe { ClearModp::push(&v) };
        done=true;
    }

    // Add in fake push's to avoid the emulator crashing
    if a == 503 {
        unsafe { i64::push(&i64::from(ConstI32::<0>)) };
    }

    // Warning if LF is really fake
    if !done {
       println!("Called LF routine number {}: This will not emulate correctly",a);
    }
}

