/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _aANDThread
#define _aANDThread

/* This defines the thread control for the thread
 * producing aANDs
 *
 */

#include <list>
#include <mutex>
using namespace std;

#include "OT/aAND.h"
#include "OT/aBit.h"
#include "Offline/offline_data.h"
#include "System/Player.h"

/* Holds the data and the mutex for it 
 *  - We hold multiple lists of aANDs.
 *    One per online thread
 */
class aAND_Data
{

public:
  /* The aBit queues */
  vector<list<aTriple>> aANDs;

  mutex aAD_mutex;

  // Gets a single share from queue q
  aTriple get_aAND(unsigned int q);

  // Gets many shares at once from queue q
  list<aTriple> get_aANDs(unsigned int q, unsigned int num);
};

void aAND_Thread(Player &P, unsigned int no_online_threads,
                 offline_control_data &OCD, int verbose);

#endif
