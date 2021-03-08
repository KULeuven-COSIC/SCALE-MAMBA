/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#include "ROT.h"

void OT_Sender::init(CryptoPP::RandomPool &RNG)
{
  complete= false;
  state= 0;

  // Sender Inputs
  M.resize(2);
  CryptoPP::Integer z;
  z.Randomize(RNG, crs.FieldSize().BitCount());
  M[0]= crs.Multiply(z, crs.Get_G(0));
  z.Randomize(RNG, crs.FieldSize().BitCount());
  M[1]= crs.Multiply(z, crs.Get_G(0));

  PRG.resize(2);
}

void OT_Receiver::init(CryptoPP::RandomPool &RNG, int choicebit)
{
  complete= false;
  state= 0;

  // Choice bit
  b= choicebit;

  // Generate public keys on receivers branch
  KeyGen(sk, pk, b, crs, RNG);
}

void OT_Receiver::message(string &output, const string &input,
                          unsigned int domain)
{
  if (complete)
    {
      return;
    }
  switch (state)
    {
      case 0:
        // If state=0 then send the public key across
        output= pk.to_string(crs);
        break;
      case 1:
        // Receive the two encrypted messages and decrypt
        // the correct one
        vector<CryptoPP::ECPPoint> c0(2), c1(2);
        int len= crs.EncodedPointSize();
        const unsigned char *data= (unsigned char *) input.c_str();

        crs.DecodePoint(c0[0], data, len);
        crs.DecodePoint(c1[0], data + len, len);
        crs.DecodePoint(c0[1], data + 2 * len, len);
        crs.DecodePoint(c1[1], data + 3 * len, len);

        sk.Decrypt(M, c0[b], c1[b], crs);
        complete= true;
        break;
    }
  state++;

  if (complete)
    {
      int len= crs.EncodedPointSize();
      if (len < SEED_SIZE)
        {
          throw invalid_length();
        }
      unsigned char *data= new unsigned char[len + 4];
      INT_TO_BYTES(data + len, domain);
      crs.EncodePoint(data, M);
      PRG.SetSeed(data, len + 4);
      delete[] data;
    }
}

void OT_Sender::message(string &output, const string &input,
                        unsigned int domain,
                        CryptoPP::RandomPool &RNG)
{
  if (complete)
    {
      return;
    }
  switch (state)
    {
      case 0:
        // If state=0 then receive the public key
        // and encrypt the messages
        pk.from_string(input, crs);
        vector<CryptoPP::ECPPoint> c0(2), c1(2);
        for (int i= 0; i < 2; i++)
          {
            pk.Encrypt(c0[i], c1[i], M[i], i, crs, RNG);
          }
        int len= crs.EncodedPointSize();
        unsigned char *data= new unsigned char[len];
        string ss;

        crs.EncodePoint(data, c0[0]);
        output.assign((char *) data, len);

        crs.EncodePoint(data, c1[0]);
        ss.assign((char *) data, len);
        output.append(ss);

        crs.EncodePoint(data, c0[1]);
        ss.assign((char *) data, len);
        output.append(ss);

        crs.EncodePoint(data, c1[1]);
        ss.assign((char *) data, len);
        output.append(ss);

        delete[] data;

        complete= true;
        break;
    }
  state++;
  if (complete)
    {
      int len= crs.EncodedPointSize();
      if (len < SEED_SIZE)
        {
          throw invalid_length();
        }
      unsigned char *data= new unsigned char[len + 4];
      INT_TO_BYTES(data + len, domain);

      crs.EncodePoint(data, M[0]);
      PRG[0].SetSeed(data, len + 4);

      crs.EncodePoint(data, M[1]);
      PRG[1].SetSeed(data, len + 4);
      delete[] data;
    }
}

void OT_Sender::get_random_bits(unsigned int i, unsigned int row, BitMatrix &M)
{
  if (!complete)
    {
      throw OT_error();
    }
  M.randomize(row, PRG[i]);
}

void OT_Receiver::get_random_bits(unsigned int row, BitMatrix &M)
{
  if (!complete)
    {
      throw OT_error();
    }
  M.randomize(row, PRG);
}
