// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use super::guard::{Guard, GuardMut};
use super::heap::Box;
use crate::iter::CompileTimeLengthIterator;
use crate::slice::Slice;
use core::marker::PhantomData;
use core::ops::{Add, Div, Mul, Rem, Sub};
use core::ops::{Bound, RangeBounds};
use scale::alloc::GetAllocator;
use scale::*;
use scale::{alloc::Allocate, LoadFromMem, Reveal, Stack, StackAddress, StoreInMem};

/// An array datastructure that allocates memory and deallocates when goes out of scope
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
    fn size_type() -> u64 {
        1
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
            new_one.set(i, &T::load_from_mem(self.addr(start + i * T::size_type())));
        }

        new_one
    }
    /// Create an Array that points to the same data but has `S` fewer elements.
    pub fn static_slice_from_start<const S: u64>(&self) -> Array<T, { N - S }> {
        let mut new_one: Array<T, { N - S }> = Array::uninitialized();

        for i in 0..(N - S) {
            new_one.set(i, &T::load_from_mem(self.addr(i * T::size_type())));
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
            inner: T::load_from_mem(self.addr(i * T::size_type())),
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
            inner: T::load_from_mem(self.addr(i * T::size_type())),
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
            inner: T::load_from_mem(self.addr(i * T::size_type())),
            phantom: PhantomData,
        })
    }

    /// Get the mutable reference of an element from the array without checking we do not make any memory overflow
    #[inline(always)]
    pub fn get_mut_unchecked(&'a mut self, i: u64) -> GuardMut<'a, T> {
        GuardMut {
            inner: T::load_from_mem(self.addr(i * T::size_type())),
            phantom: PhantomData,
        }
    }
}

impl<T: GetAllocator, const N: u64> Array<T, N> {
    pub fn addr(&self, i: u64) -> i64 {
        (self.first_element.address + i * T::Allocator::N) as i64
    }
}

impl<T, const N: u64> Array<T, N>
where
    T: GetAllocator + StoreInMem<i64>,
{
    #[inline(always)]
    pub fn set(&mut self, i: u64, val: &T) {
        unsafe { val.store_in_mem(self.addr(i * T::size_type())) }
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

impl<const N: u64> Array<SecretModp, N> {
    #[inline(always)]
    pub fn private_input(player: i64, channel: i64) -> Self {
        let array = Self::uninitialized();
        unsafe { __mprivate_input(array.first_element.address as i64, N as i64, player, channel) }
        array
    }

    #[inline(always)]
    pub fn private_output(self, player: i64, channel: i64) { 
        unsafe {
            __mprivate_output(self.first_element.address as i64, N as i64, player, channel);
        }
    }
}

/* Operator Versions of Arithmetic Operations */

// a_add_cc_t = &ca_arr + cb_arr.clone();
impl<'a, const N: u64> Add<Array<ClearModp, N>> for &'a Array<ClearModp, N> {
    type Output = Array<ClearModp, N>;
    #[inline(always)]
    fn add(self, other: Array<ClearModp, N>) -> Self::Output {
        unsafe {
            __maddc(
                other.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        other
    }
}

// a_add_cc_t = ca_arr.clone + &cb_arr;
impl<'a, const N: u64> Add<&'a Array<ClearModp, N>> for Array<ClearModp, N> {
    type Output = Array<ClearModp, N>;
    #[inline(always)]
    fn add(self, other: &'a Array<ClearModp, N>) -> Self::Output {
        unsafe {
            __maddc(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        self
    }
}

// a_add_cc_t = &ca_arr + &cb_arr;
impl<'a, const N: u64> Add<&'a Array<ClearModp, N>> for &'a Array<ClearModp, N> {
    type Output = Array<ClearModp, N>;
    #[inline(always)]
    fn add(self, other: &'a Array<ClearModp, N>) -> Self::Output {
        let array = Array::uninitialized();
        unsafe {
            __maddc(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
}

// a_add_cc_t = ca_arr.clone() + cb_arr.clone();
impl<const N: u64> Add<Array<ClearModp, N>> for Array<ClearModp, N> {
    type Output = Array<ClearModp, N>;
    #[inline(always)]
    fn add(self, other: Array<ClearModp, N>) -> Self::Output {
        unsafe {
            __maddc(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        self
    }
}

// a_add_sc_t = &sa_arr + cb_arr.clone();
impl<'a, const N: u64> Add<Array<ClearModp, N>> for &'a Array<SecretModp, N> {
    type Output = Array<SecretModp, N>;
    #[inline(always)]
    fn add(self, other: Array<ClearModp, N>) -> Self::Output {
        let array = Array::uninitialized();
        unsafe {
            __maddm(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
}

// a_add_sc_t = sa_arr.clone + &cb_arr;
impl<'a, const N: u64> Add<&'a Array<ClearModp, N>> for Array<SecretModp, N> {
    type Output = Array<SecretModp, N>;
    #[inline(always)]
    fn add(self, other: &'a Array<ClearModp, N>) -> Self::Output {
        unsafe {
            __maddm(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        self
    }
}

// a_add_sc_t = &sa_arr + &cb_arr;
impl<'a, const N: u64> Add<&'a Array<ClearModp, N>> for &'a Array<SecretModp, N> {
    type Output = Array<SecretModp, N>;
    #[inline(always)]
    fn add(self, other: &'a Array<ClearModp, N>) -> Self::Output {
        let array = Array::uninitialized();
        unsafe {
            __maddm(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
}

// a_add_sc_t = sa_arr.clone() + cb_arr.clone();
impl<const N: u64> Add<Array<ClearModp, N>> for Array<SecretModp, N> {
    type Output = Array<SecretModp, N>;
    #[inline(always)]
    fn add(self, other: Array<ClearModp, N>) -> Self::Output {
        unsafe {
            __maddm(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        self
    }
}

// a_add_cs_t = &ca_arr + sb_arr.clone();
impl<'a, const N: u64> Add<Array<SecretModp, N>> for &'a Array<ClearModp, N> {
    type Output = Array<SecretModp, N>;
    #[inline(always)]
    fn add(self, other: Array<SecretModp, N>) -> Self::Output {
        unsafe {
            __maddm(
                other.first_element.address as i64,
                other.first_element.address as i64,
                self.first_element.address as i64,
                N as i64,
            );
        }
        other
    }
}

// a_add_cs_t = ca_arr.clone + &sb_arr;
impl<'a, const N: u64> Add<&'a Array<SecretModp, N>> for Array<ClearModp, N> {
    type Output = Array<SecretModp, N>;
    #[inline(always)]
    fn add(self, other: &'a Array<SecretModp, N>) -> Self::Output {
        let array = Array::uninitialized();
        unsafe {
            __maddm(
                array.first_element.address as i64,
                other.first_element.address as i64,
                self.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
}

// a_add_cs_t = &ca_arr + &sb_arr;
impl<'a, const N: u64> Add<&'a Array<SecretModp, N>> for &'a Array<ClearModp, N> {
    type Output = Array<SecretModp, N>;
    #[inline(always)]
    fn add(self, other: &'a Array<SecretModp, N>) -> Self::Output {
        let array = Array::uninitialized();
        unsafe {
            __maddm(
                array.first_element.address as i64,
                other.first_element.address as i64,
                self.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
}

// a_add_sc_t = ca_arr.clone() + sb_arr.clone();
impl<const N: u64> Add<Array<SecretModp, N>> for Array<ClearModp, N> {
    type Output = Array<SecretModp, N>;
    #[inline(always)]
    fn add(self, other: Array<SecretModp, N>) -> Self::Output {
        let array: Array<SecretModp, N> = Array::uninitialized();
        unsafe {
            __maddm(
                array.first_element.address as i64,
                other.first_element.address as i64,
                self.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
}

// a_add_ss_t = &sa_arr + sb_arr.clone();
impl<'a, const N: u64> Add<Array<SecretModp, N>> for &'a Array<SecretModp, N> {
    type Output = Array<SecretModp, N>;
    #[inline(always)]
    fn add(self, other: Array<SecretModp, N>) -> Self::Output {
        unsafe {
            __madds(
                other.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        other
    }
}

// a_add_ss_t = sa_arr.clone + &sb_arr;
impl<'a, const N: u64> Add<&'a Array<SecretModp, N>> for Array<SecretModp, N> {
    type Output = Self;
    #[inline(always)]
    fn add(self, other: &'a Array<SecretModp, N>) -> Self::Output {
        unsafe {
            __madds(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        self
    }
}

// a_add_ss_t = &sa_arr + &sb_arr;
impl<'a, const N: u64> Add<&'a Array<SecretModp, N>> for &'a Array<SecretModp, N> {
    type Output = Array<SecretModp, N>;
    #[inline(always)]
    fn add(self, other: &'a Array<SecretModp, N>) -> Self::Output {
        let array = Array::uninitialized();
        unsafe {
            __madds(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
}

// a_add_ss_t = sa_arr.clone() + sb_arr.clone();
impl<const N: u64> Add<Array<SecretModp, N>> for Array<SecretModp, N> {
    type Output = Self;
    #[inline(always)]
    fn add(self, other: Array<SecretModp, N>) -> Self::Output {
        unsafe {
            __madds(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        self
    }
}

// a_sub_cc_t = &ca_arr - cb_arr.clone();
impl<'a, const N: u64> Sub<Array<ClearModp, N>> for &'a Array<ClearModp, N> {
    type Output = Array<ClearModp, N>;
    #[inline(always)]
    fn sub(self, other: Array<ClearModp, N>) -> Self::Output {
        unsafe {
            __msubc(
                other.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        other
    }
}

// a_sub_cc_t = ca_arr.clone - &cb_arr;
impl<'a, const N: u64> Sub<&'a Array<ClearModp, N>> for Array<ClearModp, N> {
    type Output = Array<ClearModp, N>;
    #[inline(always)]
    fn sub(self, other: &'a Array<ClearModp, N>) -> Self::Output {
        unsafe {
            __msubc(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        self
    }
}

// a_sub_cc_t = &ca_arr - &cb_arr;
impl<'a, const N: u64> Sub<&'a Array<ClearModp, N>> for &'a Array<ClearModp, N> {
    type Output = Array<ClearModp, N>;
    #[inline(always)]
    fn sub(self, other: &'a Array<ClearModp, N>) -> Self::Output {
        let array = Array::uninitialized();
        unsafe {
            __msubc(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
}

// a_sub_cc_t = ca_arr.clone() - cb_arr.clone();
impl<const N: u64> Sub<Array<ClearModp, N>> for Array<ClearModp, N> {
    type Output = Array<ClearModp, N>;
    #[inline(always)]
    fn sub(self, other: Array<ClearModp, N>) -> Self::Output {
        unsafe {
            __msubc(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        self
    }
}

// a_sub_sc_t = &sa_arr - cb_arr.clone();
impl<'a, const N: u64> Sub<Array<ClearModp, N>> for &'a Array<SecretModp, N> {
    type Output = Array<SecretModp, N>;
    #[inline(always)]
    fn sub(self, other: Array<ClearModp, N>) -> Self::Output {
        let array = Array::uninitialized();
        unsafe {
            __msubml(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
}

// a_sub_sc_t = sa_arr.clone - &cb_arr;
impl<'a, const N: u64> Sub<&'a Array<ClearModp, N>> for Array<SecretModp, N> {
    type Output = Array<SecretModp, N>;
    #[inline(always)]
    fn sub(self, other: &'a Array<ClearModp, N>) -> Self::Output {
        unsafe {
            __msubml(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        self
    }
}

// a_sub_sc_t = &sa_arr - &cb_arr;
impl<'a, const N: u64> Sub<&'a Array<ClearModp, N>> for &'a Array<SecretModp, N> {
    type Output = Array<SecretModp, N>;
    #[inline(always)]
    fn sub(self, other: &'a Array<ClearModp, N>) -> Self::Output {
        let array = Array::uninitialized();
        unsafe {
            __msubml(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
}

// a_sub_sc_t = sa_arr.clone() - cb_arr.clone();
impl<const N: u64> Sub<Array<ClearModp, N>> for Array<SecretModp, N> {
    type Output = Array<SecretModp, N>;
    #[inline(always)]
    fn sub(self, other: Array<ClearModp, N>) -> Self::Output {
        unsafe {
            __msubml(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        self
    }
}

// a_sub_cs_t = &ca_arr - sb_arr.clone();
impl<'a, const N: u64> Sub<Array<SecretModp, N>> for &'a Array<ClearModp, N> {
    type Output = Array<SecretModp, N>;
    #[inline(always)]
    fn sub(self, other: Array<SecretModp, N>) -> Self::Output {
        unsafe {
            __msubmr(
                other.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        other
    }
}

// a_sub_cs_t = ca_arr.clone - &sb_arr;
impl<'a, const N: u64> Sub<&'a Array<SecretModp, N>> for Array<ClearModp, N> {
    type Output = Array<SecretModp, N>;
    #[inline(always)]
    fn sub(self, other: &'a Array<SecretModp, N>) -> Self::Output {
        let array = Array::uninitialized();
        unsafe {
            __msubmr(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
}

// a_sub_cs_t = &ca_arr - &sb_arr;
impl<'a, const N: u64> Sub<&'a Array<SecretModp, N>> for &'a Array<ClearModp, N> {
    type Output = Array<SecretModp, N>;
    #[inline(always)]
    fn sub(self, other: &'a Array<SecretModp, N>) -> Self::Output {
        let array = Array::uninitialized();
        unsafe {
            __msubmr(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
}

// a_sub_sc_t = ca_arr.clone() - sb_arr.clone();
impl<const N: u64> Sub<Array<SecretModp, N>> for Array<ClearModp, N> {
    type Output = Array<SecretModp, N>;
    #[inline(always)]
    fn sub(self, other: Array<SecretModp, N>) -> Self::Output {
        let array: Array<SecretModp, N> = Array::uninitialized();
        unsafe {
            __msubmr(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
}

// a_sub_ss_t = &sa_arr - sb_arr.clone();
impl<'a, const N: u64> Sub<Array<SecretModp, N>> for &'a Array<SecretModp, N> {
    type Output = Array<SecretModp, N>;
    #[inline(always)]
    fn sub(self, other: Array<SecretModp, N>) -> Self::Output {
        unsafe {
            __msubs(
                other.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        other
    }
}

// a_sub_ss_t = sa_arr.clone - &sb_arr;
impl<'a, const N: u64> Sub<&'a Array<SecretModp, N>> for Array<SecretModp, N> {
    type Output = Self;
    #[inline(always)]
    fn sub(self, other: &'a Array<SecretModp, N>) -> Self::Output {
        unsafe {
            __msubs(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        self
    }
}

// a_sub_ss_t = &sa_arr - &sb_arr;
impl<'a, const N: u64> Sub<&'a Array<SecretModp, N>> for &'a Array<SecretModp, N> {
    type Output = Array<SecretModp, N>;
    #[inline(always)]
    fn sub(self, other: &'a Array<SecretModp, N>) -> Self::Output {
        let array = Array::uninitialized();
        unsafe {
            __msubs(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
}

// a_sub_ss_t = sa_arr.clone() - sb_arr.clone();
impl<const N: u64> Sub<Array<SecretModp, N>> for Array<SecretModp, N> {
    type Output = Self;
    #[inline(always)]
    fn sub(self, other: Array<SecretModp, N>) -> Self::Output {
        unsafe {
            __msubs(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        self
    }
}

/* Operator Versions of Arithmetic Operations */

// a_mul_cc_t = &ca_arr * cb_arr.clone();
impl<'a, const N: u64> Mul<Array<ClearModp, N>> for &'a Array<ClearModp, N> {
    type Output = Array<ClearModp, N>;
    #[inline(always)]
    fn mul(self, other: Array<ClearModp, N>) -> Self::Output {
        unsafe {
            __mmulc(
                other.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        other
    }
}

// a_mul_cc_t = ca_arr.clone * &cb_arr;
impl<'a, const N: u64> Mul<&'a Array<ClearModp, N>> for Array<ClearModp, N> {
    type Output = Array<ClearModp, N>;
    #[inline(always)]
    fn mul(self, other: &'a Array<ClearModp, N>) -> Self::Output {
        unsafe {
            __mmulc(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        self
    }
}

// a_mul_cc_t = &ca_arr * &cb_arr;
impl<'a, const N: u64> Mul<&'a Array<ClearModp, N>> for &'a Array<ClearModp, N> {
    type Output = Array<ClearModp, N>;
    #[inline(always)]
    fn mul(self, other: &'a Array<ClearModp, N>) -> Self::Output {
        let array = Array::uninitialized();
        unsafe {
            __mmulc(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
}

// a_mul_cc_t = ca_arr.clone() * cb_arr.clone();
impl<const N: u64> Mul<Array<ClearModp, N>> for Array<ClearModp, N> {
    type Output = Array<ClearModp, N>;
    #[inline(always)]
    fn mul(self, other: Array<ClearModp, N>) -> Self::Output {
        unsafe {
            __mmulc(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        self
    }
}

// a_mul_sc_t = &sa_arr * cb_arr.clone();
impl<'a, const N: u64> Mul<Array<ClearModp, N>> for &'a Array<SecretModp, N> {
    type Output = Array<SecretModp, N>;
    #[inline(always)]
    fn mul(self, other: Array<ClearModp, N>) -> Self::Output {
        let array = Array::uninitialized();
        unsafe {
            __mmulm(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
}

// a_mul_sc_t = sa_arr.clone * &cb_arr;
impl<'a, const N: u64> Mul<&'a Array<ClearModp, N>> for Array<SecretModp, N> {
    type Output = Array<SecretModp, N>;
    #[inline(always)]
    fn mul(self, other: &'a Array<ClearModp, N>) -> Self::Output {
        unsafe {
            __mmulm(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        self
    }
}

// a_mul_sc_t = &sa_arr * &cb_arr;
impl<'a, const N: u64> Mul<&'a Array<ClearModp, N>> for &'a Array<SecretModp, N> {
    type Output = Array<SecretModp, N>;
    #[inline(always)]
    fn mul(self, other: &'a Array<ClearModp, N>) -> Self::Output {
        let array = Array::uninitialized();
        unsafe {
            __mmulm(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
}

// a_mul_sc_t = sa_arr.clone() * cb_arr.clone();
impl<const N: u64> Mul<Array<ClearModp, N>> for Array<SecretModp, N> {
    type Output = Array<SecretModp, N>;
    #[inline(always)]
    fn mul(self, other: Array<ClearModp, N>) -> Self::Output {
        unsafe {
            __mmulm(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        self
    }
}

// a_mul_cs_t = &ca_arr * sb_arr.clone();
impl<'a, const N: u64> Mul<Array<SecretModp, N>> for &'a Array<ClearModp, N> {
    type Output = Array<SecretModp, N>;
    #[inline(always)]
    fn mul(self, other: Array<SecretModp, N>) -> Self::Output {
        unsafe {
            __mmulm(
                other.first_element.address as i64,
                other.first_element.address as i64,
                self.first_element.address as i64,
                N as i64,
            );
        }
        other
    }
}

// a_mul_cs_t = ca_arr.clone * &sb_arr;
impl<'a, const N: u64> Mul<&'a Array<SecretModp, N>> for Array<ClearModp, N> {
    type Output = Array<SecretModp, N>;
    #[inline(always)]
    fn mul(self, other: &'a Array<SecretModp, N>) -> Self::Output {
        let array = Array::uninitialized();
        unsafe {
            __mmulm(
                array.first_element.address as i64,
                other.first_element.address as i64,
                self.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
}

// a_mul_cs_t = &ca_arr * &sb_arr;
impl<'a, const N: u64> Mul<&'a Array<SecretModp, N>> for &'a Array<ClearModp, N> {
    type Output = Array<SecretModp, N>;
    #[inline(always)]
    fn mul(self, other: &'a Array<SecretModp, N>) -> Self::Output {
        let array = Array::uninitialized();
        unsafe {
            __mmulm(
                array.first_element.address as i64,
                other.first_element.address as i64,
                self.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
}

// a_mul_sc_t = ca_arr.clone() * &sb_arr.clone();
impl<const N: u64> Mul<Array<SecretModp, N>> for Array<ClearModp, N> {
    type Output = Array<SecretModp, N>;
    #[inline(always)]
    fn mul(self, other: Array<SecretModp, N>) -> Self::Output {
        let array: Array<SecretModp, N> = Array::uninitialized();
        unsafe {
            __mmulm(
                array.first_element.address as i64,
                other.first_element.address as i64,
                self.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
}

// a_div_cc_t = &ca_arr / cb_arr.clone();
impl<'a, const N: u64> Div<Array<ClearModp, N>> for &'a Array<ClearModp, N> {
    type Output = Array<ClearModp, N>;
    #[inline(always)]
    fn div(self, other: Array<ClearModp, N>) -> Self::Output {
        unsafe {
            __mdivc(
                other.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        other
    }
}

// a_mod_cc_t = ca_arr.clone / &cb_arr;
impl<'a, const N: u64> Div<&'a Array<ClearModp, N>> for Array<ClearModp, N> {
    type Output = Array<ClearModp, N>;
    #[inline(always)]
    fn div(self, other: &'a Array<ClearModp, N>) -> Self::Output {
        unsafe {
            __mdivc(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        self
    }
}

// a_mod_cc_t = &ca_arr / &cb_arr;
impl<'a, const N: u64> Div<&'a Array<ClearModp, N>> for &'a Array<ClearModp, N> {
    type Output = Array<ClearModp, N>;
    #[inline(always)]
    fn div(self, other: &'a Array<ClearModp, N>) -> Self::Output {
        let array = Array::uninitialized();
        unsafe {
            __mdivc(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
}

// a_mod_cc_t = ca_arr.clone() / cb_arr.clone();
impl<const N: u64> Div<Array<ClearModp, N>> for Array<ClearModp, N> {
    type Output = Array<ClearModp, N>;
    #[inline(always)]
    fn div(self, other: Array<ClearModp, N>) -> Self::Output {
        unsafe {
            __mdivc(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        self
    }
}

// a_mod_cc_t = &ca_arr % cb_arr.clone();
impl<'a, const N: u64> Rem<Array<ClearModp, N>> for &'a Array<ClearModp, N> {
    type Output = Array<ClearModp, N>;
    #[inline(always)]
    fn rem(self, other: Array<ClearModp, N>) -> Self::Output {
        unsafe {
            __mmodc(
                other.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        other
    }
}

// a_mod_cc_t = ca_arr.clone % &cb_arr;
impl<'a, const N: u64> Rem<&'a Array<ClearModp, N>> for Array<ClearModp, N> {
    type Output = Array<ClearModp, N>;
    #[inline(always)]
    fn rem(self, other: &'a Array<ClearModp, N>) -> Self::Output {
        unsafe {
            __mmodc(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        self
    }
}

// a_mod_cc_t = &ca_arr % &cb_arr;
impl<'a, const N: u64> Rem<&'a Array<ClearModp, N>> for &'a Array<ClearModp, N> {
    type Output = Array<ClearModp, N>;
    #[inline(always)]
    fn rem(self, other: &'a Array<ClearModp, N>) -> Self::Output {
        let array = Array::uninitialized();
        unsafe {
            __mmodc(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
}

// a_mod_cc_t = ca_arr.clone() % cb_arr.clone();
impl<const N: u64> Rem<Array<ClearModp, N>> for Array<ClearModp, N> {
    type Output = Array<ClearModp, N>;
    #[inline(always)]
    fn rem(self, other: Array<ClearModp, N>) -> Self::Output {
        unsafe {
            __mmodc(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        self
    }
}

/* Member function versions of artihemtic operations */

impl<const N: u64> Array<ClearModp, N> {
    #[inline(always)]
    pub fn add_clear(&self, other: &Array<ClearModp, N>) -> Array<ClearModp, N> {
        let array = Self::uninitialized();
        unsafe {
            __maddc(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
    #[inline(always)]
    pub fn add_secret(&self, other: &Array<SecretModp, N>) -> Array<SecretModp, N> {
        let array: Array<SecretModp, N> = Array::uninitialized();
        unsafe {
            __maddm(
                array.first_element.address as i64,
                other.first_element.address as i64,
                self.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
    #[inline(always)]
    pub fn sub_clear(&self, other: &Array<ClearModp, N>) -> Array<ClearModp, N> {
        let array = Self::uninitialized();
        unsafe {
            __msubc(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
    #[inline(always)]
    pub fn sub_secret(&self, other: &Array<SecretModp, N>) -> Array<SecretModp, N> {
        let array: Array<SecretModp, N> = Array::uninitialized();
        unsafe {
            __msubmr(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
    #[inline(always)]
    pub fn mul_clear(&self, other: &Array<ClearModp, N>) -> Array<ClearModp, N> {
        let array = Self::uninitialized();
        unsafe {
            __mmulc(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
    #[inline(always)]
    pub fn mul_secret(&self, other: &Array<SecretModp, N>) -> Array<SecretModp, N> {
        let array: Array<SecretModp, N> = Array::uninitialized();
        unsafe {
            __mmulm(
                array.first_element.address as i64,
                other.first_element.address as i64,
                self.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
    #[inline(always)]
    pub fn div_clear(&self, other: &Array<ClearModp, N>) -> Array<ClearModp, N> {
        let array = Self::uninitialized();
        unsafe {
            __mdivc(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
    #[inline(always)]
    pub fn mod_clear(&self, other: &Array<ClearModp, N>) -> Array<ClearModp, N> {
        let array = Self::uninitialized();
        unsafe {
            __mmodc(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
}

impl<const N: u64> Array<SecretModp, N> {
    #[inline(always)]
    pub fn add_clear(&self, other: &Array<ClearModp, N>) -> Array<SecretModp, N> {
        let array = Self::uninitialized();
        unsafe {
            __maddm(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
    #[inline(always)]
    pub fn add_secret(&self, other: &Array<SecretModp, N>) -> Array<SecretModp, N> {
        let array = Self::uninitialized();
        unsafe {
            __madds(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
    #[inline(always)]
    pub fn sub_clear(&self, other: &Array<ClearModp, N>) -> Array<SecretModp, N> {
        let array = Self::uninitialized();
        unsafe {
            __msubml(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
    #[inline(always)]
    pub fn sub_secret(&self, other: &Array<SecretModp, N>) -> Array<SecretModp, N> {
        let array = Self::uninitialized();
        unsafe {
            __msubs(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
    #[inline(always)]
    pub fn mul_clear(&self, other: &Array<ClearModp, N>) -> Array<SecretModp, N> {
        let array = Self::uninitialized();
        unsafe {
            __mmulm(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
}

// Reverse an array

impl<const N: u64> Array<ClearModp, N> {
    #[inline(always)]
    pub fn reverse(&self) -> Array<ClearModp, N> {
        let array = Self::uninitialized();
        unsafe {
            __mrevc(
                array.first_element.address as i64,
                self.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
}

impl<const N: u64> Array<SecretModp, N> {
    #[inline(always)]
    pub fn reverse(&self) -> Array<SecretModp, N> {
        let array = Self::uninitialized();
        unsafe {
            __mrevs(
                array.first_element.address as i64,
                self.first_element.address as i64,
                N as i64,
            );
        }
        array
    }
}

// Evaluate a polynomial represented by an array at a value x
//
impl<const N: u64> Array<ClearModp, N> {
    #[inline(always)]
    pub fn evaluate(&self, x: ClearModp) -> ClearModp {
        unsafe { __mevalcc(self.first_element.address as i64, N as i64, x) }
    }
}

impl<const N: u64> Array<SecretModp, N> {
    #[inline(always)]
    pub fn evaluate(&self, x: ClearModp) -> SecretModp {
        unsafe { __mevalsc(self.first_element.address as i64, N as i64, x) }
    }
}

impl<const N: u64> Array<ClearModp, N> {
    #[inline(always)]
    #[allow(non_snake_case)]
    pub fn bit_decomposition_ClearModp(val: ClearModp) -> Self {
        let array = Self::uninitialized();
        unsafe { __mbitdecc(array.first_element.address as i64, val, N as i64) }
        array
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    pub fn bit_decomposition_ClearModp_Signed(val: ClearModp) -> Self {
        let array = Self::uninitialized();
        unsafe { __mbitdeccs(array.first_element.address as i64, val, N as i64) }
        array
    }

}

impl<const N: u64> Array<i64, N> {
    #[inline(always)]
    #[allow(non_snake_case)]
    pub fn bit_decomposition_i64(val: i64) -> Self {
        let array = Self::uninitialized();
        unsafe { __mbitdecint(array.first_element.address as i64, val, N as i64) }
        array
    }
}
