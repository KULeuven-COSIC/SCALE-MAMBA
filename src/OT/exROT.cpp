/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/

#include "exROT.h"
#include "COT.h"
#include "Tools/MMO.h"
#include "config.h"

using namespace std::chrono;

void exROT_Sender::init(Player &P, int i, CryptoPP::RandomPool &RNG, vector<int> &choicebits, unsigned int connectionNb)
{
  nb_rcver= i;
  gf2n::init_field(OT_comp_sec);

  COTR.init(P, nb_rcver, RNG, choicebits, connectionNb);
  Delta= COTR.get_Delta();
  aBit::set_nplayers(P.nplayers(), P.whoami(), Delta);

  vector<unsigned int> A(P.nplayers());
  for (unsigned int k= 0; k < P.nplayers(); k++)
    {
      if (k == P.whoami() || k == nb_rcver)
        {
          A[k]= 1;
        }
      else
        {
          A[k]= 0;
        }
    }
  AgreeRandom(P, A, base_key, AES_BLK_SIZE, 1);
  counter= 0;

  cout << "[exROT - Sender::init] Finished Base-OT for Sender (" << P.whoami() << "," << nb_rcver << ")" << endl;
}

void exROT_Receiver::init(Player &P, int i, CryptoPP::RandomPool &RNG, unsigned int connectionNb)
{
  nb_sender= i;
  gf2n::init_field(OT_comp_sec);

  COTS.init(P, nb_sender, RNG, connectionNb);
  vector<unsigned int> A(P.nplayers());
  for (unsigned int k= 0; k < P.nplayers(); k++)
    {
      if (k == P.whoami() || k == nb_sender)
        {
          A[k]= 1;
        }
      else
        {
          A[k]= 0;
        }
    }
  AgreeRandom(P, A, base_key, AES_BLK_SIZE, 1);
  counter= 0;

  cout << "[exRot - Receiver::init] Finished Base-OT for Receiver (" << nb_sender << "," << P.whoami() << ")" << endl;
}

void exROT_Sender::next_iteration(Player &P, unsigned int size, vector<vector<gf2n>> &out_vec)
{
  // Make the MMO key
  uint8_t buffer[AES_BLK_SIZE];
  memset(buffer, 0, AES_BLK_SIZE);
  INT_TO_BYTES(buffer, counter);
  for (unsigned int i= 0; i < AES_BLK_SIZE; i++)
    {
      buffer[i]^= base_key[i];
    }
  counter++;
  mmo.setIV(buffer);

  vector<aBit> aB(size);
  vector<vector<gf2n>> aBLifted(size, vector<gf2n>(2));
  uint8_t *array0;
  uint8_t *array1;

  array0= (uint8_t *) malloc(16 * size * sizeof(uint8_t));
  array1= (uint8_t *) malloc(16 * size * sizeof(uint8_t));

  COTR.next_iteration(P, size, aB);

  for (unsigned int i= 0; i < size; i++)
    {
      aBLifted[i][0]= aB[i].get_Key(nb_rcver);
      aBLifted[i][1]= aB[i].get_Key(nb_rcver) + Delta;
    }

  for (unsigned int i= 0; i < size; i++)
    {
      aBLifted[i][0].store_into_buffer(&array0[16 * i]);
      aBLifted[i][1].store_into_buffer(&array1[16 * i]);
    }

  unsigned int nbBlocks= ceil(static_cast<float>(size) / OT_comp_sec);
  for (unsigned int i= 0; i < nbBlocks; i++)
    {
      mmo.hashBlockWise<gf2n, OT_comp_sec>(array0 + i * 16 * OT_comp_sec, array0 + i * 16 * OT_comp_sec);
      mmo.hashBlockWise<gf2n, OT_comp_sec>(array1 + i * 16 * OT_comp_sec, array1 + i * 16 * OT_comp_sec);
    }

  for (unsigned int i= 0; i < size; i++)
    {
      out_vec[i][0].load_from_buffer(&array0[16 * i]);
      out_vec[i][1].load_from_buffer(&array1[16 * i]);
    }

  free(array0);
  free(array1);
}

void exROT_Receiver::next_iteration(Player &P, unsigned int size, BitVector &choice_vec, vector<gf2n> &out_vec)
{

  // Make the MMO key
  uint8_t buffer[AES_BLK_SIZE];
  memset(buffer, 0, AES_BLK_SIZE);
  INT_TO_BYTES(buffer, counter);
  for (unsigned int i= 0; i < AES_BLK_SIZE; i++)
    {
      buffer[i]^= base_key[i];
    }
  counter++;
  mmo.setIV(buffer);

  vector<aBit> aB(size);
  vector<gf2n> aBLifted(size);
  uint8_t *array;
  array= (uint8_t *) malloc(16 * size * sizeof(uint8_t));

  for (unsigned int i= 0; i < size; i++)
    {
      aB[i].set_value(choice_vec.get_bit(i));
    }

  COTS.next_iteration(P, size, aB, choice_vec);

  for (unsigned int i= 0; i < size; i++)
    {
      aBLifted[i]= aB[i].get_MAC(nb_sender);
      aBLifted[i].store_into_buffer(&array[16 * i]);
    }

  unsigned int nbBlocks= ceil(static_cast<float>(size) / OT_comp_sec);
  for (unsigned int i= 0; i < nbBlocks; i++)
    {
      mmo.hashBlockWise<gf2n, OT_comp_sec>(array + i * 16 * OT_comp_sec, array + i * 16 * OT_comp_sec);
    }

  for (unsigned int i= 0; i < size; i++)
    {
      out_vec[i].load_from_buffer(&array[16 * i]);
    }
  free(array);
}
