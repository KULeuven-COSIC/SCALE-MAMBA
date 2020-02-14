/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include <queue>
using namespace std;

#include "MSP.h"

void MSP::assign(const MSP &M)
{
  Gen= M.Gen;
  ns= M.ns;
}

ostream &operator<<(ostream &s, const MSP &M)
{
  s << M.Gen << endl;
  s << M.ns.size() << endl;
  for (unsigned int i= 0; i < M.ns.size(); i++)
    {
      s << M.ns[i] << " ";
    }
  s << endl;

  return s;
}

istream &operator>>(istream &s, MSP &M)
{
  s >> M.Gen;
  unsigned int sz;
  s >> sz;
  M.ns.resize(sz);
  for (unsigned int i= 0; i < sz; i++)
    {
      s >> M.ns[i];
    }

  return s;
}

void MSP::Initialize_Full_Threshold(unsigned int nn)
{
  ns.resize(nn);
  Gen.resize(nn, vector<gfp>(nn));
  for (unsigned int i= 0; i < nn; i++)
    {
      ns[i]= 1;
      for (unsigned int j= 0; j < nn; j++)
        {
          Gen[i][j].assign_zero();
        }
      Gen[i][i].assign_one();
    }
}

void MSP::Initialize_Shamir(unsigned int nn, unsigned int t)
{
  ns.resize(nn);

  Gen.resize(nn, vector<gfp>(t + 1));
  for (unsigned int i= 0; i < nn; i++)
    {
      Gen[i][0].assign_one();
      gfp te, pow;
      te.assign((int) i + 1);
      pow= te;
      for (unsigned int j= 1; j < t + 1; j++)
        {
          Gen[i][j].add(pow, Gen[i][0]);
          pow.mul(pow, te);
        }
      ns[i]= 1;
    }
}

void MSP::Initialize_Replicated(const CAS &AS)
{
  // Compute number of shares in total
  int m= 0;
  unsigned int szk= AS.delta_plus.size();
  for (unsigned int i= 0; i < szk; i++)
    {
      m+= (AS.n - row_sum(AS.delta_plus, i));
    }
  Gen.resize(m, vector<gfp>(szk));
  ns.resize(AS.n);
  int k= 0;
  for (unsigned int i= 0; i < AS.n; i++)
    {
      ns[i]= 0;
      for (unsigned int j= 0; j < szk; j++)
        {
          if (AS.delta_plus[j][i] == 0)
            {
              for (unsigned int t= 0; t < szk; t++)
                {
                  Gen[k][t].assign_zero();
                }
              Gen[k][j].assign_one();
              ns[i]++;
              k++;
            }
        }
    }
}

gfp_matrix MSP::Make_Parity() const
{
  unsigned int nr= Gen.size(), nc= Gen[0].size();
  gfp_matrix GG(nr, vector<gfp>(nr + nc));
  for (unsigned int i= 0; i < nr; i++)
    {
      for (unsigned int j= 0; j < nc; j++)
        {
          GG[i][j]= Gen[i][j];
        }
      for (unsigned int j= 0; j < nr; j++)
        {
          GG[i][j + nc].assign_zero();
        }
      GG[i][i + nc].assign_one();
    }

  Gauss_Elim(GG, nc);

  gfp_matrix P(nr, vector<gfp>(nr));
  for (unsigned int i= 0; i < nr; i++)
    {
      for (unsigned int j= 0; j < nr; j++)
        {
          P[i][j]= GG[i][j + nc];
        }
    }
  return P;
}

/* Given a set of rows of Gen in Mrows this creates the
 * reconstruction vector for the share s, and the
 * reconstruction matrix for the set of ALL shares vec{s}.
 * Assumes Mrows is full rank
 */
void MSP::Make_Recon(vector<gfp> &ReconS, gfp_matrix &ReconSS,
                     const vector<int> &MRows) const
{
  int nr= 0, nc= Gen[0].size();
  for (unsigned int i= 0; i < Gen.size(); i++)
    {
      if (MRows[i] == 1)
        {
          nr++;
        }
    }

  gfp_matrix A(nr, vector<gfp>(nc + nr));
  /* Add in rows of people who send me stuff */
  int c= 0;
  for (unsigned int r= 0; r < Gen.size(); r++)
    {
      if (MRows[r] == 1)
        {
          for (int k= 0; k < nc; k++)
            {
              A[c][k]= Gen[r][k];
            }
          for (int k= 0; k < nr; k++)
            {
              A[c][nc + k].assign_zero();
            }
          A[c][c + nc].assign_one();
          c++;
        }
    }

  Gauss_Elim(A, nc);

  /* Set reconstruction vector for player i */
  gfp_matrix B(nr, vector<gfp>(nr));

  ReconS.resize(nr);
  for (int i= 0; i < nr; i++)
    {
      ReconS[i].assign_zero();
      for (int j= 0; j < nr; j++)
        {
          ReconS[i].add(A[j][i + nc]);
          B[i][j]= A[i][j + nc];
        }
    }

  /* Now compute reconstruction matrix for player i */
  ReconSS= Mul(Gen, B);
}

vector<gfp> MSP::Random_Sharing(const gfp &val, PRNG &G) const
{
  unsigned int k= Gen[0].size();
  vector<gfp> kk(k);
  kk[0]= val;
  for (unsigned int i= 1; i < k; i++)
    {
      kk[i].randomize(G);
      kk[0].sub(kk[i]);
    }
  return Sharing(kk);
}

bool MSP::check_qualified(const vector<int> &players) const
{
  int nc= 0;
  for (unsigned int i= 0; i < players.size(); i++)
    {
      if (players[i] == 1)
        {
          nc+= ns[i];
        }
    }
  if (nc == 0)
    {
      return false;
    }

  gfp_matrix A(Gen[0].size(), vector<gfp>(nc + 1));
  int c= 0, i= 0, cc= 0;
  while (c < nc)
    {
      if (players[i] == 1)
        {
          for (unsigned int j= 0; j < ns[i]; j++)
            {
              for (unsigned int k= 0; k < A.size(); k++)
                {
                  A[k][c]= Gen[cc + j][k];
                }
              c++;
            }
        }
      cc= cc + ns[i];
      i++;
    }
  for (unsigned int i= 0; i < Gen[0].size(); i++)
    {
      A[i][nc]= 1;
    }

  return Solvable(A);
}

class msp_node
{
public:
  vector<int> v;
  int code;
};

/* Finds all unqualified sets */
imatrix MSP::find_all_unqualified() const
{
  imatrix Unqual;
  queue<msp_node> q;
  msp_node Nd;
  Nd.v.resize(ns.size());
  for (unsigned int i= 0; i < ns.size(); i++)
    {
      Nd.v[i]= 0;
    }
  Nd.code= 0;
  vector<bool> used(1 << ns.size());
  for (unsigned int i= 0; i < (1UL << ns.size()); i++)
    {
      used[i]= false;
    }

  q.push(Nd);
  while (!q.empty())
    {
      Nd= q.front();
      q.pop();
      bool flag= check_qualified(Nd.v);
      if (flag == false)
        {
          Unqual.resize(Unqual.size() + 1, vector<int>(ns.size()));
          for (unsigned int i= 0; i < ns.size(); i++)
            {
              Unqual[Unqual.size() - 1][i]= Nd.v[i];
            }
          for (unsigned int i= 0; i < ns.size(); i++)
            {
              msp_node Nd1= Nd;
              if (Nd1.v[i] == 0)
                {
                  Nd1.v[i]= 1;
                  Nd1.code+= (1 << i);
                }
              if (used[Nd1.code] == false)
                {
                  used[Nd1.code]= true;
                  q.push(Nd1);
                }
            }
        }
    }
  return Unqual;
}

/* Extracts matrix rows corresponding to indicator set players */
gfp_matrix MSP::extract_rows(const vector<int> &players) const
{
  unsigned int nr= Gen.size(), nc= Gen[0].size();
  gfp_matrix A(nr, vector<gfp>(nc));

  unsigned int c= 0, r= 0;
  for (unsigned int i= 0; i < players.size(); i++)
    {
      if (players[i] == 1)
        {
          for (unsigned int j= 0; j < ns[i]; j++)
            {
              for (unsigned int k= 0; k < nc; k++)
                {
                  A[r][k]= Gen[c + j][k];
                }
              r++;
            }
        }
      c+= ns[i];
    }
  A.resize(r);
  return A;
}

/* Gets full reconstruction vector (i.e. over all rows) for a qualified
 * set
 */
vector<gfp> MSP::get_full_reconstruct(const vector<int> &qual) const
{
  unsigned int nr= Gen.size(), nc= Gen[0].size();
  gfp_matrix A(nr, vector<gfp>(nc + nr));

  int c= 0;
  for (unsigned int i= 0; i < qual.size(); i++)
    {
      for (unsigned int j= 0; j < ns[i]; j++)
        {
          for (unsigned int k= 0; k < nc + nr; k++)
            {
              if (qual[i] == 1 && k < nc)
                {
                  A[c][k]= Gen[c][k];
                }
              else
                {
                  A[c][k].assign_zero();
                }
            }
          A[c][c + nc]= 1;
          c++;
        }
    }

  Gauss_Elim(A, nc);

  vector<gfp> ans(nr);
  c= 0;
  for (unsigned int i= 0; i < qual.size(); i++)
    {
      for (unsigned int j= 0; j < ns[i]; j++)
        {
          ans[c].assign_zero();
          if (qual[i] == 1)
            {
              for (unsigned int j= 0; j < nr; j++)
                {
                  ans[c].add(A[j][c + nc]);
                }
            }
          c++;
        }
    }

  return ans;
}

MSP MSP::make_multiplicative(Schur_Matrices &Sch) const
{
  imatrix Unqual= find_all_unqualified();
  CAS AS(Unqual, true);

  if (!AS.is_q(2))
    {
      throw not_Q2();
    }

  bool ismult= Sch.initialize(*this);
  if (ismult)
    {
      return *this;
    }

  /* Now create the new MSP
   * Only need to go up add in the  minimum of gamma_minus.size() 
   * and G[0].size() new columns (minus 1)
   */
  unsigned int nnew= min(AS.gamma_minus.size(), Gen[0].size());
  gfp_matrix Gen2(2 * Gen.size(),
                  vector<gfp>(Gen[0].size() + nnew - 1));
  vector<unsigned int> ns2(ns.size());
  for (unsigned int i= 0; i < Gen2.size(); i++)
    {
      for (unsigned int j= 0; j < Gen2[0].size(); j++)
        {
          Gen2[i][j].assign_zero();
        }
    }
  /* First copy original matrix into place 
   * and put the first column into the new last columns
   */
  int c1= 0, c2= 0, c;
  for (unsigned int i= 0; i < ns.size(); i++)
    {
      for (unsigned int j= 0; j < ns[i]; j++)
        {
          for (unsigned int t= 0; t < Gen[0].size(); t++)
            {
              Gen2[c2][t]= Gen[c1][t];
              if (t == 0)
                {
                  for (unsigned int k= 0; k < nnew - 1; k++)
                    {
                      Gen2[c2][k + Gen[0].size()]= Gen[c1][t];
                    }
                }
            }
          c1++;
          c2++;
        }
      c2+= ns[i];
      ns2[i]= 2 * ns[i];
    }
  c1= 0;
  for (unsigned int i= 0; i < nnew; i++)
    {
      vector<gfp> te= get_full_reconstruct(AS.gamma_minus[i]);
      // Insert in column c1 (or in first Gen[0].size() columns)
      c= 0;
      c2= 0;
      for (unsigned int j= 0; j < ns.size(); j++)
        {
          for (unsigned int t= 0; t < ns[j]; t++)
            {
              if (c1 != 0)
                {
                  Gen2[c + ns[j] + t][c1]= te[c2];
                }
              else
                {
                  for (unsigned int k= 0; k < Gen[0].size(); k++)
                    {
                      Gen2[c + ns[j] + t][k]= te[c2];
                    }
                }
              c2++;
            }
          c+= ns2[j];
        }
      if (c1 == 0)
        {
          c1= Gen[0].size();
        }
      else
        {
          c1++;
        }
    }

  MSP M2(Gen2, ns2);
  ismult= Sch.initialize(M2);
  if (!ismult)
    {
      throw bad_value();
    }
  return M2;
}
