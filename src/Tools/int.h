/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
/*
 * int.h
 *
 *  Created on: 18 Jan 2016
 *      Author: csmksk
 */

#ifndef TOOLS_INT_H_
#define TOOLS_INT_H_

// Assumes word is a 64 bit value
#ifdef WIN32
typedef unsigned __int64 word;
#else
typedef unsigned long word;
#endif

inline int CEIL_LOG2(int x)
{
  int result= 0;
  x--;
  while (x > 0)
    {
      result++;
      x>>= 1;
    }
  return result;
}

inline int FLOOR_LOG2(int x)
{
  int result= 0;
  while (x > 1)
    {
      result++;
      x>>= 1;
    }
  return result;
}

// ceil(n / k)
inline int DIV_CEIL(long n, int k) { return (n + k - 1) / k; }

inline void INT_TO_BYTES(uint8_t *buff, int x)
{
  buff[0]= x & 255;
  buff[1]= (x >> 8) & 255;
  buff[2]= (x >> 16) & 255;
  buff[3]= (x >> 24) & 255;
}

inline int BYTES_TO_INT(uint8_t *buff)
{
  return buff[0] + 256 * buff[1] + 65536 * buff[2] + 16777216 * buff[3];
}

inline int positive_modulo(int i, int n) { return (i % n + n) % n; }

#endif /* TOOLS_INT_H_ */
