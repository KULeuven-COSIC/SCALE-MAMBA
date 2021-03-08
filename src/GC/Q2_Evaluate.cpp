/*
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Q2_Evaluate.h"
#include "Mod2Engine/Mod2Maurer.h"
#include "Mod2Engine/Mod2_Thread.h"

extern Mod2_Thread_Data MTD;

void Execute_MAND_Gate(unsigned int i,
                       vector<Share2> &W,
                       const Circuit &C,
                       const vector<vector<Share2>> &triples,
                       unsigned int cnt,
                       Player &P)
{
  static vector<Share2> input;
  static vector<word> output;

  unsigned int n= C.MAND_Gate_Size(i);
  unsigned int len= (2 * n) / 64;
  if (((2 * n) % 64) != 0)
    {
      len++;
    }
  input.resize(len, Share2(P.whoami()));
  output.resize(len);

  int k= 0, count= 0;
  Share2 te;
  for (unsigned int j= 0; j < n; j++)
    {
      unsigned int in0= C.Gate_Wire_In(i, j);
      unsigned int in1= C.Gate_Wire_In(i, n + j);

      input[k].SHL(input[k], 2);
      te.multi_add(triples[cnt + j][1], W[in1], triples[cnt + j][0], W[in0]);
      input[k].add(te);
      count+= 2;
      if (count == 64)
        {
          count= 0;
          k++;
        }
    }
  if (count != 0)
    {
      input[k].SHL(input[k], 64 - count);
    }

  P.OP2->Open_To_All(output, input, P, 2);

  k= 0;
  count= 63;
  for (unsigned int j= 0; j < n; j++)
    {
      unsigned int out= C.Gate_Wire_Out(i, j);
      W[out]= triples[cnt + j][2];

      unsigned val= (output[k] >> (count - 1)) & 3;
      if ((val & 1) == 1)
        {
          W[out].add(triples[cnt + j][1]);
        }
      if ((val & 2) == 2)
        {
          W[out].add(triples[cnt + j][0]);
          if (val == 3)
            {
              W[out].negate();
            }
        }
      count-= 2;
      if (count < 0)
        {
          count= 63;
          k++;
        }
    }
}

/* The input and output are assumed to be really bits,
 * not 64-bit shares
 */
void Evaluate(vector<vector<Share2>> &output,
              const vector<vector<Share2>> &input,
              const Circuit &C,
              Player &P,
              unsigned int online_thread_num)
{
  MTD.check();

  // Create wires
  vector<Share2> W(C.get_nWires());

  // Enter the input bits
  unsigned int cnt= 0;
  for (unsigned int i= 0; i < C.num_inputs(); i++)
    {
      for (unsigned int j= 0; j < C.num_iWires(i); j++)
        {
          W[cnt]= input[i][j];
          cnt++;
        }
    }

  // Get number of AND gates and get the associated triples
  unsigned int n_and_gates= C.total_num_AND_gates();
  vector<vector<Share2>> triples(n_and_gates, vector<Share2>(3));
  MTD.get_Single_Triples(triples, online_thread_num);

#ifdef BENCH_OFFLINE
  P.mod2s+= n_and_gates;
#endif

  // Evaluate the circuit
  cnt= 0;
  for (unsigned int i= 0; i < C.get_nGates(); i++)
    {
      GateType GT= C.get_GateType(i);
      // First deal with MAND's as they are special
      if (GT == MAND)
        {
          Execute_MAND_Gate(i, W, C, triples, cnt, P);
          cnt+= C.MAND_Gate_Size(i);
        }
      else
        {
          unsigned int out= C.Gate_Wire_Out(i);
          unsigned int in0= C.Gate_Wire_In(i, 0), in1= -1;
          if (GT == AND || GT == XOR)
            {
              in1= C.Gate_Wire_In(i, 1);
            }

          // Now evaluate the gate
          if (GT == AND)
            {
              Mult_Bit(W[out], W[in0], W[in1], triples[cnt], P);
              cnt++;
            }
          else if (GT == XOR)
            {
              W[out].add(W[in0], W[in1]);
            }
          else if (GT == EQ)
            {
              W[out]= in0;
            }
          else if (GT == EQW)
            {
              W[out]= W[in0];
            }
          else if (GT == INV)
            {
              W[out].negate(W[in0]);
            }
          else
            {
              throw bad_value();
            }
        }
    }

  // Now produce the output
  output.resize(C.num_outputs());
  cnt= C.get_nWires();
  for (unsigned int i= 0; i < C.num_outputs(); i++)
    {
      cnt-= C.num_oWires(i);
    }
  for (unsigned int i= 0; i < C.num_outputs(); i++)
    {
      output[i].resize(C.num_oWires(i));
      for (unsigned int j= 0; j < C.num_oWires(i); j++)
        {
          output[i][j]= W[cnt];
          cnt++;
        }
    }
}
