/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "FHE/FFT.h"
#include "FHE/FHE_Keys.h"
#include "FHE/FHE_Params.h"
#include "FHE/Rq_Element.h"

/* Tests the FHE code */

extern bigint make_prime(int lg2, int N, const bigint &q= 0,
                         const bigint &x= 0);

void Test_FHE(const Ring &R, const FFT_Data &PD, const bigint &p0,
              const bigint &p1, int hwt)
{
  FHE_Params params;

  // Use small HwT as ring dimension is small in these test examples
  params.set(R, p0, p1, hwt, false);

  bigint pr= PD.get_prime();
  FHE_SK sk(params, pr);
  FHE_PK pk(params, pr);
  cout << "Plaintext = " << pr << endl;
  cout << "Root = ";
  PD.get_root(0).output(cout, PD.get_prD(), true);
  cout << endl;
  cout << "p0 = " << p0 << endl;
  cout << "p1 = " << p1 << endl;
  bool small= (pr < 1000);

  PRNG G;
  unsigned char seed[SEED_SIZE];
  memset(seed, 0, SEED_SIZE);
  G.SetSeedFromRandom(seed);

  KeyGen(pk, sk, G);
  Rq_Element secret= sk.s();

  if (small)
    {
      cout << "sk : ";
      sk.s().output(cout, true);
      cout << endl;
      cout << "pk : a = ";
      pk.a().output(cout, true);
      cout << endl;
      cout << "     b = ";
      pk.b().output(cout, true);
      cout << endl;
    }

  // Make a random message
  Plaintext mess(PD);
  mess.randomize(G);
  if (small)
    {
      cout << "Plaintext : " << mess << endl;
    }

  Random_Coins rc(params);
  rc.generate(G);
  if (small)
    {
      cout << "Random Coins : ";
      rc.output(cout, true);
      cout << endl;
    }

  Ciphertext c(params);
  pk.encrypt(c, mess, rc);
  if (small)
    {
      cout << "Ciphertext : ";
      c.output(cout, true);
      cout << endl;
    }

  Plaintext ans(PD);
  c.Scale(pr);
  sk.decrypt(ans, c);
  if (small)
    {
      cout << "Plaintext : " << ans << endl;
    }
  if (ans != mess)
    {
      cout << "Invalid decryption" << endl;
      abort();
    }

  cout << "\nTesting Arithmetic\n"
       << endl;
  Plaintext m1(PD), m2(PD), m3(PD), m4(PD);
  m1.randomize(G);
  m2.randomize(G);
  m3.randomize(G);
  if (small)
    {
      cout << "m1=" << m1 << endl;
      cout << "m2=" << m2 << endl;
      cout << "m3=" << m3 << endl;
    }

  // Testing to/from polys
  vector<bigint> te= m1.get_poly();
  m4.set_poly(te);
  if (m1 != m4)
    {
      cout << "Invalid conversion" << endl;
      abort();
    }

  cout << "pr= " << pr << endl;

  Ciphertext c1(params), c2(params), c3(params), c4(params);
  rc.generate(G);
  pk.encrypt(c1, m1, rc);
  if (small)
    {
      cout << "m1 encrypted with rc=\n";
      rc.output(cout, true);
      cout << endl;
      cout << "Giving ciphertext \n";
      c1.output(cout, true);
      cout << endl;
    }
  rc.generate(G);
  pk.encrypt(c2, m2, rc);
  if (small)
    {
      cout << "m2 encrypted with rc=\n";
      rc.output(cout, true);
      cout << endl;
      cout << "Giving ciphertext \n";
      c2.output(cout, true);
      cout << endl;
    }
  rc.generate(G);
  pk.encrypt(c3, m3, rc);
  if (small)
    {
      cout << "m3 encrypted with rc=\n";
      rc.output(cout, true);
      cout << endl;
      cout << "Giving ciphertext \n";
      c3.output(cout, true);
      cout << endl;
    }
  c4= c1;
  c4.Scale(pr);
  sk.decrypt(ans, c4);
  if (ans != m1)
    {
      cout << "Invalid decryption 1" << endl;
      abort();
    }
  c4= c2;
  c4.Scale(pr);
  sk.decrypt(ans, c4);
  if (ans != m2)
    {
      cout << "Invalid decryption 2" << endl;
      abort();
    }
  c4= c3;
  c4.Scale(pr);
  sk.decrypt(ans, c4);
  if (ans != m3)
    {
      cout << "Invalid decryption 3" << endl;
      abort();
    }

  cout << "m1+m2 testing" << endl;
  m1.from_poly();
  m2.from_poly();

  add(m4, m1, m2);
  add(c4, c1, c2);
  c4.Scale(pr);
  sk.decrypt(ans, c4);
  if (ans != m4)
    {
      cout << "Invalid decryption 4" << endl;
      abort();
    }

  cout << "m1*m2 testing" << endl;
  if (small)
    {
      cout << "m1=\n";
      m1.output(cout);
      cout << "m2=\n";
      m2.output(cout);
    }

  mul(m1, m1, m2);

  if (small)
    {
      cout << "m1*m2 = ";
      m1.output(cout);
      cout << endl;
    }
  mul(c1, c1, c2, pk);
  c4= c1;
  sk.decrypt(ans, c4);
  if (small)
    {
      cout << "ans = ";
      ans.output(cout);
    }
  if (ans != m1)
    {
      cout << "Invalid decryption 5" << endl;
      abort();
    }

  cout << "m1*m2+m3 testing " << endl;
  m1.to_poly();
  add(m1, m1, m3);

  c3.Scale(pr);
  add(c1, c1, c3);

  c1.Scale(pr);
  sk.decrypt(ans, c1);
  if (ans != m1)
    {
      cout << "Invalid decryption 6" << endl;
      abort();
    }
}

/* fast=true implies dont bother checking multiply as it takes ages */
void Test_Ring_Element(int num, const FFT_Data &FFTD, bool fast)
{
  cout << "\nIn Test_Ring_Element" << endl;
  int i;
  double start, stop;
  vector<Ring_Element> a(num, FFTD), b(num, FFTD), c0(num, FFTD), c1(num, FFTD),
      d0(num, FFTD), d1(num, FFTD), ao(num, FFTD), bo(num, FFTD);
  Ring_Element temp(FFTD);

  PRNG G;
  G.ReSeed(0);

  start= clock();
  for (i= 0; i < num; i++)
    {
      a[i].randomize(G);
      b[i].randomize(G);
      ao[i]= a[i];
      bo[i]= b[i];
    }
  stop= clock();
  cout << "Randomize \t: " << (stop - start) / (num * CLOCKS_PER_SEC)
       << " seconds" << endl;

  start= clock();
  for (i= 0; i < num; i++)
    {
      add(c0[i], a[i], b[i]);
    }
  stop= clock();
  cout << "Addition \t: " << (stop - start) / (num * CLOCKS_PER_SEC)
       << " seconds" << endl;

  if (!fast)
    {
      start= clock();
      for (i= 0; i < num; i++)
        {
          mul(d0[i], a[i], b[i]);
        }
      stop= clock();
      cout << "Multiply \t: " << (stop - start) / (num * CLOCKS_PER_SEC)
           << " seconds" << endl;
    }

  start= clock();
  for (i= 0; i < num; i++)
    {
      a[i].change_rep(evaluation);
      b[i].change_rep(evaluation);
    }
  stop= clock();
  cout << "Change Rep \t: " << (stop - start) / (num * CLOCKS_PER_SEC)
       << " seconds" << endl;

  start= clock();
  for (i= 0; i < num; i++)
    {
      add(c1[i], a[i], b[i]);
    }
  stop= clock();
  cout << "Addition \t: " << (stop - start) / (num * CLOCKS_PER_SEC)
       << " seconds" << endl;

  start= clock();
  for (i= 0; i < num; i++)
    {
      mul(d1[i], a[i], b[i]);
    }
  stop= clock();
  cout << "Multiply \t: " << (stop - start) / (num * CLOCKS_PER_SEC)
       << " seconds" << endl;

  cout << "Checking..." << endl;
  for (i= 0; i < num; i++)
    {
      temp= c1[i];
      c1[i].change_rep(polynomial);
      if (!c0[i].equals(c1[i]))
        {
          cout << "Error on addition entry " << i << endl;
          cout << "\t" << c0[i] << " " << c1[i] << endl;
          cout << "\t" << a[i] << " " << b[i] << endl;
          cout << "\t" << ao[i] << " " << bo[i] << endl;
          cout << "\t" << temp << endl;
        }
      if (!fast)
        {
          d1[i].change_rep(polynomial);
          if (!d0[i].equals(d1[i]))
            {
              cout << "Error on multiplication entry " << i << endl;
              cout << "\t" << d0[i] << " " << d1[i] << endl;
            }
        }
    }
  cout << "Checks done" << endl;
}

void Test_Rq_Element(int num, const vector<FFT_Data> &prd)
{
  cout << "\nIn Test_Rq_Element" << endl;
  int i;
  double start, stop;
  vector<Rq_Element> a(num, prd), b(num, prd), c(num, prd);

  PRNG G;
  G.ReSeed(0);

  start= clock();
  for (i= 0; i < num; i++)
    {
      a[i].randomize(G);
      b[i].randomize(G);
    }
  stop= clock();
  cout << "Randomize \t: " << (stop - start) / (num * CLOCKS_PER_SEC)
       << " seconds" << endl;

  start= clock();
  for (i= 0; i < num; i++)
    {
      add(c[i], a[i], b[i]);
    }
  stop= clock();
  cout << "Addition \t: " << (stop - start) / (num * CLOCKS_PER_SEC)
       << " seconds" << endl;

  for (i= 0; i < num; i++)
    {
      a[i].lower_level();
      b[i].lower_level();
    }
  cout << "Lowered Level and Timing..." << endl;

  start= clock();
  for (i= 0; i < num; i++)
    {
      add(c[i], a[i], b[i]);
    }
  stop= clock();
  cout << "Addition \t: " << (stop - start) / (num * CLOCKS_PER_SEC)
       << " seconds" << endl;

  start= clock();
  for (i= 0; i < num; i++)
    {
      mul(c[i], a[i], b[i]);
    }
  stop= clock();
  cout << "Multiply \t: " << (stop - start) / (num * CLOCKS_PER_SEC)
       << " seconds" << endl;

  cout << "Checking Stuff" << endl;
  vector<bigint> va, vb, vc;
  for (i= 0; i < num; i++)
    {
      a[i].randomize(G, 1);
      b[i].randomize(G, 1);
      va= a[i].to_vec_bigint();
      vb= b[i].to_vec_bigint();
      add(c[i], a[i], b[i]);
      vc= c[i].to_vec_bigint();
      bigint Q= a[i].get_modulus();
      for (unsigned int j= 0; j < vc.size(); j++)
        {
          bigint te= (va[j] + vb[j] - vc[j]) % Q;
          if (te != 0)
            {
              cout << va[j] << " + ";
              cout << vb[j] << " = ";
              cout << vc[j] << "   ";
              cout << "mod " << Q << endl;
              cout << a[i].level() << " " << b[i].level() << " " << c[i].level()
                   << endl;
              exit(1);
            }
        }
    }

  /* Do test of mult by a power of X 
   * Only run when degree is small
   */
  cout << "Testing mult by powers of X" << endl;
  if (prd[0].phi_m() < 1024)
    {
      Rq_Element xx(prd), yy(prd);
      xx.assign_one();
      cout << " xx = ";
      xx.output(cout, true);
      cout << endl;
      for (int i= 0; i < 2 * prd[0].phi_m(); i++)
        {
          mul_by_X_i(yy, xx, i);
          cout << i << " : ";
          yy.output(cout, true);
          cout << endl;
        }
    }
}

/*  Tests power of two FFT routines  */
void Test_2(const Ring &R, const Zp_Data &prD)
{

  PRNG G;
  G.ReSeed(0);

  FFT_Data FFTD(R, prD);

  cout << "Testing correctness of FFT mod x^n + 1\n";
  vector<modp> a(R.m()), b(R.phi_m());
  for (unsigned int i= 0; i < R.phi_m(); i++)
    {
      a[i].randomize(G, prD);
      b[i]= a[i];
    }

  a.resize(R.m());
  FFT_Iter(a, R.m(), FFTD.get_root(0), prD);
  vector<modp> temp(R.phi_m());
  for (unsigned int i= 0; i < R.phi_m(); i++)
    temp[i]= a[R.p(i)];
  a= temp;

  FFT_Iter2(b, R.phi_m(), FFTD.get_root(0), prD);

  /* Check results match */
  bigint ans;
  for (unsigned int j= 0; j < R.phi_m(); j++)
    {
      if (!areEqual(a[j], b[j], prD))
        {
          cout << "Mismatch at position " << j << ":\n";
          to_bigint(ans, a[j], prD);
          cout << "Standard FFT result:    " << ans << endl;
          to_bigint(ans, b[j], prD);
          cout << "FFT mod x^n + 1 result: " << ans << endl;
        }
    }
  cout << "Testing performance\n";

  int trials= 100;
  double time, rec_time= 0, iter_time= 0, mod_time= 0;
  vector<modp> test_data(R.m());

  vector<modp> m_data(R.m()), phim_data(R.phi_m());

  for (int i= 0; i < trials; i++)
    {
      for (unsigned int j= 0; j < R.m(); j++)
        {
          test_data[j].randomize(G, prD);
        }
      a= test_data;
      time= clock();
      FFT(a, R.m(), FFTD.get_root(0), prD);
      rec_time+= clock() - time;

      a= test_data;
      time= clock();
      FFT_Iter(a, R.m(), FFTD.get_root(0), prD);
      iter_time+= clock() - time;

      a= test_data;
      time= clock();
      FFT_Iter2(a, R.phi_m(), FFTD.get_root(0), prD);
      mod_time+= clock() - time;
    }
  cout << "Recursive FFT, size " << R.m() << " : " << rec_time / trials << endl;
  cout << "Iterative FFT, size " << R.m() << " : " << iter_time / trials
       << endl;
  cout << "FFT mod x^n+1, size " << R.phi_m() << " : " << mod_time / trials
       << endl;
}

int main()
{
  Ring Rg;

  int num= 500;
  int m= 1024;
  int size_q= 32;

  bigint q0= 1, q1;
  q0= (q0 << size_q) * m + 1;
  while (!probPrime(q0))
    {
      q0+= m;
    }
  cout << "q0 = " << q0 << "  :   " << numBits(q0) << endl;

  q1= q0 + m;
  while (!probPrime(q1))
    {
      q1+= m;
    }
  cout << "q1 = " << q1 << "  :   " << numBits(q1) << endl;

  Rg.initialize(m);
  Zp_Data prD0(q0);
  Zp_Data prD1(q1);

  // Test and time FFT
  Test_2(Rg, prD0);

  FFT_Data FFTD0(Rg, prD0);
  FFT_Data FFTD1(Rg, prD1);

  // Test and time Ring_Element
  Test_Ring_Element(num, FFTD0, false);

  vector<FFT_Data> FFTD(2);
  FFTD[0]= FFTD0;
  FFTD[1]= FFTD1;

  // Test and time Rq_Element
  Test_Rq_Element(num, FFTD);

  // Get some baby FHE parameters
  cout << "\n\nTesting Baby FHE parameters" << endl;
  unsigned int N= 4;
  bigint p= make_prime(10, N);
  bigint p0= make_prime(40, N);
  bigint p1= make_prime(45, N, p, p0);

  Rg.initialize(2 * N);

  gfp::init_field(p);
  FFT_Data PTD(Rg, gfp::get_ZpD());

  Test_FHE(Rg, PTD, p0, p1, 1);

  // Get some bigger parameters
  cout << "\n\nTesting Main FHE parameters" << endl;
  unsigned int hwt= 64;
  p= 0;
  Generate_Parameters(N, p0, p1, p, 128, 3, TopGear, hwt);

  Rg.initialize(2 * N);

  gfp::init_field(p);
  PTD.init(Rg, gfp::get_ZpD());

  Test_FHE(Rg, PTD, p0, p1, hwt);
  return 0;
}
