/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _MSP
#define _MSP

/* This class defines the MSP (Monotone Span Program)
 * underlying the LSSS we use.
 *
 *  - Gen is assumed to be the generator matrix
 *         s = G.k
 *   - Secret shared is (1,1,...,1).k
 *   - Thus for full threshold G is the identity matrix
 *     and for Shamir it is NOT the Vandemonde matrix
 *
 *   - For Full a sharing of one is (1,0,...,0) for
 *     Shamir it is (1,1,...,1), for Replicated/Other
 *     it is s = G*(1,0,...0)
 */

#include "LSSS/CAS.h"
#include "LSSS/Schur_Matrices.h"
#include "Math/Matrix.h"

class Schur_Matrices;

class MSP
{
  /* Whilst we dont need the following for Full, we use the
   * following in fake_offline so we still define them for
   * all LSSS versions
   */
  gfp_matrix Gen;          // Generator matrix \sum_{i=0}^{n-1} ns[i] rows and k columns
  vector<unsigned int> ns; // Number of share values per player

  /* Gets full reconstruction vector (i.e. over all rows) for a qualified set */
  vector<gfp> get_full_reconstruct(const vector<int> &qual) const;

public:
  void Initialize_Full_Threshold(unsigned int n);
  void Initialize_Shamir(unsigned int n, unsigned int t);
  void Initialize_Replicated(const CAS &AS);
  void Initialize(const gfp_matrix &A,
                  const vector<unsigned int> &shares_per_player)
  {
    Gen= A;
    ns= shares_per_player;
  }

  void assign(const MSP &M);
  MSP(const MSP &M)
  {
    assign(M);
  }
  MSP(unsigned int n)
  {
    Initialize_Full_Threshold(n);
  }
  MSP(unsigned int n, unsigned int t)
  {
    Initialize_Shamir(n, t);
  }
  MSP(const CAS &AS)
  {
    Initialize_Replicated(AS);
  }
  MSP(const gfp_matrix &A, const vector<unsigned int> &shares_per_player)
  {
    Initialize(A, shares_per_player);
  }

  MSP()
  {
    ;
  }
  MSP &operator=(const MSP &M)
  {
    if (&M != this)
      {
        assign(M);
      }
    return *this;
  }
  ~MSP()
  {
    ;
  }

  unsigned int nplayers() const
  {
    return ns.size();
  }
  unsigned int shares_per_player(int i) const
  {
    return ns[i];
  }
  unsigned int col_dim() const
  {
    return Gen[0].size();
  }
  unsigned int row_dim() const
  {
    return Gen.size();
  }
  gfp G(int i, int j) const
  {
    return Gen[i][j];
  }
  const vector<gfp> &G(int i) const
  {
    return Gen[i];
  }

  friend ostream &operator<<(ostream &s, const MSP &M);
  friend istream &operator>>(istream &s, MSP &M);

  // Make the Parity Check matrix for this MSP
  gfp_matrix Make_Parity() const;

  /* Given a set of rows of Gen in Mrows this creates the
   * reconstruction vector for the share s, and the
   * reconstruction matrix for the set of ALL shares vec{s}.
   * Assumes Mrows is full rank
   */
  void Make_Recon(vector<gfp> &ReconS, gfp_matrix &ReconSS,
                  const vector<int> &MRows) const;

  /* Create a random sharing of val */
  vector<gfp> Random_Sharing(const gfp &val, PRNG &G) const;

  /* Generate a sharing from an input vector */
  vector<gfp> Sharing(const vector<gfp> &kk) const
  {
    return Mul(Gen, kk);
  }

  /* Checks whether the list of players indicate by the binary vector
   * players is qualified or not
   */
  bool check_qualified(const vector<int> &players) const;

  /* Extracts matrix rows corresponding to indicator set players */
  gfp_matrix extract_rows(const vector<int> &players) const;

  /* Finds all unqualified sets */
  imatrix find_all_unqualified() const;

  /* This will return an equivalent multiplicative MSP, or return the
   * current MSP if the current MSP is multiplicative.
   * Will abort if *this is not Q2
   * Also returns the associated Schur matrices */
  MSP make_multiplicative(Schur_Matrices &Sch) const;
};

#endif
