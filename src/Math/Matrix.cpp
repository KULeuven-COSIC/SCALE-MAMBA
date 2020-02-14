/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Math/Matrix.h"
#include "Exceptions/Exceptions.h"

#include <iostream>
#include <stdlib.h>

using namespace std;

void Gauss_Elim(gfp_matrix &A, unsigned int maxrow)
{
  unsigned int nr= A.size(), nc= A[0].size();
  gfp t, ti;
  unsigned int c= 0;
  for (unsigned int r= 0; r < maxrow && c < nc; r++)
    { // Find pivot
      unsigned int p= r;
      while (A[p][c].is_zero())
        {
          p++;
          if (p == A.size())
            { // If zero column do a restart of finding the pivot
              c++;
              p= r;
              if (c == A[0].size())
                {
                  return;
                }
            }
        }
      // Do pivoting
      if (p != r)
        {
          for (unsigned int j= 0; j < nc; j++)
            {
              t= A[p][j];
              A[p][j]= A[r][j];
              A[r][j]= t;
            }
        }
      // Make Lcoeff=1
      ti.invert(A[r][c]);
      for (unsigned int j= 0; j < nc; j++)
        {
          A[r][j].mul(ti);
        }
      // Now kill off other entries in this column
      for (unsigned int rr= 0; rr < nr; rr++)
        {
          if (r != rr)
            {
              t= A[rr][c];
              for (unsigned int j= 0; j < nc; j++)
                {
                  ti.mul(A[r][j], t);
                  A[rr][j].sub(A[rr][j], ti);
                }
            }
        }
      c++;
    }
}

gfp_matrix Mul(const gfp_matrix &A, const gfp_matrix &B)
{
  unsigned int m= A.size(), n= B[0].size(), t= A[0].size();
  if (t != B.size())
    {
      throw invalid_length();
    }
  gfp_matrix C(m, vector<gfp>(n));
  gfp te;
  for (unsigned int i= 0; i < m; i++)
    {
      for (unsigned int j= 0; j < n; j++)
        {
          C[i][j].assign_zero();
          for (unsigned int k= 0; k < t; k++)
            {
              te.mul(A[i][k], B[k][j]);
              C[i][j].add(te);
            }
        }
    }
  return C;
}

gfp dot_product(const vector<gfp> &x, const vector<gfp> &y)
{
  unsigned int n= x.size();
  if (n != y.size())
    {
      throw invalid_length();
    }
  gfp ans, te;
  ans.assign_zero();
  for (unsigned int i= 0; i < n; i++)
    {
      te.mul(x[i], y[i]);
      ans.add(te);
    }
  return ans;
}

int dot_product(const vector<int> &x, const vector<int> &y)
{
  unsigned int n= x.size();
  if (n != y.size())
    {
      throw invalid_length();
    }
  int ans= 0;
  for (unsigned int i= 0; i < n; i++)
    {
      ans+= x[i] * y[i];
    }
  return ans;
}

vector<gfp> Mul(const gfp_matrix &A, const vector<gfp> &x)
{
  unsigned int m= A.size(), t= A[0].size();
  if (t != x.size())
    {
      throw invalid_length();
    }
  vector<gfp> y(m);
  gfp te;
  for (unsigned int i= 0; i < m; i++)
    {
      y[i].assign_zero();
      for (unsigned int k= 0; k < t; k++)
        {
          te.mul(A[i][k], x[k]);
          y[i].add(te);
        }
    }
  return y;
}

ostream &operator<<(ostream &s, const gfp_matrix &A)
{
  s << A.size() << " " << A[0].size() << endl;
  for (unsigned int i= 0; i < A.size(); i++)
    {
      for (unsigned int j= 0; j < A[0].size(); j++)
        {
          s << A[i][j] << " ";
        }
      s << endl;
    }
  return s;
}

istream &operator>>(istream &s, gfp_matrix &A)
{
  int r, c;
  s >> r >> c;
  A.resize(r, vector<gfp>(c));
  for (int i= 0; i < r; i++)
    {
      for (int j= 0; j < c; j++)
        {
          s >> A[i][j];
        }
    }
  return s;
}

ostream &operator<<(ostream &s, const imatrix &A)
{
  s << A.size();
  if (A.size() != 0)
    {
      s << " " << A[0].size();
    }
  s << endl;
  for (unsigned int i= 0; i < A.size(); i++)
    {
      for (unsigned int j= 0; j < A[0].size(); j++)
        {
          s << A[i][j] << " ";
        }
      s << endl;
    }
  return s;
}

istream &operator>>(istream &s, imatrix &A)
{
  int r, c;
  s >> r;
  if (r != 0)
    {
      s >> c;
      A.resize(r, vector<int>(c));
      for (int i= 0; i < r; i++)
        {
          for (int j= 0; j < c; j++)
            {
              s >> A[i][j];
            }
        }
    }
  else
    {
      A.resize(0);
    }
  return s;
}

void print(const imatrix &S)
{
  unsigned int m= S.size(), n= S[0].size();
  for (unsigned int i= 0; i < m; i++)
    {
      for (unsigned int j= 0; j < n; j++)
        {
          cout << S[i][j] << " ";
        }
      cout << endl;
    }
}

void print(const gfp_matrix &S)
{
  unsigned int m= S.size(), n= S[0].size();
  for (unsigned int i= 0; i < m; i++)
    {
      for (unsigned int j= 0; j < n; j++)
        {
          cout << S[i][j] << " ";
        }
      cout << endl;
    }
}

int row_sum(const imatrix &D, int i)
{
  int sum= 0;
  for (unsigned int j= 0; j < D[0].size(); j++)
    {
      sum+= D[i][j];
    }
  return sum;
}

// Can we solve sustem (A' x = b) where A=A'||b
// Does the row reduction as well
bool Solvable(gfp_matrix &A)
{
  Gauss_Elim(A, min(A.size(), A[0].size()));

  for (unsigned int i= 0; i < A.size(); i++)
    {
      if (!A[i][A[i].size() - 1].is_zero())
        {
          bool ok= false;
          for (unsigned int j= 0; j < A[i].size() - 1; j++)
            {
              if (!A[i][j].is_zero())
                {
                  ok= true;
                }
            }
          if (!ok)
            {
              return false;
            }
        }
    }
  return true;
}

// Assumes already sun Solvable
vector<gfp> BackSubst(const gfp_matrix &A)
{
  unsigned int nc= A[0].size() - 1;
  vector<gfp> ans(nc);
  for (unsigned int j= 0; j < nc; j++)
    {
      ans[j].assign_zero();
    }

  gfp te;
  for (int i= (int) A.size() - 1; i >= 0; i--)
    {
      unsigned int c= 0;
      while (c < nc && A[i][c].is_zero())
        {
          c++;
        }
      if (c == nc && !A[i][c].is_zero())
        {
          throw bad_value();
        }
      if (c != nc)
        {
          ans[c]= A[i][nc];
          for (unsigned int j= c + 1; j < nc; j++)
            {
              te.mul(A[i][j], ans[j]);
              ans[c].sub(te);
            }
        }
    }
  return ans;
}

bool is_zero(const vector<gfp> &x)
{
  for (unsigned int i= 0; i < x.size(); i++)
    {
      if (!x[i].is_zero())
        {
          return false;
        }
    }
  return true;
}

int Hwt(const vector<int> &D)
{
  int hwt= 0;
  for (unsigned int i= 0; i < D.size(); i++)
    {
      if (D[i] != 0)
        {
          hwt++;
        }
    }
  return hwt;
}
