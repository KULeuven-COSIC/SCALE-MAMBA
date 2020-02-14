/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _FFT_Data
#define _FFT_Data

#include "FHE/Ring.h"
#include "Math/Zp_Data.h"
#include "Math/gfp.h"
#include "Math/modp.h"

/* Class for holding modular arithmetic data wrt the ring
 *
 * It also holds the ring
 */

class FFT_Data
{
  Ring R;
  Zp_Data prData;

  vector<modp> root; // 2m'th Root of Unity mod pr and it's inverse

  modp iphi; // 1/phi_m mod pr

public:
  void init(const Ring &Rg, const Zp_Data &PrD);

  void init_field() const
  {
    gfp::init_field(prData.pr);
  }

  FFT_Data()
  {
    ;
  }
  FFT_Data(const Ring &Rg, const Zp_Data &PrD)
  {
    init(Rg, PrD);
  }

  const Zp_Data &get_prD() const
  {
    return prData;
  }
  const bigint &get_prime() const
  {
    return prData.pr;
  }
  int phi_m() const
  {
    return R.phi_m();
  }
  int m() const
  {
    return R.m();
  }
  int num_slots() const
  {
    return R.phi_m();
  }

  int p(int i) const
  {
    return R.p(i);
  }
  int p_inv(int i) const
  {
    return R.p_inv(i);
  }
  const vector<int> &Phi() const
  {
    return R.Phi();
  }

  modp get_root(int i) const
  {
    return root[i];
  }
  modp get_iphi() const
  {
    return iphi;
  }

  const Ring &get_R() const
  {
    return R;
  }

  friend ostream &operator<<(ostream &s, const FFT_Data &FFTD);
  friend istream &operator>>(istream &s, FFT_Data &FFTD);
};

#endif
