/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "aes.h"

/**********************
 *    M-Code Version  *
 **********************/

inline __m128i AES_128_ASSIST(__m128i temp1, __m128i temp2)
{
  __m128i temp3;
  temp2= _mm_shuffle_epi32(temp2, 0xff);
  temp3= _mm_slli_si128(temp1, 0x4);
  temp1= _mm_xor_si128(temp1, temp3);
  temp3= _mm_slli_si128(temp3, 0x4);
  temp1= _mm_xor_si128(temp1, temp3);
  temp3= _mm_slli_si128(temp3, 0x4);
  temp1= _mm_xor_si128(temp1, temp3);
  temp1= _mm_xor_si128(temp1, temp2);
  return temp1;
}

void aes_128_schedule(uint8_t *key, const uint8_t *userkey)
{
  __m128i temp1, temp2;
  __m128i *Key_Schedule= (__m128i *) key;
  temp1= _mm_loadu_si128((__m128i *) userkey);
  Key_Schedule[0]= temp1;
  temp2= _mm_aeskeygenassist_si128(temp1, 0x1);
  temp1= AES_128_ASSIST(temp1, temp2);
  Key_Schedule[1]= temp1;
  temp2= _mm_aeskeygenassist_si128(temp1, 0x2);
  temp1= AES_128_ASSIST(temp1, temp2);
  Key_Schedule[2]= temp1;
  temp2= _mm_aeskeygenassist_si128(temp1, 0x4);
  temp1= AES_128_ASSIST(temp1, temp2);
  Key_Schedule[3]= temp1;
  temp2= _mm_aeskeygenassist_si128(temp1, 0x8);
  temp1= AES_128_ASSIST(temp1, temp2);
  Key_Schedule[4]= temp1;
  temp2= _mm_aeskeygenassist_si128(temp1, 0x10);
  temp1= AES_128_ASSIST(temp1, temp2);
  Key_Schedule[5]= temp1;
  temp2= _mm_aeskeygenassist_si128(temp1, 0x20);
  temp1= AES_128_ASSIST(temp1, temp2);
  Key_Schedule[6]= temp1;
  temp2= _mm_aeskeygenassist_si128(temp1, 0x40);
  temp1= AES_128_ASSIST(temp1, temp2);
  Key_Schedule[7]= temp1;
  temp2= _mm_aeskeygenassist_si128(temp1, 0x80);
  temp1= AES_128_ASSIST(temp1, temp2);
  Key_Schedule[8]= temp1;
  temp2= _mm_aeskeygenassist_si128(temp1, 0x1b);
  temp1= AES_128_ASSIST(temp1, temp2);
  Key_Schedule[9]= temp1;
  temp2= _mm_aeskeygenassist_si128(temp1, 0x36);
  temp1= AES_128_ASSIST(temp1, temp2);
  Key_Schedule[10]= temp1;
}

inline void KEY_192_ASSIST(__m128i *temp1, __m128i *temp2, __m128i *temp3)
{
  __m128i temp4;
  *temp2= _mm_shuffle_epi32(*temp2, 0x55);
  temp4= _mm_slli_si128(*temp1, 0x4);
  *temp1= _mm_xor_si128(*temp1, temp4);
  temp4= _mm_slli_si128(temp4, 0x4);
  *temp1= _mm_xor_si128(*temp1, temp4);
  temp4= _mm_slli_si128(temp4, 0x4);
  *temp1= _mm_xor_si128(*temp1, temp4);
  *temp1= _mm_xor_si128(*temp1, *temp2);
  *temp2= _mm_shuffle_epi32(*temp1, 0xff);
  temp4= _mm_slli_si128(*temp3, 0x4);
  *temp3= _mm_xor_si128(*temp3, temp4);
  *temp3= _mm_xor_si128(*temp3, *temp2);
}

void aes_192_schedule(uint8_t *key, const uint8_t *userkey)
{
  __m128i temp1, temp2, temp3;
  __m128i *Key_Schedule= (__m128i *) key;
  temp1= _mm_loadu_si128((__m128i *) userkey);
  temp3= _mm_loadu_si128((__m128i *) (userkey + 16));
  Key_Schedule[0]= temp1;
  Key_Schedule[1]= temp3;
  temp2= _mm_aeskeygenassist_si128(temp3, 0x1);
  KEY_192_ASSIST(&temp1, &temp2, &temp3);
  Key_Schedule[1]=
      (__m128i) _mm_shuffle_pd((__m128d) Key_Schedule[1], (__m128d) temp1, 0);
  Key_Schedule[2]= (__m128i) _mm_shuffle_pd((__m128d) temp1, (__m128d) temp3, 1);
  temp2= _mm_aeskeygenassist_si128(temp3, 0x2);
  KEY_192_ASSIST(&temp1, &temp2, &temp3);
  Key_Schedule[3]= temp1;
  Key_Schedule[4]= temp3;
  temp2= _mm_aeskeygenassist_si128(temp3, 0x4);
  KEY_192_ASSIST(&temp1, &temp2, &temp3);
  Key_Schedule[4]=
      (__m128i) _mm_shuffle_pd((__m128d) Key_Schedule[4], (__m128d) temp1, 0);
  Key_Schedule[5]= (__m128i) _mm_shuffle_pd((__m128d) temp1, (__m128d) temp3, 1);
  temp2= _mm_aeskeygenassist_si128(temp3, 0x8);
  KEY_192_ASSIST(&temp1, &temp2, &temp3);
  Key_Schedule[6]= temp1;
  Key_Schedule[7]= temp3;
  temp2= _mm_aeskeygenassist_si128(temp3, 0x10);
  KEY_192_ASSIST(&temp1, &temp2, &temp3);
  Key_Schedule[7]=
      (__m128i) _mm_shuffle_pd((__m128d) Key_Schedule[7], (__m128d) temp1, 0);
  Key_Schedule[8]= (__m128i) _mm_shuffle_pd((__m128d) temp1, (__m128d) temp3, 1);
  temp2= _mm_aeskeygenassist_si128(temp3, 0x20);
  KEY_192_ASSIST(&temp1, &temp2, &temp3);
  Key_Schedule[9]= temp1;
  Key_Schedule[10]= temp3;
  temp2= _mm_aeskeygenassist_si128(temp3, 0x40);
  KEY_192_ASSIST(&temp1, &temp2, &temp3);
  Key_Schedule[10]=
      (__m128i) _mm_shuffle_pd((__m128d) Key_Schedule[10], (__m128d) temp1, 0);
  Key_Schedule[11]= (__m128i) _mm_shuffle_pd((__m128d) temp1, (__m128d) temp3, 1);
  temp2= _mm_aeskeygenassist_si128(temp3, 0x80);
  KEY_192_ASSIST(&temp1, &temp2, &temp3);
  Key_Schedule[12]= temp1;
}

inline void KEY_256_ASSIST_1(__m128i *temp1, __m128i *temp2)
{
  __m128i temp4;
  *temp2= _mm_shuffle_epi32(*temp2, 0xff);
  temp4= _mm_slli_si128(*temp1, 0x4);
  *temp1= _mm_xor_si128(*temp1, temp4);
  temp4= _mm_slli_si128(temp4, 0x4);
  *temp1= _mm_xor_si128(*temp1, temp4);
  temp4= _mm_slli_si128(temp4, 0x4);
  *temp1= _mm_xor_si128(*temp1, temp4);
  *temp1= _mm_xor_si128(*temp1, *temp2);
}

inline void KEY_256_ASSIST_2(__m128i *temp1, __m128i *temp3)
{
  __m128i temp2, temp4;
  temp4= _mm_aeskeygenassist_si128(*temp1, 0x0);
  temp2= _mm_shuffle_epi32(temp4, 0xaa);
  temp4= _mm_slli_si128(*temp3, 0x4);
  *temp3= _mm_xor_si128(*temp3, temp4);
  temp4= _mm_slli_si128(temp4, 0x4);
  *temp3= _mm_xor_si128(*temp3, temp4);
  temp4= _mm_slli_si128(temp4, 0x4);
  *temp3= _mm_xor_si128(*temp3, temp4);
  *temp3= _mm_xor_si128(*temp3, temp2);
}

void aes_256_schedule(uint8_t *key, const uint8_t *userkey)
{
  __m128i temp1, temp2, temp3;
  __m128i *Key_Schedule= (__m128i *) key;
  temp1= _mm_loadu_si128((__m128i *) userkey);
  temp3= _mm_loadu_si128((__m128i *) (userkey + 16));
  Key_Schedule[0]= temp1;
  Key_Schedule[1]= temp3;
  temp2= _mm_aeskeygenassist_si128(temp3, 0x01);
  KEY_256_ASSIST_1(&temp1, &temp2);
  Key_Schedule[2]= temp1;
  KEY_256_ASSIST_2(&temp1, &temp3);
  Key_Schedule[3]= temp3;
  temp2= _mm_aeskeygenassist_si128(temp3, 0x02);
  KEY_256_ASSIST_1(&temp1, &temp2);
  Key_Schedule[4]= temp1;
  KEY_256_ASSIST_2(&temp1, &temp3);
  Key_Schedule[5]= temp3;
  temp2= _mm_aeskeygenassist_si128(temp3, 0x04);
  KEY_256_ASSIST_1(&temp1, &temp2);
  Key_Schedule[6]= temp1;
  KEY_256_ASSIST_2(&temp1, &temp3);
  Key_Schedule[7]= temp3;
  temp2= _mm_aeskeygenassist_si128(temp3, 0x08);
  KEY_256_ASSIST_1(&temp1, &temp2);
  Key_Schedule[8]= temp1;
  KEY_256_ASSIST_2(&temp1, &temp3);
  Key_Schedule[9]= temp3;
  temp2= _mm_aeskeygenassist_si128(temp3, 0x10);
  KEY_256_ASSIST_1(&temp1, &temp2);
  Key_Schedule[10]= temp1;
  KEY_256_ASSIST_2(&temp1, &temp3);
  Key_Schedule[11]= temp3;
  temp2= _mm_aeskeygenassist_si128(temp3, 0x20);
  KEY_256_ASSIST_1(&temp1, &temp2);
  Key_Schedule[12]= temp1;
  KEY_256_ASSIST_2(&temp1, &temp3);
  Key_Schedule[13]= temp3;
  temp2= _mm_aeskeygenassist_si128(temp3, 0x40);
  KEY_256_ASSIST_1(&temp1, &temp2);
  Key_Schedule[14]= temp1;
}

void aes_128_encrypt(uint8_t *out, const uint8_t *in, const uint8_t *key)
{
  __m128i tmp;
  tmp= _mm_loadu_si128(&((__m128i *) in)[0]);
  tmp= aes_128_encrypt(tmp, key);
  _mm_storeu_si128(&((__m128i *) out)[0], tmp);
}

void aes_192_encrypt(uint8_t *out, const uint8_t *in, const uint8_t *key)
{
  __m128i tmp;
  tmp= _mm_loadu_si128(&((__m128i *) in)[0]);
  tmp= _mm_xor_si128(tmp, ((__m128i *) key)[0]);
  int j;
  for (j= 1; j < 12; j++)
    {
      tmp= _mm_aesenc_si128(tmp, ((__m128i *) key)[j]);
    }
  tmp= _mm_aesenclast_si128(tmp, ((__m128i *) key)[j]);
  _mm_storeu_si128(&((__m128i *) out)[0], tmp);
}

void aes_256_encrypt(uint8_t *out, const uint8_t *in, const uint8_t *key)
{
  __m128i tmp;
  tmp= _mm_loadu_si128(&((__m128i *) in)[0]);
  tmp= _mm_xor_si128(tmp, ((__m128i *) key)[0]);
  int j;
  for (j= 1; j < 14; j++)
    {
      tmp= _mm_aesenc_si128(tmp, ((__m128i *) key)[j]);
    }
  tmp= _mm_aesenclast_si128(tmp, ((__m128i *) key)[j]);
  _mm_storeu_si128(&((__m128i *) out)[0], tmp);
}
