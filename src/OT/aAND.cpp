/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "aAND.h"
#include "LSSS/PRSS.h"
#include "LaAND.h"
#include "Tools/int.h"
#include "config.h"

void aAND::make_more(aBitFactory &aBF, Player &P)
{
  LA[0].make_more(aBF, P);
  unsigned int number= LA[0].get_number();
  x.resize(number);
  y.resize(number);
  z.resize(number);

  int lg2number= CEIL_LOG2(number);
  unsigned int B= DIV_CEIL(OT_stat_sec, lg2number);
  LA.resize(B);
  for (unsigned int i= 1; i < B; i++)
    {
      LA[i].make_more(aBF, P);
    }

  // Step 2
  uint8_t seed[SEED_SIZE];
  AgreeRandom(P, seed, SEED_SIZE);
  PRNG G2;
  G2.SetSeed(seed);

  // To do the permutation of the objects we
  // take each element and swap it with another one
  //   - That this is a good shuffle is well known
  aBit t;
  for (unsigned int i= 0; i < B; i++)
    {
      for (unsigned int j= 0; j < number; j++)
        {
          unsigned int i0= G2.get_uint() % B;
          unsigned int j0= G2.get_uint() % number;
          t= LA[i].x[j];
          LA[i].x[j]= LA[i0].x[j0];
          LA[i0].x[j0]= t;
          t= LA[i].y[j];
          LA[i].y[j]= LA[i0].y[j0];
          LA[i0].y[j0]= t;
          t= LA[i].z[j];
          LA[i].z[j]= LA[i0].z[j0];
          LA[i0].z[j0]= t;
        }
    }

  // Step 3
  // We assign LA[0] into x,y,z
  // Then we add in LA[i], i=1,..,B checking as we go
  for (unsigned int i= 0; i < number; i++)
    {
      x[i]= LA[0].x[i];
      y[i]= LA[0].y[i];
      z[i]= LA[0].z[i];
    }
  vector<aBit> d(number);
  vector<int> dv(number);
  for (unsigned int i= 1; i < B; i++)
    {
      for (unsigned int t= 0; t < number; t++)
        {
          d[t].add(y[t], LA[i].y[t]);
        }

      Open_aBits(dv, d, P, aBF.get_Delta());

      for (unsigned int t= 0; t < number; t++)
        {
          x[t].add(LA[i].x[t]);
          z[t].add(LA[i].z[t]);
          if (dv[t] == 1)
            {
              z[t].add(LA[i].x[t]);
            }
        }
    }
  used= 0;
}

void aAND::get_aAND(aBit &xx, aBit &yy, aBit &zz, aBitFactory &aBF, Player &P)
{
  if (used == x.size())
    {
      make_more(aBF, P);
    }
  xx= x[used];
  yy= y[used];
  zz= z[used];
  used++;
}

/* This executes Beaver's protocol */
void Mult_aBits(vector<aBit> &z, const vector<aBit> &x, const vector<aBit> &y,
                aAND &aA, aBitFactory &aBF, Player &P)
{
  unsigned int sz= x.size();
  vector<aBit> a(sz), b(sz), c(sz), d(sz), e(sz);
  for (unsigned int i= 0; i < sz; i++)
    {
      aA.get_aAND(a[i], b[i], c[i], aBF, P);
      d[i].add(a[i], x[i]);
      e[i].add(b[i], y[i]);
    }

  vector<int> dd(sz), ee(sz);
  Open_aBits(dd, d, P, aBF.get_Delta());
  Open_aBits(ee, e, P, aBF.get_Delta());

  for (unsigned int i= 0; i < sz; i++)
    {
      z[i]= c[i];
      if (dd[i] == 1)
        {
          z[i].add(b[i]);
        }
      if (ee[i] == 1)
        {
          z[i].add(a[i]);
        }
      if (dd[i] == 1 && ee[i] == 1)
        {
          z[i].negate();
        }
    }
}
