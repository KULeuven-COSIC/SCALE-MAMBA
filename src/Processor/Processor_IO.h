/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _Processor_IO
#define _Processor_IO

/* Class for handling private input and output from parties */
#include "Offline/offline_data.h"
#include "System/Player.h"

template<class SRegint, class SBit>
class Processor;
template<class SRegint, class SBit>
class Machine;

#include <deque>
using namespace std;

template<class SRegint, class SBit>
class Processor_IO
{
  vector<Share> rshares;
  vector<gfp> epsilon;

  void Common_Subroutine_I(unsigned int player, unsigned int channel,
                           unsigned int number,
                           Processor<SRegint, SBit> &Proc, Player &P,
                           Machine<SRegint, SBit> &machine,
                           offline_control_data &OCD);

  void Common_Subroutine_O(unsigned int player, unsigned int number,
                           Processor<SRegint, SBit> &Proc, Player &P,
                           offline_control_data &OCD);

public:
  Processor_IO(unsigned int nplayers)
      : rshares(nplayers)
  {
    ;
  }

  // Get a private input values from a player on a given channel
  void private_input(unsigned int player, int target, unsigned int channel,
                     Processor<SRegint, SBit> &Proc, Player &P,
                     Machine<SRegint, SBit> &machine, offline_control_data &OCD,
                     unsigned int vectorize= 1);

  // Output a private value to a player on a channel
  void private_output(unsigned int player, int source, unsigned int channel,
                      Processor<SRegint, SBit> &Proc, Player &P,
                      Machine<SRegint, SBit> &machine,
                      offline_control_data &OCD,
                      unsigned int vectorize= 1);

  // Get n private input values from a player on a given channel and
  // assign them to memory location given by register in target
  void mprivate_input(unsigned int player, int target, unsigned int channel,
                      unsigned int n,
                      Processor<SRegint, SBit> &Proc, Player &P,
                      Machine<SRegint, SBit> &machine, offline_control_data &OCD,
                      unsigned int vectorize= 1);

  // Output n private value to a player on a channel where values
  // come from memory location given by register in source
  void mprivate_output(unsigned int player, int source, unsigned int channel,
                       unsigned int n,
                       Processor<SRegint, SBit> &Proc, Player &P,
                       Machine<SRegint, SBit> &machine,
                       offline_control_data &OCD,
                       unsigned int vectorize= 1);
};

#endif
