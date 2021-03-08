/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

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
using namespace std;

/* Takes num items off the front of the input list.
 * Inserts them at front of the output list
 * And produces a vector version of this sublist
 */
template<class T>
void Split_Lists(vector<T> &out_vector,
                 list<T> &out_list,
                 list<T> &input_list,
                 unsigned int size);

/* Does the same but ignores the output list */
template<class T>
void Split_Lists(vector<T> &out_vector,
                 list<T> &input_list,
                 unsigned int size);

/* Does the same but ignores the output vector and output list
 * and just returns the front bit as a list, as well as deleting
 * this from the input_list
 */
template<class T>
void Split_Lists(list<T> &front,
                 list<T> &input_list,
                 unsigned int size);

#endif /* SRC_TOOLS_UTIL_CONTAINERS_H_ */
