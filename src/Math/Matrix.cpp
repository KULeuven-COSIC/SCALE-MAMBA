/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Math/Matrix.h"
#include "Exceptions/Exceptions.h"

#include <iostream>
#include <stdlib.h>

using namespace std;

template<class T>
void Gauss_Elim(vector<vector<T>> &A, unsigned int maxrow)
{
  unsigned int nr= A.size(), nc= A[0].size();
  T t, ti;
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

template<class T>
void Mul(vector<vector<T>> &ans, const vector<vector<T>> &A, const vector<vector<T>> &B)
{
  unsigned int m= A.size(), n= B[0].size(), t= A[0].size();
  if (t != B.size() || ans.size() != m || ans[0].size() != n)
    {
      throw invalid_length();
    }
  T te;
  for (unsigned int i= 0; i < m; i++)
    {
      for (unsigned int j= 0; j < n; j++)
        {
          ans[i][j].assign_zero();
          for (unsigned int k= 0; k < t; k++)
            {
              te.mul(A[i][k], B[k][j]);
              ans[i][j].add(te);
            }
        }
    }
}

template<class T>
T dot_product(const vector<T> &x, const vector<T> &y)
{
  unsigned int n= x.size();
  if (n != y.size())
    {
      throw invalid_length();
    }
  T ans, te;
  ans.assign_zero();
  for (unsigned int i= 0; i < n; i++)
    {
      te.mul(x[i], y[i]);
      ans.add(te);
    }
  return ans;
}

template<class T>
void dot_product(vector<T> &ans, const vector<T> &x, const vector<vector<T>> &y)
{
  if (x.size() != y.size() || ans.size() != y[0].size())
    {
      throw invalid_length();
    }
  T te;
  for (unsigned int k= 0; k < ans.size(); k++)
    {
      ans[k].assign_zero();
      for (unsigned int i= 0; i < x.size(); i++)
        {
          te.mul(x[i], y[i][k]);
          ans[k].add(te);
        }
    }
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

word dot_product(const vector<word> &x, const vector<gf2> &y)
{
  unsigned int n= x.size();
  if (n != y.size())
    {
      throw invalid_length();
    }
  word ans= 0;
  for (unsigned int i= 0; i < n; i++)
    {
      if (y[i].is_one())
        {
          ans^= x[i];
        }
    }
  return ans;
}

void dot_product(vector<word> &ans, const vector<gf2> &x, const vector<vector<word>> &y)
{
  if (x.size() != y.size() || ans.size() != y[0].size())
    {
      throw invalid_length();
    }
  for (unsigned int k= 0; k < ans.size(); k++)
    {
      ans[k]= 0;
    }
  for (unsigned int i= 0; i < x.size(); i++)
    {
      if (x[i].is_one())
        {
          for (unsigned int k= 0; k < ans.size(); k++)
            {
              ans[k]^= y[i][k];
            }
        }
    }
}

template<class T>
void Mul(vector<T> &ans, const vector<vector<T>> &A, const vector<T> &x)
{
  unsigned int m= A.size(), t= A[0].size();
  if (t != x.size() || ans.size() != m)
    {
      throw invalid_length();
    }
  T te;
  for (unsigned int i= 0; i < m; i++)
    {
      ans[i].assign_zero();
      for (unsigned int k= 0; k < t; k++)
        {
          te.mul(A[i][k], x[k]);
          ans[i].add(te);
        }
    }
}

void Mul(vector<word> &ans, const vector<vector<gf2>> &A, const vector<word> &x)
{
  unsigned int m= A.size(), t= A[0].size();
  if (t != x.size() || ans.size() != m)
    {
      throw invalid_length();
    }
  for (unsigned int i= 0; i < m; i++)
    {
      ans[i]= 0;
      for (unsigned int k= 0; k < t; k++)
        {
          if (A[i][k].is_one())
            {
              ans[i]^= x[k];
            }
        }
    }
}

void Mul(vector<vector<word>> &ans, const vector<vector<gf2>> &A, const vector<vector<word>> &x)
{
  unsigned int m= A.size(), t= A[0].size(), n= x[0].size();
  if (t != x.size() || ans.size() != m || ans[0].size() != n)
    {
      throw invalid_length();
    }
  for (unsigned int i= 0; i < m; i++)
    {
      for (unsigned int j= 0; j < n; j++)
        {
          ans[i][j]= 0;
        }
      for (unsigned int k= 0; k < t; k++)
        {
          if (A[i][k].is_one())
            {
              for (unsigned int j= 0; j < n; j++)
                {
                  ans[i][j]^= x[k][j];
                }
            }
        }
    }
}

template<class T>
ostream &operator<<(ostream &s, const vector<vector<T>> &A)
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

template<class T>
istream &operator>>(istream &s, vector<vector<T>> &A)
{
  int r, c;
  s >> r >> c;
  A.resize(r, vector<T>(c));
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

void print(const vector<vector<gfp>> &S)
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
template<class T>
bool Solvable(vector<vector<T>> &A)
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
template<class T>
void BackSubst(vector<T> &ans, const vector<vector<T>> &A)
{
  unsigned int nc= A[0].size() - 1;
  if (ans.size() != nc)
    {
      throw invalid_length();
    }
  for (unsigned int j= 0; j < nc; j++)
    {
      ans[j].assign_zero();
    }

  T te;
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
}

template<class T>
bool is_zero(const vector<T> &x)
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

template void Gauss_Elim(vector<vector<gfp>> &A, unsigned int maxrow);
template void Gauss_Elim(vector<vector<gf2>> &A, unsigned int maxrow);
template void Mul(vector<vector<gfp>> &ans,
                  const vector<vector<gfp>> &A, const vector<vector<gfp>> &B);
template void Mul(vector<vector<gf2>> &ans,
                  const vector<vector<gf2>> &A, const vector<vector<gf2>> &B);
template void Mul(vector<gfp> &ans, const vector<vector<gfp>> &A, const vector<gfp> &x);
template void Mul(vector<gf2> &ans, const vector<vector<gf2>> &A, const vector<gf2> &x);
template void BackSubst(vector<gfp> &ans, const vector<vector<gfp>> &A);
template void BackSubst(vector<gf2> &ans, const vector<vector<gf2>> &A);
template gfp dot_product(const vector<gfp> &x, const vector<gfp> &y);
template void dot_product(vector<gfp> &ans, const vector<gfp> &x, const vector<vector<gfp>> &y);
template gf2 dot_product(const vector<gf2> &x, const vector<gf2> &y);
template bool Solvable(vector<vector<gfp>> &A);
template bool Solvable(vector<vector<gf2>> &A);
template ostream &operator<<(ostream &s, const vector<vector<gfp>> &A);
template ostream &operator<<(ostream &s, const vector<vector<gf2>> &A);
template istream &operator>>(istream &s, vector<vector<gfp>> &A);
template istream &operator>>(istream &s, vector<vector<gf2>> &A);
template bool is_zero(const vector<gfp> &x);
template bool is_zero(const vector<gf2> &x);
