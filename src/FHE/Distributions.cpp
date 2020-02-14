/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Distributions.h"
#include "config.h"
#include "math.h"

int sample_half(PRNG &G)
{
  int v= G.get_uchar() & 3;
  if (v == 0 || v == 1)
    return 0;
  else if (v == 2)
    return 1;
  else
    return -1;
}

/* This uses the approximation to a Gaussian via 
 * binomial distribution 
 *  
 * This procedure consumes 2*NewHopeB bits, where NewHopeB
 * is defined in config.h
 */
int sample_Gauss(PRNG &G)
{
  int s= 0;
  uint8_t ss[(2 * NewHopeB + 7) / 8];
  G.get_random_bytes(ss, (2 * NewHopeB + 7) / 8);
  int cnt= 0;
  for (int j= 0; j < (2 * NewHopeB + 7) / 8; j++)
    {
      for (int k= 0; k < 4 && cnt < NewHopeB; k++)
        {
          s= s + (int) (ss[j] & 1);
          ss[j]= ss[j] >> 1;
          s= s - (int) (ss[j] & 1);
          ss[j]= ss[j] >> 1;
          cnt++;
        }
    }
  return s;
}
