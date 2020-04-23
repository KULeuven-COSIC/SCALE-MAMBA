/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _BaseCircuits
#define _BaseCircuits

/* This class holds in memory the set of basic circuits
 * we are going to use. This means we only read them
 * from disk once.
 */

#include "GC/Circuit.h"
#include "Math/bigint.h"
#include <map>

/* This maps the basic functions *directly* called in Instructions.cpp
 * to numbers in the map table
 */
enum Circ_Type {
  Adder64,
  Sub64,
  Mult2_64,
  Mult64,
  Divide64,
  Neg64,
  Zero_Equal,
  LSSS_to_GC,
  Number_Circuits
};

/* Mapping integers upto 65535 are reserved for the system.
 * Please add your own GC's using integers bigger than 65535
 * If you think the circuits are useful to others please
 * contact the maintainers and we might add them to future
 * releases in the reserved section
 *   
 * Our reserved numbers are currently (for non-directly called
 * circuits)
 *   100 AES-128
 *   101 AES-192
 *   102 AES-256
 *   103 Keccak_f
 *   104 sha256
 *   105 sha512
 *
 *   120 FP-add
 *   121 FP-mul
 *   122 FP-div
 *   123 FP-eq
 *   124 FP-f2i
 *   125 FP-i2f
 *   126 FP-sqrt
 *
 */
class Base_Circuits
{
public:
  map<int, Circuit> Circuits;

  /* These are only used for the non-directly called
   * circuits, means they are only fetched from disk
   * when first used
   */
  map<int, bool> loaded;
  map<int, string> location;

  bool convert_ok; // Signals whether we can do GC <-> LSSS conversion or not

  void initialize(const bigint &p);

  // Checks if the indirect circuit is loaded, it not it loads it */
  void check(int num);
};

#endif
