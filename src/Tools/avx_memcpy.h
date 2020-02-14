/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef TOOLS_AVX_MEMCPY_H_
#define TOOLS_AVX_MEMCPY_H_

#include <immintrin.h>
#include <string.h>

inline void avx_memcpy(void *dest, const void *source, size_t length)
{
  __m256i *d= (__m256i *) dest, *s= (__m256i *) source;
  while (length >= 32)
    {
      _mm256_storeu_si256(d++, _mm256_loadu_si256(s++));
      length-= 32;
    }
  __m128i *d2= (__m128i *) d;
  __m128i *s2= (__m128i *) s;
  while (length >= 16)
    {
      _mm_storeu_si128(d2++, _mm_loadu_si128(s2++));
      length-= 16;
    }
  if (length)
    memcpy(d2, s2, length);
}

inline void avx_memzero(void *dest, size_t length)
{
  __m256i *d= (__m256i *) dest;
  __m256i s= _mm256_setzero_si256();
  while (length >= 32)
    {
      _mm256_storeu_si256(d++, s);
      length-= 32;
    }
  if (length)
    memset((void *) d, 0, length);
}

#endif /* TOOLS_AVX_MEMCPY_H_ */
