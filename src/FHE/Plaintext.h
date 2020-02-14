/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _Plaintext
#define _Plaintext

/*
 * Defines plaintext either as a vector of field elements (gfp)
 * or as a vector of bigints.
 *
 * The first format is the actual data we want, the second format
 * is the data we use to encrypt/decrypt. Passing between the two
 * uses the FFT as we select prime moduli so that this is possible
 *
 * We hold data however in vector format, as this is easier to
 * deal with
 */

#include "FHE/FFT_Data.h"
#include "FHE/Generator.h"
#include "Math/bigint.h"
#include "Math/gfp.h"

#include <vector>
using namespace std;

enum condition {
  General,
  Diagonal
};

enum PT_Type {
  Polynomial,
  Evaluation,
  Both
};

/* Forward declaration of friends where needed */
class Plaintext;
void mul(Plaintext &z, const Plaintext &x, const Plaintext &y);
void sub(Plaintext &z, const Plaintext &x, const Plaintext &y);

class Plaintext
{
  mutable vector<gfp> a;    // The thing in evaluation/FFT form
  mutable vector<bigint> b; // Now in polynomial form

  mutable PT_Type type;

  /* We keep pointers to the basic data here */

  const FFT_Data *Field_Data;

public:
  const FFT_Data &get_field() const
  {
    return *Field_Data;
  }

  Plaintext(const FFT_Data &FieldD, PT_Type type= Polynomial)
  {
    Field_Data= &FieldD;
    allocate(type);
  }

  void allocate(PT_Type type) const;
  void allocate() const
  {
    allocate(type);
  }
  void allocate_slots(const bigint &value);
  int get_min_alloc();

  // Access evaluation representation
  gfp element(int i) const
  {
    if (type == Polynomial)
      {
        from_poly();
      }
    return a[i];
  }
  void set_element(int i, const gfp &e)
  {
    if (type == Polynomial)
      {
        throw not_implemented();
      }
    a.resize(Field_Data->num_slots());
    a[i]= e;
    type= Evaluation;
  }

  // Access poly representation
  const bigint &coeff(int i) const
  {
    if (type != Polynomial)
      {
        to_poly();
      }
    return b[i];
  }

  void set_coeff(int i, const bigint &e)
  {
    to_poly();
    type= Polynomial;
    b[i]= e;
  }

  const bigint &operator[](int i) const
  {
    return coeff(i);
  }

  // Assumes v is of the correct length (phi_m) for the given ring
  // and is already reduced modulo the correct prime etc
  void set_poly(const vector<bigint> &v)
  {
    type= Polynomial;
    b= v;
  }
  const vector<bigint> &get_poly() const
  {
    if (type == Evaluation)
      {
        throw data_mismatch();
      }
    return b;
  }

  Iterator<bigint> get_iterator() const
  {
    to_poly();
    return b;
  }

  // This sets a poly from a vector of bigint's which needs centering
  // modulo mod, before assigning (used in decryption)
  //    vv[i] is already assumed reduced modulo mod though but in
  //    range [0,...,mod)
  void set_poly_mod(const vector<bigint> &vv, const bigint &mod);
  void set_poly_mod(const Generator<bigint> &generator, const bigint &mod);

  // Converts between Evaluation,Polynomial and Both representations
  //    Marked as const because does not change value, only changes the
  //    internal representation
  void from_poly() const;
  void to_poly() const;

  void randomize(PRNG &G, condition cond= General);

  void assign_zero(PT_Type t= Evaluation);
  void assign_one(PT_Type t= Evaluation);

  friend void add(Plaintext &z, const Plaintext &x, const Plaintext &y);
  friend void sub(Plaintext &z, const Plaintext &x, const Plaintext &y);
  friend void mul(Plaintext &z, const Plaintext &x, const Plaintext &y);
  friend void sqr(Plaintext &z, const Plaintext &x);

  Plaintext operator+(const Plaintext &x) const
  {
    Plaintext res(*Field_Data);
    add(res, *this, x);
    return res;
  }
  Plaintext operator-(const Plaintext &x) const
  {
    Plaintext res(*Field_Data);
    sub(res, *this, x);
    return res;
  }

  void mul(const Plaintext &x, const Plaintext &y)
  {
    x.from_poly();
    y.from_poly();
    ::mul(*this, x, y);
  }

  Plaintext operator*(const Plaintext &x)
  {
    Plaintext res(*Field_Data);
    res.mul(*this, x);
    return res;
  }

  Plaintext &operator+=(const Plaintext &y);
  Plaintext &operator-=(const Plaintext &y)
  {
    to_poly();
    y.to_poly();
    ::sub(*this, *this, y);
    return *this;
  }

  void negate();

  bool equals(const Plaintext &x) const;
  bool operator!=(const Plaintext &x)
  {
    return !equals(x);
  }

  friend ostream &operator<<(ostream &s, const Plaintext &e);
  friend istream &operator>>(istream &s, Plaintext &e);

  // For test purposes
  void output(ostream &s) const;
};

#endif
