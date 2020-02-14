/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _Memory
#define _Memory

/* Class to hold global memory of our system */

#include <iostream>
#include <mutex>
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

class Machine;

template<class T>
class Memory
{
  mutex memory_mutex;

  vector<T> M;
  T def_value; // A default value of type T for copying

public:
  void set_default(T d)
  {
    def_value= d;
  }

  int size()
  {
    return M.size();
  }

  int reduce_size(unsigned int sz)
  {
    if (sz < M.size())
      {
        M.resize(sz);
      }
  }

  const T &read(int i, int verbose)
  {
    if (i >= M.size())
      {
        minimum_size(2 * i, "access_r", verbose);
      }
    return M[i];
  }

  void write(unsigned int i, const T &x, int verbose, int PC= -1)
  {
    if (i >= M.size())
      {
        minimum_size(2 * i, "access_w", verbose);
      }
    M[i]= x;
    (void) PC; // To avoid a compiler warning. We pass in PC for gdb debugging purposes
  }

  /* Clears the memory */
  void clear_memory();

  void minimum_size(unsigned int sz, string threadname, int verbose);

  friend ostream &operator<<<>(ostream &s, const Memory<T> &M);
  friend istream &operator>><>(istream &s, Memory<T> &M);
};

#endif
