/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _FFT
#define _FFT

#include <vector>

#include "FHE/FFT_Data.h"

/* This is the basic 2^n - FFT routine, it works in an recursive
 * manner. The FFT_Iter routine below does the same thing but in
 * an iterative manner, and is therefore more efficient.
 */
void FFT(vector<modp> &a, int N, const modp &theta, const Zp_Data &PrD);

/* This uses a different root of unity and is used for going
 * forward when doing 2^n FFT's. This avoids the need for padding
 * out to a vector of size 2*n. Going backwards you still need to
 * use FFT, but some other post-processing. Again the iterative
 * version is given by FFT_Iter2 below, and this is usually faster
 */
void FFT2(vector<modp> &a, int N, const modp &theta, const Zp_Data &PrD);

void FFT_Iter(vector<modp> &a, int N, const modp &theta, const Zp_Data &PrD);

void FFT_Iter2(vector<modp> &a, int N, const modp &theta, const Zp_Data &PrD);

/* Computes the FFT via Horner's Rule
   This is stupid and only used for checking purposes and base cases
   theta is assumed to be an Nth root of unity
   This works for any value of N
*/
void NaiveFFT(vector<modp> &ans, vector<modp> &a, int N, const modp &theta,
              const Zp_Data &PrD);

#endif
