/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef TOOLS_MMO_H_
#define TOOLS_MMO_H_

#include "Tools/aes.h"

// Matyas-Meyer-Oseas hashing
class MMO
{
  uint8_t IV[176] __attribute__((aligned(16)));

  template<int N>
  static void encrypt_and_xor(void *output, const void *input,
                              const uint8_t *key);
  template<int N>
  static void encrypt_and_xor(void *output, const void *input,
                              const uint8_t *key, const int *indices);

public:
  MMO() { zeroIV(); }
  void zeroIV();
  void setIV(uint8_t key[AES_BLK_SIZE]);

  template<class T>
  void hashOneBlock(uint8_t *output, const uint8_t *input);

  template<class T, int N>
  void hashBlockWise(uint8_t *output, const uint8_t *input);

  template<class T>
  void outputOneBlock(uint8_t *output);
};

#endif /* TOOLS_MMO_H_ */
