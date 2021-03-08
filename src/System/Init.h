/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _Initialisation
#define _Initialisation

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

#include "openssl/err.h"
#include "openssl/ssl.h"

#include "FHE/FHE_Keys.h"
#include "FHE/FHE_Params.h"
#include "GC/SimplifyCircuit.h"
#include "LSSS/Share.h"
#include "LSSS/ShareData.h"
#include "LSSS/ShareData2.h"
#include "OT/aBitVector.h"
#include "config.h"

struct SecretSharing
{
  ShareData SD;
  ShareData2 SD2;
  vector<vector<gfp>> mkeys;
};

enum class ReplicationMode {
  // Set of unqualified sets
  UnqualifiedSets= 1,
  // Set of qualified sets
  QualifiedSets= 2,
  // Replicated threshold structure
  Threshold= 3,
};

void init_certs(std::istream &cin);
void init_conversion();

/* Note this may update p in the case of FHE parameters */
SecretSharing init_secret_sharing(ShareType lsss, bigint &p, int lg2p,
                                  unsigned int t, unsigned int n,
                                  vector<unsigned int> ns,
                                  vector<vector<gfp>> &Gen,
                                  ReplicationMode mode, OfflineType offline_type, imatrix Sets);

void enter_sets(std::istream &cin, imatrix &Sets, bool unqualified, unsigned int n);

#endif
