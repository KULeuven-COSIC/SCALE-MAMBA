/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _Open_Protocol
#define _Open_Protocol

/* This class manages the opening of shares
 * We have two types of opening.
 *    - OpenToAll
 *    - OpenToOne
 *
 * For SPDZ OpenToAll uses the MAC_Check to
 * ensure all is OK.
 *
 * For Q2 access structures OpenToAll uses
 * a restricted network and then reconstructs
 * and hashes the result (to be checked later).
 *
 * OpenToOne for SPDZ does just the naive sending
 * of shares. Only for use in the offline phase really
 *
 * OpenToOne for Q2 access structures sends all
 * the shares in, and then uses the parity check
 * matrix to ensure all is OK
 */

#include "Share.h"
#include "System/Player.h"
#include <openssl/sha.h>

class Open_Protocol
{
  /* Open Data */
  int open_cnt;
  int counter;
  vector<vector<gfp>> macs;
  vector<gfp> vals;

  // Add vectors to the MAC and Value list above
  void AddToMacs(const vector<Share> &shares);
  void AddToValues(const vector<gfp> &shares);

  // Hash function to define the hash for checking when type!=Full
  SHA256_CTX sha256;

public:
  Open_Protocol();

  /* These take vectors of shares and values
   *  - Implicit assume that the amount of data being sent does
   *    not overload the OS
   * Begin and End expect the same arrays values and S passed to them
   * and they expect values to be of the same size as S
   */
  void Open_To_All_Begin(vector<gfp> &values, const vector<Share> &S, Player &P,
                         bool verbose= false);
  void Open_To_All_End(vector<gfp> &values, const vector<Share> &S, Player &P,
                       bool verbose= false);

  /* First routine called by all bar player_num
   * Second routines called by player_num only
   * This is a naive method, with no checking etc
   * Mainly used in the offline phase. Not for use in the online phase
   */
  void Open_To_One_Begin(unsigned int player_num, const vector<Share> &S,
                         Player &P);
  void Open_To_One_End(vector<gfp> &values, const vector<Share> &S, Player &P);

  // Usually called automatically, unless you need to do it
  // yourself (e.g. before an open/end of protocol)
  void RunOpenCheck(Player &P, bool verbose= false); // Runs MAC check or Hash check
};

#endif
