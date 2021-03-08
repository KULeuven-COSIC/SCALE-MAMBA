// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![no_std]
#![no_main]
#![feature(const_evaluatable_checked)]
#![allow(non_snake_case)]

use scale_std::array::Array;
use scale_std::circuits::*;

#[scale::main(KAPPA = 40)]
#[inline(always)]
fn main() {
    let zero = SecretI64::from(ConstI32::<0>);
    let one = SecretI64::from(ConstI32::<1>);
    let mone = SecretI64::from(ConstI32::<-1>);

    let key128: [SecretI64; 2] = [zero, mone];
    let key192: [SecretI64; 3] = [zero, mone, zero];
    let key256: [SecretI64; 4] = [zero, mone, zero, zero];
    let mess: [SecretI64; 2] = [mone, one];

    print!("AES-128\n");
    let ciph = AES128(key128, mess);

    let mut m: [i64; 2] = Default::default();
    let mut k: [i64; 4] = Default::default();
    let mut c: [i64; 2] = Default::default();

    for i in 0..2 {
        m[i] = mess[i].reveal();
        k[i] = key128[i].reveal();
        c[i] = ciph[i].reveal();
    }

    print!("Key     :", k[1], " ", k[0], "\n");
    print!("Message :", m[1], " ", m[0], "\n");
    print!("Cipher  :", c[1], " ", c[0], "\n");

    c[1].test_value(0x406bab6335ce415f);
    c[0].test_value(0x4f943dc8966682aa);

    print!("AES-192\n");
    let ciph = AES192(key192, mess);

    k[2] = key192[2].reveal();
    for i in 0..2 {
        c[i] = ciph[i].reveal();
    }

    print!("Key     :", k[2], " ", k[1], " ", k[0], "\n");
    print!("Message :", m[1], " ", m[0], "\n");
    print!("Cipher  :", c[1], " ", c[0], "\n");

    c[1].test_value(0x5431a62c98d9bd2c_u64 as i64);
    c[0].test_value(0xb3bb68de3b46c05c_u64 as i64);

    print!("AES-256\n");
    let ciph = AES256(key256, mess);

    k[3] = key256[3].reveal();
    for i in 0..2 {
        c[i] = ciph[i].reveal();
    }

    print!("Key     :", k[3], " ", k[2], " ", k[1], " ", k[0], "\n");
    print!("Message :", m[1], " ", m[0], "\n");
    print!("Cipher  :", c[1], " ", c[0], "\n");

    c[1].test_value(0x67f2f9adfcff97cd_u64 as i64);
    c[0].test_value(0x5e07c0d5edba1e69_u64 as i64);

    print!("SHA-3\n");
    let istate: Array<SecretI64, 25> = Array::fill(zero);

    let ostate1 = SHA3(istate);

    print!("First invocation of SHA-3\n");
    let costate1 = ostate1.reveal();
    for (i, val) in costate1.iter().enumerate() {
        print!("State ", i as i64, "  = ", val, "\n");
    }

    costate1
        .get(24)
        .unwrap()
        .test_value(0xe7dde140798f25f1_u64 as i64);
    costate1
        .get(23)
        .unwrap()
        .test_value(0x8a47c033f9ccd584_u64 as i64);
    costate1
        .get(22)
        .unwrap()
        .test_value(0xeea95aa61e2698d5_u64 as i64);

    let ostate2 = SHA3(ostate1);

    print!("Second invocation of SHA-3\n");
    let costate2 = ostate2.reveal();
    for (i, val) in costate2.iter().enumerate() {
        print!("State ", i as i64, "  = ", val, "\n");
    }

    costate2
        .get(24)
        .unwrap()
        .test_value(0x3ccb6ef94d955c2d_u64 as i64);
    costate2
        .get(23)
        .unwrap()
        .test_value(0x6db55770d02c336a_u64 as i64);
    costate2
        .get(22)
        .unwrap()
        .test_value(0x6c6bd770128d3d09_u64 as i64);
}
