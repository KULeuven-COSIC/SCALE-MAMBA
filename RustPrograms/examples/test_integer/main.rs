// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![no_std]
#![no_main]
#![feature(const_evaluatable_checked)]
#![allow(non_snake_case, incomplete_features)]

use scale_std::integer::*;

#[scale::main(KAPPA = 40)]
#[inline(always)]
fn main() {
    // This should be p-24, which will be odd as p is prime.
    let a: ClearInteger<11> = ClearInteger::from(-24);
    let b: SecretInteger<11> = SecretInteger::from(a);
    let c: ClearInteger<11> = ClearInteger::from(1020);
    let d: SecretInteger<11> = SecretInteger::from(1020);
    let e: ClearInteger<11> = ClearInteger::from(24);

    /**** Test Mod2m ****/
    // This should be 8 mod 32
    let ma = a.Mod2m(5, true);
    let mb = b.Mod2m(5,true);
    let t = ClearModp::from(8);

    ma.rep().test_value(t);
    mb.reveal().rep().test_value(t);

    print!(" 8=", ma);
    print!(" 8=", mb.reveal(), "\n");

    /**** Test simple arithmetic ****/

    /* XXXX OPERATOR ARITHMETIC IS NOT COMPLETELY WORKING YET XXXXX
    let x = a * a;
    let y1 = b * b;
    let y2 = a * b;
    let y3 = b * a;

    let u = a + a;
    let v1 = b + b;
    let v2 = a + b;
    let v3 = b + a;

    let r = c + c;
    let s1 = d + d;
    let s2 = d + c;
    let s3 = c + d;

    let p = a * e;
    let q1 = b * e;
    let q2 = e * b;
    let q3 = b * e;

    let twoa: ClearInteger<10> = ClearInteger::from(-48);
    let tu = twoa.rep();
    let tx = ClearModp::from(576);
    let tr = ClearModp::from(-8);
    let tp = ClearModp::from(-576);

    print!(" a = ", a, "  ", a.rep(), "\n");
    print!(" a+a = ", twoa, "  ", twoa.rep(), "\n");

    print!(" u = ", u);
    print!(" v = ", v1.reveal(), "   : -48 \n");

    print!(" r = ", r);
    print!(" s = ", s1.reveal(), "   : 1016 \n");

    print!(" x = ", x);
    print!(" y = ", y1.reveal(), "   : 576 \n");

    print!(" p = ", p);
    print!(" q = ", q1.reveal(), "   : -576\n");

    u.rep().test_value(tu);
    v1.reveal().rep().test_value(tu);
    v2.reveal().rep().test_value(tu);
    v3.reveal().rep().test_value(tu);

    x.rep().test_value(tx);
    y1.reveal().rep().test_value(tx);
    y2.reveal().rep().test_value(tx);
    y3.reveal().rep().test_value(tx);

    r.rep().test_value(tr);
    s1.reveal().rep().test_value(tr);
    s2.reveal().rep().test_value(tr);
    s3.reveal().rep().test_value(tr);

    p.rep().test_value(tp);
    q1.reveal().rep().test_value(tp);
    q2.reveal().rep().test_value(tp);
    q3.reveal().rep().test_value(tp);
    */

    /**** Test Pow2 ****/
    let b = SecretModp::from(28);
    let d = Pow2::<32, 40>(b);
    d.reveal().test_value(ClearModp::from(268435456));

    /**** Test B2U ****/
    let v = SecretModp::from(5);
    let (u, _pow2v) = B2U::<8, 40>(v);
    u.get_unchecked(0).reveal().test_value(ClearModp::from(1));
    u.get_unchecked(1).reveal().test_value(ClearModp::from(1));
    u.get_unchecked(2).reveal().test_value(ClearModp::from(1));
    u.get_unchecked(3).reveal().test_value(ClearModp::from(1));
    u.get_unchecked(4).reveal().test_value(ClearModp::from(1));
    u.get_unchecked(5).reveal().test_value(ClearModp::from(0));
    u.get_unchecked(6).reveal().test_value(ClearModp::from(0));
    u.get_unchecked(7).reveal().test_value(ClearModp::from(0));

    let b23 = SecretModp::from(23);
    let x23: SecretInteger<6> = SecretInteger::from(b23);
    let b25 = SecretModp::from(25);
    let x25: SecretInteger<6> = SecretInteger::from(b25);
    let xm23: SecretInteger<6> = SecretInteger::from(-b23);
    let xm25: SecretInteger<6> = SecretInteger::from(-b25);

    /**** Test TruncPr ****/
    let mut ok = ClearModp::from(1);
    for _i in 0..25 {
        let t23 = x23.TruncPr(3, true).reveal().rep();
        let t25 = x25.TruncPr(3, true).reveal().rep();
        let tm23 = xm23
            .TruncPr(3, true)
            .reveal()
            .rep();
        let tm25 = xm25
            .TruncPr(3, true)
            .reveal()
            .rep();
        // t23 should be 2 or 3, i.e. flag23=0
        let flag23 = (t23 - ClearModp::from(2)) * (t23 - ClearModp::from(3));
        // t25 should be 3 or 4, i.e. flag25=0
        let flag25 = (t25 - ClearModp::from(3)) * (t25 - ClearModp::from(4));
        // tm23 should be -2 or -3, i.e. flagm23=0
        let flagm23 = (tm23 - ClearModp::from(-2)) * (tm23 - ClearModp::from(-3));
        // tm25 should be -3 or -4, i.e. flagm25=0
        let flagm25 = (tm25 - ClearModp::from(-3)) * (tm25 - ClearModp::from(-4));
        ok = ok * (ClearModp::from(1) - flag23);
        ok = ok * (ClearModp::from(1) - flag25);
        ok = ok * (ClearModp::from(1) - flagm23);
        ok = ok * (ClearModp::from(1) - flagm25);
        //print!("i=",_i as i64," ok=",ok,"\t",t23,"\t",t25,"\t",tm23,"\t",tm25,"\n");
    }
    ok.test_value(ClearModp::from(1));

    /**** Test Trunc ****/
    ok = ClearModp::from(1);
    for _i in 0..25 {
        let t23 = x23.Trunc(3, true).reveal().rep();
        let t25 = x25.Trunc(3, true).reveal().rep();
        let tm23 = xm23.Trunc(3, true).reveal().rep();
        let tm25 = xm25.Trunc(3, true).reveal().rep();
        // t23 should be 2, i.e. flag23=0
        let flag23 = t23 - ClearModp::from(2);
        // t25 should be 3, i.e. flag25=0
        let flag25 = t25 - ClearModp::from(3);
        // tm23 should be -3 , i.e. flagm23=0
        let flagm23 = tm23 - ClearModp::from(-3);
        // tm25 should be -4, i.e. flagm25=0
        let flagm25 = tm25 - ClearModp::from(-4);
        ok = ok * (ClearModp::from(1) - flag23);
        ok = ok * (ClearModp::from(1) - flag25);
        ok = ok * (ClearModp::from(1) - flagm23);
        ok = ok * (ClearModp::from(1) - flagm25);
        //print!("i=",_i as i64," ok=",ok,"\t",t23,"\t",t25,"\t",tm23,"\t",tm25,"\n");
    }
    ok.test_value(ClearModp::from(1));

    /**** Test TruncRoundNearest ****/
    ok = ClearModp::from(1);
    for _i in 0..25 {
        let r1 = x23
            .TruncRoundNearest(4, true)
            .reveal()
            .rep();
        let r3 = x23
            .TruncRoundNearest(3, true)
            .reveal()
            .rep();
        let r6 = x23
            .TruncRoundNearest(2, true)
            .reveal()
            .rep();
        let q2 = x25
            .TruncRoundNearest(4, true)
            .reveal()
            .rep();
        let q3 = x25
            .TruncRoundNearest(3, true)
            .reveal()
            .rep();
        let q6 = x25
            .TruncRoundNearest(2, true)
            .reveal()
            .rep();
        let rm1 = xm23
            .TruncRoundNearest(4, true)
            .reveal()
            .rep();
        let rm3 = xm23
            .TruncRoundNearest(3, true)
            .reveal()
            .rep();
        let rm6 = xm23
            .TruncRoundNearest(2, true)
            .reveal()
            .rep();
        let qm2 = xm25
            .TruncRoundNearest(4, true)
            .reveal()
            .rep();
        let qm3 = xm25
            .TruncRoundNearest(3, true)
            .reveal()
            .rep();
        let qm6 = xm25
            .TruncRoundNearest(2, true)
            .reveal()
            .rep();
        let f1 = (r1 - ClearModp::from(1)) * (rm1 - ClearModp::from(-1));
        let f3 = (r3 - ClearModp::from(3)) * (rm3 - ClearModp::from(-3));
        let f6 = (r6 - ClearModp::from(6)) * (rm6 - ClearModp::from(-6));
        let g2 = (q2 - ClearModp::from(2)) * (qm2 - ClearModp::from(-2));
        let g3 = (q3 - ClearModp::from(3)) * (qm3 - ClearModp::from(-3));
        let g6 = (q6 - ClearModp::from(6)) * (qm6 - ClearModp::from(-6));
        ok = ok * (ClearModp::from(1) - f1);
        ok = ok * (ClearModp::from(1) - f3);
        ok = ok * (ClearModp::from(1) - f6);
        ok = ok * (ClearModp::from(1) - g2);
        ok = ok * (ClearModp::from(1) - g3);
        ok = ok * (ClearModp::from(1) - g6);
    }
    ok.test_value(ClearModp::from(1));

    /**** Test ObliviousTrunc ****/
    let m3 = SecretModp::from(3);
    ok = ClearModp::from(1);
    for _i in 0..25 {
        let t23 = x23.ObliviousTrunc(m3).reveal().rep();
        let t25 = x25.ObliviousTrunc(m3).reveal().rep();
        let tm23 = xm23.ObliviousTrunc(m3).reveal().rep();
        let tm25 = xm25.ObliviousTrunc(m3).reveal().rep();
        // t23 should be 2, i.e. flag23=0
        let flag23 = t23 - ClearModp::from(2);
        // t25 should be 3, i.e. flag25=0
        let flag25 = t25 - ClearModp::from(3);
        // tm23 should be -3 , i.e. flagm23=0
        let flagm23 = tm23 - ClearModp::from(-3);
        // tm25 should be -4, i.e. flagm25=0
        let flagm25 = tm25 - ClearModp::from(-4);
        ok = ok * (ClearModp::from(1) - flag23);
        ok = ok * (ClearModp::from(1) - flag25);
        ok = ok * (ClearModp::from(1) - flagm23);
        ok = ok * (ClearModp::from(1) - flagm25);
        //print!("i=",_i as i64," ok=",ok,"\t",t23,"\t",t25,"\t",tm23,"\t",tm25,"\n");
    }
    ok.test_value(ClearModp::from(1));

    /**** Test Comparisons ****/
    let c1: ClearInteger<10> = ClearInteger::from(-24);
    let c2: ClearInteger<10> = ClearInteger::from(-26);
    let c3: ClearInteger<10> = ClearInteger::from(24);
    let c4: ClearInteger<10> = ClearInteger::from(26);
    let s1: SecretInteger<10> = SecretInteger::from(c1);
    let s2: SecretInteger<10> = SecretInteger::from(c2);
    let s3: SecretInteger<10> = SecretInteger::from(c3);
    let s4: SecretInteger<10> = SecretInteger::from(c4);

    //
    c1.lt(c1).test_value(ClearModp::from(0));
    c1.lt(c2).test_value(ClearModp::from(0));
    c1.lt(c3).test_value(ClearModp::from(1));
    c1.lt(c4).test_value(ClearModp::from(1));

    c1.le(c1).test_value(ClearModp::from(1));
    c1.le(c2).test_value(ClearModp::from(0));
    c1.le(c3).test_value(ClearModp::from(1));
    c1.le(c4).test_value(ClearModp::from(1));

    c1.gt(c1).test_value(ClearModp::from(0));
    c1.gt(c2).test_value(ClearModp::from(1));
    c1.gt(c3).test_value(ClearModp::from(0));
    c1.gt(c4).test_value(ClearModp::from(0));

    c1.ge(c1).test_value(ClearModp::from(1));
    c1.ge(c2).test_value(ClearModp::from(1));
    c1.ge(c3).test_value(ClearModp::from(0));
    c1.ge(c4).test_value(ClearModp::from(0));

    c1.eq(c1).test_value(ClearModp::from(1));
    c1.eq(c2).test_value(ClearModp::from(0));
    c1.eq(c3).test_value(ClearModp::from(0));
    c1.eq(c4).test_value(ClearModp::from(0));

    c1.ne(c1).test_value(ClearModp::from(0));
    c1.ne(c2).test_value(ClearModp::from(1));
    c1.ne(c3).test_value(ClearModp::from(1));
    c1.ne(c4).test_value(ClearModp::from(1));

    //
    c2.lt(c1).test_value(ClearModp::from(1));
    c2.lt(c2).test_value(ClearModp::from(0));
    c2.lt(c3).test_value(ClearModp::from(1));
    c2.lt(c4).test_value(ClearModp::from(1));

    c2.le(c1).test_value(ClearModp::from(1));
    c2.le(c2).test_value(ClearModp::from(1));
    c2.le(c3).test_value(ClearModp::from(1));
    c2.le(c4).test_value(ClearModp::from(1));

    c2.gt(c1).test_value(ClearModp::from(0));
    c2.gt(c2).test_value(ClearModp::from(0));
    c2.gt(c3).test_value(ClearModp::from(0));
    c2.gt(c4).test_value(ClearModp::from(0));

    c2.ge(c1).test_value(ClearModp::from(0));
    c2.ge(c2).test_value(ClearModp::from(1));
    c2.ge(c3).test_value(ClearModp::from(0));
    c2.ge(c4).test_value(ClearModp::from(0));

    c2.eq(c1).test_value(ClearModp::from(0));
    c2.eq(c2).test_value(ClearModp::from(1));
    c2.eq(c3).test_value(ClearModp::from(0));
    c2.eq(c4).test_value(ClearModp::from(0));

    c2.ne(c1).test_value(ClearModp::from(1));
    c2.ne(c2).test_value(ClearModp::from(0));
    c2.ne(c3).test_value(ClearModp::from(1));
    c2.ne(c4).test_value(ClearModp::from(1));

    //
    c3.lt(c1).test_value(ClearModp::from(0));
    c3.lt(c2).test_value(ClearModp::from(0));
    c3.lt(c3).test_value(ClearModp::from(0));
    c3.lt(c4).test_value(ClearModp::from(1));

    c3.le(c1).test_value(ClearModp::from(0));
    c3.le(c2).test_value(ClearModp::from(0));
    c3.le(c3).test_value(ClearModp::from(1));
    c3.le(c4).test_value(ClearModp::from(1));

    c3.gt(c1).test_value(ClearModp::from(1));
    c3.gt(c2).test_value(ClearModp::from(1));
    c3.gt(c3).test_value(ClearModp::from(0));
    c3.gt(c4).test_value(ClearModp::from(0));

    c3.ge(c1).test_value(ClearModp::from(1));
    c3.ge(c2).test_value(ClearModp::from(1));
    c3.ge(c3).test_value(ClearModp::from(1));
    c3.ge(c4).test_value(ClearModp::from(0));

    c3.eq(c1).test_value(ClearModp::from(0));
    c3.eq(c2).test_value(ClearModp::from(0));
    c3.eq(c3).test_value(ClearModp::from(1));
    c3.eq(c4).test_value(ClearModp::from(0));

    c3.ne(c1).test_value(ClearModp::from(1));
    c3.ne(c2).test_value(ClearModp::from(1));
    c3.ne(c3).test_value(ClearModp::from(0));
    c3.ne(c4).test_value(ClearModp::from(1));

    //
    c4.lt(c1).test_value(ClearModp::from(0));
    c4.lt(c2).test_value(ClearModp::from(0));
    c4.lt(c3).test_value(ClearModp::from(0));
    c4.lt(c4).test_value(ClearModp::from(0));

    c4.le(c1).test_value(ClearModp::from(0));
    c4.le(c2).test_value(ClearModp::from(0));
    c4.le(c3).test_value(ClearModp::from(0));
    c4.le(c4).test_value(ClearModp::from(1));

    c4.gt(c1).test_value(ClearModp::from(1));
    c4.gt(c2).test_value(ClearModp::from(1));
    c4.gt(c3).test_value(ClearModp::from(1));
    c4.gt(c4).test_value(ClearModp::from(0));

    c4.ge(c1).test_value(ClearModp::from(1));
    c4.ge(c2).test_value(ClearModp::from(1));
    c4.ge(c3).test_value(ClearModp::from(1));
    c4.ge(c4).test_value(ClearModp::from(1));

    c4.eq(c1).test_value(ClearModp::from(0));
    c4.eq(c2).test_value(ClearModp::from(0));
    c4.eq(c3).test_value(ClearModp::from(0));
    c4.eq(c4).test_value(ClearModp::from(1));

    c4.ne(c1).test_value(ClearModp::from(1));
    c4.ne(c2).test_value(ClearModp::from(1));
    c4.ne(c3).test_value(ClearModp::from(1));
    c4.ne(c4).test_value(ClearModp::from(0));

    //
    s1.lt(s1).reveal().test_value(ClearModp::from(0));
    s1.lt(s2).reveal().test_value(ClearModp::from(0));
    s1.lt(s3).reveal().test_value(ClearModp::from(1));
    s1.lt(s4).reveal().test_value(ClearModp::from(1));

    s1.le(s1).reveal().test_value(ClearModp::from(1));
    s1.le(s2).reveal().test_value(ClearModp::from(0));
    s1.le(s3).reveal().test_value(ClearModp::from(1));
    s1.le(s4).reveal().test_value(ClearModp::from(1));

    s1.gt(s1).reveal().test_value(ClearModp::from(0));
    s1.gt(s2).reveal().test_value(ClearModp::from(1));
    s1.gt(s3).reveal().test_value(ClearModp::from(0));
    s1.gt(s4).reveal().test_value(ClearModp::from(0));

    s1.ge(s1).reveal().test_value(ClearModp::from(1));
    s1.ge(s2).reveal().test_value(ClearModp::from(1));
    s1.ge(s3).reveal().test_value(ClearModp::from(0));
    s1.ge(s4).reveal().test_value(ClearModp::from(0));

    s1.eq(s1).reveal().test_value(ClearModp::from(1));
    s1.eq(s2).reveal().test_value(ClearModp::from(0));
    s1.eq(s3).reveal().test_value(ClearModp::from(0));
    s1.eq(s4).reveal().test_value(ClearModp::from(0));

    s1.ne(s1).reveal().test_value(ClearModp::from(0));
    s1.ne(s2).reveal().test_value(ClearModp::from(1));
    s1.ne(s3).reveal().test_value(ClearModp::from(1));
    s1.ne(s4).reveal().test_value(ClearModp::from(1));

    //
    s2.lt(s1).reveal().test_value(ClearModp::from(1));
    s2.lt(s2).reveal().test_value(ClearModp::from(0));
    s2.lt(s3).reveal().test_value(ClearModp::from(1));
    s2.lt(s4).reveal().test_value(ClearModp::from(1));

    s2.le(s1).reveal().test_value(ClearModp::from(1));
    s2.le(s2).reveal().test_value(ClearModp::from(1));
    s2.le(s3).reveal().test_value(ClearModp::from(1));
    s2.le(s4).reveal().test_value(ClearModp::from(1));

    s2.gt(s1).reveal().test_value(ClearModp::from(0));
    s2.gt(s2).reveal().test_value(ClearModp::from(0));
    s2.gt(s3).reveal().test_value(ClearModp::from(0));
    s2.gt(s4).reveal().test_value(ClearModp::from(0));

    s2.ge(s1).reveal().test_value(ClearModp::from(0));
    s2.ge(s2).reveal().test_value(ClearModp::from(1));
    s2.ge(s3).reveal().test_value(ClearModp::from(0));
    s2.ge(s4).reveal().test_value(ClearModp::from(0));

    s2.eq(s1).reveal().test_value(ClearModp::from(0));
    s2.eq(s2).reveal().test_value(ClearModp::from(1));
    s2.eq(s3).reveal().test_value(ClearModp::from(0));
    s2.eq(s4).reveal().test_value(ClearModp::from(0));

    s2.ne(s1).reveal().test_value(ClearModp::from(1));
    s2.ne(s2).reveal().test_value(ClearModp::from(0));
    s2.ne(s3).reveal().test_value(ClearModp::from(1));
    s2.ne(s4).reveal().test_value(ClearModp::from(1));

    //
    s3.lt(s1).reveal().test_value(ClearModp::from(0));
    s3.lt(s2).reveal().test_value(ClearModp::from(0));
    s3.lt(s3).reveal().test_value(ClearModp::from(0));
    s3.lt(s4).reveal().test_value(ClearModp::from(1));

    s3.le(s1).reveal().test_value(ClearModp::from(0));
    s3.le(s2).reveal().test_value(ClearModp::from(0));
    s3.le(s3).reveal().test_value(ClearModp::from(1));
    s3.le(s4).reveal().test_value(ClearModp::from(1));

    s3.gt(s1).reveal().test_value(ClearModp::from(1));
    s3.gt(s2).reveal().test_value(ClearModp::from(1));
    s3.gt(s3).reveal().test_value(ClearModp::from(0));
    s3.gt(s4).reveal().test_value(ClearModp::from(0));

    s3.ge(s1).reveal().test_value(ClearModp::from(1));
    s3.ge(s2).reveal().test_value(ClearModp::from(1));
    s3.ge(s3).reveal().test_value(ClearModp::from(1));
    s3.ge(s4).reveal().test_value(ClearModp::from(0));

    s3.eq(s1).reveal().test_value(ClearModp::from(0));
    s3.eq(s2).reveal().test_value(ClearModp::from(0));
    s3.eq(s3).reveal().test_value(ClearModp::from(1));
    s3.eq(s4).reveal().test_value(ClearModp::from(0));

    s3.ne(s1).reveal().test_value(ClearModp::from(1));
    s3.ne(s2).reveal().test_value(ClearModp::from(1));
    s3.ne(s3).reveal().test_value(ClearModp::from(0));
    s3.ne(s4).reveal().test_value(ClearModp::from(1));

    //
    s4.lt(s1).reveal().test_value(ClearModp::from(0));
    s4.lt(s2).reveal().test_value(ClearModp::from(0));
    s4.lt(s3).reveal().test_value(ClearModp::from(0));
    s4.lt(s4).reveal().test_value(ClearModp::from(0));

    s4.le(s1).reveal().test_value(ClearModp::from(0));
    s4.le(s2).reveal().test_value(ClearModp::from(0));
    s4.le(s3).reveal().test_value(ClearModp::from(0));
    s4.le(s4).reveal().test_value(ClearModp::from(1));

    s4.gt(s1).reveal().test_value(ClearModp::from(1));
    s4.gt(s2).reveal().test_value(ClearModp::from(1));
    s4.gt(s3).reveal().test_value(ClearModp::from(1));
    s4.gt(s4).reveal().test_value(ClearModp::from(0));

    s4.ge(s1).reveal().test_value(ClearModp::from(1));
    s4.ge(s2).reveal().test_value(ClearModp::from(1));
    s4.ge(s3).reveal().test_value(ClearModp::from(1));
    s4.ge(s4).reveal().test_value(ClearModp::from(1));

    s4.eq(s1).reveal().test_value(ClearModp::from(0));
    s4.eq(s2).reveal().test_value(ClearModp::from(0));
    s4.eq(s3).reveal().test_value(ClearModp::from(0));
    s4.eq(s4).reveal().test_value(ClearModp::from(1));

    s4.ne(s1).reveal().test_value(ClearModp::from(1));
    s4.ne(s2).reveal().test_value(ClearModp::from(1));
    s4.ne(s3).reveal().test_value(ClearModp::from(1));
    s4.ne(s4).reveal().test_value(ClearModp::from(0));
}
