/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _Local_Function
#define _Local_Function

#include "Processor/Processor.h"
#include <map>

class Local_Functions
{
  map<int, void (*)(Processor &Proc)> functions;

public:
  Local_Functions();

  void apply_Function(int instr, Processor &Proc)
  {
    functions[instr](Proc);
  }
};

#endif
