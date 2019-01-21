/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _aAND
#define _aAND

/* This boosts a leaky AND into a real checked aAND
 * We use bucketing where the bucket size is
 *     B =  ceil(OT_stat_sec/log_2 number)
 * where number is the size of the array in the LaAND
 * datastructure
 *
 * We "expect" with normal operation B=3
 *
 * The protocol to be run is Figure 18 of 2017/189
 */

#include "LaAND.h"
#include "aBitFactory.h"

class aAND
{
  // This is here to avoid making/deleting memory as we go

  vector<LaAND> LA;
  unsigned int used;
  vector<aBit> x, y, z;

public:
  aAND()
  {
    LA.resize(1);
    x.resize(0);
    used= 0;
  }

  /* This is Figure 18 of ePrint 2017/189 with l=number */
  void make_more(aBitFactory &aBF, Player &P);

  unsigned int get_number() const { return x.size(); }

  /* This is the function you use to get new aANDs
   *   It manages all the control stuff for you
   */
  void get_aAND(aBit &x, aBit &y, aBit &z, aBitFactory &aBF, Player &P);
};

/* Multiply a vector of aBits, z=x*y */
void Mult_aBits(vector<aBit> &z, const vector<aBit> &x, const vector<aBit> &y,
                aAND &aA, aBitFactory &aBF, Player &P);

#endif
