/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Local_Functions.h"
#include "BLAS.h"

/* Include any other files here which define the
 * signatures of your local functions
 */

template<class SRegint, class SBit>
Local_Functions<SRegint, SBit>::Local_Functions()
{
  // System functions are numbers up to 65536
  functions.insert(make_pair(0, &matrix_mult_CC<SRegint, SBit>));
  functions.insert(make_pair(1, &matrix_mult_SC<SRegint, SBit>));
  functions.insert(make_pair(2, &matrix_mult_CS<SRegint, SBit>));
  functions.insert(make_pair(3, &Gaussian_Elimination<SRegint, SBit>));

  // User functions are numbers above 65536
  //
  //  Put your functions here
}

template class Local_Functions<aBitVector, aBit>;
template class Local_Functions<aBitVector2, Share2>;
