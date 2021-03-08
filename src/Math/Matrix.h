/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _myHNF
#define _myHNF

#include <vector>
using namespace std;

#include "Math/bigint.h"
#include "Math/gf2.h"
#include "Math/gfp.h"
#include "Math/modp.h"

typedef vector<vector<int>> imatrix;

void print(const imatrix &S);
void print(const vector<vector<gfp>> &S);

// Apply Gaussian Elmination to a matrix
//   - Stop when done maxrow rows
template<class T>
void Gauss_Elim(vector<vector<T>> &A, unsigned int maxrow);

// All the following assume ans has the correct dimensions

// Multiply two matrices
template<class T>
void Mul(vector<vector<T>> &ans, const vector<vector<T>> &A, const vector<vector<T>> &B);

// vector * vector multiply
template<class T>
T dot_product(const vector<T> &x, const vector<T> &y);

template<class T>
void dot_product(vector<T> &ans, const vector<T> &x, const vector<vector<T>> &y);

int dot_product(const vector<int> &x, const vector<int> &y);
word dot_product(const vector<word> &x, const vector<gf2> &y);
void dot_product(vector<word> &ans, const vector<gf2> &x, const vector<vector<word>> &y);

// Matrix * vector multiply
template<class T>
void Mul(vector<T> &ans, const vector<vector<T>> &A, const vector<T> &x);

/* Do this on words with a bit matrix */
void Mul(vector<word> &ans, const vector<vector<gf2>> &A, const vector<word> &x);
void Mul(vector<vector<word>> &ans, const vector<vector<gf2>> &A, const vector<vector<word>> &x);

template<class T>
ostream &operator<<(ostream &s, const vector<vector<T>> &A);
ostream &operator<<(ostream &s, const imatrix &A);
template<class T>
istream &operator>>(istream &s, vector<vector<T>> &A);
istream &operator>>(istream &s, imatrix &A);

/* Various Matrix Helper Routines */
int row_sum(const imatrix &D, int i);

// Can we solve sustem (A' x = b) where A=A'||b
// Does the row reduction on A needed as well
template<class T>
bool Solvable(vector<vector<T>> &A);

// Assumes already sun Solvable [ans has size ((num cols of A) - 1)]
template<class T>
void BackSubst(vector<T> &ans, const vector<vector<T>> &A);

template<class T>
bool is_zero(const vector<T> &x);

// Hwt returns the hamming weight of vector D
int Hwt(const vector<int> &D);

#endif
