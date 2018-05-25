/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "FHE_Factory.h"
#include "FHE/ZKPoK.h"
#include "LSSS/PRSS.h"

#include <list>
#include <mutex>
#include <unistd.h>
using namespace std;

mutex Factory_List_Lock;
list<ZKPoK> Factory;

mutex Current_Factory_Lock;
int batch_number= -1;
ZKPoK Current_Factory;

vector<Ciphertext> ctx_macs;

// Checks exit, by player zero making decisions and conveying this
// to all others. This means players are in sync with player zero. Even
// if they know themselves they should die gracefully/wait
// Returns
//    0 = OK, prepare some more stuff
//    1 = Exit
int check_exit(int num_online, offline_control_data &OCD)
{
  int result= 0;
  OCD.sacrifice_mutex[num_online].lock();
  if (OCD.finish_offline[num_online] == 1)
    {
      result= 1;
    }
  OCD.sacrifice_mutex[num_online].unlock();
  return result;
}

// Assume Current_Factory_Lock is locked!!!
//   Returns -1 if we should exit, else 0
int Update_If_Empty(int num_online, offline_control_data &OCD)
{
  if (Current_Factory.isempty())
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
          Factory_List_Lock.lock();
          if (Factory.size() == 0)
            {
              wait= true;
            }
          Factory_List_Lock.unlock();
          if (wait)
            {
              // Wait for ages as ZKPoK takes ages
              sleep(5);
            }
        }
      Factory_List_Lock.lock();
      batch_number++;
      printf("Moving a factory into current batch %d\n", batch_number);
      Current_Factory= Factory.front();
      Factory.pop_front();
      printf("Size of factory list is now %lu\n", Factory.size());
      Factory_List_Lock.unlock();
    }
  return 0;
}

/* Returns the number for checking purposes 
 *  -1 if should exit
 */
int Next_Off_Production_Line(Plaintext &mess, Ciphertext &ctx,
                             const Player &P, int num_online,
                             offline_control_data &OCD)
{
  // Player zero decides which batch and which number to
  // take. Broadcasts it.
  // The other players wait for this batch/number to be
  // ready and then use it
  int batch, i;
  // If player zero broadcast batch_number and the item to use
  if (P.whoami() == 0)
    {
      Current_Factory_Lock.lock();
      i= Update_If_Empty(num_online, OCD);
      batch= batch_number;
      if (i == 0)
        {
          i= Current_Factory.get_next_unused();
          Current_Factory.get_entry(mess, ctx, i);
        }
      Current_Factory_Lock.unlock();
      stringstream ss;
      ss << batch << " " << i;
      // printf("Telling players to access item : %s\n",ss.str().c_str());
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
      is >> batch >> i;
      if (i >= 0)
        {
          bool wait= true;
          while (wait)
            {
              wait= false;
              Factory_List_Lock.lock();
              if (batch != batch_number)
                {
                  wait= true;
                }
              Factory_List_Lock.unlock();
              if (wait)
                {
                  sleep(1);
                  Current_Factory_Lock.lock();
                  Update_If_Empty(num_online, OCD);
                  Current_Factory_Lock.unlock();
                }
            }
          Current_Factory_Lock.lock();
          Current_Factory.get_entry(mess, ctx, i);
          Current_Factory_Lock.unlock();
        }
    }
  return i;
}

/* If one party says every thread is finished then finish */
bool is_finished(Player &P, const offline_control_data &OCD)
{
  bool finished= true;
  for (unsigned int i= 0; i < OCD.finished_offline.size() && finished; i++)
    {
      OCD.sacrifice_mutex[i].lock();
      if (OCD.finished_offline[i] < 4)
        {
          finished= false;
        }
      OCD.sacrifice_mutex[i].unlock();
    }
  // Need to sync players
  vector<string> o(P.nplayers());
  o[P.whoami()]= "Y";
  if (!finished)
    {
      o[P.whoami()]= "N";
    }
  P.Broadcast_Receive(o);
  for (unsigned int p= 0; p < P.nplayers(); p++)
    {
      if (o[p].compare("Y") == 0)
        {
          finished= true;
        }
    }
  return finished;
}

/* Returns true if we have finished now and we are exiting */
bool Do_ZKPoK(condition type, ZKPoK &ZK, Player &P, const FHE_PK &pk,
              const FFT_Data &PTD, PRNG &G, const offline_control_data &OCD,
              const vector<gfp> &alpha= {})
{
  int whoami= P.whoami(), nplayers= P.nplayers();

  // Step 1 first step
  printf("ZKPoK Step 1\n");
  ZK.Step1(type, pk, PTD, G, alpha);
  if (is_finished(P, OCD))
    {
      return true;
    }

  // Transmit E and A data
  //   - Put within a scoping to ensure data is removed
  printf("Transmitting ZKPoK Stage 1 Data\n");
  {
    vector<string> vsE(nplayers), vsA(nplayers);
    stringstream osE, osA;
    ZK.get_vE(osE);
    ZK.get_vA(osA);
    vsE[whoami]= osE.str();
    vsA[whoami]= osA.str();
    P.Broadcast_Receive(vsE);
    P.Broadcast_Receive(vsA);
    for (int i= 0; i < nplayers; i++)
      {
        if (i != whoami)
          {
            istringstream isE(vsE[i]), isA(vsA[i]);
            ZK.Step1_Step(isE, isA, pk);
          }
      }
  }
  if (is_finished(P, OCD))
    {
      return true;
    }

  // Step 2 first step
  printf("ZKPoK Agree Random\n");
  uint8_t seed[stat_sec / 8];
  AgreeRandom(P, seed, stat_sec);
  vector<int> e(stat_sec);
  int c= 0;
  for (int i= 0; i < stat_sec / 8; i++)
    {
      for (int j= 0; j < 8; j++)
        {
          e[c]= (seed[i] >> j) & 1;
          c++;
        }
    }
  if (is_finished(P, OCD))
    {
      return true;
    }

  printf("ZKPoK Step 2\n");
  ZK.Step2(e, pk);
  if (is_finished(P, OCD))
    {
      return true;
    }

  // Transmit T and Z data
  printf("Transmitting ZKPoK Stage 2 Data\n");
  {
    vector<string> vsT(nplayers), vsZ(nplayers);
    stringstream osT, osZ;
    ZK.get_vT(osT);
    ZK.get_vZ(osZ);
    vsT[whoami]= osT.str();
    vsZ[whoami]= osZ.str();
    P.Broadcast_Receive(vsT);
    P.Broadcast_Receive(vsZ);
    for (int i= 0; i < nplayers; i++)
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
  printf("ZKPoK Step 3\n");
  if (!ZK.Step3(pk, PTD, nplayers))
    {
      throw ZKPoK_Fail();
    }

  return false;
}

void FHE_Factory(Player &P, const offline_control_data &OCD, const FHE_PK &pk,
                 const FFT_Data &PTD)
{
  PRNG G;
  G.ReSeed(1000);

  // Do the ZKPoKs for the MAC ciphertexts
  //   No need to lock as no player will access these
  //   until the main factory is running
  printf("Doing ZKPoKs for the mac ctxs\n");
  {
    ZKPoK ZK;
    Do_ZKPoK(Diagonal, ZK, P, pk, PTD, G, OCD, P.get_mac_keys());
    ctx_macs.resize(P.get_mac_keys().size(), Ciphertext(pk.get_params()));
    Plaintext dummy(PTD);
    for (unsigned int i= 0; i < P.get_mac_keys().size(); i++)
      {
        ZK.get_entry(dummy, ctx_macs[i], i);
      }
  }

  bool finished= false;
  while (!finished)
    {
      bool wait= true;
      while (wait)
        {
          wait= false;
          Factory_List_Lock.lock();
          if (Factory.size() > 1)
            {
              wait= true;
            }
          Factory_List_Lock.unlock();
          if (wait)
            {
              sleep(1);
            }
          if (is_finished(P, OCD))
            {
              wait= false;
              finished= true;
            }
        }

      if (!finished)
        {
          ZKPoK ZK;
          printf("Generating a factory\n");
          finished= Do_ZKPoK(General, ZK, P, pk, PTD, G, OCD);
          if (!finished)
            {
              Factory_List_Lock.lock();
              Factory.push_back(ZK);
              printf("Size of factory list is now %lu\n", Factory.size());
              Factory_List_Lock.unlock();
            }
        }
    }
}
