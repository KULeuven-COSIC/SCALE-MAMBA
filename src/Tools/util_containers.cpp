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
void take_first_to_vector(std::vector<T> &v, std::list<T> &l, size_t k)
{
  list<T> temp;
  temp.splice(temp.begin(), l, l.begin(), next(l.begin(), k));

  v.reserve(k);
  std::copy(begin(temp), end(temp), std::back_inserter(v));
}

template void take_first_to_vector(std::vector<gfp> &v, std::list<gfp> &l, size_t k);
template void take_first_to_vector(std::vector<Share> &v, std::list<Share> &l, size_t k);
template void take_first_to_vector(std::vector<aBit> &v, std::list<aBit> &l, size_t k);
