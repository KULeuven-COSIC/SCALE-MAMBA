/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _Rq_Element
#define _Rq_Element

/* An Rq Element is something held modulo Q_0 = p0 or Q_1 = p0*p1
 *
 * The level is the value of Q_level which is being used.
 * Elements can be held in either representation and one can switch
 * representations at will.
 *   - Although in the evaluation we do not multiply at level 1
 *     we do need to multiply at level 1 for KeyGen and Encryption.
 *
 * Usually we keep level 0 in evaluation and level 1 in polynomial
 * representation though
 */

#include "FHE/Generator.h"
#include "FHE/Ring_Element.h"
#include <vector>

/* Forward declarations of some friends */
class Rq_Element;
void mul(Rq_Element &ans, const Rq_Element &a, const Rq_Element &b);

class Rq_Element
{
  vector<Ring_Element> a;
  int lev;

public:
  void change_rep(RepType r);
  void change_rep(RepType r0, RepType r1);
  void set_data(const vector<FFT_Data> &prd, RepType r0, RepType r1);

  void assign_zero(const vector<FFT_Data> &prd);
  void assign_zero();
  void assign_one();
  void assign(const Rq_Element &e);

  // Must be careful not to call by mistake
  Rq_Element(RepType r0= evaluation, RepType r1= polynomial)
      : a({r0, r1}), lev(1)
  {
  }

  // Pass in a pair of FFT_Data as a vector
  Rq_Element(const vector<FFT_Data> &prd, RepType r0= evaluation,
             RepType r1= polynomial);

  Rq_Element(const Ring_Element &b0, const Ring_Element &b1)
      : a({b0, b1}), lev(1)
  {
  }

  // Copy Assignment
  Rq_Element &operator=(const Rq_Element &e)
  {
    if (this != &e)
      {
        assign(e);
      }
    return *this;
  }

  const Ring_Element &get(int i) const
  {
    return a[i];
  }

  /* Functional Operators */
  void negate();
  friend void add(Rq_Element &ans, const Rq_Element &a, const Rq_Element &b);
  friend void sub(Rq_Element &ans, const Rq_Element &a, const Rq_Element &b);
  friend void mul(Rq_Element &ans, const Rq_Element &a, const Rq_Element &b);
  friend void mul(Rq_Element &ans, const Rq_Element &a, const bigint &b);

  // Multiply by X^i
  friend void mul_by_X_i(Rq_Element &ans, const Rq_Element &a, unsigned int j);

  Rq_Element &operator+=(const Rq_Element &other)
  {
    add(*this, *this, other);
    return *this;
  }

  Rq_Element operator+(const Rq_Element &b) const
  {
    Rq_Element res(*this);
    add(res, *this, b);
    return res;
  }
  Rq_Element operator-(const Rq_Element &b) const
  {
    Rq_Element res(*this);
    sub(res, *this, b);
    return res;
  }
  template<class T>
  Rq_Element operator*(const T &b) const
  {
    Rq_Element res(*this);
    mul(res, *this, b);
    return res;
  }

  // Multiply something by p1 and make level 1
  void mul_by_p1();

  void randomize(PRNG &G, int lev= 1);

  // Scale from level 1 to level 0, if at level 0 do nothing
  void Scale(const bigint &p);

  bool equals(const Rq_Element &a) const;
  bool operator!=(const Rq_Element &a) const
  {
    return !equals(a);
  }

  int level() const
  {
    return lev;
  }
  void lower_level()
  {
    if (lev == 1)
      {
        lev= 0;
      }
  }
  // raise_level boosts a level 0 to a level 1 (or does nothing if level =1)
  void raise_level();
  void check_level() const;

  // Converting to and from a vector of bigint's Again I/O is in poly rep
  void from_vec(const vector<bigint> &v, int level= 1);
  void from_vec(const vector<int> &v, int level= 1);
  vector<bigint> to_vec_bigint() const;
  void to_vec_bigint(vector<bigint> &v) const;

  ConversionIterator get_iterator();
  template<class T>
  void from(const Generator<T> &generator, int level= 1);

  bigint infinity_norm() const;

  bigint get_prime(int i) const
  {
    return a[i].get_prime();
  }

  bigint get_modulus() const
  {
    bigint ans= a[0].get_prime();
    if (lev == 1)
      {
        ans*= a[1].get_prime();
      }
    return ans;
  }

  void output(ostream &s, bool human= false) const;
  void input(istream &s);

  bool is_zero() const;
};

inline void mul(Rq_Element &ans, const bigint &a, const Rq_Element &b)
{
  mul(ans, b, a);
}

#endif
