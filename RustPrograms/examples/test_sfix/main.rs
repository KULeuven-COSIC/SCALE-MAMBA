// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![no_std]
#![no_main]
#![feature(const_evaluatable_checked, const_generics)]
#![allow(non_snake_case, incomplete_features)]

use scale_std::fixed_point::*;
use scale_std::integer::*;
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

#[scale::main(KAPPA = 40)]
#[inline(always)]
fn main() {
    let one: ClearFixed<40, 20> = ClearFixed::from(1);
    let mone: ClearFixed<40, 20> = ClearFixed::from(-1);
    let zero: ClearFixed<40, 20> = ClearFixed::from(0);

    let a: ClearFixed<40, 20> = ClearFixed::from(3.141592);
    let b: SecretFixed<40, 20> = SecretFixed::from(a);

    let t: ClearInteger<40> = ClearInteger::from(3);
    let c1: ClearFixed<40, 20> = ClearFixed::from(3);
    let c2: ClearFixed<40, 20> = ClearFixed::from(ClearModp::from(3));
    let c3: ClearFixed<40, 20> = ClearFixed::from(t);

    let rep3: i64 = 3 * (1 << 20);
    c1.rep().rep().test_value(ClearModp::from(rep3));
    c2.rep().rep().test_value(ClearModp::from(rep3));
    c3.rep().rep().test_value(ClearModp::from(rep3));

    let s: SecretInteger<40> = SecretInteger::from(3);
    let c1: SecretFixed<40, 20> = SecretFixed::from(3);
    let c2: SecretFixed<40, 20> = SecretFixed::from(ClearModp::from(3));
    let c3: SecretFixed<40, 20> = SecretFixed::from(t);
    let c4: SecretFixed<40, 20> = SecretFixed::from(s);
    let c5: SecretFixed<40, 20> = SecretFixed::from(SecretModp::from(3));
    c1.reveal().rep().rep().test_value(ClearModp::from(rep3));
    c2.reveal().rep().rep().test_value(ClearModp::from(rep3));
    c3.reveal().rep().rep().test_value(ClearModp::from(rep3));
    c4.reveal().rep().rep().test_value(ClearModp::from(rep3));
    c5.reveal().rep().rep().test_value(ClearModp::from(rep3));

    let rep3b: i64 = 3 * (1 << 30);
    let c1: ClearFixed<40, 20> = ClearFixed::from(3);
    let c2: ClearFixed<50, 30> = unsafe { c1.recast() };
    let c3: ClearFixed<40, 20> = unsafe { c2.recast() };
    c2.rep().rep().test_value(ClearModp::from(rep3b));
    c3.rep().rep().test_value(ClearModp::from(rep3));

    let c1: SecretFixed<40, 20> = SecretFixed::from(3);
    let c2: SecretFixed<50, 30> = unsafe { c1.recast() };
    let c3: SecretFixed<40, 20> = unsafe { c2.recast() };
    c2.reveal().rep().rep().test_value(ClearModp::from(rep3b));
    c3.reveal().rep().rep().test_value(ClearModp::from(rep3));

    let am: ClearFixed<40, 20> = ClearFixed::from(-3.141592);
    let bm: SecretFixed<40, 20> = SecretFixed::from(am);

    let ok = test_approx(a + am, zero);
    ok.test_value(ClearModp::from(1));
    let ok = test_approx((a + bm).reveal(), zero);
    ok.test_value(ClearModp::from(1));
    let ok = test_approx((b + am).reveal(), zero);
    ok.test_value(ClearModp::from(1));
    let ok = test_approx((b + bm).reveal(), zero);
    ok.test_value(ClearModp::from(1));

    // Test abs
    let ok = test_approx(a - am.fabs(), zero);
    ok.test_value(ClearModp::from(1));
    let ok = test_approx((a - bm.fabs()).reveal(), zero);
    ok.test_value(ClearModp::from(1));
    let ok = test_approx((b - am.fabs()).reveal(), zero);
    ok.test_value(ClearModp::from(1));
    let ok = test_approx((b - bm.fabs()).reveal(), zero);
    ok.test_value(ClearModp::from(1));

    // Test floor
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

    // Test add
    let two: ClearFixed<40, 20> = ClearFixed::from(2);
    let e = a + a;
    let t = two * a;
    let f1 = b + a;
    let f2 = a + b;
    let f3 = b + b;
    let ok = test_approx(e - t, zero);
    ok.test_value(ClearModp::from(1));
    let ok = test_approx((f1 - t).reveal(), zero);
    ok.test_value(ClearModp::from(1));
    let ok = test_approx((f2 - t).reveal(), zero);
    ok.test_value(ClearModp::from(1));
    let ok = test_approx((f3 - t).reveal(), zero);
    ok.test_value(ClearModp::from(1));

    // Test mult - Note the last bit is random, so testing equality is a bit complicated
    let e = a * a;
    let t: ClearFixed<40, 20> = ClearFixed::set(ClearInteger::set(ClearModp::from(10349019)));
    let f1 = b * a;
    let f2 = a * b;
    let f3 = b * b;
    (e - t).rep().rep().test_value(ClearModp::from(0));

    let t1 = (f1 - t).reveal().rep().rep();
    let ok = t1 * (t1 - ClearModp::from(1)) * (t1 - ClearModp::from(-1));
    ok.test_value(ClearModp::from(0));

    let t1 = (f2 - t).reveal().rep().rep();
    let ok = t1 * (t1 - ClearModp::from(1)) * (t1 - ClearModp::from(-1));
    ok.test_value(ClearModp::from(0));

    let t1 = (f3 - t).reveal().rep().rep();
    let ok = t1 * (t1 - ClearModp::from(1)) * (t1 - ClearModp::from(-1));
    ok.test_value(ClearModp::from(0));

    // TEST COMPARISONS
    a.lt(a).test_value(ClearModp::from(0));
    a.lt(am).test_value(ClearModp::from(0));

    a.le(a).test_value(ClearModp::from(1));
    a.le(am).test_value(ClearModp::from(0));

    a.gt(a).test_value(ClearModp::from(0));
    a.gt(am).test_value(ClearModp::from(1));

    a.ge(a).test_value(ClearModp::from(1));
    a.ge(am).test_value(ClearModp::from(1));

    a.eq(a).test_value(ClearModp::from(1));
    a.eq(am).test_value(ClearModp::from(0));

    a.ne(a).test_value(ClearModp::from(0));
    a.ne(am).test_value(ClearModp::from(1));

    //
    am.lt(a).test_value(ClearModp::from(1));
    am.lt(am).test_value(ClearModp::from(0));

    am.le(a).test_value(ClearModp::from(1));
    am.le(am).test_value(ClearModp::from(1));

    am.gt(a).test_value(ClearModp::from(0));
    am.gt(am).test_value(ClearModp::from(0));

    am.ge(a).test_value(ClearModp::from(0));
    am.ge(am).test_value(ClearModp::from(1));

    am.eq(a).test_value(ClearModp::from(0));
    am.eq(am).test_value(ClearModp::from(1));

    am.ne(a).test_value(ClearModp::from(1));
    am.ne(am).test_value(ClearModp::from(0));

    //
    b.lt(b).reveal().test_value(ClearModp::from(0));
    b.lt(bm).reveal().test_value(ClearModp::from(0));

    b.le(b).reveal().test_value(ClearModp::from(1));
    b.le(bm).reveal().test_value(ClearModp::from(0));

    b.gt(b).reveal().test_value(ClearModp::from(0));
    b.gt(bm).reveal().test_value(ClearModp::from(1));

    b.ge(b).reveal().test_value(ClearModp::from(1));
    b.ge(bm).reveal().test_value(ClearModp::from(1));

    b.eq(b).reveal().test_value(ClearModp::from(1));
    b.eq(bm).reveal().test_value(ClearModp::from(0));

    b.ne(b).reveal().test_value(ClearModp::from(0));
    b.ne(bm).reveal().test_value(ClearModp::from(1));

    //
    bm.lt(b).reveal().test_value(ClearModp::from(1));
    bm.lt(bm).reveal().test_value(ClearModp::from(0));

    bm.le(b).reveal().test_value(ClearModp::from(1));
    bm.le(bm).reveal().test_value(ClearModp::from(1));

    bm.gt(b).reveal().test_value(ClearModp::from(0));
    bm.gt(bm).reveal().test_value(ClearModp::from(0));

    bm.ge(b).reveal().test_value(ClearModp::from(0));
    bm.ge(bm).reveal().test_value(ClearModp::from(1));

    bm.eq(b).reveal().test_value(ClearModp::from(0));
    bm.eq(bm).reveal().test_value(ClearModp::from(1));

    bm.ne(b).reveal().test_value(ClearModp::from(1));
    bm.ne(bm).reveal().test_value(ClearModp::from(0));

    // Division

    // Following should all be one
    let c1 = a / a;
    let c2 = b / a;
    let c3 = a / b;
    let c4 = b / b;
    let c5 = am / am;
    let c6 = bm / am;
    let c7 = am / bm;
    let c8 = bm / bm;

    let ok = test_approx(c1, one);
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c2.reveal(), one);
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c3.reveal(), one);
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c4.reveal(), one);
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c5, one);
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c6.reveal(), one);
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c7.reveal(), one);
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c8.reveal(), one);
    ok.test_value(ClearModp::from(1));

    // Following should all be minus one
    let c1 = am / a;
    let c2 = bm / a;
    let c3 = am / b;
    let c4 = bm / b;
    let c5 = a / am;
    let c6 = b / am;
    let c7 = a / bm;
    let c8 = b / bm;

    let ok = test_approx(c1, mone);
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c2.reveal(), mone);
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c3.reveal(), mone);
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c4.reveal(), mone);
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c5, mone);
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c6.reveal(), mone);
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c7.reveal(), mone);
    ok.test_value(ClearModp::from(1));
    let ok = test_approx(c8.reveal(), mone);
    ok.test_value(ClearModp::from(1));
}
