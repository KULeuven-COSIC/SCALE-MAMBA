/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "FHE/Plaintext.h"
#include "FHE/Ring_Element.h"

void Plaintext::from_poly() const
{
  if (type != Polynomial)
    {
      return;
    }

  Ring_Element e(*Field_Data, polynomial);
  e.from_vec(b);
  e.change_rep(evaluation);
  a.resize((*Field_Data).phi_m());
  for (unsigned int i= 0; i < a.size(); i++)
    {
      a[i]= e.get_element(i);
    }
  type= Both;
}

void Plaintext::to_poly() const
{
  if (type != Evaluation)
    {
      return;
    }

  Ring_Element e(*Field_Data, evaluation);
  for (unsigned int i= 0; i < a.size(); i++)
    {
      e.set_element(i, a[i].get());
    }
  e.change_rep(polynomial);
  e.to_vec_bigint(b);
  type= Both;
}

void Plaintext::allocate(PT_Type type) const
{
  if (type != Evaluation)
    b.resize((*Field_Data).phi_m());
  if (type != Polynomial)
    a.resize((*Field_Data).phi_m());
  this->type= type;
}

void Plaintext::allocate_slots(const bigint &value)
{
  b.resize((*Field_Data).phi_m());
  for (auto &x : b)
    x= value;
}

void signed_mod(bigint &x, const bigint &mod, const bigint &half_mod,
                const bigint &dest_mod)
{
  if (x > half_mod)
    x-= mod;
  x%= dest_mod;
  if (x < 0)
    x+= dest_mod;
}

void Plaintext::set_poly_mod(const Generator<bigint> &generator,
                             const bigint &mod)
{
  allocate(Polynomial);
  bigint half_mod= mod / 2;
  for (unsigned int i= 0; i < b.size(); i++)
    {
      generator.get(b[i]);
      signed_mod(b[i], mod, half_mod, Field_Data->get_prime());
    }
}

void Plaintext::set_poly_mod(const vector<bigint> &vv, const bigint &mod)
{
  b= vv;
  vector<bigint> &pol= b;
  bigint half_mod= mod / 2;
  for (unsigned int i= 0; i < vv.size(); i++)
    {
      pol[i]= vv[i];
      if (pol[i] > half_mod)
        pol[i]-= mod;
      pol[i]%= (*Field_Data).get_prime();
      if (pol[i] < 0)
        {
          pol[i]+= (*Field_Data).get_prime();
        }
    }
  type= Polynomial;
}

void rand_poly(vector<bigint> &b, PRNG &G, const bigint &pr)
{
  for (unsigned int i= 0; i < b.size(); i++)
    {
      b[i]= randomBnd(G, pr);
    }
}

void Plaintext::randomize(PRNG &G, condition cond)
{
  switch (cond)
    {
      case General:
        rand_poly(b, G, (*Field_Data).get_prime());
        type= Polynomial;
        break;
      case Diagonal:
        a.resize((*Field_Data).phi_m());
        a[0].randomize(G);
        for (unsigned int i= 1; i < a.size(); i++)
          {
            a[i]= a[0];
          }
        type= Evaluation;
        break;
      default:
        throw bad_value();
        break;
    }
}

void Plaintext::output(ostream &s) const
{
  if (type == Polynomial || type == Both)
    {
      s << "P : [";
      for (unsigned int i= 0; i < b.size(); i++)
        {
          s << b[i] << " ";
        }
      s << "]\n";
    }
  if (type == Evaluation || type == Both)
    {
      s << "E : [";
      for (unsigned int i= 0; i < a.size(); i++)
        {
          s << a[i] << " ";
        }
      s << "]\n";
    }
}

ostream &operator<<(ostream &s, const Plaintext &e)
{
  e.to_poly();
  s << "[";
  for (unsigned int i= 0; i < e.b.size(); i++)
    {
      s << e.b[i] << " ";
    }
  s << "]";
  return s;
}

istream &operator>>(istream &s, Plaintext &e)
{
  int ch= s.get();
  while (isspace(ch))
    {
      ch= s.get();
    }

  if (ch != '[')
    {
      throw IO_Error("Bad Plaintext input: no '['");
    }
  for (unsigned int i= 0; i < e.b.size(); i++)
    {
      s >> e.b[i];
    }
  ch= s.get();
  while (isspace(ch))
    {
      ch= s.get();
    }

  if (ch != ']')
    {
      throw IO_Error("Bad Plaintext input: no ']'");
    }

  e.type= Polynomial;

  return s;
}

void Plaintext::assign_zero(PT_Type t)
{
  type= t;
  allocate();
  if (type != Polynomial)
    {
      a.resize((*Field_Data).phi_m());
      for (unsigned int i= 0; i < a.size(); i++)
        {
          a[i].assign_zero();
        }
    }
  if (type != Evaluation)
    {
      for (unsigned int i= 0; i < b.size(); i++)
        {
          b[i]= 0;
        }
    }
}

void Plaintext::assign_one(PT_Type t)
{
  type= t;
  allocate();
  if (type != Polynomial)
    {
      a.resize((*Field_Data).phi_m());
      for (unsigned int i= 0; i < a.size(); i++)
        {
          a[i].assign_one();
        }
    }
  if (type != Evaluation)
    {
      for (unsigned int i= 1; i < b.size(); i++)
        {
          b[i]= 0;
        }
      b[0]= 1;
    }
}

Plaintext &Plaintext::operator+=(const Plaintext &y)
{
  if (Field_Data != y.Field_Data)
    {
      throw data_mismatch();
    }

  to_poly();
  y.to_poly();

  if (b.size() != y.b.size())
    throw length_error("size mismatch");

  for (unsigned int i= 0; i < b.size(); i++)
    b[i]+= y.b[i];

  type= Polynomial;
  return *this;
}

void add(Plaintext &z, const Plaintext &x, const Plaintext &y)
{
  if (z.Field_Data != x.Field_Data)
    {
      throw data_mismatch();
    }
  if (z.Field_Data != y.Field_Data)
    {
      throw data_mismatch();
    }

  if (x.type == Both && y.type != Both)
    {
      z.type= y.type;
    }
  else if (y.type == Both && x.type != Both)
    {
      z.type= x.type;
    }
  else if (x.type != y.type)
    {
      throw data_mismatch();
    }
  else
    {
      z.type= x.type;
    }

  z.allocate();
  if (z.type != Polynomial)
    {
      z.a.resize(x.Field_Data->phi_m());
      for (unsigned int i= 0; i < z.a.size(); i++)
        {
          z.a[i].add(x.a[i], y.a[i]);
        }
    }
  if (z.type != Evaluation)
    {
      for (unsigned int i= 0; i < z.b.size(); i++)
        {
          z.b[i]= x.b[i] + y.b[i];
          if (z.b[i] > (*z.Field_Data).get_prime())
            {
              z.b[i]-= (*z.Field_Data).get_prime();
            }
        }
    }
}

void sub(Plaintext &z, const Plaintext &x, const Plaintext &y)
{
  if (z.Field_Data != x.Field_Data)
    {
      throw data_mismatch();
    }
  if (z.Field_Data != y.Field_Data)
    {
      throw data_mismatch();
    }

  if (x.type == Both && y.type != Both)
    {
      z.type= y.type;
    }
  else if (y.type == Both && x.type != Both)
    {
      z.type= x.type;
    }
  else if (x.type != y.type)
    {
      throw data_mismatch();
    }
  else
    {
      z.type= x.type;
    }

  z.allocate();
  if (z.type != Polynomial)
    {
      z.a.resize(z.Field_Data->phi_m());
      for (unsigned int i= 0; i < z.a.size(); i++)
        {
          z.a[i].sub(x.a[i], y.a[i]);
        }
    }
  if (z.type != Evaluation)
    {
      for (unsigned int i= 0; i < z.b.size(); i++)
        {
          z.b[i]= x.b[i];
          z.b[i]-= y.b[i];
          if (z.b[i] < 0)
            {
              z.b[i]+= (*z.Field_Data).get_prime();
            }
        }
    }
}

// Only needed for testing (I think)

void mul(Plaintext &z, const Plaintext &x, const Plaintext &y)
{
  if (z.Field_Data != x.Field_Data)
    {
      throw data_mismatch();
    }
  if (z.Field_Data != y.Field_Data)
    {
      throw data_mismatch();
    }

  if (y.type == Polynomial)
    {
      throw not_implemented();
    }
  if (x.type == Polynomial)
    {
      throw not_implemented();
    }
  z.type= Evaluation;

  z.allocate();
  for (unsigned int i= 0; i < z.a.size(); i++)
    {
      z.a[i].mul(x.a[i], y.a[i]);
    }
}

void sqr(Plaintext &z, const Plaintext &x)
{
  if (z.Field_Data != x.Field_Data)
    {
      throw data_mismatch();
    }

  if (x.type == Polynomial)
    {
      throw not_implemented();
    }
  z.type= Evaluation;

  z.allocate();
  for (unsigned int i= 0; i < z.a.size(); i++)
    {
      z.a[i].square(x.a[i]);
    }
}

void Plaintext::negate()
{
  if (type != Polynomial)
    {
      a.resize((*Field_Data).phi_m());
      for (unsigned int i= 0; i < a.size(); i++)
        {
          a[i].negate();
        }
    }
  if (type != Evaluation)
    {
      for (unsigned int i= 0; i < b.size(); i++)
        {
          if (b[i] != 0)
            {
              b[i]= (*Field_Data).get_prime() - b[i];
            }
        }
    }
}

bool Plaintext::equals(const Plaintext &x) const
{
  if (Field_Data != x.Field_Data)
    {
      return false;
    }
  if (type != x.type)
    {
      if (type == Evaluation)
        {
          x.from_poly();
        }
      else
        {
          from_poly();
        }
    }

  if (type != Polynomial and x.type != Polynomial)
    {
      for (unsigned int i= 0; i < a.size(); i++)
        {
          if (!a[i].equal(x.a[i]))
            {
              return false;
            }
        }
    }
  else
    {
      for (unsigned int i= 0; i < b.size(); i++)
        {
          if (b[i] != x.b[i])
            {
              return false;
            }
        }
    }
  return true;
}
