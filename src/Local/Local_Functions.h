/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _Local_Function
#define _Local_Function

#include <map>
using namespace std;

#include "Exceptions/Exceptions.h"

template<class SRegint, class SBit>
class Processor;

template<class SRegint, class SBit>
class Local_Functions
{
  map<int, void (*)(Processor<SRegint, SBit> &Proc)> functions;

public:
  Local_Functions();

  void apply_Function(int instr, Processor<SRegint, SBit> &Proc)
  {
    if (functions.count(instr) == 0)
      {
        throw invalid_local_function();
      }
    functions[instr](Proc);
  }
};

#endif
