/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Local_Functions.h"
#include "BLAS.h"
#include "Float.h"

/* Include any other files here which define the
 * signatures of your local functions
 */

template<class SRegint, class SBit>
Local_Functions<SRegint, SBit>::Local_Functions()
{
  // System functions are numbers up to 65536

  // BLAS Functions
  functions.insert(make_pair(0, &matrix_mult_CC<SRegint, SBit>));
  functions.insert(make_pair(1, &matrix_mult_SC<SRegint, SBit>));
  functions.insert(make_pair(2, &matrix_mult_CS<SRegint, SBit>));
  functions.insert(make_pair(3, &Gaussian_Elimination<SRegint, SBit>));

  // Basic floating point routines (matching the equivalent GC ones)
  functions.insert(make_pair(120, &FP_add<SRegint, SBit>));
  functions.insert(make_pair(121, &FP_mul<SRegint, SBit>));
  functions.insert(make_pair(122, &FP_div<SRegint, SBit>));
  functions.insert(make_pair(123, &FP_eq<SRegint, SBit>));
  functions.insert(make_pair(124, &FP_f2i<SRegint, SBit>));
  functions.insert(make_pair(125, &FP_i2f<SRegint, SBit>));
  functions.insert(make_pair(126, &FP_sqrt<SRegint, SBit>));
  functions.insert(make_pair(127, &FP_lt<SRegint, SBit>));
  functions.insert(make_pair(128, &FP_floor<SRegint, SBit>));
  functions.insert(make_pair(129, &FP_ceil<SRegint, SBit>));

  // Floating point math routines
  functions.insert(make_pair(200, &FP_acos<SRegint, SBit>));
  functions.insert(make_pair(201, &FP_asin<SRegint, SBit>));
  functions.insert(make_pair(202, &FP_atan<SRegint, SBit>));
  functions.insert(make_pair(203, &FP_cos<SRegint, SBit>));
  functions.insert(make_pair(204, &FP_cosh<SRegint, SBit>));
  functions.insert(make_pair(205, &FP_sin<SRegint, SBit>));
  functions.insert(make_pair(206, &FP_sinh<SRegint, SBit>));
  functions.insert(make_pair(207, &FP_tanh<SRegint, SBit>));
  functions.insert(make_pair(208, &FP_exp<SRegint, SBit>));
  functions.insert(make_pair(209, &FP_log<SRegint, SBit>));
  functions.insert(make_pair(210, &FP_log10<SRegint, SBit>));
  functions.insert(make_pair(211, &FP_fabs<SRegint, SBit>));

  // Float conversion routines
  functions.insert(make_pair(500, &FP_to_fix<SRegint, SBit>));
  functions.insert(make_pair(501, &FP_from_fix<SRegint, SBit>));
  functions.insert(make_pair(502, &FP_to_float<SRegint, SBit>));
  functions.insert(make_pair(503, &FP_from_float<SRegint, SBit>));

  // User functions are numbers above 65536
  //
  //  Put your functions here
}

template class Local_Functions<aBitVector, aBit>;
template class Local_Functions<aBitVector2, Share2>;
