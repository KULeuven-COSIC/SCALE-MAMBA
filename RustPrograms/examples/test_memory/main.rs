
// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![no_std]
#![no_main]
#![feature(const_evaluatable_checked)]

#[scale::main(KAPPA = 40)]
#[inline(always)]
fn main() {
    /* Do Stores */
    let a = ConstI32::<8>;
    let ra: i64 = a.into();
    let sra = SecretI64::from(a);
    let ca = ClearModp::from(a);
    let sa = SecretModp::from(a);

    unsafe {
        ra.store_in_mem(ConstU32::<1>);
        sra.store_in_mem(ConstU32::<1>);
        ca.store_in_mem(ConstU32::<1>);
        sa.store_in_mem(ConstU32::<1>);
    }

    /* Test Stores
     *   - In the real environment these do nothing.
     */
    let rd: i64 = a.into();
    let srd = SecretI64::from(a);
    let cd = ClearModp::from(a);
    let sd = SecretModp::from(a);
    rd.test_mem(ConstU32::<1>);
    srd.test_mem(ConstU32::<1>);
    cd.test_mem(ConstU32::<1>);
    sd.test_mem(ConstU32::<1>);

    /* Test loads */
    let rb = i64::load_from_mem(ConstU32::<1>);
    let srb = SecretI64::load_from_mem(ConstU32::<1>);
    let cb = ClearModp::load_from_mem(ConstU32::<1>);
    let sb = SecretModp::load_from_mem(ConstU32::<1>);
    rb.test_value(ra);
    srb.test_value(sra);
    cb.test_value(ca);
    sb.test_value(sa);

    /* Test run time loads */
    let x: i64 = 1;
    let rc = i64::load_from_mem(x);
    let src = SecretI64::load_from_mem(x);
    let cc = ClearModp::load_from_mem(x);
    let sc = SecretModp::load_from_mem(x);
    rc.test_value(ra);
    src.test_value(sra);
    cc.test_value(ca);
    sc.test_value(sa);

    /* Test run time stores */
    unsafe {
        ra.store_in_mem(x);
        sra.store_in_mem(x);
        ca.store_in_mem(x);
        sa.store_in_mem(x);
    }
    let rc = i64::load_from_mem(x);
    let src = SecretI64::load_from_mem(x);
    let cc = ClearModp::load_from_mem(x);
    let sc = SecretModp::load_from_mem(x);
    rc.test_value(ra);
    src.test_value(sra);
    cc.test_value(ca);
    sc.test_value(sa);
}
