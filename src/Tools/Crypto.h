/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _Crypto
#define _Crypto

#include <string>
using namespace std;

#include "Math/gfp.h"
#include "System/Player.h"
#include "random.h"

/* This file defines various helper crypto routines we need */

/* SHA256 based hash of a string */
string Hash(const string &data);

/* Define the hash based commitment scheme */
void Commit(string &comm, string &open, const string &data, PRNG &G);
bool Open(string &data, const string &comm, const string &open);

/* Protocols for agreeing on random data */

/* Create a random gfp element agreed between the players */
void Create_Random(gfp &ans, Player &P, int connectin= 0);
/* Produce a random seed of length len */
void Create_Random_Seed(uint8_t *seed, int len, Player &P, int connection= 0);

/* Protocols to commit and open to data */

/* This runs the Commit and Open Protocol for data[i][j] of type T
 *   0 <= i < data.size()
 *   0 <= j < num_players
 * On input data[i][j] is only defined for j=me
 *
 * Check defines whether we do broadcast checking
 */
template<class T>
void Commit_And_Open(vector<vector<T>> &data, Player &P, bool check= false, int connection= 0);

/* This runs the Commit and Open Protocol for data[j] of type T
 *   0 <= j < num_players
 * On input data[j] is only defined for j=me
 *
 * Check defines whether we do broadcast checking
 */
template<class T>
void Commit_And_Open(vector<T> &data, Player &P, bool check= false, int connection= 0);

#endif
