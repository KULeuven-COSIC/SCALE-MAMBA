/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _PRSS
#define _PRSS

#include <vector>
using namespace std;

#include "CAS.h"
#include "Share.h"
#include "System/Player.h"
#include "Tools/random.h"

class PRSS
{
  // This is the system for Shamir/Q2MSP PRSS
  vector<vector<unsigned int>> Asets;

  // For Shamir/Q2MSP for each Aset containing me we have a PRG
  // For Replicated for each *share* we have a PRG
  vector<PRNG> G;

  // For general Shamir/Q2 we have a vector of shares values for each max-unqual set
  vector<Share> Shares_Of_One;

  // My number
  unsigned int whoami;

  void ReplicatedSetUp(Player &P, const CAS &AS, const MSP &M);
  void MSP_SetUp(Player &P, const CAS &AS, const MSP &M);

  // When using a protocol we do batch production to save time
  vector<Share> batch;
  unsigned int batch_pos;

  // Protocol to produce a batch
  void batch_production(Player &P);

public:
  PRSS(Player &P);

  Share next_share(Player &P);
};

/* Some helper functions, useful elsewhere */

/* Agree a random seed between all players in set A
 *    Gauranteed that whoami is in A
 */
void AgreeRandom(Player &P, vector<unsigned int> &A, uint8_t *seed, int len,
                 int connection= 0);

// Same but for all players
void AgreeRandom(Player &P, uint8_t *seed, int len,
                 int connection= 0);

#endif
