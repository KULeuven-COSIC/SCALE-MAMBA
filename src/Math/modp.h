/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _Modp
#define _Modp

/*
 * Currently we only support an MPIR based implementation.
 *
 * What ever is type-def'd to bigint is assumed to have
 * operator overloading for all standard operators, has
 * comparison operations and istream/ostream operators >>/<<.
 *
 * All "integer" operations will be done using operator notation
 * all "modp" operations should be done using the function calls
 * below (interchange with Montgomery arithmetic).
 *
 */

#include "Tools/random.h"

#include "Math/Zp_Data.h"
#include "Math/bigint.h"

class modp
{
  mp_limb_t x[MAX_MOD_SZ];

public:
  // NEXT FUNCTION IS FOR DEBUG PURPOSES ONLY
  mp_limb_t get_limb(int i)
  {
    return x[i];
  }

  // use mem* functions instead of mpn_*, so the compiler can optimize
  modp()
  {
    memset(x, 0, sizeof(x));
  }
  /*
  modp(const modp &y)
  {
    memcpy(x, y.x, sizeof(x));
  }
  modp &operator=(const modp &y)
  {
    if (this != &y)
      {
        memcpy(x, y.x, sizeof(x));
      }
    return *this;
  }
  */

  void assign(const char *buffer, int t)
  {
    memcpy(x, buffer, t * sizeof(mp_limb_t));
  }

  void randomize(PRNG &G, const Zp_Data &ZpD);

  /**********************************
    *         Modp Operations        *
    **********************************/

  // Convert representation to and from a modp number
  //   On some machines have an issue with friend functions with default args
  friend void to_bigint(bigint &ans, const modp &x, const Zp_Data &ZpD,
                        bool reduce);
  friend void to_bigint(bigint &ans, const modp &x, const Zp_Data &ZpD)
  {
    to_bigint(ans, x, ZpD, true);
  }

  friend void to_modp(modp &ans, int x, const Zp_Data &ZpD);
  friend void to_modp(modp &ans, const bigint &x, const Zp_Data &ZpD);
  void to_modp(const bigint &x, const Zp_Data &ZpD);
  void to_modp(int x, const Zp_Data &ZpD)
  {
    to_modp(bigint(x), ZpD);
  }

  friend void Add(modp &ans, const modp &x, const modp &y, const Zp_Data &ZpD)
  {
    ZpD.Add(ans.x, x.x, y.x);
  }
  friend void Sub(modp &ans, const modp &x, const modp &y, const Zp_Data &ZpD)
  {
    ZpD.Sub(ans.x, x.x, y.x);
  }
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

  // Input and output from a stream
  //  - Can do in human or machine only format (later should be faster)
  //  - If human output appends a space to help with reading
  //    and also convert back/forth from Montgomery if needed
  void output(ostream &s, const Zp_Data &ZpD, bool human) const;
  void input(istream &s, const Zp_Data &ZpD, bool human);

  friend class gfp;
};

inline void assignZero(modp &x, const Zp_Data &ZpD)
{
  if (sizeof(x.x) <= 3 * 16)
    // use memset to allow the compiler to optimize
    // if x.x is at most 3*128 bits
    memset(x.x, 0, sizeof(x.x));
  else
    mpn_zero(x.x, ZpD.t + 1);
}

#endif
