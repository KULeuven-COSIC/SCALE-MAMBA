/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Random_Coins.h"
#include "FHE_Keys.h"

Random_Coins::Random_Coins(const FHE_PK &pk)
    : Random_Coins(pk.get_params()) {}

void add(Random_Coins &ans, const Random_Coins &x, const Random_Coins &y)
{
  if (x.params != y.params)
    {
      throw data_mismatch();
    }
  if (ans.params != y.params)
    {
      throw data_mismatch();
    }

  add(ans.uu, x.uu, y.uu);
  add(ans.vv, x.vv, y.vv);
  add(ans.ww, x.ww, y.ww);
}

void Random_Coins::output(ostream &s, bool human) const
{
  if (human)
    {
      s << "u = ";
      uu.output(s, true);
      s << endl;
      s << "v = ";
      vv.output(s, true);
      s << endl;
      s << "w = ";
      ww.output(s, true);
      s << endl;
    }
  else
    {
      uu.output(s);
      vv.output(s);
      ww.output(s);
    }
}

void Random_Coins::input(istream &s)
{
  uu.input(s);
  vv.input(s);
  ww.input(s);
}
