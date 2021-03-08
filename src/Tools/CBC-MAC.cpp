/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, Un
ited Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, B
elgium.

All rights reserved
*/

#include "CBC-MAC.h"
#include "cpu-support.h"
#include <string.h>
#include <wmmintrin.h>

void CBC_MAC::zero_key()
{
  uint8_t key[AES_BLK_SIZE];
  memset(key, 0, AES_BLK_SIZE * sizeof(uint8_t));
  Set_Key(key);
}

CBC_MAC::CBC_MAC()
{
  useC= (cpu_has_aes() == false);
  zero_key();
}

CBC_MAC::CBC_MAC(uint8_t key[AES_BLK_SIZE])
{
  useC= (cpu_has_aes() == false);
  Set_Key(key);
}

void CBC_MAC::Set_Key(uint8_t key[AES_BLK_SIZE])
{
  if (useC)
    {
      aes_schedule(KeyScheduleC, key);
    }
  else
    {
      aes_schedule(KeySchedule, key);
    }
  Reset();
}

void CBC_MAC::Reset()
{
  memset(state, 0, AES_BLK_SIZE * sizeof(uint8_t));
  buff_length= 0;
}

void CBC_MAC::Update(const uint8_t *data, int length)
{
  // If stream+buff_length < AES_BLK_SIZE then just do a xor in
  if ((length + buff_length) < AES_BLK_SIZE)
    {
      for (int i= 0; i < length; i++)
        {
          state[i + buff_length]^= data[i];
        }
      buff_length+= length;
    }
  else
    {
      uint8_t output[AES_BLK_SIZE];
      int pos= 0;
      // First deal with the bit in the buffer
      if (buff_length != 0)
        {
          for (int i= 0; i < AES_BLK_SIZE - buff_length; i++)
            {
              state[i + buff_length]^= data[pos];
              pos++;
            }
          buff_length= 0;
          if (useC)
            {
              aes_encrypt(output, state, KeyScheduleC);
            }
          else
            {
              aes_encrypt(output, state, KeySchedule);
            }
          memcpy(state, output, AES_BLK_SIZE * sizeof(uint8_t));
        }
      // Now deal with the main bit of s
      if (useC)
        {
          while ((pos + AES_BLK_SIZE) <= length)
            {
              for (int i= 0; i < AES_BLK_SIZE; i++)
                {
                  *(state + i)^= *(data + i + pos);
                }
              aes_encrypt(output, state, KeyScheduleC);
              pos+= AES_BLK_SIZE;
            }
          memcpy(state, output, AES_BLK_SIZE * sizeof(uint8_t));
        }
      else
        {
          __m128i tmp= _mm_loadu_si128((__m128i *) state);
          while ((pos + AES_BLK_SIZE) <= length)
            {
              // manually convert uint8_t in data to __m128i
              // as simple casting causes overflow
              __m128i tmp2= _mm_cvtsi32_si128((int) (data[pos]));
              tmp= _mm_xor_si128(tmp, tmp2); // ( (__m128i*)(data+pos) )[0]);
              tmp= aes_encrypt(tmp, KeySchedule);
              pos+= AES_BLK_SIZE;
            }
          _mm_storeu_si128((__m128i *) state, tmp);
        }
      // Now buffer the remaining stuff until next time
      buff_length= length - pos;
      for (int i= 0; i < buff_length; i++)
        {
          state[i]^= data[i + pos];
        }
    }
}

void CBC_MAC::Finalize(uint8_t output[AES_BLK_SIZE])
{
  // Deal with remaining stuff in buffer if needed
  if (buff_length != 0)
    {
      if (useC)
        {
          aes_encrypt(output, state, KeyScheduleC);
        }
      else
        {
          aes_encrypt(output, state, KeySchedule);
        }
    }
  else
    {
      memcpy(output, state, AES_BLK_SIZE * sizeof(uint8_t));
    }
}
