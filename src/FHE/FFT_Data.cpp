/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#include "FHE/FFT_Data.h"
#include "FHE/FFT.h"

/* Find an mth primitive root moduli the current prime
 *  *   This is deterministic so all players have the same root of unity
 *   * This assumes m is a power of two and so the cyclotomic polynomial
 *    * is  F=X^{m/2}+1
 *     */
modp Find_Primitive_Root_2power(int m, const Zp_Data &ZpD)
{
  modp ans, e, one, base;
  assignOne(one, ZpD);
  assignOne(base, ZpD);
  bigint exp;
  exp= (ZpD.pr - 1) / m;
  bool flag= true;
  while (flag)
    {
      /* Keep incrementing base until we hit the answer */
      Add(base, base, one, ZpD);
      Power(ans, base, exp, ZpD);
      /* e=ans^{m/2}+1  */
      Power(e, ans, m / 2, ZpD);
      Add(e, e, one, ZpD);
      if (isZero(e, ZpD))
        {
          flag= false;
        }
    }
  return ans;
}

void FFT_Data::init(const Ring &Rg, const Zp_Data &PrD)
{
  R= Rg;
  prData= PrD;
  root.resize(2);

  bigint prm1= PrD.pr - 1;

  // cout << Rg.m() << " " << PrD.pr << endl;
  root[0]= Find_Primitive_Root_2power(Rg.m(), PrD);
  Inv(root[1], root[0], PrD);
  to_modp(iphi, Rg.phi_m(), PrD);
  Inv(iphi, iphi, PrD);
}

ostream &operator<<(ostream &s, const FFT_Data &FFTD)
{
  bigint ans;
  s << FFTD.prData << endl;
  s << FFTD.R << endl;

  to_bigint(ans, FFTD.root[0], FFTD.prData);
  s << ans << " ";
  to_bigint(ans, FFTD.root[1], FFTD.prData);
  s << ans << endl;

  to_bigint(ans, FFTD.iphi, FFTD.prData);
  s << ans << endl;
  return s;
}

istream &operator>>(istream &s, FFT_Data &FFTD)
{
  bigint ans;
  s >> FFTD.prData >> FFTD.R;
  FFTD.root.resize(2);
  s >> ans;
  to_modp(FFTD.root[0], ans, FFTD.prData);
  s >> ans;
  to_modp(FFTD.root[1], ans, FFTD.prData);

  s >> ans;
  to_modp(FFTD.iphi, ans, FFTD.prData);

  return s;
}
