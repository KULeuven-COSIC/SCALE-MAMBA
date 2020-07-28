/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "LaAND.h"
#include "HaAND.h"
#include "OT_Thread_Data.h"
#include "Tools/Crypto.h"

void LaAND::make_more(Player &P, int num_online)
{
  extern OT_Thread_Data OTD;

  //P.clocks[2].reset(); P.clocks[2].start(); cout << "\t\tIn LaAND" << endl;
  // First steps 1,2,3
  HaAND HA;
  HA.make_more(P, num_online);
  unsigned int number= HA.get_number();

  triples.resize(number);

  gf2n Delta= aBit::get_Delta();

  list<aBit> get_r= OTD.aBD.get_aShares(num_online, number);
  vector<string> o(P.nplayers());
  ostringstream os;
  unsigned int t= 0;
  for (list<aBit>::const_iterator zz= get_r.begin(); zz != get_r.end(); ++zz)
    {
      triples[t].x= HA.x[t];
      triples[t].y= HA.y[t];
      triples[t].z= *zz;
      int zt= (triples[t].x.get_value() * triples[t].y.get_value()) ^ HA.v[t];
      int et= zt ^ triples[t].z.get_value();
      os << et << " ";
      // Sets my z value to z+e
      triples[t].z.set_value(triples[t].z.get_value() ^ et);
      t++;
    }

  o[P.whoami()]= os.str();
  P.Broadcast_Receive(o, true, 2);
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
                { // Tweak the key (If there is any bug this is where it is)
                  // As we tweaked the z value above
                  nk.add(triples[t].z.get_Key(i), Delta);
                  triples[t].z.set_Key(i, nk);
                }
            }
        }
    }

  // Step 4
  vector<gf2n> Psi(number);
  for (unsigned int t= 0; t < number; t++)
    {
      Psi[t].assign_zero();
      if (triples[t].y.get_value() == 1)
        {
          Psi[t]= Delta;
        }
      for (unsigned int k= 0; k < P.nplayers(); k++)
        {
          if (k != P.whoami())
            {
              Psi[t].add(triples[t].y.get_Key(k));
              Psi[t].add(triples[t].y.get_MAC(k));
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
  P.Send_Distinct_And_Receive(o, 2);

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
      if (triples[t].x.get_value() == 1)
        {
          H= Psi[t];
        }
      if (triples[t].z.get_value() == 1)
        {
          H.add(Delta);
        }
      for (unsigned int k= 0; k < P.nplayers(); k++)
        {
          if (k != P.whoami())
            {
              H.add(HA.HKi[t][k]);
              U.input(is[k]);
              if (triples[t].x.get_value() == 0)
                {
                  U.assign_zero();
                }
              U.add(HA.HMi[t][k]);
              H.add(U);
              H.add(triples[t].z.get_Key(k));
              H.add(triples[t].z.get_MAC(k));
            }
        }
      Hv[t][P.whoami()]= H;
    }
  Commit_And_Open(Hv, P, true, 2);

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
  P.Check_Broadcast(2);
  //P.clocks[2].stop(); cout << "\t\tLaAND " << P.clocks[2].elapsed() << endl;
}

/* This is a routine for DEBUG purposes only */
void check_triples(Player &P, const vector<aTriple> &ee)
{
  unsigned int sz= ee.size();
  vector<int> x(sz), y(sz), z(sz);
  vector<aBit> xB(sz), yB(sz), zB(sz);
  for (unsigned int i= 0; i < sz; i++)
    {
      xB[i]= ee[i].x;
      yB[i]= ee[i].y;
      zB[i]= ee[i].z;
    }

  Open_aBits(x, xB, P);
  Open_aBits(y, yB, P);
  Open_aBits(z, zB, P);

  for (unsigned int i= 0; i < sz; i++)
    {
      int zz= x[i] * y[i];
      if (zz != z[i])
        {
          throw OT_error();
        }
    }
}
