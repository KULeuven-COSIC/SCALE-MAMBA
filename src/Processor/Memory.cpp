/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Processor/Memory.h"
#include "LSSS/Share.h"
#include "Math/Integer.h"
#include "Math/gfp.h"
#include "OT/aBitVector.h"

#include <fstream>

template<class T>
void Memory<T>::minimum_size(unsigned int sz, string threadname, int verbose)
{
  memory_mutex.lock();
  if (sz > M.size())
    {
      if (verbose > 0)
        {
          fprintf(stderr, "%s needs more %s memory, resizing to %d\n",
                  threadname.c_str(), T::type_string().c_str(), sz);
        }
      M.resize(sz, def_value);
    }
  memory_mutex.unlock();
}

template<class T>
ostream &operator<<(ostream &s, const Memory<T> &M)
{
  s << M.M.size() << endl;

  // printf("Memory %s : size = %d\n",T::type_string().c_str(),M.M.size());

#ifdef DEBUG
  for (unsigned int i= 0; i < M.M.size(); i++)
    {
      M.M[i].output(s, true);
      s << endl;
    }
  s << endl;

#else
  for (unsigned int i= 0; i < M.M.size(); i++)
    {
      M.M[i].output(s, false);
      // cout << i << " : "; M.M[i].output(cout,true); cout << endl;
    }

#endif

  return s;
}

template<class T>
istream &operator>>(istream &s, Memory<T> &M)
{
  int len;

  s >> len;
  M.M.resize(len, M.def_value);
  s.seekg(1, istream::cur);

  for (unsigned int i= 0; i < M.M.size(); i++)
    {
      M.M[i].input(s, false);
    }

  return s;
}

template<class T>
void Memory<T>::clear_memory()
{
  for (unsigned int i= 0; i < M.size(); i++)
    {
      M[i].assign_zero();
    }
}

template class Memory<gfp>;
template class Memory<Share>;
template class Memory<Integer>;
template class Memory<aBitVector>;

template istream &operator>>(istream &s, Memory<gfp> &M);
template istream &operator>>(istream &s, Memory<Share> &M);
template istream &operator>>(istream &s, Memory<Integer> &M);

template ostream &operator<<(ostream &s, const Memory<gfp> &M);
template ostream &operator<<(ostream &s, const Memory<Share> &M);
template ostream &operator<<(ostream &s, const Memory<Integer> &M);
