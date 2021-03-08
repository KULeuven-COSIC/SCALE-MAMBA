/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

/* Tests the circuit evaluation on floating pint circuits */

#include "GC/Circuit.h"
#include "Tools/random.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
using namespace std;

#define num_tests 1000

void double_to_bits(double x, vector<int> &bits)
{
  uint8_t *ptr= (uint8_t *) &x;

  for (int index= 0; index < 8; index++)
    {
      uint8_t byte= ptr[index];
      for (int i= 0; i < 8; i++)
        {
          bits[63 - i - index * 8]= byte & 1;
          byte>>= 1;
        }
    }
}

void long_to_bits(unsigned long x, vector<int> &bits)
{
  for (int index= 0; index < 64; index++)
    {
      bits[index]= x & 1;
      x>>= 1;
    }
}

void bits_to_long(const vector<int> &bits, long &x)
{
  x= 0;
  for (int index= 0; index < 64; index++)
    {
      x<<= 1;
      x+= bits[63 - index];
    }
}

void bits_to_ulong(const vector<int> &bits, unsigned long &x)
{
  x= 0;
  for (int index= 0; index < 64; index++)
    {
      x<<= 1;
      x+= bits[index];
    }
}

void bits_to_double(const vector<int> &bits, double &x)
{
  uint8_t *ptr= (uint8_t *) &x;

  for (int index= 0; index < 8; index++)
    {
      uint8_t byte= 0;
      for (int i= 0; i < 8; i++)
        {
          byte<<= 1;
          byte+= bits[56 + i - index * 8];
        }
      ptr[index]= byte;
    }
}

void ulong_to_double(unsigned long y, double &x)
{
  // First convert long to bits
  vector<int> bits(64);
  for (int index= 0; index < 64; index++)
    {
      bits[63 - index]= y & 1;
      y>>= 1;
    }
  // Now convert bits to double
  uint8_t *ptr= (uint8_t *) &x;
  for (int index= 0; index < 8; index++)
    {
      uint8_t byte= 0;
      for (int i= 0; i < 8; i++)
        {
          byte<<= 1;
          byte+= bits[56 + i - index * 8];
        }
      ptr[index]= byte;
    }
}

void extract_data(int &sign, int &exponent, unsigned long &mantissa, const vector<int> &bits)
{
  sign= bits[0];
  exponent= 0;
  mantissa= 1;
  for (int i= 0; i < 11; i++)
    {
      exponent<<= 1;
      exponent+= bits[i + 1];
    }
  exponent-= 1024;
  for (int i= 0; i < 52; i++)
    {
      mantissa<<= 1;
      mantissa+= bits[i + 12];
    }
}

void print_bits(const vector<int> &bits)
{
  for (int i= 0; i < 64; i++)
    {
      cout << bits[i];
      if (i == 0 || i == 11)
        {
          cout << " ";
        }
    }
  cout << endl;
}

void print_bits_l(const vector<int> &bits)
{
  for (int i= 0; i < 64; i++)
    {
      cout << bits[i];
    }
  cout << endl;
}

// When flag=true be full random, otherwise floating bits
// which are not too far away from a long
void random_bits(vector<int> &bits, PRNG &G, bool flag= true)
{
  for (int i= 0; i < 64; i++)
    {
      bits[i]= G.get_uchar() & 1;
    }
  if (!flag)
    {
      bits[1]= 1;
      bits[2]= 0;
      bits[3]= 0;
      bits[4]= 0;
      bits[5]= 0;
    }
}

void test_FP_Add(PRNG &G)
{
  cout << "Testing a FP-Add" << endl;
  Circuit C;
  ifstream inpf("Circuits/Bristol/FP-add.txt");
  inpf >> C;
  inpf.close();

  vector<vector<int>> inp, outp;
  inp.resize(2);
  inp[0].resize(64);
  inp[1].resize(64);
  outp.resize(1);
  outp[0].resize(64);

  vector<int> x(64), y(64), z(64), z2(64);
  double xx, yy, zz, ans, test;
  int sign, exponent;
  unsigned long mantissa;
  for (unsigned long i= 0; i < num_tests; i++)
    {
      random_bits(x, G);
      random_bits(y, G);
      bits_to_double(x, xx);
      bits_to_double(y, yy);
      zz= xx + yy;
      double_to_bits(zz, z);

      for (unsigned int j= 0; j < 64; j++)
        {
          inp[0][j]= x[63 - j];
          inp[1][j]= y[63 - j];
        }
      C.evaluate(inp, outp);
      for (unsigned int j= 0; j < 64; j++)
        {
          z2[j]= outp[0][63 - j];
        }
      bits_to_double(z2, ans);

      test= zz - ans;
      if (test != 0 && !isnan(test))
        {
          cout << "Error: Add" << endl;
          cout << i << " : " << xx << "\t" << yy << "\t" << zz << "\t" << ans << "\t" << endl;

          cout << "x : ";
          print_bits(x);
          extract_data(sign, exponent, mantissa, x);
          cout << "\t" << sign << " " << exponent << " " << mantissa << endl;
          cout << "y : ";
          print_bits(y);
          extract_data(sign, exponent, mantissa, y);
          cout << "\t" << sign << " " << exponent << " " << mantissa << endl;
          cout << "z : ";
          print_bits(z);
          extract_data(sign, exponent, mantissa, z);
          cout << "\t" << sign << " " << exponent << " " << mantissa << endl;
          cout << "o : ";
          print_bits(z2);
          extract_data(sign, exponent, mantissa, z2);
          cout << "\t" << sign << " " << exponent << " " << mantissa << endl;
          cout << endl;
        }
    }
}

void test_FP_Mul(PRNG &G)
{
  cout << "Testing a FP-Mul" << endl;
  Circuit C;
  ifstream inpf("Circuits/Bristol/FP-mul.txt");
  inpf >> C;
  inpf.close();

  vector<vector<int>> inp, outp;
  inp.resize(2);
  inp[0].resize(64);
  inp[1].resize(64);
  outp.resize(1);
  outp[0].resize(64);

  vector<int> x(64), y(64), z(64), z2(64);
  double xx, yy, zz, ans, test;
  int sign, exponent;
  unsigned long mantissa;
  for (unsigned long i= 0; i < num_tests; i++)
    {
      random_bits(x, G);
      random_bits(y, G);
      bits_to_double(x, xx);
      bits_to_double(y, yy);
      zz= xx * yy;
      double_to_bits(zz, z);

      for (unsigned int j= 0; j < 64; j++)
        {
          inp[0][j]= x[63 - j];
          inp[1][j]= y[63 - j];
        }
      C.evaluate(inp, outp);
      for (unsigned int j= 0; j < 64; j++)
        {
          z2[j]= outp[0][63 - j];
        }
      bits_to_double(z2, ans);

      test= zz - ans;
      if (test != 0 && !isnan(test))
        {
          cout << "Error: Mul" << endl;
          cout << i << " : " << xx << "\t" << yy << "\t" << zz << "\t" << ans << "\t" << endl;
          cout << "x : ";
          print_bits(x);
          extract_data(sign, exponent, mantissa, x);
          cout << "\t" << sign << " " << exponent << " " << mantissa << endl;
          cout << "y : ";
          print_bits(y);
          extract_data(sign, exponent, mantissa, y);
          cout << "\t" << sign << " " << exponent << " " << mantissa << endl;
          cout << "z : ";
          print_bits(z);
          extract_data(sign, exponent, mantissa, z);
          cout << "\t" << sign << " " << exponent << " " << mantissa << endl;
          cout << "o : ";
          print_bits(z2);
          extract_data(sign, exponent, mantissa, z2);
          cout << "\t" << sign << " " << exponent << " " << mantissa << endl;
          cout << endl;
        }
    }
}

void test_FP_Div(PRNG &G)
{
  cout << "Testing a FP-Div" << endl;
  Circuit C;
  ifstream inpf("Circuits/Bristol/FP-div.txt");
  inpf >> C;
  inpf.close();

  vector<vector<int>> inp, outp;
  inp.resize(2);
  inp[0].resize(64);
  inp[1].resize(64);
  outp.resize(1);
  outp[0].resize(64);

  vector<int> x(64), y(64), z(64), z2(64);
  double xx, yy, zz, ans, test;
  int sign, exponent;
  unsigned long mantissa;
  for (unsigned long i= 0; i < num_tests; i++)
    {
      random_bits(x, G);
      random_bits(y, G);
      bits_to_double(x, xx);
      bits_to_double(y, yy);
      zz= xx / yy;
      double_to_bits(zz, z);

      for (unsigned int j= 0; j < 64; j++)
        {
          inp[0][j]= x[63 - j];
          inp[1][j]= y[63 - j];
        }
      C.evaluate(inp, outp);
      for (unsigned int j= 0; j < 64; j++)
        {
          z2[j]= outp[0][63 - j];
        }
      bits_to_double(z2, ans);

      test= zz - ans;
      if (test != 0 && !isnan(test))
        {
          cout << "Error: Div" << endl;
          cout << i << " : " << xx << "\t" << yy << "\t" << zz << "\t" << ans << "\t" << endl;
          cout << "x : ";
          print_bits(x);
          extract_data(sign, exponent, mantissa, x);
          cout << "\t" << sign << " " << exponent << " " << mantissa << endl;
          cout << "y : ";
          print_bits(y);
          extract_data(sign, exponent, mantissa, y);
          cout << "\t" << sign << " " << exponent << " " << mantissa << endl;
          cout << "z : ";
          print_bits(z);
          extract_data(sign, exponent, mantissa, z);
          cout << "\t" << sign << " " << exponent << " " << mantissa << endl;
          cout << "o : ";
          print_bits(z2);
          extract_data(sign, exponent, mantissa, z2);
          cout << "\t" << sign << " " << exponent << " " << mantissa << endl;
          cout << endl;
        }
    }
}

void test_FP_Sqrt(PRNG &G)
{
  cout << "Testing a FP-Sqrt" << endl;
  Circuit C;
  ifstream inpf("Circuits/Bristol/FP-sqrt.txt");
  inpf >> C;
  inpf.close();

  vector<vector<int>> inp, outp;
  inp.resize(1);
  inp[0].resize(64);
  outp.resize(1);
  outp[0].resize(64);

  vector<int> x(64), z(64), z2(64);
  double xx, zz, ans, test;
  int sign, exponent;
  unsigned long mantissa;
  for (unsigned long i= 0; i < num_tests; i++)
    {
      random_bits(x, G);
      bits_to_double(x, xx);
      zz= sqrt(xx);
      double_to_bits(zz, z);

      for (unsigned int j= 0; j < 64; j++)
        {
          inp[0][j]= x[63 - j];
        }
      C.evaluate(inp, outp);
      for (unsigned int j= 0; j < 64; j++)
        {
          z2[j]= outp[0][63 - j];
        }
      bits_to_double(z2, ans);

      test= zz - ans;
      if (test != 0 && !isnan(test))
        {
          cout << "Error: Sqrt" << endl;
          cout << i << " : " << xx << "\t" << zz << "\t" << ans << "\t" << endl;
          cout << "x : ";
          print_bits(x);
          extract_data(sign, exponent, mantissa, x);
          cout << "\t" << sign << " " << exponent << " " << mantissa << endl;
          cout << "z : ";
          print_bits(z);
          extract_data(sign, exponent, mantissa, z);
          cout << "\t" << sign << " " << exponent << " " << mantissa << endl;
          cout << "o : ";
          print_bits(z2);
          extract_data(sign, exponent, mantissa, z2);
          cout << "\t" << sign << " " << exponent << " " << mantissa << endl;
          cout << endl;
        }
    }
}

void test_FP_F2I(PRNG &G)
{
  cout << "Testing a FP-F2I" << endl;
  Circuit C;
  ifstream inpf("Circuits/Bristol/FP-f2i.txt");
  inpf >> C;
  inpf.close();

  vector<vector<int>> inp, outp;
  inp.resize(1);
  inp[0].resize(64);
  outp.resize(1);
  outp[0].resize(64);

  vector<int> x(64), z(64), z2(64);
  int sign, exponent;
  unsigned long mantissa;
  double xx;
  unsigned long zz, ans, test;
  bool flag= false;
  for (unsigned long i= 0; i < 2 * num_tests; i++)
    {
      random_bits(x, G, flag);
      bits_to_double(x, xx);
      zz= (long) nearbyint(xx); // Round nearest-even rounding
      long_to_bits(zz, z);

      for (unsigned int j= 0; j < 64; j++)
        {
          inp[0][j]= x[63 - j];
        }
      C.evaluate(inp, outp);
      for (unsigned int j= 0; j < 64; j++)
        {
          z2[j]= outp[0][63 - j];
        }
      bits_to_ulong(z2, ans);

      test= zz - ans;
      if (test != 0 && zz != (1UL << 63))
        {
          cout << "Error: F2I : " << test << endl;
          cout << i << " : " << xx << "\t" << zz << "\t" << ans << "\t" << endl;
          cout << "x : ";
          print_bits(x);
          extract_data(sign, exponent, mantissa, x);
          cout << "\t" << sign << " " << exponent << " " << mantissa << endl;
          cout << "z : ";
          print_bits_l(z);
          cout << endl;
        }
      flag= not flag;
    }
}

void test_FP_I2F(PRNG &G)
{
  cout << "Testing a FP-I2F" << endl;
  Circuit C;
  ifstream inpf("Circuits/Bristol/FP-i2f.txt");
  inpf >> C;
  inpf.close();

  vector<vector<int>> inp, outp;
  inp.resize(1);
  inp[0].resize(64);
  outp.resize(1);
  outp[0].resize(64);

  vector<int> x(64), z(64), z2(64);
  int sign, exponent;
  unsigned long mantissa;
  double zz, ans, test;
  long xx;
  for (unsigned long i= 0; i < num_tests; i++)
    {
      random_bits(x, G);
      bits_to_long(x, xx);
      zz= (double) (xx);
      double_to_bits(zz, z);

      for (unsigned int j= 0; j < 64; j++)
        {
          inp[0][j]= x[j];
        }
      C.evaluate(inp, outp);
      for (unsigned int j= 0; j < 64; j++)
        {
          z2[j]= outp[0][63 - j];
        }
      bits_to_double(z2, ans);

      test= zz - ans;
      if (test != 0)
        {
          cout << "Error: I2F : " << test << endl;
          cout << i << " : " << xx << "\t" << zz << "\t" << ans << "\t" << endl;
          cout << "x : ";
          print_bits(z);
          extract_data(sign, exponent, mantissa, x);
          cout << "\t" << sign << " " << exponent << " " << mantissa << endl;
          cout << "z : ";
          print_bits_l(z);
          cout << "o : ";
          print_bits_l(z2);
          cout << endl;
        }
    }
}

void test_FP_EQ(PRNG &G)
{
  cout << "Testing a FP-EQ" << endl;
  Circuit C;
  ifstream inpf("Circuits/Bristol/FP-eq.txt");
  inpf >> C;
  inpf.close();

  vector<vector<int>> inp, outp;
  inp.resize(2);
  inp[0].resize(64);
  inp[1].resize(64);
  outp.resize(1);
  outp[0].resize(64);

  vector<int> x(64), y(64), z2(64);
  int sign, exponent;
  unsigned long mantissa;
  double xx, yy, test;
  unsigned long zz, ans;
  bool flag= false;
  for (unsigned long i= 0; i < 4 * num_tests; i++)
    {
      random_bits(x, G);
      if (flag)
        {
          y= x;
        }
      else
        {
          random_bits(y, G);
        }
      bits_to_double(x, xx);
      bits_to_double(y, yy);

      for (unsigned int j= 0; j < 64; j++)
        {
          inp[0][j]= x[63 - j];
          inp[1][j]= y[63 - j];
        }
      C.evaluate(inp, outp);
      for (unsigned int j= 0; j < 64; j++)
        {
          z2[j]= outp[0][63 - j];
        }
      bits_to_ulong(z2, ans);

      zz= 0;
      if (flag == true)
        {
          zz= 1;
        }
      test= zz - ans;
      if (test != 0)
        {
          cout << "Error: EQ : " << test << endl;
          cout << i << " : " << xx << "\t" << yy << "\t" << zz << "\t" << ans << "\t" << endl;
          cout << "x : ";
          print_bits(x);
          extract_data(sign, exponent, mantissa, x);
          cout << "\t" << sign << " " << exponent << " " << mantissa << endl;
          cout << "y : ";
          print_bits(y);
          extract_data(sign, exponent, mantissa, y);
          cout << "\t" << sign << " " << exponent << " " << mantissa << endl;
          cout << endl;
        }
      flag= not flag;
    }
}

/*
void test_FP_CEIL_LOG2(PRNG& G)
{
  cout << "Testing a FP-CEIL_LOG2" << endl;
  Circuit C;
  ifstream inpf("Circuits/Bristol/FP-ceil-log2.txt");
  inpf >> C;
  inpf.close();

  vector<vector<int>> inp, outp;
  inp.resize(1);  inp[0].resize(64);
  outp.resize(1); outp[0].resize(64);

  vector<int> x(64),z(64);
  int sign,exponent;
  unsigned long mantissa;
  double xx;
  long zz,ans,test;
  bool flag=false;
  for (unsigned long i=0; i<4*num_tests; i++)
    { random_bits(x,G,flag);
      bits_to_double(x,xx);
      zz=(long) floor(max(log2(fabs(xx)),0.0)); 
      long_to_bits(zz,z);

      for (unsigned int j=0; j<64; j++)
        { inp[0][j]=x[63-j]; }
      C.evaluate(inp, outp);
      bits_to_ulong(outp[0],ans);

      test=zz-ans;
      if (test!=0)
        { cout << "Error: CEIL_LOG2 : " << test << endl;
          cout << i << " : " << xx << "\t" << zz << "\t" << ans << "\t" << endl;
          cout << "x : "; print_bits(x);
          extract_data(sign,exponent,mantissa,x); cout << "\t" << sign << " " << exponent << " " << mantissa << endl;
          cout << "z : "; print_bits_l(z);
          cout << "o : "; print_bits_l(outp[0]);
          cout << endl;
        }
      flag=not flag;
   }
}
*/

int main()
{
  vector<int> a(64);
  double aa= 3.125;
  double_to_bits(aa, a);
  unsigned long te;
  bits_to_ulong(a, te);
  cout << aa << "\n";
  print_bits(a);
  cout << te << endl;
  double gg;
  ulong_to_double(te, gg);
  cout << gg << endl;

  PRNG G;
  G.ReSeed(0);
  test_FP_Add(G);
  test_FP_Mul(G);
  test_FP_Div(G);
  test_FP_Sqrt(G);
  test_FP_F2I(G);
  test_FP_I2F(G);
  test_FP_EQ(G);
  // test_FP_CEIL_LOG2(G);
}
