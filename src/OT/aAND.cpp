/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "aAND.h"
#include "LSSS/PRSS.h"
#include "LaAND.h"
#include "Tools/int.h"
#include "aBit_Thread.h"
#include "config.h"

void aAND::make_aANDs(Player &P, int num_online)
{
  extern aBit_Data aBD;

  //P.clocks[1].reset(); P.clocks[1].start(); cout << "\tIn aAND" << endl;

  LA[0].make_more(P, num_online);

  unsigned int number= LA[0].get_number();
  triples.resize(number);

  int lg2number= CEIL_LOG2(number);
  unsigned int B= DIV_CEIL(OT_stat_sec, lg2number);
  LA.resize(B);
  for (unsigned int i= 1; i < B; i++)
    {
      LA[i].make_more(P, num_online);
    }

  // Step 2
  uint8_t seed[SEED_SIZE];
  AgreeRandom(P, seed, SEED_SIZE, 2);
  PRNG G2;
  G2.SetSeedFromRandom(seed);

  // To do the permutation of the objects we do a Knuth shuffle.
  unsigned int pos= 0, total= B * number;
  for (unsigned int i= 0; i < B; i++)
    {
      for (unsigned int j= 0; j < number && pos <= total - 2; j++)
        {
          unsigned int pos2= pos + G2.get_uint() % (total - pos);
          unsigned int j0= pos2 % number;
          unsigned int i0= (pos2 - j0) / number;
          swap(LA[i].triples[j], LA[i0].triples[j0]);
          pos++;
        }
    }

  // Step 3
  // We assign LA[0] into x,y,z
  // Then we add in LA[i], i=1,..,B checking as we go
  for (unsigned int i= 0; i < number; i++)
    {
      triples[i]= LA[0].triples[i];
    }
  vector<aBit> d(number);
  vector<int> dv(number);
  for (unsigned int i= 1; i < B; i++)
    {
      for (unsigned int t= 0; t < number; t++)
        {
          d[t].add(triples[t].y, LA[i].triples[t].y);
        }

      Open_aBits(dv, d, P);

      for (unsigned int t= 0; t < number; t++)
        {
          triples[t].x.add(LA[i].triples[t].x);
          triples[t].z.add(LA[i].triples[t].z);
          if (dv[t] == 1)
            {
              triples[t].z.add(LA[i].triples[t].x);
            }
        }
    }
  //P.clocks[1].stop(); cout << "\taAND " << P.clocks[1].elapsed() << endl;
}

/* This executes Beaver's protocol */
void Mult_aBits(vector<aBit> &z, const vector<aBit> &x, const vector<aBit> &y,
                list<aTriple> &T, Player &P)
{
  extern aBit_Data aBD;

  if (T.size() != x.size() || T.size() != y.size() || T.size() != z.size())
    {
      throw OT_error();
    }

  unsigned int sz= x.size();
  vector<aBit> a(sz), b(sz), ed(2 * sz);
  aTriple aT;
  for (unsigned int i= 0; i < sz; i++)
    {
      aT= T.front();
      T.pop_front();
      a[i]= aT.x;
      b[i]= aT.y;
      ed[i].add(a[i], x[i]);
      ed[sz + i].add(b[i], y[i]);
      z[i]= aT.z;
    }

  vector<int> eedd(2 * sz);
  Open_aBits(eedd, ed, P);

  for (unsigned int i= 0; i < sz; i++)
    {
      if (eedd[i] == 1)
        {
          z[i].add(b[i]);
        }
      if (eedd[sz + i] == 1)
        {
          z[i].add(a[i]);
          if (eedd[i] == 1)
            {
              z[i].negate();
            }
        }
    }
}

void Mult_aBit(aBit &z, const aBit &x, const aBit &y,
               aTriple &T, Player &P)
{
  extern aBit_Data aBD;

  vector<aBit> a(1), b(1), ed(2);
  ed[0].add(T.x, x);
  ed[1].add(T.y, y);
  z= T.z;

  vector<int> eedd(2);
  Open_aBits(eedd, ed, P);

  if (eedd[0] == 1)
    {
      z.add(T.y);
    }
  if (eedd[1] == 1)
    {
      z.add(T.x);
      if (eedd[0] == 1)
        {
          z.negate();
        }
    }
}
