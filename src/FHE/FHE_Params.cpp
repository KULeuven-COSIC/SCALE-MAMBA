/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "FHE_Params.h"
#include "Exceptions/Exceptions.h"
#include "FHE/Ring_Element.h"
#include "Math/gfp.h"
#include "config.h"
#include "math.h"

/* The following table is the crucial table to determine FHE
 * security parameters. Please see the Chapter in the document
 * for how this is created etc
 */
#define num_params 6
int FHE_Sec_Params[num_params][4]= {
    {1024, 44, 29, 16}, {2048, 86, 56, 31}, {4096, 171, 111, 60}, {8192, 344, 220, 120}, {16384, 690, 440, 239}, {32768, 998, 883, 478}};

#define sigma 3.16

// Make a prime of lg2 bits for which N-1 is divisible by 2*N
//   We ensure that p!=not_x
//   We also ensure that p=1 mod q if q!=0
bigint make_prime(int lg2, int N, const bigint &q= 0,
                  const bigint &not_x= 0)
{
  bigint step= 2 * N;
  if (q != 0)
    {
      step= 2 * N * q;
    }
  int te= lg2 - numBits(step);
  bigint p= (bigint(1) << te) * step + 1;
  while (!probPrime(p) || p == not_x)
    {
      p+= step;
    }
  return p;
}

/* Generates N,p0,p1 and p given input hwt h, log2p, n=nplayers */
void Generate_Parameters(int &N, bigint &p0, bigint &p1, bigint &p, int lg2p,
                         int h, int n)
{
  double pp= exp2((double) lg2p), ss= exp2((double) stat_sec),
         S= exp2(stat_sec);
  double S32= S * sqrt(S);
  double lgp0, lgp1, lgq;
  double hh= h;
  int index;
  switch (comp_sec)
    {
      case 80:
        index= 1;
        break;
      case 128:
        index= 2;
        break;
      case 256:
        index= 3;
        break;
      default:
        throw invalid_params();
        break;
    }
  /* We first go through the possible N values */
  bool done= false;
  p= -1;
  for (int i= 0; i < num_params && !done; i++)
    {
      N= FHE_Sec_Params[i][0];
      double phim= N;

      // New
      double B_Clean=
          1.0 / 2 + 20 * 6 * sigma * sqrt(phim) + 20 + 20 * 6 * sqrt(h);
      B_Clean*= phim * S32 * 2 * n * pp;

      double B_Scale= pp * sqrt(3 * phim) * (1 + 8 * sqrt(hh / 3));
      double B_KS=
          pp * phim * sigma * (1.49 * sqrt(hh * phim) + 2.11 * hh +
                               5.54 * sqrt(hh) + 1.96 * sqrt(phim) + 4.62);
      for (lgq= 10; lgq < FHE_Sec_Params[i][index] && !done; lgq+= 10)
        {
          for (lgp0= 0; lgp0 < lgq - 1 && !done; lgp0++)
            {
              lgp1= lgq - lgp0;
              double pp0= exp2(lgp0), pp1= exp2(lgp1);

              double v= n * B_Clean;

              double U1= (v / pp1 + B_Scale);
              U1= U1 * U1 + B_KS * pp0 / pp1 + B_Scale;
              double U2= U1 + n * B_Clean / pp1 + B_Scale;

              double LHS= 2 * U2 * (1 + n * ss);
              if (LHS < pp0)
                {
                  p= make_prime(lg2p, N);
                  p0= make_prime(lgp0, N);
                  p1= make_prime(lgp1, N, p, p0);
                  done= true;
                }
            }
        }
    }
  if (p < 0)
    {
      throw FHE_params();
    }
  cout << "N  = " << N << endl;
  cout << "p  = " << p << " : " << lg2p << " " << numBits(p) << " "
       << p % (2 * N) << endl;
  cout << "p0 = " << p0 << " : " << lgp0 << " " << numBits(p0) << " "
       << p0 % (2 * N) << endl;
  cout << "p1 = " << p1 << " : " << lgp1 << " " << numBits(p1) << " "
       << p1 % (2 * N) << endl;
}

void FHE_Params::set(const Ring &R, const bigint &p0, const bigint &p1, int h, int n,
                     bool check)
{
  Zp_Data prD0(p0);
  Zp_Data prD1(p1);

  FFTData.resize(2);
  FFTData[0].init(R, prD0);
  FFTData[1].init(R, prD1);

  hwt= h;

  Bval= 1;
  Bval= Bval << stat_sec;
  Bval= FFTData[0].get_prime() / (2 * (1 + n * Bval));
  if (Bval == 0 && check)
    throw runtime_error("distributed decryption bound is zero");
}

vector<bigint> FHE_Params::sample_Hwt(PRNG &G) const
{
  int n= FFTData[0].phi_m();
  vector<bigint> ans(n);
  for (int i= 0; i < n; i++)
    {
      ans[i]= 0;
    }
  int cnt= 0, j= 0;
  uint8_t ch= 0;
  while (cnt < hwt)
    {
      unsigned int i= G.get_uint() % n;
      if (ans[i] == 0)
        {
          cnt++;
          if (j == 0)
            {
              j= 8;
              ch= G.get_uchar();
            }
          int v= ch & 1;
          j--;
          if (v == 0)
            {
              ans[i]= -1;
            }
          else
            {
              ans[i]= 1;
            }
        }
    }
  return ans;
}
