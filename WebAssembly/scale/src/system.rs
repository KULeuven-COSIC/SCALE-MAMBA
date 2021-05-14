// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use super::*;

/* This file contains all the simple "system style" commands */

// Re-initializes the specified timer
#[inline(always)]
pub fn start_clock<const ID: i32>(_: ConstI32<ID>) {
    unsafe { __start_clock(ID) }
}

// Stops the clock and prints the time
#[inline(always)]
pub fn stop_clock<const ID: i32>(_: ConstI32<ID>) {
    unsafe { __stop_clock(ID) }
}

// Requests a given minimum bit length of the prime
#[inline(always)]
pub fn require_bit_length<const BIT_LENGTH: u32>(_: ConstU32<BIT_LENGTH>) {
    unsafe { __reqbl(BIT_LENGTH) }
}

// Crash the system
#[inline(always)]
pub fn crash() {
    unsafe { __crash() }
}

// Restart the system
#[inline(always)]
pub fn restart() {
    unsafe { __restart() }
}

// Clear Memory
#[inline(always)]
pub fn clear_memory() {
    unsafe { __clear_memory() }
}

// Clear Registers
#[inline(always)]
pub fn clear_registers() {
    unsafe { __clear_registers() }
}

// GC call
#[inline(always)]
pub unsafe fn execute_garbled_circuit<const ID: u32>(_: ConstU32<ID>) {
    __gc(ID)
}

// LF call
#[inline(always)]
pub unsafe fn execute_local_function<const ID: u32>(_: ConstU32<ID>) {
    __lf(ID)
}

#[inline(always)]
pub fn reserve_i64_stack(n: u32) -> StackAddress {
    unsafe { __reserve_i64_stack(n) }
}

#[inline(always)]
pub fn reserve_secret_i64_stack(n: u32) -> StackAddress {
    unsafe { __reserve_secret_i64_stack(n) }
}

#[inline(always)]
pub fn reserve_clear_modp_stack(n: u32) -> StackAddress {
    unsafe { __reserve_clear_modp_stack(n) }
}

#[inline(always)]
pub fn reserve_secret_modp_stack(n: u32) -> StackAddress {
    unsafe { __reserve_secret_modp_stack(n) }
}

extern "C" {
    fn __reserve_i64_stack(n: u32) -> StackAddress;
    fn __reserve_secret_i64_stack(n: u32) -> StackAddress;
    fn __reserve_clear_modp_stack(n: u32) -> StackAddress;
    fn __reserve_secret_modp_stack(n: u32) -> StackAddress;
}
