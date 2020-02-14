/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _Online
#define _Online

#include "Offline/offline_data.h"
#include "Online/Machine.h"
#include "System/Player.h"

void online_phase(int online_num, Player &P, offline_control_data &OCD,
                  Machine &machine);

#endif
