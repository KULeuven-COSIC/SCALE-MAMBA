/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#include "modp.h"
#include "Zp_Data.h"

#include "Exceptions/Exceptions.h"

/***********************************************************************
 *  The following functions remain the same in Real and Montgomery rep *
 ***********************************************************************/

template <int L>
inline void assignZero(modp_<L> &x, const Zp_Data &ZpD)
{
  if (sizeof(x.x) <= 3 * 16)
    // use memset to allow the compiler to optimize
    // if x.x is at most 3*128 bits
    memset(x.x, 0, sizeof(x.x));
  else
    inline_mpn_zero(x.x, ZpD.t);  
}


template<int L>
inline void modp_<L>::randomize(PRNG &G, const Zp_Data &ZpD)
{
  bigint y= randomBnd(G, ZpD.pr);
  inline_mpn_zero(x, ZpD.t ); 
  inline_mpn_copyi(x, y.get_mpz_t()->_mp_d, y.get_mpz_t()->_mp_size);
}

template<int L>
inline void Negate(modp_<L> &ans, const modp_<L> &x, const Zp_Data &ZpD)
{
  if (isZero(x, ZpD))
    {
      ans= x;
      return;
    }
  mpn_sub_n(ans.x, ZpD.prA, x.x, ZpD.t);
}

template<int L>
inline bool areEqual(const modp_<L> &x, const modp_<L> &y, const Zp_Data &ZpD)
{
  if (mpn_cmp(x.x, y.x, ZpD.t) != 0)
    {
      return false;
    }
  return true;
}

template<int L>
inline bool isZero(const modp_<L> &ans, const Zp_Data &ZpD)
{
  for (int i= 0; i < ZpD.t; i++)
    {
      if (ans.x[i] != 0)
        {
          return false;
        }
    }
  return true;
}

/***********************************************************************
 *  All the remaining functions have Montgomery variants which we need *
 *  to deal with                                                       *
 ***********************************************************************/

template<int L>
inline void assignOne(modp_<L> &x, const Zp_Data &ZpD)
{
  if (ZpD.montgomery)
    {
      inline_mpn_copyi(x.x, ZpD.R, ZpD.t); 
    }
  else
    {
      assignZero(x, ZpD);
      x.x[0]= 1;
    }
}

template<int L>
inline bool isOne(const modp_<L> &x, const Zp_Data &ZpD)
{
  if (ZpD.montgomery)
    {
      if (mpn_cmp(x.x, ZpD.R, ZpD.t) != 0)
        {
          return false;
        }
    }
  else
    {
      if (x.x[0] != 1)
        {
          return false;
        }
      for (int i= 1; i < ZpD.t; i++)
        {
          if (x.x[i] != 0)
            {
              return false;
            }
        }
    }
  return true;
}

template<int L>
void modp_<L>::to_bigint(bigint &ans, const Zp_Data &ZpD, bool reduce) const
{
  auto& x = *this;
  mpz_ptr a = ans.get_mpz_t();
  if (a->_mp_alloc < ZpD.t)
     { mpz_realloc(a, ZpD.t); }
  if (ZpD.montgomery)
    {
      mp_limb_t one[MAX_MOD_SZ];
      inline_mpn_zero(one, ZpD.t); 
      one[0]= 1;
      ZpD.Mont_Mult(a->_mp_d, x.x, one);
    }
  else
    {
      inline_mpn_copyi(a->_mp_d, x.x, ZpD.t); 
    }
  a->_mp_size= ZpD.t;
  if (reduce)
    while (a->_mp_size >= 1 && (a->_mp_d)[a->_mp_size - 1] == 0)
      {
        a->_mp_size--;
      }
}


template<int L>
void to_bigint(bigint& ans,const modp_<L>& x,const Zp_Data& ZpD,bool reduce=true)
{
  x.to_bigint(ans, ZpD, reduce);
}


template<int L>
void to_modp(modp_<L> &ans, int x, const Zp_Data &ZpD)
{
  inline_mpn_zero(ans.x, ZpD.t ); 
  if (x >= 0)
    {
      ans.x[0]= x;
      if (ZpD.t == 1)
        {
          ans.x[0]= ans.x[0] % ZpD.prA[0];
        }
    }
  else
    {
      if (ZpD.t == 1)
        {
          ans.x[0]= (ZpD.prA[0] + x) % ZpD.prA[0];
        }
      else
        {
          bigint xx= ZpD.pr + x;
          to_modp(ans, xx, ZpD);
          return;
        }
    }
  if (ZpD.montgomery)
    {
      ZpD.Mont_Mult(ans.x, ans.x, ZpD.R2);
    }
}

template<int L>
inline void to_modp(modp_<L> &ans, const bigint &x, const Zp_Data &ZpD)
{
  bigint xx(x);
  ans.to_modp(xx, ZpD);
}

template<int L>
void modp_<L>::to_modp(const bigint &source, const Zp_Data &ZpD)
{
  bigint xx= source % ZpD.pr;
  if (xx < 0)
    {
      xx+= ZpD.pr;
    }
  // mpz_mod(xx.get_mpz_t(),x.get_mpz_t(),ZpD.pr.get_mpz_t());
  inline_mpn_zero(x, ZpD.t);
  inline_mpn_copyi(x, xx.get_mpz_t()->_mp_d, xx.get_mpz_t()->_mp_size);
  if (ZpD.montgomery)
    {
      ZpD.Mont_Mult(x, x, ZpD.R2);
    }
}


template<int L>
void Mul(modp_<L> &ans, const modp_<L> &x, const modp_<L> &y, const Zp_Data &ZpD)
{
  if (ZpD.montgomery)
    {
      ZpD.Mont_Mult(ans.x, x.x, y.x);
    }
  else
    { // ans.x=(x.x*y.x)%ZpD.pr;
      mp_limb_t aa[2 * MAX_MOD_SZ], q[2 * MAX_MOD_SZ];
      mpn_mul_n(aa, x.x, y.x, ZpD.t);
      mpn_tdiv_qr(q, ans.x, 0, aa, 2 * ZpD.t, ZpD.prA, ZpD.t);
    }
}


template<int L>
inline void modp_<L>::mul(const modp_<L>& x, const modp_<L>& y, const Zp_Data& ZpD)
{
  if (ZpD.montgomery)
    { ZpD.Mont_Mult(this->x, x.x, y.x); }
  else
    { Mul<L>(*this, x, y, ZpD); }
}

template<int L>
void Sqr(modp_<L> &ans, const modp_<L> &x, const Zp_Data &ZpD)
{
  if (ZpD.montgomery)
    {
      ZpD.Mont_Mult(ans.x, x.x, x.x);
    }
  else
    { // ans.x=(x.x*x.x)%ZpD.pr;
      mp_limb_t aa[2 * MAX_MOD_SZ], q[2 * MAX_MOD_SZ];
      mpn_sqr(aa, x.x, ZpD.t);
      mpn_tdiv_qr(q, ans.x, 0, aa, 2 * ZpD.t, ZpD.prA, ZpD.t);
    }
}

template<int L>
void Inv(modp_<L> &ans, const modp_<L> &x, const Zp_Data &ZpD)
{
  mp_limb_t g[MAX_MOD_SZ], xx[MAX_MOD_SZ + 1], yy[MAX_MOD_SZ + 1];
  mp_size_t sz;
  inline_mpn_copyi(xx, x.x, ZpD.t);
  inline_mpn_copyi(yy, ZpD.prA, ZpD.t);
  mpn_gcdext(g, ans.x, &sz, xx, ZpD.t, yy, ZpD.t);
  if (sz < 0)
    {
      mpn_sub(ans.x, ZpD.prA, ZpD.t, ans.x, -sz);
      sz= -sz;
    }
  else
    {
      for (int i= sz; i < ZpD.t; i++)
        {
          ans.x[i]= 0;
        }
    }
  if (ZpD.montgomery)
    {
      ZpD.Mont_Mult(ans.x, ans.x, ZpD.R3);
    }
}

// XXXX This is a crap version. Hopefully this is not time critical
template<int L>
void Power(modp_<L> &ans, const modp_<L> &x, int exp, const Zp_Data &ZpD)
{
  if (exp == 1)
    {
      ans= x;
      return;
    }
  if (exp == 0)
    {
      assignOne(ans, ZpD);
      return;
    }
  if (exp < 0)
    {
      throw not_implemented();
    }
  modp_<L> t= x;
  assignOne(ans, ZpD);
  while (exp != 0)
    {
      if ((exp & 1) == 1)
        {
          Mul(ans, ans, t, ZpD);
        }
      exp>>= 1;
      Sqr(t, t, ZpD);
    }
}

// XXXX This is a crap version. Hopefully this is not time critical
template<int L>
void Power(modp_<L> &ans, const modp_<L> &x, const bigint &exp, const Zp_Data &ZpD)
{
  if (exp == 1)
    {
      ans= x;
      return;
    }
  if (exp == 0)
    {
      assignOne(ans, ZpD);
      return;
    }
  if (exp < 0)
    {
      throw not_implemented();
    }
  modp_<L> t= x;
  assignOne(ans, ZpD);
  bigint e= exp;
  while (e != 0)
    {
      if ((e & 1) == 1)
        {
          Mul(ans, ans, t, ZpD);
        }
      e>>= 1;
      Sqr(t, t, ZpD);
    }
}

template<int L>
void modp_<L>::output(ostream &s, const Zp_Data &ZpD, bool human) const
{
  if (human)
    {
      bigint te;
      to_bigint(te, ZpD);
      if (te < ZpD.pr / 2)
        s << te;
      else
        s << (te - ZpD.pr);
    }
  else
    {
      s.write((char *) x, ZpD.t * sizeof(mp_limb_t));
    }
}

template<int L>
void modp_<L>::input(istream &s, const Zp_Data &ZpD, bool human)
{
  if (s.peek() == EOF)
    {
      if (s.tellg() == 0)
        {
          cout << "IO problem. Empty file?" << endl;
          throw file_error("Input Mod p");
        }
      throw file_error("Input Mod p");
    }

  if (human)
    {
      bigint te;
      s >> te;
      to_modp(te, ZpD);
    }
  else
    {
      s.read((char *) x, ZpD.t * sizeof(mp_limb_t));
    }
}
