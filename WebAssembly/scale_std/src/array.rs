use crate::iter::CompileTimeLengthIterator;
use crate::slice::Slice;
use core::ops::{Bound, RangeBounds};
use scale::alloc::GetAllocator;
use scale::{alloc::Allocate, LoadFromMem, Reveal, Stack, StackAddress, StoreInMem};

/// An array datastructure that allocates memory and never frees it
#[derive(Copy, Clone)]
pub struct Array<T, const N: u64> {
    // Act like we contain elements of type T
    phantom: core::marker::PhantomData<T>,
    // Base address of the array in memory
    base_address: u64,
}

impl<T: GetAllocator, const N: u64> Array<T, N> {
    #[inline(always)]
    pub fn uninitialized() -> Self {
        Self {
            phantom: core::marker::PhantomData,
            base_address: T::get_allocator().allocate(N),
        }
    }
}

pub struct ArrayAllocator<T: GetAllocator, const N: u64> {
    element_allocator: T::Allocator,
}

impl<T: GetAllocator, const N: u64> scale::alloc::Allocate for ArrayAllocator<T, N> {
    const N: u64 = N;
    #[inline(always)]
    unsafe fn free(&self, n: u64) {
        self.element_allocator.free(n * N);
    }

    #[inline(always)]
    fn allocate(&self, n: u64) -> u64 {
        self.element_allocator.allocate(n * N)
    }
}

impl<T: GetAllocator, const N: u64> GetAllocator for Array<T, N> {
    type Allocator = ArrayAllocator<T, N>;
    fn get_allocator() -> Self::Allocator {
        ArrayAllocator {
            element_allocator: T::get_allocator(),
        }
    }
}

impl<T: GetAllocator, const N: u64> LoadFromMem<i64> for Array<T, N> {
    fn load_from_mem(base_address: i64) -> Self {
        Self {
            base_address: base_address as u64,
            phantom: core::marker::PhantomData,
        }
    }
}

impl<T: GetAllocator + StoreInMem<i64>, const N: u64> Array<T, N> {
    #[inline(always)]
    pub fn fill(t: T) -> Self {
        let mut array = Self::uninitialized();
        for i in 0..N {
            array.set(i, &t);
        }
        array
    }

    #[inline(always)]
    pub fn from_iter(iter: impl IntoIterator<Item = T>) -> Self {
        let mut array = Self::uninitialized();
        for (i, val) in iter.into_iter().enumerate() {
            array.set(i as u64, &val);
        }
        array
    }
}

impl<T, const N: u64> Array<T, N> {
    pub fn slice(&self, range: impl RangeBounds<u64>) -> Slice<T> {
        let start = match range.start_bound() {
            Bound::Unbounded => 0,
            Bound::Included(n) => *n,
            Bound::Excluded(n) => n + 1,
        };
        Slice {
            phantom: core::marker::PhantomData,
            base_address: self.base_address + start,
            length: match range.end_bound() {
                Bound::Unbounded => N,
                Bound::Included(n) => n + 1,
                Bound::Excluded(n) => *n,
            } - start,
        }
    }
    /// Create an Array that points to the same data but has `S` fewer elements.
    pub fn static_slice_from_start<const S: u64>(&self) -> Array<T, { N - S }> {
        Array {
            phantom: self.phantom,
            base_address: self.base_address,
        }
    }
}

impl<T: GetAllocator + LoadFromMem<i64>, const N: u64> Array<T, N> {
    #[inline(always)]
    pub fn iter(
        &self,
    ) -> impl Iterator<Item = T> + CompileTimeLengthIterator<N> + DoubleEndedIterator + '_ {
        unsafe { crate::iter::Iter::new((0..N).map(move |i| self.get(i))) }
    }

    #[inline(always)]
    pub fn get(&self, i: u64) -> T {
        T::load_from_mem(self.addr(i))
    }
}

impl<T: GetAllocator, const N: u64> Array<T, N> {
    fn addr(&self, i: u64) -> i64 {
        (self.base_address + i * T::Allocator::N) as i64
    }
}

impl<T: GetAllocator + StoreInMem<i64>, const N: u64> Array<T, N> {
    #[inline(always)]
    pub fn set(&mut self, i: u64, val: &T) {
        unsafe { val.store_in_mem(self.addr(i)) }
    }
}

impl<T: LoadFromMem<i64> + GetAllocator + Stack + Reveal, const N: u64> Reveal for Array<T, N>
where
    <T as Reveal>::Output: GetAllocator + StoreInMem<i64>,
{
    type Output = Array<<T as Reveal>::Output, N>;
    #[inline(always)]
    fn reveal(&self) -> Self::Output {
        let mut array = Self::Output::uninitialized();
        for (i, val) in self.iter().enumerate() {
            array.set(i as u64, &val.reveal());
        }
        array
    }
}

impl<T: GetAllocator + StoreInMem<i64> + LoadFromMem<i64> + Stack, const N: u64> Stack
    for Array<T, N>
{
    #[inline(always)]
    unsafe fn push(array: &Self) {
        for val in array.iter() {
            T::push(&val);
        }
    }
    #[inline(always)]
    unsafe fn pop() -> Self {
        let mut array = Self::uninitialized();
        for i in (0..N).rev() {
            array.set(i, &T::pop())
        }
        array
    }

    #[inline(always)]
    unsafe fn peek(addr: StackAddress) -> Self {
        let mut array = Self::uninitialized();
        for i in 0..N {
            array.set(i, &T::peek(addr + i as _));
        }
        array
    }

    #[inline(always)]
    unsafe fn poke(addr: StackAddress, array: &Self) {
        for (i, val) in array.iter().enumerate() {
            T::poke(addr + i as _, &val);
        }
    }

    #[inline(always)]
    unsafe fn peek_from_top(offset: i64) -> Self {
        let mut array = Self::uninitialized();
        for i in 0..N {
            array.set(i, &T::peek_from_top(offset + i as i64));
        }
        array
    }

    #[inline(always)]
    unsafe fn poke_from_top(offset: i64, array: &Self) {
        for (i, val) in array.iter().enumerate() {
            T::poke_from_top(offset + i as i64, &val);
        }
    }

    #[inline(always)]
    fn get_stack_pointer() -> StackAddress {
        unimplemented!("this operation makes no sense");
    }
}
