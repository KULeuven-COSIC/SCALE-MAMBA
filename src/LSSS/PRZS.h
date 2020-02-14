/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _PRZS
#define _PRZS

#include <vector>
using namespace std;

#include "System/Player.h"
#include "Tools/random.h"

/* Creates a PRZS system for a full threshold structure
 * using shared keys k_{i,j} where 0 <= i,j < n.
 *
 * This is needed in the Reduced Comm variant of multiplication
 * for Replicated secret sharing
 */

class PRZS
{
  // Each party holds two PRNGs with each other party.
  // One it created and one the other player created
  vector<vector<PRNG>> G;

  // My number
  unsigned int whoami;

public:
  PRZS(Player &P);

  gfp next_share();
};

#endif
