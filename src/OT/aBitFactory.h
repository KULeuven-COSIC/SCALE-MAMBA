/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _aBitFactory
#define _aBitFactory

#include "COT.h"
#include "aBit.h"

class aBitFactory
{
  vector<Sender_COT> COTS;
  vector<Receiver_COT> COTR;

  PRNG G;

  gf2n Delta; // My Delta value

  vector<aBit> aBits;

  BitVector xvec; // Defined here so we dont keep destroying and creating

  // default_l for the Abits and aShares
  unsigned int default_l;

  /* This runs the Protocol 16 of ePrint 2017/214
   * steps 1-4, in the context of ePrint 2017/189
   * this actually results in aShares being produced.
   *
   * This actually creates 8*l new aBits
   * Returns how many made (i.e. 8*l)
   */
  unsigned int make_aBits(unsigned int l, Player &P);

public:
  void Initialize(Player &P);

  unsigned int make_aBits(Player &P)
  {
    unsigned int m= make_aBits(default_l, P);
    return m;
  }

  // Copy existing array into back of the list aBl
  void copy_aBits(list<aBit> &aBl, unsigned int m)
  {
    copy(aBits.begin(), aBits.begin() + m, back_inserter(aBl));
  }

  /* This procedure is used to tune what parameter we should
   * pass to next_iteration in l. 
   * Any produced aBits are added into aBl
   */
  void tune(Player &P, list<aBit> &aBl, int verbose);

  /* Should not call this until Initialize has had time to finish */
  gf2n get_Delta() const
  {
    return Delta;
  }
};

#endif
