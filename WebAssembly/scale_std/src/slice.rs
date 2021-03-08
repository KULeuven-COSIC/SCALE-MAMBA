// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use super::guard::{Guard, GuardMut};
use super::heap::Box;
use core::marker::PhantomData;
use core::ops::{Bound, RangeBounds};
use scale::alloc::GetAllocator;
use scale::{alloc::Allocate, LoadFromMem, Reveal, StoreInMem};

/// An array datastructure that allocates memory and never frees it
pub struct Slice<T>
where
    T: GetAllocator,
{
    pub(crate) first_element: Box<T>,
    // Length of the slice
    pub(crate) length: u64,
}

impl<T: GetAllocator> Slice<T> {
    #[inline(always)]
    pub fn uninitialized(length: u64) -> Self {
        Self {
            first_element: Box::uninitialized_multiple(length),
            length,
        }
    }
}

impl<T> Slice<T>
where
    T: GetAllocator + StoreInMem<i64>,
{
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
        (self.first_element.address + i * T::Allocator::N) as i64
    }
}

impl<T> Slice<T>
where
    T: GetAllocator + LoadFromMem<i64> + Clone,
{
    #[inline(always)]
    fn iter(&self) -> impl Iterator<Item = T> + '_ {
        (0..self.length).map(move |i| self.get_unchecked(i).clone())
    }
}

// Access to slice
impl<'a, T> Slice<T>
where
    T: GetAllocator + LoadFromMem<i64>,
{
    /// Get the reference of an element from the slice without checking about memory overflow
    #[inline(always)]
    pub fn get_unchecked(&'a self, i: u64) -> Guard<'a, T> {
        Guard {
            inner: T::load_from_mem(self.addr(i)),
            phantom: PhantomData,
        }
    }

    /// Get the reference of an element from the slice by checking we do not make any memory overflow
    #[inline(always)]
    pub fn get(&'a self, i: u64) -> Option<Guard<'a, T>> {
        if i >= self.length {
            return None;
        }

        Some(Guard {
            inner: T::load_from_mem(self.addr(i)),
            phantom: PhantomData,
        })
    }

    /// Get the mutable reference of an element from the slice by checking we do not make any memory overflow
    #[inline(always)]
    pub fn get_mut(&'a mut self, i: u64) -> Option<GuardMut<'a, T>> {
        if i >= self.length {
            return None;
        }

        Some(GuardMut {
            inner: T::load_from_mem(self.addr(i)),
            phantom: PhantomData,
        })
    }

    /// Get the mutable reference of an element from the slice without checking about memory overflow
    #[inline(always)]
    pub fn get_mut_unchecked(&'a mut self, i: u64) -> GuardMut<'a, T> {
        GuardMut {
            inner: T::load_from_mem(self.addr(i)),
            phantom: PhantomData,
        }
    }
}

pub struct SliceIter<T>
where
    T: GetAllocator,
{
    cursor: u64,
    slice: Slice<T>,
}

impl<T> SliceIter<T>
where
    T: GetAllocator + LoadFromMem<i64>,
{
    fn new(slice: Slice<T>) -> Self {
        Self { cursor: 0, slice }
    }
}

impl<T> Iterator for SliceIter<T>
where
    T: GetAllocator + LoadFromMem<i64> + Copy,
{
    type Item = T;

    fn next(&mut self) -> Option<Self::Item> {
        if self.cursor >= self.slice.length {
            return None;
        }
        let ret_elt = *self.slice.get_unchecked(self.cursor);
        self.cursor += 1;

        Some(ret_elt)
    }
}

impl<T> IntoIterator for Slice<T>
where
    T: GetAllocator + LoadFromMem<i64> + Copy,
{
    type Item = T;

    type IntoIter = SliceIter<Self::Item>;

    fn into_iter(self) -> Self::IntoIter {
        SliceIter::new(self)
    }
}

impl<T> Slice<T>
where
    T: GetAllocator + StoreInMem<i64>,
{
    #[inline(always)]
    pub fn set(&mut self, i: u64, val: &T) {
        unsafe { val.store_in_mem(self.addr(i)) }
    }
}

impl<T> Reveal for Slice<T>
where
    T: LoadFromMem<i64> + GetAllocator + Reveal + Clone,
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

impl<T: LoadFromMem<i64> + GetAllocator> Slice<T> {
    pub fn len(&self) -> u64 {
        self.length
    }
}

impl<T: GetAllocator + StoreInMem<i64> + LoadFromMem<i64> + Clone> Slice<T> {
    pub fn slice(&self, range: impl RangeBounds<u64>) -> Slice<T> {
        let start = match range.start_bound() {
            Bound::Unbounded => 0,
            Bound::Included(n) => *n,
            Bound::Excluded(n) => n + 1,
        };
        let length = match range.end_bound() {
            Bound::Unbounded => self.length,
            Bound::Included(n) => n + 1,
            Bound::Excluded(n) => *n,
        } - start;

        let mut new_one = Slice::uninitialized(length);
        for i in 0..length {
            new_one.set(i, &T::load_from_mem(self.addr(start + i)));
        }

        new_one
    }
}

impl<T> Clone for Slice<T>
where
    T: GetAllocator + StoreInMem<i64> + LoadFromMem<i64> + Clone,
{
    fn clone(&self) -> Self {
        let mut uninit_slice: Slice<T> = Self::uninitialized(self.length);
        for idx in 0..self.length {
            uninit_slice.set(idx, &*self.get_unchecked(idx));
        }

        uninit_slice
    }
}
