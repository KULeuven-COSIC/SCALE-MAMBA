// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![no_std]
#![no_main]
#![feature(const_evaluatable_checked, const_generics)]
#![allow(non_snake_case, incomplete_features)]

use scale_std::fixed_point::*;
use scale_std::math::*;

fn test_approx<const K: u64, const F: u64>(a: ClearFixed<K, F>, val: ClearFixed<K, F>) -> ClearModp
where
    ConstU64<{ K + 1 }>: ,
    ConstU64<{ K - 1 }>: ,
{
    let lower = val - ClearFixed::from(0.00001);
    let upper = val + ClearFixed::from(0.00001);
    let ok = a.gt(lower) & a.lt(upper);
    ok
}

/* This test a relative error */
fn test_ratio<const K: u64, const F: u64>(a: ClearFixed<K, F>, val: ClearFixed<K, F>) -> ClearModp
where
    // HACK: due to a bug in the Rust compiler, constraints from SecretFixed are being picked up here
    ConstU64<{ 2 * F }>: ,
    ConstU64<{ 2 * K }>: ,
    ConstU64<{ 2 * (K - F) }>: ,
    // All constraints below are correct, only those above should not be required.
    ConstU64<{ K + 1 }>: ,
    ConstU64<{ K - 1 }>: ,
    ConstU64<{ ClearFixed::<K, F>::THETA }>: ,
    ConstI32<{ f_as_i32(F) }>: ,
    ConstI32<{ f_as_i32(K) }>: ,
{
    let test = a / val;
    let lower = ClearFixed::from(0.999);
    let upper = ClearFixed::from(1.001);
    let ok = test.gt(lower) & test.lt(upper);
    ok
}

#[scale::main(KAPPA = 40)]
#[inline(always)]
fn main() {
    start_clock(ConstI32::<1>);
    print!("\n\n Test the math ClearFixed library \n");
    let c: ClearFixed<40, 20> = ClearFixed::from(0.5431);
    let mc: ClearFixed<40, 20> = ClearFixed::from(-0.5431);

    let t = c.acos();
    print!("acos(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(0.99667167));
    ok.test_value(ClearModp::from(1));
    let t = c.asin();
    print!("asin(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(0.57412465));
    ok.test_value(ClearModp::from(1));
    let t = c.atan();
    print!("atan(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(0.49753027));
    ok.test_value(ClearModp::from(1));
    let t = c.cos();
    print!("cos(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(0.85611074));
    ok.test_value(ClearModp::from(1));
    let t = c.cosh();
    print!("cosh(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(1.15113963));
    ok.test_value(ClearModp::from(1));
    let t = c.sin();
    print!("sin(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(0.51679241));
    ok.test_value(ClearModp::from(1));
    let t = c.sinh();
    print!("sinh(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(0.57019510));
    ok.test_value(ClearModp::from(1));
    let t = c.tan();
    print!("tan(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(0.6036513));
    ok.test_value(ClearModp::from(1));
    let t = c.tanh();
    print!("tanh(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(0.49533096));
    ok.test_value(ClearModp::from(1));
    let t = c.exp();
    print!("exp(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(1.72133473));
    ok.test_value(ClearModp::from(1));
    let t = c.log();
    print!("log(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(-0.61046181));
    ok.test_value(ClearModp::from(1));
    let t = c.log2();
    print!("log2(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(-0.8807102));
    ok.test_value(ClearModp::from(1));
    let t = c.log10();
    print!("log10(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(-0.26512019));
    ok.test_value(ClearModp::from(1));
    let t = c.ceil();
    print!("ceil(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(1.0));
    ok.test_value(ClearModp::from(1));
    let t = c.fabs();
    print!("fabs(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, c);
    ok.test_value(ClearModp::from(1));
    let t = c.floor();
    print!("floor(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(0.0));
    ok.test_value(ClearModp::from(1));
    let t = c.sqrt();
    print!("sqrt(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(0.73695318));
    ok.test_value(ClearModp::from(1));
    let t = c.exp2();
    print!("exp2(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(1.457100108));
    ok.test_value(ClearModp::from(1));

    let t = mc.acos();
    print!("acos(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(2.14492098));
    ok.test_value(ClearModp::from(1));
    let t = mc.asin();
    print!("asin(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(-0.57412465));
    ok.test_value(ClearModp::from(1));
    let t = mc.atan();
    print!("atan(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(-0.49753027));
    ok.test_value(ClearModp::from(1));
    let t = mc.cos();
    print!("cos(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(0.85611074));
    ok.test_value(ClearModp::from(1));
    let t = mc.cosh();
    print!("cosh(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(1.15113963));
    ok.test_value(ClearModp::from(1));
    let t = mc.sin();
    print!("sin(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(-0.51679241));
    ok.test_value(ClearModp::from(1));
    let t = mc.sinh();
    print!("sinh(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(-0.57019510));
    ok.test_value(ClearModp::from(1));
    let t = mc.tan();
    print!("tan(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(-0.6036513));
    ok.test_value(ClearModp::from(1));
    let t = mc.tanh();
    print!("tanh(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(-0.49533096));
    ok.test_value(ClearModp::from(1));
    let t = mc.exp();
    print!("exp(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(0.580944530));
    ok.test_value(ClearModp::from(1));
    let t = mc.ceil();
    print!("ceil(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(0.0));
    ok.test_value(ClearModp::from(1));
    let t = mc.fabs();
    print!("fabs(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, c);
    ok.test_value(ClearModp::from(1));
    let t = mc.floor();
    print!("floor(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(-1.0));
    ok.test_value(ClearModp::from(1));
    let t = mc.exp2();
    print!("exp2(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(0.68629464));
    ok.test_value(ClearModp::from(1));

    print!("\n\n Test the math SecretFixed library \n");
    let c: SecretFixed<40, 20> = SecretFixed::from(0.5431);
    let mc: SecretFixed<40, 20> = SecretFixed::from(-0.5431);

    let t = c.acos().reveal();
    print!("acos(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(0.99667167));
    ok.test_value(ClearModp::from(1));
    let t = c.asin().reveal();
    print!("asin(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(0.57412465));
    ok.test_value(ClearModp::from(1));
    let t = c.atan().reveal();
    print!("atan(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(0.49753027));
    ok.test_value(ClearModp::from(1));
    let t = c.cos().reveal();
    print!("cos(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(0.85611074));
    ok.test_value(ClearModp::from(1));
    let t = c.cosh().reveal();
    print!("cosh(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(1.15113963));
    ok.test_value(ClearModp::from(1));
    let t = c.sin().reveal();
    print!("sin(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(0.51679241));
    ok.test_value(ClearModp::from(1));
    let t = c.sinh().reveal();
    print!("sinh(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(0.57019510));
    ok.test_value(ClearModp::from(1));
    let t = c.tan().reveal();
    print!("tan(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(0.6036513));
    ok.test_value(ClearModp::from(1));
    let t = c.tanh().reveal();
    print!("tanh(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(0.49533096));
    ok.test_value(ClearModp::from(1));
    let t = c.exp().reveal();
    print!("exp(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(1.72133473));
    ok.test_value(ClearModp::from(1));
    let t = c.log().reveal();
    print!("log(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(-0.61046181));
    ok.test_value(ClearModp::from(1));
    let t = c.log2().reveal();
    print!("log2(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(-0.8807102));
    ok.test_value(ClearModp::from(1));
    let t = c.log10().reveal();
    print!("log10(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(-0.26512019));
    ok.test_value(ClearModp::from(1));
    let t = c.ceil().reveal();
    print!("ceil(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(1.0));
    ok.test_value(ClearModp::from(1));
    let t = c.fabs().reveal();
    print!("fabs(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, c.reveal());
    ok.test_value(ClearModp::from(1));
    let t = c.floor().reveal();
    print!("floor(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(0.0));
    ok.test_value(ClearModp::from(1));
    let t = c.sqrt().reveal();
    print!("sqrt(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(0.73695318));
    ok.test_value(ClearModp::from(1));
    let t = c.exp2().reveal();
    print!("exp2(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(1.457100108));
    ok.test_value(ClearModp::from(1));

    let t = mc.acos().reveal();
    print!("acos(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(2.14492098));
    ok.test_value(ClearModp::from(1));
    let t = mc.asin().reveal();
    print!("asin(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(-0.57412465));
    ok.test_value(ClearModp::from(1));
    let t = mc.atan().reveal();
    print!("atan(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(-0.49753027));
    ok.test_value(ClearModp::from(1));
    let t = mc.cos().reveal();
    print!("cos(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(0.85611074));
    ok.test_value(ClearModp::from(1));
    let t = mc.cosh().reveal();
    print!("cosh(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(1.15113963));
    ok.test_value(ClearModp::from(1));
    let t = mc.sin().reveal();
    print!("sin(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(-0.51679241));
    ok.test_value(ClearModp::from(1));
    let t = mc.sinh().reveal();
    print!("sinh(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(-0.57019510));
    ok.test_value(ClearModp::from(1));
    let t = mc.tan().reveal();
    print!("tan(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(-0.6036513));
    ok.test_value(ClearModp::from(1));
    let t = mc.tanh().reveal();
    print!("tanh(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(-0.49533096));
    ok.test_value(ClearModp::from(1));
    let t = mc.exp().reveal();
    print!("exp(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(0.580944530));
    ok.test_value(ClearModp::from(1));
    let t = mc.ceil().reveal();
    print!("ceil(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(0.0));
    ok.test_value(ClearModp::from(1));
    let t = mc.fabs().reveal();
    print!("fabs(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, c.reveal());
    ok.test_value(ClearModp::from(1));
    let t = mc.floor().reveal();
    print!("floor(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(-1.0));
    ok.test_value(ClearModp::from(1));
    let t = mc.exp2().reveal();
    print!("exp2(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearFixed::from(0.68629464));
    ok.test_value(ClearModp::from(1));

    /* More extensive sqrt testing */
    let a1: ClearFixed<40, 20> = ClearFixed::from(3.141592);
    let a2: ClearFixed<40, 20> = ClearFixed::from(0.3131331);
    let a3: ClearFixed<40, 20> = ClearFixed::from(542.44242131);
    let a4: ClearFixed<40, 20> = ClearFixed::from(652.1311764);
    let b1: SecretFixed<40, 20> = SecretFixed::from(a1);
    let b2: SecretFixed<40, 20> = SecretFixed::from(a2);
    let b3: SecretFixed<40, 20> = SecretFixed::from(a3);
    let b4: SecretFixed<40, 20> = SecretFixed::from(a4);

    let SQ1 = b1.sqrt();
    let SQ2 = b2.sqrt();
    let SQ3 = b3.sqrt();
    let SQ4 = b4.sqrt();
    let t1 = SQ1 * SQ1;
    let t2 = SQ2 * SQ2;
    let t3 = SQ3 * SQ3;
    let t4 = SQ4 * SQ4;

    println!(t1.reveal(), " =  ", a1);
    let ok = test_ratio(t1.reveal(), a1);
    ok.test_value(ClearModp::from(1));
    println!(t2.reveal(), " =  ", a2);
    let ok = test_ratio(t2.reveal(), a2);
    ok.test_value(ClearModp::from(1));
    println!(t3.reveal(), " =  ", a3);
    let ok = test_ratio(t3.reveal(), a3);
    ok.test_value(ClearModp::from(1));
    println!(t4.reveal(), " =  ", a4);
    let ok = test_ratio(t4.reveal(), a4);
    ok.test_value(ClearModp::from(1));

    let SQ1 = a1.sqrt();
    let SQ2 = a2.sqrt();
    let SQ3 = a3.sqrt();
    let SQ4 = a4.sqrt();
    let t1 = SQ1 * SQ1;
    let t2 = SQ2 * SQ2;
    let t3 = SQ3 * SQ3;
    let t4 = SQ4 * SQ4;

    println!(t1.reveal(), " =  ", a1);
    let ok = test_ratio(t1, a1);
    ok.test_value(ClearModp::from(1));
    println!(t2.reveal(), " =  ", a2);
    let ok = test_ratio(t2, a2);
    ok.test_value(ClearModp::from(1));
    let ok = test_ratio(t3, a3);
    println!(t3.reveal(), " =  ", a3);
    ok.test_value(ClearModp::from(1));
    let ok = test_ratio(t4, a4);
    println!(t4.reveal(), " =  ", a4);
    ok.test_value(ClearModp::from(1));
    stop_clock(ConstI32::<1>);
}
