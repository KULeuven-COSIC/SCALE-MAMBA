/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Circuit.h"

#include <algorithm>
#include <iostream>
using namespace std;

unsigned int cnt_numI(const GateType &T)
{
  unsigned int nI= -1;
  if (T == INV || T == EQW)
    {
      nI= 1;
    }
  else if (T == EQ)
    {
      nI= 0;
    }
  else if (T == AND || T == XOR)
    {
      nI= 2;
    }
  else
    {
      throw not_implemented();
    }

  return nI;
}

istream &operator>>(istream &s, Circuit &C)
{
  unsigned int nG, te;
  s >> nG >> C.nWires;

  C.GateT.resize(nG);
  C.GateI.resize(nG);
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
      C.GateI[i].resize(max(2, (int) in));
      C.GateO[i].resize(out);
      for (unsigned int j= 0; j < in; j++)
        {
          s >> C.GateI[i][j];
        }
      for (unsigned int j= 0; j < out; j++)
        {
          s >> C.GateO[i][j];
        }
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
      else if (ss.compare("MAND") == 0)
        {
          C.GateT[i]= MAND;
        }
      else
        {
          throw circuit_error();
        }

      // Check dimensions of in and out are all OK
      if ((C.GateT[i] == INV || C.GateT[i] == EQ || C.GateT[i] == EQW) && in != 1)
        {
          throw circuit_error();
        }
      if ((C.GateT[i] == AND || C.GateT[i] == XOR) && in != 2)
        {
          throw circuit_error();
        }
      if ((C.GateT[i] != MAND) && out != 1)
        {
          throw circuit_error();
        }
      if ((C.GateT[i] == MAND) && out != in / 2)
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
  total_num_AND= 0;
  for (unsigned int i= 0; i < nG; i++)
    {
      if (GateT[i] == AND)
        {
          num_AND++;
          total_num_AND++;
        }
      if (GateT[i] == MAND)
        {
          total_num_AND+= GateO[i].size();
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

void Circuit::output_gate(ostream &s, unsigned int i) const
{
  if (GateT[i] == INV)
    {
      s << "1 1 " << GateI[i][0] << " " << GateO[i][0] << " INV";
    }
  else if (GateT[i] == EQ)
    {
      s << "1 1 " << GateI[i][0] << " " << GateO[i][0] << " EQ";
    }
  else if (GateT[i] == EQW)
    {
      s << "1 1 " << GateI[i][0] << " " << GateO[i][0] << " EQW";
    }
  else
    {
      s << GateI[i].size() << " " << GateO[i].size() << " ";
      for (unsigned int j= 0; j < GateI[i].size(); j++)
        {
          s << GateI[i][j] << " ";
        }
      for (unsigned int j= 0; j < GateO[i].size(); j++)
        {
          s << GateO[i][j] << " ";
        }
      if (GateT[i] == AND)
        {
          s << "AND";
        }
      else if (GateT[i] == MAND)
        {
          s << "MAND";
        }
      else
        {
          s << "XOR";
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
      C.output_gate(s, i);
      s << "\n";
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
      if (GateT[i] != INV && GateT[i] != EQ && GateT[i] != EQW)
        {
          for (unsigned int j= 1; j < GateO[i].size(); j++)
            {
              if (W[GateI[i][j]] < 0)
                {
                  throw circuit_error();
                }
            }
        }
      // Now evaluate the gate
      if (GateT[i] == AND)
        {
          W[GateO[i][0]]= W[GateI[i][0]] & W[GateI[i][1]];
        }
      else if (GateT[i] == MAND)
        {
          for (unsigned int j= 0; j < GateO[i].size(); j++)
            {
              W[GateO[i][j]]= W[GateI[i][j]] & W[GateI[i][j + GateO[i].size()]];
            }
        }
      else if (GateT[i] == XOR)
        {
          W[GateO[i][0]]= W[GateI[i][0]] ^ W[GateI[i][1]];
        }
      else if (GateT[i] == EQ)
        {
          W[GateO[i][0]]= GateI[i][0];
        }
      else if (GateT[i] == EQW)
        {
          W[GateO[i][0]]= W[GateI[i][0]];
        }
      else if (GateT[i] == INV)
        {
          W[GateO[i][0]]= 1 - W[GateI[i][0]];
        }
      else
        {
          throw not_implemented();
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
  unsigned int num;
  if (GateT[j] == MAND)
    {
      num= GateI[j].size();
    }
  else
    {
      num= cnt_numI(GateT[j]);
    }
  for (unsigned int i= 0; i < num; i++)
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
  swap(GateT[i], GateT[j]);
  swap(GateI[i], GateI[j]);
  swap(GateO[i], GateO[j]);
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
                  output_gate(cout, i);
                  cout << endl;
                  abort();
                }
            }
          // Swap gate i and gate j if i<>j
          if (i != j)
            {
              swap_gate(i, j);
            }
        }
      // Set output wires to be OK
      for (unsigned int j= 0; j < GateO[i].size(); j++)
        {
          used[GateO[i][j]]= true;
        }
    }
  recompute_map();
}

vector<unsigned int> Circuit::compute_depth() const
{
  // This is the gate depth, which is the input wire depth
  vector<unsigned int> depth(GateT.size());

  // These are the wire depths
  vector<unsigned int> wire_depth(nWires);
  for (unsigned int i= 0; i < nWires; i++)
    {
      wire_depth[i]= 0;
    }

  // Now go through each gate and compute its depth
  for (unsigned int i= 0; i < depth.size(); i++)
    {
      if (GateT[i] == EQ)
        {
          depth[i]= 0;
          wire_depth[GateO[i][0]]= 0;
        }
      else if (GateT[i] == INV || GateT[i] == EQW)
        {
          depth[i]= wire_depth[GateI[i][0]];
          wire_depth[GateO[i][0]]= depth[i];
        }
      else if (GateT[i] == XOR)
        {
          depth[i]= max(wire_depth[GateI[i][0]], wire_depth[GateI[i][1]]);
          wire_depth[GateO[i][0]]= depth[i];
        }
      else
        {
          depth[i]= wire_depth[GateI[i][0]];
          for (unsigned int j= 1; j < GateI[i].size(); j++)
            {
              depth[i]= max(depth[i], wire_depth[GateI[i][j]]);
            }
          // Now assign this to output wires, but add one as
          // we have an AND or MAND gate
          for (unsigned int j= 0; j < GateO[i].size(); j++)
            {
              wire_depth[GateO[i][j]]= depth[i] + 1;
            }
        }
    }

  return depth;
}

void Circuit::merge_AND_gates()
{
  vector<unsigned int> depth= compute_depth();
  vector<unsigned int>::iterator pos_max= max_element(depth.begin(), depth.end());

  unsigned int maxd= *pos_max;

  for (unsigned int d= 0; d < maxd; d++)
    {
      // Find the first AND/MAND of this depth
      int last, i= 0, first= -1;
      while (first == -1)
        {
          if ((depth[i] == d) && (GateT[i] == AND || GateT[i] == MAND))
            {
              first= i;
            }
          i++;
        }

      // Now search for others to merge with this first one
      for (unsigned int j= first + 1; j < GateT.size(); j++)
        {
          if ((depth[j] == d) && (GateT[j] == AND || GateT[j] == MAND))
            {
              last= GateT.size() - 1;
              // Swap the last gate with the j-th one
              swap_gate(j, last);
              swap(depth[j], depth[last]);

              // Merge last gate with the first one
              GateT[first]= MAND;
              unsigned int n= GateO[first].size();
              unsigned int m= GateO[last].size();
              GateO[first].resize(n + m);
              GateI[first].resize(2 * (n + m));
              // Move old data into correct place
              for (int k= n - 1; k >= 0; k--)
                {
                  GateI[first][n + m + k]= GateI[first][n + k];
                }
              // Move new data in
              for (unsigned int k= 0; k < m; k++)
                {
                  GateI[first][n + k]= GateI[last][k];
                  GateI[first][2 * n + m + k]= GateI[last][k + m];
                  GateO[first][n + k]= GateO[last][k];
                }
              // Now delete the old gate
              GateT.resize(last);
              GateI.resize(last);
              GateO.resize(last);

              // Subtract one from j as we need to go again in case
              // the old last gate was an AND
              j--;
            }
        }
    }

  sort();
}
