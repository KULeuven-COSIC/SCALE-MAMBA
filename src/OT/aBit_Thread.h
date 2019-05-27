/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2019, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _aBitThread
#define _aBitThread

/* This defines the thread control for the threads
 * producing aBits
 *
 * The aBit thread first executes a random OT with 
 * each player then it produces loads of authenticated bits.
 * Placing them a queue for each online thread
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
 *  - We hold multiple lists of aBits.
 *    One per online thread
 */
class aBit_Data
{

public:
  /* The aBit queues */
  vector<list<aBit>> aBits;

  mutex aBD_mutex;

  // Gets a single share from queue q
  aBit get_aShare(unsigned int q);

  // Gets many shares at once from queue q
  list<aBit> get_aShares(unsigned int q, unsigned int num);
};

void aBit_Thread(Player &P, unsigned int no_online_threads,
                 offline_control_data &OCD, int verbose);

#endif
