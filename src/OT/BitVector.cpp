/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "OT/BitVector.h"
#include "Math/gf2n.h"
#include "Tools/random.h"

#include <fstream>

void BitVector::randomize(PRNG &G)
{
  G.get_random_bytes(bytes, nbytes);
}

void BitVector::randomize_at(int a, int nb, PRNG &G)
{
  if (nb < 1)
    throw invalid_length();
  G.get_random_bytes(bytes + a, nb);
}

/*
 * TODO: test input/output functions
 */

void BitVector::output(ostream &s, bool human) const
{
  if (human)
    {
      s << nbits << " " << hex;
      for (unsigned int i= 0; i < nbytes; i++)
        {
          s << int(bytes[i]) << " ";
        }
      s << dec << endl;
    }
  else
    {
      int len= nbits;
      s.write((char *) &len, sizeof(int));
      s.write((char *) bytes, nbytes);
    }
}

void BitVector::input(istream &s, bool human)
{
  int len;
  if (human)
    {
      s >> len >> hex;
      resize(len);
      for (size_t i= 0; i < nbytes; i++)
        {
          s >> bytes[i];
        }
      s >> dec;
    }
  else
    {
      s.read((char *) &len, sizeof(int));
      resize(len);
      s.read((char *) bytes, nbytes);
    }
}
