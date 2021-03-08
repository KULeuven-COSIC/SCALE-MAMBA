/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

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

#include "LSSS/Open_Protocol2.h"
#include "LaAND.h"
#include "config.h"
#include <list>

class aAND
{
  // This is here to avoid making/deleting memory as we go

  vector<LaAND> LA;
  vector<aTriple> triples;

public:
  aAND(Player &P)
  {
    unsigned int number= HaAND::get_number();
    int lg2number= CEIL_LOG2(number);
    unsigned int B= DIV_CEIL(OT_stat_sec, lg2number);
    LA.resize(B);
    for (unsigned int i= 0; i < B; i++)
      {
        LA[i].Init(P, 2);
      }
    triples.resize(number);
  }

  /* This is Figure 18 of ePrint 2017/189 with l=number */
  void make_aANDs(Player &P, int num_online);

  // Copy existing array into back of the list aBl
  void copy_aANDs(list<aTriple> &aAl)
  {
    copy(triples.begin(), triples.end(), back_inserter(aAl));
  }

  unsigned int get_number() const
  {
    return triples.size();
  }
};

/* Multiply a vector of aBits, z=x*y 
 * Given a vector of aAND triples
 */
void Mult_aBits(vector<aBit> &z, const vector<aBit> &x, const vector<aBit> &y,
                list<aTriple> &T, Player &P);

/* Multiply two aBits, given a triple */
void Mult_Bit(aBit &z, const aBit &x, const aBit &y,
              const aTriple &T, Player &P);

#endif
