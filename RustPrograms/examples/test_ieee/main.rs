#![no_std]
#![no_main]
#![feature(const_evaluatable_checked)]
#![allow(non_snake_case)]

use scale_std::secret_ieee::*;

scale::main! {
    I64_MEMORY = 0;
    SECRET_I64_MEMORY = 0;
    SECRET_MODP_MEMORY = 0;
    CLEAR_MODP_MEMORY = 0;
    KAPPA = 40;
}

#[inline(always)]
fn main() {
    let c= ClearIEEE::from(3.141592);
    print!(c,"\n");

    let sc=SecretIEEE::from(c);
    print!(sc.reveal(),"\n");

    let i: i64= 1677216;
    let si= SecretI64::from(i);
    let sfi=SecretIEEE::from(si);
    print!(sfi.reveal(),"\n");

    let ic=SecretI64::from(sc);
    print!(ic.reveal(),"\n");

    let is=SecretI64::from(si);
    print!(is.reveal(),"\n");

    let two= SecretIEEE::from(2);
    let a1=sc+sc;
    let a2=two*sc;
    let a3=sfi/two;
    let a4=a3+a3;
    let a5=a1*a1;
    let a6=a5.sqrt();
    (a1-a2).reveal().rep().test_value(0);
    (a4-sfi).reveal().rep().test_value(0);
    (a6-a1).reveal().rep().test_value(0);

    let cmpa=sc.lt(sfi);
    let cmpb=sc.ge(sfi);
    let cmpc=sc.eq(sc);
    let cmpd=sc.ne(sc);
    let cmpe=sc.le(sfi);
    let cmpf=sc.ge(sfi);
    let cmpg=sc.le(sc);
    let cmph=sc.ge(sc);
    cmpa.reveal().test_value(true);
    cmpb.reveal().test_value(false);
    cmpc.reveal().test_value(true);
    cmpd.reveal().test_value(false);
    cmpe.reveal().test_value(true);
    cmpf.reveal().test_value(false);
    cmpg.reveal().test_value(true);
    cmph.reveal().test_value(true);

    let cza=sc.ltz();
    let czb=sc.lez();
    let czc=sc.gtz();
    let czd=sc.gez();
    let cze=sc.eqz();
    let czf=sc.nez();
    cza.reveal().test_value(false);
    czb.reveal().test_value(false);
    czc.reveal().test_value(true);
    czd.reveal().test_value(true);
    cze.reveal().test_value(false);
    czf.reveal().test_value(true);
}
