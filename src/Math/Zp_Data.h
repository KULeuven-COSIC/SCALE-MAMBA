/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

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

/* If MAX_MOD_SZ is not defined then something is wrong in the compilation.
 * Not having a default value helps application developers which are including
 * this file as well as linking against the library
 *   - Since the two values should be identical
 */
#ifndef MAX_MOD_SZ
#error "MAX_MOD_SZ is not defined."
#endif

class modp;

class Zp_Data
{
  bool montgomery; // True if we are using Montgomery arithmetic
  mp_limb_t R[MAX_MOD_SZ], R2[MAX_MOD_SZ], R3[MAX_MOD_SZ], pi;
  mp_limb_t prA[MAX_MOD_SZ];
  int t; // More Montgomery data

  void Mont_Mult(mp_limb_t *z, const mp_limb_t *x, const mp_limb_t *y) const;

public:
  bigint pr;
  mp_limb_t mask;

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

  void Add(mp_limb_t *ans, const mp_limb_t *x, const mp_limb_t *y) const;
  void Sub(mp_limb_t *ans, const mp_limb_t *x, const mp_limb_t *y) const;

  friend void to_bigint(bigint &ans, const modp &x, const Zp_Data &ZpD,
                        bool reduce);

  friend void to_modp(modp &ans, int x, const Zp_Data &ZpD);
  friend void to_modp(modp &ans, const bigint &x, const Zp_Data &ZpD);

  friend void Add(modp &ans, const modp &x, const modp &y, const Zp_Data &ZpD);
  friend void Sub(modp &ans, const modp &x, const modp &y, const Zp_Data &ZpD);
  friend void Mul(modp &ans, const modp &x, const modp &y, const Zp_Data &ZpD);
  friend void Sqr(modp &ans, const modp &x, const Zp_Data &ZpD);
  friend void Negate(modp &ans, const modp &x, const Zp_Data &ZpD);
  friend void Inv(modp &ans, const modp &x, const Zp_Data &ZpD);

  friend void Power(modp &ans, const modp &x, int exp, const Zp_Data &ZpD);
  friend void Power(modp &ans, const modp &x, const bigint &exp,
                    const Zp_Data &ZpD);

  friend void assignOne(modp &x, const Zp_Data &ZpD);
  friend void assignZero(modp &x, const Zp_Data &ZpD);
  friend bool isZero(const modp &x, const Zp_Data &ZpD);
  friend bool isOne(const modp &x, const Zp_Data &ZpD);
  friend bool areEqual(const modp &x, const modp &y, const Zp_Data &ZpD);

  friend class modp;

  friend ostream &operator<<(ostream &s, const Zp_Data &ZpD);
  friend istream &operator>>(istream &s, Zp_Data &ZpD);
};

#endif
