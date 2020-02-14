/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _CAS
#define _CAS

/* Defines an arbitrary Complete Access Structure */

#include "Math/Matrix.h"

class CAS
{

  // Determines if each subset of parites is qualified
  // or unqualified and inserts them in gamma/delta
  void check_all_sets(int dim, vector<int> &v);

  // Puts maximal elements of delta into delta_plus
  void set_delta_plus();

  // Puts minimal elements of gamma into gamma_minus
  void set_gamma_minus();

  // Helper routine for checking Ql or not
  bool check_not_q_l(unsigned int hw, int dim, vector<int> &v) const;

  // Helper routine for generating threshold structure
  void generate_sets(unsigned int hw, int dim, vector<int> &v);

public:
  unsigned int n; // Number of parties

  // For Shamir initialisation only delta_plus is defined
  // If the size of this is too big we set it to zero
  // We only need delta_plus for the PRSS, which we can
  // do via a protocol if delta_plus is too big

  // For the following a_{i,j} is one if the ith set contains j
  imatrix gamma; // Qualified sets as a matrix
  imatrix delta; // Unqualified sets as a matrix

  imatrix gamma_minus; // Minimal elements of gamma
  imatrix delta_plus;  // Maximal elements of delta

  // Helper variables/routines
  bool spec_as_unqualified;

  void assign(const CAS &AS);

  CAS()
  {
    ;
  }
  CAS(const CAS &AS)
  {
    assign(AS);
  }
  CAS &operator=(const CAS &AS)
  {
    if (this != &AS)
      {
        assign(AS);
      }
    return *this;
  }
  // Following takes a matrix input as either qualified or unqualified
  void assign(const imatrix &Sets, bool unqualified);
  CAS(const imatrix &Sets, bool unqualified)
  {
    assign(Sets, unqualified);
  }
  // Following assigns a threshold structure
  void assign(unsigned int nn, unsigned int t);
  CAS(unsigned int nn, unsigned int t)
  {
    assign(nn, t);
  }

  // Checks whether the structure is Ql or not
  bool is_q(unsigned int l) const;

  friend ostream &operator<<(ostream &s, const CAS &AS);
  friend istream &operator>>(istream &s, CAS &AS);
};

#endif
