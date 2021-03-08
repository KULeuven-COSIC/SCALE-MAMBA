// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

pub trait CompileTimeLengthIterator<const N: u64> {}

impl<I, const N: u64> CompileTimeLengthIterator<N> for Iter<I, N> {}

impl<I: CompileTimeLengthIterator<N>, const N: u64> CompileTimeLengthIterator<N>
    for core::iter::Rev<I>
{
}

pub struct Iter<I, const N: u64> {
    iter: I,
}

impl<I: Iterator, const N: u64> Iter<I, N> {
    pub unsafe fn new(iter: I) -> Self {
        Self { iter }
    }
}

impl<I: Iterator, const N: u64> Iterator for Iter<I, N> {
    type Item = I::Item;
    fn next(&mut self) -> Option<I::Item> {
        self.iter.next()
    }
}

impl<I: Iterator + DoubleEndedIterator, const N: u64> DoubleEndedIterator for Iter<I, N> {
    fn next_back(&mut self) -> Option<I::Item> {
        self.iter.next_back()
    }
}
