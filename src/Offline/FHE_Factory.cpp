/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "FHE_Factory.h"
#include "FHE/ZKPoK.h"
#include "LSSS/PRSS.h"
#include "Tools/Timer.h"

#include <list>
#include <unistd.h>
using namespace std;

extern Timer global_time;

FHE_Industry::FHE_Industry(unsigned int maxnumber)
{
  Factory_List_Lock= new mutex[maxnumber];
  Current_Factory_Lock= new mutex[maxnumber];
  Factory.resize(maxnumber);
  batch_number.resize(maxnumber);
  Current_Factory.resize(maxnumber);
  zkc.resize(maxnumber);
  for (unsigned int i= 0; i < maxnumber; i++)
    {
      batch_number[i]= -1;
      zkc[i]= 0;
      // Locking current factories for i<>0 so we dont write to
      // them before the mac ctx's are produced
      if (i != 0)
        {
          Current_Factory_Lock[i].lock();
        }
    }
}

FHE_Industry::~FHE_Industry()
{
  delete[] Factory_List_Lock;
  delete[] Current_Factory_Lock;
}

// Checks exit, by player zero making decisions and conveying this
// to all others. This means players are in sync with player zero. Even
// if they know themselves they should die gracefully
// Returns
//    0 = OK, prepare some more stuff
//    1 = Exit
int check_exit(unsigned int num_online, offline_control_data &OCD)
{
  int result= 0;
  //printf("Waiting for sacrifice lock %d A\n",num_online); fflush(stdout);
  OCD.sacrifice_mutex[num_online].lock();
  //printf("Got sacrifice lock %d A\n",num_online); fflush(stdout);
  if (OCD.finish_offline[num_online] == 1)
    {
      result= 1;
    }
  OCD.sacrifice_mutex[num_online].unlock();
  //printf("Released sacrifice lock %d A\n",num_online); fflush(stdout);
  return result;
}

// Assume Current_Factory_Lock[num] is locked!!!
//   Returns -1 if we should exit, else 0
//   Checks whether we should close all the factories
int FHE_Industry::Update_If_Empty(unsigned int num, unsigned int num_online, offline_control_data &OCD)
{
  if (Current_Factory[num].isempty())
    {
      bool wait= true;
      while (wait)
        {
          if (check_exit(num_online, OCD))
            {
              printf("Trying to exit on thread %d\n", num_online);
              return -1;
            }
          wait= false;
          Factory_List_Lock[num].lock();
          if (Factory[num].size() == 0)
            {
              wait= true;
            }
          Factory_List_Lock[num].unlock();
          if (wait)
            {
              // Wait for ages as ZKPoK takes ages
              sleep(10);
            }
        }
      Factory_List_Lock[num].lock();
      batch_number[num]++;
      Current_Factory[num]= Factory[num].front();
      Factory[num].pop_front();
      Factory_List_Lock[num].unlock();
    }
  return 0;
}

/* Returns the number for checking purposes 
 *  -1 if should exit
 */
int FHE_Industry::Next_Off_Production_Line(Plaintext &mess, Ciphertext &ctx,
                                           const Player &P, unsigned int num_online,
                                           offline_control_data &OCD)
{
  // Player zero decides which list, batch and which number to
  // take. Broadcasts it.
  // The other players wait for this list/batch/number to be
  // ready and then use it
  int batch, i, num;
  // If player zero broadcast factor list, the batch_number and the item to use
  if (P.whoami() == 0)
    {
      /* Here we lock Current_Factory num and do not release until
       * we want to use the data in it, or we decide we are going
       * to use another Current_Factory. We spin around the 
       * factories to try to get one which is currently onlocked
       */
      num= -1;
      for (unsigned int j= 0; j < Factory.size(); j++)
        {
          if (Current_Factory_Lock[j].try_lock())
            {
              if (!Current_Factory[j].isempty())
                {
                  num= j;
                  break;
                }
              else
                {
                  Current_Factory_Lock[j].unlock();
                }
            }
        }
      /* So if num>=0 we have a lock on num */
      i= 0;
      if (num < 0)
        {
          /* All are empty at this point so pick a new one from the 
           * current biggest list, otherwise pick one modulo 
           * the size of the factory list (as one if it exists
           * is more likely to exist on the first iteration
           */
          num= 1 % Factory.size();
          unsigned int sz= 0;
          for (unsigned int j= 0; j < Factory.size(); j++)
            {
              Factory_List_Lock[j].lock();
              if (Factory[j].size() > sz)
                {
                  num= j;
                  sz= Factory[j].size();
                }
              Factory_List_Lock[j].unlock();
            }
          Current_Factory_Lock[num].lock();
          i= Update_If_Empty(num, num_online, OCD);
        }
      // Now Current_Factory_Lock[num] is locked and non-empty
      batch= batch_number[num];
      if (i == 0)
        {
          i= Current_Factory[num].get_next_unused();
          if (i < 0)
            {
              throw bad_value();
            }
          Current_Factory[num].get_entry(mess, ctx, (unsigned int) i);
        }
      Current_Factory_Lock[num].unlock();
      stringstream ss;
      ss << num << " " << batch << " " << i;
      //printf("Telling players to access item : %s\n",ss.str().c_str());
      P.send_all(ss.str());
    }

  // It not player zero wait for batch_number and item number
  // from player one
  //   - Is batch_number equal to my current one, if not wait
  //     calling Update_If_Empty() to make sure we update the
  //     current factory
  if (P.whoami() != 0)
    {
      string ss;
      P.receive_from_player(0, ss);
      // printf("Been told to access item : %s\n",ss.c_str());
      istringstream is(ss);
      is >> num >> batch >> i;
      if (i >= 0)
        {
          bool wait= true;
          while (wait)
            {
              wait= false;
              Factory_List_Lock[num].lock();
              if (batch != batch_number[num])
                {
                  wait= true;
                }
              Factory_List_Lock[num].unlock();
              if (wait)
                {
                  sleep(1);
                  Current_Factory_Lock[num].lock();
                  Update_If_Empty(num, num_online, OCD);
                  Current_Factory_Lock[num].unlock();
                }
            }
          Current_Factory_Lock[num].lock();
          Current_Factory[num].get_entry(mess, ctx, (unsigned int) i);
          Current_Factory_Lock[num].unlock();
        }
    }

  return i;
}

/* If one party says every thread is finished then finish */
bool FHE_Industry::is_finished(unsigned int num, Player &P, const offline_control_data &OCD)
{
  bool finished= true, wait= false;
  int wt= 60;
  for (unsigned int i= 0; i < OCD.finished_offline.size() && finished; i++)
    {
      //printf("Waiting for sacrifice lock %d %d B\n",num,i); fflush(stdout);
      OCD.sacrifice_mutex[i].lock();
      //printf("Got sacrifice lock %d %d B\n",num,i); fflush(stdout);
      if (OCD.finished_offline[i] < 4)
        {
          finished= false;
        }
      OCD.sacrifice_mutex[i].unlock();
      //printf("Released sacrifice lock %d %d B\n",num,i); fflush(stdout);
    }
  // We want to sleep if the factory list is getting too big
  //printf("Waiting for FL lock %d C\n",num); fflush(stdout);
  Factory_List_Lock[num].lock();
  //printf("Got FL lock %d C\n",num); fflush(stdout);
  if (Factory[num].size() > 0)
    {
      wait= true;
      wt= Factory[num].size() * 60;
    }
  Factory_List_Lock[num].unlock();
  //printf("Released FL lock %d C\n",num); fflush(stdout);
  // Need to sync players
  vector<string> o(P.nplayers());
  o[P.whoami()]= "N";
  if (wait)
    {
      o[P.whoami()]= "W";
    }
  if (finished)
    {
      o[P.whoami()]= "Y";
    }
  P.Broadcast_Receive(o);
  for (unsigned int p= 0; p < P.nplayers(); p++)
    {
      if (o[p].compare("Y") == 0)
        {
          finished= true;
        }
      if (o[p].compare("W") == 0)
        {
          wait= true;
        }
    }
  if (wait && !finished)
    {
      sleep(wt);
    }
  return finished;
}

/* Returns true if we have finished now and we are exiting */
bool Do_ZKPoK(condition type, ZKPoK &ZK, Player &P,
              FHE_Industry &Industry,
              const FHE_PK &pk,
              const FFT_Data &PTD, PRNG &G, const offline_control_data &OCD,
              unsigned int num,
              int verbose,
              const vector<gfp> &alpha= {})
{
  unsigned int whoami= P.whoami(), nplayers= P.nplayers();

  // Step 1 first step
  if (verbose > 0)
    {
      printf("ZKPoK %d Step 1: %d\n", num, Industry.get_zkc(num));
      //printf("Time : %d %f \n",num,global_time.elapsed());
    }
  ZK.Step1(type, pk, PTD, G, alpha);
  if (Industry.is_finished(num, P, OCD))
    {
      return true;
    }

  // Transmit E and A data
  //   - Put within a scoping to ensure data is removed
  if (verbose > 0)
    {
      printf("Transmitting ZKPoK %d Stage 1 Data: %d\n", num, Industry.get_zkc(num));
      //printf("Time : %d %f \n",num,global_time.elapsed());
    }
  {
    vector<string> vsE(nplayers), vsA(nplayers);
    stringstream osE, osA;
    ZK.get_vE(osE);
    ZK.get_vA(osA);
    vsE[whoami]= osE.str();
    vsA[whoami]= osA.str();
    P.Broadcast_Receive(vsE);
    P.Broadcast_Receive(vsA);
    for (unsigned int i= 0; i < nplayers; i++)
      {
        if (i != whoami)
          {
            istringstream isE(vsE[i]), isA(vsA[i]);
            ZK.Step1_Step(isE, isA, pk);
          }
      }
  }
  if (Industry.is_finished(num, P, OCD))
    {
      return true;
    }

  // Step 2 first step
  if (verbose > 0)
    {
      printf("ZKPoK %d Agree Random: %d\n", num, Industry.get_zkc(num));
      //printf("Time : %d %f \n",num,global_time.elapsed());
    }
  uint8_t seed[ZK_stat_sec / 8];
  AgreeRandom(P, seed, ZK_stat_sec / 8);
  vector<int> e(ZK_stat_sec);
  int c= 0;
  for (unsigned int i= 0; i < ZK_stat_sec / 8; i++)
    {
      for (unsigned int j= 0; j < 8; j++)
        {
          e[c]= (seed[i] >> j) & 1;
          c++;
        }
    }
  if (Industry.is_finished(num, P, OCD))
    {
      return true;
    }

  if (verbose > 0)
    {
      printf("ZKPoK %d Step 2: %d\n", num, Industry.get_zkc(num));
      //printf("Time : %d %f \n",num,global_time.elapsed());
    }
  ZK.Step2(e, pk);
  if (Industry.is_finished(num, P, OCD))
    {
      return true;
    }

  // Transmit T and Z data
  if (verbose > 0)
    {
      printf("Transmitting ZKPoK %d Stage 2 Data: %d\n", num, Industry.get_zkc(num));
      //printf("Time : %d %f \n",num,global_time.elapsed());
    }
  {
    vector<string> vsT(nplayers), vsZ(nplayers);
    stringstream osT, osZ;
    ZK.get_vT(osT);
    ZK.get_vZ(osZ);
    vsT[whoami]= osT.str();
    vsZ[whoami]= osZ.str();
    P.Broadcast_Receive(vsT);
    P.Broadcast_Receive(vsZ);
    for (unsigned int i= 0; i < nplayers; i++)
      {
        if (i != whoami)
          {
            istringstream isT(vsT[i]), isZ(vsZ[i]);
            ZK.Step2_Step(isT, isZ, pk);
          }
      }
  }
  // Here we do not check whether finished or not, as Step3 involves
  // no communication, and hence no autosyncing, so we could have one
  // party rushed ahead and finished, with another waiting to catch up
  // and still needing Step3 to terminate

  // Final Step 3
  if (verbose > 0)
    {
      printf("ZKPoK %d Step 3: %d\n", num, Industry.get_zkc(num));
      //printf("Time : %d %f \n",num,global_time.elapsed());
    }
  if (!ZK.Step3(pk, PTD, nplayers))
    {
      throw ZKPoK_Fail();
    }

  Industry.increment_zkc(num);
  return false;
}

/* This function should be enterred in thread i with Factory_List i
 * locked, except for i=0
 *   - i.e. we call this in Factory thread i after the global 
 *     constructor is called.
 */
void FHE_Industry::FHE_Factory(Player &P, const offline_control_data &OCD, const FHE_PK &pk,
                               const FFT_Data &PTD,
                               unsigned int mynumber,
                               int verbose)
{
  PRNG G;
  G.ReSeed(1000 + mynumber);

  /* If I am number zero then I set all the stuff up 
   * and we lock the other ones out
   */
  if (mynumber == 0)
    { // Do the ZKPoKs for the MAC ciphertexts
      //   No need to lock as no player will access these
      //   until the main factory is running
      if (verbose > 0)
        {
          printf("Doing ZKPoKs for the mac ctxs\n");
        }
      {
        ZKPoK ZK;
        Do_ZKPoK(Diagonal, ZK, P, *this, pk, PTD, G, OCD, mynumber, verbose, P.get_mac_keys());
        ctx_macs.resize(P.get_mac_keys().size(), Ciphertext(pk.get_params()));
        Plaintext dummy(PTD);
        for (unsigned int i= 0; i < P.get_mac_keys().size(); i++)
          {
            ZK.get_entry(dummy, ctx_macs[i], i);
          }
        // Now do the unlocks from the constructor
        for (unsigned int i= 1; i < Factory.size(); i++)
          {
            Current_Factory_Lock[i].unlock();
          }
      }
    }

  if (verbose > 0)
    {
      printf("Finished mac ctxs : %f \n", global_time.elapsed());
    }

  bool finished= false;
  while (!finished)
    {
      finished= is_finished(mynumber, P, OCD);

      if (!finished)
        {
          ZKPoK ZK;
          if (verbose > 0)
            {
              printf("Generating a factory\n");
            }
          finished= Do_ZKPoK(General, ZK, P, *this, pk, PTD, G, OCD, mynumber, verbose);
          if (!finished)
            {
              Factory_List_Lock[mynumber].lock();
              Factory[mynumber].push_back(ZK);
              if (verbose > 0)
                {
                  printf("Size of factory %d list is now %lu\n", mynumber, Factory[mynumber].size());
                }
              Factory_List_Lock[mynumber].unlock();
            }
        }
    }
}
