/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "GC/SimplifyCircuit.h"
#include "Math/bigint.h"
#include "config.h"
#include <fstream>

#define numT 50

/* Tests the simplification of 
 *   - A generic Mod512 bit adder circuit down to a specialized adder circuit
 *   - A generic LSSS_to_GC converter down to a specialized adder circuit
 */

void do_test(const vector<vector<int>> &x_bits, const vector<vector<int>> &y_bits,
             const vector<vector<int>> &z_bits,
             const Circuit &C)
{
  vector<vector<int>> inp(2), outp(1);
  outp[0].resize(z_bits[0].size());
  for (unsigned int i= 0; i < x_bits.size(); i++)
    {
      inp[0]= x_bits[i];
      inp[1]= y_bits[i];
      C.evaluate(inp, outp);
      for (unsigned int j= 0; j < z_bits[0].size(); j++)
        {
          if (outp[0][j] != z_bits[i][j])
            {
              cout << "Error 3 at " << i << " " << j << endl;
              abort();
            }
        }
    }
}

void test_add_p(const bigint &p, PRNG &G)
{
  // Find bit representation of p
  vector<int> p_bits(512);
  bigint_to_bits(p_bits, p);

  /* Now get test data */
  vector<bigint> x(numT), y(numT), z(numT);
  vector<vector<int>> x_bits(numT), y_bits(numT), z_bits(numT);
  for (unsigned int i= 0; i < numT; i++)
    {
      x[i]= randomBnd(G, p);
      y[i]= randomBnd(G, p);
      z[i]= (x[i] + y[i]) % p;
      x_bits[i].resize(512);
      y_bits[i].resize(512);
      z_bits[i].resize(512);
      bigint_to_bits(x_bits[i], x[i]);
      bigint_to_bits(y_bits[i], y[i]);
      bigint_to_bits(z_bits[i], z[i]);
    }

  /* Now load the 512 bit adder and test it is all OK */
  ifstream inpf;
  Circuit C512;
  inpf.open("Circuits/Bristol/ModAdd512.txt");
  inpf >> C512;
  inpf.close();

  vector<vector<int>> inp(3), outp(1);
  outp[0].resize(512);
  for (unsigned int i= 0; i < numT; i++)
    {
      inp[0]= x_bits[i];
      inp[1]= y_bits[i];
      inp[2]= p_bits;
      C512.evaluate(inp, outp);
      for (unsigned int j= 0; j < 512; j++)
        {
          if (outp[0][j] != z_bits[i][j])
            {
              cout << "Error 1 at " << i << " " << j << endl;
              abort();
            }
        }
    }
  cout << "Passed Test 1" << endl;

  /* Now do the specialization */
  vector<unsigned int> assign_in(512 * 3);
  unsigned int nBits= numBits(p);
  unsigned int nzeros= 512 - nBits;
  cout << "nBits= " << nBits << " nzeros=" << nzeros << endl;
  for (unsigned int i= 0; i < 512 * 2; i++)
    {
      assign_in[i]= 2;
    }
  for (unsigned int i= 0; i < nzeros; i++)
    {
      assign_in[nBits + i]= 0;
      assign_in[512 + nBits + i]= 0;
    }
  for (unsigned int i= 0; i < 512; i++)
    {
      assign_in[1024 + i]= p_bits[i];
    }

  SimplifyCircuit CC;
  CC.assign(C512);
  CC.Set_Inputs(assign_in);

  vector<unsigned int> rem(512);
  for (unsigned int i= 0; i < nBits; i++)
    {
      rem[i]= 0;
    }
  for (unsigned int i= 0; i < nzeros; i++)
    {
      rem[i + nBits]= 1;
    }
  CC.Remove_Outputs(rem);
  CC.Simplify();

  Circuit C128= CC.Get_Circuit();

  /* Now resize the test data */
  for (unsigned int i= 0; i < numT; i++)
    {
      x_bits[i].resize(nBits);
      y_bits[i].resize(nBits);
      z_bits[i].resize(nBits);
    }

  // Do Test
  do_test(x_bits, y_bits, z_bits, C128);
  cout << "Passed Test 2" << endl;
  // Printing C128 to do a quick look at it
  // cout << C128 << endl;

  // Now start testing the substitution stuff

  int numI= 0;
  bool flag= true;
  Circuit CSub;
  while (flag && numI < iter_modp_Search_SubCircuit)
    {
      CSub= CC.Get_Circuit();
      cout << numI << " : ";
      cout << "nwires : " << CSub.get_nWires() << "    ";
      cout << "ngates : " << CSub.get_nGates() << "    ";
      cout << "and gt : " << CSub.num_AND_gates() << endl;
      flag= CC.Search_SubCircuit();

      CSub= CC.Get_Circuit();
      do_test(x_bits, y_bits, z_bits, CSub);

      CC.Simplify();
      CSub= CC.Get_Circuit();
      do_test(x_bits, y_bits, z_bits, CSub);

      numI++;
    }
  // Print Circuit to have a look at it
  cout << CSub << endl;
  cout << "Passed Test 3" << endl;
}

/* Circuit is
 *   z=x+y mod p
 *   if (z>p/2)
 *      { long zz=p-z; 
 *        zz=-z;
 *      }
 *   else
 *      { long zz=z;   }
 *   Output zz;
 */
void test_LSSS_to_GC(const bigint &p, PRNG &G)
{
  // Find bit representation of p
  vector<int> p_bits(512);
  bigint_to_bits(p_bits, p);

  /* Now get test data */
  vector<bigint> x(numT), y(numT);
  vector<long> z(numT);
  vector<vector<int>> x_bits(numT), y_bits(numT), z_bits(numT);
  for (unsigned int i= 0; i < numT; i++)
    {
      z[i]= G.get_word();
      y[i]= randomBnd(G, p);
      x[i]= (z[i] - y[i]) % p;
      if (x[i] < 0)
        {
          x[i]+= p;
        }
      x_bits[i].resize(512);
      y_bits[i].resize(512);
      z_bits[i].resize(64);
      bigint_to_bits(x_bits[i], x[i]);
      bigint_to_bits(y_bits[i], y[i]);
      long_to_bits(z_bits[i], z[i]);
    }
  /* Now load the convert and test it is all OK */
  ifstream inpf;
  Circuit C512;
  inpf.open("Circuits/Bristol/LSSS_to_GC.txt");
  inpf >> C512;
  inpf.close();

  vector<vector<int>> inp(3), outp(1);
  outp[0].resize(512);
  for (unsigned int i= 0; i < numT; i++)
    {
      inp[0]= x_bits[i];
      inp[1]= y_bits[i];
      inp[2]= p_bits;
      C512.evaluate(inp, outp);
      for (unsigned int j= 0; j < 64; j++)
        {
          if (outp[0][j] != z_bits[i][j])
            {
              cout << "Error 1 at " << i << " " << j << endl;
              cout << "x = " << x[i] << endl;
              cout << "y = " << y[i] << endl;
              cout << "z = " << z[i] << endl;
              for (int j= 63; j >= 0; j--)
                {
                  cout << z_bits[i][j];
                }
              cout << endl;
              for (int j= 63; j >= 0; j--)
                {
                  cout << outp[0][j];
                }
              cout << endl;
              abort();
            }
        }
    }
  cout << "Passed Test 1" << endl;

  /* Now do the specialization */
  vector<unsigned int> assign_in(512 * 3);
  unsigned int nBits= numBits(p);
  unsigned int nzeros= 512 - nBits;
  cout << "nBits= " << nBits << " nzeros=" << nzeros << endl;
  for (unsigned int i= 0; i < 512 * 2; i++)
    {
      assign_in[i]= 2;
    }
  for (unsigned int i= 0; i < nzeros; i++)
    {
      assign_in[nBits + i]= 0;
      assign_in[512 + nBits + i]= 0;
    }
  for (unsigned int i= 0; i < 512; i++)
    {
      assign_in[1024 + i]= p_bits[i];
    }

  SimplifyCircuit CC;
  CC.assign(C512);
  CC.Set_Inputs(assign_in);
  CC.Simplify();

  Circuit C128= CC.Get_Circuit();
  /* Now resize the test data */
  for (unsigned int i= 0; i < numT; i++)
    {
      x_bits[i].resize(nBits);
      y_bits[i].resize(nBits);
    }

  // Do Test
  do_test(x_bits, y_bits, z_bits, C128);
  cout << "Passed Test 2" << endl;
  // Printing C128 to do a quick look at it
  // cout << C128 << endl;

  // Now start testing the substitution stuff

  int numI= 0;
  bool flag= true;
  Circuit CSub;
  while (flag && numI < iter_modp_Search_SubCircuit)
    {
      CSub= CC.Get_Circuit();
      cout << numI << " : ";
      cout << "nwires : " << CSub.get_nWires() << "    ";
      cout << "ngates : " << CSub.get_nGates() << "    ";
      cout << "and gt : " << CSub.num_AND_gates() << endl;
      flag= CC.Search_SubCircuit();

      CSub= CC.Get_Circuit();
      do_test(x_bits, y_bits, z_bits, CSub);

      CC.Simplify();
      CSub= CC.Get_Circuit();
      do_test(x_bits, y_bits, z_bits, CSub);

      numI++;
    }
  // Print Circuit to have a look at it
  cout << CSub << endl;
  cout << "Passed Test 3" << endl;
}

int main()
{
  PRNG G;
  G.ReSeed(0);

  /* Make one prime just less than 128 bits
   * and one a bit more
   */

  bigint p= (bigint(1) << 127) + (bigint(1) << 126);
  while (!probPrime(p))
    {
      p+= 1;
    }
  cout << "p = " << p << endl;

  cout << "Test adder (0=N, 1=Y) ? ";
  int aa= -1;
  while (aa != 0 && aa != 1)
    {
      cin >> aa;
    }

  if (aa == 1)
    {
      cout << "Testing adder..." << endl;
      test_add_p(p, G);
    }

  cout << "\n\n\nTesting LSSS_to_GC" << endl;
  test_LSSS_to_GC(p, G);

  p= bigint(1) << 128;
  while (!probPrime(p))
    {
      p+= 1;
    }
  cout << "\n\n\np = " << p << endl;

  if (aa == 1)
    {
      cout << "Testing adder..." << endl;
      test_add_p(p, G);
    }
  cout << "\n\n\nTesting LSSS_to_GC" << endl;
  test_LSSS_to_GC(p, G);
  return 0;
}
