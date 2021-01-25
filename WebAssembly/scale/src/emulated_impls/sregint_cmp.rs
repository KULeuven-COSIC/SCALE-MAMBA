use crate::{RawSecretBit, SecretI64};

#[no_mangle]
extern "C" fn __eqzsint(value: SecretI64) -> RawSecretBit {
    RawSecretBit((value.0 == 0) as i64)
}

#[no_mangle]
extern "C" fn __ltzsint(value: SecretI64) -> RawSecretBit {
    RawSecretBit((value.0 < 0) as i64)
}
