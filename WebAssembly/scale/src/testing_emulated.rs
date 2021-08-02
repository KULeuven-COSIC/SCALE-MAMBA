// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use crate::{
    ClearModp, ConstU32, LoadFromMem, Reveal, SecretBit, SecretI64, SecretModp, Test, TestMem,
    TestValue, BYTE_LENGTH, TEST_MEMORY_OFFSET,
};
use num_bigint::{BigInt, Sign};
use num_integer::Integer;
use num_traits::{identities::One, Signed};
use std::convert::{TryFrom, TryInto};
use std::str::FromStr;

struct Memory {
    clear: Vec<u8>,
    regint: Vec<u8>,
}

#[no_mangle]
fn init_wasm_heap_memory() {}

fn bits_to_bignum(bits: &[bool]) -> BigInt {
    let bytes = bits
        .chunks(8)
        .map(|byte| {
            byte.iter()
                .copied()
                .rev()
                .enumerate()
                .fold(0, |sum, (i, b)| sum + ((b as u8) << i))
        })
        .collect::<Vec<u8>>();
    BigInt::from_bytes_be(Sign::Plus, &bytes)
}

lazy_static::lazy_static! {
    pub static ref P: BigInt = bits_to_bignum(crate::P);
    static ref R: BigInt = {
        // R = 2 ** (8 * byte_length) % p
        BigInt::from(2).pow(8 * u32::try_from(BYTE_LENGTH).unwrap()) % &*P
    };

    static ref R_INV: BigInt = {
        mod_inverse(&*R, &*P).unwrap_or_default()
    };
    static ref MEMORY: Memory = {
        let file = std::fs::read("Data/Memory-P0").unwrap();
        let mut file = file.splitn(2, |&c| c == b'\n');
        let size_c = file.next().unwrap();
        let size_c = usize::from_str(std::str::from_utf8(size_c).unwrap()).unwrap();
        let base = file.next().unwrap();
        let (clear, file) = base.split_at(size_c * BYTE_LENGTH as usize);
        let clear = clear.to_vec();
        let mut file = file.splitn(2, |&c| c == b'\n');
        let size_s = file.next().unwrap();
        let s = std::str::from_utf8(size_s).unwrap();
        let _size_s = usize::from_str(s).unwrap_or_else(|_| panic!("not a number: {}", s));
        let regint = file.next().unwrap().to_vec();
        Memory {
            clear,
            regint,
        }
    };
}

// Taken from
// https://github.com/dignifiedquire/num-bigint/blob/6f17ec8059c7b748ada4f015eddde993a8938033/src/algorithms/mod_inverse.rs#L11
pub fn mod_inverse(g: &BigInt, n: &BigInt) -> Option<BigInt> {
    let egcd = g.extended_gcd_lcm(n).0;
    let d = egcd.gcd;
    let x = egcd.x;

    if !d.is_one() {
        return None;
    }

    if x.is_negative() {
        Some(x + n)
    } else {
        Some(x)
    }
}

pub fn read_int(address: i64) -> i64 {
    let address = address as usize;
    let bytes = &MEMORY.regint[address * 8..][..8];
    i64::from_le_bytes(bytes.try_into().unwrap())
}

pub fn read_clear(address: i64) -> ClearModp {
    let address = address as usize * BYTE_LENGTH as usize;
    let clear = &MEMORY.clear[address..];
    let mut n = BigInt::default();
    for i in 0..(BYTE_LENGTH as usize / 8) {
        let bytes = &clear[i * 8..][..8];
        let val = u64::from_le_bytes(bytes.try_into().unwrap());
        n += BigInt::from(val) << (i * 64);
    }
    let mut res = n * &*R_INV % &*P;
    if res > ((&*P) / 2) {
        res -= &*P;
    }
    res.into()
}

impl Test for i64 {
    #[track_caller]
    fn test(self) {
        let loc = core::panic::Location::caller();
        let vm = read_int(i64::from(loc.line()) + TEST_MEMORY_OFFSET);
        assert_eq!(
            self, vm,
            "value at {} differs between emulation and VM execution",
            loc
        );
    }
}

impl Test for bool {
    #[track_caller]
    fn test(self) {
        (self as i64).test()
    }
}

impl Test for ClearModp {
    #[track_caller]
    fn test(self) {
        let loc = core::panic::Location::caller();
        let vm = read_clear(i64::from(loc.line()) + TEST_MEMORY_OFFSET);
        println!("{} {}", loc, BigInt::from(vm));
        assert_eq!(
            BigInt::from(self) % &*P,
            BigInt::from(vm) % &*P,
            "value at {} differs between emulation and VM execution",
            loc
        );
    }
}

impl Test for SecretI64 {
    #[track_caller]
    fn test(self) {
        self.reveal().test()
    }
}

impl Test for SecretModp {
    #[track_caller]
    fn test(self) {
        self.reveal().test()
    }
}

impl Test for SecretBit {
    #[track_caller]
    fn test(self) {
        self.reveal().test()
    }
}

impl TestValue for SecretI64 {
    #[track_caller]
    fn test_value(self, val: SecretI64) {
        self.reveal().test_value(val.reveal())
    }
}

impl TestValue for SecretBit {
    #[track_caller]
    fn test_value(self, val: SecretBit) {
        self.reveal().test_value(val.reveal())
    }
}

impl TestValue for ClearModp {
    #[track_caller]
    fn test_value(self, val: Self) {
        let loc = core::panic::Location::caller();
        let vm = read_clear(i64::from(loc.line()) + TEST_MEMORY_OFFSET);
        println!("loc = {} val = {} vm = {}", loc, BigInt::from(val), BigInt::from(vm) % &*P);
        assert_eq!(
            BigInt::from(val) % &*P,
            BigInt::from(vm) % &*P,
            "value at {} differs between emulation and VM execution",
            loc
        );
    }
}

impl TestValue for SecretModp {
    #[track_caller]
    fn test_value(self, val: Self) {
        self.reveal().test_value(val.reveal())
    }
}

impl TestValue for i64 {
    #[track_caller]
    fn test_value(self, val: i64) {
        let loc = core::panic::Location::caller();
        let vm = read_int(i64::from(loc.line()) + TEST_MEMORY_OFFSET);
        println!("loc = {} val = {} vm = {}", loc, val, vm);
        assert_eq!(
            vm, val,
            "value at {} differs between emulation and tested value",
            loc
        );
    }
}

impl TestValue for bool {
    #[track_caller]
    fn test_value(self, val: bool) {
        (self as i64).test_value(val as i64)
    }
}

impl TestMem for i64 {
    #[track_caller]
    fn test_mem<const I: u32>(self, t_location: ConstU32<I>) {
        let loc = core::panic::Location::caller();
        let vm = i64::load_from_mem(t_location);
        println!("{} {}", loc, vm);
        assert_eq!(
            self, vm,
            "value at {} differs between emulation and VM execution",
            loc
        );
    }
}

impl TestMem for ClearModp {
    #[track_caller]
    fn test_mem<const I: u32>(self, t_location: ConstU32<I>) {
        let loc = core::panic::Location::caller();
        let vm = ClearModp::load_from_mem(t_location);
        println!("{} {}", loc, BigInt::from(vm));
        assert_eq!(
            BigInt::from(self) % &*P,
            BigInt::from(vm) % &*P,
            "value at {} differs between emulation and VM execution",
            loc
        );
    }
}

impl TestMem for SecretModp {
    #[track_caller]
    fn test_mem<const I: u32>(self, t_location: ConstU32<I>) {
        self.reveal().test_mem(t_location);
    }
}

impl TestMem for SecretI64 {
    #[track_caller]
    fn test_mem<const I: u32>(self, t_location: ConstU32<I>) {
        self.reveal().test_mem(t_location);
    }
}
