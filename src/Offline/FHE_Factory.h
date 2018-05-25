/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/
#ifndef _FHE_Factory
#define _FHE_Factory

/* These functions allow the control of the FHE Factor
 * which produces "valid" ciphertexts via the ZKPoK
 * machinary.
 *
 * We have only one factory for the entire system.
 * This could cause a problem if the field size is small
 * AND the number of threads is large. But the probability
 * of this in an application is quite small one suspects.
 */

#include "FHE/FHE_Keys.h"
#include "System/Player.h"
#include "offline_data.h"

void FHE_Factory(Player &P, const offline_control_data &OCD, const FHE_PK &pk,
                 const FFT_Data &PTD);

int Next_Off_Production_Line(Plaintext &mess, Ciphertext &ctx,
                             const Player &P, int num_online,
                             offline_control_data &OCD);

#endif
