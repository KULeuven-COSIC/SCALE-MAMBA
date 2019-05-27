/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2019, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "aBit_Thread.h"
#include "aBitFactory.h"
#include <unistd.h>

#include <fstream>

aBit_Data aBD;

// Checks exit, by player zero making decisions and conveying this
// to all others. This means players are in sync with player zero. Even
// if they know themselves they should die gracefully/wait
// Returns
//    0 = OK, prepare some more stuff
//    1 = Exit
int check_exit(Player &P, unsigned int no_online_threads, offline_control_data &OCD)
{
  int result= 1;
  string ss= "E";
  if (P.whoami() == 0)
    {
      for (unsigned int i= 0; i < no_online_threads; i++)
        {
          OCD.sacrifice_mutex[i].lock();
          if (OCD.finish_offline[i] != 1)
            {
              ss= "-";
              result= 0;
            }
          OCD.sacrifice_mutex[i].unlock();
        }
      P.send_all(ss, 2);
    }
  else
    {
      P.receive_from_player(0, ss, 2);
      if (ss.compare("-") == 0)
        {
          result= 0;
        }
    }
  return result;
}

// Decide what to make
// Again player 0 makes the decision
//    0 = Nothing
//    1 = aBits queue 0 only
//    2 = aBits queue 1 only
//    3 = aBits queue 0 and 1 only
//    4 = aBits queue 2 only
//    5 = aBits queue 0 and 2 only
//    .... and so on
int make_aBit_Thread_decision(Player &P)
{
  int result= 0;
  string ss= "";
  if (P.whoami() == 0)
    { // Decide whether to pause (lists are too big)
      aBD.aBD_mutex.lock();
      for (unsigned int j= 0; j < aBD.aBits.size(); j++)
        {
          if (aBD.aBits[j].size() < max_aBit_queue)
            {
              result+= (1 << j);
              ss= ss + "A";
            }
          else
            {
              ss= ss + "N";
            }
        }
      aBD.aBD_mutex.unlock();
      P.send_all(ss, 2);
    }
  else
    {
      P.receive_from_player(0, ss, 2);
      for (unsigned int j= 0; j < aBD.aBits.size(); j++)
        {
          if (ss.c_str()[j] == 'A')
            {
              result+= (1 << j);
            }
        }
    }

  return result;
}

void aBit_Thread(Player &P, unsigned int no_online_threads,
                 offline_control_data &OCD, int verbose)
{
  aBitFactory aBF;

  aBD.aBits.resize(no_online_threads);

  aBF.Initialize(P);

  aBD.aBD_mutex.lock();
  aBF.tune(P, aBD.aBits[0], verbose);
  aBD.aBD_mutex.unlock();

  while (0 == 0)
    { /* Decide whether to finish */
      if (check_exit(P, no_online_threads, OCD) == 1)
        {
          printf("Exiting aBit production thread\n");
          return;
        }

      /* Decide whether to pause (lists are too big) */
      int decide= make_aBit_Thread_decision(P);
      if (decide == 0)
        {
          sleep(1);
        }
      else
        {
          for (unsigned int j= 0; j < aBD.aBits.size(); j++)
            {
              if ((decide & 1) == 1)
                {
                  unsigned int m= aBF.make_aBits(P);
                  aBD.aBD_mutex.lock();
                  aBF.copy_aBits(aBD.aBits[j], m);
                  if (verbose)
                    {
                      printf("Size of aBit queue %d : %lu \n", j, aBD.aBits[j].size());
                    }
                  aBD.aBD_mutex.unlock();
                }
              decide>>= 1;
            }
        }
    }
}

aBit aBit_Data::get_aShare(unsigned int q)
{
  bool wait= true;
  while (wait)
    {
      wait= false;
      aBD_mutex.lock();
      if (aBits[q].size() == 0)
        {
          wait= true;
          aBD_mutex.unlock();
          sleep(1);
        }
    }
  // At this point the mutex is locked
  aBit ans= aBits[q].front();
  aBits[q].pop_front();

  aBD_mutex.unlock();

  return ans;
}

list<aBit> aBit_Data::get_aShares(unsigned int q, unsigned int num)
{
  if (num >= max_aBit_queue)
    {
      throw invalid_length();
    }
  bool wait= true;
  while (wait)
    {
      wait= false;
      aBD_mutex.lock();
      if (aBits[q].size() < num)
        {
          wait= true;
          aBD_mutex.unlock();
          sleep(1);
        }
    }
  // At this point the mutex is locked
  list<aBit> ans;
  ans.splice(ans.begin(), aBits[q], aBits[q].begin(), next(aBits[q].begin(), num));
  aBD_mutex.unlock();

  return ans;
}
