/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _LaAND
#define _LaAND

/* Executed the leaky-AND protocol of Figure 8
 * of ePrint 2017/189
 *
 * We do number values at a time, where number is
 * the default_l values from aBF
 *   - This is basically inherited from HaAND
 * Which means we call the generation in aBF
 * three times on each invocation of make_more
 *
 */

#include "HaAND.h"
#include "aBitFactory.h"

class LaAND
{
public:
  vector<aBit> x, y, z;

  /* This is Figure 8 of ePrint 2017/189 executed number times */
  void make_more(aBitFactory &aBF, Player &P);

  unsigned int get_number() const { return x.size(); }
};

#endif
