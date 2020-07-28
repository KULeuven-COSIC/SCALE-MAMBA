/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <memory>
#include <pthread.h>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>
using namespace std;

#include "System/Networking.h"
#include "System/RunTime.h"
#include "config.h"

#include "Tools/ezOptionParser.h"
using namespace ez;

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
          "Port number base to attempt to start connections"
          " from (default: 5000)", // Help description.
          "-pnb",                  // Flag token.
          "-portnumbase"           // Flag token.
  );
  opt.add("",  // Default.
          0,   // Required?
          -1,  // Number of args expected.
          ',', // Delimiter if expecting multiple args.
          "List of portnums, one per player\n"
          "\tMust be the same on each machine\n"
          "\tThis option overides any defined portnumbase", // Help description.
          "-pns",                                           // Flag token.
          "-portnums"                                       // Flag token.
  );
  opt.add("0", // Default.
          0,   // Required?
          1,   // Number of args expected.
          0,   // Delimiter if expecting multiple args.
          "Set the verbose level.\n"
          "\tIf positive the higher the value the more\n"
          "\tstuff printed for offline.\n"
          "\tIf negative we print verbose for the online\n"
          "\tphase",  // Help description.
          "-verbose", // Flag token.
          "-v"        // Flag token.
  );
  opt.add("0", // Default.
          0,   // Required?
          0,   // Number of args expected.
          0,   // Delimiter if expecting multiple args.
          "Disable OT production. Will be faster/less memory, but some operations will not work.\n",
          "-dOT" // Flag token.
  );
  opt.add("empty", // Default.
          0,       // Required?
          1,       // Number of args expected.
          0,       // Delimiter if expecting multiple args.
          "Where to obtain memory, old|empty (default: empty)\n"
          "\told: reuse previous memory in Memory-P<i>\n"
          "\tempty: create new empty memory", // Help description.
          "-m",                               // Flag token.
          "-memory"                           // Flag token.
  );
  opt.add("0,0,0", // Default.
          0,       // Required?
          3,       // Number of args expected.
          ',',     // Delimiter if expecting multiple args.
          "Do not start online phase until we have\n"
          "\tm triples, s squares and b bits\n"
          "\tMust be the same on each machine", // Help description.
          "-min"                                // Flag token.
  );
  opt.add("0,0,0", // Default.
          0,       // Required?
          3,       // Number of args expected.
          ',',     // Delimiter if expecting multiple args.
          "Stop the offline phase when\n"
          "\tm triples, s squares and b bits\n"
          "have been produced\n"
          "\tZero argument means infinity here\n"
          "\tMust be the same on each machine", // Help description.
          "-max"                                // Flag token.
  );
  opt.add("0", // Default.
          0,   // Required?
          1,   // Number of args expected.
          0,   // Delimiter if expecting multiple args.
          "Similar to -max but for the input queue only",
          "-maxI" // Flag token.
  );
  opt.add("2", // Default.
          0,   // Required?
          1,   // Number of args expected.
          0,   // Delimiter if expecting multiple args.
          "Number of FHE Factories we run in parallel\n",
          "-f",           // Flag token.
          "-fhefactories" // Flag token.
  );

  opt.parse(argc, argv);

  unsigned int my_number;
  int verbose, fhefacts;
  string progname;
  string memtype;
  unsigned int portnumbase;
  bool OT_disable= false;

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
      my_number= (unsigned int) atoi(allArgs[1]->c_str());
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

  vector<int> pns, minimums, maximums;
  int te;
  opt.get("-portnumbase")->getInt(te);
  portnumbase= (unsigned int) te;
  opt.get("-memory")->getString(memtype);
  opt.get("-verbose")->getInt(verbose);
  opt.get("-fhefactories")->getInt(fhefacts);
  opt.get("-pns")->getInts(pns);
  opt.get("-min")->getInts(minimums);
  opt.get("-max")->getInts(maximums);

  if (opt.isSet("-dOT"))
    {
      OT_disable= true;
    }

  /******************************************
   *      Setup offline_control_data OCD    *
   *  -Note if you want to programmatically *
   *   chnage max_triples_sacrifice etc do  *
   *   it here in the call to the OCD       *
   *   constructor                          *
   ******************************************/
  offline_control_data OCD;
  OCD.minm= (unsigned int) minimums[0];
  OCD.mins= (unsigned int) minimums[1];
  OCD.minb= (unsigned int) minimums[2];
  OCD.maxm= (unsigned int) maximums[0];
  OCD.maxs= (unsigned int) maximums[1];
  OCD.maxb= (unsigned int) maximums[2];
  opt.get("-maxI")->getInt(te);
  OCD.maxI= (unsigned int) te;

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

  /*************************************
   *     Initialise the system data    *
   *************************************/
  SystemData SD("Data/NetworkData.txt");
  if (my_number >= SD.n)
    {
      throw data_mismatch();
    }

  if (SD.n != pns.size() && pns.size() != 0)
    {
      throw data_mismatch();
    }

  /*************************************
   *    Initialize the portnums        *
   *************************************/
  vector<unsigned int> portnum(SD.n);
  for (unsigned int i= 0; i < SD.n; i++)
    {
      if (pns.size() == 0)
        {
          portnum[i]= portnumbase + i;
        }
      else
        {
          portnum[i]= (unsigned int) pns[i];
        }
    }

  /*************************************
   * Initialise the secret sharing     *
   * data and the gfp field data       *
   *************************************/
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
  if (SD.fake_offline == 1)
    {
      ShD.Otype= Fake;
    }
  Share::init_share_data(ShD);

  // Fix this here so when we instantiate the memory
  // below in creating the machine we know what size
  // to make the default sregint memory
  aBit::set_nplayers(SD.n);

  /*************************************
   *    Load in MAC keys (if any)      *
   *************************************/
  vector<gfp> MacK(Share::SD.nmacs);
  stringstream ss;
  ss << "Data/MKey-" << my_number << ".key";
  inp.open(ss.str().c_str());
  if (inp.fail())
    {
      throw file_error(ss.str());
    }
  for (unsigned int i= 0; i < Share::SD.nmacs; i++)
    {
      inp >> MacK[i];
    }
  inp.close();

  /*************************************
   * Now initialize the FHE data       *
   *    - If needed                    *
   *************************************/
  FHE_Params params;
  FFT_Data PTD;
  Ring Rg;
  if (Share::SD.type == Full)
    {
      stringstream ss;
      ss << "Data/FHE-Key-" << my_number << ".key";
      inp.open(ss.str().c_str());
      if (inp.fail())
        {
          throw file_error(ss.str());
        }
      bigint p0, p1, pr;
      unsigned int hwt, N;
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

  /*************************************
   * Load FHE keys                     *
   *************************************/
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

  /* Initialize SSL */
  SSL_CTX *ctx;
  Init_SSL_CTX(ctx, my_number, SD);

  /* Initialize the machine */
  Machine machine;
  if (verbose < 0)
    {
      machine.set_verbose();
      verbose= 0;
    }
  machine.SetUp_Memory(my_number, memtype);

  // Here you configure the IO in the machine
  //  - This depends on what IO machinary you are using
  //  - Here we are just using the simple IO class
  unique_ptr<Input_Output_Simple> io(new Input_Output_Simple);
  io->init(cin, cout, true);
  machine.Setup_IO(std::move(io));

  // Load the initial tapes for the first program into the schedule
  unsigned int no_online_threads= machine.schedule.Load_Programs(progname);

  unsigned int number_FHE_threads= 0;
  if (Share::SD.type == Full && (SD.fake_offline == 0 || SD.fake_sacrifice == 0))
    {
      number_FHE_threads= fhefacts;
    }

  Run_Scale(my_number, no_online_threads,
            PTD, pk, sk, MacK,
            ctx, portnum,
            SD, machine, OCD,
            number_FHE_threads,
            OT_disable,
            verbose);

  machine.Dump_Memory(my_number);

  Destroy_SSL_CTX(ctx);

  printf("End of prog\n");
  fflush(stdout);
}
