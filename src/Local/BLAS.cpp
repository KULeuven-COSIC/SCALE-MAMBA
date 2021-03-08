/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "BLAS.h"
#include "LSSS/Share.h"
#include "Math/Matrix.h"
#include "Mod2Engine/aBitVector2.h"
#include "OT/aBitVector.h"

/* Pop a cint matrix A */
template<class SRegint, class SBit>
void pop_cint_matrix(vector<vector<gfp>> &A, Processor<SRegint, SBit> &Proc)
{
  long r, c;
  Proc.pop_int(c);
  Proc.pop_int(r);
  A.resize(r, vector<gfp>(c));
  for (long i= r - 1; i >= 0; i--)
    {
      for (long j= c - 1; j >= 0; j--)
        {
          Proc.pop_Cp(A[i][j]);
        }
    }
}

/* Push a cint matrix A */
template<class SRegint, class SBit>
void push_cint_matrix(const vector<vector<gfp>> &A, Processor<SRegint, SBit> &Proc)
{
  unsigned int r= A.size(), c= A[0].size();
  Proc.push_int(r);
  Proc.push_int(c);
  for (unsigned int i= 0; i < r; i++)
    {
      for (unsigned int j= 0; j < c; j++)
        {
          Proc.push_Cp(A[i][j]);
        }
    }
}

/* Pop a sint matrix A */
template<class SRegint, class SBit>
void pop_sint_matrix(vector<vector<Share>> &A, Processor<SRegint, SBit> &Proc)
{
  long r, c;
  Proc.pop_int(c);
  Proc.pop_int(r);
  A.resize(r, vector<Share>(c));
  for (long i= r - 1; i >= 0; i--)
    {
      for (long j= c - 1; j >= 0; j--)
        {
          Proc.pop_Sp(A[i][j]);
        }
    }
}

/* Push a sint matrix A */
template<class SRegint, class SBit>
void push_sint_matrix(const vector<vector<Share>> &A, Processor<SRegint, SBit> &Proc)
{
  unsigned int r= A.size(), c= A[0].size();
  Proc.push_int(r);
  Proc.push_int(c);
  for (unsigned int i= 0; i < r; i++)
    {
      for (unsigned int j= 0; j < c; j++)
        {
          Proc.push_Sp(A[i][j]);
        }
    }
}

/* This does cint n-by-k matrix A by cint k-by-m matrix B
 * result in cint n-by-m matrix C. 
 */
template<class SRegint, class SBit>
void matrix_mult_CC(Processor<SRegint, SBit> &Proc)
{
  vector<vector<gfp>> A, B;
  pop_cint_matrix(B, Proc);
  pop_cint_matrix(A, Proc);
  if (A[0].size() != B.size())
    {
      throw LF_Error("matrix_mult_CC: Dimension error");
    }
  unsigned int r= A.size(), c= B[0].size(), k= A[0].size();

  vector<vector<gfp>> C(r, vector<gfp>(c));

  for (unsigned int i= 0; i < r; i++)
    {
      for (unsigned int j= 0; j < c; j++)
        {
          C[i][j]= 0;
          for (unsigned int l= 0; l < k; l++)
            {
              C[i][j]+= A[i][l] * B[l][j];
            }
        }
    }
  push_cint_matrix(C, Proc);
}

/* This does sint n-by-k matrix A by cint k-by-m matrix B
 * result in sint n-by-m matrix C. 
 */
template<class SRegint, class SBit>
void matrix_mult_SC(Processor<SRegint, SBit> &Proc)
{
  vector<vector<Share>> A;
  vector<vector<gfp>> B;
  pop_cint_matrix(B, Proc);
  pop_sint_matrix(A, Proc);
  if (A[0].size() != B.size())
    {
      throw LF_Error("matrix_mult_SC: Dimension error");
    }
  unsigned int r= A.size(), c= B[0].size(), k= A[0].size();

  vector<vector<Share>> C(r, vector<Share>(c));

  for (unsigned int i= 0; i < r; i++)
    {
      for (unsigned int j= 0; j < c; j++)
        {
          C[i][j]= A[i][0] * B[0][j];
          for (unsigned int l= 1; l < k; l++)
            {
              C[i][j]+= A[i][l] * B[l][j];
            }
        }
    }
  push_sint_matrix(C, Proc);
}

/* This does cint n-by-k matrix A by sint k-by-m matrix B
 * result in sint n-by-m matrix C. 
 */
template<class SRegint, class SBit>
void matrix_mult_CS(Processor<SRegint, SBit> &Proc)
{
  vector<vector<gfp>> A;
  vector<vector<Share>> B;
  pop_sint_matrix(B, Proc);
  pop_cint_matrix(A, Proc);
  if (A[0].size() != B.size())
    {
      throw LF_Error("matrix_mult_CS: Dimension error");
    }
  unsigned int r= A.size(), c= B[0].size(), k= A[0].size();

  vector<vector<Share>> C(r, vector<Share>(c));

  for (unsigned int i= 0; i < r; i++)
    {
      for (unsigned int j= 0; j < c; j++)
        {
          C[i][j]= B[0][j] * A[i][0];
          for (unsigned int l= 1; l < k; l++)
            {
              C[i][j]+= B[l][j] * A[i][l];
            }
        }
    }
  push_sint_matrix(C, Proc);
}

/* This does Gaussian elimination on an n by m cint matrix A
 * returning the result. 
 */
template<class SRegint, class SBit>
void Gaussian_Elimination(Processor<SRegint, SBit> &Proc)
{
  vector<vector<gfp>> A;
  pop_cint_matrix(A, Proc);
  Gauss_Elim(A, A.size());
  push_cint_matrix(A, Proc);
}

template void matrix_mult_CC(Processor<aBitVector, aBit> &Proc);
template void matrix_mult_SC(Processor<aBitVector, aBit> &Proc);
template void matrix_mult_CS(Processor<aBitVector, aBit> &Proc);
template void Gaussian_Elimination(Processor<aBitVector, aBit> &Proc);

template void matrix_mult_CC(Processor<aBitVector2, Share2> &Proc);
template void matrix_mult_SC(Processor<aBitVector2, Share2> &Proc);
template void matrix_mult_CS(Processor<aBitVector2, Share2> &Proc);
template void Gaussian_Elimination(Processor<aBitVector2, Share2> &Proc);
