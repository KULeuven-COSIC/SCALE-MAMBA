/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "LaAND.h"
#include "HaAND.h"
#include "Tools/Crypto.h"

void LaAND::make_more(aBitFactory &aBF, Player &P)
{
  // First steps 1,2,3
  HaAND HA;
  HA.make_more(aBF, P);
  unsigned int number= HA.get_number();

  x.resize(number);
  y.resize(number);
  z.resize(number);

  vector<string> o(P.nplayers());
  vector<aBit> r(number);
  ostringstream os;
  for (unsigned int t= 0; t < number; t++)
    {
      x[t]= HA.x[t];
      y[t]= HA.y[t];
      z[t]= aBF.get_aShare(P);
      int et= (x[t].get_value() * y[t].get_value()) ^ HA.v[t] ^ z[t].get_value();
      os << et << " ";
      z[t].set_value(z[t].get_value() ^ et); // Sets the z value to z+e
    }
  o[P.whoami()]= os.str();
  P.Broadcast_Receive(o, true);
  for (unsigned int i= 0; i < P.nplayers(); i++)
    {
      if (i != P.whoami())
        {
          istringstream os(o[i]);
          int ei;
          gf2n nk;
          for (unsigned int t= 0; t < number; t++)
            {
              os >> ei;
              if (ei == 1)
                { // Tweak the key (XXXX Any bug this is where it is)
                  // As we tweaked the z value above
                  nk.add(z[t].get_Key(i), aBF.get_Delta());
                  z[t].set_Key(i, nk);
                }
            }
        }
    }

  // Step 4
  vector<gf2n> Psi(number);
  for (unsigned int t= 0; t < number; t++)
    {
      Psi[t].assign_zero();
      if (y[t].get_value() == 1)
        {
          Psi[t]= aBF.get_Delta();
        }
      for (unsigned int k= 0; k < P.nplayers(); k++)
        {
          if (k != P.whoami())
            {
              Psi[t].add(y[t].get_Key(k));
              Psi[t].add(y[t].get_MAC(k));
            }
        }
    }

  // Step 5 (Stage 1)
  gf2n U;
  for (unsigned int j= 0; j < P.nplayers(); j++)
    {
      if (j != P.whoami())
        {
          ostringstream os;
          for (unsigned int t= 0; t < number; t++)
            {
              U.add(HA.HKiD[t][j], HA.HKi[t][j]);
              U.add(Psi[t]);
              U.output(os);
            }
          o[j]= os.str();
        }
    }
  P.Send_Distinct_And_Receive(o);

  // Step 5 (Stage 2) and Step 6
  vector<vector<gf2n>> Hv(number, vector<gf2n>(P.nplayers()));
  gf2n H;
  vector<istringstream> is(P.nplayers());
  for (unsigned int i= 0; i < P.nplayers(); i++)
    {
      if (i != P.whoami())
        {
          is[i].str(o[i]);
        }
    }
  for (unsigned int t= 0; t < number; t++)
    {
      H.assign_zero();
      if (x[t].get_value() == 1)
        {
          H= Psi[t];
        }
      if (z[t].get_value() == 1)
        {
          H.add(aBF.get_Delta());
        }
      for (unsigned int k= 0; k < P.nplayers(); k++)
        {
          if (k != P.whoami())
            {
              H.add(HA.HKi[t][k]);
              U.input(is[k]);
              if (x[t].get_value() == 0)
                {
                  U.assign_zero();
                }
              U.add(HA.HMi[t][k]);
              H.add(U);
              H.add(z[t].get_Key(k));
              H.add(z[t].get_MAC(k));
            }
        }
      Hv[t][P.whoami()]= H;
    }
  Commit_And_Open(Hv, P, true);

  // Step 7
  for (unsigned int t= 0; t < number; t++)
    {
      H.assign_zero();
      for (unsigned int i= 0; i < P.nplayers(); i++)
        {
          H.add(Hv[t][i]);
        }
      if (!H.is_zero())
        {
          throw OT_error();
        }
    }
  P.Check_Broadcast();
}
