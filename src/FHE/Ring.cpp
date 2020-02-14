/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Ring.h"
#include "Exceptions/Exceptions.h"
#include "Math/bigint.h"
#include "Tools/int.h"

void Ring::initialize(unsigned int m)
{
  int lg2= CEIL_LOG2(m);
  if (m != (1UL << lg2))
    {
      fprintf(stderr, "Ring dimension must be a power of two");
      throw not_implemented();
    }
  mm= m;
  phim= m / 2;

  pi.resize(phim);
  pi_inv.resize(mm);
  for (unsigned int i= 0; i < mm; i++)
    {
      pi_inv[i]= -1;
    }

  int k= 0;
  for (unsigned int i= 1; i < mm; i++)
    {
      if (gcd(i, mm) == 1)
        {
          pi[k]= i;
          pi_inv[i]= k;
          k++;
        }
    }
}

ostream &operator<<(ostream &s, const Ring &R)
{
  s << R.mm << " " << R.phim << endl;
  for (unsigned int i= 0; i < R.phim; i++)
    {
      s << R.pi[i] << " ";
    }
  s << endl;
  for (unsigned int i= 0; i < R.mm; i++)
    {
      s << R.pi_inv[i] << " ";
    }
  s << endl;
  return s;
}

istream &operator>>(istream &s, Ring &R)
{
  if (s.fail())
    {
      throw IO_Error("can't read ring");
    }
  s >> R.mm >> R.phim;
  R.pi.resize(R.phim);
  R.pi_inv.resize(R.mm);
  for (unsigned int i= 0; i < R.phim; i++)
    {
      s >> R.pi[i];
    }
  for (unsigned int i= 0; i < R.mm; i++)
    {
      s >> R.pi_inv[i];
    }
  return s;
}

bool Ring::operator!=(const Ring &other) const
{
  if (mm != other.mm or phim != other.phim or pi != other.pi or
      pi_inv != other.pi_inv or poly != other.poly)
    return true;
  else
    return false;
}
