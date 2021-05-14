// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![no_std]
#![no_main]
#![feature(const_evaluatable_checked)]
#![allow(non_snake_case, incomplete_features)]

use scale_std::array::*;
use scale_std::bit_protocols::*;
use scale_std::slice::*;

#[scale::main(KAPPA = 40)]
#[inline(always)]
fn main() {
    // a = 7 b = 5
    // 7 + 5 = 12
    let mut ca: Slice<ClearModp> = Slice::uninitialized(3);
    let mut sa: Slice<SecretModp> = Slice::uninitialized(3);
    ca.set(0, &ClearModp::from(1));
    ca.set(1, &ClearModp::from(1));
    ca.set(2, &ClearModp::from(1));
    sa.set(0, &SecretModp::from(1));
    sa.set(1, &SecretModp::from(1));
    sa.set(2, &SecretModp::from(1));

    let mut cb: Slice<ClearModp> = Slice::uninitialized(3);
    let mut sb: Slice<SecretModp> = Slice::uninitialized(3);
    cb.set(0, &ClearModp::from(1));
    cb.set(1, &ClearModp::from(0));
    cb.set(2, &ClearModp::from(1));
    sb.set(0, &SecretModp::from(1));
    sb.set(1, &SecretModp::from(0));
    sb.set(2, &SecretModp::from(1));

    let v_ca_sb = BitAdd(&ca, &sb, ca.len() + 1);
    let v_sa_cb = BitAdd(&sa, &cb, sa.len() + 1);
    let v_sa_sb = BitAdd(&sa, &sb, sa.len() + 1);

    v_ca_sb
        .get(0)
        .unwrap()
        .reveal()
        .test_value(ClearModp::from(0));
    v_ca_sb
        .get(1)
        .unwrap()
        .reveal()
        .test_value(ClearModp::from(0));
    v_ca_sb
        .get(2)
        .unwrap()
        .reveal()
        .test_value(ClearModp::from(1));
    v_ca_sb
        .get(3)
        .unwrap()
        .reveal()
        .test_value(ClearModp::from(1));

    v_sa_cb
        .get(0)
        .unwrap()
        .reveal()
        .test_value(ClearModp::from(0));
    v_sa_cb
        .get(1)
        .unwrap()
        .reveal()
        .test_value(ClearModp::from(0));
    v_sa_cb
        .get(2)
        .unwrap()
        .reveal()
        .test_value(ClearModp::from(1));
    v_sa_cb
        .get(3)
        .unwrap()
        .reveal()
        .test_value(ClearModp::from(1));

    v_sa_sb
        .get(0)
        .unwrap()
        .reveal()
        .test_value(ClearModp::from(0));
    v_sa_sb
        .get(1)
        .unwrap()
        .reveal()
        .test_value(ClearModp::from(0));
    v_sa_sb
        .get(2)
        .unwrap()
        .reveal()
        .test_value(ClearModp::from(1));
    v_sa_sb
        .get(3)
        .unwrap()
        .reveal()
        .test_value(ClearModp::from(1));

    let mut sas: Slice<SecretModp> = Slice::uninitialized(3);
    let mut saa: Array<SecretModp, 3> = Array::uninitialized();
    saa.set(0, &SecretModp::from(1));
    saa.set(1, &SecretModp::from(1));
    saa.set(2, &SecretModp::from(1));
    sas.set(0, &SecretModp::from(1));
    sas.set(1, &SecretModp::from(1));
    sas.set(2, &SecretModp::from(1));
    let v_inc_sa = BitIncrement(&saa);

    v_inc_sa
        .get(0)
        .unwrap()
        .reveal()
        .test_value(ClearModp::from(0));
    v_inc_sa
        .get(1)
        .unwrap()
        .reveal()
        .test_value(ClearModp::from(0));
    v_inc_sa
        .get(2)
        .unwrap()
        .reveal()
        .test_value(ClearModp::from(0));
    v_inc_sa
        .get(3)
        .unwrap()
        .reveal()
        .test_value(ClearModp::from(1));

    sas.set(1, &SecretModp::from(0));
    let four = ClearModp::from(4);
    let six = ClearModp::from(6);
    let cmp_four = BitLT(four, &sas, sas.len());
    let cmp_six = BitLT(six, &sas, sas.len());

    cmp_four.reveal().test_value(ClearModp::from(1));
    cmp_six.reveal().test_value(ClearModp::from(0));

    let cmp_a1 = BitLTFull(&ca, &sb);
    let cmp_a2 = BitLTFull(&sa, &sb);
    let cmp_a3 = BitLTFull(&sa, &sb);
    let cmp_a4 = BitLTFull(&cb, &sa);
    let cmp_a5 = BitLTFull(&sb, &sa);
    let cmp_a6 = BitLTFull(&sb, &sa);

    cmp_a1.reveal().test_value(ClearModp::from(0));
    cmp_a2.reveal().test_value(ClearModp::from(0));
    cmp_a3.reveal().test_value(ClearModp::from(0));
    cmp_a4.reveal().test_value(ClearModp::from(1));
    cmp_a5.reveal().test_value(ClearModp::from(1));
    cmp_a6.reveal().test_value(ClearModp::from(1));

    let ss1 = SecretModp::from(5);
    let ss2 = SecretModp::from(-5);
    let v1 = BitDec::<10, 5, 40>(ss1);
    let v2 = BitDec::<10, 5, 40>(ss2);
    v1.get(0).unwrap().reveal().test_value(ClearModp::from(1));
    v1.get(1).unwrap().reveal().test_value(ClearModp::from(0));
    v1.get(2).unwrap().reveal().test_value(ClearModp::from(1));
    v1.get(3).unwrap().reveal().test_value(ClearModp::from(0));
    v1.get(4).unwrap().reveal().test_value(ClearModp::from(0));

    v2.get(0).unwrap().reveal().test_value(ClearModp::from(1));
    v2.get(1).unwrap().reveal().test_value(ClearModp::from(1));
    v2.get(2).unwrap().reveal().test_value(ClearModp::from(0));
    v2.get(3).unwrap().reveal().test_value(ClearModp::from(1));
    v2.get(4).unwrap().reveal().test_value(ClearModp::from(1));
}
