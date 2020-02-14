/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _Ciphertext
#define _Ciphertext

#include "FHE/FHE_Params.h"
#include "FHE/Plaintext.h"
#include "FHE/Rq_Element.h"

class FHE_PK;

/* Forward declaration of friends where needed */
class Ciphertext;
void mul(Ciphertext &ans, const Plaintext &a, const Ciphertext &c);
void add(Ciphertext &ans, const Ciphertext &c0, const Ciphertext &c1);
void mul(Ciphertext &ans, const Ciphertext &c0, const Ciphertext &c1,
         const FHE_PK &pk);

class Ciphertext
{
  Rq_Element cc0, cc1;
  const FHE_Params *params;

public:
  static string type_string()
  {
    return "ciphertext";
  }
  static int t()
  {
    return 0;
  }
  static int size()
  {
    return 0;
  }

  const FHE_Params &get_params() const
  {
    return *params;
  }

  Ciphertext(const FHE_Params &p)
      : cc0(p.FFTD(), evaluation, evaluation),
        cc1(p.FFTD(), evaluation, evaluation)
  {
    params= &p;
  }

  Ciphertext(const FHE_PK &pk);

  // Rely on default copy assignment/constructor

  void set(const Rq_Element &a0, const Rq_Element &a1)
  {
    cc0= a0;
    cc1= a1;
  }

  const Rq_Element &c0() const
  {
    return cc0;
  }
  const Rq_Element &c1() const
  {
    return cc1;
  }

  void assign_zero()
  {
    cc0.assign_zero();
    cc1.assign_zero();
  }

  // Scale down an element from level 1 to level 0, if at level 0 do nothing
  void Scale(const bigint &p)
  {
    cc0.Scale(p);
    cc1.Scale(p);
  }

  // Throws error if ans,c0,c1 etc have different params settings
  //   - Thus programmer needs to ensure this rather than this being done
  //     automatically. This saves some time in space initialization
  friend void add(Ciphertext &ans, const Ciphertext &c0, const Ciphertext &c1);
  friend void sub(Ciphertext &ans, const Ciphertext &c0, const Ciphertext &c1);
  friend void mul(Ciphertext &ans, const Plaintext &a, const Ciphertext &c);
  friend void mul(Ciphertext &ans, const Ciphertext &c, const Plaintext &a)
  {
    ::mul(ans, a, c);
  }
  void mul(const Ciphertext &c, const Rq_Element &a);
  void mul(const Ciphertext &c, const Plaintext &a)
  {
    ::mul(*this, a, c);
  }

  /* When doing a multiplication of ciphertexts we need pk to do the key
   * switching */
  friend void mul(Ciphertext &ans, const Ciphertext &c0, const Ciphertext &c1,
                  const FHE_PK &pk);
  Ciphertext mul(const Ciphertext &x, const FHE_PK &pk) const
  {
    Ciphertext res(*params);
    ::mul(res, *this, x, pk);
    return res;
  }

  // Multiply by X^i
  friend void mul_by_X_i(Ciphertext &ans, const Ciphertext &a, unsigned int j)
  {
    mul_by_X_i(ans.cc0, a.cc0, j);
    mul_by_X_i(ans.cc1, a.cc1, j);
  }

  bool operator==(const Ciphertext &c)
  {
    return cc0.equals(c.cc0) && cc1.equals(c.cc1);
  }
  bool operator!=(const Ciphertext &c)
  {
    return !(*this == c);
  }

  Ciphertext operator+(const Ciphertext &other) const
  {
    Ciphertext res(*params);
    ::add(res, *this, other);
    return res;
  }
  Ciphertext operator*(const Plaintext &other) const
  {
    Ciphertext res(*params);
    ::mul(res, other, *this);
    return res;
  }
  Ciphertext &operator+=(const Ciphertext &other)
  {
    ::add(*this, *this, other);
    return *this;
  }
  Ciphertext &operator*=(const Plaintext &other)
  {
    ::mul(*this, other, *this);
    return *this;
  }

  int level() const
  {
    return cc0.level();
  }

  void output(ostream &s, bool human= false) const;
  void input(istream &s)
  {
    cc0.input(s);
    cc1.input(s);
  }
};

#endif
