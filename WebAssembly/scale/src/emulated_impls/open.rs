// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#[no_mangle]
extern "C" fn __opensint(value: crate::SecretI64) -> i64 {
    value.0
}

#[no_mangle]
extern "C" fn __opensbit(value: crate::RawSecretBit) -> bool {
    assert!(value.0 <= 1);
    value.0 == 1
}
