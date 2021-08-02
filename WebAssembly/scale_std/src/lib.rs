// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![feature(
    const_generics,
    const_evaluatable_checked,
    exclusive_range_pattern,
    half_open_range_patterns,
    const_panic
)]
#![no_std]
#![allow(incomplete_features)]

//! Some convenience datastructures and functions that are not necessary
//! for working with scasm, but make life easier and require no unsafe
//! code on the user side.

pub mod array;
pub mod bit_protocols;
pub mod circuits;
pub mod fixed_point;
pub mod float_subroutines;
pub mod floating_point;
pub mod guard;
pub mod heap;
pub mod ieee;
pub mod integer;
pub mod iter;
pub mod local_functions;
pub mod math;
pub mod math_generic;
pub mod matrix;
pub mod oram;
pub mod slice;
pub mod sqrt;

pub use heap::Box;
