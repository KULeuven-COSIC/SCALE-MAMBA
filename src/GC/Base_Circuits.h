/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2019, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _BaseCircuits
#define _BaseCircuits

/* This class holds in memory the set of basic circuits
 * we are going to use. This means we only read them
 * from disk once.
 */

#include "GC/Circuit.h"
#include "Math/bigint.h"

enum Circ_Type {
  Adder64,
  Sub64,
  Mult2_64,
  Mult64,
  Divide64,
  Neg64,
  Zero_Equal,
  Compare_Eq,
  Compare,
  LSSS_to_GC,
  Number_Circuits
};

class Base_Circuits
{
public:
  vector<Circuit> BaseC;
  bool convert_ok; // Signals whether we can do GC <-> LSSS conversion or not

  void initialize(const bigint &p);
};

#endif
