/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _offline_data
#define _offline_data

#include <list>
#include <mutex>
using namespace std;

#include "LSSS/Share.h"

/* These datatypes are just the data store for the offline phase
 *  - We implement them as lists as we want to do constant
 *    time appending of lists etc
 */

class triples_data
{
public:
  /* Triple queues */
  list<Share> ta, tb, tc;
};

class squares_data
{
public:
  /* Square queues */
  list<Share> sa, sb;
};

class bits_data
{
public:
  /* Bits queue */
  list<Share> bb;
};

class IO_data
{
public:
  /* One list of shares per player */
  vector<list<Share>> ios;

  /* One value opened for that players list
   *   is[whoami].size() = openis.size()
   */
  list<gfp> opened_ios;

  void initialize(unsigned int n)
  {
    ios.resize(n);
  }
};

class sacrificed_data
{
public:
  triples_data TD;
  squares_data SD;
  bits_data BD;
  IO_data ID; // This is empty for all bar thread zero

  void initialize(unsigned int n)
  {
    ID.initialize(n);
  }
};

/* This holds the offline control data
 *   - Thread locking mutex's
 *   - Max number of data to produce in offline
 *   - Min number before we start online
 *   - Amount of data we have produced so far
 *   - Whether the offline phase SHOULD now gracefully die (finish_offline)
 *   - Counts how many offline threads HAVE died (finished_offline)
 */
class offline_control_data
{
  void clean_up()
  {
    if (mult_mutex != NULL)
      {
        delete[] mult_mutex;
        delete[] square_mutex;
        delete[] bit_mutex;
        delete[] sacrifice_mutex;
        mult_mutex= NULL;
      }
  }

public:
  mutex *mult_mutex;      // Lock for mult threads
  mutex *square_mutex;    // Lock for square threads
  mutex *bit_mutex;       // Lock for bit threads
  mutex *sacrifice_mutex; // Lock for sacrifice threads

  // Min number produced before we start online
  unsigned int minm, mins, minb;
  // Max number to ever produce
  unsigned int maxm, maxs, maxb, maxI;
  // Actual numbers computed by sacrifice phase so far
  vector<unsigned int> totm, tots, totb;
  unsigned int totI; // Not an array as only associated with thread zero

  vector<int> finish_offline;   // Flag to say whether we SHOULD finish offline
  vector<int> finished_offline; // Counts how many threads HAVE died

  void resize(unsigned int num_threads)
  {
    clean_up();
    mult_mutex= new mutex[num_threads];
    square_mutex= new mutex[num_threads];
    bit_mutex= new mutex[num_threads];
    sacrifice_mutex= new mutex[num_threads];
    totm.resize(num_threads);
    tots.resize(num_threads);
    totb.resize(num_threads);
    finish_offline.resize(num_threads);
    finished_offline.resize(num_threads);
    for (unsigned int i= 0; i < num_threads; i++)
      {
        totm[i]= 0;
        tots[i]= 0;
        totb[i]= 0;
        finish_offline[i]= 0;
        finished_offline[i]= 0;
      }
    totI= 0;
  }

  offline_control_data()
  {
    mult_mutex= NULL;
  }
  ~offline_control_data()
  {
    clean_up();
  }
};

#endif
