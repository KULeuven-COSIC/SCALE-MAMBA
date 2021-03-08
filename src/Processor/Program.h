/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _Program
#define _Program

#include "Processor/Instruction.h"
template<class SRegint, class SBit>
class Processor;
template<class SRegint, class SBit>
class Machine;

class offline_control_data;

// Forward declaration as apparently this is needed for friends in templates
template<class SRegint, class SBit>
class Program;
template<class SRegint, class SBit>
ostream &operator<<(ostream &s, const Program<SRegint, SBit> &P);

/* A program is a vector of instructions */

template<class SRegint, class SBit>
class Program
{
  vector<Instruction<SRegint, SBit>> p;

  // Maximal register used
  int max_reg[MAX_REG_TYPE];

  void compute_constants();

public:
  Program()
  {
    compute_constants();
  }

  // Read in a program
  void parse(stringstream &s);

  int num_reg(RegType reg_type) const
  {
    return max_reg[reg_type];
  }

  friend ostream &operator<<<>(ostream &s, const Program<SRegint, SBit> &P);

  unsigned int size() const
  {
    return p.size();
  }

  // Returns true if we should execute a restart
  bool execute_instr(unsigned int i, Processor<SRegint, SBit> &Proc, Player &P,
                     Machine<SRegint, SBit> &machine, offline_control_data &OCD) const
  {
    return p[i].execute(Proc, P, machine, OCD);
  }
};

#endif
