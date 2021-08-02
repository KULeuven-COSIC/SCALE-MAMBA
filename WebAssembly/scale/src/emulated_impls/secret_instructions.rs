// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.


#[no_mangle]
extern "C" fn __ldsint(value: i32) -> crate::SecretI64 {
    crate::SecretI64(value.into())
}

#[no_mangle]
extern "C" fn __ldsbit(value: i32) -> crate::RawSecretBit {
    crate::RawSecretBit(value.into())
}

#[no_mangle]
extern "C" fn __addsint(left: crate::SecretI64, right: crate::SecretI64) -> crate::SecretI64 {
    crate::SecretI64(left.0 + right.0)
}
#[no_mangle]
extern "C" fn __addsintc(left: crate::SecretI64, right: i64) -> crate::SecretI64 {
    crate::SecretI64(left.0 + right)
}

#[no_mangle]
extern "C" fn __subsint(left: crate::SecretI64, right: crate::SecretI64) -> crate::SecretI64 {
    crate::SecretI64(left.0 - right.0)
}

#[no_mangle]
extern "C" fn __subsintc(left: crate::SecretI64, right: i64) -> crate::SecretI64 {
    crate::SecretI64(left.0 - right)
}

#[no_mangle]
extern "C" fn __subcints(left: i64, right: crate::SecretI64) -> crate::SecretI64 {
    crate::SecretI64(left - right.0)
}

#[no_mangle]
extern "C" fn __mulsint(left: crate::SecretI64, right: crate::SecretI64) -> crate::SecretI64 {
    crate::SecretI64(left.0 * right.0)
}

#[no_mangle]
extern "C" fn __mulsintc(left: crate::SecretI64, right: i64) -> crate::SecretI64 {
    crate::SecretI64(left.0 * right)
}

#[no_mangle]
extern "C" fn __divsint(left: crate::SecretI64, right: crate::SecretI64) -> crate::SecretI64 {
    crate::SecretI64(left.0 / right.0)
}
