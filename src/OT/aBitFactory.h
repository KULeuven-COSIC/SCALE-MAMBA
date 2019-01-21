/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _aBitFactory
#define _aBitFactory

#include "COT.h"
#include "aBit.h"

// Defines whether the vector is one of the following types
enum aBFstatus { EMPTY,
                 aShares };

class aBitFactory
{
  vector<Sender_COT> COTS;
  vector<Receiver_COT> COTR;

  PRNG G;

  gf2n Delta; // My Delta value

  aBFstatus status;
  vector<aBit> aBits;
  unsigned int used; // How many used of this vector

  BitVector xvec; // Defined here so we dont keep destroying and creating

  // default_l for the Abits and aShares
  unsigned int default_l;

  // size of the "used" aBits
  //   We do not keep resizing aBits as that costs time
  unsigned int sz_aBits;

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

  void make_aBits(Player &P)
  {
    make_aBits(default_l, P);
  }

  /* This procedure is used to tune what parameter we should
   * pass to next_iteration in l. 
   */
  void tune(Player &P, int verbose);

  /* This is the function you use to get new aBits
   *   It manages all the control stuff for you
   */
  aBit get_aShare(Player &P);

  gf2n get_Delta() const { return Delta; }

  uint8_t get_random_byte() { return G.get_uchar(); }
  gf2n get_random_gf2n()
  {
    gf2n x;
    x.randomize(G);
    return x;
  }

  unsigned int get_default_l() const { return default_l; }
};

#endif
