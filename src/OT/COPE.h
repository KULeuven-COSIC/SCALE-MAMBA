/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef COPE_H
#define COPE_H

#include "SimpleROT.h"
#include "System/Player.h"
#include "Tools/random.h"

/* Correlated Oblivious Product Evaluation
   Follows protocol 1 from 2016/505 (MASCOT).
   Init with a key Delta from Receiver.
   At each extend with input x from Sender, creates a sharing of x*Delta
   (Used pairwise between parties to authenticate shares, see protocol 3 from 2016/505).
*/
class Sender_COPE
{
  unsigned int bit_size_field;
  unsigned int nbReceiver;
  unsigned int connection;
  vector<SimpleOT_Sender> ROT_S;

  vector<vector<vector<gfp>>> t;
  vector<gfp> u;
  vector<gfp> g;

public:
  void init(Player &P, int i, unsigned int fsize, CryptoPP::RandomPool &RNG, unsigned int threadNb= 0);
  void extend(Player &P, gfp &x, gfp &out);
  void extend_vec(Player &P, vector<gfp> &x, vector<gfp> &out, bool resize= false);
};

class Receiver_COPE
{
  unsigned int bit_size_field;
  unsigned int nbSender;
  unsigned int connection;
  gfp Delta;
  vector<int> choicebits;
  vector<SimpleOT_Receiver> ROT_R;

  vector<vector<gfp>> q;
  vector<gfp> u;
  vector<vector<gfp>> t;
  vector<gfp> g;

public:
  void init(Player &P, int i, unsigned int fsize, gfp &delta, CryptoPP::RandomPool &RNG, unsigned int threadNb= 0);

  void extend(Player &P, gfp &out);
  void extend_vec(Player &P, vector<gfp> &out, bool resize= false);
};

#endif
