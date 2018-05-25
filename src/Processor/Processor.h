/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _Processor
#define _Processor

#include "LSSS/Open_Protocol.h"
#include "Online/Machine.h"

#include "Processor_IO.h"
#include "Program.h"

#include <stack>
using namespace std;

struct TempVars
{
  gfp ansp;
  Share Sansp;
  bigint aa, aa2;
  // INPUT and LDSI
  gfp rrp, tp, tmpp;
  gfp xip;
};

class Processor
{
  // Integer stack (used for loops to store the running variable)
  stack<long> stacki;

  // Optional argument to the tape
  int arg;

  // Registers of various types
  vector<gfp> Cp;
  vector<Share> Sp;
  vector<long> Ci;

// In DEBUG mode we keep track of valid/invalid read/writes on the registers
#ifdef DEBUG
  vector<int> rwp;
  vector<int> rwi;
#endif

  // Program counter
  unsigned int PC;

  // This is the vector of partially opened values and shares we need to store
  // as the Open commands are split in two
  vector<gfp> PO;
  vector<Share> Sh_PO;
  Open_Protocol OP;

  int online_thread_num;

  // Keep track of how much data sent, and in how many rounds
  unsigned int sent, rounds;

  PRNG prng;

public:
  Processor(int online_thread_num, unsigned int nplayers);
  ~Processor();

  int get_thread_num() const
  {
    return online_thread_num;
  }

  // Stack operations
  void pushi(long x)
  {
    stacki.push(x);
  }
  void popi(long &x)
  {
    x= stacki.top();
    stacki.pop();
  }

  // Argument operations
  int get_arg() const
  {
    return arg;
  }
  void set_arg(int new_arg)
  {
    arg= new_arg;
  }

  // Increment program counter
  void increment_PC()
  {
    PC++;
  }
  // Perform a jump
  void jump(unsigned int new_pc)
  {
    PC= new_pc;
  }
  void relative_jump(signed int jump)
  {
    PC+= jump;
  }
  unsigned int get_PC() const
  {
    return PC;
  }

  // To make sure we do not need to keep allocating/deallocating memory
  // we maintain some scratch variables for use in routines
  TempVars temp;

  // Data structures for input and output of private data
  //    XXXX Should perhaps put in private and access via a function
  Processor_IO iop;

/* Read and write the registers */
#ifdef DEBUG
  const gfp &read_Cp(int i) const
  {
    if (rwp[i] == 0)
      {
        throw Processor_Error("Invalid read on clear register");
      }
    return Cp.at(i);
  }
  const Share &read_Sp(int i) const
  {
    if (rwp[i + reg_maxp] == 0)
      {
        throw Processor_Error("Invalid read on shared register");
      }
    return Sp.at(i);
  }
  gfp &get_Cp_ref(int i)
  {
    rwp[i]= 1;
    return Cp.at(i);
  }
  Share &get_Sp_ref(int i)
  {
    rwp[i + reg_maxp]= 1;
    return Sp.at(i);
  }
  void write_Cp(int i, const gfp &x)
  {
    rwp[i]= 1;
    Cp.at(i)= x;
  }
  void write_Sp(int i, const Share &x)
  {
    rwp[i + reg_maxp]= 1;
    Sp.at(i)= x;
  }

  const long &read_Ci(int i) const
  {
    if (rwi[i] == 0)
      {
        throw Processor_Error("Invalid read on integer register");
      }
    return Ci.at(i);
  }
  long &get_Ci_ref(int i)
  {
    rwi[i]= 1;
    return Ci.at(i);
  }
  void write_Ci(int i, const long &x)
  {
    rwi[i]= 1;
    Ci.at(i)= x;
  }
#else
  const gfp &read_Cp(int i) const
  {
    return Cp[i];
  }
  const Share &read_Sp(int i) const
  {
    return Sp[i];
  }
  gfp &get_Cp_ref(int i)
  {
    return Cp[i];
  }
  Share &get_Sp_ref(int i)
  {
    return Sp[i];
  }
  void write_Cp(int i, const gfp &x)
  {
    Cp[i]= x;
  }
  void write_Sp(int i, const Share &x)
  {
    Sp[i]= x;
  }

  const long &read_Ci(int i) const
  {
    return Ci[i];
  }
  long &get_Ci_ref(int i)
  {
    return Ci[i];
  }
  void write_Ci(int i, const long &x)
  {
    Ci[i]= x;
  }
#endif

  /* Run interaction with other players */
  void POpen_Start(const vector<int> &reg, int size, Player &P);
  void POpen_Stop(const vector<int> &reg, int size, Player &P);
  void RunOpenCheck(Player &P)
  {
    OP.RunOpenCheck(P, false); // XXXX
  }

  // Now the routine to execute a program with given argument
  void execute(const Program &prog, int argument, Player &P, Machine &machine,
               offline_control_data &OCD);

  unsigned int get_random_uint()
  {
    return prng.get_uint();
  }

  // Add rounds up and add data sent in
  void increment_counters(unsigned int size)
  {
    sent+= size;
    rounds++;
  }
};

#endif
