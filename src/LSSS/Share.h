/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _Share
#define _Share

/* Class for holding a share of a gfp element */

#include <iostream>
#include <vector>
using namespace std;

#include "Math/gfp.h"
#include "ShareData.h"

class Share
{
  int p;
  vector<gfp> a;   // The share
  vector<gfp> mac; // Shares of the mac (when type=FULL)

  // Some sanity checking that shares are assigned OK
  void check() const
  {
#ifdef SH_DEBUG
    if (p == -1)
      {
        throw bad_value();
      }
    if (a.size() != SD.M.shares_per_player(p))
      {
        throw bad_value();
      }
#endif
  }

public:
  static ShareData SD;

  static void init_share_data(const ShareData &S)
  {
    SD= S;
  }
  static string type_string()
  {
    return "Share";
  }

  void assign(const Share &S)
  {
    p= S.p;
    a= S.a;
    mac= S.mac;
    check();
  }

  void assign(unsigned int i, vector<gfp> sv, vector<gfp> macs)
  {
    p= (int) i;
    a= sv;
    mac= macs;
    check();
  }

  // Assign the share value when assigning constant aa
  //   alphai are the local mac keys
  void assign(const gfp &aa, const vector<gfp> &alphai);

  void assign_zero();

  Share()
  { // To try and create a run time error to capture Shares
    // without a player assigned
    p= -1;
    mac.resize(SD.nmacs);
  }
  Share(int pp)
  {
    p= pp;
    a.resize(SD.M.shares_per_player(p));
    mac.resize(SD.nmacs);
    assign_zero();
  }
  Share(const Share &S)
  {
    p= S.p;
    a= S.a;
    mac= S.mac;
  }
  ~Share()
  {
    ;
  }
  Share &operator=(const Share &S)
  {
    if (this != &S)
      {
        assign(S);
      }
    return *this;
  }
  Share(const gfp &aa, int my_num, const vector<gfp> &alphai)
  {
    p= my_num;
    a.resize(SD.M.shares_per_player(p));
    assign(aa, alphai);
  }

  const vector<gfp> &get_shares() const
  {
    return a;
  }
  const vector<gfp> &get_macs() const
  {
    return mac;
  }
  gfp get_share(unsigned int i) const
  {
    return a[i];
  }
  gfp get_mac(unsigned int i) const
  {
    return mac[i];
  }
  void set_shares(const vector<gfp> &aa);
  void set_macs(const vector<gfp> &aa);
  void set_share(unsigned int i, const gfp &aa)
  {
    a[i]= aa;
  }
  void set_mac(unsigned int i, const gfp &aa)
  {
    mac[i]= aa;
  }
  void set_player(int pp)
  {
    p= pp;
    a.resize(SD.M.shares_per_player(p));
    assign_zero();
  }
  void set_player_and_shares(int pp, const vector<gfp> &aa);

  /* Arithmetic Routines */
  void mul(const Share &S, const gfp &aa);
  void add(const Share &S, const gfp &aa, const vector<gfp> &alphai);
  void sub(const Share &S, const gfp &aa, const vector<gfp> &alphai);
  void sub(const gfp &aa, const Share &S, const vector<gfp> &alphai);
  void add(const Share &S1, const Share &S2);
  void sub(const Share &S1, const Share &S2);
  void add(const Share &S1)
  {
    add(*this, S1);
  }
  void negate();

  // Input and output from a stream
  //  - Can do in human or machine only format (later should be faster)
  void output(ostream &s, bool human) const;
  void input(istream &s, bool human);

  /* Takes a vector of shares, one from each player and
     * determines the shared value
     *   - i.e. Partially open the shares
     */
  friend gfp combine(const vector<Share> &S);

  /* Given a set of shares, one from each player and
    * the global key, determines if the sharing is valid
    *   - Mainly for test purposes
    */
  friend bool check_macs(const vector<Share> &S, const vector<gfp> &key);

  friend class Open_Protocol;
};

/* Makes shares of a value val, but ignores the MAC values if any */
void make_shares(vector<Share> &share, const gfp &val, PRNG &G);

#endif
