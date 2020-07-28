/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef exROT_H
#define exROT_H

#include "COT.h"
#include "Math/gf2n.h"
#include "Math/gfp.h"
#include "System/Player.h"
#include "Tools/MMO.h"
#include "cryptopp/randpool.h"
//2015/472 Fig.15

/*

Now to get your ext_ROT (extended ROT) what you
should do is produce the COTs for the receivers
input bits. Then you get a bunch of
    M_0^i, M_1^i=M_0^i+Delta,   M_b^i
values. Each of which is a gf2n element. Then you
can convert these into ext_ROTs (defined over gf2n)
by simply hashing each value by applying the MMO
function (use the amortization of 128 bits is probably
good).

This gives you OTs of
    R_0^i,   R_1^i,   R_b^i
which are gf2n elements. Suggest defining a class in
the OT directory to do this. Probably needs to have
the COT class as member values

Then what you need, for your application, to do is convert
this output into gfp elements. Let log_2 p = ell. And set
k = ceil((ell+128)/128).

Now take k outputs from the above ext_ROT so we have
     R_b^1,...,R_b^k
Using the gf2n function store_into_buffer stick these into
a uint8_t buffer of size k*16.

*/

class exROT_Sender
{
  int nb_rcver;
  Receiver_COT COTR;
  gf2n Delta;

  MMO mmo;

public:
  //ROT with player i, P is receiver
  void init(Player &P, int i, CryptoPP::RandomPool &RNG, vector<int> &choicebits, unsigned int connectionNb= 2);

  //Gets size gfp elements out of the OT (times 2 for Sender)
  void next_iteration(Player &P, unsigned int size, vector<vector<gf2n>> &out_vec);
};

class exROT_Receiver
{
  int nb_sender;
  Sender_COT COTS;

  MMO mmo;

public:
  //ROT with player i, P is receiver
  void init(Player &P, int i, CryptoPP::RandomPool &RNG, unsigned int connectionNb= 2);

  //Gets size gfp elements out of the OT, choosing via choice_vec
  void next_iteration(Player &P, unsigned int size, BitVector &choice_vec, vector<gf2n> &out_vec);
};

#endif
