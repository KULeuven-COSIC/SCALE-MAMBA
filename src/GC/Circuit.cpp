/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Circuit.h"

#include <iostream>
using namespace std;

unsigned int cnt_numI(const GateType &T)
{
  unsigned int nI= 2;
  if (T == INV || T == EQW)
    {
      nI= 1;
    }
  else if (T == EQ)
    {
      nI= 0;
    }
  return nI;
}

istream &operator>>(istream &s, Circuit &C)
{
  unsigned int nG, te;
  s >> nG >> C.nWires;

  C.GateT.resize(nG);
  C.GateI.resize(nG, vector<unsigned int>(2));
  C.GateO.resize(nG);

  s >> te;
  C.numI.resize(te);
  for (unsigned int i= 0; i < C.numI.size(); i++)
    {
      s >> C.numI[i];
    }

  s >> te;
  C.numO.resize(te);
  for (unsigned int i= 0; i < C.numO.size(); i++)
    {
      s >> C.numO[i];
    }

  unsigned int in, out;
  string ss;
  for (unsigned int i= 0; i < nG; i++)
    {
      s >> in >> out;
      if (in > 2 || out > 1)
        {
          throw circuit_error();
        }
      for (unsigned int j= 0; j < in; j++)
        {
          s >> C.GateI[i][j];
        }
      s >> C.GateO[i];
      s >> ss;
      if (ss.compare("AND") == 0)
        {
          C.GateT[i]= AND;
        }
      else if (ss.compare("XOR") == 0)
        {
          C.GateT[i]= XOR;
        }
      else if (ss.compare("INV") == 0)
        {
          C.GateT[i]= INV;
        }
      else if (ss.compare("EQ") == 0)
        {
          C.GateT[i]= EQ;
        }
      else if (ss.compare("EQW") == 0)
        {
          C.GateT[i]= EQW;
        }
      else
        {
          throw circuit_error();
        }
      if ((C.GateT[i] == INV || C.GateT[i] == EQ || C.GateT[i] == EQW) && in == 2)
        {
          throw circuit_error();
        }
      if ((C.GateT[i] != INV && C.GateT[i] != EQ && C.GateT[i] != EQW) && in == 1)
        {
          throw circuit_error();
        }
    }

  C.recompute_map();

  return s;
}

void Circuit::recompute_map()
{
  unsigned int nG= GateT.size();

  num_AND= 0;
  for (unsigned int i= 0; i < nG; i++)
    {
      if (GateT[i] == AND)
        {
          num_AND++;
        }
    }

  // Define map between AND gates and the actual gates
  map.resize(num_AND);
  imap.resize(nG);

  // Set stupid default value for imap, to be caught in access function
  for (unsigned int i= 0; i < nG; i++)
    {
      imap[i]= 2 * nG;
    }
  unsigned int cnt= 0;
  for (unsigned int i= 0; i < nG; i++)
    {
      if (GateT[i] == AND)
        {
          map[cnt]= i;
          imap[i]= cnt;
          cnt++;
        }
    }
}

ostream &operator<<(ostream &s, const Circuit &C)
{
  s << C.GateT.size() << " " << C.nWires << endl;

  s << C.numI.size() << " ";
  for (unsigned int i= 0; i < C.numI.size(); i++)
    {
      s << C.numI[i] << " ";
    }
  s << endl;

  s << C.numO.size() << " ";
  for (unsigned int i= 0; i < C.numO.size(); i++)
    {
      s << C.numO[i] << " ";
    }
  s << endl
    << endl;
  for (unsigned int i= 0; i < C.GateT.size(); i++)
    {
      if (C.GateT[i] == INV)
        {
          s << "1 1 " << C.GateI[i][0] << " " << C.GateO[i] << " INV\n";
        }
      else if (C.GateT[i] == EQ)
        {
          s << "1 1 " << C.GateI[i][0] << " " << C.GateO[i] << " EQ\n";
        }
      else if (C.GateT[i] == EQW)
        {
          s << "1 1 " << C.GateI[i][0] << " " << C.GateO[i] << " EQW\n";
        }
      else
        {
          s << "2 1 " << C.GateI[i][0] << " " << C.GateI[i][1] << " ";
          s << C.GateO[i] << " ";
          if (C.GateT[i] == AND)
            {
              s << "AND\n";
            }
          else
            {
              s << "XOR\n";
            }
        }
    }
  s << endl;
  return s;
}

void Circuit::evaluate(const vector<vector<int>> &inputs,
                       vector<vector<int>> &outputs) const
{
  vector<int> W(nWires);
  for (unsigned int i= 0; i < nWires; i++)
    {
      W[i]= -1;
    }

  // Load inputs
  unsigned int cnt= 0;
  for (unsigned int i= 0; i < numI.size(); i++)
    {
      for (unsigned int j= 0; j < numI[i]; j++)
        {
          W[cnt]= inputs[i][j];
          cnt++;
        }
    }

  // Evaluate the circuit
  for (unsigned int i= 0; i < GateT.size(); i++)
    { // First check if ordering is broken
      if (GateT[i] != EQ && W[GateI[i][0]] < 0)
        {
          throw circuit_error();
        }
      if ((GateT[i] != INV && GateT[i] != EQ && GateT[i] != EQW) && W[GateI[i][1]] < 0)
        {
          throw circuit_error();
        }
      // Now evaluate the gate
      if (GateT[i] == AND)
        {
          W[GateO[i]]= W[GateI[i][0]] & W[GateI[i][1]];
        }
      else if (GateT[i] == XOR)
        {
          W[GateO[i]]= W[GateI[i][0]] ^ W[GateI[i][1]];
        }
      else if (GateT[i] == EQ)
        {
          W[GateO[i]]= GateI[i][0];
        }
      else if (GateT[i] == EQW)
        {
          W[GateO[i]]= W[GateI[i][0]];
        }
      else
        {
          W[GateO[i]]= 1 - W[GateI[i][0]];
        }
    }

  // Now produce the output
  outputs.resize(numO.size());
  cnt= nWires;
  for (unsigned int i= 0; i < numO.size(); i++)
    {
      cnt-= numO[i];
    }
  for (unsigned int i= 0; i < numO.size(); i++)
    {
      outputs[i].resize(numO[i]);
      for (unsigned int j= 0; j < numO[i]; j++)
        {
          outputs[i][j]= W[cnt];
          cnt++;
        }
    }
}

bool Circuit::gate_is_ok(unsigned int j, const vector<bool> &used) const
{
  for (unsigned int i= 0; i < cnt_numI(GateT[j]); i++)
    {
      if (used[GateI[j][i]] == false)
        {
          return false;
        }
    }
  return true;
}

void Circuit::swap_gate(unsigned int i, unsigned int j)
{
  GateType T= GateT[i];
  GateT[i]= GateT[j];
  GateT[j]= T;
  unsigned int t= GateO[i];
  GateO[i]= GateO[j];
  GateO[j]= t;
  for (unsigned int k= 0; k < 2; k++)
    {
      t= GateI[i][k];
      GateI[i][k]= GateI[j][k];
      GateI[j][k]= t;
    }
}

void Circuit::sort(bool test)
{
  vector<bool> used(nWires);
  for (unsigned int i= 0; i < nWires; i++)
    {
      used[i]= false;
    }

  // Define inputs
  unsigned int cnt= 0;
  for (unsigned int i= 0; i < numI.size(); i++)
    {
      for (unsigned int j= 0; j < numI[i]; j++)
        {
          used[cnt]= true;
          cnt++;
        }
    }

  // Now go through each gate and work out if defined inputs or not
  for (unsigned int i= 0; i < GateT.size(); i++)
    { // Find next ok gate
      unsigned int j= i;
      if (test && gate_is_ok(i, used) == false)
        {
          cout << "Problem in topological sort" << endl;
          abort();
        }
      else
        {
          while (gate_is_ok(j, used) == false)
            {
              j++;
              if (j == GateT.size())
                {
                  cout << "Problem in conversion with (current) gate " << i << endl;
                  cout << "Of type " << GateT[i] << endl;
                  cout << "With input " << GateI[i][0] << " " << GateI[i][1] << endl;
                  cout << "Which produces output " << GateO[i] << endl;
                  abort();
                }
            }
          // Swap gate i and gate j if i<>j
          if (i != j)
            {
              swap_gate(i, j);
            }
          // Set output gate to be OK
        }
      used[GateO[i]]= true;
    }
  recompute_map();
}
