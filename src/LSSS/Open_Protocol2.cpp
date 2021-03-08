/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include <iomanip>
#include <iostream>
using namespace std;

#include "Math/Matrix.h"
#include "Open_Protocol2.h"
#include "Tools/Crypto.h"
#include "config.h"

Open_Protocol2::Open_Protocol2()
{
  sha256.resize(3);
  for (unsigned int i= 0; i < 3; i++)
    {
      SHA256_Init(&sha256[i]);
      // Counts the number opens in each connection, so we do a check every so often
      open_cnt[i]= 0;
      // This is a string of stuff which we will eventually hash
      hss[i]= "";
    }
}

Open_Protocol2::~Open_Protocol2()
{
  for (int i= 0; i < 3; i++)
    {
      if (open_cnt[i] > 0)
        {
          cerr << "Problem cleaning up Open_Protocol object.\n";
          cerr << "Data is still hanging around for connection " << i << endl;
          abort();
        }
    }
}

void Open_Protocol2::Open_Bit(word &dv, const aBit &v, Player &P)
{
  vector<string> o(P.nplayers());
  // No need to resize buff as we know it is big enough for one bit
  for (unsigned int i= 0; i < P.nplayers(); i++)
    {
      if (i != P.whoami())
        {
          buff.get_buffer()[0]= (uint8_t) v.get_value();
          v.get_MAC(i).output(buff.get_buffer() + 1);
          o[i]= string((char *) buff.get_buffer(), 1 + gf2n::size());
        }
    }

  P.Send_Distinct_And_Receive(o, 2);

  dv= v.get_value();

  gf2n M;
  for (unsigned int i= 0; i < P.nplayers(); i++)
    {
      if (i != P.whoami())
        {
          char c= o[i].c_str()[0];
          M.input(o[i], 1);
          dv= dv ^ ((unsigned int) c);
          if (c == 1)
            {
              M.add(aBit::get_Delta());
            }
          if (M != v.get_Key(i))
            {
              throw OT_error();
            }
        }
    }
}

void Open_Protocol2::Open_Bit(word &dv, const Share2 &v, Player &P, int connection, int num)
{
  if (num != 8 && num != 1)
    {
      throw not_implemented();
    }
  unsigned long max_len= num * Share2::SD.M.row_dim();

  buff.resize(max_len);
  uint8_t v8;

  for (unsigned int p= 0; p < P.nplayers(); p++)
    {
      if (Share2::SD.OpenC[P.whoami()][p] == 1 && p != P.whoami())
        {
          unsigned int length= 0;
          for (unsigned int k= 0; k < v.a.size(); k++)
            {
              if (Share2::SD.RCt[P.whoami()][p][k] == 1)
                {
                  if (num == 8)
                    {
                      memcpy(buff.get_buffer() + length, &v.a[k], sizeof(word));
                      length+= sizeof(word);
                    }
                  else
                    {
                      v8= (uint8_t) v.a[k];
                      memcpy(buff.get_buffer() + length, &v8, 1);
                      length+= 1;
                    }
                }
            }
          P.send_to_player(p, buff.get_buffer(), length, connection);
        }
    }

  unsigned int p= P.whoami(), len;
  const uint8_t *P_sbuff= 0;
  vector<word> vs(Share2::SD.ReconS[p].size(), -1);
  int c= 0;
  for (unsigned int j= 0; j < P.nplayers(); j++)
    {
      if (Share2::SD.OpenC[j][p] == 1 && j != p)
        {
          P_sbuff= P.receive_from_player(j, len, connection, false);
        }

      unsigned int length= 0;
      for (unsigned int k= 0; k < Share2::SD.M.shares_per_player(j); k++)
        {
          if (j == p)
            {
              vs[c]= v.a[k];
              c++;
            }
          else if (Share2::SD.RCt[j][p][k] == 1)
            {
              if (num == 8)
                {
                  vs[c]= *(word *) (P_sbuff + length);
                  length+= sizeof(word);
                }
              else
                {
                  vs[c]= *(char *) (P_sbuff + length);
                  length+= 1;
                }
              c++;
            }
        }
    }

  dv= dot_product(vs, Share2::SD.ReconS[p]);
  vector<word> ssv(Share2::SD.ReconSS[p].size());
  Mul(ssv, Share2::SD.ReconSS[p], vs);

  // Prepare to hash into the state
  unsigned int length= 0;
  if (num == 8)
    {
      for (unsigned int j= 0; j < ssv.size(); j++)
        {
          memcpy(buff.get_buffer() + length, &ssv[j], sizeof(word));
          length+= sizeof(word);
        }
    }
  else
    {
      for (unsigned int j= 0; j < ssv.size(); j++)
        {
          v8= (uint8_t) ssv[j];
          memcpy(buff.get_buffer() + length, &v8, 1);
          length+= 1;
        }
    }
  hss[connection]+= string((char *) buff.get_buffer(), length);

  // Hash into the state
  if (hss[connection].size() > 10000)
    {
      SHA256_Update(&sha256[connection], hss[connection].c_str(), hss[connection].size());
      hss[connection]= "";
    }

  open_cnt[connection]+= 1;
  if (open_cnt[connection] > open_check_gap)
    {
      RunOpenCheck(P, "", connection);
    }
}

void Open_Protocol2::Open_BitVector(word &dv, const aBitVector &v, Player &P)
{
  vector<word> vec(sreg_bitl);
  Open_Bits(vec, v.internal_rep(), P);
  dv= 0;
  for (int i= sreg_bitl - 1; i >= 0; i--)
    {
      dv<<= 1;
      dv+= vec[i];
    }
}

void Open_Protocol2::Open_To_All(vector<word> &values,
                                 const vector<Share2> &S, Player &P,
                                 int connection)
{
  values.resize(S.size()); // Get rid of tiresome segfault errors
  unsigned long max_len= S.size() * sizeof(word) * Share2::SD.M.row_dim();

  buff.resize(max_len);

  for (unsigned int p= 0; p < P.nplayers(); p++)
    {
      if (Share2::SD.OpenC[P.whoami()][p] == 1 && p != P.whoami())
        {
          unsigned int length= 0;
          for (unsigned int k= 0; k < S[0].a.size(); k++)
            {
              for (unsigned int i= 0; i < S.size(); i++)
                {
                  if (Share2::SD.RCt[P.whoami()][p][k] == 1)
                    {
                      memcpy(buff.get_buffer() + length, &S[i].a[k], sizeof(word));
                      length+= sizeof(word);
                    }
                }
            }
          P.send_to_player(p, buff.get_buffer(), length, connection);
        }
    }

  unsigned int p= P.whoami(), len;
  const uint8_t *P_sbuff= 0;
  vector<vector<word>> vs(Share2::SD.ReconS[p].size(), vector<word>(S.size()));
  int c= 0;
  for (unsigned int j= 0; j < P.nplayers(); j++)
    {
      if (Share2::SD.OpenC[j][p] == 1 && j != p)
        {
          P_sbuff= P.receive_from_player(j, len, connection);
        }

      unsigned int length= 0;
      for (unsigned int k= 0; k < Share2::SD.M.shares_per_player(j); k++)
        {
          if (j == p)
            {
              for (unsigned int i= 0; i < S.size(); i++)
                {
                  vs[c][i]= S[i].a[k];
                }
              c++;
            }
          else if (Share2::SD.RCt[j][p][k] == 1)
            {
              for (unsigned int i= 0; i < S.size(); i++)
                {
                  vs[c][i]= *(word *) (P_sbuff + length);
                  length+= sizeof(word);
                }
              c++;
            }
        }
    }

  dot_product(values, Share2::SD.ReconS[p], vs);
  vector<vector<word>> ssv(Share2::SD.ReconSS[p].size(), vector<word>(vs[0].size()));
  Mul(ssv, Share2::SD.ReconSS[p], vs);

  // Prepare to hash into the state
  unsigned int length= 0;
  for (unsigned int i= 0; i < S.size(); i++)
    {
      for (unsigned int j= 0; j < ssv.size(); j++)
        {
          memcpy(buff.get_buffer() + length, &ssv[j][i], sizeof(word));
          length+= sizeof(word);
        }
    }
  hss[connection]+= string((char *) buff.get_buffer(), length);

  // Hash into the state
  if (hss[connection].size() > 10000)
    {
      SHA256_Update(&sha256[connection], hss[connection].c_str(), hss[connection].size());
      hss[connection]= "";
    }

  open_cnt[connection]+= values.size();
  if (open_cnt[connection] > open_check_gap)
    {
      RunOpenCheck(P, "", connection);
    }
}

/* We run it on the connection c as other connection might have
 * some opening still to do
 */
void Open_Protocol2::RunOpenCheck(Player &P, const string &aux, int connection, bool verbose)
{
  // Do a final hash if stuff left over
  if (hss[connection].size() > 0)
    {
      SHA256_Update(&sha256[connection], hss[connection].c_str(), hss[connection].size());
      hss[connection]= "";
    }

  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256_Final(hash, &sha256[connection]);
  string ss((char *) hash, SHA256_DIGEST_LENGTH);
  if (verbose)
    {
      printf("Open Check: Sending  : %d : ", P.whoami());
      for (unsigned int i= 0; i < SHA256_DIGEST_LENGTH; i++)
        {
          printf("%02x", (uint8_t) ss.c_str()[i]);
        }
      printf("\n");
      printf("%lu %d\n", ss.size(), SHA256_DIGEST_LENGTH);
    }
  P.send_all(ss, connection, verbose);
  for (unsigned int i= 0; i < P.nplayers(); i++)
    {
      if (i != P.whoami())
        {
          string is;
          P.receive_from_player(i, is, connection, verbose);
          if (verbose)
            {
              printf("Open Check: Received : %d : ", i);
              for (unsigned int i= 0; i < SHA256_DIGEST_LENGTH; i++)
                {
                  printf("%02x", (uint8_t) is.c_str()[i]);
                }
              printf("\n");
              printf("%lu %d\n", is.size(), SHA256_DIGEST_LENGTH);
            }
          if (is != ss)
            {
              throw hash_fail();
            }
        }
    }
  SHA256_Init(&sha256[connection]);

  // Now deal with the auxillary string
  if (aux.size() != 0)
    {
      if (verbose)
        {
          printf("Auxillary Check: Sending  : %d : ", P.whoami());
          for (unsigned int i= 0; i < aux.size(); i++)
            {
              printf("%02x", (uint8_t) aux.c_str()[i]);
            }
          printf("\n");
        }
      P.send_all(aux, connection, verbose);
      for (unsigned int i= 0; i < P.nplayers(); i++)
        {
          if (i != P.whoami())
            {
              string is;
              P.receive_from_player(i, is, connection, verbose);
              if (verbose)
                {
                  printf("Auxillary Check: Received : %d : ", i);
                  for (unsigned int i= 0; i < is.size(); i++)
                    {
                      printf("%02x", (uint8_t) is.c_str()[i]);
                    }
                  printf("\n");
                }
              if (is != aux)
                {
                  throw inconsistent_inputs();
                }
            }
        }
    }
  open_cnt[connection]= 0;
}

void Open_Protocol2::Open_Bits_To(vector<word> &values, unsigned int j, const vector<aBit> &v,
                                  Player &P)
{
  unsigned long max_len= v.size() * (1 + gf2n::size());

  buff.resize(max_len);

  if (j != P.whoami())
    {
      unsigned int pos= 0;
      for (unsigned int t= 0; t < v.size(); t++)
        {
          buff.get_buffer()[pos]= (uint8_t) v[t].get_value();
          pos++;
          pos+= v[t].get_MAC(j).output(buff.get_buffer() + pos);
        }
      P.send_to_player(j, buff.get_buffer(), pos, 2);
    }
  else
    {
      values.resize(v.size());
      for (unsigned int t= 0; t < v.size(); t++)
        {
          values[t]= v[t].get_value();
        }

      gf2n M;
      for (unsigned int i= 0; i < P.nplayers(); i++)
        {
          if (i != P.whoami())
            {
              string ss;
              unsigned int len;
              const uint8_t *P_buff= P.receive_from_player(i, len, 2);
              unsigned int pos= 0;
              for (unsigned int t= 0; t < v.size(); t++)
                {
                  uint8_t c= P_buff[pos];
                  pos++;
                  pos+= M.input(P_buff + pos);
                  values[t]= values[t] ^ ((unsigned int) c);
                  if (c == 1)
                    {
                      M.add(aBit::get_Delta());
                    }
                  if (M != v[t].get_Key(i))
                    {
                      throw OT_error();
                    }
                }
            }
        }
    }
}

void Open_Protocol2::Open_Bits_To(vector<word> &values, unsigned int j, const vector<Share2> &v,
                                  Player &P)
{
  unsigned long max_len= v.size() * Share2::SD.M.row_dim() * sizeof(word);

  buff.resize(max_len);

  unsigned int p= P.whoami();
  if (j != p)
    {
      unsigned int length= 0;
      for (unsigned int k= 0; k < Share2::SD.M.shares_per_player(P.whoami()); k++)
        {
          for (unsigned int i= 0; i < v.size(); i++)
            {
              memcpy(buff.get_buffer() + length, &v[i].a[k], sizeof(word));
              length+= sizeof(word);
            }
        }
      P.send_to_player(j, buff.get_buffer(), length, 2);
    }
  else
    {
      const uint8_t *P_sbuff= 0;
      unsigned int length;
      vector<vector<word>> shares(Share2::SD.Parity[0].size(), vector<word>(v.size()));
      int c= 0;
      for (unsigned int j= 0; j < P.nplayers(); j++)
        {
          if (j != p)
            {
              P_sbuff= P.receive_from_player(j, length, 2);
            }
          length= 0;
          for (unsigned int k= 0; k < Share2::SD.M.shares_per_player(j); k++)
            {
              if (j == p)
                {
                  for (unsigned int i= 0; i < v.size(); i++)
                    {
                      shares[c][i]= v[i].a[k];
                    }
                }
              else
                {
                  for (unsigned int i= 0; i < v.size(); i++)
                    {
                      memcpy(&shares[c][i], P_sbuff + length, sizeof(word));
                      length+= sizeof(word);
                    }
                }
              c++;
            }
        }

      // We now have the matrix of shares. So check parity and reconstruct the secret
      values.resize(v.size());
      vector<vector<word>> check(Share2::SD.Parity.size(), vector<word>(v.size()));
      Mul(check, Share2::SD.Parity, shares);
      for (unsigned int i= 0; i < v.size(); i++)
        {
          for (unsigned int j= Share2::SD.M.col_dim(); j < check.size(); j++)
            {
              if (check[j][i] != 0)
                {
                  throw receiving_error();
                }
            }
          values[i]= 0;
          for (unsigned int j= 0; j < Share2::SD.M.col_dim(); j++)
            {
              values[i]^= check[j][i];
            }
        }
    }
}

void Open_Protocol2::Open_Bits(vector<word> &dv, const vector<aBit> &v, Player &P)
{
  dv.resize(v.size());
  unsigned long max_len= (1 + gf2n::size()) * v.size();

  buff.resize(max_len);

  vector<string> o(P.nplayers());
  for (unsigned int i= 0; i < P.nplayers(); i++)
    {
      if (i != P.whoami())
        {
          unsigned int pos= 0;
          for (unsigned int t= 0; t < v.size(); t++)
            {
              buff.get_buffer()[pos]= (uint8_t) v[t].get_value();
              pos++;
              pos+= v[t].get_MAC(i).output(buff.get_buffer() + pos);
            }
          o[i]= string((char *) buff.get_buffer(), pos);
        }
    }

  P.Send_Distinct_And_Receive(o, 2);

  for (unsigned int t= 0; t < v.size(); t++)
    {
      dv[t]= v[t].get_value();
    }

  gf2n M;
  for (unsigned int i= 0; i < P.nplayers(); i++)
    {
      if (i != P.whoami())
        {
          unsigned int pos= 0;
          for (unsigned int t= 0; t < v.size(); t++)
            {
              char c= o[i].c_str()[pos];
              pos++;
              pos+= M.input(o[i], pos);
              dv[t]= dv[t] ^ ((word) c);
              if (c == 1)
                {
                  M.add(aBit::get_Delta());
                }
              if (M != v[t].get_Key(i))
                {
                  throw OT_error();
                }
            }
        }
    }
}
