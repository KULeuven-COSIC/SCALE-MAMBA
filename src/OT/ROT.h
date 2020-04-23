/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _ROTDMC
#define _ROTDMC

/* This defines a random OT protocol based on ECC Dual Mode
 * Encryption, from ePrint 2007/348.
 *
 * The issue with this is that a malicious sender can choose
 * the two messages to be identical. Thus this is not a good
 * OT for our application. It is therefore preferred to
 * use the one from ePrint 2015/267 defined in SimpleROT
 *
 * There are two classes a sender and a receiver.
 *
 * We assume here that the receiver always goes first
 * and the receiver always ends the protocol
 *   - We iterate calling sender/receiver as many times
 *     as it takes to complete the protocol
 * You can check if a protocol has completed by checking
 * the complete flag
 */

#include "BitMatrix.h"
#include "DMC.h"
#include "Tools/random.h"
#include "config.h"
#include "cryptopp/randpool.h"
#include <string>

class OT_Sender
{
  CRS crs;

  vector<CryptoPP::ECPPoint> M;

  // The two messages as a seeded PRG
  vector<PRNG> PRG;

  bool complete;
  DM_PK pk;

  int state; // Where we have got to in the protocol

public:
  void init(CryptoPP::RandomPool &RNG);

  bool is_complete() const
  {
    return complete;
  }
  int get_state() const
  {
    return state;
  }

  void message(string &output, const string &input, CryptoPP::RandomPool &RNG);

  void output(ostream &o, int i) const
  {
    o << M[i].x << " " << M[i].y;
  }

  /* Returns M.size() random bits from PRG and assigns them to given row */
  void get_random_bits(unsigned int i, unsigned int row, BitMatrix &M);
};

class OT_Receiver
{
  CRS crs;

  CryptoPP::ECPPoint M;
  // The message as a seeded PRG
  PRNG PRG;

  int b;

  bool complete;

  DM_PK pk;
  DM_SK sk;

  int state; // Where we have got to in the protocol

public:
  void init(CryptoPP::RandomPool &RNG, int choicebit);

  bool is_complete() const
  {
    return complete;
  }
  int get_state() const
  {
    return state;
  }

  void message(string &output, const string &input);

  void output(ostream &o) const
  {
    o << b << " : " << M.x << " " << M.y;
  }

  /* Returns M.size() random bits from PRG and assigns them to given row */
  void get_random_bits(unsigned int row, BitMatrix &M);

  int get_bit() const
  {
    return b;
  }
};

#endif
