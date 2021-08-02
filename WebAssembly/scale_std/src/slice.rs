// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use super::array::Array;
use super::guard::{Guard, GuardMut};
use super::heap::Box;
use core::marker::PhantomData;
use core::ops::{Add, Div, Mul, Rem, Sub};
use core::ops::{Bound, RangeBounds};
use scale::alloc::GetAllocator;
use scale::*;
use scale::{alloc::Allocate, LoadFromMem, Reveal, StoreInMem};

/// An array datastructure that allocates memory and deallocates when goes out of scope
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
    pub fn addr(&self, i: u64) -> i64 {
        (self.first_element.address + i * T::Allocator::N) as i64
    }
}

impl<T> Slice<T>
where
    T: GetAllocator + LoadFromMem<i64> + Clone,
{
    #[inline(always)]
    pub fn iter(&self) -> impl Iterator<Item = T> + '_ {
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
            inner: T::load_from_mem(self.addr(i * T::size_type())),
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
            inner: T::load_from_mem(self.addr(i * T::size_type())),
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
            inner: T::load_from_mem(self.addr(i * T::size_type())),
            phantom: PhantomData,
        })
    }

    /// Get the mutable reference of an element from the slice without checking about memory overflow
    #[inline(always)]
    pub fn get_mut_unchecked(&'a mut self, i: u64) -> GuardMut<'a, T> {
        GuardMut {
            inner: T::load_from_mem(self.addr(i * T::size_type())),
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
        unsafe { val.store_in_mem(self.addr(i * T::size_type())) }
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
            new_one.set(i, &T::load_from_mem(self.addr(start + i * T::size_type())));
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
            uninit_slice.set(idx, &self.get_unchecked(idx).clone());
        }

        uninit_slice
    }
}

impl<T, const N: u64> Clone for Slice<Array<T, N>>
where
    T: GetAllocator + StoreInMem<i64> + LoadFromMem<i64> + Clone,
{
    fn clone(&self) -> Self {
        let mut new_a: Slice<Array<T, N>> = Slice::uninitialized(self.length);
        for i in 0..self.length {
            let array = self.get_unchecked(i).clone();
            for j in 0..N {
                new_a.get_mut_unchecked(i).set(j, &*array.get_unchecked(j));
            }
        }
        new_a
    }
}

impl Slice<SecretModp> {
    #[inline(always)]
    pub fn private_input( length: u64, player: i64, channel: i64) -> Self {
        let array = Self::uninitialized(length);
        unsafe { __mprivate_input(array.first_element.address as i64, length as i64, player, channel) }
        array
    }

    #[inline(always)]
    pub fn private_output(self, player: i64, channel: i64) { 
        unsafe {
            __mprivate_output(self.first_element.address as i64, self.length as i64, player, channel);
        }
    }
}

/* Operator Versions of Arithmetic Operations */

// a_add_cc_t = &ca_sli + cb_sli.clone();
impl<'a> Add<Slice<ClearModp>> for &'a Slice<ClearModp> {
    type Output = Slice<ClearModp>;
    #[inline(always)]
    fn add(self, other: Slice<ClearModp>) -> Self::Output {
        unsafe {
            __maddc(
                other.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        other
    }
}

// a_add_cc_t = ca_sli.clone + &cb_sli;
impl<'a> Add<&'a Slice<ClearModp>> for Slice<ClearModp> {
    type Output = Slice<ClearModp>;
    #[inline(always)]
    fn add(self, other: &'a Slice<ClearModp>) -> Self::Output {
        unsafe {
            __maddc(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        self
    }
}

// a_add_cc_t = &ca_sli + &cb_sli;
impl<'a> Add<&'a Slice<ClearModp>> for &'a Slice<ClearModp> {
    type Output = Slice<ClearModp>;
    #[inline(always)]
    fn add(self, other: &'a Slice<ClearModp>) -> Self::Output {
        let sliay = Slice::uninitialized(self.length);
        unsafe {
            __maddc(
                sliay.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        sliay
    }
}

// a_add_cc_t = ca_sli.clone() + cb_sli.clone();
impl Add<Slice<ClearModp>> for Slice<ClearModp> {
    type Output = Slice<ClearModp>;
    #[inline(always)]
    fn add(self, other: Slice<ClearModp>) -> Self::Output {
        unsafe {
            __maddc(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        self
    }
}

// a_add_sc_t = &sa_sli + cb_sli.clone();
impl<'a> Add<Slice<ClearModp>> for &'a Slice<SecretModp> {
    type Output = Slice<SecretModp>;
    #[inline(always)]
    fn add(self, other: Slice<ClearModp>) -> Self::Output {
        let sliay = Slice::uninitialized(self.length);
        unsafe {
            __maddm(
                sliay.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        sliay
    }
}

// a_add_sc_t = sa_sli.clone + &cb_sli;
impl<'a> Add<&'a Slice<ClearModp>> for Slice<SecretModp> {
    type Output = Slice<SecretModp>;
    #[inline(always)]
    fn add(self, other: &'a Slice<ClearModp>) -> Self::Output {
        unsafe {
            __maddm(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        self
    }
}

// a_add_sc_t = &sa_sli + &cb_sli;
impl<'a> Add<&'a Slice<ClearModp>> for &'a Slice<SecretModp> {
    type Output = Slice<SecretModp>;
    #[inline(always)]
    fn add(self, other: &'a Slice<ClearModp>) -> Self::Output {
        let sliay = Slice::uninitialized(self.length);
        unsafe {
            __maddm(
                sliay.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        sliay
    }
}

// a_add_sc_t = sa_sli.clone() + cb_sli.clone();
impl Add<Slice<ClearModp>> for Slice<SecretModp> {
    type Output = Slice<SecretModp>;
    #[inline(always)]
    fn add(self, other: Slice<ClearModp>) -> Self::Output {
        unsafe {
            __maddm(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        self
    }
}

// a_add_cs_t = &ca_sli + sb_sli.clone();
impl<'a> Add<Slice<SecretModp>> for &'a Slice<ClearModp> {
    type Output = Slice<SecretModp>;
    #[inline(always)]
    fn add(self, other: Slice<SecretModp>) -> Self::Output {
        unsafe {
            __maddm(
                other.first_element.address as i64,
                other.first_element.address as i64,
                self.first_element.address as i64,
                self.length as i64,
            );
        }
        other
    }
}

// a_add_cs_t = ca_sli.clone + &sb_sli;
impl<'a> Add<&'a Slice<SecretModp>> for Slice<ClearModp> {
    type Output = Slice<SecretModp>;
    #[inline(always)]
    fn add(self, other: &'a Slice<SecretModp>) -> Self::Output {
        let sliay = Slice::uninitialized(self.length);
        unsafe {
            __maddm(
                sliay.first_element.address as i64,
                other.first_element.address as i64,
                self.first_element.address as i64,
                self.length as i64,
            );
        }
        sliay
    }
}

// a_add_cs_t = &ca_sli + &sb_sli;
impl<'a> Add<&'a Slice<SecretModp>> for &'a Slice<ClearModp> {
    type Output = Slice<SecretModp>;
    #[inline(always)]
    fn add(self, other: &'a Slice<SecretModp>) -> Self::Output {
        let sliay = Slice::uninitialized(self.length);
        unsafe {
            __maddm(
                sliay.first_element.address as i64,
                other.first_element.address as i64,
                self.first_element.address as i64,
                self.length as i64,
            );
        }
        sliay
    }
}

// a_add_sc_t = ca_sli.clone() + &sb_sli.clone();
impl Add<Slice<SecretModp>> for Slice<ClearModp> {
    type Output = Slice<SecretModp>;
    #[inline(always)]
    fn add(self, other: Slice<SecretModp>) -> Self::Output {
        let sliay: Slice<SecretModp> = Slice::uninitialized(self.length);
        unsafe {
            __maddm(
                sliay.first_element.address as i64,
                other.first_element.address as i64,
                self.first_element.address as i64,
                self.length as i64,
            );
        }
        sliay
    }
}

// a_add_ss_t = &sa_sli + sb_sli.clone();
impl<'a> Add<Slice<SecretModp>> for &'a Slice<SecretModp> {
    type Output = Slice<SecretModp>;
    #[inline(always)]
    fn add(self, other: Slice<SecretModp>) -> Self::Output {
        unsafe {
            __madds(
                other.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        other
    }
}

// a_add_ss_t = sa_sli.clone + &sb_sli;
impl<'a> Add<&'a Slice<SecretModp>> for Slice<SecretModp> {
    type Output = Self;
    #[inline(always)]
    fn add(self, other: &'a Slice<SecretModp>) -> Self::Output {
        unsafe {
            __madds(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        self
    }
}

// a_add_ss_t = &sa_sli + &sb_sli;
impl<'a> Add<&'a Slice<SecretModp>> for &'a Slice<SecretModp> {
    type Output = Slice<SecretModp>;
    #[inline(always)]
    fn add(self, other: &'a Slice<SecretModp>) -> Self::Output {
        let sliay = Slice::uninitialized(self.length);
        unsafe {
            __madds(
                sliay.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        sliay
    }
}

// a_add_ss_t = sa_sli.clone() + &sb_sli.clone();
impl Add<Slice<SecretModp>> for Slice<SecretModp> {
    type Output = Self;
    #[inline(always)]
    fn add(self, other: Slice<SecretModp>) -> Self::Output {
        unsafe {
            __madds(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        self
    }
}

// a_sub_cc_t = &ca_sli - cb_sli.clone();
impl<'a> Sub<Slice<ClearModp>> for &'a Slice<ClearModp> {
    type Output = Slice<ClearModp>;
    #[inline(always)]
    fn sub(self, other: Slice<ClearModp>) -> Self::Output {
        unsafe {
            __msubc(
                other.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        other
    }
}

// a_sub_cc_t = ca_sli.clone - &cb_sli;
impl<'a> Sub<&'a Slice<ClearModp>> for Slice<ClearModp> {
    type Output = Slice<ClearModp>;
    #[inline(always)]
    fn sub(self, other: &'a Slice<ClearModp>) -> Self::Output {
        unsafe {
            __msubc(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        self
    }
}

// a_sub_cc_t = &ca_sli - &cb_sli;
impl<'a> Sub<&'a Slice<ClearModp>> for &'a Slice<ClearModp> {
    type Output = Slice<ClearModp>;
    #[inline(always)]
    fn sub(self, other: &'a Slice<ClearModp>) -> Self::Output {
        let sliay = Slice::uninitialized(self.length);
        unsafe {
            __msubc(
                sliay.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        sliay
    }
}

// a_sub_cc_t = ca_sli.clone() - cb_sli.clone();
impl Sub<Slice<ClearModp>> for Slice<ClearModp> {
    type Output = Slice<ClearModp>;
    #[inline(always)]
    fn sub(self, other: Slice<ClearModp>) -> Self::Output {
        unsafe {
            __msubc(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        self
    }
}

// a_sub_sc_t = &sa_sli - cb_sli.clone();
impl<'a> Sub<Slice<ClearModp>> for &'a Slice<SecretModp> {
    type Output = Slice<SecretModp>;
    #[inline(always)]
    fn sub(self, other: Slice<ClearModp>) -> Self::Output {
        let sliay = Slice::uninitialized(self.length);
        unsafe {
            __msubml(
                sliay.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        sliay
    }
}

// a_sub_sc_t = sa_sli.clone - &cb_sli;
impl<'a> Sub<&'a Slice<ClearModp>> for Slice<SecretModp> {
    type Output = Slice<SecretModp>;
    #[inline(always)]
    fn sub(self, other: &'a Slice<ClearModp>) -> Self::Output {
        unsafe {
            __msubml(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        self
    }
}

// a_sub_sc_t = &sa_sli - &cb_sli;
impl<'a> Sub<&'a Slice<ClearModp>> for &'a Slice<SecretModp> {
    type Output = Slice<SecretModp>;
    #[inline(always)]
    fn sub(self, other: &'a Slice<ClearModp>) -> Self::Output {
        let sliay = Slice::uninitialized(self.length);
        unsafe {
            __msubml(
                sliay.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        sliay
    }
}

// a_sub_sc_t = sa_sli.clone() - cb_sli.clone();
impl Sub<Slice<ClearModp>> for Slice<SecretModp> {
    type Output = Slice<SecretModp>;
    #[inline(always)]
    fn sub(self, other: Slice<ClearModp>) -> Self::Output {
        unsafe {
            __msubml(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        self
    }
}

// a_sub_cs_t = &ca_sli - sb_sli.clone();
impl<'a> Sub<Slice<SecretModp>> for &'a Slice<ClearModp> {
    type Output = Slice<SecretModp>;
    #[inline(always)]
    fn sub(self, other: Slice<SecretModp>) -> Self::Output {
        unsafe {
            __msubmr(
                other.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        other
    }
}

// a_sub_cs_t = ca_sli.clone - &sb_sli;
impl<'a> Sub<&'a Slice<SecretModp>> for Slice<ClearModp> {
    type Output = Slice<SecretModp>;
    #[inline(always)]
    fn sub(self, other: &'a Slice<SecretModp>) -> Self::Output {
        let sliay = Slice::uninitialized(self.length);
        unsafe {
            __msubmr(
                sliay.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        sliay
    }
}

// a_sub_cs_t = &ca_sli - &sb_sli;
impl<'a> Sub<&'a Slice<SecretModp>> for &'a Slice<ClearModp> {
    type Output = Slice<SecretModp>;
    #[inline(always)]
    fn sub(self, other: &'a Slice<SecretModp>) -> Self::Output {
        let sliay = Slice::uninitialized(self.length);
        unsafe {
            __msubmr(
                sliay.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        sliay
    }
}

// a_sub_sc_t = ca_sli.clone() - &sb_sli.clone();
impl Sub<Slice<SecretModp>> for Slice<ClearModp> {
    type Output = Slice<SecretModp>;
    #[inline(always)]
    fn sub(self, other: Slice<SecretModp>) -> Self::Output {
        let sliay: Slice<SecretModp> = Slice::uninitialized(self.length);
        unsafe {
            __msubmr(
                sliay.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        sliay
    }
}

// a_sub_ss_t = &sa_sli - sb_sli.clone();
impl<'a> Sub<Slice<SecretModp>> for &'a Slice<SecretModp> {
    type Output = Slice<SecretModp>;
    #[inline(always)]
    fn sub(self, other: Slice<SecretModp>) -> Self::Output {
        unsafe {
            __msubs(
                other.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        other
    }
}

// a_sub_ss_t = sa_sli.clone - &sb_sli;
impl<'a> Sub<&'a Slice<SecretModp>> for Slice<SecretModp> {
    type Output = Self;
    #[inline(always)]
    fn sub(self, other: &'a Slice<SecretModp>) -> Self::Output {
        unsafe {
            __msubs(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        self
    }
}

// a_sub_ss_t = &sa_sli - &sb_sli;
impl<'a> Sub<&'a Slice<SecretModp>> for &'a Slice<SecretModp> {
    type Output = Slice<SecretModp>;
    #[inline(always)]
    fn sub(self, other: &'a Slice<SecretModp>) -> Self::Output {
        let sliay = Slice::uninitialized(self.length);
        unsafe {
            __msubs(
                sliay.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        sliay
    }
}

// a_sub_ss_t = sa_sli.clone() - &sb_sli.clone();
impl Sub<Slice<SecretModp>> for Slice<SecretModp> {
    type Output = Self;
    #[inline(always)]
    fn sub(self, other: Slice<SecretModp>) -> Self::Output {
        unsafe {
            __msubs(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        self
    }
}

/* Operator Versions of Arithmetic Operations */

// a_mul_cc_t = &ca_sli * cb_sli.clone();
impl<'a> Mul<Slice<ClearModp>> for &'a Slice<ClearModp> {
    type Output = Slice<ClearModp>;
    #[inline(always)]
    fn mul(self, other: Slice<ClearModp>) -> Self::Output {
        unsafe {
            __mmulc(
                other.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        other
    }
}

// a_mul_cc_t = ca_sli.clone * &cb_sli;
impl<'a> Mul<&'a Slice<ClearModp>> for Slice<ClearModp> {
    type Output = Slice<ClearModp>;
    #[inline(always)]
    fn mul(self, other: &'a Slice<ClearModp>) -> Self::Output {
        unsafe {
            __mmulc(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        self
    }
}

// a_mul_cc_t = &ca_sli * &cb_sli;
impl<'a> Mul<&'a Slice<ClearModp>> for &'a Slice<ClearModp> {
    type Output = Slice<ClearModp>;
    #[inline(always)]
    fn mul(self, other: &'a Slice<ClearModp>) -> Self::Output {
        let sliay = Slice::uninitialized(self.length);
        unsafe {
            __mmulc(
                sliay.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        sliay
    }
}

// a_mul_cc_t = ca_sli.clone() * cb_sli.clone();
impl Mul<Slice<ClearModp>> for Slice<ClearModp> {
    type Output = Slice<ClearModp>;
    #[inline(always)]
    fn mul(self, other: Slice<ClearModp>) -> Self::Output {
        unsafe {
            __mmulc(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        self
    }
}

// a_mul_sc_t = &sa_sli * cb_sli.clone();
impl<'a> Mul<Slice<ClearModp>> for &'a Slice<SecretModp> {
    type Output = Slice<SecretModp>;
    #[inline(always)]
    fn mul(self, other: Slice<ClearModp>) -> Self::Output {
        let sliay = Slice::uninitialized(self.length);
        unsafe {
            __mmulm(
                sliay.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        sliay
    }
}

// a_mul_sc_t = sa_sli.clone * &cb_sli;
impl<'a> Mul<&'a Slice<ClearModp>> for Slice<SecretModp> {
    type Output = Slice<SecretModp>;
    #[inline(always)]
    fn mul(self, other: &'a Slice<ClearModp>) -> Self::Output {
        unsafe {
            __mmulm(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        self
    }
}

// a_mul_sc_t = &sa_sli * &cb_sli;
impl<'a> Mul<&'a Slice<ClearModp>> for &'a Slice<SecretModp> {
    type Output = Slice<SecretModp>;
    #[inline(always)]
    fn mul(self, other: &'a Slice<ClearModp>) -> Self::Output {
        let sliay = Slice::uninitialized(self.length);
        unsafe {
            __mmulm(
                sliay.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        sliay
    }
}

// a_mul_sc_t = sa_sli.clone() * cb_sli.clone();
impl Mul<Slice<ClearModp>> for Slice<SecretModp> {
    type Output = Slice<SecretModp>;
    #[inline(always)]
    fn mul(self, other: Slice<ClearModp>) -> Self::Output {
        unsafe {
            __mmulm(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        self
    }
}

// a_mul_cs_t = &ca_sli * sb_sli.clone();
impl<'a> Mul<Slice<SecretModp>> for &'a Slice<ClearModp> {
    type Output = Slice<SecretModp>;
    #[inline(always)]
    fn mul(self, other: Slice<SecretModp>) -> Self::Output {
        unsafe {
            __mmulm(
                other.first_element.address as i64,
                other.first_element.address as i64,
                self.first_element.address as i64,
                self.length as i64,
            );
        }
        other
    }
}

// a_mul_cs_t = ca_sli.clone * &sb_sli;
impl<'a> Mul<&'a Slice<SecretModp>> for Slice<ClearModp> {
    type Output = Slice<SecretModp>;
    #[inline(always)]
    fn mul(self, other: &'a Slice<SecretModp>) -> Self::Output {
        let sliay = Slice::uninitialized(self.length);
        unsafe {
            __mmulm(
                sliay.first_element.address as i64,
                other.first_element.address as i64,
                self.first_element.address as i64,
                self.length as i64,
            );
        }
        sliay
    }
}

// a_mul_cs_t = &ca_sli * &sb_sli;
impl<'a> Mul<&'a Slice<SecretModp>> for &'a Slice<ClearModp> {
    type Output = Slice<SecretModp>;
    #[inline(always)]
    fn mul(self, other: &'a Slice<SecretModp>) -> Self::Output {
        let sliay = Slice::uninitialized(self.length);
        unsafe {
            __mmulm(
                sliay.first_element.address as i64,
                other.first_element.address as i64,
                self.first_element.address as i64,
                self.length as i64,
            );
        }
        sliay
    }
}

// a_mul_sc_t = ca_sli.clone() * &sb_sli.clone();
impl Mul<Slice<SecretModp>> for Slice<ClearModp> {
    type Output = Slice<SecretModp>;
    #[inline(always)]
    fn mul(self, other: Slice<SecretModp>) -> Self::Output {
        let sliay: Slice<SecretModp> = Slice::uninitialized(self.length);
        unsafe {
            __mmulm(
                sliay.first_element.address as i64,
                other.first_element.address as i64,
                self.first_element.address as i64,
                self.length as i64,
            );
        }
        sliay
    }
}

// a_div_cc_t = &ca_sli / cb_sli.clone();
impl<'a> Div<Slice<ClearModp>> for &'a Slice<ClearModp> {
    type Output = Slice<ClearModp>;
    #[inline(always)]
    fn div(self, other: Slice<ClearModp>) -> Self::Output {
        unsafe {
            __mdivc(
                other.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        other
    }
}

// a_mod_cc_t = ca_sli.clone / &cb_sli;
impl<'a> Div<&'a Slice<ClearModp>> for Slice<ClearModp> {
    type Output = Slice<ClearModp>;
    #[inline(always)]
    fn div(self, other: &'a Slice<ClearModp>) -> Self::Output {
        unsafe {
            __mdivc(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        self
    }
}

// a_mod_cc_t = &ca_sli / &cb_sli;
impl<'a> Div<&'a Slice<ClearModp>> for &'a Slice<ClearModp> {
    type Output = Slice<ClearModp>;
    #[inline(always)]
    fn div(self, other: &'a Slice<ClearModp>) -> Self::Output {
        let sliay = Slice::uninitialized(self.length);
        unsafe {
            __mdivc(
                sliay.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        sliay
    }
}

// a_mod_cc_t = ca_sli.clone() / cb_sli.clone();
impl Div<Slice<ClearModp>> for Slice<ClearModp> {
    type Output = Slice<ClearModp>;
    #[inline(always)]
    fn div(self, other: Slice<ClearModp>) -> Self::Output {
        unsafe {
            __mdivc(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        self
    }
}

// a_mod_cc_t = &ca_sli % cb_sli.clone();
impl<'a> Rem<Slice<ClearModp>> for &'a Slice<ClearModp> {
    type Output = Slice<ClearModp>;
    #[inline(always)]
    fn rem(self, other: Slice<ClearModp>) -> Self::Output {
        unsafe {
            __mmodc(
                other.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        other
    }
}

// a_mod_cc_t = ca_sli.clone % &cb_sli;
impl<'a> Rem<&'a Slice<ClearModp>> for Slice<ClearModp> {
    type Output = Slice<ClearModp>;
    #[inline(always)]
    fn rem(self, other: &'a Slice<ClearModp>) -> Self::Output {
        unsafe {
            __mmodc(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        self
    }
}

// a_mod_cc_t = &ca_sli % &cb_sli;
impl<'a> Rem<&'a Slice<ClearModp>> for &'a Slice<ClearModp> {
    type Output = Slice<ClearModp>;
    #[inline(always)]
    fn rem(self, other: &'a Slice<ClearModp>) -> Self::Output {
        let sliay = Slice::uninitialized(self.length);
        unsafe {
            __mmodc(
                sliay.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        sliay
    }
}

// a_mod_cc_t = ca_sli.clone() % cb_sli.clone();
impl Rem<Slice<ClearModp>> for Slice<ClearModp> {
    type Output = Slice<ClearModp>;
    #[inline(always)]
    fn rem(self, other: Slice<ClearModp>) -> Self::Output {
        unsafe {
            __mmodc(
                self.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        self
    }
}

/* Member function versions of artihemtic operations */

impl Slice<ClearModp> {
    #[inline(always)]
    pub fn add_clear(&self, other: &Slice<ClearModp>) -> Slice<ClearModp> {
        let array = Self::uninitialized(self.length);
        unsafe {
            __maddc(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        array
    }
    #[inline(always)]
    pub fn add_secret(&self, other: &Slice<SecretModp>) -> Slice<SecretModp> {
        let array: Slice<SecretModp> = Slice::uninitialized(self.length);
        unsafe {
            __maddm(
                array.first_element.address as i64,
                other.first_element.address as i64,
                self.first_element.address as i64,
                self.length as i64,
            );
        }
        array
    }
    #[inline(always)]
    pub fn sub_clear(&self, other: &Slice<ClearModp>) -> Slice<ClearModp> {
        let array = Self::uninitialized(self.length);
        unsafe {
            __msubc(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        array
    }
    #[inline(always)]
    pub fn sub_secret(&self, other: &Slice<SecretModp>) -> Slice<SecretModp> {
        let array: Slice<SecretModp> = Slice::uninitialized(self.length);
        unsafe {
            __msubmr(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        array
    }
    #[inline(always)]
    pub fn mul_clear(&self, other: &Slice<ClearModp>) -> Slice<ClearModp> {
        let array = Self::uninitialized(self.length);
        unsafe {
            __mmulc(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        array
    }
    #[inline(always)]
    pub fn mul_secret(&self, other: &Slice<SecretModp>) -> Slice<SecretModp> {
        let array: Slice<SecretModp> = Slice::uninitialized(self.length);
        unsafe {
            __mmulm(
                array.first_element.address as i64,
                other.first_element.address as i64,
                self.first_element.address as i64,
                self.length as i64,
            );
        }
        array
    }
    #[inline(always)]
    pub fn div_clear(&self, other: &Slice<ClearModp>) -> Slice<ClearModp> {
        let array = Self::uninitialized(self.length);
        unsafe {
            __mdivc(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        array
    }
    #[inline(always)]
    pub fn mod_clear(&self, other: &Slice<ClearModp>) -> Slice<ClearModp> {
        let array = Self::uninitialized(self.length);
        unsafe {
            __mmodc(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        array
    }
}

impl Slice<SecretModp> {
    #[inline(always)]
    pub fn add_clear(&self, other: &Slice<ClearModp>) -> Slice<SecretModp> {
        let array = Self::uninitialized(self.length);
        unsafe {
            __maddm(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        array
    }
    #[inline(always)]
    pub fn add_secret(&self, other: &Slice<SecretModp>) -> Slice<SecretModp> {
        let array = Self::uninitialized(self.length);
        unsafe {
            __madds(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        array
    }
    #[inline(always)]
    pub fn sub_clear(&self, other: &Slice<ClearModp>) -> Slice<SecretModp> {
        let array = Self::uninitialized(self.length);
        unsafe {
            __msubml(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        array
    }
    #[inline(always)]
    pub fn sub_secret(&self, other: &Slice<SecretModp>) -> Slice<SecretModp> {
        let array = Self::uninitialized(self.length);
        unsafe {
            __msubs(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        array
    }
    #[inline(always)]
    pub fn mul_clear(&self, other: &Slice<ClearModp>) -> Slice<SecretModp> {
        let array = Self::uninitialized(self.length);
        unsafe {
            __mmulm(
                array.first_element.address as i64,
                self.first_element.address as i64,
                other.first_element.address as i64,
                self.length as i64,
            );
        }
        array
    }
}

// Reverse a slice

impl Slice<ClearModp> {
    #[inline(always)]
    pub fn reverse(&self) -> Slice<ClearModp> {
        let array = Self::uninitialized(self.length);
        unsafe {
            __mrevc(
                array.first_element.address as i64,
                self.first_element.address as i64,
                self.length as i64,
            );
        }
        array
    }
}

impl Slice<SecretModp> {
    #[inline(always)]
    pub fn reverse(&self) -> Slice<SecretModp> {
        let array = Self::uninitialized(self.length);
        unsafe {
            __mrevs(
                array.first_element.address as i64,
                self.first_element.address as i64,
                self.length as i64,
            );
        }
        array
    }
}

// Evaluate a polynomial represented by an slice at a value x
//
impl Slice<ClearModp> {
    #[inline(always)]
    pub fn evaluate(&self, x: ClearModp) -> ClearModp {
        unsafe { __mevalcc(self.first_element.address as i64, self.length as i64, x) }
    }
}

impl Slice<SecretModp> {
    #[inline(always)]
    pub fn evaluate(&self, x: ClearModp) -> SecretModp {
        unsafe { __mevalsc(self.first_element.address as i64, self.length as i64, x) }
    }
}

impl Slice<ClearModp> {
    #[inline(always)]
    #[allow(non_snake_case)]
    pub fn bit_decomposition_ClearModp(val: ClearModp, length: u64) -> Self {
        let array = Self::uninitialized(length);
        unsafe { __mbitdecc(array.first_element.address as i64, val, length as i64) }
        array
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    pub fn bit_decomposition_ClearModp_Signed(val: ClearModp, length: u64) -> Self {
        let array = Self::uninitialized(length);
        unsafe { __mbitdeccs(array.first_element.address as i64, val, length as i64) }
        array
    }
}

impl Slice<i64> {
    #[inline(always)]
    #[allow(non_snake_case)]
    pub fn bit_decomposition_i64(val: i64, length: u64) -> Self {
        let array = Self::uninitialized(length);
        unsafe { __mbitdecint(array.first_element.address as i64, val, length as i64) }
        array
    }
}
