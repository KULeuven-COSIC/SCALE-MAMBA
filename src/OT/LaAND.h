/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _LaAND
#define _LaAND

/* Executed the leaky-AND protocol of Figure 8
 * of ePrint 2017/189
 *
 * We do 32768 values at a time
 *   - This is basically inherited from HaAND
 *
 */

#include "HaAND.h"

class aTriple
{
public:
  aBit x, y, z;
};

class LaAND
{
public:
  vector<aTriple> triples;

  /* This is Figure 8 of ePrint 2017/189 executed number times */
  void make_more(Player &P, int num_online);

  unsigned int get_number() const
  {
    return triples.size();
  }
};

/* This is a routine for DEBUG purposes only */
void check_triples(Player &P, const vector<aTriple> &ee);

#endif
