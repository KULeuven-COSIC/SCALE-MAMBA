/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Base_Circuits.h"
#include <fstream>
#include <mutex>

#include "OT/aBitVector.h"
#include "config.h"

/* This it the global store of circuits */
Base_Circuits Global_Circuit_Store;

void Base_Circuits::initialize(const bigint &p)
{
  Circuit C;

  ifstream inpf;

  inpf.open("Circuits/Bristol/adder64.txt");
  if (inpf.fail())
    {
      throw file_error("Circuits/Bristol/adder64.txt");
    }
  inpf >> C;
  inpf.close();
  Circuits.insert(make_pair(0, C));

  inpf.open("Circuits/Bristol/sub64.txt");
  if (inpf.fail())
    {
      throw file_error("Circuits/Bristol/sub64.txt");
    }
  inpf >> C;
  inpf.close();
  Circuits.insert(make_pair(1, C));

  inpf.open("Circuits/Bristol/mult2_64.txt");
  if (inpf.fail())
    {
      throw file_error("Circuits/Bristol/mult2_64.txt");
    }
  inpf >> C;
  inpf.close();
  Circuits.insert(make_pair(2, C));

  inpf.open("Circuits/Bristol/mult64.txt");
  if (inpf.fail())
    {
      throw file_error("Circuits/Bristol/mult64.txt");
    }
  inpf >> C;
  inpf.close();
  Circuits.insert(make_pair(3, C));

  inpf.open("Circuits/Bristol/divide64.txt");
  if (inpf.fail())
    {
      throw file_error("Circuits/Bristol/divide64.txt");
    }
  inpf >> C;
  inpf.close();
  Circuits.insert(make_pair(4, C));

  inpf.open("Circuits/Bristol/neg64.txt");
  if (inpf.fail())
    {
      throw file_error("Circuits/Bristol/neg64.txt");
    }
  inpf >> C;
  inpf.close();
  Circuits.insert(make_pair(5, C));

  inpf.open("Circuits/Bristol/zero_equal.txt");
  if (inpf.fail())
    {
      throw file_error("Circuits/Bristol/zero_equal.txt");
    }
  inpf >> C;
  inpf.close();
  Circuits.insert(make_pair(6, C));

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
      if (inpf.fail())
        {
          throw file_error("Data/ConversionCircuit-LSSS_to_GC.txt");
        }
      inpf >> C;
      inpf.close();
      Circuits.insert(make_pair(7, C));

      convert_ok= true;
    }
  else
    {
      convert_ok= false;
    }

  /* Now the reserved indirect called circuits 
   *   - Those called by the GC opcode
   */
  loaded.insert(make_pair(100, false));
  location.insert(make_pair(100, "Circuits/Bristol/aes_128.txt"));
  loaded.insert(make_pair(101, false));
  location.insert(make_pair(101, "Circuits/Bristol/aes_192.txt"));
  loaded.insert(make_pair(102, false));
  location.insert(make_pair(102, "Circuits/Bristol/aes_256.txt"));
  loaded.insert(make_pair(103, false));
  location.insert(make_pair(103, "Circuits/Bristol/Keccak_f.txt"));
  loaded.insert(make_pair(104, false));
  location.insert(make_pair(104, "Circuits/Bristol/sha256.txt"));
  loaded.insert(make_pair(105, false));
  location.insert(make_pair(105, "Circuits/Bristol/sha512.txt"));

  /* Now any user defined ones */

  /* PUT YOUR ONES HERE 
   *    - USE INTEGERS BIGGER THAN 65536 IN THE MAP OPERATIONS
   */
}

mutex mutex_GC_load;

void Base_Circuits::check(int num)
{
  /* Use mutex here just in case */
  mutex_GC_load.lock();
  if (loaded[num] == false)
    {
      loaded[num]= true;

      Circuit C;
      ifstream inpf(location[num]);
      if (inpf.fail())
        {
          throw file_error(location[num]);
        }
      inpf >> C;
      inpf.close();
      Circuits.insert(make_pair(num, C));
    }
  mutex_GC_load.unlock();
}
