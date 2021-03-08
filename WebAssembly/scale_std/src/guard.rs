use core::marker::PhantomData;
use scale::alloc::GetAllocator;
use scale::Print;

pub struct Guard<'a, T>
where
    T: GetAllocator,
{
    pub(crate) phantom: PhantomData<&'a T>,
    pub(crate) inner: T,
}

impl<'a, T> core::ops::Deref for Guard<'a, T>
where
    T: GetAllocator,
{
    type Target = T;

    fn deref(&self) -> &Self::Target {
        &self.inner
    }
}

pub struct GuardMut<'a, T>
where
    T: GetAllocator,
{
    pub(crate) phantom: PhantomData<&'a mut T>,
    pub(crate) inner: T,
}

impl<'a, T> core::ops::Deref for GuardMut<'a, T>
where
    T: GetAllocator,
{
    type Target = T;

    fn deref(&self) -> &Self::Target {
        &self.inner
    }
}

impl<'a, T> core::ops::DerefMut for GuardMut<'a, T>
where
    T: GetAllocator,
{
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.inner
    }
}

impl<'a, T> Print for Guard<'a, T>
where
    T: GetAllocator + Print,
{
    #[inline(always)]
    fn print(self) {
        self.inner.print();
    }
}

impl<'a, T> Print for GuardMut<'a, T>
where
    T: GetAllocator + Print,
{
    #[inline(always)]
    fn print(self) {
        self.inner.print();
    }
}
