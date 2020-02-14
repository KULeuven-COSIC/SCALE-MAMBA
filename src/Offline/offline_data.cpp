/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

/*
 * offline_data.cpp
 *
 *  Created on: Mar 10, 2019
 *      Author: dragos
 */

#include "offline_data.h"
#include "Processor/Instruction.h"
#include <unistd.h>

extern vector<sacrificed_data> SacrificeD;

void Wait_For_Preproc(int type, unsigned int size, int thread,
                      offline_control_data &OCD, unsigned int player)
{
  // Check to see if we have to wait
  bool wait= true;
  while (wait)
    {
      wait= false;
      switch (type)
        {
          case DATA_TRIPLE:
            OCD.mul_mutex[thread].lock();
            if (SacrificeD[thread].TD.ta.size() < size)
              {
                wait= true;
              }
            OCD.mul_mutex[thread].unlock();
            break;
          case DATA_SQUARE:
            OCD.sqr_mutex[thread].lock();
            if (SacrificeD[thread].SD.sa.size() < size)
              {
                wait= true;
              }
            OCD.sqr_mutex[thread].unlock();
            break;
          case DATA_BIT:
            OCD.bit_mutex[thread].lock();
            if (SacrificeD[thread].BD.bb.size() < size)
              {
                wait= true;
              }
            OCD.bit_mutex[thread].unlock();
            break;
          case DATA_INPUT_MASK:
            OCD.OCD_mutex[thread].lock();
            if (SacrificeD[thread].ID.ios[player].size() < size)
              {
                wait= true;
              }
            OCD.OCD_mutex[thread].unlock();
            break;
        }
      if (wait)
        {
          sleep(1);
        }
    }
}
