// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use crate::array::Array;
use scale::alloc::GetAllocator;
use scale::{alloc::Allocate, LoadFromMem, Reveal, Stack, StackAddress, StoreInMem};

/// A matrix datastructure that allocates memory and deallocates when goes out of scope
pub struct Matrix<T, const N: u64, const M: u64> {
    // Act like we are a matrix
    phantom: core::marker::PhantomData<T>,
    // Base address of the matrix in memory
    base_address: u64,
}

impl<T: GetAllocator + StoreInMem<i64>, const N: u64, const M: u64> Matrix<T, N, M> {
    #[inline(always)]
    pub fn fill(t: T) -> Self {
        let mut matrix = Self::uninitialized();
        for i in 0..(N * M) {
            matrix.set_mem(i, &t);
        }
        matrix
    }

    #[inline(always)]
    pub fn uninitialized() -> Self {
        Self {
            phantom: core::marker::PhantomData,
            base_address: T::get_allocator().allocate(N * M),
        }
    }
}

impl<T: LoadFromMem<i64> + GetAllocator + StoreInMem<i64>, const N: u64, const M: u64>
    Matrix<T, N, M>
{
    #[inline(always)]
    pub fn iter(&self) -> impl Iterator<Item = T> + '_ {
        (0..(N * M)).map(move |i| self.get_mem(i))
    }
    #[inline(always)]
    pub fn get(&self, i: u64, j: u64) -> T {
        T::load_from_mem((self.base_address + i * M + j) as _)
    }

    /* This version just indexes according to the 'standard' ordering */
    #[inline(always)]
    pub fn get_mem(&self, i: u64) -> T {
        T::load_from_mem((self.base_address + i) as _)
    }

    #[inline(always)]
    pub fn get_row(&self, i: u64) -> Array<T, M> {
        Array::from_iter((0..M).map(|j| T::load_from_mem((self.base_address + i * M + j) as _)))
    }
    #[inline(always)]
    pub fn get_column(&self, j: u64) -> Array<T, N> {
        Array::from_iter((0..N).map(|i| T::load_from_mem((self.base_address + i * M + j) as _)))
    }
}

impl<T: StoreInMem<i64>, const N: u64, const M: u64> Matrix<T, N, M> {
    #[inline(always)]
    pub fn set(&mut self, i: u64, j: u64, val: &T) {
        unsafe { val.store_in_mem((self.base_address + i * M + j) as _) }
    }

    /* This version just indexes according to the 'standard' ordering */
    #[inline(always)]
    pub fn set_mem(&mut self, i: u64, val: &T) {
        unsafe { val.store_in_mem((self.base_address + i) as _) }
    }
}

impl<
        T: LoadFromMem<i64> + StoreInMem<i64> + GetAllocator + Stack + Reveal,
        const N: u64,
        const M: u64,
    > Reveal for Matrix<T, N, M>
where
    <T as Reveal>::Output: GetAllocator + StoreInMem<i64>,
{
    type Output = Matrix<<T as Reveal>::Output, N, M>;
    #[inline(always)]
    fn reveal(&self) -> Self::Output {
        let mut matrix = Self::Output::uninitialized();
        for (i, val) in self.iter().enumerate() {
            matrix.set_mem(i as u64, &val.reveal());
        }
        matrix
    }
}

impl<T: GetAllocator + StoreInMem<i64> + LoadFromMem<i64> + Stack, const N: u64, const M: u64> Stack
    for Matrix<T, N, M>
{
    #[inline(always)]
    unsafe fn push(matrix: &Self) {
        for val in matrix.iter() {
            T::push(&val);
        }
    }
    #[inline(always)]
    unsafe fn pop() -> Self {
        let mut matrix = Self::uninitialized();
        for i in (0..(N * M)).rev() {
            matrix.set_mem(i, &T::pop())
        }
        matrix
    }

    #[inline(always)]
    unsafe fn peek(addr: StackAddress) -> Self {
        let mut matrix = Self::uninitialized();
        for i in 0..(N * M) {
            matrix.set_mem(i, &T::peek(addr + i as _));
        }
        matrix
    }

    #[inline(always)]
    unsafe fn poke(addr: StackAddress, matrix: &Self) {
        for (i, val) in matrix.iter().enumerate() {
            T::poke(addr + i as _, &val);
        }
    }

    #[inline(always)]
    unsafe fn peek_from_top(offset: i64) -> Self {
        let mut matrix = Self::uninitialized();
        for i in 0..(N * M) {
            matrix.set_mem(i, &T::peek_from_top(offset + i as i64));
        }
        matrix
    }

    #[inline(always)]
    unsafe fn poke_from_top(offset: i64, matrix: &Self) {
        for (i, val) in matrix.iter().enumerate() {
            T::poke_from_top(offset + i as i64, &val);
        }
    }

    #[inline(always)]
    fn get_stack_pointer() -> StackAddress {
        unimplemented!("this operation makes no sense");
    }
}
