/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Math/gfp.h"

#include "Exceptions/Exceptions.h"

template<int L>
thread_local Zp_Data gfp_<L>::ZpD;


template<int L>
void gfp_<L>::init_field(const bigint& p, bool mont)
{
  ZpD.init(p, mont);
  string name = "gfp<" + to_string(L) + ">";
  if (ZpD.get_t() > L)
    {
      cout << "modulus is " << p << endl;
      throw wrong_gfp_size(name + " too small for modulus. "
              "Maybe change GFP_MOD_SZ to " + to_string(ZpD.get_t()));
    }
  if (ZpD.get_t() < L)
    {   
	printf("\n\n You may obtain additional performance for this case if you set MAX_GFP_SZ to %d\n",ZpD.get_t());
    }
}



template<int L>
void gfp_<L>::AND(const gfp_ &x, const gfp_ &y)
{
  bigint bi1, bi2;
  to_bigint(bi1, x);
  to_bigint(bi2, y);
  mpz_and(bi1.get_mpz_t(), bi1.get_mpz_t(), bi2.get_mpz_t());
  to_gfp(*this, bi1);
}

template<int L>
void gfp_<L>::OR(const gfp_ &x, const gfp_ &y)
{
  bigint bi1, bi2;
  to_bigint(bi1, x);
  to_bigint(bi2, y);
  mpz_ior(bi1.get_mpz_t(), bi1.get_mpz_t(), bi2.get_mpz_t());
  to_gfp(*this, bi1);
}

template<int L>
void gfp_<L>::XOR(const gfp_ &x, const gfp_ &y)
{
  bigint bi1, bi2;
  to_bigint(bi1, x);
  to_bigint(bi2, y);
  mpz_xor(bi1.get_mpz_t(), bi1.get_mpz_t(), bi2.get_mpz_t());
  to_gfp(*this, bi1);
}

template<int L>
void gfp_<L>::AND(const gfp_ &x, const bigint &y)
{
  bigint bi;
  to_bigint(bi, x);
  mpz_and(bi.get_mpz_t(), bi.get_mpz_t(), y.get_mpz_t());
  to_gfp(*this, bi);
}

template<int L>
void gfp_<L>::OR(const gfp_ &x, const bigint &y)
{
  bigint bi;
  to_bigint(bi, x);
  mpz_ior(bi.get_mpz_t(), bi.get_mpz_t(), y.get_mpz_t());
  to_gfp(*this, bi);
}

template<int L>
void gfp_<L>::XOR(const gfp_ &x, const bigint &y)
{
  bigint bi;
  to_bigint(bi, x);
  mpz_xor(bi.get_mpz_t(), bi.get_mpz_t(), y.get_mpz_t());
  to_gfp(*this, bi);
}

template<int L>
void gfp_<L>::SHL(const gfp_ &x, int n)
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

template<int L>
void gfp_<L>::SHR(const gfp_ &x, int n)
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

template<int L>
void gfp_<L>::SHL(const gfp_ &x, const bigint &n)
{
  SHL(x, mpz_get_si(n.get_mpz_t()));
}

template<int L>
void gfp_<L>::SHR(const gfp_ &x, const bigint &n)
{
  SHR(x, mpz_get_si(n.get_mpz_t()));
}

template<int L>
gfp_<L> gfp_<L>::sqrRoot()
{
  // Temp move to bigint so as to call sqrRootMod
  bigint ti;
  to_bigint(ti, *this);
  ti= sqrRootMod(ti, ZpD.pr);
  if (!isOdd(ti))
    ti= ZpD.pr - ti;
  gfp_ temp;
  to_gfp(temp, ti);
  return temp;
}
