/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _timer
#define _timer

#include <sys/time.h>
#include <sys/wait.h> /* Wait for Process Termination */
#include <time.h>

#include "Exceptions/Exceptions.h"

long long timeval_diff(struct timeval *start_time, struct timeval *end_time);
double timeval_diff_in_seconds(struct timeval *start_time, struct timeval *end_time);

// no clock_gettime() on OS X
#ifdef __MACH__
#define timespec timeval
#define clockid_t int
#define CLOCK_MONOTONIC 0
#define CLOCK_PROCESS_CPUTIME_ID 0
#define CLOCK_THREAD_CPUTIME_ID 0
#define timespec_diff timeval_diff
#define clock_gettime(x, y) gettimeofday(y, 0)
#else
long long timespec_diff(struct timespec *start_time, struct timespec *end_time);
#endif

#define BENCH_TEXT_BOLD "\x1B[1m"
#define BENCH_COLOR_RED "\x1B[31m"
#define BENCH_COLOR_GREEN "\x1B[32m"
#define BENCH_COLOR_BLUE "\x1B[34m"
#define BENCH_ATTR_RESET "\x1B[0m"

#define BENCH_MAGIC_START "<b3m4>\n"
#define BENCH_MAGIC_END "</b3m4>\n"

class Timer
{
  timespec startv;
  bool running;
  long long elapsed_time;
  clockid_t clock_id;

  long long elapsed_since_last_start()
  {
    timespec endv;
    clock_gettime(clock_id, &endv);
    return timespec_diff(&startv, &endv);
  }

public:
  Timer(clockid_t clock_id= CLOCK_MONOTONIC)
      : running(false), elapsed_time(0), clock_id(clock_id)
  {
    clock_gettime(clock_id, &startv);
  }

  Timer &start()
  {
    if (running)
      {
        throw Timer_Error("Timer already running.");
      }
    // clock() is not suitable in threaded programs so time using something else
    clock_gettime(clock_id, &startv);
    running= true;
    return *this;
  }

  void stop()
  {
    if (!running)
      {
        throw Timer_Error("Time not running.");
      }
    elapsed_time+= elapsed_since_last_start();

    running= false;
    clock_gettime(clock_id, &startv);
  }

  void reset()
  {
    elapsed_time= 0;
    clock_gettime(clock_id, &startv);
  }

  double elapsed();
};

#endif
