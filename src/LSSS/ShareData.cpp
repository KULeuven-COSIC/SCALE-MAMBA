/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "ShareData.h"
#include "config.h"

void ShareData::assign(const ShareData &SD)
{
  type= SD.type;
  Otype= SD.Otype;
  M= SD.M;
  nmacs= SD.nmacs;
  threshold= SD.threshold;

  if (type == Full)
    {
      RCt= SD.RCt;
      ReconS= SD.ReconS;
    }
  else
    {
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
      if (Otype == Reduced)
        {
          mult_proc= SD.mult_proc;
          mult_chans= SD.mult_chans;
          maurer_chans= SD.maurer_chans;
        }
    }
}

void ShareData::Initialize_Full_Threshold(unsigned int nn)
{
  type= Full;
  Otype= SPDZ;
  M.Initialize_Full_Threshold(nn);
  nmacs= macs_stat_sec / numBits(gfp::pr()) + 1;

  unsigned int n= M.nplayers();
  RCt.resize(n, vector<vector<int>>(n));
  ReconS.resize(n, vector<gfp>(n));
  for (unsigned int i= 0; i < n; i++)
    {
      for (unsigned int j= 0; j < n; j++)
        {
          RCt[i][j].resize(1);
          RCt[i][j][0]= 1;
          ReconS[i][j].assign_one();
        }
    }
}

/* This finds ReconS, ReconSS, Parity and OpenC given RCt
 * and the MSP matrix
 */
void ShareData::Initialize_Sub()
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
          share_of_one[i][j]= M.G(c, 0);
          c++;
        }
    }
}

void ShareData::Initialize_Shamir(unsigned int nn, unsigned int t)
{
  type= Shamir;
  Otype= Maurer;
  threshold= t;
  nmacs= 0;
  M.Initialize_Shamir(nn, t);
  AS.assign(nn, t);
  Schur.initialize(M);

  unsigned int n= M.nplayers();

  RCt.resize(n, vector<vector<int>>(n));
  for (unsigned int i= 0; i < n; i++)
    {
      for (unsigned int j= 0; j < n; j++)
        {
          RCt[i][j].resize(1);
        }
      for (unsigned int j= 1; j < t + 1; j++)
        {
          RCt[i][(j + i) % n][0]= 1;
        }
      RCt[i][i][0]= 1; // Send own share to oneself
    }

  Initialize_Sub();
}

/* Replicated SS Helper Routines */

bool set_already_assigned(vector<int> indexes, unsigned int i)
{
  for (unsigned int j= 0; j < i; j++)
    {
      if (indexes[i] == indexes[j] && i != j)
        {
          return true;
        }
    }
  return false;
}

unsigned int number_sets_assigned(const CAS &AS, vector<int> indexes)
{
  vector<bool> sets_assigned(AS.delta_plus.size());
  int m= AS.delta_plus.size();

  for (unsigned int j= 0; j < AS.n; j++)
    {
      if (indexes[j] > -1 && indexes[j] < m)
        {
          sets_assigned[indexes[j]]= 1;
        }
    }

  int n_assigned= 0;
  for (unsigned int j= 0; j < AS.delta_plus.size(); j++)
    {
      if (sets_assigned[j])
        {
          n_assigned++;
        }
    }
  return n_assigned;
}

bool find_one_assignment(const CAS &AS, vector<int> &indices,
                         unsigned int i, vector<int> &poss_maps)
{
  /* -2 means party not assigned a set (yet)
     -1 means null */

  if (number_sets_assigned(AS, indices) == AS.delta_plus.size())
    {
      for (unsigned int j= 0; j < AS.n; j++)
        {
          poss_maps[j]= indices[j];
        }
      return true;
    }
  else if (i >= AS.n)
    {
      if (number_sets_assigned(AS, indices) >= number_sets_assigned(AS, poss_maps))
        {
          for (unsigned int j= 0; j < AS.n; j++)
            {
              poss_maps[j]= indices[j];
            }
        }
    }
  else
    {
      int next_non_null= AS.n;
      int previous_non_null= -1;
      int m= AS.delta_plus.size();
      int int_n= AS.n;

      if (i < AS.n - 1)
        {
          next_non_null= i + 1;
          while (indices[next_non_null] == -1 && next_non_null < int_n)
            {
              next_non_null++;
            }
        }

      if (i > 0)
        {
          previous_non_null= i - 1;
          while (indices[previous_non_null] == -1 && previous_non_null > -1)
            {
              previous_non_null--;
            }
        }

      bool found_set= false;

      if (indices[i] == -2)
        {
          indices[i]= 0;
        }

      while (indices[i] < m && !found_set)
        {
          if (AS.delta_plus[indices[i]][i] == 0 &&
              !set_already_assigned(indices, i))
            {
              found_set= true;
            }
          else
            {
              indices[i]++;
            }
        }

      if (found_set)
        {
          find_one_assignment(AS, indices, next_non_null, poss_maps);
        }
      else if (i > 0)
        {
          vector<int> indices_to_recurse(AS.n);
          for (unsigned int j= 0; j < AS.n; j++)
            {
              indices_to_recurse[j]= indices[j];
            }
          indices_to_recurse[i]= -1;
          if (!find_one_assignment(AS, indices_to_recurse, next_non_null, poss_maps))
            {
              for (unsigned int j= 0; j < AS.n; j++)
                {
                  indices_to_recurse[j]= indices[j];
                }
              indices_to_recurse[i]= -2;
              if (previous_non_null > -1)
                {
                  indices_to_recurse[previous_non_null]++;
                  find_one_assignment(AS, indices_to_recurse, previous_non_null, poss_maps);
                }
              else
                {
                  indices_to_recurse[i]= -1;
                  find_one_assignment(AS, indices_to_recurse, next_non_null, poss_maps);
                }
            }
        }
      else
        {
          indices[i]= -1;
          find_one_assignment(AS, indices, next_non_null, poss_maps);
        }
    }
  return false;
}

bool vec_is_zero(vector<unsigned int> v)
{
  for (unsigned int i= 0; i < v.size(); i++)
    {
      if (v[i] != 0)
        {
          return false;
        }
    }
  return true;
}

void set_sets_to_parties(const CAS &AS, vector<int> &poss_maps,
                         vector<int> &sets_to_parties)
{
  int int_n= AS.n;
  int m= AS.delta_plus.size();

  for (unsigned int i= 0; i < AS.n; i++)
    {
      poss_maps[i]= -2;
    }

  find_one_assignment(AS, poss_maps, 0, poss_maps);

  for (unsigned int i= 0; i < AS.delta_plus.size(); i++)
    {
      sets_to_parties[i]= -2;
    }

  // Assign all remaining
  for (int i= 0; i < int_n; i++)
    {
      if (poss_maps[i] > -1 && poss_maps[i] < m)
        {
          sets_to_parties[poss_maps[i]]= i;
        }
    }

  for (unsigned int j= 0; j < AS.delta_plus.size(); j++)
    {
      if (sets_to_parties[j] == -2)
        {
          int i= 0;
          while (AS.delta_plus[j][i] != 0 && i < int_n - 1)
            {
              i++;
            }
          if (i == int_n - 1 && AS.delta_plus[j][i] != 0)
            {
              sets_to_parties[j]= -1;
            }
          else
            {
              sets_to_parties[j]= i;
            }
        }
    }
}

void ShareData::Initialize_Replicated(const CAS &AccStr,
                                      OfflineType offline_type)
{
  type= Replicated;
  Otype= offline_type;
  M.Initialize_Replicated(AccStr);
  unsigned int m= M.row_dim();
  AS= AccStr;
  nmacs= 0;
  threshold= 0;

  if (!AS.is_q(2))
    {
      throw not_Q2();
    }

  int k= 0;
  unsigned int szk= AS.delta_plus.size();
  vector<int> Gen_to_delta_plus(m); // Map of row of Gen to set in Delta_plus
  vector<vector<int>> Gen_to_player_share(
      m, vector<int>(2)); // Map of row of Gen to player/share number
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
  string new_string= "";
  for (unsigned int j= 0; j < AS.delta_plus.size(); j++)
    {
      new_string+= to_string(sets_to_parties[j]) + " ";
    }
  cout << "Map of sets to parties: " << new_string << endl;

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
  Schur.initialize(M);

  /* Now deal with the data for the Reduced Mult Protocol */
  if (Otype == Reduced)
    {
      mult_proc.resize(M.nplayers());
      mult_chans.resize(M.nplayers());
      maurer_chans.resize(0);
      for (unsigned int i= 0; i < M.nplayers(); i++)
        {
          mult_proc[i].resize(M.shares_per_player(i));
          mult_chans[i].resize(M.shares_per_player(i));
          for (unsigned int j= 0; j < M.shares_per_player(i); j++)
            {
              mult_chans[i][j].resize(M.shares_per_player(i));
              for (unsigned int k= 0; k < M.shares_per_player(i); k++)
                {
                  mult_chans[i][j].resize(0);
                  mult_proc[i][j]= 0;
                }
            }
        }
      unsigned int c= 0;
      for (unsigned int i= 0; i < M.nplayers(); i++)
        {
          bool used= false;
          for (unsigned int j= 0; j < M.shares_per_player(i); j++)
            {
              int b_set= -1;
              for (unsigned int k= 0; k < M.col_dim(); k++)
                {
                  if (M.G(c + j, k).is_one())
                    {
                      b_set= k;
                    }
                }
              if (b_set == -1)
                {
                  throw bad_value();
                }
              unsigned int p= sets_to_parties[b_set];
              if (p == i)
                {
                  if (used == false)
                    {
                      used= true;
                      mult_proc[i][j]= 1;
                      for (unsigned int k= 0; k < M.row_dim(); k++)
                        {
                          if (k != (c + j) && M.G(k, b_set).is_one())
                            {
                              int pl= Gen_to_player_share[k][0];
                              int sh= Gen_to_player_share[k][1];
                              mult_chans[i][j].push_back(pl);
                              mult_proc[pl][sh]= 2;
                              mult_chans[pl][sh].push_back(i);
                            }
                        }
                    }
                  else
                    { // This is the share we need to delete in the PRSS
                      mult_proc[i][j]= -1;
                    }
                }
            }
          c+= M.shares_per_player(i);
          // Deal with case when we need to use Maurer for this player
          if (used == false)
            {
              mult_proc[i][0]= 1;
              mult_chans[i][0].resize(0);
              maurer_chans.push_back(i);
            }
        }
    }
}

void ShareData::Initialize_Q2(const MSP &MM)
{
  type= Q2MSP;
  Otype= Maurer;

  M= MM.make_multiplicative(Schur);
  nmacs= 0;
  threshold= 0;

  imatrix Unqual= M.find_all_unqualified();
  AS.assign(Unqual, true);

  /* Work out RCt data */
  RCt.resize(AS.n, vector<vector<int>>(AS.n));
  vector<int> ns(AS.n);
  for (unsigned int i= 0; i < AS.n; i++)
    {
      ns[i]= M.shares_per_player(i);
      for (unsigned int j= 0; j < AS.n; j++)
        {
          RCt[i][j].resize(ns[i]);
          for (int k= 0; k < ns[i]; k++)
            {
              RCt[i][j][k]= 0;
            }
        }
    }
  for (unsigned int j= 0; j < AS.n;
       j++)
    { // Who does player j need to receive data from?
      // Find a minimally qualified set containing j with smaller hwt
      // and the smallest number of things to combine
      int i0= -1, hwt= 1000, wt= 1000;
      for (unsigned int i= 0; i < AS.gamma_minus.size(); i++)
        {
          if (AS.gamma_minus[i][j] == 1)
            {
              int h= Hwt(AS.gamma_minus[i]);
              int w= dot_product(AS.gamma_minus[i], ns);
              if (h == hwt && w < wt)
                {
                  wt= w;
                  i0= i;
                }
              else if (h < hwt)
                {
                  hwt= h;
                  wt= w;
                  i0= i;
                }
            }
        }
      // Progressively add rows to Test, only add into RCt when
      // this gives us an increase in rank
      // First add player j's into Test
      gfp_matrix Test(ns[j], vector<gfp>(M.col_dim()));
      int c= 0;
      for (unsigned int i= 0; i < j; i++)
        {
          c+= ns[i];
        }
      for (int k= 0; k < ns[j]; k++)
        {
          for (unsigned int t= 0; t < M.col_dim(); t++)
            {
              Test[k][t]= M.G(c + k, t);
            }
          RCt[j][j][k]= 1;
        }
      // Now add in the other rows, testing as we go
      c= 0;
      int nr= ns[j];
      for (unsigned int i= 0; i < AS.n && Test.size() != M.col_dim(); i++)
        {
          if (i != j && AS.gamma_minus[i0][i] == 1)
            {
              for (int k= 0; k < ns[i] && Test.size() != M.col_dim();
                   k++)
                { /* Test whether adding this row to the matrix gives us
                             anything */
                  gfp_matrix nTest= Test;
                  nTest.resize(nr + 1, vector<gfp>(M.col_dim()));
                  for (unsigned int t= 0; t < M.col_dim(); t++)
                    {
                      nTest[nr][t]= M.G(c, t);
                    }
                  c++;
                  Gauss_Elim(nTest, nr + 1);
                  if (!is_zero(nTest[nr]))
                    {
                      Test= nTest;
                      nr++;
                      RCt[i][j][k]= 1;
                    }
                }
            }
          else
            {
              c+= ns[i];
            }
        }
      if (Test.size() !=
          M.col_dim())
        { // We need to add data from some other players
          c= 0;
          for (unsigned int i= 0; i < AS.n && Test.size() != M.col_dim(); i++)
            {
              if (i != j && AS.gamma_minus[i0][i] == 0)
                {
                  for (int k= 0; k < ns[i] && Test.size() != M.col_dim();
                       k++)
                    { /* Test whether adding this row to the matrix gives us
                         anything */
                      gfp_matrix nTest= Test;
                      nTest.resize(nr + 1, vector<gfp>(M.col_dim()));
                      for (unsigned int t= 0; t < M.col_dim(); t++)
                        {
                          nTest[nr][t]= M.G(c, t);
                        }
                      c++;
                      Gauss_Elim(nTest, nr + 1);
                      if (!is_zero(nTest[nr]))
                        {
                          Test= nTest;
                          nr++;
                          RCt[i][j][k]= 1;
                        }
                    }
                }
              else
                {
                  c+= ns[i];
                }
            }
        }
    }

  // Now sort out ReconS, ReconSS, Parity and OpenC
  Initialize_Sub();
}

ostream &operator<<(ostream &s, const ShareData &SD)
{
  s << (int) SD.type << endl;
  s << (int) SD.Otype << endl;
  s << SD.M;
  s << SD.nmacs << " " << SD.threshold << endl;
  if (SD.type != Full)
    {
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
              SD.share_of_one[i][j].output(s, true);
              s << " ";
            }
          s << endl;
        }

      s << SD.AS << endl;

      if (SD.Otype == Reduced)
        {
          for (unsigned int i= 0; i < SD.M.nplayers(); i++)
            {
              for (unsigned int j= 0; j < SD.M.shares_per_player(i); j++)
                {
                  s << SD.mult_proc[i][j] << " ";
                  if (SD.mult_proc[i][j] != 0)
                    {
                      s << SD.mult_chans[i][j].size() << " ";
                      for (unsigned int k= 0; k < SD.mult_chans[i][j].size(); k++)
                        {
                          s << SD.mult_chans[i][j][k] << " ";
                        }
                      s << endl;
                    }
                }
            }
          s << SD.maurer_chans.size() << endl;
          for (unsigned int i= 0; i < SD.maurer_chans.size(); i++)
            {
              s << SD.maurer_chans[i] << " ";
            }
          s << endl;
        }
    }

  return s;
}

istream &operator>>(istream &s, ShareData &SD)
{
  int l;
  s >> l;
  SD.type= (ShareType) l;
  s >> l;
  SD.Otype= (OfflineType) l;
  s >> SD.M;
  s >> SD.nmacs >> SD.threshold;
  if (SD.type != Full)
    {
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
              SD.share_of_one[i][j].input(s, true);
            }
        }

      s >> SD.AS;

      if (SD.Otype == Reduced)
        {
          SD.mult_proc.resize(SD.M.nplayers());
          SD.mult_chans.resize(SD.M.nplayers());
          for (unsigned int i= 0; i < SD.M.nplayers(); i++)
            {
              SD.mult_proc[i].resize(SD.M.shares_per_player(i));
              SD.mult_chans[i].resize(SD.M.shares_per_player(i));
              for (unsigned int j= 0; j < SD.M.shares_per_player(i); j++)
                {
                  s >> SD.mult_proc[i][j];
                  if (SD.mult_proc[i][j] != 0)
                    {
                      s >> l;
                      SD.mult_chans[i][j].resize(l);
                      for (int k= 0; k < l; k++)
                        {
                          s >> SD.mult_chans[i][j][k];
                        }
                    }
                }
            }
          s >> l;
          SD.maurer_chans.resize(l);
          for (int i= 0; i < l; i++)
            {
              s >> SD.maurer_chans[i];
            }
        }
    }
  else
    {
      unsigned int n= SD.M.nplayers();
      SD.RCt.resize(n, vector<vector<int>>(n));
      SD.ReconS.resize(n, vector<gfp>(n));
      for (unsigned int i= 0; i < n; i++)
        {
          for (unsigned int j= 0; j < n; j++)
            {
              SD.RCt[i][j].resize(1);
              SD.RCt[i][j][0]= 1;
              SD.ReconS[i][j].assign_one();
            }
        }
    }
  return s;
}
