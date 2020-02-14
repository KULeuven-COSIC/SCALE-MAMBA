/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

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
  int open_cnt[3];
  int counter[3];
  vector<vector<vector<gfp>>> macs;
  vector<vector<gfp>> vals;

  // Add vectors to the MAC and Value list above
  void AddToMacs(const vector<Share> &shares, int connection);
  void AddToValues(const vector<gfp> &shares, int connection);

  // Hash function to define the hash for checking when type!=Full
  vector<SHA256_CTX> sha256;

public:
  Open_Protocol();
  ~Open_Protocol();

  /* These take vectors of shares and values
   *  - Implicit assume that the amount of data being sent does
   *    not overload the OS
   * Begin and End expect the same arrays values and S passed to them
   * and they expect values to be of the same size as S
   * The connection is zero for START/STOP OPENs and one for private IO commands
   */
  void Open_To_All_Begin(vector<gfp> &values, const vector<Share> &S, Player &P,
                         int connection= 0, bool verbose= false);
  void Open_To_All_End(vector<gfp> &values, const vector<Share> &S, Player &P,
                       int connection= 0, bool verbose= false);

  /* First routine called by all bar player_num
   * Second routines called by player_num only
   * This is a naive method, with no checking etc
   * Used in the offline phase. Not for use in the online phase
   * No need to do these on different connections etc
   */
  void Open_To_One_Begin(unsigned int player_num, const vector<Share> &S,
                         Player &P);
  void Open_To_One_End(vector<gfp> &values, const vector<Share> &S, Player &P);

  // Runs MAC check or Hash check
  // Usually called automatically, unless you need to do it
  // yourself (e.g. before an open/end of protocol)
  // Takes in some auxillary string which is also checked
  void RunOpenCheck(Player &P, const string &aux, int connection, bool verbose= false);
};

#endif
