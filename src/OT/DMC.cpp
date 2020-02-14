/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#include "DMC.h"

#include "Exceptions/Exceptions.h"

void KeyGen(DM_SK &sk, DM_PK &pk, int sigma, const CRS &crs, CryptoPP::RandomNumberGenerator &RNG)
{
  sk.sk.Randomize(RNG, crs.FieldSize().BitCount());
  pk.g= crs.Multiply(sk.sk, crs.Get_G(sigma));
  pk.h= crs.Multiply(sk.sk, crs.Get_H(sigma));
}

void DM_PK::Randomize(CryptoPP::ECPPoint &U, CryptoPP::ECPPoint &V,
                      const CryptoPP::ECPPoint &G, const CryptoPP::ECPPoint &H,
                      const CryptoPP::ECPPoint &Gd, const CryptoPP::ECPPoint &Hd,
                      const CRS &crs,
                      CryptoPP::RandomNumberGenerator &RNG)
{
  CryptoPP::Integer s, t;
  s.Randomize(RNG, crs.FieldSize().BitCount());
  t.Randomize(RNG, crs.FieldSize().BitCount());

  // u = [s]*G+[t]*H
  U= crs.CascadeMultiply(s, G, t, H);

  // v = [s]*Gd+[t]*Hd
  V= crs.CascadeMultiply(s, Gd, t, Hd);
}

void DM_PK::Encrypt(CryptoPP::ECPPoint &c0, CryptoPP::ECPPoint &c1, const CryptoPP::ECPPoint &m, int branch,
                    const CRS &crs, CryptoPP::RandomNumberGenerator &RNG)
{
  Randomize(c0, c1, crs.Get_G(branch), crs.Get_H(branch), g, h, crs, RNG);
  c1= crs.Add(c1, m);
}

void DM_SK::Decrypt(CryptoPP::ECPPoint &m, const CryptoPP::ECPPoint &c0, const CryptoPP::ECPPoint &c1, const CRS &crs)
{
  m= crs.Multiply(sk, c0);
  m= crs.Inverse(m);
  m= crs.Add(m, c1);
}

string DM_PK::to_string(const CRS &crs) const
{
  int len= crs.EncodedPointSize();
  unsigned char *data= new unsigned char[len];
  crs.EncodePoint(data, g);

  string ss, ss2;
  ss.assign((char *) data, len);

  crs.EncodePoint(data, h);
  ss2.assign((char *) data, len);

  ss.append(ss2);

  delete[] data;

  return ss;
}

void DM_PK::from_string(const string &s, const CRS &crs)
{
  int len= crs.EncodedPointSize();
  const unsigned char *data= (const unsigned char *) s.c_str();
  crs.DecodePoint(g, data, len);
  crs.DecodePoint(h, data + len, len);

  // Abort if the public key is invalid
  if (g.identity == true || h.identity == true)
    {
      throw OT_error();
    }
}
