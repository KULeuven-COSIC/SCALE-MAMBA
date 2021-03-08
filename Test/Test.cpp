/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

/* General test file for LSSS schemes */

#include "LSSS/CAS.h"
#include "LSSS/ShareData.h"

int main()
{

  // Test the problamatic example

  imatrix P(2, vector<int>(5));
  for (int i= 0; i < 2; i++)
    {
      for (int j= 0; j < 5; j++)
        {
          P[i][j]= 0;
        }
    }
  P[0][0]= 1;
  P[0][1]= 1;
  P[1][2]= 1;
  P[1][3]= 1;
  P[1][4]= 1;

  cout << "Input Matrix\n";
  print(P);
  cout << endl;

  CAS ASP(P, false);

  cout << "CAS Structure\n";
  cout << ASP << endl;
  for (unsigned int l= 1; l < 4; l++)
    {
      cout << "l = " << l << " : Q_l = " << ASP.is_q(l) << endl;
    }

  cout << "----------------------------------------------\n";

  // Test a general example; input delta_plus
  imatrix A(4, vector<int>(4));
  for (int i= 0; i < 4; i++)
    {
      for (int j= 0; j < 4; j++)
        {
          A[i][j]= 0;
        }
    }
  A[0][0]= 1;
  A[0][1]= 1;
  A[1][0]= 1;
  A[1][2]= 1;
  A[2][0]= 1;
  A[2][3]= 1;
  A[3][1]= 1;
  A[3][2]= 1;
  A[3][3]= 1;

  cout << "Input Matrix\n";
  print(A);
  cout << endl;

  CAS AS(A, true);

  cout << "CAS Structure\n";
  cout << AS << endl;
  for (unsigned int l= 1; l < 4; l++)
    {
      cout << "l = " << l << " : Q_l = " << AS.is_q(l) << endl;
    }

  cout << "----------------------------------------------\n";

  // Test a general example; input gamma_minus
  imatrix B(4, vector<int>(4));
  for (int i= 0; i < 4; i++)
    {
      for (int j= 0; j < 4; j++)
        {
          B[i][j]= 0;
        }
    }
  B[0][0]= 1;
  B[0][1]= 1;
  B[1][0]= 1;
  B[1][2]= 1;
  B[2][0]= 1;
  B[2][3]= 1;
  B[3][1]= 1;
  B[3][2]= 1;
  B[3][3]= 1;

  cout << "Input Matrix\n";
  print(B);
  cout << endl;

  CAS ASB(B, false);

  cout << "CAS Structure\n";
  cout << ASB << endl;
  for (unsigned int l= 1; l < 4; l++)
    {
      cout << "l = " << l << " : Q_l = " << ASB.is_q(l) << endl;
    }

  cout << "----------------------------------------------\n";

  // Example with no sets_to_parties map
  imatrix C(3, vector<int>(4));
  for (int i= 0; i < 3; i++)
    {
      for (int j= 0; j < 4; j++)
        {
          C[i][j]= 0;
        }
    }
  C[0][0]= 1;
  C[0][1]= 1;
  C[1][0]= 1;
  C[1][2]= 1;
  C[2][0]= 1;
  C[2][3]= 1;

  cout << "Input Matrix\n";
  print(C);
  cout << endl;

  CAS ASC(C, true);

  cout << "CAS Structure\n";
  cout << ASC << endl;
  for (unsigned int l= 1; l < 3; l++)
    {
      cout << "l = " << l << " : Q_l = " << ASC.is_q(l) << endl;
    }

  cout << "----------------------------------------------\n";

  //
  imatrix D(1, vector<int>(4));
  for (int i= 0; i < 1; i++)
    {
      for (int j= 0; j < 4; j++)
        {
          D[i][j]= 0;
        }
    }
  D[0][0]= 1;
  D[0][1]= 1;

  cout << "Input Matrix\n";
  print(D);
  cout << endl;

  CAS ASD(D, true);

  cout << "CAS Structure\n";
  cout << ASD << endl;
  for (unsigned int l= 1; l < 2; l++)
    {
      cout << "l = " << l << " : Q_l = " << ASD.is_q(l) << endl;
    }

  cout << "----------------------------------------------\n";

  //
  imatrix E(1, vector<int>(4));
  for (int i= 0; i < 1; i++)
    {
      for (int j= 0; j < 4; j++)
        {
          E[i][j]= 0;
        }
    }
  E[0][0]= 1;

  cout << "Input Matrix\n";
  print(E);
  cout << endl;

  CAS ASE(E, false);

  cout << "CAS Structure\n";
  cout << ASE << endl;
  for (unsigned int l= 1; l < 2; l++)
    {
      cout << "l = " << l << " : Q_l = " << ASE.is_q(l) << endl;
    }

  cout << "----------------------------------------------\n";

  for (unsigned int n= 3; n < 6; n++)
    {
      for (unsigned int t= 1; t < n / 2 + 1; t++)
        {
          cout << "N=" << n << "   T=" << t << endl;
          CAS AS(n, t);

          cout << "CAS Structure\n";
          cout << AS << endl;

          for (unsigned int l= 1; l < n; l++)
            {
              cout << "l = " << l << " : Q_l = " << AS.is_q(l) << endl;
            }

          cout << "----------------------------------------------\n";
        }
    }

  // Test another problematic example

  imatrix Q(2, vector<int>(4));
  for (int i= 0; i < 2; i++)
    {
      for (int j= 0; j < 4; j++)
        {
          Q[i][j]= 0;
        }
    }
  Q[0][0]= 1;
  Q[0][1]= 1;
  Q[1][0]= 1;
  Q[1][2]= 1;
  Q[1][3]= 1;

  cout << "Input Matrix\n";
  print(Q);
  cout << endl;

  CAS ASQ(Q, false);

  cout << "CAS Structure\n";
  cout << ASQ << endl;
  for (unsigned int l= 1; l < 4; l++)
    {
      cout << "l = " << l << " : Q_l = " << ASQ.is_q(l) << endl;
    }

  // Test General Stuff

  gfp::init_field(65537);
  ShareData SD;
  SD.Initialize_Replicated(ASD, Maurer);

  cout << "----------------------------------------------\n";
  return 0;
}
