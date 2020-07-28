/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _COT
#define _COT

/* This implements the COT protocol in Figure 19 of eprint 2015/901
 * The role of sender and receiver are reversed, so as to match
 * the sender and receiver of the initial seed ROT
 *
 * To get active security the outputs are then checked using the
 * protocol in Figure 7 of eprint 2015/546.
 *
 * We do OT_comp_sec such OTs at once, so in terms of the above
 * paper kappa=OT_comp_sec, but as we use gf2n elements to store
 * the keys/mac/deltas and our gf2n class is fixed for GF(2^{128})
 * this means we are fixed (for now) to OT_comp_sec=128
 */

#include "ROT.h"
#include "SimpleROT.h"
#include "System/Player.h"
#include "Tools/random.h"
#include "aBit.h"

class Sender_COT
{
#ifdef SimpleOT
  vector<SimpleOT_Sender> ROT_S;
#else
  vector<OT_Sender> ROT_S;
#endif
  int pair;
  unsigned int connection;

public:
  // Execute the next extraction procedure
  // This is protocol of Figure 19 of eprint 2015/901
  // We pass in the aBit here and this procedure sets the t value
  // Note we pass in sz_aB, as aB/x could be much bigger
  //   - This is passively secure only!
  void next_iteration(Player &P, unsigned int sz_aB,
                      vector<aBit> &aB,
                      const BitVector &x);

  void init(Player &P, int i, CryptoPP::RandomPool &RNG, unsigned int connectionNb= 2);

  // This is a COT with player i

  // This executes Fig 19 of eprint 2015/901 and then does
  // Figure 7 of 2015/546
  // Note we pass in sz_aB, as aB and x could be much bigger
  void next_checked_iteration(Player &P,
                              unsigned int sz_aB,
                              vector<aBit> &aB,
                              BitVector &x);
};

class Receiver_COT
{
#ifdef SimpleOT
  vector<SimpleOT_Receiver> ROT_R;
#else
  vector<OT_Receiver> ROT_R;
#endif
  int pair;
  gf2n Delta;
  unsigned int connection;

  void compute_Delta();

public:
  // Execute the next extraction procedure
  // This is protocol of Figure 19 of eprint 2015/901
  // We pass in the aBit here and this procedure sets the q value
  // Note we pass in sz_aB, as aB could be much bigger
  //   - This is passively secure only!
  void next_iteration(Player &P, unsigned int sz_aB, vector<aBit> &aB);

  // This is a COT with player i
  void init(Player &P, int i, CryptoPP::RandomPool &RNG, vector<int> choicebits, unsigned int connectionNb= 2);

  gf2n get_Delta() const
  {
    return Delta;
  }

  // This executes Fig 19 of eprint 2015/901 and then does
  // Figure 7 of 2015/546
  void next_checked_iteration(Player &P, unsigned int sz_aB, vector<aBit> &aB);
};

#endif
