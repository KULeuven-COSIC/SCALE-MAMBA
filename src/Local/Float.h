/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _FLOAT_LOCAL
#define _FLOAT_LOCAL

/* This defines floating point operations on regint values
 * by converting them to double in C++ and doing the 
 * operations there. This enables fast ClearIEEE arithmetic
 * in the Rust pipeline
 */

#include "Processor/Processor.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
// Convert long to double
inline double convert_to_double(long x)
{
  double y= *((double *) (&x));
  return y;
}

// Convert double to long
inline long convert_to_long(double x)
{
  long y= *((long *) (&x));
  return y;
}
#pragma GCC diagnostic pop

// Convert modp to signed bigint of a given bit length
void to_signed_bigint(bigint &bi, const gfp &x, int len);

// Create mpf_class object from v,k,f
void convert_from_fix(mpf_class &res, const gfp &v, long k, long f);

// Create mpf_class object from v,p,z,s
void convert_from_float(mpf_class &res,
                        const gfp &v, const gfp &p, const gfp &z,
                        const gfp &s);

template<class SRegint, class SBit>
void FP_add(Processor<SRegint, SBit> &Proc);
template<class SRegint, class SBit>
void FP_mul(Processor<SRegint, SBit> &Proc);
template<class SRegint, class SBit>
void FP_div(Processor<SRegint, SBit> &Proc);
template<class SRegint, class SBit>
void FP_eq(Processor<SRegint, SBit> &Proc);
template<class SRegint, class SBit>
void FP_f2i(Processor<SRegint, SBit> &Proc);
template<class SRegint, class SBit>
void FP_i2f(Processor<SRegint, SBit> &Proc);
template<class SRegint, class SBit>
void FP_sqrt(Processor<SRegint, SBit> &Proc);
template<class SRegint, class SBit>
void FP_lt(Processor<SRegint, SBit> &Proc);

template<class SRegint, class SBit>
void FP_to_fix(Processor<SRegint, SBit> &Proc);
template<class SRegint, class SBit>
void FP_from_fix(Processor<SRegint, SBit> &Proc);
template<class SRegint, class SBit>
void FP_to_float(Processor<SRegint, SBit> &Proc);
template<class SRegint, class SBit>
void FP_from_float(Processor<SRegint, SBit> &Proc);

template<class SRegint, class SBit>
void FP_acos(Processor<SRegint, SBit> &Proc);
template<class SRegint, class SBit>
void FP_asin(Processor<SRegint, SBit> &Proc);
template<class SRegint, class SBit>
void FP_atan(Processor<SRegint, SBit> &Proc);
template<class SRegint, class SBit>
void FP_cos(Processor<SRegint, SBit> &Proc);
template<class SRegint, class SBit>
void FP_cosh(Processor<SRegint, SBit> &Proc);
template<class SRegint, class SBit>
void FP_sin(Processor<SRegint, SBit> &Proc);
template<class SRegint, class SBit>
void FP_sinh(Processor<SRegint, SBit> &Proc);
template<class SRegint, class SBit>
void FP_tanh(Processor<SRegint, SBit> &Proc);
template<class SRegint, class SBit>
void FP_exp(Processor<SRegint, SBit> &Proc);
template<class SRegint, class SBit>
void FP_log(Processor<SRegint, SBit> &Proc);
template<class SRegint, class SBit>
void FP_log10(Processor<SRegint, SBit> &Proc);
template<class SRegint, class SBit>
void FP_ceil(Processor<SRegint, SBit> &Proc);
template<class SRegint, class SBit>
void FP_fabs(Processor<SRegint, SBit> &Proc);
template<class SRegint, class SBit>
void FP_floor(Processor<SRegint, SBit> &Proc);

#endif
