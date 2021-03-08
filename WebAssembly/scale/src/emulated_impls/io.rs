
// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

/* DO WE NEED THIS AT ALL? NPS
 *
use crate::{ClearModp, SecretModp};
use num_bigint::BigInt;

#[no_mangle]
extern "C" fn __output_clear(value: ClearModp, channel: i32) {
    let value: BigInt = value.into();
    println!("output_clear[{}]: {}", channel, value);
}
/*
#[no_mangle]
extern "C" fn __input_clear(channel: i32) -> ClearModp {}
*/
#[no_mangle]
extern "C" fn __private_input(player: u32, channel: u32) -> SecretModp {
    println!("private_input[{}, player {}]", channel, player);
    BigInt::from(((player as i64) << 32) + channel as i64).into()
}
#[no_mangle]
extern "C" fn __private_output(value: SecretModp, player: u32, channel: u32) {
    let value: BigInt = value.into();
    println!("private_output[{}, player {}]: {}", channel, player, value);
}
/*
#[no_mangle]
extern "C" fn __output_int(value: i64, channel: i32) {}
#[no_mangle]
extern "C" fn __input_int(channel: i32) -> i64 {}
#[no_mangle]
extern "C" fn __open_channel(channel: i32) -> i64 {}
#[no_mangle]
extern "C" fn __close_channel(dest: i32) {}
*/

*/
