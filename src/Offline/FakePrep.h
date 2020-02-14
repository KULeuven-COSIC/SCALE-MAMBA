/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _FAKEPREP
#define _FAKEPREP

#include "LSSS/PRSS.h"
#include "LSSS/PRZS.h"
#include "System/Player.h"
#include <list>

class FakePrep
{
  //stored triples
  vector<list<Share>> triples;

  // stored squares
  vector<list<Share>> squares;

  // stored bits
  list<Share> bits;

  bool rewind_triples, rewind_squares, rewind_bits;

  Player &P;

public:
  FakePrep(Player &P);
  void produce_triples(list<Share> &a, list<Share> &b, list<Share> &c);
  void produce_squares(list<Share> &a, list<Share> &b, unsigned int rep= 1);
  void produce_bits(list<Share> &b);
};

#endif
