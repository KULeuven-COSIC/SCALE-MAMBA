/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

#include "FHE/FFT.h"
#include "FHE/FFT_Data.h"
#include "FHE/FHE_Keys.h"
#include "FHE/FHE_Params.h"
#include "FHE/Ring.h"
#include "FHE/Ring_Element.h"
#include "FHE/Rq_Element.h"
#include "KeyGenM.h"
#include "LSSS/PRSS.h"
#include "Math/gfp.h"
#include "Tools/Crypto.h"
#include "config.h"

using namespace std::chrono;

KeyGenM::KeyGenM(Player &Pl) : P(Pl)
{
}

void KeyGenM::genKey(unsigned int N, bigint fsize1, bigint fsize2, bigint pMPC, unsigned int nb_threads_1, unsigned int nb_threads_2, SSL_CTX *ctx, const SystemData &SD, vector<vector<vector<int>>> &csockets, vector<gfp> &MacK)
{
  unsigned int whoami= P.whoami();
  unsigned int nbPlayers= P.nplayers();
  unsigned int nbThreads1= nb_threads_1;
  unsigned int nbThreads2= nb_threads_2;

  totalBitsGenerated= 0;

  gfp::init_field(fsize1);
  //Player Pmt1(whoami, SD, 0, ctx, csockets[1], MacK, 0);
  MASCOTTriples mt1(P, fsize1);
  mt1.Multiply();
  mt1.Combine();
  mt1.Authenticate();
  mt1.Sacrifice();

  //Init the factories
  gfp::init_field(fsize2);
  //Player Pmt2(whoami, SD, 0, ctx, csockets[nbThreads1 + 2], MacK, 0);
  MASCOTTriples mt2(P, fsize2);
  mt2.Multiply();
  mt2.Combine();
  mt2.Authenticate();
  mt2.Sacrifice();

  sec= 40;
  bit_batch_size= 51000;
  cout << "[KeyGenM - genKey] sec parameter for RandomBit set to " << sec << endl;
  cout << "[KeyGenM - genKey] batch size for RandomBit set to " << bit_batch_size << endl;

  //Init mutexes
  mtx1.resize(nbThreads1);
  mtx2.resize(nbThreads2);
  //Execute the offline phase of MASCOT in both worlds in threads
  gfp::init_field(fsize1);
  u_triples_w1.resize(nbThreads1);
  mac_u_triples_w1.resize(nbThreads1);
  vector<thread> TFactory1;
  unsigned long int totalProduced1= 0;
  bool keepGoing= true;
  for (unsigned int i= 0; i < nbThreads1; i++)
    {
      u_triples_w1[i].resize(3);
      mac_u_triples_w1[i].resize(3);
      Player *Ptmp1= new Player(whoami, SD, 0, ctx, csockets[i + 2], MacK, 0);
      MASCOTTriples *mt= new MASCOTTriples(*Ptmp1, fsize1, mt1.Delta, i + 2);
      thread th(&MASCOTTriples::execute, mt, ref(mtx1[i]), ref(u_triples_w1[i]), ref(mac_u_triples_w1[i]), ref(keepGoing), &totalProduced1);
      TFactory1.push_back(move(th));
    }

  gfp::init_field(fsize2);
  u_triples_w2.resize(nbThreads2);
  mac_u_triples_w2.resize(nbThreads2);
  vector<thread> TFactory2;
  unsigned long int totalProduced2= 0;
  for (unsigned int i= 0; i < nbThreads2; i++)
    {
      u_triples_w2[i].resize(3);
      mac_u_triples_w2[i].resize(3);
      Player *Ptmp2= new Player(whoami, SD, 0, ctx, csockets[i + 3 + nbThreads1], MacK, 0);
      MASCOTTriples *mt= new MASCOTTriples(*Ptmp2, fsize2, mt2.Delta, i + 3 + nbThreads1);
      thread th(&MASCOTTriples::execute, mt, ref(mtx2[i]), ref(u_triples_w2[i]), ref(mac_u_triples_w2[i]), ref(keepGoing), &totalProduced2);
      TFactory2.push_back(move(th));
    }

  //Compute a and a' in both worlds (as a big vector)
  Uniform(2 * N, mt1, mt2, fsize1, fsize2);
  //Compute enough randomBit to deal with Hamming and Gauss
  unsigned int h= HwtSK;
  if (h == 0)
    {
      //Compute e and e' in both worlds (as a big vector)
      Gauss(3 * N, mt1, mt2, fsize1, fsize2);
      gfp::init_field(fsize1);
      for (unsigned int i= 0; i < N; i++)
        {
          h_b_w1.push_back(g_w1.back());
          mac_h_b_w1.push_back(mac_g_w1.back());
          g_w1.pop_back();
          mac_g_w1.pop_back();
        }
      gfp::init_field(fsize2);
      for (unsigned int i= 0; i < N; i++)
        {
          h_b_w2.push_back(g_w2.back());
          mac_h_b_w2.push_back(mac_g_w2.back());
          g_w2.pop_back();
          mac_g_w2.pop_back();
        }
    }
  else
    {
      int nu= int(log(N) / log(2.0));
      int l= int(log(h) / log(2.0));

      //Compute s in both worlds
      Hamming(nu, l, mt1, mt2, fsize1, fsize2);

      //Compute e and e' in both worlds (as a big vector)
      Gauss(2 * N, mt1, mt2, fsize1, fsize2);
    }

  //Every parties input random vector (polynomial) in both worlds (bar P0)
  //First world 1
  gfp::init_field(fsize1);
  vector<vector<gfp>> sk_w1(nbPlayers, vector<gfp>(N));
  vector<vector<gfp>> mac_sk_w1(nbPlayers, vector<gfp>(N));
  vector<gfp> my_sk_w1(N);

  for (unsigned int i= 0; i < N; i++)
    {
      my_sk_w1[i].randomize(mt1.G);
    }

  for (unsigned int i= 1; i < whoami; i++)
    {
      Input_other(i, sk_w1[i], mac_sk_w1[i], fsize1, mt1);
    }

  if (whoami != 0)
    {
      Input_self(my_sk_w1, sk_w1[whoami], mac_sk_w1[whoami], fsize1, mt1);
    }

  for (unsigned int i= whoami + 1; i < nbPlayers; i++)
    {
      Input_other(i, sk_w1[i], mac_sk_w1[i], fsize1, mt1);
    }

  //Then world 2
  gfp::init_field(fsize2);
  vector<vector<gfp>> sk_w2(nbPlayers, vector<gfp>(N));
  vector<vector<gfp>> mac_sk_w2(nbPlayers, vector<gfp>(N));
  vector<gfp> my_sk_w2(N);

  for (unsigned int i= 0; i < N; i++)
    {
      my_sk_w2[i].randomize(mt2.G);
    }

  for (unsigned int i= 1; i < whoami; i++)
    {
      Input_other(i, sk_w2[i], mac_sk_w2[i], fsize2, mt2);
    }

  if (whoami != 0)
    {
      Input_self(my_sk_w2, sk_w2[whoami], mac_sk_w2[whoami], fsize2, mt2);
    }

  for (unsigned int i= whoami + 1; i < nbPlayers; i++)
    {
      Input_other(i, sk_w2[i], mac_sk_w2[i], fsize2, mt2);
    }

  //Compute the FFT on s
  gfp::init_field(fsize1);
  Ring Rg(2 * N);
  FFT_Data FFTD;
  FFTD.init(Rg, gfp::get_ZpD());

  Ring_Element Re(FFTD);
  Ring_Element Re_mac(FFTD);

  //Extract bingint vector from gfp for FFT format
  vector<bigint> h_b_w1_modp(N);
  vector<bigint> h_b_w1_modp_mac(N);
  for (unsigned int i= 0; i < N; i++)
    {
      to_bigint(h_b_w1_modp[i], h_b_w1[i]);
      to_bigint(h_b_w1_modp_mac[i], mac_h_b_w1[i]);
    }
  Re.from_vec(h_b_w1_modp);
  Re_mac.from_vec(h_b_w1_modp_mac);

  RepType rep_type= evaluation;

  //Go to evaluation representation
  Re.change_rep(rep_type);
  Re_mac.change_rep(rep_type);

  //Put it back into gfp
  Re.to_vec_bigint(h_b_w1_modp, false);
  Re_mac.to_vec_bigint(h_b_w1_modp_mac, false);
  vector<gfp> s_prime(N);
  vector<gfp> s_prime_mac(N);
  for (unsigned int i= 0; i < N; i++)
    {
      to_gfp(s_prime[i], h_b_w1_modp[i]);
      to_gfp(s_prime_mac[i], h_b_w1_modp_mac[i]);
    }

  //Square this (Requires triples)
  mt1.Multiplication(s_prime, s_prime_mac, s_prime, s_prime_mac, s_prime, s_prime_mac, u_triples_w1, mac_u_triples_w1, mtx1);

  //Stop triple production
  keepGoing= false;

  //Put it back to Ring_element
  for (unsigned int i= 0; i < N; i++)
    {
      to_bigint(h_b_w1_modp[i], s_prime[i]);
      to_bigint(h_b_w1_modp_mac[i], s_prime_mac[i]);

      //We dont want to use from_vec as it assumes we are in poly rep
      Re.set_element(i, h_b_w1_modp[i]);
      Re_mac.set_element(i, h_b_w1_modp_mac[i]);
    }

  //Go back to pol representation
  rep_type= polynomial;
  Re.change_rep(rep_type);
  Re_mac.change_rep(rep_type);

  //Put it back into gfp
  Re.to_vec_bigint(h_b_w1_modp);
  Re_mac.to_vec_bigint(h_b_w1_modp_mac);
  for (unsigned int i= 0; i < N; i++)
    {
      to_gfp(s_prime[i], h_b_w1_modp[i]);
      to_gfp(s_prime_mac[i], h_b_w1_modp_mac[i]);
    }

  //Open a and a' in both worlds
  //First in world 1
  vector<gfp> u_w1_opened(2 * N);
  for (unsigned int i= 0; i < 2 * N; i++)
    {
      mt1.Open(u_w1[i], mac_u_w1[i], u_w1_opened[i]);
    }

  //Then in world 2
  gfp::init_field(fsize2);
  vector<gfp> u_w2_opened(2 * N);
  for (unsigned int i= 0; i < 2 * N; i++)
    {
      mt2.Open(u_w2[i], mac_u_w2[i], u_w2_opened[i]);
    }

  //Compute b and b' in both worlds
  //First in world 1
  gfp::init_field(fsize1);
  Ring_Element Re_a0_w1(FFTD);
  Ring_Element Re_a0_prime_w1(FFTD);
  Ring_Element Re_s_w1(FFTD);
  Ring_Element Re_s_mac_w1(FFTD);
  Ring_Element Re_b_w1(FFTD);
  Ring_Element Re_b_mac_w1(FFTD);
  Ring_Element Re_b_prime_w1(FFTD);
  Ring_Element Re_b_prime_mac_w1(FFTD);
  //Extract bigint vectorf from gfp for Ring_Element format
  vector<bigint> tmp_bigint(N);
  for (unsigned int i= 0; i < N; i++)
    {
      to_bigint(tmp_bigint[i], u_w1_opened[i]);
    }
  Re_a0_w1.from_vec(tmp_bigint);

  for (unsigned int i= 0; i < N; i++)
    {
      to_bigint(tmp_bigint[i], u_w1_opened[N + i]);
    }
  Re_a0_prime_w1.from_vec(tmp_bigint);

  for (unsigned int i= 0; i < N; i++)
    {
      to_bigint(tmp_bigint[i], h_b_w1[i]);
    }
  Re_s_w1.from_vec(tmp_bigint);

  for (unsigned int i= 0; i < N; i++)
    {
      to_bigint(tmp_bigint[i], mac_h_b_w1[i]);
    }
  Re_s_mac_w1.from_vec(tmp_bigint);

  //Compute the polynomial products
  mul(Re_b_w1, Re_a0_w1, Re_s_w1);
  mul(Re_b_mac_w1, Re_a0_w1, Re_s_mac_w1);
  mul(Re_b_prime_w1, Re_a0_prime_w1, Re_s_w1);
  mul(Re_b_prime_mac_w1, Re_a0_prime_w1, Re_s_mac_w1);

  //Convert back to gfp and compute the remaing part
  vector<gfp> b_w1_local(N);
  vector<gfp> b_w1_local_mac(N);
  vector<gfp> b_w1_local_prime(N);
  vector<gfp> b_w1_local_prime_mac(N);

  Re_b_w1.to_vec_bigint(tmp_bigint);
  for (unsigned int i= 0; i < N; i++)
    {
      to_gfp(b_w1_local[i], tmp_bigint[i]);
    }

  Re_b_mac_w1.to_vec_bigint(tmp_bigint);
  for (unsigned int i= 0; i < N; i++)
    {
      to_gfp(b_w1_local_mac[i], tmp_bigint[i]);
    }

  Re_b_prime_w1.to_vec_bigint(tmp_bigint);
  for (unsigned int i= 0; i < N; i++)
    {
      to_gfp(b_w1_local_prime[i], tmp_bigint[i]);
    }

  Re_b_prime_mac_w1.to_vec_bigint(tmp_bigint);
  for (unsigned int i= 0; i < N; i++)
    {
      to_gfp(b_w1_local_prime_mac[i], tmp_bigint[i]);
    }

  gfp fsize2_gfp_w1;
  gfp pMPC_gfp_w1;

  fsize2_gfp_w1.assign(fsize2);
  pMPC_gfp_w1.assign(pMPC);
  for (unsigned int i= 0; i < N; i++)
    {
      b_w1_local[i]= b_w1_local[i] + (pMPC_gfp_w1 * g_w1[i]);
      b_w1_local_mac[i]= b_w1_local_mac[i] + (pMPC_gfp_w1 * mac_g_w1[i]);

      b_w1_local_prime[i]= b_w1_local_prime[i] + (pMPC_gfp_w1 * g_w1[N + i]) - (fsize2_gfp_w1 * s_prime[i]);
      b_w1_local_prime_mac[i]= b_w1_local_prime_mac[i] + (pMPC_gfp_w1 * mac_g_w1[N + i]) - (fsize2_gfp_w1 * s_prime_mac[i]);
    }

  //Then in world 2
  gfp::init_field(fsize2);
  FFT_Data FFTD_w2;
  FFTD_w2.init(Rg, gfp::get_ZpD());
  Ring_Element Re_a0_w2(FFTD_w2);
  Ring_Element Re_a0_prime_w2(FFTD_w2);
  Ring_Element Re_s_w2(FFTD_w2);
  Ring_Element Re_s_mac_w2(FFTD_w2);
  Ring_Element Re_b_w2(FFTD_w2);
  Ring_Element Re_b_mac_w2(FFTD_w2);
  Ring_Element Re_b_prime_w2(FFTD_w2);
  Ring_Element Re_b_prime_mac_w2(FFTD_w2);
  //Extract bigint vectorf from gfp for Ring_Element format
  for (unsigned int i= 0; i < N; i++)
    {
      to_bigint(tmp_bigint[i], u_w2_opened[i]);
    }
  Re_a0_w2.from_vec(tmp_bigint);

  for (unsigned int i= 0; i < N; i++)
    {
      to_bigint(tmp_bigint[i], u_w2_opened[N + i]);
    }
  Re_a0_prime_w2.from_vec(tmp_bigint);

  for (unsigned int i= 0; i < N; i++)
    {
      to_bigint(tmp_bigint[i], h_b_w2[i]);
    }
  Re_s_w2.from_vec(tmp_bigint);

  for (unsigned int i= 0; i < N; i++)
    {
      to_bigint(tmp_bigint[i], mac_h_b_w2[i]);
    }
  Re_s_mac_w2.from_vec(tmp_bigint);

  //Compute the polynomial products
  mul(Re_b_w2, Re_a0_w2, Re_s_w2);
  mul(Re_b_mac_w2, Re_a0_w2, Re_s_mac_w2);
  mul(Re_b_prime_w2, Re_a0_prime_w2, Re_s_w2);
  mul(Re_b_prime_mac_w2, Re_a0_prime_w2, Re_s_mac_w2);

  //Convert back to gfp and compute the remaing part
  vector<gfp> b_w2_local(N);
  vector<gfp> b_w2_local_mac(N);
  vector<gfp> b_w2_local_prime(N);
  vector<gfp> b_w2_local_prime_mac(N);

  Re_b_w2.to_vec_bigint(tmp_bigint);
  for (unsigned int i= 0; i < N; i++)
    {
      to_gfp(b_w2_local[i], tmp_bigint[i]);
    }

  Re_b_mac_w2.to_vec_bigint(tmp_bigint);
  for (unsigned int i= 0; i < N; i++)
    {
      to_gfp(b_w2_local_mac[i], tmp_bigint[i]);
    }

  Re_b_prime_w2.to_vec_bigint(tmp_bigint);
  for (unsigned int i= 0; i < N; i++)
    {
      to_gfp(b_w2_local_prime[i], tmp_bigint[i]);
    }

  Re_b_prime_mac_w2.to_vec_bigint(tmp_bigint);
  for (unsigned int i= 0; i < N; i++)
    {
      to_gfp(b_w2_local_prime_mac[i], tmp_bigint[i]);
    }
  gfp pMPC_gfp_w2;
  pMPC_gfp_w2.assign(pMPC);

  for (unsigned int i= 0; i < N; i++)
    {
      b_w2_local[i]= b_w2_local[i] + (pMPC_gfp_w2 * g_w2[i]);
      b_w2_local_mac[i]= b_w2_local_mac[i] + (pMPC_gfp_w2 * mac_g_w2[i]);

      b_w2_local_prime[i]= b_w2_local_prime[i] + (pMPC_gfp_w2 * g_w2[N + i]);
      b_w2_local_prime_mac[i]= b_w2_local_prime_mac[i] + (pMPC_gfp_w2 * mac_g_w2[N + i]);
    }

  //Compute secret key share for P0
  //First in world 1
  gfp::init_field(fsize1);

  for (unsigned int i= 0; i < N; i++)
    {
      sk_w1[0][i]= h_b_w1[i];
      mac_sk_w1[0][i]= mac_h_b_w1[i];
      for (unsigned int j= 1; j < nbPlayers; j++)
        {
          sk_w1[0][i]= sk_w1[0][i] - sk_w1[j][i];
          mac_sk_w1[0][i]= mac_sk_w1[0][i] - mac_sk_w1[j][i];
        }
    }

  //Then in world 2
  gfp::init_field(fsize2);

  for (unsigned int i= 0; i < N; i++)
    {
      sk_w2[0][i]= h_b_w2[i];
      mac_sk_w2[0][i]= mac_h_b_w2[i];
      for (unsigned int j= 1; j < nbPlayers; j++)
        {
          sk_w2[0][i]= sk_w2[0][i] - sk_w2[j][i];
          mac_sk_w2[0][i]= mac_sk_w2[0][i] - mac_sk_w2[j][i];
        }
    }

  //Open b and b' in both worlds
  //First in world 1
  gfp::init_field(fsize1);
  vector<gfp> b0_w1_opened(N);
  vector<gfp> b0_prime_w1_opened(N);

  for (unsigned int i= 0; i < N; i++)
    {
      mt1.Open(b_w1_local[i], b_w1_local_mac[i], b0_w1_opened[i]);
      mt1.Open(b_w1_local_prime[i], b_w1_local_prime_mac[i], b0_prime_w1_opened[i]);
    }

  //Then in world 2
  gfp::init_field(fsize2);
  vector<gfp> b0_w2_opened(N);
  vector<gfp> b0_prime_w2_opened(N);

  for (unsigned int i= 0; i < N; i++)
    {
      mt2.Open(b_w2_local[i], b_w2_local_mac[i], b0_w2_opened[i]);
      mt2.Open(b_w2_local_prime[i], b_w2_local_prime_mac[i], b0_prime_w2_opened[i]);
    }

  //Output secret key to P0
  //First in world 1
  gfp::init_field(fsize1);
  vector<gfp> epsilon_w1(N);
  vector<gfp> epsilon_mac_w1(N);
  vector<gfp> my_epsilon_w1(N);
  if (whoami == 0)
    {
      for (unsigned int i= 0; i < N; i++)
        {
          my_epsilon_w1[i].randomize(mt1.G);
        }
      Input_self(my_epsilon_w1, epsilon_w1, epsilon_mac_w1, fsize1, mt1);
    }
  else
    {
      Input_other(0, epsilon_w1, epsilon_mac_w1, fsize1, mt1);
    }

  vector<vector<gfp>> sk_w1_opened(nbPlayers, vector<gfp>(N));
  for (unsigned int i= 0; i < N; i++)
    {
      sk_w1[0][i]= sk_w1[0][i] - epsilon_w1[i];
      mac_sk_w1[0][i]= mac_sk_w1[0][i] - epsilon_mac_w1[i];
      mt1.Open(sk_w1[0][i], mac_sk_w1[0][i], sk_w1_opened[0][i]);
      if (whoami == 0)
        {
          sk_w1_opened[0][i]= sk_w1_opened[0][i] + my_epsilon_w1[i];
        }
    }

  //Wait for production threads to finish
  for (unsigned int i= 0; i < nbThreads1; i++)
    {
      TFactory1[i].join();
    }
  //Check that MPC went OK
  mt1.Check();

  //Then in world 2
  gfp::init_field(fsize2);
  vector<gfp> epsilon_w2(N);
  vector<gfp> epsilon_mac_w2(N);
  vector<gfp> my_epsilon_w2(N);
  if (whoami == 0)
    {
      for (unsigned int i= 0; i < N; i++)
        {
          my_epsilon_w2[i].randomize(mt2.G);
        }
      Input_self(my_epsilon_w2, epsilon_w2, epsilon_mac_w2, fsize2, mt2);
    }
  else
    {
      Input_other(0, epsilon_w2, epsilon_mac_w2, fsize2, mt2);
    }

  vector<vector<gfp>> sk_w2_opened(nbPlayers, vector<gfp>(N));
  for (unsigned int i= 0; i < N; i++)
    {
      sk_w2[0][i]= sk_w2[0][i] - epsilon_w2[i];
      mac_sk_w2[0][i]= mac_sk_w2[0][i] - epsilon_mac_w2[i];
      mt2.Open(sk_w2[0][i], mac_sk_w2[0][i], sk_w2_opened[0][i]);
      if (whoami == 0)
        {
          sk_w2_opened[0][i]= sk_w2_opened[0][i] + my_epsilon_w2[i];
        }
    }

  //Wait for factory threads
  for (unsigned int i= 0; i < nbThreads2; i++)
    {
      TFactory2[i].join();
    }
  //Check that MPC went OK
  mt2.Check();

  //CRT
  gfp::init_field(fsize1);
  Ring_Element Re_a0(FFTD);
  Ring_Element Re_b0(FFTD);
  Ring_Element Re_sk_w1(FFTD);
  Ring_Element Re_a0_prime(FFTD);
  Ring_Element Re_b0_prime(FFTD);

  for (unsigned int i= 0; i < N; i++)
    {
      to_bigint(tmp_bigint[i], u_w1_opened[i]);
    }
  Re_a0.from_vec(tmp_bigint);

  for (unsigned int i= 0; i < N; i++)
    {
      to_bigint(tmp_bigint[i], b0_w1_opened[i]);
    }
  Re_b0.from_vec(tmp_bigint);

  for (unsigned int i= 0; i < N; i++)
    {
      if (whoami == 0)
        {
          to_bigint(tmp_bigint[i], sk_w1_opened[0][i]);
        }
      else
        {
          to_bigint(tmp_bigint[i], my_sk_w1[i]);
        }
    }
  Re_sk_w1.from_vec(tmp_bigint);

  for (unsigned int i= 0; i < N; i++)
    {
      to_bigint(tmp_bigint[i], u_w1_opened[N + i]);
    }
  Re_a0_prime.from_vec(tmp_bigint);

  for (unsigned int i= 0; i < N; i++)
    {
      to_bigint(tmp_bigint[i], b0_prime_w1_opened[i]);
    }
  Re_b0_prime.from_vec(tmp_bigint);

  //World 2
  gfp::init_field(fsize2);
  Ring_Element Re_a1(FFTD_w2);
  Ring_Element Re_b1(FFTD_w2);
  Ring_Element Re_sk_w2(FFTD_w2);
  Ring_Element Re_a1_prime(FFTD_w2);
  Ring_Element Re_b1_prime(FFTD_w2);

  for (unsigned int i= 0; i < N; i++)
    {
      to_bigint(tmp_bigint[i], u_w2_opened[i]);
    }
  Re_a1.from_vec(tmp_bigint);

  for (unsigned int i= 0; i < N; i++)
    {
      to_bigint(tmp_bigint[i], b0_w2_opened[i]);
    }
  Re_b1.from_vec(tmp_bigint);

  for (unsigned int i= 0; i < N; i++)
    {
      if (whoami == 0)
        {
          to_bigint(tmp_bigint[i], sk_w2_opened[0][i]);
        }
      else
        {
          to_bigint(tmp_bigint[i], my_sk_w2[i]);
        }
    }
  Re_sk_w2.from_vec(tmp_bigint);

  for (unsigned int i= 0; i < N; i++)
    {
      to_bigint(tmp_bigint[i], u_w2_opened[N + i]);
    }
  Re_a1_prime.from_vec(tmp_bigint);

  for (unsigned int i= 0; i < N; i++)
    {
      to_bigint(tmp_bigint[i], b0_prime_w2_opened[i]);
    }
  Re_b1_prime.from_vec(tmp_bigint);

  Rq_Element Rq_a(Re_a0, Re_a1);
  Rq_Element Rq_b(Re_b0, Re_b1);
  Rq_Element Rq_sk(Re_sk_w1, Re_sk_w2);
  Rq_Element Rq_a_2(Re_a0_prime, Re_a1_prime);
  Rq_Element Rq_b_2(Re_b0_prime, Re_b1_prime);

  Rq_a.change_rep(evaluation, evaluation);
  Rq_b.change_rep(evaluation, evaluation);
  Rq_sk.change_rep(evaluation, evaluation);
  Rq_a_2.change_rep(evaluation, evaluation);
  Rq_b_2.change_rep(evaluation, evaluation);
  FHE_Params params;
  params.set(Rg, fsize1, fsize2, h, nbPlayers);
  FHE_PK pk(params, pMPC);
  pk.assign(Rq_a, Rq_b, Rq_a_2, Rq_b_2);
  FHE_SK sk(params, pMPC);
  sk.assign(Rq_sk);

  cout << "[KeyGenM - genKey] Total amount of bits consumed in w1 : " << totalBitsGenerated - b_w1.size() << endl;
  cout << "[KeyGenM - genKey] Total amount of bits consumed in w2 : " << totalBitsGenerated - b_w2.size() << endl;

  cout << "[KeyGenM - genKey] Writing to file" << endl;
  stringstream ss;
  ss << "Data/FHE-Key-" << whoami << ".key";
  ofstream outk(ss.str().c_str());
  outk << N << " " << fsize1 << " " << fsize2 << " " << pMPC << " " << (unsigned int) h << endl
       << ":";
  outk << pk;
  outk << sk << endl;
  outk.close();
}

void KeyGenM::RandomMaBit(int M, int sec, MASCOTTriples &mt1, MASCOTTriples &mt2, bigint fsize1, bigint fsize2)
{
  cout << "[KeyGenM - RandomMaBit] Starting bit generation" << endl;
  unsigned int nbPlayers= P.nplayers();
  unsigned int whoami= P.whoami();

  /*Step 1*/
  //Sample random bits

  PRNG G;
  G.ReSeed(0);
  //Input bits in world 1
  gfp::init_field(fsize2);
  vector<gfp> b_mine_w2;
  vector<vector<gfp>> b_shared_w2;
  vector<vector<gfp>> mac_b_shared_w2;

  b_mine_w2.resize(M + sec);
  b_shared_w2.resize(nbPlayers);
  mac_b_shared_w2.resize(nbPlayers);
  for (unsigned int i= 0; i < nbPlayers; i++)
    {
      b_shared_w2[i].resize(M + sec);
      mac_b_shared_w2[i].resize(M + sec);
    }

  //Sample uniform random from Fp
  for (int i= 0; i < M + sec; i++)
    {
      b_mine_w2[i].randomize(G);
    }

  for (unsigned int i= 0; i < whoami; i++)
    {
      Input_other(i, b_shared_w2[i], mac_b_shared_w2[i], fsize2, mt2);
    }

  Input_self(b_mine_w2, b_shared_w2[whoami], mac_b_shared_w2[whoami], fsize2, mt2);

  for (unsigned int i= whoami + 1; i < nbPlayers; i++)
    {
      Input_other(i, b_shared_w2[i], mac_b_shared_w2[i], fsize2, mt2);
    }

  cout << "[1] Input all to w2" << endl;
  //Sum the inputs from all parties
  vector<gfp> x_w2(M + sec);
  vector<gfp> mac_x_w2(M + sec);
  for (int i= 0; i < M + sec; i++)
    {
      x_w2[i]= b_shared_w2[0][i];
      mac_x_w2[i]= mac_b_shared_w2[0][i];
      for (unsigned int j= 1; j < nbPlayers; j++)
        {
          x_w2[i]+= b_shared_w2[j][i];
          mac_x_w2[i]+= mac_b_shared_w2[j][i];
        }
    }

  //Comput the square of all the inputs
  vector<gfp> y_w2(M + sec);
  vector<gfp> mac_y_w2(M + sec);
  mt2.Multiplication(x_w2, mac_x_w2, x_w2, mac_x_w2, y_w2, mac_y_w2, u_triples_w2, mac_u_triples_w2, mtx2);

  //Open the squares to everyone
  vector<gfp> open_y_w2(M + sec);
  for (int i= 0; i < M + sec; i++)
    {
      mt2.Open(y_w2[i], mac_y_w2[i], open_y_w2[i]);
    }
  //Check and keep in memory those that are not equal to zero
  vector<unsigned int> non_zero_index;
  for (int i= 0; i < M + sec; i++)
    {
      if (!open_y_w2[i].is_zero())
        {
          non_zero_index.push_back(i);
        }
    }

  //Compute the sqrt of the opened y and the final output
  vector<gfp> bits_w2(non_zero_index.size());
  vector<gfp> mac_bits_w2(non_zero_index.size());
  gfp const_2;
  gfp const_1;
  const_1.assign(1);
  const_2.assign(2);
  const_2.invert();
  unsigned int currIndex;
  for (unsigned int i= 0; i < non_zero_index.size(); i++)
    {
      currIndex= non_zero_index[i];
      open_y_w2[currIndex]= (open_y_w2[currIndex].sqrRoot());
      open_y_w2[currIndex].invert();

      bits_w2[currIndex]= x_w2[currIndex] * open_y_w2[currIndex];
      mac_bits_w2[currIndex]= mac_x_w2[currIndex] * open_y_w2[currIndex];

      if (P.whoami() == 0)
        {
          bits_w2[currIndex]+= const_1;
        }
      mac_bits_w2[currIndex]+= const_1 * mt2.Delta;

      bits_w2[currIndex]= bits_w2[currIndex] * (const_2);
      mac_bits_w2[currIndex]= mac_bits_w2[currIndex] * (const_2);
    }

  cout << "[2] Finished GenBit" << endl;

  //Compute the scaling factor (delta in the protocol)
  bigint scaling_factor;
  bigint bigint_np= nbPlayers;
  scaling_factor= div_c(fsize2, bigint_np);

  //Compute h_{i,j} = floor(b_{i,j}/delta)
  vector<bigint> local_h(non_zero_index.size());
  for (unsigned int i= 0; i < non_zero_index.size(); i++)
    {
      currIndex= non_zero_index[i];
      bigint tmp_conv;
      to_bigint(tmp_conv, bits_w2[currIndex], true);
      local_h[i]= div_f(tmp_conv, scaling_factor);
    }

  //Send local_h to P0
  /*convert bigint to gfp for easier IO functionalities*/
  unsigned int curr_size= non_zero_index.size();
  gfp::init_field(fsize1);
  vector<gfp> bits_w1(curr_size);
  vector<gfp> mac_bits_w1(curr_size);
  gfp::init_field(fsize2);
  if (P.whoami() != 0)
    {
      ostringstream os;
      for (unsigned int i= 0; i < curr_size; i++)
        {
          gfp tmp_gfp;
          to_gfp(tmp_gfp, local_h[i]);
          tmp_gfp.output(os, false);
        }
      P.send_to_player(0, os.str(), 1);

      //Pl l!=0 computes b mod fsize1 for later input
      vector<bigint> bits_w2_bigint_tmp(curr_size);
      for (unsigned int j= 0; j < curr_size; j++)
        {
          to_bigint(bits_w2_bigint_tmp[j], bits_w2[non_zero_index[j]]);
        }
      gfp::init_field(fsize1);
      for (unsigned int j= 0; j < curr_size; j++)
        {
          to_gfp(bits_w1[j], bits_w2_bigint_tmp[j]);
        }
      //Pl l!=0 computes b mod fsize2 for later input
      // (basically here reordering if some were 0)
      gfp::init_field(fsize2);
      for (unsigned int j= 0; j < curr_size; j++)
        {
          bits_w2[j]= bits_w2[non_zero_index[j]];
        }
      bits_w2.resize(curr_size);
    }
  else
    {
      for (unsigned int i= 1; i < nbPlayers; i++)
        {
          string ss;
          P.receive_from_player(i, ss, 1);
          istringstream is(ss);
          for (unsigned int j= 0; j < curr_size; j++)
            {
              gfp tmp_gfp;
              tmp_gfp.input(is, false);
              bigint tmp_bigint;
              to_bigint(tmp_bigint, tmp_gfp, true);
              local_h[j]= local_h[j] + tmp_bigint;
            }
        }

      //As P0, compute k in step (e)
      for (unsigned int j= 0; j < curr_size; j++)
        {
          local_h[j]= local_h[j] * scaling_factor;
          local_h[j]= div_c(local_h[j], fsize2);
        }

      //P0 computes b - k*fsize2 mod fsize1 for later input
      vector<bigint> bits_w2_bigint_tmp(curr_size);
      for (unsigned int j= 0; j < curr_size; j++)
        {
          to_bigint(bits_w2_bigint_tmp[j], bits_w2[non_zero_index[j]]);
        }

      gfp::init_field(fsize1);
      for (unsigned int j= 0; j < curr_size; j++)
        {
          gfp tmp_gfp;
          to_gfp(tmp_gfp, local_h[j] * fsize2);
          to_gfp(bits_w1[j], bits_w2_bigint_tmp[j]);
          bits_w1[j]= bits_w1[j] - tmp_gfp;
        }
      //P0 computes b - k*fsize2 mod fsize2 for later input
      gfp::init_field(fsize2);
      for (unsigned int j= 0; j < curr_size; j++)
        {
          gfp tmp_gfp;
          to_gfp(tmp_gfp, local_h[j] * fsize2);
          bits_w2[j]= bits_w2[non_zero_index[j]] - tmp_gfp;
        }
      bits_w2.resize(curr_size);
    }

  //Input in world 2
  for (unsigned int i= 0; i < nbPlayers; i++)
    {
      b_shared_w2[i].resize(curr_size);
      mac_b_shared_w2[i].resize(curr_size);
    }

  for (unsigned int i= 0; i < whoami; i++)
    {
      Input_other(i, b_shared_w2[i], mac_b_shared_w2[i], fsize2, mt2);
    }

  Input_self(bits_w2, b_shared_w2[whoami], mac_b_shared_w2[whoami], fsize2, mt2);

  for (unsigned int i= whoami + 1; i < nbPlayers; i++)
    {
      Input_other(i, b_shared_w2[i], mac_b_shared_w2[i], fsize2, mt2);
    }

  cout << "[3] Finished all the inputs in w2" << endl;
  //Sum up the inputs
  x_w2.resize(curr_size);
  mac_x_w2.resize(curr_size);
  for (unsigned int i= 0; i < curr_size; i++)
    {
      x_w2[i]= b_shared_w2[0][i];
      mac_x_w2[i]= mac_b_shared_w2[0][i];
      for (unsigned int j= 1; j < nbPlayers; j++)
        {
          x_w2[i]+= b_shared_w2[j][i];
          mac_x_w2[i]+= mac_b_shared_w2[j][i];
        }
    }

  //Input in world 1
  gfp::init_field(fsize1);
  vector<vector<gfp>> b_shared_w1;
  vector<vector<gfp>> mac_b_shared_w1;

  b_shared_w1.resize(nbPlayers);
  mac_b_shared_w1.resize(nbPlayers);
  for (unsigned int i= 0; i < nbPlayers; i++)
    {
      b_shared_w1[i].resize(curr_size);
      mac_b_shared_w1[i].resize(curr_size);
    }

  for (unsigned int i= 0; i < whoami; i++)
    {
      Input_other(i, b_shared_w1[i], mac_b_shared_w1[i], fsize1, mt1);
    }

  Input_self(bits_w1, b_shared_w1[whoami], mac_b_shared_w1[whoami], fsize1, mt1);

  for (unsigned int i= whoami + 1; i < nbPlayers; i++)
    {
      Input_other(i, b_shared_w1[i], mac_b_shared_w1[i], fsize1, mt1);
    }

  cout << "[4] Finished all the inputs in w1" << endl;
  //Sum up the inputs
  vector<gfp> x_w1(curr_size);
  vector<gfp> mac_x_w1(curr_size);
  for (unsigned int i= 0; i < curr_size; i++)
    {
      x_w1[i]= b_shared_w1[0][i];
      mac_x_w1[i]= mac_b_shared_w1[0][i];
      for (unsigned int j= 1; j < nbPlayers; j++)
        {
          x_w1[i]+= b_shared_w1[j][i];
          mac_x_w1[i]+= mac_b_shared_w1[j][i];
        }
    }

  //Initialize Frand
  uint8_t seed[SEED_SIZE];
  PRNG G2;
  AgreeRandom(P, seed, SEED_SIZE, 1);
  G2.SetSeedFromRandom(seed);

  //Generate curr_size (m + gamma ideally) random number in [0,...,2^sec]
  vector<BitVector> r;
  r.resize(curr_size);

  for (unsigned int i= 0; i < curr_size; i++)
    {
      r[i].resize(sec);
      r[i].randomize(G2);
    }

  vector<ostringstream> os;
  os.resize(curr_size);

  for (unsigned int i= 0; i < curr_size; i++)
    {
      for (unsigned int j= 0; j < r[0].size_bytes(); j++)
        {
          os[i] << int((r[i].get_ptr())[j]);
        }
      os[i] << endl;
    }

  //Translate r into w2
  gfp::init_field(fsize2);
  vector<gfp> r_w2(curr_size);

  for (unsigned int i= 0; i < curr_size; i++)
    {
      istringstream is(os[i].str());
      r_w2[i].input(is, true);
    }

  //Compute linear combination of bits in w1
  gfp S_w2;
  gfp mac_S_w2;

  S_w2.assign(0);
  mac_S_w2.assign(0);

  for (unsigned int i= 0; i < curr_size; i++)
    {
      S_w2+= r_w2[i] * x_w2[i];
      mac_S_w2+= r_w2[i] * mac_x_w2[i];
    }

  cout << "[5] Finished computing S in w2" << endl;
  //Translate r into w2
  gfp::init_field(fsize1);
  vector<gfp> r_w1(curr_size);

  for (unsigned int i= 0; i < curr_size; i++)
    {
      istringstream is(os[i].str());
      r_w1[i].input(is, true);
    }

  //Compute linear combination of bits in w2
  gfp S_w1;
  gfp mac_S_w1;

  S_w1.assign(0);
  mac_S_w1.assign(0);

  for (unsigned int i= 0; i < curr_size; i++)
    {
      S_w1+= r_w1[i] * x_w1[i];
      mac_S_w1+= r_w1[i] * mac_x_w1[i];
    }

  cout << "[6] Finished computing S in w1" << endl;
  //Open both
  gfp::init_field(fsize2);
  gfp open_S_w2;
  mt2.Open(S_w2, mac_S_w2, open_S_w2);

  //Convert S_w2 to bigint, to put it into w1
  bigint open_S_w2_bigint;
  to_bigint(open_S_w2_bigint, open_S_w2, true);

  gfp::init_field(fsize1);
  gfp open_S_w1;
  mt1.Open(S_w1, mac_S_w1, open_S_w1);

  gfp open_S_w2_in_w1;
  to_gfp(open_S_w2_in_w1, open_S_w2_bigint);

  if (open_S_w2_in_w1 != open_S_w1)
    {
      cout << "Abort bits are not equal in both worlds" << endl;
      throw bad_value();
    }

  cout << "[KeyGenM - RandomMaBit] End of bit generation " << endl;
  //Outputing M bits
  gfp::init_field(fsize1);
  for (int i= 0; i < M; i++)
    {
      b_w1.push_back(x_w1[i]);
      mac_b_w1.push_back(mac_x_w1[i]);
    }

  gfp::init_field(fsize2);
  for (int i= 0; i < M; i++)
    {
      b_w2.push_back(x_w2[i]);
      mac_b_w2.push_back(mac_x_w2[i]);
    }

  totalBitsGenerated+= M;
  cout << "[KeyGenM - RandomBit] Generated " << totalBitsGenerated << " in total" << endl;
}

void KeyGenM::RandomBit(int M, int sec, MASCOTTriples &mt1, MASCOTTriples &mt2, bigint fsize1, bigint fsize2)
{
  cout << "[KeyGenM - RandomBit] Starting bit generation" << endl;
  unsigned int nbPlayers= P.nplayers();
  unsigned int whoami= P.whoami();

  /*Step 1*/
  //Sample random bits

  vector<int> b_mine;
  PRNG G;

  G.ReSeed(0);
  b_mine.resize(M + sec);

  for (int i= 0; i < M + sec; i++)
    {
      b_mine[i]= (G.get_uchar() & 1);
    }

  //Input bits in world 1
  gfp::init_field(fsize1);
  vector<gfp> b_mine_w1;
  vector<vector<gfp>> b_shared_w1;
  vector<vector<gfp>> mac_b_shared_w1;

  b_mine_w1.resize(M + sec);
  b_shared_w1.resize(nbPlayers);
  mac_b_shared_w1.resize(nbPlayers);
  for (unsigned int i= 0; i < nbPlayers; i++)
    {
      b_shared_w1[i].resize(M + sec);
      mac_b_shared_w1[i].resize(M + sec);
    }

  for (int i= 0; i < M + sec; i++)
    {
      b_mine_w1[i].assign(b_mine[i]);
    }

  for (unsigned int i= 0; i < whoami; i++)
    {
      Input_other(i, b_shared_w1[i], mac_b_shared_w1[i], fsize1, mt1);
    }

  Input_self(b_mine_w1, b_shared_w1[whoami], mac_b_shared_w1[whoami], fsize1, mt1);

  for (unsigned int i= whoami + 1; i < nbPlayers; i++)
    {
      Input_other(i, b_shared_w1[i], mac_b_shared_w1[i], fsize1, mt1);
    }

  //Input bits in world 2
  gfp::init_field(fsize2);
  vector<gfp> b_mine_w2;
  vector<vector<gfp>> b_shared_w2;
  vector<vector<gfp>> mac_b_shared_w2;

  b_mine_w2.resize(M + sec);
  b_shared_w2.resize(nbPlayers);
  mac_b_shared_w2.resize(nbPlayers);
  for (unsigned int i= 0; i < nbPlayers; i++)
    {
      b_shared_w2[i].resize(M + sec);
      mac_b_shared_w2[i].resize(M + sec);
    }

  for (int i= 0; i < M + sec; i++)
    {
      b_mine_w2[i].assign(b_mine[i]);
    }

  for (unsigned int i= 0; i < whoami; i++)
    {
      Input_other(i, b_shared_w2[i], mac_b_shared_w2[i], fsize2, mt2);
    }

  Input_self(b_mine_w2, b_shared_w2[whoami], mac_b_shared_w2[whoami], fsize2, mt2);

  for (unsigned int i= whoami + 1; i < nbPlayers; i++)
    {
      Input_other(i, b_shared_w2[i], mac_b_shared_w2[i], fsize2, mt2);
    }

  /*Step 2*/
  // World 1 (2-a)
  gfp::init_field(fsize1);
  gfp gfp2_w1;
  vector<gfp> b_w1_local;
  vector<gfp> mac_b_w1_local;

  b_w1_local.resize(M + sec);
  mac_b_w1_local.resize(M + sec);
  gfp2_w1.assign(2);

  for (int i= 0; i < M + sec; i++)
    {
      b_w1_local[i]= b_shared_w1[0][i];
      mac_b_w1_local[i]= mac_b_shared_w1[0][i];
    }

  for (unsigned int i= 1; i < nbPlayers; i++)
    {
      //Prepare multiplication terms
      vector<gfp> mult_term1;
      vector<gfp> mac_mult_term1;
      vector<gfp> mult_term2;
      vector<gfp> mac_mult_term2;
      vector<gfp> res_mult;
      vector<gfp> mac_res_mult;

      mult_term1.resize(M + sec);
      mac_mult_term1.resize(M + sec);
      mult_term2.resize(M + sec);
      mac_mult_term2.resize(M + sec);
      res_mult.resize(M + sec);
      mac_res_mult.resize(M + sec);

      for (int j= 0; j < M + sec; j++)
        {
          mult_term1[j]= b_w1_local[j];
          mac_mult_term1[j]= mac_b_w1_local[j];

          mult_term2[j]= b_shared_w1[i][j];
          mac_mult_term2[j]= mac_b_shared_w1[i][j];
        }

      mt1.Multiplication(mult_term1, mac_mult_term1, mult_term2, mac_mult_term2, res_mult, mac_res_mult, u_triples_w1, mac_u_triples_w1, mtx1);
      //compute the new bi
      for (int j= 0; j < M + sec; j++)
        {
          b_w1_local[j]= b_w1_local[j] + b_shared_w1[i][j] - (gfp2_w1 * res_mult[j]);
          mac_b_w1_local[j]= mac_b_w1_local[j] + mac_b_shared_w1[i][j] - (gfp2_w1 * mac_res_mult[j]);
        }
    }

  // World 2 (2-b)
  gfp::init_field(fsize2);
  gfp gfp2_w2;
  vector<gfp> b_w2_local;
  vector<gfp> mac_b_w2_local;

  b_w2_local.resize(M + sec);
  mac_b_w2_local.resize(M + sec);
  gfp2_w2.assign(2);

  for (int i= 0; i < M + sec; i++)
    {
      b_w2_local[i]= b_shared_w2[0][i];
      mac_b_w2_local[i]= mac_b_shared_w2[0][i];
    }

  for (unsigned int i= 1; i < nbPlayers; i++)
    {
      //Prepare multiplication terms
      vector<gfp> mult_term1;
      vector<gfp> mac_mult_term1;
      vector<gfp> mult_term2;
      vector<gfp> mac_mult_term2;
      vector<gfp> res_mult;
      vector<gfp> mac_res_mult;

      mult_term1.resize(M + sec);
      mac_mult_term1.resize(M + sec);
      mult_term2.resize(M + sec);
      mac_mult_term2.resize(M + sec);
      res_mult.resize(M + sec);
      mac_res_mult.resize(M + sec);

      for (int j= 0; j < M + sec; j++)
        {
          mult_term1[j]= b_w2_local[j];
          mac_mult_term1[j]= mac_b_w2_local[j];

          mult_term2[j]= b_shared_w2[i][j];
          mac_mult_term2[j]= mac_b_shared_w2[i][j];
        }

      mt2.Multiplication(mult_term1, mac_mult_term1, mult_term2, mac_mult_term2, res_mult, mac_res_mult, u_triples_w2, mac_u_triples_w2, mtx2);
      //compute the new bi
      for (int j= 0; j < M + sec; j++)
        {
          b_w2_local[j]= b_w2_local[j] + b_shared_w2[i][j] - (gfp2_w2 * res_mult[j]);
          mac_b_w2_local[j]= mac_b_w2_local[j] + mac_b_shared_w2[i][j] - (gfp2_w2 * mac_res_mult[j]);
        }
    }

  /*Step 3*/
  //Get random elements in both worlds and compute t0 and t1
  uint8_t seed[SEED_SIZE];
  PRNG G2;
  AgreeRandom(P, seed, SEED_SIZE, 1);
  G2.SetSeedFromRandom(seed);

  //World 1
  //Sample r
  gfp::init_field(fsize1);
  vector<gfp> r_w1;

  r_w1.resize(M + sec);

  for (int i= 0; i < M + sec; i++)
    {
      r_w1[i].randomize(G2);
    }

  //Compute t0
  gfp t_w1;
  gfp mac_t_w1;
  gfp gfp1_w1;
  gfp1_w1.assign(1);

  vector<gfp> mult_term2_w1;
  vector<gfp> mac_mult_term2_w1;
  vector<gfp> res_mult_w1;
  vector<gfp> mac_res_mult_w1;

  mult_term2_w1.resize(M + sec);
  mac_mult_term2_w1.resize(M + sec);
  res_mult_w1.resize(M + sec);
  mac_res_mult_w1.resize(M + sec);

  for (int i= 0; i < M + sec; i++)
    {
      if (whoami == 0)
        {
          mult_term2_w1[i]= b_w1_local[i] - gfp1_w1;
        }
      else
        {
          mult_term2_w1[i]= b_w1_local[i];
        }
      mac_mult_term2_w1[i]= mac_b_w1_local[i] - gfp1_w1 * mt1.Delta;
    }

  mt1.Multiplication(b_w1_local, mac_b_w1_local, mult_term2_w1, mac_mult_term2_w1, res_mult_w1, mac_res_mult_w1, u_triples_w1, mac_u_triples_w1, mtx1);

  t_w1.assign(0);
  mac_t_w1.assign(0);
  for (int i= 0; i < M + sec; i++)
    {
      t_w1+= r_w1[i] * res_mult_w1[i];
      mac_t_w1+= r_w1[i] * mac_res_mult_w1[i];
    }

  //World 2
  //Sample r
  gfp::init_field(fsize2);
  vector<gfp> r_w2;

  r_w2.resize(M + sec);

  for (int i= 0; i < M + sec; i++)
    {
      r_w2[i].randomize(G2);
    }

  //Compute t0
  gfp t_w2;
  gfp mac_t_w2;
  gfp gfp1_w2;
  gfp1_w2.assign(1);

  vector<gfp> mult_term2_w2;
  vector<gfp> mac_mult_term2_w2;
  vector<gfp> res_mult_w2;
  vector<gfp> mac_res_mult_w2;

  mult_term2_w2.resize(M + sec);
  mac_mult_term2_w2.resize(M + sec);
  res_mult_w2.resize(M + sec);
  mac_res_mult_w2.resize(M + sec);

  for (int i= 0; i < M + sec; i++)
    {
      if (whoami == 0)
        {
          mult_term2_w2[i]= b_w2_local[i] - gfp1_w2;
        }
      else
        {
          mult_term2_w2[i]= b_w2_local[i];
        }
      mac_mult_term2_w2[i]= mac_b_w2_local[i] - gfp1_w2 * mt2.Delta;
    }

  mt2.Multiplication(b_w2_local, mac_b_w2_local, mult_term2_w2, mac_mult_term2_w2, res_mult_w2, mac_res_mult_w2, u_triples_w2, mac_u_triples_w2, mtx2);

  t_w2.assign(0);
  mac_t_w2.assign(0);
  for (int i= 0; i < M + sec; i++)
    {
      t_w2+= r_w2[i] * res_mult_w2[i];
      mac_t_w2+= r_w2[i] * mac_res_mult_w2[i];
    }

  /*Step 6, generate random s in [0,...,2^sec] */
  vector<BitVector> s;

  s.resize(M + sec);

  for (int i= 0; i < M + sec; i++)
    {
      s[i].resize(sec);
      s[i].randomize(G2);
    }

  vector<ostringstream> os;
  os.resize(M + sec);

  for (int i= 0; i < M + sec; i++)
    {
      for (unsigned int j= 0; j < s[0].size_bytes(); j++)
        {
          os[i] << int((s[i].get_ptr())[j]);
        }
      os[i] << endl;
    }

  //Translate s into w1
  gfp::init_field(fsize1);
  vector<gfp> s_w1;
  s_w1.resize(M + sec);

  for (int i= 0; i < M + sec; i++)
    {
      istringstream is(os[i].str());
      s_w1[i].input(is, true);
    }

  //Compute v0
  gfp v_w1;
  gfp mac_v_w1;

  v_w1.assign(0);
  mac_v_w1.assign(0);
  for (int i= 0; i < M + sec; i++)
    {
      v_w1+= s_w1[i] * b_w1_local[i];
      mac_v_w1+= s_w1[i] * mac_b_w1_local[i];
    }

  //Translate s into w2
  gfp::init_field(fsize2);
  vector<gfp> s_w2;
  s_w2.resize(M + sec);

  for (int i= 0; i < M + sec; i++)
    {
      istringstream is(os[i].str());
      s_w2[i].input(is, true);
    }

  //Compute v1
  gfp v_w2;
  gfp mac_v_w2;

  v_w2.assign(0);
  mac_v_w2.assign(0);
  for (int i= 0; i < M + sec; i++)
    {
      v_w2+= s_w2[i] * b_w2_local[i];
      mac_v_w2+= s_w2[i] * mac_b_w2_local[i];
    }

  //Open t0, t1, v0, v1
  gfp::init_field(fsize1);
  gfp open_t_w1;
  gfp open_v_w1;
  mt1.Open(t_w1, mac_t_w1, open_t_w1);
  mt1.Open(v_w1, mac_v_w1, open_v_w1);

  bigint open_v_w1_bigint;
  to_bigint(open_v_w1_bigint, open_v_w1, true);
  if (open_t_w1 != 0)
    {
      cout << "ABORT t_w1 neq 0" << endl;
      throw bad_value();
    }

  gfp::init_field(fsize2);
  gfp open_t_w2;
  gfp open_v_w2;
  mt2.Open(t_w2, mac_t_w2, open_t_w2);
  mt2.Open(v_w2, mac_v_w2, open_v_w2);

  bigint open_v_w2_bigint;
  to_bigint(open_v_w2_bigint, open_v_w2, true);

  if (open_t_w2 != 0)
    {
      cout << "ABORT t_w2 neq 0" << endl;
      throw bad_value();
    }

  if ((open_v_w1_bigint % fsize2) != (open_v_w2_bigint % fsize2))
    {
      cout << "ABORT v_w1 neq v_w2 " << endl;
      cout << "v_w1 = " << open_v_w1_bigint << endl;
      cout << "v_w2 = " << open_v_w2_bigint << endl;
      throw bad_value();
    }

  cout << "[KeyGenM - RandomBit] End of bit generation " << endl;

  //Outputing M bits
  gfp::init_field(fsize1);
  for (int i= 0; i < M; i++)
    {
      b_w1.push_back(b_w1_local[i]);
      mac_b_w1.push_back(mac_b_w1_local[i]);
    }

  gfp::init_field(fsize2);
  for (int i= 0; i < M; i++)
    {
      b_w2.push_back(b_w2_local[i]);
      mac_b_w2.push_back(mac_b_w2_local[i]);
    }

  totalBitsGenerated+= M;
  cout << "[KeyGenM - RandomBit] Generated " << totalBitsGenerated << " in total" << endl;
}

void KeyGenM::Hamming(int nu, int l, MASCOTTriples &mt1, MASCOTTriples &mt2, bigint fsize1, bigint fsize2)
{
  cout << "[KeyGenM - Hamming] Starting hamming" << endl;
  /*Step 1 set parameters*/
  unsigned int N= 1 << nu;

  //The following assumes that there are enough shared bits that have been processed
  /*Step 3 compute the product to get bi in both worlds*/
  //First world 1
  gfp::init_field(fsize1);
  vector<vector<gfp>> hamming_b_w1;
  vector<vector<gfp>> mac_hamming_b_w1;

  hamming_b_w1.resize(N);
  mac_hamming_b_w1.resize(N);

  for (unsigned int i= 0; i < N; i++)
    {
      hamming_b_w1[i].resize(nu - l);
      mac_hamming_b_w1[i].resize(nu - l);
      while (b_w1.empty() || b_w1.size() < (unsigned) nu - l)
        {
          RandomMaBit(bit_batch_size, sec, mt1, mt2, fsize1, fsize2);
          gfp::init_field(fsize1);
        }
      for (int j= 0; j < nu - l; j++)
        {
          hamming_b_w1[i][j]= b_w1.front();
          b_w1.pop_front();
          mac_hamming_b_w1[i][j]= mac_b_w1.front();
          mac_b_w1.pop_front();
        }
    }

  //Multiplication with minimized number of rounds
  while (hamming_b_w1[N - 1].size() != 1)
    {
      for (int i= 0; i < ceil(hamming_b_w1[N - 1].size() / 2.); i++)
        {
          if ((unsigned int) 2 * i + 1 < hamming_b_w1[N - 1].size())
            //if (2*i + 1 < b_w1[0].size())
            {
              vector<gfp> mult_term1(N);
              vector<gfp> mac_mult_term1(N);
              vector<gfp> mult_term2(N);
              vector<gfp> mac_mult_term2(N);
              vector<gfp> res(N);
              vector<gfp> mac_res(N);
              for (unsigned int j= 0; j < N; j++)
                {
                  mult_term1[j]= hamming_b_w1[j][2 * i];
                  mac_mult_term1[j]= mac_hamming_b_w1[j][2 * i];
                  mult_term2[j]= hamming_b_w1[j][2 * i + 1];
                  mac_mult_term2[j]= mac_hamming_b_w1[j][2 * i + 1];
                }

              mt1.Multiplication(mult_term1, mac_mult_term1, mult_term2, mac_mult_term2, res, mac_res, u_triples_w1, mac_u_triples_w1, mtx1);

              for (unsigned int j= 0; j < N; j++)
                {
                  hamming_b_w1[j][2 * i]= res[j];
                  mac_hamming_b_w1[j][2 * i]= mac_res[j];
                  //Removing 2*i + 1 element
                  hamming_b_w1[j].erase(hamming_b_w1[j].begin() + 2 * i + 1, hamming_b_w1[j].begin() + 2 * i + 2);
                  mac_hamming_b_w1[j].erase(mac_hamming_b_w1[j].begin() + 2 * i + 1, mac_hamming_b_w1[j].begin() + 2 * i + 2);
                }
            }
        }
    }

  //Do the same in world 2
  gfp::init_field(fsize2);
  vector<vector<gfp>> hamming_b_w2;
  vector<vector<gfp>> mac_hamming_b_w2;

  hamming_b_w2.resize(N);
  mac_hamming_b_w2.resize(N);

  for (unsigned int i= 0; i < N; i++)
    {
      hamming_b_w2[i].resize(nu - l);
      mac_hamming_b_w2[i].resize(nu - l);
      for (int j= 0; j < nu - l; j++)
        {
          hamming_b_w2[i][j]= b_w2.front();
          b_w2.pop_front();
          mac_hamming_b_w2[i][j]= mac_b_w2.front();
          mac_b_w2.pop_front();
        }
    }

  //Multiplication with minimized number of rounds
  while (hamming_b_w2[N - 1].size() != 1)
    {
      for (int i= 0; i < ceil(hamming_b_w2[N - 1].size() / 2.); i++)
        {
          if ((unsigned int) 2 * i + 1 < hamming_b_w2[N - 1].size())
            {
              vector<gfp> mult_term1(N);
              vector<gfp> mac_mult_term1(N);
              vector<gfp> mult_term2(N);
              vector<gfp> mac_mult_term2(N);
              vector<gfp> res(N);
              vector<gfp> mac_res(N);
              for (unsigned int j= 0; j < N; j++)
                {
                  mult_term1[j]= hamming_b_w2[j][2 * i];
                  mac_mult_term1[j]= mac_hamming_b_w2[j][2 * i];
                  mult_term2[j]= hamming_b_w2[j][2 * i + 1];
                  mac_mult_term2[j]= mac_hamming_b_w2[j][2 * i + 1];
                }

              mt2.Multiplication(mult_term1, mac_mult_term1, mult_term2, mac_mult_term2, res, mac_res, u_triples_w2, mac_u_triples_w2, mtx2);

              for (unsigned int j= 0; j < N; j++)
                {
                  hamming_b_w2[j][2 * i]= res[j];
                  mac_hamming_b_w2[j][2 * i]= mac_res[j];
                  //Removing 2*i + 1 element
                  hamming_b_w2[j].erase(hamming_b_w2[j].begin() + 2 * i + 1, hamming_b_w2[j].begin() + 2 * i + 2);
                  mac_hamming_b_w2[j].erase(mac_hamming_b_w2[j].begin() + 2 * i + 1, mac_hamming_b_w2[j].begin() + 2 * i + 2);
                }
            }
        }
    }

  /*Compute the final bi's*/
  //First in world 1
  gfp::init_field(fsize1);
  vector<gfp> s_w1;
  vector<gfp> mac_s_w1;
  vector<gfp> flat_hamming_w1;
  vector<gfp> mac_flat_hamming_w1;
  gfp gfp2_w1;
  gfp gfp1_w1;

  s_w1.resize(N);
  mac_s_w1.resize(N);
  flat_hamming_w1.resize(N);
  mac_flat_hamming_w1.resize(N);

  h_b_w1.resize(N);
  mac_h_b_w1.resize(N);
  while (b_w1.empty() || b_w1.size() < N)
    {
      RandomMaBit(bit_batch_size, sec, mt1, mt2, fsize1, fsize2);
      gfp::init_field(fsize1);
    }

  for (unsigned int i= 0; i < N; i++)
    {
      s_w1[i]= b_w1.front();
      b_w1.pop_front();
      mac_s_w1[i]= mac_b_w1.front();
      mac_b_w1.pop_front();
      flat_hamming_w1[i]= hamming_b_w1[i][0];
      mac_flat_hamming_w1[i]= mac_hamming_b_w1[i][0];
    }

  gfp2_w1.assign(2);
  gfp1_w1.assign(1);

  //Compute 2*si - 1
  for (unsigned int i= 0; i < N; i++)
    {
      s_w1[i]= gfp2_w1 * s_w1[i];
      if (P.whoami() == 0)
        {
          s_w1[i]= s_w1[i] - gfp1_w1;
        }

      mac_s_w1[i]= gfp2_w1 * mac_s_w1[i] - gfp1_w1 * mt1.Delta;
    }

  mt1.Multiplication(flat_hamming_w1, mac_flat_hamming_w1, s_w1, mac_s_w1, h_b_w1, mac_h_b_w1, u_triples_w1, mac_u_triples_w1, mtx1);

  //Then in world 2
  gfp::init_field(fsize2);
  vector<gfp> s_w2;
  vector<gfp> mac_s_w2;
  vector<gfp> flat_hamming_w2;
  vector<gfp> mac_flat_hamming_w2;
  gfp gfp2_w2;
  gfp gfp1_w2;

  s_w2.resize(N);
  mac_s_w2.resize(N);
  flat_hamming_w2.resize(N);
  mac_flat_hamming_w2.resize(N);

  h_b_w2.resize(N);
  mac_h_b_w2.resize(N);

  for (unsigned int i= 0; i < N; i++)
    {
      s_w2[i]= b_w2.front();
      b_w2.pop_front();
      mac_s_w2[i]= mac_b_w2.front();
      mac_b_w2.pop_front();
      flat_hamming_w2[i]= hamming_b_w2[i][0];
      mac_flat_hamming_w2[i]= mac_hamming_b_w2[i][0];
    }

  gfp2_w2.assign(2);
  gfp1_w2.assign(1);

  //Compute 2*si - 1
  for (unsigned int i= 0; i < N; i++)
    {
      s_w2[i]= gfp2_w2 * s_w2[i];
      if (P.whoami() == 0)
        {
          s_w2[i]= s_w2[i] - gfp1_w2;
        }

      mac_s_w2[i]= gfp2_w2 * mac_s_w2[i] - gfp1_w2 * mt2.Delta;
    }

  mt2.Multiplication(flat_hamming_w2, mac_flat_hamming_w2, s_w2, mac_s_w2, h_b_w2, mac_h_b_w2, u_triples_w2, mac_u_triples_w2, mtx2);

  cout << "[KeyGenM - Hamming] Finished Hamming" << endl;
}

void KeyGenM::Gauss(unsigned int N, MASCOTTriples &mt1, MASCOTTriples &mt2, bigint fsize1, bigint fsize2)
{
  cout << "[KeyGenM - Gauss] Starting Gauss" << endl;

  unsigned int newHopeB= NewHopeB;
  //First in world 1
  gfp::init_field(fsize1);
  g_w1.resize(N);
  mac_g_w1.resize(N);

  for (unsigned int i= 0; i < N; i++)
    {
      g_w1[i].assign(0);
      mac_g_w1[i].assign(0);
      while (b_w1.empty() || b_w1.size() < 2 * newHopeB)
        {
          RandomMaBit(bit_batch_size, sec, mt1, mt2, fsize1, fsize2);
          gfp::init_field(fsize1);
        }

      for (unsigned int j= 0; j < newHopeB; j++)
        {
          g_w1[i]= g_w1[i] + b_w1.front();
          mac_g_w1[i]= mac_g_w1[i] + mac_b_w1.front();
          b_w1.pop_front();
          mac_b_w1.pop_front();
          g_w1[i]= g_w1[i] - b_w1.front();
          mac_g_w1[i]= mac_g_w1[i] - mac_b_w1.front();
          b_w1.pop_front();
          mac_b_w1.pop_front();
        }
    }
  //Then in world 2
  gfp::init_field(fsize2);
  g_w2.resize(N);
  mac_g_w2.resize(N);

  for (unsigned int i= 0; i < N; i++)
    {
      g_w2[i].assign(0);
      mac_g_w2[i].assign(0);

      for (unsigned int j= 0; j < newHopeB; j++)
        {
          g_w2[i]= g_w2[i] + b_w2.front();
          mac_g_w2[i]= mac_g_w2[i] + mac_b_w2.front();
          b_w2.pop_front();
          mac_b_w2.pop_front();
          g_w2[i]= g_w2[i] - b_w2.front();
          mac_g_w2[i]= mac_g_w2[i] - mac_b_w2.front();
          b_w2.pop_front();
          mac_b_w2.pop_front();
        }
    }

  cout << "[KeyGenM - Gauss] Finished Gauss" << endl;
}

void KeyGenM::Uniform(unsigned int N, MASCOTTriples &mt1, MASCOTTriples &mt2, bigint fsize1, bigint fsize2)
{
  cout << "[KeyGenM - Uniform] Starting Uniform" << endl;
  unsigned int nbPlayers= P.nplayers();
  unsigned int whoami= P.whoami();
  //First in world 1
  gfp::init_field(fsize1);
  vector<gfp> v_mine_w1;
  vector<vector<gfp>> v_shared_w1;
  vector<vector<gfp>> mac_v_shared_w1;

  v_shared_w1.resize(nbPlayers);
  mac_v_shared_w1.resize(nbPlayers);
  v_mine_w1.resize(N);
  for (unsigned int i= 0; i < nbPlayers; i++)
    {
      v_shared_w1[i].resize(N);
      mac_v_shared_w1[i].resize(N);
      if (i == whoami)
        {
          for (unsigned int j= 0; j < N; j++)
            {
              v_mine_w1[j].randomize(mt1.G);
            }
        }
    }

  //Input from all parties
  for (unsigned int i= 0; i < whoami; i++)
    {
      Input_other(i, v_shared_w1[i], mac_v_shared_w1[i], fsize1, mt1);
    }

  Input_self(v_mine_w1, v_shared_w1[whoami], mac_v_shared_w1[whoami], fsize1, mt1);

  for (unsigned int i= whoami + 1; i < nbPlayers; i++)
    {
      Input_other(i, v_shared_w1[i], mac_v_shared_w1[i], fsize1, mt1);
    }

  //Sum up everything
  u_w1.resize(N);
  mac_u_w1.resize(N);
  for (unsigned int i= 0; i < N; i++)
    {
      u_w1[i].assign(0);
      mac_u_w1[i].assign(0);
      for (unsigned int j= 0; j < nbPlayers; j++)
        {
          u_w1[i]= u_w1[i] + v_shared_w1[j][i];
          mac_u_w1[i]= mac_u_w1[i] + mac_v_shared_w1[j][i];
        }
    }

  //Then in world 2
  gfp::init_field(fsize2);
  vector<gfp> v_mine_w2;
  vector<vector<gfp>> v_shared_w2;
  vector<vector<gfp>> mac_v_shared_w2;

  v_shared_w2.resize(nbPlayers);
  mac_v_shared_w2.resize(nbPlayers);
  v_mine_w2.resize(N);
  for (unsigned int i= 0; i < nbPlayers; i++)
    {
      v_shared_w2[i].resize(N);
      mac_v_shared_w2[i].resize(N);
      if (i == whoami)
        {
          for (unsigned int j= 0; j < N; j++)
            {
              v_mine_w2[j].randomize(mt2.G);
            }
        }
    }

  //Input from all parties
  for (unsigned int i= 0; i < whoami; i++)
    {
      Input_other(i, v_shared_w2[i], mac_v_shared_w2[i], fsize2, mt2);
    }

  Input_self(v_mine_w2, v_shared_w2[whoami], mac_v_shared_w2[whoami], fsize2, mt2);

  for (unsigned int i= whoami + 1; i < nbPlayers; i++)
    {
      Input_other(i, v_shared_w2[i], mac_v_shared_w2[i], fsize2, mt2);
    }

  //Sum up everything
  u_w2.resize(N);
  mac_u_w2.resize(N);
  for (unsigned int i= 0; i < N; i++)
    {
      u_w2[i].assign(0);
      mac_u_w2[i].assign(0);
      for (unsigned int j= 0; j < nbPlayers; j++)
        {
          u_w2[i]= u_w2[i] + v_shared_w2[j][i];
          mac_u_w2[i]= mac_u_w2[i] + mac_v_shared_w2[j][i];
        }
    }

  cout << "[KeyGenM - Uniform] Finished Uniform" << endl;
}

void KeyGenM::Input_self(vector<gfp> &in, vector<gfp> &out, vector<gfp> &mac_out, bigint p, MASCOTTriples &mt)
{
  unsigned int nbElements= in.size();
  unsigned int nbPlayers= P.nplayers();
  unsigned int whoami= P.whoami();
  //Init gfp in the correct world
  gfp::init_field(p);

  //1 - Sample x0
  PRNG G;
  G.ReSeed(0);
  gfp x0;

  x0.randomize(G);

  //2 - Create random additive sharing for x0 and in
  vector<vector<gfp>> additive_sharing;
  additive_sharing.resize(nbElements + 1);

  for (unsigned int i= 0; i < nbElements + 1; i++)
    {
      additive_sharing[i].resize(nbPlayers);

      if (i == 0)
        {
          additive_sharing[i][nbPlayers - 1]= x0;
        }
      else
        {
          additive_sharing[i][nbPlayers - 1]= in[(i - 1)];
        }

      for (unsigned int j= 0; j < nbPlayers - 1; j++)
        {
          additive_sharing[i][j].randomize(G);
          additive_sharing[i][nbPlayers - 1]-= additive_sharing[i][j];
        }
    }

  for (unsigned int i= 0; i < nbElements; i++)
    {
      out[i]= additive_sharing[i + 1][whoami];
    }

  //Send the additive sharings
  for (unsigned int i= 0; i < nbPlayers; i++)
    {
      if (i != whoami)
        {
          ostringstream os;
          for (unsigned int j= 0; j < nbElements + 1; j++)
            {
              additive_sharing[j][i].output(os, false);
            }
          P.send_to_player(i, os.str(), 1);
        }
    }

  //3 - Call to COPE.extend as sender
  vector<vector<gfp>> t;
  vector<gfp> x_for_COPE(nbElements + 1);
  t.resize(nbPlayers);

  x_for_COPE[0]= x0;
  for (unsigned int i= 0; i < nbElements; i++)
    {
      x_for_COPE[i + 1]= in[i];
    }

  for (unsigned int i= 0; i < nbPlayers; i++)
    {
      t[i].resize(nbElements + 1);
      if (i != whoami)
        {
          mt.COPE_S[i].extend_vec(P, x_for_COPE, t[i], true);
        }
    }

  //5 - Compute the MAC shares
  gfp mac_x0;
  for (unsigned int i= 0; i < nbElements + 1; i++)
    {
      if (i == 0)
        {
          mac_x0= x0 * mt.Delta;
          for (unsigned int j= 0; j < nbPlayers; j++)
            {
              if (j != whoami)
                {
                  mac_x0+= t[j][i];
                }
            }
        }
      else
        {
          mac_out[i - 1]= in[(i - 1)] * mt.Delta;
          for (unsigned int j= 0; j < nbPlayers; j++)
            {
              if (j != whoami)
                {
                  mac_out[i - 1]+= t[j][i];
                }
            }
        }
    }

  //6 - Sample r for linear combination
  uint8_t seed[SEED_SIZE];
  PRNG G2;
  vector<gfp> r;

  AgreeRandom(P, seed, SEED_SIZE, 1);
  G2.SetSeedFromRandom(seed);

  r.resize(nbElements + 1);

  for (unsigned int i= 0; i < nbElements + 1; i++)
    {
      r[i].randomize(G2);
    }

  //7 - compute and broadcast y
  gfp y;
  y= x0 * r[0];
  for (unsigned int i= 0; i < nbElements; i++)
    {
      y+= r[i + 1] * in[i];
    }

  ostringstream os;
  y.output(os, false);
  P.send_all(os.str(), 1);

  //8 - compute m
  gfp m;
  m= r[0] * mac_x0;

  for (unsigned int i= 0; i < nbElements; i++)
    {
      m+= r[i + 1] * mac_out[i];
    }

  //9 - MacCheck
  MacCheck(y, m, p, mt);
}

void KeyGenM::Input_other(int nbP_Input, vector<gfp> &out, vector<gfp> &mac_out, bigint p, MASCOTTriples &mt)
{
  unsigned int nbElements= out.size();
  //Init gfp in the correct world
  gfp::init_field(p);

  //2 - Receive additive sharings
  gfp share_x0;
  string ss;
  P.receive_from_player(nbP_Input, ss, 1);
  istringstream is(ss);

  for (unsigned int i= 0; i < nbElements + 1; i++)
    {
      if (i == 0)
        {
          share_x0.input(is, false);
        }
      else
        {
          out[i - 1].input(is, false);
        }
    }

  //3 - Call to COPE.extend as receiver
  vector<gfp> receive_COPE(nbElements + 1);
  gfp mac_x0;

  mt.COPE_R[nbP_Input].extend_vec(P, receive_COPE, true);

  for (unsigned int i= 0; i < nbElements + 1; i++)
    {
      if (i == 0)
        {
          mac_x0= receive_COPE[i];
        }
      else
        {
          mac_out[i - 1]= receive_COPE[i];
        }
    }

  //6 - Sample r for linear comb
  uint8_t seed[SEED_SIZE];
  PRNG G2;
  vector<gfp> r;

  AgreeRandom(P, seed, SEED_SIZE, 1);
  G2.SetSeedFromRandom(seed);

  r.resize(nbElements + 1);

  for (unsigned int i= 0; i < nbElements + 1; i++)
    {
      r[i].randomize(G2);
    }

  //7 - receive y
  gfp y;
  string ss2;

  P.receive_from_player(nbP_Input, ss2, 1);
  istringstream is2(ss2);
  y.input(is2, false);

  //8 - compute m
  gfp m;
  m= r[0] * mac_x0;

  for (unsigned int i= 0; i < nbElements; i++)
    {
      m+= r[i + 1] * mac_out[i];
    }

  //MACCheck
  MacCheck(y, m, p, mt);
}

void KeyGenM::MacCheck(gfp y, gfp m, bigint p, MASCOTTriples &mt)
{
  gfp::init_field(p);
  vector<gfp> sigma;
  gfp check;

  sigma.resize(P.nplayers());

  sigma[P.whoami()]= m - (y * mt.Delta);
  Commit_And_Open(sigma, P, true, 1);
  check.assign(0);

  for (unsigned int i= 0; i < P.nplayers(); i++)
    {
      check+= sigma[i];
    }

  if (check != 0)
    {
      cout << "ABORT CHECK KEYGEN " << endl;
      exit(0);
    }
}
