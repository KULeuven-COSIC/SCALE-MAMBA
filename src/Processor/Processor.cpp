/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Processor.h"
#include "GC/Garbled.h"
#include "GC/Q2_Evaluate.h"
#include "Offline/DABitMachine.h"
#include "Tools/util_containers.h"

extern Base_Circuits Global_Circuit_Store;
extern vector<sacrificed_data> SacrificeD;

template<class SRegint, class SBit>
Processor<SRegint, SBit>::Processor(int thread_num, unsigned int nplayers,
                                    Machine<SRegint, SBit> &machine, Player &P)
    : online_thread_num(thread_num), iop(nplayers)
{

  daBitGen= machine.daBitMachine.new_generator(P, thread_num);
  rounds= 0;
  sent= 0;
  stack_Cp.reserve(16384);
  stack_Sp.reserve(16384);
  stack_int.reserve(16384);
  stack_srint.reserve(16384);
  stack_sbit.reserve(16384);
}

template<class SRegint, class SBit>
Processor<SRegint, SBit>::~Processor()
{
  fprintf(stderr, "Sent %d elements in %d rounds\n", sent, rounds);
#ifdef VERBOSE
  cout << "dabitgen statistics:" << endl;
  cout << "Produced " << daBitGen->total << " dabits" << endl;
  for (auto &timer : daBitGen->timers)
    cout << timer.first << " took time " << timer.second.elapsed() / 1e6 << endl;
#endif
  delete daBitGen;
}

template<class SRegint, class SBit>
void Processor<SRegint, SBit>::POpen_Start(const vector<int> &reg, int size, Player &P)
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
      /*
      stringstream os;
      Sp[reg[0]].output(os, true);
      printf("Share : %d :  %s \n",reg[0],os.str().c_str());
      */
      for (int i= 0; i < sz; i++)
        {
          Sh_PO.push_back(get_Sp_ref(reg[i]));
        }
    }
  PO.resize(sz * size);
  P.OP->Open_To_All_Begin(PO, Sh_PO, P, 0);
}

template<class SRegint, class SBit>
void Processor<SRegint, SBit>::POpen_Stop(const vector<int> &reg, int size, Player &P)
{
  int sz= reg.size();
  PO.resize(sz * size);
  P.OP->Open_To_All_End(PO, Sh_PO, P, 0);
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

template<class SRegint, class SBit>
void Processor<SRegint, SBit>::clear_registers()
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
  for (unsigned int i= 0; i < srint.size(); i++)
    {
      srint[i].assign_zero();
    }
  for (unsigned int i= 0; i < sbit.size(); i++)
    {
      sbit[i].assign_zero();
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
template<class SRegint, class SBit>
void Processor<SRegint, SBit>::execute(const Program<SRegint, SBit> &prog, int argument,
                                       unsigned int start,
                                       Player &P,
                                       Machine<SRegint, SBit> &machine, offline_control_data &OCD)
{
  reg_maxp= prog.num_reg(MODP);
  reg_maxi= prog.num_reg(INT);
  reg_maxb= prog.num_reg(SBIT);

  Cp.resize(reg_maxp);
  Sp.resize(reg_maxp);
  Ri.resize(reg_maxi);
  srint.resize(reg_maxi);
  sbit.resize(reg_maxb);

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
  for (int i= 0; i < reg_maxi; i++)
    {
      rwi[i]= 0;
      rwsr[i]= 0;
    }
  rwsb.resize(reg_maxb);
  for (int i= 0; i < reg_maxb; i++)
    {
      rwsb[i]= 0;
    }
#endif

  unsigned int size= prog.size();
  PC= start;
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

template<class SRegint, class SBit>
void Processor<SRegint, SBit>::convert_sint_to_sregint_small(int i0, int i1, Player &P)
{
  unsigned int size= numBits(gfp::pr());
  unsigned long p= gfp::pr().get_ui();
  SRegint x, y, z, r2;
  SBit bit;
  word b;
  bool done= false;
  vector<Share> bpr(size);
  vector<SBit> b2r(size);
  auto &daBitGen= get_generator();
  // Get a set of size daBits, until the value is less than p
  while (!done)
    { // Get daBits
      daBitV.get_daBits(bpr, b2r, daBitGen, P);
      r2.assign_zero();
      for (unsigned int i= 0; i < size; i++)
        {
          r2.set_bit(i, b2r[i]);
        }
      z.sub(r2, p, P, online_thread_num);
      bit= z.less_than_zero();
      P.OP2->Open_Bit(b, bit, P);
      if (b == 1)
        {
          done= true;
        }
    }

  // Now create the integer r, which is guaranteed to be uniform mod p
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
  P.OP->Open_To_All_Begin(gfp_xr, S_xr, P, 2);
  P.OP->Open_To_All_End(gfp_xr, S_xr, P, 2);

  // Now add z=(x-r)+r in the GC routines
  bigint bi_xr;
  to_bigint(bi_xr, gfp_xr[0]);
  z.add(bi_xr.get_ui(), r2, P, online_thread_num);

  // Remember to do this modp
  y.sub(z, p, P, online_thread_num);
  bit= y.less_than_zero();
  // z=bit*z+(1-b)*y
  z.Bit_AND(z, bit, P, online_thread_num);
  bit.negate();
  y.Bit_AND(y, bit, P, online_thread_num);
  z.Bitwise_XOR(z, y);

  // Now compare to p/2 to work out whether we need to negate
  y.sub(z, p / 2, P, online_thread_num);
  bit= y.less_than_zero();
  // Now compute   bit*z + (1-bit)*(-z)
  y.sub(p, z, P, online_thread_num);
  y.negate(y, P, online_thread_num);
  z.Bit_AND(z, bit, P, online_thread_num);
  bit.negate();
  y.Bit_AND(y, bit, P, online_thread_num);
  x.Bitwise_XOR(z, y);

  // Write back into the processor
  write_srint(i1, x);
}

template<>
void Processor<aBitVector, aBit>::convert_sint_to_sregint_sub(
    const vector<vector<aBit>> &input, int i1, Player &P)
{
  vector<vector<aBit>> output(1, vector<aBit>(sreg_bitl));

  // Evaluate the garbled circuit
  Base_Garbled_Circuit GC;
  GC.Garble(Global_Circuit_Store.Circuits[LSSS_to_GC], P, online_thread_num);
  GC.Evaluate(output, input, Global_Circuit_Store.Circuits[LSSS_to_GC], P);

  // Write back into the processor
  write_srint(i1, output[0]);
}

template<>
void Processor<aBitVector2, Share2>::convert_sint_to_sregint_sub(
    const vector<vector<Share2>> &input, int i1, Player &P)
{
  vector<vector<Share2>> output(1, vector<Share2>(sreg_bitl));

  // Evaluate the garbled circuit
  Evaluate(output, input, Global_Circuit_Store.Circuits[LSSS_to_GC], P, online_thread_num);

  // Write back into the processor
  aBitVector2 temp;
  collapse(temp, output[0]);
  write_srint(i1, temp);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
template<>
void Processor<aBitVector2, Share2>::convert_sint_to_sregint_sub(
    const vector<vector<aBit>> &input, int i1, Player &P)
{
  // This is a dummy function to get around a C++ quirk
  throw not_implemented();
}

template<>
void Processor<aBitVector, aBit>::convert_sint_to_sregint_sub(
    const vector<vector<Share2>> &input, int i1, Player &P)
{
  // This is a dummy function to get around a C++ quirk
  throw not_implemented();
}
#pragma GCC diagnostic pop

template<class SRegint, class SBit>
void Processor<SRegint, SBit>::convert_sint_to_sregint(int i0, int i1, Player &P)
{
  unsigned int size0= numBits(gfp::pr());
  unsigned int size1= sreg_bitl + conv_stat_sec;
  if (size1 > size0)
    {
      size1= size0;
    }
  if (Global_Circuit_Store.convert_ok == false)
    {
      if (sreg_bitl < size0)
        {
          throw cannot_do_conversion();
        }
      convert_sint_to_sregint_small(i0, i1, P);
      return;
    }
  // Set up arrays for the GC stuff
  vector<vector<SBit>> input(2);
  input[0].resize(size0);
  input[1].resize(size1);

  // Get a daBit
  vector<Share> bpr(size1);
  auto &daBitGen= get_generator();
  daBitV.get_daBits(bpr, input[1], daBitGen, P);

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
  P.OP->Open_To_All_Begin(gfp_xr, S_xr, P, 2);
  P.OP->Open_To_All_End(gfp_xr, S_xr, P, 2);

  // Form vector<SBit> of gfp_xr
  bigint bi_xr;
  to_bigint(bi_xr, gfp_xr[0]);
  for (unsigned int i= 0; i < size0; i++)
    {
      if ((bi_xr & 1) == 0)
        {
          input[0][i].assign_zero(P.whoami());
        }
      else
        {
          input[0][i].assign_one(P.whoami());
        }
      bi_xr>>= 1;
    }

  // Evaluate the garbled circuit
  convert_sint_to_sregint_sub(input, i1, P);
}

template<class SRegint, class SBit>
void Processor<SRegint, SBit>::convert_sregint_to_sint_sub(int i0, vector<Share> &apr, Player &P)
{
  /* Get sreg_bitl daBits */
  vector<SBit> a2r(sreg_bitl);
  auto &daBitGen= get_generator();
  daBitV.get_daBits(apr, a2r, daBitGen, P);

  /* Add onto the input register */
  for (unsigned int i= 0; i < sreg_bitl; i++)
    {
      a2r[i].add(read_srint(i0).get_bit(i));
    }

  /* Now open these */
  vector<word> v(sreg_bitl);
  P.OP2->Open_Bits(v, a2r, P);
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
}

template<class SRegint, class SBit>
void Processor<SRegint, SBit>::convert_sregint_to_sint(int i0, int i1, Player &P)
{
  vector<Share> apr(sreg_bitl);

  convert_sregint_to_sint_sub(i0, apr, P);

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

template<class SRegint, class SBit>
void Processor<SRegint, SBit>::convert_suregint_to_sint(int i0, int i1, Player &P)
{
  vector<Share> apr(sreg_bitl);

  convert_sregint_to_sint_sub(i0, apr, P);

  /* Now form  ans = sum_{i=0}^sreg_bitl-1 2^i*apr[i]
   */
  bigint te= 2;
  Share tes, ans= apr[0];
  for (unsigned int i= 1; i < sreg_bitl; i++)
    {
      tes.mul(apr[i], te);
      ans.add(tes);
      te<<= 1;
    }

  // Write back into the processor
  write_Sp(i1, ans);
}

template<class SRegint, class SBit>
void Processor<SRegint, SBit>::convert_sbit_to_sint(int i0, int i1, Player &P)
{
  Share apr;
  SBit a2r;

  /* Get a single daBit */
  auto &daBitGen= get_generator();
  daBitV.get_daBit(apr, a2r, daBitGen, P);

  /* Add onto the input register */
  a2r.add(read_sbit(i0));

  /* Now open it */
  word v;
  P.OP2->Open_Bit(v, a2r, P);
  Share one(1, P.whoami(), P.get_mac_keys());

  /* Now form the XOR on the modp side
   *   apr= apr+v-2*v*apr
   *      = (v==0) * apr  + (v==1)*(-apr+1)
   */
  if ((v & 1) == 1)
    {
      apr.negate();
      apr.add(one);
    }

  // Write back into the processor
  write_Sp(i1, apr);
}

template<class SRegint, class SBit>
void Processor<SRegint, SBit>::convert_sint_to_sbit(int i0, int i1, Player &P,
                                                    offline_control_data &OCD)
{
  Share apr;
  SBit a2r;

  /* Get a single daBit */
  auto &daBitGen= get_generator();
  daBitV.get_daBit(apr, a2r, daBitGen, P);

  /* Multiply apr by the input register */

  // First get the triple
  list<Share> la, lb, lc;
  Wait_For_Preproc(TRIPLE, 1, online_thread_num, OCD);
  OCD.mul_mutex[online_thread_num].lock();
  Split_Lists(la, SacrificeD[online_thread_num].TD.ta, 1);
  Split_Lists(lb, SacrificeD[online_thread_num].TD.tb, 1);
  Split_Lists(lc, SacrificeD[online_thread_num].TD.tc, 1);
  OCD.mul_mutex[online_thread_num].unlock();

  // Now compute rho and sigma
  vector<Share> rho_sigma(2);
  Share a= la.front();
  Share b= lb.front();
  Share c= lc.front();

  rho_sigma[0].sub(read_Sp(i0), a);
  rho_sigma[1].sub(apr, b);

  // Open rho and sigma
  vector<gfp> gfp_rho_sigma(2);
  P.OP->Open_To_All_Begin(gfp_rho_sigma, rho_sigma, P, 2);
  P.OP->Open_To_All_End(gfp_rho_sigma, rho_sigma, P, 2);

  // Compute the product
  Share te1, te2, product= c;
  gfp te3;
  te3.mul(gfp_rho_sigma[0], gfp_rho_sigma[1]);
  te1.mul(b, gfp_rho_sigma[0]);
  te2.mul(a, gfp_rho_sigma[1]);

  product.add(te1);
  product.add(te2);
  product.add(product, te3, P.get_mac_keys());

  // Now compute the shared bit on the modp side of apr xor Sp[i0]
  vector<Share> bit(1);
  bit[0].add(apr, read_Sp(i0));
  bit[0].sub(product);
  bit[0].sub(product);

  // Open this bit
  vector<gfp> gfp_bit(1);
  P.OP->Open_To_All_Begin(gfp_bit, bit, P, 2);
  P.OP->Open_To_All_End(gfp_bit, bit, P, 2);

  // Xor the bit on the gf2 side
  bigint ans;
  to_bigint(ans, gfp_bit[0]);
  a2r.add(ans.get_ui());

  // Write back into the processor
  write_sbit(i1, a2r);
}

/* Arguments are obtained from the srint stack, and then outputs
 * are pushed back onto the srint stack
 */
template<>
void Processor<aBitVector, aBit>::apply_GC(unsigned int circuit_number, Player &P)
{
  // Check circuit is loaded
  Global_Circuit_Store.check(circuit_number);

  // Set up arrays for the IO from the GC and load in the inputs
  unsigned int numI= Global_Circuit_Store.Circuits[circuit_number].num_inputs();
  unsigned int numO= Global_Circuit_Store.Circuits[circuit_number].num_outputs();
  vector<vector<aBit>> input(numI);
  vector<vector<aBit>> output(numO);
  aBitVector temp;

  int regib;
  bool empty= true;
  // The first argument is at the bottom of the stack, the
  // last is on the top. So need to take this into account
  for (int i= numI - 1; i >= 0; i--)
    {
      input[i].resize(Global_Circuit_Store.Circuits[circuit_number].num_iWires(i));
      for (int j= input[i].size() - 1; j >= 0; j--)
        {
          if (empty)
            {
              pop_srint(temp);
              empty= false;
              regib= sreg_bitl - 1;
            }
          input[i][j]= temp.get_bit(regib);
          regib--;
          if (regib < 0)
            {
              empty= true;
            }
        }
    }

  for (unsigned int i= 0; i < numO; i++)
    {
      output[i].resize(Global_Circuit_Store.Circuits[circuit_number].num_oWires(i));
    }

  // Evaluate the garbled circuit
  Base_Garbled_Circuit GC;
  GC.Garble(Global_Circuit_Store.Circuits[circuit_number], P, online_thread_num);
  GC.Evaluate(output, input, Global_Circuit_Store.Circuits[circuit_number], P);

  // Now process the outputs
  regib= 0;
  temp.assign_zero();
  for (int i= numO - 1; i >= 0; i--)
    {
      for (unsigned int j= 0; j < output[i].size(); j++)
        {
          temp.set_bit(regib, output[i][j]);
          regib++;
          if (regib == sreg_bitl)
            {
              regib= 0;
              push_srint(temp);
              temp.assign_zero();
            }
        }
    }
}

/* Arguments are obtained from the srint stack, and then outputs
 * are pushed back onto the srint stack
 */
template<>
void Processor<aBitVector2, Share2>::apply_GC(unsigned int circuit_number, Player &P)
{
  // Check circuit is loaded
  Global_Circuit_Store.check(circuit_number);

  // Set up arrays for the IO from the GC and load in the inputs
  unsigned int numI= Global_Circuit_Store.Circuits[circuit_number].num_inputs();
  unsigned int numO= Global_Circuit_Store.Circuits[circuit_number].num_outputs();
  vector<vector<Share2>> input(numI);
  vector<vector<Share2>> output(numO);
  aBitVector2 temp;
  vector<Share2> temp_bits;

  int regib;
  bool empty= true;
  // The first argument is at the bottom of the stack, the
  // last is on the top. So need to take this into account
  for (int i= numI - 1; i >= 0; i--)
    {
      input[i].resize(Global_Circuit_Store.Circuits[circuit_number].num_iWires(i));
      for (int j= input[i].size() - 1; j >= 0; j--)
        {
          if (empty)
            {
              pop_srint(temp);
              empty= false;
              expand(temp_bits, temp);
              regib= sreg_bitl - 1;
            }
          input[i][j]= temp_bits[regib];
          regib--;
          if (regib < 0)
            {
              empty= true;
            }
        }
    }

  for (unsigned int i= 0; i < numO; i++)
    {
      output[i].resize(Global_Circuit_Store.Circuits[circuit_number].num_oWires(i));
    }

  // Evaluate the garbled circuit
  Evaluate(output, input, Global_Circuit_Store.Circuits[circuit_number], P, online_thread_num);

  // Now process the outputs
  regib= 0;
  for (int i= numO - 1; i >= 0; i--)
    {
      for (unsigned int j= 0; j < output[i].size(); j++)
        {
          temp_bits[regib]= output[i][j];
          regib++;
          if (regib == sreg_bitl)
            {
              regib= 0;
              collapse(temp, temp_bits);
              push_srint(temp);
            }
        }
    }
}

template class Processor<aBitVector, aBit>;
template class Processor<aBitVector2, Share2>;
