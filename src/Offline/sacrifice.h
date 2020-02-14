/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _SACRIFICE
#define _SACRIFICE

#include "LSSS/Open_Protocol.h"
#include <list>

void sacrifice_phase_triples(Player &P, int fake_sacrifice, list<Share> &a,
                             list<Share> &b, list<Share> &c, Open_Protocol &OP);

void sacrifice_phase_squares(Player &P, int fake_sacrifice, list<Share> &a,
                             list<Share> &b, Open_Protocol &OP);

void sacrifice_phase_bits(Player &P, int fake_sacrifice, list<Share> &bit,
                          list<Share> a, list<Share> &sa, Open_Protocol &OP);

#endif
