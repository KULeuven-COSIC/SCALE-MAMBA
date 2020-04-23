/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
/*
 * BitMatrix.h
 *
 *  Created on: 31 Jul 2015
 *      Author: csmksk
 */

#ifndef OT_BITMATRIX_H_
#define OT_BITMATRIX_H_

#include <emmintrin.h>
#include <vector>

#include "BitVector.h"
#include "Math/gf2n.h"
#include "Tools/random.h"

using namespace std;

union square128 {
  __m128i rows[128];
  uint8_t bytes[128][16];
  int16_t doublebytes[128][8];
  int32_t words[128][4];

  bool get_bit(int x, int y)
  {
    return (bytes[x][y / 8] >> (y % 8)) & 1;
  }

  void set_zero();

  square128 &operator^=(square128 &other);
  square128 &operator^=(__m128i *other);
  square128 &operator^=(BitVector &other);
  bool operator==(square128 &other);

  square128 &add(square128 &other);
  square128 &sub(square128 &other);
  square128 &rsub(square128 &other);
  square128 &sub(__m128i *other);

  void randomize(PRNG &G);
  void randomize(int row, PRNG &G);
  void conditional_add(BitVector &conditions, square128 &other, int offset);
  void transpose();
  void to(gf2n &result);

  void check_transpose(square128 &dual, int i, int k);
  void print(int i, int k);
  void print();

  void output(ostream &s, bool human= false) const;
  void input(istream &s, bool human= false);
};

class BitMatrixSlice;

class BitMatrix
{
public:
  vector<square128> squares;

  BitMatrix()
  {
  }
  BitMatrix(int length);
  void resize(int length);
  int size();

  BitMatrix &add(BitMatrix &other);
  BitMatrix &sub(BitMatrix &other);
  BitMatrix &rsub(BitMatrixSlice &other);
  BitMatrix &sub(BitVector &other);
  bool operator==(BitMatrix &other);
  bool operator!=(BitMatrix &other);

  void randomize(PRNG &G);
  void randomize(int row, PRNG &G);
  void transpose();

  void check_transpose(BitMatrix &dual);
  void print_side_by_side(BitMatrix &other);
  void print_conditional(BitVector &conditions);

  void to(vector<BitVector> &output);
  // Following assumes V is same size as a row
  void set_row(unsigned int row, const BitVector &V);
  // Following only fills row up with the first bit of V
  void set_row_up_to(unsigned int row, const BitVector &V);
  BitVector get_row(unsigned int row);
  void zero_row(unsigned int row);

  void output(ostream &s, bool human= false) const;
  void input(istream &s, bool human= false);

  // Convert row r of block c into a gf2n element
  gf2n to_gf2n(int r, int c) const;
};

class BitMatrixSlice
{
  friend class BitMatrix;

  BitMatrix &bm;
  size_t start, end;

public:
  BitMatrixSlice(BitMatrix &bm, size_t start, size_t size);

  BitMatrixSlice &rsub(BitMatrixSlice &other);
  BitMatrixSlice &add(BitVector &other, int repeat= 1);

  void randomize(int row, PRNG &G);
  void conditional_add(BitVector &conditions, BitMatrix &other, bool useOffset= false);
  void transpose();

  void print();
};

#endif /* OT_BITMATRIX_H_ */
