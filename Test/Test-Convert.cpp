/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#include <fstream>
#include <iostream>
using namespace std;

#include "GC/Circuit.h"
#include "GC/SimplifyCircuit.h"
#include "Math/bigint.h"
#include "Tools/random.h"

/* This routine tests the conversion and simplification done in the
 * Circuits directory, to ensure we get the correct functionality 
 * for the main circuits we use
 */

/* This routine here is used to test simplification on the main circuits
 * within this file. This is helpful for debugging purposes sometimes
 */
void Simplify(Circuit &C, int iter)
{
  SimplifyCircuit SC(C);
  SC.Simplify();
  bool flag= true;
  int numI= 0;
  while (flag && numI < iter)
    {
      C= SC.Get_Circuit();
      cout << "\t" << numI << " : ";
      cout << "nwires : " << C.get_nWires() << "    ";
      cout << "ngates : " << C.get_nGates() << "    ";
      cout << "and gt : " << C.num_AND_gates() << endl;
      flag= SC.Search_SubCircuit();
      SC.Simplify();
      numI++;
    }
  C= SC.Get_Circuit();
}

unsigned long bits_to_ulong(const vector<int> &bits, int bound)
{
  unsigned long x= 0;
  for (int i= bound - 1; i >= 0; i--)
    {
      x= 2 * x + bits[i];
    }
  return x;
}

unsigned long random_int(PRNG &G)
{
  unsigned long x= G.get_uint();
  unsigned long y= G.get_uint();

  x= (x << 32) + y;

  return x;
}

void test_adder64(PRNG &G)
{
  Circuit C;
  ifstream inpf("Circuits/Bristol/adder64.txt");
  inpf >> C;
  inpf.close();

  vector<vector<int>> inp, outp;
  inp.resize(2);
  outp.resize(1);
  inp[0].resize(64);
  inp[1].resize(64);
  outp[0].resize(64);

  unsigned long x, y, z, zz;
  for (unsigned long i= 0; i < 100; i++)
    {
      x= random_int(G);
      y= random_int(G);
      z= x + y;
      ulong_to_bits(inp[0], x);
      ulong_to_bits(inp[1], y);
      C.evaluate(inp, outp);
      zz= bits_to_ulong(outp[0], 64);
      if (z != zz)
        {
          cout << "Error test_adder64:  " << z << " " << zz << endl;
        }
    }
}

void test_mult64(PRNG &G)
{
  Circuit C;
  ifstream inpf("Circuits/Bristol/mult64.txt");
  inpf >> C;
  inpf.close();

  vector<vector<int>> inp, outp;
  inp.resize(2);
  outp.resize(1);
  inp[0].resize(64);
  inp[1].resize(64);
  outp[0].resize(64);

  unsigned long x, y, z, zz;
  for (unsigned long i= 0; i < 100; i++)
    {
      x= random_int(G);
      y= random_int(G);
      z= x * y;
      ulong_to_bits(inp[0], x);
      ulong_to_bits(inp[1], y);
      C.evaluate(inp, outp);
      zz= bits_to_ulong(outp[0], 64);
      //cout<<zz<<" z=x*y "<< z<<"="<<x<<"*"<<y<<endl;
      if (z != zz)
        {
          cout << "Error test_mult64:  " << z << " " << zz << endl;
        }
    }
}

void test_divide64(PRNG &G)
{
  Circuit C;
  ifstream inpf("Circuits/Bristol/divide64.txt");
  inpf >> C;
  inpf.close();

  vector<vector<int>> inp, outp;
  inp.resize(2);
  outp.resize(1);
  inp[0].resize(64);
  inp[1].resize(64);
  outp[0].resize(64);

  unsigned long x, y, z, zz;
  for (unsigned long i= 0; i < 100; i++)
    {
      x= random_int(G);
      y= random_int(G);
      z= (signed long) x / (signed long) y;
      ulong_to_bits(inp[0], x);
      ulong_to_bits(inp[1], y);
      C.evaluate(inp, outp);
      zz= bits_to_ulong(outp[0], 64);
      if (z != zz)
        {
          cout << "Error test_divide64:  " << i << endl;
          cout << "\t x  = " << x << " " << (signed long) x << endl;
          cout << "\t y  = " << y << " " << (signed long) y << endl;
          cout << "\t z  = " << z << " " << (signed long) z << endl;
          cout << "\t zz = " << zz << " " << (signed long) zz << endl;
        }
    }
}

void test_sub64(PRNG &G)
{
  Circuit C;
  ifstream inpf("Circuits/Bristol/sub64.txt");
  inpf >> C;
  inpf.close();

  vector<vector<int>> inp, outp;
  inp.resize(2);
  outp.resize(1);
  inp[0].resize(64);
  inp[1].resize(64);
  outp[0].resize(64);

  unsigned long x, y, z, zz;
  for (unsigned long i= 0; i < 100; i++)
    {
      x= random_int(G);
      y= random_int(G);
      z= x - y;
      ulong_to_bits(inp[0], x);
      ulong_to_bits(inp[1], y);
      C.evaluate(inp, outp);
      zz= bits_to_ulong(outp[0], 64);
      if (z != zz)
        {
          cout << "Error test_sub64:  " << z << " " << zz << endl;
        }
    }
}

void test_neg64(PRNG &G)
{
  Circuit C;
  ifstream inpf("Circuits/Bristol/neg64.txt");
  inpf >> C;
  inpf.close();

  vector<vector<int>> inp, outp;
  inp.resize(1);
  outp.resize(1);
  inp[0].resize(64);
  outp[0].resize(64);

  unsigned long x, z, zz;
  for (unsigned long i= 0; i < 100; i++)
    {
      x= random_int(G);
      z= -x;
      ulong_to_bits(inp[0], x);
      C.evaluate(inp, outp);
      zz= bits_to_ulong(outp[0], 64);
      if (z != zz)
        {
          cout << i << " Error test_neg64:  "
               << "x=" << x << " z=" << z << " zz=" << zz << endl;
        }
    }
}

void test_mult2_64(PRNG &G)
{
  Circuit C;
  ifstream inpf("Circuits/Bristol/mult2_64.txt");
  inpf >> C;
  inpf.close();

  vector<vector<int>> inp, outp;
  vector<int> v;
  inp.resize(2);
  outp.resize(1);
  inp[0].resize(64);
  inp[1].resize(64);
  outp[0].resize(64);
  outp[1].resize(64);

  unsigned __int128 x, y, z, zz;
  unsigned long top, bot, top1, bot1;
  for (unsigned long i= 0; i < 100; i++)
    {
      x= random_int(G);
      y= random_int(G);
      z= x * y;
      ulong_to_bits(inp[0], (unsigned long) x);
      ulong_to_bits(inp[1], (unsigned long) y);
      C.evaluate(inp, outp);
      top= bits_to_ulong(outp[0], 64);
      bot= bits_to_ulong(outp[1], 64);
      top1= z >> 64;
      bot1= z & 0xFFFFFFFFFFFFFFFF;
      zz= (((unsigned __int128) top) << 64) + bot;
      if (z != zz)
        {
          cout << "Error test_mult2_64: " << i << "\n\t";
          cout << (unsigned long) x << " " << (unsigned long) y << " : ";
          cout << top << " " << top1 << " : ";
          cout << bot << " " << bot1 << " : " << endl;
        }
    }
}

void test_zero_equal(PRNG &G)
{
  Circuit C;
  ifstream inpf("Circuits/Bristol/zero_equal.txt");
  inpf >> C;
  inpf.close();

  vector<vector<int>> inp, outp;
  inp.resize(1);
  outp.resize(1);
  inp[0].resize(64);
  outp[0].resize(1);

  unsigned long x, z, zz;
  for (unsigned long i= 0; i < 100; i++)
    {
      x= random_int(G);
      if (x == 0)
        {
          z= 1;
        }
      else
        {
          z= 0;
        }
      ulong_to_bits(inp[0], x);
      C.evaluate(inp, outp);
      zz= bits_to_ulong(outp[0], 1);
      if (z != zz)
        {
          cout << "Error test_zeroEqual:  " << z << " " << zz << endl;
        }
    }
}

int main()
{
  PRNG G;
  G.ReSeed(0);
  cout << "Testing adder64" << endl;
  test_adder64(G);
  cout << "Testing sub64" << endl;
  test_sub64(G);
  cout << "Testing neg64" << endl;
  test_neg64(G);
  cout << "Testing mult64" << endl;
  test_mult64(G);
  cout << "Testing mult2_64" << endl;
  test_mult2_64(G);
  cout << "Testing zero_equal" << endl;
  test_zero_equal(G);
  cout << "Testing divide64" << endl;
  test_divide64(G);
  return 0;
}
