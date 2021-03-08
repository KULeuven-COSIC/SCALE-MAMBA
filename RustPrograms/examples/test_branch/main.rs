
// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![no_std]
#![no_main]
#![feature(const_evaluatable_checked)]

#[scale::main(KAPPA = 40)]
#[inline(always)]
fn main() {
    /* This program has some weird assign to
     * memorys between the if statements. This is
     * to ensure SCALE does not jump these after
     * the if. i.e. the if jumps are computed
     * correctly in scasm
     */

    let s = SecretModp::from(ConstI32::<35>);
    let t = SecretModp::from(ConstI32::<34>);

    unsafe {
        s.store_in_mem(ConstU32::<50>);
        t.store_in_mem(ConstU32::<51>);
    }

    let c0 = 0;
    let c1 = 1;

    c0.test();
    c1.test();

    // Testing c0 = False
    if c0 == 1 {
        let a = ClearModp::from(ConstI32::<0>);
        let a = a + SecretModp::load_from_mem(ConstU32::<50>);
        unsafe {
            a.reveal().store_in_mem(ConstU32::<0>);
        }
    } else {
        let a = ClearModp::from(ConstI32::<25>);
        let a = a + SecretModp::load_from_mem(ConstU32::<50>);

        unsafe {
            a.reveal().store_in_mem(ConstU32::<0>);
        }
    }
    ClearModp::from(ConstI32::<60>).test_mem(ConstU32::<0>);

    let c = ClearModp::from(ConstI32::<8>);
    unsafe {
        c.store_in_mem(ConstU32::<3>);
    }
    ClearModp::from(ConstI32::<8>).test_mem(ConstU32::<3>);

    // Testing c1 = True
    if c1 == 1 {
        let a = ClearModp::from(ConstI32::<4>);
        let a = a + SecretModp::load_from_mem(ConstU32::<51>);
        unsafe {
            a.reveal().store_in_mem(ConstU32::<1>);
        }
    } else {
        let a = ClearModp::from(ConstI32::<26>);
        let a = a + SecretModp::load_from_mem(ConstU32::<51>);
        unsafe {
            a.reveal().store_in_mem(ConstU32::<1>);
        }
    }
    ClearModp::from(ConstI32::<38>).test_mem(ConstU32::<1>);

    let c = ClearModp::from(ConstI32::<9>);
    unsafe {
        c.store_in_mem(ConstU32::<4>);
    }
    ClearModp::from(ConstI32::<9>).test_mem(ConstU32::<4>);

    let c = ClearModp::from(ConstI32::<19>);
    unsafe {
        c.store_in_mem(ConstU32::<12>);
    }
    ClearModp::from(ConstI32::<19>).test_mem(ConstU32::<12>);

    // Now a nested if...
    if c1 == 1 {
        let a = ClearModp::from(ConstI32::<23>);
        let b = a * SecretModp::load_from_mem(ConstU32::<50>);
        unsafe {
            b.store_in_mem(ConstU32::<10>);
            b.reveal().store_in_mem(ConstU32::<10>);
        }
        if i64::from(b.reveal()) != 0 {
            let t = SecretModp::load_from_mem(ConstU32::<10>);
            let c = (t * t + ConstI32::<11>).reveal();
            unsafe {
                c.store_in_mem(ConstU32::<11>);
            }
        } else {
            let t = SecretModp::load_from_mem(ConstU32::<10>);
            let c = (t * t * t).reveal();
            unsafe {
                c.store_in_mem(ConstU32::<11>);
            }
        }
    }
    let t = ClearModp::from(ConstI32::<{ 23 * 35 }>);
    t.test_mem(ConstU32::<10>);
    (t * t + ConstI32::<11>).test_mem(ConstU32::<11>);

    // And another...
    if c1 == 1 {
        let a = ClearModp::from(ConstI32::<23>);
        let b = a * SecretModp::load_from_mem(ConstU32::<50>);
        unsafe {
            b.store_in_mem(ConstU32::<10>);
        }
        if i64::from(b.reveal()) == 0 {
            let t = SecretModp::load_from_mem(ConstU32::<10>);
            let c = (t * t + ConstI32::<11>).reveal();
            unsafe {
                c.store_in_mem(ConstU32::<12>);
            }
        } else {
            let t = SecretModp::load_from_mem(ConstU32::<10>);
            let c = (t * t * t).reveal();
            unsafe {
                c.store_in_mem(ConstU32::<12>);
            }
        }
    }
    let t = ClearModp::from(ConstI32::<{ 23 * 35 }>);
    (t * t * t).test_mem(ConstU32::<12>);
}
