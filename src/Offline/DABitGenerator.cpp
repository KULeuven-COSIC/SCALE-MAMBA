/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "DABitGenerator.h"
#include "DABitMachine.h"
#include "OT/BitVector.h"
#include "OT/OT_Thread_Data.h"
#include "Processor/Processor_IO.h"
#include "Tools/util_containers.h"

#include "unistd.h"
#include <cassert>
#include <list>

extern vector<sacrificed_data> SacrificeD;
extern OT_Thread_Data OTD;

template<>
list<aBit> AbstractDABitGenerator<aBit>::generate_masks(unsigned int num)
{
  return OTD.aBD.get_aShares(online_thread_num, num);
}

template<>
list<Share2> AbstractDABitGenerator<Share2>::generate_masks(unsigned int num)
{
  list<Share2> ans;
  vector<Share2> ex;
  aBitVector2 aBV;
  unsigned int cnt= 0, p;
  while (cnt < num)
    {
      Share2 temp= prss.next_share();
      aBV.assign(temp);
      expand(ex, aBV);
      p= 0;
      while (cnt < num && p < 64)
        {
          ans.push_back(ex[p]);
          p++;
          cnt++;
        }
    }
  return ans;
}

// adjusting the MACs and shares for the input bits
template<class SBit>
void SmallPrimeDABitGenerator<SBit>::adjust_and_pack(vector<Share> &Shp, vector<SBit> &Sh2,
                                                     unsigned int player, vector<bool> bits,
                                                     Player &P)
{
  // Assume we take SBit shares from Garbling
  //
  (*this).timers["Processing inputs bits in gf(2)"].start();
  // Seems that get_aShares is the costly bit here
  (*this).timers["get aShares"].start();
  list<SBit> random_masks_2= (*this).generate_masks((*this).num_dabits);
  (*this).timers["get aShares"].stop();
  vector<SBit> vec_random_masks_2;
#ifdef BENCH_OFFLINE
  if (Share::SD.Etype == HSS)
    {
      P.abits+= (*this).num_dabits;
    }
  else
    {
      P.mod2s+= (*this).num_dabits;
    }
#endif

  // dump the list into vector
  vec_random_masks_2.reserve((*this).num_dabits);
  copy(begin(random_masks_2), end(random_masks_2), back_inserter(vec_random_masks_2));
  random_masks_2.clear();

  // Now open masks in F_2 to current player
  vector<word> opened_masks_2;

  P.OP2->Open_Bits_To(opened_masks_2, player, vec_random_masks_2, P);

  (*this).timers["Processing inputs bits in gf(2)"].stop();

  // Now retrieve random masks from gf(p) world
  // Wait until enough in the queue
  (*this).timers["Waiting for bits in GF(p)"].start();
  Wait_For_Preproc(DATA_INPUT_MASK, (*this).num_dabits, (*this).online_thread_num, (*this).OCD, player);
  (*this).timers["Waiting for bits in GF(p)"].stop();

  (*this).timers["Retrieving input masks"].start();
  vector<gfp> opened_masks_p;
  vector<Share> vec_random_masks_p;

  // Lock to retrieve gf(p) masks
  (*this).OCD.OCD_mutex[(*this).online_thread_num].lock();
  Split_Lists(vec_random_masks_p, SacrificeD[(*this).online_thread_num].ID.ios[player], (*this).num_dabits);
  if (player == P.whoami())
    {
      Split_Lists(opened_masks_p, SacrificeD[(*this).online_thread_num].ID.opened_ios, (*this).num_dabits);
    }
  (*this).OCD.OCD_mutex[(*this).online_thread_num].unlock();
  (*this).timers["Retrieving input masks"].stop();

#ifdef BENCH_OFFLINE
  P.inputs+= (*this).num_dabits;
#endif

  (*this).timers["Serializing bits"].start();
  if (player == P.whoami())
    {
      unsigned int pos= 0;
      for (size_t i= 0; i < (*this).num_dabits; i++)
        {
          // Deal with gf(2) case
          int wire_bit= bits[i];
          wire_bit^= opened_masks_2[i];
          (*this).buff.get_buffer()[pos]= (char) wire_bit;
          pos++;
          SBit epsilon_2= vec_random_masks_2[i];
          epsilon_2.add(wire_bit);
          Sh2[i]= epsilon_2;

          // Deal with gf(p) case
          gfp epsilon_p;
          if (bits[i])
            epsilon_p.assign_one();
          else
            epsilon_p.assign_zero();

          // Deal with gf(p) case
          epsilon_p.sub(opened_masks_p[i]);
          pos+= epsilon_p.output((*this).buff.get_buffer() + pos);
          // place mask + epsilon into Shp
          Shp[i].add(vec_random_masks_p[i], epsilon_p, P.get_mac_keys());
        }
    }
  else
    {
      for (size_t i= 0; i < (*this).num_dabits; i++)
        {
          Sh2[i]= vec_random_masks_2[i];
          Shp[i]= vec_random_masks_p[i];
        }
    }

  (*this).timers["Serializing bits"].stop();
}

template<class SBit>
void SmallPrimeDABitGenerator<SBit>::provide_random_inputs(vector<vector<Share>> &Shp, vector<vector<SBit>> &Sh2, Player &P)
{

  vector<bool> inp_bits((*this).num_dabits);
  // Get random bits based on unsigned char blocks
  // This assumes that 1 byte = 8 bits
  size_t chunk_size= sizeof(unsigned char) * 8;
  size_t num_chunks= (*this).num_dabits / chunk_size + 1;
  (*this).timers["PRG bit gen"].start();
  for (size_t i= 0; i < num_chunks; ++i)
    {
      unsigned char block= G.get_uchar();
      for (size_t j= 0; j < chunk_size; j++)
        {
          if (i * chunk_size + j < (*this).num_dabits)
            inp_bits[i * chunk_size + j]= (block >> j) & 1;
        }
    }

  (*this).timers["PRG bit gen"].stop();

  // Now each party inputs their random bits
  for (unsigned int player= 0; player < P.nplayers(); ++player)
    {
      Shp[player].resize((*this).num_dabits);
      Sh2[player].resize((*this).num_dabits);

      adjust_and_pack(Shp[player], Sh2[player], player, inp_bits, P);
    }
  for (unsigned int player= 0; player < P.nplayers(); player++)
    {
      if (player == P.whoami())
        {
          (*this).timers["Sending input bits"].start();
          P.send_all((*this).buff.get_buffer(), (*this).num_dabits * (1 + gfp::size()), 2, false);
          (*this).timers["Sending input bits"].stop();
        }
      else
        {
          (*this).timers["Receiving input bits"].start();
          // Receive via Channel 2
          unsigned int len;
          const uint8_t *P_buff= P.receive_from_player(player, len, 2, false);
          unsigned int pos= 0;
          for (size_t i= 0; i < (*this).num_dabits; i++)
            {
              // gf2n case
              char wire_bit= (char) P_buff[pos];
              pos++;
              Sh2[player][i].add(wire_bit);

              // gfp case
              gfp tep;
              pos+= tep.input(P_buff + pos);
              Shp[player][i].add(Shp[player][i], tep, P.get_mac_keys());
            }

          (*this).timers["Receiving input bits"].stop();
        }
    }
}

// Protocol \pi_{daBits + MPC} from https://eprint.iacr.org/2019/207
template<class SBit>
void SmallPrimeDABitGenerator<SBit>::run(daBitVector<SBit> &dabs, Player &P)
{
  vector<Share> allShp, Shp_buckets;
  vector<SBit> allSh2, Sh2_buckets;

  // Containers for each player's inputs
  vector<vector<Share>> Shp(P.nplayers());
  vector<vector<SBit>> Sh2(P.nplayers());

  provide_random_inputs(Shp, Sh2, P);

  // Permutation Array
  vector<int> perm((*this).num_dabits);
  for (unsigned int i= 0; i < (*this).num_dabits; i++)
    perm[i]= i;

  // Randomness for FY permutation
  uint8_t seed[SEED_SIZE];
  AgreeRandom(P, seed, SEED_SIZE, 2);
  (*this).FRand.SetSeedFromRandom(seed);

  // Generate a random permutation using FY shuffle
  // To select a random subset
  int unopened= (*this).num_dabits / (*this).machine.cnc_param; // (CBl / C)
  for (int i= (*this).num_dabits - 1; i > 0; --i)
    {
      int k= (int) ((*this).FRand.get_word() % i);
      swap(perm[k], perm[i]);
    }

  // Open a few to check consistency
  (*this).timers["Consistency Check"].start();
  (*this).xor_machine.consistency_check(Shp, Sh2, perm, unopened);
  (*this).timers["Consistency Check"].stop();

  int sz_after_cnc= (*this).num_dabits / (*this).machine.cnc_param * P.nplayers();
  Shp_buckets.resize(sz_after_cnc);
  Sh2_buckets.resize(sz_after_cnc);

  // XOR bits
  // First take the unopened
  for (size_t player= 0; player < P.nplayers(); player++)
    {
      for (int i= 0; i < unopened; i++)
        {
          Shp_buckets[unopened * player + i]= Shp[player][perm[i]];
          Sh2_buckets[unopened * player + i]= Sh2[player][perm[i]];
        }
    }

  // Now combine shares [b^1]...[b^n]
  // from each party into one bit share [b] by XORing them
  vector<Share> combinedp;
  vector<SBit> combined2;
  (*this).timers["Party XOR"].start();
  (*this).xor_machine.combine(combinedp, combined2, Shp_buckets, Sh2_buckets);
  (*this).timers["Party XOR"].stop();

  int N= unopened;
  // Safety check
  assert(N == (*this).machine.bucket_size * (*this).machine.nBitsPerLoop); // B * L
                                                                           // Shuffle to place bits into buckets
  for (int i= N - 1; i > 0; i--)
    {
      int k= (int) ((*this).FRand.get_word() % i);
      swap(combinedp[i], combinedp[k]);
      swap(combined2[i], combined2[k]);
    }

  (*this).timers["Correctness Check"].start();
  (*this).xor_machine.check_correctness(combinedp, combined2,
                                        (*this).machine.bucket_size, (*this).machine.nBitsPerLoop);
  (*this).timers["Correctness Check"].stop();

  P.OP->RunOpenCheck(P, "", 2);

  // Final bit output
  dabs.b2.resize((*this).machine.nBitsPerLoop);
  dabs.bp.resize((*this).machine.nBitsPerLoop);
  for (int i= 0; i < (*this).machine.nBitsPerLoop; ++i)
    {
      dabs.bp[i]= combinedp[i * (*this).machine.bucket_size];
      dabs.b2[i]= combined2[i * (*this).machine.bucket_size];
    }
  (*this).total+= (*this).machine.nBitsPerLoop;
  dabs.used= 0;
}

//Use the new technique described in 2019/1300-Fig.6
template<class SBit>
void LargePrimeDABitGenerator<SBit>::run(daBitVector<SBit> &dabs, Player &P)
{
  unsigned int pos;

  //Now retrieve random GF(p) bits
  //Wait until enough in the queue
  (*this).timers["Waiting for random bits in GF(p)"].start();
  Wait_For_Preproc(DATA_BIT, (*this).num_dabits, (*this).online_thread_num, (*this).OCD, P.whoami());
  (*this).timers["Waiting for random bits in GF(p)"].stop();

  vector<Share> random_bits_modp;
  //Lock to retrieve GF(p) random bits
  (*this).OCD.bit_mutex[(*this).online_thread_num].lock();
  Split_Lists(random_bits_modp, SacrificeD[(*this).online_thread_num].BD.bb, (*this).num_dabits);
  (*this).OCD.bit_mutex[(*this).online_thread_num].unlock();

#ifdef BENCH_OFFLINE
  P.bits+= (*this).num_dabits;
#endif

  //Find out if I will take part in the tweaking step
  unsigned int local_nb_shares= Share::SD.M.shares_per_player(P.whoami());
  vector<bool> amPart(local_nb_shares);
  vector<bool> shouldRcv(P.nplayers());
  vector<int> myRelativeNumber(local_nb_shares);

  for (unsigned int k= 0; k < local_nb_shares; k++)
    {
      if (Share::SD.RCt[P.whoami()][0][k] == 1)
        amPart[k]= true;
      else
        amPart[k]= false;
    }

  int counter= 0;
  for (unsigned int i= 0; i < P.nplayers(); i++)
    {
      for (unsigned int k= 0; k < Share::SD.M.shares_per_player(i); k++)
        {
          if (Share::SD.RCt[i][0][k] == 1)
            {
              if (i == P.whoami() and amPart[k])
                myRelativeNumber[k]= counter;
              counter++;
            }
        }
    }

  for (unsigned int j= 0; j < P.nplayers(); j++)
    {
      shouldRcv[j]= false;
      for (unsigned int k= 0; k < Share::SD.M.shares_per_player(j); k++)
        {
          if (Share::SD.RCt[j][0][k] == 1)
            shouldRcv[j]= true;
        }
    }

  //Compute h_{i,j} = floor(b_{i,j}/Delta) locally

  vector<bigint> new_bits_input((*this).num_dabits);
  vector<bigint> local_h((*this).num_dabits);
  bool haveSomethingToSend= false;
  for (unsigned int i= 0; i < (*this).num_dabits; i++)
    {
      new_bits_input[i]= 0; //.assign_zero();
      for (unsigned int k= 0; k < local_nb_shares; k++)
        {
          if (amPart[k])
            {
              bigint tmp_bigint;
              to_bigint(tmp_bigint, Share::SD.ReconS[0][myRelativeNumber[k]] * random_bits_modp[i].get_shares()[k], true);
              new_bits_input[i]+= tmp_bigint;
              haveSomethingToSend= true;
            }
        }
      new_bits_input[i]= mod(new_bits_input[i], gfp::pr());
      local_h[i]= div_f(new_bits_input[i], (*this).machine.Delta);
    }

  //Send local_h to P0
  if (P.whoami() != 0 and haveSomethingToSend)
    {
      pos= 0;
      for (unsigned int i= 0; i < (*this).num_dabits; i++)
        {
          gfp tmp_gfp;
          to_gfp(tmp_gfp, local_h[i]);
          pos+= tmp_gfp.output((*this).buff.get_buffer() + pos);
        }
      P.send_to_player(0, (*this).buff.get_buffer(), pos, 2);
    }
  if (P.whoami() == 0)
    {
      //As P0, receive all local_h from others
      for (unsigned int j= 1; j < P.nplayers(); j++)
        {
          if (shouldRcv[j])
            {
              unsigned int len;
              const uint8_t *P_buff= P.receive_from_player(j, len, 2);
              pos= 0;
              for (unsigned int i= 0; i < (*this).num_dabits; i++)
                {
                  gfp tmp_gfp;
                  pos+= tmp_gfp.input(P_buff + pos);
                  bigint tmp_bigint;
                  to_bigint(tmp_bigint, tmp_gfp, true);
                  local_h[i]= local_h[i] + tmp_bigint;
                }
            }
        }
      //Compute k = ceil((delta * \sum h)/p)
      for (unsigned int i= 0; i < (*this).num_dabits; i++)
        {
          local_h[i]= local_h[i] * (*this).machine.Delta;
          local_h[i]= div_c(local_h[i], gfp::pr());
        }

      //Compute b-k*p to re-input in GF(p) and GF(2)
      for (unsigned int i= 0; i < (*this).num_dabits; i++)
        {
          new_bits_input[i]= new_bits_input[i] - (local_h[i] * gfp::pr());
        }
    }

  //All parties re-input their new_bits_input in both GF(p) and GF(2)
  //Retrieve random masks from GF(p) world
  //Wait until enough in the queue
  vector<gfp> opened_masks_p;
  vector<vector<Share>> vec_random_masks_p(P.nplayers());

  for (unsigned int j= 0; j < P.nplayers(); j++)
    {
      if (shouldRcv[j])
        {
          (*this).timers["Waiting for bits in GF(p)"].start();
          Wait_For_Preproc(DATA_INPUT_MASK, (*this).num_dabits, (*this).online_thread_num, (*this).OCD, j);
          (*this).timers["Waiting for bits in GF(p)"].stop();

          // Lock to retrieve gf(p) masks
          (*this).OCD.OCD_mutex[(*this).online_thread_num].lock();
          Split_Lists(vec_random_masks_p[j], SacrificeD[(*this).online_thread_num].ID.ios[j], (*this).num_dabits);
          if (j == P.whoami())
            {
              Split_Lists(opened_masks_p, SacrificeD[(*this).online_thread_num].ID.opened_ios, (*this).num_dabits);
            }
#ifdef BENCH_OFFLINE
          P.inputs+= (*this).num_dabits;
#endif
          (*this).OCD.OCD_mutex[(*this).online_thread_num].unlock();
        }
    }

  //All parties locally use their opened local masks to input the sharing of their share of the bits
  //P0 locally substracted k*p before doing so
  //if (amPart)
  if (haveSomethingToSend)
    {
      for (unsigned int i= 0; i < (*this).num_dabits; i++)
        {
          if (P.whoami() != 0)
            {
              opened_masks_p[i].negate();
              for (unsigned int k= 0; k < local_nb_shares; k++)
                {
                  if (amPart[k])
                    opened_masks_p[i].add(Share::SD.ReconS[0][myRelativeNumber[k]] * random_bits_modp[i].get_shares()[k]);
                }
            }
          else
            {
              gfp tmp_gfp;
              to_gfp(tmp_gfp, new_bits_input[i]);
              opened_masks_p[i].negate();
              opened_masks_p[i].add(tmp_gfp);
            }
        }
    }

  vector<string> bcast_string(P.nplayers());
  pos= 0;
  if (haveSomethingToSend)
    {
      for (unsigned int i= 0; i < (*this).num_dabits; i++)
        {
          pos+= opened_masks_p[i].output((*this).buff.get_buffer() + pos);
        }
    }
  bcast_string[P.whoami()]= string((char *) (*this).buff.get_buffer(), pos);
  P.Broadcast_Receive(bcast_string, false, 2);

  for (unsigned int j= 0; j < P.nplayers(); j++)
    {
      if (shouldRcv[j])
        {
          pos= 0;
          for (unsigned int i= 0; i < (*this).num_dabits; i++)
            {
              gfp tmp_gfp;
              pos+= tmp_gfp.input(bcast_string[j], pos);
              vec_random_masks_p[j][i].add(vec_random_masks_p[j][i], tmp_gfp, P.get_mac_keys());
            }
        }
      bcast_string[j].clear();
    }

  //Parties locally add the resharing of each share to end up with a new resharing of the same bit
  vector<Share> final_sharing_p((*this).num_dabits);
  for (unsigned int i= 0; i < (*this).num_dabits; i++)
    {
      final_sharing_p[i]= vec_random_masks_p[0][i];
      for (unsigned int j= 1; j < P.nplayers(); j++)
        {
          if (shouldRcv[j])
            {
              final_sharing_p[i]+= vec_random_masks_p[j][i];
            }
        }
    }

  //Do the same for GF(2)
  vector<word> opened_masks_2;
  vector<vector<SBit>> vec_random_masks_2(P.nplayers());

  for (unsigned int j= 0; j < P.nplayers(); j++)
    {
      if (shouldRcv[j])
        {
          (*this).timers["get aShares"].start();
          list<SBit> random_masks_2= (*this).generate_masks((*this).num_dabits);
          (*this).timers["get aShares"].stop();
#ifdef BENCH_OFFLINE
          if (Share::SD.Etype == HSS)
            {
              P.abits+= (*this).num_dabits;
            }
          else
            {
              P.mod2s+= (*this).num_dabits;
            }
#endif
          // Now open masks in F_2 to current player
          Split_Lists(vec_random_masks_2[j], random_masks_2, (*this).num_dabits);
          P.OP2->Open_Bits_To(opened_masks_2, j, vec_random_masks_2[j], P);
        }
    }

  //All parties locally use their opened local masks to input the sharing of their share of the bits
  //P0 locally substracted k*p before doing so
  //if (amPart)
  if (haveSomethingToSend)
    {
      for (unsigned int i= 0; i < (*this).num_dabits; i++)
        {

          int input_mod_2;
          int invert_lsb_input= isOdd(new_bits_input[i]);
          if (invert_lsb_input == 0)
            {
              input_mod_2= 0;
            }
          else
            {
              input_mod_2= 1;
            }

          opened_masks_2[i]^= input_mod_2;
        }
    }

  if (haveSomethingToSend)
    {
      for (unsigned int i= 0; i < (*this).num_dabits; i++)
        {
          (*this).buff.get_buffer()[i]= (uint8_t) opened_masks_2[i];
        }
    }
  bcast_string[P.whoami()]= string((char *) (*this).buff.get_buffer(), (*this).num_dabits);
  P.Broadcast_Receive(bcast_string, false, 2);

  for (unsigned int j= 0; j < P.nplayers(); j++)
    {
      if (shouldRcv[j])
        {
          for (unsigned int i= 0; i < (*this).num_dabits; i++)
            {
              char open_bit= bcast_string[j].c_str()[i];
              vec_random_masks_2[j][i].add(open_bit);
            }
        }
    }

  //Parties locally add the resharing of each share to end up with a new resharing of the same bit
  vector<SBit> final_sharing_2((*this).num_dabits);
  for (unsigned int i= 0; i < (*this).num_dabits; i++)
    {
      final_sharing_2[i]= vec_random_masks_2[0][i];
      for (unsigned int j= 1; j < P.nplayers(); j++)
        {
          if (shouldRcv[j])
            {
              final_sharing_2[i].add(vec_random_masks_2[j][i]);
            }
        }
    }

  // Randomness for checking sec LSBs
  uint8_t seed[SEED_SIZE];
  AgreeRandom(P, seed, SEED_SIZE, 2);
  (*this).FRand.SetSeedFromRandom(seed);

  //Check against malicious
  vector<Share> S_p((*this).machine.gamma);
  vector<SBit> S_2((*this).machine.gamma);
  bigint Bnd= bigint(1) << (*this).machine.sec;
  for (unsigned int k= 0; k < (*this).machine.gamma; k++)
    {
      //Generate num_dabits random coefficients
      bigint r;
      vector<gfp> r_p((*this).num_dabits);
      vector<int> r_2((*this).num_dabits);
      for (unsigned int i= 0; i < (*this).num_dabits; i++)
        {
          r= randomBnd((*this).FRand, Bnd);
          r_p[i].assign(r);
          r_2[i]= isOdd(r);
        }

      //Take the linear combination in GF(p)
      S_p[k]= final_sharing_p[0];
      S_p[k]*= r_p[0];
      for (unsigned int i= 1; i < (*this).num_dabits; i++)
        {
          S_p[k]+= (final_sharing_p[i] * r_p[i]);
        }

      S_2[k].assign_zero(P.whoami());
      for (unsigned int i= 0; i < (*this).num_dabits; i++)
        {
          if (r_2[i] == 1)
            {
              S_2[k].add(final_sharing_2[i]);
            }
        }
    }

  //Open the linear combinations
  vector<word> result_2;
  vector<gfp> result_p;
  P.OP->Open_To_All_Begin(result_p, S_p, P, 2);
  P.OP->Open_To_All_End(result_p, S_p, P, 2);

  P.OP2->Open_Bits(result_2, S_2, P);
  //Check equality

  for (unsigned int k= 0; k < (*this).machine.gamma; k++)
    {
      gfp one_gfp;
      one_gfp.assign_one();
      bool x= (result_p[k] & one_gfp).is_one();
      bool y= result_2[k];
      if (x != y)
        {
          throw Sacrifice_Check_Error("newDabit correctness error");
        }
    }

  //Final bit output, discard first gamma*sec bits
  dabs.b2.resize((*this).machine.nBitsPerLoop);
  dabs.bp.resize((*this).machine.nBitsPerLoop);
  for (int i= 0; i < (*this).machine.nBitsPerLoop; i++)
    {
      dabs.bp[i]= final_sharing_p[(*this).machine.gamma * (*this).machine.sec + i];
      dabs.b2[i]= final_sharing_2[(*this).machine.gamma * (*this).machine.sec + i];
    }
  dabs.used= 0;
  (*this).total+= (*this).machine.nBitsPerLoop;
}

template<class SBit>
void daBitVector<SBit>::get_daBit(Share &bpr, SBit &b2r, AbstractDABitGenerator<SBit> &daBitGen, Player &P)
{
  OTD.check();
  if (used >= bp.size())
    {
      daBitGen.run(*this, P);
    }
  bpr= bp[used];
  b2r= b2[used];
  used++;

#ifdef BENCH_OFFLINE
  P.dabits++;
#endif
}

template<class SBit>
void daBitVector<SBit>::get_daBits(vector<Share> &bpr, vector<SBit> &b2r,
                                   AbstractDABitGenerator<SBit> &daBitGen, Player &P)
{
  OTD.check();
  if (bpr.size() != b2r.size())
    {
      throw bad_value();
    }
  for (unsigned int i= 0; i < bpr.size(); i++)
    {
      get_daBit(bpr[i], b2r[i], daBitGen, P);
    }
}

template class daBitVector<aBit>;
template class daBitVector<Share2>;

template class SmallPrimeDABitGenerator<aBit>;
template class SmallPrimeDABitGenerator<Share2>;

template class LargePrimeDABitGenerator<aBit>;
template class LargePrimeDABitGenerator<Share2>;
