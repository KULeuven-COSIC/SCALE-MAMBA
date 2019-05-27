/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2019, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "aAND_Factory.h"

aTriple aAND_Factory::get_aAND(Player &P)
{
  if (aANDs.size() == 0)
    {
      make_more(P);
    }

  aTriple ans= aANDs.front();
  aANDs.pop_front();

  return ans;
}

list<aTriple> aAND_Factory::get_aANDs(unsigned int num, Player &P)
{
  while (aANDs.size() < num)
    {
      make_more(P);
    }

  list<aTriple> ans;
  ans.splice(ans.begin(), aANDs, aANDs.begin(), next(aANDs.begin(), num));
  return ans;
}

void aAND_Factory::make_more(Player &P)
{
  aAF.make_aANDs(P, num_online);
  aAF.copy_aANDs(aANDs);
}
