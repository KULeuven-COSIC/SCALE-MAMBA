/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

/*
 * utill_containers.cpp
 *
 *  Created on: Mar 6, 2019
 *      Author: drotaru
 */

#include "util_containers.h"
#include "LSSS/Share.h"
#include "Math/gfp.h"
#include "OT/aBit.h"

template<class T>
void Split_Lists(vector<T> &out_vector,
                 list<T> &out_list,
                 list<T> &input_list,
                 unsigned int size)
{
  list<T> front;
  front.splice(front.begin(), input_list, input_list.begin(), next(input_list.begin(), size));
  out_vector.resize(size);
  if (out_vector.size() != front.size())
    {
      throw invalid_length();
    }

  copy(front.begin(), front.end(), out_vector.begin());
  out_list.splice(out_list.begin(), front);
}

template<class T>
void Split_Lists(vector<T> &out_vector,
                 list<T> &input_list,
                 unsigned int size)
{
  list<T> front;
  front.splice(front.begin(), input_list, input_list.begin(), next(input_list.begin(), size));
  out_vector.resize(size);
  if (out_vector.size() != front.size())
    {
      throw invalid_length();
    }
  copy(front.begin(), front.end(), out_vector.begin());
}

template<class T>
void Split_Lists(list<T> &front,
                 list<T> &input_list,
                 unsigned int size)
{
  front.clear();
  front.splice(front.begin(), input_list, input_list.begin(), next(input_list.begin(), size));
}

template void Split_Lists(vector<Share> &, list<Share> &, list<Share> &, unsigned int);

template void Split_Lists(list<Share> &, list<Share> &, unsigned int);
template void Split_Lists(list<gfp> &, list<gfp> &, unsigned int);

template void Split_Lists(vector<Share> &, list<Share> &, unsigned int);
template void Split_Lists(vector<gfp> &, list<gfp> &, unsigned int);
template void Split_Lists(vector<aBit> &, list<aBit> &, unsigned int);
