/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Mod2_Thread.h"
#include <unistd.h>

#include "LSSS/Open_Protocol2.h"
#include "Mod2Maurer.h"
#include "Tools/Timer.h"

#include <fstream>

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
  return result;
}

extern Mod2_Thread_Data MTD;

/* Entering here assumes the thread is locked */
void Mod2_Thread_Data::Tune(Player &P, PRSS2 &prss, int verbose)
{
  B= 3;
  C= 64;
  // Require (B-1)*log_2 N > SP2017_stat_sec
  N= 1UL << (SP2017_stat_sec / (B - 1));
  vector<vector<Share2>> Gtriples;

  Timer T;
  unsigned int best_L= -1;
  double bestv= 0;
  // L is mult of 64
  for (unsigned int L= 64; L <= 2048; L*= 2)
    {
      X= N / L + C;
      T.reset();
      T.start();
      Gen_Checked_Triples(P, prss, Gtriples, B, C, X, L);
      T.stop();
      double persec= (Gtriples.size() * 64) / T.elapsed();

      triples[0].insert(triples[0].end(), Gtriples.begin(), Gtriples.end());

      if (verbose)
        {
          cout << "Batch Size = (" << N << "," << L << ")" << endl;
          cout << "\tPer Second = " << persec << endl;
        }
      if (persec > bestv)
        {
          bestv= persec;
          best_L= L;
        }
    }
  if (verbose)
    {
      cout << "Best performance when L = " << best_L << endl;
      cout << "\tPer Second = " << bestv << endl;
      cout << "Finished Tuning: Generated " << (triples[0].size() * 64) << " triples" << endl;
    }
  // Player 0 makes the choice as to which L
  if (P.whoami() == 0)
    {
      stringstream ss;
      ss << best_L;
      P.send_all(ss.str(), 2);
      L= best_L;
    }
  else
    {
      string ss;
      P.receive_from_player(0, ss, 2);
      istringstream iss(ss);
      iss >> L;
    }
  X= N / L + C;

  if (verbose)
    {
      cout << "Chosen L = " << L << endl;
    }
}

void Mod2_Thread_Data::Update_Queue(unsigned int q, Player &P, PRSS2 &prss)
{
  static vector<vector<Share2>> Gtriples;
  Gen_Checked_Triples(P, prss, Gtriples, B, C, X, L);

  MD2_mutex.lock();
  triples[q].insert(triples[q].end(), Gtriples.begin(), Gtriples.end());
  MD2_mutex.unlock();
}

// Decide what to make
// Again player 0 makes the decision
//    0 = Nothing
//    1 = Queue 0 only
//    2 = Queue 1 only
//    3 = Queue 0 and 1 only
//    4 = Queue 2 only
//    5 = Queue 0 and 2 only
//    .... and so on
int make_Mod2_Thread_decision(Player &P)
{
  int result= 0;
  string ss= "";
  if (P.whoami() == 0)
    { // Decide whether to pause (lists are too big)
      MTD.MD2_mutex.lock();
      for (unsigned int j= 0; j < MTD.triples.size(); j++)
        {
          if (MTD.triples[j].size() < max_Mod2_Triple_queue)
            {
              result+= (1 << j);
              ss= ss + "A";
            }
          else
            {
              ss= ss + "N";
            }
        }
      MTD.MD2_mutex.unlock();
      P.send_all(ss, 0);
    }
  else
    {
      P.receive_from_player(0, ss, 0);
      for (unsigned int j= 0; j < MTD.triples.size(); j++)
        {
          if (ss.c_str()[j] == 'A')
            {
              result+= (1 << j);
            }
        }
    }

  return result;
}

void Mod2_Triple_Thread(Player &P, unsigned int no_online_threads,
                        offline_control_data &OCD, int verbose)
{
  PRSS2 prss(P);

  MTD.MD2_mutex.lock();
  MTD.Tune(P, prss, verbose);
  MTD.MD2_mutex.unlock();

  while (0 == 0)
    { /* Decide whether to finish */
      if (check_exit(P, no_online_threads, OCD) == 1)
        {
          printf("Exiting Mod2 Triple production thread\n");
          return;
        }

      /* Decide whether to pause (lists are too big) */
      int decide= make_Mod2_Thread_decision(P);
      if (decide == 0)
        {
          nanosleep(&time_s, NULL);
        }
      else
        {
          for (unsigned int j= 0; j < MTD.triples.size(); j++)
            {
              if ((decide & 1) == 1)
                {
                  MTD.Update_Queue(j, P, prss);
                  if (verbose)
                    {
                      printf("Size of Triple queue %d : %lu \n", j, MTD.triples[j].size());
                    }
                }
              decide>>= 1;
            }
        }
    }
}

void Mod2_Thread_Data::get_Triple(vector<Share2> &T, unsigned int q)
{
  bool wait= true;
  while (wait)
    {
      wait= false;
      MD2_mutex.lock();
      if (triples[q].size() == 0)
        {
          wait= true;
          MD2_mutex.unlock();
          nanosleep(&time_s, NULL);
        }
    }
  // At this point the mutex is locked
  T= triples[q].front();
  triples[q].pop_front();

  MD2_mutex.unlock();
}

list<vector<Share2>> Mod2_Thread_Data::get_Triples(unsigned int q, unsigned int num)
{
  if (num >= max_Mod2_Triple_queue)
    {
      throw Mod2Engine_Error("Too many Mod2 triples being requested. Contact the developers");
    }
  bool wait= true;
  while (wait)
    {
      wait= false;
      MD2_mutex.lock();
      if (triples[q].size() < num)
        {
          wait= true;
          MD2_mutex.unlock();
          //cout << "Waiting to produce " << num << " Mod2Engine Triples" << endl;
          nanosleep(&time_s, NULL);
        }
    }
  // At this point the mutex is locked
  list<vector<Share2>> ans;
  ans.splice(ans.begin(), triples[q], triples[q].begin(), next(triples[q].begin(), num));
  MD2_mutex.unlock();

  return ans;
}

void Mod2_Thread_Data::get_Single_Triple(vector<Share2> &T, unsigned int q)
{
  // Get a new triple if we have run out of the temporary one
  if (T_cnt[q] == 64)
    {
      get_Triple(T_Store[q], q);
      T_cnt[q]= 0;
    }
  T.resize(3);
  for (unsigned int i= 0; i < 3; i++)
    {
      T[i].mul(T_Store[q][i], 1ULL);
      T_Store[q][i].SHR(T_Store[q][i], 1);
    }
  T_cnt[q]++;
}

void Mod2_Thread_Data::get_Single_Triples(vector<vector<Share2>> &T, unsigned int q)
{
  unsigned int sz= T.size();
  unsigned int sz_big= T.size() / 64;

  // First do the full word ones
  list<vector<Share2>> lst= get_Triples(q, sz_big);
  int cnt= 0;
  for (unsigned int i= 0; i < sz_big; i++)
    {
      vector<Share2> v= lst.front();
      lst.pop_front();
      for (unsigned int j= 0; j < 64; j++)
        {
          for (unsigned int k= 0; k < 3; k++)
            {
              T[cnt][k].mul(v[k], 1ULL);
              v[k].SHR(v[k], 1);
            }
          cnt++;
        }
    }

  for (unsigned int i= cnt; i < sz; i++)
    {
      get_Single_Triple(T[i], q);
    }
}
