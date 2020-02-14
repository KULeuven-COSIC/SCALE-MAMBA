/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _DMC
#define _DMC

/* The dual mode encryption scheme based on ECC from ePrint 2007/348 */

#include "CRS.h"
#include "cryptopp/nbtheory.h"
#include <string>

class DM_PK;

class DM_SK
{
  CryptoPP::Integer sk;

public:
  void Decrypt(CryptoPP::ECPPoint &m, const CryptoPP::ECPPoint &c0, const CryptoPP::ECPPoint &c1, const CRS &crs);

  friend void KeyGen(DM_SK &sk, DM_PK &pk, int sigma, const CRS &crs, CryptoPP::RandomNumberGenerator &RNG);
};

class DM_PK
{
  CryptoPP::ECPPoint g, h;

  void Randomize(CryptoPP::ECPPoint &U, CryptoPP::ECPPoint &V,
                 const CryptoPP::ECPPoint &G, const CryptoPP::ECPPoint &H,
                 const CryptoPP::ECPPoint &Gd, const CryptoPP::ECPPoint &Hd,
                 const CRS &crs,
                 CryptoPP::RandomNumberGenerator &RNG);

public:
  void Encrypt(CryptoPP::ECPPoint &c0, CryptoPP::ECPPoint &c1, const CryptoPP::ECPPoint &m, int branch, const CRS &crs, CryptoPP::RandomNumberGenerator &RNG);

  friend void KeyGen(DM_SK &sk, DM_PK &pk, int sigma, const CRS &crs, CryptoPP::RandomNumberGenerator &RNG);

  string to_string(const CRS &crs) const;
  void from_string(const string &cc, const CRS &crs);
};

void KeyGen(DM_SK &sk, DM_PK &pk, int sigma, const CRS &crs, CryptoPP::RandomNumberGenerator &RNG);

#endif
