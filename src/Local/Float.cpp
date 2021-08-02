/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Float.h"
#include <math.h>

template<class SRegint, class SBit>
void FP_add(Processor<SRegint, SBit> &Proc)
{
  long x, y, z;
  Proc.pop_int(x);
  Proc.pop_int(y);
  double a, b, c;
  a= convert_to_double(x);
  b= convert_to_double(y);
  c= a + b;
  z= convert_to_long(c);
  Proc.push_int(z);
}

template<class SRegint, class SBit>
void FP_mul(Processor<SRegint, SBit> &Proc)
{
  long x, y, z;
  Proc.pop_int(x);
  Proc.pop_int(y);
  double a, b, c;
  a= convert_to_double(x);
  b= convert_to_double(y);
  c= a * b;
  z= convert_to_long(c);
  Proc.push_int(z);
}

template<class SRegint, class SBit>
void FP_div(Processor<SRegint, SBit> &Proc)
{
  long x, y, z;
  Proc.pop_int(x);
  Proc.pop_int(y);
  double a, b, c;
  a= convert_to_double(x);
  b= convert_to_double(y);
  c= b / a;
  z= convert_to_long(c);
  Proc.push_int(z);
}

template<class SRegint, class SBit>
void FP_eq(Processor<SRegint, SBit> &Proc)
{
  long x, y;
  Proc.pop_int(x);
  Proc.pop_int(y);
  double a, b, c;
  a= convert_to_double(x);
  b= convert_to_double(y);
  c= (a == b);
  Proc.push_int(c);
}

template<class SRegint, class SBit>
void FP_f2i(Processor<SRegint, SBit> &Proc)
{
  long x, c;
  Proc.pop_int(x);
  double a= convert_to_double(x);
  c= a;
  Proc.push_int(c);
}

template<class SRegint, class SBit>
void FP_i2f(Processor<SRegint, SBit> &Proc)
{
  long x, z;
  Proc.pop_int(x);
  double a;
  a= (double) x;
  z= convert_to_long(a);
  Proc.push_int(z);
}

template<class SRegint, class SBit>
void FP_sqrt(Processor<SRegint, SBit> &Proc)
{
  long x, z;
  Proc.pop_int(x);
  double a, c;
  a= convert_to_double(x);
  c= sqrt(a);
  z= convert_to_long(c);
  Proc.push_int(z);
}

template<class SRegint, class SBit>
void FP_lt(Processor<SRegint, SBit> &Proc)
{
  long x, y;
  Proc.pop_int(x);
  Proc.pop_int(y);
  double a, b, c;
  a= convert_to_double(x);
  b= convert_to_double(y);
  c= (b < a);
  Proc.push_int(c);
}

template<class SRegint, class SBit>
void FP_acos(Processor<SRegint, SBit> &Proc)
{
  long x, z;
  Proc.pop_int(x);
  double a, c;
  a= convert_to_double(x);
  c= acos(a);
  z= convert_to_long(c);
  Proc.push_int(z);
}

template<class SRegint, class SBit>
void FP_asin(Processor<SRegint, SBit> &Proc)
{
  long x, z;
  Proc.pop_int(x);
  double a, c;
  a= convert_to_double(x);
  c= asin(a);
  z= convert_to_long(c);
  Proc.push_int(z);
}

template<class SRegint, class SBit>
void FP_atan(Processor<SRegint, SBit> &Proc)
{
  long x, z;
  Proc.pop_int(x);
  double a, c;
  a= convert_to_double(x);
  c= atan(a);
  z= convert_to_long(c);
  Proc.push_int(z);
}

template<class SRegint, class SBit>
void FP_cos(Processor<SRegint, SBit> &Proc)
{
  long x, z;
  Proc.pop_int(x);
  double a, c;
  a= convert_to_double(x);
  c= cos(a);
  z= convert_to_long(c);
  Proc.push_int(z);
}

template<class SRegint, class SBit>
void FP_cosh(Processor<SRegint, SBit> &Proc)
{
  long x, z;
  Proc.pop_int(x);
  double a, c;
  a= convert_to_double(x);
  c= cosh(a);
  z= convert_to_long(c);
  Proc.push_int(z);
}

template<class SRegint, class SBit>
void FP_sin(Processor<SRegint, SBit> &Proc)
{
  long x, z;
  Proc.pop_int(x);
  double a, c;
  a= convert_to_double(x);
  c= sin(a);
  z= convert_to_long(c);
  Proc.push_int(z);
}

template<class SRegint, class SBit>
void FP_sinh(Processor<SRegint, SBit> &Proc)
{
  long x, z;
  Proc.pop_int(x);
  double a, c;
  a= convert_to_double(x);
  c= sinh(a);
  z= convert_to_long(c);
  Proc.push_int(z);
}

template<class SRegint, class SBit>
void FP_tanh(Processor<SRegint, SBit> &Proc)
{
  long x, z;
  Proc.pop_int(x);
  double a, c;
  a= convert_to_double(x);
  c= tanh(a);
  z= convert_to_long(c);
  Proc.push_int(z);
}

template<class SRegint, class SBit>
void FP_exp(Processor<SRegint, SBit> &Proc)
{
  long x, z;
  Proc.pop_int(x);
  double a, c;
  a= convert_to_double(x);
  c= exp(a);
  z= convert_to_long(c);
  Proc.push_int(z);
}

template<class SRegint, class SBit>
void FP_log(Processor<SRegint, SBit> &Proc)
{
  long x, z;
  Proc.pop_int(x);
  double a, c;
  a= convert_to_double(x);
  c= log(a);
  z= convert_to_long(c);
  Proc.push_int(z);
}

template<class SRegint, class SBit>
void FP_log10(Processor<SRegint, SBit> &Proc)
{
  long x, z;
  Proc.pop_int(x);
  double a, c;
  a= convert_to_double(x);
  c= log10(a);
  z= convert_to_long(c);
  Proc.push_int(z);
}

template<class SRegint, class SBit>
void FP_ceil(Processor<SRegint, SBit> &Proc)
{
  long x, z;
  Proc.pop_int(x);
  double a, c;
  a= convert_to_double(x);
  c= ceil(a);
  z= convert_to_long(c);
  Proc.push_int(z);
}

template<class SRegint, class SBit>
void FP_fabs(Processor<SRegint, SBit> &Proc)
{
  long x, z;
  Proc.pop_int(x);
  double a, c;
  a= convert_to_double(x);
  c= fabs(a);
  z= convert_to_long(c);
  Proc.push_int(z);
}

template<class SRegint, class SBit>
void FP_floor(Processor<SRegint, SBit> &Proc)
{
  long x, z;
  Proc.pop_int(x);
  double a, c;
  a= convert_to_double(x);
  c= floor(a);
  z= convert_to_long(c);
  Proc.push_int(z);
}

/* Conversion Routines */
// Convert modp to signed bigint of a given bit length
void to_signed_bigint(bigint &bi, const gfp &x, int len)
{
  to_bigint(bi, x);
  int neg;
  // get sign and abs(x)
  bigint p_half= (gfp::pr() - 1) / 2;
  if (mpz_cmp(bi.get_mpz_t(), p_half.get_mpz_t()) < 0)
    neg= 0;
  else
    {
      bi= gfp::pr() - bi;
      neg= 1;
    }
  // reduce to range -2^(len-1), ..., 2^(len-1)
  bigint one= 1;
  bi&= (one << (len - 1)) - 1;
  if (neg)
    bi= -bi;
}

void convert_from_fix(mpf_class &res, const gfp &v, long k, long f)
{
  // v has k bits max
  bigint aa;
  to_signed_bigint(aa, v, k);
  res= aa;
  // compute v * 2^{-f}
  mpf_div_2exp(res.get_mpf_t(), res.get_mpf_t(), f);
}

void convert_from_float(mpf_class &res,
                        const gfp &v, const gfp &p, const gfp &z, const gfp &s)
{
  bigint aa, aa2;
  to_bigint(aa, v);
  // MPIR can't handle more precision in exponent
  to_signed_bigint(aa2, p, 31);
  long exp= aa2.get_si();
  res= aa;
  if (exp > 0)
    mpf_mul_2exp(res.get_mpf_t(), res.get_mpf_t(), exp);
  else
    mpf_div_2exp(res.get_mpf_t(), res.get_mpf_t(), -exp);
  if (z.is_one())
    res= 0;
  if (!s.is_zero())
    res*= -1;
  if (not z.is_bit() or not s.is_bit())
    throw Processor_Error("invalid floating point number");
}

template<class SRegint, class SBit>
void FP_to_fix(Processor<SRegint, SBit> &Proc)
{
  long f, k, x;
  Proc.pop_int(f);
  Proc.pop_int(k);
  Proc.pop_int(x);
  double double_x= convert_to_double(x);

  // Check OK
  double a= double_x * (1 << f);
  if (abs(a) > pow(2.0, k - 1))
    {
      throw not_implemented();
    }

  // Do conversion
  mpf_class float_x(double_x);
  mpf_class scaled;
  bigint intapprox;
  mpf_mul_2exp(scaled.get_mpf_t(), float_x.get_mpf_t(), f);
  mpz_set_f(intapprox.get_mpz_t(), scaled.get_mpf_t());
  gfp ans= intapprox;

  Proc.push_Cp(ans);
}

template<class SRegint, class SBit>
void FP_from_fix(Processor<SRegint, SBit> &Proc)
{
  long k, f;
  gfp x;
  Proc.pop_int(f);
  Proc.pop_int(k);
  Proc.pop_Cp(x);

  // Do conversion to mpf_class
  mpf_class res;
  convert_from_fix(res, x, k, f);

  double d= res.get_d();

  long ans= convert_to_long(d);
  Proc.push_int(ans);
}

template<class SRegint, class SBit>
void FP_to_float(Processor<SRegint, SBit> &Proc)
{
  long x, plen, vlen;
  Proc.pop_int(plen);
  Proc.pop_int(vlen);
  Proc.pop_int(x);
  if (vlen > 60 || plen > 60)
    {
      throw not_implemented();
    }

  double a= convert_to_double(x);
  gfp v, p, z, s, err;
  err= 0;
  s= 0;
  z= 0;
  if (a < 0)
    {
      s= 1;
    }
  if (isnan(a))
    {
      err= 1;
    }
  else if (a == 0)
    {
      z= 1;
      v= 0;
      p= 0;
    }
  else
    {
      mpf_class vv(abs(a));
      long pp= floor(log(abs(a) / log(2.0))) - vlen + 1;
      mpf_mul_2exp(vv.get_mpf_t(), vv.get_mpf_t(), -pp);
      bigint vi;
      mpz_set_f(vi.get_mpz_t(), vv.get_mpf_t());
      bigint twop= 2;
      mpz_pow_ui(twop.get_mpz_t(), twop.get_mpz_t(), vlen);

      while (vi >= twop)
        {
          vi>>= 1;
          pp++;
        }
      if (pp < (-(1 << (plen - 1))))
        { // Truncate towards zero
          z= 1;
          v= 0;
          p= 0;
        }
      else if (pp >= (1 >> (plen - 1)))
        {
          throw invalid_length();
        }
      else
        {
          p= pp;
          v= vi;
        }
    }
  Proc.push_Cp(err);
  Proc.push_Cp(s);
  Proc.push_Cp(z);
  Proc.push_Cp(p);
  Proc.push_Cp(v);
}

template<class SRegint, class SBit>
void FP_from_float(Processor<SRegint, SBit> &Proc)
{
  gfp v, p, z, s, err;
  Proc.pop_Cp(err);
  Proc.pop_Cp(s);
  Proc.pop_Cp(z);
  Proc.pop_Cp(p);
  Proc.pop_Cp(v);

  // Do conversion to mpf_class
  mpf_class res;
  convert_from_float(res, v, p, z, s);

  double d= res.get_d();
  if (!err.is_zero())
    {
      d= NAN;
    }

  long ans= convert_to_long(d);
  Proc.push_int(ans);
}

template void FP_add(Processor<aBitVector2, Share2> &Proc);
template void FP_mul(Processor<aBitVector2, Share2> &Proc);
template void FP_div(Processor<aBitVector2, Share2> &Proc);
template void FP_eq(Processor<aBitVector2, Share2> &Proc);
template void FP_f2i(Processor<aBitVector2, Share2> &Proc);
template void FP_i2f(Processor<aBitVector2, Share2> &Proc);
template void FP_sqrt(Processor<aBitVector2, Share2> &Proc);
template void FP_lt(Processor<aBitVector2, Share2> &Proc);

template void FP_add(Processor<aBitVector, aBit> &Proc);
template void FP_mul(Processor<aBitVector, aBit> &Proc);
template void FP_div(Processor<aBitVector, aBit> &Proc);
template void FP_eq(Processor<aBitVector, aBit> &Proc);
template void FP_f2i(Processor<aBitVector, aBit> &Proc);
template void FP_i2f(Processor<aBitVector, aBit> &Proc);
template void FP_sqrt(Processor<aBitVector, aBit> &Proc);
template void FP_lt(Processor<aBitVector, aBit> &Proc);

template void FP_to_fix(Processor<aBitVector2, Share2> &Proc);
template void FP_from_fix(Processor<aBitVector2, Share2> &Proc);
template void FP_to_float(Processor<aBitVector2, Share2> &Proc);
template void FP_from_float(Processor<aBitVector2, Share2> &Proc);

template void FP_to_fix(Processor<aBitVector, aBit> &Proc);
template void FP_from_fix(Processor<aBitVector, aBit> &Proc);
template void FP_to_float(Processor<aBitVector, aBit> &Proc);
template void FP_from_float(Processor<aBitVector, aBit> &Proc);

template void FP_acos(Processor<aBitVector2, Share2> &Proc);
template void FP_asin(Processor<aBitVector2, Share2> &Proc);
template void FP_atan(Processor<aBitVector2, Share2> &Proc);
template void FP_cos(Processor<aBitVector2, Share2> &Proc);
template void FP_cosh(Processor<aBitVector2, Share2> &Proc);
template void FP_sin(Processor<aBitVector2, Share2> &Proc);
template void FP_sinh(Processor<aBitVector2, Share2> &Proc);
template void FP_tanh(Processor<aBitVector2, Share2> &Proc);
template void FP_exp(Processor<aBitVector2, Share2> &Proc);
template void FP_log(Processor<aBitVector2, Share2> &Proc);
template void FP_log10(Processor<aBitVector2, Share2> &Proc);
template void FP_ceil(Processor<aBitVector2, Share2> &Proc);
template void FP_fabs(Processor<aBitVector2, Share2> &Proc);
template void FP_floor(Processor<aBitVector2, Share2> &Proc);

template void FP_acos(Processor<aBitVector, aBit> &Proc);
template void FP_asin(Processor<aBitVector, aBit> &Proc);
template void FP_atan(Processor<aBitVector, aBit> &Proc);
template void FP_cos(Processor<aBitVector, aBit> &Proc);
template void FP_cosh(Processor<aBitVector, aBit> &Proc);
template void FP_sin(Processor<aBitVector, aBit> &Proc);
template void FP_sinh(Processor<aBitVector, aBit> &Proc);
template void FP_tanh(Processor<aBitVector, aBit> &Proc);
template void FP_exp(Processor<aBitVector, aBit> &Proc);
template void FP_log(Processor<aBitVector, aBit> &Proc);
template void FP_log10(Processor<aBitVector, aBit> &Proc);
template void FP_ceil(Processor<aBitVector, aBit> &Proc);
template void FP_fabs(Processor<aBitVector, aBit> &Proc);
template void FP_floor(Processor<aBitVector, aBit> &Proc);
