/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _Garbled
#define _Garbled

/* This defines a GC using the method of 2017/214,
 * we use this rather than 2017/189 as we want active
 * functionality. And the protocol of 2017/214 is easier
 * to adapt to this case.
 *
 * As the circular 2-correlation robust PRF F_K(m) for 
 * K=(k1,k2) we use the * following function, MMO based function.
 * The tweak with the keys is to enable free-XOR to be
 * secure, with "*x" meaning multiplying by x in GF(2^128)
 *
 *   M = m xor (k1*x) xor (k2*x^2)
 *   c = AES_k(M) xor M
 */

#include "Circuit.h"
#include "OT/aAND.h"
#include "OT/aBitFactory.h"

class Garbled_Circuit
{
  // Shared signal bits
  vector<aBit> lambda;

  // Wire labels
  vector<vector<gf2n>> k;

  // Shares of garbled AND gates
  // When opened these become the sums across all players
  vector<vector<vector<vector<gf2n>>>> gab;

  // The wire labels for the "one" wire
  //   - An invert is (a XOR 1) so we use a fixed
  //     wire label for the one wire
  // One label per party. When opened we add in Delta here
  vector<gf2n> one_label;

  /* Assignment of input/output variables to players
   * Vector of length C.num_inputs()/C.num_ouputs()
   * saying who owns this input/output variable
   */
  vector<int> i_assign;
  vector<int> o_assign;

  /* The next two vectors have first dimension size 
   *        C.num_inputs()/C.num_outputs()
   * Then the second dimension is only non-zero if
   * this player holds this input/output variable
   */

  // My output wire openings, one for each variable assigned to me
  vector<vector<int>> outputs;

  // My input wire openings, one for each variable assigned to me
  vector<vector<int>> inputs;

  void Open_Garbling(Player &P);

public:
  void Garble(const Circuit &C,
              const vector<int> &i_a, const vector<int> &o_a,
              aAND &aA, aBitFactory &aBF, Player &P);

  /* input/output is this players input/output. Again these 
   * are of length 
   *         C.num_inputs()/C.num_outputs()
   * Then the second dimension is only non-zero if
   * this player holds this input/output variable
   *
   * output is resized by this function, so can be blank on entry
   */
  void Evaluate(vector<vector<int>> &output,
                const vector<vector<int>> &input,
                const Circuit &C, Player &P);

public:
  void Garble(const Circuit &C,
              aAND &aA, aBitFactory &aBF, Player &P, PRNG &G);
};

#endif
