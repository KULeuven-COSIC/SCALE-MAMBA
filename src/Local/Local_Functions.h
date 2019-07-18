/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2019, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _Local_Function
#define _Local_Function

#include "LSSS/Share.h"
#include "OT/aBitVector.h"
#include <map>

class Local_Functions
{
  map<int, void (*)(vector<long> &Cr_out, vector<aBitVector> &Sr_out,
                    vector<gfp> &Cp_out, vector<Share> &Sp_out,
                    const vector<long> &Cr_inp, const vector<aBitVector> &Sr_inp,
                    const vector<gfp> &Cp_inp, const vector<Share> &Sp_inp)>
      functions;

public:
  Local_Functions();

  /* Note only one of the output vectors should have length !=0
   * At no point does the system check whether instr corresponds to
   * valid sizes in the various vectors. It is up to the function
   * pointed to do to this, and abort if things are wrong
   */
  void apply_Function(int instr,
                      vector<long> &Cr_out, vector<aBitVector> &Sr_out,
                      vector<gfp> &Cp_out, vector<Share> &Sp_out,
                      const vector<long> &Cr_inp, const vector<aBitVector> &Sr_inp,
                      const vector<gfp> &Cp_inp, const vector<Share> &Sp_inp)
  {
    functions[instr](Cr_out, Sr_out, Cp_out, Sp_out,
                     Cr_inp, Sr_inp, Cp_inp, Sp_inp);
  }
};

#endif
