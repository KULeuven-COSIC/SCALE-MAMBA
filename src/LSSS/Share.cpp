/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Share.h"
#include "Math/gfp.h"

ShareData Share::SD;

void Share::assign_zero()
{
  if (SD.type == Full)
    {
      a[0].assign_zero();
      for (unsigned int i= 0; i < SD.nmacs; i++)
        {
          mac[i].assign_zero();
        }
    }
  else if (SD.type == Shamir)
    {
      a[0].assign_zero();
    }
  else
    {
      for (unsigned int i= 0; i < SD.M.shares_per_player(p); i++)
        {
          a[i].assign_zero();
        }
    }
}

void Share::assign(const gfp &aa, const vector<gfp> &alphai)
{
  if (SD.type == Other || SD.type == Replicated || SD.type == Q2MSP)
    {
      for (unsigned int i= 0; i < SD.M.shares_per_player(p); i++)
        {
          a[i].mul(aa, SD.share_of_one[p][i]);
        }
    }
  else
    {
      if (p == 0 || SD.type == Shamir)
        a[0]= aa;
      else
        a[0].assign_zero();

      if (SD.type == Full)
        {
          for (unsigned int i= 0; i < SD.nmacs; i++)
            {
              mac[i].mul(aa, alphai[i]);
            }
        }
    }
}

void Share::set_shares(const vector<gfp> &aa)
{
  if (aa.size() != SD.M.shares_per_player(p))
    {
      throw invalid_length();
    }
  a= aa;
}

void Share::set_macs(const vector<gfp> &aa)
{
  if (aa.size() != SD.nmacs)
    {
      throw invalid_length();
    }
  mac= aa;
}

void Share::set_player_and_shares(int pp, const vector<gfp> &aa)
{
  p= pp;
  if (aa.size() != SD.M.shares_per_player(p))
    {
      throw invalid_length();
    }
  a= aa;
}

void Share::mul(const Share &S, const gfp &aa)
{
  if (p != S.p)
    {
      p= S.p;
      a.resize(SD.M.shares_per_player(p));
    }
  for (unsigned int i= 0; i < SD.M.shares_per_player(p); i++)
    {
      a[i].mul(S.a[i], aa);
    }
  if (SD.type == Full)
    {
      for (unsigned int i= 0; i < SD.nmacs; i++)
        {
          mac[i].mul(S.mac[i], aa);
        }
    }
}

void Share::add(const Share &S, const gfp &aa, const vector<gfp> &alphai)
{

  if (p != S.p)
    {
      p= S.p;
      a.resize(SD.M.shares_per_player(p));
    }

  gfp tmp;
  if (SD.type == Other || SD.type == Replicated || SD.type == Q2MSP)
    {
      for (unsigned int i= 0; i < SD.M.shares_per_player(p); i++)
        {
          tmp.mul(aa, SD.share_of_one[p][i]);
          a[i].add(S.a[i], tmp);
        }
    }
  else
    {
      if (p == 0 || SD.type == Shamir)
        {
          a[0].add(S.a[0], aa);
        }
      else
        {
          a[0]= S.a[0];
        }

      if (SD.type == Full)
        {
          for (unsigned int i= 0; i < SD.nmacs; i++)
            {
              tmp.mul(alphai[i], aa);
              mac[i].add(S.mac[i], tmp);
            }
        }
    }
}

void Share::sub(const Share &S, const gfp &aa, const vector<gfp> &alphai)
{

  if (p != S.p)
    {
      p= S.p;
      a.resize(SD.M.shares_per_player(p));
    }

  gfp tmp;
  if (SD.type == Other || SD.type == Replicated || SD.type == Q2MSP)
    {
      for (unsigned int i= 0; i < SD.M.shares_per_player(p); i++)
        {
          tmp.mul(aa, SD.share_of_one[p][i]);
          a[i].sub(S.a[i], tmp);
        }
    }
  else
    {
      if (p == 0 || SD.type == Shamir)
        {
          a[0].sub(S.a[0], aa);
        }
      else
        {
          a[0]= S.a[0];
        }

      if (SD.type == Full)
        {
          gfp tmp;
          for (unsigned int i= 0; i < SD.nmacs; i++)
            {
              tmp.mul(alphai[i], aa);
              mac[i].sub(S.mac[i], tmp);
            }
        }
    }
}

void Share::sub(const gfp &aa, const Share &S, const vector<gfp> &alphai)
{
  if (p != S.p)
    {
      p= S.p;
      a.resize(SD.M.shares_per_player(p));
    }

  gfp tmp;
  if (SD.type == Other || SD.type == Replicated || SD.type == Q2MSP)
    {
      for (unsigned int i= 0; i < SD.M.shares_per_player(p); i++)
        {
          tmp.mul(aa, SD.share_of_one[p][i]);
          a[i].sub(tmp, S.a[i]);
        }
    }
  else
    {
      if (p == 0 || SD.type == Shamir)
        {
          a[0].sub(aa, S.a[0]);
        }
      else
        {
          a[0]= S.a[0];
          a[0].negate();
        }

      if (SD.type == Full)
        {
          gfp tmp;
          for (unsigned int i= 0; i < SD.nmacs; i++)
            {
              tmp.mul(alphai[i], aa);
              mac[i].sub(tmp, S.mac[i]);
            }
        }
    }
}

void Share::add(const Share &S1, const Share &S2)
{
  if (S1.p != S2.p)
    {
      throw bad_value();
    }
  if (p != S1.p)
    {
      p= S1.p;
      a.resize(SD.M.shares_per_player(p));
    }
  for (unsigned int i= 0; i < SD.M.shares_per_player(p); i++)
    {
      a[i].add(S1.a[i], S2.a[i]);
    }

  if (SD.type == Full)
    {
      for (unsigned int i= 0; i < SD.nmacs; i++)
        {
          mac[i].add(S1.mac[i], S2.mac[i]);
        }
    }
}

void Share::sub(const Share &S1, const Share &S2)
{
  if (S1.p != S2.p)
    {
      throw bad_value();
    }
  if (p != S1.p)
    {
      p= S1.p;
      a.resize(SD.M.shares_per_player(p));
    }
  for (unsigned int i= 0; i < SD.M.shares_per_player(p); i++)
    {
      a[i].sub(S1.a[i], S2.a[i]);
    }
  if (SD.type == Full)
    {
      for (unsigned int i= 0; i < SD.nmacs; i++)
        {
          mac[i].sub(S1.mac[i], S2.mac[i]);
        }
    }
}

void Share::negate()
{
  for (unsigned int i= 0; i < SD.M.shares_per_player(p); i++)
    {
      a[i].negate();
    }
  if (SD.type == Full)
    {
      for (unsigned int i= 0; i < SD.nmacs; i++)
        {
          mac[i].negate();
        }
    }
}

gfp combine(const vector<Share> &S)
{
  gfp ans;
  if (Share::SD.type == Full)
    {
      ans= S[0].a[0];
      for (unsigned int i= 1; i < S.size(); i++)
        {
          ans.add(ans, S[i].a[0]);
        }
    }
  else
    {
      vector<gfp> SS(Share::SD.ReconS[0].size());
      int c= 0;
      for (unsigned int i= 0; i < Share::SD.M.nplayers(); i++)
        {
          for (unsigned int k= 0; k < Share::SD.M.shares_per_player(i); k++)
            {
              if (Share::SD.RCt[i][0][k] == 1)
                {
                  SS[c]= S[i].a[k];
                  c++;
                }
            }
        }
      ans= dot_product(SS, Share::SD.ReconS[0]);
    }
  return ans;
}

bool check_macs(const vector<Share> &S, const vector<gfp> &key)
{
  if (Share::SD.type != Full)
    {
      return true;
    }

  gfp val= combine(S);
  // Now check the MAC is valid
  for (unsigned int j= 0; j < Share::SD.nmacs; j++)
    {
      val.mul(val, key[j]);
      for (unsigned i= 0; i < S.size(); i++)
        {
          val.sub(val, S[i].mac[j]);
        }
      if (!val.is_zero())
        {
          return false;
        }
    }
  return true;
}

void Share::output(ostream &s, bool human) const
{
  if (human)
    {
      s << p << " ";
    }
  else
    {
      char ch= (char) p;
      s.write(&ch, 1);
    }
  for (unsigned int i= 0; i < SD.M.shares_per_player(p); i++)
    {
      a[i].output(s, human);
      if (human)
        {
          s << " ";
        }
    }
  if (SD.type == Full)
    {
      for (unsigned int i= 0; i < SD.nmacs; i++)
        {
          mac[i].output(s, human);
          if (human)
            {
              s << " ";
            }
        }
    }
  if (human)
    {
      s << endl;
    }
}

void Share::input(istream &s, bool human)
{
  if (s.peek() == EOF)
    {
      if (s.tellg() == 0)
        {
          throw file_error("Input Share");
        }
      throw file_error("Input Share");
    }

  int t;
  if (human)
    {
      s >> t;
    }
  else
    {
      char ch;
      s.read(&ch, 1);
      t= (int) ch;
    }
  if (t != p)
    {
      p= t;
      a.resize(SD.M.shares_per_player(p));
    }
  for (unsigned int i= 0; i < SD.M.shares_per_player(p); i++)
    {
      a[i].input(s, human);
    }
  if (SD.type == Full)
    {
      for (unsigned int i= 0; i < SD.nmacs; i++)
        {
          mac[i].input(s, human);
        }
    }
}

void make_shares(vector<Share> &share, const gfp &val, PRNG &G)
{
  vector<gfp> ss= Share::SD.M.Random_Sharing(val, G);

  vector<gfp> sv;
  int c= 0;
  for (unsigned int i= 0; i < share.size(); i++)
    {
      sv.resize(Share::SD.M.shares_per_player(i));
      for (unsigned int j= 0; j < Share::SD.M.shares_per_player(i); j++)
        {
          sv[j]= ss[c + j];
        }
      c+= Share::SD.M.shares_per_player(i);
      share[i].set_player_and_shares(i, sv);
    }
}
