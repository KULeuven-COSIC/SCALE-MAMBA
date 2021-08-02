// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![no_std]
#![no_main]
#![feature(const_evaluatable_checked, const_generics)]
#![allow(non_snake_case, incomplete_features)]

use scale_std::array::*;
use scale_std::slice::*;
use scale_std::oram::*;

#[scale::main(KAPPA = 40)]
#[inline(always)]
fn main() {
    let mut arr : Array<SecretModp, 20> = Array::uninitialized();
    let mut sli : Slice<SecretModp> = Slice::uninitialized(20);
    for i in 0..20 {
       arr.set(i, &SecretModp::from(i as i64));
       sli.set(i, &SecretModp::from(i as i64));
    }

    let a = SecretModp::from(7);
    let va = arr.read_oram::<5,40>(a);
    let vs = sli.read_oram::<5,40>(a);

    println!(" a = ",a.reveal());
    println!("va = ",va.reveal());
    println!("vs = ",vs.reveal());

    va.test_value(a);
    vs.test_value(a);

    let b = SecretModp::from(13);
    arr.write_oram::<5,40>(b,SecretModp::from(666));
    sli.write_oram::<5,40>(b,SecretModp::from(666));

    arr.get_unchecked(13).test_value(SecretModp::from(666));

    for i in 0..20 {
        println!(i,"\t",arr.get_unchecked(i).reveal(),"\t",sli.get_unchecked(i).reveal());
    }

    let c = SecretModp::from(19);
    let demux_c = Demux::<5,40>(c);
    let va = arr.read_with_demux(&demux_c);
    let vs = sli.read_with_demux(&demux_c);

    va.test_value(c);
    vs.test_value(c);

    println!(" c = ",c.reveal());
    println!("va = ",va.reveal());
    println!("vs = ",vs.reveal());

    let d = SecretModp::from(18);
    let demux_d = Demux::<5,40>(d);
    arr.write_with_demux(&demux_d, SecretModp::from(1001));
    sli.write_with_demux(&demux_d, SecretModp::from(1001));
    for i in 0..20 {
        println!(i,"\t",arr.get_unchecked(i).reveal(),"\t",sli.get_unchecked(i).reveal());
    }
    arr.get_unchecked(18).test_value(SecretModp::from(1001));

    let e = SecretModp::from(5);
    let len: i64 = 3;
    let demux_e = demux::<40>(e, len);

    let a = SecretModp::from(15);
    let va = arr.read_with_demux_and_offset(10, &demux_e);
    let vs = arr.read_with_demux_and_offset(10, &demux_e);
    va.test_value(a);
    vs.test_value(a);

    arr.write_with_demux_and_offset(10, &demux_e, SecretModp::from(2002));
    sli.write_with_demux_and_offset(10, &demux_e, SecretModp::from(2002));
    for i in 0..20 {
        println!(i,"\t",arr.get_unchecked(i).reveal(),"\t",sli.get_unchecked(i).reveal());
    }
    arr.get_unchecked(15).test_value(SecretModp::from(2002));

}


