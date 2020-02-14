/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

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
#define num_params 7
int FHE_Sec_Params[num_params][4]= {
    {1024, 40, 25, 12}, {2048, 82, 52, 26}, {4096, 167, 106, 56}, {8192, 340, 215, 115}, {16384, 686, 436, 235}, {32768, 1392, 879, 473}, {65536, 2830, 1778, 953}};

void produce_epsilon_constants(double C[3])
{
  for (int i= 0; i < 3; i++)
    {
      C[i]= -1;
    }
  for (double x= 0.1; x < 10.0; x+= .1)
    {
      double t= erfc(x), tp= 1;
      for (int i= 1; i < 3; i++)
        {
          tp*= t;
          double lgtp= log(tp) / log(2.0);
          if (C[i] < 0 && lgtp < FHE_epsilon)
            {
              C[i]= pow(x, i);
            }
        }
    }
}

// Make a prime of lg2 bits for which p-1 is divisible by 2*N
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
void Generate_Parameters(unsigned int &N, bigint &p0, bigint &p1, bigint &p, int lg2p,
                         unsigned int n,
                         PoKVersion version,
                         unsigned int h,
                         int NewHopeB_t,
                         int comp_sec_t,
                         int DD_stat_sec_t,
                         int ZK_sound_sec_t,
                         int ZK_slack_sec_t)
{
  double pp= exp2((double) lg2p), ss= exp2((double) DD_stat_sec_t);
  double Sslack= exp2((double) ZK_slack_sec_t);
  double lgp0, lgp1, lgq;
  double sigma= sqrt(NewHopeB_t / 2.0);

  double C[3];
  produce_epsilon_constants(C);

  int index;
  switch (comp_sec_t)
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

  int lg2N, U= 0, V= 0; // Assign U and V to avoid a compiler warning
  double Ssound= exp2((double) ZK_sound_sec_t);
  double S32= Sslack * sqrt(Ssound);

  /* We first go through the possible N values */
  bool done= false;
  for (int i= 0; i < num_params && !done; i++)
    {
      N= FHE_Sec_Params[i][0];
      double phim= N;
      double varsk= sigma * sqrt(phim);
      if (h > 0)
        {
          varsk= sqrt((double) h);
        }

      // New
      double B_Clean=
          1.0 / 2 + NewHopeB * C[1] * sigma * sqrt(phim) + NewHopeB + NewHopeB * C[1] * varsk;
      if (version == HighGear)
        {
          B_Clean*= phim * S32 * 2 * n * pp;
        }
      else
        {
          lg2N= CEIL_LOG2(2 * N + 1);
          V= DIV_CEIL(ZK_sound_sec_t + 2, lg2N);
          U= 2 * V;
          B_Clean*= phim * Sslack * 4.0 * n * pp;
        }

      double B_Scale= pp * (C[1] * sqrt(phim / 12) + C[2] * sqrt(phim / 12) * varsk);
      double B_KS= pp * C[2] * sigma * phim / sqrt(12);
      for (lgq= 10; lgq < FHE_Sec_Params[i][index] && !done; lgq+= 10)
        {
          for (lgp0= 0; lgp0 < lgq - 1 && !done; lgp0++)
            {
              lgp1= lgq - lgp0;
              double pp0= exp2(lgp0), pp1= exp2(lgp1);

              double U1= (B_Clean / pp1 + B_Scale);
              U1= U1 * U1 + B_KS * pp0 / pp1 + B_Scale;
              double U2= U1 + B_Clean / pp1 + B_Scale;

              double LHS= 2 * U2 * (1 + n * ss);
              if (LHS < pp0)
                {
                  if (p == 0)
                    {
                      p= make_prime(lg2p, N);
                    }
                  else
                    {
                      if (!probPrime(p) || ((p - 1) % (2 * N)) != 0)
                        {
                          throw FHE_params();
                        }
                    }
                  p0= make_prime(lgp0, N);
                  p1= make_prime(lgp1, N, p, p0);
                  done= true;
                }
            }
        }
    }
  if (!done)
    {
      throw FHE_params();
    }
  cout << "N  = " << N << endl;
  if (version == TopGear)
    {
      cout << "U = " << U << "  V = " << V << endl;
    }
  cout << "p  = " << p << " : " << lg2p << " " << numBits(p) << " "
       << p % (2 * N) << endl;
  cout << "p0 = " << p0 << " : " << lgp0 << " " << numBits(p0) << " "
       << p0 % (2 * N) << endl;
  cout << "p1 = " << p1 << " : " << lgp1 << " " << numBits(p1) << " "
       << p1 % (2 * N) << endl;
}

void FHE_Params::set(const Ring &R, const bigint &p0, const bigint &p1,
                     int h, unsigned int n,
                     bool check)
{
  Zp_Data prD0(p0);
  Zp_Data prD1(p1);

  FFTData.resize(2);
  FFTData[0].init(R, prD0);
  FFTData[1].init(R, prD1);

  hwt= h;

  Bval= 1;
  Bval= Bval << DD_stat_sec;
  Bval= FFTData[0].get_prime() / (2 * (1 + n * Bval));
  if (Bval == 0 && check)
    throw runtime_error("distributed decryption bound is zero");
}

vector<bigint> FHE_Params::sample_Hwt(PRNG &G) const
{
  unsigned int n= FFTData[0].phi_m();
  vector<bigint> ans(n);
  for (unsigned int i= 0; i < n; i++)
    {
      ans[i]= 0;
    }
  uint8_t ch= 0;
  int cnt= 0, j= 0;
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
