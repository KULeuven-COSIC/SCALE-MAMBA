use crate::{RawSecretBit, SecretI64};

#[no_mangle]
extern "C" fn __neg(value: SecretI64) -> SecretI64 {
    SecretI64(-value.0)
}

#[no_mangle]
extern "C" fn __negb(value: RawSecretBit) -> RawSecretBit {
    RawSecretBit(1 - value.0)
}

#[no_mangle]
extern "C" fn __sintbit(s: crate::SecretI64, b: crate::RawSecretBit, pos: u32) -> crate::SecretI64 {
    crate::SecretI64(s.0 | (b.0 << pos))
}

#[no_mangle]
extern "C" fn __bitsint(s: crate::SecretI64, pos: u32) -> crate::RawSecretBit {
    crate::RawSecretBit((s.0 >> pos) & 1)
}

#[no_mangle]
extern "C" fn __invsint(s: crate::SecretI64) -> crate::SecretI64 {
    crate::SecretI64(!(s.0))
}

#[no_mangle]
extern "C" fn __sand(s: crate::SecretI64, b: crate::RawSecretBit) -> crate::SecretI64 {
    crate::SecretI64(s.0 * b.0)
}
