/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2019, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Processor.h"
#include "GC/Garbled.h"
#include "Offline/DABitMachine.h"

extern MaliciousDABitMachine daBitMachine;
extern Base_Circuits Global_Circuit_Store;

Processor::Processor(int thread_num, unsigned int nplayers,
                     Player &P, offline_control_data &OCD)
    : online_thread_num(thread_num),
      daBitGen(daBitMachine, P, thread_num, OCD),
      aAF(thread_num),
      iop(nplayers)
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
  for (unsigned int i= 0; i < rwsr.size(); i++)
    {
      rwsr[i]= 0;
    }
  for (unsigned int i= 0; i < rwsb.size(); i++)
    {
      rwsb[i]= 0;
    }
#endif
}

// Now the routine to execute a program
void Processor::execute(const Program &prog, int argument, Player &P,
                        Machine &machine, offline_control_data &OCD)
{
  reg_maxp= prog.num_reg(MODP);
  reg_maxi= prog.num_reg(INT);
  Cp.resize(reg_maxp);
  Sp.resize(reg_maxp);
  Ri.resize(reg_maxi);
  srint.resize(reg_maxi);
  sbit.resize(reg_maxi);

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
  rwi.resize(reg_maxi);
  rwsr.resize(reg_maxi);
  rwsb.resize(reg_maxi);
  for (int i= 0; i < reg_maxi; i++)
    {
      rwi[i]= 0;
      rwsr[i]= 0;
      rwsb[i]= 0;
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
  prng.SetSeedFromRandom(seed);

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
          for (unsigned int connection= 0; connection < 3; connection++)
            {
              RunOpenCheck(P, machine.get_IO().Get_Check(), connection);
            }
          // Now parse any new schedule into the main machine
          machine.Load_Schedule_Into_Memory();
          printf("Loaded programs\n");
          fflush(stdout);
          PC= size;
        }
    }
}


void Processor::convert_sint_to_sregint_small(int i0, int i1, Player &P)
{
  unsigned int size= numBits(gfp::pr());
  unsigned long p=gfp::pr().get_ui();
  aBitVector x,y,z,r2;
  aBit bit;
  int b;
  bool done=false;
  vector<Share> bpr(size);
  vector<aBit>  b2r(size);
  // Get a set of size daBits, until the value is less than p
  while (!done)
    { // Get daBits
      daBitV.get_daBits(bpr, b2r, daBitGen);
      r2.assign_zero();
      for (unsigned int i=0; i<size; i++)
	{ r2.set_bit(i,b2r[i]); }
      z.sub(r2,p,P,aAF);
      bit=z.less_than_zero();
      Open_aBit(b, bit, P);
      if (b==1) 
        { done=true; }
    }

  // Now create the integer r, which is gauranteed to be uniform mod p
  Share r(P.whoami());
  r.assign_zero();
  for (int i= size - 1; i >= 0; i--)
    {
      r.add(r); // r=2*r
      r.add(bpr[i]);
    }

  // Now form x-r
  vector<Share> S_xr(1);
  S_xr[0].sub(read_Sp(i0), r);

  // Now open x-r
  vector<gfp> gfp_xr(1);
  OP.Open_To_All_Begin(gfp_xr, S_xr, P, 2);
  OP.Open_To_All_End(gfp_xr, S_xr, P, 2);

  // Now add z=(x-r)+r in the GC routines
  bigint bi_xr;
  to_bigint(bi_xr, gfp_xr[0]);
  z.add(bi_xr.get_ui(),r2,P,aAF);

  // Remember to do this modp
  y.sub(z,p,P,aAF);
  bit=y.less_than_zero();
  // z=bit*z+(1-b)*y
  z.Bit_AND(z,bit,P,aAF);
  bit.negate();
  y.Bit_AND(y,bit,P, aAF);
  z.add(z,y,P,aAF);


  // Now compare to p/2 to work out whether we need to negate
  y.sub(z,p/2,P,aAF);
  bit=y.less_than_zero();
  // Now compute   bit*z + (1-bit)*(-z)
  y.sub(p,z,P,aAF);   y.negate(y,P,aAF);
  z.Bit_AND(z,bit,P, aAF);
  bit.negate();
  y.Bit_AND(y,bit,P, aAF);
  x.add(z,y,P,aAF);

  // Write back into the processor
  write_srint(i1, x);
}


void Processor::convert_sint_to_sregint(int i0, int i1, Player &P)
{
  unsigned int size0= numBits(gfp::pr());
  unsigned int size1= 64+conv_stat_sec;
  if (size1>size0) { size1=size0; }
  if (Global_Circuit_Store.convert_ok == false)
    {
      if (sreg_bitl<size0)
        { throw cannot_do_conversion(); }
      convert_sint_to_sregint_small(i0, i1, P);
      return;
    }
  // Set up arrays for the GC stuff
  vector<vector<aBit>> input(2);
  input[0].resize(size0);
  input[1].resize(size1);
  vector<vector<aBit>> output(1, vector<aBit>(sreg_bitl));

  // Get a daBit
  vector<Share> bpr(size1);
  daBitV.get_daBits(bpr, input[1], daBitGen);

  // Now form r
  Share r(P.whoami());
  r.assign_zero();
  for (int i= size1 - 1; i >= 0; i--)
    {
      r.add(r); // r=2*r
      r.add(bpr[i]);
    }

  // Now form x-r
  vector<Share> S_xr(1);
  S_xr[0].sub(read_Sp(i0), r);

  // Now open x-r
  vector<gfp> gfp_xr(1);
  OP.Open_To_All_Begin(gfp_xr, S_xr, P, 2);
  OP.Open_To_All_End(gfp_xr, S_xr, P, 2);

  // Form vector<aBit> of gfp_xr
  bigint bi_xr;
  to_bigint(bi_xr, gfp_xr[0]);
  for (unsigned int i= 0; i < size0; i++)
    {
      if ((bi_xr & 1) == 0)
        {
          input[0][i].assign_zero();
        }
      else
        {
          input[0][i].assign_one();
        }
      bi_xr>>= 1;
    }

  // Evaluate the garbled circuit
  Base_Garbled_Circuit GC;
  GC.Garble(Global_Circuit_Store.BaseC[LSSS_to_GC], P, aAF);
  GC.Evaluate(output, input, Global_Circuit_Store.BaseC[LSSS_to_GC], P);

  // Write back into the processor
  write_srint(i1, output[0]);
}



void Processor::convert_sregint_to_sint(int i0, int i1, Player &P)
{
  /* Get sreg_bitl daBits */
  vector<aBit> a2r(sreg_bitl);
  vector<Share> apr(sreg_bitl);
  daBitV.get_daBits(apr, a2r, daBitGen);

  /* Add onto the input register */
  for (unsigned int i= 0; i < sreg_bitl; i++)
    {
      a2r[i].add(read_srint(i0).get_bit(i));
    }

  /* Now open these */
  vector<int> v(sreg_bitl);
  Open_aBits(v, a2r, P);
  Share one(1, P.whoami(), P.get_mac_keys());

  /* Now form the XOR on the modp side 
   *   apr[i]= apr[i]+v[i]-2*v[i]*apr[i]
   *         = (v[i]==0) * apr[i]  + (v[i]==1)*(-apr[i]+1)
   */
  for (unsigned int i= 0; i < sreg_bitl; i++)
    {
      if (v[i] == 1)
        {
          apr[i].negate();
          apr[i].add(one);
        }
    }

  /* Now form  ans = -2^{sreg_bitl-1} * apr[sreg_bitl-1]
   *                 + sum_{i=0}^sreg_bitl-2 2^i*apr[i]
   */
  bigint te= 2;
  Share tes, ans= apr[0];
  for (unsigned int i= 1; i < sreg_bitl - 1; i++)
    {
      tes.mul(apr[i], te);
      ans.add(tes);
      te<<= 1;
    }
  tes.mul(apr[sreg_bitl - 1], te);
  ans.sub(tes);

  // Write back into the processor
  write_Sp(i1, ans);
}
