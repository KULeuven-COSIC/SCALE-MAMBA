/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

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

template<int L>
class gfp_
{
  modp_<L> a;
  thread_local static Zp_Data ZpD;

public:
  static void init_field(const bigint &p, bool mont= true);

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

  modp_<L> get() const
  {
    return a;
  }

  // Assumes prD behind x is equal to ZpD
  void assign(modp_<L> &x)
  {
    a= x;
  }

  gfp_()
  {
    assignZero(a, ZpD);
  }
  template<int LL>
  gfp_(const modp_<LL> &g)
  {
    a= g;
  }
  gfp_(const bigint &x)
  {
    to_modp(a, x, ZpD);
  }
  gfp_(int x)
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

  bool equal(const gfp_ &y) const
  {
    return areEqual(a, y.a, ZpD);
  }
  bool operator==(const gfp_ &y) const
  {
    return equal(y);
  }
  bool operator!=(const gfp_ &y) const
  {
    return !equal(y);
  }

  // x+y
  void add(const gfp_ &x, const gfp_ &y)
  {
    ZpD.Add<L>(a.x, x.a.x, y.a.x);
  }
  void add(const gfp_ &x)
  {
    ZpD.Add<L>(a.x, a.x, x.a.x);
  }
  void add(void *x)
  {
    ZpD.Add<L>(a.x, a.x, (mp_limb_t *) x);
  }
  void sub(const gfp_ &x, const gfp_ &y)
  {
    ZpD.Sub<L>(a.x, x.a.x, y.a.x);
  }
  void sub(const gfp_ &x)
  {
    ZpD.Sub<L>(a.x, a.x, x.a.x);
  }

  // = x * y
  void mul(const gfp_ &x, const gfp_ &y)
  {
    a.mul(x.a, y.a, ZpD);
  }
  void mul(const gfp_ &x)
  {
    a.mul(a, x.a, ZpD);
  }

  gfp_ operator+(const gfp_ &x) const
  {
    gfp_ res;
    res.add(*this, x);
    return res;
  }
  gfp_ operator-(const gfp_ &x) const
  {
    gfp_ res;
    res.sub(*this, x);
    return res;
  }
  gfp_ operator*(const gfp_ &x) const
  {
    gfp_ res;
    res.mul(*this, x);
    return res;
  }
  gfp_ &operator+=(const gfp_ &x)
  {
    add(x);
    return *this;
  }
  gfp_ &operator-=(const gfp_ &x)
  {
    sub(x);
    return *this;
  }
  gfp_ &operator*=(const gfp_ &x)
  {
    mul(x);
    return *this;
  }

  void square(const gfp_ &aa)
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
  void invert(const gfp_ &aa)
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
  gfp_ sqrRoot();

  void randomize(PRNG &G)
  {
    a.randomize(G, ZpD);
  }

  void output(ostream &s, bool human) const
  {
    a.output(s, ZpD, human);
  }
  void input(istream &s, bool human)
  {
    a.input(s, ZpD, human);
  }

  // Direct IO to a buffer
  unsigned int output(uint8_t *buff) const
  {
    return a.output(buff, ZpD);
  }
  unsigned int input(const uint8_t *buff)
  {
    return a.input(buff, ZpD);
  }

  /* Input/Output to a string at position pos.
   * String is already of enough size in both cases.
   * The number of chars read/written is returned
   */
  unsigned int output(string &s, unsigned long pos) const
  {
    return a.output((uint8_t *) s.c_str() + pos, ZpD);
  }
  unsigned int input(const string &s, unsigned long pos)
  {
    return a.input((uint8_t *) s.c_str() + pos, ZpD);
  }

  friend ostream &operator<<(ostream &s, const gfp_ &x)
  {
    x.output(s, true);
    return s;
  }
  friend istream &operator>>(istream &s, gfp_ &x)
  {
    x.input(s, true);
    return s;
  }

  /* Bitwise Ops
   *   - Converts gfp_ args to bigints and then converts answer back to gfp_
   */
  void AND(const gfp_ &x, const gfp_ &y);
  void XOR(const gfp_ &x, const gfp_ &y);
  void OR(const gfp_ &x, const gfp_ &y);
  void AND(const gfp_ &x, const bigint &y);
  void XOR(const gfp_ &x, const bigint &y);
  void OR(const gfp_ &x, const bigint &y);
  void SHL(const gfp_ &x, int n);
  void SHR(const gfp_ &x, int n);
  void SHL(const gfp_ &x, const bigint &n);
  void SHR(const gfp_ &x, const bigint &n);

  gfp_ operator&(const gfp_ &x)
  {
    gfp_ res;
    res.AND(*this, x);
    return res;
  }
  gfp_ operator^(const gfp_ &x)
  {
    gfp_ res;
    res.XOR(*this, x);
    return res;
  }
  gfp_ operator|(const gfp_ &x)
  {
    gfp_ res;
    res.OR(*this, x);
    return res;
  }
  gfp_ operator<<(int i)
  {
    gfp_ res;
    res.SHL(*this, i);
    return res;
  }
  gfp_ operator>>(int i)
  {
    gfp_ res;
    res.SHR(*this, i);
    return res;
  }

  // Convert representation to and from a bigint number
  friend void to_bigint(bigint &ans, const gfp_ &x, bool reduce= true)
  {
    to_bigint(ans, x.a, x.ZpD, reduce);
  }
  friend void to_gfp(gfp_ &ans, const bigint &x)
  {
    to_modp(ans.a, x, ans.ZpD);
  }
};

#include "gfp.hpp"

typedef gfp_<MAX_GFP_SZ> gfp;

#endif
