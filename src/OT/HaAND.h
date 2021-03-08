/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _HaAND
#define _HaAND

/* This produces the output from Figure 16 of
 * ePrint 2017/189
 *
 * We do OT_Amort values at a time
 *   This is basically the number of executions of the MMO
 *   function we do in parallel. So if you change this you
 *   need to change the MMO template instantiation.
 *
 *   Assumption that 32768 is divisible by OT_Amort
 */

#define OT_Amort 128

#include "Tools/buffer.h"
#include "aBit.h"

class HaAND
{
  static const unsigned int number= 32768;
  // Base key for the MMO key
  uint8_t base_key[AES_BLK_SIZE];
  // Counter to increase the MMO key on each major iteration
  unsigned int counter;

  vector<aBit> x, y;
  vector<vector<gf2n>> HKi, HKiD, HMi;
  vector<int> v;

  buffer buff;

public:
  HaAND()
  {
    memset(base_key, 0, AES_BLK_SIZE);
  }
  HaAND(Player &P, int connection)
  {
    Init(P, connection);
  }
  void Init(Player &P, int connection);

  /* This is protocol 16 of ePrint 2017/189 executed 131072 times */
  void make_more(Player &P, int num_online);

  static unsigned int get_number()
  {
    return number;
  }

  // Access functions
  aBit get_x(unsigned int i) const { return x[i]; }
  aBit get_y(unsigned int i) const { return y[i]; }
  int get_v(unsigned int i) const { return v[i]; }
  gf2n get_HKi(unsigned int i, unsigned int p) const { return HKi[i][p]; }
  gf2n get_HKiD(unsigned int i, unsigned int p) const { return HKiD[i][p]; }
  gf2n get_HMi(unsigned int i, unsigned int p) const { return HMi[i][p]; }
};

#endif
