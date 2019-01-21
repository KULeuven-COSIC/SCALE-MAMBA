/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

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

extern vector<triples_data> TriplesD;
extern vector<squares_data> SquaresD;
extern vector<bits_data> BitsD;
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
int check_exit(int num_online, const Player &P, offline_control_data &OCD, ODtype T)
{
  int result= 0;
  string ss= "-";
  if (P.whoami() == 0)
    {
      OCD.sacrifice_mutex[num_online].lock();
      if (OCD.finish_offline[num_online] == 1)
        {
          ss= "E";
          result= 1;
        }
      OCD.sacrifice_mutex[num_online].unlock();
      if (result == 0)
        { /* wait if the queues are too big */
          switch (T)
            {
              case Triples:
                OCD.mult_mutex[num_online].lock();
                if ((TriplesD[num_online].ta.size() > max_triples_offline) || (OCD.totm[num_online] > OCD.maxm && OCD.maxm != 0))
                  {
                    result= 2;
                    ss= "W";
                  }
                OCD.mult_mutex[num_online].unlock();
                break;
              case Squares:
                OCD.square_mutex[num_online].lock();
                if ((SquaresD[num_online].sa.size() > max_squares_offline) || (OCD.tots[num_online] > OCD.maxs && OCD.maxs != 0 && OCD.totb[num_online] > OCD.maxb && OCD.maxb != 0))
                  {
                    result= 2;
                    ss= "W";
                  }
                OCD.square_mutex[num_online].unlock();
                break;
              case Bits:
                OCD.bit_mutex[num_online].lock();
                if ((BitsD[num_online].bb.size() > max_bits_offline) || (OCD.totb[num_online] > OCD.maxb && OCD.maxb != 0))
                  {
                    result= 2;
                    ss= "W";
                  }
                OCD.bit_mutex[num_online].unlock();
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

void mult_phase(int num_online, Player &P, offline_control_data &OCD,
                const FHE_PK &pk, const FHE_SK &sk, const FFT_Data &PTD,
                FHE_Industry &industry,
                int verbose)
{
  // Initialize PRSS stuff
  PRSS prss(P);
  PRZS przs(P);

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
          OCD.sacrifice_mutex[num_online].lock();
          OCD.finished_offline[num_online]++;
          OCD.sacrifice_mutex[num_online].unlock();
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
          offline_phase_triples(P, prss, przs, a, b, c, pk, sk, PTD, industry);
          if (verbose > 1)
            {
              printf("Out of triples: thread = %d\n", num_online);
              fflush(stdout);
            }

          /* Add to queues */
          OCD.mult_mutex[num_online].lock();
          it= TriplesD[num_online].ta.end();
          TriplesD[num_online].ta.splice(it, a);
          it= TriplesD[num_online].tb.end();
          TriplesD[num_online].tb.splice(it, b);
          it= TriplesD[num_online].tc.end();
          TriplesD[num_online].tc.splice(it, c);
          OCD.mult_mutex[num_online].unlock();
        }
    }
}

void square_phase(int num_online, Player &P, offline_control_data &OCD,
                  const FHE_PK &pk, const FHE_SK &sk, const FFT_Data &PTD,
                  FHE_Industry &industry,
                  int verbose)
{
  // Initialize PRSS stuff
  PRSS prss(P);
  PRZS przs(P);

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
          OCD.sacrifice_mutex[num_online].lock();
          OCD.finished_offline[num_online]++;
          OCD.sacrifice_mutex[num_online].unlock();
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
          offline_phase_squares(P, prss, przs, a, b, pk, sk, PTD, industry);
          if (verbose > 1)
            {
              printf("Out of squares: thread = %d\n", num_online);
              fflush(stdout);
            }

          /* Add to queues */
          OCD.square_mutex[num_online].lock();
          it= SquaresD[num_online].sa.end();
          SquaresD[num_online].sa.splice(it, a);
          it= SquaresD[num_online].sb.end();
          SquaresD[num_online].sb.splice(it, b);
          OCD.square_mutex[num_online].unlock();
        }
    }
}

void bit_phase(int num_online, Player &P, offline_control_data &OCD,
               const FHE_PK &pk, const FHE_SK &sk, const FFT_Data &PTD,
               FHE_Industry &industry,
               int verbose)
{
  // Initialize PRSS stuff
  PRSS prss(P);
  PRZS przs(P);

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
          OCD.sacrifice_mutex[num_online].lock();
          OCD.finished_offline[num_online]++;
          OCD.sacrifice_mutex[num_online].unlock();
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
          offline_phase_bits(P, prss, przs, b, OP, pk, sk, PTD, industry);
          if (verbose > 1)
            {
              printf("Out of bits: thread = %d\n", num_online);
              fflush(stdout);
            }

          /* Add to queues */
          OCD.bit_mutex[num_online].lock();
          it= BitsD[num_online].bb.end();
          BitsD[num_online].bb.splice(it, b);
          OCD.bit_mutex[num_online].unlock();
        }
    }
}

/* This proposes a number of things to sacrifice to get
 * agreement amongst all players. All players propose
 * and then they take the minumum
 */
bool propose_numbers_sacrifice(int num_online, Player &P, int &nm, int &ns,
                               int &nb, vector<int> &make_inputs,
                               offline_control_data &OCD,
                               int verbose)
{
  // The number of sacrifice equations we need per item produced
  int rep= sacrifice_stat_sec / numBits(gfp::pr()) + 1;

  // Each player first either proposes a set of numbers or an exit
  nm= 0;
  ns= 0;
  nb= 0;
  bool minputs= false;
  for (unsigned int i= 0; i < P.nplayers(); i++)
    {
      make_inputs[i]= 0;
    }

  while (nm == 0 && ns == 0 && nb == 0 && minputs == false)
    {
      OCD.mult_mutex[num_online].lock();
      int ta= TriplesD[num_online].ta.size();
      OCD.mult_mutex[num_online].unlock();

      OCD.square_mutex[num_online].lock();
      int sa= SquaresD[num_online].sa.size();
      OCD.square_mutex[num_online].unlock();

      OCD.bit_mutex[num_online].lock();
      int bb= BitsD[num_online].bb.size();
      OCD.bit_mutex[num_online].unlock();

      nm= min((rep + 1) * sz_triples_sacrifice, ta) - 1;
      nm= (nm / (rep + 1)) * (rep + 1); // Make a round mult of (rep+1)
      if (nm < 0)
        {
          nm= 0;
        }

      nb= min(sz_bits_sacrifice, bb) - 1;
      nb= min(nb, sa / rep) -
          10; // Leave some gap for making squares
      if (nb < 0)
        {
          nb= 0;
        }

      ns= min((rep + 1) * sz_squares_sacrifice, sa - rep * nb) - 1;
      ns= (ns / (rep + 1)) * (rep + 1); // Make a round mult of (rep+1)
      if (ns < 0)
        {
          ns= 0;
        }

      if (verbose > 1)
        {
          printf("In sacrifice proposal: thread = %d : %d %d %d\n", num_online, ta, sa, bb);
          fflush(stdout);
        }

      OCD.sacrifice_mutex[num_online].lock();
      if (SacrificeD[num_online].TD.ta.size() > max_triples_sacrifice)
        {
          nm= 0;
        }
      if (OCD.totm[num_online] > OCD.maxm && OCD.maxm != 0)
        {
          nm= 0;
        }
      if (SacrificeD[num_online].SD.sa.size() > max_squares_sacrifice)
        {
          ns= 0;
        }
      if (OCD.tots[num_online] > OCD.maxs && OCD.maxs != 0)
        {
          ns= 0;
        }
      if (SacrificeD[num_online].BD.bb.size() > max_bits_sacrifice)
        {
          nb= 0;
        }
      if (OCD.totb[num_online] > OCD.maxb && OCD.maxb != 0)
        {
          nb= 0;
        }
      // Only thread zero cares about input/output bits
      if (num_online == 0)
        {
          for (unsigned int i= 0; i < P.nplayers(); i++)
            {
              if ((OCD.maxI == 0 && SacrificeD[num_online].ID.ios[i].size() < max_IO_sacrifice) || (OCD.totI < OCD.maxI))
                {
                  make_inputs[i]= 1;
                  minputs= true;
                }
            }
        }

      /* Needs to die gracefully if we are told to */
      // If I am exiting tell other players
      if (OCD.finish_offline[num_online] == 1)
        {
          nm= -1;
        }
      OCD.sacrifice_mutex[num_online].unlock();

      // Propose to other players what I have
      stringstream ss;
      ss << nm << " " << ns << " " << nb << " ";
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
              int nmt, nst, nbt, ni;
              is >> nmt >> nst >> nbt;
              nm= min(nm, nmt);
              ns= min(ns, nst);
              nb= min(nb, nbt);
              for (unsigned int i= 0; i < P.nplayers(); i++)
                {
                  is >> ni;
                  make_inputs[i]= max(make_inputs[i], ni);
                  if (make_inputs[i] == 1)
                    {
                      minputs= true;
                    }
                }
            }
        }
      if (nm == 0 && ns == 0 && nb == 0 && minputs == false)
        {
          sleep(1);
        }
    }

  bool exit= false;
  // Signal exit if anyone signalled exit
  if (nm == -1)
    {
      exit= true;
    }
  return exit;
}

void sacrifice_phase(int num_online, Player &P, int fake_sacrifice,
                     offline_control_data &OCD, const FHE_PK &pk,
                     const FHE_SK &sk, const FFT_Data &PTD,
                     FHE_Industry &industry,
                     int verbose)
{
  int nm, ns, nb;
  int rep= sacrifice_stat_sec / numBits(gfp::pr()) + 1;

  // Initialize PRSS stuff for IO production
  //   - We do IO production here as it is rarely done, and hence this
  //     keep the sacrifice phase thread busy whilst other threads
  //     do the main work
  PRSS prss(P);

  Open_Protocol OP;

  list<Share> a, b, c;
  list<gfp> opened;
  list<Share>::iterator first, last, it;
  list<gfp>::iterator it_g;
  vector<int> minputs(P.nplayers());
  bool exit= false, ready= false;
  while (0 == 0)
    {
      exit= propose_numbers_sacrifice(num_online, P, nm, ns, nb, minputs, OCD,
                                      verbose - 1);

      // Do the input bits first as the other operations wait until
      // enough data is ready to sacrifice
      for (unsigned int i= 0; i < P.nplayers() && !exit; i++)
        {
          if (minputs[i])
            {
              make_IO_data(P, fake_sacrifice, prss, i, a, opened, pk, sk, PTD, OP, industry);

              /* Add to queues */
              OCD.sacrifice_mutex[num_online].lock();
              OCD.totI+= a.size();
              it= SacrificeD[num_online].ID.ios[i].end();
              SacrificeD[num_online].ID.ios[i].splice(it, a);
              if (i == P.whoami())
                {
                  it_g= SacrificeD[num_online].ID.opened_ios.end();
                  SacrificeD[num_online].ID.opened_ios.splice(it_g, opened);
                }
              OCD.sacrifice_mutex[num_online].unlock();
            }
        }

      // Wait until we have enough offline data in this thread to be able to do
      // the required sacrifice
      if (!exit)
        {
          ready= false;
          while (ready == false && !exit)
            {
              ready= true;
              OCD.mult_mutex[num_online].lock();
              if ((int) TriplesD[num_online].ta.size() < nm)
                {
                  ready= false;
                }
              OCD.mult_mutex[num_online].unlock();

              OCD.square_mutex[num_online].lock();
              if ((int) SquaresD[num_online].sa.size() < rep * nb + ns)
                {
                  ready= false;
                }
              OCD.square_mutex[num_online].unlock();

              OCD.bit_mutex[num_online].lock();
              if ((int) BitsD[num_online].bb.size() < nb)
                {
                  ready= false;
                }
              OCD.bit_mutex[num_online].unlock();

              /* Wait if nothing to do */
              if (ready == false)
                {
                  sleep(1);
                }
            }
        }

      /* Do the actual work we need to do */
      if (nm > 0 && !exit)
        {
          if (verbose > 1)
            {
              printf("Sac: thread = %d : T: %d\n", num_online, nm);
              fflush(stdout);
            }

          OCD.mult_mutex[num_online].lock();

          first= TriplesD[num_online].ta.begin();
          last= TriplesD[num_online].ta.begin();
          advance(last, nm);
          a.clear();
          a.splice(a.begin(), TriplesD[num_online].ta, first, last);

          first= TriplesD[num_online].tb.begin();
          last= TriplesD[num_online].tb.begin();
          advance(last, nm);
          b.clear();
          b.splice(b.begin(), TriplesD[num_online].tb, first, last);

          first= TriplesD[num_online].tc.begin();
          last= TriplesD[num_online].tc.begin();
          advance(last, nm);
          c.clear();
          c.splice(c.begin(), TriplesD[num_online].tc, first, last);

          OCD.mult_mutex[num_online].unlock();

          sacrifice_phase_triples(P, fake_sacrifice, a, b, c, OP);

          /* Add to queues */
          OCD.sacrifice_mutex[num_online].lock();
          OCD.totm[num_online]+= a.size();
          it= SacrificeD[num_online].TD.ta.end();
          SacrificeD[num_online].TD.ta.splice(it, a);
          it= SacrificeD[num_online].TD.tb.end();
          SacrificeD[num_online].TD.tb.splice(it, b);
          it= SacrificeD[num_online].TD.tc.end();
          SacrificeD[num_online].TD.tc.splice(it, c);
          OCD.sacrifice_mutex[num_online].unlock();
        }
      if (ns > 0 && !exit)
        {
          if (verbose > 1)
            {
              printf("Sac: thread = %d : S: %d\n", num_online, ns);
              fflush(stdout);
            }

          OCD.square_mutex[num_online].lock();

          first= SquaresD[num_online].sa.begin();
          last= SquaresD[num_online].sa.begin();
          advance(last, ns);
          a.clear();
          a.splice(a.begin(), SquaresD[num_online].sa, first, last);

          first= SquaresD[num_online].sb.begin();
          last= SquaresD[num_online].sb.begin();
          advance(last, ns);
          b.clear();
          b.splice(b.begin(), SquaresD[num_online].sb, first, last);

          OCD.square_mutex[num_online].unlock();

          sacrifice_phase_squares(P, fake_sacrifice, a, b, OP);

          /* Add to queues */
          OCD.sacrifice_mutex[num_online].lock();
          OCD.tots[num_online]+= a.size();
          it= SacrificeD[num_online].SD.sa.end();
          SacrificeD[num_online].SD.sa.splice(it, a);
          it= SacrificeD[num_online].SD.sb.end();
          SacrificeD[num_online].SD.sb.splice(it, b);
          OCD.sacrifice_mutex[num_online].unlock();
        }
      if (nb > 0 && !exit)
        {
          if (verbose > 1)
            {
              printf("Sac: thread = %d : B: %d\n", num_online, nb);
              fflush(stdout);
            }

          OCD.bit_mutex[num_online].lock();

          first= BitsD[num_online].bb.begin();
          last= BitsD[num_online].bb.begin();
          advance(last, nb);
          a.clear();
          a.splice(a.begin(), BitsD[num_online].bb, first, last);

          OCD.bit_mutex[num_online].unlock();

          OCD.square_mutex[num_online].lock();

          first= SquaresD[num_online].sa.begin();
          last= SquaresD[num_online].sa.begin();
          advance(last, rep * nb);
          b.clear();
          b.splice(b.begin(), SquaresD[num_online].sa, first, last);

          first= SquaresD[num_online].sb.begin();
          last= SquaresD[num_online].sb.begin();
          advance(last, rep * nb);
          c.clear();
          c.splice(c.begin(), SquaresD[num_online].sb, first, last);

          OCD.square_mutex[num_online].unlock();

          sacrifice_phase_bits(P, fake_sacrifice, a, b, c, OP);

          /* Add to queues */
          OCD.sacrifice_mutex[num_online].lock();
          OCD.totb[num_online]+= a.size();
          it= SacrificeD[num_online].BD.bb.end();
          SacrificeD[num_online].BD.bb.splice(it, a);
          OCD.sacrifice_mutex[num_online].unlock();
        }
      OCD.sacrifice_mutex[num_online].lock();
      if (verbose > 0)
        { // Print this data it is useful for debugging stuff
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
          printf("Seconds per Mult Triple (all threads) %f : Total %f\n", avg, total);

          total= 0;
          for (unsigned int i= 0; i < SacrificeD.size(); i++)
            {
              total+= OCD.tots[i];
            }
          btotal+= total;
          avg= time / total;
          printf("Seconds per Square Pair (all threads) %f : Total %f\n", avg, total);

          total= 0;
          for (unsigned int i= 0; i < SacrificeD.size(); i++)
            {
              total+= OCD.totb[i];
            }
          btotal+= total;
          avg= time / total;
          printf("Seconds per Bit (all threads) %f : Total %f\n", avg, total);
          avg= time / btotal;
          printf("Seconds per `Thing` (all threads) %f : Total %f\n", avg, btotal);
        }
      /* Check whether we should kill the offline phase as we have now enough data */
      if (OCD.totm[num_online] > OCD.maxm && OCD.maxm != 0 &&
          OCD.tots[num_online] > OCD.maxs && OCD.maxs != 0 &&
          OCD.totb[num_online] > OCD.maxb && OCD.maxb != 0 &&
          OCD.totI > OCD.maxI)
        {
          OCD.finish_offline[num_online]= 1;
          printf("We have enough data to stop offline phase now\n");
          exit= true;
        }
      if (exit)
        {
          OCD.finished_offline[num_online]++;
        }
      OCD.sacrifice_mutex[num_online].unlock();
      if (exit)
        {
          printf("Exiting sacrifice phase : thread = %d\n", num_online);
          return;
        }
    }
}
