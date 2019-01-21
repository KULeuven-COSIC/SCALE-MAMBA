/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
/* This file holds the main SCALE runtime in a
 * single function call, to allow programmatic
 * setup of SCALE outside of using Player.x
 */

#include "System/RunTime.h"
#include "OT/OT_Thread.h"
#include "Offline/FHE_Factory.h"
#include "Offline/offline_phases.h"
#include "Online/Online.h"
#include "System/Networking.h"
#include "Tools/Timer.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <pthread.h>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>
using namespace std;

int USE_OT_THREAD;

class thread_info
{
public:
  int thread_num;
  const SystemData *SD;
  offline_control_data *OCD;
  SSL_CTX *ctx;
  int me;
  vector<vector<int>> csockets;
  vector<gfp> MacK;

  // FHE Data
  const FHE_PK *pk;
  const FHE_SK *sk;
  const FFT_Data *PTD;

  int verbose;

  Machine *machine;       // Pointer to the machine
  FHE_Industry *industry; // Pointer to the FHE set of factories
};

// We have 5 threads per online phase
//   - Online
//   - Sacrifice (and input tuple production)
//   - Mult Triple Production
//   - Square Pair Production
//   - Bit Production
vector<pthread_t> threads;

Timer global_time;

// Forward declarations to make code easier to read
void *Main_Func(void *ptr);

vector<triples_data> TriplesD;
vector<squares_data> SquaresD;
vector<bits_data> BitsD;
vector<sacrificed_data> SacrificeD;

/* Before calling this we assume various things have
 * been set up. In particular the following functions have
 * been called
 *
 *    Init_SSL_CTX
 *    gfp::init_field
 *    Share::init_share_data
 *    machine.SetUp_Memory
 *    machine.IO.init
 *    FHE data has been initialized if needed
 *
 * We also assume the machine.schedule has been initialised
 * with some stingstream tapes and a stringstream to a schedule file
 *
 * This function assumes that afterwards we sort out 
 * closing down SSL and Dump'ing memory if need be
 * for a future application
 *
 */
void Run_Scale(unsigned int my_number, unsigned int no_online_threads,
               const FFT_Data &PTD, const FHE_PK &pk, const FHE_SK &sk, const vector<gfp> &MacK,
               SSL_CTX *ctx, const vector<unsigned int> &portnum,
               const SystemData &SD,
               Machine &machine, offline_control_data &OCD,
               unsigned int number_FHE_threads, int verbose)
{
  machine.Load_Schedule_Into_Memory();
  machine.SetUp_Threads(no_online_threads);

  OCD.resize(no_online_threads);

  TriplesD.resize(no_online_threads);
  SquaresD.resize(no_online_threads);
  BitsD.resize(no_online_threads);
  SacrificeD.resize(no_online_threads);
  for (unsigned int i= 0; i < no_online_threads; i++)
    {
      SacrificeD[i].initialize(SD.n);
    }

  unsigned int nthreads= 5 * no_online_threads;
  // Add in the FHE threads
  unsigned int tnthreads= nthreads + number_FHE_threads;
  // Add in the OT thread
  if (USE_OT_THREAD)
    {
      tnthreads++;
    }

  /* Initialize the networking TCP sockets */
  int ssocket;
  vector<vector<vector<int>>> csockets(tnthreads, vector<vector<int>>(SD.n, vector<int>(2)));
  Get_Connections(ssocket, csockets, portnum, my_number, SD, verbose - 2);
  printf("All connections now done\n");

  FHE_Industry industry(number_FHE_threads);

  global_time.start();

  printf("Setting up threads\n");
  fflush(stdout);
  threads.resize(tnthreads);
  vector<thread_info> tinfo(tnthreads);
  for (unsigned int i= 0; i < tnthreads; i++)
    {
      if (i < nthreads)
        {
          tinfo[i].thread_num= i;
        }
      else if (i < tnthreads + number_FHE_threads - 1)
        {
          tinfo[i].thread_num= 10000 + i - nthreads;
        }
      else
        {
          tinfo[i].thread_num= 20000;
        }
      tinfo[i].SD= &SD;
      tinfo[i].OCD= &OCD;
      tinfo[i].ctx= ctx;
      tinfo[i].me= my_number;
      tinfo[i].csockets= csockets[i];
      tinfo[i].MacK= MacK;
      tinfo[i].machine= &machine;
      tinfo[i].industry= &industry;
      tinfo[i].pk= &pk;
      tinfo[i].sk= &sk;
      tinfo[i].PTD= &PTD;
      tinfo[i].verbose= verbose;
      if (pthread_create(&threads[i], NULL, Main_Func, &tinfo[i]))
        {
          throw C_problem("Problem spawning thread");
        }
    }

  // Get all online threads in sync
  machine.Synchronize();

  // Now run the programs
  machine.run();

  printf("Waiting for all clients to finish\n");
  fflush(stdout);
  for (unsigned int i= 0; i < tnthreads; i++)
    {
      pthread_join(threads[i], NULL);
    }

  Close_Connections(ssocket, csockets, my_number);
}

void *Main_Func(void *ptr)
{
  thread_info *tinfo= (thread_info *) ptr;
  unsigned int num= tinfo->thread_num;
  int me= tinfo->me;
  int verbose= tinfo->verbose;
  printf("I am player %d in thread %d\n", me, num);
  fflush(stdout);

  Player P(me, *(tinfo->SD), num, (tinfo->ctx), (tinfo->csockets), (tinfo->MacK), verbose - 1);
  printf("Set up player %d in thread %d \n", me, num);
  fflush(stdout);

  if (num < 10000)
    {
      int num5= num % 5;
      int num_online= (num - num5) / 5;
      switch (num5)
        {
          case 0:
            mult_phase(num_online, P, *(tinfo->OCD), *(tinfo->pk), *(tinfo->sk),
                       *(tinfo->PTD), *(tinfo)->industry, verbose);
            break;
          case 1:
            square_phase(num_online, P, *(tinfo->OCD), *(tinfo->pk), *(tinfo->sk),
                         *(tinfo->PTD), *(tinfo)->industry, verbose);
            break;
          case 2:
            bit_phase(num_online, P, *(tinfo->OCD), *(tinfo->pk), *(tinfo->sk),
                      *(tinfo->PTD), *(tinfo)->industry, verbose);
            break;
          case 3:
            sacrifice_phase(num_online, P, (tinfo->SD)->fake_sacrifice, *(tinfo->OCD),
                            *(tinfo->pk), *(tinfo->sk), *(tinfo->PTD), *(tinfo)->industry, verbose);
            break;
          case 4:
            online_phase(num_online, P, *(tinfo->OCD), *(tinfo)->machine);
            break;
          default:
            throw bad_value();
            break;
        }
    }
  else if (num < 20000)
    {
      (*(tinfo)->industry).FHE_Factory(P, *(tinfo->OCD), *(tinfo->pk), *(tinfo->PTD), num - 10000, verbose);
    }
  else if (USE_OT_THREAD == 1)
    { // Should remove the if on this else eventually XXXX
      OT_Thread(P, 2);
    }
  return 0;
}
