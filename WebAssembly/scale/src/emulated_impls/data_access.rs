use crate::SecretModp;
use num_bigint::BigInt;

#[no_mangle]
extern "C" fn __bit() -> SecretModp {
    let value: BigInt = Default::default();
    SecretModp(value.into())
}
