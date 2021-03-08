/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _buffer
#define _buffer

/* Defines a class to act as a uint8 buffer for IO purposes.
 * A bit like the old octetstream but less fancy and complex
 *
 * Note all copying does not copy the contents as buffer
 * are meant to be transiant
 */

#include <stddef.h>
#include <stdint.h>

class buffer
{

  uint8_t *buff;
  unsigned long sz;

public:
  // Default Contructor
  buffer()
  {
    buff= new uint8_t[1024];
    sz= 1024;
  }
  buffer(unsigned int l)
  {
    sz= l;
    buff= new uint8_t[l];
  }
  // Copy Constructor
  buffer(const buffer &B)
  {
    buff= new uint8_t[B.sz];
    sz= B.sz;
  }
  // Move Constructor
  buffer(buffer &&B)
  {
    buff= B.buff;
    sz= B.sz;
    B.buff= NULL;
  }
  // Assignment operator
  buffer &operator=(const buffer &B)
  {
    if (this != &B)
      {
        buff= new uint8_t[B.sz];
        sz= B.sz;
      }
    return *this;
  }

  ~buffer()
  {
    if (buff != NULL)
      {
        delete[] buff;
      }
  }

  // This is destructive of the contents
  //   Adds a more than needed, to avoid keep doing this
  void resize(unsigned long s1)
  {
    if (s1 >= sz)
      {
        delete[] buff;
        sz= 2 * s1;
        buff= new uint8_t[sz];
      }
  }

  // This reduces the size of the buffer to something
  // small. Call this when you dont think you will need
  // a buffer of this size again
  void clean()
  {
    delete[] buff;
    sz= 1024;
    buff= new uint8_t[sz];
  }

  uint8_t *get_buffer() const { return buff; }
};

#endif
