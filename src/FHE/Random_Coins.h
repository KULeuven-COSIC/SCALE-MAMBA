/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _Random_Coins
#define _Random_Coins

/*  Randomness used to encrypt */

#include "FHE/FHE_Params.h"
#include "FHE/Rq_Element.h"

class FHE_PK;

class Random_Coins
{
  Rq_Element uu, vv, ww;
  const FHE_Params *params;

public:
  const FHE_Params &get_params() const
  {
    return *params;
  }

  Random_Coins(const FHE_Params &p)
      : uu(p.FFTD(), evaluation, evaluation),
        vv(p.FFTD(), evaluation, evaluation),
        ww(p.FFTD(), polynomial, polynomial)
  {
    params= &p;
  }

  Random_Coins(const FHE_PK &pk);

  // Rely on default copy assignment/constructor

  const Rq_Element &u() const
  {
    return uu;
  }
  const Rq_Element &v() const
  {
    return vv;
  }
  const Rq_Element &w() const
  {
    return ww;
  }

  void assign(const Rq_Element &u, const Rq_Element &v, const Rq_Element &w)
  {
    uu= u;
    vv= v;
    ww= w;
  }

  template<class T>
  void assign(const vector<T> &u, const vector<T> &v, const vector<T> &w)
  {
    uu.from_vec(u);
    vv.from_vec(v);
    ww.from_vec(w);
  }

  /* Generate a standard distribution */
  void generate(PRNG &G)
  {
    uu.from(HalfGenerator(G));
    vv.from(GaussianGenerator(G));
    ww.from(GaussianGenerator(G));
  }

  // Generate all from Uniform in range (-B,...B)
  void generateUniform(PRNG &G, const bigint &B1, const bigint &B2,
                       const bigint &B3)
  {
    if (B1 == 0)
      {
        uu.assign_zero();
      }
    else
      {
        uu.from(UniformGenerator(G, B1));
      }
    vv.from(UniformGenerator(G, B2));
    ww.from(UniformGenerator(G, B3));
  }

  // ans,x and y must have same params otherwise error
  friend void add(Random_Coins &ans, const Random_Coins &x,
                  const Random_Coins &y);

  friend void mul_by_X_i(Random_Coins &ans, const Random_Coins &a, unsigned int j)
  {
    mul_by_X_i(ans.uu, a.uu, j);
    mul_by_X_i(ans.vv, a.vv, j);
    mul_by_X_i(ans.ww, a.ww, j);
  }

  void output(ostream &s, bool human= false) const;

  // Input always in non-human form
  void input(istream &s);
};

#endif
