/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Crypto.h"

#include <iomanip>
#include <iostream>
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
  string ss((char *) hash, SHA256_DIGEST_LENGTH);
  return ss;
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

  ans.randomize(P.G);
  string ee(gfp::size(), ' ');
  ans.output(ee, 0);
  Commit(Comm_e[P.whoami()], Open_e[P.whoami()], ee, P.G);

  P.Broadcast_Receive(Comm_e, false, connection);
  P.Broadcast_Receive(Open_e, false, connection);

  gfp e;
  for (unsigned int i= 0; i < P.nplayers(); i++)
    {
      if (i != P.whoami())
        {
          if (!Open(ee, Comm_e[i], Open_e[i]))
            {
              throw invalid_commitment();
            }
          e.input(ee, 0);
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

  string ee(T::size(), ' ');
  data[P.whoami()].output(ee, 0);

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
          data[i].input(ee, 0);
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

  string ee(T::size() * data.size(), ' ');
  unsigned int pos= 0;
  for (unsigned int j= 0; j < data.size(); j++)
    {
      pos+= data[j][P.whoami()].output(ee, pos);
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
          pos= 0;
          for (unsigned int j= 0; j < data.size(); j++)
            {
              pos+= data[j][i].input(ee, pos);
            }
        }
    }
}

template void Commit_And_Open(vector<gfp> &data, Player &P, bool check, int connection);
template void Commit_And_Open(vector<gf2n> &data, Player &P, bool check, int connection);
template void Commit_And_Open(vector<vector<gfp>> &data, Player &P, bool check, int connection);
template void Commit_And_Open(vector<vector<gf2n>> &data, Player &P, bool check, int connection);
