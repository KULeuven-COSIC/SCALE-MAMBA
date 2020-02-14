/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _FHE_Params
#define _FHE_Params

/* Class to hold the FHE Parameters
 *
 * The idea is that there is a single global FHE_Params structure
 * called params; which holds all the data
 *
 * Likewise initialising params sets up all the data for other
 * classes such the global ring etc
 *
 */
#include "FHE/Distributions.h"
#include "FHE/FFT_Data.h"
#include "Tools/random.h"
#include "config.h"

enum PoKVersion { HighGear,
                  TopGear };

/* Generates N,p0,p1 and p given input hwt h, log2p, n=nplayers
 *   - Uses statistical/computational security parameters from config.h
 *   - If p=0 this generates a new prime, otherwise it tries to use the
 *     the input prime. Will throw an error if this is not compatible
 *   - The default values here are what we use in the main system
 *     We have them as parameters here so we can test different
 *     things in the Test routines (FHE-p.x)
 *
 *     Use h=-1 to define a secret key with discrete Gaussian as
 *     opposed to Hwt distribution
 */
void Generate_Parameters(unsigned int &N, bigint &p0, bigint &p1, bigint &p, int lg2p,
                         unsigned int n,
                         PoKVersion version,
                         unsigned int h= HwtSK,
                         int NewHopeB_t= NewHopeB,
                         int comp_sec_t= comp_sec,
                         int DD_stat_sec_t= DD_stat_sec,
                         int ZK_sound_sec_t= ZK_sound_sec,
                         int ZK_slack_sec_t= ZK_ZK_sec);

class FHE_Params
{
protected:
  // Pair of moduli in FFTData[0] and FFTData[1] for the levels
  vector<FFT_Data> FFTData;

  // Data for distributed decryption
  bigint Bval; // Bound

  // Hamming weight of secret key or -1 if using Gaussian secret key
  int hwt;

public:
  FHE_Params()
      : FFTData(2)
  {
  }

  // Rely on default copy assignment/constructor (not that they should
  // ever be needed)

  void set(const Ring &R, const bigint &pr0, const bigint &pr1, int hwtSK, unsigned int n, bool check= true);

  int get_hwt() const { return hwt; }

  const vector<FFT_Data> &FFTD() const
  {
    return FFTData;
  }

  vector<bigint> sample_Hwt(PRNG &G) const;

  const bigint &p0() const
  {
    return FFTData[0].get_prime();
  }
  const bigint &p1() const
  {
    return FFTData[1].get_prime();
  }
  bigint Q() const
  {
    return p0() * p1();
  }

  const bigint &B() const
  {
    return Bval;
  }

  unsigned int phi_m() const
  {
    return FFTData[0].phi_m();
  }
  const Ring &get_ring()
  {
    return FFTData[0].get_R();
  }
};

#endif
