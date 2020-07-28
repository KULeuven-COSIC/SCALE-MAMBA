/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _SimpleROT
#define _SimpleROT

/* This defines a random OT protocol using ePrint 2015/267 
 *
 * There are two classes a sender and a receiver.
 *
 * There is only two stages. 
 *   a) The sender samples a public/ * private key and sends 
 *      it to the receiver. 
 *   b) The receiver then does there random selection
 *   c) Both parrties can then derive the associated PRG
 *
 * Calling sequence should be
 *   Sender.init(output,RNG)
 *   Receiver.init(output,choicebit);
 *   Receiver.message(output,RNG)
 *   Sender.message(output);
 *
 */

#include "BitMatrix.h"
#include "CRS.h"
#include "Math/gfp.h"
#include "Tools/random.h"
#include "cryptopp/randpool.h"
#include <string>

class SimpleOT_Sender
{
  CRS crs;

  CryptoPP::Integer y;
  CryptoPP::ECPPoint S, T;

  // The two messages as a seeded PRG
  vector<PRNG> PRG;

  bool complete;

public:
  void init(string &output, CryptoPP::RandomPool &RNG);

  bool is_complete() const
  {
    return complete;
  }
  void reset()
  {
    complete= false;
  }

  void message(const string &input);

  /* Returns M.size() random bits from PRG and assigns them to given row */
  void get_random_bits(unsigned int i, unsigned int row, BitMatrix &M);
  void get_random_bits(unsigned int i, gfp &a);
  void get_random_bits(unsigned int i, BitVector &v);
};

class SimpleOT_Receiver
{
  CRS crs;

  CryptoPP::ECPPoint S;

  // The message as a seeded PRG
  PRNG PRG;

  int b;

  bool complete;

public:
  void init(const string &input, int choicebit);

  bool is_complete() const
  {
    return complete;
  }

  void message(string &output, CryptoPP::RandomPool &RNG);

  /* Returns M.size() random bits from PRG and assigns them to given row */
  void get_random_bits(unsigned int row, BitMatrix &M);

  void get_random_bits(gfp &a);
  void get_random_bits(BitVector &v);

  int get_bit() const
  {
    return b;
  }
};

#endif
