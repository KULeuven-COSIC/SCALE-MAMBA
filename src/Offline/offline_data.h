/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _offline_data
#define _offline_data

#include <list>
#include <mutex>
using namespace std;

#include "FHE/ZKPoK.h"
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
  IO_data ID;

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
    if (OCD_mutex != NULL)
      {
        delete[] OCD_mutex;
        delete[] mul_mutex;
        delete[] sqr_mutex;
        delete[] bit_mutex;
        OCD_mutex= NULL;
      }
  }

public:
  mutex *OCD_mutex; // Lock for control OCD and input data
  mutex *mul_mutex; // Lock for mult triples
  mutex *sqr_mutex; // Lock for sqr pairs
  mutex *bit_mutex; // Lock for bits

  // Min number produced before we start online
  unsigned int minm, mins, minb;
  // Max number to ever produce
  unsigned int maxm, maxs, maxb, maxI;
  // Actual numbers computed by sacrifice phase so far
  vector<unsigned int> totm, tots, totb, totI;

  vector<int> finish_offline;   // Flag to say whether we SHOULD finish offline
  vector<int> finished_online;  // Flag to say whether online has finished
  vector<int> finished_offline; // Counts how many threads HAVE died

  // The ZKPoKs needed for IO production when doing Full Threshold
  vector<vector<ZKPoK>> IO_ZKPoKs;

  void resize(unsigned int num_threads,
              unsigned int nplayers,
              unsigned int whoami)
  {
    clean_up();
    OCD_mutex= new mutex[num_threads];
    mul_mutex= new mutex[num_threads];
    sqr_mutex= new mutex[num_threads];
    bit_mutex= new mutex[num_threads];
    totm.resize(num_threads);
    tots.resize(num_threads);
    totb.resize(num_threads);
    totI.resize(num_threads);
    finish_offline.resize(num_threads);
    finished_online.resize(num_threads);
    finished_offline.resize(num_threads);
    for (unsigned int i= 0; i < num_threads; i++)
      {
        totm[i]= 0;
        tots[i]= 0;
        totb[i]= 0;
        totI[i]= 0;
        finish_offline[i]= 0;
        finished_offline[i]= 0;
        finished_online[i]= 0;
      }
    if (Share::SD.type == Full)
      {
        IO_ZKPoKs.resize(num_threads);
        for (unsigned int i= 0; i < num_threads; i++)
          {
            IO_ZKPoKs[i].resize(nplayers);
            for (unsigned int j= 0; j < nplayers; j++)
              {
                if (j == whoami)
                  {
                    IO_ZKPoKs[i][j].set_params(true, true);
                  }
                else
                  {
                    IO_ZKPoKs[i][j].set_params(true, false);
                  }
              }
          }
      }
  }

  unsigned int num_online_threads() const
  {
    return totm.size();
  }

  offline_control_data()
  {
    OCD_mutex= NULL;
  }
  ~offline_control_data()
  {
    clean_up();
  }
};

/* The following structure connects the data access instructions from
 * Instructions.h to have a global function which waits for their
 * production.
 * Since there is no INPUT MASK retrieval in the Instructions.h
 * we assigned a random number (say 0x53) to be continous
 */

enum {
  DATA_TRIPLE= 0x50,
  DATA_BIT= 0x51,
  DATA_SQUARE= 0x52,
  DATA_INPUT_MASK= 0x53
};

// Does thread locking until data type available.
void Wait_For_Preproc(int type, unsigned int size, int thread,
                      offline_control_data &OCD, unsigned int player= 0);
#endif
