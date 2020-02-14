/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#include "sacrifice.h"

/* Here the fake sacrifice is easy. We just return the arguments as if
 * all are correct
 */

#include <sstream>
#include <vector>
using namespace std;

#include "Tools/Crypto.h"
#include "config.h"

void sacrifice_triples(Player &P, list<Share> &a, list<Share> &b,
                       list<Share> &c, Open_Protocol &OP)
{
  // The number of sacrifice equations we need per item produced
  int rep= sacrifice_stat_sec / numBits(gfp::pr()) + 1;
  gfp t, te;
  Create_Random(t, P);
  list<Share> ao, bo, co;
  vector<Share> Sh_PO(2 * amortize * rep), Sh_Tau(amortize * rep);
  vector<gfp> PO(2 * amortize * rep), Tau(amortize * rep);
  vector<Share> a2(amortize * rep), b2(amortize * rep), c2(amortize * rep);
  vector<Share> a1(amortize), b1(amortize), c1(amortize);
  Share temp;

  int left_todo= a.size() / (rep + 1);
  while (left_todo > 0)
    {
      int this_loop= amortize;

      if (this_loop > left_todo)
        {
          this_loop= left_todo;
          PO.resize(2 * this_loop * rep);
          Sh_PO.resize(2 * this_loop * rep);
          Tau.resize(this_loop * rep);
          Sh_Tau.resize(this_loop * rep);
        }

      for (int i= 0; i < this_loop; i++)
        {
          a1[i]= a.front();
          a.pop_front();
          ao.push_front(a1[i]);
          b1[i]= b.front();
          b.pop_front();
          bo.push_front(b1[i]);
          c1[i]= c.front();
          c.pop_front();
          co.push_front(c1[i]);
          for (int j= 0; j < rep; j++)
            {
              a2[i * rep + j]= a.front();
              a.pop_front();
              b2[i * rep + j]= b.front();
              b.pop_front();
              c2[i * rep + j]= c.front();
              c.pop_front();
              Sh_PO[2 * i * rep + j].mul(a1[i], t);
              Sh_PO[2 * i * rep + j].sub(Sh_PO[2 * i * rep + j], a2[i * rep + j]);
              Sh_PO[(2 * i + 1) * rep + j].sub(b1[i], b2[i * rep + j]);
            }
        }

      OP.Open_To_All_Begin(PO, Sh_PO, P);
      OP.Open_To_All_End(PO, Sh_PO, P);

      for (int i= 0; i < this_loop; i++)
        {
          for (int j= 0; j < rep; j++)
            {
              Sh_Tau[i * rep + j].mul(c1[i], t);
              Sh_Tau[i * rep + j].sub(Sh_Tau[i * rep + j], c2[i * rep + j]);
              temp.mul(a2[i * rep + j], PO[(2 * i + 1) * rep + j]);
              Sh_Tau[i * rep + j].sub(Sh_Tau[i * rep + j], temp);
              temp.mul(b2[i * rep + j], PO[2 * i * rep + j]);
              Sh_Tau[i * rep + j].sub(Sh_Tau[i * rep + j], temp);
              te.mul(PO[2 * i * rep + j], PO[(2 * i + 1) * rep + j]);
              Sh_Tau[i * rep + j].sub(Sh_Tau[i * rep + j], te, P.get_mac_keys());
            }
        }
      OP.Open_To_All_Begin(Tau, Sh_Tau, P);
      OP.Open_To_All_End(Tau, Sh_Tau, P);

      for (int i= 0; i < this_loop; i++)
        {
          for (int j= 0; j < rep; j++)
            {
              if (!Tau[i * rep + j].is_zero())
                {
                  throw Sacrifice_Check_Error("Multiplication Triples");
                }
            }
        }

      left_todo-= this_loop;
    }
  a= ao;
  b= bo;
  c= co;
}

void sacrifice_phase_triples(Player &P, int fake_sacrifice, list<Share> &a,
                             list<Share> &b, list<Share> &c,
                             Open_Protocol &OP)
{
  if (fake_sacrifice)
    {
      return;
    }
  else
    {
      sacrifice_triples(P, a, b, c, OP);
    }
}

void sacrifice_squares(Player &P, list<Share> &a, list<Share> &b,
                       Open_Protocol &OP)
{
  // The number of sacrifice equations we need per item produced
  int rep= sacrifice_stat_sec / numBits(gfp::pr()) + 1;
  gfp t, t2, te;
  Create_Random(t, P);
  t2.mul(t, t);

  list<Share> ao, bo;
  vector<Share> Sh_PO(amortize * rep), Sh_Tau(amortize * rep);
  vector<gfp> PO(amortize * rep), Tau(amortize * rep);
  vector<Share> a2(amortize * rep), b2(amortize * rep);
  vector<Share> a1(amortize), b1(amortize);
  Share temp;

  int left_todo= a.size() / (rep + 1);
  while (left_todo > 0)
    {
      int this_loop= amortize;
      if (this_loop > left_todo)
        {
          this_loop= left_todo;
          PO.resize(this_loop * rep);
          Sh_PO.resize(this_loop * rep);
          Tau.resize(this_loop * rep);
          Sh_Tau.resize(this_loop * rep);
        }

      for (int i= 0; i < this_loop; i++)
        {
          a1[i]= a.front();
          a.pop_front();
          ao.push_front(a1[i]);
          b1[i]= b.front();
          b.pop_front();
          bo.push_front(b1[i]);
          for (int j= 0; j < rep; j++)
            {
              a2[i * rep + j]= a.front();
              a.pop_front();
              b2[i * rep + j]= b.front();
              b.pop_front();
              Sh_PO[i * rep + j].mul(a1[i], t);
              Sh_PO[i * rep + j].sub(Sh_PO[i * rep + j], a2[i * rep + j]);
            }
        }

      OP.Open_To_All_Begin(PO, Sh_PO, P);
      OP.Open_To_All_End(PO, Sh_PO, P);

      for (int i= 0; i < this_loop; i++)
        {
          for (int j= 0; j < rep; j++)
            {
              Sh_Tau[i * rep + j].mul(b1[i], t2);
              Sh_Tau[i * rep + j].sub(Sh_Tau[i * rep + j], b2[i * rep + j]);

              temp.mul(a1[i], t);
              temp.add(temp, a2[i * rep + j]);
              temp.mul(temp, PO[i * rep + j]);
              Sh_Tau[i * rep + j].sub(Sh_Tau[i * rep + j], temp);
            }
        }

      OP.Open_To_All_Begin(Tau, Sh_Tau, P);
      OP.Open_To_All_End(Tau, Sh_Tau, P);

      for (int i= 0; i < this_loop; i++)
        {
          for (int j= 0; j < rep; j++)
            {
              if (!Tau[i * rep + j].is_zero())
                {
                  throw Sacrifice_Check_Error("Square Pairs");
                }
            }
        }

      left_todo-= this_loop;
    }
  a= ao;
  b= bo;
}

void sacrifice_phase_squares(Player &P, int fake_sacrifice, list<Share> &a,
                             list<Share> &b, Open_Protocol &OP)
{
  if (fake_sacrifice)
    {
      return;
    }
  else
    {
      sacrifice_squares(P, a, b, OP);
    }
}

void sacrifice_bits(Player &P, list<Share> &bits, list<Share> &a,
                    list<Share> &s, Open_Protocol &OP)
{
  // The number of sacrifice equations we need per item produced
  int rep= sacrifice_stat_sec / numBits(gfp::pr()) + 1;
  gfp t, t2, te;
  Create_Random(t, P);
  t2.mul(t, t);

  list<Share> bitso;
  vector<Share> Sh_PO(amortize * rep), Sh_Tau(amortize * rep);
  vector<gfp> PO(amortize * rep), Tau(amortize * rep);
  vector<Share> a2(amortize * rep), s2(amortize * rep);
  vector<Share> bits1(amortize);
  Share temp;

  int left_todo= min(bits.size(), a.size() / rep);
  while (left_todo > 0)
    {
      int this_loop= amortize;
      if (this_loop > left_todo)
        {
          this_loop= left_todo;
          PO.resize(this_loop * rep);
          Sh_PO.resize(this_loop * rep);
          Tau.resize(this_loop * rep);
          Sh_Tau.resize(this_loop * rep);
        }

      for (int i= 0; i < this_loop; i++)
        {
          bits1[i]= bits.front();
          bits.pop_front();
          bitso.push_front(bits1[i]);
          for (int j= 0; j < rep; j++)
            {
              a2[i * rep + j]= a.front();
              a.pop_front();
              s2[i * rep + j]= s.front();
              s.pop_front();
              Sh_PO[i * rep + j].mul(bits1[i], t);
              Sh_PO[i * rep + j].sub(Sh_PO[i * rep + j], a2[i * rep + j]);
            }
        }

      OP.Open_To_All_Begin(PO, Sh_PO, P);
      OP.Open_To_All_End(PO, Sh_PO, P);

      for (int i= 0; i < this_loop; i++)
        {
          for (int j= 0; j < rep; j++)
            {
              Sh_Tau[i * rep + j].mul(bits1[i], t2);
              Sh_Tau[i * rep + j].sub(Sh_Tau[i * rep + j], s2[i * rep + j]);

              temp.mul(bits1[i], t);
              temp.add(temp, a2[i * rep + j]);
              temp.mul(temp, PO[i * rep + j]);
              Sh_Tau[i * rep + j].sub(Sh_Tau[i * rep + j], temp);
            }
        }
      OP.Open_To_All_Begin(Tau, Sh_Tau, P);
      OP.Open_To_All_End(Tau, Sh_Tau, P);

      for (int i= 0; i < this_loop; i++)
        {
          for (int j= 0; j < rep; j++)
            {
              if (!Tau[i * rep + j].is_zero())
                {
                  throw Sacrifice_Check_Error("Bits");
                }
            }
        }

      left_todo-= this_loop;
    }
  bits= bitso;
}

void sacrifice_phase_bits(Player &P, int fake_sacrifice, list<Share> &bit,
                          list<Share> a, list<Share> &sa, Open_Protocol &OP)
{
  if (fake_sacrifice)
    {
      return;
    }
  else
    {
      sacrifice_bits(P, bit, a, sa, OP);
    }
}
