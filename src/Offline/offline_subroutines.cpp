/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "offline_subroutines.h"

/* Make a share vector which shares a value val
 *   Given (global) mac values
 */
void make_shares(vector<Share> &share, const gfp &val, const vector<gfp> &macs,
                 PRNG &G)
{
  make_shares(share, val, G);

  vector<vector<gfp>> macv(share.size(), vector<gfp>(macs.size()));

  if (Share::SD.type == Full)
    {
      for (unsigned int j= 0; j < macs.size(); j++)
        {
          macv[0][j]= macs[j];
        }
      for (unsigned int i= 1; i < share.size(); i++)
        {
          for (unsigned j= 0; j < macs.size(); j++)
            {
              macv[i][j].randomize(G);
              macv[0][j].sub(macv[i][j]);
            }
        }
    }

  for (unsigned int i= 0; i < share.size(); i++)
    {
      share[i].set_macs(macv[i]);
    }
}

/* This produces the sum of Schur products needed by
 * each party in Maurer and Reduced protocols
 */
gfp schur_sum_prod(const Share &aa, const Share &bb, const Player &P)
{
  gfp prod;
  if (Share::SD.M.shares_per_player(P.whoami()) == 1)
    {
      prod.mul(aa.get_share(0), bb.get_share(0));
      // Multiply by the Schur coefficient here
      prod.mul(prod, Share::SD.Schur.get_schur(P.whoami(), 0, 0));
    }
  else
    {
      prod.assign_zero();
      gfp t;
      for (unsigned int i= 0; i < Share::SD.M.shares_per_player(P.whoami());
           i++)
        {
          for (unsigned int j= 0; j < Share::SD.M.shares_per_player(P.whoami());
               j++)
            {
              if (Share::SD.Schur.is_non_zero(P.whoami(), i, j))
                {
                  t.mul(aa.get_share(i), bb.get_share(j));
                  t.mul(Share::SD.Schur.get_schur(P.whoami(), i, j));
                  prod.add(t);
                }
            }
        }
    }

  return prod;
}
