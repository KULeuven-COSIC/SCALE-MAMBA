
// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use crate::SecretModp;
use num_bigint::BigInt;

#[no_mangle]
extern "C" fn __bit() -> SecretModp {
    let value: BigInt = Default::default();
    SecretModp(value.into())
}
