/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _SystemData
#define _SystemData

/* This structure holds the system data which was
 * setup using the procedure Setup
 */

#include <istream>
#include <string>
#include <vector>
using namespace std;

class SystemData
{
  void init(unsigned int numplayers, const string &RootCertName,
            const vector<string> &IP_Numbers,
            const vector<string> &PlayerCertFiles,
            const vector<string> &PlayerNames,
            int fake_off, int fake_sac);

  void init_from_stream(istream &inp, const string &resource_name);

public:
  unsigned int n;
  string RootCRT;
  vector<string> IP;        // IP Addresses
  vector<string> PlayerCRT; // Player Certificate File
  vector<string> PlayerCN;  // Player Common Name

  int fake_offline;
  int fake_sacrifice;

  SystemData(unsigned int numplayers, const string &RootCertName,
             const vector<string> &IP_Numbers,
             const vector<string> &PlayerCertFiles,
             const vector<string> &PlayerNames,
             int fake_off= 0, int fake_sac= 0)
  {
    init(numplayers, RootCertName,
         IP_Numbers, PlayerCertFiles, PlayerNames,
         fake_off, fake_sac);
  }

  SystemData(const string &NetworkDataFileName);

  SystemData(istream &NetworkData, const string &resource_name);
};

#endif
