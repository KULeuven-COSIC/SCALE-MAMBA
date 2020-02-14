/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "COT.h"
#include "LSSS/PRSS.h"
#include "config.h"

void Sender_COT::init(Player &P, int i, CryptoPP::RandomPool &RNG)
{
  pair= i;

  ROT_S.resize(OT_comp_sec);
  string input, output;

#ifdef SimpleOT
  // Only generate one public key
  ROT_S[0].init(output, RNG);
  P.send_to_player(i, output, 2);
#endif

  for (unsigned int j= 0; j < OT_comp_sec; j++)
    {
#ifdef SimpleOT
      if (j != 0)
        {
          ROT_S[j]= ROT_S[0];
          ROT_S[j].reset();
        }
      P.receive_from_player(i, input, 2);
      ROT_S[j].message(input);
#else
      ROT_S[j].init(RNG);
      while (!ROT_S[j].is_complete())
        {
          P.receive_from_player(i, input, 2);
          ROT_S[j].message(output, input, RNG);
          P.send_to_player(i, output, 2);
        }
#endif
    }
}

void Receiver_COT::init(Player &P, int i, CryptoPP::RandomPool &RNG, vector<int> choicebits)
{
  pair= i;

  ROT_R.resize(OT_comp_sec);
  string input, output;

#ifdef SimpleOT
  // We only have one public key coming to us
  P.receive_from_player(i, input, 2);
#endif

  for (unsigned int j= 0; j < OT_comp_sec; j++)
    {
#ifdef SimpleOT
      ROT_R[j].init(input, choicebits[j]);
      ROT_R[j].message(output, RNG);
      P.send_to_player(i, output, 2);
#else
      ROT_R[j].init(RNG, choicebits[j]);
      while (!ROT_R[j].is_complete())
        {
          if (ROT_R[j].get_state() != 0)
            {
              P.receive_from_player(i, input, 2);
            }
          ROT_R[j].message(output, input);
          if (!ROT_R[j].is_complete())
            {
              P.send_to_player(i, output, 2);
            }
        }
#endif
    }
  compute_Delta();
}

void Sender_COT::next_iteration(Player &P,
                                unsigned int sz_aB, vector<aBit> &aB,
                                const BitVector &x)
{
  // Step 1 and 2
  vector<BitMatrix> ttM(2, BitMatrix(sz_aB));
  for (unsigned int j= 0; j < OT_comp_sec; j++)
    {
      for (unsigned int i= 0; i < 2; i++)
        {
          ROT_S[j].get_random_bits(i, j, ttM[i]);
        }
    }

  // Duplicate x to each row of xMatrix
  BitMatrix xMatrix(sz_aB);
  for (unsigned int j= 0; j < OT_comp_sec; j++)
    {
      xMatrix.set_row_up_to(j, x);
    }

  // Do x+tt[0]+tt[1]
  xMatrix.add(ttM[0]);
  xMatrix.add(ttM[1]);

  // Now send xMatrix to the other player
  stringstream ss;
  xMatrix.output(ss);
  P.send_to_player(pair, ss.str(), 2);

  // Step 4
  gf2n t;
  ttM[0].transpose();
  for (unsigned int i= 0; i < sz_aB; i++)
    {
      unsigned int blk= i >> 7, entry= i & 127;
      t= ttM[0].to_gf2n(entry, blk);
      aB[i].set_MAC(pair, t);
    }
}

void Receiver_COT::next_iteration(Player &P,
                                  unsigned int sz_aB, vector<aBit> &aB)
{
  // Step 1
  BitMatrix qqM(sz_aB);
  for (unsigned int j= 0; j < OT_comp_sec; j++)
    {
      ROT_R[j].get_random_bits(j, qqM);
    }

  // Step 2 and 3
  string ss;
  BitMatrix xMatrix(sz_aB);
  P.receive_from_player(pair, ss, 2);
  istringstream iss(ss);
  xMatrix.input(iss);

  // Zero row of xMatrix if b=0
  for (unsigned int j= 0; j < OT_comp_sec; j++)
    {
      int b= ROT_R[j].get_bit();
      if (b == 0)
        {
          xMatrix.zero_row(j);
        }
    }
  qqM.add(xMatrix);

  // Step 4
  gf2n q;
  qqM.transpose();
  for (unsigned int i= 0; i < sz_aB; i++)
    {
      unsigned int blk= i >> 7, entry= i & 127;
      q= qqM.to_gf2n(entry, blk);
      aB[i].set_Key(pair, q);
    }
}

// Convert the vector of length OT_comp_sec/8 into a GF(2^OT_comp_sec) element
gf2n to_gf2n(const vector<uint8_t> &xx)
{
  gf2n a;
  a.assign_zero();
  for (int i= OT_comp_sec / 8 - 1; i >= 0; i--)
    {
      a.SHL(a, 8);
      a.add(xx[i]);
    }
  return a;
}

void Receiver_COT::compute_Delta()
{
  vector<uint8_t> DeltaV(OT_comp_sec / 8);
  for (unsigned int i= 0; i < OT_comp_sec / 8; i++)
    {
      DeltaV[i]= 0;
      for (unsigned int j= 0; j < 8; j++)
        {
          DeltaV[i]+= (ROT_R[8 * i + j].get_bit() << j);
        }
    }

  Delta= to_gf2n(DeltaV);
}

void Sender_COT::next_checked_iteration(Player &P,
                                        unsigned int sz_aB, vector<aBit> &aB,
                                        BitVector &x)
{
  next_iteration(P, sz_aB, aB, x);

  // Agree PRNG for checking
  vector<unsigned int> A(P.nplayers());
  for (unsigned int i= 0; i < P.nplayers(); i++)
    {
      A[i]= 0;
    }
  A[P.whoami()]= 1;
  A[pair]= 1;

  uint8_t seed[SEED_SIZE];
  AgreeRandom(P, A, seed, SEED_SIZE, 2);
  PRNG G2;
  G2.SetSeedFromRandom(seed);

  // Compute the check data for the sender
  gf2n xc, chi, tc, temp;
  xc.assign_zero();
  tc.assign_zero();

  for (unsigned int j= 0; j < sz_aB; j++)
    {
      chi.randomize(G2);
      if (aB[j].get_value())
        {
          xc.add(chi);
        }
      temp.mul(chi, aB[j].get_MAC(pair));
      tc.add(temp);
    }
  // Now send xc and tc to the Receiver
  stringstream ss;
  xc.output(ss);
  tc.output(ss);
  P.send_to_player(pair, ss.str(), 2);
}

void Receiver_COT::next_checked_iteration(Player &P,
                                          unsigned int sz_aB, vector<aBit> &aB)
{
  next_iteration(P, sz_aB, aB);

  // Agree PRNG for checking
  vector<unsigned int> A(P.nplayers());
  for (unsigned int i= 0; i < P.nplayers(); i++)
    {
      A[i]= 0;
    }
  A[P.whoami()]= 1;
  A[pair]= 1;

  uint8_t seed[SEED_SIZE];
  AgreeRandom(P, A, seed, SEED_SIZE, 2);
  PRNG G2;
  G2.SetSeedFromRandom(seed);

  // Compute the check data for the receiver
  gf2n xc, tc, qc, chi, temp;
  qc.assign_zero();

  for (unsigned int j= 0; j < sz_aB; j++)
    {
      chi.randomize(G2);
      temp.mul(chi, aB[j].get_Key(pair));
      qc.add(temp);
    }

  // Get xc and tc from the Sender
  string ss;
  P.receive_from_player(pair, ss, 2);
  istringstream iss(ss);
  xc.input(iss);
  tc.input(iss);

  // Do the check
  gf2n eq;
  eq.mul(xc, Delta);
  eq.add(tc);
  eq.add(qc);
  if (!eq.is_zero())
    {
      throw bad_value();
    }
}
