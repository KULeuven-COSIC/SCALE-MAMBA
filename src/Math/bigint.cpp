/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "bigint.h"
#include "Exceptions/Exceptions.h"
#include <sstream>
using namespace std;

bigint sqrRootMod(const bigint &a, const bigint &p)
{
  bigint ans;
  if (a == 0)
    {
      ans= 0;
      return ans;
    }
  if (mpz_tstbit(p.get_mpz_t(), 1) == 1)
    { // First do case with p=3 mod 4
      bigint exp= (p + 1) / 4;
      mpz_powm(ans.get_mpz_t(), a.get_mpz_t(), exp.get_mpz_t(), p.get_mpz_t());
    }
  else
    { // Shanks algorithm
      gmp_randclass Gen(gmp_randinit_default);
      Gen.seed(0);
      bigint x, y, n, q, t, b, temp;
      // Find n such that (n/p)=-1
      int leg= 1;
      while (leg != -1)
        {
          n= Gen.get_z_range(p);
          leg= mpz_legendre(n.get_mpz_t(), p.get_mpz_t());
        }
      // Split p-1 = 2^e q
      q= p - 1;
      int e= 0;
      while (mpz_even_p(q.get_mpz_t()))
        {
          e++;
          q= q / 2;
        }
      // y=n^q mod p, x=a^((q-1)/2) mod p, r=e
      int r= e;
      mpz_powm(y.get_mpz_t(), n.get_mpz_t(), q.get_mpz_t(), p.get_mpz_t());
      temp= (q - 1) / 2;
      mpz_powm(x.get_mpz_t(), a.get_mpz_t(), temp.get_mpz_t(), p.get_mpz_t());
      // b=a*x^2 mod p, x=a*x mod p
      b= (a * x * x) % p;
      x= (a * x) % p;
      // While b!=1 do
      while (b != 1)
        { // Find smallest m such that b^(2^m)=1 mod p
          int m= 1;
          temp= (b * b) % p;
          while (temp != 1)
            {
              temp= (temp * temp) % p;
              m++;
            }
          // t=y^(2^(r-m-1)) mod p, y=t^2, r=m
          t= y;
          for (int i= 0; i < r - m - 1; i++)
            {
              t= (t * t) % p;
            }
          y= (t * t) % p;
          r= m;
          // x=x*t mod p, b=b*y mod p
          x= (x * t) % p;
          b= (b * y) % p;
        }
      ans= x;
    }
  return ans;
}

bigint powerMod(const bigint &x, const bigint &e, const bigint &p)
{
  bigint ans;
  if (e >= 0)
    {
      mpz_powm(ans.get_mpz_t(), x.get_mpz_t(), e.get_mpz_t(), p.get_mpz_t());
    }
  else
    {
      bigint xi, ei= -e;
      invMod(xi, x, p);
      mpz_powm(ans.get_mpz_t(), xi.get_mpz_t(), ei.get_mpz_t(), p.get_mpz_t());
    }

  return ans;
}

int powerMod(int x, int e, int p)
{
  if (e == 1)
    {
      return x;
    }
  if (e == 0)
    {
      return 1;
    }
  if (e < 0)
    {
      throw not_implemented();
    }
  int t= x, ans= 1;
  while (e != 0)
    {
      if ((e & 1) == 1)
        {
          ans= (ans * t) % p;
        }
      e>>= 1;
      t= (t * t) % p;
    }
  return ans;
}

bigint randomBnd(PRNG &G, const bigint &B)
{
  bigint x;
  // Hash the seed again and again until we have a lot of len bytes
  unsigned int len= ((2 * numBytes(B)) / RAND_SIZE + 1) * RAND_SIZE;
  uint8_t *bytes= new uint8_t[len];
  G.get_random_bytes(bytes, len);
  bigintFromBytes(x, bytes, len);
  x= x % B;
  delete[] bytes;
  return x;
}

void outputBigint(string &s, const bigint &x)
{
  int num= numBytes(x);
  uint8_t *buff= new uint8_t[num + 5];

  // Encode the sign
  buff[0]= 0;
  if (x < 0)
    {
      buff[0]= 1;
    }

  encode_length(buff + 1, num);

  bytesFromBigint(buff + 5, x, num);
  s.append((char *) buff, num + 5);
  delete[] buff;
}

void inputBigint(string &s, bigint &x)
{
  int sign= s.c_str()[0];
  if (sign != 0 && sign != 1)
    {
      throw bad_value();
    }

  long num= decode_length((uint8_t *) s.c_str() + 1);

  x= 0;
  if (num != 0)
    {
      bigintFromBytes(x, (uint8_t *) s.c_str() + 5, num);
      if (sign == 1)
        {
          x= -x;
        }
    }
  s.erase(0, num + 5);
}

bigint compute_binomial(int n, int k)
{
  // compute ((k+1) * ... * (n-1) * n) / (n-k)!
  bigint denom= 1; // Compute factorial[n-k]
  for (int i= 1; i <= n - k; i++)
    denom*= i;

  bigint nominator= 1;
  for (int i= k + 1; i <= n; i++)
    nominator*= i;

  return nominator / denom;
}

void bigint_to_bits(vector<int> &bits, const bigint &x)
{
  bigint y= x;
  for (unsigned long i= 0; i < bits.size(); i++)
    {
      bits[i]= 0;
      if ((y & 1) == 1)
        {
          bits[i]= 1;
        }
      y>>= 1;
    }
  if (y != 0)
    {
      throw arithmetic_bug();
    }
}

void ulong_to_bits(vector<int> &bits, unsigned long x)
{
  for (unsigned long i= 0; i < 64; i++)
    {
      bits[i]= x & 1;
      x>>= 1;
    }
}

void long_to_bits(vector<int> &bits, long x)
{
  for (unsigned long i= 0; i < 64; i++)
    {
      bits[i]= x & 1;
      x>>= 1;
    }
}
