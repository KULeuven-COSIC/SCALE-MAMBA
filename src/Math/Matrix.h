/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _myHNF
#define _myHNF

#include <vector>
using namespace std;

#include "Math/bigint.h"
#include "Math/gfp.h"
#include "Math/modp.h"

typedef vector<vector<gfp>> gfp_matrix;
typedef vector<vector<int>> imatrix;

void print(const imatrix &S);
void print(const gfp_matrix &S);

// Apply Gaussian Elmination to a gfp_matrix
//   - Stop when done maxrow rows
void Gauss_Elim(gfp_matrix &A, unsigned int maxrow);

// Multiply two gfp matrices
gfp_matrix Mul(const gfp_matrix &A, const gfp_matrix &B);

// vector * vector multiply
gfp dot_product(const vector<gfp> &x, const vector<gfp> &y);
int dot_product(const vector<int> &x, const vector<int> &y);

// Matrix * vector multiply
vector<gfp> Mul(const gfp_matrix &A, const vector<gfp> &x);

ostream &operator<<(ostream &s, const gfp_matrix &A);
ostream &operator<<(ostream &s, const imatrix &A);
istream &operator>>(istream &s, gfp_matrix &A);
istream &operator>>(istream &s, imatrix &A);

/* Various Matrix Helper Routines */
int row_sum(const imatrix &D, int i);

// Can we solve sustem (A' x = b) where A=A'||b
// Does the row reduction on A needed as well
bool Solvable(gfp_matrix &A);

// Assumes already sun Solvable
vector<gfp> BackSubst(const gfp_matrix &A);

bool is_zero(const vector<gfp> &x);

// Hwt returns the hamming weight of vector D
int Hwt(const vector<int> &D);

#endif
