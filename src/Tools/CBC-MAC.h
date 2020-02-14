/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _CBC_MAC
#define _CBC_MAC

/* This defines CBC-MAC using AES-128
 *
 * The version of CBC-MAC is with zero padding upto the block
 * boundary. As such this should not be used as a main MAC function.
 * The main purpose is to define a PRG via  
 *       F(M) = PRNG(CBC_MAC(key=0,message=M))
 * Note if message=0 then the PRNG is just the output of
 * the CBC_MAC itself
 *
 */

#include "Tools/aes.h"

class CBC_MAC
{
  bool useC;

  // Two types of key schedule for the different implementations
  // of AES
  uint KeyScheduleC[44];
  uint8_t KeySchedule[176] __attribute__((aligned(16)));

  uint8_t state[AES_BLK_SIZE];
  int buff_length;

public:
  // Default is the zero key
  CBC_MAC();
  // Now use this key
  CBC_MAC(uint8_t key[AES_BLK_SIZE]);

  void zero_key();

  // This allows you to set a different key if wanted
  void Set_Key(uint8_t key[AES_BLK_SIZE]);

  // Reset the CBC-Mode operation
  void Reset();

  // Pass in some data
  void Update(const uint8_t *data, int length);

  // Get the final value
  void Finalize(uint8_t output[AES_BLK_SIZE]);
};

#endif
