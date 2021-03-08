/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "MASCOTTriples.h"
#include "System/Player.h"
#include <functional>

class KeyGenM
{
public:
  Player P;
  int bit_batch_size;
  int sec;
  unsigned int totalBitsGenerated;

  //Avaible triples
  vector<vector<list<gfp>>> u_triples_w1;
  vector<vector<list<gfp>>> mac_u_triples_w1;
  vector<vector<list<gfp>>> u_triples_w2;
  vector<vector<list<gfp>>> mac_u_triples_w2;

  //mutex for threads
  deque<mutex> mtx1;
  deque<mutex> mtx2;

  //Result of RandomBit
  deque<gfp> b_w1;
  deque<gfp> mac_b_w1;
  deque<gfp> b_w2;
  deque<gfp> mac_b_w2;

  //Result of Hamming
  vector<gfp> h_b_w1;
  vector<gfp> mac_h_b_w1;
  vector<gfp> h_b_w2;
  vector<gfp> mac_h_b_w2;

  //Result of Gauss
  vector<gfp> g_w1;
  vector<gfp> mac_g_w1;
  vector<gfp> g_w2;
  vector<gfp> mac_g_w2;

  //Result of uniform
  vector<gfp> u_w1;
  vector<gfp> mac_u_w1;
  vector<gfp> u_w2;
  vector<gfp> mac_u_w2;

  KeyGenM(Player &P);

  //void genKey(unsigned int N, MASCOTTriples &mt1, MASCOTTriples &mt2, bigint fsize1, bigint fsize2, bigint pMPC, unsigned int nb_threads_1, unsigned int nb_threads_2);
  void genKey(unsigned int N, bigint fsize1, bigint fsize2, bigint pMPC, unsigned int nb_threads_1, unsigned int nb_threads_2, SSL_CTX *ctx, const SystemData &SD,vector<vector<vector<int>>> &csockets, vector<gfp> &MacK);


  void RandomBit(int M, int sec, MASCOTTriples &mt1, MASCOTTriples &mt2, bigint fsize1, bigint fsize2);

  void RandomMaBit(int M, int sec, MASCOTTriples &mt1, MASCOTTriples &mt2, bigint fsize1, bigint fsize2);

  void Hamming(int nu, int l, MASCOTTriples &mt1, MASCOTTriples &mt2, bigint fsize1, bigint fsize2);

  void Gauss(unsigned int N, MASCOTTriples &mt1, MASCOTTriples &mt2, bigint fsize1, bigint fsize2);

  void Uniform(unsigned int N, MASCOTTriples &mt1, MASCOTTriples &mt2, bigint fsize1, bigint fsize2);

  void Input_self(vector<gfp> &in, vector<gfp> &out, vector<gfp> &mac_out, bigint p, MASCOTTriples &mt);

  void Input_other(int nbP_Input, vector<gfp> &out, vector<gfp> &mac_out, bigint p, MASCOTTriples &mt);

  void MacCheck(gfp y, gfp m, bigint p, MASCOTTriples &mt);
};
