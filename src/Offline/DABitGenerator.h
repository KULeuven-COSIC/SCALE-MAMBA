/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

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
#include "Offline/XOR_Machine.h"
#include "Offline/offline_data.h"
#include "System/Player.h"
#include "Tools/Timer.h"

#include <map>
#include <vector>

template<class SBit>
class MaliciousDABitMachine;
template<class SBit>
class AbstractDABitGenerator;
// Need to forward declare friends in templates
template<class SBit>
class LargePrimeDABitGenerator;
template<class SBit>
class SmallPrimeDABitGenerator;

// Holds vectors of bits with are double shared
// In the future this needs to be templated
template<class SBit>
class daBitVector
{
  vector<Share> bp;
  vector<SBit> b2;

  unsigned int used;

public:
  daBitVector()
  {
    bp.resize(0);
    b2.resize(0);
    used= 1;
  }

  // Gets the next daBit, and generates more if needed
  void get_daBit(Share &bpr, SBit &b2r, AbstractDABitGenerator<SBit> &daBitGen, Player &P);

  // As above but for a vector
  void get_daBits(vector<Share> &bpr, vector<SBit> &b2r, AbstractDABitGenerator<SBit> &daBitGen, Player &P);

  friend class LargePrimeDABitGenerator<SBit>;
  friend class SmallPrimeDABitGenerator<SBit>;
};

// Main class for generating daBits

template<class SBit>
class AbstractDABitGenerator
{

protected:
  long long total;

  map<string, Timer> timers;

  MaliciousDABitMachine<SBit> &machine;

  int online_thread_num;

  // OCD for getting preprocessed data such as triples or random bits
  offline_control_data &OCD;

  // XOR Machine for the n/2-party gf(p) xor
  XOR_Machine<SBit> xor_machine;

  // PRSS version for use with Share2 variant
  PRSS2 prss;

  // Shared randomness for FRand
  PRNG FRand;

  // Generate num masks in the Mod2 domain
  list<SBit> generate_masks(unsigned int num);

  buffer buff;

  size_t num_dabits;

public:
  // Previous code created a new player, according to thread_num
  // Now we just copy by reference, might want to change this in the future.
  AbstractDABitGenerator(MaliciousDABitMachine<SBit> &machine, Player &P, int thread_num)
      : machine(machine), OCD(*machine.OCD), xor_machine(P, OCD, thread_num), prss(P)
  {
    online_thread_num= thread_num;
  }

  virtual ~AbstractDABitGenerator()
  {
    ;
  }

  virtual void run(daBitVector<SBit> &dabs, Player &P)= 0;

  long long get_total() const
  {
    return total;
  }
};

// this is classic cut-and-choose from https://eprint.iacr.org/2019/207
template<class SBit>
class SmallPrimeDABitGenerator : public AbstractDABitGenerator<SBit>
{

  // Randomness for bit generation
  PRNG G;

  // This modifies the buffer
  void adjust_and_pack(vector<Share> &Shp, vector<SBit> &Sh2,
                       unsigned int player, vector<bool> bits, Player &P);

  void provide_random_inputs(vector<vector<Share>> &Shp,
                             vector<vector<SBit>> &Sh2, Player &P);

public:
  SmallPrimeDABitGenerator(MaliciousDABitMachine<SBit> &machine, Player &P, unsigned int online_thread_num)
      : AbstractDABitGenerator<SBit>(machine, P, online_thread_num)
  {
    G.ReSeed(P.get_thread());
    (*this).num_dabits= (*this).machine.cnc_param * (*this).machine.bucket_size * (*this).machine.nBitsPerLoop;
    (*this).buff.resize((*this).num_dabits * (1 + gfp::size()));
  }

  ~SmallPrimeDABitGenerator() { ; }

  void run(daBitVector<SBit> &dabs, Player &P);
};

// This is using the improved dabit generation from https://eprint.iacr.org/2019/974
// which works for large primes; currently we've only implemented it for dishonest
// majority
template<class SBit>
class LargePrimeDABitGenerator : public AbstractDABitGenerator<SBit>
{

public:
  LargePrimeDABitGenerator(MaliciousDABitMachine<SBit> &machine, Player &P, unsigned int online_thread_num)
      : AbstractDABitGenerator<SBit>(machine, P, online_thread_num)
  {
    //We assume that p is large enough (more than 54 bits long
    //for nBitsPerLoop = 2**13 and sec = 40)
    (*this).num_dabits= (*this).machine.nBitsPerLoop + ((*this).machine.gamma * (*this).machine.sec);

    (*this).buff.resize((*this).num_dabits * (gfp::size()));
  }

  ~LargePrimeDABitGenerator() { ; }

  void run(daBitVector<SBit> &dabs, Player &P);
};

#endif /* SRC_OFFLINE_DABITGENERATOR_H_ */
