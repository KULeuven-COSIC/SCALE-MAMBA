// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use crate::{RawSecretBit, SecretI64};

#[no_mangle]
extern "C" fn __eqzsint(value: SecretI64) -> RawSecretBit {
    RawSecretBit((value.0 == 0) as i64)
}

#[no_mangle]
extern "C" fn __ltzsint(value: SecretI64) -> RawSecretBit {
    RawSecretBit((value.0 < 0) as i64)
}
