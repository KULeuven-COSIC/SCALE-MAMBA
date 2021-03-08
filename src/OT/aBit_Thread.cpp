/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "OT_Thread_Data.h"
#include "aBitFactory.h"
#include <unistd.h>

extern OT_Thread_Data OTD;

#include <fstream>

// Checks exit, by player zero making decisions and conveying this
// to all others. This means players are in sync with player zero. Even
// if they know themselves they should die gracefully/wait
// Returns
//    0 = OK, prepare some more stuff
//    1 = Exit
// Flag flag_aAND=I am in the aAND thread
//    The aBit thread must not die until the aAND thread
//    has died
int check_exit(Player &P, unsigned int no_online_threads, offline_control_data &OCD,
               bool flag_aAND)
{
  int result= 1;
  string ss= "E";
  if (P.whoami() == 0)
    {
      // Do not die if I am aBit and aAND not killed
      OTD.aBD.aBD_mutex.lock();
      if (OTD.aBD.kill == false && flag_aAND == false)
        {
          ss= "-";
          result= 0;
        }
      OTD.aBD.aBD_mutex.unlock();
      // Do not die if main offline threads still working
      for (unsigned int i= 0; i < no_online_threads; i++)
        {
          OCD.OCD_mutex[i].lock();
          if (OCD.finish_offline[i] != 1)
            {
              ss= "-";
              result= 0;
            }
          OCD.OCD_mutex[i].unlock();
        }
      // Do not die if an online thread is still working
      for (unsigned int i= 0; i < OCD.finish_offline.size(); i++)
        {
          OCD.OCD_mutex[i].lock();
          if (OCD.finished_online[i] == 0)
            {
              ss= "-";
              result= 0;
            }
          OCD.OCD_mutex[i].unlock();
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
  // Tell aBit thread if the aAND thread is dieing
  if (result == 1 && flag_aAND == true)
    {
      OTD.aBD.aBD_mutex.lock();
      OTD.aBD.kill= true;
      OTD.aBD.aBD_mutex.unlock();
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
      OTD.aBD.aBD_mutex.lock();
      for (unsigned int j= 0; j < OTD.aBD.aBits.size(); j++)
        {
          if (OTD.aBD.aBits[j].size() < max_aBit_queue)
            {
              result+= (1 << j);
              ss= ss + "A";
            }
          else
            {
              ss= ss + "N";
            }
        }
      OTD.aBD.aBD_mutex.unlock();
      P.send_all(ss, 2);
    }
  else
    {
      P.receive_from_player(0, ss, 2);
      for (unsigned int j= 0; j < OTD.aBD.aBits.size(); j++)
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

  aBF.Initialize(P);

  // Pack the last queue first, as it is used for aANDs
  OTD.aBD.aBD_mutex.lock();
  OTD.ready= true;
  aBF.tune(P, OTD.aBD.aBits[no_online_threads], verbose);
  if (verbose)
    {
      printf("Size of aBit queue %d : %lu \n", no_online_threads, OTD.aBD.aBits[no_online_threads].size());
    }
  OTD.aBD.aBD_mutex.unlock();

  while (0 == 0)
    { /* Decide whether to finish */
      if (check_exit(P, no_online_threads, OCD, false) == 1)
        {
          printf("Exiting aBit production thread\n");
          return;
        }

      /* Decide whether to pause (lists are too big) */
      int decide= make_aBit_Thread_decision(P);
      if (decide == 0)
        {
          nanosleep(&time_s, NULL);
        }
      else
        {
          for (unsigned int j= 0; j < OTD.aBD.aBits.size(); j++)
            {
              if ((decide & 1) == 1)
                {
                  unsigned int m= aBF.make_aBits(P);
                  OTD.aBD.aBD_mutex.lock();
                  aBF.copy_aBits(OTD.aBD.aBits[j], m);
                  if (verbose)
                    {
                      printf("Size of aBit queue %d : %lu \n", j, OTD.aBD.aBits[j].size());
                    }
                  OTD.aBD.aBD_mutex.unlock();
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
          nanosleep(&time_s, NULL);
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
      throw GC_Error("Too many aBits being requested. Contact the developers");
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
          nanosleep(&time_s, NULL);
        }
    }
  // At this point the mutex is locked
  list<aBit> ans;
  ans.splice(ans.begin(), aBits[q], aBits[q].begin(), next(aBits[q].begin(), num));
  aBD_mutex.unlock();

  return ans;
}
