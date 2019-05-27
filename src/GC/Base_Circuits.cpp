/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2019, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Base_Circuits.h"
#include <fstream>

#include "OT/aBitVector.h"
#include "config.h"

/* This it the global store of circuits */
Base_Circuits Global_Circuit_Store;

void Base_Circuits::initialize(const bigint &p)
{
  BaseC.resize(Number_Circuits);

  ifstream inpf;

  inpf.open("Circuits/Bristol/adder64.txt");
  inpf >> BaseC[Adder64];
  inpf.close();

  inpf.open("Circuits/Bristol/sub64.txt");
  inpf >> BaseC[Sub64];
  inpf.close();

  inpf.open("Circuits/Bristol/mult2_64.txt");
  inpf >> BaseC[Mult2_64];
  inpf.close();

  inpf.open("Circuits/Bristol/mult64.txt");
  inpf >> BaseC[Mult64];
  inpf.close();

  inpf.open("Circuits/Bristol/divide64.txt");
  inpf >> BaseC[Divide64];
  inpf.close();

  inpf.open("Circuits/Bristol/neg64.txt");
  inpf >> BaseC[Neg64];
  inpf.close();

  inpf.open("Circuits/Bristol/zero_equal.txt");
  inpf >> BaseC[Zero_Equal];
  inpf.close();

  inpf.open("Circuits/Bristol/compare_eq.txt");
  inpf >> BaseC[Compare_Eq];
  inpf.close();

  inpf.open("Circuits/Bristol/compare.txt");
  inpf >> BaseC[Compare];
  inpf.close();

  /* We can do a conversion if log_2 p bits mod p 
   * give something statistically close to random mod p
   *   AND
   * log 2_p > sreg_bitl
   *
   * For the first condition we require
   *
   * Either
   *     log_2 p > sreg_bitl+conv_stat_sec
   * or
   *     log_2 ((p - 2^(log_2 p))/2^log_2 p) < - conv_stat_sec
   *
   * or
   *     log_2 ((2^(log_2 p) - p)/p) < -conv_stat_sec
   */
  int lg2p= numBits(p);
  bigint tplg2p= bigint(1) << lg2p;
  bigint x1= (tplg2p / 2) / (p - tplg2p / 2);
  bigint x2= p / (tplg2p - p);
  int y1= numBits(x1), y2= numBits(x2);
  if (lg2p > sreg_bitl &&
      ((lg2p > sreg_bitl + conv_stat_sec) || (y1 > conv_stat_sec) || (y2 > conv_stat_sec)))
    {
      inpf.open("Data/ConversionCircuit-LSSS_to_GC.txt");
      inpf >> BaseC[LSSS_to_GC];
      inpf.close();

      convert_ok= true;
    }
  else
    {
      convert_ok= false;
    }
}
