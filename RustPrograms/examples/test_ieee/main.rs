// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![no_std]
#![no_main]
#![feature(const_evaluatable_checked)]
#![allow(non_snake_case)]

use scale_std::ieee::*;
use scale_std::math::*;

fn test_approx(a: ClearIEEE, val: ClearIEEE) -> i64 {
    let lower = val - ClearIEEE::from(0.00001);
    let upper = val + ClearIEEE::from(0.00001);
    let ok = (a > lower) & (a < upper);
    ok as i64
}

// This program will not work for small primes as the underlying
// assembly needs to execute convsintsbit sb0, s0 due to one
// of the optimizations not currently working. XXXX

#[scale::main(KAPPA = 40)]
#[inline(always)]
fn main() {
    let c = ClearIEEE::from(3.141592);
    print!(c.reveal(), "\n");

    let sc = SecretIEEE::from(3.141592);
    print!(sc.reveal(), "\n");

    let i: i64 = 1677216;
    let si = SecretI64::from(i);
    let sfi = SecretIEEE::from(si);
    let cfi = ClearIEEE::from(i);
    print!(sfi.reveal(), "\n");

    let ic = SecretI64::from(sc);
    print!(ic.reveal(), "\n");

    let is = SecretI64::from(si);
    print!(is.reveal(), "\n");

    let ifc = i64::from(c);
    print!(ifc, "\n");

    let twoc = ClearIEEE::from(2);
    let twos = SecretIEEE::from(2);
    let a1 = sc + sc;
    let a2 = twos * sc;
    let a3 = sfi / twos;
    let a4 = a3 + a3;
    let a5 = a1 * a1;
    let a6 = a5.sqrt();
    (a1 - a2).reveal().rep().test_value(0);
    (a4 - sfi).reveal().rep().test_value(0);
    (a6 - a1).reveal().rep().test_value(0);

    let a1 = sc + c;
    let a2 = twos * c;
    let a3 = sfi / twoc;
    let a4 = a3 + a3;
    let a5 = a1 * a1;
    let a6 = a5.sqrt();
    (a1 - a2).reveal().rep().test_value(0);
    (a4 - sfi).reveal().rep().test_value(0);
    (a6 - a1).reveal().rep().test_value(0);

    let a1 = c + sc;
    let a2 = twoc * sc;
    let a3 = cfi / twos;
    let a4 = a3 + a3;
    let a5 = a1 * a1;
    let a6 = a5.sqrt();
    (a1 - a2).reveal().rep().test_value(0);
    (a4 - sfi).reveal().rep().test_value(0);
    (a6 - a1).reveal().rep().test_value(0);

    let a1 = c + c;
    let a2 = twoc * c;
    let a3 = cfi / twoc;
    let a4 = a3 + a3;
    let a5 = a1 * a1;
    let a6 = a5.sqrt();
    (a1 - a2).rep().test_value(0);
    (a4 - cfi).rep().test_value(0);
    (a6 - a1).rep().test_value(0);

    let cmpa = sc.lt(sfi);
    let cmpb = sc.ge(sfi);
    let cmpc = sc.eq(sc);
    let cmpd = sc.ne(sc);
    let cmpe = sc.le(sfi);
    let cmpf = sc.ge(sfi);
    let cmpg = sc.le(sc);
    let cmph = sc.ge(sc);
    cmpa.reveal().test_value(true);
    cmpb.reveal().test_value(false);
    cmpc.reveal().test_value(true);
    cmpd.reveal().test_value(false);
    cmpe.reveal().test_value(true);
    cmpf.reveal().test_value(false);
    cmpg.reveal().test_value(true);
    cmph.reveal().test_value(true);

    let cza = sc.ltz();
    let czb = sc.lez();
    let czc = sc.gtz();
    let czd = sc.gez();
    let cze = sc.eqz();
    let czf = sc.nez();
    cza.reveal().test_value(false);
    czb.reveal().test_value(false);
    czc.reveal().test_value(true);
    czd.reveal().test_value(true);
    cze.reveal().test_value(false);
    czf.reveal().test_value(true);

    print!("\n\n Test the math ClearIEEE library \n");
    let c = ClearIEEE::from(0.5431);
    let mc = ClearIEEE::from(-0.5431);

    let t = c.acos();
    print!("acos(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(0.99667167));
    ok.test_value(1);
    let t = c.asin();
    print!("asin(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(0.57412465));
    ok.test_value(1);
    let t = c.atan();
    print!("atan(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(0.49753027));
    ok.test_value(1);
    let t = c.cos();
    print!("cos(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(0.85611074));
    ok.test_value(1);
    let t = c.cosh();
    print!("cosh(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(1.15113963));
    ok.test_value(1);
    let t = c.sin();
    print!("sin(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(0.51679241));
    ok.test_value(1);
    let t = c.sinh();
    print!("sinh(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(0.57019510));
    ok.test_value(1);
    let t = c.tan();
    print!("tan(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(0.6036513));
    ok.test_value(1);
    let t = c.tanh();
    print!("tanh(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(0.49533096));
    ok.test_value(1);
    let t = c.exp();
    print!("exp(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(1.72133473));
    ok.test_value(1);
    let t = c.log();
    print!("log(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(-0.61046181));
    ok.test_value(1);
    let t = c.log2();
    print!("log2(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(-0.8807102));
    ok.test_value(1);
    let t = c.log10();
    print!("log10(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(-0.26512019));
    ok.test_value(1);
    let t = c.ceil();
    print!("ceil(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(1.0));
    ok.test_value(1);
    let t = c.fabs();
    print!("fabs(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, c);
    ok.test_value(1);
    let t = c.floor();
    print!("floor(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(0.0));
    ok.test_value(1);
    let t = c.sqrt();
    print!("sqrt(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(0.73695318));
    ok.test_value(1);
    let t = c.exp2();
    print!("exp2(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(1.457100108));
    ok.test_value(1);

    let t = mc.acos();
    print!("acos(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(2.14492098));
    ok.test_value(1);
    let t = mc.asin();
    print!("asin(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(-0.57412465));
    ok.test_value(1);
    let t = mc.atan();
    print!("atan(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(-0.49753027));
    ok.test_value(1);
    let t = mc.cos();
    print!("cos(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(0.85611074));
    ok.test_value(1);
    let t = mc.cosh();
    print!("cosh(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(1.15113963));
    ok.test_value(1);
    let t = mc.sin();
    print!("sin(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(-0.51679241));
    ok.test_value(1);
    let t = mc.sinh();
    print!("sinh(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(-0.57019510));
    ok.test_value(1);
    let t = mc.tan();
    print!("tan(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(-0.6036513));
    ok.test_value(1);
    let t = mc.tanh();
    print!("tanh(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(-0.49533096));
    ok.test_value(1);
    let t = mc.exp();
    print!("exp(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(0.580944530));
    ok.test_value(1);
    let t = mc.ceil();
    print!("ceil(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(0.0));
    ok.test_value(1);
    let t = mc.fabs();
    print!("fabs(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, c);
    ok.test_value(1);
    let t = mc.floor();
    print!("floor(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(-1.0));
    ok.test_value(1);
    let t = mc.exp2();
    print!("exp2(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(0.68629464));
    ok.test_value(1);

    print!("\n\n Test the math SecretIEEE library \n");
    let c = SecretIEEE::from(0.5431);
    let mc = SecretIEEE::from(-0.5431);

    let t = c.acos().reveal();
    print!("acos(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(0.99667167));
    ok.test_value(1);
    let t = c.asin().reveal();
    print!("asin(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(0.57412465));
    ok.test_value(1);
    let t = c.atan().reveal();
    print!("atan(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(0.49753027));
    ok.test_value(1);
    let t = c.cos().reveal();
    print!("cos(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(0.85611074));
    ok.test_value(1);
    let t = c.cosh().reveal();
    print!("cosh(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(1.15113963));
    ok.test_value(1);
    let t = c.sin().reveal();
    print!("sin(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(0.51679241));
    ok.test_value(1);
    let t = c.sinh().reveal();
    print!("sinh(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(0.57019510));
    ok.test_value(1);
    let t = c.tan().reveal();
    print!("tan(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(0.6036513));
    ok.test_value(1);
    let t = c.tanh().reveal();
    print!("tanh(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(0.49533096));
    ok.test_value(1);
    let t = c.exp().reveal();
    print!("exp(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(1.72133473));
    ok.test_value(1);
    let t = c.log().reveal();
    print!("log(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(-0.61046181));
    ok.test_value(1);
    let t = c.log2().reveal();
    print!("log2(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(-0.8807102));
    ok.test_value(1);
    let t = c.log10().reveal();
    print!("log10(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(-0.26512019));
    ok.test_value(1);
    let t = c.ceil().reveal();
    print!("ceil(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(1.0));
    ok.test_value(1);
    let t = c.fabs().reveal();
    print!("fabs(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, c.reveal());
    ok.test_value(1);
    let t = c.floor().reveal();
    print!("floor(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(0.0));
    ok.test_value(1);
    let t = c.sqrt().reveal();
    print!("sqrt(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(0.73695318));
    ok.test_value(1);
    let t = c.exp2().reveal();
    print!("exp2(", c.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(1.457100108));
    ok.test_value(1);

    let t = mc.acos().reveal();
    print!("acos(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(2.14492098));
    ok.test_value(1);
    let t = mc.asin().reveal();
    print!("asin(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(-0.57412465));
    ok.test_value(1);
    let t = mc.atan().reveal();
    print!("atan(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(-0.49753027));
    ok.test_value(1);
    let t = mc.cos().reveal();
    print!("cos(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(0.85611074));
    ok.test_value(1);
    let t = mc.cosh().reveal();
    print!("cosh(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(1.15113963));
    ok.test_value(1);
    let t = mc.sin().reveal();
    print!("sin(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(-0.51679241));
    ok.test_value(1);
    let t = mc.sinh().reveal();
    print!("sinh(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(-0.57019510));
    ok.test_value(1);
    let t = mc.tan().reveal();
    print!("tan(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(-0.6036513));
    ok.test_value(1);
    let t = mc.tanh().reveal();
    print!("tanh(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(-0.49533096));
    ok.test_value(1);
    let t = mc.exp().reveal();
    print!("exp(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(0.580944530));
    ok.test_value(1);
    let t = mc.ceil().reveal();
    print!("ceil(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(0.0));
    ok.test_value(1);
    let t = mc.fabs().reveal();
    print!("fabs(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, c.reveal());
    ok.test_value(1);
    let t = mc.floor().reveal();
    print!("floor(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(-1.0));
    ok.test_value(1);
    let t = mc.exp2().reveal();
    print!("exp2(", mc.reveal(), ") = ", t, "\n");
    let ok = test_approx(t, ClearIEEE::from(0.68629464));
    ok.test_value(1);
}
