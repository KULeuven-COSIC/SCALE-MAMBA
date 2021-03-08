/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <memory>
#include <pthread.h>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>
using namespace std;

#include "FHE/FHE_Params.h"
#include "KeyGenM.h"
#include "MASCOTTriples.h"
#include "Math/bigint.h"
#include "System/Networking.h"
#include "System/RunTime.h"
#include "config.h"

int main(int argc, const char *argv[])
{
  unsigned int my_number;
  unsigned int nb_bits_prime;
  unsigned int nb_threads_1= 1;
  unsigned int nb_threads_2= 1;
  unsigned int portnumbase= 5000;
  int verbose= 3;
  /*Retrieve party number*/
  if (argc < 3)
    {
      cout << "Usage ./PlayerKeygen.x <playernum> <primeSize> [nbThreads]" << endl;
      cout << "This will start nbThreads for each MASCOT factory." << endl;
      cout << "Usage ./PlayerKegen.x <playernum> <primeSize> [nbThreads1] [nbThreads2]." << endl;
      cout << "This will start nbThreads1 for the MASCOT factory for p1 and nbThreads2 for the MASCOT factory for p2" << endl;
      exit(0);
    }
  my_number= atoi(argv[1]);
  nb_bits_prime= atoi(argv[2]);
  /*Retrieve number of threads to use*/
  if (argc == 4)
    {
      nb_threads_1= atoi(argv[3]);
      nb_threads_2= atoi(argv[3]);
    }
  if (argc == 5)
    {
      nb_threads_1= atoi(argv[3]);
      nb_threads_2= atoi(argv[4]);
    }

  cout << my_number;

  /*************************************
   *     Initialise the system data    *
   *************************************/
  SystemData SD("Data/NetworkData.txt");
  if (my_number >= SD.n)
    {
      throw data_mismatch();
    }

  /*************************************
   *    Initialize the portnums        *
   *************************************/
  vector<unsigned int> portnum(SD.n);
  for (unsigned int i= 0; i < SD.n; i++)
    {
      portnum[i]= portnumbase + i;
    }

  /*****************************************
   *    Create empty MAC keys (req Player) *
   *****************************************/
  vector<gfp> MacK(Share::SD.nmacs);

  /* Initialize SSL */
  SSL_CTX *ctx;
  Init_SSL_CTX(ctx, my_number, SD);

  /* Initialize the networking TCP sockets */
  int ssocket;
  unsigned int nbThreads= nb_threads_2 + nb_threads_1 + 3;
  vector<vector<vector<int>>> csockets(nbThreads, vector<vector<int>>(SD.n, vector<int>(3)));
  Get_Connections(ssocket, csockets, portnum, my_number, SD, verbose - 2);
  printf("All connections now done\n");

  Player P(my_number, SD, 0, ctx, csockets[0], MacK, verbose - 1);

  bigint fsize1, fsize2, pMPC;
  unsigned int N;

  Generate_Parameters(N, fsize1, fsize2, pMPC, nb_bits_prime, SD.n, TopGear, HwtSK);

  cout << "[PlayerKeyGen] n = " << SD.n << endl;
  cout << "[PlayerKeyGen] p1 = " << fsize1 << endl;
  cout << "[PlayerKeyGen] p2 = " << fsize2 << endl;
  cout << "[PlayerKeyGen] p = " << pMPC << endl;
  cout << "[PlayerKeyGen] N = " << N << endl;
  cout << "[PlayerKeyGen] h = " << HwtSK << endl;
  //MASCOTTriples mt1(P, fsize1);
  //MASCOTTriples mt2(P, fsize2);
  KeyGenM kg(P);
  kg.genKey(N, fsize1, fsize2, pMPC, nb_threads_1, nb_threads_2, ctx, SD, csockets, MacK);
  /* Destroy SSL */
  Destroy_SSL_CTX(ctx);

  /********************************
   *    Create Sharing Data file  *
   ********************************/
  ShareData ShareD;
  ShareD.Initialize_Full_Threshold(SD.n);
  ofstream out("Data/SharingData.txt");
  if (out.fail())
    {
      throw file_error("Data/SharingData.txt");
    }
  out << pMPC << endl;
  out << ShareD;
  out.close();

  /********************************
   *    Create MAC Key for SPDZ   *
   ********************************/
  gfp::init_field(pMPC);
  PRNG G;
  G.ReSeed(0);
  stringstream ss;
  ss << "Data/MKey-" << my_number << ".key";
  ofstream outk(ss.str().c_str());
  if (outk.fail())
    {
      throw file_error(ss.str());
    }
  for (unsigned int j= 0; j < ShareD.nmacs; j++)
    {
      gfp aa;
      aa.randomize(G);
      outk << aa << " ";
    }
  outk << endl;
  outk.close();
  exit(0);
}
