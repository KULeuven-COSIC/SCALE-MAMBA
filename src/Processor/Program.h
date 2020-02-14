/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _Program
#define _Program

#include "Processor/Instruction.h"
class Processor;
class Machine;
class offline_control_data;

/* A program is a vector of instructions */

class Program
{
  vector<Instruction> p;

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

  friend ostream &operator<<(ostream &s, const Program &P);

  unsigned int size() const
  {
    return p.size();
  }

  // Returns true if we should execute a restart
  bool execute_instr(unsigned int i, Processor &Proc, Player &P,
                     Machine &machine, offline_control_data &OCD) const
  {
    return p[i].execute(Proc, P, machine, OCD);
  }
};

#endif
