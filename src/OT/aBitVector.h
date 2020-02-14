/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _aBitVector
#define _aBitVector

/* This holds a vector of 64 authenticated bits, representing a
 * 64 bit integer value
 *
 * This implements the sregint type within the SCALE processor.
 *
 * Many operations require interaction, hence for these we pass
 * a player.
 *   - Basically we then call the GC operations to perform
 *     the basic arithmetic
 */

#include "GC/Base_Circuits.h"
#include "aBit.h"

#define sreg_bitl 64

class aBitVector
{
  vector<aBit> x;

  /* Evaluate a Circuita
   * The first two write the output to *this, the second two return the
   * one bit answer.
   */
  void eval_circuit(Circ_Type T,
                    const aBitVector &a,
                    Player &P, unsigned int online_thread_no);

  void eval_circuit(Circ_Type T,
                    const aBitVector &a, const aBitVector &b,
                    Player &P, unsigned int online_thread_no);

  friend aBit eval_circuit_bit(Circ_Type T,
                               const aBitVector &a,
                               Player &P, unsigned int online_thread_no);

  friend aBit eval_circuit_bit(Circ_Type T,
                               const aBitVector &a, const aBitVector &b,
                               Player &P, unsigned int online_thread_no);

  /* This one produces two aBitVectors as output */
  friend void eval_circuit_2(Circ_Type T,
                             aBitVector &x, aBitVector &y,
                             const aBitVector &a, const aBitVector &b,
                             Player &P, unsigned int online_thread_no);

public:
  static string type_string()
  {
    return "aBitVector";
  }

  aBitVector()
  {
    x.resize(sreg_bitl);
  }

  aBitVector(unsigned long long a)
  {
    x.resize(sreg_bitl);
    assign(a);
  }

  aBitVector(const vector<aBit> &y)
  {
    if (y.size() != sreg_bitl)
      {
        throw bad_value();
      }
    x= y;
  }

  void assign(unsigned long long a);
  void assign_zero();
  void assign_one();

  vector<aBit> internal_rep() const { return x; }

  unsigned long long Open(Player &P) const;

  /* Bitwise Operations */
  void Bitwise_XOR(const aBitVector &a, const aBitVector &b)
  {
    for (int i= 0; i < sreg_bitl; i++)
      {
        x[i].add(a.x[i], b.x[i]);
      }
  }
  void Bitwise_Negate(const aBitVector &a)
  {
    for (int i= 0; i < sreg_bitl; i++)
      {
        x[i].negate(a.x[i]);
      }
  }
  void Bitwise_AND(const aBitVector &a, const aBitVector &b, Player &P, unsigned int online_thread_no);
  void Bitwise_OR(const aBitVector &a, const aBitVector &b, Player &P, unsigned int online_thread_no)
  {
    aBitVector te;
    te.Bitwise_AND(a, b, P, online_thread_no);
    Bitwise_XOR(a, b);
    Bitwise_XOR(*this, te);
  }

  void Bitwise_XOR(const aBitVector &a, unsigned long b)
  {
    for (unsigned int i= 0; i < sreg_bitl; i++)
      {
        x[i]= a.x[i];
        if ((b & 1) == 1)
          {
            x[i].negate();
          }
        b>>= 1;
      }
  }
  void Bitwise_AND(const aBitVector &a, unsigned long b)
  {
    for (unsigned int i= 0; i < sreg_bitl; i++)
      {
        if ((b & 1) == 1)
          {
            x[i]= a.x[i];
          }
        else
          {
            x[i].assign_zero();
          }
        b>>= 1;
      }
  }
  void Bitwise_OR(const aBitVector &a, unsigned long b)
  {
    aBitVector te;
    te.Bitwise_AND(a, b);
    Bitwise_XOR(a, b);
    Bitwise_XOR(*this, te);
  }

  /* AND all bits in a by the bit b */
  void Bit_AND(const aBitVector &a, const aBit &b, Player &P, unsigned int online_thread_no);

  /* SHL/SHR */
  void SHL(const aBitVector &a, unsigned int n);
  void SHR(const aBitVector &a, unsigned int n);

  /* Arithmetic Operations */
  void add(const aBitVector &a, const aBitVector &b, Player &P, unsigned int online_thread_no)
  {
    eval_circuit(Adder64, a, b, P, online_thread_no);
  }

  void sub(const aBitVector &a, const aBitVector &b, Player &P, unsigned int online_thread_no)
  {
    eval_circuit(Sub64, a, b, P, online_thread_no);
  }

  /* This mult does the mult mod 2^64 */
  void mul(const aBitVector &a, const aBitVector &b, Player &P, unsigned int online_thread_no)
  {
    eval_circuit(Mult64, a, b, P, online_thread_no);
  }

  /* This produces two 64 bit answers */
  friend void mul(aBitVector &x, aBitVector &y,
                  const aBitVector &a, const aBitVector &b, Player &P, unsigned int online_thread_no)
  {
    eval_circuit_2(Mult2_64, x, y, a, b, P, online_thread_no);
  }

  // This does a SIGNED division
  void div(const aBitVector &a, const aBitVector &b, Player &P, unsigned int online_thread_no)
  {
    eval_circuit(Divide64, a, b, P, online_thread_no);
  }

  void negate(const aBitVector &a, Player &P, unsigned int online_thread_no)
  {
    eval_circuit(Neg64, a, P, online_thread_no);
  }

  /* Comparisons */
  aBit less_than_zero() const
  {
    return x[sreg_bitl - 1];
  }

  aBit equal_zero(Player &P, unsigned int online_thread_no) const
  {
    return eval_circuit_bit(Zero_Equal, *this, P, online_thread_no);
  }

  /* Bit access */
  aBit get_bit(unsigned int n) const
  {
    return x[n];
  }

  void set_bit(unsigned int n, const aBit &t)
  {
    x[n]= t;
  }

  // Input and output from a stream
  //  - Can do in human or machine only format (later should be faster)
  void output(ostream &s, bool human) const;
  void input(istream &s, bool human);
};

#endif
