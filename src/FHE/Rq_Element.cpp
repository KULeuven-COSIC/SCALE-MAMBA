/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#include "Rq_Element.h"
#include "Exceptions/Exceptions.h"

Rq_Element::Rq_Element(const vector<FFT_Data> &prd, RepType r0, RepType r1)
{
  if (prd.size() > 0)
    a.push_back({prd[0], r0});
  if (prd.size() > 1)
    a.push_back({prd[1], r1});
  lev= 1;
}

void Rq_Element::set_data(const vector<FFT_Data> &prd, RepType r0, RepType r1)
{
  a.resize(2);
  a[0].set_data(prd[0], r0);
  a[1].set_data(prd[1], r1);
  lev= 1;
}

void Rq_Element::assign_zero(const vector<FFT_Data> &prd)
{
  set_data(prd, a[0].representation(), a[1].representation());
  assign_zero();
}

void Rq_Element::assign_zero()
{
  a[0].assign_zero();
  if (lev != 0)
    {
      a[1].assign_zero();
    }
}

void Rq_Element::assign_one()
{
  a[0].assign_one();
  if (lev != 0)
    {
      a[1].assign_one();
    }
}

void Rq_Element::assign(const Rq_Element &other)
{
  lev= other.lev;
  a[0]= other.a[0];
  if (lev != 0)
    {
      a[1]= other.a[1];
    }
  else
    {
      a[1].partial_assign(other.a[1]);
    }
}

void Rq_Element::negate()
{
  a[0].negate();
  if (lev != 0)
    {
      a[1].negate();
    }
}

void add(Rq_Element &ans, const Rq_Element &ra, const Rq_Element &rb)
{
  if (ra.lev != rb.lev)
    {
      throw level_mismatch();
    }
  ans.lev= ra.lev;
  add(ans.a[0], ra.a[0], rb.a[0]);
  if (ans.lev != 0)
    {
      add(ans.a[1], ra.a[1], rb.a[1]);
    }
  else
    {
      ans.a[1].partial_assign(ra.a[1]);
    }
}

void sub(Rq_Element &ans, const Rq_Element &a, const Rq_Element &b)
{
  if (a.lev != b.lev)
    {
      throw level_mismatch();
    }
  ans.lev= a.lev;
  sub(ans.a[0], a.a[0], b.a[0]);
  if (ans.lev != 0)
    {
      sub(ans.a[1], a.a[1], b.a[1]);
    }
  else
    {
      ans.a[1].partial_assign(a.a[1]);
    }
}

void mul(Rq_Element &ans, const Rq_Element &a, const Rq_Element &b)
{
  if (a.lev != b.lev)
    {
      throw level_mismatch();
    }
  ans.lev= a.lev;
  mul(ans.a[0], a.a[0], b.a[0]);
  if (ans.lev != 0)
    {
      mul(ans.a[1], a.a[1], b.a[1]);
    }
  else
    {
      ans.a[1].partial_assign(a.a[1]);
    }
}

void mul(Rq_Element &ans, const Rq_Element &a, const bigint &b)
{
  ans.lev= a.lev;
  modp bp;
  to_modp(bp, b, a.a[0].get_prD());
  mul(ans.a[0], a.a[0], bp);
  if (ans.lev != 0)
    {
      to_modp(bp, b, a.a[1].get_prD());
      mul(ans.a[1], a.a[1], bp);
    }
  else
    {
      ans.a[1].partial_assign(a.a[1]);
    }
}

// Multiply by X^i
void mul_by_X_i(Rq_Element &ans, const Rq_Element &a, unsigned int j)
{
  ans.lev= a.lev;
  mul_by_X_i(ans.a[0], a.a[0], j);
  if (ans.lev != 0)
    {
      mul_by_X_i(ans.a[1], a.a[1], j);
    }
}

void Rq_Element::randomize(PRNG &G, int l)
{
  lev= l;
  a[0].randomize(G);
  if (lev != 0)
    {
      a[1].randomize(G);
    }
}

bool Rq_Element::equals(const Rq_Element &other) const
{
  if (lev != other.lev)
    {
      throw level_mismatch();
    }
  for (int i= 0; i <= lev; ++i)
    {
      if (!a[i].equals(other.a[i]))
        return false;
    }
  return true;
}

void Rq_Element::from_vec(const vector<bigint> &v, int level)
{
  lev= level;
  for (int i= 0; i <= lev; ++i)
    {
      a[i].from_vec(v);
    }
}

void Rq_Element::from_vec(const vector<int> &v, int level)
{
  lev= level;
  for (int i= 0; i <= lev; ++i)
    {
      a[i].from_vec(v);
    }
}

template<class T>
void Rq_Element::from(const Generator<T> &generator, int level)
{
  lev= level;
  if (lev == 1)
    {
      auto clone= generator.clone();
      a[1].from(*clone);
      delete clone;
    }
  a[0].from(generator);
}

vector<bigint> Rq_Element::to_vec_bigint() const
{
  vector<bigint> v;
  to_vec_bigint(v);
  return v;
}

// Doing sort of CRT;
// result mod p0 = a[0]; result mod p1 = a[1]
void Rq_Element::to_vec_bigint(vector<bigint> &v) const
{
  a[0].to_vec_bigint(v);
  if (lev == 0)
    { // Normalise in this case
      bigint p0= a[0].get_prime();
      for (size_t i= 0; i < v.size(); ++i)
        {
          if (v[i] > p0 / 2)
            {
              v[i]= (v[i] - p0);
            }
        }
    }
  else
    {
      vector<bigint> v1;
      a[1].to_vec_bigint(v1);
      bigint p0= a[0].get_prime();
      bigint p1= a[1].get_prime();
      bigint p0i, lambda, Q= p0 * p1;
      invMod(p0i, p0 % p1, p1);
      for (unsigned int i= 0; i < v.size(); i++)
        {
          lambda= ((v1[i] - v[i]) * p0i) % Q;
          v[i]= (v[i] + p0 * lambda) % Q;
        }
    }
}

ConversionIterator Rq_Element::get_iterator()
{
  if (lev != 0)
    throw not_implemented();
  return a[0].get_iterator();
}

bigint Rq_Element::infinity_norm() const
{
  bigint p0= a[0].get_prime();
  bigint p1= a[1].get_prime();
  bigint ans= 0, t, Q= p0 * p1;
  vector<bigint> te= to_vec_bigint();
  for (unsigned int i= 0; i < te.size();
       i++)
    { // Take rounded value and then abs value
      if (te[i] < Q / 2)
        {
          t= te[i];
        }
      else
        {
          t= Q - te[i];
        }
      if (t > ans)
        {
          ans= t;
        }
    }
  return ans;
}

void Rq_Element::change_rep(RepType r)
{
  if (lev == 1)
    {
      throw level_mismatch();
    }
  a[0].change_rep(r);
}

void Rq_Element::change_rep(RepType r0, RepType r1)
{
  if (lev == 0)
    {
      throw level_mismatch();
    }
  a[0].change_rep(r0);
  a[1].change_rep(r1);
}

void Rq_Element::Scale(const bigint &p)
{
  if (lev == 0)
    {
      return;
    }

  bigint p0= a[0].get_prime(), p1= a[1].get_prime(), p1i, lambda, n= p1 * p;
  invMod(p1i, p1 % p, p);

  // First multiply input by [p1]_p
  bigint te= p1 % p;
  if (te > p / 2)
    {
      te-= p;
    }
  modp tep;
  to_modp(tep, te, a[0].get_prD());
  mul(a[0], a[0], tep);
  to_modp(tep, te, a[1].get_prD());
  mul(a[1], a[1], tep);

  // Now compute delta
  Ring_Element b0(a[0].get_FFTD(), evaluation);
  Ring_Element b1(a[1].get_FFTD(), evaluation);
  {
    auto poly_a1= a[1];
    poly_a1.change_rep(polynomial);
    auto it= poly_a1.get_iterator();
    auto it0= b0.get_write_iterator();
    auto it1= b1.get_write_iterator();
    bigint half_n= n / 2;
    bigint delta;
    for (int i= 0; i < a[1].get_FFTD().phi_m(); i++)
      {
        it.get(delta);
        lambda= delta;
        lambda*= p1i;
        lambda%= p;
        lambda*= p1;
        lambda-= delta;
        lambda%= n;
        if (lambda > half_n)
          lambda-= n;
        it0.get(lambda);
        it1.get(lambda);
      }
  }

  Rq_Element bb(b0, b1);
  add(*this, *this, bb);

  // Now divide by p1 mod p0
  modp p1_inv, pp;
  to_modp(pp, p1, a[0].get_prD());
  Inv(p1_inv, pp, a[0].get_prD());
  lev= 0;
  mul(a[0], a[0], p1_inv);
}

void Rq_Element::mul_by_p1()
{
  lev= 1;
  bigint m= a[1].get_prime() % a[0].get_prime();
  modp mp;
  to_modp(mp, m, a[0].get_prD());
  mul(a[0], a[0], mp);
  a[1].assign_zero();
}

void Rq_Element::raise_level()
{
  if (lev == 1)
    {
      return;
    }
  lev= 1;
  a[1].from(a[0].get_copy_iterator());
}

void Rq_Element::output(ostream &s, bool human) const
{
  if (human)
    {
      vector<bigint> va= to_vec_bigint();
      s << "[ ";
      for (unsigned int i= 0; i < va.size(); i++)
        {
          s << va[i] << " ";
        }
      s << " ]";
    }
  else
    {
      s.write((char *) &lev, sizeof(lev));
      for (int i= 0; i <= lev; i++)
        {
          a[i].output(s);
        }
    }
}

void Rq_Element::input(istream &s)
{
  s.read((char *) &lev, sizeof(lev));
  for (int i= 0; i <= lev; i++)
    a[i].input(s);
}

bool Rq_Element::is_zero() const
{
  if (!a[0].is_zero())
    {
      return false;
    }
  if (lev == 1 && !a[1].is_zero())
    {
      return false;
    }
  return true;
}

template void Rq_Element::from<bigint>(const Generator<bigint> &, int);
template void Rq_Element::from<int>(const Generator<int> &, int);
