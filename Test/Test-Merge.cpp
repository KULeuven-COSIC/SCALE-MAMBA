/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

/* Tests merging AND gates for the circuit evaluation on AES and SHA-3 
 *   Note the old Bristol AES-non-expanded.txt has the wires
 *   in a funny order
 */

#include "GC/Circuit.h"
#include <fstream>
#include <iostream>
using namespace std;

void test_AES128()
{

  cout << "Testing a AES-128 Circuit" << endl;
  Circuit C1, C2;
  ifstream inpf1("Circuits/Bristol/AES-non-expanded.txt");
  inpf1 >> C1;
  inpf1.close();

  ifstream inpf2("Circuits/Bristol/aes_128.txt");
  inpf2 >> C2;
  inpf2.close();

  C1.merge_AND_gates();
  C2.merge_AND_gates();

  vector<vector<vector<int>>> inp;
  vector<vector<vector<int>>> outp;
  inp.resize(2);
  inp[0].resize(2);
  inp[1].resize(2);
  outp.resize(2);
  outp[0].resize(1);
  outp[1].resize(1);
  outp[0][0].resize(128);
  outp[1][0].resize(128);
  inp[0][0].resize(128);
  inp[0][1].resize(128);
  inp[1][0].resize(128);
  inp[1][1].resize(128);
  for (int i= 0; i < 128; i++)
    {
      // First circuit
      inp[0][0][i]= 1; // Message
      inp[0][1][i]= 0; // Key
      // Second circuit
      inp[1][0][i]= 0; // Key
      inp[1][1][i]= 1; // Message
    }

  C1.evaluate(inp[0], outp[0]);
  C2.evaluate(inp[1], outp[1]);

  for (int k= 0; k < 2; k++)
    {
      cout << "\nAES Circuit : " << k << endl;

      for (int i= 0; i < 128; i++)
        {
          cout << outp[k][0][i];
          if ((i & 7) == 7)
            {
              cout << "  ";
            }
          if ((i & 31) == 31)
            {
              cout << "\n";
            }
        }
      cout << endl;
      int byte;
      if (k == 0)
        {
          for (int i= 0; i < 16; i++)
            {
              byte= 0;
              for (int j= 0; j < 8; j++)
                {
                  byte<<= 1;
                  byte+= outp[k][0][i * 8 + j];
                }
              printf("%02X", byte);
            }
        }
      else
        {
          for (int i= 15; i >= 0; i--)
            {
              byte= 0;
              for (int j= 7; j >= 0; j--)
                {
                  byte<<= 1;
                  byte+= outp[k][0][i * 8 + j];
                }
              printf("%02X", byte);
            }
        }
      cout << endl;
    }
  cout << "\nShould be\n";
  cout << "3F5B8CC9EA855A0AFA7347D23E8D664E" << endl
       << endl
       << endl;
}

void test_AES192()
{

  cout << "Testing a AES-192 Circuit" << endl;
  Circuit C;
  ifstream inpf("Circuits/Bristol/aes_192.txt");
  inpf >> C;
  inpf.close();

  C.merge_AND_gates();

  vector<vector<int>> inp;
  vector<vector<int>> outp;
  inp.resize(2);
  outp.resize(1);
  outp[0].resize(128);
  inp[0].resize(192);
  inp[1].resize(128);

  /* Key */
  for (int i= 0; i < 192; i++)
    {
      inp[0][i]= 0;
    }
  /* Message */
  for (int i= 0; i < 128; i++)
    {
      inp[1][i]= 1;
    }

  C.evaluate(inp, outp);

  for (int i= 0; i < 128; i++)
    {
      cout << outp[0][i];
      if ((i & 7) == 7)
        {
          cout << "  ";
        }
      if ((i & 31) == 31)
        {
          cout << "\n";
        }
    }
  cout << endl;
  int byte;
  for (int i= 15; i >= 0; i--)
    {
      byte= 0;
      for (int j= 7; j >= 0; j--)
        {
          byte<<= 1;
          byte+= outp[0][i * 8 + j];
        }
      printf("%02X", byte);
    }
  cout << endl;
  cout << "\nShould be\n";
  cout << "B13DB4DA1F718BC6904797C82BCF2D32" << endl
       << endl
       << endl;
}

void test_AES256()
{

  cout << "Testing a AES-256 Circuit" << endl;
  Circuit C;
  ifstream inpf("Circuits/Bristol/aes_256.txt");
  inpf >> C;
  inpf.close();

  C.merge_AND_gates();

  vector<vector<int>> inp;
  vector<vector<int>> outp;
  inp.resize(2);
  outp.resize(1);
  outp[0].resize(128);
  inp[0].resize(256);
  inp[1].resize(128);

  /* Key */
  for (int i= 0; i < 256; i++)
    {
      inp[0][i]= 0;
    }
  /* Message */
  for (int i= 0; i < 128; i++)
    {
      inp[1][i]= 1;
    }

  C.evaluate(inp, outp);

  for (int i= 0; i < 128; i++)
    {
      cout << outp[0][i];
      if ((i & 7) == 7)
        {
          cout << "  ";
        }
      if ((i & 31) == 31)
        {
          cout << "\n";
        }
    }
  cout << endl;
  int byte;
  for (int i= 15; i >= 0; i--)
    {
      byte= 0;
      for (int j= 7; j >= 0; j--)
        {
          byte<<= 1;
          byte+= outp[0][i * 8 + j];
        }
      printf("%02X", byte);
    }
  cout << endl;
  cout << "\nShould be\n";
  cout << "ACDACE8078A32B1A182BFA4987CA1347" << endl
       << endl
       << endl;
}

void test_SHA3()
{
  cout << "Testing a Keccak_f Circuit" << endl;
  Circuit C;
  ifstream inpf("Circuits/Bristol/Keccak_f.txt");
  inpf >> C;
  inpf.close();

  C.merge_AND_gates();

  vector<vector<int>> inp;
  vector<vector<int>> outp;
  inp.resize(1);
  outp.resize(1);
  inp[0].resize(1600);
  outp[0].resize(1600);
  for (int i= 0; i < 1600; i++)
    {
      inp[0][i]= 0;
    }

  C.evaluate(inp, outp);

  for (int i= 0; i < 1600; i++)
    {
      cout << outp[0][i];
    }
  cout << endl;
  int byte;
  for (int i= 1600 / 8 - 1; i >= 0; i--)
    {
      byte= 0;
      for (int j= 7; j >= 0; j--)
        {
          byte<<= 1;
          byte+= outp[0][i * 8 + j];
        }
      printf("%02X", byte);
    }
  cout << endl;
  cout << "\nShould be\n";
  cout << "E7DDE140798F25F18A47C033F9CCD584EEA95AA61E2698D54D49806F304715BD57D05362054E288BD46F8E7F2DA497FFC44746A4A0E5FE90762E19D60CDA5B8C9C05191BF7A630AD64FC8FD0B75A933035D617233FA95AEB0321710D26E6A6A95F55CFDB167CA58126C84703CD31B8439F56A5111A2FF20161AED9215A63E505F270C98CF2FEBE641166C47B95703661CB0ED04F555A7CB8C832CF1C8AE83E8C14263AAE22790C94E409C5A224F94118C26504E72635F5163BA1307FE944F67549A2EC5C7BFFF1EA" << endl;

  // And apply again
  cout << "Applying function again" << endl;
  inp[0]= outp[0];
  C.evaluate(inp, outp);

  for (int i= 0; i < 1600; i++)
    {
      cout << outp[0][i];
    }
  cout << endl;
  for (int i= 1600 / 8 - 1; i >= 0; i--)
    {
      byte= 0;
      for (int j= 7; j >= 0; j--)
        {
          byte<<= 1;
          byte+= outp[0][i * 8 + j];
        }
      printf("%02X", byte);
    }
  cout << endl;
  cout << "\nShould be\n";
  cout << "3CCB6EF94D955C2D6DB55770D02C336A6C6BD770128D3D0994D06955B2D9208A56F1E7E5994F9C4F38FB65DAA2B957F90DAF7512AE3D7785F710D8C347F2F4FA59879AF7E69E1B1F25B498EE0FCCFEE4A168CEB9B661CE684F978FBAC466EADEF5B1AF6E833DC433D9DB1927045406E065128309F0A9F87C434717BFA64954FD404B99D833ADDD9774E70B5DFCD5EA483CB0B755EEC8B8E3E9429E646E22A0917BDDBAE729310E90E8CCA3FAC59E2A20B63D1C4E4602345B59104CA4624E9F605CBF8F6AD26CD020" << endl;
}

int main()
{
  test_AES128();
  test_AES192();
  test_AES256();
  test_SHA3();
  return 0;
}
