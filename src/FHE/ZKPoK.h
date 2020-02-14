/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _ZKPOK
#define _ZKPOK

/* This class gives the method to perform the ZKPoK of
 * correctness of plaintexts
 *
 * Each player runs an instance of ZKPoK
 *   When single=true this runs as the prover or verifier
 *   otherwise it runs as the prover being all (as in 
 *   HighGear/TopGear), with a sum as the final statement.
 *   The former case is needed for FHE_IO creation. When
 *   single=true we need to know if we are the prover
 *   or the verifier
 *
 * Step 1:
 *
 * It generates the plaintext/ciphertexts according to
 * the flag of type condition (from Plaintext.h) which
 * is either General or Diagonal
 *
 * For HighGear U = ZK_sound_sec and V= 2*U-1
 * For TopGear  V = ZK_sound_sec/log_2(2*N+1) and U = 2*V
 *
 * It then generates U plaintexts and randomness' for this 
 * player, as well as V test "fake" ciphertexts
 *    - Fake as the distributions are all screwy from
 *      a valid ciphertext. This is to get the proof to
 *      work
 *    - If Diagonal then the U ciphertexts are all identical, 
 *      and we set the Diagonal element to be "alpha" (which 
 *      in the end will be the MAC key)
 *
 * The encryptions of the V fake ciphertexts (A) are then
 * returned, along with the U ones (E) we are going to be 
 * using later on.
 *
 *
 * Step 2:
 *
 * The players then agree on a random vector e of size
 * V consisting of 
 *   - HighGear zeros and ones.
 *   - TopGear elements from {X^i}_{i=0}^{2N-1} \cup {0}
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
 * This allows one (up to U) times to recover both the ciphertext 
 * sum, and the plaintext for this player for the valid ciphertexts 
 * selected at the beginiing.
 */

#include "FHE_Keys.h"
#include "config.h"

class ZKPoK
{
  condition PoKType;
  PoKVersion version;
  bool single, prover;

  unsigned int U, V;
  // The *actual* soundness security we achieve
  //   - Can be different from explicit formulae due to rounding
  unsigned int ssec;

  // Associated random coins for this player for the valid ciphertexts
  //   If single=true this only holds something if I am the prover
  vector<Random_Coins> r;

  vector<int> e;

  vector<bool> used; // Vector to determine if an output is used yet

  // A[j] is the j-ciphertext of the test vectors in Step 1
  // In Step1_Step we add in the other players A vectors one by one
  // if single=false, otherwise this is just the data from the prover/me
  vector<Ciphertext> A;

  // E[j] is j th ciphertext from me in Step 1
  // In Step1_Step we add in the other players E vectors one by one
  // if single=false, otherwise this is just the data from the prover
  // when prover=false, and my ciphertexts when prover=true
  vector<Ciphertext> E;

  //   If single=true this is just the data from the prover/me
  vector<Rq_Element> Z;   // The vector of z's
  vector<Random_Coins> T; // The vector of T's

  // The plaintexts for this player
  //   If single=true this only holds something if I am the prover
  vector<Plaintext> m;

  vector<Ciphertext> eq; // Main equation checking vector

  // Gets the matrix value we have
  int M(unsigned int k, unsigned int l, const vector<int> &e);

public:
  // Default settings (standard TopGear)
  ZKPoK()
  {
    single= false;
    prover= false;
  }

  // Settings for use with the IO setting
  //   - Basically run old SPDZ-2 proof with new technology
  void set_params(bool sin= false, bool prov= false)
  {
    single= sin;
    prover= prov;
  }

  bool is_single() const
  {
    return single;
  }
  bool is_prover() const
  {
    return prover;
  }

  /* Set up the initial ciphertexts we are going to prove.
   * We do this in a seperate call, as then we can repeat the proof
   * for the same ciphertexts over and over again, to increase
   * soundness security if needed (for the Diag proofs)
   *   If type=Diagonal then alpha is the thing we put on the diagonal 
   *   else random
   */
  void Step0(condition type, PoKVersion vers,
             const FHE_PK &pk, const FFT_Data &PTD, PRNG &G,
             const vector<gfp> &alpha= {});
  // Player calls this to enter the each other players vectors
  // and sum up the vE vectors
  void Step0_Step(istream &vE, const FHE_PK &pk);

  void Step1(const FHE_PK &pk, const FFT_Data &PTD, PRNG &G);

  // Player calls this to enter the each other players vectors vA
  void Step1_Step(istream &vA, const FHE_PK &pk);

  // Get my vA for broadcasting
  void get_vA(ostream &s) const;
  // Get my vE for broadcasting
  void get_vE(ostream &s) const;
  // Get my vT for broadcasting
  void get_vT(ostream &s) const;
  // Get my vz for broadcasting
  void get_vZ(ostream &s) const;

  // Generate the vector e for Step 2 from a random seed
  void Generate_e(vector<int> &e, uint8_t seed[SEED_SIZE]);

  // Call the Step 2
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

  unsigned int size_batch() const { return U; }
  unsigned int size_checkset() const { return V; }
  unsigned int get_sound_sec() const { return ssec; }
};

#endif
