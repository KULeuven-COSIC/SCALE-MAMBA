/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#ifndef _Memory
#define _Memory

/* Class to hold global memory of our system 
 *
 * We use a buddy style memory management system, see
 *   https://www.geeksforgeeks.org/buddy-memory-allocation-program-set-1-allocation/
https://www.geeksforgeeks.org/buddy-memory-allocation-program-set-2-deallocation/ 
 *
 * from which we have borrowed the code for implementing the
 * system. The original code was from sarthak_eddy
 *  - There are some bugs in this code, especially related to
 *    deallocation which we fixed
 *
 * You can access memory which has not been reserved by a New
 * and you can access memory after a Delete. The reservation
 * system of New/Delete is for when the program wants to
 * pass memory management to the system.
 *
 */

#include <iostream>
#include <map>
#include <mutex>
#include <set>
#include <vector>
using namespace std;

#include "Exceptions/Exceptions.h"
#include "config.h"

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
  mutex memory_mutex;

  T def_value; // A default value of type T for copying

  // This is the actual memory
  vector<T> M;

  // Next is the data for having New/Delete working via
  // the Buddy system

  // Global vector of pairs to store address ranges available
  vector<vector<pair<unsigned int, unsigned int>>> free_list;

  // Map used as hash map to store the starting
  // address as key and size of allocated segment
  // key as value
  map<unsigned int, unsigned int> mp;

  bool initialized;

  // Checks whether the memory at position pos is currently
  // in an allocated block [does not check out of bounds
  // errors, just allocation]
  bool check_allocated(unsigned int pos);

public:
  Memory()
  {
    initialized= false;
  }

  void set_default(T d)
  {
    def_value= d;
  }

  /* Initializes the memory
   *   - Size is defined by the variable *MEMSIZE in config.h
   */
  void initialize(int verbose);

  /* This allocated memory of size sz, and returns the
   * address of the first element.
   * This executes a thread lock
   */
  unsigned int New(unsigned int sz);
  /* This does the deallocation. You pass in the position
   * of the value which was returned by New
   * This executes a thread lock
   */
  void Delete(unsigned int pos);

  int size()
  {
    return M.size();
  }

  const T &read(unsigned int i, int verbose)
  {
#ifdef DEBUGMEM
    if (check_allocated(i) == false)
      {
        throw memory_access_error();
      }
#endif
    if (i >= M.size())
      {
        fprintf(stderr, "Accessing memory position %d \n", i);
        throw memory_out_of_bounds();
      }
#ifdef DEBUGMEM
    stringstream ss;
    M[i].output(ss, true);
    printf("\t Reading data %s in memory %s position %lu \n", ss.str().c_str(), T::type_string().c_str(), i);
    fflush(stdout);
#endif
    return M[i];
  }

  void write(unsigned int i, const T &x, int verbose, int PC= -1)
  {
#ifdef DEBUGMEM
    if (check_allocated(i) == false)
      {
        throw memory_access_error();
      }
    stringstream ss;
    x.output(ss, true);
    printf("\t Storing data %s in memory %s position %lu \n", ss.str().c_str(), T::type_string().c_str(), i);
    fflush(stdout);
#endif
    if (i >= M.size())
      {
        fprintf(stderr, "Accessing memory position %d \n", i);
        throw memory_out_of_bounds();
      }
    M[i]= x;
    (void) PC; // To avoid a compiler warning. We pass in PC for gdb debugging purposes
  }

  /* Clears the memory */
  void clear_memory();

  // We only read/write 2^20 items here
  friend ostream &operator<<<>(ostream &s, const Memory<T> &M);
  friend istream &operator>><>(istream &s, Memory<T> &M);
};

#endif
