/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2019, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "DABitGenerator.h"
#include "DABitMachine.h"
#include "LSSS/PRSS.cpp"
#include "OT/OT_Thread_Data.h"
#include "Processor/Processor_IO.h"
#include "Tools/util_containers.h"

#include "unistd.h"
#include <cassert>

extern vector<sacrificed_data> SacrificeD;
extern OT_Thread_Data OTD;

DABitGenerator::DABitGenerator(MaliciousDABitMachine &machine,
                               Player &P, int thread_num, offline_control_data &OCDm)
    : thread_num(thread_num), P(P),
      machine(machine), OCD(OCDm),
      xor_machine(P, OCD, thread_num)
{
  G.ReSeed(thread_num);
  // Randomness for FY permutation
  uint8_t seed[SEED_SIZE];
  AgreeRandom(P, seed, SEED_SIZE);
  FRand.SetSeedFromRandom(seed);
}

// adjusting the MACs and shares for the input bits
void DABitGenerator::adjust_and_pack(stringstream &ss,
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
  OCD.sacrifice_mutex[thread_num].lock();
  take_first_to_vector(vec_random_masks_p, SacrificeD[thread_num].ID.ios[player], num_bits);
  if (player == P.whoami())
    take_first_to_vector(opened_masks_p, SacrificeD[thread_num].ID.opened_ios, num_bits);
  OCD.sacrifice_mutex[thread_num].unlock();
  timers["Retrieving input masks"].stop();

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

void DABitGenerator::provide_random_inputs(vector<vector<Share>> &Shp, vector<vector<aBit>> &Sh2, size_t num_bits)
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
void DABitGenerator::run(daBitVector &dabs)
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
  dabs.used= 0;
}

size_t DABitGenerator::report_sent()
{
  // TODO: Wait until we merge this with benchmarking branch
  return 0;
}

void daBitVector::get_daBit(Share &bpr, aBit &b2r, DABitGenerator &daBitGen)
{
  if (used >= bp.size())
    {
      daBitGen.run(*this);
    }
  bpr= bp[used];
  b2r= b2[used];
  used++;
}

void daBitVector::get_daBits(vector<Share> &bpr, vector<aBit> &b2r,
                             DABitGenerator &daBitGen)
{
  if (bpr.size() != b2r.size())
    {
      throw bad_value();
    }
  for (unsigned int i= 0; i < bpr.size(); i++)
    {
      get_daBit(bpr[i], b2r[i], daBitGen);
    }
}
