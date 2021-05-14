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
    let a = 5424_i64;
    let b = 3543_i64;
    let x = black_box(i64::max_value());
    x.test_value(i64::max_value());
    let y = black_box(i64::min_value());
    y.test_value(i64::min_value());

    let ca = a;
    let cb = b;
    let sa = SecretI64::from(ca);
    let sb = SecretI64::from(cb);

    // For testing runtime real clear operations
    let rca = black_box(a);
    let rcb = black_box(b);

    SecretI64::from(ConstI32::<10>).test();
    1_i64.test();

    // #test  CONVSREGSINT and CONVSINTSREG
    let sia = SecretModp::from(sa); // convsregsint
    sia.test();
    let sap = SecretI64::from(sia); // convsintsreg
    let sam = SecretI64::from(-sia); // convsintsreg
    sap.test();
    sap.reveal().test_value(a);
    sam.test();
    sam.reveal().test_value(-a);

    // # Test CONVMODP and CONVINT
    let cint_a = ClearModp::from(rca);
    let _sint_a = SecretModp::from(cint_a);
    let regint_a = i64::from(cint_a);
    cint_a.test();
    regint_a.test();

    // # test CONVSBITSINT, CONVSINTSBIT,  etc
    let a0 = SecretModp::from(ConstI32::<0>);
    let b0 = SecretBit::from(false);
    let a1 = SecretModp::from(ConstI32::<1>);
    let b1 = SecretBit::from(true);
    let d0 = SecretI64::from(ConstI32::<0>);
    let d1 = SecretI64::from(ConstI32::<1>);
    let bit_a0 = SecretBit::from(a0);
    let bit_a1 = SecretBit::from(a1);
    let mod_b0 = SecretModp::from(b0);
    let mod_b1 = SecretModp::from(b1);
    let si_b0 = SecretI64::from(b0);
    let si_b1 = SecretI64::from(b1);

    bit_a0.test_value(b0);
    bit_a1.test_value(b1);
    mod_b0.test_value(a0);
    mod_b1.test_value(a1);
    si_b0.test_value(d0);
    si_b1.test_value(d1);

    // #test ADDSINT, ADDSINTC, ADDINT
    (ca + cb).test();
    (sa + cb).test();
    (ca + sb).test();
    (sa + sb).test();
    (ca + b).test();
    (a + cb).test();
    (sa + b).test();
    (a + sb).test();

    (rca + rcb).test();
    (rca + b).test();
    (a + rcb).test();

    // #test SUBSINT, SUBSINTC, SUBCINTs, SUBINT
    (ca - cb).test();
    (sa - cb).test();
    (ca - sb).test();
    (sa - sb).test();

    (cb - a).test();
    (b - ca).test();
    (sb - a).test();
    (b - sa).test();

    (rca - rcb).test();
    (rca - b).test();
    (b - rca).test();

    // #test MULSINT, MULSINTC, MULINT
    (ca * cb).test();
    (sa * cb).test();
    (ca * sb).test();
    (sa * sb).test();

    (ca * b).test();
    (a * cb).test();
    (sa * b).test();
    (a * sb).test();

    (rca * rcb).test();
    (a * cb).test();
    (rca * b).test();

    // #test DIVSINT (tests all combinations and signs)
    let divisible = a - a % b;
    let regdivisible: i64 = black_box(divisible);
    let sregdivisible = SecretI64::from(divisible);
    (regdivisible / cb).test();
    (regdivisible / b).test();
    (regdivisible / sb).test();
    (divisible / cb).test();
    (divisible / sb).test();
    (sregdivisible / cb).test();
    (sregdivisible / b).test();
    (sregdivisible / sb).test();

    (ca ^ 1).test();
    (-sa).test();
    (-regdivisible / cb).test();
    (-regdivisible / b).test();
    (-regdivisible / sb).test();
    (-divisible / cb).test();
    (-divisible / sb).test();
    (-sregdivisible / cb).test();
    (-sregdivisible / b).test();
    (-sregdivisible / sb).test();

    let cdivisible = sregdivisible.reveal();

    (cdivisible / (-cb)).test();
    (cdivisible / (-b)).test();
    (cdivisible / (-sb)).test();
    (divisible / (-cb)).test();
    (divisible / (-sb)).test();
    (cdivisible / (-cb)).test();
    (cdivisible / (-b)).test();
    (cdivisible / (-sb)).test();

    (-cdivisible / (-cb)).test();
    (-divisible / (-b)).test();
    (-cdivisible / (-sb)).test();
    ((-divisible) / (-cb)).test();
    ((-divisible) / (-sb)).test();

    (regdivisible / rcb).test();
    (divisible / rcb).test();
    (sregdivisible / rcb).test();
    (rca ^ 1).test();
    (-regdivisible / rcb).test();
    (-divisible / rcb).test();
    (-sregdivisible / rcb).test();
    (cdivisible / (-rcb)).test();
    (divisible / (-rcb)).test();
    (cdivisible / (-rcb)).test();
    (-cdivisible / (-rcb)).test();
    ((-divisible) / (-rcb)).test();

    // #test SHR/SHL
    (sa << ConstU32::<3>).test();
    (sb << ConstU32::<11>).test();
    let saa = sa << ConstU32::<3>;
    let sbb = sb << ConstU32::<11>;
    (saa >> ConstU32::<3>).test();
    (sbb >> ConstU32::<3>).test();

    (rca << 3).test();
    (rcb << 11).test();
    let rcaa = rca << 3;
    let rcbb = rcb << 11;
    (rcaa >> 3).test();
    (rcbb >> 3).test();

    // # test INT MAX (instructions for sr to sr only)
    let max = i64::max_value(); // 2^{64-1)-1
    let int_max = SecretI64::from(max);
    let min = i64::min_value();
    let int_min = SecretI64::from(min);
    let zero = SecretI64::from(ConstI32::<0>);
    (1 - int_max).lt(zero).test();
    (int_max - 1).le(int_max).test();
    (int_max - 1).ge(int_max).test();
    (int_max - 1).lt(int_max).test();
    (int_max - 1).gt(int_max).test();
    int_min.lt(int_min + 1).test();
    int_min.le(int_min + 1).test();
    int_min.le(int_min).test();
    int_max.ge(zero).test();
    int_min.eq(int_min).test();
    int_max.eq(int_max).test();

    // #test LTINT, GTINT EQINT (instructions for r to r only)
    (rca < rcb).test();
    (rca < rca).test();
    (rca < rca + 1).test();
    (-rca < rca).test();
    (-rcb < rcb).test();
    (rca < -rcb).test();
    (-rca < -rcb).test();

    (rca > rcb).test();
    (rca > rca).test();
    (rca > rca + 1).test();
    (-rca > rca).test();
    (-rcb > rcb).test();
    (rca > -rcb).test();
    (-rca > -rcb).test();

    (rca <= rcb).test();
    (rca <= rca).test();
    (rca <= rca + 1).test();
    (-rca <= rca).test();
    (-rcb <= rcb).test();
    (rca <= -rcb).test();
    (-rca <= -rcb).test();

    (rca >= rcb).test();
    (rca >= rca).test();
    (rca >= (rca + 1)).test();
    (-rca >= rca).test();
    (-rcb >= rcb).test();
    (rca >= -rcb).test();
    (-rca >= -rcb).test();

    (rca == rcb).test();
    (rca == rca).test();
    (rca == rca + 1).test();
    (-rca == rca).test();
    (-rcb == rcb).test();
    (rca == -rcb).test();
    (-rca == -rcb).test();

    (rca != rcb).test();
    (rca != rca).test();
    (rca != rca + 1).test();
    (-rca != rca).test();
    (-rcb != rcb).test();
    (rca != -rcb).test();
    (-rca != -rcb).test();

    // #test LTSINT, GTSINT EQSINT (instructions for sr to sr only)
    sa.lt(sb).test();
    sa.lt(sa).test();
    sa.lt(sa + 1).test();
    (-sa).lt(sa).test();
    (-sb).lt(sb).test();
    sa.lt(-sb).test();
    (-sa).lt(-sb).test();

    sa.gt(sb).test();
    sa.gt(sa).test();
    sa.gt(sa + 1).test();
    (-sa).gt(sa).test();
    (-sb).gt(sb).test();
    sa.gt(-sb).test();
    (-sa).gt(-sb).test();

    sa.le(sb).test();
    sa.le(sa).test();
    sa.le(sa + 1).test();
    (-sa).le(sa).test();
    (-sb).le(sb).test();
    sa.le(-sb).test();
    (-sa).le(-sb).test();

    sa.ge(sb).test();
    sa.ge(sa).test();
    sa.ge(sa + 1).test();
    (-sa).ge(sa).test();
    (-sb).ge(sb).test();
    sa.ge(-sb).test();
    (-sa).ge(-sb).test();

    sa.eq(sb).test();
    sa.eq(sa).test();
    sa.eq(sa + 1).test();
    (-sa).eq(sa).test();
    (-sb).eq(sb).test();
    sa.eq(-sb).test();
    (-sa).eq(-sb).test();

    sa.ne(sb).test();
    sa.ne(sa).test();
    sa.ne(sa + 1).test();
    (-sa).ne(sa).test();
    (-sb).ne(sb).test();
    sa.ne(-sb).test();
    (-sa).ne(-sb).test();

    // #test LTSINT, GTSINT EQSINT (instructions for r to sr only)
    sa.lt(rcb).test();
    sa.lt(rca).test();
    sa.lt(rca + 1).test();
    (-sa).lt(rca).test();
    (-sb).lt(rcb).test();
    sa.lt(-rcb).test();
    (-sa).lt(-rcb).test();

    sa.gt(rcb).test();
    sa.gt(rca).test();
    sa.gt(rca + 1).test();
    (-sa).gt(rca).test();
    (-sb).gt(rcb).test();
    sa.gt(-rcb).test();
    (-sa).gt(-rcb).test();

    sa.le(rcb).test();
    sa.le(rca).test();
    sa.le(rca + 1).test();
    (-sa).le(rca).test();
    (-sb).le(rcb).test();
    sa.le(-rcb).test();
    (-sa).le(-rcb).test();

    sa.ge(rcb).test();
    sa.ge(rca).test();
    sa.ge(rca + 1).test();
    (-sa).ge(rca).test();
    (-sb).ge(rcb).test();
    sa.ge(-rcb).test();
    (-sa).ge(-rcb).test();

    sa.eq(rcb).test();
    sa.eq(rca).test();
    sa.eq(rca + 1).test();
    (-sa).eq(rca).test();
    (-sb).eq(rcb).test();
    sa.eq(-rcb).test();
    (-sa).eq(-rcb).test();

    sa.ne(rcb).test();
    sa.ne(rca).test();
    sa.ne(rca + 1).test();
    (-sa).ne(rca).test();
    (-sb).ne(rcb).test();
    sa.ne(-rcb).test();
    (-sa).ne(-rcb).test();

    // Versions with compare to zero
    (sa - rcb).ltz().test();
    (sa - rca).ltz().test();
    (sa - rca - 1).ltz().test();
    (-sa - rca).ltz().test();
    (-sb - rcb).ltz().test();
    (sa + rcb).ltz().test();
    (-sa + rcb).ltz().test();

    (sa - rcb).gtz().test();
    (sa - rca).gtz().test();
    (sa - rca - 1).gtz().test();
    (-sa - rca).gtz().test();
    (-sb - rcb).gtz().test();
    (sa + rcb).gtz().test();
    (-sa + rcb).gtz().test();

    (sa - rcb).lez().test();
    (sa - rca).lez().test();
    (sa - rca - 1).lez().test();
    (-sa - rca).lez().test();
    (-sb - rcb).lez().test();
    (sa + rcb).lez().test();
    (-sa + rcb).lez().test();

    (sa - rcb).gez().test();
    (sa - rca).gez().test();
    (sa - rca - 1).gez().test();
    (-sa - rca).gez().test();
    (-sb - rcb).gez().test();
    (sa + rcb).gez().test();
    (-sa + rcb).gez().test();

    (sa - rcb).eqz().test();
    (sa - rca).eqz().test();
    (sa - rca - 1).eqz().test();
    (-sa - rca).eqz().test();
    (-sb - rcb).eqz().test();
    (sa + rcb).eqz().test();
    (-sa + rcb).eqz().test();

    (sa - rcb).nez().test();
    (sa - rca).nez().test();
    (sa - rca - 1).nez().test();
    (-sa - rca).nez().test();
    (-sb - rcb).nez().test();
    (sa + rcb).nez().test();
    (-sa + rcb).nez().test();

    // #test LTSINT, GTSINT EQSINT (instructions for r to sr only)
    rca.lt(sb).test();
    rca.lt(sa).test();
    rca.lt(sa + 1).test();
    (-rca).lt(sa).test();
    (-rcb).lt(sb).test();
    rca.lt(-sb).test();
    (-rca).lt(-sb).test();

    rca.gt(sb).test();
    rca.gt(sa).test();
    rca.gt(sa + 1).test();
    (-rca).gt(sa).test();
    (-rcb).gt(sb).test();
    rca.gt(-sb).test();
    (-rca).gt(-sb).test();

    rca.le(sb).test();
    rca.le(sa).test();
    rca.le(sa + 1).test();
    (-rca).le(sa).test();
    (-rcb).le(sb).test();
    rca.le(-sb).test();
    (-rca).le(-sb).test();

    rca.ge(sb).test();
    rca.ge(sa).test();
    rca.ge(sa + 1).test();
    (-rca).ge(sa).test();
    (-rcb).ge(sb).test();
    rca.ge(-sb).test();
    (-rca).ge(-sb).test();

    rca.eq(sb).test();
    rca.eq(sa).test();
    rca.eq(sa + 1).test();
    (-rca).eq(sa).test();
    (-rcb).eq(sb).test();
    rca.eq(-sb).test();
    (-rca).eq(-sb).test();

    rca.ne(sb).test();
    rca.ne(sa).test();
    rca.ne(sa + 1).test();
    (-rca).ne(sa).test();
    (-rcb).ne(sb).test();
    rca.ne(-sb).test();
    (-rca).ne(-sb).test();

    // # Bitwise Tests (only between sb)
    let sba = SecretBit::from(false);
    let sbb = SecretBit::from(true);

    // #test NEGB
    (!sba).test(); //  1
    (!sbb).test(); //  0

    // # test ANDSB
    (sba & sba).test();
    (sba & sbb).test();
    (sbb & sba).test();
    (sbb & sbb).test();

    // # test ORSB
    (sba | sba).test();
    (sba | sbb).test();
    (sbb | sba).test();
    (sbb | sbb).test();

    // # test XORSB
    (sba ^ sba).test();
    (sba ^ sbb).test();
    (sbb ^ sba).test();
    (sbb ^ sbb).test();

    // # test SAND
    (sa * sba).test();
    (sa * sbb).test();
    (sb * sba).test();
    (sb * sbb).test();

    (sba * sa).test();
    (sba * sb).test();
    (sbb * sa).test();
    (sbb * sb).test();

    // #test NEG
    (!sa).test();
    (!rca).test();
    (!sb).test();
    (!rcb).test();

    // # test sregint
    let la: i64 = 32;
    let lb: i64 = 4;
    let sla = SecretI64::from(la);
    let slb = SecretI64::from(lb);

    // # test ANDSR
    (sla & sla).test();
    (sla & slb).test();
    (slb & sla).test();
    (slb & slb).test();

    // # test ORSR
    (sla | sla).test();
    (sla | slb).test();
    (slb | sla).test();
    (slb | slb).test();

    // # test XORSR
    (sla ^ sla).test();
    (sla ^ slb).test();
    (slb ^ sla).test();
    (slb ^ slb).test();

    // # test ANDSRR
    (sla & la).test();
    (sla & lb).test();
    (slb & la).test();
    (slb & lb).test();

    // # test ORSRR
    (sla | la).test();
    (sla | lb).test();
    (slb | la).test();
    (slb | lb).test();

    // # test XORSRR
    (sla ^ la).test();
    (sla ^ lb).test();
    (slb ^ la).test();
    (slb ^ lb).test();

    // # test ANDRSR
    (la & sla).test();
    (la & slb).test();
    (lb & sla).test();
    (lb & slb).test();

    // # test ORRSR
    (la | sla).test();
    (la | slb).test();
    (lb | sla).test();
    (lb | slb).test();

    // # test XORRSR
    (la ^ sla).test();
    (la ^ slb).test();
    (lb ^ sla).test();
    (lb ^ slb).test();

    // # test ANDRR
    (la & la).test();
    (la & lb).test();
    (lb & la).test();
    (lb & lb).test();

    // # test ORRR
    (la | la).test();
    (la | lb).test();
    (lb | la).test();
    (lb | lb).test();

    // # test XORRR
    (la ^ la).test();
    (la ^ lb).test();
    (lb ^ la).test();
    (lb ^ lb).test();

    // # test ANDRR
    (rca & rca).test();
    (rca & rcb).test();
    (rcb & rca).test();
    (rcb & rcb).test();

    // # test ORRR
    (rca | rca).test();
    (rca | rcb).test();
    (rcb | rca).test();
    (rcb | rcb).test();

    // # test XORRR
    (rca ^ rca).test();
    (rca ^ rcb).test();
    (rcb ^ rca).test();
    (rcb ^ rcb).test();

    // Test bit-twiddling
    let bit = SecretBit::from(true);
    let sa = SecretI64::from(ConstI32::<0>);
    let sb = sa.set_bit(bit, ConstU32::<10>);
    let sc = sb.get_bit(ConstU32::<10>);
    sb.test();
    sc.test();

    // # test mult 128 bits
    let s64a = SecretI64::from(0x4000000000000000_i64 + 28731371);
    let s64b = SecretI64::from(0x4000000000000000_i64 + 985724131);
    let (high, low) = SecretI64::mult2(s64a, s64b);
    high.test();
    low.test();

    // #test CONVSUREGSINT
    let aa = i64::max_value(); // 2^{64-1)-1
    let mut a1 = SecretI64::from(aa);
    let mut a2 = SecretModp::from(aa);
    // Make a1 and a2 big
    a1 = a1 + a1;
    a2 = a2 + a2;
    let te = SecretModp::from_unsigned(a1);
    te.test_value(a2);
}
