/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _Processor_IO
#define _Processor_IO

#include "Online/Machine.h"

/* Class for handling private input and output from parties
 *   You cannot execute two consequtive start's for the
 *   *same* player without a corresponding stop (of the same length)
 *   But you can do concurrent starts for different players
 */

class Processor;

#include <deque>
using namespace std;

class Processor_IO
{
  vector<Share> rshares;

public:
  Processor_IO(unsigned int nplayers)
      : rshares(nplayers)
  {
    ;
  }

  // Get a private input values from a player on a given channel
  void private_input(unsigned int player, int target, unsigned int channel,
                     Processor &Proc, Player &P, Machine &machine, offline_control_data &OCD);

  // Output a private value to a player on a channel
  void private_output(unsigned int player, int source, unsigned int channel,
                      Processor &Proc, Player &P, Machine &machine,
                      offline_control_data &OCD);
};

#endif
