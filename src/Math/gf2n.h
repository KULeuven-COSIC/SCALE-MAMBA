/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef MATH_GF2N_H_
#define MATH_GF2N_H_

#include <stdlib.h>
#include <string.h>

#include "Math/bigint.h"
#include "Tools/random.h"

#include <iostream>
using namespace std;

#include <smmintrin.h>

class int128
{
public:
  __m128i a;

  int128() : a(_mm_setzero_si128()) {}
  int128(const int128 &a) : a(a.a) {}
  int128(const __m128i &a) : a(a) {}
  int128(const word &a) : a(_mm_cvtsi64_si128(a)) {}
  int128(const word &upper, const word &lower) : a(_mm_set_epi64x(upper, lower)) {}

  word get_lower() { return (word) _mm_cvtsi128_si64(a); }

  bool operator==(const int128 &other) const { return _mm_test_all_zeros(a ^ other.a, a ^ other.a); }
  bool operator!=(const int128 &other) const { return !(*this == other); }

  int128 operator<<(const int &other) const;
  int128 operator>>(const int &other) const;

  int128 operator^(const int128 &other) const { return a ^ other.a; }
  int128 operator|(const int128 &other) const { return a | other.a; }
  int128 operator&(const int128 &other) const { return a & other.a; }

  int128 operator~() const { return ~a; }

  int128 &operator<<=(const int &other) { return *this= *this << other; }
  int128 &operator>>=(const int &other) { return *this= *this >> other; }

  int128 &operator^=(const int128 &other)
  {
    a^= other.a;
    return *this;
  }
  int128 &operator|=(const int128 &other)
  {
    a|= other.a;
    return *this;
  }
  int128 &operator&=(const int128 &other)
  {
    a&= other.a;
    return *this;
  }

  friend ostream &operator<<(ostream &s, const int128 &a);
};

/*
  Arithmetic in Gf_{2^n} with n<=128
*/

class gf2n
{
  int128 a;

  static int n, t1, t2, t3, nterms;
  static int l0, l1, l2, l3;
  static int128 mask, lowermask, uppermask;
  static bool rewind;

  /* Assign x[0..2*nwords] to a and reduce it...  */
  void reduce_trinomial(int128 xh, int128 xl);
  void reduce_pentanomial(int128 xh, int128 xl);

public:
  typedef gf2n value_type;
  typedef int128 internal_type;

  void reduce(int128 xh, int128 xl)
  {
    if (nterms == 3)
      {
        reduce_pentanomial(xh, xl);
      }
    else
      {
        reduce_trinomial(xh, xl);
      }
  }

  static void init_field(int nn);
  static int degree() { return n; }
  static int default_degree() { return 128; }
  static int get_nterms() { return nterms; }
  static int get_t(int i)
  {
    if (i == 0)
      {
        return t1;
      }
    else if (i == 1)
      {
        return t2;
      }
    else if (i == 2)
      {
        return t3;
      }
    return -1;
  }

  static int size() { return sizeof(a); }
  static int t() { return 0; }

  static int default_length() { return 128; }

  int128 get() const { return a; }
  __m128i to_m128i() const { return a.a; }
  word get_word() const { return _mm_cvtsi128_si64(a.a); }

  void assign_zero() { a= _mm_setzero_si128(); }
  void assign_one() { a= int128(0, 1); }
  void assign_x() { a= int128(0, 2); }
  void assign(int128 aa) { a= aa & mask; }
  void assign(int aa) { a= int128(static_cast<unsigned int>(aa)) & mask; }

  void load_from_buffer(const uint8_t *buffer)
  {
    a= _mm_loadu_si128((__m128i *) buffer);
  }

  void store_into_buffer(uint8_t *buffer) const
  {
    _mm_storeu_si128((__m128i *) buffer, a.a);
  }

  int get_bit(int i) const
  {
    return ((a >> i) & 1).get_lower();
  }
  void set_bit(int i, unsigned int b)
  {
    if (b == 1)
      {
        a|= (1UL << i);
      }
    else
      {
        a&= ~(1UL << i);
      }
  }

  gf2n() { assign_zero(); }
  gf2n(const int128 &g) { assign(g); }
  gf2n(int g) { assign(g); }

  int is_zero() const { return a == int128(0); }
  int is_one() const { return a == int128(1); }
  int equal(const gf2n &y) const { return (a == y.a); }
  bool operator==(const gf2n &y) const { return a == y.a; }
  bool operator!=(const gf2n &y) const { return a != y.a; }

  // x+y
  void add(const gf2n &x, const gf2n &y)
  {
    a= x.a ^ y.a;
  }
  void add(const gf2n &x)
  {
    a^= x.a;
  }
  template<int T>
  void add(uint8_t *x)
  {
    a^= int128(_mm_loadu_si128((__m128i *) x));
  }
  void add(uint8_t *x)
  {
    add<0>(x);
  }
  void sub(const gf2n &x, const gf2n &y)
  {
    a= x.a ^ y.a;
  }
  void sub(const gf2n &x)
  {
    a^= x.a;
  }
  // = x * y
  gf2n &mul(const gf2n &x, const gf2n &y);
  void mul(const gf2n &x) { mul(*this, x); }
  // x * y when one of x,y is a bit
  void mul_by_bit(const gf2n &x, const gf2n &y) { a= x.a.a * y.a.a; }

  gf2n operator+(const gf2n &x)
  {
    gf2n res;
    res.add(*this, x);
    return res;
  }
  gf2n operator*(const gf2n &x)
  {
    gf2n res;
    res.mul(*this, x);
    return res;
  }
  gf2n &operator+=(const gf2n &x)
  {
    add(x);
    return *this;
  }
  gf2n &operator*=(const gf2n &x)
  {
    mul(x);
    return *this;
  }
  gf2n operator-(const gf2n &x)
  {
    gf2n res;
    res.add(*this, x);
    return res;
  }
  gf2n &operator-=(const gf2n &x)
  {
    sub(x);
    return *this;
  }

  void square();
  void square(const gf2n &aa);
  void invert();
  void invert(const gf2n &aa)
  {
    *this= aa;
    invert();
  }
  void negate() { return; }
  void power(long i);

  /* Bitwise Ops */
  void AND(const gf2n &x, const gf2n &y) { a= x.a & y.a; }
  void XOR(const gf2n &x, const gf2n &y) { a= x.a ^ y.a; }
  void OR(const gf2n &x, const gf2n &y) { a= x.a | y.a; }
  void NOT(const gf2n &x) { a= (~x.a) & mask; }
  void SHL(const gf2n &x, int n) { a= (x.a << n) & mask; }
  void SHR(const gf2n &x, int n) { a= x.a >> n; }

  gf2n operator&(const gf2n &x)
  {
    gf2n res;
    res.AND(*this, x);
    return res;
  }
  gf2n operator^(const gf2n &x)
  {
    gf2n res;
    res.XOR(*this, x);
    return res;
  }
  gf2n operator|(const gf2n &x)
  {
    gf2n res;
    res.OR(*this, x);
    return res;
  }
  gf2n operator!()
  {
    gf2n res;
    res.NOT(*this);
    return res;
  }
  gf2n operator<<(int i)
  {
    gf2n res;
    res.SHL(*this, i);
    return res;
  }
  gf2n operator>>(int i)
  {
    gf2n res;
    res.SHR(*this, i);
    return res;
  }

  /* Crap RNG */
  void randomize(PRNG &G);

  void output(ostream &s, bool human= false) const;
  void input(istream &s, bool human= false);

  friend ostream &operator<<(ostream &s, const gf2n &x)
  {
    s << hex << x.a << dec;
    return s;
  }
  friend istream &operator>>(istream &s, gf2n &x)
  {
    bigint tmp;
    s >> hex >> tmp >> dec;
    x.a= 0;
    mpn_copyi((word *) &x.a.a, tmp.get_mpz_t()->_mp_d, tmp.get_mpz_t()->_mp_size);
    return s;
  }
};

inline int128 int128::operator<<(const int &other) const
{
  int128 res(_mm_slli_epi64(a, other));
  __m128i mask;
  if (other < 64)
    mask= _mm_srli_epi64(a, 64 - other);
  else
    mask= _mm_slli_epi64(a, other - 64);
  res.a^= _mm_slli_si128(mask, 8);
  return res;
}

inline int128 int128::operator>>(const int &other) const
{
  int128 res(_mm_srli_epi64(a, other));
  __m128i mask;
  if (other < 64)
    mask= _mm_slli_epi64(a, 64 - other);
  else
    mask= _mm_srli_epi64(a, other - 64);
  res.a^= _mm_srli_si128(mask, 8);
  return res;
}

inline void mul128(__m128i a, __m128i b, __m128i *res1, __m128i *res2)
{
  __m128i tmp3, tmp4, tmp5, tmp6;

  tmp3= _mm_clmulepi64_si128(a, b, 0x00);
  tmp4= _mm_clmulepi64_si128(a, b, 0x10);
  tmp5= _mm_clmulepi64_si128(a, b, 0x01);
  tmp6= _mm_clmulepi64_si128(a, b, 0x11);

  tmp4= _mm_xor_si128(tmp4, tmp5);
  tmp5= _mm_slli_si128(tmp4, 8);
  tmp4= _mm_srli_si128(tmp4, 8);
  tmp3= _mm_xor_si128(tmp3, tmp5);
  tmp6= _mm_xor_si128(tmp6, tmp4);
  // initial mul now in tmp3, tmp6
  *res1= tmp3;
  *res2= tmp6;
}

#endif /* MATH_GF2N_H_ */
