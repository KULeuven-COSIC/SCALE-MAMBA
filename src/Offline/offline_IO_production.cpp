/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "offline_IO_production.h"
#include "FakePrep.h"
#include "config.h"
#include "offline_FHE.h"
#include "offline_subroutines.h"

void make_IO_data_non_Full(Player &P, PRSS &prss, unsigned int player_num,
                           list<Share> &a, list<gfp> &opened,
                           Open_Protocol &OP)
{
  vector<Share> alist(sz_IO_batch);
  vector<gfp> openedlist(sz_IO_batch);
  /* First make the shares */
  for (int i= 0; i < sz_IO_batch; i++)
    {
      Share aa;
      aa= prss.next_share(P);
      a.push_back(aa);
      alist[i]= aa;
    }

  /* Now send the data if we need to send data
   * else receive the data
   */
  if (P.whoami() != player_num)
    {
      OP.Open_To_One_Begin(player_num, alist, P);
    }
  else
    {
      OP.Open_To_One_End(openedlist, alist, P);
      for (int i= 0; i < sz_IO_batch; i++)
        {
          opened.push_back(openedlist[i]);
        }
    }
}

/* Make sz_IO_batch input triples */
void make_IO_data(Player &P, int fake_sacrifice, PRSS &prss,
                  unsigned int player_num, list<Share> &a, list<gfp> &opened,
                  const FHE_PK &pk, const FHE_SK &sk, const FFT_Data &PTD,
                  offline_control_data &OCD, Open_Protocol &OP,
                  unsigned int online_thread,
                  FHE_Industry &industry)
{
  a.resize(0);
  opened.resize(0);

  if (fake_sacrifice)
    {
      make_IO_data_fake(P, player_num, a, opened);
    }
  else if (Share::SD.type == Full)
    {
      offline_FHE_IO(P, player_num, a, opened, pk, sk, PTD, OCD, online_thread, industry);
    }
  else
    {
      make_IO_data_non_Full(P, prss, player_num, a, opened, OP);
    }
}
