/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

/* This file tests the advanced FHE functionalties like
 * Distributed Decryption, ZKPoKs etc.
 *
 * As well as reading data from the files written to by
 * the Setup.x program
 */

#include <fstream>
#include <iostream>
#include <string>

#include "FHE/FHE_Keys.h"
#include "FHE/FHE_Params.h"
#include "FHE/ZKPoK.h"

using namespace std;

void Gen_FHE_Data(int &n, Ring &Rg, FFT_Data &PTD, FHE_Params &params,
                  FHE_PK *&pk, FHE_SK **&sk, FHE_SK *&Msk, PoKVersion version)
{
  n= 2;
  bigint p0, p1, pr= 0;
  unsigned int hwt= 64, N;
  Generate_Parameters(N, p0, p1, pr, 128, n, version, hwt);

  Rg.initialize(2 * N);
  gfp::init_field(pr);
  PTD.init(Rg, gfp::get_ZpD());
  params.set(Rg, p0, p1, hwt, n);

  sk= new FHE_SK *[n];
  pk= new FHE_PK(params, pr);
  Msk= new FHE_SK(params, pr);

  PRNG G;
  G.ReSeed(0);

  KeyGen(*pk, *Msk, G);

  // Now create the distributed secret key
  vector<Rq_Element> si(n, Rq_Element(params.FFTD(), evaluation, evaluation));
  si[0]= (*Msk).s();
  for (int i= 1; i < n; i++)
    {
      si[i].randomize(G, 0);
      sub(si[0], si[0], si[i]);
      sk[i]= new FHE_SK(params, pr);
      (*sk[i]).assign(si[i]);
    }
  sk[0]= new FHE_SK(params, pr);
  (*sk[0]).assign(si[0]);
}

void Read_FHE_Data(int &n, Ring &Rg, FFT_Data &PTD, FHE_Params &params,
                   FHE_PK *&pk, FHE_SK **&sk, FHE_SK *&Msk)
{
  ifstream input("Data/NetworkData.txt");
  string str;
  input >> str;
  input >> n;
  input.close();

  unsigned int N, hwt;
  bigint p0, p1, pr;

  sk= new FHE_SK *[n];

  for (int i= 0; i < n; i++)
    {
      stringstream ss;
      ss << "Data/FHE-Key-" << i << ".key";
      ifstream inp(ss.str().c_str());
      inp >> N >> p0 >> p1 >> pr >> hwt;
      char ch;
      do
        {
          inp.get(ch);
        }
      while (ch != ':');
      if (i == 0)
        {
          Rg.initialize(2 * N);
          gfp::init_field(pr);
          PTD.init(Rg, gfp::get_ZpD());
          params.set(Rg, p0, p1, hwt, n);
          pk= new FHE_PK(params, pr);
          inp >> *pk;
        }
      else
        {
          if (Rg.phi_m() != N || gfp::pr() != pr || params.p0() != p0 ||
              params.p1() != p1)
            {
              cout << "Something mismatched in files";
              abort();
            }
          FHE_PK pkt(params, pr);
          inp >> pkt;
          if (pkt.a() != pk->a() || pkt.b() != pk->b() || pkt.as() != pk->as() ||
              pkt.bs() != pk->bs())
            {
              cout << "Something mismatched in public keys";
              abort();
            }
        }
      sk[i]= new FHE_SK(params, pr);
      inp >> *(sk[i]);
      inp.close();
    }

  Msk= new FHE_SK(params, (*pk).p());
  Rq_Element te((*pk).get_params().FFTD());
  te.assign_zero();
  te.lower_level();
  for (int i= 0; i < n; i++)
    {
      add(te, te, sk[i]->s());
    }
  te.raise_level();
  (*Msk).assign(te);
}

void Test_All(int n, const Ring &Rg, const FFT_Data &PTD,
              const FHE_Params &params, FHE_PK *pk, FHE_SK **sk, FHE_SK *Msk,
              PoKVersion version)
{
  // Make an identifiable plaintext
  Plaintext mess(PTD);

  PRNG G;
  G.ReSeed(0);
  mess.randomize(G);

  Random_Coins rc(params);
  rc.generate(G);

  Ciphertext ctx(params);
  pk->encrypt(ctx, mess, rc);

  // Test distributed decryption
  vector<vector<bigint>> vv(n);
  for (int i= 0; i < n; i++)
    {
      sk[i]->dist_decrypt_1(vv[i], ctx, i);
    }

  vector<bigint> ans= vv[0];
  for (int i= 1; i < n; i++)
    {
      sk[i]->dist_decrypt_2(ans, vv[i]);
    }

  Plaintext output(PTD);
  output.set_poly_mod(ans, params.p0());

  for (unsigned int i= 0; i < Rg.phi_m(); i++)
    {
      if (mess.element(i) != output.element(i))
        {
          cout << "Something wrong in position - 1" << i << endl;
          abort();
        }
    }
  Ciphertext ctx2= ctx;
  ctx2.Scale((*pk).p());
  (*Msk).decrypt(output, ctx2);
  for (unsigned int i= 0; i < Rg.phi_m(); i++)
    {
      if (mess.element(i) != output.element(i))
        {
          cout << "Something wrong in position - 2 " << i << endl;
          abort();
        }
    }

  // Test squaring then distributed decryption
  mul(ctx2, ctx, ctx, *pk);

  for (int i= 0; i < n; i++)
    {
      sk[i]->dist_decrypt_1(vv[i], ctx2, i);
    }

  ans= vv[0];
  for (int i= 1; i < n; i++)
    {
      sk[i]->dist_decrypt_2(ans, vv[i]);
    }

  Plaintext mess2(PTD);
  mess2.set_poly_mod(ans, params.p0());

  (*Msk).decrypt(output, ctx2);
  for (unsigned int i= 0; i < Rg.phi_m(); i++)
    {
      gfp eq= mess.element(i);
      eq= eq * eq - output.element(i);
      if (!eq.is_zero())
        {
          cout << "Something wrong in position - 3 " << i << endl;
          abort();
        }
    }

  for (unsigned int i= 0; i < Rg.phi_m(); i++)
    {
      gfp eq= mess.element(i);
      eq= eq * eq - mess2.element(i);
      if (!eq.is_zero())
        {
          cout << "Something wrong in position - 4 " << i << endl;
          abort();
        }
    }

  cout << "\nTesting general ZKPoK routines" << endl;

  unsigned int max= 0, total= 0;
  // The ZKPoK stuff (first do non-Diagonal method)
  cout << "Doing Step 0/1 of ZK PoK" << endl;
  vector<ZKPoK> ZK(n);
  for (int i= 0; i < n; i++)
    {
      ZK[i].Step0(General, version, *pk, PTD, G);
      ZK[i].Step1(*pk, PTD, G);
    }

  // Emulate the transmission of data
  unsigned int len;
  cout << "Emulating transmission of data" << endl;
  {
    for (int i= 0; i < n; i++)
      {
        const uint8_t *buff= ZK[i].get_vE(len);
        if (len > max)
          {
            max= len;
          }
        if (i == 0)
          {
            total+= len;
          }
        for (int j= 0; j < n; j++)
          {
            if (i != j)
              {
                ZK[i].Step0_Step(buff, *pk);
              }
          }
      }
    for (int i= 0; i < n; i++)
      {
        const uint8_t *buff= ZK[i].get_vA(len);
        if (len > max)
          {
            max= len;
          }
        if (i == 0)
          {
            total+= len;
          }
        for (int j= 0; j < n; j++)
          {
            if (i != j)
              {
                ZK[i].Step1_Step(buff, *pk);
              }
          }
      }
  }

  // Generate challenge
  uint8_t seed[SEED_SIZE];
  for (unsigned int i= 0; i < SEED_SIZE; i++)
    {
      seed[i]= (uint8_t)(rand());
    }
  vector<int> e;

  cout << "Doing Step 2 of ZK PoK" << endl;
  for (int i= 0; i < n; i++)
    {
      ZK[i].Generate_e(e, seed);
      ZK[i].Step2(e, *pk);
    }

  // Emulate the transmission of data
  cout << "Emulating transmission of data" << endl;
  {
    for (int i= 0; i < n; i++)
      {
        const uint8_t *buff= ZK[i].get_vT_vZ(len);
        if (len > max)
          {
            max= len;
          }
        if (i == 0)
          {
            total+= len;
          }
        for (int j= 0; j < n; j++)
          {
            if (i != j)
              {
                ZK[i].Step2_Step(buff, *pk);
              }
          }
      }
  }

  cout << "Doing Step 3 of ZK PoK" << endl;
  for (int i= 0; i < n; i++)
    {
      if (!ZK[i].Step3(*pk, PTD, n))
        {
          cout << "Player " << i << " rejects the ZKPoK" << endl;
        }
      cout << "Soundness Parameter for Player " << i << " is " << ZK[i].get_sound_sec() << endl;
    }
  cout << "\nLargest message has size " << max << " bytes\n";
  cout << "\nTotal message size sent per player " << total << " bytes\n\n";

  cout << "\nTesting Diagonal ZKPoK routines" << endl;

  // The ZKPoK stuff doing Diagonal method
  cout << "Doing Step 1 of ZK PoK" << endl;
  vector<gfp> alpha(1);
  alpha[0].randomize(G);
  for (int i= 0; i < n; i++)
    {
      ZK[i].Step0(Diagonal, version, *pk, PTD, G, alpha);
      ZK[i].Step1(*pk, PTD, G);
    }

  // Emulate the transmission of data
  cout << "Emulating transmission of data" << endl;
  {
    for (int i= 0; i < n; i++)
      {
        const uint8_t *buff= ZK[i].get_vE(len);
        if (len > max)
          {
            max= len;
          }
        if (i == 0)
          {
            total+= len;
          }
        for (int j= 0; j < n; j++)
          {
            if (i != j)
              {
                ZK[i].Step0_Step(buff, *pk);
              }
          }
      }
    for (int i= 0; i < n; i++)
      {
        const uint8_t *buff= ZK[i].get_vA(len);
        if (len > max)
          {
            max= len;
          }
        if (i == 0)
          {
            total+= len;
          }
        for (int j= 0; j < n; j++)
          {
            if (i != j)
              {
                ZK[i].Step1_Step(buff, *pk);
              }
          }
      }
  }

  // Generate challenge
  for (unsigned int i= 0; i < SEED_SIZE; i++)
    {
      seed[i]= (uint8_t)(rand());
    }

  cout << "Doing Step 2 of ZK PoK" << endl;
  for (int i= 0; i < n; i++)
    {
      ZK[i].Generate_e(e, seed);
      ZK[i].Step2(e, *pk);
    }

  // Emulate the transmission of data
  {
    cout << "Emulating transmission of data" << endl;
    for (int i= 0; i < n; i++)
      {
        const uint8_t *buff= ZK[i].get_vT_vZ(len);
        if (len > max)
          {
            max= len;
          }
        if (i == 0)
          {
            total+= len;
          }
        for (int j= 0; j < n; j++)
          {
            if (i != j)
              {
                ZK[i].Step2_Step(buff, *pk);
              }
          }
      }
  }

  cout << "Doing Step 3 of ZK PoK" << endl;
  for (int i= 0; i < n; i++)
    {
      if (!ZK[i].Step3(*pk, PTD, n))
        {
          cout << "Player " << i << " rejects the ZKPoK" << endl;
        }
      cout << "Soundness Parameter for Player " << i << " is " << ZK[i].get_sound_sec() << endl;
    }
}

int main()
{
  int n;
  Ring Rg;
  FFT_Data PTD;
  FHE_Params params;

  FHE_PK *pk;
  FHE_SK **sk;
  FHE_SK *Msk;

  cout << "First testing TopGear" << endl;
  Gen_FHE_Data(n, Rg, PTD, params, pk, sk, Msk, TopGear);
  Test_All(n, Rg, PTD, params, pk, sk, Msk, TopGear);

  cout << "\n\n\nThen testing HighGear" << endl;
  Gen_FHE_Data(n, Rg, PTD, params, pk, sk, Msk, HighGear);
  Test_All(n, Rg, PTD, params, pk, sk, Msk, HighGear);

  delete pk;
  for (int i= 0; i < n; i++)
    {
      delete sk[i];
    }
  delete[] sk;
  return 0;
}
