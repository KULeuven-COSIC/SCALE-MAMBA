/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#include "offline_phases.h"
#include "config.h"
#include "offline.h"
#include "offline_IO_production.h"
#include "offline_data.h"
#include "sacrifice.h"

#include "LSSS/PRSS.h"
#include "LSSS/PRZS.h"
#include "Tools/Timer.h"

#include <mutex>
#include <unistd.h>
using namespace std;

extern Timer global_time;

extern vector<sacrificed_data> SacrificeD;

enum ODtype {
  Triples,
  Squares,
  Bits
};

// Checks exit/wait, by player zero making decisions and conveying this
// to all others. This means players are in sync with player zero. Even
// if they know themselves they should die gracefully/wait
// Returns
//    0 = OK, prepare some more stuff
//    1 = Exit
//    2 = Wait
// Do not apply locking as we are only reading, this should be OK
int check_exit(int num_online, const Player &P, offline_control_data &OCD, ODtype T)
{
  int result= 0;
  string ss= "-";
  if (P.whoami() == 0)
    {
      //OCD.OCD_mutex[num_online].lock();
      if (OCD.finish_offline[num_online] == 1)
        {
          ss= "E";
          result= 1;
        }
      //OCD.OCD_mutex[num_online].unlock();
      if (result == 0)
        { /* wait if the queues are too big */
          switch (T)
            {
              case Triples:
                //OCD.mul_mutex[num_online].lock();
                if ((SacrificeD[num_online].TD.ta.size() > max_triples_sacrifice) || (OCD.totm[num_online] > OCD.maxm && OCD.maxm != 0))
                  {
                    result= 2;
                    ss= "W";
                  }
                //OCD.mul_mutex[num_online].unlock();
                break;
              case Squares:
                //OCD.sqr_mutex[num_online].lock();
                if ((SacrificeD[num_online].SD.sa.size() > max_squares_sacrifice) || (OCD.tots[num_online] > OCD.maxs && OCD.maxs != 0))
                  {
                    result= 2;
                    ss= "W";
                  }
                //OCD.sqr_mutex[num_online].unlock();
                break;
              case Bits:
                //OCD.bit_mutex[num_online].lock();
                if ((SacrificeD[num_online].BD.bb.size() > max_bits_sacrifice) || (OCD.totb[num_online] > OCD.maxb && OCD.maxb != 0))
                  {
                    result= 2;
                    ss= "W";
                  }
                //OCD.bit_mutex[num_online].unlock();
                break;
              default:
                throw bad_value();
            }
        }
      P.send_all(ss);
    }
  else
    {
      P.receive_from_player(0, ss);
      if (ss.compare("E") == 0)
        {
          result= 1;
        }
      if (ss.compare("W") == 0)
        {
          result= 2;
        }
    }
  return result;
}

void mult_phase(int num_online, Player &P, int fake_sacrifice,
                offline_control_data &OCD, const FHE_PK &pk,
                const FHE_SK &sk, const FFT_Data &PTD,
                FHE_Industry &industry,
                int verbose)
{
  // Initialize PRSS stuff
  PRSS prss(P);
  PRZS przs(P);
  FakePrep prep(P);

  Open_Protocol OP;

  list<Share> a, b, c;
  list<Share>::iterator it;
  int flag;
  while (0 == 0)
    {
      flag= check_exit(num_online, P, OCD, Triples);
      /* Needs to die gracefully if online is gone */
      if (flag == 1)
        {
          printf("Exiting mult phase : thread = %d\n", num_online);
          OCD.OCD_mutex[num_online].lock();
          OCD.finished_offline[num_online]++;
          OCD.OCD_mutex[num_online].unlock();
          return;
        }

      if (flag == 2)
        {
          sleep(1);
        }
      else
        {
          if (verbose > 1)
            {
              printf("In triples: thread = %d\n", num_online);
              fflush(stdout);
            }

          offline_phase_triples(P, prss, przs, prep, a, b, c, pk, sk, PTD, fake_sacrifice, OP, industry);
          OP.RunOpenCheck(P, "", 0);
          if (verbose > 1)
            {
              printf("Out of triples: thread = %d\n", num_online);
              fflush(stdout);
            }

          /* Add to queues */
          OCD.mul_mutex[num_online].lock();
          OCD.totm[num_online]+= a.size();
          it= SacrificeD[num_online].TD.ta.end();
          SacrificeD[num_online].TD.ta.splice(it, a);
          it= SacrificeD[num_online].TD.tb.end();
          SacrificeD[num_online].TD.tb.splice(it, b);
          it= SacrificeD[num_online].TD.tc.end();
          SacrificeD[num_online].TD.tc.splice(it, c);
          OCD.mul_mutex[num_online].unlock();
        }
    }
}

void square_phase(int num_online, Player &P, int fake_sacrifice,
                  offline_control_data &OCD,
                  const FHE_PK &pk, const FHE_SK &sk, const FFT_Data &PTD,
                  FHE_Industry &industry,
                  int verbose)
{
  // Initialize PRSS stuff
  PRSS prss(P);
  PRZS przs(P);
  FakePrep prep(P);

  Open_Protocol OP;

  list<Share> a, b, c;
  list<Share>::iterator it;
  int flag;
  while (0 == 0)
    {
      flag= check_exit(num_online, P, OCD, Squares);

      /* Needs to die gracefully if online is gone */
      if (flag == 1)
        {
          printf("Exiting square phase: thread = %d\n", num_online);
          OCD.OCD_mutex[num_online].lock();
          OCD.finished_offline[num_online]++;
          OCD.OCD_mutex[num_online].unlock();
          return;
        }

      if (flag == 2)
        {
          sleep(1);
        }
      else
        {
          if (verbose > 1)
            {
              printf("In squares: thread = %d\n", num_online);
              fflush(stdout);
            }

          offline_phase_squares(P, prss, przs, prep, a, b, pk, sk, PTD, fake_sacrifice, OP, industry);
          OP.RunOpenCheck(P, "", 0);
          if (verbose > 1)
            {
              printf("Out of squares: thread = %d\n", num_online);
              fflush(stdout);
            }

          /* Add to queues */
          OCD.sqr_mutex[num_online].lock();
          OCD.tots[num_online]+= a.size();
          it= SacrificeD[num_online].SD.sa.end();
          SacrificeD[num_online].SD.sa.splice(it, a);
          it= SacrificeD[num_online].SD.sb.end();
          SacrificeD[num_online].SD.sb.splice(it, b);
          OCD.sqr_mutex[num_online].unlock();
        }
    }
}

void bit_phase(int num_online, Player &P, int fake_sacrifice,
               offline_control_data &OCD,
               const FHE_PK &pk, const FHE_SK &sk, const FFT_Data &PTD,
               FHE_Industry &industry,
               int verbose)
{
  // Initialize PRSS stuff
  PRSS prss(P);
  PRZS przs(P);
  FakePrep prep(P);

  Open_Protocol OP;

  list<Share> a, b, c;
  list<Share>::iterator it;
  int flag;
  while (0 == 0)
    {
      flag= check_exit(num_online, P, OCD, Bits);

      /* Needs to die gracefully if online is gone */
      if (flag == 1)
        {
          printf("Exiting bit phase: thread = %d\n", num_online);
          OCD.OCD_mutex[num_online].lock();
          OCD.finished_offline[num_online]++;
          OCD.OCD_mutex[num_online].unlock();
          return;
        }

      if (flag == 2)
        {
          sleep(1);
        }
      else
        {
          if (verbose > 1)
            {
              printf("In bits: thread = %d\n", num_online);
              fflush(stdout);
            }

          offline_phase_bits(P, prss, przs, prep, b, pk, sk, PTD, fake_sacrifice, OP, industry);
          OP.RunOpenCheck(P, "", 0);

          if (verbose > 1)
            {
              printf("Out of bits: thread = %d\n", num_online);
              fflush(stdout);
            }

          /* Add to queues */
          OCD.bit_mutex[num_online].lock();
          OCD.totb[num_online]+= b.size();
          it= SacrificeD[num_online].BD.bb.end();
          SacrificeD[num_online].BD.bb.splice(it, b);
          OCD.bit_mutex[num_online].unlock();
        }
    }
}

/* Decide what to do for the inputs thread
 *   Make inputs or exit or wait
 * All players propose and then they take the minumum
 * No thread locks as we are only reading
 */
bool propose_what_to_do(int num_online, Player &P, int &finish,
                        vector<int> &make_inputs, bool &minput_global,
                        offline_control_data &OCD,
                        int verbose)
{
  // The number of sacrifice equations we need per item produced
  int rep= sacrifice_stat_sec / numBits(gfp::pr()) + 1;

  // Each player first either proposes a set of numbers or an exit
  finish= 0;
  minput_global= false;
  for (unsigned int i= 0; i < P.nplayers(); i++)
    {
      make_inputs[i]= 0;
    }

  //OCD.OCD_mutex[num_online].lock();
  for (unsigned int i= 0; i < P.nplayers(); i++)
    {
      if ((OCD.maxI == 0 && SacrificeD[num_online].ID.ios[i].size() < max_IO_sacrifice) || (OCD.totI[num_online] < OCD.maxI))
        {
          make_inputs[i]= 1;
          minput_global= true;
        }
    }

  /* Needs to die gracefully if we are told to */
  // If I am exiting tell other players
  if (OCD.finish_offline[num_online] == 1)
    {
      finish= -1;
    }
  //OCD.OCD_mutex[num_online].unlock();

  // Propose to other players what I have
  stringstream ss;
  ss << finish << " ";
  for (unsigned int i= 0; i < P.nplayers(); i++)
    {
      ss << make_inputs[i] << " ";
    }
  if (verbose > 1)
    {
      printf("Proposing Sacrifice : thread = %d : %s rep=%d\n", num_online,
             ss.str().c_str(), rep);
      fflush(stdout);
    }
  P.send_all(ss.str());

  // Now get data from all players taking minimum/max where necessary
  for (unsigned int i= 0; i < P.nplayers(); i++)
    {
      if (i != P.whoami())
        {
          string ss;
          P.receive_from_player(i, ss);
          istringstream is(ss);
          int fint, ni;
          is >> fint;
          finish= min(finish, fint);
          for (unsigned int i= 0; i < P.nplayers(); i++)
            {
              is >> ni;
              make_inputs[i]= max(make_inputs[i], ni);
              if (make_inputs[i] == 1)
                {
                  minput_global= true;
                }
            }
        }
    }

  bool exit= false;
  // Signal exit if anyone signalled exit
  if (finish == -1)
    {
      exit= true;
    }
  return exit;
}

/* Thread locks removed when only reading */
void inputs_phase(int num_online, Player &P, int fake_sacrifice,
                  offline_control_data &OCD, const FHE_PK &pk,
                  const FHE_SK &sk, const FFT_Data &PTD,
                  FHE_Industry &industry,
                  int verbose)
{
  int finish;

  // Initialize PRSS stuff for IO production
  PRSS prss(P);

  Open_Protocol OP;

  list<Share> a;
  list<gfp> opened;
  list<Share>::iterator it;
  list<gfp>::iterator it_g;
  vector<int> minputs(P.nplayers());
  bool minput_global, exit= false;
  while (0 == 0)
    {
      exit= propose_what_to_do(num_online, P, finish, minputs, minput_global, OCD,
                               verbose - 1);

      if (exit != true && finish == 0 && minput_global == false)
        {
          sleep(2);
        }
      else
        {
          for (unsigned int i= 0; i < P.nplayers() && !exit; i++)
            {
              if (minputs[i])
                {
                  make_IO_data(P, fake_sacrifice, prss, i, a, opened, pk, sk, PTD, OCD, OP, num_online, industry);

                  /* Add to queues */
                  OCD.OCD_mutex[num_online].lock();
                  OCD.totI[num_online]+= a.size();
                  it= SacrificeD[num_online].ID.ios[i].end();
                  SacrificeD[num_online].ID.ios[i].splice(it, a);
                  if (i == P.whoami())
                    {
                      it_g= SacrificeD[num_online].ID.opened_ios.end();
                      SacrificeD[num_online].ID.opened_ios.splice(it_g, opened);
                    }
                  OCD.OCD_mutex[num_online].unlock();
                }
            }
        }
      if (verbose > 0)
        { // Only reading for printing purposes so we dont lock mutex's for reading

          // Print this data it is useful for debugging stuff
          printf("Sacrifice Queues : thread = %d : %lu %lu %lu : ", num_online,
                 SacrificeD[num_online].TD.ta.size(),
                 SacrificeD[num_online].SD.sa.size(),
                 SacrificeD[num_online].BD.bb.size());
          for (unsigned int i= 0; i < P.nplayers(); i++)
            {
              printf("%lu ", SacrificeD[num_online].ID.ios[i].size());
            }
          printf("\n");
          fflush(stdout);

          // Printing timing information (good for timing offline phase). Note
          // this is not properly thread locked, but we are only reading
          double total= 0, btotal= 0, avg;
          double time= global_time.elapsed();
          for (unsigned int i= 0; i < SacrificeD.size(); i++)
            {
              total+= OCD.totm[i];
            }
          btotal= total;
          avg= time / total;
          printf("Seconds per Mult Triple (all threads) %f : Total %f : Throughput %f\n", avg, total, 1 / avg);

          total= 0;
          for (unsigned int i= 0; i < SacrificeD.size(); i++)
            {
              total+= OCD.tots[i];
            }
          btotal+= total;
          avg= time / total;
          printf("Seconds per Square Pair (all threads) %f : Total %f : Throughput %f\n", avg, total, 1 / avg);

          total= 0;
          for (unsigned int i= 0; i < SacrificeD.size(); i++)
            {
              total+= OCD.totb[i];
            }
          btotal+= total;
          avg= time / total;
          printf("Seconds per Bit (all threads) %f : Total %f : Throughput %f\n", avg, total, 1 / avg);
          avg= time / btotal;
          printf("Seconds per `Thing` (all threads) %f : Total %f : Throughput %f\n", avg, total, 1 / avg);
        }
      if (!exit && (OCD.maxm != 0 || OCD.maxs != 0 || OCD.maxb != 0 || OCD.maxI != 0))
        { /* Check whether we should kill the offline phase as we have now enough data */
          exit= true;

          // OCD.mul_mutex[num_online].lock();
          if (OCD.totm[num_online] < OCD.maxm || OCD.maxm == 0)
            {
              exit= false;
            }
          //OCD.mul_mutex[num_online].unlock();

          //OCD.sqr_mutex[num_online].lock();
          if (OCD.tots[num_online] < OCD.maxs || OCD.maxs == 0)
            {
              exit= false;
            }
          //OCD.sqr_mutex[num_online].unlock();

          //OCD.bit_mutex[num_online].lock();
          if (OCD.totb[num_online] < OCD.maxb || OCD.maxb == 0)
            {
              exit= false;
            }
          //OCD.bit_mutex[num_online].unlock();

          //OCD.OCD_mutex[num_online].lock();
          if (OCD.totI[num_online] < OCD.maxI || OCD.maxI == 0)
            {
              exit= false;
            }
          //OCD.OCD_mutex[num_online].unlock();

          if (exit)
            {
              OCD.OCD_mutex[num_online].lock();
              OCD.finish_offline[num_online]= 1;
              OCD.OCD_mutex[num_online].unlock();
              printf("We have enough data to stop offline phase now\n");
            }
        }
      if (exit)
        {
          OCD.OCD_mutex[num_online].lock();
          OCD.finished_offline[num_online]++;
          OCD.OCD_mutex[num_online].unlock();
        }
      if (exit)
        {
          printf("Exiting inputs phase : thread = %d\n", num_online);
          return;
        }
    }
}
