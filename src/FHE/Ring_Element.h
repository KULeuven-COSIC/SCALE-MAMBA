/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _Ring_Element
#define _Ring_Element

/* Defines an element of the ring modulo a prime pr
 *   - Note here pr is an odd prime
 * We also assume that pr splits completely over the underlying
 * ring. Equivalently (as the ring is of m'th roots of unity),
 * we have that pr-1 is divisible by m.
 * Thus we can define both a polynomial and an evaluation
 * representation
 * We assume m is a power of two
 */

enum RepType {
  polynomial,
  evaluation
};

#include "FHE/FFT_Data.h"
#include "Tools/random.h"
#include <FHE/Generator.h>
#include <iostream>
#include <vector>
using namespace std;

class RingWriteIterator;
class RingReadIterator;

class Ring_Element
{
  RepType rep;

  /* FFTD is defined as a pointer so each different Ring_Element
   * can be wrt a different prime if need be
   *   - Recall FFTD also contains data about the Ring
   */
  const FFT_Data *FFTD;

  /* In either representation we hold the element as an array of
   * modp's of length Ring.phi_m()
   */

  vector<modp> element;

public:
  // Used to basically make sure *this is able to cope
  // with being assigned to by something of "type" e
  void partial_assign(const Ring_Element &e)
  {
    rep= e.rep;
    FFTD= e.FFTD;
    if (FFTD)
      element.resize((*FFTD).phi_m());
  }

  void set_data(const FFT_Data &prd, RepType rept)
  {
    FFTD= &prd;
    element.resize(prd.phi_m());
    rep= rept;
  }
  const FFT_Data &get_FFTD() const
  {
    return *FFTD;
  }
  const Zp_Data &get_prD() const
  {
    return (*FFTD).get_prD();
  }
  const bigint &get_prime() const
  {
    return (*FFTD).get_prime();
  }

  void assign_zero();
  void assign_one();

  /* Careful calling this one, as FFTD will not be defined */
  Ring_Element(RepType r= polynomial)
      : FFTD(0)
  {
    rep= r;
  }

  Ring_Element(const FFT_Data &prd, RepType r= polynomial);

  /* Functional Operators */
  void negate();
  friend void add(Ring_Element &ans, const Ring_Element &a,
                  const Ring_Element &b);
  friend void sub(Ring_Element &ans, const Ring_Element &a,
                  const Ring_Element &b);
  friend void mul(Ring_Element &ans, const Ring_Element &a,
                  const Ring_Element &b);
  friend void mul(Ring_Element &ans, const Ring_Element &a, const modp &b);

  // Multiply by X^i
  friend void mul_by_X_i(Ring_Element &ans, const Ring_Element &a, unsigned int j);

  void randomize(PRNG &G, bool Diag= false);

  bool equals(const Ring_Element &a) const;

  // This is a NOP in cases where we cannot do a FFT
  void change_rep(RepType r);

  RepType representation() const
  {
    return rep;
  }

  // Converting to and from a vector of bigint/int's
  // I/O is assumed to be in poly rep, so from_vec it internally alters
  // the representation to the current representation
  void from_vec(const vector<bigint> &v);
  void from_vec(const vector<int> &v);
  vector<bigint> to_vec_bigint() const;
  void to_vec_bigint(vector<bigint> &v) const;

  ConversionIterator get_iterator() const;

  friend class RingReadIterator;
  RingReadIterator get_copy_iterator() const;

  friend class RingWriteIterator;
  RingWriteIterator get_write_iterator();

  template<class T>
  void from(const Generator<T> &generator);

  // This gets the constant term of the poly rep as a modp element
  modp get_constant() const;
  modp get_element(int i) const
  {
    return element[i];
  }
  void set_element(int i, const modp &a)
  {
    element[i]= a;
  }

  friend ostream &operator<<(ostream &s, const Ring_Element &e);
  friend istream &operator>>(istream &s, Ring_Element &e);

  void check_rep();
  void check_size() const;

  void output(ostream &s) const;
  void input(istream &s);

  bool is_zero() const;
};

class RingWriteIterator : public WriteConversionIterator
{
  Ring_Element &element;
  RepType rep;

public:
  RingWriteIterator(Ring_Element &element)
      : WriteConversionIterator(element.element, element.FFTD->get_prD()),
        element(element), rep(element.rep)
  {
    element.rep= polynomial;
  }
  ~RingWriteIterator()
  {
    element.change_rep(rep);
  }
};

class RingReadIterator : public ConversionIterator
{
  Ring_Element element;

public:
  RingReadIterator(const Ring_Element &element)
      : ConversionIterator(this->element.element, element.FFTD->get_prD()),
        element(element)
  {
    this->element.change_rep(polynomial);
  }
};

inline void mul(Ring_Element &ans, const modp &a, const Ring_Element &b)
{
  mul(ans, b, a);
}

#endif
