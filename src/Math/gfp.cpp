/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Math/gfp.h"

#include "Exceptions/Exceptions.h"

Zp_Data gfp::ZpD;

void gfp::almost_randomize(PRNG &G)
{
  /*
  Why? The following e-mail explains:

  I've recently profiled the software for prime order fields, and
  I've noticed that 30 percent of the running time is spent on
  random number generation, which seems quite high to me.

  Recall that SPDZ2 checks openings a_1,...,a_t by checking a :=
  \sum_j r_j*a_j for random r_j. The random elements are
  currently generated as follows:
  1. Generate a random number with at least 2n bits (for n-bit p)
  2. Compute modulo p
  3. Convert to Montgomery

  I propose to generate as follows:
  1. Generate a random (n-1)-bit number (for n-bit p)
  2. Use directly as Montgomery representation

  This involves two changes, a theoretical and a practical one:
  - Theoretically, the resulting numbers are not uniformly
    distributed in GF(p) but in a subset more than half the
    size. This means that we lose less than 1 bit of security: We
    need that \sum_j r_j*(a_j-a'_j) = 0 only with negligible
    probability. Wlog, assume that a_1 != a'_1. Then r_1
    = (\sum_{j!=1} r_j*(a_j-a'_j)) / (a_1-a'_1), which happens
    with probability 2^(-n+1) if r_1 was chosen independently of
    r_2,...,r_t. This is very similar to batch verification by
    Bellare et al. (thanks to Nigel to pointing this out).
  - Practically, Montgomery conversion is unnecessary independent
    of the how the number was generated. The Montgomery
    conversion of an element chosen uniformly in a set is
    uniformly random distributed of a set of the same size
    because Montgomery conversion is invertible. If the set is
    the whole field, the resulting set is the whole field,
    otherwise the set changes, which doesn't matter here.
*/

  G.get_random_bytes((uint8_t *) a.x, t() * sizeof(mp_limb_t));
  a.x[t() - 1]&= ZpD.mask;
}

void gfp::AND(const gfp &x, const gfp &y)
{
  bigint bi1, bi2;
  to_bigint(bi1, x);
  to_bigint(bi2, y);
  mpz_and(bi1.get_mpz_t(), bi1.get_mpz_t(), bi2.get_mpz_t());
  to_gfp(*this, bi1);
}

void gfp::OR(const gfp &x, const gfp &y)
{
  bigint bi1, bi2;
  to_bigint(bi1, x);
  to_bigint(bi2, y);
  mpz_ior(bi1.get_mpz_t(), bi1.get_mpz_t(), bi2.get_mpz_t());
  to_gfp(*this, bi1);
}

void gfp::XOR(const gfp &x, const gfp &y)
{
  bigint bi1, bi2;
  to_bigint(bi1, x);
  to_bigint(bi2, y);
  mpz_xor(bi1.get_mpz_t(), bi1.get_mpz_t(), bi2.get_mpz_t());
  to_gfp(*this, bi1);
}

void gfp::AND(const gfp &x, const bigint &y)
{
  bigint bi;
  to_bigint(bi, x);
  mpz_and(bi.get_mpz_t(), bi.get_mpz_t(), y.get_mpz_t());
  to_gfp(*this, bi);
}

void gfp::OR(const gfp &x, const bigint &y)
{
  bigint bi;
  to_bigint(bi, x);
  mpz_ior(bi.get_mpz_t(), bi.get_mpz_t(), y.get_mpz_t());
  to_gfp(*this, bi);
}

void gfp::XOR(const gfp &x, const bigint &y)
{
  bigint bi;
  to_bigint(bi, x);
  mpz_xor(bi.get_mpz_t(), bi.get_mpz_t(), y.get_mpz_t());
  to_gfp(*this, bi);
}

void gfp::SHL(const gfp &x, int n)
{
  if (n < 0)
    {
      throw arithmetic_bug();
    }
  else if (n == 0)
    {
      a= x.a;
    }
  else if (!x.is_zero())
    {
      bigint bi;
      to_bigint(bi, x, false);
      mpn_lshift(bi.get_mpz_t()->_mp_d, bi.get_mpz_t()->_mp_d,
                 bi.get_mpz_t()->_mp_size, n);
      to_gfp(*this, bi);
    }
  else
    {
      assign_zero();
    }
}

void gfp::SHR(const gfp &x, int n)
{
  if (n < 0)
    {
      throw arithmetic_bug();
    }
  else if (n == 0)
    {
      a= x.a;
    }
  else if (!x.is_zero())
    {
      bigint bi;
      to_bigint(bi, x);
      mpn_rshift(bi.get_mpz_t()->_mp_d, bi.get_mpz_t()->_mp_d,
                 bi.get_mpz_t()->_mp_size, n);
      to_gfp(*this, bi);
    }
  else
    {
      assign_zero();
    }
}

void gfp::SHL(const gfp &x, const bigint &n)
{
  SHL(x, mpz_get_si(n.get_mpz_t()));
}

void gfp::SHR(const gfp &x, const bigint &n)
{
  SHR(x, mpz_get_si(n.get_mpz_t()));
}

gfp gfp::sqrRoot()
{
  // Temp move to bigint so as to call sqrRootMod
  bigint ti;
  to_bigint(ti, *this);
  ti= sqrRootMod(ti, ZpD.pr);
  if (!isOdd(ti))
    ti= ZpD.pr - ti;
  gfp temp;
  to_gfp(temp, ti);
  return temp;
}
