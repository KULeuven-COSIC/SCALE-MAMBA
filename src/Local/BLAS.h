/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2019, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _BLAS
#define _BLAS

/* This defines some Basic Linear Algebra Subroutines
 * for LOCAL operations on data
 *
 *
 * Note these are LOCAL operations only, which is why
 * we have no matrix_mult_SS for example!
 */

#include "LSSS/Share.h"
#include "OT/aBitVector.h"

/* This does cint n-by-k matrix A by cint k-by-m matrix B
 * result in cint n-by-m matrix C
 *    Cr_inp[0] = n
 *    Cr_inp[1] = k 
 *    Cr_inp[2] = m
 * Matrices is stored row wise in Cp_inp with output in
 * Cp_out
 *    A[0][0] = Cp_in[0]
 *    A[0][1] = Cp_in[1]
 *       ....
 *    A[1][0] = Cp_in[k]
 *       ....
 *    B[0][0] = Cp_in[n*k]
 *    B[0][1] = Cp_in[n*k+1]
 *       ....
 *    B[1][0] = Cp_in[n*k+m]
 *       ....
 *    C[0][0] = Cp_out[0]
 *    C[0][1] = Cp_out[1]
 *       ....
 *    C[1][0] = Cp_out[m]
 */
void matrix_mult_CC(vector<long> &Cr_out, vector<aBitVector> &Sr_out,
                    vector<gfp> &Cp_out, vector<Share> &Sp_out,
                    const vector<long> &Cr_inp, const vector<aBitVector> &Sr_inp,
                    const vector<gfp> &Cp_inp, const vector<Share> &Sp_inp);

/* This does sint n-by-k matrix A by cint k-by-m matrix B
 * result in sint n-by-m matrix C
 *    Cr_inp[0] = n
 *    Cr_inp[1] = k 
 *    Cr_inp[2] = m
 * Matrices is stored row wise in Cp_inp and Sp_inp with output in
 * Sp_out
 *    A[0][0] = Sp_in[0]
 *    A[0][1] = Sp_in[1]
 *       ....
 *    A[1][0] = Sp_in[k]
 *       ....
 *    B[0][0] = Cp_in[0]
 *    B[0][1] = Cp_in[1]
 *       ....
 *    B[1][0] = Cp_in[m]
 *       ....
 *    C[0][0] = Sp_out[0]
 *    C[0][1] = Sp_out[1]
 *       ....
 *    C[1][0] = Sp_out[m]
 */
void matrix_mult_SC(vector<long> &Cr_out, vector<aBitVector> &Sr_out,
                    vector<gfp> &Cp_out, vector<Share> &Sp_out,
                    const vector<long> &Cr_inp, const vector<aBitVector> &Sr_inp,
                    const vector<gfp> &Cp_inp, const vector<Share> &Sp_inp);

/* This does cint n-by-k matrix A by sint k-by-m matrix B
 * result in sint n-by-m matrix C
 *    Cr_inp[0] = n
 *    Cr_inp[1] = k 
 *    Cr_inp[2] = m
 * Matrices is stored row wise in Cp_inp and Sp_inp with output in
 * Sp_out
 *    A[0][0] = Cp_in[0]
 *    A[0][1] = Cp_in[1]
 *       ....
 *    A[1][0] = Cp_in[k]
 *       ....
 *    B[0][0] = Sp_in[0]
 *    B[0][1] = Sp_in[1]
 *       ....
 *    B[1][0] = Sp_in[m]
 *       ....
 *    C[0][0] = Sp_out[0]
 *    C[0][1] = Sp_out[1]
 *       ....
 *    C[1][0] = Sp_out[m]
 */
void matrix_mult_CS(vector<long> &Cr_out, vector<aBitVector> &Sr_out,
                    vector<gfp> &Cp_out, vector<Share> &Sp_out,
                    const vector<long> &Cr_inp, const vector<aBitVector> &Sr_inp,
                    const vector<gfp> &Cp_inp, const vector<Share> &Sp_inp);

/* This does Gaussian elimination on an n by m cint matrix A 
 * returning the result in B
 *
 * Cr_inp[0] = n
 * Cr_inp[1] = m
 * Matrices is stored row wise in Cp_inp with output in
 * Cp_out
 *    A[0][0] = Cp_in[0]
 *    A[0][1] = Cp_in[1]
 *       ....
 *    A[1][0] = Cp_in[m]
 *       ....
 *    B[0][0] = Cp_out[0]
 *    B[0][1] = Cp_out[1]
 *       ....
 *    B[1][0] = Cp_out[m]
 */
void Gaussian_Elimination(vector<long> &Cr_out, vector<aBitVector> &Sr_out,
                          vector<gfp> &Cp_out, vector<Share> &Sp_out,
                          const vector<long> &Cr_inp, const vector<aBitVector> &Sr_inp,
                          const vector<gfp> &Cp_inp, const vector<Share> &Sp_inp);

#endif
