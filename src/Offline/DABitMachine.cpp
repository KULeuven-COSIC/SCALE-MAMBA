/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

/*
 * DABitMachine.cpp
 *
 *  Created on: Mar 5, 2019
 *      Author: drotaru
 */

#include "DABitMachine.h"
#include "config.h"

DABitMachineBase::DABitMachineBase() : nBitsPerLoop(kdaBitsPerLoop), sec(daBits_stat_sec), cnc_param(0),
                                       bucket_size(0)
{
}

MaliciousDABitMachine::MaliciousDABitMachine() : n_parties(0), OCD(0)
{
}

void MaliciousDABitMachine::Initialize(uint nparties, offline_control_data &_OCD)
{
  this->OCD= &_OCD;

  n_parties= nparties;

  if (numBits(gfp::pr()) >= 64)
    {
      //We need to compute gamma and Delta
      //Since p_{min} = 2, we have gamma = sec + 1
      gamma= sec + 1;

      //Delta = ceil(p/nbParties)
      bigint n_parties_bigint= n_parties;
      Delta= div_c(gfp::pr(), n_parties_bigint);
    }
  else
    {
      pre_cnc_params[make_pair(40, 8192)]= make_pair(2, 3);
      pre_cnc_params[make_pair(64, 8192)]= make_pair(5, 4);
      pre_cnc_params[make_pair(80, 8192)]= make_pair(5, 5);
      find_cnc_params();
    }
}

void MaliciousDABitMachine::find_cnc_params()
{
  //check if bounds are already computed
  pair<int, int> param_check= make_pair(sec, nBitsPerLoop);
  if (pre_cnc_params.find(param_check) != pre_cnc_params.end())
    {
      // job done: assign parameters and then quit function
      cnc_param= pre_cnc_params[param_check].first;
      bucket_size= pre_cnc_params[param_check].second;
      return;
    }
  // otherwise need to compute cnc parameters using binomial
  // First step: Compute 2^sec
  bigint sec_bound;
  bigint two= 2;
  mpz_pow_ui(sec_bound.get_mpz_t(), two.get_mpz_t(), sec);

  int max_c= 10; // maximum size of c for bucketing
  int max_b= 10; // maximum size of bucket

  double min_preproc_cost= 1 << 20; // set a big number for triple cost

  // assuming input production in Fp is ~15 times faster than
  // producing triples
  double input_triple_factor= 15.0;

  for (int c= 2; c < max_c; c++)
    {
      for (int b= 2; b < max_b; b++)
        {
          bigint big_c= c;
          bigint achieved_sec;
          mpz_pow_ui(achieved_sec.get_mpz_t(), big_c.get_mpz_t(), b);
          achieved_sec= achieved_sec * compute_binomial(b * nBitsPerLoop, b);

          if (achieved_sec >= sec_bound)
            {
              // We have reached at least 2^sec security for CNC
              double cur_preproc_cost= (n_parties * b * c) / input_triple_factor;
              cur_preproc_cost+= (n_parties - 1) * b + b - 1;
              if (cur_preproc_cost < min_preproc_cost)
                {
                  min_preproc_cost= cur_preproc_cost;
                  cnc_param= c;
                  bucket_size= b;
                }
            }
        }
    }
  //cout << "Selected for bucketing: C = " << cnc_param << " B = " << bucket_size << endl;
}

AbstractDABitGenerator *MaliciousDABitMachine::new_generator(Player &P, int thread_num)
{
  if (numBits(gfp::pr()) >= 64)
    return new LargePrimeDABitGenerator(*this, P, thread_num);
  else
    return new SmallPrimeDABitGenerator(*this, P, thread_num);
}
