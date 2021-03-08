/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include <algorithm>
#include <string>
using namespace std;

#include "Math/Matrix.h"
#include "PRSS.h"
#include "System/Player.h"
#include "Tools/Crypto.h"

/* Agree a random seed between all players in set A
 *    Gauranteed that whoami is in A
 */
void AgreeRandom(Player &P, vector<unsigned int> &A, uint8_t *seed, int len,
                 int connection)
{
  vector<string> Comm_e(P.nplayers());
  vector<string> Open_e(P.nplayers());

  P.G.get_random_bytes(seed, len);

  string ss((char *) seed, len);
  Commit(Comm_e[P.whoami()], Open_e[P.whoami()], ss, P.G);

  // Send my commitment
  for (unsigned int i= 0; i < A.size(); i++)
    {
      if (A[i] == 1 && i != P.whoami())
        {
          P.send_to_player(i, Comm_e[P.whoami()], connection);
        }
    }
  // Receive other commitments
  for (unsigned int i= 0; i < A.size(); i++)
    {
      if (A[i] == 1 && i != P.whoami())
        {
          P.receive_from_player(i, Comm_e[i], connection);
        }
    }
  // Send my opening
  for (unsigned int i= 0; i < A.size(); i++)
    {
      if (A[i] == 1 && i != P.whoami())
        {
          P.send_to_player(i, Open_e[P.whoami()], connection);
        }
    }
  // Receive other openings
  for (unsigned int i= 0; i < A.size(); i++)
    {
      if (A[i] == 1 && i != P.whoami())
        {
          P.receive_from_player(i, Open_e[i], connection);
        }
    }

  for (unsigned int i= 0; i < A.size(); i++)
    {
      if (A[i] == 1 && i != P.whoami())
        {
          string ee;
          if (!Open(ee, Comm_e[i], Open_e[i]))
            {
              throw invalid_commitment();
            }
          for (int j= 0; j < len; j++)
            {
              seed[j]^= (ee.c_str())[j];
            }
        }
    }
}

void AgreeRandom(Player &P, uint8_t *seed, int len,
                 int connection)
{
  vector<string> Comm_e(P.nplayers());
  vector<string> Open_e(P.nplayers());

  P.G.get_random_bytes(seed, len);

  string ss((char *) seed, len);
  Commit(Comm_e[P.whoami()], Open_e[P.whoami()], ss, P.G);

  // Send my commitment to eveyone and get others back
  P.Broadcast_Receive(Comm_e, false, connection);

  // Send my opening and get others back
  P.Broadcast_Receive(Open_e, false, connection);

  for (unsigned int i= 0; i < P.nplayers(); i++)
    {
      if (i != P.whoami())
        {
          string ee;
          if (!Open(ee, Comm_e[i], Open_e[i]))
            {
              throw invalid_commitment();
            }
          for (int j= 0; j < len; j++)
            {
              seed[j]^= (ee.c_str())[j];
            }
        }
    }
}

template<class Sh, class Fld>
void PRSS_Base<Sh, Fld>::ReplicatedSetUp(Player &P, const CAS &AS, const MSP<Fld> &M)
{
  whoami= P.whoami();
  /* Go through each delta_plus set, find ones I am in
   * and then get a random key for this set
   */
  unsigned int ndelta= AS.delta_plus.size();
  vector<unsigned int> A;
  G.resize(M.shares_per_player(whoami));
  uint8_t seed[SEED_SIZE];
  for (unsigned int i= 0; i < ndelta; i++)
    {
      A.resize(AS.n);
      bool mine= false;
      for (unsigned int j= 0; j < AS.n; j++)
        {
          if (AS.delta_plus[i][j] == 0)
            {
              A[j]= 1;
              if (j == whoami)
                {
                  mine= true;
                }
            }
          else
            {
              A[j]= 0;
            }
        }
      if (mine)
        { // Find index where this seed should go
          unsigned int c= 0;
          for (unsigned int j= 0; j < whoami; j++)
            {
              c+= M.shares_per_player(j);
            }
          int k= -1;
          for (unsigned int j= 0; j < M.shares_per_player(whoami); j++)
            {
              if (M.G(c + j, i).is_one())
                {
                  k= j;
                }
            }
          if (k == -1)
            {
              throw bad_value();
            }
          AgreeRandom(P, A, seed, SEED_SIZE, 0);

          G[k].SetSeedFromRandom(seed);
        }
    }
}

template<class Sh, class Fld>
void PRSS_Base<Sh, Fld>::MSP_SetUp(Player &P, const CAS &AS, const MSP<Fld> &M)
{
  whoami= P.whoami();
  // If AS.delta.size()==0 then (for Shamir) things are too big
  // to do via the AS data. So we use a protocol. To signal
  // this we set Asets etc to be zero
  if (AS.delta_plus.size() == 0)
    {
      Asets.resize(0);
      G.resize(1);
      G[0].ReSeed(2000 + whoami);
      Shares_Of_One.resize(0);
      return;
    }

  G.resize(AS.delta_plus.size());
  Asets.resize(AS.delta_plus.size(), vector<unsigned int>(AS.n));
  Shares_Of_One.resize(AS.delta_plus.size());
  vector<Fld> sh(M.shares_per_player(whoami));
  vector<int> D(AS.n);

  // We go through maximally qualified sets and take their compliments
  // and compute a share vector with this support which encodes one
  for (unsigned int i= 0; i < AS.delta_plus.size(); i++)
    {
      for (unsigned int j= 0; j < AS.n; j++)
        {
          Asets[i][j]= 1 - AS.delta_plus[i][j];
          D[j]= AS.delta_plus[i][j];
        }
      // We want to solve Dx =0 and sum x_i=1
      vector<vector<Fld>> AA= M.extract_rows(D);
      for (unsigned int j= 0; j < AA.size(); j++)
        {
          AA[j].resize(M.col_dim() + 1);
          AA[j][M.col_dim()].assign_zero();
        }
      AA.resize(AA.size() + 1, vector<Fld>(M.col_dim() + 1));
      for (unsigned int j= 0; j < M.col_dim() + 1; j++)
        {
          AA[AA.size() - 1][j].assign_one();
        }
      if (Solvable(AA) == false)
        {
          throw bad_value();
        }

      // Do back substitution
      vector<Fld> bx(M.col_dim());
      BackSubst(bx, AA);
      vector<Fld> x(M.row_dim());
      M.Sharing(x, bx);
      // Extract my part
      int c= 0;
      for (unsigned int j= 0; j < AS.n; j++)
        {
          for (unsigned int k= 0; k < M.shares_per_player(j); k++)
            {
              if (j == whoami)
                {
                  sh[k]= x[c];
                }
              c++;
            }
        }

      // Define the PRNG for this set if I am in it
      uint8_t seed[SEED_SIZE];
      if (Asets[i][whoami] == 1)
        {
          AgreeRandom(P, Asets[i], seed, SEED_SIZE, 0);
          G[i].SetSeedFromRandom(seed);
          Shares_Of_One[i].assign(whoami, sh);
        }
      else
        {
          Shares_Of_One[i].set_player(whoami);
          Shares_Of_One[i].assign_zero();
        }
    }
}

template<class Sh, class Fld>
PRSS_Base<Sh, Fld>::PRSS_Base(Player &P)
{
  if (Sh::SD.type == Full || Sh::SD.type == Other)
    {
      return; /* This is a NOP */
    }
  else if (Sh::SD.type == Replicated)
    {
      ReplicatedSetUp(P, Sh::SD.AS, Sh::SD.M);
    }
  else if (Sh::SD.type == Q2MSP || Sh::SD.type == Shamir)
    {
      MSP_SetUp(P, Sh::SD.AS, Sh::SD.M);
    }
  else
    {
      throw not_implemented();
    }
}

Share PRSS::next_share(Player &P)
{
  Share S(whoami);

  if (Share::SD.type == Replicated)
    {
      vector<gfp> si(G.size());
      for (unsigned int i= 0; i < G.size(); i++)
        {
          si[i].randomize(G[i]);
        }
      S.set_shares(si);
    }
  else
    {
      // If Asets!=0 then use the PRSS, otherwise use a protocol
      if (Asets.size() != 0)
        {
          gfp te;
          Share temp;
          S.assign_zero();
          for (unsigned int i= 0; i < Asets.size(); i++)
            {
              if (Asets[i][whoami] == 1)
                {
                  te.randomize(G[i]);
                  temp.mul(Shares_Of_One[i], te);
                  S.add(temp);
                }
            }
        }
      else
        {
          if (batch_pos == batch.size())
            {
              batch_production(P);
            }
          batch_pos++;
          return batch[batch_pos - 1];
        }
    }

  return S;
}

void PRSS::batch_production(Player &P)
{
  batch_pos= 0;

  gfp te;
  Share temp;
  vector<Share> Sh(P.nplayers());
  // Still use stringstream's here as this is rarely used
  // and its too complex to unpick in some sense
  vector<stringstream> ss(P.nplayers());
  for (unsigned int i= 0; i < batch.size(); i++)
    {
      te.randomize(G[0]);
      make_shares(Sh, te, G[0]);
      batch[i]= Sh[whoami];
      for (unsigned int j= 0; j < P.nplayers(); j++)
        {
          if (j != whoami)
            {
              Sh[j].output(ss[j], false);
            }
        }
    }
  for (unsigned int j= 0; j < P.nplayers(); j++)
    {
      if (j != whoami)
        {
          P.send_to_player(j, ss[j].str(), 0);
        }
    }
  for (unsigned int j= 0; j < P.nplayers(); j++)
    {
      if (j != whoami)
        {
          string ss;
          unsigned int len;
          const uint8_t *P_sbuff= P.receive_from_player(j, len, 0);
          unsigned int pos= 0;
          for (unsigned int i= 0; i < batch.size(); i++)
            {
              pos+= temp.input(P_sbuff + pos);
              batch[i].add(temp);
            }
        }
    }
}

Share2 PRSS2::next_share()
{
  Share2 S(whoami);

  vector<word> si(G.size());
  for (unsigned int i= 0; i < G.size(); i++)
    {
      si[i]= G[i].get_word();
    }
  S.set_shares(si);

  return S;
}

template class PRSS_Base<Share, gfp>;
template class PRSS_Base<Share2, gf2>;
