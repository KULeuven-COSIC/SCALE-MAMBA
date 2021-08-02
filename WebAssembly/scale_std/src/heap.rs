// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use scale::alloc::GetAllocator;
use scale::{alloc::Allocate, LoadFromMem, StoreInMem};

/// A smart pointer datastructure that allocates memory and deallocates when goes out of scope
pub struct Box<T>
where
    T: GetAllocator,
{
    // Act like we contain an element of type T
    phantom: core::marker::PhantomData<T>,
    pub(crate) keep_alive: bool,
    pub(crate) address: u64,
}

impl<T> Box<T>
where
    T: GetAllocator,
{
    pub unsafe fn offset(&self, offset: u64) -> Self {
        Self {
            address: self.address + offset,
            keep_alive: self.keep_alive,
            ..*self
        }
    }
}

impl<T: GetAllocator> GetAllocator for Box<T> {
    type Allocator = T::Allocator;
    fn get_allocator() -> Self::Allocator {
        T::get_allocator()
    }
    fn size_type() -> u64 {
        T::size_type()
    }
}

impl<T: StoreInMem<i64> + GetAllocator> Box<T> {
    pub fn new(t: T) -> Self {
        let mut this = Self::uninitialized();
        this.set(t);
        this
    }
}

impl<T: GetAllocator> Box<T> {
    /// Allocate a single element without a value
    pub fn uninitialized() -> Self {
        Self {
            phantom: core::marker::PhantomData,
            keep_alive: false,
            address: T::get_allocator().allocate(T::size_type()),
        }
    }

    /// Allocate a multiple elements without a value
    pub fn uninitialized_multiple(length: u64) -> Self {
        Self {
            phantom: core::marker::PhantomData,
            keep_alive: false,
            address: T::get_allocator().allocate(length * T::size_type()),
        }
    }

    // Use with caution
    pub(crate) unsafe fn load_from_addr(address: u64) -> Self {
        Self {
            phantom: core::marker::PhantomData,
            keep_alive: true,
            address,
        }
    }

    #[allow(dead_code)]
    pub(crate) fn keep_alive(&mut self) {
        self.keep_alive = true;
    }
}

impl<T: StoreInMem<i64> + GetAllocator> Box<T> {
    pub fn set(&mut self, val: T) {
        unsafe {
            val.store_in_mem(self.address as i64);
        }
    }
}

impl<T: LoadFromMem<i64> + GetAllocator + Clone> Box<T> {
    pub fn get(&self) -> T {
        T::load_from_mem(self.address as i64).clone()
    }
}

impl<T: GetAllocator> Drop for Box<T> {
    fn drop(&mut self) {
        if !self.keep_alive {
            unsafe {
                Self::get_allocator().free(self.address);
            }
        }
    }
}
