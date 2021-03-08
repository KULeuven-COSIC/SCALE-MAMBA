/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "ShareData2.h"
#include "config.h"

// Helper routines defined in the other ShareData.cpp file...
void set_sets_to_parties(const CAS &AS, vector<int> &poss_maps,
                         vector<int> &sets_to_parties);

void ShareData2::assign(const ShareData2 &SD)
{
  M= SD.M;
  type= SD.type;

  Parity= SD.Parity;
  RCt= SD.RCt;
  ReconS.resize(M.nplayers());
  ReconSS.resize(M.nplayers());
  for (unsigned int i= 0; i < M.nplayers(); i++)
    {
      ReconS[i]= SD.ReconS[i];
      ReconSS[i]= SD.ReconSS[i];
    }
  Schur= SD.Schur;
  OpenC= SD.OpenC;
  share_of_one= SD.share_of_one;
  AS= SD.AS;
}

/* This finds ReconS, ReconSS, Parity and OpenC given RCt
 * and the MSP matrix
 *
 * Returns true if OK, false if an error
 */
void ShareData2::Initialize_Sub()
{
  ReconS.resize(M.nplayers());
  ReconSS.resize(M.nplayers());

  /* For player i the vector Mrows holds 0 if that row of Gen
   * is not received and 1 if it is in the opening protocol
   */
  vector<int> MRows(M.row_dim());

  OpenC.resize(M.nplayers(), vector<int>(M.nplayers()));
  for (unsigned int i= 0; i < M.nplayers(); i++)
    {
      for (unsigned int j= 0; j < M.nplayers(); j++)
        {
          OpenC[i][j]= 0;
        }
    }

  for (unsigned int i= 0; i < M.nplayers(); i++)
    {
      for (unsigned int j= 0; j < M.row_dim(); j++)
        {
          MRows[j]= 0;
        }
      int c= 0;
      for (unsigned int j= 0; j < M.nplayers(); j++)
        {
          for (unsigned int k= 0; k < M.shares_per_player(j); k++)
            {
              if (RCt[j][i][k] == 1)
                {
                  MRows[c + k]= 1;
                  OpenC[j][i]= 1;
                }
            }
          c+= M.shares_per_player(j);
        }
      M.Make_Recon(ReconS[i], ReconSS[i], MRows);
    }

  Parity= M.Make_Parity();

  share_of_one.resize(M.nplayers());
  int c= 0;
  for (unsigned int i= 0; i < M.nplayers(); i++)
    {
      share_of_one[i].resize(M.shares_per_player(i));
      for (unsigned int j= 0; j < M.shares_per_player(i); j++)
        {
          share_of_one[i][j]= M.G(c, 0).get();
          c++;
        }
    }
}

void ShareData2::Initialize_Replicated(const CAS &AccStr)
{
  type= Replicated;
  M.Initialize_Replicated(AccStr);
  unsigned int m= M.row_dim();
  AS= AccStr;

  if (!AS.is_q(2))
    {
      throw not_Q2();
    }

  int k= 0;
  unsigned int szk= AS.delta_plus.size();
  // Map of row of Gen to set in Delta_plus
  vector<int> Gen_to_delta_plus(m);
  // Map of row of Gen to player/share number
  vector<vector<int>> Gen_to_player_share(m, vector<int>(2));
  for (unsigned int i= 0; i < AS.n; i++)
    {
      unsigned int nst= 0;
      for (unsigned int j= 0; j < szk; j++)
        {
          if (AS.delta_plus[j][i] == 0)
            {
              Gen_to_delta_plus[k]= j;
              Gen_to_player_share[k][0]= i;
              Gen_to_player_share[k][1]= nst;
              nst++;
              k++;
            }
        }
    }

  // Create RCt data
  vector<int> poss_maps(AS.n); // Injective maps from parties to sets
  // Map of set to a party
  vector<int> sets_to_parties(AS.delta_plus.size());

  set_sets_to_parties(AS, poss_maps, sets_to_parties);

  int int_n= AS.n;

  RCt.resize(AS.n, vector<vector<int>>(AS.n));
  int c= 0;
  for (int i= 0; i < int_n; i++)
    {
      for (unsigned int j= 0; j < AS.n; j++)
        {
          RCt[i][j].resize(M.shares_per_player(i));
          for (unsigned int k= 0; k < M.shares_per_player(i); k++)
            {
              RCt[i][j][k]= 0;
            }
        }
      for (unsigned int k= 0; k < M.shares_per_player(i); k++)
        {
          int delta_row= Gen_to_delta_plus[k + c];
          if (sets_to_parties[delta_row] == i)
            {
              for (unsigned int j= 0; j < AS.n; j++)
                {
                  if (AS.delta_plus[delta_row][j] == 1)
                    {
                      RCt[i][j][k]= 1;
                    }
                }
            }
          RCt[i][i][k]= 1; // Always send own shares to oneself
        }
      c+= M.shares_per_player(i);
    }

  Initialize_Sub();

  /* Now construct the local Schur matrices */
  bool flag= Schur.initialize(M);
  if (!flag)
    {
      throw bad_value();
    }
}

ostream &operator<<(ostream &s, const ShareData2 &SD)
{
  s << SD.M;
  s << SD.Parity << endl;
  for (unsigned int i= 0; i < SD.M.nplayers(); i++)
    {
      for (unsigned int j= 0; j < SD.M.nplayers(); j++)
        {
          for (unsigned int k= 0; k < SD.M.shares_per_player(i); k++)
            {
              s << SD.RCt[i][j][k] << " ";
            }
        }
      s << endl;
    }
  for (unsigned int i= 0; i < SD.M.nplayers(); i++)
    {
      s << SD.ReconS[i].size() << endl;
      for (unsigned int j= 0; j < SD.ReconS[i].size(); j++)
        {
          s << SD.ReconS[i][j] << " ";
        }
      s << endl;
    }
  for (unsigned int i= 0; i < SD.M.nplayers(); i++)
    {
      s << SD.ReconSS[i] << endl;
    }
  s << SD.OpenC << endl;
  s << SD.Schur << endl;

  for (unsigned int i= 0; i < SD.share_of_one.size(); i++)
    {
      for (unsigned int j= 0; j < SD.M.shares_per_player(i); j++)
        {
          s << SD.share_of_one[i][j] << " ";
        }
      s << endl;
    }

  s << SD.AS << endl;

  return s;
}

istream &operator>>(istream &s, ShareData2 &SD)
{
  SD.type= Replicated;
  int l;
  s >> SD.M;
  s >> SD.Parity;
  SD.RCt.resize(SD.M.nplayers());
  SD.ReconS.resize(SD.M.nplayers());
  SD.ReconSS.resize(SD.M.nplayers());
  for (unsigned int i= 0; i < SD.M.nplayers(); i++)
    {
      SD.RCt[i].resize(SD.M.nplayers());
      for (unsigned int j= 0; j < SD.M.nplayers(); j++)
        {
          SD.RCt[i][j].resize(SD.M.shares_per_player(i));
          for (unsigned int k= 0; k < SD.M.shares_per_player(i); k++)
            {
              s >> SD.RCt[i][j][k];
            }
        }
    }
  for (unsigned int i= 0; i < SD.M.nplayers(); i++)
    {
      s >> l;
      SD.ReconS[i].resize(l);
      for (int j= 0; j < l; j++)
        {
          s >> SD.ReconS[i][j];
        }
    }
  for (unsigned int i= 0; i < SD.M.nplayers(); i++)
    {
      s >> SD.ReconSS[i];
    }

  s >> SD.OpenC;
  s >> SD.Schur;

  SD.share_of_one.resize(SD.M.nplayers());
  for (unsigned int i= 0; i < SD.M.nplayers(); i++)
    {
      SD.share_of_one[i].resize(SD.M.shares_per_player(i));
      for (unsigned int j= 0; j < SD.M.shares_per_player(i); j++)
        {
          s >> SD.share_of_one[i][j];
        }
    }

  s >> SD.AS;

  return s;
}

void ShareData2::Random_Sharing(vector<word> &ans, const word &val, PRNG &G) const
{
  unsigned int k= M.col_dim();
  vector<word> kk(k);
  kk[0]= val;
  for (unsigned int i= 1; i < k; i++)
    {
      kk[i]= G.get_word();
      kk[0]^= kk[i];
    }
  Mul(ans, M.get_G(), kk);
}
