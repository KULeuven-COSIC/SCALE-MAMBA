/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _CRS
#define _CRS

/* This is the CRS for the ECC based DualModeEncryption
 * scheme
 */

#include "Exceptions/Exceptions.h"

#include "cryptopp/eccrypto.h"
using namespace std;

class CRS
{
  vector<CryptoPP::ECPPoint> G, H;
  CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP> params;
  CryptoPP::ECP curve;

public:
  CRS();

  CryptoPP::ECPPoint Get_G(int i) const { return G[i]; }
  CryptoPP::ECPPoint Get_H(int i) const { return H[i]; }

  /* Curve operations. Put here to avoid overkill in friending */
  CryptoPP::Integer FieldSize() const { return curve.FieldSize(); }
  CryptoPP::ECPPoint Multiply(const CryptoPP::Integer &x, const CryptoPP::ECPPoint &P) const
  {
    CryptoPP::ECPPoint T= curve.Multiply(x, P);
    return T;
  }
  CryptoPP::ECPPoint CascadeMultiply(const CryptoPP::Integer &x, const CryptoPP::ECPPoint &P,
                                     const CryptoPP::Integer &y, const CryptoPP::ECPPoint &Q) const
  {
    CryptoPP::ECPPoint T= curve.CascadeMultiply(x, P, y, Q);
    return T;
  }

  CryptoPP::ECPPoint Add(const CryptoPP::ECPPoint &P, const CryptoPP::ECPPoint &Q) const
  {
    CryptoPP::ECPPoint T= curve.Add(P, Q);
    return T;
  }
  CryptoPP::ECPPoint Inverse(const CryptoPP::ECPPoint &P) const
  {
    CryptoPP::ECPPoint T= curve.Inverse(P);
    return T;
  }

  unsigned int EncodedPointSize() const
  {
    return curve.EncodedPointSize(true);
  }

  void EncodePoint(unsigned char *encodedPoint, const CryptoPP::ECPPoint &P) const
  {
    curve.EncodePoint(encodedPoint, P, true);
  }

  void DecodePoint(CryptoPP::ECPPoint &P, const unsigned char *encodedPoint, int len) const
  {
    if (!curve.DecodePoint(P, encodedPoint, len))
      {
        throw bad_value();
      }
  }
};

#endif
