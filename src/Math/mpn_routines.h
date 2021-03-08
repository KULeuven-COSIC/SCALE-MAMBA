/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

This code is from Marcel Keller at Data61

All rights reserved
*/

#ifndef MATH_MPN_ROUTINES
#define MATH_MPN_ROUTINES

#include <assert.h>
#include <mpir.h>
#include <x86intrin.h>

#include "Tools/cpu-support.h"

inline void inline_mpn_zero(mp_limb_t *x, mp_size_t size)
{
  memset(x, 0, size * sizeof(mp_limb_t));
}

inline void inline_mpn_copyi(mp_limb_t *dest, const mp_limb_t *src, mp_size_t size)
{
  memcpy(dest, src, size * sizeof(mp_limb_t));
}

inline void debug_print(const char *name, const mp_limb_t *x, int n)
{
  (void) name, (void) x, (void) n;
#ifdef DEBUG_MPN
  cout << name << " ";
  for (int i= 0; i < n; i++)
    cout << hex << x[n - i - 1] << " ";
  cout << endl;
#endif
}

template<int N>
mp_limb_t mpn_add_fixed_n_with_carry(mp_limb_t *res, const mp_limb_t *x, const mp_limb_t *y);

template<int N>
inline void mpn_add_fixed_n(mp_limb_t *res, const mp_limb_t *x, const mp_limb_t *y)
{
  mpn_add_fixed_n_with_carry<N>(res, x, y);
}

template<>
inline void mpn_add_fixed_n<1>(mp_limb_t *res, const mp_limb_t *x, const mp_limb_t *y)
{
  *res= *x + *y;
}

template<>
inline void mpn_add_fixed_n<2>(mp_limb_t *res, const mp_limb_t *x, const mp_limb_t *y)
{
  memcpy(res, y, 2 * sizeof(mp_limb_t));
  debug_print("x", x, 2);
  debug_print("y", y, 2);
  debug_print("res", res, 2);
  __asm__(
      "add %2, %0 \n"
      "adc %3, %1 \n"
      : "+&r"(res[0]), "+r"(res[1])
      : "rm"(x[0]), "rm"(x[1])
      : "cc");
  debug_print("res", res, 2);
}

template<>
inline void mpn_add_fixed_n<3>(mp_limb_t *res, const mp_limb_t *x, const mp_limb_t *y)
{
  memcpy(res, y, 3 * sizeof(mp_limb_t));
  debug_print("x", x, 3);
  debug_print("y", y, 3);
  debug_print("res", res, 3);
  __asm__(
      "add %3, %0 \n"
      "adc %4, %1 \n"
      "adc %5, %2 \n"
      : "+&r"(res[0]), "+&r"(res[1]), "+r"(res[2])
      : "rm"(x[0]), "rm"(x[1]), "rm"(x[2])
      : "cc");
  debug_print("res", res, 3);
}

template<>
inline void mpn_add_fixed_n<4>(mp_limb_t *res, const mp_limb_t *x, const mp_limb_t *y)
{
  memcpy(res, y, 4 * sizeof(mp_limb_t));
  __asm__(
      "add %4, %0 \n"
      "adc %5, %1 \n"
      "adc %6, %2 \n"
      "adc %7, %3 \n"
      : "+&r"(res[0]), "+&r"(res[1]), "+&r"(res[2]), "+r"(res[3])
      : "rm"(x[0]), "rm"(x[1]), "rm"(x[2]), "rm"(x[3])
      : "cc");
}

inline mp_limb_t mpn_add_n_with_carry(mp_limb_t *res, const mp_limb_t *x, const mp_limb_t *y, int n)
{
  // This is complicated because we want to use adc(x) whenever possible.
  // clang always offers this but GCC only with ADX enabled.
#if defined(__ADX__) || defined(__clang__)
  if (cpu_has_adx())
    {
      char carry= 0;
      for (int i= 0; i < n; i++)
#if defined(__clang__)
#if __has_builtin(__builtin_ia32_addcarryx_u64) && defined(__ADX__)
        carry= __builtin_ia32_addcarryx_u64(carry, x[i], y[i], (unsigned long long *) &res[i]);
#else
        carry= __builtin_ia32_addcarry_u64(carry, x[i], y[i], (unsigned long long *) &res[i]);
#endif
#else
        carry= _addcarryx_u64(carry, x[i], y[i], (unsigned long long *) &res[i]);
#endif
      return carry;
    }
  else
#endif
      if (n > 0)
    return mpn_add_n(res, x, y, n);
  else
    return 0;
}

template<int N>
mp_limb_t mpn_add_fixed_n_with_carry(mp_limb_t *res, const mp_limb_t *x, const mp_limb_t *y)
{
  return mpn_add_n_with_carry(res, x, y, N);
}

inline mp_limb_t mpn_sub_n_borrow(mp_limb_t *res, const mp_limb_t *x, const mp_limb_t *y, int n)
{
#ifndef __clang__
#if __GNUC__ < 7
  // GCC 6 can't handle the code below
  return mpn_sub_n(res, x, y, n);
#endif
#endif
  char borrow= 0;
  for (int i= 0; i < n; i++)
    borrow= _subborrow_u64(borrow, x[i], y[i], (unsigned long long *) &res[i]);
  return borrow;
}

template<int N>
inline void mpn_sub_fixed_n(mp_limb_t *res, const mp_limb_t *x, const mp_limb_t *y)
{
  mpn_sub_n_borrow(res, x, y, N);
}

template<int N>
inline mp_limb_t mpn_sub_fixed_n_borrow(mp_limb_t *res, const mp_limb_t *x, const mp_limb_t *y)
{
  return mpn_sub_n_borrow(res, x, y, N);
}

template<>
inline void mpn_sub_fixed_n<1>(mp_limb_t *res, const mp_limb_t *x, const mp_limb_t *y)
{
  *res= *x - *y;
}

template<>
inline mp_limb_t mpn_sub_fixed_n_borrow<1>(mp_limb_t *res, const mp_limb_t *x, const mp_limb_t *y)
{
  memcpy(res, x, 1 * sizeof(mp_limb_t));
  mp_limb_t borrow= 0;
  __asm__(
      "sub %2, %0 \n"
      "sbb $0, %1 \n"
      : "+r"(res[0]), "+r"(borrow)
      : "rm"(y[0])
      : "cc");
  return borrow;
}

template<>
inline void mpn_sub_fixed_n<2>(mp_limb_t *res, const mp_limb_t *x, const mp_limb_t *y)
{
  memcpy(res, x, 2 * sizeof(mp_limb_t));
  __asm__(
      "sub %2, %0 \n"
      "sbb %3, %1 \n"
      : "+r"(res[0]), "+r"(res[1])
      : "rm"(y[0]), "rm"(y[1])
      : "cc");
}

template<>
inline mp_limb_t mpn_sub_fixed_n_borrow<2>(mp_limb_t *res, const mp_limb_t *x, const mp_limb_t *y)
{
  memcpy(res, x, 2 * sizeof(mp_limb_t));
  mp_limb_t borrow= 0;
  __asm__ volatile(
      "sub %3, %0 \n"
      "sbb %4, %1 \n"
      "sbb $0, %2 \n"
      : "+r"(res[0]), "+r"(res[1]), "+r"(borrow)
      : "rm"(y[0]), "rm"(y[1])
      : "cc");
  return borrow;
}

template<>
inline void mpn_sub_fixed_n<3>(mp_limb_t *res, const mp_limb_t *x, const mp_limb_t *y)
{
  memcpy(res, x, 3 * sizeof(mp_limb_t));
  __asm__ volatile(
      "sub %3, %0 \n"
      "sbb %4, %1 \n"
      "sbb %5, %2 \n"
      : "+r"(res[0]), "+r"(res[1]), "+r"(res[2])
      : "rm"(y[0]), "rm"(y[1]), "rm"(y[2])
      : "cc");
}

template<>
inline void mpn_sub_fixed_n<4>(mp_limb_t *res, const mp_limb_t *x, const mp_limb_t *y)
{
  memcpy(res, x, 4 * sizeof(mp_limb_t));
  __asm__ volatile(
      "sub %4, %0 \n"
      "sbb %5, %1 \n"
      "sbb %6, %2 \n"
      "sbb %7, %3 \n"
      : "+r"(res[0]), "+r"(res[1]), "+r"(res[2]), "+r"(res[3])
      : "rm"(y[0]), "rm"(y[1]), "rm"(y[2]), "rm"(y[3])
      : "cc");
}

inline void mpn_add_n_use_fixed(mp_limb_t *res, const mp_limb_t *x, const mp_limb_t *y, mp_size_t n)
{
  switch (n)
    {
#define CASE(N)                    \
  case N:                          \
    mpn_add_fixed_n<N>(res, x, y); \
    break;
      CASE(1);
      CASE(2);
      CASE(3);
      CASE(4);
#undef CASE
      default:
        mpn_add_n_with_carry(res, x, y, n);
        break;
    }
}

#ifdef __BMI2__
template<int L, int M, bool ADD>
inline void mpn_addmul_1_fixed__(mp_limb_t *res, const mp_limb_t *y, mp_limb_t x)
{
  mp_limb_t lower[L], higher[L];
  lower[L - 1]= 0;
  higher[L - 1]= 0;
  for (int j= 0; j < M; j++)
    lower[j]= _mulx_u64(x, y[j], (long long unsigned *) higher + j);
  debug_print("lower", lower, L);
  debug_print("higher", higher, L);
  debug_print("before add", res, L + 1);
  if (ADD)
    mpn_add_fixed_n<L>(res, lower, res);
  else
    inline_mpn_copyi(res, lower, L);
  debug_print("first add", res, L + 1);
  mpn_add_fixed_n<L - 1>(res + 1, higher, res + 1);
  debug_print("second add", res, L + 1);
}

template<int L, int M>
inline void mpn_mul_1_fixed(mp_limb_t *res, const mp_limb_t *y, mp_limb_t x)
{
  mpn_addmul_1_fixed__<L, M, false>(res, y, x);
}

template<int L, int M>
inline void mpn_addmul_1_fixed_(mp_limb_t *res, const mp_limb_t *y, mp_limb_t x)
{
  mpn_addmul_1_fixed__<L, M, true>(res, y, x);
}
#else
template<int L, int M>
inline void mpn_addmul_1_fixed_(mp_limb_t *res, const mp_limb_t *y, mp_limb_t x)
{
  mp_limb_t tmp[L];
  memset(tmp, 0, sizeof(tmp));
  memcpy(tmp, y, M * sizeof(mp_limb_t));
  mpn_addmul_1(res, tmp, L, x);
}

template<int L, int M>
inline void mpn_mul_1_fixed(mp_limb_t *res, const mp_limb_t *y, mp_limb_t x)
{
  mp_limb_t tmp[L];
  memset(tmp, 0, sizeof(tmp));
  memcpy(tmp, y, M * sizeof(mp_limb_t));
  mpn_mul_1(res, tmp, L, x);
}
#endif

template<int M>
inline void mpn_addmul_1_fixed(mp_limb_t *res, const mp_limb_t *y, mp_limb_t x)
{
  mpn_addmul_1_fixed_<M + 1, M>(res, y, x);
}

template<int L, int N, int M>
inline void mpn_mul_fixed_(mp_limb_t *res, const mp_limb_t *x, const mp_limb_t *y)
{
  assert(L <= N + M + 2);
  mp_limb_t tmp[N + M + 2];
  memset(tmp, 0, sizeof(tmp));
  for (int i= 0; i < N; i++)
    mpn_addmul_1_fixed<M>(tmp + i, y, x[i]);
  inline_mpn_copyi(res, tmp, L);
}

template<>
inline void mpn_mul_fixed_<1, 1, 1>(mp_limb_t *res, const mp_limb_t *x, const mp_limb_t *y)
{
  *res= *x * *y;
}

template<>
inline void mpn_mul_fixed_<2, 2, 2>(mp_limb_t *res, const mp_limb_t *x, const mp_limb_t *y)
{
  mp_limb_t *tmp= res;
  mpn_mul_1_fixed<2, 2>(tmp, y, x[0]);
  mpn_addmul_1_fixed_<1, 1>(tmp + 1, y, x[1]);
}

template<>
inline void mpn_mul_fixed_<3, 3, 3>(mp_limb_t *res, const mp_limb_t *x, const mp_limb_t *y)
{
  inline_mpn_zero(res, 3);
  mp_limb_t *tmp= res;
  mpn_addmul_1_fixed_<3, 3>(tmp, y, x[0]);
  mpn_addmul_1_fixed_<2, 2>(tmp + 1, y, x[1]);
  mpn_addmul_1_fixed_<1, 1>(tmp + 2, y, x[2]);
}

template<>
inline void mpn_mul_fixed_<4, 4, 2>(mp_limb_t *res, const mp_limb_t *x, const mp_limb_t *y)
{
  inline_mpn_zero(res, 4);
  mp_limb_t *tmp= res;
  mpn_addmul_1_fixed_<3, 2>(tmp, y, x[0]);
  mpn_addmul_1_fixed_<3, 2>(tmp + 1, y, x[1]);
  mpn_addmul_1_fixed_<2, 2>(tmp + 2, y, x[2]);
  mpn_addmul_1_fixed_<1, 1>(tmp + 3, y, x[3]);
}

template<int N, int M>
inline void mpn_mul_fixed(mp_limb_t *res, const mp_limb_t *x, const mp_limb_t *y)
{
  mpn_mul_fixed_<N + M, N, M>(res, x, y);
}

#endif
