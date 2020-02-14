/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "MMO.h"
#include "Math/gf2n.h"
#include "avx_memcpy.h"
#include <unistd.h>

void MMO::zeroIV()
{
  uint8_t key[AES_BLK_SIZE];
  memset(key, 0, AES_BLK_SIZE * sizeof(uint8_t));
  setIV(key);
}

void MMO::setIV(uint8_t key[AES_BLK_SIZE])
{
  aes_schedule(IV, key);
}

// The sigma function from eprint 2019/074
inline __m128i sigma(__m128i a)
{
  return _mm_xor_si128(_mm_shuffle_epi32(a, 78),
                       _mm_and_si128(a, _mm_set_epi64x(0xFFFFFFFFFFFFFFFF, 0x00)));
}

template<int N>
void MMO::encrypt_and_xor(void *output, const void *input, const uint8_t *key)
{
  __m128i in[N], out[N];
  for (int i= 0; i < N; i++)
    {
      in[i]= sigma(((__m128i *) input)[i]);
    }
  ecb_aes_128_encrypt<N>(out, in, key);
  for (int i= 0; i < N; i++)
    out[i]= _mm_xor_si128(out[i], in[i]);
  avx_memcpy(output, out, sizeof(out));
}

// XXX Is this next one used?
template<int N>
void MMO::encrypt_and_xor(void *output, const void *input, const uint8_t *key,
                          const int *indices)
{
  __m128i in[N], out[N];
  for (int i= 0; i < N; i++)
    in[i]= _mm_loadu_si128(((__m128i *) input) + indices[i]);
  encrypt_and_xor<N>(out, in, key);
  for (int i= 0; i < N; i++)
    _mm_storeu_si128(((__m128i *) output) + indices[i], out[i]);
}

template<>
void MMO::hashOneBlock<gf2n>(uint8_t *output, const uint8_t *input)
{
  encrypt_and_xor<1>(output, input, IV);
}

template<>
void MMO::hashBlockWise<gf2n, 128>(uint8_t *output, const uint8_t *input)
{
  for (int i= 0; i < 16; i++)
    encrypt_and_xor<8>(&((__m128i *) output)[i * 8], &((__m128i *) input)[i * 8], IV);
}
