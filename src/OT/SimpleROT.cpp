/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#include "SimpleROT.h"

void SimpleOT_Sender::init(string &output, CryptoPP::RandomPool &RNG)
{
  complete= false;

  // Sender Inputs
  y.Randomize(RNG, crs.FieldSize().BitCount());
  S= crs.Multiply(y, crs.Get_G(0));
  T= crs.Multiply(y, S);

  //cout << "i:" <<  y << endl << S.x << endl << T.x << endl;

  PRG.resize(2);

  int len= crs.EncodedPointSize();
  unsigned char *data= new unsigned char[len];
  crs.EncodePoint(data, S);
  output.assign((char *) data, len);
  delete[] data;
}

void SimpleOT_Receiver::init(const string &input, int choicebit)
{
  complete= false;

  // Choice bit
  b= choicebit;

  int len= crs.EncodedPointSize();
  const unsigned char *data= (const unsigned char *) input.c_str();
  crs.DecodePoint(S, data, len);

  // Abort if the public key is invalid
  if (S.identity == true)
    {
      throw OT_error();
    }
}

void SimpleOT_Receiver::message(string &output, unsigned int domain,
                                CryptoPP::RandomPool &RNG)
{
  if (complete)
    {
      return;
    }

  CryptoPP::Integer x;
  CryptoPP::ECPPoint R, V;
  x.Randomize(RNG, crs.FieldSize().BitCount());
  R= crs.Multiply(x, crs.Get_G(0));
  if (b == 1)
    {
      R= crs.Add(R, S);
    }

  int len= crs.EncodedPointSize();
  unsigned char *data= new unsigned char[3 * len + 4];
  crs.EncodePoint(data, R);
  output.assign((char *) data, len);

  complete= true;

  V= crs.Multiply(x, S);
  crs.EncodePoint(data, S);
  crs.EncodePoint(data + len, R);
  crs.EncodePoint(data + 2 * len, V);
  INT_TO_BYTES(data + 3 * len, domain);
  PRG.SetSeed(data, 3 * len + 4);
  delete[] data;
}

void SimpleOT_Sender::message(const string &input, unsigned int domain)
{
  if (complete)
    {
      return;
    }
  //cout << "m:" << y << endl << S.x << endl << T.x << endl;

  CryptoPP::ECPPoint R, V0, V1, Tm;
  int len= crs.EncodedPointSize();
  const unsigned char *odata= (const unsigned char *) input.c_str();
  crs.DecodePoint(R, odata, len);

  // Abort if the public key is invalid
  if (R.identity == true)
    {
      throw OT_error();
    }

  complete= true;
  V0= crs.Multiply(y, R);
  Tm= crs.Inverse(T);
  V1= crs.Add(V0, Tm);

  unsigned char *data= new unsigned char[3 * len + 4];
  INT_TO_BYTES(data + 3 * len, domain);
  crs.EncodePoint(data, S);
  crs.EncodePoint(data + len, R);
  crs.EncodePoint(data + 2 * len, V0);
  PRG[0].SetSeed(data, 3 * len + 4);

  crs.EncodePoint(data, S);
  crs.EncodePoint(data + len, R);
  crs.EncodePoint(data + 2 * len, V1);
  PRG[1].SetSeed(data, 3 * len + 4);
  delete[] data;
}

void SimpleOT_Sender::get_random_bits(unsigned int i, unsigned int row, BitMatrix &M)
{
  if (!complete)
    {
      throw OT_error();
    }
  M.randomize(row, PRG[i]);
}

void SimpleOT_Receiver::get_random_bits(unsigned int row, BitMatrix &M)
{
  if (!complete)
    {
      throw OT_error();
    }
  M.randomize(row, PRG);
}

void SimpleOT_Sender::get_random_bits(unsigned int i, gfp &a)
{
  if (!complete)
    {
      throw OT_error();
    }
  a.randomize(PRG[i]);
}

void SimpleOT_Receiver::get_random_bits(gfp &a)
{
  if (!complete)
    {
      throw OT_error();
    }
  a.randomize(PRG);
}

void SimpleOT_Sender::get_random_bits(unsigned int i, BitVector &v)
{
  if (!complete)
    {
      throw OT_error();
    }
  v.randomize(PRG[i]);
}

void SimpleOT_Receiver::get_random_bits(BitVector &v)
{
  if (!complete)
    {
      throw OT_error();
    }
  v.randomize(PRG);
}
