use core::ops::{Bound, RangeBounds};
use scale::alloc::GetAllocator;
use scale::{alloc::Allocate, LoadFromMem, Reveal, StoreInMem};

/// An array datastructure that allocates memory and never frees it
pub struct Slice<T> {
    // Act like we contain elements of type T
    pub(crate) phantom: core::marker::PhantomData<T>,
    // Base address of the slice in memory
    pub(crate) base_address: u64,
    // Length of the slice
    pub(crate) length: u64,
}

impl<T: GetAllocator> Slice<T> {
    #[inline(always)]
    pub fn uninitialized(length: u64) -> Self {
        Self {
            phantom: core::marker::PhantomData,
            base_address: T::get_allocator().allocate(length),
            length,
        }
    }
}

impl<T: GetAllocator + StoreInMem<i64>> Slice<T> {
    #[inline(always)]
    pub fn from_iter(iter: impl IntoIterator<Item = T> + ExactSizeIterator) -> Self {
        let mut array = Self::uninitialized(iter.len() as u64);
        for (i, val) in iter.into_iter().enumerate() {
            array.set(i as u64, &val);
        }
        array
    }
}

impl<T: GetAllocator> Slice<T> {
    fn addr(&self, i: u64) -> i64 {
        (self.base_address + i * T::Allocator::N) as i64
    }
}

impl<T: GetAllocator + LoadFromMem<i64>> Slice<T> {
    #[inline(always)]
    pub fn iter(&self) -> impl Iterator<Item = T> + '_ {
        (0..self.length).map(move |i| self.get(i))
    }

    #[inline(always)]
    pub fn get(&self, i: u64) -> T {
        T::load_from_mem(self.addr(i))
    }
}

impl<T: GetAllocator + StoreInMem<i64>> Slice<T> {
    #[inline(always)]
    pub fn set(&mut self, i: u64, val: &T) {
        unsafe { val.store_in_mem(self.addr(i)) }
    }
}

impl<T: LoadFromMem<i64> + GetAllocator + Reveal> Reveal for Slice<T>
where
    <T as Reveal>::Output: GetAllocator + StoreInMem<i64>,
{
    type Output = Slice<<T as Reveal>::Output>;
    #[inline(always)]
    fn reveal(&self) -> Self::Output {
        let mut array = Self::Output::uninitialized(self.length);
        for (i, val) in self.iter().enumerate() {
            array.set(i as u64, &val.reveal());
        }
        array
    }
}

impl<T: LoadFromMem<i64>> Slice<T> {
    pub fn len(&self) -> u64 {
        self.length
    }
}

impl<T> Slice<T> {
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
                Bound::Unbounded => self.length,
                Bound::Included(n) => n + 1,
                Bound::Excluded(n) => *n,
            } - start,
        }
    }
}
