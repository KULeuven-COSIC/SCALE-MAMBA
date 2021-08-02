// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use super::*;

/* This file contains all the IO class commands
 *
 * Input/Output for i64, SecretModp and ClearModp are
 * defined in their respective classes
 *
 */

#[inline(always)]
pub fn open_channel(channel: i64) -> i64 {
    unsafe { __open_channel(channel) }
}

#[inline(always)]
pub fn close_channel(channel: i64) {
    unsafe {
        __close_channel(channel);
    }
}

/*

pub fn output_shares(XXXX) { }
pub fn input_shares(XXXX) { }

*/
