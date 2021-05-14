/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "aBitVector2.h"
#include "GC/Base_Circuits.h"
#include "GC/Q2_Evaluate.h"
#include "Mod2Maurer.h"
#include "Mod2_Thread.h"

extern Base_Circuits Global_Circuit_Store;

extern Mod2_Thread_Data MTD;

int aBitVector2::p= -1;

/* Expand 64 bits in aBitVector into an array of 64 bits */
void expand(vector<Share2> &a, const aBitVector2 &b)
{
  a.resize(64);
  Share2 x= b.internal_rep();
  for (unsigned int i= 0; i < 64; i++)
    {
      a[i].mul(x, 1ULL);
      x.SHR(x, 1);
    }
}

/* Reverses the above operation */
void collapse(aBitVector2 &b, const vector<Share2> &a)
{
  Share2 x= a[63];
  for (int i= 62; i >= 0; i--)
    {
      x.SHL(x, 1);
      x.add(x, a[i]);
    }
  b.assign(x);
}

void aBitVector2::eval_circuit(Circ_Type T,
                               const aBitVector2 &a,
                               Player &P, unsigned int online_thread_no)
{
  vector<vector<Share2>> input(1, vector<Share2>(64)), output(1, vector<Share2>(64));
  expand(input[0], a);

  Evaluate(output, input, Global_Circuit_Store.Circuits[T], P, online_thread_no);

  collapse(*this, output[0]);
}

void aBitVector2::eval_circuit(Circ_Type T,
                               const aBitVector2 &a, const aBitVector2 &b,
                               Player &P, unsigned int online_thread_no)
{
  vector<vector<Share2>> input(2, vector<Share2>(64)), output(1, vector<Share2>(64));
  expand(input[0], a);
  expand(input[1], b);

  Evaluate(output, input, Global_Circuit_Store.Circuits[T], P, online_thread_no);

  collapse(*this, output[0]);
}

Share2 eval_circuit_bit(Circ_Type T,
                        const aBitVector2 &a,
                        Player &P, unsigned int online_thread_no)
{
  vector<vector<Share2>> input(1, vector<Share2>(64)), output(1, vector<Share2>(64));
  expand(input[0], a);

  Evaluate(output, input, Global_Circuit_Store.Circuits[T], P, online_thread_no);

  return output[0][0];
}

Share2 eval_circuit_bit(Circ_Type T,
                        const aBitVector2 &a, const aBitVector2 &b,
                        Player &P, unsigned int online_thread_no)
{
  vector<vector<Share2>> input(2, vector<Share2>(64)), output(1, vector<Share2>(64));
  expand(input[0], a);
  expand(input[1], b);

  Evaluate(output, input, Global_Circuit_Store.Circuits[T], P, online_thread_no);

  return output[0][0];
}

void eval_circuit_2(Circ_Type T,
                    aBitVector2 &x, aBitVector2 &y,
                    const aBitVector2 &a, const aBitVector2 &b,
                    Player &P, unsigned int online_thread_no)
{
  vector<vector<Share2>> input(2, vector<Share2>(64)), output(1, vector<Share2>(64));
  expand(input[0], a);
  expand(input[1], b);

  Evaluate(output, input, Global_Circuit_Store.Circuits[T], P, online_thread_no);

  collapse(x, output[0]);
  collapse(y, output[1]);
}

void aBitVector2::Bitwise_AND(const aBitVector2 &a, const aBitVector2 &b,
                              Player &P, unsigned int online_thread_no)
{
  MTD.check();
  vector<Share2> T(3);
  MTD.get_Triple(T, online_thread_no);
  Mult_Bits(x, a.x, b.x, T, P);

#ifdef BENCH_OFFLINE
  P.mod2s+= 64;
#endif
}

void aBitVector2::randomize(unsigned int online_thread_no, Player &P)
{
  MTD.check();
  vector<Share2> T(3);
  MTD.get_Triple(T, online_thread_no);
#ifdef BENCH_OFFLINE
  P.mod2s+= sreg_bitl;
#endif
  x= T[0];
}
