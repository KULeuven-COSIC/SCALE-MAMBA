/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _Schedule
#define _Schedule

/* This is a programmatic representation of a schedule
 * file. This enables the user of the system to produce
 * the equivalent of schedule files, without needing
 * to go via the file system.
 */

#include <sstream>
#include <vector>
using namespace std;

class Schedule
{
  unsigned int max_nt; // Stored for use in trigger function

public:
  /********************************************************
   * The following are public in case a system wants to   *
   * create Schedule objects programmatically             *
   ********************************************************/

  // Vector of programs as streams of instructions
  vector<stringstream> progs;
  vector<string> tnames;

  // This points to after the initial header on start
  // for a fresh version of a Schedule object
  stringstream i_schedule;

  void set_max_n_threads(unsigned int nt)
  {
    max_nt= nt;
  }
  unsigned int max_n_threads() const
  {
    return max_nt;
  }

  /********************************************************
   * The rest are used for non-programmatic creation      *
   * of Schedule objects                                  *
   ********************************************************/

  // Store of the program name being executed
  //   - In long and short form
  string progname, name;

  // Load progs and returns number of online threads needed
  //   - Loads from the program directory pname
  unsigned int Load_Programs(const string &pname);

  // This is the same version, but keeps the same progname
  //   - Used for RESTART
  unsigned int Load_Programs();
};

#endif
