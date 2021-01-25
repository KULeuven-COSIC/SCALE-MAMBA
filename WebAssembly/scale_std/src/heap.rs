use scale::alloc::GetAllocator;
use scale::{alloc::Allocate, LoadFromMem, StoreInMem};

/// A smart pointer datastructure that allocates memory but never frees it
pub struct Box<T> {
    // Act like we contain an element of type T
    phantom: core::marker::PhantomData<T>,
    address: u64,
}

impl<T> Box<T> {
    pub unsafe fn offset(&self, offset: u64) -> Self {
        Self {
            address: self.address + offset,
            ..*self
        }
    }
}

impl<T: GetAllocator> GetAllocator for Box<T> {
    type Allocator = T::Allocator;
    fn get_allocator() -> Self::Allocator {
        T::get_allocator()
    }
}

impl<T: StoreInMem<i64> + GetAllocator> Box<T> {
    /// Allocate a single element without a value
    pub fn uninitialized() -> Self {
        Self {
            phantom: core::marker::PhantomData,
            address: T::get_allocator().allocate(1),
        }
    }

    pub fn new(t: T) -> Self {
        let mut this = Self::uninitialized();
        this.set(t);
        this
    }
}

impl<T: StoreInMem<i64>> Box<T> {
    pub fn set(&mut self, val: T) {
        unsafe {
            val.store_in_mem(self.address as i64);
        }
    }
}

impl<T: LoadFromMem<i64>> Box<T> {
    pub fn get(&self) -> T {
        T::load_from_mem(self.address as i64)
    }
}
