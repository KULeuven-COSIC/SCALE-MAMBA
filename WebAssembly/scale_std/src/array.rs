// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use super::guard::{Guard, GuardMut};
use super::heap::Box;
use crate::iter::CompileTimeLengthIterator;
use crate::slice::Slice;
use core::marker::PhantomData;
use core::ops::{Bound, RangeBounds};
use scale::alloc::GetAllocator;
use scale::{alloc::Allocate, LoadFromMem, Reveal, Stack, StackAddress, StoreInMem};

/// An array datastructure that allocates memory and never frees it
pub struct Array<T, const N: u64>
where
    T: GetAllocator,
{
    first_element: Box<T>,
}

impl<T: GetAllocator, const N: u64> Array<T, N> {
    #[inline(always)]
    pub fn uninitialized() -> Self {
        Self {
            first_element: Box::uninitialized_multiple(N),
        }
    }
}

pub struct ArrayAllocator<T: GetAllocator, const N: u64> {
    element_allocator: T::Allocator,
}

impl<T: GetAllocator, const N: u64> Allocate for ArrayAllocator<T, N> {
    const N: u64 = N;
    #[inline(always)]
    unsafe fn free(&self, n: u64) {
        self.element_allocator.free(n);
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
            first_element: unsafe { Box::load_from_addr(base_address as u64) },
        }
    }
}

impl<T, const N: u64> Array<T, N>
where
    T: GetAllocator + StoreInMem<i64>,
{
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

impl<T, const N: u64> Array<T, N>
where
    T: GetAllocator + StoreInMem<i64> + LoadFromMem<i64>,
{
    pub fn slice(&self, range: impl RangeBounds<u64>) -> Slice<T> {
        let start = match range.start_bound() {
            Bound::Unbounded => 0,
            Bound::Included(n) => *n,
            Bound::Excluded(n) => n + 1,
        };
        let length = match range.end_bound() {
            Bound::Unbounded => N,
            Bound::Included(n) => n + 1,
            Bound::Excluded(n) => *n,
        } - start;

        let mut new_one = Slice::uninitialized(length);
        for i in 0..length {
            new_one.set(i, &T::load_from_mem(self.addr(start + i)));
        }

        new_one
    }
    /// Create an Array that points to the same data but has `S` fewer elements.
    pub fn static_slice_from_start<const S: u64>(&self) -> Array<T, { N - S }> {
        let mut new_one: Array<T, { N - S }> = Array::uninitialized();

        for i in 0..(N - S) {
            new_one.set(i, &T::load_from_mem(self.addr(i)));
        }

        new_one
    }
}

impl<T, const N: u64> Array<T, N>
where
    T: GetAllocator + LoadFromMem<i64> + Clone,
{
    #[inline(always)]
    pub fn iter(
        &self,
    ) -> impl Iterator<Item = T> + CompileTimeLengthIterator<N> + DoubleEndedIterator + '_ {
        unsafe { crate::iter::Iter::new((0..N).map(move |i| self.get_unchecked(i).clone())) }
    }
}

// Access to array
impl<'a, T, const N: u64> Array<T, N>
where
    T: GetAllocator + LoadFromMem<i64>,
{
    /// Get the reference of an element from the array without checking we do not make any memory overflow
    #[inline(always)]
    pub fn get_unchecked(&'a self, i: u64) -> Guard<'a, T> {
        Guard {
            inner: T::load_from_mem(self.addr(i)),
            phantom: PhantomData,
        }
    }

    /// Get the reference of an element from the array by checking we do not make any memory overflow
    #[inline(always)]
    pub fn get(&'a self, i: u64) -> Option<Guard<'a, T>> {
        if i >= N {
            return None;
        }

        Some(Guard {
            inner: T::load_from_mem(self.addr(i)),
            phantom: PhantomData,
        })
    }

    /// Get the mutable reference of an element from the array by checking we do not make any memory overflow
    #[inline(always)]
    pub fn get_mut(&'a mut self, i: u64) -> Option<GuardMut<'a, T>> {
        if i >= N {
            return None;
        }

        Some(GuardMut {
            inner: T::load_from_mem(self.addr(i)),
            phantom: PhantomData,
        })
    }

    /// Get the mutable reference of an element from the array without checking we do not make any memory overflow
    #[inline(always)]
    pub fn get_mut_unchecked(&'a mut self, i: u64) -> GuardMut<'a, T> {
        GuardMut {
            inner: T::load_from_mem(self.addr(i)),
            phantom: PhantomData,
        }
    }
}

impl<T: GetAllocator, const N: u64> Array<T, N> {
    fn addr(&self, i: u64) -> i64 {
        (self.first_element.address + i * T::Allocator::N) as i64
    }
}

impl<T, const N: u64> Array<T, N>
where
    T: GetAllocator + StoreInMem<i64>,
{
    #[inline(always)]
    pub fn set(&mut self, i: u64, val: &T) {
        unsafe { val.store_in_mem(self.addr(i)) }
    }
}

impl<T, const N: u64> Reveal for Array<T, N>
where
    T: LoadFromMem<i64> + GetAllocator + Stack + Reveal + Clone,
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

impl<T, const N: u64> Stack for Array<T, N>
where
    T: GetAllocator + StoreInMem<i64> + LoadFromMem<i64> + Stack + Clone,
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

impl<T, const N: u64> Clone for Array<T, N>
where
    T: GetAllocator + StoreInMem<i64> + LoadFromMem<i64>,
{
    fn clone(&self) -> Self {
        let mut uninit_array: Array<T, N> = Self::uninitialized();
        for idx in 0..N {
            uninit_array.set(idx, &self.get_unchecked(idx));
        }

        uninit_array
    }
}
