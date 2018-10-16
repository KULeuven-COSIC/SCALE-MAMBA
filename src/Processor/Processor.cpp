/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Processor.h"

Processor::Processor(int thread_num, unsigned int nplayers)
    : online_thread_num(thread_num), iop(nplayers)
{
  rounds= 0;
  sent= 0;
}

Processor::~Processor()
{
  fprintf(stderr, "Sent %d elements in %d rounds\n", sent, rounds);
}

void Processor::POpen_Start(const vector<int> &reg, int size, Player &P)
{
  // printf("POpen_Start : size = %d     reg.size = %lu\n ",size,reg.size());
  int sz= reg.size();
  Sh_PO.clear();
  Sh_PO.reserve(sz * size);
  if (size > 1)
    {
      for (typename vector<int>::const_iterator reg_it= reg.begin();
           reg_it != reg.end(); reg_it++)
        {
          typename vector<Share>::iterator begin= Sp.begin() + *reg_it;
          Sh_PO.insert(Sh_PO.end(), begin, begin + size);
        }
    }
  else
    {
      stringstream os;
      Sp[reg[0]].output(os, true);
      // printf("Share : %d :  %s \n",reg[0],os.str().c_str());
      for (int i= 0; i < sz; i++)
        {
          Sh_PO.push_back(get_Sp_ref(reg[i]));
        }
    }
  PO.resize(sz * size);
  OP.Open_To_All_Begin(PO, Sh_PO, P);
}

void Processor::POpen_Stop(const vector<int> &reg, int size, Player &P)
{
  int sz= reg.size();
  PO.resize(sz * size);
  OP.Open_To_All_End(PO, Sh_PO, P);
  if (size > 1)
    {
      typename vector<gfp>::iterator PO_it= PO.begin();
      for (typename vector<int>::const_iterator reg_it= reg.begin();
           reg_it != reg.end(); reg_it++)
        {
          for (typename vector<gfp>::iterator C_it= Cp.begin() + *reg_it;
               C_it != Cp.begin() + *reg_it + size; C_it++)
            {
              *C_it= *PO_it;
              PO_it++;
            }
        }
    }
  else
    {
      for (unsigned int i= 0; i < reg.size(); i++)
        {
          get_Cp_ref(reg[i])= PO[i];
        }
    }

  increment_counters(reg.size() * size);
}

void Processor::clear_registers()
{
  for (unsigned int i= 0; i < Cp.size(); i++)
    {
      Cp[i].assign_zero();
    }
  for (unsigned int i= 0; i < Sp.size(); i++)
    {
      Sp[i].assign_zero();
    }
  for (unsigned int i= 0; i < Ri.size(); i++)
    {
      Ri[i]= 0;
    }

#ifdef DEBUG
  for (unsigned int i= 0; i < rwp.size(); i++)
    {
      rwp[i]= 0;
    }
  for (unsigned int i= 0; i < rwi.size(); i++)
    {
      rwi[i]= 0;
    }
#endif
}

// Now the routine to execute a program
void Processor::execute(const Program &prog, int argument, Player &P,
                        Machine &machine, offline_control_data &OCD)
{
  int reg_maxp= prog.num_reg(MODP);
  int reg_maxi= prog.num_reg(INT);
  Cp.resize(reg_maxp);
  Sp.resize(reg_maxp);
  Ri.resize(reg_maxi);

  for (int i= 0; i < reg_maxp; i++)
    {
      Sp[i].set_player(P.whoami());
    }

#ifdef DEBUG
  rwp.resize(2 * reg_maxp);
  for (int i= 0; i < 2 * reg_maxp; i++)
    {
      rwp[i]= 0;
    }
  rwi.resize(2 * reg_maxi);
  for (int i= 0; i < 2 * reg_maxi; i++)
    {
      rwi[i]= 0;
    }
#endif

  unsigned int size= prog.size();
  PC= 0;
  arg= argument;
  // This is a deterministic PRNG, used in some ORAM routines
  // Having a fixed seed is therefore no problem
  // Used to allow the generation of a pseudo-random CINT
  uint8_t seed[SEED_SIZE];
  memset(seed, 0, SEED_SIZE);
  prng.SetSeed(seed);

  while (PC < size)
    {
      bool restart= prog.execute_instr(PC, *this, P, machine, OCD);
      if (restart)
        {
          /* Call trigger 
           * This halts machine, until something external says OK to go
           * Possibly loading a new schedule
           */
          machine.get_IO().trigger(machine.schedule);
          printf("Restarting...\n");
          RunOpenCheck(P, machine.get_IO().Get_Check());
          // Now parse any new schedule into the main machine
          machine.Load_Schedule_Into_Memory();
          printf("Loaded programs\n");
          fflush(stdout);
          PC= size;
        }
    }
}
