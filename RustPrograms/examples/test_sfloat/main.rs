// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![no_std]
#![no_main]
#![feature(const_evaluatable_checked, const_generics)]
#![allow(non_snake_case, incomplete_features)]

use scale_std::array::*;
use scale_std::fixed_point::*;
use scale_std::ieee::*;
use scale_std::integer::*;
use scale_std::math::*;

fn test_approx<const V: u64, const P: u64>(a: ClearFloat<V, P>, val: ClearFloat<V, P>) -> ClearModp
where
   ConstU64<{ P + 1 }>: ,
   ConstU64<{ V + 1 }>: ,
   ConstU64<{ V - 1 }>: ,
   ConstU64<{ 2 * V }>: ,
   ConstU64<{ V + 2 }>: ,
{
    //println!("test_approx : a ",&a);
    //println!("test_approx : v ",&val);
    let eps: ClearFloat<V, P> = ClearFloat::from(0.0001);
    let lower = &val - &eps;
    let upper = &val + &eps;
    //println!("test_approx : l ",&lower);
    //println!("test_approx : u ",&upper);
    let ok = a.clone().ge(lower) & a.le(upper);
    ok
}

#[scale::main(KAPPA = 40)]
fn main() {
    //arithm_tests()
    let zero: ClearFloat<40, 20> = ClearFloat::from(0);
    let one: ClearFloat<40, 20> = ClearFloat::from(1);
    let mone: ClearFloat<40, 20> = ClearFloat::from(-1);

    let mut cb_param: Array<ClearModp, 5> = Array::fill(ClearModp::from(0));
    cb_param.set(0, &ClearModp::from(847182299136));
    cb_param.set(1, &ClearModp::from(-30));

    // From<> (and reveal)
    let t_int: ClearInteger<41> = ClearInteger::from(123456); // K(int) must be bigger than V (float)
    let t_cfix: ClearFixed<41, 20> = ClearFixed::from(123456); // K(fix) must be bigger than V (float)
    let t_ieee: ClearIEEE = ClearIEEE::from(123456);
    let mut c1: ClearFloat<40, 20> = ClearFloat::from(123456_i64);
    let c2: ClearFloat<40, 20> = ClearFloat::from(123456_f64);
    let c3: ClearFloat<40, 20> = ClearFloat::from(t_int);
    let c4: ClearFloat<40, 20> = ClearFloat::from(t_cfix);
    let c5: ClearFloat<40, 20> = ClearFloat::from(t_ieee);
    let s: SecretFloat<40, 20> = SecretFloat::from(c1.clone());

    let va: ClearModp = ClearModp::from(1035623989248);
    let pa: ClearModp = ClearModp::from(-23);
    c1.v().test_value(va);
    c1.p().test_value(pa);
    c2.v().test_value(va);
    c2.p().test_value(pa);
    c3.v().test_value(va);
    c3.p().test_value(pa);
    c4.v().test_value(va);
    c4.p().test_value(pa);
    c5.v().test_value(va);
    c5.p().test_value(pa);
    s.v().reveal().test_value(va);
    s.reveal().p().test_value(pa);

    // set
    c1 = ClearFloat::set(cb_param);
    let vb = ClearModp::from(847182299136);
    let pb = ClearModp::from(-30);
    c1.v().test_value(vb);
    c1.p().test_value(pb);

    // addition
    let ca: ClearFloat<40, 20> = ClearFloat::from(123456);
    let sa: SecretFloat<40, 20> = SecretFloat::from(ca.clone());
    let cb: ClearFloat<40, 20> = ClearFloat::from(789);
    let sb: SecretFloat<40, 20> = SecretFloat::from(cb.clone());
    let sum: f64 = 124245.0;
    let diff: f64 = 122667.0;
    let prod: f64 = 97406784.0;
    let quot: f64 = 156.4714828;

    let sscr = sa.clone() + &sb;
    let ssrr = &sa + &sb;
    let ssrc = &sa + sb.clone();
    let sscc = sa.clone() + sb.clone();

    let sccr = sa.clone() + &cb;
    let scrr = &sa + &cb;
    let scrc = &sa + cb.clone();
    let sccc = sa.clone() + cb.clone();

    let cscr = ca.clone() + &sb;
    let csrr = &ca + &sb;
    let csrc = &ca + sb.clone();
    let cscc = ca.clone() + sb.clone();

    let cccr = ca.clone() + &cb;
    let ccrr = &ca + &cb;
    let ccrc = &ca + cb.clone();
    let cccc = ca.clone() + cb.clone();

    let mut ok: ClearModp = test_approx(sscr.reveal(), ClearFloat::<40, 20>::from(sum));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(ssrr.reveal(), ClearFloat::<40, 20>::from(sum));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(ssrc.reveal(), ClearFloat::<40, 20>::from(sum));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(sscc.reveal(), ClearFloat::<40, 20>::from(sum));
    ok.test_value(ClearModp::from(1));

    ok = test_approx(sccr.reveal(), ClearFloat::<40, 20>::from(sum));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(scrr.reveal(), ClearFloat::<40, 20>::from(sum));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(scrc.reveal(), ClearFloat::<40, 20>::from(sum));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(sccc.reveal(), ClearFloat::<40, 20>::from(sum));
    ok.test_value(ClearModp::from(1));

    ok = test_approx(cscr.reveal(), ClearFloat::<40, 20>::from(sum));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(csrr.reveal(), ClearFloat::<40, 20>::from(sum));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(csrc.reveal(), ClearFloat::<40, 20>::from(sum));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(cscc.reveal(), ClearFloat::<40, 20>::from(sum));
    ok.test_value(ClearModp::from(1));

    ok = test_approx(cccr, ClearFloat::<40, 20>::from(sum));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(ccrr, ClearFloat::<40, 20>::from(sum));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(ccrc, ClearFloat::<40, 20>::from(sum));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(cccc, ClearFloat::<40, 20>::from(sum));
    ok.test_value(ClearModp::from(1));

    let sscr = sa.clone() - &sb;
    let ssrr = &sa - &sb;
    let ssrc = &sa - sb.clone();
    let sscc = sa.clone() - sb.clone();

    let sccr = sa.clone() - &cb;
    let scrr = &sa - &cb;
    let scrc = &sa - cb.clone();
    let sccc = sa.clone() - cb.clone();

    let cscr = ca.clone() - &sb;
    let csrr = &ca - &sb;
    let csrc = &ca - sb.clone();
    let cscc = ca.clone() - sb.clone();

    let cccr = ca.clone() - &cb;
    let ccrr = &ca - &cb;
    let ccrc = &ca - cb.clone();
    let cccc = ca.clone() - cb.clone();

    let mut ok: ClearModp = test_approx(sscr.reveal(), ClearFloat::<40, 20>::from(diff));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(ssrr.reveal(), ClearFloat::<40, 20>::from(diff));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(ssrc.reveal(), ClearFloat::<40, 20>::from(diff));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(sscc.reveal(), ClearFloat::<40, 20>::from(diff));
    ok.test_value(ClearModp::from(1));

    ok = test_approx(sccr.reveal(), ClearFloat::<40, 20>::from(diff));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(scrr.reveal(), ClearFloat::<40, 20>::from(diff));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(scrc.reveal(), ClearFloat::<40, 20>::from(diff));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(sccc.reveal(), ClearFloat::<40, 20>::from(diff));
    ok.test_value(ClearModp::from(1));

    ok = test_approx(cscr.reveal(), ClearFloat::<40, 20>::from(diff));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(csrr.reveal(), ClearFloat::<40, 20>::from(diff));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(csrc.reveal(), ClearFloat::<40, 20>::from(diff));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(cscc.reveal(), ClearFloat::<40, 20>::from(diff));
    ok.test_value(ClearModp::from(1));

    ok = test_approx(cccr, ClearFloat::<40, 20>::from(diff));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(ccrr, ClearFloat::<40, 20>::from(diff));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(ccrc, ClearFloat::<40, 20>::from(diff));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(cccc, ClearFloat::<40, 20>::from(diff));
    ok.test_value(ClearModp::from(1));

    let sscr = sa.clone() * &sb;
    let ssrr = &sa * &sb;
    let ssrc = &sa * sb.clone();
    let sscc = sa.clone() * sb.clone();

    let sccr = sa.clone() * &cb;
    let scrr = &sa * &cb;
    let scrc = &sa * cb.clone();
    let sccc = sa.clone() * cb.clone();

    let cscr = ca.clone() * &sb;
    let csrr = &ca * &sb;
    let csrc = &ca * sb.clone();
    let cscc = ca.clone() * sb.clone();

    let cccr = ca.clone() * &cb;
    let ccrr = &ca * &cb;
    let ccrc = &ca * cb.clone();
    let cccc = ca.clone() * cb.clone();

    let mut ok: ClearModp = test_approx(sscr.reveal(), ClearFloat::<40, 20>::from(prod));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(ssrr.reveal(), ClearFloat::<40, 20>::from(prod));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(ssrc.reveal(), ClearFloat::<40, 20>::from(prod));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(sscc.reveal(), ClearFloat::<40, 20>::from(prod));
    ok.test_value(ClearModp::from(1));

    ok = test_approx(sccr.reveal(), ClearFloat::<40, 20>::from(prod));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(scrr.reveal(), ClearFloat::<40, 20>::from(prod));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(scrc.reveal(), ClearFloat::<40, 20>::from(prod));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(sccc.reveal(), ClearFloat::<40, 20>::from(prod));
    ok.test_value(ClearModp::from(1));

    ok = test_approx(cscr.reveal(), ClearFloat::<40, 20>::from(prod));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(csrr.reveal(), ClearFloat::<40, 20>::from(prod));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(csrc.reveal(), ClearFloat::<40, 20>::from(prod));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(cscc.reveal(), ClearFloat::<40, 20>::from(prod));
    ok.test_value(ClearModp::from(1));

    ok = test_approx(cccr, ClearFloat::<40, 20>::from(prod));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(ccrr, ClearFloat::<40, 20>::from(prod));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(ccrc, ClearFloat::<40, 20>::from(prod));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(cccc, ClearFloat::<40, 20>::from(prod));
    ok.test_value(ClearModp::from(1));

    let sscr = sa.clone() / &sb;
    let ssrr = &sa / &sb;
    let ssrc = &sa / sb.clone();
    let sscc = sa.clone() / sb.clone();

    let sccr = sa.clone() / &cb;
    let scrr = &sa / &cb;
    let scrc = &sa / cb.clone();
    let sccc = sa.clone() / cb.clone();

    let cscr = ca.clone() / &sb;
    let csrr = &ca / &sb;
    let csrc = &ca / sb.clone();
    let cscc = ca.clone() / sb.clone();

    let cccr = ca.clone() / &cb;
    let ccrr = &ca / &cb;
    let ccrc = &ca / cb.clone();
    let cccc = ca.clone() / cb.clone();

    let mut ok: ClearModp = test_approx(sscr.reveal(), ClearFloat::<40, 20>::from(quot));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(ssrr.reveal(), ClearFloat::<40, 20>::from(quot));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(ssrc.reveal(), ClearFloat::<40, 20>::from(quot));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(sscc.reveal(), ClearFloat::<40, 20>::from(quot));
    ok.test_value(ClearModp::from(1));

    ok = test_approx(sccr.reveal(), ClearFloat::<40, 20>::from(quot));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(scrr.reveal(), ClearFloat::<40, 20>::from(quot));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(scrc.reveal(), ClearFloat::<40, 20>::from(quot));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(sccc.reveal(), ClearFloat::<40, 20>::from(quot));
    ok.test_value(ClearModp::from(1));

    ok = test_approx(cscr.reveal(), ClearFloat::<40, 20>::from(quot));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(csrr.reveal(), ClearFloat::<40, 20>::from(quot));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(csrc.reveal(), ClearFloat::<40, 20>::from(quot));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(cscc.reveal(), ClearFloat::<40, 20>::from(quot));
    ok.test_value(ClearModp::from(1));

    ok = test_approx(cccr, ClearFloat::<40, 20>::from(quot));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(ccrr, ClearFloat::<40, 20>::from(quot));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(ccrc, ClearFloat::<40, 20>::from(quot));
    ok.test_value(ClearModp::from(1));
    ok = test_approx(cccc, ClearFloat::<40, 20>::from(quot));
    ok.test_value(ClearModp::from(1));
    
    /*****************************************************************/

    /* Now run same tests as used in testing the cfix/sfix */

    let a: ClearFloat<40, 20> = ClearFloat::from(3.141592);
    let b: SecretFloat<40, 20> = SecretFloat::from(a.clone());

    let am: ClearFloat<40, 20> = ClearFloat::from(-3.141592);
    let bm: SecretFloat<40, 20> = SecretFloat::from(am.clone());

    let ok = test_approx(&a + &am, zero.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx((&a + &bm).reveal(), zero.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx((&b + &am).reveal(), zero.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx((&b + &bm).reveal(), zero.clone());
    ok.test_value(ClearModp::from(1));

    // Test abs
    let ok = test_approx(&a - am.clone().fabs(), zero.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx((&a - bm.clone().fabs()).reveal(), zero.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx((&b - am.clone().fabs()).reveal(), zero.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx((&b - bm.clone().fabs()).reveal(), zero.clone());
    ok.test_value(ClearModp::from(1));

    // Test floor
    /* XXXX NOT IMPLEMENTED YET
    a.floor().rep_integer().test_value(ClearModp::from(3));
    b.floor()
        .reveal()
        .rep_integer()
        .test_value(ClearModp::from(3));
    am.floor().rep_integer().test_value(ClearModp::from(-4));
    bm.floor()
        .reveal()
        .rep_integer()
        .test_value(ClearModp::from(-4));
    */

    // Test add
    let two: ClearFloat<40, 20> = ClearFloat::from(2);
    let e = &a + &a;
    let t = &two * &a;
    let f1 = &b + &a;
    let f2 = &a + &b;
    let f3 = &b + &b;
    let ok = test_approx(e - &t, zero.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx((f1 - &t).reveal(), zero.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx((f2 - &t).reveal(), zero.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx((f3 - &t).reveal(), zero.clone());
    ok.test_value(ClearModp::from(1));

    // Test mult
    let e = &a * &a;
    let t: ClearFloat<40, 20> = ClearFloat::from(9.8696003);
    let f1 = &b * &a;
    let f2 = &a * &b;
    let f3 = &b * &b;

    let ok = test_approx(e - &t, zero.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx((f1 - &t).reveal(), zero.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx((f2 - &t).reveal(), zero.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx((f3 - &t).reveal(), zero.clone());
    ok.test_value(ClearModp::from(1));

    // TEST COMPARISONS
    a.clone().lt(a.clone()).test_value(ClearModp::from(0));
    a.clone().lt(am.clone()).test_value(ClearModp::from(0));

    a.clone().le(a.clone()).test_value(ClearModp::from(1));
    a.clone().le(am.clone()).test_value(ClearModp::from(0));

    a.clone().gt(a.clone()).test_value(ClearModp::from(0));
    a.clone().gt(am.clone()).test_value(ClearModp::from(1));

    a.clone().ge(a.clone()).test_value(ClearModp::from(1));
    a.clone().ge(am.clone()).test_value(ClearModp::from(1));

    a.clone().eq(a.clone()).test_value(ClearModp::from(1));
    a.clone().eq(am.clone()).test_value(ClearModp::from(0));

    a.clone().ne(a.clone()).test_value(ClearModp::from(0));
    a.clone().ne(am.clone()).test_value(ClearModp::from(1));

    //
    am.clone().lt(a.clone()).test_value(ClearModp::from(1));
    am.clone().lt(am.clone()).test_value(ClearModp::from(0));

    am.clone().le(a.clone()).test_value(ClearModp::from(1));
    am.clone().le(am.clone()).test_value(ClearModp::from(1));

    am.clone().gt(a.clone()).test_value(ClearModp::from(0));
    am.clone().gt(am.clone()).test_value(ClearModp::from(0));

    am.clone().ge(a.clone()).test_value(ClearModp::from(0));
    am.clone().ge(am.clone()).test_value(ClearModp::from(1));

    am.clone().eq(a.clone()).test_value(ClearModp::from(0));
    am.clone().eq(am.clone()).test_value(ClearModp::from(1));

    am.clone().ne(a.clone()).test_value(ClearModp::from(1));
    am.clone().ne(am.clone()).test_value(ClearModp::from(0));

    //
    b.clone()
        .lt(b.clone())
        .reveal()
        .test_value(ClearModp::from(0));
    b.clone()
        .lt(bm.clone())
        .reveal()
        .test_value(ClearModp::from(0));

    b.clone()
        .le(b.clone())
        .reveal()
        .test_value(ClearModp::from(1));
    b.clone()
        .le(bm.clone())
        .reveal()
        .test_value(ClearModp::from(0));

    b.clone()
        .gt(b.clone())
        .reveal()
        .test_value(ClearModp::from(0));
    b.clone()
        .gt(bm.clone())
        .reveal()
        .test_value(ClearModp::from(1));

    b.clone()
        .ge(b.clone())
        .reveal()
        .test_value(ClearModp::from(1));
    b.clone()
        .ge(bm.clone())
        .reveal()
        .test_value(ClearModp::from(1));

    b.clone()
        .eq(b.clone())
        .reveal()
        .test_value(ClearModp::from(1));
    b.clone()
        .eq(bm.clone())
        .reveal()
        .test_value(ClearModp::from(0));

    b.clone()
        .ne(b.clone())
        .reveal()
        .test_value(ClearModp::from(0));
    b.clone()
        .ne(bm.clone())
        .reveal()
        .test_value(ClearModp::from(1));

    //
    bm.clone()
        .lt(b.clone())
        .reveal()
        .test_value(ClearModp::from(1));
    bm.clone()
        .lt(bm.clone())
        .reveal()
        .test_value(ClearModp::from(0));

    bm.clone()
        .le(b.clone())
        .reveal()
        .test_value(ClearModp::from(1));
    bm.clone()
        .le(bm.clone())
        .reveal()
        .test_value(ClearModp::from(1));

    bm.clone()
        .gt(b.clone())
        .reveal()
        .test_value(ClearModp::from(0));
    bm.clone()
        .gt(bm.clone())
        .reveal()
        .test_value(ClearModp::from(0));

    bm.clone()
        .ge(b.clone())
        .reveal()
        .test_value(ClearModp::from(0));
    bm.clone()
        .ge(bm.clone())
        .reveal()
        .test_value(ClearModp::from(1));

    bm.clone()
        .eq(b.clone())
        .reveal()
        .test_value(ClearModp::from(0));
    bm.clone()
        .eq(bm.clone())
        .reveal()
        .test_value(ClearModp::from(1));

    bm.clone()
        .ne(b.clone())
        .reveal()
        .test_value(ClearModp::from(1));
    bm.clone()
        .ne(bm.clone())
        .reveal()
        .test_value(ClearModp::from(0));

    // Division

    // Following should all be one
    let c1 = &a / &a;
    let c2 = &b / &a;
    let c3 = &a / &b;
    let c4 = &b / &b;
    let c5 = &am / &am;
    let c6 = &bm / &am;
    let c7 = &am / &bm;
    let c8 = &bm / &bm;

    let ok = test_approx(c1, one.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c2.reveal(), one.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c3.reveal(), one.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c4.reveal(), one.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c5, one.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c6.reveal(), one.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c7.reveal(), one.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c8.reveal(), one.clone());
    ok.test_value(ClearModp::from(1));

    // Following should all be minus one
    let c1 = &am / &a;
    let c2 = &bm / &a;
    let c3 = &am / &b;
    let c4 = &bm / &b;
    let c5 = &a / &am;
    let c6 = &b / &am;
    let c7 = &a / &bm;
    let c8 = &b / &bm;

    let ok = test_approx(c1, mone.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c2.reveal(), mone.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c3.reveal(), mone.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c4.reveal(), mone.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c5, mone.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c6.reveal(), mone.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c7.reveal(), mone.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c8.reveal(), mone.clone());
    ok.test_value(ClearModp::from(1));

    // Following should all be one
    let c1 = a.clone() / &a;
    let c2 = b.clone() / &a;
    let c3 = a.clone() / &b;
    let c4 = b.clone() / &b;
    let c5 = am.clone() / &am;
    let c6 = bm.clone() / &am;
    let c7 = am.clone() / &bm;
    let c8 = bm.clone() / &bm;

    let ok = test_approx(c1, one.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c2.reveal(), one.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c3.reveal(), one.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c4.reveal(), one.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c5, one.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c6.reveal(), one.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c7.reveal(), one.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c8.reveal(), one.clone());
    ok.test_value(ClearModp::from(1));

    // Following should all be minus one
    let c1 = am.clone() / &a;
    let c2 = bm.clone() / &a;
    let c3 = am.clone() / &b;
    let c4 = bm.clone() / &b;
    let c5 = a.clone() / &am;
    let c6 = b.clone() / &am;
    let c7 = a.clone() / &bm;
    let c8 = b.clone() / &bm;

    let ok = test_approx(c1, mone.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c2.reveal(), mone.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c3.reveal(), mone.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c4.reveal(), mone.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c5, mone.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c6.reveal(), mone.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c7.reveal(), mone.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c8.reveal(), mone.clone());
    ok.test_value(ClearModp::from(1));

    // Following should all be one
    let c1 = &a / a.clone();
    let c2 = &b / a.clone();
    let c3 = &a / b.clone();
    let c4 = &b / b.clone();
    let c5 = &am / am.clone();
    let c6 = &bm / am.clone();
    let c7 = &am / bm.clone();
    let c8 = &bm / bm.clone();

    let ok = test_approx(c1, one.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c2.reveal(), one.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c3.reveal(), one.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c4.reveal(), one.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c5, one.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c6.reveal(), one.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c7.reveal(), one.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c8.reveal(), one.clone());
    ok.test_value(ClearModp::from(1));

    // Following should all be minus one
    let c1 = &am / a.clone();
    let c2 = &bm / a.clone();
    let c3 = &am / b.clone();
    let c4 = &bm / b.clone();
    let c5 = &a / am.clone();
    let c6 = &b / am.clone();
    let c7 = &a / bm.clone();
    let c8 = &b / bm.clone();

    let ok = test_approx(c1, mone.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c2.reveal(), mone.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c3.reveal(), mone.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c4.reveal(), mone.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c5, mone.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c6.reveal(), mone.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c7.reveal(), mone.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c8.reveal(), mone.clone());
    ok.test_value(ClearModp::from(1));

    // Following should all be one
    let c1 = a.clone() / a.clone();
    let c2 = b.clone() / a.clone();
    let c3 = a.clone() / b.clone();
    let c4 = b.clone() / b.clone();
    let c5 = am.clone() / am.clone();
    let c6 = bm.clone() / am.clone();
    let c7 = am.clone() / bm.clone();
    let c8 = bm.clone() / bm.clone();

    let ok = test_approx(c1, one.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c2.reveal(), one.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c3.reveal(), one.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c4.reveal(), one.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c5, one.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c6.reveal(), one.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c7.reveal(), one.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c8.reveal(), one.clone());
    ok.test_value(ClearModp::from(1));

    // Following should all be minus one
    let c1 = am.clone() / a.clone();
    let c2 = bm.clone() / a.clone();
    let c3 = am.clone() / b.clone();
    let c4 = bm.clone() / b.clone();
    let c5 = a.clone() / am.clone();
    let c6 = b.clone() / am.clone();
    let c7 = a.clone() / bm.clone();
    let c8 = b.clone() / bm.clone();

    let ok = test_approx(c1, mone.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c2.reveal(), mone.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c3.reveal(), mone.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c4.reveal(), mone.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c5, mone.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c6.reveal(), mone.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c7.reveal(), mone.clone());
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c8.reveal(), mone.clone());
    ok.test_value(ClearModp::from(1));
   
}
