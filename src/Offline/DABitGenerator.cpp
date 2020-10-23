/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

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

SmallPrimeDABitGenerator::SmallPrimeDABitGenerator(MaliciousDABitMachine &machine,
                                                   Player &P, int thread_num)
    : AbstractDABitGenerator(thread_num, P),
      machine(machine), OCD(*machine.OCD), xor_machine(P, OCD, thread_num)
{
  G.ReSeed(thread_num);
  // Randomness for FY permutation
  uint8_t seed[SEED_SIZE];
  AgreeRandom(P, seed, SEED_SIZE, 2);
  FRand.SetSeedFromRandom(seed);
}

// adjusting the MACs and shares for the input bits
void SmallPrimeDABitGenerator::adjust_and_pack(stringstream &ss,
                                               vector<Share> &Shp, vector<aBit> &Sh2,
                                               unsigned int player, vector<bool> bits)
{
  size_t num_bits= bits.size();
  // Assume we take aBit shares from Garbling
  //
  timers["Processing inputs bits in gf(2)"].start();
  // Seems that get_aShares is the costly bit here
  timers["get aShares"].start();
  list<aBit> random_masks_2= OTD.aBD.get_aShares(xor_machine.get_thread(), num_bits);
  timers["get aShares"].stop();
  vector<aBit> vec_random_masks_2;
  #ifdef BENCH_OFFLINE
     P.abits+=num_bits;
  #endif

  // dump the list into vector
  vec_random_masks_2.reserve(num_bits);
  copy(begin(random_masks_2), end(random_masks_2), back_inserter(vec_random_masks_2));
  random_masks_2.clear();

  // Now open masks in F_2 to current player
  vector<int> opened_masks_2;

  Open_aBits_To(opened_masks_2, player, vec_random_masks_2, P);

  timers["Processing inputs bits in gf(2)"].stop();

  // Now retrieve random masks from gf(p) world
  // Wait until enough in the queue
  timers["Waiting for bits in GF(p)"].start();
  Wait_For_Preproc(DATA_INPUT_MASK, num_bits, thread_num, OCD, player);
  timers["Waiting for bits in GF(p)"].stop();

  timers["Retrieving input masks"].start();
  vector<gfp> opened_masks_p;
  vector<Share> vec_random_masks_p;

  // Lock to retrieve gf(p) masks
  OCD.OCD_mutex[thread_num].lock();
  Split_Lists(vec_random_masks_p, SacrificeD[thread_num].ID.ios[player], num_bits);
  if (player == P.whoami())
    Split_Lists(opened_masks_p, SacrificeD[thread_num].ID.opened_ios, num_bits);
  OCD.OCD_mutex[thread_num].unlock();
  timers["Retrieving input masks"].stop();

  #ifdef BENCH_OFFLINE
    P.inputs+=num_bits;
  #endif

  timers["Serializing bits"].start();
  if (player == P.whoami())
    {
      for (size_t i= 0; i < num_bits; i++)
        {
          // Deal with gf(2) case
          int wire_bit= bits[i];
          wire_bit^= opened_masks_2[i];
          ss << (char) wire_bit;
          aBit epsilon_2= vec_random_masks_2[i];
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
          epsilon_p.output(ss, false);
          // place mask + epsilon into Shp
          Shp[i].add(vec_random_masks_p[i], epsilon_p, P.get_mac_keys());
        }
    }
  else
    {
      for (size_t i= 0; i < num_bits; i++)
        {
          Sh2[i]= vec_random_masks_2[i];
          Shp[i]= vec_random_masks_p[i];
        }
    }

  timers["Serializing bits"].stop();
}

void SmallPrimeDABitGenerator::provide_random_inputs(vector<vector<Share>> &Shp, vector<vector<aBit>> &Sh2, size_t num_bits)
{

  vector<bool> inp_bits(num_bits);
  // Get random bits based on unsigned char blocks
  // This assumes that 1 byte = 8 bits
  size_t chunk_size= sizeof(unsigned char) * 8;
  size_t num_chunks= num_bits / chunk_size + 1;
  timers["PRG bit gen"].start();
  for (size_t i= 0; i < num_chunks; ++i)
    {
      unsigned char block= G.get_uchar();
      for (size_t j= 0; j < chunk_size; j++)
        {
          if (i * chunk_size + j < num_bits)
            inp_bits[i * chunk_size + j]= (block >> j) & 1;
        }
    }

  timers["PRG bit gen"].stop();

  // Now each party inputs their random bits
  stringstream ss;
  for (unsigned int player= 0; player < P.nplayers(); ++player)
    {
      Shp[player].resize(num_bits);
      Sh2[player].resize(num_bits);

      adjust_and_pack(ss, Shp[player], Sh2[player], player, inp_bits);
    }
  for (unsigned int player= 0; player < P.nplayers(); player++)
    {
      if (player == P.whoami())
        {
          timers["Sending input bits"].start();
          P.send_all(ss.str(), 2, false);
          timers["Sending input bits"].stop();
        }
      else
        {
          timers["Receiving input bits"].start();
          // Receive via Channel 1
          string ss;
          P.receive_from_player(player, ss, 2, false);
          stringstream is(ss);
          gf2n te2;
          for (size_t i= 0; i < num_bits; i++)
            {
              // gf2n case
              char wire_bit;
              is >> wire_bit;
              if (wire_bit)
                te2.assign_one();
              else
                te2.assign_zero();
              Sh2[player][i].add(wire_bit);

              // gfp case
              gfp tep;
              tep.input(is, false);
              Shp[player][i].add(Shp[player][i], tep, P.get_mac_keys());
            }

          timers["Receiving input bits"].stop();
        }
    }
}

// Protocol \pi_{daBits + MPC} from https://eprint.iacr.org/2019/207
void SmallPrimeDABitGenerator::run(daBitVector &dabs)
{
  vector<Share> allShp, Shp_buckets;
  vector<aBit> allSh2, Sh2_buckets;

  int num_allBits= machine.cnc_param * machine.bucket_size * machine.nBitsPerLoop;

  // Containers for each player's inputs
  vector<vector<Share>> Shp(P.nplayers());
  vector<vector<aBit>> Sh2(P.nplayers());

  provide_random_inputs(Shp, Sh2, num_allBits);

  // Permutation Array
  vector<int> perm(num_allBits);
  for (int i= 0; i < num_allBits; i++)
    perm[i]= i;

  // Generate a random permutation using FY shuffle
  // To select a random subset
  int unopened= num_allBits / machine.cnc_param; // (CBl / C)

  for (int i= num_allBits - 1; i > 0; --i)
    {
      int k= (int) (FRand.get_word() % i);
      swap(perm[k], perm[i]);
    }

  // Open a few to check consistency
  timers["Consistency Check"].start();
  xor_machine.consistency_check(Shp, Sh2, perm, unopened, OP);
  timers["Consistency Check"].stop();

  int sz_after_cnc= num_allBits / machine.cnc_param * P.nplayers();
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
  vector<aBit> combined2;
  timers["Party XOR"].start();
  xor_machine.combine(combinedp, combined2, Shp_buckets, Sh2_buckets, OP);
  timers["Party XOR"].stop();

  int N= unopened;
  // Safety check
  assert(N == machine.bucket_size * machine.nBitsPerLoop); // B * L
                                                           // Shuffle to place bits into buckets
  for (int i= N - 1; i > 0; i--)
    {
      int k= (int) (FRand.get_word() % i);
      swap(combinedp[i], combinedp[k]);
      swap(combined2[i], combined2[k]);
    }

  timers["Correctness Check"].start();
  xor_machine.check_correctness(combinedp, combined2,
                                machine.bucket_size, machine.nBitsPerLoop, OP);
  timers["Correctness Check"].stop();

  OP.RunOpenCheck(P, "", 2);

  // Final bit output
  dabs.b2.resize(machine.nBitsPerLoop);
  dabs.bp.resize(machine.nBitsPerLoop);
  for (int i= 0; i < machine.nBitsPerLoop; ++i)
    {
      dabs.bp[i]= combinedp[i * machine.bucket_size];
      dabs.b2[i]= combined2[i * machine.bucket_size];
    }
  total+= machine.nBitsPerLoop;
  dabs.used= 0;
}

LargePrimeDABitGenerator::LargePrimeDABitGenerator(MaliciousDABitMachine &machine,
                                                   Player &P, int thread_num)
    : AbstractDABitGenerator(thread_num, P),
      machine(machine), OCD(*machine.OCD), xor_machine(P, OCD, thread_num)
{
  // Randomness for checking sec LSBs
  uint8_t seed[SEED_SIZE];
  AgreeRandom(P, seed, SEED_SIZE, 2);
  FRand.SetSeedFromRandom(seed);
}

//Use the new technique described in 2019/1300-Fig.6
void LargePrimeDABitGenerator::run(daBitVector &dabs)
{
  //We assume that p is large enough (more than 54 bits long
  //for nBitsPerLoop = 2**13 and sec = 40)
  size_t num_dabits= machine.nBitsPerLoop + (machine.gamma * machine.sec);

  //Now retrieve random GF(p) bits
  //Wait until enough in the queue
  timers["Waiting for random bits in GF(p)"].start();
  Wait_For_Preproc(DATA_BIT, num_dabits, thread_num, OCD, P.whoami());
  timers["Waiting for random bits in GF(p)"].stop();

  vector<Share> random_bits_modp;
  //Lock to retrieve GF(p) random bits
  OCD.bit_mutex[thread_num].lock();
  Split_Lists(random_bits_modp, SacrificeD[thread_num].BD.bb, num_dabits);
  OCD.bit_mutex[thread_num].unlock();

  #ifdef BENCH_OFFLINE
    P.bits+=num_dabits;
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

  vector<bigint> new_bits_input(num_dabits);
  vector<bigint> local_h(num_dabits);
  bool haveSomethingToSend= false;
  for (unsigned int i= 0; i < num_dabits; i++)
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
      local_h[i]= div_f(new_bits_input[i], machine.Delta);
    }

  //Send local_h to P0
  if (P.whoami() != 0 and haveSomethingToSend)
    {
      ostringstream os;
      for (unsigned int i= 0; i < num_dabits; i++)
        {
          gfp tmp_gfp;
          to_gfp(tmp_gfp, local_h[i]);
          tmp_gfp.output(os, false);
        }
      P.send_to_player(0, os.str(), 2);
    }
  if (P.whoami() == 0)
    {
      //As P0, receive all local_h from others
      for (unsigned int j= 1; j < P.nplayers(); j++)
        {
          if (shouldRcv[j])
            {
              string ss;
              P.receive_from_player(j, ss, 2);
              istringstream is(ss);
              for (unsigned int i= 0; i < num_dabits; i++)
                {
                  gfp tmp_gfp;
                  tmp_gfp.input(is, false);
                  bigint tmp_bigint;
                  to_bigint(tmp_bigint, tmp_gfp, true);
                  local_h[i]= local_h[i] + tmp_bigint;
                }
            }
        }
      //Compute k = ceil((delta * \sum h)/p)
      for (unsigned int i= 0; i < num_dabits; i++)
        {
          local_h[i]= local_h[i] * machine.Delta;
          local_h[i]= div_c(local_h[i], gfp::pr());
        }

      //Compute b-k*p to re-input in GF(p) and GF(2)
      for (unsigned int i= 0; i < num_dabits; i++)
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
          timers["Waiting for bits in GF(p)"].start();
          Wait_For_Preproc(DATA_INPUT_MASK, num_dabits, thread_num, OCD, j);
          timers["Waiting for bits in GF(p)"].stop();

          // Lock to retrieve gf(p) masks
          OCD.OCD_mutex[thread_num].lock();
          Split_Lists(vec_random_masks_p[j], SacrificeD[thread_num].ID.ios[j], num_dabits);
          if (j == P.whoami())
            {
              Split_Lists(opened_masks_p, SacrificeD[thread_num].ID.opened_ios, num_dabits);
            }
	  #ifdef BENCH_OFFLINE
             P.inputs+=num_dabits;
          #endif
          OCD.OCD_mutex[thread_num].unlock();
        }
    }

  //All parties locally use their opened local masks to input the sharing of their share of the bits
  //P0 locally substracted k*p before doing so
  //if (amPart)
  if (haveSomethingToSend)
    {
      for (unsigned int i= 0; i < num_dabits; i++)
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
  ostringstream os;
  if (haveSomethingToSend)
    {
      for (unsigned int i= 0; i < num_dabits; i++)
        {
          opened_masks_p[i].output(os, false);
        }
    }
  bcast_string[P.whoami()]= os.str();
  P.Broadcast_Receive(bcast_string, false, 2);

  for (unsigned int j= 0; j < P.nplayers(); j++)
    {
      if (shouldRcv[j])
        {
          istringstream is(bcast_string[j]);
          for (unsigned int i= 0; i < num_dabits; i++)
            {
              gfp tmp_gfp;
              tmp_gfp.input(is, false);
              vec_random_masks_p[j][i].add(vec_random_masks_p[j][i], tmp_gfp, P.get_mac_keys());
            }
        }
      bcast_string[j].clear();
    }

  //Parties locally add the resharing of each share to end up with a new resharing of the same bit
  vector<Share> final_sharing_p(num_dabits);
  for (unsigned int i= 0; i < num_dabits; i++)
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
  vector<int> opened_masks_2;
  vector<vector<aBit>> vec_random_masks_2(P.nplayers());

  for (unsigned int j= 0; j < P.nplayers(); j++)
    {
      if (shouldRcv[j])
        {
          timers["get aShares"].start();
          list<aBit> random_masks_2= OTD.aBD.get_aShares(thread_num, num_dabits);
          timers["get aShares"].stop();
          #ifdef BENCH_OFFLINE
	  	P.abits+=num_dabits;
          #endif
          // Now open masks in F_2 to current player
          Split_Lists(vec_random_masks_2[j], random_masks_2, num_dabits);
          Open_aBits_To(opened_masks_2, j, vec_random_masks_2[j], P);
        }
    }

  //All parties locally use their opened local masks to input the sharing of their share of the bits
  //P0 locally substracted k*p before doing so
  //if (amPart)
  if (haveSomethingToSend)
    {
      for (unsigned int i= 0; i < num_dabits; i++)
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

  //vector<string> bcast_string(P.nplayers());
  //ostringstream os;
  os.str("");
  os.clear();
  //if (amPart)
  if (haveSomethingToSend)
    {
      for (unsigned int i= 0; i < num_dabits; i++)
        {
          os << (char) opened_masks_2[i];
        }
    }
  bcast_string[P.whoami()]= os.str();
  P.Broadcast_Receive(bcast_string, false, 2);

  for (unsigned int j= 0; j < P.nplayers(); j++)
    {
      if (shouldRcv[j])
        {
          istringstream is(bcast_string[j]);
          for (unsigned int i= 0; i < num_dabits; i++)
            {
              char open_bit;
              is >> open_bit;
              vec_random_masks_2[j][i].add(open_bit);
            }
        }
    }

  //Parties locally add the resharing of each share to end up with a new resharing of the same bit
  vector<aBit> final_sharing_2(num_dabits);
  for (unsigned int i= 0; i < num_dabits; i++)
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

  //Check against malicious
  vector<Share> S_p(machine.gamma);
  vector<aBit> S_2(machine.gamma);
  for (unsigned int k= 0; k < machine.gamma; k++)
    {
      //Generate num_dabits random coefficients
      vector<BitVector> r(num_dabits);
      for (unsigned int i= 0; i < num_dabits; i++)
        {
          r[i].resize(machine.sec);
          r[i].randomize(FRand);
        }

      vector<ostringstream> os(num_dabits);
      for (unsigned int i= 0; i < num_dabits; i++)
        {
          for (unsigned int l= 0; l < r[0].size_bytes(); l++)
            {
              os[i] << int((r[i].get_ptr())[l]);
            }
          os[i] << endl;
        }

      //Take the linear combination in GF(p)
      vector<gfp> r_p(num_dabits);

      for (unsigned int i= 0; i < num_dabits; i++)
        {
          istringstream is(os[i].str());
          r_p[i].input(is, true);
        }
      S_p[k]= final_sharing_p[0];
      S_p[k]*= r_p[0];
      for (unsigned int i= 1; i < num_dabits; i++)
        {
          S_p[k]+= (final_sharing_p[i] * r_p[i]);
        }

      //Take the linear combinator in GF(2)
      vector<gf2n> r_2(num_dabits);
      gf2n gf2n_one;
      gf2n_one.assign_one();
      for (unsigned int i= 0; i < num_dabits; i++)
        {
          istringstream is(os[i].str());
          r_2[i].input(is, true);
          r_2[i]= r_2[i] & gf2n_one;
        }
      S_2[k].mult_by(r_2[0], final_sharing_2[0]);
      for (unsigned int i= 1; i < num_dabits; i++)
        {
          aBit tmp_res;
          tmp_res.mult_by(r_2[i], final_sharing_2[i]);
          S_2[k].add(tmp_res);
        }
    }

  //Open the linear combinations
  vector<int> result_2;
  vector<gfp> result_p;
  OP.Open_To_All_Begin(result_p, S_p, P, 2);
  OP.Open_To_All_End(result_p, S_p, P, 2);
  Open_aBits(result_2, S_2, P);
  //Check equality

  for (unsigned int k= 0; k < machine.gamma; k++)
    {
      gfp one_gfp;
      one_gfp.assign_one();
      bool x= (result_p[k] & one_gfp).is_one();
      bool y= result_2[k];
      if (x != y)
        throw Sacrifice_Check_Error("newDabit correctness error");
    }

  //Final bit output, discard first gamma*sec bits
  dabs.b2.resize(machine.nBitsPerLoop);
  dabs.bp.resize(machine.nBitsPerLoop);
  for (int i= 0; i < machine.nBitsPerLoop; i++)
    {
      dabs.bp[i]= final_sharing_p[machine.gamma * machine.sec + i];
      dabs.b2[i]= final_sharing_2[machine.gamma * machine.sec + i];
    }
  dabs.used= 0;
  total+= machine.nBitsPerLoop;
}

void daBitVector::get_daBit(Share &bpr, aBit &b2r, AbstractDABitGenerator &daBitGen)
{
  OTD.check();
  if (used >= bp.size())
    {
      daBitGen.run(*this);
    }
  bpr= bp[used];
  b2r= b2[used];
  used++;

  #ifdef BENCH_OFFLINE
    daBitGen.P.dabits++;
  #endif
}

void daBitVector::get_daBits(vector<Share> &bpr, vector<aBit> &b2r,
                             AbstractDABitGenerator &daBitGen)
{
  OTD.check();
  if (bpr.size() != b2r.size())
    {
      throw bad_value();
    }
  for (unsigned int i= 0; i < bpr.size(); i++)
    {
      get_daBit(bpr[i], b2r[i], daBitGen);
    }
}
