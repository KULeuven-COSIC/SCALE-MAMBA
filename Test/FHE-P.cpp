/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "FHE/FHE_Params.h"

int main()
{
  unsigned int N;
  bigint p0, p1, p;
  unsigned int hwt= 64;

  int lg2p;
  cout << "Enter plaintext modulus size " << endl;
  cin >> lg2p;

  int newHB;
  cout << "Enter the NewHope loop bound" << endl;
  cin >> newHB;

  vector<int> sec= {1, 40, 80, 128};

  for (int s1= 2; s1 < 4; s1++)
    {
      for (int s2= 1; s2 < 4; s2++)
        {
          for (int s4= 0; s4 < 4; s4++)
            {
              for (int s3= s4; s3 < 4; s3++)
                {
                  cout << "\nn = 2: Comp_Sec = " << sec[s1] << ": DD_stat_sec = " << sec[s2];
                  cout << ": ZK_sound_sec = " << sec[s3] << ": ZK_slack_sec=" << sec[s4] << endl;
                  p= 0;
                  Generate_Parameters(N, p0, p1, p, lg2p, 2, TopGear, hwt, newHB, sec[s1], sec[s2], sec[s3], sec[s4]);
                }
            }
        }
    }
}
