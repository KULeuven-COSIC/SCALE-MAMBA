/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "Processor/Memory.h"
#include "LSSS/Share.h"
#include "LSSS/Share2.h"
#include "Math/Integer.h"
#include "Math/gfp.h"
#include "Mod2Engine/aBitVector2.h"
#include "OT/aBitVector.h"
#include <math.h>

#include <fstream>
using namespace std;

template<class T>
void Memory<T>::initialize(int verbose)
{
  // Lock to ensure only one thread actually creates the new memory
  memory_mutex.lock();
  if (initialized)
    {
      memory_mutex.unlock();
      return;
    }
  unsigned long MEMSIZE;
  if (T::type_string() == "gfp")
    {
      MEMSIZE= CINT_MEMSIZE;
    }
  else if (T::type_string() == "Share")
    {
      MEMSIZE= SINT_MEMSIZE;
    }
  else if (T::type_string() == "aBitVector2" || T::type_string() == "aBitVector")
    {
      MEMSIZE= SREGINT_MEMSIZE;
    }
  else if (T::type_string() == "integer")
    {
      MEMSIZE= REGINT_MEMSIZE;
    }
  else
    {
      throw not_implemented();
    }

  if (MEMSIZE < 13)
    {
      throw memory_invalid_size();
    }
  free_list.resize(MEMSIZE + 1);
  unsigned int sz= 1 << MEMSIZE;
  if (sz > M.size())
    {
      if (verbose > 0)
        {
          fprintf(stderr, "Initializing SCALE memory %s to size %d\n",
                  T::type_string().c_str(), sz);
        }
      try
        {
          M.resize(sz, def_value);
        }
      catch (std::bad_alloc &e)
        {
          fprintf(stderr, "Initializing SCALE memory %s to size %d failed\n",
                  T::type_string().c_str(), sz);
          fflush(stderr);
          throw e;
        }
    }

  for (unsigned int i= 0; i <= MEMSIZE; i++)
    {
      free_list[i].clear();
    }

  // Initially whole block of specified size is available
  free_list[MEMSIZE].push_back(make_pair(0, sz - 1));
  initialized= true;
  memory_mutex.unlock();
}

template<class T>
unsigned int Memory<T>::New(unsigned int sz)
{
  // Calculate index in free list to search for block if available
  int n= ceil(log(sz) / log(2));

  memory_mutex.lock();

  // Block available
  if (free_list[n].size() > 0)
    {
      pair<unsigned int, unsigned int> temp= free_list[n][0];

      // Remove block from free list
      free_list[n].erase(free_list[n].begin());

      // map starting address with size to make deallocating easy
      mp[temp.first]= temp.second - temp.first + 1;

#ifdef DEBUGMEM
      printf("\n New %s memory of size %d at position %d\n", T::type_string().c_str(), sz, temp.first);
      fflush(stdout);
#endif

      memory_mutex.unlock();

      // Memory from temp.first to temp.second is allocated
      return temp.first;
    }

  unsigned int i;
  for (i= n + 1; i < free_list.size(); i++)
    {
      // Find block size greater than request
      if (free_list[i].size() != 0)
        {
          break;
        }
    }

  // If no such block is found, i.e., no memory block available
  if (i == free_list.size())
    {
      throw memory_new_error(T::type_string());
    }
  pair<unsigned int, unsigned int> temp= free_list[i][0];

  // Remove first block to split it into halves
  free_list[i].erase(free_list[i].begin());
  i--;

  // Switch to signed looping as n could be zero
  for (int ii= i; ii >= n; ii--)
    {
      // Divide block into two halves
      pair<unsigned int, unsigned int> pair1, pair2;
      pair1= make_pair(temp.first, temp.first + (temp.second - temp.first) / 2);
      pair2= make_pair(temp.first + (temp.second - temp.first + 1) / 2, temp.second);

      free_list[ii].push_back(pair1);

      // Push them in free list
      free_list[ii].push_back(pair2);
      temp= free_list[ii][0];

      // Remove first free block to further split
      free_list[ii].erase(free_list[ii].begin());
    }
  mp[temp.first]= temp.second - temp.first + 1;

#ifdef DEBUGMEM
  printf("\n New %s memory of size %d at position %d\n", T::type_string().c_str(), sz, temp.first);
  fflush(stdout);
#endif

  memory_mutex.unlock();

  // Memory from temp.first to temp.second is allocated
  return temp.first;
}

template<class T>
void Memory<T>::Delete(unsigned int pos)
{
  memory_mutex.lock();

#ifdef DEBUGMEM
  printf("\n Delete %s memory at position %d\n", T::type_string().c_str(), pos);
  fflush(stdout);
#endif

  // If no such starting address available exit
  if (mp.find(pos) == mp.end())
    {
      throw memory_delete_error();
    }

  // Size of block to be searched
  unsigned int n= ceil(log(mp[pos]) / log(2));

  // Add the block in free list
  free_list[n].push_back(make_pair(pos, pos + pow(2, n) - 1));

  // Memoryblock freed from pos to pos + pow(2, n) - 1

  // Recursively go through free_list trying to merge values
  // and its buddy. There must be a better way of doing this
  bool done= false;
  while (!done)
    {
      done= true;
      for (unsigned int i= 0; i < free_list.size(); i++)
        {
          if (free_list[i].size() > 1)
            {
              for (unsigned int j= 0; j < free_list[i].size(); j++)
                {
                  unsigned int start= free_list[i][j].first;
                  unsigned int end= free_list[i][j].second;
                  unsigned int buddyNumber= start / pow(2, i);
                  if (buddyNumber % 2 == 0)
                    {
                      unsigned int buddyAddress= end + 1;
                      // Search to find buddy so as to merge
                      for (unsigned int k= 0; k < free_list[i].size(); k++)
                        {
                          // If buddy found and is also free
                          if (free_list[i][k].first == buddyAddress)
                            { // Merge the buddies to make them one large free memory block
                              // Coalescing of blocks starting at start and buddyAddress
                              free_list[i + 1].push_back(make_pair(start, start + 2 * pow(2, i) - 1));
                              free_list[i].erase(free_list[i].begin() + j);
                              free_list[i].erase(free_list[i].begin() + k);
                              done= false;
                            }
                        }
                    }
                }
            }
        }
    }

  // Remove the key existence from map
  mp.erase(pos);
  memory_mutex.unlock();
}

template<class T>
bool Memory<T>::check_allocated(unsigned int pos)
{
  map<unsigned int, unsigned int>::iterator it;

  for (it= mp.begin(); it != mp.end(); it++)
    {
      if (pos >= it->first && pos < (it->first + it->second))
        {
          return true;
        }
    }
  return false;
}

template<class T>
ostream &operator<<(ostream &s, const Memory<T> &M)
{
  unsigned int psz= 1 << 20;
  s << psz << endl;
  if (M.M.size() < psz)
    {
      throw memory_invalid_size();
    }

    // printf("Memory %s : size = %d\n",T::type_string().c_str(),M.M.size());

#ifdef DEBUG
  for (unsigned int i= 0; i < psz; i++)
    {
      M.M[i].output(s, true);
      s << endl;
    }
  s << endl;

#else
  for (unsigned int i= 0; i < psz; i++)
    {
      M.M[i].output(s, false);
      //cout << i << " : " << T::type_string() << " : "; M.M[i].output(cout,true); cout << endl;
    }

#endif

  return s;
}

template<class T>
istream &operator>>(istream &s, Memory<T> &M)
{
  unsigned int len;
  unsigned int psz= 1 << 20;

  s >> len;
  if (len != psz)
    {
      throw memory_invalid_size();
    }

  s.seekg(1, istream::cur);

  for (unsigned int i= 0; i < psz; i++)
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
template class Memory<aBitVector2>;

template istream &operator>>(istream &s, Memory<gfp> &M);
template istream &operator>>(istream &s, Memory<Share> &M);
template istream &operator>>(istream &s, Memory<Integer> &M);

template ostream &operator<<(ostream &s, const Memory<gfp> &M);
template ostream &operator<<(ostream &s, const Memory<Share> &M);
template ostream &operator<<(ostream &s, const Memory<Integer> &M);
