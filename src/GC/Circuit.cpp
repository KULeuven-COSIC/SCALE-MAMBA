/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Circuit.h"

istream &operator>>(istream &s, Circuit &C)
{
  unsigned int nG, te;
  s >> nG >> C.nWires;
  C.num_AND= 0;

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
          C.num_AND++;
        }
      else if (ss.compare("XOR") == 0)
        {
          C.GateT[i]= XOR;
        }
      else if (ss.compare("INV") == 0)
        {
          C.GateT[i]= INV;
        }
      else
        {
          throw circuit_error();
        }
      if (C.GateT[i] == INV && in == 2)
        {
          throw circuit_error();
        }
      if (C.GateT[i] != INV && in == 1)
        {
          throw circuit_error();
        }
    }

  // Define map between AND gates and the actual gates
  C.map.resize(C.num_AND);
  C.imap.resize(nG);

  // Set stupid default value for imap, to be caught in access function
  for (unsigned int i= 0; i < nG; i++)
    {
      C.imap[i]= 2 * nG;
    }
  unsigned int cnt= 0;
  for (unsigned int i= 0; i < nG; i++)
    {
      if (C.GateT[i] == AND)
        {
          C.map[cnt]= i;
          C.imap[i]= cnt;
          cnt++;
        }
    }
  return s;
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
      if (W[GateI[i][0]] < 0)
        {
          throw circuit_error();
        }
      if (GateT[i] != INV && W[GateI[i][1]] < 0)
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
