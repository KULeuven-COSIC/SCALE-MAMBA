/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "MASCOTTriples.h"
#include "LSSS/PRSS.h"
#include "Math/gfp.h"
#include "OT/SimpleROT.h"
#include "Tools/Crypto.h"
#include "config.h"
#include <unistd.h>

using namespace std::chrono;

MASCOTTriples::MASCOTTriples(Player &Pl, bigint fsize, gfp delta, unsigned int connection) : P(Pl)
{
  last_thread= 0;
  myThreadNb= connection;
  totalTriplesGenerated= 0;
  size_field= fsize;
  bit_size_field= numBits(size_field);
  keyGen_sec= 128;
  gf2n_to_gfp_sec= ceil((bit_size_field + keyGen_sec) / (float) OT_comp_sec);
  tau= 3;
  cout << "[MASCOTTriples - constructor] tau (number of triples to be generated per output triples) is set to " << tau << endl;
  nbTriples= 1 << 9;
  cout << "[MASCOTTriples - constructor] Batch size for triple generation set to " << nbTriples << endl;
  gf2n::init_field(OT_comp_sec);
  gfp::init_field(size_field);

  mac_usable_triples.resize(3);
  usable_triples.resize(3);

  nbPlayers= P.nplayers();
  nbOTs= (unsigned long int) nbTriples * (unsigned long int) gf2n_to_gfp_sec * (unsigned long int) tau * (unsigned long int) bit_size_field;

  nbOTsOutput= tau * bit_size_field * nbTriples;
  //Initalize every vector once and for all (trying to solve memory issue)
  triples.resize(3);
  triples[0].resize(tau * nbTriples);
  triples[1].resize(nbTriples);
  triples[2].resize(tau * nbTriples);

  ROTS.resize(nbPlayers);
  ROTR.resize(nbPlayers);
  COPE_S.resize(nbPlayers);
  COPE_R.resize(nbPlayers);

  g.resize(bit_size_field);
  for (unsigned int i= 0; i < bit_size_field; i++)
    {
      g[i].assign(2);
      g[i].power(i);
    }

  combined_triples.resize(5);
  mac_combined_triples.resize(5);
  for (unsigned int i= 0; i < 5; i++)
    {
      if (i != 1)
        {
          combined_triples[i].resize(nbTriples);
        }
      mac_combined_triples[i].resize(nbTriples);
    }

  //Initialize every PRG
  G.ReSeed(myThreadNb + 1);

  uint8_t seed[SEED_SIZE];
  AgreeRandom(P, seed, SEED_SIZE, 1);
  G2.SetSeedFromRandom(seed);

  isInit= false;
  Delta= delta;
}

MASCOTTriples::MASCOTTriples(Player &Pl, bigint fsize, unsigned int connection) : P(Pl)
{
  last_thread= 0;
  myThreadNb= connection;
  totalTriplesGenerated= 0;
  size_field= fsize;
  bit_size_field= numBits(size_field);
  keyGen_sec= 128;
  gf2n_to_gfp_sec= ceil((bit_size_field + keyGen_sec) / (float) OT_comp_sec);
  tau= 3;
  cout << "[MASCOTTriples - constructor] tau (number of triples to be generated per output triples) is set to " << tau << endl;
  nbTriples= 1 << 9;
  cout << "[MASCOTTriples - constructor] Batch size for triple generation set to " << nbTriples << endl;
  gf2n::init_field(OT_comp_sec);
  gfp::init_field(size_field);

  mac_usable_triples.resize(3);
  usable_triples.resize(3);
  Delta.assign(0);

  nbPlayers= P.nplayers();
  nbOTs= (unsigned long int) nbTriples * (unsigned long int) gf2n_to_gfp_sec * (unsigned long int) tau * (unsigned long int) bit_size_field;

  nbOTsOutput= tau * bit_size_field * nbTriples;
  //Initalize every vector once and for all (trying to solve memory issue)
  triples.resize(3);
  triples[0].resize(tau * nbTriples);
  triples[1].resize(nbTriples);
  triples[2].resize(tau * nbTriples);

  ROTS.resize(nbPlayers);
  ROTR.resize(nbPlayers);
  COPE_S.resize(nbPlayers);
  COPE_R.resize(nbPlayers);

  g.resize(bit_size_field);
  for (unsigned int i= 0; i < bit_size_field; i++)
    {
      g[i].assign(2);
      g[i].power(i);
    }

  combined_triples.resize(5);
  mac_combined_triples.resize(5);
  for (unsigned int i= 0; i < 5; i++)
    {
      if (i != 1)
        {
          combined_triples[i].resize(nbTriples);
        }
      mac_combined_triples[i].resize(nbTriples);
    }

  //Initialize every PRG
  G.ReSeed(0);

  uint8_t seed[SEED_SIZE];
  AgreeRandom(P, seed, SEED_SIZE, 1);
  G2.SetSeedFromRandom(seed);

  isInit= false;
}

//Player 0 says if we should stop triple generation
int check_exit(const Player &P, int myThreadNb, bool kg)
{
  int result= 0;
  string ss= "-";
  if (P.whoami() == 0)
    {
      if (not kg)
        {
          result= 1;
          ss= "E";
        }
      P.send_all(ss, 1);
    }
  else
    {
      P.receive_from_player(0, ss, 1);
      if (ss.compare("E") == 0)
        {
          result= 1;
        }
    }
  return result;
}

void MASCOTTriples::execute(std::mutex &mtx, vector<list<gfp>> &u_triples, vector<list<gfp>> &mac_u_triples, bool &keepGoing, unsigned long int *totalProduced)
{
  gfp::init_field(size_field);
  cout << "Starting producing in THREAD" << myThreadNb << endl;
  //bool kg = true;
  unsigned int localProduced= 0;
  while (true)
    {
      if (check_exit(P, myThreadNb, keepGoing) == 1)
        {
          break;
        }

      Multiply();
      Combine();
      Authenticate();
      Sacrifice();
      mtx.lock();
      *totalProduced= *totalProduced + nbTriples;
      localProduced+= nbTriples;
      for (unsigned int j= 0; j < 3; j++)
        {
          while (!usable_triples[j].empty())
            {
              u_triples[j].push_back(usable_triples[j].back());
              usable_triples[j].pop_back();

              mac_u_triples[j].push_back(mac_usable_triples[j].back());
              mac_usable_triples[j].pop_back();
            }
        }
      if (*totalProduced % 100000 < nbTriples)
        {
          cout << "Produced " << *totalProduced << " triples so far " << endl;
        }
      mtx.unlock();
    }

  cout << "Ending production in THREAD" << myThreadNb << endl;
  cout << "Produced " << *totalProduced << " triples in total in this field" << endl;
}

void MASCOTTriples::Multiply()
{
  //1st step of MASCOT

  for (unsigned int i= 0; i < nbTriples; i++)
    {
      triples[1][i].randomize(G);
      for (unsigned int j= 0; j < tau; j++)
        {
          triples[0][i * tau + j].randomize(G);
        }
    }

  //2nd step of MASCOT
  unsigned int whoami= P.whoami();
  CryptoPP::RandomPool RNG;

  //Extract nbOTs bits from triples[0] (which represents a)
  bigint tmp;
  unsigned long int currIndex;
  BitVector bit_triples0(nbOTs);
  for (unsigned long int i= 0; i < nbTriples * tau; i++)
    {
      vector<int> tmp_bit_array(bit_size_field);
      to_bigint(tmp, triples[0][i], false);
      bigint_to_bits(tmp_bit_array, tmp);
      for (unsigned int j= 0; j < bit_size_field; j++)
        {
          for (unsigned int jj= 0; jj < gf2n_to_gfp_sec; jj++)
            {
              currIndex= (i * bit_size_field * gf2n_to_gfp_sec) + (j * gf2n_to_gfp_sec) + jj;
              bit_triples0.set_bit(currIndex, tmp_bit_array[j]);
            }
        }
    }

  /* Create choicebits for Delta */
  if (!isInit)
    {
      vector<int> choicebits(OT_comp_sec);
      for (unsigned int i= 0; i < OT_comp_sec; i++)
        {
          choicebits[i]= G.get_uchar() & 1;
        }
      /*(a) and (b), ROT stuff */
      //We do first the init COT between i and whoami
      for (unsigned int i= 0; i < nbPlayers; i++)
        {
          if (i < whoami)
            {
              ROTS[i].init(P, i, RNG, choicebits, 1);
            }
          else if (i > whoami)
            {
              ROTR[i].init(P, i, RNG, 1);
            }
        }

      //The other way around
      for (unsigned int i= 0; i < nbPlayers; i++)
        {
          if (i > whoami)
            {
              ROTS[i].init(P, i, RNG, choicebits, 1);
            }
          else if (i < whoami)
            {
              ROTR[i].init(P, i, RNG, 1);
            }
        }
    }

  /*Doing calls to OT next round and converting output from gf2n to gfp*/
  vector<vector<gfp>> dh_receiver(nbPlayers, vector<gfp>(nbOTsOutput));
  vector<vector<vector<gfp>>> out_vec_sender_gfp(nbPlayers, vector<vector<gfp>>(nbOTsOutput, vector<gfp>(2)));

  {
    vector<vector<gf2n>> out_vec_receiver(nbPlayers, vector<gf2n>(nbOTs));
    vector<vector<vector<gf2n>>> out_vec_sender(nbPlayers, vector<vector<gf2n>>(nbOTs, vector<gf2n>(2)));
    for (unsigned int i= 0; i < nbPlayers; i++)
      {
        if (i < whoami)
          {
            ROTR[i].next_iteration(P, nbOTs, bit_triples0, out_vec_receiver[i]);
          }
        else if (i > whoami)
          {
            ROTS[i].next_iteration(P, nbOTs, out_vec_sender[i]);
          }
      }

    for (unsigned int i= 0; i < nbPlayers; i++)
      {
        if (i > whoami)
          {
            ROTR[i].next_iteration(P, nbOTs, bit_triples0, out_vec_receiver[i]);
          }
        else if (i < whoami)
          {
            ROTS[i].next_iteration(P, nbOTs, out_vec_sender[i]);
          }
      }

    //Converting gf2n output of OTs to gfp
    uint8_t *buffer_rcv;
    uint8_t *buffer_send0;
    uint8_t *buffer_send1;
    buffer_rcv= (uint8_t *) malloc(16 * gf2n_to_gfp_sec * sizeof(uint8_t));
    buffer_send0= (uint8_t *) malloc(16 * gf2n_to_gfp_sec * sizeof(uint8_t));
    buffer_send1= (uint8_t *) malloc(16 * gf2n_to_gfp_sec * sizeof(uint8_t));

    for (unsigned int i= 0; i < nbPlayers; i++)
      {
        if (i != P.whoami())
          {
            for (unsigned int j= 0; j < nbOTsOutput; j++)
              {
                for (unsigned int jj= 0; jj < gf2n_to_gfp_sec; jj++)
                  {
                    currIndex= j * gf2n_to_gfp_sec + jj;
                    out_vec_receiver[i][currIndex].store_into_buffer(&buffer_rcv[jj * 16]);
                    out_vec_sender[i][currIndex][0].store_into_buffer(&buffer_send0[jj * 16]);
                    out_vec_sender[i][currIndex][1].store_into_buffer(&buffer_send1[jj * 16]);
                  }
                bigintFromBytes(tmp, buffer_rcv, 16 * gf2n_to_gfp_sec);
                dh_receiver[i][j].assign(tmp);

                bigintFromBytes(tmp, buffer_send0, 16 * gf2n_to_gfp_sec);
                out_vec_sender_gfp[i][j][0].assign(tmp);

                bigintFromBytes(tmp, buffer_send1, 16 * gf2n_to_gfp_sec);
                out_vec_sender_gfp[i][j][1].assign(tmp);
              }
          }
      }

    free(buffer_rcv);
    free(buffer_send0);
    free(buffer_send1);
  }

  /* (c) - compute the dh and send them */
  unsigned int indexRound;
  int currBit;
  gfp tmp_gfp;
  for (unsigned int i= 0; i < nbPlayers; i++)
    {
      if (i < P.whoami())
        {
          ostringstream os;
          for (unsigned int j= 0; j < nbOTsOutput; j++)
            {
              indexRound= floor(j / (tau * bit_size_field));
              tmp_gfp= (out_vec_sender_gfp[i][j][0] - out_vec_sender_gfp[i][j][1]) + triples[1][indexRound];
              tmp_gfp.output(os, false);
            }
          P.send_to_player(i, os.str(), 1);
        }
      else if (i > P.whoami())
        {
          string ss;
          P.receive_from_player(i, ss, 1);
          istringstream is(ss);

          for (unsigned int j= 0; j < nbOTsOutput; j++)
            {
              tmp_gfp.input(is, false);
              currBit= bit_triples0.get_bit(j * gf2n_to_gfp_sec);
              if (currBit != 0)
                {
                  dh_receiver[i][j]+= tmp_gfp;
                }
            }
        }
    }

  for (unsigned int i= 0; i < nbPlayers; i++)
    {
      if (i > P.whoami())
        {

          ostringstream os;
          for (unsigned int j= 0; j < nbOTsOutput; j++)
            {
              indexRound= floor(j / (tau * bit_size_field));
              tmp_gfp= (out_vec_sender_gfp[i][j][0] - out_vec_sender_gfp[i][j][1]) + triples[1][indexRound];
              tmp_gfp.output(os, false);
            }
          P.send_to_player(i, os.str(), 1);
        }
      else if (i < P.whoami())
        {
          string ss;
          P.receive_from_player(i, ss, 1);
          istringstream is(ss);

          for (unsigned int j= 0; j < nbOTsOutput; j++)
            {
              tmp_gfp.input(is, false);
              currBit= bit_triples0.get_bit(j * gf2n_to_gfp_sec);
              if (currBit != 0)
                {
                  dh_receiver[i][j]+= tmp_gfp;
                }
            }
        }
    }

  /*(3) compute c from intermediate c vectors */
  for (unsigned int i= 0; i < nbTriples * tau; i++)
    {
      currIndex= floor(i / tau);
      triples[2][i]= triples[0][i] * triples[1][currIndex];
      for (unsigned int j= 0; j < nbPlayers; j++)
        {
          if (j != P.whoami())
            {
              for (unsigned int k= 0; k < bit_size_field; k++)
                {
                  triples[2][i]+= g[k] * (dh_receiver[j][k + ((i % tau) + currIndex * tau) * bit_size_field] - out_vec_sender_gfp[j][k + ((i % tau) + currIndex * tau) * bit_size_field][0]);
                }
            }
        }
    }
}

void MASCOTTriples::Combine()
{
  /*(1) sampling r, \hat{r}*/
  int currIndex;

  /*(2) combine from multiply to a,c,\hat{a},\hat{c}*/
  //0-a, 1-b, 2-c, 3-\hat{a}, 4-\hat{c}
  gfp tmp_gfp;
  for (unsigned int j= 0; j < nbTriples; j++)
    {
      combined_triples[0][j].assign_zero();
      combined_triples[2][j].assign_zero();
      combined_triples[3][j].assign_zero();
      combined_triples[4][j].assign_zero();

      for (unsigned int jj= 0; jj < tau; jj++)
        {
          currIndex= j * tau + jj;
          tmp_gfp.randomize(G2);
          combined_triples[0][j]+= triples[0][currIndex] * tmp_gfp;
          combined_triples[2][j]+= triples[2][currIndex] * tmp_gfp;
          tmp_gfp.randomize(G2);
          combined_triples[3][j]+= triples[0][currIndex] * tmp_gfp;
          combined_triples[4][j]+= triples[2][currIndex] * tmp_gfp;
        }
    }

  //b is already of the right form
  combined_triples[1]= triples[1];
}

void MASCOTTriples::Authenticate()
{

  unsigned int nbPlayers= P.nplayers();
  unsigned int whoami= P.whoami();
  /*Create authenticated shares from the output of Combine*/
  /*Init MAC key and COPE OT*/
  CryptoPP::RandomPool RNG;

  if (Delta == 0)
    {
      Delta.randomize(G);
    }

  if (!isInit)
    {

      for (unsigned int i= 0; i < nbPlayers; i++)
        {
          if (i < whoami)
            {
              COPE_R[i].init(P, i, bit_size_field, Delta, RNG, 1);
            }
          else if (i > whoami)
            {
              COPE_S[i].init(P, i, bit_size_field, RNG, 1);
            }
        }

      for (unsigned int i= 0; i < nbPlayers; i++)
        {
          if (i > whoami)
            {
              COPE_R[i].init(P, i, bit_size_field, Delta, RNG, 1);
            }
          else if (i < whoami)
            {
              COPE_S[i].init(P, i, bit_size_field, RNG, 1);
            }
        }
      cout << "[MASCOTTriples - Authenticate] Finished Base-OTs" << endl;
      isInit= true;
    }
  /*Input share to get authenticated share*/
  //1- Sample x0
  gfp x0;
  x0.randomize(G);

  //2- Create additive sharings - WE ALREADY HAVE A SHARING !

  //3- Call to COPE.extend
  vector<vector<gfp>> q(nbPlayers, vector<gfp>(5 * nbTriples + 1));
  vector<vector<gfp>> t(nbPlayers, vector<gfp>(5 * nbTriples + 1));
  vector<gfp> x_for_COPE(5 * nbTriples + 1);
  x_for_COPE[0]= x0;
  for (unsigned int i= 0; i < 5 * nbTriples; i++)
    {
      x_for_COPE[i + 1]= combined_triples[(i) % 5][floor((i) / 5)];
    }

  for (unsigned int i= 0; i < nbPlayers; i++)
    {
      if (i < whoami)
        {
          COPE_S[i].extend_vec(P, x_for_COPE, t[i]);
        }
      else if (i > whoami)
        {
          COPE_R[i].extend_vec(P, q[i]);
        }
    }

  for (unsigned int i= 0; i < nbPlayers; i++)
    {
      if (i > whoami)
        {
          COPE_S[i].extend_vec(P, x_for_COPE, t[i]);
        }
      else if (i < whoami)
        {
          COPE_R[i].extend_vec(P, q[i]);
        }
    }

  //Skip 4, included in COPE
  //5 - define the MAC shares
  vector<gfp> mac_shares_sender(5 * nbTriples + 1);
  for (unsigned int i= 0; i < 5 * nbTriples + 1; i++)
    {
      if (i == 0)
        {
          mac_shares_sender[i]= x0 * Delta;
        }
      else
        {
          mac_shares_sender[i]= combined_triples[(i - 1) % 5][floor((i - 1) / 5)] * Delta;
        }

      for (unsigned int j= 0; j < nbPlayers; j++)
        {
          if (j != whoami)
            {
              mac_shares_sender[i]+= t[j][i];
            }
        }
    }

  //6 - Parties sample random vector for linear combination

  //7 - Compute y and broadcast it
  gfp tmp_gfp;
  vector<gfp> y(nbPlayers);
  vector<gfp> m(nbPlayers);
  y[whoami].assign(0);

  for (unsigned int j= 0; j < nbPlayers; j++)
    {
      m[j].assign(0);
      for (unsigned int i= 0; i < 5 * nbTriples + 1; i++)
        {
          tmp_gfp.randomize(G2);
          if (j == whoami)
            {
              if (i == 0)
                {
                  y[j]+= tmp_gfp * x0;
                }
              else
                {
                  y[j]+= tmp_gfp * combined_triples[(i - 1) % 5][floor((i - 1) / 5)];
                }

              m[j]+= tmp_gfp * mac_shares_sender[i];
            }
          else
            {
              m[j]+= tmp_gfp * q[j][i];
            }
        }
    }

  ostringstream os;
  y[whoami].output(os, false);
  P.send_all(os.str(), 1);

  for (unsigned int i= 0; i < nbPlayers; i++)
    {
      if (i != whoami)
        {
          string ss;
          P.receive_from_player(i, ss, 1);
          istringstream is(ss);
          y[i].input(is, false);
        }
    }

  //Mac check
  gfp check;
  vector<vector<gfp>> sigma(nbPlayers, vector<gfp>(nbPlayers));
  for (unsigned int i= 0; i < nbPlayers; i++)
    {
      sigma[i][whoami]= m[i] - y[i] * Delta;
      Commit_And_Open(sigma[i], P, true, 1);
      check.assign(0);
      for (unsigned int j= 0; j < nbPlayers; j++)
        {
          check+= sigma[i][j];
        }
      if (check != 0)
        {
          cout << "ABORT CHECK" << endl;
          exit(0);
        }
    }

  // put shares of mac in mac_combined_triples nicely

  for (unsigned int i= 0; i < nbTriples; i++)
    {
      for (unsigned int j= 0; j < 5; j++)
        {
          mac_combined_triples[j][i]= mac_shares_sender[i * 5 + j + 1];
          for (unsigned int jj= 0; jj < nbPlayers; jj++)
            {
              if (jj != whoami)
                {
                  mac_combined_triples[j][i]+= q[jj][i * 5 + j + 1];
                }
            }
        }
    }
}

void MASCOTTriples::Sacrifice()
{

  unsigned int whoami= P.whoami();
  unsigned int nbPlayers= P.nplayers();
  //2 - Linear comb to get rho
  gfp tmp_gfp;
  vector<vector<gfp>> rho(nbTriples, vector<gfp>(nbPlayers));
  vector<gfp> mac_rho(nbTriples);
  vector<vector<gfp>> theta(nbTriples, vector<gfp>(nbPlayers));
  vector<gfp> mac_theta(nbTriples);
  for (unsigned int i= 0; i < nbTriples; i++)
    {

      rho[i][whoami]= (tmp_gfp * combined_triples[0][i]) - combined_triples[3][i];
      mac_rho[i]= (tmp_gfp * mac_combined_triples[0][i]) - mac_combined_triples[3][i];

      theta[i][whoami]= (tmp_gfp * combined_triples[2][i]);
      mac_theta[i]= (tmp_gfp * mac_combined_triples[2][i]);
    }

  //3 - Open rho
  vector<gfp> rho_opened(nbTriples);
  for (unsigned int i= 0; i < nbTriples; i++)
    {
      ostringstream os;
      vector<string> str_rho_bcast;
      str_rho_bcast.resize(nbPlayers);

      rho[i][whoami].output(os, false);
      str_rho_bcast[whoami]= os.str();

      P.Broadcast_Receive(str_rho_bcast, false, 1);
      rho_opened[i].assign(0);
      for (unsigned int j= 0; j < nbPlayers; j++)
        {
          istringstream is(str_rho_bcast[j]);
          rho[i][j].input(is, false);
          rho_opened[i]+= rho[i][j];
        }
    }

  //4 - Linear comb to get theta
  for (unsigned int i= 0; i < nbTriples; i++)
    {
      theta[i][whoami]= (theta[i][whoami]) - combined_triples[4][i] - (combined_triples[1][i] * rho_opened[i]);
      mac_theta[i]= (mac_theta[i]) - mac_combined_triples[4][i] - (mac_combined_triples[1][i] * rho_opened[i]);
    }

  //5 - Check(rho,theta,rho_opened,0)

  for (unsigned int i= 0; i < nbTriples; i++)
    {
      tmp_gfp.randomize(G2);
      theta[i][whoami]= rho_opened[i] * tmp_gfp * Delta;
      theta[i][whoami]= mac_rho[i] * tmp_gfp - theta[i][whoami];
      tmp_gfp.randomize(G2);
      theta[i][whoami]= theta[i][whoami] + mac_theta[i] * tmp_gfp;
    }

  //Mac check y, mac_y
  gfp check;
  for (unsigned int i= 0; i < nbTriples; i++)
    {
      Commit_And_Open(theta[i], P, true, 1);
      check.assign(0);
      for (unsigned int j= 0; j < nbPlayers; j++)
        {
          check+= theta[i][j];
        }
      if (check != 0)
        {
          cout << "ABORT CHECK at iteration " << i << endl;
          exit(0);
        }
    }

  //Push this batch of triples into usable ones
  for (unsigned int i= 0; i < nbTriples; i++)
    {
      for (unsigned int j= 0; j < 3; j++)
        {
          usable_triples[j].push_back(combined_triples[j][i]);
          mac_usable_triples[j].push_back(mac_combined_triples[j][i]);
        }
    }

  totalTriplesGenerated+= nbTriples;
}

void MASCOTTriples::Multiplication(vector<gfp> &a, vector<gfp> &mac_a, vector<gfp> &b, vector<gfp> &mac_b, vector<gfp> &out, vector<gfp> &mac_out, vector<vector<list<gfp>>> &u_triples, vector<vector<list<gfp>>> &mac_u_triples, deque<mutex> &mtx)
{

  unsigned int nbMults= a.size();
  vector<vector<gfp>> curr_triples;
  vector<vector<gfp>> mac_curr_triples;
  curr_triples.resize(3);
  mac_curr_triples.resize(3);

  //Get enough triples for all multiplications
  unsigned int nbThreads= u_triples.size();
  unsigned int currsize= 0;
  unsigned int currThread;
  while (nbMults > currsize)
    {
      for (unsigned int i= 0; i < nbThreads; i++)
        {
          currThread= (i + last_thread) % nbThreads;

          mtx[currThread].lock();
          for (unsigned int j= 0; j < 3; j++)
            {

              for (unsigned int k= 0; k < nbTriples; k++)
                {
                  while (u_triples[currThread][j].size() < nbTriples)
                    {
                      mtx[currThread].unlock();
                      usleep(200000);
                      mtx[currThread].lock();
                    }
                  curr_triples[j].push_back((u_triples[currThread][j]).front());
                  mac_curr_triples[j].push_back((mac_u_triples[currThread][j]).front());
                  (u_triples[currThread][j]).pop_front();
                  (mac_u_triples[currThread][j]).pop_front();
                }
            }
          mtx[currThread].unlock();
          currsize= curr_triples[0].size();
          if (currsize >= nbMults)
            {
              last_thread= currThread + 1;
              break;
            }
        }
    }

  //Compute epsilon and rho
  vector<gfp> epsilon;
  vector<gfp> mac_epsilon;
  vector<gfp> rho;
  vector<gfp> mac_rho;

  epsilon.resize(nbMults);
  mac_epsilon.resize(nbMults);
  rho.resize(nbMults);
  mac_rho.resize(nbMults);

  for (unsigned int i= 0; i < nbMults; i++)
    {
      epsilon[i]= a[i] - curr_triples[0][i];
      mac_epsilon[i]= mac_a[i] - mac_curr_triples[0][i];

      rho[i]= b[i] - curr_triples[1][i];
      mac_rho[i]= mac_b[i] - mac_curr_triples[1][i];
    }

  //Open rho and epsilon
  vector<gfp> opened_epsilon;
  vector<gfp> opened_rho;
  gfp tmp;

  opened_epsilon.resize(nbMults);
  opened_rho.resize(nbMults);

  for (unsigned int i= 0; i < nbMults; i++)
    {
      ostringstream os;
      vector<string> str_rho_bcast;

      str_rho_bcast.resize(P.nplayers());
      rho[i].output(os, false);
      str_rho_bcast[P.whoami()]= os.str();
      P.Broadcast_Receive(str_rho_bcast, false, 1);

      opened_rho[i].assign(0);

      for (unsigned int j= 0; j < P.nplayers(); j++)
        {
          istringstream is(str_rho_bcast[j]);
          tmp.input(is, false);
          opened_rho[i]+= tmp;
        }
    }

  for (unsigned int i= 0; i < nbMults; i++)
    {
      ostringstream os;
      vector<string> str_epsilon_bcast;

      str_epsilon_bcast.resize(P.nplayers());
      epsilon[i].output(os, false);
      str_epsilon_bcast[P.whoami()]= os.str();
      P.Broadcast_Receive(str_epsilon_bcast, false, 1);

      opened_epsilon[i].assign(0);

      for (unsigned int j= 0; j < P.nplayers(); j++)
        {
          istringstream is(str_epsilon_bcast[j]);
          tmp.input(is, false);
          opened_epsilon[i]+= tmp;
        }
    }

  //Put epsilon, rho and their macs to list for check later
  for (unsigned int i= 0; i < nbMults; i++)
    {
      opened_values.push_back(opened_rho[i]);
      opened_values.push_back(opened_epsilon[i]);

      mac_opened_values.push_back(mac_rho[i]);
      mac_opened_values.push_back(mac_epsilon[i]);
    }

  //Compute the output
  for (unsigned int i= 0; i < nbMults; i++)
    {
      out[i]= curr_triples[2][i] + opened_epsilon[i] * curr_triples[1][i] + opened_rho[i] * curr_triples[0][i];
      mac_out[i]= mac_curr_triples[2][i] + opened_epsilon[i] * mac_curr_triples[1][i] + opened_rho[i] * mac_curr_triples[0][i] + opened_epsilon[i] * opened_rho[i] * Delta;
      if (P.whoami() == 0)
        {
          out[i]+= opened_rho[i] * opened_epsilon[i];
        }
    }
}

void MASCOTTriples::Check()
{
  unsigned long int nbOpened= opened_values.size();
  cout << "[MASCOTTriples - Check ] Checking " << nbOpened << " opened values " << endl;
  vector<gfp> r;

  //Agree on a random r
  r.resize(nbOpened);
  for (unsigned long int i= 0; i < nbOpened; i++)
    {
      r[i].randomize(G2);
    }

  //compute y and mac_y
  gfp y;
  gfp mac_y;

  y.assign(0);
  mac_y.assign(0);
  for (unsigned long int i= 0; i < nbOpened; i++)
    {
      y+= r[i] * opened_values[i];
      mac_y+= r[i] * mac_opened_values[i];
    }

  //Mac check on y and mac_y
  vector<gfp> sigma;
  gfp check;
  sigma.resize(P.nplayers());

  sigma[P.whoami()]= mac_y - y * Delta;
  Commit_And_Open(sigma, P, true, 1);

  check.assign(0);

  for (unsigned int i= 0; i < P.nplayers(); i++)
    {
      check+= sigma[i];
    }

  if (check != 0)
    {
      cout << "ABORT CHECK OPENED VALUES" << endl;
      throw bad_value();
    }
}

void MASCOTTriples::Open(gfp &a, gfp &mac_a, gfp &out)
{
  ostringstream os;
  vector<string> str_a_bcast;
  gfp tmp;

  str_a_bcast.resize(P.nplayers());
  a.output(os, false);
  str_a_bcast[P.whoami()]= os.str();
  P.Broadcast_Receive(str_a_bcast, false, 1);

  out.assign(0);
  for (unsigned int i= 0; i < P.nplayers(); i++)
    {
      istringstream is(str_a_bcast[i]);
      tmp.input(is, false);
      out+= tmp;
    }

  opened_values.push_back(out);
  mac_opened_values.push_back(mac_a);
}
