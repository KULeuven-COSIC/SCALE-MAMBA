/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "OT_Thread.h"

#include "Tools/Timer.h"
#include "aAND.h"

// For testing GC stuff
#include "GC/Circuit.h"
#include "GC/Garbled.h"
#include <fstream>

void OT_Thread(Player &P, int verbose)
{
  aBitFactory aBF;

  aBF.Initialize(P);

  aBF.tune(P, verbose);

  /* Test a Garbled Circuit */
  ifstream inpf("Circuits/AES-non-expanded.txt");
  //ifstream inpf("Circuits/Test.txt");
  Circuit C;
  inpf >> C;

  vector<vector<int>> inp(C.num_inputs()), outp(C.num_outputs());
  for (unsigned int j= 0; j < C.num_inputs(); j++)
    {
      inp[j].resize(C.num_iWires(j));
    }
  for (unsigned int j= 0; j < C.num_outputs(); j++)
    {
      outp[j].resize(C.num_oWires(j));
    }
  for (unsigned int i= 0; i < C.num_iWires(0); i++)
    {
      inp[0][i]= 1;
    }
  for (unsigned int i= 0; i < C.num_iWires(1); i++)
    {
      inp[1][i]= 0;
    }

  C.evaluate(inp, outp);

  for (unsigned int i= 0; i < C.num_oWires(0); i++)
    {
      cout << outp[0][i];
    }
  cout << endl;
  int byte;
  for (unsigned int i= 0; i < C.num_oWires(0) / 8; i++)
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

  /* Now Garble a Circuit */
  aAND aA;

  double gcnt= 0;
  Garbled_Circuit GC;
  Timer T1, T2;
  T1.reset();
  T2.reset();

  // Have player zero provide the first input, player one
  // the second, with output going to player two
  vector<int> i_a(2), o_a(1);
  i_a[0]= 0;
  i_a[1]= 1;
  o_a[0]= 2;

  while (0 == 0)
    {
      gcnt++;

      cout << "Garbling a Circuit" << endl;
      double te= T1.elapsed();
      T1.start();
      GC.Garble(C, i_a, o_a, aA, aBF, P);
      T1.stop();
      cout << "Average to garble : " << T1.elapsed() / gcnt << endl;
      cout << "For this garble : " << T1.elapsed() - te << endl;

      cout << "Evaluate the Garbling" << endl;
      vector<vector<int>> input(C.num_inputs());
      for (unsigned int i= 0; i < C.num_inputs(); i++)
        {
          if (i_a[i] == (int) P.whoami())
            {
              input[i].resize(C.num_iWires(i));
              for (unsigned int k= 0; k < input[i].size(); k++)
                {
                  if (P.whoami() == 0)
                    {
                      input[i][k]= 1;
                    }
                  else if (P.whoami() == 1)
                    {
                      input[i][k]= 0;
                    }
                  else
                    {
                      throw bad_value();
                    }
                }
            }
          else
            {
              input[i].resize(0);
            }
        }
      T2.start();
      vector<vector<int>> output;
      GC.Evaluate(output, input, C, P);
      T2.stop();
      cout << "Average to evaluate : " << T2.elapsed() / gcnt << endl;

      cout << "Players output is...\n";
      for (unsigned int p= 0; p < C.num_outputs(); p++)
        {
          cout << "Output : " << p << endl;
          for (unsigned int i= 0; i < output[p].size(); i++)
            {
              cout << output[p][i];
            }
          cout << endl;
          for (unsigned int i= 0; i < output[p].size() / 8; i++)
            {
              byte= 0;
              for (int j= 0; j < 8; j++)
                {
                  byte<<= 1;
                  byte+= output[p][i * 8 + j];
                }
              printf("%02X", byte);
            }
          cout << endl;
        }
    }

  Timer T;
  T.reset();
  T.start();
  unsigned int cnt= 0;
  while (true)
    {
      aA.make_more(aBF, P);
      cnt+= aA.get_number();
      double e= cnt / T.elapsed();
      cout << "# = " << cnt << " :  #/sec = " << e << endl;
    }
}
