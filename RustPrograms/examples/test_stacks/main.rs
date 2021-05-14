// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![no_std]
#![no_main]
#![feature(const_evaluatable_checked)]

// This program will not work for small primes as the underlying
// assembly needs to execute convsintsbit sb0, s0 due to one
// of the optimizations not currently working. XXXX

#[scale::main(KAPPA = 40)]
#[inline(always)]
fn main() {
    let one = 1_i64;
    let two = 2_i64;
    let three = 3_i64;
    let four = 3_i64;

    // You need to use this in an unsafe environment
    // as peek/pop etc done in willy-nilly manner
    // will muck up the call-stack and parameter
    // stacks for function calls. So this forces you
    // to use it with care!
    unsafe {
        i64::push(&one);
        i64::push(&two);
        let sp = i64::get_stack_pointer();
        let c = i64::peek(sp - 1);
        let cc = i64::peek_from_top(1);
        i64::poke(sp, &three);
        i64::poke_from_top(1, &four);
        let d = i64::pop();
        let e = i64::pop();
        c.test_value(one);
        cc.test_value(one);
        d.test_value(three);
        e.test_value(four);
    }

    let one = SecretI64::from(ConstI32::<1>);
    let two = SecretI64::from(ConstI32::<2>);
    let three = SecretI64::from(ConstI32::<3>);
    let four = SecretI64::from(ConstI32::<4>);

    unsafe {
        SecretI64::push(&one);
        SecretI64::push(&two);
        let sp = SecretI64::get_stack_pointer();
        let c = SecretI64::peek(sp - 1);
        let cc = SecretI64::peek_from_top(1);
        SecretI64::poke(sp, &three);
        SecretI64::poke_from_top(1, &four);
        let d = SecretI64::pop();
        let e = SecretI64::pop();
        c.test_value(one);
        cc.test_value(one);
        d.test_value(three);
        e.test_value(four);
    }

    let one = SecretModp::from(ConstI32::<1>);
    let two = SecretModp::from(ConstI32::<2>);
    let three = SecretModp::from(ConstI32::<3>);
    let four = SecretModp::from(ConstI32::<4>);

    unsafe {
        SecretModp::push(&one);
        SecretModp::push(&two);
        let sp = SecretModp::get_stack_pointer();
        let c = SecretModp::peek(sp - 1);
        let cc = SecretModp::peek_from_top(1);
        SecretModp::poke(sp, &three);
        SecretModp::poke_from_top(1, &four);
        let d = SecretModp::pop();
        let e = SecretModp::pop();
        c.test_value(one);
        cc.test_value(one);
        d.test_value(three);
        e.test_value(four);
    }

    let one = ClearModp::from(ConstI32::<1>);
    let two = ClearModp::from(ConstI32::<2>);
    let three = ClearModp::from(ConstI32::<3>);
    let four = ClearModp::from(ConstI32::<4>);

    unsafe {
        ClearModp::push(&one);
        ClearModp::push(&two);
        let sp = ClearModp::get_stack_pointer();
        let c = ClearModp::peek(sp - 1);
        let cc = ClearModp::peek_from_top(1);
        ClearModp::poke(sp, &three);
        ClearModp::poke_from_top(1, &four);
        let d = ClearModp::pop();
        let e = ClearModp::pop();
        c.test_value(one);
        cc.test_value(one);
        d.test_value(three);
        e.test_value(four);
    }

    let one = SecretBit::from(true);
    let zero = SecretBit::from(false);

    unsafe {
        RawSecretBit::push(&one.to_raw());
        RawSecretBit::push(&one.to_raw());
        let sp = RawSecretBit::get_stack_pointer();
        let c = RawSecretBit::peek(sp - 1).to_real();
        let cc = RawSecretBit::peek_from_top(1).to_real();
        RawSecretBit::poke(sp, &zero.to_raw());
        RawSecretBit::poke_from_top(1, &zero.to_raw());
        let d = RawSecretBit::pop().to_real();
        let e = RawSecretBit::pop().to_real();
        c.test_value(one);
        cc.test_value(one);
        d.test_value(zero);
        e.test_value(zero);
    }
}
