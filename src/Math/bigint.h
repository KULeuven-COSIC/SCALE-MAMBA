/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _bigint
#define _bigint

#include <iostream>
using namespace std;

#include <mpirxx.h>
#include <stddef.h>

typedef mpz_class bigint;

#include "Exceptions/Exceptions.h"
#include "Tools/int.h"
#include "Tools/random.h"

/**********************************
 *       Utility Functions        *
 **********************************/

inline bigint div_f(const bigint n, const bigint d)
{
  bigint q;
  mpz_fdiv_q(q.get_mpz_t(), n.get_mpz_t(), d.get_mpz_t());
  return q;
}

inline bigint div_c(const bigint n, const bigint d)
{
  bigint q;
  mpz_cdiv_q(q.get_mpz_t(), n.get_mpz_t(), d.get_mpz_t());
  return q;
}

inline bigint mod(const bigint n, const bigint d)
{
  bigint q;
  mpz_mod(q.get_mpz_t(), n.get_mpz_t(), d.get_mpz_t());
  return q;
}

inline int gcd(const int x, const int y)
{
  bigint xx= x;
  return mpz_gcd_ui(NULL, xx.get_mpz_t(), y);
}

inline bigint gcd(const bigint &x, const bigint &y)
{
  bigint g;
  mpz_gcd(g.get_mpz_t(), x.get_mpz_t(), y.get_mpz_t());
  return g;
}

inline void invMod(bigint &ans, const bigint &x, const bigint &p)
{
  mpz_invert(ans.get_mpz_t(), x.get_mpz_t(), p.get_mpz_t());
}

inline int numBits(const bigint &m) { return mpz_sizeinbase(m.get_mpz_t(), 2); }

inline int numBits(int m)
{
  bigint te= m;
  return mpz_sizeinbase(te.get_mpz_t(), 2);
}

inline int numBytes(const bigint &m)
{
  return mpz_sizeinbase(m.get_mpz_t(), 256);
}

inline int probPrime(const bigint &x)
{
  gmp_randstate_t rand_state;
  gmp_randinit_default(rand_state);
  int ans= mpz_probable_prime_p(x.get_mpz_t(), rand_state, 40, 0);
  gmp_randclear(rand_state);
  return ans;
}

inline void encode_length(uint8_t *buff, int len)
{
  if (len < 0)
    {
      throw invalid_length();
    }
  INT_TO_BYTES(buff, len);
}

inline int decode_length(uint8_t *buff)
{
  int len= BYTES_TO_INT(buff);
  if (len < 0)
    {
      throw invalid_length();
    }
  return len;
}

inline void bigintFromBytes(bigint &x, uint8_t *bytes, int len)
{
  mpz_import(x.get_mpz_t(), len, 1, sizeof(uint8_t), 0, 0, bytes);
}

inline void bytesFromBigint(uint8_t *bytes, const bigint &x, unsigned int len)
{
  size_t ll;
  mpz_export(bytes, &ll, 1, sizeof(uint8_t), 0, 0, x.get_mpz_t());
  if (ll > len)
    {
      throw invalid_length();
    }
  for (unsigned int i= ll; i < len; i++)
    {
      bytes[i]= 0;
    }
}

void outputBigint(string &s, const bigint &x);
void inputBigint(string &s, bigint &x);

inline int isOdd(const bigint &x) { return mpz_odd_p(x.get_mpz_t()); }

bigint sqrRootMod(const bigint &x, const bigint &p);

bigint powerMod(const bigint &x, const bigint &e, const bigint &p);

// Assume e>=0
int powerMod(int x, int e, int p);

inline int Hwt(int N)
{
  int result= 0;
  while (N)
    {
      result++;
      N&= (N - 1);
    }
  return result;
}

bigint randomBnd(PRNG &G, const bigint &B);

bigint compute_binomial(int n, int k);

void bigint_to_bits(vector<int> &bits, const bigint &x);
void ulong_to_bits(vector<int> &bits, unsigned long x);
void long_to_bits(vector<int> &bits, long x);

#endif
