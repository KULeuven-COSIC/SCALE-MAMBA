/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#include "CRS.h"

#include "cryptopp/nbtheory.h"
#include "cryptopp/oids.h"

using namespace CryptoPP;

CRS::CRS()
{
  params= DL_GroupParameters_EC<ECP>(ASN1::secp256r1());
  curve= params.GetCurve();

  G.resize(2);
  H.resize(2);

  /* Find the CRS by just searching for the smallest four
   * points with smallest x-coord. Need to make this deterministic
   * even if sqrt is probabilistic
   */
  Integer A= curve.GetA(), B= curve.GetB(), p= curve.FieldSize();
  Integer x= 1, y, y2;
  int count= 0;
  while (count < 4)
    {
      y2= x * x * x + A * x + B;
      if (Jacobi(y2, p) == 1)
        {
          y= ModularSquareRoot(y2, p);
          if (y > p / 2)
            {
              y= p - y;
            }
          if (count == 0)
            {
              G[0]= ECPPoint(x, y);
            }
          else if (count == 1)
            {
              H[0]= ECPPoint(x, y);
            }
          else if (count == 2)
            {
              G[1]= ECPPoint(x, y);
            }
          else
            {
              H[1]= ECPPoint(x, y);
            }
          count++;
        }
      x= x + 1;
    }
}
