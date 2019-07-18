/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2019, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "BLAS.h"
#include "Math/Matrix.h"

/* Teis does cint n-by-k matrix A by cint k-by-m matrix B
 * result in cint n-by-m matrix C. For argument definitions
 * see the .h file
 */
void matrix_mult_CC(vector<long> &Cr_out, vector<aBitVector> &Sr_out,
                    vector<gfp> &Cp_out, vector<Share> &Sp_out,
                    const vector<long> &Cr_inp, const vector<aBitVector> &Sr_inp,
                    const vector<gfp> &Cp_inp, const vector<Share> &Sp_inp)
{
  if (Cr_inp.size() != 3)
    {
      throw LF_Error("matrix_mult_CC: Dimension error");
    }
  unsigned int n= Cr_inp[0], k= Cr_inp[1], m= Cr_inp[2];

  if (Cp_inp.size() != (n * k + k * m) || Cp_out.size() != n * m)
    {
      throw LF_Error("matrix_mult_CC: Dimension error");
    }

  if (Cr_out.size() != 0 || Sr_out.size() != 0 || Sp_out.size() != 0 || Sr_inp.size() != 0 || Sp_inp.size() != 0)
    {
      throw LF_Error("matrix_mult_CC: Dimension error");
    }

  for (unsigned int i= 0; i < n; i++)
    {
      for (unsigned int j= 0; j < m; j++)
        {
          Cp_out[i * m + j]= 0;
          for (unsigned int l= 0; l < k; l++)
            {
              Cp_out[i * m + j]+= Cp_inp[i * k + l] * Cp_inp[n * k + l * m + j];
            }
        }
    }
}

/* This does sint n-by-k matrix A by cint k-by-m matrix B
 * result in sint n-by-m matrix C. For argument definitions
 * see the .h file
 */
void matrix_mult_SC(vector<long> &Cr_out, vector<aBitVector> &Sr_out,
                    vector<gfp> &Cp_out, vector<Share> &Sp_out,
                    const vector<long> &Cr_inp, const vector<aBitVector> &Sr_inp,
                    const vector<gfp> &Cp_inp, const vector<Share> &Sp_inp)
{
  if (Cr_inp.size() != 3)
    {
      throw LF_Error("matrix_mult_SC: Dimension error");
    }
  unsigned int n= Cr_inp[0], k= Cr_inp[1], m= Cr_inp[2];

  if (Sp_inp.size() != n * k || Cp_inp.size() != k * m || Sp_out.size() != n * m)
    {
      throw LF_Error("matrix_mult_SC: Dimension error");
    }

  if (Cr_out.size() != 0 || Sr_out.size() != 0 || Cp_out.size() != 0 || Sr_inp.size() != 0)
    {
      throw LF_Error("matrix_mult_SC: Dimension error");
    }

  for (unsigned int i= 0; i < n; i++)
    {
      for (unsigned int j= 0; j < m; j++)
        {
          Sp_out[i * m + j].assign_zero();
          for (unsigned int l= 0; l < k; l++)
            {
              Sp_out[i * m + j]+= Sp_inp[i * k + l] * Cp_inp[l * m + j];
            }
        }
    }
}

/* This does cint n-by-k matrix A by sint k-by-m matrix B
 * result in sint n-by-m matrix C. For argument definitions
 * see the .h file
 */
void matrix_mult_CS(vector<long> &Cr_out, vector<aBitVector> &Sr_out,
                    vector<gfp> &Cp_out, vector<Share> &Sp_out,
                    const vector<long> &Cr_inp, const vector<aBitVector> &Sr_inp,
                    const vector<gfp> &Cp_inp, const vector<Share> &Sp_inp)
{
  if (Cr_inp.size() != 3)
    {
      throw LF_Error("matrix_mult_CS: Dimension error");
    }
  unsigned int n= Cr_inp[0], k= Cr_inp[1], m= Cr_inp[2];

  if (Cp_inp.size() != n * k || Sp_inp.size() != k * m || Sp_out.size() != n * m)
    {
      throw LF_Error("matrix_mult_CS: Dimension error");
    }

  if (Cr_out.size() != 0 || Sr_out.size() != 0 || Cp_out.size() != 0 || Sr_inp.size() != 0)
    {
      throw LF_Error("matrix_mult_CS: Dimension error");
    }

  for (unsigned int i= 0; i < n; i++)
    {
      for (unsigned int j= 0; j < m; j++)
        {
          Sp_out[i * m + j].assign_zero();
          for (unsigned int l= 0; l < k; l++)
            {
              Sp_out[i * m + j]+= Sp_inp[l * m + j] * Cp_inp[i * k + l];
            }
        }
    }
}

/* This does Gaussian elimination on an n by m cint matrix A
 * returning the result in B. For argument definitions
 * see the .h file
 */
void Gaussian_Elimination(vector<long> &Cr_out, vector<aBitVector> &Sr_out,
                          vector<gfp> &Cp_out, vector<Share> &Sp_out,
                          const vector<long> &Cr_inp, const vector<aBitVector> &Sr_inp,
                          const vector<gfp> &Cp_inp, const vector<Share> &Sp_inp)
{
  if (Cr_inp.size() != 2)
    {
      throw LF_Error("Gaussian_Elimination: Dimension error");
    }
  unsigned int n= Cr_inp[0], m= Cr_inp[1];

  if (Cp_inp.size() != n * m || Cp_out.size() != n * m)
    {
      throw LF_Error("Gaussian_Elimination: Dimension error");
    }

  if (Cr_out.size() != 0 || Sr_out.size() != 0 || Sp_out.size() != 0 || Sr_inp.size() != 0 || Sp_inp.size() != 0)
    {
      throw LF_Error("Gaussian_Elimination: Dimension error");
    }

  // Get Matrix
  gfp_matrix A(n, vector<gfp>(m));
  for (unsigned int i= 0; i < n; i++)
    {
      for (unsigned int j= 0; j < m; j++)
        {
          A[i][j]= Cp_inp[i * m + j];
        }
    }
  // Do the operation
  Gauss_Elim(A, n);
  // Store the result
  for (unsigned int i= 0; i < n; i++)
    {
      for (unsigned int j= 0; j < m; j++)
        {
          Cp_out[i * m + j]= A[i][j];
        }
    }
}
