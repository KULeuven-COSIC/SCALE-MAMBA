#![feature(const_generics, const_evaluatable_checked)]
#![no_std]
#![allow(incomplete_features)]

//! Some convenience datastructures and functions that are not necessary
//! for working with scasm, but make life easier and require no unsafe
//! code on the user side.

pub mod array;
pub mod bit_protocols;
pub mod circuits;
pub mod heap;
pub mod integer;
pub mod iter;
pub mod local_functions;
pub mod matrix;
pub mod secret_ieee;
pub mod slice;

pub use heap::Box;
