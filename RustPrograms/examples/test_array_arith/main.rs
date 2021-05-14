// Copyright (c) N21, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) N21, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

#![no_std]
#![no_main]
#![feature(const_evaluatable_checked)]

use scale_std::array::Array;
use scale_std::slice::Slice;

#[scale::main(KAPPA = 40)]
#[inline(always)]
fn main() {
    const N: u64 = 200;

    let mut ca_arr: Array<ClearModp, N> = Array::uninitialized();
    let mut cb_arr: Array<ClearModp, N> = Array::uninitialized();
    let mut sa_arr: Array<SecretModp, N> = Array::uninitialized();
    let mut sb_arr: Array<SecretModp, N> = Array::uninitialized();

    let mut ca_sli: Slice<ClearModp> = Slice::uninitialized(N);
    let mut cb_sli: Slice<ClearModp> = Slice::uninitialized(N);
    let mut sa_sli: Slice<SecretModp> = Slice::uninitialized(N);
    let mut sb_sli: Slice<SecretModp> = Slice::uninitialized(N);
    for i in 0..N {
        ca_arr.set(i, &ClearModp::from(3 * i as i64));
        sa_arr.set(i, &SecretModp::from(3 * i as i64));
        ca_sli.set(i, &ClearModp::from(3 * i as i64));
        sa_sli.set(i, &SecretModp::from(3 * i as i64));
        cb_arr.set(i, &ClearModp::from(i as i64 + 10));
        sb_arr.set(i, &SecretModp::from(i as i64 + 10));
        cb_sli.set(i, &ClearModp::from(i as i64 + 10));
        sb_sli.set(i, &SecretModp::from(i as i64 + 10));
    }

    let mut a_add_cc: Array<ClearModp, N> = Array::uninitialized();
    let mut a_add_sc: Array<SecretModp, N> = Array::uninitialized();
    let mut a_add_cs: Array<SecretModp, N> = Array::uninitialized();
    let mut a_add_ss: Array<SecretModp, N> = Array::uninitialized();
    let mut a_sub_cc: Array<ClearModp, N> = Array::uninitialized();
    let mut a_sub_sc: Array<SecretModp, N> = Array::uninitialized();
    let mut a_sub_cs: Array<SecretModp, N> = Array::uninitialized();
    let mut a_sub_ss: Array<SecretModp, N> = Array::uninitialized();
    let mut a_mul_cc: Array<ClearModp, N> = Array::uninitialized();
    let mut a_mul_sc: Array<SecretModp, N> = Array::uninitialized();
    let mut a_mul_cs: Array<SecretModp, N> = Array::uninitialized();
    let mut a_div_cc: Array<ClearModp, N> = Array::uninitialized();
    let mut a_mod_cc: Array<ClearModp, N> = Array::uninitialized();

    let mut s_add_cc: Slice<ClearModp> = Slice::uninitialized(N);
    let mut s_add_sc: Slice<SecretModp> = Slice::uninitialized(N);
    let mut s_add_cs: Slice<SecretModp> = Slice::uninitialized(N);
    let mut s_add_ss: Slice<SecretModp> = Slice::uninitialized(N);
    let mut s_sub_cc: Slice<ClearModp> = Slice::uninitialized(N);
    let mut s_sub_sc: Slice<SecretModp> = Slice::uninitialized(N);
    let mut s_sub_cs: Slice<SecretModp> = Slice::uninitialized(N);
    let mut s_sub_ss: Slice<SecretModp> = Slice::uninitialized(N);
    let mut s_mul_cc: Slice<ClearModp> = Slice::uninitialized(N);
    let mut s_mul_sc: Slice<SecretModp> = Slice::uninitialized(N);
    let mut s_mul_cs: Slice<SecretModp> = Slice::uninitialized(N);
    let mut s_div_cc: Slice<ClearModp> = Slice::uninitialized(N);
    let mut s_mod_cc: Slice<ClearModp> = Slice::uninitialized(N);

    // Time each one using a loop for each one in a naive manner
    start_clock(ConstI32::<1>);
    for i in 0..N {
        a_add_cc.set(i, &(*ca_arr.get_unchecked(i) + *cb_arr.get_unchecked(i)));
    }
    for i in 0..N {
        a_add_sc.set(i, &(*sa_arr.get_unchecked(i) + *cb_arr.get_unchecked(i)));
    }
    for i in 0..N {
        a_add_cs.set(i, &(*ca_arr.get_unchecked(i) + *sb_arr.get_unchecked(i)));
    }
    for i in 0..N {
        a_add_ss.set(i, &(*sa_arr.get_unchecked(i) + *sb_arr.get_unchecked(i)));
    }
    for i in 0..N {
        a_sub_cc.set(i, &(*ca_arr.get_unchecked(i) - *cb_arr.get_unchecked(i)));
    }
    for i in 0..N {
        a_sub_sc.set(i, &(*sa_arr.get_unchecked(i) - *cb_arr.get_unchecked(i)));
    }
    for i in 0..N {
        a_sub_cs.set(i, &(*ca_arr.get_unchecked(i) - *sb_arr.get_unchecked(i)));
    }
    for i in 0..N {
        a_sub_ss.set(i, &(*sa_arr.get_unchecked(i) - *sb_arr.get_unchecked(i)));
    }
    for i in 0..N {
        a_mul_cc.set(i, &(*ca_arr.get_unchecked(i) * *cb_arr.get_unchecked(i)));
    }
    for i in 0..N {
        a_mul_sc.set(i, &(*sa_arr.get_unchecked(i) * *cb_arr.get_unchecked(i)));
    }
    for i in 0..N {
        a_mul_cs.set(i, &(*ca_arr.get_unchecked(i) * *sb_arr.get_unchecked(i)));
    }
    for i in 0..N {
        a_div_cc.set(i, &(*ca_arr.get_unchecked(i) / *cb_arr.get_unchecked(i)));
    }
    for i in 0..N {
        a_mod_cc.set(i, &(*ca_arr.get_unchecked(i) % *cb_arr.get_unchecked(i)));
    }

    for i in 0..N {
        s_add_cc.set(i, &(*ca_sli.get_unchecked(i) + *cb_sli.get_unchecked(i)));
    }
    for i in 0..N {
        s_add_sc.set(i, &(*sa_sli.get_unchecked(i) + *cb_sli.get_unchecked(i)));
    }
    for i in 0..N {
        s_add_cs.set(i, &(*ca_sli.get_unchecked(i) + *sb_sli.get_unchecked(i)));
    }
    for i in 0..N {
        s_add_ss.set(i, &(*sa_sli.get_unchecked(i) + *sb_sli.get_unchecked(i)));
    }
    for i in 0..N {
        s_sub_cc.set(i, &(*ca_sli.get_unchecked(i) - *cb_sli.get_unchecked(i)));
    }
    for i in 0..N {
        s_sub_sc.set(i, &(*sa_sli.get_unchecked(i) - *cb_sli.get_unchecked(i)));
    }
    for i in 0..N {
        s_sub_cs.set(i, &(*ca_sli.get_unchecked(i) - *sb_sli.get_unchecked(i)));
    }
    for i in 0..N {
        s_sub_ss.set(i, &(*sa_sli.get_unchecked(i) - *sb_sli.get_unchecked(i)));
    }
    for i in 0..N {
        s_mul_cc.set(i, &(*ca_sli.get_unchecked(i) * *cb_sli.get_unchecked(i)));
    }
    for i in 0..N {
        s_mul_sc.set(i, &(*sa_sli.get_unchecked(i) * *cb_sli.get_unchecked(i)));
    }
    for i in 0..N {
        s_mul_cs.set(i, &(*ca_sli.get_unchecked(i) * *sb_sli.get_unchecked(i)));
    }
    for i in 0..N {
        s_div_cc.set(i, &(*ca_sli.get_unchecked(i) / *cb_sli.get_unchecked(i)));
    }
    for i in 0..N {
        s_mod_cc.set(i, &(*ca_sli.get_unchecked(i) % *cb_sli.get_unchecked(i)));
    }
    stop_clock(ConstI32::<1>);

    // Now time the same with the operator versions of the memory
    // based vector instructions

    // Version 1  (using clone)
    start_clock(ConstI32::<2>);
    let a_add_cc_t = ca_arr.clone() + cb_arr.clone();
    let a_add_cs_t = ca_arr.clone() + sb_arr.clone();
    let a_add_sc_t = sa_arr.clone() + cb_arr.clone();
    let a_add_ss_t = sa_arr.clone() + sb_arr.clone();
    let a_sub_cc_t = ca_arr.clone() - cb_arr.clone();
    let a_sub_cs_t = ca_arr.clone() - sb_arr.clone();
    let a_sub_sc_t = sa_arr.clone() - cb_arr.clone();
    let a_sub_ss_t = sa_arr.clone() - sb_arr.clone();
    let a_mul_cc_t = ca_arr.clone() * cb_arr.clone();
    let a_mul_cs_t = ca_arr.clone() * sb_arr.clone();
    let a_mul_sc_t = sa_arr.clone() * cb_arr.clone();
    let a_div_cc_t = ca_arr.clone() / cb_arr.clone();
    let a_mod_cc_t = ca_arr.clone() % cb_arr.clone();

    let s_add_cc_t = ca_sli.clone() + cb_sli.clone();
    let s_add_cs_t = ca_sli.clone() + sb_sli.clone();
    let s_add_sc_t = sa_sli.clone() + cb_sli.clone();
    let s_add_ss_t = sa_sli.clone() + sb_sli.clone();
    let s_sub_cc_t = ca_sli.clone() - cb_sli.clone();
    let s_sub_cs_t = ca_sli.clone() - sb_sli.clone();
    let s_sub_sc_t = sa_sli.clone() - cb_sli.clone();
    let s_sub_ss_t = sa_sli.clone() - sb_sli.clone();
    let s_mul_cc_t = ca_sli.clone() * cb_sli.clone();
    let s_mul_cs_t = ca_sli.clone() * sb_sli.clone();
    let s_mul_sc_t = sa_sli.clone() * cb_sli.clone();
    let s_div_cc_t = ca_sli.clone() / cb_sli.clone();
    let s_mod_cc_t = ca_sli.clone() % cb_sli.clone();
    stop_clock(ConstI32::<2>);

    /* Now do tests */
    let mut flag = ClearModp::from(1);
    for i in 0..N {
        let t1 = *a_add_cc_t.get_unchecked(i) - *a_add_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *a_add_sc_t.get_unchecked(i) - *a_add_sc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_add_cs_t.get_unchecked(i) - *a_add_cs.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_add_ss_t.get_unchecked(i) - *a_add_ss.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_sub_cc_t.get_unchecked(i) - *a_sub_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *a_sub_sc_t.get_unchecked(i) - *a_sub_sc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_sub_cs_t.get_unchecked(i) - *a_sub_cs.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_sub_ss_t.get_unchecked(i) - *a_sub_ss.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_mul_cc_t.get_unchecked(i) - *a_mul_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *a_mul_sc_t.get_unchecked(i) - *a_mul_sc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_mul_cs_t.get_unchecked(i) - *a_mul_cs.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_div_cc_t.get_unchecked(i) - *a_div_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *a_mod_cc_t.get_unchecked(i) - *a_mod_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *s_add_cc_t.get_unchecked(i) - *s_add_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *s_add_sc_t.get_unchecked(i) - *s_add_sc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_add_cs_t.get_unchecked(i) - *s_add_cs.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_add_ss_t.get_unchecked(i) - *s_add_ss.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_sub_cc_t.get_unchecked(i) - *s_sub_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *s_sub_sc_t.get_unchecked(i) - *s_sub_sc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_sub_cs_t.get_unchecked(i) - *s_sub_cs.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_sub_ss_t.get_unchecked(i) - *s_sub_ss.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_mul_cc_t.get_unchecked(i) - *s_mul_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *s_mul_sc_t.get_unchecked(i) - *s_mul_sc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_mul_cs_t.get_unchecked(i) - *s_mul_cs.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_div_cc_t.get_unchecked(i) - *s_div_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *s_mod_cc_t.get_unchecked(i) - *s_mod_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);
    }
    flag.test_value(ClearModp::from(1));

    // Version 2 (one clone on left)
    start_clock(ConstI32::<3>);
    let a_add_cc_t = ca_arr.clone() + &cb_arr;
    let a_add_cs_t = ca_arr.clone() + &sb_arr;
    let a_add_sc_t = sa_arr.clone() + &cb_arr;
    let a_add_ss_t = sa_arr.clone() + &sb_arr;
    let a_sub_cc_t = ca_arr.clone() - &cb_arr;
    let a_sub_cs_t = ca_arr.clone() - &sb_arr;
    let a_sub_sc_t = sa_arr.clone() - &cb_arr;
    let a_sub_ss_t = sa_arr.clone() - &sb_arr;
    let a_mul_cc_t = ca_arr.clone() * &cb_arr;
    let a_mul_cs_t = ca_arr.clone() * &sb_arr;
    let a_mul_sc_t = sa_arr.clone() * &cb_arr;
    let a_div_cc_t = ca_arr.clone() / &cb_arr;
    let a_mod_cc_t = ca_arr.clone() % &cb_arr;

    let s_add_cc_t = ca_sli.clone() + &cb_sli;
    let s_add_cs_t = ca_sli.clone() + &sb_sli;
    let s_add_sc_t = sa_sli.clone() + &cb_sli;
    let s_add_ss_t = sa_sli.clone() + &sb_sli;
    let s_sub_cc_t = ca_sli.clone() - &cb_sli;
    let s_sub_cs_t = ca_sli.clone() - &sb_sli;
    let s_sub_sc_t = sa_sli.clone() - &cb_sli;
    let s_sub_ss_t = sa_sli.clone() - &sb_sli;
    let s_mul_cc_t = ca_sli.clone() * &cb_sli;
    let s_mul_cs_t = ca_sli.clone() * &sb_sli;
    let s_mul_sc_t = sa_sli.clone() * &cb_sli;
    let s_div_cc_t = ca_sli.clone() / &cb_sli;
    let s_mod_cc_t = ca_sli.clone() % &cb_sli;
    stop_clock(ConstI32::<3>);

    /* Now do tests */
    let mut flag = ClearModp::from(1);
    for i in 0..N {
        let t1 = *a_add_cc_t.get_unchecked(i) - *a_add_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *a_add_sc_t.get_unchecked(i) - *a_add_sc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_add_cs_t.get_unchecked(i) - *a_add_cs.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_add_ss_t.get_unchecked(i) - *a_add_ss.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_sub_cc_t.get_unchecked(i) - *a_sub_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *a_sub_sc_t.get_unchecked(i) - *a_sub_sc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_sub_cs_t.get_unchecked(i) - *a_sub_cs.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_sub_ss_t.get_unchecked(i) - *a_sub_ss.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_mul_cc_t.get_unchecked(i) - *a_mul_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *a_mul_sc_t.get_unchecked(i) - *a_mul_sc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_mul_cs_t.get_unchecked(i) - *a_mul_cs.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_div_cc_t.get_unchecked(i) - *a_div_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *a_mod_cc_t.get_unchecked(i) - *a_mod_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *s_add_cc_t.get_unchecked(i) - *s_add_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *s_add_sc_t.get_unchecked(i) - *s_add_sc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_add_cs_t.get_unchecked(i) - *s_add_cs.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_add_ss_t.get_unchecked(i) - *s_add_ss.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_sub_cc_t.get_unchecked(i) - *s_sub_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *s_sub_sc_t.get_unchecked(i) - *s_sub_sc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_sub_cs_t.get_unchecked(i) - *s_sub_cs.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_sub_ss_t.get_unchecked(i) - *s_sub_ss.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_mul_cc_t.get_unchecked(i) - *s_mul_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *s_mul_sc_t.get_unchecked(i) - *s_mul_sc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_mul_cs_t.get_unchecked(i) - *s_mul_cs.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_div_cc_t.get_unchecked(i) - *s_div_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *s_mod_cc_t.get_unchecked(i) - *s_mod_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);
    }
    flag.test_value(ClearModp::from(1));

    // Version 3 (one clone on right)
    start_clock(ConstI32::<4>);
    let a_add_cc_t = &ca_arr + cb_arr.clone();
    let a_add_cs_t = &ca_arr + sb_arr.clone();
    let a_add_sc_t = &sa_arr + cb_arr.clone();
    let a_add_ss_t = &sa_arr + sb_arr.clone();
    let a_sub_cc_t = &ca_arr - cb_arr.clone();
    let a_sub_cs_t = &ca_arr - sb_arr.clone();
    let a_sub_sc_t = &sa_arr - cb_arr.clone();
    let a_sub_ss_t = &sa_arr - sb_arr.clone();
    let a_mul_cc_t = &ca_arr * cb_arr.clone();
    let a_mul_cs_t = &ca_arr * sb_arr.clone();
    let a_mul_sc_t = &sa_arr * cb_arr.clone();
    let a_div_cc_t = &ca_arr / cb_arr.clone();
    let a_mod_cc_t = &ca_arr % cb_arr.clone();

    let s_add_cc_t = &ca_sli + cb_sli.clone();
    let s_add_cs_t = &ca_sli + sb_sli.clone();
    let s_add_sc_t = &sa_sli + cb_sli.clone();
    let s_add_ss_t = &sa_sli + sb_sli.clone();
    let s_sub_cc_t = &ca_sli - cb_sli.clone();
    let s_sub_cs_t = &ca_sli - sb_sli.clone();
    let s_sub_sc_t = &sa_sli - cb_sli.clone();
    let s_sub_ss_t = &sa_sli - sb_sli.clone();
    let s_mul_cc_t = &ca_sli * cb_sli.clone();
    let s_mul_cs_t = &ca_sli * sb_sli.clone();
    let s_mul_sc_t = &sa_sli * cb_sli.clone();
    let s_div_cc_t = &ca_sli / cb_sli.clone();
    let s_mod_cc_t = &ca_sli % cb_sli.clone();
    stop_clock(ConstI32::<4>);

    /* Now do tests */
    let mut flag = ClearModp::from(1);
    for i in 0..N {
        let t1 = *a_add_cc_t.get_unchecked(i) - *a_add_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *a_add_sc_t.get_unchecked(i) - *a_add_sc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_add_cs_t.get_unchecked(i) - *a_add_cs.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_add_ss_t.get_unchecked(i) - *a_add_ss.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_sub_cc_t.get_unchecked(i) - *a_sub_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *a_sub_sc_t.get_unchecked(i) - *a_sub_sc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_sub_cs_t.get_unchecked(i) - *a_sub_cs.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_sub_ss_t.get_unchecked(i) - *a_sub_ss.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_mul_cc_t.get_unchecked(i) - *a_mul_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *a_mul_sc_t.get_unchecked(i) - *a_mul_sc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_mul_cs_t.get_unchecked(i) - *a_mul_cs.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_div_cc_t.get_unchecked(i) - *a_div_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *a_mod_cc_t.get_unchecked(i) - *a_mod_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *s_add_cc_t.get_unchecked(i) - *s_add_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *s_add_sc_t.get_unchecked(i) - *s_add_sc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_add_cs_t.get_unchecked(i) - *s_add_cs.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_add_ss_t.get_unchecked(i) - *s_add_ss.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_sub_cc_t.get_unchecked(i) - *s_sub_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *s_sub_sc_t.get_unchecked(i) - *s_sub_sc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_sub_cs_t.get_unchecked(i) - *s_sub_cs.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_sub_ss_t.get_unchecked(i) - *s_sub_ss.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_mul_cc_t.get_unchecked(i) - *s_mul_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *s_mul_sc_t.get_unchecked(i) - *s_mul_sc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_mul_cs_t.get_unchecked(i) - *s_mul_cs.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_div_cc_t.get_unchecked(i) - *s_div_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *s_mod_cc_t.get_unchecked(i) - *s_mod_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);
    }
    flag.test_value(ClearModp::from(1));

    // Version 4 (no clones)
    start_clock(ConstI32::<5>);
    let a_add_cc_t = &ca_arr + &cb_arr;
    let a_add_cs_t = &ca_arr + &sb_arr;
    let a_add_sc_t = &sa_arr + &cb_arr;
    let a_add_ss_t = &sa_arr + &sb_arr;
    let a_sub_cc_t = &ca_arr - &cb_arr;
    let a_sub_cs_t = &ca_arr - &sb_arr;
    let a_sub_sc_t = &sa_arr - &cb_arr;
    let a_sub_ss_t = &sa_arr - &sb_arr;
    let a_mul_cc_t = &ca_arr * &cb_arr;
    let a_mul_cs_t = &ca_arr * &sb_arr;
    let a_mul_sc_t = &sa_arr * &cb_arr;
    let a_div_cc_t = &ca_arr / &cb_arr;
    let a_mod_cc_t = &ca_arr % &cb_arr;

    let s_add_cc_t = &ca_sli + &cb_sli;
    let s_add_cs_t = &ca_sli + &sb_sli;
    let s_add_sc_t = &sa_sli + &cb_sli;
    let s_add_ss_t = &sa_sli + &sb_sli;
    let s_sub_cc_t = &ca_sli - &cb_sli;
    let s_sub_cs_t = &ca_sli - &sb_sli;
    let s_sub_sc_t = &sa_sli - &cb_sli;
    let s_sub_ss_t = &sa_sli - &sb_sli;
    let s_mul_cc_t = &ca_sli * &cb_sli;
    let s_mul_cs_t = &ca_sli * &sb_sli;
    let s_mul_sc_t = &sa_sli * &cb_sli;
    let s_div_cc_t = &ca_sli / &cb_sli;
    let s_mod_cc_t = &ca_sli % &cb_sli;
    stop_clock(ConstI32::<5>);

    /* Now do tests */
    let mut flag = ClearModp::from(1);
    for i in 0..N {
        let t1 = *a_add_cc_t.get_unchecked(i) - *a_add_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *a_add_sc_t.get_unchecked(i) - *a_add_sc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_add_cs_t.get_unchecked(i) - *a_add_cs.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_add_ss_t.get_unchecked(i) - *a_add_ss.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_sub_cc_t.get_unchecked(i) - *a_sub_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *a_sub_sc_t.get_unchecked(i) - *a_sub_sc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_sub_cs_t.get_unchecked(i) - *a_sub_cs.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_sub_ss_t.get_unchecked(i) - *a_sub_ss.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_mul_cc_t.get_unchecked(i) - *a_mul_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *a_mul_sc_t.get_unchecked(i) - *a_mul_sc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_mul_cs_t.get_unchecked(i) - *a_mul_cs.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_div_cc_t.get_unchecked(i) - *a_div_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *a_mod_cc_t.get_unchecked(i) - *a_mod_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *s_add_cc_t.get_unchecked(i) - *s_add_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *s_add_sc_t.get_unchecked(i) - *s_add_sc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_add_cs_t.get_unchecked(i) - *s_add_cs.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_add_ss_t.get_unchecked(i) - *s_add_ss.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_sub_cc_t.get_unchecked(i) - *s_sub_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *s_sub_sc_t.get_unchecked(i) - *s_sub_sc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_sub_cs_t.get_unchecked(i) - *s_sub_cs.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_sub_ss_t.get_unchecked(i) - *s_sub_ss.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_mul_cc_t.get_unchecked(i) - *s_mul_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *s_mul_sc_t.get_unchecked(i) - *s_mul_sc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_mul_cs_t.get_unchecked(i) - *s_mul_cs.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_div_cc_t.get_unchecked(i) - *s_div_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *s_mod_cc_t.get_unchecked(i) - *s_mod_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);
    }
    flag.test_value(ClearModp::from(1));

    // Now time the same with the member function based versions
    // using the memory based vector instructions
    start_clock(ConstI32::<6>);
    let a_add_cc_t = ca_arr.add_clear(&cb_arr);
    let a_add_cs_t = ca_arr.add_secret(&sb_arr);
    let a_add_sc_t = sa_arr.add_clear(&cb_arr);
    let a_add_ss_t = sa_arr.add_secret(&sb_arr);
    let a_sub_cc_t = ca_arr.sub_clear(&cb_arr);
    let a_sub_cs_t = ca_arr.sub_secret(&sb_arr);
    let a_sub_sc_t = sa_arr.sub_clear(&cb_arr);
    let a_sub_ss_t = sa_arr.sub_secret(&sb_arr);
    let a_mul_cc_t = ca_arr.mul_clear(&cb_arr);
    let a_mul_cs_t = ca_arr.mul_secret(&sb_arr);
    let a_mul_sc_t = sa_arr.mul_clear(&cb_arr);
    let a_div_cc_t = ca_arr.div_clear(&cb_arr);
    let a_mod_cc_t = ca_arr.mod_clear(&cb_arr);

    let s_add_cc_t = ca_sli.add_clear(&cb_sli);
    let s_add_cs_t = ca_sli.add_secret(&sb_sli);
    let s_add_sc_t = sa_sli.add_clear(&cb_sli);
    let s_add_ss_t = sa_sli.add_secret(&sb_sli);
    let s_sub_cc_t = ca_sli.sub_clear(&cb_sli);
    let s_sub_cs_t = ca_sli.sub_secret(&sb_sli);
    let s_sub_sc_t = sa_sli.sub_clear(&cb_sli);
    let s_sub_ss_t = sa_sli.sub_secret(&sb_sli);
    let s_mul_cc_t = ca_sli.mul_clear(&cb_sli);
    let s_mul_cs_t = ca_sli.mul_secret(&sb_sli);
    let s_mul_sc_t = sa_sli.mul_clear(&cb_sli);
    let s_div_cc_t = ca_sli.div_clear(&cb_sli);
    let s_mod_cc_t = ca_sli.mod_clear(&cb_sli);
    stop_clock(ConstI32::<6>);

    /* Now do tests */
    let mut flag = ClearModp::from(1);
    for i in 0..N {
        let t1 = *a_add_cc_t.get_unchecked(i) - *a_add_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *a_add_sc_t.get_unchecked(i) - *a_add_sc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_add_cs_t.get_unchecked(i) - *a_add_cs.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_add_ss_t.get_unchecked(i) - *a_add_ss.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_sub_cc_t.get_unchecked(i) - *a_sub_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *a_sub_sc_t.get_unchecked(i) - *a_sub_sc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_sub_cs_t.get_unchecked(i) - *a_sub_cs.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_sub_ss_t.get_unchecked(i) - *a_sub_ss.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_mul_cc_t.get_unchecked(i) - *a_mul_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *a_mul_sc_t.get_unchecked(i) - *a_mul_sc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_mul_cs_t.get_unchecked(i) - *a_mul_cs.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *a_div_cc_t.get_unchecked(i) - *a_div_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *a_mod_cc_t.get_unchecked(i) - *a_mod_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *s_add_cc_t.get_unchecked(i) - *s_add_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *s_add_sc_t.get_unchecked(i) - *s_add_sc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_add_cs_t.get_unchecked(i) - *s_add_cs.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_add_ss_t.get_unchecked(i) - *s_add_ss.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_sub_cc_t.get_unchecked(i) - *s_sub_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *s_sub_sc_t.get_unchecked(i) - *s_sub_sc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_sub_cs_t.get_unchecked(i) - *s_sub_cs.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_sub_ss_t.get_unchecked(i) - *s_sub_ss.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_mul_cc_t.get_unchecked(i) - *s_mul_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *s_mul_sc_t.get_unchecked(i) - *s_mul_sc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_mul_cs_t.get_unchecked(i) - *s_mul_cs.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1.reveal());

        let t1 = *s_div_cc_t.get_unchecked(i) - *s_div_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        let t1 = *s_mod_cc_t.get_unchecked(i) - *s_mod_cc.get_unchecked(i);
        flag = flag * (ClearModp::from(1) - t1);

        /*
        println!(i," ",ca_arr.get_unchecked(i)," ",cb_arr.get_unchecked(i));
        println!("\t",s_mod_cc_t.get_unchecked(i)," ",s_mod_cc.get_unchecked(i)," ",flag);
        */
    }
    flag.test_value(ClearModp::from(1));
}
