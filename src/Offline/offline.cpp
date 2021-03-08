/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include <fstream>

#include "config.h"
#include "offline.h"
#include "offline_FHE.h"
#include "offline_Maurer.h"
#include "offline_Reduced.h"
#include "offline_subroutines.h"
#include "sacrifice.h"

using namespace std;

void offline_phase_triples(Player &P, PRSS &prss, PRZS &przs, FakePrep &prep, list<Share> &a,
                           list<Share> &b, list<Share> &c, const FHE_PK &pk,
                           const FHE_SK &sk, const FFT_Data &PTD,
                           int fake_sacrifice,
                           FHE_Industry &industry)
{
  if (Share::SD.Otype == Fake)
    {
      prep.produce_triples(a, b, c);
    }
  else if (Share::SD.Otype == Maurer)
    {
      offline_Maurer_triples(P, prss, a, b, c);
    }
  else if (Share::SD.Otype == Reduced)
    {
      offline_Reduced_triples(P, prss, przs, a, b, c);
    }
  else
    {
      offline_FHE_triples(P, a, b, c, pk, sk, PTD, industry);
    }

  sacrifice_phase_triples(P, fake_sacrifice, a, b, c);
}

void offline_phase_squares(Player &P, PRSS &prss, PRZS &przs, FakePrep &prep,
                           list<Share> &a, list<Share> &b,
                           const FHE_PK &pk, const FHE_SK &sk,
                           const FFT_Data &PTD,
                           int fake_sacrifice,
                           FHE_Industry &industry)
{
  if (Share::SD.Otype == Fake)
    {
      prep.produce_squares(a, b);
    }
  else if (Share::SD.Otype == Maurer)
    {
      offline_Maurer_squares(P, prss, a, b);
    }
  else if (Share::SD.Otype == Reduced)
    {
      offline_Reduced_squares(P, prss, przs, a, b);
    }
  else
    {
      offline_FHE_squares(P, a, b, pk, sk, PTD, industry);
    }

  sacrifice_phase_squares(P, fake_sacrifice, a, b);
}

void offline_phase_bits(Player &P, PRSS &prss, PRZS &przs, FakePrep &prep,
                        list<Share> &bits,
                        const FHE_PK &pk, const FHE_SK &sk,
                        const FFT_Data &PTD,
                        int fake_sacrifice,
                        FHE_Industry &industry)
{
  // We need to get rep more squares than bits
  int rep= sacrifice_stat_sec / numBits(gfp::pr()) + 1;

  list<Share> a, b;
  if (Share::SD.Otype == Fake)
    {
      prep.produce_bits(bits);
      prep.produce_squares(a, b, rep);
    }
  else if (Share::SD.Otype == Maurer)
    {
      offline_Maurer_bits(P, prss, bits);
      offline_Maurer_squares(P, prss, a, b, rep);
    }
  else if (Share::SD.Otype == Reduced)
    {
      offline_Reduced_bits(P, prss, przs, bits);
      offline_Reduced_squares(P, prss, przs, a, b, rep);
    }
  else
    {
      offline_FHE_bits(P, bits, pk, sk, PTD, industry);
      offline_FHE_squares(P, a, b, pk, sk, PTD, industry, rep);
    }

  sacrifice_phase_bits(P, fake_sacrifice, bits, a, b);
}
