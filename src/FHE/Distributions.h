/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _Distributions
#define _Distributions

/* Classes to sample from a different distributions */

#include "FHE/Generator.h"
#include "Math/modp.h"
#include "Tools/random.h"
#include <vector>

// 1/2=0, 1/4=-1, 1/4=1
int sample_half(PRNG &G);

// Following is std dev = sigma
int sample_Gauss(PRNG &G);

class RandomGenerator : public Generator<bigint>
{
protected:
  mutable PRNG G;

public:
  RandomGenerator(PRNG &G)
  {
    this->G.SetSeed(G);
  }
};

class UniformGenerator : public RandomGenerator
{
  bigint B;

public:
  UniformGenerator(PRNG &G, const bigint &BB)
      : RandomGenerator(G), B(BB)
  {
  }

  Generator *clone() const
  {
    return new UniformGenerator(*this);
  }

  void get(bigint &x) const
  {
    x= randomBnd(G, 2 * B + 1) - B;
  }
};

class GaussianGenerator : public RandomGenerator
{

public:
  GaussianGenerator(PRNG &G)
      : RandomGenerator(G)
  {
  }

  Generator *clone() const
  {
    return new GaussianGenerator(*this);
  }

  void get(bigint &x) const
  {
    mpz_set_si(x.get_mpz_t(), sample_Gauss(G));
  }
};

class HalfGenerator : public RandomGenerator
{
public:
  HalfGenerator(PRNG &G)
      : RandomGenerator(G)
  {
  }

  Generator *clone() const
  {
    return new HalfGenerator(*this);
  }

  void get(bigint &x) const
  {
    mpz_set_si(x.get_mpz_t(), sample_half(G));
  }
};

#endif
