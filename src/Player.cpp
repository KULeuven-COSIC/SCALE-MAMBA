/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <pthread.h>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>
using namespace std;

#include "Offline/FHE_Factory.h"
#include "Offline/offline_phases.h"
#include "Online/Online.h"
#include "System/Networking.h"
#include "config.h"

#include "LSSS/Open_Protocol.h"
#include "Processor/Processor.h"

#include "Tools/ezOptionParser.h"
using namespace ez;

class thread_info
{
public:
  int thread_num;
  SystemData *SD;
  offline_control_data *OCD;
  SSL_CTX *ctx;
  int me;
  vector<int> csockets;

  // FHE Data
  FHE_PK *pk;
  FHE_SK *sk;
  FFT_Data *PTD;

  Machine *machine; // Pointer to the machine
};

// We have 5 threads per online phase
//   - Online
//   - Sacrifice (and input tuple production)
//   - Mult Triple Production
//   - Square Pair Production
//   - Bit Production
vector<pthread_t> threads;

// Forward declarations to make code easier to read
void *Main_Func(void *ptr);
void online_phase(int online_num, Player &P, offline_control_data &OCD,
                  Machine &machine);

vector<triples_data> TriplesD;
vector<squares_data> SquaresD;
vector<bits_data> BitsD;
vector<sacrificed_data> SacrificeD;

void Usage(ezOptionParser &opt)
{
  string usage;
  opt.getUsage(usage);
  cout << usage;
}

int main(int argc, const char *argv[])
{
  ezOptionParser opt;

  opt.overview= "Usage of the main Player.x program features.";
  opt.syntax= "./Player.x [OPTIONS] <playernum> <progname>\n";
  opt.example= "./Player-Online.x -m new 0 sample-prog\n"
               "./Player-Online.x -pn 13000 -min 10,10,10 -max "
               "1000,1000,10000 0 sample-prog\n"
               "<progname> defines the *directory* where the program has been "
               "compiled to\n";

  opt.add("5000", // Default.
          0,      // Required?
          1,      // Number of args expected.
          0,      // Delimiter if expecting multiple args.
          "Port number base to attempt to start connections from (default: "
          "5000)",       // Help description.
          "-pnb",        // Flag token.
          "-portnumbase" // Flag token.
          );
  opt.add("",  // Default.
          0,   // Required?
          -1,  // Number of args expected.
          ',', // Delimiter if expecting multiple args.
          "List of portnums, one per player\n\t"
          "Must be the same on each machine\n\t"
          "This option overides any defined portnumbase", // Help description.
          "-pns",                                         // Flag token.
          "-portnums"                                     // Flag token.
          );
  opt.add("empty", // Default.
          0,       // Required?
          1,       // Number of args expected.
          0,       // Delimiter if expecting multiple args.
          "Where to obtain memory, old|empty (default: empty)\n\t"
          "old: reuse previous memory in Memory-P<i>\n\t"
          "empty: create new empty memory", // Help description.
          "-m",                             // Flag token.
          "-memory"                         // Flag token.
          );
  opt.add("0,0,0", // Default.
          0,       // Required?
          3,       // Number of args expected.
          ',',     // Delimiter if expecting multiple args.
          "Do not start online phase until we have m triples, s squares and b "
          "bits\n"
          "\tMust be the same on each machine", // Help description.
          "-min"                                // Flag token.
          );
  opt.add("0,0,0", // Default.
          0,       // Required?
          3,       // Number of args expected.
          ',',     // Delimiter if expecting multiple args.
          "Stop the offline phase when m triples, s squares and b bits have "
          "been produced\n"
          "\tZero argument means infinity here\n"
          "\tMust be the same on each machine", // Help description.
          "-max"                                // Flag token.
          );

  opt.parse(argc, argv);

  int my_number;
  string progname;
  string memtype;
  int portnumbase;
  vector<int> minimums, maximums;

  vector<string *> allArgs(opt.firstArgs);
  allArgs.insert(allArgs.end(), opt.lastArgs.begin(), opt.lastArgs.end());
  string usage;
  vector<string> badOptions;
  unsigned int i;

  if (allArgs.size() != 3)
    {
      cerr << "ERROR: incorrect number of arguments to Player.x\n";
      cerr << "Arguments given were:\n";
      for (unsigned int j= 1; j < allArgs.size(); j++)
        cout << "'" << *allArgs[j] << "'" << endl;
      opt.getUsage(usage);
      cout << usage;
      return 1;
    }
  else
    {
      my_number= atoi(allArgs[1]->c_str());
      progname= *allArgs[2];
    }

  if (!opt.gotRequired(badOptions))
    {
      for (i= 0; i < badOptions.size(); ++i)
        cerr << "ERROR: Missing required option " << badOptions[i] << ".";
      opt.getUsage(usage);
      cout << usage;
      return 1;
    }

  if (!opt.gotExpected(badOptions))
    {
      for (i= 0; i < badOptions.size(); ++i)
        cerr << "ERROR: Got unexpected number of arguments for option "
             << badOptions[i] << ".";
      opt.getUsage(usage);
      cout << usage;
      return 1;
    }

  class offline_control_data OCD;

  vector<int> pns;
  opt.get("-portnumbase")->getInt(portnumbase);
  opt.get("-pns")->getInts(pns);
  opt.get("-memory")->getString(memtype);
  opt.get("-min")->getInts(minimums);
  opt.get("-max")->getInts(maximums);
  OCD.minm= minimums[0];
  OCD.mins= minimums[1];
  OCD.minb= minimums[2];
  OCD.maxm= maximums[0];
  OCD.maxs= maximums[1];
  OCD.maxb= maximums[2];

  cout << "(Min,Max) number of ...\n";
  cout << "\t(" << OCD.minm << ",";
  if (OCD.maxm == 0)
    {
      cout << "infty";
    }
  else
    {
      cout << OCD.maxm;
    }
  cout << ") multiplication triples" << endl;

  cout << "\t(" << OCD.mins << ",";
  if (OCD.maxs == 0)
    {
      cout << "infty";
    }
  else
    {
      cout << OCD.maxs;
    }
  cout << ") square pairs" << endl;

  cout << "\t(" << OCD.minb << ",";
  if (OCD.maxb == 0)
    {
      cout << "infty";
    }
  else
    {
      cout << OCD.maxb;
    }
  cout << ") bits" << endl;

  // Initialise the system data
  SystemData SD;

  // Initialise the secret sharing data and the gfp field
  ifstream inp("Data/SharingData.txt");
  if (inp.fail())
    {
      throw file_error("Data/SharingData.txt");
    }
  bigint p;
  inp >> p;
  cout << "\n\np=" << p << endl;
  gfp::init_field(p);
  ShareData ShD;
  inp >> ShD;
  inp.close();
  if (ShD.M.nplayers() != SD.n)
    {
      throw data_mismatch();
    }
  if (SD.n != pns.size() && pns.size() != 0)
    {
      throw data_mismatch();
    }
  if (SD.fake_offline == 1)
    {
      ShD.Otype= Fake;
    }
  Share::init_share_data(ShD);

  /* Now initialize the FHE data if needed */
  FHE_Params params;
  FFT_Data PTD;
  Ring Rg;
  if (Share::SD.type == Full)
    {
      stringstream ss;
      ss << "Data/FHE-Key-" << my_number << ".key";
      inp.open(ss.str().c_str());
      bigint p0, p1, pr;
      int hwt, N;
      inp >> N >> p0 >> p1 >> pr >> hwt;
      if (p != pr)
        {
          throw data_mismatch();
        }

      // Initialize data
      Rg.initialize(2 * N);
      PTD.init(Rg, gfp::get_ZpD());
      params.set(Rg, p0, p1, hwt, SD.n);
    }
  else
    { // Make dummy params to avoid having pk/sk as pointers
      Rg.initialize(8);
      params.set(Rg, 17, 17, 1, false);
    }
  FHE_PK pk(params, p);
  FHE_SK sk(params, p);
  if (Share::SD.type == Full)
    {
      char ch;
      do
        {
          inp.get(ch);
        }
      while (ch != ':');
      inp >> pk >> sk;
      inp.close();
    }

  SSL_CTX *ctx;
  Init_SSL_CTX(ctx, my_number, SD);

  if (my_number < 0 || (unsigned int) my_number >= SD.n)
    {
      throw data_mismatch();
    }

  Machine machine;
  machine.SetUp_Memory(my_number, memtype);

  // Here you configure the IO in the machine
  //  - This depends on what IO machinary you are using
  //  - Here we are just using the simple IO class
  machine.IO.init(cin, cout, true);

  int no_online_threads= machine.Load_Programs(progname);

  OCD.resize(no_online_threads);

  TriplesD.resize(no_online_threads);
  SquaresD.resize(no_online_threads);
  BitsD.resize(no_online_threads);
  SacrificeD.resize(no_online_threads);
  for (int i= 0; i < no_online_threads; i++)
    {
      SacrificeD[i].initialize(SD.n);
    }

  int nthreads= 5 * no_online_threads;
  int tnthreads= nthreads;
  if (Share::SD.type == Full)
    {
      tnthreads++;
    } // Add one thread to do FHE ZKPoKs

  /* Initialize the networking TCP sockets */
  int ssocket;
  vector<vector<int>> csockets(tnthreads, vector<int>(SD.n));
  vector<int> portnum(SD.n);
  for (unsigned int i= 0; i < SD.n; i++)
    {
      if (pns.size() == 0)
        {
          portnum[i]= portnumbase + i;
        }
      else
        {
          portnum[i]= pns[i];
        }
    }
  Get_Connections(ssocket, csockets, portnum, my_number, SD);
  printf("All connections now done\n");

  printf("Setting up threads\n");
  fflush(stdout);
  threads.resize(tnthreads);
  vector<thread_info> tinfo(tnthreads);
  for (int i= 0; i < nthreads; i++)
    {
      tinfo[i].thread_num= i;
      tinfo[i].SD= &SD;
      tinfo[i].OCD= &OCD;
      tinfo[i].ctx= ctx;
      tinfo[i].me= my_number;
      tinfo[i].csockets= csockets[i];
      tinfo[i].machine= &machine;
      tinfo[i].pk= &pk;
      tinfo[i].sk= &sk;
      tinfo[i].PTD= &PTD;
      if (pthread_create(&threads[i], NULL, Main_Func, &tinfo[i]))
        {
          throw C_problem("Problem spawning thread");
        }
    }
  if (tnthreads != nthreads)
    {
      i= nthreads;
      tinfo[i].thread_num= 1000;
      tinfo[i].SD= &SD;
      tinfo[i].OCD= &OCD;
      tinfo[i].ctx= ctx;
      tinfo[i].me= my_number;
      tinfo[i].csockets= csockets[nthreads];
      tinfo[i].machine= &machine;
      tinfo[i].pk= &pk;
      tinfo[i].sk= &sk;
      tinfo[i].PTD= &PTD;
      pthread_create(&threads[i], NULL, Main_Func, &tinfo[i]);
    }

  // Get all online threads in sync
  machine.Synchronize();

  // Now run the programs
  machine.run();

  printf("Waiting for all clients to finish\n");
  fflush(stdout);
  for (int i= 0; i < tnthreads; i++)
    {
      pthread_join(threads[i], NULL);
    }

  machine.Dump_Memory(my_number);

  Destroy_SSL_CTX(ctx);

  Close_Connections(ssocket, csockets, my_number);

  printf("End of prog\n");
  fflush(stdout);
}

void *Main_Func(void *ptr)
{
  thread_info *tinfo= (thread_info *) ptr;
  int num= tinfo->thread_num;
  int me= tinfo->me;
  printf("I am player %d in thread %d\n", me, num);
  fflush(stdout);

  Player P(me, *(tinfo->SD), num, (tinfo->ctx), (tinfo->csockets));
  if (Share::SD.nmacs != 0)
    {
      P.load_mac_keys(Share::SD.nmacs);
    }
  printf("Set up player %d in thread %d \n", me, num);
  fflush(stdout);

  if (num < 1000)
    {
      int num5= num % 5;
      int num_online= (num - num5) / 5;
      switch (num5)
        {
          case 0:
            mult_phase(num_online, P, *(tinfo->OCD), *(tinfo->pk), *(tinfo->sk),
                       *(tinfo->PTD));
            break;
          case 1:
            square_phase(num_online, P, *(tinfo->OCD), *(tinfo->pk), *(tinfo->sk),
                         *(tinfo->PTD));
            break;
          case 2:
            bit_phase(num_online, P, *(tinfo->OCD), *(tinfo->pk), *(tinfo->sk),
                      *(tinfo->PTD));
            break;
          case 3:
            sacrifice_phase(num_online, P, (tinfo->SD)->fake_sacrifice, *(tinfo->OCD),
                            *(tinfo->pk), *(tinfo->sk), *(tinfo->PTD));
            break;
          case 4:
            online_phase(num_online, P, *(tinfo->OCD), *(tinfo)->machine);
            break;
        }
    }
  else
    {
      FHE_Factory(P, *(tinfo->OCD), *(tinfo->pk), *(tinfo->PTD));
    }
  return 0;
}
