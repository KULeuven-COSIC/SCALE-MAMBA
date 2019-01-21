/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "GC/Circuit.h"
#include <fstream>
#include <iostream>
using namespace std;

int main()
{
  /* Test a Garbled Circuit */
  ifstream inpf("Circuits/AES-non-expanded.txt");
  Circuit C;
  inpf >> C;

  vector<vector<int>> inp, outp;
  inp.resize(2);
  outp.resize(1);
  inp[0].resize(128);
  inp[1].resize(128);
  outp[0].resize(128);
  for (int i= 0; i < 128; i++)
    {
      inp[0][i]= 1;
      inp[1][i]= 0;
    }

  C.evaluate(inp, outp);

  for (int i= 0; i < 128; i++)
    {
      cout << outp[0][i];
    }
  cout << endl;
  int byte;
  for (int i= 0; i < 16; i++)
    {
      byte= 0;
      for (int j= 0; j < 8; j++)
        {
          byte<<= 1;
          byte+= outp[0][i * 8 + j];
        }
      printf("%02X", byte);
    }
  cout << "\nShould be\n";
  cout << "3f5b8cc9ea855a0afa7347d23e8d664e" << endl;
}
