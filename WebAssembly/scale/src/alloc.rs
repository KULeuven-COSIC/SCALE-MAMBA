// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use crate::{ClearModp, SecretI64, SecretModp};
use crate::{__deletec, __deleteint, __deletes, __deletesint, __newc, __newint, __news, __newsint};
pub struct Allocator<T> {
    // Act like we contain `T`
    phantom: core::marker::PhantomData<T>,
}

impl<T> Allocator<T> {
    const DEFAULT: Self = Self::new();
    const fn new() -> Self {
        Self {
            phantom: core::marker::PhantomData,
        }
    }
}

pub trait Allocate {
    /// This constant states how many elements will get allocated for each element
    /// given to `free`.
    const N: u64;
    unsafe fn free(&self, n: u64);
    fn allocate(&self, n: u64) -> u64;
}

impl Allocate for &'static Allocator<ClearModp> {
    const N: u64 = 1;

    #[inline(always)]
    unsafe fn free(&self, addr: u64) {
        __deletec(addr as i64);
    }

    #[inline(always)]
    fn allocate(&self, n: u64) -> u64 {
        unsafe { __newc(n as i64) as u64 }
    }
}

impl Allocate for &'static Allocator<SecretModp> {
    const N: u64 = 1;

    #[inline(always)]
    unsafe fn free(&self, addr: u64) {
        __deletes(addr as i64);
    }

    #[inline(always)]
    fn allocate(&self, n: u64) -> u64 {
        unsafe { __news(n as i64) as u64 }
    }
}

impl Allocate for &'static Allocator<i64> {
    const N: u64 = 1;

    #[inline(always)]
    unsafe fn free(&self, addr: u64) {
        __deleteint(addr as i64);
    }

    #[inline(always)]
    fn allocate(&self, n: u64) -> u64 {
        unsafe { __newint(n as i64) as u64 }
    }
}

impl Allocate for &'static Allocator<SecretI64> {
    const N: u64 = 1;

    #[inline(always)]
    unsafe fn free(&self, addr: u64) {
        __deletesint(addr as i64);
    }

    #[inline(always)]
    fn allocate(&self, n: u64) -> u64 {
        unsafe { __newsint(n as i64) as u64 }
    }
}

pub trait GetAllocator: Sized + 'static {
    type Allocator: Allocate;
    fn get_allocator() -> Self::Allocator;

    fn size_type() -> u64; 
}

impl GetAllocator for SecretI64 {
    type Allocator = &'static Allocator<Self>;
    fn get_allocator() -> &'static Allocator<Self> {
        &Allocator::DEFAULT
    }
    fn size_type() -> u64 { 1 }
}

impl GetAllocator for i64 {
    type Allocator = &'static Allocator<Self>;
    fn get_allocator() -> &'static Allocator<Self> {
        &Allocator::DEFAULT
    }
    fn size_type() -> u64 { 1 }
}

impl GetAllocator for ClearModp {
    type Allocator = &'static Allocator<Self>;
    fn get_allocator() -> &'static Allocator<Self> {
        &Allocator::DEFAULT
    }
    fn size_type() -> u64 { 1 }
}

impl GetAllocator for SecretModp {
    type Allocator = &'static Allocator<Self>;
    fn get_allocator() -> &'static Allocator<Self> {
        &Allocator::DEFAULT
    }
    fn size_type() -> u64 { 1 }
}
