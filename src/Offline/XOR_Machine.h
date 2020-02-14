/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

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
#include "OT/aBit.h"
#include "Offline/offline_data.h"

class XOR_Machine
{

  vector<gfp> opened;
  vector<Share> shares;
  Player &P;

  vector<Share> allShp;
  vector<aBit> allSh2;

  vector<gfp> OPp;
  vector<int> OP2;
  offline_control_data &OCD;

  int thread;

  void xors(vector<Share> &result, const vector<Share> &LBits,
            const vector<Share> &RBits,
            Open_Protocol &OP);

  bool bit_equality(const gfp &x, const gf2n &y);

public:
  XOR_Machine(Player &P, offline_control_data &OCD, int thread);

  int get_thread() const { return thread; }

  void combine(vector<Share> &combinedp, vector<aBit> &combined2,
               const vector<Share> &Shp, const vector<aBit> Sh2,
               Open_Protocol &OP);

  void check_correctness(vector<Share> &combinedp,
                         vector<aBit> &combined2, int bucket_size, int nFinalBits,
                         Open_Protocol &OP);

  void party_log_xor(vector<Share> &accumulator, vector<vector<Share>> &lsts, Open_Protocol &OP);

  void consistency_check(const vector<vector<Share>> &Shp,
                         const vector<vector<aBit>> &Sh2, const vector<int> &perm, int left_out,
                         Open_Protocol &OP);
};

#endif /* SRC_OFFLINE_XOR_MACHINE_H_ */
