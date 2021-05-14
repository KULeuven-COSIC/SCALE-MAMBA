/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

#include "openssl/err.h"
#include "openssl/ssl.h"

#include "FHE/FHE_Keys.h"
#include "FHE/FHE_Params.h"
#include "GC/SimplifyCircuit.h"
#include "LSSS/Share.h"
#include "OT/aBitVector.h"
#include "System/Init.h"
#include "config.h"

void init_secret_sharing_inner(std::istream &cin)
{
  ifstream input("Data/NetworkData.txt");
  if (input.fail())
    {
      throw file_error("Data/NetworkData.txt");
    }

  string str;
  input >> str;
  unsigned int n;
  input >> n;
  input.close();

  cout << "Number of players = " << n << endl;

  cout << "\nChoose which LSSS you want..." << endl;
  cout << "\t0) Full Threshold\n";
  cout << "\t1) Shamir\n";
  cout << "\t2) Replicated\n";
  cout << "\t3) General Q2 MSP" << endl;

  unsigned int v= 4;
  while (v > 3)
    {
      cout << "Enter a number (0-3).." << endl;
      cin >> v;
    }

  ShareType lsss= (ShareType) v;
  bigint p= 0;
  int lg2p= -1;
  switch (lsss)
    {
      case Full:
        {
          int answer= -1;
          while (answer != 0 && answer != 1)
            {
              cout << "Do you want me to find a prime for the LSSS, or do you want to try one yourself?\n";
              cout << "\t0) You find one\n\t1) I will enter one" << endl;
              cin >> answer;
            }
          if (answer == 0)
            {
              while (lg2p < 16 || lg2p > 1024)
                {
                  cout << "What bit length of modulus do you want for secret sharing? (16-1024)" << endl;
                  cin >> lg2p;
                }
            }
          else
            {
              cout << "Enter a prime for the LSSS.";
              cout << "Note if I cannot find FHE parameters suitable for this prime I will abort\n";
              cout << "This option is really for expert use only..." << endl;
              cin >> p;
              lg2p= numBits(p);
            }
          break;
        }
      case Shamir:
      case Replicated:
      case Q2MSP:
        {
          while (p < 2)
            {
              cout << "\nWhat modulus do you want to use for secret sharing?" << endl;
              cin >> p;
            }
          cout << "Using modulus p=" << p << endl;
          // NOTE: we need to initialize the `gfp` static here in
          // order to allow Q2MSP to initialize the `Gen` `vector<vector<gfp>>`.
          gfp::init_field(p);
          break;
        }
      case Other:
        throw not_implemented();
    }
  unsigned int t= 0;
  vector<unsigned int> ns;
  vector<vector<gfp>> Gen;
  ReplicationMode mode= (ReplicationMode) 0;
  OfflineType offline_type= (OfflineType) 0;
  imatrix Sets;

  switch (lsss)
    {
      case Shamir:
        {
          unsigned int bd= n / 2;
          if ((n & 1) == 1)
            {
              bd++;
            }
          while (t < 1 || t >= bd)
            {
              cout << "Enter threshold 0 < t < " << bd << endl;
              cin >> t;
            }
          break;
        }
      case Q2MSP:
        {
          cout << "Enter number of shares per player" << endl;
          unsigned int nr= 0;
          for (unsigned int i= 0; i < n; i++)
            {
              cout << "\t Player " << i << endl;
              unsigned int s;
              cin >> s;
              nr+= s;
              ns.push_back(s);
            }

          cout << "Enter dimension of the MSP, i.e. number of columns of the generator "
                  "matrix"
               << endl;
          unsigned int k;
          cin >> k;

          cout << "Now enter the matrix" << endl;
          Gen= vector<vector<gfp>>(nr, vector<gfp>(k));
          for (unsigned int i= 0; i < nr; i++)
            {
              for (unsigned int j= 0; j < k; j++)
                {
                  cin >> Gen[i][j];
                }
            }
          break;
        }
      case Replicated:
        {
          cout << "How do you want to initialize the secret sharing scheme?\n";
          cout << "\t 1) Set of unqualified sets\n";
          cout << "\t 2) Set of qualified sets\n";
          cout << "\t 3) Replicated threshold structure";
          cout << endl;
          int v= -1;
          while (v < 1 || v > 3)
            {
              cout << "Enter a number (1-3).." << endl;
              cin >> v;
            }
          mode= (ReplicationMode) v;

          t= n;
          if (v == 1 || v == 2)
            {
              enter_sets(cin, Sets, v == 1, n);
            }
          else
            {
              unsigned int bd= n / 2;
              if ((n & 1) == 1)
                {
                  bd++;
                }
              while (t < 1 || t >= bd)
                {
                  cout << "Enter threshold 0 < t < " << bd << endl;
                  cin >> t;
                }
            }

          cout << "What type of Offline Phase do you want ?\n";
          cout << "\t 1) Maurer\n";
          cout << "\t 2) Reduced";
          cout << endl;

          v= -1;
          while (v < 1 || v > 2)
            {
              cout << "Enter a number (1-2).." << endl;
              cin >> v;
            }
          offline_type= (OfflineType) v;
          break;
        }
      default:
        break;
    }

  auto secret_sharing= init_secret_sharing(lsss, p, lg2p, t, n, ns, Gen, mode, offline_type, Sets);

  ofstream out("Data/SharingData.txt");
  if (out.fail())
    {
      throw file_error("Data/SharingData.txt");
    }
  out << p << endl;
  out << secret_sharing.SD;
  out.close();

  if (secret_sharing.SD.Etype != HSS)
    {
      out.open("Data/SharingData2.txt");
      if (out.fail())
        {
          throw file_error("Data/SharingData2.txt");
        }
      out << secret_sharing.SD2;
      out.close();
      cout << "We have setup the data for non-HSS based Mod2 engine" << endl;
    }
  else
    {
      cout << "We will be using an HSS based Mod2 engine" << endl;
    }

  for (unsigned int i= 0; i < n; i++)
    {
      stringstream ss;
      ss << "Data/MKey-" << i << ".key";
      ofstream outk(ss.str().c_str());
      if (outk.fail())
        {
          throw file_error(ss.str());
        }
      for (auto &aa : secret_sharing.mkeys[i])
        {
          outk << aa << " ";
        }
      outk << endl;
      outk.close();
    }
}

extern "C" int init_helper(std::istream &command_line_input, std::function<void(std::istream &)> func)
{
  try
    {
      func(command_line_input);
      return 0;
    }
  catch (std::exception &e)
    {
      std::cerr << "sm:: Failed to init secret sharing: " << e.what()
                << std::endl;
      return 1;
    }
}

extern "C" int init_secret_sharing_rust(const char *command_line_input)
{
  std::stringstream ss(command_line_input);
  return init_helper(ss, init_secret_sharing_inner);
}

extern "C" int init_certs_rust(const char *command_line_input)
{
  std::stringstream ss(command_line_input);
  return init_helper(ss, init_certs);
}

extern "C" int init_secret_sharing_cin()
{
  return init_helper(cin, init_secret_sharing_inner);
}

extern "C" int init_certs_cin()
{
  return init_helper(cin, init_certs);
}

extern "C" int init_conversion_rust()
{
  try
    {
      init_conversion();
      return 0;
    }
  catch (std::exception &e)
    {
      std::cerr << "sm:: Failed to init secret sharing: " << e.what()
                << std::endl;
      return 1;
    }
}

#ifndef USE_RUST
int main(int argc, const char *argv[])
{
  int ans= -1;
  if (argc == 1)
    {
      cout << "What do you want to set up?\n";
      cout << "\t1) Certs\n";
      cout << "\t2) Secret Sharing\n";
      cout << "\t3) Conversion circuit for LSSS<->GC computations\n";
      cout << "\t4) All three\n"
           << endl;

      while (ans < 1 || ans > 4)
        {
          cout << "Enter a number (1-4).." << endl;
          cin >> ans;
        }
    }
  else if (argc == 2)
    {
      ans= atoi(argv[1]);
      if (ans < 0 || ans > 4)
        {
          throw bad_value();
        }
    }

  if (ans == 1 || ans == 4)
    {
      init_certs(cin);
    }
  if (ans == 2 || ans == 4)
    {
      init_secret_sharing_inner(cin);
    }
  if (ans == 3 || ans == 4)
    {
      init_conversion();
    }
}
#endif
