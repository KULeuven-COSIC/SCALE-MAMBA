/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

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
  vector<vector<Share>> rshares;
  vector<gfp> i_epsilon; // i_epsilon_i = x_i-r_i   (player p knows r_i and has
                         // input x_i)
  deque<gfp> o_epsilon;  // o_epsilon_i = r_i       (player p knows r_i)

public:
  Processor_IO(unsigned int nplayers)
      : rshares(nplayers)
  {
    ;
  }

  // Prepare to get n_inputs private input values from player player
  void start_input(unsigned int player, unsigned int n_inputs, unsigned int channel,
                   Processor &Proc, Player &P, Machine &machine, offline_control_data &OCD);

  // Assign the input shares to registers given by targets
  void stop_input(unsigned int player, vector<int> targets, Processor &Proc,
                  Player &P);

  // Prepare to get n_inputs private input values from player player
  void start_output(unsigned int player, int target, int source,
                    Processor &Proc, Player &P, offline_control_data &OCD);

  // Assign the input shares to registers given by targets
  void stop_output(unsigned int player, int source, unsigned int channel,
                   Processor &Proc, Player &P, Machine &machine);
};

#endif
