/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

/*
 * This class is used to generate the 
 * Cut and Choose parameters just once. Then
 * it is used start new instances if daBit generators
 * with the same parameters but (if wanted) with different
 * threads.
 */

#ifndef SRC_OFFLINE_DABITMACHINE_H_
#define SRC_OFFLINE_DABITMACHINE_H_

#include "Math/gf2n.h"
#include "Math/gfp.h"
#include "Offline/DABitGenerator.h"
#include "Offline/offline_data.h"
#include "System/Player.h"
#include "Tools/Timer.h"

/* This is used as a class, maybe in the future we can
 * create different types of daBits, each with its own
 * cnc parameters.
 */

class DABitMachineBase
{
protected:
  Timer timer;
  map<pair<int, int>, pair<int, int>> pre_cnc_params;

public:
  int nBitsPerLoop;
  int sec;
  int cnc_param;
  int bucket_size;

  DABitMachineBase();
};

/* Implemented using Cut and Choose method from
 * eprint.iacr.org/2019/207
 */
class MaliciousDABitMachine : public DABitMachineBase
{

public:
  unsigned int n_parties;
  offline_control_data *OCD;
  unsigned int gamma;
  bigint Delta;

  MaliciousDABitMachine();
  void Initialize(unsigned int n_parties, offline_control_data &OCD);
  void find_cnc_params();
  AbstractDABitGenerator *new_generator(Player &P, int thread_num);
};

#endif /* SRC_OFFLINE_DABITMACHINE_H_ */
