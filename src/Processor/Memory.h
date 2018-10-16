/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _Memory
#define _Memory

/* Class to hold global memory of our system */

#include <iostream>
#include <set>
#include <vector>
using namespace std;

// Forward declaration as apparently this is needed for friends in templates
template<class T>
class Memory;
template<class T>
ostream &operator<<(ostream &s, const Memory<T> &M);
template<class T>
istream &operator>>(istream &s, Memory<T> &M);

template<class T>
class Memory
{
  vector<T> M;
  T def_value; // A default value of type T for copying

public:
  void set_default(T d)
  {
    def_value= d;
  }
  void resize(int sz)
  {
    M.resize(sz, def_value);
  }

  int size()
  {
    return M.size();
  }

  const T &read(int i) const
  {
    return M[i];
  }

  void write(unsigned int i, const T &x, int PC= -1)
  {
    M[i]= x;
    (void) PC;
  }

  /* Clears the memory */
  void clear_memory();

  void minimum_size(int sz, string threadname);

  friend ostream &operator<<<>(ostream &s, const Memory<T> &M);
  friend istream &operator>><>(istream &s, Memory<T> &M);
};

/* This function loads a global memory from disk and
 * produces the memory
 *
 * The global unshared memory is of the form
 *     sz     <- Size
 *    n val   <- values
 *    n val   <- values
 *    -1 -1   <- End of values
 */
template<class T>
void Load_Memory(Memory<T> &M, ifstream &inpf);

#endif
