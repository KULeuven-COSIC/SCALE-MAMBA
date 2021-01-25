use crate::{ClearModp, SecretI64, SecretModp};
use core::sync::atomic::{AtomicU64, Ordering};

static SECRET_I64_ALLOCATOR: Allocator<SecretI64> = Allocator::new();
static SECRET_MODP_ALLOCATOR: Allocator<SecretModp> = Allocator::new();
static CLEAR_MODP_ALLOCATOR: Allocator<ClearModp> = Allocator::new();
static I64_ALLOCATOR: Allocator<i64> = Allocator::new();

pub struct Allocator<T> {
    next_free_address: AtomicU64,
    // Act like we contain `T`
    phantom: core::marker::PhantomData<T>,
}

impl<T> Allocator<T> {
    const fn new() -> Self {
        Self {
            next_free_address: AtomicU64::new(0),
            phantom: core::marker::PhantomData,
        }
    }
}

pub trait Allocate {
    /// This constant states how many elements will get allocated for each element
    /// given to `free`.
    const N: u64;
    unsafe fn free(&self, n: u64);
    fn allocate(&self, n: u64) -> u64;
}

impl<T> Allocate for &'static Allocator<T> {
    const N: u64 = 1;

    #[inline(always)]
    unsafe fn free(&self, n: u64) {
        self.next_free_address.fetch_add(n, Ordering::Relaxed);
    }

    #[inline(always)]
    fn allocate(&self, n: u64) -> u64 {
        let val = self
            .next_free_address
            .fetch_sub(n as u64, Ordering::Relaxed);
        // Check that we did not use more memory than selected via the `*_MEMORY` constants
        match val.checked_sub(n) {
            None => crate::panic!(
                "only ",
                val as i64,
                " memory left in ",
                core::any::type_name::<T>(),
                ", but requested ",
                n as i64
            ),
            Some(addr) => addr,
        }
    }
}

pub trait GetAllocator: Sized + 'static {
    type Allocator: Allocate;
    fn get_allocator() -> Self::Allocator;
}

impl GetAllocator for SecretI64 {
    type Allocator = &'static Allocator<Self>;
    fn get_allocator() -> &'static Allocator<Self> {
        &SECRET_I64_ALLOCATOR
    }
}

impl GetAllocator for i64 {
    type Allocator = &'static Allocator<Self>;
    fn get_allocator() -> &'static Allocator<Self> {
        &I64_ALLOCATOR
    }
}

impl GetAllocator for ClearModp {
    type Allocator = &'static Allocator<Self>;
    fn get_allocator() -> &'static Allocator<Self> {
        &CLEAR_MODP_ALLOCATOR
    }
}

impl GetAllocator for SecretModp {
    type Allocator = &'static Allocator<Self>;
    fn get_allocator() -> &'static Allocator<Self> {
        &SECRET_MODP_ALLOCATOR
    }
}
