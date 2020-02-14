/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _Machine
#define _Machine

#include "Input_Output/IO.h"
#include "LSSS/Share.h"
#include "Math/Integer.h"
#include "OT/aBitVector.h"
#include "Processor/Memory.h"
#include "Processor/Program.h"
#include "Schedule.h"
#include "Tools/Timer.h"
#include "config.h"
#include <memory>

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

  // Initialize Online Thread Info
  void Init_OTI(unsigned int nthreads);

  // Timing information (there are N_TIMERS timers)
  vector<Timer> timers;

public:
  // The vector of programs used by the schedule file
  vector<Program> progs;

  int get_OTI_arg(unsigned int i)
  {
    return OTI[i].arg;
  }

  unsigned int max_n_threads() const
  {
    return OTI.size();
  }

  // This defines the precise IO functionality used in this program
  unique_ptr<Input_Output_Base> io;

  Input_Output_Base &get_IO()
  {
    return *io;
  }

  void Setup_IO(unique_ptr<Input_Output_Base> io_ptr)
  {
    io= move(io_ptr);
  }

  // The memory
  Memory<gfp> Mc;
  Memory<Share> Ms;
  Memory<Integer> Mr;
  Memory<aBitVector> Msr;

  // The Schedule process we are running
  Schedule schedule;

  bool verbose;
  void set_verbose()
  {
    verbose= true;
  }

  // -----------------------------------------------------

  // Now the member functions

  Machine()
  {
    verbose= false;
  }

  // Set up/store data
  //   Msr memory is not dumped or loaded
  void SetUp_Memory(unsigned int whoami, const string &memtype);
  void Dump_Memory(unsigned int whoami);

  void Load_Schedule_Into_Memory();
  void SetUp_Threads(unsigned int nthreads);

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
