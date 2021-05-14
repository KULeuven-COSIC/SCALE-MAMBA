/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "aBit.h"
#include "Exceptions/Exceptions.h"
#include "LSSS/Open_Protocol2.h"
#include "OT_Thread_Data.h"

extern OT_Thread_Data OTD;

unsigned int aBit::n;
unsigned int aBit::whoami;
gf2n aBit::Delta;

void aBit::set_value(int b)
{
  if (b == 0)
    {
      x.assign_zero();
    }
  else if (b == 1)
    {
      x.assign_one();
    }
  else
    {
      throw bad_value();
    }
}

int aBit::get_value() const
{
  if (x.is_zero())
    {
      return 0;
    }
  else if (x.is_one())
    {
      return 1;
    }
  else
    {
      throw bad_value();
    }
}

void aBit::mult_by(const gf2n &y, const aBit &z)
{
  x.mul(z.x, y);
  for (unsigned int i= 0; i < n; i++)
    {
      M[i].mul(z.M[i], y);
      K[i].mul(z.K[i], y);
    }
}

void aBit::add(const gf2n &y)
{
  if (whoami == 0)
    x.add(y);
  else if (whoami)
    {
      if (y.is_one())
        K[0].add(Delta);
    }
}

void aBit::add(const aBit &y)
{
  x.add(y.x);
  for (unsigned int i= 0; i < n; i++)
    {
      M[i].add(M[i], y.M[i]);
      K[i].add(K[i], y.K[i]);
    }
}

void aBit::add(const aBit &xx, const aBit &yy)
{
  x.add(xx.x, yy.x);
  for (unsigned int i= 0; i < n; i++)
    {
      M[i].add(xx.M[i], yy.M[i]);
      K[i].add(xx.K[i], yy.K[i]);
    }
}

void aBit::negate()
{
  if (whoami == 0)
    {
      x.XOR(x, gf2n(1));
    }
  else
    {
      K[0].add(Delta);
    }
}

void aBit::get_Share_x_Delta_j(vector<gf2n> &Z) const
{
  Z= M;
  if (x.is_one())
    {
      Z[whoami]= Delta;
    }
  else
    {
      Z[whoami].assign_zero();
    }
  for (unsigned int k= 0; k < n; k++)
    {
      if (k != whoami)
        {
          Z[whoami].add(K[k]);
        }
    }
}

ostream &operator<<(ostream &s, const aBit &y)
{
  s << y.x << " : M ";
  for (unsigned int i= 0; i < y.n; i++)
    {
      s << y.M[i] << " ";
    }
  s << " : K ";
  for (unsigned int i= 0; i < y.n; i++)
    {
      s << y.K[i] << " ";
    }
  return s;
}

void aBit::output(ostream &s, bool human) const
{
  x.output(s, human);
  if (human)
    {
      s << " ";
    }
  for (unsigned int i= 0; i < M.size(); i++)
    {
      M[i].output(s, human);
      if (human)
        {
          s << " ";
        }
    }
  for (unsigned int i= 0; i < K.size(); i++)
    {
      K[i].output(s, human);
      if (human)
        {
          s << " ";
        }
    }
  s << endl;
}

void aBit::input(istream &s, bool human)
{
  x.input(s, human);
  for (unsigned int i= 0; i < M.size(); i++)
    {
      M[i].input(s, human);
    }
  for (unsigned int i= 0; i < K.size(); i++)
    {
      K[i].input(s, human);
    }
}

void check_Bits(const vector<aBit> &xB, Player &P)
{
  unsigned int sz= xB.size();
  vector<word> x(sz);
  P.OP2->Open_Bits(x, xB, P);
}

void check_Bits(const list<aBit> &xL, Player &P)
{
  unsigned int sz= xL.size();
  vector<word> x(sz);
  vector<aBit> xB;
  xB.reserve(sz);
  copy(begin(xL), end(xL), back_inserter(xB));
  P.OP2->Open_Bits(x, xB, P);
}

void aBit::randomize(unsigned int online_thread_no, Player &P)
{
  OTD.check();
  *this= OTD.aBD.get_aShare(online_thread_no);
#ifdef BENCH_OFFLINE
  P.abits+= 1;
#endif
}
