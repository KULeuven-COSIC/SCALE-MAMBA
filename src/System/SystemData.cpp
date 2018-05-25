/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "SystemData.h"
#include "Exceptions/Exceptions.h"
#include <fstream>

#include <iostream>
SystemData::SystemData()
{
  unsigned int i, j;
  ifstream inp("Data/NetworkData.txt");
  if (inp.fail())
    {
      throw file_error("Data/NetworkData.txt");
    }

  inp >> RootCRT;

  inp >> n;
  cout << "n=" << n << endl;

  IP.resize(n);
  PlayerCRT.resize(n);
  PlayerCN.resize(n);
  for (i= 0; i < n; i++)
    {
      inp >> j >> IP[i] >> PlayerCRT[i] >> PlayerCN[i];
      cout << j << endl;
      cout << IP[i] << endl;
      cout << PlayerCRT[i] << endl;
      cout << PlayerCN[i] << endl;
    }

  inp >> fake_offline;
  inp >> fake_sacrifice;
}
