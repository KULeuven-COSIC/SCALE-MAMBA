/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _SimplifyCircuit
#define _SimplifyCircuit

/* Class to perform Circuit simplification */

#include "GC/Circuit.h"

class SimplifyCircuit
{
  Circuit C;

  void Create_SubCircuit(unsigned int numi, vector<bool> &used,
                         vector<unsigned int> &mapwires,
                         vector<unsigned int> &mapgates,
                         Circuit &Sub);

  // Returns true if we inserted F/Sub into C
  bool Insert(const Circuit &F, const Circuit &Sub,
              const vector<unsigned int> &mapwires,
              const vector<unsigned int> &mapgates);

  /*
   * Remove EQW gates for optimizing circuit based on the following rules;
   * in: input wire, out: output wire, inter: intermediate wire
   * in out EQW        : do nothing
   * in inter EQW      : replace inter with in
   * inter1 inter2 EQW : replace inter2 with inter1
   * inter out EQW     : replace inter with out
   */
  void Remove_EQW();

  /* Removes duplicate gates
   * May need to be run repeatedly to completely eliminate gates
   * Returns true if any gate is deleted
   */
  bool remove_duplicate_gates();

  /* This goes through all wires i and removes them if
   * they are not input to a gate, or output of the circuit.
   * This also needs to delete gates as well, if such a
   * wire is the output of a gate
   */
  void remove_redundant_wires(bool verbose= false);

public:
  void assign(const Circuit &CC)
  {
    C= CC;
  }

  SimplifyCircuit()
  {
    ;
  }

  SimplifyCircuit(const Circuit &CC)
  {
    assign(CC);
  }

  Circuit Get_Circuit() const
  {
    return C;
  }

  // Apply the simplification rules implied by any
  //    EQ 0 x
  //    EQ 1 x
  // instructions. Returns true if something has been done
  // Applies the optimization only to gates of type T
  bool ApplyAssignment(const GateType &T, bool verbose= false);

  // Change circuit to add in a bunch of new
  //   EQ 0 x
  //   EQ 1 x
  // gates according to the array set_input
  //    set_input=0,1 or 2  (With 2 meaning *)
  // Should probably run simplify after this
  void Set_Inputs(const vector<unsigned int> &set_input);

  // Remove output wires if we want to ignore them
  //    set_output= 0 to keep
  //                1 to throw away
  // Should probably run Simplify after this
  void Remove_Outputs(const vector<unsigned int> &remove_output);

  // This is the main simplify routine
  void Simplify();

  // This routine searchs for sub-circuits with two input wires
  // which can be made simplify. Does this in a step wise
  // manner. Should simplify on output of true (probably check is
  // all OK, and then call again)
  //   - Problem is this recursion can take ages, so perhaps
  //     best to limit it in some way
  bool Search_SubCircuit();
};

#endif
