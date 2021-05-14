/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

/*
 * Wrapper class for doing heavy XORing in gf(p)
 * using the MPC engine. This is used for generating
 * daBits - it also Opens partial data to check
 * that it is consistent in both fields, method names
 * should be pretty self-explanatory.
 */

#ifndef SRC_OFFLINE_XOR_MACHINE_H_
#define SRC_OFFLINE_XOR_MACHINE_H_

#include "LSSS/Open_Protocol.h"
#include "Math/gf2n.h"
#include "Offline/offline_data.h"

template<class SBit>
class XOR_Machine
{

  vector<gfp> opened;
  vector<Share> shares;
  Player &P;

  vector<Share> allShp;
  vector<SBit> allSh2;

  vector<gfp> valuesp;
  vector<word> values2;
  offline_control_data &OCD;

  int thread;

  void xors(vector<Share> &result, const vector<Share> &LBits,
            const vector<Share> &RBits);

  bool bit_equality(const gfp &x, const word &y);

public:
  XOR_Machine(Player &P, offline_control_data &OCD, int online_thread);

  int get_thread() const
  {
    return thread;
  }

  void combine(vector<Share> &combinedp, vector<SBit> &combined2,
               const vector<Share> &Shp, const vector<SBit> Sh2);

  void check_correctness(vector<Share> &combinedp,
                         vector<SBit> &combined2, int bucket_size, int nFinalBits);

  void party_log_xor(vector<Share> &accumulator, vector<vector<Share>> &lsts);

  void consistency_check(const vector<vector<Share>> &Shp,
                         const vector<vector<SBit>> &Sh2, const vector<int> &perm, int left_out);

  /* A routine for testing all dabits */
  void check_debug(const vector<vector<Share>> &combinedp,
                   const vector<vector<SBit>> &combined2);
};

#endif /* SRC_OFFLINE_XOR_MACHINE_H_ */
