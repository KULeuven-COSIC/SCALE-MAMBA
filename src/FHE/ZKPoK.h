/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _ZKPOK
#define _ZKPOK

/* This class gives the method to perform the ZKPoK of
 * correctness of plaintexts
 *
 * Each player runs an instance of ZKPoK
 *
 *
 * Step 1:
 *
 * It generates the plaintext/ciphertexts according to
 * the flag of type condition (from Plaintext.h) which
 * is either General or Diagonal
 *
 * It then generates ZK_stat_sec plaintexts and randomness'
 * for this player, as well as V test "fake" ciphertexts
 *    - Fake as the distributions are all screwy from
 *      a valid ciphertext. This is to get the proof to
 *      work
 *    - If Diagonal then the ZK_stat_sec test ciphertexts
 *      are all identical, and we set the Diagonal element
 *      to be "alpha" (which in the end will be the MAC key)
 *
 * The encryptions of the V fake ciphertexts (A) are then
 * returned, along with the ZK_stat_sec ones (E) we are going
 * to be using later on.
 *
 *
 * Step 2:
 *
 * The players then agree on a random vector e of size
 * ZK_stat_sec consisting of zeros and ones.
 *
 * This stage then computes the z^{(i)},^{(i)}T pairs for
 * each player to compute (see the documentation Line 6 of
 * the protocol spec for what these are)
 *
 *
 * Step 3:
 *
 * This is the stage which performs the final check.
 *
 * Step 4:
 *
 * This allows one (up to ZK_stat_sec) times to recover both
 * the ciphertext sum, and the plaintext for this player
 * for the valid ciphertexts selected at the beginiing.
 */

#include "FHE_Keys.h"
#include "config.h"

class ZKPoK
{

  condition PoKType;

  unsigned int V;

  // Associated random coins for this player for the valid ciphertexts
  vector<Random_Coins> r;

  vector<int> e;

  vector<bool> used; // Vector to determine if an output is used yet

  // A[j] is the j-ciphertext of the test vectors in Step 1
  // In Step1_Step we add in the other players A vectors one by one
  vector<Ciphertext> A;

  // E[j] is j th ciphertext from me in Step 1
  // In Step1_Step we add in the other players E vectors one by one
  vector<Ciphertext> E;

  vector<Rq_Element> Z;   // The vector of z's
  vector<Random_Coins> T; // The vector of T's

  vector<Plaintext> m; // The plaintexts for this player

  vector<Ciphertext> eq; // Main equation checking vector

public:
  // If type=Diagonal then alpha is the thing we put on the diagonal
  // else random
  void Step1(condition type, const FHE_PK &pk, const FFT_Data &PTD, PRNG &G,
             const vector<gfp> &alpha= {});

  // Get my vA for broadcasting
  void get_vA(ostream &s) const;
  // Get my vE for broadcasting
  void get_vE(ostream &s) const;
  // Get my vT for broadcasting
  void get_vT(ostream &s) const;
  // Get my vz for broadcasting
  void get_vZ(ostream &s) const;

  // Player calls this to enter the each other players vectors vE and vA
  void Step1_Step(istream &vE, istream &vA, const FHE_PK &pk);

  void Step2(const vector<int> &e, const FHE_PK &pk);

  // Player calls this to enter the each other players vector vT
  void Step2_Step(istream &vT, istream &vZ, const FHE_PK &pk);

  bool Step3(const FHE_PK &pk, const FFT_Data &PTD, unsigned int nplayers);

  // Have we used up all the values yet of a completed proof?
  bool isempty() const;

  // Following returns -1 if empty
  int get_next_unused() const;

  // Get entry i, set it to be used (abort if already used)
  void get_entry(Plaintext &mess, Ciphertext &ctx, unsigned int i);

  unsigned int size_batch() const
  {
    return ZK_stat_sec;
  }
  unsigned int size_checkset() const
  {
    return V;
  }
};

#endif
