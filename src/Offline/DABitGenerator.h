/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2019, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

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
class DABitGenerator;

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
  void get_daBit(Share &bpr, aBit &b2r, DABitGenerator &daBitGen);

  // As above but for a vector
  void get_daBits(vector<Share> &bpr, vector<aBit> &b2r, DABitGenerator &daBitGen);

  friend class DABitGenerator;
};

// Main class for generating daBits
class DABitGenerator
{
  unsigned int thread_num;

  Player &P;

  MaliciousDABitMachine &machine;
  offline_control_data &OCD;

  // Randomness for bit generation
  PRNG G;

  // Shared randomness for FRand
  PRNG FRand;

  // MAC Check for GFp bits
  //   Do not use the one from the main online thread in case the
  //   open/closes get mixed up
  Open_Protocol OP;

  // XOR Machine for doing the CNC and n-party gf(p) xor
  XOR_Machine xor_machine;

  void adjust_and_pack(stringstream &ss, vector<Share> &Shp, vector<aBit> &Sh2,
                       unsigned int player, vector<bool> bits);

  void provide_random_inputs(vector<vector<Share>> &Shp,
                             vector<vector<aBit>> &Sh2, size_t nBits);

public:
  DABitGenerator(MaliciousDABitMachine &machine, Player &P, int thread_num, offline_control_data &OCD);
  void run(daBitVector &dabs);
  int get_thread_num() const { return thread_num; }

  map<string, Timer> timers;

  size_t report_sent();
};

#endif /* SRC_OFFLINE_DABITGENERATOR_H_ */
