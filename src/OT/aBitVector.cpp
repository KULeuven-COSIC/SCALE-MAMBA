/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "aBitVector.h"
#include "GC/Garbled.h"
#include "OT_Thread_Data.h"
#include <fstream>

extern Base_Circuits Global_Circuit_Store;
extern OT_Thread_Data OTD;

void aBitVector::assign(unsigned long long a)
{
  for (unsigned int i= 0; i < sreg_bitl; i++)
    {
      if ((a & 1) == 0)
        {
          x[i].assign_zero();
        }
      else
        {
          x[i].assign_one();
        }
      a>>= 1;
    }
}

void aBitVector::assign_zero()
{
  for (unsigned int i= 0; i < sreg_bitl; i++)
    {
      x[i].assign_zero();
    }
}

void aBitVector::assign_one()
{
  x[0].assign_one();
  for (unsigned int i= 1; i < sreg_bitl; i++)
    {
      x[i].assign_zero();
    }
}

unsigned long long aBitVector::Open(Player &P) const
{
  vector<int> dv(sreg_bitl);
  Open_aBits(dv, x, P);
  unsigned long long xx= 0;
  for (int i= sreg_bitl - 1; i >= 0; i--)
    {
      xx<<= 1;
      xx+= dv[i];
    }
  return xx;
}

void aBitVector::Bitwise_AND(const aBitVector &a, const aBitVector &b, Player &P, unsigned int online_thread_no)
{
  OTD.check();
  list<aTriple> triples= OTD.aAD.get_aANDs(online_thread_no, sreg_bitl);
  Mult_aBits(x, a.x, b.x, triples, P);
}

void aBitVector::Bit_AND(const aBitVector &a, const aBit &bb, Player &P, unsigned int online_thread_no)
{
  aBitVector b;
  for (int i= 0; i < sreg_bitl; i++)
    {
      b.x[i]= bb;
    }
  Bitwise_AND(a, b, P, online_thread_no);
}

void aBitVector::eval_circuit(Circ_Type T,
                              const aBitVector &a, const aBitVector &b,
                              Player &P, unsigned int online_thread_no)
{
  Base_Garbled_Circuit GC;
  GC.Garble(Global_Circuit_Store.Circuits[T], P, online_thread_no);

  vector<vector<aBit>> input(2);
  vector<vector<aBit>> output(1, vector<aBit>(Global_Circuit_Store.Circuits[T].num_oWires(0)));
  input[0]= a.x;
  input[1]= b.x;
  GC.Evaluate(output, input, Global_Circuit_Store.Circuits[T], P);

  x= output[0];
}

void eval_circuit_2(Circ_Type T,
                    aBitVector &x, aBitVector &y,
                    const aBitVector &a, const aBitVector &b,
                    Player &P, unsigned int online_thread_no)
{
  Base_Garbled_Circuit GC;
  GC.Garble(Global_Circuit_Store.Circuits[T], P, online_thread_no);

  vector<vector<aBit>> input(2);
  vector<vector<aBit>> output(2, vector<aBit>(Global_Circuit_Store.Circuits[T].num_oWires(0)));
  input[0]= a.x;
  input[1]= b.x;
  GC.Evaluate(output, input, Global_Circuit_Store.Circuits[T], P);

  x.x= output[0];
  y.x= output[1];
}

void aBitVector::eval_circuit(Circ_Type T,
                              const aBitVector &a,
                              Player &P, unsigned int online_thread_no)
{
  Base_Garbled_Circuit GC;
  GC.Garble(Global_Circuit_Store.Circuits[T], P, online_thread_no);

  vector<vector<aBit>> input(1);
  vector<vector<aBit>> output(1, vector<aBit>(Global_Circuit_Store.Circuits[T].num_oWires(0)));
  input[0]= a.x;
  GC.Evaluate(output, input, Global_Circuit_Store.Circuits[T], P);

  x= output[0];
}

aBit eval_circuit_bit(Circ_Type T,
                      const aBitVector &a,
                      Player &P, unsigned int online_thread_no)
{
  Base_Garbled_Circuit GC;
  GC.Garble(Global_Circuit_Store.Circuits[T], P, online_thread_no);

  vector<vector<aBit>> input(1);
  vector<vector<aBit>> output(1, vector<aBit>(1));
  input[0]= a.x;
  GC.Evaluate(output, input, Global_Circuit_Store.Circuits[T], P);

  return output[0][0];
}

aBit eval_circuit_bit(Circ_Type T,
                      const aBitVector &a, const aBitVector &b,
                      Player &P, unsigned int online_thread_no)
{
  Base_Garbled_Circuit GC;
  GC.Garble(Global_Circuit_Store.Circuits[T], P, online_thread_no);

  vector<vector<aBit>> input(2);
  vector<vector<aBit>> output(1, vector<aBit>(1));
  input[0]= a.x;
  input[1]= b.x;
  GC.Evaluate(output, input, Global_Circuit_Store.Circuits[T], P);

  return output[0][0];
}

void aBitVector::output(ostream &s, bool human) const
{
  for (unsigned int i= 0; i < sreg_bitl; i++)
    {
      x[i].output(s, human);
    }
}

void aBitVector::input(istream &s, bool human)
{
  for (unsigned int i= 0; i < sreg_bitl; i++)
    {
      x[i].input(s, human);
    }
}

void aBitVector::SHL(const aBitVector &a, unsigned int n)
{
  if (n >= sreg_bitl)
    {
      assign_zero();
      return;
    }
  for (int i= sreg_bitl - n - 1; i >= 0; i--)
    {
      x[i + n]= a.x[i];
    }
  for (unsigned int i= 0; i < n; i++)
    {
      x[i].assign_zero();
    }
}

void aBitVector::SHR(const aBitVector &a, unsigned int n)
{
  if (n >= sreg_bitl)
    {
      assign_zero();
      return;
    }
  for (unsigned int i= 0; i < sreg_bitl - n; i++)
    {
      x[i]= a.x[i + n];
    }
  for (unsigned int i= 0; i < n; i++)
    {
      x[sreg_bitl - n + i].assign_zero();
    }
}
