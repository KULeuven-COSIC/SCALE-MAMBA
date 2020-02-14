/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include <iomanip>
#include <iostream>
#include <sstream>
using namespace std;

#include "Math/Matrix.h"
#include "Open_Protocol.h"
#include "Tools/Crypto.h"
#include "config.h"

Open_Protocol::Open_Protocol()
{
  vals.resize(3);
  macs.resize(3);
  sha256.resize(3);
  for (unsigned int i= 0; i < 3; i++)
    {
      macs[i].resize(Share::SD.nmacs);
      SHA256_Init(&sha256[i]);
      // Counts the balance of sends and receives in each connection
      counter[i]= 0;
      // Counts the balance of all opens in each connection
      open_cnt[i]= 0;
    }
}

Open_Protocol::~Open_Protocol()
{
  for (int i= 0; i < 3; i++)
    {
      if (open_cnt[i] > 0)
        {
          cerr << "Problem cleaning up Open_Protocol object. Data is still hanging around" << endl;
          abort();
        }
    }
}

void Open_Protocol::AddToMacs(const vector<Share> &shares, int connection)
{
  for (unsigned int i= 0; i < shares.size(); i++)
    {
      for (unsigned int j= 0; j < Share::SD.nmacs; j++)
        {
          macs[connection][j].push_back(shares[i].mac[j]);
        }
    }
}

void Open_Protocol::AddToValues(const vector<gfp> &values, int connection)
{
  vals[connection].insert(vals[connection].end(), values.begin(), values.end());
}

void Open_Protocol::Open_To_All_Begin(vector<gfp> &values,
                                      const vector<Share> &S, Player &P,
                                      int connection,
                                      bool verbose)
{
  values.resize(S.size()); // Get rid of tiresome segfault errors
  if (verbose)
    {
      cout << "Size: " << S.size() << " " << S[0].a.size() << endl;
    }
  if (Share::SD.type == Full)
    {
      stringstream ss;
      for (unsigned int i= 0; i < S.size(); i++)
        {
          values[i]= S[i].a[0];
          values[i].output(ss, false);
        }
      P.send_all(ss.str(), connection);
    }
  else
    {
      for (unsigned int p= 0; p < P.nplayers(); p++)
        {
          if (Share::SD.OpenC[P.whoami()][p] == 1 && p != P.whoami())
            {
              stringstream ss;
              for (unsigned int i= 0; i < S.size(); i++)
                {
                  for (unsigned int k= 0; k < S[0].a.size(); k++)
                    {
                      if (Share::SD.RCt[P.whoami()][p][k] == 1)
                        {
                          if (!verbose)
                            {
                              S[i].a[k].output(ss, false);
                            }
                          else
                            {
                              S[i].a[k].output(ss, true);
                              ss << " ";
                            }
                        }
                    }
                }
              if (verbose)
                {
                  printf("Sending from %d to %d : %s\n", P.whoami(), p,
                         ss.str().c_str());
                }
              P.send_to_player(p, ss.str(), connection);
            }
        }
      counter[connection]+= S.size();
    }
}

/* All data for the checkig is processed here to ensure 
 * we get no inconsistency in check when swapping between
 * connection zero and connection one
 */
void Open_Protocol::Open_To_All_End(vector<gfp> &values, const vector<Share> &S,
                                    Player &P, int connection, bool verbose)
{
  if (Share::SD.type == Full)
    {
      istringstream is;
      string ss;
      gfp a;
      for (unsigned int j= 0; j < P.nplayers(); j++)
        {
          if (j != P.whoami())
            {
              P.receive_from_player(j, ss, connection);
              istringstream is(ss);
              for (unsigned int i= 0; i < S.size(); i++)
                {
                  a.input(is, false);
                  values[i].add(a);
                }
            }
        }
      AddToMacs(S, connection);
      AddToValues(values, connection);
      if (open_cnt[connection] > open_check_gap)
        {
          RunOpenCheck(P, "", connection);
        }
    }
  else
    {
      unsigned int p= P.whoami();
      vector<stringstream> is(P.nplayers());
      string ss;
      for (unsigned int i= 0; i < P.nplayers(); i++)
        {
          if (Share::SD.OpenC[i][p] == 1 && i != p)
            {
              P.receive_from_player(i, ss, connection, verbose);
              is[i] << ss;
              if (verbose)
                {
                  printf("Received from %d to %d : %s\n", i, P.whoami(), ss.c_str());
                }
            }
        }

      vector<gfp> vs(Share::SD.ReconS[p].size());
      gfp t;
      for (unsigned int i= 0; i < S.size(); i++)
        {
          int c= 0;
          for (unsigned int j= 0; j < Share::SD.M.nplayers(); j++)
            {
              for (unsigned int k= 0; k < Share::SD.M.shares_per_player(j); k++)
                {
                  if (j == p)
                    {
                      vs[c]= S[i].a[k];
                      c++;
                    }
                  else if (Share::SD.RCt[j][p][k] == 1)
                    {
                      if (!verbose)
                        {
                          vs[c].input(is[j], false);
                        }
                      else
                        {
                          vs[c].input(is[j], true);
                        }
                      c++;
                    }
                }
            }
          values[i]= dot_product(vs, Share::SD.ReconS[p]);
          vector<gfp> ssv= Mul(Share::SD.ReconSS[p], vs);

          // Hash into the state
          stringstream ss;
          for (unsigned int j= 0; j < ssv.size(); j++)
            {
              ssv[j].output(ss, false);
            }
          SHA256_Update(&sha256[connection], ss.str().c_str(), ss.str().size());
        }
      counter[connection]-= S.size();
    }
  open_cnt[connection]+= values.size();
  if (open_cnt[connection] > open_check_gap && counter[connection] == 0)
    {
      RunOpenCheck(P, "", connection);
    }
}

/* We run it on the connection c as other connection might have
 * some opening still to do
 */
void Open_Protocol::RunOpenCheck(Player &P, const string &aux, int connection, bool verbose)
{
  if (Share::SD.type == Full)
    {
      uint8_t seed[SEED_SIZE];
      Create_Random_Seed(seed, SEED_SIZE, P, connection);
      PRNG G;
      G.SetSeedFromRandom(seed);

      int neqs= Share::SD.nmacs;
      vector<vector<gfp>> tau(neqs, vector<gfp>(P.nplayers()));

      vector<gfp> gami(neqs), a(neqs);

      gfp r, temp;
      for (int j= 0; j < neqs; j++)
        {
          a[j].assign_zero();
          gami[j].assign_zero();
          for (int i= 0; i < open_cnt[connection]; i++)
            {
              r.almost_randomize(G);
              temp.mul(r, vals[connection][i]);
              a[j].add(temp);

              temp.mul(r, macs[connection][j][i]);
              gami[j].add(temp);
            }
          macs[connection][j].erase(macs[connection][j].begin(), macs[connection][j].begin() + open_cnt[connection]);
          temp.mul(a[j], P.get_mac_key(j));
          tau[j][P.whoami()].sub(gami[j], temp);
        }
      vals[connection].erase(vals[connection].begin(), vals[connection].begin() + open_cnt[connection]);

      Commit_And_Open(tau, P, false, connection);

      gfp t;
      for (int j= 0; j < neqs; j++)
        {
          t.assign_zero();
          for (unsigned int i= 0; i < P.nplayers(); i++)
            {
              t.add(tau[j][i]);
            }
          if (!t.is_zero())
            {
              throw mac_fail();
            }
        }

      // Now check everyone is OK, in case I am honest and another honest
      // player aborted
      //   - Removes need for the Broadcast check in original SPDZ
      vector<string> OK(P.nplayers());
      OK[P.whoami()]= "Y";
      P.Broadcast_Receive(OK, false, connection);
      for (unsigned int i= 0; i < P.nplayers(); i++)
        {
          if (OK[i].compare("Y") != 0)
            {
              throw mac_fail();
            }
        }
    }
  else
    {
      unsigned char hash[SHA256_DIGEST_LENGTH];
      SHA256_Final(hash, &sha256[connection]);
      string ss((char *) hash, SHA256_DIGEST_LENGTH);
      if (verbose)
        {
          printf("Open Check: Sending  : %d : ", P.whoami());
          for (unsigned int i= 0; i < SHA256_DIGEST_LENGTH; i++)
            {
              printf("%02x", (uint8_t) ss.c_str()[i]);
            }
          printf("\n");
          printf("%lu %d\n", ss.size(), SHA256_DIGEST_LENGTH);
        }
      P.send_all(ss, connection, verbose);
      for (unsigned int i= 0; i < P.nplayers(); i++)
        {
          if (i != P.whoami())
            {
              string is;
              P.receive_from_player(i, is, connection, verbose);
              if (verbose)
                {
                  printf("Open Check: Received : %d : ", i);
                  for (unsigned int i= 0; i < SHA256_DIGEST_LENGTH; i++)
                    {
                      printf("%02x", (uint8_t) is.c_str()[i]);
                    }
                  printf("\n");
                  printf("%lu %d\n", is.size(), SHA256_DIGEST_LENGTH);
                }
              if (is != ss)
                {
                  throw hash_fail();
                }
            }
        }
      SHA256_Init(&sha256[connection]);
    }

  // Now deal with the auxillary string
  if (aux.size() != 0)
    {
      if (verbose)
        {
          printf("Auxillary Check: Sending  : %d : ", P.whoami());
          for (unsigned int i= 0; i < aux.size(); i++)
            {
              printf("%02x", (uint8_t) aux.c_str()[i]);
            }
          printf("\n");
        }
      P.send_all(aux, connection, verbose);
      for (unsigned int i= 0; i < P.nplayers(); i++)
        {
          if (i != P.whoami())
            {
              string is;
              P.receive_from_player(i, is, connection, verbose);
              if (verbose)
                {
                  printf("Auxillary Check: Received : %d : ", i);
                  for (unsigned int i= 0; i < is.size(); i++)
                    {
                      printf("%02x", (uint8_t) is.c_str()[i]);
                    }
                  printf("\n");
                }
              if (is != aux)
                {
                  throw inconsistent_inputs();
                }
            }
        }
    }
  open_cnt[connection]= 0;
}

void Open_Protocol::Open_To_One_Begin(unsigned int player_num,
                                      const vector<Share> &S, Player &P)
{
  if (player_num == P.whoami())
    {
      throw sending_error();
    }
  stringstream ss;
  for (unsigned int i= 0; i < S.size(); i++)
    {
      for (unsigned int k= 0; k < Share::SD.M.shares_per_player(P.whoami());
           k++)
        {
          S[i].a[k].output(ss, false);
        }
    }
  P.send_to_player(player_num, ss.str());
}

void Open_Protocol::Open_To_One_End(vector<gfp> &values, const vector<Share> &S,
                                    Player &P)
{
  vector<istringstream> is(P.nplayers());
  for (unsigned int i= 0; i < P.nplayers(); i++)
    {
      if (i != P.whoami())
        {
          string ss;
          P.receive_from_player(i, ss);
          is[i].str(ss);
        }
    }
  if (Share::SD.type == Full)
    {
      for (unsigned int i= 0; i < S.size(); i++)
        {
          values[i].assign_zero();
          gfp temp;
          for (unsigned int p= 0; p < P.nplayers(); p++)
            {
              if (p == P.whoami())
                {
                  values[i].add(S[i].a[0]);
                }
              else
                {
                  temp.input(is[p], false);
                  values[i].add(temp);
                }
            }
        }
    }
  else
    {
      vector<gfp> shares(Share::SD.Parity[0].size());
      for (unsigned int i= 0; i < S.size(); i++)
        {
          int c= 0;
          for (unsigned int p= 0; p < P.nplayers(); p++)
            {
              if (p == P.whoami())
                {
                  for (unsigned int k= 0; k < Share::SD.M.shares_per_player(p); k++)
                    {
                      shares[c]= S[i].a[k];
                      c++;
                    }
                }
              else
                {
                  for (unsigned int k= 0; k < Share::SD.M.shares_per_player(p); k++)
                    {
                      shares[c].input(is[p], false);
                      c++;
                    }
                }
            }
          // We now have the vector of shares. So check parity and reconstruct the secret
          vector<gfp> check= Mul(Share::SD.Parity, shares);

          for (unsigned int j= Share::SD.M.col_dim(); j < check.size(); j++)
            {
              if (!check[j].is_zero())
                {
                  throw receiving_error();
                }
            }
          values[i].assign_zero();
          for (unsigned int j= 0; j < Share::SD.M.col_dim(); j++)
            {
              values[i].add(check[j]);
            }
        }
    }
}
