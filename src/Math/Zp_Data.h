/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _Zp_Data
#define _Zp_Data

/* Class to define helper information for a Zp element
 *
 * Basically the data needed for Montgomery operations
 *
 * Almost all data is public as this is basically a container class
 *
 */

#include "Math/bigint.h"
#include "Tools/random.h"

#include <iostream>
#include <smmintrin.h>
using namespace std;

/* If MAX_MOD_SZ/MAX_GFP_SZ is not defined then something is wrong in the compilation.
 * Not having a default value helps application developers which are including
 * this file as well as linking against the library
 *   - Since the two values should be identical
 */
#ifndef MAX_MOD_SZ
#error "MAX_MOD_SZ is not defined."
#endif

#ifndef MAX_GFP_SZ
#error "MAX_GFP_SZ is not defined."
#endif

#if MAX_GFP_SZ > MAX_MOD_SZ
#error MAX_GFP_SZ must be at most MAX_MOD_SZ
#endif

template<int L>
class modp_;

class Zp_Data
{
  bool montgomery; // True if we are using Montgomery arithmetic
  mp_limb_t R[MAX_MOD_SZ], R2[MAX_MOD_SZ], R3[MAX_MOD_SZ], pi;
  mp_limb_t prA[MAX_MOD_SZ + 1];
  int t; // More Montgomery data

  // Marcel's trick for MontMult...
  void Mont_Mult_Normal(mp_limb_t *z, const mp_limb_t *x, const mp_limb_t *y) const;
  template<int T>
  void Mont_Mult_(mp_limb_t *z, const mp_limb_t *x, const mp_limb_t *y) const;
  void Mont_Mult(mp_limb_t *z, const mp_limb_t *x, const mp_limb_t *y) const;

public:
  bigint pr;

  void assign(const Zp_Data &Zp);
  void init(const bigint &p, bool mont= true);
  int get_t() const
  {
    return t;
  }
  const mp_limb_t *get_prA() const
  {
    return prA;
  }

  // This one does nothing, needed so as to make vectors of Zp_Data
  Zp_Data()
  {
    t= 1;
  }

  // The main init funciton
  Zp_Data(const bigint &p, bool mont= true)
  {
    init(p, mont);
  }

  // Copy constructor
  Zp_Data(const Zp_Data &Zp)
  {
    assign(Zp);
  }
  // Move constructor
  Zp_Data(Zp_Data &&Zp)
  {
    assign(Zp);
  }
  // Copy assignment
  Zp_Data &operator=(const Zp_Data &Zp)
  {
    if (this != &Zp)
      {
        assign(Zp);
      }
    return *this;
  }
  // Move assignment
  Zp_Data &operator=(Zp_Data &&Zp)
  {
    if (this != &Zp)
      {
        assign(Zp);
      }
    return *this;
  }
  ~Zp_Data()
  {
    ;
  }

  template<int T>
  void Add(mp_limb_t *ans, const mp_limb_t *x, const mp_limb_t *y) const;
  void Add(mp_limb_t *ans, const mp_limb_t *x, const mp_limb_t *y) const;

  template<int T>
  void Sub(mp_limb_t *ans, const mp_limb_t *x, const mp_limb_t *y) const;
  void Sub(mp_limb_t *ans, const mp_limb_t *x, const mp_limb_t *y) const;

  template<int L>
  friend void to_modp(modp_<L> &ans, int x, const Zp_Data &ZpD);
  template<int L>
  friend void to_modp(modp_<L> &ans, const bigint &x, const Zp_Data &ZpD);

  template<int L>
  friend void Add(modp_<L> &ans, const modp_<L> &x, const modp_<L> &y, const Zp_Data &ZpD);
  template<int L>
  friend void Sub(modp_<L> &ans, const modp_<L> &x, const modp_<L> &y, const Zp_Data &ZpD);
  template<int L>
  friend void Mul(modp_<L> &ans, const modp_<L> &x, const modp_<L> &y, const Zp_Data &ZpD);
  template<int L>
  friend void Sqr(modp_<L> &ans, const modp_<L> &x, const Zp_Data &ZpD);
  template<int L>
  friend void Negate(modp_<L> &ans, const modp_<L> &x, const Zp_Data &ZpD);
  template<int L>
  friend void Inv(modp_<L> &ans, const modp_<L> &x, const Zp_Data &ZpD);

  template<int L>
  friend void Power(modp_<L> &ans, const modp_<L> &x, int exp, const Zp_Data &ZpD);
  template<int L>
  friend void Power(modp_<L> &ans, const modp_<L> &x, const bigint &exp,
                    const Zp_Data &ZpD);

  template<int L>
  friend void assignOne(modp_<L> &x, const Zp_Data &ZpD);
  template<int L>
  friend void assignZero(modp_<L> &x, const Zp_Data &ZpD);
  template<int L>
  friend bool isZero(const modp_<L> &x, const Zp_Data &ZpD);
  template<int L>
  friend bool isOne(const modp_<L> &x, const Zp_Data &ZpD);
  template<int L>
  friend bool areEqual(const modp_<L> &x, const modp_<L> &y, const Zp_Data &ZpD);

  template<int L>
  friend class modp_;

  friend ostream &operator<<(ostream &s, const Zp_Data &ZpD);
  friend istream &operator>>(istream &s, Zp_Data &ZpD);
};

template<>
inline void Zp_Data::Add<0>(mp_limb_t *ans, const mp_limb_t *x, const mp_limb_t *y) const
{
  mp_limb_t carry= mpn_add_n_with_carry(ans, x, y, t);
  if (carry != 0 || mpn_cmp(ans, prA, t) >= 0)
    {
      mpn_sub_n_borrow(ans, ans, prA, t);
    }
}

template<>
inline void Zp_Data::Add<1>(mp_limb_t *ans, const mp_limb_t *x, const mp_limb_t *y) const
{
#ifdef __clang__
  Add<0>(ans, x, y);
#else
  *ans= *x + *y;
  asm goto("jc %l[sub]" :: ::sub);
  if (*ans >= *prA)
  sub:
    *ans-= *prA;
#endif
}

template<>
inline void Zp_Data::Add<2>(mp_limb_t *ans, const mp_limb_t *x, const mp_limb_t *y) const
{
#ifdef __clang__
  Add<0>(ans, x, y);
#else
  __uint128_t a, b, p;
  memcpy(&a, x, sizeof(__uint128_t));
  memcpy(&b, y, sizeof(__uint128_t));
  memcpy(&p, prA, sizeof(__uint128_t));
  __uint128_t c= a + b;
  asm goto("jc %l[sub]" :: ::sub);
  if (c >= p)
  sub:
    c-= p;
  memcpy(ans, &c, sizeof(__uint128_t));
#endif
}

template<int T>
inline void Zp_Data::Add(mp_limb_t *ans, const mp_limb_t *x, const mp_limb_t *y) const
{
  mp_limb_t carry= mpn_add_fixed_n_with_carry<T>(ans, x, y);
  if (carry != 0 || mpn_cmp(ans, prA, T) >= 0)
    {
      mpn_sub_n_borrow(ans, ans, prA, T);
    }
}

inline void Zp_Data::Add(mp_limb_t *ans, const mp_limb_t *x, const mp_limb_t *y) const
{
  switch (t)
    {
      case 4:
        return Add<4>(ans, x, y);
      case 2:
        return Add<2>(ans, x, y);
      case 1:
        return Add<1>(ans, x, y);
      default:
        return Add<0>(ans, x, y);
    }
}

template<int T>
inline void Zp_Data::Sub(mp_limb_t *ans, const mp_limb_t *x, const mp_limb_t *y) const
{
  mp_limb_t tmp[T];
  mp_limb_t borrow= mpn_sub_fixed_n_borrow<T>(tmp, x, y);
  if (borrow != 0)
    mpn_add_fixed_n<T>(ans, tmp, prA);
  else
    inline_mpn_copyi(ans, tmp, T);
}

template<>
inline void Zp_Data::Sub<0>(mp_limb_t *ans, const mp_limb_t *x, const mp_limb_t *y) const
{
  mp_limb_t borrow= mpn_sub_n_borrow(ans, x, y, t);
  if (borrow != 0)
    {
      mpn_add_n_with_carry(ans, ans, prA, t);
    }
}

inline void Zp_Data::Sub(mp_limb_t *ans, const mp_limb_t *x, const mp_limb_t *y) const
{
  switch (t)
    {
      /*
      case 2:
        Sub<2>(ans, x, y);
        break;
      case 1:
        Sub<1>(ans, x, y);
        break;
       */
      default:
        Sub<0>(ans, x, y);
        break;
    }
}

template<int T>
inline void Zp_Data::Mont_Mult_(mp_limb_t *z, const mp_limb_t *x, const mp_limb_t *y) const
{
#ifdef __BMI2__
  mp_limb_t ans[2 * MAX_MOD_SZ + 1], u;
  inline_mpn_zero(ans + T + 1, T);
  // First loop
  u= x[0] * y[0] * pi;
  mpn_mul_1_fixed<T + 1, T>(ans, y, x[0]);
  mpn_addmul_1_fixed_<T + 2, T + 1>(ans, prA, u);
  for (int i= 1; i < T; i++)
    { // u=(ans0+xi*y0)*pd
      u= (ans[i] + x[i] * y[0]) * pi;
      // ans=ans+xi*y+u*pr
      mpn_addmul_1_fixed_<T + 1, T>(ans + i, y, x[i]);
      mpn_addmul_1_fixed_<T + 2, T + 1>(ans + i, prA, u);
    }
  // if (ans>=pr) { ans=z-pr; }
  // else         { z=ans;    }
  if (mpn_cmp(ans + T, prA, T + 1) >= 0)
    {
      mpn_sub_fixed_n<T>(z, ans + T, prA);
    }
  else
    {
      inline_mpn_copyi(z, ans + T, T);
    }
#else
  Mont_Mult_Normal(z, x, y);
#endif
}

inline void Zp_Data::Mont_Mult(mp_limb_t *z, const mp_limb_t *x, const mp_limb_t *y) const
{
  switch (t)
    {
#ifdef __BMI2__
#define CASE(N)             \
  case N:                   \
    Mont_Mult_<N>(z, x, y); \
    break;
      CASE(1)
      CASE(2)
#if MAX_MOD_SZ >= 3
      CASE(3)
      CASE(4)
      CASE(5)
#endif
#undef CASE
#endif
      default:
        Mont_Mult_Normal(z, x, y);
        break;
    }
}

#endif
