/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

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

#include "Processor/Processor.h"

/* This does cint n-by-k matrix A by cint k-by-m matrix B
 * result in cint n-by-m matrix C.
 *
 * See the main BLAS.cpp file for how data is pushed/poped
 * and which order it is done in.
 */
void matrix_mult_CC(Processor &Proc);

/* This does sint n-by-k matrix A by cint k-by-m matrix B
 * result in sint n-by-m matrix C
 *
 * See the main BLAS.cpp file for how data is pushed/poped
 * and which order it is done in.
 */
void matrix_mult_SC(Processor &Proc);

/* This does cint n-by-k matrix A by sint k-by-m matrix B
 * result in sint n-by-m matrix C
 *
 * See the main BLAS.cpp file for how data is pushed/poped
 * and which order it is done in.
 */
void matrix_mult_CS(Processor &Proc);

/* This does Gaussian elimination on an n by m cint matrix A 
 * returning the result in B
 *
 * See the main BLAS.cpp file for how data is pushed/poped
 * and which order it is done in.
 */
void Gaussian_Elimination(Processor &Proc);

#endif
