/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom. 
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

This code is based on code contributed by Marcel Keller of Data61

All rights reserved
*/

#ifndef _CPU_SUPPORT_H_
#define _CPU_SUPPORT_H_

#include "Exceptions/Exceptions.h"

/* Make sure we only call these things once */
static bool cpu_initialized= false;

static bool adx_flag, bmi2_flag, avx2_flag, avx_flag, pclmul_flag, aes_flag;

inline bool check_cpu(int func, bool ecx, int feature)
{
  int ax= func, bx, cx= 0, dx;
  __asm__ __volatile__("cpuid"
                       : "+a"(ax), "=b"(bx), "+c"(cx), "=d"(dx));
  return ((ecx ? cx : bx) >> feature) & 1;
}

inline void initialize_flags()
{
  cpu_initialized= true;
  adx_flag= check_cpu(7, false, 19);
  bmi2_flag= check_cpu(7, false, 8);
  avx2_flag= check_cpu(7, false, 5);
  avx_flag= check_cpu(1, true, 28);
  pclmul_flag= check_cpu(1, true, 1);
  aes_flag= check_cpu(1, true, 25);
}

inline bool cpu_has_adx()
{
  if (!cpu_initialized)
    {
      initialize_flags();
    }
  return adx_flag;
}

inline bool cpu_has_bmi2()
{
  if (!cpu_initialized)
    {
      initialize_flags();
    }
  return bmi2_flag;
}

inline bool cpu_has_avx2()
{
  if (!cpu_initialized)
    {
      initialize_flags();
    }
  return avx2_flag;
}

inline bool cpu_has_avx()
{
  if (!cpu_initialized)
    {
      initialize_flags();
    }
  return avx_flag;
}

inline bool cpu_has_pclmul()
{
  if (!cpu_initialized)
    {
      initialize_flags();
    }
  return pclmul_flag;
}

inline bool cpu_has_aes()
{
  if (!cpu_initialized)
    {
      initialize_flags();
    }
  return aes_flag;
}

#endif /* TOOLS_CPU_SUPPORT_H_ */
