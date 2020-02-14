/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _ShareData
#define _ShareData

/* Class for holding data which defines the LSSS
 *   - Gen is assumed to be the generator matrix
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
#include "LSSS/MSP.h"
#include "LSSS/Schur_Matrices.h"
#include "Math/Matrix.h"

enum ShareType {
  Full,
  Shamir,
  Replicated,
  Q2MSP,
  Other
};

// The following is only needed when Type=Replicated
// (otherwise we have always Maurer (or SPDZ for Full))
enum OfflineType {
  Fake,
  Maurer,
  Reduced,
  SPDZ
};

class ShareData
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
  ShareType type;    // Sharing type
  OfflineType Otype; // Offline type

  MSP M;

  // Number of MAC values (when needed assume stat parameter is macs_stat_sec)
  unsigned int nmacs;
  unsigned int threshold; // When Shamir

  // The following are only defined if type!=Full

  /* The Parity Check Matrix
   *        Actually we store a generalized inverse of Gen
   *        Top rows are how to recover the k random values
   *        from all shares
   *        Bottom rows are the Parity check rows
   */
  gfp_matrix Parity;

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
  vector<vector<gfp>> ReconS;

  /* Matrix i corresponds to how to reconstruct the total set
   * of share values for player i
   */
  vector<gfp_matrix> ReconSS;

  /* n by n matrix OpenC[i][j]=1 if exist k such that
   *       RCt[i][j][k]=1
   * Gives channels needed for opening
   */
  imatrix OpenC;

  Schur_Matrices Schur;

  vector<vector<gfp>> share_of_one; /* A sharing of the value one for other LSSS
                                     This cannot be a vector of Share's as SD
                                     in Share might not be defined yet. So we
                                     use a vector of vectors
                                   */

  CAS AS;

  // The following only defined if Otype = Reduced

  /* For each player i mult_proc[i][j] defines where share j should be
   *      0 Produced using PRSS in the mult protocol
   *     -1 Produced using PRSS in the mult protocol and subtract
   *      1 Assigned the share value from the schur+PRZS and sent
   *        or Maurer
   *      2 Received from another player
   *  (sum_j (mult_proc[i]=1)) = 1 for all i
   */
  vector<vector<int>> mult_proc;

  /* For each i,j for which mult_proc[i][j]=1 or 2
   * this defines the channels for sending/receiving
   * If mult_proc[i][j]=1 and this list is empty then
   * player i uses Maurer method for his share
   * If mult_proc[i][j]=2 then this only contains one entry
   */
  vector<vector<vector<int>>> mult_chans;

  /* Which channels can all players expect Maurer shares from*/
  vector<int> maurer_chans;

  // Now the member functions

  void assign(const ShareData &SD);
  ShareData(const ShareData &SD)
  {
    assign(SD);
  }
  ShareData()
  {
    ;
  }
  ShareData &operator=(const ShareData &SD)
  {
    if (&SD != this)
      {
        assign(SD);
      }
    return *this;
  }
  ~ShareData()
  {
    ;
  }

  void Initialize_Full_Threshold(unsigned int n);
  void Initialize_Shamir(unsigned int n, unsigned int t);
  void Initialize_Replicated(const CAS &AccStr, OfflineType offline_type);
  // Initializes via MSP M, if M not multiplicative then it extends it
  void Initialize_Q2(const MSP &MM);

  friend ostream &operator<<(ostream &s, const ShareData &SD);
  friend istream &operator>>(istream &s, ShareData &SD);
};

#endif
