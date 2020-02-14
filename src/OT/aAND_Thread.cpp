/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "OT_Thread_Data.h"
#include "config.h"
#include <unistd.h>

#include <fstream>

extern OT_Thread_Data OTD;

// Decide what to make
// Again player 0 makes the decision
//    0 = Nothing
//    1 = aANDs queue 0 only
//    2 = aANDs queue 1 only
//    3 = aANDs queue 0 and 1 only
//    4 = aANDs queue 2 only
//    5 = aANDs queue 0 and 2 only
//    .... and so on
int make_aAND_Thread_decision(Player &P)
{
  int result= 0;
  string ss= "";
  if (P.whoami() == 0)
    { // Decide whether to pause (lists are too big)
      OTD.aAD.aAD_mutex.lock();
      for (unsigned int j= 0; j < OTD.aAD.aANDs.size(); j++)
        {
          if (OTD.aAD.aANDs[j].size() < max_aAND_queue)
            {
              result+= (1 << j);
              ss= ss + "A";
            }
          else
            {
              ss= ss + "N";
            }
        }
      OTD.aAD.aAD_mutex.unlock();
      P.send_all(ss, 2);
    }
  else
    {
      P.receive_from_player(0, ss, 2);
      for (unsigned int j= 0; j < OTD.aAD.aANDs.size(); j++)
        {
          if (ss.c_str()[j] == 'A')
            {
              result+= (1 << j);
            }
        }
    }

  return result;
}

void aAND_Thread(Player &P, unsigned int no_online_threads,
                 offline_control_data &OCD, int verbose)
{
  aAND aAF;

  while (0 == 0)
    { /* Decide whether to finish */
      if (check_exit(P, no_online_threads, OCD, true) == 1)
        {
          printf("Exiting aAND production thread\n");
          return;
        }
      /* Decide whether to pause (lists are too big) */
      int decide= make_aAND_Thread_decision(P);
      if (decide == 0)
        {
          sleep(1);
        }
      else
        {
          for (unsigned int j= 0; j < OTD.aAD.aANDs.size(); j++)
            {
              if ((decide & 1) == 1)
                {
                  // We make the aANDs here using the extra aBit queue
                  aAF.make_aANDs(P, no_online_threads);
                  OTD.aAD.aAD_mutex.lock();
                  aAF.copy_aANDs(OTD.aAD.aANDs[j]);
                  if (verbose)
                    {
                      printf("Size of aAND queue %d : %lu \n", j, OTD.aAD.aANDs[j].size());
                    }
                  OTD.aAD.aAD_mutex.unlock();
                }
              decide>>= 1;
            }
        }
    }
}

aTriple aAND_Data::get_aAND(unsigned int q)
{
  bool wait= true;
  while (wait)
    {
      wait= false;
      aAD_mutex.lock();
      if (aANDs[q].size() == 0)
        {
          wait= true;
          aAD_mutex.unlock();
          sleep(1);
        }
    }
  // At this point the mutex is locked
  aTriple ans= aANDs[q].front();
  aANDs[q].pop_front();

  aAD_mutex.unlock();

  return ans;
}

list<aTriple> aAND_Data::get_aANDs(unsigned int q, unsigned int num)
{
  if (num >= max_aAND_queue)
    {
      throw GC_Error("Too many aANDs being requested. Contact the developers");
    }
  bool wait= true;
  while (wait)
    {
      wait= false;
      aAD_mutex.lock();
      if (aANDs[q].size() < num)
        {
          wait= true;
          aAD_mutex.unlock();
          sleep(1);
        }
    }
  // At this point the mutex is locked
  list<aTriple> ans;
  ans.splice(ans.begin(), aANDs[q], aANDs[q].begin(), next(aANDs[q].begin(), num));
  aAD_mutex.unlock();

  return ans;
}
