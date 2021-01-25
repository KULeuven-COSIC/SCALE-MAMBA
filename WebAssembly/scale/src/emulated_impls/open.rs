#[no_mangle]
extern "C" fn __opensint(value: crate::SecretI64) -> i64 {
    value.0
}

#[no_mangle]
extern "C" fn __opensbit(value: crate::RawSecretBit) -> bool {
    assert!(value.0 <= 1);
    value.0 == 1
}
