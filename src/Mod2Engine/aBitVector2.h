/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _aBitVector2
#define _aBitVector2

/* This holds a vector of 64 authenticated bits, representing a
 * 64 bit integer value
 *
 * This implements the sregint type within the SCALE processor
 * when using the nonHSS subsystem
 *
 * Many operations require interaction, hence for these we pass
 * a player.
 *   - Basically we then call the operation to evaluate the
 *     circuit to perform the basic arithmetic
 */

#include "GC/Base_Circuits.h"
#include "LSSS/Share2.h"
#include "System/Player.h"

#define sreg_bitl 64

class aBitVector2
{
  static int p; // The default player
  Share2 x;

  /* Evaluate a Circuit
   * The first two write the output to *this, the second two return the
   * one bit answer.
   */
  void eval_circuit(Circ_Type T,
                    const aBitVector2 &a,
                    Player &P, unsigned int online_thread_no);

  void eval_circuit(Circ_Type T,
                    const aBitVector2 &a, const aBitVector2 &b,
                    Player &P, unsigned int online_thread_no);

  friend Share2 eval_circuit_bit(Circ_Type T,
                                 const aBitVector2 &a,
                                 Player &P, unsigned int online_thread_no);

  friend Share2 eval_circuit_bit(Circ_Type T,
                                 const aBitVector2 &a, const aBitVector2 &b,
                                 Player &P, unsigned int online_thread_no);

  /* This one produces two aBitVectors as output */
  friend void eval_circuit_2(Circ_Type T,
                             aBitVector2 &x, aBitVector2 &y,
                             const aBitVector2 &a, const aBitVector2 &b,
                             Player &P, unsigned int online_thread_no);

public:
  static string type_string()
  {
    return "aBitVector2";
  }

  static void set_player(int i)
  {
    p= i;
  }

  aBitVector2()
  {
    assign_zero();
  }

  aBitVector2(unsigned long long a)
  {
    assign(a);
  }

  aBitVector2(const Share2 &y)
  {
    assign(y);
  }

  void assign(unsigned long long a)
  {
    x.assign(p, a);
  }
  void assign(const Share2 &y)
  {
    x= y;
  }
  void assign_zero()
  {
    x.assign_zero(p);
  }
  void assign_one()
  {
    x.assign_one(p);
  }

  Share2 internal_rep() const
  {
    return x;
  }

  /* Bitwise Operations */
  void Bitwise_XOR(const aBitVector2 &a, const aBitVector2 &b)
  {
    x.add(a.x, b.x);
  }
  void Bitwise_Negate(const aBitVector2 &a)
  {
    Share2 y(0xFFFFFFFFFFFFFFFFULL, p);
    x.add(a.x, y);
  }
  void Bitwise_AND(const aBitVector2 &a, const aBitVector2 &b, Player &P,
                   unsigned int online_thread_no);

  void Bitwise_OR(const aBitVector2 &a, const aBitVector2 &b, Player &P,
                  unsigned int online_thread_no)
  {
    aBitVector2 te;
    te.Bitwise_AND(a, b, P, online_thread_no);
    Bitwise_XOR(a, b);
    Bitwise_XOR(*this, te);
  }
  void Bitwise_XOR(const aBitVector2 &a, unsigned long b)
  {
    Share2 y(b, p);
    x.add(a.x, y);
  }
  void Bitwise_AND(const aBitVector2 &a, unsigned long b)
  {
    x.mul(a.x, b);
  }
  void Bitwise_OR(const aBitVector2 &a, unsigned long b)
  {
    aBitVector2 te;
    te.Bitwise_AND(a, b);
    Bitwise_XOR(a, b);
    Bitwise_XOR(*this, te);
  }

  /* AND all bits in a by the bit b 
   *   - b is meant only hold a single bit
   */
  void Bit_AND(const aBitVector2 &a, const Share2 &b, Player &P,
               unsigned int online_thread_no)
  {
    aBitVector2 v;
    v.x.duplicate(b);
    Bitwise_AND(a, v, P, online_thread_no);
  }

  /* SHL/SHR */
  void SHL(const aBitVector2 &a, unsigned int n)
  {
    x.SHL(a.x, n);
  }
  void SHR(const aBitVector2 &a, unsigned int n)
  {
    x.SHR(a.x, n);
  }

  /* Bit access */
  Share2 get_bit(unsigned int n) const
  {
    Share2 v(p);
    v.SHR(x, n);
    v.mul(v, 1ULL);
    return v;
  }

  void set_bit(unsigned int n, const Share2 &t)
  {
    Share2 mask(p), tshift;
    word one= 1ULL << n;
    tshift.SHL(t, n);
    mask.mul(x, one);
    x.add(x, mask);   // Remove the current bit
    x.add(x, tshift); // Add in the desired bit
  }

  // Input and output from a stream
  //  - Can do in human or machine only format (later should be faster)
  void output(ostream &s, bool human) const
  {
    x.output(s, human);
  }

  void input(istream &s, bool human)
  {
    x.input(s, human);
  }

  /* Arithmetic Operations */
  void add(const aBitVector2 &a, const aBitVector2 &b, Player &P,
           unsigned int online_thread_no)
  {
    eval_circuit(Adder64, a, b, P, online_thread_no);
  }

  void sub(const aBitVector2 &a, const aBitVector2 &b, Player &P,
           unsigned int online_thread_no)
  {
    eval_circuit(Sub64, a, b, P, online_thread_no);
  }

  /* This mult does the mult mod 2^64 */
  void mul(const aBitVector2 &a, const aBitVector2 &b, Player &P,
           unsigned int online_thread_no)
  {
    eval_circuit(Mult64, a, b, P, online_thread_no);
  }

  /* This produces two 64 bit answers */
  friend void mul(aBitVector2 &x, aBitVector2 &y,
                  const aBitVector2 &a, const aBitVector2 &b, Player &P,
                  unsigned int online_thread_no)
  {
    eval_circuit_2(Mult2_64, x, y, a, b, P, online_thread_no);
  }

  // This does a SIGNED division
  void div(const aBitVector2 &a, const aBitVector2 &b, Player &P,
           unsigned int online_thread_no)
  {
    eval_circuit(Divide64, a, b, P, online_thread_no);
  }

  void negate(const aBitVector2 &a, Player &P,
              unsigned int online_thread_no)
  {
    eval_circuit(Neg64, a, P, online_thread_no);
  }

  /* Comparisons */
  Share2 less_than_zero() const
  {
    Share2 a(p);
    a.SHR(x, sreg_bitl - 1);
    return a;
  }

  Share2 equal_zero(Player &P, unsigned int online_thread_no) const
  {
    return eval_circuit_bit(Zero_Equal, *this, P, online_thread_no);
  }

  /* Randomize */
  void randomize(unsigned int online_thread_no, Player &P);
};

/* Expand 64 bits in aBitVector into an array of 64 bits */
void expand(vector<Share2> &a, const aBitVector2 &b);

/* Reverses the above operation */
void collapse(aBitVector2 &b, const vector<Share2> &a);

#endif
