// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boï¿½tie, Paris, France.

use crate::ClearModp;
use crate::SecretModp;
use crate::{
    __addc, __addm, __adds, __divc, 
    __modc, __mulc, __mulm, 
    __ldmci, __ldmsi, 
    __stmci, __stmsi, __stminti, 
    __subc, __subml, __submr,
};
use crate::BigInt;

use crate::testing_emulated::P;


// Doing nothing, at runtime as we test these instructions in a different
// way in the test routines [at present]
#[no_mangle]
extern "C" fn __maddc(dest: i64, a: i64, b: i64, len: i64) {
    for k in 0..len {
        let c1 = unsafe { __ldmci(a + k) };
        let c2 = unsafe { __ldmci(b + k) };
        let c3 = unsafe { __addc(c1, c2) };

        unsafe { __stmci(c3, dest + k) }
    }
}

#[no_mangle]
extern "C" fn __madds(dest: i64, a: i64, b: i64, len: i64) {
    for k in 0..len {
        let s1 = unsafe { __ldmsi(a + k) };
        let s2 = unsafe { __ldmsi(b + k) };
        let s3 = unsafe { __adds(s1, s2) };

        unsafe { __stmsi(s3, dest + k) }
    }
}

#[no_mangle]
extern "C" fn __maddm(dest: i64, a: i64, b: i64, len: i64) {
    for k in 0..len {
        let s1 = unsafe { __ldmsi(a + k) };
        let c2 = unsafe { __ldmci(b + k) };
        let resp = unsafe { __addm(s1, c2) };

        unsafe { __stmsi(resp, dest + k) }
    }
}

#[no_mangle]
extern "C" fn __msubc(dest: i64, a: i64, b: i64, len: i64) {
    for k in 0..len {
        let c1 = unsafe { __ldmci(a + k) };
        let c2 = unsafe { __ldmci(b + k) };
        let c3 = unsafe { __subc(c1, c2) };

        unsafe { __stmci(c3, dest + k) }
    }
}

#[no_mangle]
extern "C" fn __msubs(dest: i64, a: i64, b: i64, len: i64) {
    for k in 0..len {
        let c1 = unsafe { __ldmsi(a + k) };
        let c2 = unsafe { __ldmsi(b + k) };
        let c3 = c1 - c2;

        unsafe { __stmsi(c3, dest + k) }
    }
}

#[no_mangle]
extern "C" fn __msubml(dest: i64, a: i64, b: i64, len: i64) {
    for k in 0..len {
        let s1 = unsafe { __ldmsi(a + k) };
        let c2 = unsafe { __ldmci(b + k) };
        let resp = unsafe { __subml(s1, c2) };

        unsafe { __stmsi(resp, dest + k) }
    }
}

#[no_mangle]
extern "C" fn __msubmr(dest: i64, a: i64, b: i64, len: i64) {
    for k in 0..len {
        let c1 = unsafe { __ldmci(a + k) };
        let s2 = unsafe { __ldmsi(b + k) };
        let resp = unsafe { __submr(c1, s2) };

        unsafe { __stmsi(resp, dest + k) }
    }
}

#[no_mangle]
extern "C" fn __mmulc(dest: i64, a: i64, b: i64, len: i64) {
    for k in 0..len {
        let c1 = unsafe { __ldmci(a + k) };
        let c2 = unsafe { __ldmci(b + k) };
        let c3 = unsafe { __mulc(c1, c2) };

        unsafe { __stmci(c3, dest + k) }
    }
}

#[no_mangle]
extern "C" fn __mmulm(dest: i64, a: i64, b: i64, len: i64) {
    for k in 0..len {
        let s1 = unsafe { __ldmsi(a + k) };
        let c2 = unsafe { __ldmci(b + k) };
        let resp = unsafe { __mulm(s1, c2) };

        unsafe { __stmsi(resp, dest + k) }
    }
}

#[no_mangle]
extern "C" fn __mdivc(dest: i64, a: i64, b: i64, len: i64) {
    for k in 0..len {
        let c1 = unsafe { __ldmci(a + k) };
        let c2 = unsafe { __ldmci(b + k) };
        let c3 = unsafe { __divc(c1, c2) };

        unsafe { __stmci(c3, dest + k) }
    }
}

#[no_mangle]
extern "C" fn __mmodc(dest: i64, a: i64, b: i64, len: i64) {
    for k in 0..len {
        let c1 = unsafe { __ldmci(a + k) };
        let c2 = unsafe { __ldmci(b + k) };
        let c3 = unsafe { __modc(c1, c2) };

        unsafe { __stmci(c3, dest + k) }
    }
}

#[no_mangle]
extern "C" fn __mrevc(dest: i64, a: i64, len: i64) 
{
  let mut store: Vec<ClearModp> = vec![ClearModp::from(0); len as usize];
  for k in 0..len {
      store[k as usize] = unsafe { __ldmci( len - 1 - k + a ) };
  }
  for k in 0..len {
      unsafe { __stmci(store[k as usize], dest+k) };
  }
}


#[no_mangle]
extern "C" fn __mrevs(dest: i64, a: i64, len: i64) 
{
  let mut store: Vec<SecretModp> = vec![SecretModp::from(0); len as usize];
  for k in 0..len {
      store[k as usize] = unsafe { __ldmsi( len - 1 - k + a ) };
  }
  for k in 0..len {
      unsafe { __stmsi(store[k as usize], dest+k) };
  }
}

#[no_mangle]
extern "C" fn __mevalcc(addr: i64, len: i64, val: ClearModp) -> ClearModp {
    let mut sum = ClearModp::from(0);
    let mut pow = ClearModp::from(1);
    for t in 0..len {
        let slice_elt = unsafe { __ldmci(addr + t) };
        sum += slice_elt * pow;
        pow = pow * val;
    }
    sum
}

#[no_mangle]
extern "C" fn __mevalsc(addr: i64, len: i64, val: ClearModp) -> SecretModp {
    let mut sum = SecretModp::from(0);
    let mut pow = ClearModp::from(1);
    for t in 0..len {
        let slice_elt = unsafe { __ldmsi(addr + t) };
        sum += slice_elt * pow;
        pow = pow * val;
    }
    sum
}

#[no_mangle]
extern "C" fn __mbitdecc(dest: i64, val: ClearModp, len: i64) {
    let mut num : BigInt = val.into();
    for k in 0..len {
        let bit = num.clone() % BigInt::from(2);
        num = (num - bit.clone())/BigInt::from(2);
        let cbit=ClearModp::from(bit);
        unsafe { __stmci(cbit, dest+k) };
    }
}

#[no_mangle]
extern "C" fn __mbitdeccs(dest: i64, val: ClearModp, len: i64) {
    let mut num : BigInt = val.into();
    if num > (&*P / 2) {
        num -= &*P;
    }

    for k in 0..len {
        let bit = num.clone() % BigInt::from(2);
        num = (num - bit.clone())/BigInt::from(2);
        let cbit=ClearModp::from(bit);
        unsafe { __stmci(cbit, dest+k) };
    }
}

#[no_mangle]
extern "C" fn __mbitdecint(dest: i64, val: i64, len: i64) {
    let mut num = val;
    for k in 0..len {
        let bit = num % 2;
        num = (num - bit)/2;
        unsafe { __stminti(bit, dest+k) };
    }
}
