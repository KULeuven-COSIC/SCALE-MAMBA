/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _Q2Eval
#define _Q2Eval

/* 
 * Evaluate a circuit using the Q2 Mod2Engine machinary
 */

#include "Circuit.h"
#include "LSSS/Open_Protocol2.h"

// Note: The vector output is resized by this function
//       and so can be blank on entry
void Evaluate(vector<vector<Share2>> &output,
              const vector<vector<Share2>> &input,
              const Circuit &C,
              Player &P,
              unsigned int online_thread_num);

#endif
