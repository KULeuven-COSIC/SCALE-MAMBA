/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _Mod2Thread
#define _Mod2Thread

/* This defines the thread control for the thread
 * producing triples Mod2 when we are not using HSS (i.e.
 * OT/GC based) methods
 *
*/

#include <list>
#include <mutex>
using namespace std;

#include "LSSS/Open_Protocol2.h"
#include "LSSS/PRSS.h"
#include "Offline/offline_data.h"
#include "System/Player.h"

/* Holds the data and the mutex for it 
 *  - We hold multiple lists of triples
 *    One per online thread 
 */
class Mod2_Thread_Data
{

  unsigned int B, C, N, X, L;

  vector<vector<Share2>> T_Store;
  vector<int> T_cnt;

public:
  vector<list<vector<Share2>>> triples;
  bool disabled;

  void init(unsigned int no_online_threads, bool disable= false)
  {
    disabled= disable;
    L= -1;
    triples.resize(no_online_threads);
    T_Store.resize(no_online_threads, vector<Share2>(3));
    T_cnt.resize(no_online_threads);
    for (unsigned int i= 0; i < no_online_threads; i++)
      {
        T_cnt[i]= 64;
      }
  }

  mutex MD2_mutex;

  // Tune's the implementation (by picking the best L)
  //  - At same time generates loads of triples for
  //    online thread 0
  //  - Assumes thread is locked
  void Tune(Player &P, PRSS2 &prss, int verbose= 0);

  // Updates queue q with another batch
  //  - Assumes thread not locked on entry
  void Update_Queue(unsigned int q, Player &P, PRSS2 &prss);

  // Gets a single triple from queue q
  //   Note this is 64 triples really
  void get_Triple(vector<Share2> &T, unsigned int q);

  // This really only gets a single triple
  void get_Single_Triple(vector<Share2> &T, unsigned int q);

  // Get many at once from queue q
  //   Note this is 64*num triples really
  list<vector<Share2>> get_Triples(unsigned int q, unsigned int num);

  // This gets triples.size() triples, one per slot. So exactly num triples
  void get_Single_Triples(vector<vector<Share2>> &T, unsigned int q);

  void check() const
  {
    if (disabled)
      {
        throw OT_disabled();
      }
  }
};

void Mod2_Triple_Thread(Player &P, unsigned int no_online_threads,
                        offline_control_data &OCD, int verbose);

#endif
