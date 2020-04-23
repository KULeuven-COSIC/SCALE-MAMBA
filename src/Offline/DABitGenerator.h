/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

/*
 * Wrapper for various classes
 * used for generating daBits.
 * 
 */

#ifndef SRC_OFFLINE_DABITGENERATOR_H_
#define SRC_OFFLINE_DABITGENERATOR_H_

#include "LSSS/Open_Protocol.h"
#include "LSSS/PRSS.h"
#include "LSSS/Share.h"
#include "Math/gf2n.h"
#include "Math/gfp.h"
#include "OT/aBit.h"
#include "Offline/XOR_Machine.h"
#include "Offline/offline_data.h"
#include "System/Player.h"
#include "Tools/Timer.h"

#include <map>
#include <vector>

class MaliciousDABitMachine;
class AbstractDABitGenerator;

// Holds vectors of bits with are double shared
// In the future this needs to be templated
class daBitVector
{
  vector<Share> bp;
  vector<aBit> b2;

  unsigned int used;

public:
  daBitVector()
  {
    bp.resize(0);
    b2.resize(0);
    used= 1;
  }

  // Gets the next daBit, and generates more if needed
  void get_daBit(Share &bpr, aBit &b2r, AbstractDABitGenerator &daBitGen);

  // As above but for a vector
  void get_daBits(vector<Share> &bpr, vector<aBit> &b2r, AbstractDABitGenerator &daBitGen);

  friend class LargePrimeDABitGenerator;
  friend class SmallPrimeDABitGenerator;
};

// Main class for generating daBits

class AbstractDABitGenerator
{
protected:
  int thread_num;

  // MAC Check for GFp bits
  //   Do not use the one from the main online thread in case the
  //   open/closes get mixed up
  Open_Protocol OP;

public:
  Player &P;
  long long total;

  map<string, Timer> timers;

  // Previous code created a new player, according to thread_num
  // Now we just copy by reference, might want to change this in the future.
  AbstractDABitGenerator(int thread_num, Player &P)
      : thread_num(thread_num), P(P), total(0)
  {
  }

  virtual ~AbstractDABitGenerator()
  {
  }
  virtual void run(daBitVector &dabs)= 0;
  int get_thread_num() const
  {
    return thread_num;
  }
};

// this is classic cut-and-choose from https://eprint.iacr.org/2019/207
class SmallPrimeDABitGenerator : public AbstractDABitGenerator
{

  MaliciousDABitMachine &machine;

  // Randomness for bit generation
  PRNG G;

  // Shared randomness for FRand
  PRNG FRand;

  // OCD for getting preprocessed data such as triples or random bits
  offline_control_data &OCD;

  // XOR Machine for doing the CNC and n-party gf(p) xor
  XOR_Machine xor_machine;

  // Counter for debug
  unsigned long long total;

  void adjust_and_pack(stringstream &ss, vector<Share> &Shp, vector<aBit> &Sh2,
                       unsigned int player, vector<bool> bits);

  void provide_random_inputs(vector<vector<Share>> &Shp,
                             vector<vector<aBit>> &Sh2, size_t nBits);

public:
  SmallPrimeDABitGenerator(MaliciousDABitMachine &machine, Player &P, int thread_num);
  void run(daBitVector &dabs);
};

// This is using the improved dabit generation from https://eprint.iacr.org/2019/974
// which works for large primes; currently we've only implemented it for dishonest
// majority
class LargePrimeDABitGenerator : public AbstractDABitGenerator
{
  // to get parameters state
  MaliciousDABitMachine &machine;

  // OCD for getting preprocessed data such as triples or random bits
  offline_control_data &OCD;

  //XOR Machine for doing n/2 party XOR
  // Shared randomness for FRand
  PRNG FRand;

  // XOR Machine for the n/2-party gf(p) xor
  XOR_Machine xor_machine;

  void input_GF2_bits(vector<aBit> &out_sh2, const vector<bool> &bits);
  void check_public_lsbs(const vector<Share> modp_bits, const vector<aBit> mod2_bits);
  void split_shares(vector<gfp> &out_modp_shares, const vector<Share> &modp_bits);
  void prepare_for_xor(vector<vector<Share>> &bit_rows, const vector<Share> &modp_bits);

public:
  LargePrimeDABitGenerator(MaliciousDABitMachine &machine, Player &P, int thread_num);
  void run(daBitVector &dabs);
};

#endif /* SRC_OFFLINE_DABITGENERATOR_H_ */
