/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

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

template<int L>
class modp_
{
  mp_limb_t x[L];

public:
  // NEXT FUNCTION IS FOR DEBUG PURPOSES ONLY
  mp_limb_t get_limb(int i)
  {
    return x[i];
  }

  // use mem* functions instead of mpn_*, so the compiler can optimize
  modp_()
  {
    memset(x, 0, sizeof(x));
  }

  const mp_limb_t *get() const { return x; }

  template<int M>
  modp_(const modp_<M> &other)
  {
    inline_mpn_copyi(x, other.get(), min(L, M));
    if (L > M)
      {
        inline_mpn_zero(x + M, L - M);
      }
  }

  void assign(const char *buffer, int t)
  {
    memcpy(x, buffer, t * sizeof(mp_limb_t));
  }

  void randomize(PRNG &G, const Zp_Data &ZpD);

  /**********************************
    *         Modp Operations        *
    **********************************/

  // Convert representation to and from a modp_ number
  //   On some machines have an issue with friend functions with default args
  void to_bigint(bigint &ans, const Zp_Data &ZpD, bool reduce= true) const;

  template<int M>
  friend void to_modp(modp_<M> &ans, int x, const Zp_Data &ZpD);
  template<int M>
  friend void to_modp(modp_<M> &ans, const bigint &x, const Zp_Data &ZpD);
  void to_modp(const bigint &x, const Zp_Data &ZpD);
  void to_modp(int x, const Zp_Data &ZpD)
  {
    to_modp(bigint(x), ZpD);
  }

  void mul(const modp_ &x, const modp_ &y, const Zp_Data &ZpD);

  friend void Add(modp_ &ans, const modp_ &x, const modp_ &y, const Zp_Data &ZpD)
  {
    ZpD.Add(ans.x, x.x, y.x);
  }
  friend void Sub(modp_ &ans, const modp_ &x, const modp_ &y, const Zp_Data &ZpD)
  {
    ZpD.Sub(ans.x, x.x, y.x);
  }
  template<int M>
  friend void Mul(modp_<M> &ans, const modp_<M> &x, const modp_<M> &y, const Zp_Data &ZpD);
  template<int M>
  friend void Sqr(modp_<M> &ans, const modp_<M> &x, const Zp_Data &ZpD);
  template<int M>
  friend void Negate(modp_<M> &ans, const modp_<M> &x, const Zp_Data &ZpD);
  template<int M>
  friend void Inv(modp_<M> &ans, const modp_<M> &x, const Zp_Data &ZpD);

  template<int M>
  friend void Power(modp_<M> &ans, const modp_<M> &x, int exp, const Zp_Data &ZpD);
  template<int M>
  friend void Power(modp_<M> &ans, const modp_<M> &x, const bigint &exp,
                    const Zp_Data &ZpD);

  template<int M>
  friend void assignOne(modp_<M> &x, const Zp_Data &ZpD);
  template<int M>
  friend void assignZero(modp_<M> &x, const Zp_Data &ZpD);
  template<int M>
  friend bool isZero(const modp_<M> &x, const Zp_Data &ZpD);
  template<int M>
  friend bool isOne(const modp_<M> &x, const Zp_Data &ZpD);
  template<int M>
  friend bool areEqual(const modp_<M> &x, const modp_<M> &y, const Zp_Data &ZpD);

  // Input and output from a stream
  //  - Can do in human or machine only format (later should be faster)
  //  - If human output appends a space to help with reading
  //    and also convert back/forth from Montgomery if needed
  void output(ostream &s, const Zp_Data &ZpD, bool human) const;
  void input(istream &s, const Zp_Data &ZpD, bool human);

  // Output directly to a string of chars, assumes enough space is
  // allocated. Returns the number of chars written
  unsigned int output(uint8_t *buff, const Zp_Data &ZpD) const
  {
    memcpy(buff, (char *) x, ZpD.t * sizeof(mp_limb_t));
    return ZpD.t * sizeof(mp_limb_t);
  }
  // Input directly from a string of chars
  unsigned int input(const uint8_t *buff, const Zp_Data &ZpD)
  {
    memcpy((char *) x, buff, ZpD.t * sizeof(mp_limb_t));
    return ZpD.t * sizeof(mp_limb_t);
  }

  /* Input/Output to a string at position pos. 
   * String is already of enough size in both cases. 
   * The number of chars read/written is returned 
   */
  unsigned int output(string &s, unsigned long pos, const Zp_Data &ZpD) const
  {
    return output((uint8_t *) s.c_str() + pos, ZpD);
  }
  unsigned int input(const string &s, unsigned long pos, const Zp_Data &ZpD)
  {
    return input((uint8_t *) s.c_str() + pos, ZpD);
  }

  template<int K>
  friend class gfp_;
};

#include "modp.hpp"

typedef modp_<MAX_MOD_SZ> modp;

#endif
