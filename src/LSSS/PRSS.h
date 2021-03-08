/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _PRSS
#define _PRSS

#include <vector>
using namespace std;

#include "CAS.h"
#include "Share.h"
#include "Share2.h"
#include "System/Player.h"
#include "Tools/random.h"

template<class Sh, class Fld>
class PRSS_Base
{
protected:
  // This is the system for Shamir/Q2MSP PRSS
  vector<vector<unsigned int>> Asets;

  // For Shamir/Q2MSP for each Aset containing me we have a PRG
  // For Replicated for each *share* we have a PRG
  vector<PRNG> G;

  // For general Shamir/Q2 we have a vector of shares values for each max-unqual set
  vector<Sh> Shares_Of_One;

  // My number
  unsigned int whoami;

  void ReplicatedSetUp(Player &P, const CAS &AS, const MSP<Fld> &M);
  void MSP_SetUp(Player &P, const CAS &AS, const MSP<Fld> &M);

public:
  PRSS_Base(Player &P);
};

class PRSS : public PRSS_Base<Share, gfp>
{
  // When using a protocol we do batch production to save time
  vector<Share> batch;
  unsigned int batch_pos;

  // Protocol to produce a batch
  void batch_production(Player &P);

public:
  PRSS(Player &P) : PRSS_Base(P)
  {
    if ((Share::SD.type == Shamir || Share::SD.type == Q2MSP) && Asets.size() == 0)
      {
        batch_pos= 0;
        batch.resize(1024, Share(whoami));
        batch_production(P);
      }
  }

  Share next_share(Player &P);
};

/* For the mod2 sharing, when using PRSS, we only have
 * the non-interactive version (otherwise we would be in
 * the HSS version) and we generate a lot of shares in
 * one go
 */
class PRSS2 : public PRSS_Base<Share2, gf2>
{
public:
  PRSS2(Player &P) : PRSS_Base(P) { ; }

  Share2 next_share();
};

/* Some helper functions, useful elsewhere */

/* Agree a random seed between all players in set A
 *    Gauranteed that whoami is in A
 */
void AgreeRandom(Player &P, vector<unsigned int> &A, uint8_t *seed, int len, int connection);

// Same but for all players
void AgreeRandom(Player &P, uint8_t *seed, int len, int connection);

#endif
