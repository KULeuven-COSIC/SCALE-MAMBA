/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _Ring
#define _Ring

/* Class to define the basic ring we are working with
 * and any associated data
 */

#include <iostream>
#include <vector>
using namespace std;

class Ring
{
  unsigned int mm, phim;
  vector<int> pi;
  vector<int> pi_inv;
  vector<int> poly;

public:
  void initialize(unsigned int m);

  Ring()
      : mm(0), phim(0)
  {
    ;
  }
  Ring(unsigned int m)
  {
    initialize(m);
  }

  unsigned int phi_m() const
  {
    return phim;
  }
  unsigned int m() const
  {
    return mm;
  }

  int p(int i) const
  {
    return pi.at(i);
  }
  int p_inv(int i) const
  {
    return pi_inv.at(i);
  }
  const vector<int> &Phi() const
  {
    return poly;
  }

  friend ostream &operator<<(ostream &s, const Ring &R);
  friend istream &operator>>(istream &s, Ring &R);

  bool operator!=(const Ring &other) const;
};

#endif
