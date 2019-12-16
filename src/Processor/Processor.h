/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2019, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _Processor
#define _Processor

#include "LSSS/Open_Protocol.h"
#include "Online/Machine.h"

#include "OT/aAND.h"
#include "OT/aBitVector.h"
#include "Offline/DABitGenerator.h"
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
  aBitVector aBV;
  aBit aB;
  aTriple T;
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
  vector<long> Ri;
  vector<aBitVector> srint;
  vector<aBit> sbit;

// In DEBUG mode we keep track of valid/invalid read/writes on the registers
#ifdef DEBUG
  vector<int> rwp;
  vector<int> rwi;
  vector<int> rwsr;
  vector<int> rwsb;
#endif

  // Program counter
  unsigned int PC;

  // These are here for DEBUG mode
  int reg_maxp, reg_maxi, reg_maxb;

  // This is the vector of partially opened values and shares we need to store
  // as the Open commands are split in two
  vector<gfp> PO;
  vector<Share> Sh_PO;
  Open_Protocol OP;

  int online_thread_num;

  // Keep track of how much data sent, and in how many rounds
  unsigned int sent, rounds;

  PRNG prng;

  /* Stuff for dealing with the Garbled Circuit functionality
   * within instructions
   */
  // In the case when the OT thread is active this holds the daBitGenerator for this thread
  AbstractDABitGenerator* daBitGen;
  // This holds the computed daBits
  daBitVector daBitV;

  // To make sure we do not need to keep allocating/deallocating memory
  // we maintain some scratch variables for use in routines
  TempVars temp;

  // Data structures for input and output of private data
  Processor_IO iop;

  // retrieve dabit generator to avoid dealing with pointers
  AbstractDABitGenerator& get_generator()
  {
    return *daBitGen;
  }

public:
  friend class Instruction;

  Processor(int online_thread_num, unsigned int nplayers, Player &P);
  ~Processor();

  void clear_registers();

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

  const long &read_Ri(int i) const
  {
    if (rwi[i] == 0)
      {
        throw Processor_Error("Invalid read on integer register");
      }
    return Ri.at(i);
  }
  long &get_Ri_ref(int i)
  {
    rwi[i]= 1;
    return Ri.at(i);
  }
  void write_Ri(int i, const long &x)
  {
    rwi[i]= 1;
    Ri.at(i)= x;
  }

  const aBitVector &read_srint(int i) const
  {
    if (rwsr[i] == 0)
      {
        throw Processor_Error("Invalid read on srint register");
      }
    return srint.at(i);
  }
  aBitVector &get_srint_ref(int i)
  {
    rwsr[i]= 1;
    return srint.at(i);
  }
  void write_srint(int i, const aBitVector &x)
  {
    rwsr[i]= 1;
    srint.at(i)= x;
  }

  const aBit &read_sbit(int i) const
  {
    if (rwsb[i] == 0)
      {
        throw Processor_Error("Invalid read on sbit register");
      }
    return sbit.at(i);
  }
  aBit &get_sbit_ref(int i)
  {
    rwsb[i]= 1;
    return sbit.at(i);
  }
  void write_sbit(int i, const aBit &x)
  {
    rwsb[i]= 1;
    sbit.at(i)= x;
  }

  void write_daBit(int i1, int j1)
  {
    daBitV.get_daBit(temp.Sansp, temp.aB, *daBitGen);
    rwp[i1 + reg_maxp]= 1;
    rwsb[j1]= 1;
    Sp.at(i1)= temp.Sansp;
    sbit.at(j1)= temp.aB;
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

  const long &read_Ri(int i) const
  {
    return Ri[i];
  }
  long &get_Ri_ref(int i)
  {
    return Ri[i];
  }
  void write_Ri(int i, const long &x)
  {
    Ri[i]= x;
  }

  const aBitVector &read_srint(int i) const
  {
    return srint[i];
  }
  aBitVector &get_srint_ref(int i)
  {
    return srint[i];
  }
  void write_srint(int i, const aBitVector &x)
  {
    srint[i]= x;
  }

  const aBit &read_sbit(int i) const
  {
    return sbit[i];
  }
  aBit &get_sbit_ref(int i)
  {
    return sbit[i];
  }
  void write_sbit(int i, const aBit &x)
  {
    sbit[i]= x;
  }
  void write_daBit(int i1, int j1)
  {
     daBitV.get_daBit(temp.Sansp, temp.aB, *daBitGen);
     write_Sp(i1, temp.Sansp);
     write_sbit(j1, temp.aB);
  }

#endif

  /* Run interaction with other players */

  /* Direct access to PO class*/
  void Open_To_All_Begin(vector<gfp> &values, const vector<Share> &S, Player &P, int connection)
  {
    OP.Open_To_All_Begin(values, S, P, connection);
  }
  void Open_To_All_End(vector<gfp> &values, const vector<Share> &S, Player &P, int connection)
  {
    OP.Open_To_All_End(values, S, P, connection);
  }

  /* Open/Close Registers*/
  void POpen_Start(const vector<int> &reg, int size, Player &P);
  void POpen_Stop(const vector<int> &reg, int size, Player &P);

  void RunOpenCheck(Player &P, const string &aux, int connection)
  {
    OP.RunOpenCheck(P, aux, connection, false);
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

  // Converts a sint register i0 to a sregint register i1
  //   Uses the daBits
  void convert_sint_to_sregint(int i0, int i1, Player &P);

  // This is a special version for when log_2(p)<sreg_bitl
  void convert_sint_to_sregint_small(int i0, int i1, Player &P);

  // Converts a sregint register i0 to a srint register i1
  //   Uses the daBits
  void convert_sregint_to_sint(int i0, int i1, Player &P);

  // Apply one of the indirect GC's
  void apply_GC(const vector<int> &arguments, Player &P);

  // Apply one of the local functions
  void apply_local_function(RegType RT, SecrecyType ST,
                            const vector<int> &arguments);
};

#endif
