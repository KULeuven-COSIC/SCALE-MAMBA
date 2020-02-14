/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "FHE/FFT.h"
#include "Math/Zp_Data.h"

/* Computes the FFT via Horner's Rule
   This is stupid and only used for checking purposes and base cases
   theta is assumed to be an Nth root of unity
*/
void NaiveFFT(vector<modp> &ans, vector<modp> &a, int N, const modp &theta,
              const Zp_Data &PrD)
{
  int i, j;
  modp thetaPow;
  assignOne(thetaPow, PrD);
  for (i= 0; i < N; i++)
    {
      ans[i]= a[N - 1];
      for (j= N - 2; j >= 0; j--)
        {
          Mul(ans[i], ans[i], thetaPow, PrD);
          Add(ans[i], ans[i], a[j], PrD);
        }
      Mul(thetaPow, thetaPow, theta, PrD);
    }
}

void FFT(vector<modp> &a, int N, const modp &theta, const Zp_Data &PrD)
{
  if (N == 1)
    {
      return;
    }

  if (N < 5)
    {
      vector<modp> b(N);
      NaiveFFT(b, a, N, theta, PrD);
      a= b;
      return;
    }

  vector<modp> a0(N / 2), a1(N / 2);
  int i;
  for (i= 0; i < N / 2; i++)
    {
      a0[i]= a[2 * i];
      a1[i]= a[2 * i + 1];
    }
  modp theta2, w, t;
  Sqr(theta2, theta, PrD);
  FFT(a0, N / 2, theta2, PrD);
  FFT(a1, N / 2, theta2, PrD);
  assignOne(w, PrD);
  for (i= 0; i < N / 2; i++)
    {
      Mul(t, w, a1[i], PrD);
      Add(a[i], a0[i], t, PrD);
      Sub(a[i + N / 2], a0[i], t, PrD);
      Mul(w, w, theta, PrD);
    }
}

/*
 * Standard FFT for n a power of two, root a n-th primitive root of unity.
 */
void FFT_Iter(vector<modp> &ioput, int n, const modp &root,
              const Zp_Data &PrD)
{
  int i, j, m;

  // Bit-reversal of input
  for (i= j= 0; i < n; ++i)
    {
      if (j >= i)
        {
          swap(ioput[i], ioput[j]);
        }
      m= n / 2;

      while ((m >= 1) && (j >= m))
        {
          j-= m;
          m/= 2;
        }
      j+= m;
    }
  modp u, alpha, alpha2, t;

  m= 0;
  j= 0;
  i= 0;
  // Do the transform
  for (int s= 1; s < n; s= 2 * s)
    {
      m= 2 * s;
      Power(alpha, root, n / m, PrD);
      assignOne(alpha2, PrD);
      for (int j= 0; j < m / 2; ++j)
        {
          // root = root_table[j*n/m];
          for (int k= j; k < n; k+= m)
            {
              Mul(t, alpha2, ioput[k + m / 2], PrD);
              u= ioput[k];
              Add(ioput[k], u, t, PrD);
              Sub(ioput[k + m / 2], u, t, PrD);
            }
          Mul(alpha2, alpha2, alpha, PrD);
        }
    }
}

/*
 * FFT modulo x^n + 1.
 *
 * n must be a power of two, root a 2n-th primitive root of unity.
 */
void FFT_Iter2(vector<modp> &ioput, int n, const modp &root,
               const Zp_Data &PrD)
{
  int i, j, m;

  // Bit-reversal of input
  for (i= j= 0; i < n; ++i)
    {
      if (j >= i)
        {
          swap(ioput[i], ioput[j]);
        }
      m= n / 2;

      while ((m >= 1) && (j >= m))
        {
          j-= m;
          m/= 2;
        }
      j+= m;
    }
  modp u, alpha, alpha2, t;
  m= 0;
  j= 0;
  i= 0;
  // Do the transform
  for (int s= 1; s < n; s= 2 * s)
    {
      m= 2 * s;
      Power(alpha, root, n / m, PrD);
      alpha2= alpha;
      Mul(alpha, alpha, alpha, PrD);
      for (int j= 0; j < m / 2; ++j)
        {
          // root = root_table[(2*j+1)*n/m];
          for (int k= j; k < n; k+= m)
            {
              Mul(t, alpha2, ioput[k + m / 2], PrD);
              u= ioput[k];
              Add(ioput[k], u, t, PrD);
              Sub(ioput[k + m / 2], u, t, PrD);
            }
          Mul(alpha2, alpha2, alpha, PrD);
        }
    }
}

/* This does FFT for X^N+1,
   Input and output is an array of size N (shared)
   alpha is assumed to be a generator of the N'th roots of unity mod p
   Starts at w=alpha and updates by alpha^2
*/
void FFT2(vector<modp> &a, int N, const modp &alpha, const Zp_Data &PrD)
{
  int i;
  if (N == 1)
    {
      return;
    }

  vector<modp> a0(N / 2), a1(N / 2);
  for (i= 0; i < N / 2; i++)
    {
      a0[i]= a[2 * i];
      a1[i]= a[2 * i + 1];
    }

  modp w, alpha2, temp;
  Sqr(alpha2, alpha, PrD);
  FFT2(a0, N / 2, alpha2, PrD);
  FFT2(a1, N / 2, alpha2, PrD);

  w= alpha;
  for (i= 0; i < N / 2; i++)
    {
      Mul(temp, w, a1[i], PrD);
      Add(a[i], a0[i], temp, PrD);
      Sub(a[i + N / 2], a0[i], temp, PrD);
      Mul(w, w, alpha2, PrD);
    }
}
