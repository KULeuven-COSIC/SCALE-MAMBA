/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef MASCOTTRIPLES_H
#define MASCOTTRIPLES_H

#include "OT/COPE.h"
#include "OT/exROT.h"

class MASCOTTriples
{
public:
  vector<exROT_Sender> ROTS;
  vector<exROT_Receiver> ROTR;
  vector<Sender_COPE> COPE_S;
  vector<Receiver_COPE> COPE_R;
  PRNG G;
  PRNG G2;
  vector<vector<gfp>> triples;
  vector<vector<gfp>> combined_triples;
  vector<vector<gfp>> mac_combined_triples;
  vector<vector<gfp>> usable_triples;
  vector<vector<gfp>> mac_usable_triples;
  vector<gfp> opened_values;
  vector<gfp> mac_opened_values;

  gfp Delta;
  unsigned int myThreadNb;
  MASCOTTriples(Player& Pl, bigint fsize, unsigned int connection= 0);
  MASCOTTriples(Player& Pl, bigint fsize, gfp delta, unsigned int connection= 0);
  void execute(std::mutex &mtx, vector<list<gfp>> &u_triples, vector<list<gfp>> &mac_u_triples, bool &keepGoing, unsigned long int *totalProduced);

  void Multiply();
  void Combine();
  void Authenticate();
  void Sacrifice();
  void Multiplication(vector<gfp> &a, vector<gfp> &mac_a, vector<gfp> &b, vector<gfp> &mac_b, vector<gfp> &out, vector<gfp> &mac_out, vector<vector<list<gfp>>> &u_triples, vector<vector<list<gfp>>> &mac_u_triples, deque<mutex> &mtx);
  void Check();
  void Open(gfp &a, gfp &mac_a, gfp &out);
  void OpenToOne(unsigned int i, gfp &a, gfp &mac_a, gfp &out);

private:
  Player &P;
  bigint size_field;
  unsigned int bit_size_field;
  unsigned int keyGen_sec;
  unsigned int gf2n_to_gfp_sec;
  unsigned int nbTriples;
  unsigned int nbPlayers;
  // tau is 'number of triples generated per output triple, geq 3'
  unsigned int tau;
  unsigned long int nbOTs;
  unsigned long int nbOTsOutput;
  unsigned long int totalTriplesGenerated;
  //All the vectors we will need
  vector<gfp> g;
  bool isInit;
  unsigned int last_thread;
};

#endif
