/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _Offline_Reduced
#define _Offline_Reduced

#include <list>
using namespace std;

#include "LSSS/Open_Protocol.h"
#include "LSSS/PRSS.h"
#include "LSSS/PRZS.h"

void offline_Reduced_triples(Player &P, PRSS &prss, PRZS &przs, list<Share> &a,
                             list<Share> &b, list<Share> &c);

void offline_Reduced_squares(Player &P, PRSS &prss, PRZS &przs,
                             list<Share> &a, list<Share> &b,
                             unsigned int rep= 1);

void offline_Reduced_bits(Player &P, PRSS &prss, PRZS &przs, list<Share> &b,
                          Open_Protocol &OP);

#endif
