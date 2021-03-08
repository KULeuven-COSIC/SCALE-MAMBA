/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _ShareData2
#define _ShareData2

/* Class for holding data which defines the LSSS mod 2
 *   - Gen is assumed to be the generator matrix
 *         s = G.k
 *   - Secret shared is (1,1,...,1).k
 *   - A sharing of one is one = G*(1,0,...0)
 */

#include "LSSS/CAS.h"
#include "LSSS/MSP.h"
#include "LSSS/Schur_Matrices.h"
#include "LSSS/ShareData.h"
#include "Math/Matrix.h"

class ShareData2
{

  /* Given Gen and RCt this creates
   *    ReconS
   *    ReconSS
   *    Parity
   *    OpenC
   *    share_of_one
   */
  void Initialize_Sub();

public:
  ShareType type= Other; // Means constructor not called

  MSP<gf2> M;

  /* The Parity Check Matrix
   *        Actually we store a generalized inverse of Gen
   *        Top rows are how to recover the k random values
   *        from all shares
   *        Bottom rows are the Parity check rows
   */
  vector<vector<gf2>> Parity;

  /* Channels for low cost (non-robust) reconstruction of a share.
   *    Player i sends his k'th share to party j if
   *                RCt[i][j][k]=1
   *    Includes sending to themselves
   */
  vector<vector<vector<int>>> RCt;

  /* Reconstruction vectors.
   * Entry i corresponds to how player i reconstructs s from
   * the input values from players described in RCt[*][j][*]
   */
  vector<vector<gf2>> ReconS;

  /* Matrix i corresponds to how to reconstruct the total set
   * of share values for player i
   */
  vector<vector<vector<gf2>>> ReconSS;

  /* n by n matrix OpenC[i][j]=1 if exist k such that
   *       RCt[i][j][k]=1
   * Gives channels needed for opening
   */
  imatrix OpenC;

  Schur_Matrices<gf2> Schur;

  vector<vector<word>> share_of_one; /* A sharing of the value one for other LSSS
                                        This cannot be a vector of Share's as SD
                                        in Share might not be defined yet. So we
                                        use a vector of vectors
			  	        Needs to be zeros or ones, of type word
					as this is how we are going to use it
                                     */

  CAS AS;

  // Now the member functions

  void assign(const ShareData2 &SD);
  ShareData2(const ShareData2 &SD)
  {
    type= Replicated;
    assign(SD);
  }
  ShareData2()
  {
    ;
  }
  ShareData2 &operator=(const ShareData2 &SD)
  {
    if (&SD != this)
      {
        assign(SD);
      }
    return *this;
  }
  ~ShareData2()
  {
    ;
  }

  void Initialize_Replicated(const CAS &AccStr);

  friend ostream &operator<<(ostream &s, const ShareData2 &SD);
  friend istream &operator>>(istream &s, ShareData2 &SD);

  // Like RandomSharing in MSP.h, but on 64 bits at a time
  void Random_Sharing(vector<word> &ans, const word &val, PRNG &G) const;
};

#endif
