/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "FHE_Factory.h"
#include "FHE/ZKPoK.h"
#include "LSSS/PRSS.h"
#include "Tools/Crypto.h"
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
  ready= false;
}

FHE_Industry::~FHE_Industry()
{
  delete[] Factory_List_Lock;
  delete[] Current_Factory_Lock;
}

// Assume Current_Factory_Lock[num] is locked!!!
void FHE_Industry::Update_If_Empty(unsigned int num)
{
  if (Current_Factory[num].isempty())
    {
      bool wait= true;
      while (wait)
        {
          wait= false;
          Factory_List_Lock[num].lock();
          if (Factory[num].size() == 0)
            {
              wait= true;
            }
          Factory_List_Lock[num].unlock();
          if (wait)
            {
              sleep(1);
            }
        }
      Factory_List_Lock[num].lock();
      batch_number[num]++;
      Current_Factory[num]= Factory[num].front();
      Factory[num].pop_front();
      Factory_List_Lock[num].unlock();
    }
}

/* Returns the number for checking purposes */
int FHE_Industry::Next_Off_Production_Line(Plaintext &mess, Ciphertext &ctx,
                                           const Player &P, const string &info)
{
  //printf("\tIn next off production line %s \n",info.c_str());
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
          Update_If_Empty(num);
        }
      // Now Current_Factory_Lock[num] is locked and non-empty
      batch= batch_number[num];
      i= Current_Factory[num].get_next_unused();
      if (i < 0)
        {
          printf("Error in call from %s\n", info.c_str());
          throw bad_value();
        }
      Current_Factory[num].get_entry(mess, ctx, (unsigned int) i);
      Current_Factory_Lock[num].unlock();
      stringstream ss;
      ss << num << " " << batch << " " << i;
      //printf("\tTelling players to access item : %s : %s\n",ss.str().c_str(),info.c_str());
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
      //printf("\tBeen told to access item : %s : %s\n",ss.c_str(),info.c_str());
      istringstream is(ss);
      is >> num >> batch >> i;
      bool wait= true;
      while (wait)
        {
          wait= false;
          Factory_List_Lock[num].lock();
          //printf("\t%d : %d %d\n",num,batch,batch_number[num]);
          if (batch != batch_number[num])
            {
              wait= true;
            }
          Factory_List_Lock[num].unlock();
          if (wait)
            {
              sleep(1);
              Current_Factory_Lock[num].lock();
              Update_If_Empty(num);
              Current_Factory_Lock[num].unlock();
            }
        }
      Current_Factory_Lock[num].lock();
      Current_Factory[num].get_entry(mess, ctx, (unsigned int) i);
      Current_Factory_Lock[num].unlock();
    }
  //printf("\tExiting next off production line : %s\n",info.c_str());

  return i;
}

/* If one party says every thread is finished then finish */
bool FHE_Industry::is_finished(unsigned int num, Player &P, const offline_control_data &OCD)
{
  bool finished= false, wait= true;
  // Loop if we are waiting and not finished
  while (wait && !finished)
    {
      finished= true, wait= false;
      for (unsigned int i= 0; i < OCD.finished_offline.size() && finished; i++)
        {
          //printf("Waiting for sacrifice lock %d %d B\n",num,i); fflush(stdout);
          OCD.OCD_mutex[i].lock();
          //printf("Got sacrifice lock %d %d B\n",num,i); fflush(stdout);
          if (OCD.finished_offline[i] < 4)
            {
              finished= false;
            }
          OCD.OCD_mutex[i].unlock();
          //printf("Released sacrifice lock %d %d B\n",num,i); fflush(stdout);
        }
      // We want to sleep if the factory list is getting too big the amount
      // and trigger for sleep depending on whether we are using HighGear
      // or TopGear
      //printf("Waiting for FL lock %d C\n",num); fflush(stdout);
      Factory_List_Lock[num].lock();
#ifndef TOP_GEAR
      if (Factory[num].size() > 2)
        {
          wait= true;
        }
#else
      if (Factory[num].size() > 4)
        {
          wait= true;
        }
#endif
      Factory_List_Lock[num].unlock();
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
      //printf("\n is_finished B : %d %s\n",P.whoami(),o[P.whoami()].c_str());
      P.Broadcast_Receive(o);
      for (unsigned int p= 0; p < P.nplayers(); p++)
        {
          //printf("\n is_finished R : %d %s\n",p,o[p].c_str());
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
          sleep(5);
        }
    }
  return finished;
}

void Do_Step0_Step(ZKPoK &ZK, Player &P, const FHE_PK &pk,
                   unsigned int num, int verbose)
{
  unsigned int whoami= P.whoami(), nplayers= P.nplayers();

  // Transmit E data
  if (verbose > 0)
    {
      printf("Transmitting ZKPoK %d Stage 0  \n", num);
    }
  vector<string> vsE(nplayers);
  stringstream osE;
  ZK.get_vE(osE);
  vsE[whoami]= osE.str();
  //printf("\n Do_Step0 : B %d\n",P.whoami());

  // Do commit and open to avoid the attack of Ivan
  Commit_And_Open(vsE, P, true);

  //printf("\n Do_Step0 : R %d\n",P.whoami());
  for (unsigned int i= 0; i < nplayers; i++)
    {
      if (i != whoami)
        {
          istringstream isE(vsE[i]);
          ZK.Step0_Step(isE, pk);
        }
    }
}

/* Returns true if we have finished now and we are exiting
 *  - Assumes Step0 has already been executed
 */
bool Do_ZKPoK(ZKPoK &ZK, Player &P,
              FHE_Industry &Industry,
              const FHE_PK &pk,
              const FFT_Data &PTD, PRNG &G, const offline_control_data &OCD,
              unsigned int num,
              int verbose)
{
  unsigned int whoami= P.whoami(), nplayers= P.nplayers();

  // Step 1 first step
  if (verbose > 0)
    {
      printf("ZKPoK %d Step 1: %d\n", num, Industry.get_zkc(num));
      //printf("Time : %d %f \n",num,global_time.elapsed());
    }
  ZK.Step1(pk, PTD, G);
  if (Industry.is_finished(num, P, OCD))
    {
      return true;
    }

  // Transmit A data
  //   - Put within a scoping to ensure data is removed
  if (verbose > 0)
    {
      printf("Transmitting ZKPoK %d Stage 1 Data: %d\n", num, Industry.get_zkc(num));
      //printf("Time : %d %f \n",num,global_time.elapsed());
    }
  {
    vector<string> vsA(nplayers);
    stringstream osA;
    ZK.get_vA(osA);
    vsA[whoami]= osA.str();
    //printf("\n Do_ZKPoK : B vsA %d\n",P.whoami());
    P.Broadcast_Receive(vsA);
    //printf("\n Do_ZKPoK : R vsA %d\n",P.whoami());
    for (unsigned int i= 0; i < nplayers; i++)
      {
        if (i != whoami)
          {
            istringstream isA(vsA[i]);
            ZK.Step1_Step(isA, pk);
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
  uint8_t seed[SEED_SIZE];
  //printf("\n Do_ZKPoK : ARS %d\n",P.whoami());
  AgreeRandom(P, seed, SEED_SIZE);
  //printf("\n Do_ZKPoK : ARE %d\n",P.whoami());
  vector<int> e;
  ZK.Generate_e(e, seed);
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
    //printf("\n Do_ZKPoK : B vsT %d\n",P.whoami());
    P.Broadcast_Receive(vsT);
    //printf("\n Do_ZKPoK : R vsT %d\n",P.whoami());
    //printf("\n Do_ZKPoK : B vzZ %d\n",P.whoami());
    P.Broadcast_Receive(vsZ);
    //printf("\n Do_ZKPoK : R vzZ %d\n",P.whoami());
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
  G.ReSeed(10000 + mynumber);

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
#ifdef TOP_GEAR
        ZK.Step0(Diagonal, TopGear, pk, PTD, G, P.get_mac_keys());
#else
        ZK.Step0(Diagonal, HighGear, pk, PTD, G, P.get_mac_keys());
#endif
        Do_Step0_Step(ZK, P, pk, mynumber, verbose);

        // Here we need to repeat the ZKPoK until we have the required
        // soundness, as using TopGear we might have too low a soundness
        // at this point
        unsigned int sound= 0, increment= ZK.get_sound_sec();
        while (sound < ZK_sound_sec)
          {
            Do_ZKPoK(ZK, P, *this, pk, PTD, G, OCD, mynumber, verbose);
            sound+= increment;
            if (verbose > 0)
              {
                cout << "mac ctxs Soundness = " << sound << endl;
              }
          }

        // Now get the ciphertext we are going to use
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

        // And signal we are ready
        ready= true;
      }
      if (verbose > 0)
        {
          printf("\nFinished mac ctxs : %f \n\n", global_time.elapsed());
        }
    }

  bool finished= false;
  while (!finished)
    {
      finished= is_finished(mynumber, P, OCD);

      // Only execute a new ZKPoK if we have not finished
      if (!finished)
        {
          ZKPoK ZK;
          if (verbose > 0)
            {
              printf("Generating a factory\n");
            }
#ifdef TOP_GEAR
          ZK.Step0(General, TopGear, pk, PTD, G, P.get_mac_keys());
#else
          ZK.Step0(General, HighGear, pk, PTD, G, P.get_mac_keys());
#endif
          Do_Step0_Step(ZK, P, pk, mynumber, verbose);

          // Here we do not repeat as we assume as for general ZKPoKs
          // the ZKPoK itself takes care of the correct soundness security
          finished= Do_ZKPoK(ZK, P, *this, pk, PTD, G, OCD, mynumber, verbose);

          if (!finished)
            {
              Factory_List_Lock[mynumber].lock();
              Factory[mynumber].push_back(move(ZK));
              if (verbose > 0)
                {
                  printf("Size of factory %d list is now %lu: %d\n", mynumber, Factory[mynumber].size(),
                         get_zkc(mynumber));
                }
              Factory_List_Lock[mynumber].unlock();
            }
        }
    }
}
