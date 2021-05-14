/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _Mod2Maurer
#define _Mod2Maurer

#include <vector>
using namespace std;

#include "LSSS/Open_Protocol2.h"
#include "LSSS/PRSS.h"
#include "System/Player.h"

/* Generate triples using Maurers method for replicated
 * sharing mod 2
 *   - Produces batch_size in one go
 * NOTE THIS IS NOT THE MOST EFFICIENT MULTIPLIER WHEN
 * REPLICATED AND (n,t)=(3,1) etc
 */
void offline_Maurer_triples(Player &P, PRSS2 &prss,
                            vector<vector<Share2>> &triples,
                            unsigned int batch_size);

/* Generate the checked triples
 * Protocol 3.1 of Araki et al SP2017
 * Generates N = (X-C)*L triples
 * Security is (B-1)*log_2 N
 */
void Gen_Checked_Triples(Player &P, PRSS2 &prss,
                         vector<vector<Share2>> &triples,
                         unsigned int B,
                         unsigned int C,
                         unsigned int X,
                         unsigned int L);

/* For debugging */
void check_triples(vector<vector<Share2>> &triples, Player &P);

/* Used to multiply two Share2's using a single Triple T 
 *  - This assumes the Share2's hold single bits!!!!!
 */
void Mult_Bit(Share2 &z, const Share2 &x, const Share2 &y,
              const vector<Share2> &T, Player &P);

/* Same but now we do 64 in parallel */
void Mult_Bits(Share2 &z, const Share2 &x, const Share2 &y,
               const vector<Share2> &T, Player &P);

#endif
