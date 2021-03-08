// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![no_std]
#![no_main]
#![feature(const_evaluatable_checked)]

use scale_std::local_functions::*;
use scale_std::matrix::Matrix;

#[scale::main(KAPPA = 40)]
#[inline(always)]
fn main() {
    const N: u64 = 2;
    const L: u64 = 3;
    const M: u64 = 2;

    /* Mult the two matrices
         A = [1,2,3;4,5,6]
         B = [7,8;9,10;11,12]
       which should give us
         C = [58,64; 139, 154]
    */

    let mut c_a = Matrix::<ClearModp, N, L>::uninitialized();
    let mut s_a = Matrix::<SecretModp, N, L>::uninitialized();
    let mut cnt = ClearModp::from(ConstI32::<1>);
    for i in 0..N {
        for j in 0..L {
            c_a.set(i, j, &cnt);
            s_a.set(i, j, &cnt.into());
            cnt = cnt + ConstI32::<1>;
        }
    }

    let mut c_b = Matrix::<ClearModp, L, M>::uninitialized();
    let mut s_b = Matrix::<SecretModp, L, M>::uninitialized();
    for i in 0..L {
        for j in 0..M {
            c_b.set(i, j, &cnt);
            s_b.set(i, j, &cnt.into());
            cnt = cnt + ConstI32::<1>;
        }
    }

    print!("Doing C*C\n");
    let cc_ab = Matrix_Mul_CC(&c_a, &c_b);
    print!("Done C*C\n");

    print!("Doing S*C\n");
    let sc_ab = Matrix_Mul_SC(&s_a, &c_b);
    print!("Done S*C\n");

    print!("Doing C*S\n");
    let cs_ab = Matrix_Mul_CS(&c_a, &s_b);
    print!("Done C*S\n");

    print!("Doing Gauss Elim on C_A\n");
    let ge = Gauss_Elim(&c_a);
    print!("Done Gauss Elim\n");

    cc_ab.get(0, 0).test_value(ClearModp::from(58));
    cc_ab.get(0, 1).test_value(ClearModp::from(64));
    cc_ab.get(1, 0).test_value(ClearModp::from(139));
    cc_ab.get(1, 1).test_value(ClearModp::from(154));

    sc_ab.get(0, 0).test_value(SecretModp::from(58));
    sc_ab.get(0, 1).test_value(SecretModp::from(64));
    sc_ab.get(1, 0).test_value(SecretModp::from(139));
    sc_ab.get(1, 1).test_value(SecretModp::from(154));

    cs_ab.get(0, 0).test_value(SecretModp::from(58));
    cs_ab.get(0, 1).test_value(SecretModp::from(64));
    cs_ab.get(1, 0).test_value(SecretModp::from(139));
    cs_ab.get(1, 1).test_value(SecretModp::from(154));

    print!("Before GE\n");
    for i in 0..2 {
        let rowi = c_a.get_row(i);
        for j in 0..3 {
            print!(rowi.get(j as u64).unwrap(), " ");
        }
        print!("\n")
    }

    print!("After GE\n");
    for i in 0..2 {
        let rowi = ge.get_row(i);
        for j in 0..3 {
            print!(rowi.get(j as u64).unwrap(), " ");
        }
        print!("\n")
    }
    ge.get(0, 0).test_value(ClearModp::from(1));
    ge.get(0, 1).test_value(ClearModp::from(0));
    ge.get(0, 2).test_value(ClearModp::from(-1));
    ge.get(1, 0).test_value(ClearModp::from(0));
    ge.get(1, 1).test_value(ClearModp::from(1));
    ge.get(1, 2).test_value(ClearModp::from(2));
}
