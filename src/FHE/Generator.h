/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

/*
 * Generator.h
 *
 *  Created on: 6 Aug 2017
 *      Author: csmksk
 */

#ifndef FHE_GENERATOR_H_
#define FHE_GENERATOR_H_

#include <vector>
using namespace std;

#include "Math/bigint.h"
#include "Math/modp.h"

template<class T>
class Generator
{
public:
  virtual ~Generator()
  {
  }
  virtual Generator *clone() const= 0;
  virtual void get(T &x) const= 0;
};

template<class T>
class Iterator : public Generator<T>
{
  const vector<T> &v;
  mutable size_t i;

public:
  Iterator(const vector<T> &v)
      : v(v), i(0)
  {
  }
  Generator<T> *clone() const
  {
    return new Iterator(*this);
  }
  void get(T &x) const
  {
    x= v[i];
    i++;
  }
};

class ConversionIterator : public Generator<bigint>
{
  const vector<modp> &v;
  const Zp_Data &ZpD;
  mutable size_t i;

public:
  ConversionIterator(const vector<modp> &v, const Zp_Data &ZpD)
      : v(v), ZpD(ZpD), i(0)
  {
  }
  Generator<bigint> *clone() const
  {
    return new ConversionIterator(*this);
  }
  void get(bigint &x) const
  {
    to_bigint(x, v[i], ZpD);
    i++;
  }
};

class WriteConversionIterator : public Generator<bigint>
{
  vector<modp> &v;
  const Zp_Data &ZpD;
  mutable size_t i;
  mutable bigint tmp;

public:
  WriteConversionIterator(vector<modp> &v, const Zp_Data &ZpD)
      : v(v), ZpD(ZpD), i(0)
  {
  }
  Generator<bigint> *clone() const
  {
    return new WriteConversionIterator(*this);
  }
  void get(bigint &x) const
  {
    tmp= x;
    v[i].to_modp(tmp, ZpD);
    i++;
  }
};

#endif /* FHE_GENERATOR_H_ */
