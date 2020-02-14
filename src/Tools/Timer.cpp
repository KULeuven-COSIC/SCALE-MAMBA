/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Tools/Timer.h"
#include "Exceptions/Exceptions.h"

long long timeval_diff(struct timeval *start_time, struct timeval *end_time)
{
  struct timeval temp_diff;
  struct timeval *difference;
  difference= &temp_diff;
  difference->tv_sec= end_time->tv_sec - start_time->tv_sec;
  difference->tv_usec= end_time->tv_usec - start_time->tv_usec;
  while (difference->tv_usec < 0)
    {
      difference->tv_usec+= 1000000;
      difference->tv_sec-= 1;
    }
  return 1000000LL * difference->tv_sec + difference->tv_usec;
}

double timeval_diff_in_seconds(struct timeval *start_time, struct timeval *end_time)
{
  return double(timeval_diff(start_time, end_time)) / 1e6;
}

#ifndef __MACH__
long long timespec_diff(struct timespec *start_time, struct timespec *end_time)
{
  long long sec= end_time->tv_sec - start_time->tv_sec;
  long long nsec= end_time->tv_nsec - start_time->tv_nsec;
  while (nsec < 0)
    {
      nsec+= 1000000000;
      sec-= 1;
    }
  return 1000000000 * sec + nsec;
}
#endif

double convert_ns_to_seconds(long long x)
{
#ifdef __MACH__
  return double(x) / 1e6;
#else
  return double(x) / 1e9;
#endif
}

double Timer::elapsed()
{
  long long res= elapsed_time;
  if (running)
    res+= elapsed_since_last_start();
  return convert_ns_to_seconds(res);
}
