/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Crypto.h"

#include <iomanip>
#include <iostream>
#include <sstream>
using namespace std;
#include <openssl/sha.h>

#include "Math/gf2n.h"

string Hash(const string &data)
{
  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256_CTX sha256;
  SHA256_Init(&sha256);
  SHA256_Update(&sha256, data.c_str(), data.size());
  SHA256_Final(hash, &sha256);
  stringstream ss;
  for (int i= 0; i < SHA256_DIGEST_LENGTH; i++)
    {
      // (human form) ss << hex << setw(2) << setfill('0') << (int) hash[i];
      ss << hash[i];
    }

  return ss.str();
}

void Commit(string &comm, string &open, const string &data, PRNG &G)
{
  uint8_t random[SEED_SIZE];
  G.get_random_bytes(random, SEED_SIZE);
  open= data + string((char *) random, SEED_SIZE);
  comm= Hash(open);
}

bool Open(string &data, const string &comm, const string &open)
{
  string h= Hash(open);
  if (h != comm)
    {
      return false;
    }
  data= open.substr(0, open.size() - SEED_SIZE);
  return true;
}

void Create_Random(gfp &ans, Player &P, int connection)
{
  vector<string> Comm_e(P.nplayers());
  vector<string> Open_e(P.nplayers());

  gfp e;
  ans.randomize(P.G);
  stringstream ee;
  ans.output(ee, false);
  Commit(Comm_e[P.whoami()], Open_e[P.whoami()], ee.str(), P.G);

  P.Broadcast_Receive(Comm_e, false, connection);
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
          istringstream is(ee);
          e.input(is, false);
          ans.add(ans, e);
        }
    }
}

void Create_Random_Seed(uint8_t *seed, int len, Player &P, int connection)
{
  vector<string> Comm_e(P.nplayers());
  vector<string> Open_e(P.nplayers());

  P.G.get_random_bytes(seed, len);

  string ss((char *) seed, len);
  Commit(Comm_e[P.whoami()], Open_e[P.whoami()], ss, P.G);

  P.Broadcast_Receive(Comm_e, false, connection);
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

template<class T>
void Commit_And_Open(vector<T> &data, Player &P, bool check, int connection)
{
  vector<string> Comm_data(P.nplayers());
  vector<string> Open_data(P.nplayers());

  stringstream ss;
  data[P.whoami()].output(ss, false);
  string ee= ss.str();
  Commit(Comm_data[P.whoami()], Open_data[P.whoami()], ee, P.G);

  P.Broadcast_Receive(Comm_data, check, connection);
  P.Broadcast_Receive(Open_data, check, connection);

  for (unsigned int i= 0; i < P.nplayers(); i++)
    {
      if (i != P.whoami())
        {
          if (!Open(ee, Comm_data[i], Open_data[i]))
            {
              throw invalid_commitment();
            }
          istringstream is(ee);
          data[i].input(is, false);
        }
    }
}

template<>
void Commit_And_Open(vector<string> &data, Player &P, bool check, int connection)
{
  vector<string> Comm_data(P.nplayers());
  vector<string> Open_data(P.nplayers());

  Commit(Comm_data[P.whoami()], Open_data[P.whoami()], data[P.whoami()], P.G);

  P.Broadcast_Receive(Comm_data, check, connection);
  P.Broadcast_Receive(Open_data, check, connection);

  for (unsigned int i= 0; i < P.nplayers(); i++)
    {
      if (i != P.whoami())
        {
          if (!Open(data[i], Comm_data[i], Open_data[i]))
            {
              throw invalid_commitment();
            }
        }
    }
}

template<class T>
void Commit_And_Open(vector<vector<T>> &data, Player &P, bool check, int connection)
{
  vector<string> Comm_data(P.nplayers());
  vector<string> Open_data(P.nplayers());

  string ee;
  for (unsigned int j= 0; j < data.size(); j++)
    {
      stringstream ss;
      data[j][P.whoami()].output(ss, false);
      ee+= ss.str();
    }
  Commit(Comm_data[P.whoami()], Open_data[P.whoami()], ee, P.G);

  P.Broadcast_Receive(Comm_data, check, connection);
  P.Broadcast_Receive(Open_data, check, connection);

  for (unsigned int i= 0; i < P.nplayers(); i++)
    {
      if (i != P.whoami())
        {
          if (!Open(ee, Comm_data[i], Open_data[i]))
            {
              throw invalid_commitment();
            }
          istringstream is(ee);
          for (unsigned int j= 0; j < data.size(); j++)
            {
              data[j][i].input(is, false);
            }
        }
    }
}

template void Commit_And_Open(vector<gfp> &data, Player &P, bool check, int connection);
template void Commit_And_Open(vector<gf2n> &data, Player &P, bool check, int connection);
template void Commit_And_Open(vector<vector<gfp>> &data, Player &P, bool check, int connection);
template void Commit_And_Open(vector<vector<gf2n>> &data, Player &P, bool check, int connection);
