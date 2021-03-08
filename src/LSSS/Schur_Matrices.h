/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _Schur_Matrices
#define _Schur_Matrices

/* This class holds the Schur matrices for a Q2 multiplicative
 * Schur
 *
 * It derives these matrices directly from the Schur, which has
 * to be multiplicative.
 *
 */

#include "LSSS/MSP.h"
#include "Math/Matrix.h"

template<class T>
class MSP;

// Forward declaration as apparently this is needed for friends in templates
template<class T>
class Schur_Matrices;
template<class T>
ostream &operator<<(ostream &s, const Schur_Matrices<T> &M);
template<class T>
istream &operator>>(istream &s, Schur_Matrices<T> &M);

template<class T>
class Schur_Matrices
{
  /* Vector of Schur product matrices per party
          Schur[i][j][k]=coeff of share xx[j]*yy[k]
                         added into product by party i, where
                             0 <= j,k < ns[i]
  */
  vector<vector<vector<T>>> Schur;

  /* Same but just 0/1 to signal if entry is non zero or not */
  vector<imatrix> iSchur;

public:
  /* Initialize the Schur matrices given the MSP
   *  - Returns false is MSP is not multiplicative
   */
  bool initialize(const MSP<T> &M);

  void assign(const Schur_Matrices<T> &S)
  {
    Schur= S.Schur;
    iSchur= S.iSchur;
  }

  Schur_Matrices(const Schur_Matrices<T> &S)
  {
    assign(S);
  }
  Schur_Matrices()
  {
    ;
  }
  Schur_Matrices<T> &operator=(const Schur_Matrices &S)
  {
    if (&S != this)
      {
        assign(S);
      }
    return *this;
  }
  ~Schur_Matrices()
  {
    ;
  }

  T get_schur(int i, int j, int k) const
  {
    return Schur[i][j][k];
  }
  int is_non_zero(int i, int j, int k) const
  {
    return iSchur[i][j][k];
  }

  friend ostream &operator<<<>(ostream &s, const Schur_Matrices<T> &S);
  friend istream &operator>><>(istream &s, Schur_Matrices<T> &S);
};

#endif
