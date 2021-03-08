/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _gf2
#define _gf2

#include <iostream>
using namespace std;

class gf2
{
  bool a;

public:
  static string type_string()
  {
    return "gf2";
  }

  static int size()
  {
    return sizeof(bool);
  }

  void assign_zero()
  {
    a= 0;
  }
  void assign_one()
  {
    a= 1;
  }
  void assign(word aa)
  {
    a= aa & 1;
  }
  void assign(long aa)
  {
    a= aa & 1;
  }
  void assign(int aa)
  {
    a= aa & 1;
  }
  void assign(const char *buffer)
  {
    a= buffer[0] & 1;
  }

  bool get() const
  {
    return a;
  }

  void assign(bool x)
  {
    a= x;
  }

  gf2()
  {
    a= 0;
  }
  gf2(const bool g)
  {
    a= g;
  }
  gf2(int x)
  {
    assign(x);
  }
  bool is_zero() const
  {
    return (a == 0);
  }
  bool is_one() const
  {
    return a;
  }

  bool equal(const gf2 &y) const
  {
    return (a == y.a);
  }
  bool operator==(const gf2 &y) const
  {
    return equal(y);
  }
  bool operator!=(const gf2 &y) const
  {
    return !equal(y);
  }

  // x+y
  void add(const gf2 &x, const gf2 &y)
  {
    a= x.a + y.a;
  }
  void add(const gf2 &x)
  {
    a+= x.a;
  }
  void sub(const gf2 &x, const gf2 &y)
  {
    a= x.a - y.a;
  }
  void sub(const gf2 &x)
  {
    a-= x.a;
  }

  // = x * y
  void mul(const gf2 &x, const gf2 &y)
  {
    a= x.a & y.a;
  }
  void mul(const gf2 &x)
  {
    a= a & x.a;
  }

  gf2 operator+(const gf2 &x) const
  {
    gf2 res;
    res.add(*this, x);
    return res;
  }
  gf2 operator-(const gf2 &x) const
  {
    gf2 res;
    res.sub(*this, x);
    return res;
  }
  gf2 operator*(const gf2 &x) const
  {
    gf2 res;
    res.mul(*this, x);
    return res;
  }
  gf2 &operator+=(const gf2 &x)
  {
    add(x);
    return *this;
  }
  gf2 &operator-=(const gf2 &x)
  {
    sub(x);
    return *this;
  }
  gf2 &operator*=(const gf2 &x)
  {
    mul(x);
    return *this;
  }

  void square(const gf2 &aa)
  {
    a= aa.a;
  }
  void square()
  {
    ;
  }
  void invert()
  {
    ;
  }
  void invert(const gf2 &aa)
  {
    a= aa.a;
  }
  void negate()
  {
    ;
  }

  void randomize(PRNG &G)
  {
    a= G.get_uchar() & 1;
  }

  void output(ostream &s, bool human) const
  {
    if (human)
      {
        s << a;
      }
    else
      {
        s.write((char *) &a, sizeof(bool));
      }
  }
  void input(istream &s, bool human)
  {
    if (s.peek() == EOF)
      {
        if (s.tellg() == 0)
          {
            cout << "IO problem. Empty file?" << endl;
            throw file_error("Input gf2");
          }
        throw file_error("Input gf2");
      }

    if (human)
      {
        s >> a;
      }
    else
      {
        s.read((char *) a, sizeof(bool));
      }
  }

  // Output directly to a string of chars, assumes enough space is
  // allocated. Returns the number of chars written
  unsigned int output(uint8_t *buff) const
  {
    memcpy(buff, (char *) a, sizeof(bool));
    return sizeof(bool);
  }
  // Input directly from a string of chars
  unsigned int input(const uint8_t *buff)
  {
    memcpy((char *) a, buff, sizeof(bool));
    return sizeof(bool);
  }

  /* Input/Output to a string at position pos.
   * String is already of enough size in both cases.
   * The number of chars read/written is returned
   */
  unsigned int output(string &s, unsigned long pos) const
  {
    return output((uint8_t *) s.c_str() + pos);
  }
  unsigned int input(const string &s, unsigned long pos)
  {
    return input((uint8_t *) s.c_str() + pos);
  }

  friend ostream &operator<<(ostream &s, const gf2 &x)
  {
    x.output(s, true);
    return s;
  }
  friend istream &operator>>(istream &s, gf2 &x)
  {
    x.input(s, true);
    return s;
  }

  void AND(const gf2 &x, const gf2 &y)
  {
    a= x.a & y.a;
  }
  void XOR(const gf2 &x, const gf2 &y)
  {
    a= x.a ^ y.a;
  }
  void OR(const gf2 &x, const gf2 &y)
  {
    a= x.a | y.a;
  }

  gf2 operator&(const gf2 &x)
  {
    gf2 res;
    res.AND(*this, x);
    return res;
  }
  gf2 operator^(const gf2 &x)
  {
    gf2 res;
    res.XOR(*this, x);
    return res;
  }
  gf2 operator|(const gf2 &x)
  {
    gf2 res;
    res.OR(*this, x);
    return res;
  }
};

#endif
