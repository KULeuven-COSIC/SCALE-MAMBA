/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2019, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

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
      OCD.sacrifice_mutex[thread].lock();
      wait= false;
      if (type == DATA_TRIPLE && SacrificeD[thread].TD.ta.size() < size)
        {
          wait= true;
        }
      if (type == DATA_SQUARE && SacrificeD[thread].SD.sa.size() < size)
        {
          wait= true;
        }
      if (type == DATA_BIT && SacrificeD[thread].BD.bb.size() < size)
        {
          wait= true;
        }
      if (type == DATA_INPUT_MASK && SacrificeD[thread].ID.ios[player].size() < size)
        {
          wait= true;
        }
      OCD.sacrifice_mutex[thread].unlock();
      if (wait)
        {
          sleep(1);
        }
    }
}
