/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2019, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _aANDFactory
#define _aANDFactory

#include "OT/aAND.h"

// We have one of these factories per online thread
class aAND_Factory
{
  int num_online;

  void make_more(Player &P);

  aAND aAF;

public:
  aAND_Factory(int num_onl) { num_online= num_onl; }

  /* The aAND queue */
  list<aTriple> aANDs;

  // Get a single aAND
  aTriple get_aAND(Player &P);

  // Gets many aANDs at once
  list<aTriple> get_aANDs(unsigned int num, Player &P);

  int get_thread_number() const { return num_online; }
};

#endif
