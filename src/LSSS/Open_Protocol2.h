/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _Open_Protocol2
#define _Open_Protocol2

/* This class manages the opening of shares
 * We have two types of opening.
 *    - OpenToAll
 *    - OpenToOne
 *
 * For Q2 access structures OpenToAll uses
 * a restricted network and then reconstructs
 * and hashes the result (to be checked later).
 *
 * For full threshold access structures the BDOZ
 * MACs are checked
 *
 * OpenToOne for Q2 access structures sends all
 * the shares in, and then uses the parity check
 * matrix to ensure all is OK. We do not implement
 * a version of OpenToOne for full threshold as
 * it is not needed here
 *
 */

#include <openssl/sha.h>

#include "LSSS/Share2.h"
#include "Mod2Engine/aBitVector2.h"
#include "OT/aBit.h"
#include "OT/aBitVector.h"
#include "Tools/buffer.h"

class Player;

class Open_Protocol2
{
  /* Open Data */
  int open_cnt[3];
  string hss[3]; // The hash state (kept so we only hash every so often)

  // Hash function to define the hash for checking when type!=Full
  vector<SHA256_CTX> sha256;

  // This is memory which we keep around for buffering IO
  // It stops us having to create/delete memory on every
  // receive operation
  mutable buffer buff;

public:
  Open_Protocol2();
  ~Open_Protocol2();

  /* These take vectors of shares and values
   *  - Implicit assume that the amount of data being sent does
   *    not overload the OS
   * Connection should normally be two for Open_Protocol2
   *  - If not specified it is connection 2
   */

  /* This executes assuming Share2 contains 64 bits of data */
  void Open_To_All(vector<word> &values, const vector<Share2> &S, Player &P,
                   int connection);

  /* This version opens a single value of a Share2 or an aBit */

  /* Open (and hash) a single Share2 
   *   - Whether a single bit or 64 bits
   *   - num = Number of bytes to open (usually eight)
   */
  void Open_Bit(word &dv, const Share2 &v, Player &P, int connection= 2, int num= 8);

  /* Open (and check) a single aBit */
  void Open_Bit(word &dv, const aBit &v, Player &P);

  /* Same for vectors */
  void Open_Bits(vector<word> &dv, const vector<aBit> &v, Player &P);

  void Open_Bits(vector<word> &dv, const vector<Share2> &v, Player &P)
  { // This is a bit inefficient as we fix num=8, when we could use num=1 here
    Open_To_All(dv, v, P, 2);
  }

  /* Open (and hash) a single aBitVector2.  */
  void Open_BitVector(word &dv, const aBitVector2 &v, Player &P)
  {
    Open_Bit(dv, v.internal_rep(), P);
  }

  /* Open (and check) a single aBitVector */
  void Open_BitVector(word &dv, const aBitVector &v, Player &P);

  /* Open to a vector of Bits to party j
   *   dv zero if j<>P.whoami
   *   Used in the offline phase. Not for use in the online phase
   *   Uses connection 2
   */
  void Open_Bits_To(vector<word> &values, unsigned int j, const vector<aBit> &v, Player &P);
  void Open_Bits_To(vector<word> &values, unsigned int j, const vector<Share2> &v, Player &P);

  // Runs the Hash check
  // Usually called automatically, unless you need to do it
  // yourself (e.g. before an open/end of protocol)
  // Takes in some auxillary string which is also checked
  void RunOpenCheck(Player &P, const string &aux, int connection, bool verbose= false);
};

#endif
