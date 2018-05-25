/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include <fstream>
using namespace std;

#include "Machine.h"

void Machine::SetUp_Memory(unsigned int whoami, const string &memtype)
{
  char filename[2048];

  Mc.set_default(gfp(0));
  Ms.set_default(Share(whoami));
  Mi.set_default(Integer(0));

  // Initialize the global memory
  if (memtype.compare("old") == 0)
    {
      sprintf(filename, "Data/Memory-P%d", whoami);
      ifstream inpf(filename, ios::in | ios::binary);
      if (inpf.fail())
        {
          throw file_error(filename);
        }
      // See below for ordering here
      inpf >> Mc >> Mi >> Ms;
      inpf.close();
    }
  else if (!(memtype.compare("empty") == 0))
    {
      cerr << "Invalid memory argument" << endl;
      exit(1);
    }
}

void Machine::Dump_Memory(unsigned int whoami)
{
  // Reduce memory size to speed up
  int max_size= 1 << 20;
  if (Mc.size() > max_size)
    Mc.resize(max_size);
  if (Ms.size() > max_size)
    Ms.resize(max_size);
  if (Mi.size() > max_size)
    Mi.resize(max_size);

  // Write out the memory to use next time
  char filename[2048];
  sprintf(filename, "Data/Memory-P%d", whoami);
  ofstream outf(filename, ios::out | ios::binary);
  // We do it in this order as this is needed by Script/test_result.py
  // and we do not want that Script to worry about sizes of Shares
  outf << Mc << Mi << Ms;
  outf.close();
}

unsigned int Machine::Load_Programs_Subroutine()
{
  char filename[2048];
  sprintf(filename, "%s/%s.sch", progname.c_str(), name.c_str());
  fprintf(stderr, "Opening file %s\n", filename);
  i_schedule.open(filename);
  if (i_schedule.fail())
    {
      throw file_error("Missing '" + string(filename) + "'. Did you compile '" +
                       name + "'?");
    }

  unsigned int nthreads;
  i_schedule >> nthreads;
  fprintf(stderr, "Number of online threads I will run in parallel =  %d\n",
          nthreads);

  int nprogs;
  i_schedule >> nprogs;
  fprintf(stderr, "Number of program sequences I need to load =  %d\n", nprogs);

  // Load in the programs
  progs.resize(nprogs);
  char threadname[1024];
  for (int i= 0; i < nprogs; i++)
    {
      i_schedule >> threadname;
      sprintf(filename, "%s/%s.bc", progname.c_str(), threadname);
      fprintf(stderr, "Loading program %d from %s\n", i, filename);
      ifstream pinp(filename);
      if (pinp.fail())
        {
          throw file_error(filename);
        }
      progs[i].parse(pinp);
      pinp.close();
      Mc.minimum_size(progs[i].direct_mem(MODP)[CLEAR], threadname);
      Ms.minimum_size(progs[i].direct_mem(MODP)[SECRET], threadname);
      Mi.minimum_size(progs[i].direct_mem(INT)[CLEAR], threadname);
    }

  return nthreads;
}

void Machine::Init_OTI(unsigned int nthreads)
{
  for (unsigned int i= 0; i < nthreads; i++)
    {
      pthread_mutex_init(&online_mutex[i], NULL);
      pthread_cond_init(&machine_ready[i], NULL);
      pthread_cond_init(&online_thread_ready[i], NULL);
      OTI[i].prognum= -2; // Dont do anything until we are ready
      OTI[i].finished= true;
      OTI[i].ready= false;
      OTI[i].arg= 0;
      // lock online thread for synchronization
      pthread_mutex_lock(&online_mutex[i]);
    }
}

unsigned int Machine::Load_Programs()
{
  i_schedule.close();

  unsigned int nthreads= Load_Programs_Subroutine();
  if (OTI.size() < nthreads)
    {
      throw Processor_Error("Restart requires more threads, cannot do this");
    }
  return nthreads;
}

unsigned int Machine::Load_Programs(const string &pname)
{
  progname= pname;
  /* First find progname itself, minus the directory path */
  int pos= progname.size() - 1, tot= progname.size() - 1;
  if (progname.c_str()[pos] == '/')
    {
      pos--;
      tot--;
    }
  while (progname.c_str()[pos] != '/' && pos >= 0)
    {
      pos--;
    }
  name= progname.substr(pos + 1, tot - pos);

  unsigned int nthreads= Load_Programs_Subroutine();

  /* Set up the thread data */
  OTI.resize(nthreads);
  online_mutex.resize(nthreads);
  online_thread_ready.resize(nthreads);
  machine_ready.resize(nthreads);

  Init_OTI(nthreads);

  return nthreads;
}

void Machine::Synchronize()
{
  for (unsigned int i= 0; i < OTI.size(); i++)
    {
      while (!OTI[i].ready)
        {
          fprintf(stderr, "Waiting for thread %d to be ready\n", i);
          pthread_cond_wait(&online_thread_ready[i], &online_mutex[i]);
        }
      pthread_mutex_unlock(&online_mutex[i]);
    }
}

void Machine::Signal_Ready(unsigned int num, bool flag)
{
  pthread_mutex_lock(&online_mutex[num]);
  OTI[num].ready= flag;
  if (flag)
    {
      pthread_cond_signal(&online_thread_ready[num]);
    }
  else
    {
      OTI[num].prognum= -1;
      pthread_cond_signal(&machine_ready[num]);
    }
  pthread_mutex_unlock(&online_mutex[num]);
}

void Machine::Lock_Until_Ready(unsigned int num)
{
  pthread_mutex_lock(&online_mutex[num]);
  if (!OTI[num].ready)
    pthread_cond_wait(&machine_ready[num], &online_mutex[num]);
  pthread_mutex_unlock(&online_mutex[num]);
}

int Machine::Pause_While_Nothing_To_Do(unsigned int num)
{
  pthread_mutex_lock(&online_mutex[num]);
  if (OTI[num].prognum == -2)
    {
      pthread_cond_wait(&machine_ready[num], &online_mutex[num]);
    }
  int program= OTI[num].prognum;
  OTI[num].prognum= -2; // Reset it to run nothing next
  pthread_mutex_unlock(&online_mutex[num]);
  return program;
}

void Machine::Signal_Finished_Tape(unsigned int num)
{
  pthread_mutex_lock(&online_mutex[num]);
  OTI[num].finished= true;
  pthread_cond_signal(&online_thread_ready[num]);
  pthread_mutex_unlock(&online_mutex[num]);
}

void Machine::Lock_Until_Finished_Tape(unsigned int num)
{
  pthread_mutex_lock(&online_mutex[num]);
  if (!OTI[num].finished)
    pthread_cond_wait(&online_thread_ready[num], &online_mutex[num]);
  pthread_mutex_unlock(&online_mutex[num]);
}

// This runs tape tape_number on thread thread_number with argument arg
void Machine::run_tape(unsigned int thread_number, unsigned int tape_number,
                       int arg)
{
  if (thread_number >= OTI.size())
    {
      throw Processor_Error("invalid thread number: " + to_string(thread_number) +
                            "/" + to_string(OTI.size()));
    }
  if (tape_number >= progs.size())
    {
      throw Processor_Error("invalid tape number: " + to_string(tape_number) +
                            "/" + to_string(progs.size()));
    }

  pthread_mutex_lock(&online_mutex[thread_number]);
  OTI[thread_number].prognum= tape_number;
  OTI[thread_number].arg= arg;
  OTI[thread_number].finished= false;
  pthread_cond_signal(&machine_ready[thread_number]);
  pthread_mutex_unlock(&online_mutex[thread_number]);
}

void Machine::run()
{
  // Initialise the timers
  timers.resize(N_TIMERS);
  for (unsigned int i= 0; i < N_TIMERS; i++)
    {
      timers[i]= clock();
    }

  // First go through the schedule and execute what is there
  unsigned int tape_number, threads_on_line;
  bool flag= true;
  while (flag)
    {
      i_schedule >> threads_on_line;
      if (threads_on_line == 0)
        {
          flag= false;
        }
      else
        {
          for (unsigned int i= 0; i < threads_on_line; i++)
            { // Now load up data
              i_schedule >> tape_number;

              // Cope with passing an integer parameter to a tape
              int arg;
              if (i_schedule.get() == ':')
                i_schedule >> arg;
              else
                arg= 0;

              // cerr << "Run scheduled tape " << tn << " in thread " << i << endl;
              run_tape(i, tape_number, arg);
            }
          // Make sure all terminate before we continue
          for (unsigned int i= 0; i < threads_on_line; i++)
            {
              Lock_Until_Finished_Tape(i);
            }
        }
    }

  // Now we have finished the schedule file output what we have done

  // What program did we run?
  char compiler_command[1000];
  i_schedule.get();
  i_schedule.getline(compiler_command, 1000);
  if (compiler_command[0] != 0)
    {
      cerr << "Compiler: " << compiler_command << endl;
    }
  i_schedule.close();

  // Tell all online threads to stop
  for (unsigned int i= 0; i < OTI.size(); i++)
    {
      Signal_Ready(i, false);
    }

  cerr << "Waiting for all clients to finish" << endl;
  // Wait until all clients have signed out
  for (unsigned int i= 0; i < OTI.size(); i++)
    {
      pthread_mutex_lock(&online_mutex[i]);
      OTI[i].ready= true;
      pthread_cond_signal(&machine_ready[i]);
      pthread_mutex_unlock(&online_mutex[i]);

      pthread_mutex_destroy(&online_mutex[i]);
      pthread_cond_destroy(&online_thread_ready[i]);
      pthread_cond_destroy(&machine_ready[i]);
    }
}

void Machine::start_timer(unsigned int i)
{
  if (i >= N_TIMERS)
    {
      throw invalid_length();
    }
  timers[i]= clock();
}

void Machine::stop_timer(unsigned int i)
{
  if (i >= N_TIMERS)
    {
      throw invalid_length();
    }
  double time= clock() - timers[i];
  time/= CLOCKS_PER_SEC;
  printf("Elapsed time on timer %d is %.6f seconds \n", i, time);
}
