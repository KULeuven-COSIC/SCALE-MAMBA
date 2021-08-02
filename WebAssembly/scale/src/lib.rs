// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![cfg_attr(not(feature = "emulate"), no_std)]
#![cfg_attr(not(feature = "emulate"), feature(wasm_simd, simd_ffi))]

#[cfg(feature = "emulate")]
pub use std::process::exit;

scale_impl_generator::impls!();

#[cfg(not(feature = "emulate"))]
mod types {
    #[repr(transparent)]
    #[derive(Copy, Clone)]
    pub struct ClearModp(pub core::arch::wasm32::v128);
    #[repr(transparent)]
    #[derive(Copy, Clone)]
    pub struct SecretModp(f32);
    #[repr(transparent)]
    #[derive(Copy, Clone)]
    pub struct SecretI64(f64);
    #[repr(transparent)]
    #[derive(Copy, Clone)]
    pub struct SecretBit(pub(crate) SecretI64);
    #[repr(transparent)]
    #[derive(Copy, Clone)]
    pub struct RawSecretBit(pub(crate) f64);
}

#[cfg(feature = "emulate")]
mod types {
    #[repr(transparent)]
    #[derive(Copy, Clone)]
    pub struct ClearModp(pub(crate) i64);
    #[repr(transparent)]
    #[derive(Copy, Clone)]
    pub struct SecretModp(pub(crate) ClearModp);
    #[repr(transparent)]
    #[derive(Copy, Clone)]
    pub struct SecretI64(pub(crate) i64);
    #[repr(transparent)]
    #[derive(Copy, Clone)]
    // FIXME: use `bool` as the field
    pub struct SecretBit(pub(crate) i64);
    // Due to the fact that our rust -> wasm -> scasm pipeline
    // does not support `externref` yet, so we can't properly
    // encode more than 4 different register types, we need
    // to have this hacky datastructure that the raw asm
    // instructions use.
    #[repr(transparent)]
    #[derive(Copy, Clone)]
    pub struct RawSecretBit(pub(crate) i64);
}

pub use types::*;

macro_rules! any_register {
    ($($bound:path),*) => {
        /// A helper trait that is implemented for all register types
        /// and has bounds for all operations implemented by those types.
        /// This allows one to easily write code that is generic over
        /// the various register types.
        pub trait AnyRegister: $($bound+)* Sized {}
    };
}

impl AnyRegister for ClearModp {}
impl AnyRegister for SecretModp {}
impl AnyRegister for i64 {}
impl AnyRegister for SecretI64 {}

use core::ops::*;

any_register! {
    Stack,
    alloc::GetAllocator,
    Add<Output = Self>,
    Sub<Output = Self>,
    Mul<Output = Self>,
    StoreInMem<i64>,
    LoadFromMem<i64>
}

pub trait InnerModp:
    AnyRegister
    + From<ClearModp>
    + Into<SecretModp>
    + RevealIfSecret
    + Mul<ClearModp, Output = Self>
    + Add<ClearModp, Output = Self>
    + Sub<ClearModp, Output = Self>
    + Mul<SecretModp, Output = SecretModp>
    + Add<SecretModp, Output = SecretModp>
    + Sub<SecretModp, Output = SecretModp>
{
}

impl InnerModp for ClearModp {}
impl InnerModp for SecretModp {}

pub trait Modp<Other: InnerModp>:
    InnerModp
    + Mul<Other, Output = SecretModp>
    + Add<Other, Output = SecretModp>
    + Sub<Other, Output = SecretModp>
{
}

impl Modp<ClearModp> for SecretModp {}
impl Modp<SecretModp> for ClearModp {}
impl Modp<SecretModp> for SecretModp {}

pub trait StoreInMem<Index> {
    /// Internal operation for writing to memory. If you want
    /// to use memory, consider using one of the safe wrappers like
    /// `Array`, `Slice` or `Box`.
    unsafe fn store_in_mem(&self, idx: Index);
}

pub trait LoadFromMem<Index> {
    fn load_from_mem(idx: Index) -> Self;
}

impl<T: StoreInMem<i64>> StoreInMem<(i64, i64)> for Option<T> {
    unsafe fn store_in_mem(&self, idk: (i64, i64)) {
        match self {
            None => 0_i64.store_in_mem(idk.0),
            Some(a) => {
                1_i64.store_in_mem(idk.0);
                a.store_in_mem(idk.1);
            }
        };
    }
}

impl<T: LoadFromMem<i64>> LoadFromMem<(i64, i64)> for Option<T> {
    fn load_from_mem(idk: (i64, i64)) -> Option<T> {
        let ind = i64::load_from_mem(idk.0);
        match ind {
            1 => Some(T::load_from_mem(idk.1)),
            _ => None,
        }
    }
}

mod stack_address;

#[repr(transparent)]
#[derive(Copy, Clone)]
pub struct StackAddress(i64);

pub trait Stack {
    /// Push a new item to the stack
    ///
    /// SAFETY: this function is unsafe, because runtime function calls also
    /// generate these instructions, and thus modifying the stack must be done
    /// very carefully.
    unsafe fn push(val: &Self);
    /// Read and remove the top item from the stack
    ///
    /// SAFETY: this function is unsafe, because runtime function calls also
    /// generate these instructions, and thus modifying the stack must be done
    /// very carefully.
    unsafe fn pop() -> Self;
    /// Read an item at the given stack address
    unsafe fn peek(addr: StackAddress) -> Self;
    /// Replace the item at the given stack address
    ///
    /// SAFETY: this function is unsafe, because runtime function calls also
    /// generate these instructions, and thus modifying the stack must be done
    /// very carefully.
    unsafe fn poke(addr: StackAddress, val: &Self);
    /// Get the stack address of the top most element on the stack
    fn get_stack_pointer() -> StackAddress;
    /// Get the value at the stack address "stackpointer - offset"
    unsafe fn peek_from_top(offset: i64) -> Self;
    /// Overwrite the value at the stack address "stackpointer - offset"
    unsafe fn poke_from_top(offset: i64, val: &Self);
}

use core::mem::MaybeUninit;

impl<T: alloc::GetAllocator + Stack, const N: usize> Stack for [T; N] {
    #[inline(always)]
    unsafe fn push(array: &Self) {
        for val in array.iter() {
            T::push(&val);
        }
    }
    #[inline(always)]
    unsafe fn pop() -> Self {
        let mut array: [MaybeUninit<T>; N] = MaybeUninit::uninit().assume_init();
        for i in (0..N).rev() {
            array[i] = MaybeUninit::new(T::pop());
        }
        core::mem::transmute_copy(&array)
    }

    #[inline(always)]
    unsafe fn peek(addr: StackAddress) -> Self {
        let mut array: [MaybeUninit<T>; N] = MaybeUninit::uninit().assume_init();
        for i in 0..N {
            array[i] = MaybeUninit::new(T::peek(addr + i as _));
        }
        core::mem::transmute_copy(&array)
    }

    #[inline(always)]
    unsafe fn poke(addr: StackAddress, array: &Self) {
        for (i, val) in array.iter().enumerate() {
            T::poke(addr + i as _, &val);
        }
    }

    #[inline(always)]
    unsafe fn peek_from_top(offset: i64) -> Self {
        let mut array: [MaybeUninit<T>; N] = MaybeUninit::uninit().assume_init();
        for i in 0..N {
            array[i] = MaybeUninit::new(T::peek_from_top(offset + i as i64));
        }
        core::mem::transmute_copy(&array)
    }

    #[inline(always)]
    unsafe fn poke_from_top(offset: i64, array: &Self) {
        for (i, val) in array.iter().enumerate() {
            T::poke_from_top(offset + i as i64, &val);
        }
    }

    #[inline(always)]
    fn get_stack_pointer() -> StackAddress {
        panic!("this operation makes no sense")
    }
}

pub trait Reveal {
    type Output;
    fn reveal(&self) -> Self::Output;
}

pub trait RevealIfSecret {
    type Output;
    fn reveal_if_secret(&self) -> Self::Output;
}

mod clear_modp;
mod i64;
mod io;
mod print;
mod secret_bit;
mod secret_i64;
mod secret_modp;
mod system;

pub use io::*;
pub use print::*;
pub use system::*;

#[cfg(not(feature = "emulate"))]
extern "Rust" {
    fn __black_box(i: i64) -> i64;
    /// This is a helper doing `startopen` and `stopopen` at the same time for a single register.
    /// We don't have a setup for passing arrays to assembly yet, so we do this for now.
    pub fn reveal(s: SecretModp) -> ClearModp;
    /// A helper for multi-return-value instructions to extract the next instruction
    fn pop_secret_modp() -> SecretModp;
    /// A helper for multi-return-value instructions to extract the next instruction
    fn pop_secret_bit() -> RawSecretBit;
    /// A helper for multi-return-value instructions to extract the next instruction
    fn pop_secret_i64() -> SecretI64;
    // A helper for bit operations [again here only do one at a time for now]
    //   - Let the assembler to the merging
    fn __bit() -> SecretModp;
}

#[inline(always)]
#[cfg(not(feature = "emulate"))]
pub fn __triple() -> (SecretModp, SecretModp, SecretModp) {
    extern "C" {
        fn __triple();
    }
    unsafe {
        __triple();
        (pop_secret_modp(), pop_secret_modp(), pop_secret_modp())
    }
}

#[inline(always)]
#[cfg(not(feature = "emulate"))]
pub fn __square() -> (SecretModp, SecretModp) {
    extern "C" {
        fn __square();
    }
    unsafe {
        __square();
        (pop_secret_modp(), pop_secret_modp())
    }
}

/// We reserve 1000 memory entries for the testing data
/// This is done in the first instructions, so we know
/// testing data is allocated from position 0 to 999
const TEST_MEMORY_OFFSET: i64 = 0;

#[cfg(feature = "emulate")]
#[inline(never)]
pub fn black_box(i: i64) -> i64 {
    i
}

#[cfg(not(feature = "emulate"))]
#[inline(always)]
/// This function prevents wasm-opt from optimizing out operations on the value
/// if said value is constant or dead code.
pub fn black_box(i: i64) -> i64 {
    unsafe { __black_box(i) }
}

#[cfg(feature = "emulate")]
#[inline(never)]
pub unsafe fn reveal(s: SecretModp) -> ClearModp {
    s.0
}

#[panic_handler]
#[cfg(not(feature = "emulate"))]
fn panic_handler(_: &core::panic::PanicInfo) -> ! {
    /*
    // FIXME: this requires some fancy memory operations to work
    if let Some(loc) = info.location() {
        print!("panic in line ", i64::from(loc.line()));
    }*/
    unsafe { __crash() }
}

#[cfg(feature = "emulate")]
mod emulated_impls;

#[cfg(feature = "emulate")]
pub use emulated_impls::__mul2sint;

#[cfg(feature = "emulate")]
use num_bigint::BigInt;

#[cfg(feature = "emulate")]
// __triple and __square are here, because they have multiple return values and this isn't supported properly
// in our wasm transpiler. Look at the code generator for these to see how wasm handles it.
pub fn __triple() -> (SecretModp, SecretModp, SecretModp) {
    let value: BigInt = From::from(1);
    let s = SecretModp(value.into());
    (s, s, s)
}

#[cfg(feature = "emulate")]
pub fn __square() -> (SecretModp, SecretModp) {
    let value: BigInt = From::from(1);
    let s = SecretModp(value.into());
    (s, s)
}

#[cfg(feature = "emulate")]
pub unsafe fn __bit() -> SecretModp {
    SecretModp::from(0)
}

pub trait Test {
    #[track_caller]
    fn test(self);
}

pub trait TestValue {
    #[track_caller]
    fn test_value(self, val: Self);
}

// Note test_mem is never implemented for secret types
//   - This makes things a bit simpler
pub trait TestMem {
    #[track_caller]
    fn test_mem<const I: u32>(self, t_location: ConstU32<I>);
}

pub trait Output {
    fn output(self, ch: i64);
}

pub trait Input {
    fn input(ch: i64) -> Self;
}

#[cfg(not(feature = "emulate"))]
mod testing;

#[cfg(feature = "emulate")]
mod testing_emulated;

pub mod alloc;

pub use scale_impl_generator::main;

#[doc(hidden)]
#[macro_export]
macro_rules! __main {
    (KAPPA = $kappa:expr;) => {
        use $crate::print;
        use $crate::*;
        type SecretInteger<const K: u64> = scale_std::integer::SecretInteger<K, $kappa>;
        type SecretFixed<const K: u64, const F: u64> =
            scale_std::fixed_point::SecretFixed<K, F, $kappa>;
        type SecretFloat<const V: u64, const P: u64> =
            scale_std::floating_point::SecretFloat<V, P, $kappa, true>;
        type ClearFloat<const V: u64, const P: u64> =
            scale_std::floating_point::ClearFloat<V,P,true>;
        #[cfg(not(test))]
        mod helper {
            #[no_mangle]
            fn main() {
                extern "Rust" {
                    fn init_wasm_heap_memory();
                }
                unsafe {
                    init_wasm_heap_memory();
                }
                super::main();
                #[cfg(feature = "emulate")]
                $crate::exit(0);
            }
        }
    };
    (KAPPA = $kappa:expr; DETECT_OVERFLOW = $detect_overflow:expr) => {
        use $crate::print;
        use $crate::*;
        type SecretInteger<const K: u64> = scale_std::integer::SecretInteger<K, $kappa>;
        type SecretFixed<const K: u64, const F: u64> =
            scale_std::fixed_point::SecretFixed<K, F, $kappa>;
        type SecretFloat<const V: u64, const P: u64> =
            scale_std::floating_point::SecretFloat<V, P, $kappa, $detect_overflow>;
        type ClearFloat<const V: u64, const P: u64> =
            scale_std::floating_point::ClearFloat<V,P,$detect_overflow>
        #[cfg(not(test))]
        mod helper {
            #[no_mangle]
            fn main() {
                extern "Rust" {
                    fn init_wasm_heap_memory();
                }
                unsafe {
                    init_wasm_heap_memory();
                }
                super::main();
                #[cfg(feature = "emulate")]
                $crate::exit(0);
            }
        }
    };
}

#[macro_export]
macro_rules! panic {
    ($($tt:tt)*) => {{
        $crate::print!($($tt)*);
        unsafe { $crate::__crash() }
    }};
}

#[macro_export]
macro_rules! assert {
    ($e:expr) => {
        if !$e {
            $crate::panic!("assertion failed: ", stringify!($e));
        }
    };
}

/* Comparison outputting a A */
pub trait ScaleCmp<Other, A> {
    fn lt(self, other: Other) -> A;
    fn le(self, other: Other) -> A;
    fn gt(self, other: Other) -> A;
    fn ge(self, other: Other) -> A;
    fn eq(self, other: Other) -> A;
    fn ne(self, other: Other) -> A;
}

pub trait ScaleCmpZ<A> {
    fn ltz(self) -> A;
    fn lez(self) -> A;
    fn gtz(self) -> A;
    fn gez(self) -> A;
    fn eqz(self) -> A;
    fn nez(self) -> A;
}

pub trait Rand {
    fn rand(self) -> i64;
}

pub trait Randomize {
    fn randomize() -> Self;
}

#[macro_export]
macro_rules! execute_garbled_circuit {
    ($id:ident($($arg:expr),*) -> $($ret:ty),*) => {{
        $(Stack::push(&$arg);)*
        execute_garbled_circuit(ConstU32::<$id>);
        ($(<$ret>::pop()),*)
    }};
}

#[macro_export]
macro_rules! execute_local_function {
    ($id:ident($($arg:expr),*) -> $($ret:ty),*) => {{
        $(Stack::push(&$arg);)*
        execute_local_function(ConstU32::<$id>);
        ($(<$ret>::pop()),*)
    }};
}

#[derive(Copy, Clone)]
pub struct ConstU32<const U: u32>;

#[derive(Copy, Clone)]
pub struct ConstU64<const U: u64>;

#[derive(Copy, Clone)]
pub struct ConstBool<const U: bool>;

#[derive(Copy, Clone)]
pub struct ConstI32<const I: i32>;

/// Internal helper to map user-visible types
/// to internal types.
trait AssemblyType {
    type Type;
    fn to_asm(self) -> Self::Type;
}

impl AssemblyType for i64 {
    type Type = i64;

    #[inline(always)]
    fn to_asm(self) -> Self::Type {
        self
    }
}

impl AssemblyType for ClearModp {
    type Type = ClearModp;

    #[inline(always)]
    fn to_asm(self) -> Self::Type {
        self
    }
}

impl AssemblyType for SecretModp {
    type Type = SecretModp;

    #[inline(always)]
    fn to_asm(self) -> Self::Type {
        self
    }
}

impl AssemblyType for SecretBit {
    type Type = SecretBit;

    #[inline(always)]
    fn to_asm(self) -> Self::Type {
        self
    }
}

impl AssemblyType for SecretI64 {
    type Type = SecretI64;

    #[inline(always)]
    fn to_asm(self) -> Self::Type {
        self
    }
}


impl<const I: i32> AssemblyType for ConstI32<I> {
    type Type = i32;

    #[inline(always)]
    fn to_asm(self) -> Self::Type {
        I
    }
}

impl<const I: u32> AssemblyType for ConstU32<I> {
    type Type = u32;

    #[inline(always)]
    fn to_asm(self) -> Self::Type {
        I
    }
}

impl<const I: u64> AssemblyType for ConstU64<I> {
    type Type = u64;

    #[inline(always)]
    fn to_asm(self) -> Self::Type {
        I
    }
}

impl<const I: bool> AssemblyType for ConstBool<I> {
    type Type = bool;

    #[inline(always)]
    fn to_asm(self) -> Self::Type {
        I
    }
}


impl AssemblyType for Never {
    type Type = Never;
    #[inline(always)]
    fn to_asm(self) -> Self::Type {
        self
    }
}

type Never = <F as HasOutput>::Output;
/// Helper type giving us access to the `!` type on the stable compiler
pub trait HasOutput {
    type Output;
}

impl<O> HasOutput for fn() -> O {
    type Output = O;
}

type F = fn() -> !;
