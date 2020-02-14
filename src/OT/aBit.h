/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _aBit
#define _aBit

/* This holds an authenticated bit, where authentication
 * is to all parties and each party holds a bit
 *
 * This also holds an aShare (in the sense of Wang, Ranellucci 
 * and Katz ePrint: 2017/189
 */

#include <list>
#include <vector>
using namespace std;
#include "Math/gf2n.h"

#include "System/Player.h"

/* Note aBit's can also hold authenticated gf2n elements */

class aBit
{
  gf2n x; // The bit held by this party

  vector<gf2n> M; // The MACs on x towards party k
  vector<gf2n> K; // The key on the bit x[k] held by party k

  static unsigned int n;
  static unsigned int whoami;
  static gf2n Delta;

public:
  static gf2n get_Delta() { return Delta; }

  static void set_nplayers(int nn)
  {
    n= nn;
  }

  static void set_nplayers(int nn, int who, gf2n &D)
  {
    n= nn;
    whoami= who;
    Delta= D;
  }

  void assign_zero()
  {
    x.assign_zero();
    for (unsigned int i= 0; i < n; i++)
      {
        M[i].assign_zero();
        K[i].assign_zero();
      }
  }

  void assign_one()
  {
    assign_zero();
    negate();
  }

  aBit()
  {
    M.resize(n);
    K.resize(n);
    assign_zero();
  }

  gf2n get_MAC(int i) const { return M[i]; }
  gf2n get_Key(int i) const { return K[i]; }

  // These should be used with care, as could make an invalid aBit
  void set_value(int b);
  void set_MAC(int i, const gf2n &y) { M[i]= y; }
  void set_Key(int i, const gf2n &y) { K[i]= y; }

  // The following checks we actually have 0/1 entries in x
  int get_value() const;

  // Return the raw x value
  gf2n get_raw_value() const { return x; }

  // Mult the aBit by a gf2n constant
  void mult_by(const gf2n &y, const aBit &z);
  // Add a constant to aBit
  void add(const gf2n &y);

  // Add two aBit's together
  void add(const aBit &y);
  void add(const aBit &x, const aBit &y);

  // This only applies to x when a bit, not a gf2n element
  void negate();
  void negate(const aBit &y)
  {
    *this= y;
    negate();
  }

  /* Does Claim 4.1 of ePrint 2017/214 
   * Producing a share of x*Delta_j for each player 
   */
  void get_Share_x_Delta_j(vector<gf2n> &ans) const;

  /* Arithmetic Operators */
  aBit operator+(const aBit &x) const
  {
    aBit res;
    res.add(*this, x);
    return res;
  }

  aBit &operator+=(const aBit &x)
  {
    add(x);
    return *this;
  }

  friend ostream &operator<<(ostream &s, const aBit &y);

  void output(ostream &s, bool human) const;
  void input(istream &s, bool human);
};

/* Open (and check) a single of aBit */
void Open_aBit(int &dv, const aBit &v, Player &P);

/* Open (and check) a vector of aBits */
void Open_aBits(vector<int> &dv, const vector<aBit> &v, Player &P);

/* Open a vector of aBits to party j 
 *  dv zero if j<>P.whoami
 */
void Open_aBits_To(vector<int> &dv, unsigned int j, const vector<aBit> &v, Player &P);

void check_Bits(const vector<aBit> &ee, Player &P);
void check_Bits(const list<aBit> &ee, Player &P);

#endif
