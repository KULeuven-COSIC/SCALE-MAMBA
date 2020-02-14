/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _BITVECTOR
#define _BITVECTOR

/* Vector of bits */

#include <iostream>
#include <vector>
using namespace std;
#include <pmmintrin.h>
#include <stdlib.h>

#include "Exceptions/Exceptions.h"
// just for util functions
#include "Math/bigint.h"
#include "Math/gf2n.h"

class PRNG;

class BitVector
{
  uint8_t *bytes;

  size_t nbytes;
  size_t nbits;
  size_t length;

public:
  void assign(const BitVector &K)
  {
    if (nbits != K.nbits)
      {
        resize(K.nbits);
      }
    memcpy(bytes, K.bytes, nbytes);
  }
  void assign_bytes(char *new_bytes, int len)
  {
    resize(len * 8);
    memcpy(bytes, new_bytes, len);
  }
  void assign_zero()
  {
    memset(bytes, 0, nbytes);
  }
  // only grows, never destroys
  void resize(size_t new_nbits)
  {
    if (nbits != new_nbits)
      {
        int new_nbytes= DIV_CEIL(new_nbits, 8);

        if (nbits < new_nbits)
          {
            uint8_t *tmp= new uint8_t[new_nbytes];
            memcpy(tmp, bytes, nbytes);
            delete[] bytes;
            bytes= tmp;
          }

        nbits= new_nbits;
        nbytes= new_nbytes;
        /*
            // use realloc to preserve original contents
            if (new_nbits < nbits)
            {
                memcpy(tmp, bytes, new_nbytes);
            }
            else
            {
                memset(tmp, 0, new_nbytes);
                memcpy(tmp, bytes, nbytes);   
            }*/

        // realloc may fail on size 0
        /*if (new_nbits == 0)
            {
                free(bytes);
                bytes = (uint8_t*) malloc(0);//new uint8_t[0];
                //free(bytes);
                return;
            }
            bytes = (uint8_t*)realloc(bytes, nbytes);
            if (bytes == NULL)
            {
                cerr << "realloc failed\n";
                exit(1);
            }*/
        /*delete[] bytes;
            nbits = new_nbits;
            nbytes = DIV_CEIL(nbits, 8);
            bytes = new uint8_t[nbytes];*/
      }
  }
  unsigned int size() const { return nbits; }
  unsigned int size_bytes() const { return nbytes; }
  uint8_t *get_ptr() { return bytes; }

  BitVector(size_t n= 128)
  {
    nbits= n;
    nbytes= DIV_CEIL(nbits, 8);
    bytes= new uint8_t[nbytes];
    length= n;
    assign_zero();
  }
  BitVector(const BitVector &K)
  {
    bytes= new uint8_t[K.nbytes];
    nbytes= K.nbytes;
    nbits= K.nbits;
    assign(K);
  }
  ~BitVector()
  {
    //cout << "Destroy, size = " << nbytes << endl;
    delete[] bytes;
  }
  BitVector &operator=(const BitVector &K)
  {
    if (this != &K)
      {
        assign(K);
      }
    return *this;
  }

  uint8_t get_byte(int i) const { return bytes[i]; }

  void set_byte(int i, uint8_t b) { bytes[i]= b; }

  // get the i-th 64-bit word
  word get_word(int i) const { return *(word *) (bytes + i * 8); }

  void set_word(int i, word w)
  {
    int offset= i * sizeof(word);
    memcpy(bytes + offset, (uint8_t *) &w, sizeof(word));
  }

  int128 get_int128(int i) const { return _mm_lddqu_si128((__m128i *) bytes + i); }
  void set_int128(int i, int128 a) { *((__m128i *) bytes + i)= a.a; }

  int get_bit(int i) const
  {
    return (bytes[i / 8] >> (i % 8)) & 1;
  }
  void set_bit(int i, unsigned int a)
  {
    int j= i / 8, k= i & 7;
    if (a == 1)
      {
        bytes[j]|= (uint8_t)(1UL << k);
      }
    else
      {
        bytes[j]&= (uint8_t) ~(1UL << k);
      }
  }

  void add(const BitVector &A, const BitVector &B)
  {
    if (A.nbits != B.nbits)
      {
        throw invalid_length();
      }
    resize(A.nbits);
    for (unsigned int i= 0; i < nbytes; i++)
      {
        bytes[i]= A.bytes[i] ^ B.bytes[i];
      }
  }

  void add(const BitVector &A)
  {
    if (nbits != A.nbits)
      {
        throw invalid_length();
      }
    for (unsigned int i= 0; i < nbytes; i++)
      {
        bytes[i]^= A.bytes[i];
      }
  }

  bool equals(const BitVector &K) const
  {
    if (nbits != K.nbits)
      {
        throw invalid_length();
      }
    for (unsigned int i= 0; i < nbytes; i++)
      {
        if (bytes[i] != K.bytes[i])
          {
            return false;
          }
      }
    return true;
  }

  void randomize(PRNG &G);
  // randomize bytes a, ..., a+nb-1
  void randomize_at(int a, int nb, PRNG &G);

  void output(ostream &s, bool human= false) const;
  void input(istream &s, bool human= false);

  string str()
  {
    stringstream ss;
    ss << hex;
    for (size_t i(0); i < nbytes; ++i)
      ss << (int) bytes[i] << " ";
    return ss.str();
  }
};

#endif
