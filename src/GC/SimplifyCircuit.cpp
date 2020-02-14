/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "SimplifyCircuit.h"
#include <algorithm>
#include <iostream>
#include <list>
using namespace std;

void SimplifyCircuit::remove_redundant_wires(bool verbose)
{
  // First compute for each wire how many times it
  // is input to a gate, if zero this wire is redundant
  vector<unsigned int> gate_count(C.get_nWires());
  for (unsigned int i= 0; i < C.get_nWires(); i++)
    {
      gate_count[i]= 0;
    }
  for (unsigned int i= 0; i < C.get_nGates(); i++)
    {
      for (unsigned int j= 0; j < cnt_numI(C.get_GateType(i)); j++)
        {
          unsigned int te= C.Gate_Wire_In(i, j);
          gate_count[te]++;
        }
    }

  /* Sort out output wire gate count, giving them 
   * a big number
   */
  unsigned int cnt= C.get_nWires();
  for (unsigned int i= 0; i < C.num_outputs(); i++)
    {
      cnt-= C.num_oWires(i);
    }
  for (unsigned int i= 0; i < C.num_outputs(); i++)
    {
      for (unsigned int j= 0; j < C.num_oWires(i); j++)
        {
          gate_count[cnt]= 0xFFFFFFFF;
          cnt++;
        }
    }

  /* Same for the input wires */
  cnt= 0;
  for (unsigned int i= 0; i < C.num_inputs(); i++)
    {
      for (unsigned int j= 0; j < C.num_iWires(i); j++)
        {
          gate_count[cnt]= 0xFFFFFFFF;
          cnt++;
        }
    }

  // How many should we delete?
  // Also work out what gate we should delete,
  //   - Assign 0xFFFFFFFF if an input gate here, or
  //     no gate to delete
  vector<vector<unsigned int>> wire_d(C.get_nWires(), vector<unsigned int>(2));
  unsigned int del_count= 0;
  for (unsigned int i= 0; i < C.get_nWires(); i++)
    {
      if (gate_count[i] == 0)
        {
          wire_d[del_count][0]= i;
          wire_d[del_count][1]= 0xFFFFFFFF;
          for (unsigned int j= 0; j < C.get_nGates(); j++)
            {
              if (C.Gate_Wire_Out(j) == i)
                {
                  wire_d[del_count][1]= j;
                }
            }
          if (verbose)
            {
              cout << "Wire R : " << i << " G : " << wire_d[del_count][1] << endl;
            }
          del_count++;
        }
    }
  wire_d.resize(del_count);

  // Return if none to delete
  if (del_count == 0)
    {
      return;
    }

  // Produce the match from old wire numbers to new wire numbers
  vector<unsigned int> match(C.get_nWires());
  cnt= 0;
  for (unsigned int i= 0; i < C.get_nWires(); i++)
    {
      if (gate_count[i] != 0)
        {
          match[i]= cnt;
          cnt++;
        }
    }

  // Produce the match from new gate numbers
  // to old gate numbers
  vector<unsigned int> matchG(C.get_nGates());
  unsigned int gcnt= 0;
  for (unsigned int i= 0; i < C.get_nGates(); i++)
    {
      bool flag= false;
      for (unsigned int j= 0; j < del_count; j++)
        {
          if (wire_d[j][1] == i)
            {
              flag= true;
            }
        }
      if (flag == false)
        {
          matchG[gcnt]= i;
          gcnt++;
        }
    }

  Circuit CC= C;
  CC.nWires= cnt;
  unsigned int te;
  for (unsigned int i= 0; i < CC.get_nGates(); i++)
    {
      for (unsigned int j= 0; j < cnt_numI(CC.get_GateType(i)); j++)
        {
          te= CC.Gate_Wire_In(i, j);
          CC.GateI[i][j]= match[te];
        }
      te= CC.Gate_Wire_Out(i);
      CC.GateO[i]= match[te];
    }

  // Remove gates
  unsigned int num= C.get_nGates() - gcnt;
  for (unsigned int i= 0; i < gcnt; i++)
    {
      CC.GateT[i]= CC.GateT[matchG[i]];
      CC.GateI[i]= CC.GateI[matchG[i]];
      CC.GateO[i]= CC.GateO[matchG[i]];
    }
  CC.GateT.resize(gcnt);
  CC.GateI.resize(gcnt);
  CC.GateO.resize(gcnt);
  CC.recompute_map();
  assign(CC);

  if (verbose)
    {
      cout << "In removing redundant wires: ";
      cout << "Removed " << del_count << " wires and " << num << " gates" << endl;
    }
}

void SimplifyCircuit::Remove_EQW()
{
  unsigned int neqw= 0;
  // Also a match from old wire numbers to new wire numbers
  vector<unsigned int> matchW(C.get_nWires());
  for (unsigned int i= 0; i < C.get_nWires(); i++)
    {
      matchW[i]= i;
    }

  unsigned int out_boundary= C.get_nWires();
  for (unsigned int i= 0; i < C.num_outputs(); i++)
    {
      out_boundary-= C.num_oWires(i);
    }

  unsigned int input_boundary= 0;
  for (unsigned int i= 0; i < C.num_inputs(); i++)
    {
      input_boundary+= C.num_iWires(i);
    }

  vector<bool> keepG(C.get_nGates());
  for (unsigned int i= 0; i < C.get_nGates(); i++)
    {
      keepG[i]= true;
      unsigned int out= C.Gate_Wire_Out(i);
      if (C.get_GateType(i) == EQW)
        { // cout << "\t\t" << i << " ";
          neqw++;
          /* If input and output are equal, then do nothing*/
          if (C.GateI[i][0] < input_boundary && out >= out_boundary)
            { /* Do nothing */
              // cout << "A" << endl;
            }
          /* If output wire not an output for the circuit replace
             * it with the input wire
             */
          else if (out < out_boundary)
            {
              keepG[i]= false;
              matchW[out]= C.GateI[i][0];
              //cout << "B" << endl;
            }
          /* If this input been used before, i.e. it is already an output
           * then keep this EQW gate between outputs
           */
          else if (matchW[C.GateI[i][0]] >= out_boundary)
            { /* Do nothing */
              //cout << "D" << endl;
            }
          /* Else make the input wire equal to the output wire */
          else
            {
              keepG[i]= false;
              matchW[C.GateI[i][0]]= out;
              //cout << "C" << endl;
            }
        }
    }

  /* Now go over the circuit and relabel the wires and remove the 
     * EQW gates
     */
  unsigned int ngates= 0;
  for (unsigned int i= 0; i < C.get_nGates(); i++)
    {
      if (keepG[i] == true)
        {
          for (unsigned int j= 0; j < cnt_numI(C.get_GateType(i)); j++)
            {
              C.GateI[ngates][j]= matchW[C.GateI[i][j]];
            }
          C.GateO[ngates]= matchW[C.GateO[i]];
          C.GateT[ngates]= C.GateT[i];
          ngates++;
        }
    }
  C.GateO.resize(ngates);
  C.GateI.resize(ngates);
  C.GateT.resize(ngates);
  C.sort();
  remove_redundant_wires();
}

bool SimplifyCircuit::ApplyAssignment(const GateType &T, bool verbose)
{
  bool done= false;
  Circuit CC= C;
  // Array which defines whether an old variable is zero/one or not
  vector<bool> is_zero(C.get_nWires());
  vector<bool> is_one(C.get_nWires());
  for (unsigned int i= 0; i < C.get_nWires(); i++)
    {
      is_zero[i]= false;
      is_one[i]= false;
    }

  // First produce the match from new gate numbers
  // to old gate numbers
  vector<unsigned int> matchG(C.get_nGates());
  unsigned int gcnt= 0;

  // Also a match from old wire numbers to new wire
  // numbers
  vector<unsigned int> matchW(C.get_nWires());
  for (unsigned int i= 0; i < C.get_nWires(); i++)
    {
      matchW[i]= i;
    }

  unsigned int out_boundary= CC.get_nWires();
  for (unsigned int i= 0; i < CC.num_outputs(); i++)
    {
      out_boundary-= CC.num_oWires(i);
    }

  unsigned int input_boundary= 0;
  for (unsigned int i= 0; i < CC.num_inputs(); i++)
    {
      input_boundary+= CC.num_iWires(i);
    }

  unsigned int neq= 0;
  ;
  for (unsigned int i= 0; i < C.get_nGates(); i++)
    {
      unsigned int out= C.Gate_Wire_Out(i);
      if (C.get_GateType(i) == EQ && C.Gate_Wire_In(i, 0) == 0)
        {
          is_zero[out]= true;
        }
      if (C.get_GateType(i) == EQ && C.Gate_Wire_In(i, 0) == 1)
        {
          is_one[out]= true;
        }
      if (C.get_GateType(i) == EQ)
        {
          neq++;
        }

      // Map input variables on gate
      bool zero[2]= {0, 0}, one[2]= {0, 0};
      for (unsigned int j= 0; j < cnt_numI(C.get_GateType(i)); j++)
        {
          unsigned int k= C.Gate_Wire_In(i, j);
          CC.GateI[i][j]= matchW[k];
          zero[j]= is_zero[k];
          one[j]= is_one[k];
        }

      if (C.get_GateType(i) != T)
        {
          matchG[gcnt]= i;
          gcnt++;
        }
      else if (T == INV)
        {
          if (zero[0] == true)
            { // NOT 0 = 1
              CC.GateT[i]= EQ;
              CC.GateI[i][0]= 1;
              CC.GateO[i]= out;
              done= true;
            }
          else if (one[0] == true)
            { // NOT 1 = 0
              CC.GateT[i]= EQ;
              CC.GateI[i][0]= 0;
              CC.GateO[i]= out;
              done= true;
            }
          matchG[gcnt]= i;
          gcnt++;
        }
      else if (T == EQW)
        {
          if (zero[0] == true)
            {
              CC.GateT[i]= EQ;
              CC.GateI[i][0]= 0;
              CC.GateO[i]= out;
              matchG[gcnt]= i;
              gcnt++;
              done= true;
            }
          else if (one[0] == true)
            {
              CC.GateT[i]= EQ;
              CC.GateI[i][0]= 1;
              CC.GateO[i]= out;
              matchG[gcnt]= i;
              gcnt++;
              done= true;
            }
          else
            {
              matchG[gcnt]= i;
              gcnt++;
            }
        }
      else
        { // AND and XOR here
          // First do a re-order, to get the first arg to be 0 or 1 if zero or one
          if (zero[1] == true || one[1] == true)
            {
              swap(CC.GateI[i][0], CC.GateI[i][1]);
              swap(zero[0], zero[1]);
              swap(one[0], one[1]);
            }
          if (T == AND)
            {
              if (zero[0] == true)
                { // 0 AND x = 0
                  CC.GateT[i]= EQ;
                  CC.GateI[i][0]= 0;
                  CC.GateO[i]= out;
                  done= true;
                  matchG[gcnt]= i;
                  gcnt++;
                }
              else if (one[0] == true && out < out_boundary)
                { // 1 AND x = x
                  // Delete gate and keep the output wire as i1d
                  matchW[out]= CC.GateI[i][1];
                  done= true;
                }
              else if (one[0] == true && out >= out_boundary)
                { // 1 AND x = x
                  // Replace with EQW gate
                  CC.GateT[i]= EQW;
                  CC.GateI[i][0]= CC.GateI[i][1];
                  CC.GateO[i]= out;
                  done= true;
                  matchG[gcnt]= i;
                  gcnt++;
                }
              else
                {
                  matchG[gcnt]= i;
                  gcnt++;
                }
            }
          else if (T == XOR)
            {
              if (one[0] == true)
                { // 1 XOR x = INV x
                  CC.GateT[i]= INV;
                  CC.GateI[i][0]= CC.GateI[i][1];
                  done= true;
                  matchG[gcnt]= i;
                  gcnt++;
                }
              else if (zero[0] == true && out < out_boundary)
                { // 0 XOR x = x
                  // Delete gate and keep the output wire as i1d
                  matchW[out]= CC.GateI[i][1];
                  done= true;
                }
              else if (zero[0] == true && out >= out_boundary)
                { // 0 XOR x = x
                  // Replace with EQW gate
                  CC.GateT[i]= EQW;
                  CC.GateI[i][0]= CC.GateI[i][1];
                  CC.GateO[i]= out;
                  done= true;
                  matchG[gcnt]= i;
                  gcnt++;
                }
              else
                {
                  matchG[gcnt]= i;
                  gcnt++;
                }
            }
        }
    }

  // Remove gates
  unsigned int num= C.get_nGates() - gcnt;
  int new_neq= 0;
  for (unsigned int i= 0; i < gcnt; i++)
    {
      CC.GateT[i]= CC.GateT[matchG[i]];
      CC.GateI[i]= CC.GateI[matchG[i]];
      CC.GateO[i]= CC.GateO[matchG[i]];
      if (CC.GateT[i] == EQ)
        {
          new_neq++;
        }
    }
  CC.GateT.resize(gcnt);
  CC.GateI.resize(gcnt);
  CC.GateO.resize(gcnt);
  CC.recompute_map();
  C= CC;

  if (verbose)
    {
      cout << "In ApplyAssignment(" << (int) T << ") : ";
      cout << "Removed " << num << " gates." << endl;
      cout << "\tNumber of EQ gates = " << neq << endl;
      cout << "\tNew number of EQ gates = " << new_neq << endl;
      cout << "\tNumber of gates = " << gcnt << endl;
    }

  return done;
}

void SimplifyCircuit::Set_Inputs(const vector<unsigned int> &set_input)
{
  // Check length of set_input is correct
  unsigned int check= set_input.size();
  for (unsigned int i= 0; i < C.num_inputs(); i++)
    {
      check-= C.num_iWires(i);
    }
  if (check != 0)
    {
      throw circuit_error();
    }

  // Map of old wire numbers to new wire numbers
  vector<unsigned int> matchW(C.get_nWires());
  for (unsigned int i= 0; i < C.get_nWires(); i++)
    {
      matchW[i]= i;
    }

  // First count number of gates to add and fix the new inputs
  // correctly to new wire numbers
  unsigned int ii= 0, cnt= 0, cnt2= 0;
  for (unsigned int i= 0; i < C.num_inputs(); i++)
    {
      unsigned int numi= C.num_iWires(i);
      for (unsigned int j= 0; j < numi; j++)
        {
          if (set_input[ii] == 0 || set_input[ii] == 1)
            {
              cnt++;
              C.numI[i]--;
            }
          else
            {
              matchW[ii]= cnt2;
              cnt2++;
            }
          ii++;
        }
    }
  // Now fix the old (deleted) inputs to new wire numbers
  for (unsigned int i= 0; i < set_input.size(); i++)
    {
      if (set_input[i] == 0 || set_input[i] == 1)
        {
          matchW[i]= cnt2;
          cnt2++;
        }
    }

  unsigned int ngate= C.get_nGates();
  C.GateT.resize(ngate + cnt);
  C.GateI.resize(ngate + cnt);
  C.GateO.resize(ngate + cnt);
  for (int i= (int) ngate - 1; i >= 0; i--)
    {
      C.GateT[i + cnt]= C.GateT[i];
      C.GateI[i + cnt]= C.GateI[i];
      C.GateO[i + cnt]= C.GateO[i];
      for (unsigned int j= 0; j < cnt_numI(C.get_GateType(i + cnt)); j++)
        {
          unsigned int k= C.Gate_Wire_In(i + cnt, j);
          C.GateI[i + cnt][j]= matchW[k];
        }
      unsigned int k= C.Gate_Wire_Out(i + cnt);
      C.GateO[i + cnt]= matchW[k];
    }

  unsigned int te= 0;
  for (unsigned int i= 0; i < set_input.size(); i++)
    {
      if (set_input[i] == 0)
        {
          C.GateT[te]= EQ;
          C.GateI[te][0]= 0;
          C.GateO[te]= matchW[i];
          te++;
        }
      else if (set_input[i] == 1)
        {
          C.GateT[te]= EQ;
          C.GateI[te][0]= 1;
          C.GateO[te]= matchW[i];
          te++;
        }
    }

  C.recompute_map();
}

void SimplifyCircuit::Remove_Outputs(const vector<unsigned int> &remove_output)
{
  // Here we need to do a match on wire labels again
  // and remove gates which produce a wire label not needed

  // Also a match from old wire numbers to new wire numbers
  vector<unsigned int> matchW(C.get_nWires());
  vector<bool> deleteW(C.get_nWires());
  for (unsigned int i= 0; i < C.get_nWires(); i++)
    {
      matchW[i]= i;
      deleteW[i]= false;
    }

  unsigned int out_boundary= C.get_nWires();
  for (unsigned int i= 0; i < C.num_outputs(); i++)
    {
      out_boundary-= C.num_oWires(i);
    }

  // Do the match for output wires
  unsigned int cnt= 0, wcnt= 0, dcnt= 0;
  for (unsigned int i= 0; i < C.num_outputs(); i++)
    {
      unsigned int lwcnt= 0;
      for (unsigned int j= 0; j < C.num_oWires(i); j++)
        {
          if (remove_output[cnt] == 1)
            {
              matchW[cnt + out_boundary]= out_boundary + wcnt;
              deleteW[cnt + out_boundary]= true;
              dcnt++;
            }
          else
            {
              deleteW[cnt + out_boundary]= false;
              wcnt++;
              lwcnt++;
            }
          cnt++;
        }
      C.numO[i]= lwcnt;
    }
  C.nWires-= dcnt;

  // Now go through each gate relabeling wires
  vector<unsigned int> matchG(C.get_nGates());
  unsigned int gcnt= 0;
  for (unsigned int i= 0; i < C.get_nGates(); i++)
    {
      for (unsigned int j= 0; j < cnt_numI(C.GateT[i]); j++)
        {
          unsigned int k= C.GateI[i][j];
          C.GateI[i][j]= matchW[k];
        }
      unsigned int k= C.GateO[i];
      if (!deleteW[k])
        {
          matchG[gcnt]= i;
          gcnt++;
        }
      else
        { /*cout << "Deleting gate " << i << " " << C.GateT[i] << " " << k << endl; */
        }
      C.GateO[i]= matchW[k];
    }

  // Do gate relabeling
  for (unsigned int i= 0; i < gcnt; i++)
    {
      C.GateT[i]= C.GateT[matchG[i]];
      C.GateI[i]= C.GateI[matchG[i]];
      C.GateO[i]= C.GateO[matchG[i]];
    }
  C.GateT.resize(gcnt);
  C.GateI.resize(gcnt);
  C.GateO.resize(gcnt);
  C.sort();
}

bool SimplifyCircuit::remove_duplicate_gates()
{
  // First produce the match from new gate numbers to old gate numbers
  vector<unsigned int> matchG(C.get_nGates());
  unsigned int gcnt= 0;

  // List of gates we must keep, may need to iterate
  vector<bool> keepG(C.get_nGates());
  for (unsigned int i= 0; i < C.get_nGates(); i++)
    {
      keepG[i]= false;
    }

  // Also a match from old wire numbers to new wire numbers
  vector<unsigned int> matchW(C.get_nWires());
  for (unsigned int i= 0; i < C.get_nWires(); i++)
    {
      matchW[i]= i;
    }

  unsigned int out_boundary= C.get_nWires();
  for (unsigned int i= 0; i < C.num_outputs(); i++)
    {
      out_boundary-= C.num_oWires(i);
    }

  bool done= false;
  for (unsigned int i= 0; i < C.get_nGates(); i++)
    {
      bool deleted= false;
      if (keepG[i] == false)
        {
          for (unsigned int j= i + 1; j < C.get_nGates(); j++)
            {
              if (C.GateT[i] == C.GateT[j] && keepG[j] == false)
                {
                  bool equal= false;
                  if (cnt_numI(C.GateT[i]) == 1 && C.GateI[i][0] == C.GateI[j][0])
                    {
                      equal= true;
                    }
                  else if (cnt_numI(C.GateT[i]) == 2 && C.GateI[i][0] == C.GateI[j][0] && C.GateI[i][1] == C.GateI[j][1])
                    {
                      equal= true;
                    }
                  else if (cnt_numI(C.GateT[i]) == 2 && C.GateI[i][0] == C.GateI[j][1] && C.GateI[i][1] == C.GateI[j][0])
                    {
                      equal= true;
                    }
                  if (equal)
                    { // Swap so gate i has the lowest output wire (might need to sort afterwards, which we do below)
                      if (C.GateO[i] > C.GateO[j])
                        {
                          C.swap_gate(i, j);
                        }
                      if (C.GateO[i] >= out_boundary)
                        { // Two output wires are essentially duplicate
                          equal= false;
                        }
                      else
                        { // Delete gate i
                          matchW[C.GateO[i]]= matchW[C.GateO[j]];
                          keepG[j]= true;
                          deleted= true;
                          done= true;
                          // We can only match one gate with i on every iteration, so now push j to the end of the loop
                          j= C.get_nGates();
                        }
                    }
                }
            }
        }
      if (!deleted)
        {
          matchG[gcnt]= i;
          gcnt++;
        }
    }

  // Do wire relabeling...
  for (unsigned int i= 0; i < C.get_nGates(); i++)
    {
      for (unsigned int j= 0; j < cnt_numI(C.GateT[i]); j++)
        {
          unsigned int k= C.GateI[i][j];
          C.GateI[i][j]= matchW[k];
        }
      unsigned int k= C.GateO[i];
      C.GateO[i]= matchW[k];
    }

  // Do gate relabeling
  for (unsigned int i= 0; i < gcnt; i++)
    {
      C.GateT[i]= C.GateT[matchG[i]];
      C.GateI[i]= C.GateI[matchG[i]];
      C.GateO[i]= C.GateO[matchG[i]];
    }
  C.GateT.resize(gcnt);
  C.GateI.resize(gcnt);
  C.GateO.resize(gcnt);
  C.sort();

  return done;
}

void SimplifyCircuit::Simplify()
{
  bool changed= true;
  while (changed)
    {
      changed= false;
      changed|= ApplyAssignment(INV);
      changed|= ApplyAssignment(AND);
      changed|= ApplyAssignment(XOR);
      changed|= ApplyAssignment(EQW);
    }
  do
    {
      changed= remove_duplicate_gates();
    }
  while (changed);

  Remove_EQW();

  remove_redundant_wires();

  // Remove zero'd input/output variables
  for (unsigned int i= 0; i < C.numI.size(); i++)
    {
      if (C.numI[i] == 0)
        {
          for (unsigned int j= i + 1; j < C.numI.size(); j++)
            {
              C.numI[j - 1]= C.numI[j];
            }
          C.numI.resize(C.numI.size() - 1);
        }
    }
  for (unsigned int i= 0; i < C.numO.size(); i++)
    {
      if (C.numO[i] == 0)
        {
          for (unsigned int j= i + 1; j < C.numO.size(); j++)
            {
              C.numO[j - 1]= C.numO[j];
            }
          C.numO.resize(C.numO.size() - 1);
        }
    }
}

// Creates the sub-circuit given with numi inputs
// specified in the array used
void SimplifyCircuit::Create_SubCircuit(unsigned int numi, vector<bool> &used,
                                        vector<unsigned int> &mapwires,
                                        vector<unsigned int> &mapgates,
                                        Circuit &Sub)
{
  unsigned int out_boundary= C.get_nWires();
  for (unsigned int i= 0; i < C.num_outputs(); i++)
    {
      out_boundary-= C.num_oWires(i);
    }

  unsigned int nw= numi, ng= 0;
  // First go through gates and see what depends on these input wires
  for (unsigned int k= 0; k < C.get_nGates(); k++)
    { // Add Gate into subcircuit if depends
      if (C.gate_is_ok(k, used))
        {
          mapgates.push_back(k);
          unsigned int out= C.Gate_Wire_Out(k);
          used[out]= true;
          mapwires.push_back(out);
          Sub.GateT.push_back(C.get_GateType(k));
          Sub.GateO.push_back(nw);
          if (C.get_GateType(k) == AND)
            {
              Sub.num_AND++;
            }
          Sub.GateI.push_back(vector<unsigned int>(2));
          for (unsigned int j= 0; j < cnt_numI(C.get_GateType(k)); j++)
            {
              Sub.GateI[ng][j]= distance(mapwires.begin(),
                                         find(mapwires.begin(), mapwires.end(),
                                              C.Gate_Wire_In(k, j)));
            }
          nw++;
          ng++;
        }
    }
  Sub.nWires= nw;
  // Fix output gates
  // Go through every wire in the Sub circuit and see if it
  // is used elsewhere in the main circuit, or it is an output
  // wire of the main circuit
  // No need to look at the numi input wires here
  Sub.numO[0]= 0;
  for (unsigned int k= numi; k < Sub.get_nWires() - Sub.numO[0]; k++)
    {
      bool output= false;
      unsigned int wnum= mapwires[k];
      if (wnum >= out_boundary)
        {
          output= true;
        }
      else
        {
          for (unsigned int g= 0; g < C.get_nGates(); g++)
            {
              if (output == false && find(mapgates.begin(), mapgates.end(), g) == mapgates.end())
                {
                  for (unsigned int j= 0; j < cnt_numI(C.get_GateType(g)); j++)
                    {
                      if (C.Gate_Wire_In(g, j) == wnum)
                        {
                          output= true;
                        }
                    }
                }
            }
        }
      unsigned int te= nw - Sub.numO[0] - 1;
      if (output)
        {
          if (te != k)
            { // Swap wire nw-Sub.numO[0]-1 with wire k
              for (unsigned int g= 0; g < Sub.get_nGates(); g++)
                {
                  for (unsigned int j= 0; j < cnt_numI(Sub.GateT[g]); j++)
                    {
                      if (Sub.GateI[g][j] == k)
                        {
                          Sub.GateI[g][j]= te;
                        }
                      else if (Sub.GateI[g][j] == te)
                        {
                          Sub.GateI[g][j]= k;
                        }
                    }
                  if (Sub.GateO[g] == k)
                    {
                      Sub.GateO[g]= te;
                    }
                  else if (Sub.GateO[g] == te)
                    {
                      Sub.GateO[g]= k;
                    }
                }
              // Decrement k as we have a new wire in this position
              swap(mapwires[k], mapwires[te]);
              k--;
            }
          Sub.numO[0]++;
        }
    }
  // Tidy up
  Sub.recompute_map();
}

/* This finds the function for the given input circuit
 * with one input wire
 */
void Find_Function_One(Circuit &F, const Circuit &Sub)
{
  vector<vector<int>> inp;
  vector<vector<vector<int>>> outp;
  inp.resize(1);
  outp.resize(2);
  inp[0].resize(1);

  bool verbose= false;

  if (verbose)
    {
      cout << "Function Signature " << endl;
    }

  vector<int> code(Sub.num_oWires(0));
  for (unsigned int i= 0; i < Sub.num_oWires(0); i++)
    {
      code[i]= 0;
    }
  for (int b= 0; b < 2; b++)
    {
      outp[b].resize(1);
      outp[b][0].resize(Sub.num_oWires(0));
      inp[0][0]= b;
      Sub.evaluate(inp, outp[b]);
      if (verbose)
        {
          cout << b << " : ";
        }
      for (unsigned int i= 0; i < Sub.num_oWires(0); i++)
        {
          if (verbose)
            {
              cout << outp[b][0][i] << " ";
            }
          code[i]= code[i] * 2 + outp[b][0][i];
        }
      if (verbose)
        {
          cout << endl;
        }
    }
  if (verbose)
    {
      cout << endl;
      cout << "Code : ";
      for (unsigned int i= 0; i < Sub.num_oWires(0); i++)
        {
          cout << code[i] << " ";
        }
      cout << endl
           << endl;
    }

  // We now construct circuit F which gives the same function
  // in a canonical form
  F.nWires= 1 + Sub.num_oWires(0);
  F.numI.resize(1);
  F.numI[0]= 1;
  F.numO.resize(1);
  F.numO[0]= Sub.num_oWires(0);
  F.GateT.resize(Sub.num_oWires(0));
  F.GateI.resize(Sub.num_oWires(0));
  F.GateO.resize(Sub.num_oWires(0));
  for (unsigned int i= 0; i < Sub.num_oWires(0); i++)
    {
      F.GateO[i]= i + 1;
      F.GateI[i].resize(2);
      if (code[i] == 0)
        { // EQ 0 Gate
          F.GateT[i]= EQ;
          F.GateI[i][0]= 0;
        }
      else if (code[i] == 1)
        { // EQW Gate
          F.GateT[i]= EQW;
          F.GateI[i][0]= 0;
        }
      else if (code[i] == 2)
        { // INV Gate
          F.GateT[i]= INV;
          F.GateI[i][0]= 0;
        }
      else if (code[i] == 3)
        { // EQ 1 Gate
          F.GateT[i]= EQ;
          F.GateI[i][0]= 1;
        }
      else
        {
          throw bad_value();
        }
    }
  F.num_AND= 0;
  F.recompute_map();

  if (verbose)
    {
      cout << F << endl;
    }
}

/* This finds the function for the given input circuit
 * with one input wire
 */
void Find_Function_Two(Circuit &F, const Circuit &Sub)
{
  vector<vector<int>> inp;
  vector<vector<vector<int>>> outp;
  inp.resize(1);
  outp.resize(4);
  inp[0].resize(2);

  bool verbose= false;

  if (verbose)
    {
      cout << "Function Signature " << endl;
    }
  vector<int> code(Sub.num_oWires(0));
  for (unsigned int i= 0; i < Sub.num_oWires(0); i++)
    {
      code[i]= 0;
    }
  unsigned int cnt= 0;
  for (int b0= 0; b0 < 2; b0++)
    {
      for (int b1= 0; b1 < 2; b1++)
        {
          outp[cnt].resize(1);
          outp[cnt][0].resize(Sub.num_oWires(0));
          inp[0][0]= b0;
          inp[0][1]= b1;
          Sub.evaluate(inp, outp[cnt]);
          if (verbose)
            {
              cout << cnt << " : " << b0 << " " << b1 << " : ";
            }
          for (unsigned int i= 0; i < Sub.num_oWires(0); i++)
            {
              if (verbose)
                {
                  cout << outp[cnt][0][i] << " ";
                }
              code[i]= code[i] * 2 + outp[cnt][0][i];
            }
          if (verbose)
            {
              cout << endl;
            }
          cnt++;
        }
    }
  if (verbose)
    {
      cout << endl;
      cout << "Code : ";
      for (unsigned int i= 0; i < Sub.num_oWires(0); i++)
        {
          cout << code[i] << " ";
        }
      cout << endl
           << endl;
    }

  // We now construct circuit F which gives the same function
  // in a canonical form. We do this in a redundant manner
  // which then allows us to simplify later
  //   i.e. We add more gates than strictly needed,
  //   as we will then get rid of ones which are not used
  F.nWires= 6 + Sub.num_oWires(0);
  F.numI.resize(1);
  F.numI[0]= 2;
  F.numO.resize(1);
  F.numO[0]= Sub.num_oWires(0);
  F.GateT.resize(4 + Sub.num_oWires(0));
  F.GateI.resize(4 + Sub.num_oWires(0));
  F.GateO.resize(4 + Sub.num_oWires(0));

  // INV w0
  F.GateO[0]= 2;
  F.GateT[0]= INV;
  F.GateI[0].resize(2);
  F.GateI[0][0]= 0;
  // INV w1
  F.GateO[1]= 3;
  F.GateT[1]= INV;
  F.GateI[1].resize(2);
  F.GateI[1][0]= 1;

  // w0 AND w1
  F.GateO[2]= 4;
  F.GateT[2]= AND;
  F.GateI[2].resize(2);
  F.GateI[2][0]= 0;
  F.GateI[2][1]= 1;

  // w0 XOR w1
  F.GateO[3]= 5;
  F.GateT[3]= XOR;
  F.GateI[3].resize(2);
  F.GateI[3][0]= 0;
  F.GateI[3][1]= 1;

  for (unsigned int i= 0; i < Sub.num_oWires(0); i++)
    {
      F.GateO[i + 4]= i + 6;
      F.GateI[i + 4].resize(2);
      if (code[i] == 0)
        { // EQ 0 Gate
          F.GateT[i + 4]= EQ;
          F.GateI[i + 4][0]= 0;
        }
      else if (code[i] == 1)
        { // w0 AND w1
          F.GateT[i + 4]= AND;
          F.GateI[i + 4][0]= 0;
          F.GateI[i + 4][1]= 1;
        }
      else if (code[i] == 2)
        { // w0 AND (not w1)
          F.GateT[i + 4]= AND;
          F.GateI[i + 4][0]= 0;
          F.GateI[i + 4][1]= 3;
        }
      else if (code[i] == 3)
        { // EQ w0
          F.GateT[i + 4]= EQW;
          F.GateI[i + 4][0]= 0;
        }
      else if (code[i] == 4)
        { // w1 AND (not w0)
          F.GateT[i + 4]= AND;
          F.GateI[i + 4][0]= 1;
          F.GateI[i + 4][1]= 2;
        }
      else if (code[i] == 5)
        { // EQ w1
          F.GateT[i + 4]= EQW;
          F.GateI[i + 4][0]= 1;
        }
      else if (code[i] == 6)
        { // w0 XOR w1
          F.GateT[i + 4]= XOR;
          F.GateI[i + 4][0]= 0;
          F.GateI[i + 4][1]= 1;
        }
      else if (code[i] == 7)
        { // (w0 XOR w1) XOR (w0 AND w1)
          F.GateT[i + 4]= XOR;
          F.GateI[i + 4][0]= 5;
          F.GateI[i + 4][1]= 4;
        }
      else if (code[i] == 8)
        { // (not w0) AND (not w1)
          F.GateT[i + 4]= AND;
          F.GateI[i + 4][0]= 2;
          F.GateI[i + 4][1]= 3;
        }
      else if (code[i] == 9)
        { // not (w0 XOR w1)
          F.GateT[i + 4]= INV;
          F.GateI[i + 4][0]= 5;
        }
      else if (code[i] == 10)
        { // not w1
          F.GateT[i + 4]= EQW;
          F.GateI[i + 4][0]= 3;
        }
      else if (code[i] == 11)
        { // (not w1) XOR (w0 AND w1)
          F.GateT[i + 4]= XOR;
          F.GateI[i + 4][0]= 3;
          F.GateI[i + 4][1]= 4;
        }
      else if (code[i] == 12)
        { // not w0
          F.GateT[i + 4]= EQW;
          F.GateI[i + 4][0]= 2;
        }
      else if (code[i] == 13)
        { // (not w0) XOR (w0 AND w1)
          F.GateT[i + 4]= XOR;
          F.GateI[i + 4][0]= 2;
          F.GateI[i + 4][1]= 4;
        }
      else if (code[i] == 14)
        { // not (w0 AND w1)
          F.GateT[i + 4]= INV;
          F.GateI[i + 4][0]= 4;
        }
      else if (code[i] == 15)
        { // EQ 1
          F.GateT[i + 4]= EQ;
          F.GateI[i + 4][0]= 1;
        }
      else
        {
          throw bad_value();
        }
    }
  F.recompute_map();

  if (verbose)
    {
      cout << F << endl;
    }
}

bool SimplifyCircuit::Insert(const Circuit &F, const Circuit &Sub,
                             const vector<unsigned int> &mapwires,
                             const vector<unsigned int> &mapgates)
{
  SimplifyCircuit CC;
  CC.assign(F);
  CC.Simplify();
  Circuit S_F= CC.Get_Circuit();

  unsigned int nG_S= Sub.get_nGates();
  unsigned int nG_F= S_F.get_nGates();

  /* If nG_S > nG_F then nW_S > nW_F and vice-versa */
  if (nG_S > nG_F)
    {
      bool verbose= false;
      /*if (C.get_nGates()<2456) 
         { verbose=true; }
      */

      if (verbose)
        {
          cout << "nG_S nG_F : " << nG_S << " " << nG_F << endl;
          cout << "==========================================\n";
          cout << "Found sub-circuit\n"
               << Sub << endl;
          cout << "Replacement circuit will be...\n"
               << F << endl;
          cout << "The simplified sub-circuit will be \n"
               << S_F << endl;
        }

      // Work out mapping from S_F's to C's wires
      vector<unsigned int> new_map_wires(S_F.get_nWires());
      // First do input wires
      for (unsigned int i= 0; i < S_F.num_iWires(0); i++)
        {
          new_map_wires[i]= mapwires[i];
        }
      // Now do output wires
      for (unsigned int i= 0; i < S_F.num_oWires(0); i++)
        {
          new_map_wires[S_F.get_nWires() - S_F.num_oWires(0) + i]= mapwires[Sub.get_nWires() - Sub.num_oWires(0) + i];
        }

      // Now rest
      unsigned int number_internal_S_F= S_F.get_nWires() - S_F.num_oWires(0) - S_F.num_iWires(0);
      for (unsigned int i= 0; i < number_internal_S_F; i++)
        {
          new_map_wires[i + S_F.num_iWires(0)]= mapwires[i + S_F.num_iWires(0)];
        }
      // This should be a wire which is not used

      if (verbose)
        {
          cout << "Map Wires" << endl;
          for (unsigned int i= 0; i < mapwires.size(); i++)
            {
              cout << i << "->" << mapwires[i] << endl;
            }

          cout << "New Map Wires" << endl;
          for (unsigned int i= 0; i < new_map_wires.size(); i++)
            {
              cout << i << "->" << new_map_wires[i] << endl;
            }
        }

      // Now go through C and replace the gates which I am going to keep
      if (verbose)
        {
          cout << "Gates I am keeping" << endl;
        }
      for (unsigned int i= 0; i < nG_F; i++)
        {
          unsigned int old_g= mapgates[i];
          C.GateT[old_g]= S_F.GateT[i];
          C.GateO[old_g]= new_map_wires[S_F.GateO[i]];
          if (verbose)
            {
              cout << "Gate " << old_g << " :\tType " << C.GateT[old_g] << endl;
              cout << "\tInput wires: ";
            }
          if (S_F.GateT[i] != EQ)
            {
              for (unsigned int j= 0; j < cnt_numI(C.GateT[old_g]); j++)
                {
                  C.GateI[old_g][j]= new_map_wires[S_F.GateI[i][j]];
                  if (verbose)
                    {
                      cout << C.GateI[old_g][j] << " ";
                    }
                }
            }
          else
            {
              C.GateI[old_g][0]= S_F.GateI[i][0];
            }
          if (verbose)
            {
              cout << "\n\tOutput wire: " << C.GateO[old_g] << endl;
            }
        }
      // Now go through the gates we are going to get rid of
      if (verbose)
        {
          cout << "Gates I am removing" << endl;
        }
      for (unsigned int i= nG_F; i < nG_S; i++)
        {
          unsigned int old_g= mapgates[i];
          // Fix output wire to something which will not be used
          C.GateO[old_g]= mapwires[number_internal_S_F + S_F.num_iWires(0) + i - nG_F];
          if (verbose)
            {
              cout << "Gate " << old_g << " :\tType " << C.GateT[old_g] << endl;
              cout << "\tInput wires: ";
            }
          // Fix input wire to get to be any old input wire to the sub-circuit
          for (unsigned int j= 0; j < cnt_numI(C.GateT[old_g]); j++)
            {
              C.GateI[old_g][j]= new_map_wires[j];
              if (verbose)
                {
                  cout << C.GateI[old_g][j] << " ";
                }
            }
          if (verbose)
            {
              cout << "\n\tOutput wire: " << C.GateO[old_g] << endl;
            }
        }
      C.sort();
      return true;
    }
  return false;
}

bool SimplifyCircuit::Search_SubCircuit()
{
  vector<bool> used(C.get_nWires());
  vector<unsigned int> mapwires, mapgates;

  unsigned int out_boundary= C.get_nWires();
  for (unsigned int i= 0; i < C.num_outputs(); i++)
    {
      out_boundary-= C.num_oWires(i);
    }

  Circuit F;

  // First look at one input sub-circuits
  for (unsigned int i0= 0; i0 < out_boundary; i0++)
    {
      Circuit Sub;
      Sub.numI.resize(1);
      Sub.numI[0]= 1;
      Sub.numO.resize(1);
      Sub.num_AND= 0;
      for (unsigned int i= 0; i < C.get_nWires(); i++)
        {
          used[i]= false;
        }
      used[i0]= true;
      mapwires.resize(1);
      mapwires[0]= i0;
      mapgates.resize(0);

      Create_SubCircuit(1, used, mapwires, mapgates, Sub);
      if (Sub.get_nGates() > 1)
        {
          Find_Function_One(F, Sub);
          if (Insert(F, Sub, mapwires, mapgates))
            {
              return true;
            }
        }
    }

  // Look at two input sub-circuits
  for (unsigned int i0= 0; i0 < out_boundary; i0++)
    {
      for (unsigned int j0= i0 + 1; j0 < out_boundary; j0++)
        {
          Circuit Sub;
          Sub.numI.resize(1);
          Sub.numI[0]= 2;
          Sub.numO.resize(1);
          Sub.num_AND= 0;
          for (unsigned int i= 0; i < C.get_nWires(); i++)
            {
              used[i]= false;
            }
          used[i0]= true;
          used[j0]= true;
          mapwires.resize(2);
          mapwires[0]= i0;
          mapwires[1]= j0;
          mapgates.resize(0);

          Create_SubCircuit(2, used, mapwires, mapgates, Sub);
          if (Sub.get_nGates() > 1)
            {
              Find_Function_Two(F, Sub);
              if (Insert(F, Sub, mapwires, mapgates))
                {
                  return true;
                }
            }
        }
    }
  return false;
}
