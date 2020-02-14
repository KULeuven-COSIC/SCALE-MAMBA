/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _offline_subroutines
#define _offline_subroutines

#include "LSSS/Share.h"
#include "System/Player.h"

/* Make a share vector which shares a value val
 *   Given (global) mac values
 */
void make_shares(vector<Share> &share, const gfp &val, const vector<gfp> &macs,
                 PRNG &G);

/* Same but does no MACs at all */
void make_shares(vector<Share> &share, const gfp &val, PRNG &G);

/* This produces the sum of Schur products needed by
 * each party in Maurer and Reduced protocols
 */
gfp schur_sum_prod(const Share &aa, const Share &bb, const Player &P);

/* Fake sub-routines */
void init_fake();
void make_fake_macs(vector<gfp> &macs, const gfp &val);

#endif
