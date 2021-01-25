use crate::testing_emulated::P;
use crate::ClearModp;
use num_bigint::BigInt;
use num_integer::Integer;
use num_traits::ToPrimitive;

fn two_over(n: &BigInt) -> i8 {
    let nn: u32 = (n % BigInt::from(8)).to_u32().unwrap();
    if nn == 1 || nn == 7 {
        1
    } else {
        -1
    }
}

fn reciprocity(num: &BigInt, den: &BigInt) -> i8 {
    let nn: u32 = (num % BigInt::from(4)).to_u32().unwrap();
    let dd: u32 = (den % BigInt::from(4)).to_u32().unwrap();
    if nn == 3 && dd == 3 {
        -1
    } else {
        1
    }
}

fn is_even(num: &BigInt) -> bool {
    let nn: u32 = (num % BigInt::from(2)).to_u32().unwrap();
    if nn == 0 {
        true
    } else {
        false
    }
}

pub fn jacobi(a: &BigInt, n: &BigInt) -> i8 {
    let mut acc = 1;
    let mut num = a % n;
    let mut den = n.clone();
    loop {
        num = num % &den;
        if num == BigInt::from(0) {
            return 0;
        }

        while is_even(&num) {
            acc *= two_over(&den);
            num /= 2;
        }

        if num == BigInt::from(1) {
            return acc;
        }

        if num.gcd(&den) > BigInt::from(1) {
            return 0;
        }

        acc *= reciprocity(&num, &den);
        let tmp = num;
        num = den;
        den = tmp;
    }
}

#[no_mangle]
extern "C" fn __legendrec(value: ClearModp) -> ClearModp {
    let a: BigInt = value.into();
    let ans = jacobi(&a, &*P);
    BigInt::from(ans).into()
}
