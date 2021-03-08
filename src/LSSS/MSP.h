/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

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

template<class T>
class Schur_Matrices;

// Forward declaration as apparently this is needed for friends in templates
template<class T>
class MSP;
template<class T>
ostream &operator<<(ostream &s, const MSP<T> &M);
template<class T>
istream &operator>>(istream &s, MSP<T> &M);

template<class T>
class MSP
{
  /* Whilst we dont need the following for Full, we use the
   * following in fake_offline so we still define them for
   * all LSSS versions
   */
  vector<vector<T>> Gen;   // Generator matrix \sum_{i=0}^{n-1} ns[i] rows and k columns
  vector<unsigned int> ns; // Number of share values per player

  /* Gets full reconstruction vector (i.e. over all rows) for a qualified set */
  vector<T> get_full_reconstruct(const vector<int> &qual) const;

public:
  void Initialize_Full_Threshold(unsigned int n);
  // This next one will not work for gf2
  void Initialize_Shamir(unsigned int n, unsigned int t);

  void Initialize_Replicated(const CAS &AS);
  void Initialize(const vector<vector<T>> &A,
                  const vector<unsigned int> &shares_per_player)
  {
    Gen= A;
    ns= shares_per_player;
  }

  void assign(const MSP<T> &M);
  MSP(const MSP<T> &M)
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
  MSP(const vector<vector<T>> &A, const vector<unsigned int> &shares_per_player)
  {
    Initialize(A, shares_per_player);
  }

  MSP()
  {
    ;
  }
  MSP &operator=(const MSP<T> &M)
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
  T G(int i, int j) const
  {
    return Gen[i][j];
  }
  const vector<T> &G(int i) const
  {
    return Gen[i];
  }
  const vector<vector<T>> &get_G() const
  {
    return Gen;
  }

  friend ostream &operator<<<>(ostream &s, const MSP<T> &M);
  friend istream &operator>><>(istream &s, MSP<T> &M);

  // Make the Parity Check matrix for this MSP
  vector<vector<T>> Make_Parity() const;

  /* Given a set of rows of Gen in Mrows this creates the
   * reconstruction vector for the share s, and the
   * reconstruction matrix for the set of ALL shares vec{s}.
   * Assumes Mrows is full rank
   */
  void Make_Recon(vector<T> &ReconS, vector<vector<T>> &ReconSS,
                  const vector<int> &MRows) const;

  /* Create a random sharing of val */
  void Random_Sharing(vector<T> &ans, const T &val, PRNG &G) const;

  /* Generate a sharing from an input vector */
  void Sharing(vector<T> &ans, const vector<T> &kk) const
  {
    Mul(ans, Gen, kk);
  }

  /* Checks whether the list of players indicate by the binary vector
   * players is qualified or not
   */
  bool check_qualified(const vector<int> &players) const;

  /* Extracts matrix rows corresponding to indicator set players */
  vector<vector<T>> extract_rows(const vector<int> &players) const;

  /* Finds all unqualified sets */
  imatrix find_all_unqualified() const;

  /* This will return an equivalent multiplicative MSP, or return the
   * current MSP if the current MSP is multiplicative.
   * Will abort if *this is not Q2
   * Also returns the associated Schur matrices */
  MSP<T> make_multiplicative(Schur_Matrices<T> &Sch) const;
};

#endif
