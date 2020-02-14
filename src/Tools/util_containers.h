/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

/*
 * containers.h
 *
 *  Created on: Mar 6, 2019
 *      Author: drotaru
 */

#ifndef SRC_TOOLS_UTIL_CONTAINERS_H_
#define SRC_TOOLS_UTIL_CONTAINERS_H_

#include <list>
#include <vector>

template<class T>
void take_first_to_vector(std::vector<T> &v, std::list<T> &l, std::size_t k);

#endif /* SRC_TOOLS_UTIL_CONTAINERS_H_ */
