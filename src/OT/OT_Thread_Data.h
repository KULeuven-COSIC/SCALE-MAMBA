/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _OT_Thread_Data
#define _OT_Thread_Data

#include "OT/aAND_Thread.h"
#include "OT/aBit_Thread.h"

class OT_Thread_Data
{
public:
  aBit_Data aBD;
  aAND_Data aAD;
  bool ready;

  void init(unsigned int no_online_threads)
  {
    aBD.aBits.resize(no_online_threads + 1);
    aAD.aANDs.resize(no_online_threads);
    ready= false;
  }
};

#endif
