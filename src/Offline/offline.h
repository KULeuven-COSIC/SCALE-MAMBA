/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _OFFLINE
#define _OFFLINE

#include "FHE_Factory.h"
#include "FakePrep.h"
#include "LSSS/Open_Protocol.h"
#include "LSSS/PRSS.h"
#include "LSSS/PRZS.h"
#include "System/Player.h"
#include <list>

void offline_phase_triples(Player &P, PRSS &prss, PRZS &przs, FakePrep &prep,
                           list<Share> &a, list<Share> &b, list<Share> &c,
                           const FHE_PK &pk, const FHE_SK &sk,
                           const FFT_Data &PTD,
                           int fake_sacrifice, Open_Protocol &OP,
                           FHE_Industry &industry);

void offline_phase_squares(Player &P, PRSS &prss, PRZS &przs, FakePrep &prep,
                           list<Share> &a, list<Share> &b,
                           const FHE_PK &pk, const FHE_SK &sk,
                           const FFT_Data &PTD,
                           int fake_sacrifice, Open_Protocol &OP,
                           FHE_Industry &industry);

void offline_phase_bits(Player &P, PRSS &prss, PRZS &przs, FakePrep &prep,
                        list<Share> &b,
                        const FHE_PK &pk, const FHE_SK &sk,
                        const FFT_Data &PTD,
                        int fake_sacrifice, Open_Protocol &OP,
                        FHE_Industry &industry);

#endif
