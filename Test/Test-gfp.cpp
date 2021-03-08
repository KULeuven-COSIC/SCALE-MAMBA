/*
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#include <fstream>
using namespace std;

#include "Math/gfp.h"

void test_add(bigint a, bigint b, bigint c)
{
  gfp ga(a), gb(b), gc(c);
  gfp sum= ga + gb;
  if (sum != gc)
    {
      cout << ga << " + " << gb << " = " << sum << " != " << gc << " mod " << gfp::pr() << endl;
    }
}

void test_mul(bigint a, bigint b, bigint c)
{
  gfp ga(a), gb(b), gc(c);
  gfp prod= ga * gb;
  if (prod != gc)
    {
      cout << ga << " * " << gb << " = " << prod << " != " << gc << " mod " << gfp::pr() << endl;
    }
}

void test_arithmetic(const bigint &p)
{
  test_add(1, 1, 2);
  test_add(0, 1, 1);
  test_add(1, -1, 0);
  test_add(-1, -1, -2);

  test_mul(1, 1, 1);
  test_mul(0, 1, 0);
  test_mul(1, -1, -1);
  test_mul(-1, -1, 1);

  uint8_t seed[16];
  for (int i= 0; i < 16; i++)
    {
      seed[i]= 0;
    }
  PRNG G;
  G.SetSeed(seed, 16);
  bigint x, y, z;
  for (int i= 0; i < 1000; i++)
    {
      x= randomBnd(G, p);
      y= randomBnd(G, p);
      z= (x + y) % p;
      test_add(x, y, z);
      z= (x * y) % p;
      test_mul(x, y, z);
    }
}

void test(int num)
{
  bigint p;
  stringstream ss;
  ss << "Auto-Test-Data/" << num << "/SharingData.txt";
  cout << "Using prime for file " << ss.str() << endl;
  ifstream inpf(ss.str());
  inpf >> p;
  inpf.close();
  cout << "p = " << p << endl;

  gfp::init_field(p);

  test_arithmetic(p);

  cout << endl;
}

int main()
{
  test(1);
  test(14);

  /* Test current prime in SharingData.txt */
  bigint p;
  stringstream ss;
  ss << "Data/SharingData.txt";
  cout << "Using prime for file " << ss.str() << endl;
  ifstream inpf(ss.str());
  inpf >> p;
  inpf.close();
  cout << "p = " << p << endl;

  gfp::init_field(p);

  test_arithmetic(p);

  cout << endl;
}
