/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _gfp
#define _gfp

#include <iostream>
using namespace std;
#include "Math/Zp_Data.h"
#include "Math/modp.h"

/* This is a wrapper class for the modp data type
 *
 * gfp is used ONLY for the stuff in the finite fields
 * we are going to be doing MPC over, not the modp stuff
 * for any FHE scheme etc
 */

class gfp
{
  modp a;
  thread_local static Zp_Data ZpD;

public:
  static void init_field(const bigint &p, bool mont= true)
  {
    ZpD.init(p, mont);
  }
  static bigint pr()
  {
    return ZpD.pr;
  }
  static int t()
  {
    return ZpD.get_t();
  }
  static Zp_Data &get_ZpD()
  {
    return ZpD;
  }
  static string type_string()
  {
    return "gfp";
  }

  static int size()
  {
    return t() * sizeof(mp_limb_t);
  }

  void assign_zero()
  {
    assignZero(a, ZpD);
  }
  void assign_one()
  {
    assignOne(a, ZpD);
  }
  void assign(word aa)
  {
    bigint b= aa;
    to_gfp(*this, b);
  }
  void assign(long aa)
  {
    bigint b= aa;
    to_gfp(*this, b);
  }
  void assign(int aa)
  {
    bigint b= aa;
    to_gfp(*this, b);
  }
  void assign(const bigint &aa)
  {
    to_gfp(*this, aa);
  }
  void assign(const char *buffer)
  {
    a.assign(buffer, ZpD.get_t());
  }

  modp get() const
  {
    return a;
  }

  // Assumes prD behind x is equal to ZpD
  void assign(modp &x)
  {
    a= x;
  }

  gfp()
  {
    assignZero(a, ZpD);
  }
  // Assumes prD behind x is equal to ZpD
  gfp(const modp &g)
  {
    a= g;
  }
  gfp(const bigint &x)
  {
    to_modp(a, x, ZpD);
  }
  gfp(int x)
  {
    assign(x);
  }

  bool is_zero() const
  {
    return isZero(a, ZpD);
  }
  bool is_one() const
  {
    return isOne(a, ZpD);
  }
  bool is_bit() const
  {
    return is_zero() or is_one();
  }

  bool equal(const gfp &y) const
  {
    return areEqual(a, y.a, ZpD);
  }
  bool operator==(const gfp &y) const
  {
    return equal(y);
  }
  bool operator!=(const gfp &y) const
  {
    return !equal(y);
  }

  // x+y
  void add(const gfp &x, const gfp &y)
  {
    Add(a, x.a, y.a, ZpD);
  }
  void add(const gfp &x)
  {
    Add(a, a, x.a, ZpD);
  }
  void add(void *x)
  {
    ZpD.Add(a.x, a.x, (mp_limb_t *) x);
  }
  void sub(const gfp &x, const gfp &y)
  {
    Sub(a, x.a, y.a, ZpD);
  }
  void sub(const gfp &x)
  {
    Sub(a, a, x.a, ZpD);
  }

  // = x * y
  void mul(const gfp &x, const gfp &y)
  {
    Mul(a, x.a, y.a, ZpD);
  }
  void mul(const gfp &x)
  {
    Mul(a, a, x.a, ZpD);
  }

  gfp operator+(const gfp &x) const
  {
    gfp res;
    res.add(*this, x);
    return res;
  }
  gfp operator-(const gfp &x) const
  {
    gfp res;
    res.sub(*this, x);
    return res;
  }
  gfp operator*(const gfp &x) const
  {
    gfp res;
    res.mul(*this, x);
    return res;
  }
  gfp &operator+=(const gfp &x)
  {
    add(x);
    return *this;
  }
  gfp &operator-=(const gfp &x)
  {
    sub(x);
    return *this;
  }
  gfp &operator*=(const gfp &x)
  {
    mul(x);
    return *this;
  }

  void square(const gfp &aa)
  {
    Sqr(a, aa.a, ZpD);
  }
  void square()
  {
    Sqr(a, a, ZpD);
  }
  void invert()
  {
    Inv(a, a, ZpD);
  }
  void invert(const gfp &aa)
  {
    Inv(a, aa.a, ZpD);
  }
  void negate()
  {
    Negate(a, a, ZpD);
  }
  void power(long i)
  {
    Power(a, a, i, ZpD);
  }

  // deterministic square root
  gfp sqrRoot();

  void randomize(PRNG &G)
  {
    a.randomize(G, ZpD);
  }

  // faster randomization, see implementation for explanation
  void almost_randomize(PRNG &G);

  void output(ostream &s, bool human) const
  {
    a.output(s, ZpD, human);
  }
  void input(istream &s, bool human)
  {
    a.input(s, ZpD, human);
  }

  friend ostream &operator<<(ostream &s, const gfp &x)
  {
    x.output(s, true);
    return s;
  }
  friend istream &operator>>(istream &s, gfp &x)
  {
    x.input(s, true);
    return s;
  }

  /* Bitwise Ops
   *   - Converts gfp args to bigints and then converts answer back to gfp
   */
  void AND(const gfp &x, const gfp &y);
  void XOR(const gfp &x, const gfp &y);
  void OR(const gfp &x, const gfp &y);
  void AND(const gfp &x, const bigint &y);
  void XOR(const gfp &x, const bigint &y);
  void OR(const gfp &x, const bigint &y);
  void SHL(const gfp &x, int n);
  void SHR(const gfp &x, int n);
  void SHL(const gfp &x, const bigint &n);
  void SHR(const gfp &x, const bigint &n);

  gfp operator&(const gfp &x)
  {
    gfp res;
    res.AND(*this, x);
    return res;
  }
  gfp operator^(const gfp &x)
  {
    gfp res;
    res.XOR(*this, x);
    return res;
  }
  gfp operator|(const gfp &x)
  {
    gfp res;
    res.OR(*this, x);
    return res;
  }
  gfp operator<<(int i)
  {
    gfp res;
    res.SHL(*this, i);
    return res;
  }
  gfp operator>>(int i)
  {
    gfp res;
    res.SHR(*this, i);
    return res;
  }

  // Convert representation to and from a bigint number
  friend void to_bigint(bigint &ans, const gfp &x, bool reduce= true)
  {
    to_bigint(ans, x.a, x.ZpD, reduce);
  }
  friend void to_gfp(gfp &ans, const bigint &x)
  {
    to_modp(ans.a, x, ans.ZpD);
  }
};

#endif
