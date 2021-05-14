/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Share2.h"

#include "Mod2Engine/Mod2_Thread.h"

extern Mod2_Thread_Data MTD;

ShareData2 Share2::SD;

void Share2::assign_zero()
{
  for (unsigned int i= 0; i < SD.M.shares_per_player(p); i++)
    {
      a[i]= 0;
    }
}

void Share2::assign(const word &aa)
{
  for (unsigned int i= 0; i < SD.M.shares_per_player(p); i++)
    {
      a[i]= aa * SD.share_of_one[p][i];
    }
}

void Share2::assign_one()
{
  for (unsigned int i= 0; i < SD.M.shares_per_player(p); i++)
    {
      a[i]= SD.share_of_one[p][i];
    }
}

void Share2::set_shares(const vector<word> &aa)
{
  if (aa.size() != SD.M.shares_per_player(p))
    {
      throw invalid_length();
    }
  a= aa;
}

void Share2::set_player_and_shares(int pp, const vector<word> &aa)
{
  p= pp;
  if (aa.size() != SD.M.shares_per_player(p))
    {
      throw invalid_length();
    }
  a= aa;
}

void Share2::mul(const Share2 &S, const word &aa)
{
  if (p != S.p)
    {
      p= S.p;
      a.resize(SD.M.shares_per_player(p));
    }
  for (unsigned int i= 0; i < SD.M.shares_per_player(p); i++)
    {
      a[i]= S.a[i] & aa;
    }
}

void Share2::add(const Share2 &S, const word &aa)
{

  if (p != S.p)
    {
      p= S.p;
      a.resize(SD.M.shares_per_player(p));
    }

  word tmp;
  for (unsigned int i= 0; i < SD.M.shares_per_player(p); i++)
    {
      tmp= aa * SD.share_of_one[p][i];
      a[i]= S.a[i] ^ tmp;
    }
}

void Share2::add(const Share2 &S1, const Share2 &S2)
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
      a[i]= S1.a[i] ^ S2.a[i];
    }
}

void Share2::SHL(const Share2 &S1, unsigned int n)
{
  if (p != S1.p)
    {
      p= S1.p;
      a.resize(SD.M.shares_per_player(p));
    }
  for (unsigned int i= 0; i < SD.M.shares_per_player(p); i++)
    {
      a[i]= S1.a[i] << n;
    }
}

void Share2::SHR(const Share2 &S1, unsigned int n)
{
  if (p != S1.p)
    {
      p= S1.p;
      a.resize(SD.M.shares_per_player(p));
    }
  for (unsigned int i= 0; i < SD.M.shares_per_player(p); i++)
    {
      a[i]= S1.a[i] >> n;
    }
}

void Share2::multi_add(const Share2 &x1, const Share2 &y1,
                       const Share2 &x0, const Share2 &y0)
{
  if (x1.p != y1.p || x1.p != x0.p || x1.p != y0.p)
    {
      throw bad_value();
    }
  if (p != x1.p)
    {
      p= x1.p;
      a.resize(SD.M.shares_per_player(p));
    }
  for (unsigned int i= 0; i < SD.M.shares_per_player(p); i++)
    {
      a[i]= ((x1.a[i] ^ y1.a[i]) << 1) ^ (x0.a[i] ^ y0.a[i]);
    }
}

void Share2::duplicate(const Share2 &S1)
{
  if (p != S1.p)
    {
      p= S1.p;
      a.resize(SD.M.shares_per_player(p));
    }
  unsigned long one= 0xFFFFFFFFFFFFFFFFULL;
  for (unsigned int i= 0; i < SD.M.shares_per_player(p); i++)
    {
      a[i]= (S1.a[i] & 1ULL) * one;
    }
}

word combine(const vector<Share2> &S)
{
  vector<word> SS(Share2::SD.ReconS[0].size());
  int c= 0;
  for (unsigned int i= 0; i < Share2::SD.M.nplayers(); i++)
    {
      for (unsigned int k= 0; k < Share2::SD.M.shares_per_player(i); k++)
        {
          if (Share2::SD.RCt[i][0][k] == 1)
            {
              SS[c]= S[i].a[k];
              c++;
            }
        }
    }
  word ans= dot_product(SS, Share2::SD.ReconS[0]);
  return ans;
}

void Share2::output(ostream &s, bool human) const
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
      if (human)
        {
          s << a[i] << " ";
        }
      else
        {
          s.write((char *) &a[i], sizeof(word));
        }
    }
  if (human)
    {
      s << endl;
    }
}

void Share2::input(istream &s, bool human)
{
  if (s.peek() == EOF)
    {
      if (s.tellg() == 0)
        {
          throw file_error("Input Share2");
        }
      throw file_error("Input Share2");
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
      if (human)
        {
          s >> a[i];
        }
      else
        {
          s.read((char *) &a[i], sizeof(word));
        }
    }
}

void make_shares(vector<Share2> &share, const gf2 &val, PRNG &G)
{
  vector<gf2> ss(Share2::SD.M.row_dim());
  Share2::SD.M.Random_Sharing(ss, val, G);

  vector<word> sv;
  int c= 0;
  for (unsigned int i= 0; i < share.size(); i++)
    {
      sv.resize(Share2::SD.M.shares_per_player(i));
      for (unsigned int j= 0; j < Share2::SD.M.shares_per_player(i); j++)
        {
          sv[j]= ss[c + j].get();
        }
      c+= Share2::SD.M.shares_per_player(i);
      share[i].set_player_and_shares(i, sv);
    }
}

void make_shares(vector<Share2> &share, const word &val, PRNG &G)
{
  vector<word> ss(Share2::SD.M.row_dim());
  Share2::SD.Random_Sharing(ss, val, G);

  vector<word> sv;
  int c= 0;
  for (unsigned int i= 0; i < share.size(); i++)
    {
      sv.resize(Share2::SD.M.shares_per_player(i));
      for (unsigned int j= 0; j < Share2::SD.M.shares_per_player(i); j++)
        {
          sv[j]= ss[c + j];
        }
      c+= Share2::SD.M.shares_per_player(i);
      share[i].set_player_and_shares(i, sv);
    }
}

/* Create a const array of 2^i */
template<int N>
struct powers2
{
  constexpr powers2() : arr()
  {
    arr[0]= 1;
    for (unsigned int i= 1; i < N; i++)
      {
        arr[i]= arr[i - 1] << 1;
      }
  }
  word arr[N];
};

constexpr auto powers_of_two= powers2<64>();

void swap_shares(Share2 &S1, Share2 &S2, unsigned int i, unsigned int j)
{
  for (unsigned int k= 0; k < S1.SD.M.shares_per_player(S1.p); k++)
    {
      // Extract the bits
      word a0= S1.a[k] & powers_of_two.arr[i];
      word b0= S2.a[k] & powers_of_two.arr[j];
      // Remove this bit from existing words
      S1.a[k]^= a0;
      S2.a[k]^= b0;
      // Now shift the bits to the correct position in the other word
      if (i > j)
        {
          a0= a0 >> (i - j);
          b0= b0 << (i - j);
        }
      else
        {
          a0= a0 << (j - i);
          b0= b0 >> (j - i);
        }
      // Now stick back in the other word
      S1.a[k]^= b0;
      S2.a[k]^= a0;
    }
}

void Share2::randomize(unsigned int online_thread_no, Player &P)
{
  MTD.check();
  vector<Share2> T(3);
  MTD.get_Single_Triple(T, online_thread_no);
#ifdef BENCH_OFFLINE
  P.mod2s+= 1;
#endif
  *this= T[0];
}
