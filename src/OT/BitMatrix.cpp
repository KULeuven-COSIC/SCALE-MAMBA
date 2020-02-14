/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
/*
 * BitMatrix.cpp
 *
 *  Created on: 31 Jul 2015
 *      Author: csmksk
 */

#include <immintrin.h>
#include <mpirxx.h>
#include <smmintrin.h>

#include "BitMatrix.h"
#include "Math/gf2n.h"

union matrix16x8 {
  __m128i whole;
  uint8_t rows[16];

  bool get_bit(int x, int y)
  {
    return (rows[x] >> y) & 1;
  }

  void input(square128 &input, int x, int y);
  void transpose(square128 &output, int x, int y);
};

class square16
{
public:
  // 16x16 in two halves, 128 bits each
  matrix16x8 halves[2];

  bool get_bit(int x, int y)
  {
    return halves[y / 8].get_bit(x, y % 8);
  }

  void input(square128 &output, int x, int y);
  void transpose(square128 &output, int x, int y);

  void check_transpose(square16 &dual);
  void print();
};

#ifdef __clang__
#define UNROLL_LOOPS
#else
#define UNROLL_LOOPS __attribute__((optimize("unroll-loops")))
#endif

UNROLL_LOOPS
inline void matrix16x8::input(square128 &input, int x, int y)
{
  for (int l= 0; l < 16; l++)
    rows[l]= input.bytes[16 * x + l][y];
}

UNROLL_LOOPS
inline void square16::input(square128 &input, int x, int y)
{
  for (int i= 0; i < 2; i++)
    halves[i].input(input, x, 2 * y + i);
}

UNROLL_LOOPS
inline void matrix16x8::transpose(square128 &output, int x, int y)
{
  for (int j= 0; j < 8; j++)
    {
      int row= _mm_movemask_epi8(whole);
      whole= _mm_slli_epi64(whole, 1);

      // _mm_movemask_epi8 uses most significant bit, hence +7-j
      output.doublebytes[8 * x + 7 - j][y]= row;
    }
}

UNROLL_LOOPS
inline void square16::transpose(square128 &output, int x, int y)
{
  for (int i= 0; i < 2; i++)
    halves[i].transpose(output, 2 * x + i, y);
}

#ifdef __AVX2__
union matrix32x8 {
  __m256i whole;
  uint8_t rows[32];

  void input(square128 &input, int x, int y);
  void transpose(square128 &output, int x, int y);
};

class square32
{
public:
  matrix32x8 quarters[4];

  void input(square128 &input, int x, int y);
  void transpose(square128 &output, int x, int y);
};

UNROLL_LOOPS
inline void matrix32x8::input(square128 &input, int x, int y)
{
  for (int l= 0; l < 32; l++)
    rows[l]= input.bytes[32 * x + l][y];
}

UNROLL_LOOPS
inline void square32::input(square128 &input, int x, int y)
{
  for (int i= 0; i < 4; i++)
    quarters[i].input(input, x, 4 * y + i);
}

UNROLL_LOOPS
inline void matrix32x8::transpose(square128 &output, int x, int y)
{
  for (int j= 0; j < 8; j++)
    {
      int row= _mm256_movemask_epi8(whole);
      whole= _mm256_slli_epi64(whole, 1);

      // _mm_movemask_epi8 uses most significant bit, hence +7-j
      output.words[8 * x + 7 - j][y]= row;
    }
}

UNROLL_LOOPS
inline void square32::transpose(square128 &output, int x, int y)
{
  for (int i= 0; i < 4; i++)
    quarters[i].transpose(output, 4 * x + i, y);
}
#endif

#ifdef __AVX2__
typedef square32 subsquare;
#define N_SUBSQUARES 4
#else
typedef square16 subsquare;
#define N_SUBSQUARES 8
#endif

UNROLL_LOOPS
void square128::transpose()
{
  for (int j= 0; j < N_SUBSQUARES; j++)
    for (int k= 0; k < j; k++)
      {
        subsquare a, b;
        a.input(*this, k, j);
        b.input(*this, j, k);
        a.transpose(*this, j, k);
        b.transpose(*this, k, j);
      }

  for (int j= 0; j < N_SUBSQUARES; j++)
    {
      subsquare a;
      a.input(*this, j, j);
      a.transpose(*this, j, j);
    }
}

void square128::randomize(PRNG &G)
{
  G.get_random_bytes((uint8_t *) &rows, sizeof(rows));
}

void square128::randomize(int row, PRNG &G)
{
  rows[row]= G.get_doubleword();
}

void square128::conditional_add(BitVector &conditions, square128 &other, int offset)
{
  for (int i= 0; i < 128; i++)
    if (conditions.get_bit(128 * offset + i))
      rows[i]^= other.rows[i];
}

void square128::to(gf2n &result)
{
  int128 high, low;
  for (int i= 0; i < 128; i++)
    {
      low^= int128(rows[i]) << i;
      high^= int128(rows[i]) >> (128 - i);
    }
  result.reduce(high, low);
}

void square128::check_transpose(square128 &dual, int i, int k)
{
  for (int j= 0; j < 16; j++)
    for (int l= 0; l < 16; l++)
      if (get_bit(16 * i + j, 16 * k + l) != dual.get_bit(16 * k + l, 16 * i + j))
        {
          cout << "Error in 16x16 square (" << i << "," << k << ")" << endl;
          print(i, k);
          dual.print(i, k);
          exit(1);
        }
}

void square16::print()
{
  for (int i= 0; i < 2; i++)
    {
      for (int j= 0; j < 8; j++)
        {
          for (int k= 0; k < 2; k++)
            {
              for (int l= 0; l < 8; l++)
                cout << halves[k].get_bit(8 * i + j, l);
              cout << " ";
            }
          cout << endl;
        }
      cout << endl;
    }
}

void square128::print(int i, int k)
{
  square16 a;
  a.input(*this, i, k);
  a.print();
}

void square128::print()
{
  for (int i= 0; i < 128; i++)
    {
      for (int j= 0; j < 128; j++)
        cout << get_bit(i, j);
      cout << endl;
    }
}

void square128::set_zero()
{
  for (int i= 0; i < 128; i++)
    rows[i]= _mm_setzero_si128();
}

square128 &square128::operator^=(square128 &other)
{
  for (int i= 0; i < 128; i++)
    rows[i]^= other.rows[i];
  return *this;
}

square128 &square128::add(square128 &other)
{
  return *this^= other;
}

square128 &square128::sub(square128 &other)
{
  return *this^= other;
}

square128 &square128::rsub(square128 &other)
{
  return *this^= other;
}

square128 &square128::operator^=(__m128i *other)
{
  __m128i value= _mm_loadu_si128(other);
  for (int i= 0; i < 128; i++)
    rows[i]^= value;
  return *this;
}

square128 &square128::sub(__m128i *other)
{
  return *this^= other;
}

square128 &square128::operator^=(BitVector &other)
{
  return *this^= (__m128i *) other.get_ptr();
}

bool square128::operator==(square128 &other)
{
  for (int i= 0; i < 128; i++)
    {
      __m128i tmp= rows[i] ^ other.rows[i];
      if (not _mm_test_all_zeros(tmp, tmp))
        return false;
    }
  return true;
}

void square128::output(ostream &s, bool human) const
{
  for (int i= 0; i < 128; i++)
    {
      if (human)
        {
          for (unsigned int j= 0; j < 16; j++)
            {
              s << int(bytes[i][j]) << " ";
            }
          s << endl;
        }
      else
        {
          s.write((char *) bytes[i], 16);
        }
    }
}

void square128::input(istream &s, bool human)
{
  for (int i= 0; i < 128; i++)
    {
      if (human)
        {
          for (unsigned int j= 0; j < 16; j++)
            {
              s >> bytes[i][j];
            }
        }
      else
        {
          s.read((char *) bytes[i], 16);
        }
    }
}

BitMatrix::BitMatrix(int length)
{
  resize(length);
}

void BitMatrix::resize(int length)
{
  if (length % 128 != 0)
    throw invalid_length();
  squares.resize(length / 128);
}

int BitMatrix::size()
{
  return squares.size() * 128;
}

BitMatrix &BitMatrix::add(BitMatrix &other)
{
  if (squares.size() != other.squares.size())
    throw invalid_length();
  for (size_t i= 0; i < squares.size(); i++)
    squares[i].add(other.squares[i]);
  return *this;
}

BitMatrix &BitMatrix::sub(BitMatrix &other)
{
  if (squares.size() != other.squares.size())
    throw invalid_length();
  for (size_t i= 0; i < squares.size(); i++)
    squares[i].sub(other.squares[i]);
  return *this;
}

BitMatrix &BitMatrix::rsub(BitMatrixSlice &other)
{
  if (squares.size() < other.end)
    throw invalid_length();
  for (size_t i= other.start; i < other.end; i++)
    squares[i].rsub(other.bm.squares[i]);
  return *this;
}

BitMatrix &BitMatrix::sub(BitVector &other)
{
  if (squares.size() * 128 != other.size())
    throw invalid_length();
  for (size_t i= 0; i < squares.size(); i++)
    squares[i].sub((__m128i *) other.get_ptr() + i);
  return *this;
}

bool BitMatrix::operator==(BitMatrix &other)
{
  if (squares.size() != other.squares.size())
    throw invalid_length();
  for (size_t i= 0; i < squares.size(); i++)
    if (not(squares[i] == other.squares[i]))
      return false;
  return true;
}

bool BitMatrix::operator!=(BitMatrix &other)
{
  return not(*this == other);
}

void BitMatrix::randomize(PRNG &G)
{
  for (size_t i= 0; i < squares.size(); i++)
    squares[i].randomize(G);
}

void BitMatrix::randomize(int row, PRNG &G)
{
  for (size_t i= 0; i < squares.size(); i++)
    squares[i].randomize(row, G);
}

void BitMatrix::transpose()
{
  for (size_t i= 0; i < squares.size(); i++)
    squares[i].transpose();
}

void BitMatrix::check_transpose(BitMatrix &dual)
{
  for (size_t i= 0; i < squares.size(); i++)
    {
      for (int j= 0; j < 128; j++)
        for (int k= 0; k < 128; k++)
          if (squares[i].get_bit(j, k) != dual.squares[i].get_bit(k, j))
            {
              cout << "First error in square " << i << " row " << j
                   << " column " << k << endl;
              squares[i].print(i / 8, j / 8);
              dual.squares[i].print(i / 8, j / 8);
              return;
            }
    }
  cout << "No errors in transpose" << endl;
}

void BitMatrix::print_side_by_side(BitMatrix &other)
{
  for (int i= 0; i < 32; i++)
    {
      for (int j= 0; j < 64; j++)
        cout << squares[0].get_bit(i, j);
      cout << " ";
      for (int j= 0; j < 64; j++)
        cout << other.squares[0].get_bit(i, j);
      cout << endl;
    }
}

void BitMatrix::print_conditional(BitVector &conditions)
{
  for (int i= 0; i < 32; i++)
    {
      if (conditions.get_bit(i))
        for (int j= 0; j < 65; j++)
          cout << " ";
      for (int j= 0; j < 64; j++)
        cout << squares[0].get_bit(i, j);
      if (!conditions.get_bit(i))
        for (int j= 0; j < 65; j++)
          cout << " ";
      cout << endl;
    }
}

void BitMatrix::to(vector<BitVector> &output)
{
  output.resize(128);
  for (int i= 0; i < 128; i++)
    {
      output[i].resize(128 * squares.size());
      for (size_t j= 0; j < squares.size(); j++)
        output[i].set_int128(j, squares[j].rows[i]);
    }
}

void BitMatrix::set_row(unsigned int row, const BitVector &V)
{
  if (squares.size() * 128 != V.size())
    {
      throw invalid_length();
    }

  for (size_t j= 0; j < squares.size(); j++)
    {
      for (size_t k= 0; k < 16; k++)
        {
          squares[j].bytes[row][k]= V.get_byte(j * 16 + k);
        }
    }
}

void BitMatrix::set_row_up_to(unsigned int row, const BitVector &V)
{
  if (squares.size() * 128 > V.size())
    {
      throw invalid_length();
    }

  for (size_t j= 0; j < squares.size(); j++)
    {
      for (size_t k= 0; k < 16; k++)
        {
          squares[j].bytes[row][k]= V.get_byte(j * 16 + k);
        }
    }
}

BitVector BitMatrix::get_row(unsigned int row)
{
  BitVector output(squares.size() * 128);
  for (size_t j= 0; j < squares.size(); j++)
    {
      output.set_int128(j, squares[j].rows[row]);
    }

  return output;
}

void BitMatrix::zero_row(unsigned int row)
{
  for (size_t j= 0; j < squares.size(); j++)
    {
      memset(squares[j].bytes[row], 0, 16);
    }
}

void BitMatrix::output(ostream &s, bool human) const
{
  for (unsigned int i= 0; i < squares.size(); i++)
    {
      squares[i].output(s, human);
    }
}

void BitMatrix::input(istream &s, bool human)
{
  for (unsigned int i= 0; i < squares.size(); i++)
    {
      squares[i].input(s, human);
    }
}

gf2n BitMatrix::to_gf2n(int r, int c) const
{
  gf2n ans((int128) squares[c].rows[r]);
  return ans;
}

BitMatrixSlice::BitMatrixSlice(BitMatrix &bm, size_t start, size_t size) : bm(bm), start(start)
{
  end= start + size;
  if (end > bm.squares.size())
    {
      stringstream ss;
      ss << "Matrix slice (" << start << "," << end << ") larger than matrix (" << bm.squares.size() << ")";
      throw invalid_argument(ss.str());
    }
}

BitMatrixSlice &BitMatrixSlice::rsub(BitMatrixSlice &other)
{
  bm.rsub(other);
  return *this;
}

BitMatrixSlice &BitMatrixSlice::add(BitVector &other, int repeat)
{
  if (end * 128 > other.size() * repeat)
    throw invalid_length();
  for (size_t i= start; i < end; i++)
    bm.squares[i].sub((__m128i *) other.get_ptr() + i / repeat);
  return *this;
}

void BitMatrixSlice::randomize(int row, PRNG &G)
{
  for (size_t i= start; i < end; i++)
    bm.squares[i].randomize(row, G);
}

void BitMatrixSlice::conditional_add(BitVector &conditions, BitMatrix &other, bool useOffset)
{
  for (size_t i= start; i < end; i++)
    bm.squares[i].conditional_add(conditions, other.squares[i], useOffset * i);
}

void BitMatrixSlice::transpose()
{
  for (size_t i= start; i < end; i++)
    bm.squares[i].transpose();
}

void BitMatrixSlice::print()
{
  cout << "hex / value" << endl;
  for (int i= 0; i < 16; i++)
    {
      cout << int128(bm.squares[0].rows[i]) << " " << gf2n(bm.squares[0].rows[i]) << endl;
    }
  cout << endl;
}
