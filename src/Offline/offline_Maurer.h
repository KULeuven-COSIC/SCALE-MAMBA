/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _Offline_Maurer
#define _Offline_Maurer

#include <list>
using namespace std;

#include "LSSS/Open_Protocol.h"
#include "LSSS/PRSS.h"

void offline_Maurer_triples(Player &P, PRSS &prss, list<Share> &a,
                            list<Share> &b, list<Share> &c);

void offline_Maurer_squares(Player &P, PRSS &prss, list<Share> &a,
                            list<Share> &b,
                            unsigned int rep= 1);

void offline_Maurer_bits(Player &P, PRSS &prss, list<Share> &b,
                         Open_Protocol &OP);

#endif
