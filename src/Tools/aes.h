/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef __AES_H
#define __AES_H

#include <cstdint>
#include <sys/types.h>
#include <wmmintrin.h>

#define AES_BLK_SIZE 16

/************* C Version *************/
// Key Schedule
void aes_schedule(int nb, int nr, uint8_t *k, uint *RK);

inline void aes_schedule(uint *RK, uint8_t *K) { aes_schedule(4, 10, K, RK); }
inline void aes_128_schedule(uint *RK, uint8_t *K)
{
  aes_schedule(4, 10, K, RK);
}
inline void aes_192_schedule(uint *RK, uint8_t *K)
{
  aes_schedule(6, 12, K, RK);
}
inline void aes_256_schedule(uint *RK, uint8_t *K)
{
  aes_schedule(8, 14, K, RK);
}

// Encryption Function
void aes_128_encrypt(uint8_t *C, uint8_t *M, uint *RK);
void aes_192_encrypt(uint8_t *C, uint8_t *M, uint *RK);
void aes_256_encrypt(uint8_t *C, uint8_t *M, uint *RK);

inline void aes_encrypt(uint8_t *C, uint8_t *M, uint *RK)
{
  aes_128_encrypt(C, M, RK);
}

/*********** M-Code Version ***********/

// Key Schedule
void aes_128_schedule(uint8_t *key, const uint8_t *userkey);
void aes_192_schedule(uint8_t *key, const uint8_t *userkey);
void aes_256_schedule(uint8_t *key, const uint8_t *userkey);

inline void aes_schedule(uint8_t *key, const uint8_t *userkey)
{
  aes_128_schedule(key, userkey);
}

// Encryption Function
void aes_128_encrypt(uint8_t *C, const uint8_t *M, const uint8_t *RK);
void aes_192_encrypt(uint8_t *C, const uint8_t *M, const uint8_t *RK);
void aes_256_encrypt(uint8_t *C, const uint8_t *M, const uint8_t *RK);

__attribute__((optimize("unroll-loops"))) inline __m128i aes_128_encrypt(__m128i in, const uint8_t *key)
{
  __m128i &tmp= in;
  tmp= _mm_xor_si128(tmp, ((__m128i *) key)[0]);
  int j;
  for (j= 1; j < 10; j++)
    {
      tmp= _mm_aesenc_si128(tmp, ((__m128i *) key)[j]);
    }
  tmp= _mm_aesenclast_si128(tmp, ((__m128i *) key)[j]);
  return tmp;
}

template<int N>
__attribute__((optimize("unroll-loops"))) inline void ecb_aes_128_encrypt(__m128i *out, __m128i *in, const uint8_t *key)
{
  __m128i tmp[N];
  for (int i= 0; i < N; i++)
    tmp[i]= _mm_xor_si128(in[i], ((__m128i *) key)[0]);
  int j;
  for (j= 1; j < 10; j++)
    for (int i= 0; i < N; i++)
      tmp[i]= _mm_aesenc_si128(tmp[i], ((__m128i *) key)[j]);
  for (int i= 0; i < N; i++)
    out[i]= _mm_aesenclast_si128(tmp[i], ((__m128i *) key)[j]);
}

template<int N>
inline void ecb_aes_128_encrypt(__m128i *out, const __m128i *in,
                                const uint8_t *key, const int *indices)
{
  __m128i tmp[N];
  for (int i= 0; i < N; i++)
    tmp[i]= in[indices[i]];
  ecb_aes_128_encrypt<N>(tmp, tmp, key);
  for (int i= 0; i < N; i++)
    out[indices[i]]= tmp[i];
}

inline void aes_encrypt(uint8_t *C, const uint8_t *M, const uint8_t *RK)
{
  aes_128_encrypt(C, M, RK);
}

inline __m128i aes_encrypt(__m128i M, const uint8_t *RK)
{
  return aes_128_encrypt(M, RK);
}

#endif
