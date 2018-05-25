/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _Machine
#define _Machine

#include "Input_Output/IO.h"
#include "LSSS/Share.h"
#include "Math/Integer.h"
#include "Processor/Memory.h"
#include "Processor/Program.h"
#include "config.h"
#include <time.h>

/* The machine controls the entire evaluation engine.
 * The evaluation engine is made up of many processors (think
 * of cores). There is one processor per online thread.
 *
 * What the machine class does is coordinate these threads
 * via passing data via online_thread_info
 *
 */

class online_thread_info
{
public:
  int prognum;   // Currently program this thread should run
                 //    -1 end of run on this thread
                 //    -2 no instruction to run anything
  bool finished; // Signals if threads current program has ended
  bool ready;    // The condition signal for the online-mutex's below

  // Integer argument to the program (optional)
  int arg;
};

#define N_TIMERS 100

class Machine
{
  /* This is the thread control data
   *  - XXXX Should really be moved to C++ style thread control
   *         at some point (but then you cannot have vectors)
   */

  vector<pthread_mutex_t> online_mutex;
  vector<pthread_cond_t> online_thread_ready;
  vector<pthread_cond_t> machine_ready;

  vector<online_thread_info> OTI;

  // The ifstream attached to the schedule file
  ifstream i_schedule;

  // Store of the program name being executed
  //   - In long and short form
  string progname, name;

  // Open i_schedule file and load in the programs
  unsigned int Load_Programs_Subroutine();
  void Init_OTI(unsigned int nthreads);

  // Timing information (there are N_TIMERS timers)
  vector<clock_t> timers;

public:
  // The vector of programs used by the schedule file
  vector<Program> progs;

  int get_OTI_arg(unsigned int i)
  {
    return OTI[i].arg;
  }

  // This defines the precise IO functionality used in this program
  IO_FUNCTIONALITY IO;

  // The memory
  Memory<gfp> Mc;
  Memory<Share> Ms;
  Memory<Integer> Mi;

  // -----------------------------------------------------

  // Now the member functions

  // Set up/store data
  void SetUp_Memory(unsigned int whoami, const string &memtype);
  void Dump_Memory(unsigned int whoami);

  // Load progs and returns number of online threads needed
  unsigned int Load_Programs(const string &pname);
  // This is the same version, but keeps the progname
  //   - Used for RESTART
  //   - Abort if number of online threads needs to increase
  unsigned int Load_Programs();

  // Synchronize the machine with the online threads
  void Synchronize();

  // Signal that online thread is ready to the machine (true)
  // Or that online thread should stop from the machine (false)
  //   - In later case we also set prognum to -1 for this thread
  void Signal_Ready(unsigned int num, bool flag);
  // Corresponding lock
  //   Locks this thread until thread num signals ready
  void Lock_Until_Ready(unsigned int num);

  // Pause online thread until we have a program to run
  //   - Returns the program to run next on this thread
  int Pause_While_Nothing_To_Do(unsigned int num);

  // Signal online phase has finished this program tape
  void Signal_Finished_Tape(unsigned int num);
  // Corresponding lock
  //   Locks this thread until thread num has finished the tape
  void Lock_Until_Finished_Tape(unsigned int num);

  // This runs tape tape_number on thread thread_number with argument arg
  void run_tape(unsigned int thread_number, unsigned int tape_number, int arg);

  // Main run routine
  void run();

  void start_timer(unsigned int i);
  void stop_timer(unsigned int i);
};

#endif
