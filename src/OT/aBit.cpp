/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#include "aBit.h"
#include "Exceptions/Exceptions.h"

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

/* Open (and check) a vector of aBits */
void Open_aBits(vector<int> &dv, const vector<aBit> &v, Player &P, const gf2n &Delta)
{
  if (dv.size() != v.size())
    {
      throw bad_value();
    }
  vector<string> o(P.nplayers());
  for (unsigned int i= 0; i < P.nplayers(); i++)
    {
      if (i != P.whoami())
        {
          ostringstream ss;
          for (unsigned int t= 0; t < v.size(); t++)
            {
              ss << (char) v[t].get_value();
              v[t].get_MAC(i).output(ss);
            }
          o[i]= ss.str();
        }
    }

  P.Send_Distinct_And_Receive(o);

  for (unsigned int t= 0; t < v.size(); t++)
    {
      dv[t]= v[t].get_value();
    }

  char c;
  gf2n M;
  for (unsigned int i= 0; i < P.nplayers(); i++)
    {
      if (i != P.whoami())
        {
          istringstream is(o[i]);
          for (unsigned int t= 0; t < v.size(); t++)
            {
              is >> c;
              M.input(is);
              dv[t]= dv[t] ^ ((unsigned int) c);
              if (c == 1)
                {
                  M.add(Delta);
                }
              if (M != v[t].get_Key(i))
                {
                  throw OT_error();
                }
            }
        }
    }
}

void Open_aBits_To(vector<int> &dv, unsigned int j, const vector<aBit> &v,
                   Player &P, const gf2n &Delta)
{
  if (j != P.whoami())
    {
      dv.resize(0);
      ostringstream ss;
      for (unsigned int t= 0; t < v.size(); t++)
        {
          ss << (char) v[t].get_value();
          v[t].get_MAC(j).output(ss);
        }
      P.send_to_player(j, ss.str());
    }
  else
    {
      dv.resize(v.size());
      for (unsigned int t= 0; t < v.size(); t++)
        {
          dv[t]= v[t].get_value();
        }

      char c;
      gf2n M;
      for (unsigned int i= 0; i < P.nplayers(); i++)
        {
          if (i != P.whoami())
            {
              string ss;
              P.receive_from_player(i, ss);
              istringstream is(ss);
              for (unsigned int t= 0; t < v.size(); t++)
                {
                  is >> c;
                  M.input(is);
                  dv[t]= dv[t] ^ ((unsigned int) c);
                  if (c == 1)
                    {
                      M.add(Delta);
                    }
                  if (M != v[t].get_Key(i))
                    {
                      throw OT_error();
                    }
                }
            }
        }
    }
}
