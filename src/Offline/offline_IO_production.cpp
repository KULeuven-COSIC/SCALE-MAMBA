/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "offline_IO_production.h"
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

void make_IO_data_fake(Player &P, unsigned int player_num, list<Share> &a,
                       list<gfp> &opened)
{
  gfp aa;
  if (P.whoami() == 0)
    {
      PRNG PRG;
      unsigned char Seed[SEED_SIZE];
      for (int i= 0; i < SEED_SIZE; i++)
        {
          Seed[i]= 0;
        }
      PRG.SetSeedFromRandom(Seed);

      init_fake();
      int n= P.nplayers();
      vector<gfp> amacs(Share::SD.nmacs);
      vector<Share> sa(n);
      for (int i= 0; i < sz_IO_batch; i++)
        {
          aa.randomize(PRG);
          make_fake_macs(amacs, aa);
          make_shares(sa, aa, amacs, PRG);

          /* Now send the data to all parties and fix own shares */
          a.push_back(sa[0]);
          if (player_num == 0)
            {
              opened.push_back(aa);
            }

          for (unsigned int j= 1; j < P.nplayers(); j++)
            {
              stringstream ss;
              sa[j].output(ss, false);
              if (j == player_num)
                {
                  aa.output(ss, false);
                }
              P.send_to_player(j, ss.str());
            }
        }
    }
  else
    {
      for (int i= 0; i < sz_IO_batch; i++)
        {
          string ss;
          P.receive_from_player(0, ss);
          istringstream is(ss);
          Share s(P.whoami());
          s.input(is, false);
          a.push_back(s);
          if (P.whoami() == player_num)
            {
              aa.input(is, false);
              opened.push_back(aa);
            }
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

  if (Share::SD.type == Full && !fake_sacrifice)
    {
      offline_FHE_IO(P, player_num, a, opened, pk, sk, PTD, OCD, online_thread, industry);
    }
  else if (fake_sacrifice)
    {
      make_IO_data_fake(P, player_num, a, opened);
    }
  else
    {
      make_IO_data_non_Full(P, prss, player_num, a, opened, OP);
    }
}
