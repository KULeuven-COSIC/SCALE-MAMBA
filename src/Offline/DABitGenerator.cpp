/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2019, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "DABitGenerator.h"
#include "DABitMachine.h"
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
  AgreeRandom(P, seed, SEED_SIZE);
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
  take_first_to_vector(vec_random_masks_p, SacrificeD[thread_num].ID.ios[player], num_bits);
  if (player == P.whoami())
    take_first_to_vector(opened_masks_p, SacrificeD[thread_num].ID.opened_ios, num_bits);
  OCD.OCD_mutex[thread_num].unlock();
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
  AgreeRandom(P, seed, SEED_SIZE);
  FRand.SetSeedFromRandom(seed);
}

void LargePrimeDABitGenerator::split_shares(vector<gfp> &out_modp_shares,
                                            const vector<Share> &modp_bits)
{

  // TODO: needs fixing if more values per share
  int m= machine.nBitsPerLoop + machine.sec;
  int n= (P.nplayers() + 1) / 2;

  vector<stringstream> to_send(P.nplayers());
  int offset= P.whoami() % 2;

  // Bits are arranges as following
  // [b_1, ..., b_n, b_{n+1}, ..., b_{2*n}...
  // current party will send its shares to its group P_{2*i+offset}
  // Eg:
  // P0 should get all shares of b_1^j where j is even,
  // P1 should get all shares of b_1^j where j is odd
  // P2 should get all shares of b_2^j where j is even
  // P3 should get all shares of b_2^j where j is odd.. etc:
  // where offset = {0,1}, depending on the group parity
  int last_player= P.nplayers() - 1;
  for (int k= 0; k < m; k++)
    for (int i= 0; i < n; i++)
      {
        // when odd number of players then 2*i+offset is too large
        // need to place this if to avoid seg faults
        if (2 * i + offset < (int) P.nplayers())
          modp_bits[k * n + i].get_share(0).output(to_send[2 * i + offset], false);
        else
          modp_bits[k * n + i].get_share(0).output(to_send[last_player], false);
      }
  // see whether P is in the even / odd group
  // adjust the offset accordingly
  vector<int> parity_group;
  for (size_t i= offset; i < P.nplayers(); i+= 2)
    if (i != P.whoami())
      parity_group.push_back(i);

  size_t num_ingroup= parity_group.size();

  // add in the os buffer solution
  vector<string> os(P.nplayers());
  for (size_t i= 0; i < num_ingroup; i++)
    {
      size_t other_player= parity_group[i];
      if (other_player > P.whoami())
        {
          P.send_to_player(other_player, to_send[other_player].str());
        }
      else if (other_player < P.whoami())
        {
          P.receive_from_player(other_player, os[other_player]);
        }
    }

  // now do it reverse
  for (size_t i= 0; i < num_ingroup; i++)
    {
      size_t other_player= parity_group[i];
      if (other_player > P.whoami())
        {
          P.receive_from_player(other_player, os[other_player]);
        }
      else if (other_player < P.whoami())
        {
          P.send_to_player(other_player, to_send[other_player].str());
        }
    }

  // if odd nr of players then parties who belong to different parity group
  // send their bit share to the last party
  if (P.nplayers() % 2)
    {
      if (P.whoami() % 2)
        P.send_to_player(last_player, to_send[last_player].str());
      if ((int) P.whoami() == last_player)
        {
          for (size_t i= 1; i < P.nplayers(); i+= 2)
            {
              P.receive_from_player(i, os[i]);
              // add outsider player to same collecting group
              parity_group.push_back(i);
              // this is for the last loop to pickup and sum the received
              // shares from all parties
            }
        }
    }

  out_modp_shares.resize(m * n);
  // copy own shares
  for (int i= 0; i < m; i++)
    for (int k= 0; k < n; k++)
      out_modp_shares[i * n + k]= modp_bits[i * n + k].get_share(0);

  int own_index= P.whoami() / 2;
  // Parse the GF(p) shares and add them locally
  for (int other_player : parity_group)
    {
      stringstream is(os[other_player]);
      gfp tmp;
      for (int j= 0; j < m; j++)
        {
          tmp.input(is, false);
          out_modp_shares[j * n + own_index]+= tmp;
        }
    }
}

// converts the modp bits vector to a matrix of shares to do a row wise gfp xor
void LargePrimeDABitGenerator::prepare_for_xor(vector<vector<Share>> &bit_rows, const vector<Share> &modp_bits)
{

  int n= (P.nplayers() + 1) / 2;
  int m= machine.nBitsPerLoop + machine.sec;

  bit_rows.clear();
  for (int i= 0; i < m; i++)
    {
      vector<Share> tmp;
      for (int j= 0; j < n; ++j)
        tmp.push_back(modp_bits[i * n + j]);
      bit_rows.push_back(tmp);
    }
}

void LargePrimeDABitGenerator::input_GF2_bits(vector<aBit> &Sh2, const vector<bool> &bits)
{

  vector<vector<aBit>> allRandomMasks(P.nplayers());
  vector<vector<int>> allOpenedMasks(P.nplayers());

  int num_dabits= machine.nBitsPerLoop + machine.sec;

  // for n = 2 players, then everyone has to enter all their shares
  // n > 2 then things change slightly
  vector<bool> compressed_bits(num_dabits);

  int extra_bits= (P.nplayers() + 1) / 2;
  int own_bit_index= P.whoami() / 2;
  for (int i= 0; i < num_dabits; i++)
    compressed_bits[i]= bits[i * extra_bits + own_bit_index];

  for (size_t pnum= 0; pnum < P.nplayers(); pnum++)
    {
      timers["get aShares"].start();
      list<aBit> random_masks_2= OTD.aBD.get_aShares(thread_num, num_dabits);
      timers["get aShares"].stop();
      // Now open masks in F_2 to current player
      take_first_to_vector(allRandomMasks[pnum], random_masks_2, num_dabits);
      Open_aBits_To(allOpenedMasks[pnum], pnum, allRandomMasks[pnum], P);
    }

  Sh2.resize(num_dabits);
  //adjust shares mod2
  size_t my_num= P.whoami();
  // pack signal bits
  stringstream ss;
  for (int i= 0; i < num_dabits; i++)
    {
      int wire_bit= compressed_bits[i];
      wire_bit^= allOpenedMasks[my_num][i];
      ss << (char) wire_bit;
      aBit epsilon_2= allRandomMasks[my_num][i];
      epsilon_2.add(wire_bit);
      Sh2[i]= epsilon_2;
    }

  // sum all signal bits from other players
  vector<string> os(P.nplayers());
  os[P.whoami()]= ss.str();
  P.Broadcast_Receive(os);
  for (size_t pnum= 0; pnum < P.nplayers(); pnum++)
    {
      if (pnum != P.whoami())
        {
          istringstream is(os[pnum]);
          gf2n te2;
          for (int i= 0; i < num_dabits; i++)
            {
              char wire_bit;
              is >> wire_bit;
              Sh2[i].add(wire_bit);
              // Need to locally add the rest of the sharings for each party
              Sh2[i].add(allRandomMasks[pnum][i]);
            }
        }
    }
}

void LargePrimeDABitGenerator::check_public_lsbs(vector<Share> modp_bits, vector<aBit> mod2_bits)
{
  int sec= machine.sec;

  Share sum_p(P.whoami());
  aBit sum_2;

  // use first sec bits to perform the checks
  for (size_t i= sec + 1; i < modp_bits.size(); i++)
    {
      sum_p.add(modp_bits[i]);
      sum_2.add(mod2_bits[i]);
    }

  int log_nBits= 1;
  while ((1LL << log_nBits) <= machine.nBitsPerLoop)
    ++log_nBits;

  // Now need to retrieve sec ^ 2 * log(nBitsPerLoop) random bits modp
  int num_bits_needed= sec * sec * log_nBits;
  timers["Waiting for random bits in GF(p)"].start();
  Wait_For_Preproc(DATA_BIT, num_bits_needed, thread_num, OCD, P.whoami());
  timers["Waiting for random bits in GF(p)"].stop();

  list<Share> prep_bits;
  // Lock to retrieve GF(p) random bits
  OCD.bit_mutex[thread_num].lock();
  auto &sacrificed_bits= SacrificeD[thread_num].BD.bb;
  prep_bits.splice(prep_bits.begin(), sacrificed_bits,
                   sacrificed_bits.begin(), next(sacrificed_bits.begin(), num_bits_needed));
  OCD.bit_mutex[thread_num].unlock();

  // Now compute private lsbs
  vector<Share> lsb_modp(sec, Share(P.whoami()));
  vector<aBit> lsb_mod2(sec);

  // add in a random mask [2*r] of sec + log_nbits bitlength
  auto it_prep_bit= prep_bits.begin();
  vector<gfp> power_of_two(sec + log_nBits + 1, 1);
  for (int i = 1; i <= sec + log_nBits; i++)
    power_of_two[i] = power_of_two[i - 1] * 2;

  for (int i= 0; i < sec; i++)
    {
      for (int j= 0; j < sec + log_nBits; j++)
        {
          // Need to preserve parity of the opened lsb mask
          // so we keep the lsb unmasked by offsetting bit operations by 1
          lsb_modp[i]+= (*it_prep_bit * power_of_two[j + 1]);
          it_prep_bit++;
        }
    }
  // add in extra bit which will be discarded
  for (int i= 0; i < sec; i++)
    {
      lsb_modp[i]+= modp_bits[i];
      lsb_mod2[i]+= mod2_bits[i];
    }
  // Now open and compare lsb
  vector<gfp> opened_modp;
  OP.Open_To_All_Begin(opened_modp, lsb_modp, P, 2);
  OP.Open_To_All_End(opened_modp, lsb_modp, P, 2);

  vector<int> opened_mod2;
  Open_aBits(opened_mod2, lsb_mod2, P);
  for (int i= 0; i < sec; ++i)
    {
      bool x= (opened_modp[i] & 1).is_one();
      bool y= opened_mod2[i];
      if (x != y)
        throw Sacrifice_Check_Error("daBit correctness error");
    }
  OP.RunOpenCheck(P, "", 2);
}
void LargePrimeDABitGenerator::run(daBitVector &dabs)
{
  size_t num_dabits= machine.nBitsPerLoop + machine.sec;
  // Now retrieve random GF(p) bits
  // Wait until enough in the queue
  timers["Waiting for random bits in GF(p)"].start();
  Wait_For_Preproc(DATA_BIT, num_dabits, thread_num, OCD, P.whoami());
  timers["Waiting for random bits in GF(p)"].stop();

  int extra_gfp_xors= int(P.nplayers() + 1) / 2 - 1;
  int m= num_dabits * (extra_gfp_xors + 1);

  vector<Share> random_bits_modp;
  // Lock to retrieve GF(p) random bits
  OCD.bit_mutex[thread_num].lock();
  take_first_to_vector(random_bits_modp, SacrificeD[thread_num].BD.bb, m);
  OCD.bit_mutex[thread_num].unlock();

  // if n-party case then simplify to 2-party case
  vector<gfp> half_shares;
  if (P.nplayers() > 2)
    {
      this->split_shares(half_shares, random_bits_modp);
      vector<vector<Share>> row_arranged;
      this->prepare_for_xor(row_arranged, random_bits_modp);
      this->xor_machine.party_log_xor(random_bits_modp, row_arranged, this->OP);
    }
  else
    {
      for (auto &rbit : random_bits_modp)
        half_shares.push_back(rbit.get_share(0));
    }

  vector<bool> mod2_half_shares(half_shares.size());
  for (size_t i= 0; i < mod2_half_shares.size(); i++)
    mod2_half_shares[i]= (half_shares[i] & 1).is_one();

  // Input shares to GC
  vector<aBit> mod2_shares(num_dabits);
  this->input_GF2_bits(mod2_shares, mod2_half_shares);

  // Offset bit shares by 1
  // Only tested for N <= 3 parties
  aBit x;
  if ((P.nplayers() / 2) % 2)
    x.assign_one();
  else
    x.assign_zero();
  for (size_t i= 0; i < mod2_shares.size(); i++)
    mod2_shares[i]+= x;

  // Now do sec checks and compare public LSBs
  this->check_public_lsbs(random_bits_modp, mod2_shares);

  // Final bit output, discard first sec bits which were used for checks
  dabs.b2.resize(machine.nBitsPerLoop);
  dabs.bp.resize(machine.nBitsPerLoop);
  for (int i= 0; i < machine.nBitsPerLoop; ++i)
    {
      dabs.bp[i]= random_bits_modp[machine.sec + i];
      dabs.b2[i]= mod2_shares[machine.sec + i];
    }
  dabs.used= 0;
  total+= machine.nBitsPerLoop;
}

void daBitVector::get_daBit(Share &bpr, aBit &b2r, AbstractDABitGenerator &daBitGen)
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
                             AbstractDABitGenerator &daBitGen)
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
