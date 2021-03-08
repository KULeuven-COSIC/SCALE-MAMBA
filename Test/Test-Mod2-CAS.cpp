/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "LSSS/CAS.h"
#include "LSSS/ShareData2.h"

int main()
{
  // Threshhold replicated mod 2
  CAS AS;
  AS.assign(3, 1);

  cout << AS << endl;

  cout << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n"
       << endl;

  MSP<gf2> M;
  M.Initialize_Replicated(AS);

  cout << M << endl;

  cout << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n"
       << endl;

  ShareData2 SD;

  SD.Initialize_Replicated(AS);

  cout << SD << endl;

  return 0;
}
