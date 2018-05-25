/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _Player
#define _Player

#include "openssl/ssl.h"

#include "Math/gfp.h"
#include "SystemData.h"
#include "Tools/random.h"

void Init_SSL_CTX(SSL_CTX *&ctx, int me, const SystemData &SD);

inline void Destroy_SSL_CTX(SSL_CTX *ctx) { SSL_CTX_free(ctx); }

class Player
{
  unsigned int me; // My player number

  vector<SSL *> ssl;

  vector<gfp> mac_keys;

public:
  PRNG G; // Each player has a local PRNG
          // Avoids needing to set one up all the time

  // Thread specifies which thread this instance is related to
  Player(int mynumber, const SystemData &SD, int thread, SSL_CTX *ctx,
         vector<int> &csockets, bool verbose= false);

  ~Player();

  // Send and receive strings
  void send_all(const string &o, bool verbose= false) const;
  void send_to_player(int player, const string &o) const;
  void receive_from_player(int i, string &o, bool verbose= false) const;

  unsigned int whoami() const
  {
    return me;
  }
  unsigned int nplayers() const
  {
    return ssl.size();
  }

  gfp get_mac_key(int i) const
  {
    return mac_keys[i];
  }
  const vector<gfp> &get_mac_keys() const
  {
    return mac_keys;
  }
  void load_mac_keys(int num);

  /* Broadcast and Receive data to/from all players
   *  - Assumes o[me] contains the thing broadcast by me
   */
  void Broadcast_Receive(vector<string> &o) const;

  /* This sends o[i] to player i for all i,
   * then receives back o[i] from player i
   */
  void Send_Distinct_And_Receive(vector<string> &o) const;
};

#endif
