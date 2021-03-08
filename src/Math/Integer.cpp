/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

/*
 * Integer.cpp
 *
 */

#include "Integer.h"

void Integer::output(ostream &s, bool human) const
{
  if (human)
    s << a;
  else
    s.write((char *) &a, sizeof(a));
}

void Integer::input(istream &s, bool human)
{
  if (human)
    s >> a;
  else
    s.read((char *) &a, sizeof(a));
}
