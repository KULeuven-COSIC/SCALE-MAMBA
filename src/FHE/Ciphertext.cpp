/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "FHE/Ciphertext.h"
#include "Exceptions/Exceptions.h"
#include "FHE/FHE_Keys.h"
#include "FHE/Rq_Element.h"

Ciphertext::Ciphertext(const FHE_PK &pk)
    : Ciphertext(pk.get_params()) {}

void add(Ciphertext &ans, const Ciphertext &c0, const Ciphertext &c1)
{
  if (c0.params != c1.params)
    {
      throw data_mismatch();
    }
  if (ans.params != c1.params)
    {
      throw data_mismatch();
    }
  add(ans.cc0, c0.cc0, c1.cc0);
  add(ans.cc1, c0.cc1, c1.cc1);
}

void sub(Ciphertext &ans, const Ciphertext &c0, const Ciphertext &c1)
{
  if (c0.params != c1.params)
    {
      throw data_mismatch();
    }
  if (ans.params != c1.params)
    {
      throw data_mismatch();
    }
  sub(ans.cc0, c0.cc0, c1.cc0);
  sub(ans.cc1, c0.cc1, c1.cc1);
}

void mul(Ciphertext &ans, const Ciphertext &c0, const Ciphertext &c1,
         const FHE_PK &pk)
{
  if (c0.params != c1.params)
    {
      throw data_mismatch();
    }
  if (ans.params != c1.params)
    {
      throw data_mismatch();
    }

  // Switch Modulus for c0 and c1 down to level 0
  Ciphertext cc0= c0, cc1= c1;
  cc0.Scale(pk.p());
  cc1.Scale(pk.p());

  // Now do the multiply
  Rq_Element d0, d1, d2;

  mul(d0, cc0.cc0, cc1.cc0);
  mul(d1, cc0.cc0, cc1.cc1);
  mul(d2, cc0.cc1, cc1.cc0);
  add(d1, d1, d2);
  mul(d2, cc0.cc1, cc1.cc1);
  d2.negate();

  // Now do the switch key
  d2.raise_level();
  Rq_Element t;
  d0.mul_by_p1();
  mul(t, pk.bs(), d2);
  add(d0, d0, t);

  d1.mul_by_p1();
  mul(t, pk.as(), d2);
  add(d1, d1, t);

  ans.set(d0, d1);
  ans.Scale(pk.p());
}

void mul(Ciphertext &ans, const Plaintext &a, const Ciphertext &c)
{
  a.to_poly();
  const vector<bigint> &aa= a.get_poly();

  int lev= c.cc0.level();
  Rq_Element ra((*ans.params).FFTD(), evaluation, evaluation);
  if (lev == 0)
    {
      ra.lower_level();
    }
  ra.from_vec(aa);
  ans.mul(c, ra);
}

void Ciphertext::mul(const Ciphertext &c, const Rq_Element &ra)
{
  if (params != c.params)
    {
      throw data_mismatch();
    }

  ::mul(cc0, ra, c.cc0);
  ::mul(cc1, ra, c.cc1);
}

void Ciphertext::output(ostream &s, bool human) const
{

  if (human)
    {
      s << "cc0=";
      cc0.output(s, human);
      s << endl;
      s << "cc1=";
      cc1.output(s, human);
      s << endl;
    }
  else
    {
      cc0.output(s, human);
      cc1.output(s, human);
    }
}
