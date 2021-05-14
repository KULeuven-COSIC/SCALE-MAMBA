// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![no_std]
#![no_main]
#![feature(const_evaluatable_checked)]

#[scale::main(KAPPA = 40)]
#[inline(always)]
fn main() {
    let a = 5424;
    const A: i32 = 5424;
    let const_a = ConstI32::<A>;
    let b = 3543;
    const B: i32 = 3543;
    let const_b = ConstI32::<B>;

    let ca = ClearModp::from(a);
    let cb = ClearModp::from(b);
    let sa = SecretModp::from(a);
    let sb = SecretModp::from(b);

    ClearModp::from(0).test();
    ClearModp::from(1).test();
    ca.test();
    sb.test();

    ClearModp::from(ca).test();
    SecretModp::from(sa).test();

    (ca + cb).test();
    (sa + cb).test();
    (ca + sb).test();
    (sa + sb).test();

    (ca + const_b).test();
    (const_a + cb).test();
    (sa + const_b).test();
    (const_a + sb).test();

    (ca - cb).test();
    (sa - cb).test();
    (ca - sb).test();
    (sa - sb).test();

    (ca - const_b).test();
    (const_a - cb).test();
    (sa - const_b).test();
    (const_a - sb).test();

    (ca * cb).test();
    (sa * cb).test();
    (ca * sb).test();
    (sa * sb).test();

    (ca * const_b).test();
    (const_a * cb).test();
    (sa * const_b).test();
    (const_a * sb).test();

    (ConstI32::<42> - ca).test();
    (ConstI32::<42> + ca).test();
    (ConstI32::<42> - sa).test();
    (ConstI32::<42> + sa).test();
    (ca - ConstI32::<42>).test();
    (ca + ConstI32::<42>).test();
    (sb - ConstI32::<42>).test();
    (sb + ConstI32::<42>).test();

    let divisible = a - a % b;
    let const_divisible = ConstI32::<{ A - A % B }>;
    let cdivisible = ClearModp::from(divisible);
    let sdivisible = SecretModp::from(divisible);
    (cdivisible / cb).test();
    (cdivisible / const_b).test();
    (const_divisible / cb).test();
    (sdivisible / cb).test();
    (sdivisible / const_b).test();

    (ca / ConstI32::<1>).test();

    (ca % cb).test();
    (ca % const_b).test();
    (const_a % cb).test();

    // Logical operators test
    (ca & ca).test();
    (ca & cb).test();
    (cb & ca).test();
    (cb & cb).test();

    (ca ^ ca).test();
    (ca ^ cb).test();
    (cb ^ ca).test();
    (cb ^ cb).test();

    (ca | ca).test();
    (ca | cb).test();
    (cb | ca).test();
    (cb | cb).test();

    (ca & const_a).test();
    (ca & const_b).test();
    (const_b & ca).test();
    (const_b & cb).test();

    (ca ^ const_a).test();
    (ca ^ const_b).test();
    (const_b ^ ca).test();
    (const_b ^ cb).test();

    (ca | const_a).test();
    (ca | const_b).test();
    (const_b | ca).test();
    (const_b | cb).test();

    (0_i64..2)
        .contains(&SecretModp::get_random_bit().reveal().into())
        .test();

    let (t1, t2, t3) = SecretModp::get_random_triple();
    (t1.reveal() * t2.reveal() - t3.reveal()).test_value(ClearModp::from(0));
    let (s1, s2) = SecretModp::get_random_square();
    (s1.reveal() * s1.reveal() - s2.reveal()).test_value(ClearModp::from(0));

    let i = ConstI32::<3>;
    let ci = ClearModp::from(i);
    let cz = ClearModp::from(ConstI32::<0>);

    // Shifting
    (ca << ci).test();
    (ca << i).test();
    (ca >> ci).test();
    (ca >> i).test();
    (cz << i).test();
    (cz << ci).test();
    (cz >> i).test();
    (cz >> ci).test();
    (ca >> ConstI32::<0>).test();
    (ca << ConstI32::<0>).test();
    (ca >> cz).test();
    (ca << cz).test();

    // Now test Legendre
    ClearModp::from(0).legendre().test();
    ClearModp::from(1).legendre().test();
    ClearModp::from(2).legendre().test();
    ClearModp::from(3).legendre().test();
    ClearModp::from(4).legendre().test();
    ClearModp::from(5).legendre().test();
    ClearModp::from(6).legendre().test();
    ClearModp::from(7).legendre().test();
    ClearModp::from(8).legendre().test();
    ClearModp::from(15).legendre().test();
    ClearModp::from(24).legendre().test();
    ClearModp::from(100).legendre().test();
}
