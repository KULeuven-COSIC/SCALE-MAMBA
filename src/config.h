/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2020, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/


#ifndef _CONFIG
#define _CONFIG

/* The minimum batch size for offline production per call 
 * to base routine. 
 */
#define sz_offline_batch 60000
#define sz_IO_batch 10000

/* The max number of triples etc in the sacrifice production queue 
 * where we stop producing stuff.
 * The actual queues may end up being larger, as we dont want to throw
 * good data away
 */
#define max_triples_sacrifice 5000000
#define max_squares_sacrifice 5000000
#define max_bits_sacrifice 5000000
// Following is per player 
#define max_IO_sacrifice 100000

/* The amount of amortization in sacrifice and offline (when we do it) */
#define amortize 512

/* Number of openings between a OpenCheck call */
#define open_check_gap 10000


/****************************
 * Stat security parameters *
 ****************************/

/* Sacrifice stat_sec parameter. 
 * Defines how many sacrifices we do per triple etc (need to 
 * divide by log_2 p to get the number of sacrifices)
 *   - So it is basically log_2 p unless p is small
 */
#define sacrifice_stat_sec 80

/* Number of macs stat_sec.
 * This defines how many macs we use in the full threshold case
 * per data item, again need to divide by log_2 p.
 * So for log_2 p>macs_stat_sec this then makes 1 MAC
 *   - So it is basically log_2 p unless p is small
 */
#define macs_stat_sec 80

/* The ZKPoK sound_sec.
 * Gives the soundness error of the ZKPoKs in the full 
 * threshold case. This is the one which affects offline 
 * runtime the most
 */
#define ZK_sound_sec 128

/* The ZKPoK ZK_sec. This gives the statistical
 * distance between the ZK simulation and the real
 * distribution of transcripts. This does not affect
 * parameters or run time that much
 */
#define ZK_ZK_sec 80


/* The Distributed Decryption stat_sec
 * This defines the closeness of the distribution produced
 * during distributed decryption to uniform in the full
 * threshold case.
 */
#define DD_stat_sec 80


/* Note stat security parameter for fixed/floating point numbers
 * is defined in the compiler. To alter this from the default
 * of 40 you need to use the following command in MAMBA...
 *     program.security = 80
 */



/* Defines the probability that an FHE ciphertext might decrypt
 * incorrectly. This is bounded by 2^{-FHE_epsilon}
 */
#define FHE_epsilon 55


/* Computational security parameter for FHE schemes
 *   - Valid values are 80, 128 and 256
 *   - For large plaintext space it might be best to select 80
 */   
#define comp_sec 128

/* The OT_comp_sec has to be equal to 128, as we only support
 * gf2n over 128 bit fields. Fix this and this can be any value
 * you want.
 */
#define OT_comp_sec 128

/* This is the value rho from the paper of Wang, Ranellucci and Katz 
 *  - This can be any value we want, it basically defines the size
 *    of the buckets used in the aAND protocol
 * Wang et al choose 40 here, so we will too
 */
#define OT_stat_sec 40

/* This is the bound we use on for the NewHope approximation
 * to a discrete Gaussian with sigma=sqrt(B/2)
 */
#define NewHopeB 1

/* This gives the Hamming Weight of the secret key distribution.
 * Set this to 0 to use a Gaussian secret key distribution
 */
#define HwtSK 64

/* Approx max size of each aBit and aAND queues in the OT thread */
#define max_aBit_queue 500000
#define max_aAND_queue 200000

/* This says use the SimpleROT from ePrint 2015/267 instead of the DMC 
 * version (the latter introduces a problem in our usage)
 */
#define SimpleOT

/* Number of daBits per loop. This is used to determine the Cut-and-Choose
 * parameters such as bucket size, and how many inputs are needed for
 * production.
 */
#define kdaBitsPerLoop 8192

/* We use the same statistical security as the WRK protocol
 * for the dABit bucketing procedure
 */
#define daBits_stat_sec 40

/* This defines the statistical security for LSSS<->GC conversion */
#define conv_stat_sec 40

#define MEMORY_DIR "Data/"

/* This tells the runtime to use the TopGear variant of the ZKPoKs */
#define TOP_GEAR

/* This gives the max number of iterations which we use to simplify
 * a circuit using Search_SubCircuit. Only used to simplify the modp
 * circuit
 */
#define iter_modp_Search_SubCircuit 60

#endif
