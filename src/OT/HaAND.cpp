/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#include "HaAND.h"
#include "OT_Thread_Data.h"
#include "Tools/MMO.h"

void HaAND::make_more(Player &P, int num_online)
{
  extern OT_Thread_Data OTD;

  //P.clocks[0].reset(); P.clocks[0].start(); cout << "\t\t\tIn HaAND" << endl;
  unsigned int n= P.nplayers();

  unsigned int number= 32768;
  x.resize(number);
  y.resize(number);
  v.resize(number);
  HKi.resize(number, vector<gf2n>(n));
  HKiD.resize(number, vector<gf2n>(n));
  HMi.resize(number, vector<gf2n>(n));

  list<aBit> xL= OTD.aBD.get_aShares(num_online, number);
  list<aBit> yL= OTD.aBD.get_aShares(num_online, number);

  x.assign(xL.begin(), xL.end());
  y.assign(yL.begin(), yL.end());
  gf2n te, Delta= aBit::get_Delta();

  vector<vector<int>> s(number, vector<int>(n));
  vector<vector<int>> t(number, vector<int>(n));

  vector<string> o(n);
  uint8_t array[OT_Amort * 16];
  MMO mmo;
  for (unsigned int i= 0; i < n; i++)
    {
      if (i != P.whoami())
        {
          for (unsigned int jj= 0; jj < number; jj+= OT_Amort)
            {
              // Make HKi
              for (unsigned int j= 0; j < OT_Amort; j++)
                {
                  x[jj + j].get_Key(i).store_into_buffer(&(array[j * 16]));
                }
              mmo.hashBlockWise<gf2n, OT_Amort>(array, array);
              for (unsigned int j= 0; j < OT_Amort; j++)
                {
                  HKi[jj + j][i].load_from_buffer(&(array[16 * j]));
                }

              // Make HKiD
              for (unsigned int j= 0; j < OT_Amort; j++)
                {
                  te= x[jj + j].get_Key(i);
                  te.add(Delta);
                  te.store_into_buffer(&(array[j * 16]));
                }
              mmo.hashBlockWise<gf2n, OT_Amort>(array, array);
              for (unsigned int j= 0; j < OT_Amort; j++)
                {
                  HKiD[jj + j][i].load_from_buffer(&(array[16 * j]));
                }

              // Make HMi
              for (unsigned int j= 0; j < OT_Amort; j++)
                {
                  x[jj + j].get_MAC(i).store_into_buffer(&(array[j * 16]));
                }
              mmo.hashBlockWise<gf2n, OT_Amort>(array, array);
              for (unsigned int j= 0; j < OT_Amort; j++)
                {
                  HMi[jj + j][i].load_from_buffer(&(array[16 * j]));
                }
            }

          // Compute sjk values
          for (unsigned int k= 0; k < number / 8; k++)
            {
              uint8_t a= P.G.get_uchar();
              for (unsigned int kk= 0; kk < 8; kk++)
                {
                  s[k * 8 + kk][i]= a & 1;
                  a>>= 1;
                }
            }

          ostringstream oo;
          for (unsigned int k= 0; k < number; k++)
            {
              int h0= HKi[k][i].get_bit(0) ^ s[k][i];
              int h1= HKiD[k][i].get_bit(0) ^ s[k][i] ^ y[k].get_value();
              oo << h0 << " " << h1 << endl;
            }
          o[i]= oo.str();
        }
    }

  P.Send_Distinct_And_Receive(o, 2);

  for (unsigned int i= 0; i < n; i++)
    {
      if (i != P.whoami())
        {
          istringstream iss(o[i]);
          int h0, h1, h;
          for (unsigned int k= 0; k < number; k++)
            {
              iss >> h0 >> h1;
              h= h1;
              if (x[k].get_value() == 0)
                {
                  h= h0;
                }
              t[k][i]= h ^ HMi[k][i].get_bit(0);
            }
        }
    }

  for (unsigned int i= 0; i < number; i++)
    {
      v[i]= 0;
      for (unsigned int k= 0; k < n; k++)
        {
          if (k != P.whoami())
            {
              v[i]^= (t[i][k] ^ s[i][k]);
            }
        }
    }
  //P.clocks[0].stop(); cout << "\t\t\tHaAND " << P.clocks[0].elapsed() << endl;
}
