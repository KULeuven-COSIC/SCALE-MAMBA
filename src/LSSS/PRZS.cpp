/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "PRZS.h"
#include "Share.h"
#include "Tools/Crypto.h"

PRZS::PRZS(Player &P)
{
  if (Share::SD.Otype != Reduced)
    {
      return; /* This is a NOP */
    }

  unsigned int n= P.nplayers();
  whoami= P.whoami();

  G.resize(n, vector<PRNG>(2));
  for (unsigned int i= 0; i < n; i++)
    {
      if (i != whoami)
        {
          uint8_t my_seed[SEED_SIZE];
          uint8_t their_seed[SEED_SIZE];
          P.G.get_random_bytes(my_seed, SEED_SIZE);

          string ss((char *) my_seed, SEED_SIZE), my_Comm, my_Open, their_Comm,
              their_Open;
          Commit(my_Comm, my_Open, ss, P.G);

          // Send my commitment
          P.send_to_player(i, my_Comm);

          // Receive their commitment
          P.receive_from_player(i, their_Comm);

          // Send my opening
          P.send_to_player(i, my_Open);

          // Receive their opening
          P.receive_from_player(i, their_Open);

          // Get their seed
          string ee;
          if (!Open(ee, their_Comm, their_Open))
            {
              throw invalid_commitment();
            }
          for (int j= 0; j < SEED_SIZE; j++)
            {
              their_seed[j]= (ee.c_str())[j];
            }

          // Set seeds
          G[i][0].SetSeedFromRandom(my_seed);
          G[i][1].SetSeedFromRandom(their_seed);
        }
    }
}

gfp PRZS::next_share()
{
  unsigned int n= G.size();
  gfp ans, te;
  ans.assign_zero();
  for (unsigned int i= 0; i < n; i++)
    {
      if (i != whoami)
        {
          te.randomize(G[i][0]);
          ans.add(te);
          te.randomize(G[i][1]);
          ans.sub(te);
        }
    }
  return ans;
}
