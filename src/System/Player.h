/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _Player
#define _Player

#include "openssl/ssl.h"

#include <openssl/sha.h>

#include "Math/gfp.h"
#include "SystemData.h"
#include "Tools/Timer.h"
#include "Tools/random.h"
#include <map>

void Init_SSL_CTX(SSL_CTX *&ctx, unsigned int me, const SystemData &SD, const string &rootDirectory= "");

inline void Destroy_SSL_CTX(SSL_CTX *ctx) { SSL_CTX_free(ctx); }

class Player
{
  unsigned int me; // My player number

#ifdef BENCH_NETDATA
  // network data in bytes
  mutable long data_sent;
  mutable long data_received;
  // messages sent (broadcast and pp)
  mutable long pp_messages_sent;
  mutable long br_messages_sent;
#endif

  // We have an array of ssl[nplayer][3] connections
  // The 0th connection is for normal communication
  // The 1th connection is for private input and output
  //  - To avoid problems with instructions ordering
  // The 2nd connection is for GC and OT operations, again
  // to avoid issues with instruction ordering
  //   - Note if you increase the number of connections
  //     you also need to increase the second dimension
  //     in csockets
  vector<vector<SSL *>> ssl;

  vector<gfp> mac_keys;

  // This maintains a running hash to check broadcasts are
  // correct, when we need/want to do this
  vector<SHA256_CTX> sha256;

public:
  PRNG G; // Each player has a local PRNG
          // Avoids needing to set one up all the time

  // We have a set of timers here to use for debug purposes
  // when needed.
  mutable vector<Timer> clocks;

  // Thread specifies which thread this instance is related to
  Player(int mynumber, const SystemData &SD, int thread, SSL_CTX *ctx,
         vector<vector<int>> &csockets,
         const vector<gfp> &MacK, int verbose);

  ~Player();

  // Send and receive strings
  void send_all(const string &o, int connection= 0, bool verbose= false) const;
  void send_to_player(int player, const string &o, int connection= 0) const;
  void receive_from_player(int i, string &o, int connection= 0, bool verbose= false) const;

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

  /* Broadcast and Receive data to/from all players
   *  - Assumes o[me] contains the thing broadcast by me
   *  - Check says whether we do a hash check or not
   */
  void Broadcast_Receive(vector<string> &o, bool check= false, int connection= 0);

  /* Runs the broadcast check for any checked broadcast */
  void Check_Broadcast(int connection= 0);

  /* This sends o[i] to player i for all i,
   * then receives back o[i] from player i
   */
  void Send_Distinct_And_Receive(vector<string> &o, int connection= 0) const;

#ifdef BENCH_NETDATA
  void print_network_data(int thread_num);
#endif
};

#endif
