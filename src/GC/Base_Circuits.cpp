/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2019, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Base_Circuits.h"
#include <fstream>
#include <mutex>

#include "OT/aBitVector.h"
#include "config.h"

/* This it the global store of circuits */
Base_Circuits Global_Circuit_Store;

Circuit Base_Circuits::load_circuit(const string &path)
{
  ifstream inpf;
  Circuit C;

  inpf.open(path);
  if (inpf.fail())
    throw file_error(path);
  inpf >> C;
  inpf.close();

  return C;
}

void Base_Circuits::initialize(const bigint &p)
{

  Circuits.insert(make_pair(
    0, load_circuit("Circuits/Bristol/adder64.txt")
  ));

  Circuits.insert(make_pair(
    1, load_circuit("Circuits/Bristol/sub64.txt")
  ));

  Circuits.insert(make_pair(
    2, load_circuit("Circuits/Bristol/mult2_64.txt")
  ));

  Circuits.insert(make_pair(
    3, load_circuit("Circuits/Bristol/mult64.txt")
  ));

  Circuits.insert(make_pair(
    4, load_circuit("Circuits/Bristol/divide64.txt")
  ));

  Circuits.insert(make_pair(
    5, load_circuit("Circuits/Bristol/neg64.txt")
  ));

  Circuits.insert(make_pair(
    6, load_circuit("Circuits/Bristol/zero_equal.txt")
  ));

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
      Circuits.insert(make_pair(
        7, load_circuit("Data/ConversionCircuit-LSSS_to_GC.txt")
      ));

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

      Circuits.insert(make_pair(
        num, load_circuit(location[num])
      ));
    }
  mutex_GC_load.unlock();
}
