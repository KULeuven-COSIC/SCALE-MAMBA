/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _HaAND
#define _HaAND

/* This produces the output from Figure 16 of
 * ePrint 2017/189
 *
 * We do OT_Amort values at a time
 *   This is basically the number of executions of the MMO
 *   function we do in parallel. So if you change this you
 *   need to change the MMO template instantiation.
 *
 *   Assumption that 32768 is divisible by OT_Amort
 */

#define OT_Amort 128

#include "aBit.h"

class HaAND
{

public:
  vector<aBit> x, y;
  vector<vector<gf2n>> HKi, HKiD, HMi;
  vector<int> v;

  /* This is protocol 16 of ePrint 2017/189 executed 131072 times */
  void make_more(Player &P, int num_online);

  unsigned int get_number() const
  {
    return x.size();
  }
};

#endif
