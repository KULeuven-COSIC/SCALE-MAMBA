#![no_std]
#![no_main]
#![feature(const_evaluatable_checked)]
#![allow(non_snake_case, incomplete_features)]

use scale_std::integer::*;

scale::main! {
    I64_MEMORY = 0;
    SECRET_I64_MEMORY = 0;
    SECRET_MODP_MEMORY = 20000;
    CLEAR_MODP_MEMORY = 20000;
    KAPPA = 40;
}

#[inline(always)]
fn main() {

    // This should be p-24, which will be odd as p is prime.
    let a: ClearInteger<10> = ClearInteger::from(-24);
    let b: SecretInteger<10> = SecretInteger::from(a);

    /**** Test Mod2m ****/
    // This should be 8 mod 32
    let d = a.Mod2m(ConstU64::<5>, ConstBool::<true>);
    let e = b.Mod2m(ConstU64::<5>, ConstBool::<true>);
    let t = ClearModp::from(8);

    d.rep().test_value(t);
    e.reveal().rep().test_value(t);

    print!(" 8=",d);
    print!(" 8=",e.reveal(), "\n");

    /**** Test simple arithmetic ****/
    let x=a*a;
    let y=b*b;
    let t2=ClearModp::from(576);

    print!(" x = ",x);
    print!(" y = ",y.reveal(),"\n");

    x.rep().test_value(t2);
    y.reveal().rep().test_value(t2);

    /**** Test Pow2 ****/
    let b = SecretModp::from(28);
    let d = Pow2::<32,40>(b);
    d.reveal().test_value(ClearModp::from(268435456));

    /**** Test B2U ****/
    let v = SecretModp::from(5);
    let (u,_pow2v) = B2U::<8,40>(v);
    u.get(0).reveal().test_value(ClearModp::from(1));
    u.get(1).reveal().test_value(ClearModp::from(1));
    u.get(2).reveal().test_value(ClearModp::from(1));
    u.get(3).reveal().test_value(ClearModp::from(1));
    u.get(4).reveal().test_value(ClearModp::from(1));
    u.get(5).reveal().test_value(ClearModp::from(0));
    u.get(6).reveal().test_value(ClearModp::from(0));
    u.get(7).reveal().test_value(ClearModp::from(0));

    let b23 = SecretModp::from(23);
    let x23: SecretInteger::<6> = SecretInteger::from(b23);
    let b25 = SecretModp::from(25);
    let x25: SecretInteger::<6> = SecretInteger::from(b25);
    let xm23: SecretInteger::<6> = SecretInteger::from(-b23);
    let xm25: SecretInteger::<6> = SecretInteger::from(-b25);

    /**** Test TruncPr ****/
    let mut ok = ClearModp::from(1);
    for _i in 0..25 {
        let t23 = x23.TruncPr(ConstU64::<3>, ConstBool::<true>).reveal().rep();
        let t25 = x25.TruncPr(ConstU64::<3>, ConstBool::<true>).reveal().rep();
        let tm23 = xm23.TruncPr(ConstU64::<3>, ConstBool::<true>).reveal().rep();
        let tm25 = xm25.TruncPr(ConstU64::<3>, ConstBool::<true>).reveal().rep();
        // t23 should be 2 or 3, i.e. flag23=0
        let flag23 = (t23-ClearModp::from(2))*(t23-ClearModp::from(3));
        // t25 should be 3 or 4, i.e. flag25=0
        let flag25 = (t25-ClearModp::from(3))*(t25-ClearModp::from(4));
        // tm23 should be -2 or -3, i.e. flagm23=0
        let flagm23 = (tm23-ClearModp::from(-2))*(tm23-ClearModp::from(-3));
        // tm25 should be -3 or -4, i.e. flagm25=0
        let flagm25 = (tm25-ClearModp::from(-3))*(tm25-ClearModp::from(-4));
        ok = ok * (ClearModp::from(1)-flag23);
        ok = ok * (ClearModp::from(1)-flag25);
        ok = ok * (ClearModp::from(1)-flagm23);
        ok = ok * (ClearModp::from(1)-flagm25);
        //print!("i=",_i as i64," ok=",ok,"\t",t23,"\t",t25,"\t",tm23,"\t",tm25,"\n");
    }
    ok.test_value(ClearModp::from(1));


    /**** Test Trunc ****/
    ok = ClearModp::from(1);
    for _i in 0..25 {
        let t23 = x23.Trunc(ConstU64::<3>, ConstBool::<true>).reveal().rep();
        let t25 = x25.Trunc(ConstU64::<3>, ConstBool::<true>).reveal().rep();
        let tm23 = xm23.Trunc(ConstU64::<3>, ConstBool::<true>).reveal().rep();
        let tm25 = xm25.Trunc(ConstU64::<3>, ConstBool::<true>).reveal().rep();
        // t23 should be 2, i.e. flag23=0
        let flag23 = t23-ClearModp::from(2);
        // t25 should be 3, i.e. flag25=0
        let flag25 = t25-ClearModp::from(3);
        // tm23 should be -3 , i.e. flagm23=0
        let flagm23 = tm23-ClearModp::from(-3);
        // tm25 should be -4, i.e. flagm25=0
        let flagm25 = tm25-ClearModp::from(-4);
        ok = ok * (ClearModp::from(1)-flag23);
        ok = ok * (ClearModp::from(1)-flag25);
        ok = ok * (ClearModp::from(1)-flagm23);
        ok = ok * (ClearModp::from(1)-flagm25);
        //print!("i=",_i as i64," ok=",ok,"\t",t23,"\t",t25,"\t",tm23,"\t",tm25,"\n");
    }
    ok.test_value(ClearModp::from(1));

    /**** Test TruncRoundNearest ****/
    ok = ClearModp::from(1);
    for _i in 0..25 {
        let r1 = x23.TruncRoundNearest(ConstU64::<4>, ConstBool::<true>).reveal().rep();
        let r3 = x23.TruncRoundNearest(ConstU64::<3>, ConstBool::<true>).reveal().rep();
        let r6 = x23.TruncRoundNearest(ConstU64::<2>, ConstBool::<true>).reveal().rep();
        let q2 = x25.TruncRoundNearest(ConstU64::<4>, ConstBool::<true>).reveal().rep();
        let q3 = x25.TruncRoundNearest(ConstU64::<3>, ConstBool::<true>).reveal().rep();
        let q6 = x25.TruncRoundNearest(ConstU64::<2>, ConstBool::<true>).reveal().rep();
        let rm1 = xm23.TruncRoundNearest(ConstU64::<4>, ConstBool::<true>).reveal().rep();
        let rm3 = xm23.TruncRoundNearest(ConstU64::<3>, ConstBool::<true>).reveal().rep();
        let rm6 = xm23.TruncRoundNearest(ConstU64::<2>, ConstBool::<true>).reveal().rep();
        let qm2 = xm25.TruncRoundNearest(ConstU64::<4>, ConstBool::<true>).reveal().rep();
        let qm3 = xm25.TruncRoundNearest(ConstU64::<3>, ConstBool::<true>).reveal().rep();
        let qm6 = xm25.TruncRoundNearest(ConstU64::<2>, ConstBool::<true>).reveal().rep();
        let f1 = (r1-ClearModp::from(1)) * (rm1-ClearModp::from(-1));
        let f3 = (r3-ClearModp::from(3)) * (rm3-ClearModp::from(-3));
        let f6 = (r6-ClearModp::from(6)) * (rm6-ClearModp::from(-6));
        let g2 = (q2-ClearModp::from(2)) * (qm2-ClearModp::from(-2));
        let g3 = (q3-ClearModp::from(3)) * (qm3-ClearModp::from(-3));
        let g6 = (q6-ClearModp::from(6)) * (qm6-ClearModp::from(-6));
        ok = ok * (ClearModp::from(1)-f1);
        ok = ok * (ClearModp::from(1)-f3);
        ok = ok * (ClearModp::from(1)-f6);
        ok = ok * (ClearModp::from(1)-g2);
        ok = ok * (ClearModp::from(1)-g3);
        ok = ok * (ClearModp::from(1)-g6);
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
        let flag23 = t23-ClearModp::from(2);
        // t25 should be 3, i.e. flag25=0
        let flag25 = t25-ClearModp::from(3);
        // tm23 should be -3 , i.e. flagm23=0
        let flagm23 = tm23-ClearModp::from(-3);
        // tm25 should be -4, i.e. flagm25=0
        let flagm25 = tm25-ClearModp::from(-4);
        ok = ok * (ClearModp::from(1)-flag23);
        ok = ok * (ClearModp::from(1)-flag25);
        ok = ok * (ClearModp::from(1)-flagm23);
        ok = ok * (ClearModp::from(1)-flagm25);
        //print!("i=",_i as i64," ok=",ok,"\t",t23,"\t",t25,"\t",tm23,"\t",tm25,"\n");
    }
    ok.test_value(ClearModp::from(1));
}
