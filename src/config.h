/*
Copyright (c) 2017, The University of Bristol, Senate House, Tyndall Avenue, Bristol, BS8 1TH, United Kingdom.
Copyright (c) 2018, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.

All rights reserved
*/


#ifndef _CONFIG
#define _CONFIG

/* The minimum batch size for offline production per call 
 * to base routine. For FHE based routines these are likely to
 * be exceeded by a huge amount!
 */
#define sz_offline_batch 60000
#define sz_IO_batch 1000


/* This are the maximum batch sizes for sacrificing per call
 * 
 * Note we do not need many squares in the end so this max
 * can be set quite low if we want
 */

#define sz_triples_sacrifice 10000
#define sz_squares_sacrifice 10000
#define sz_bits_sacrifice 10000

/* The max number of triples etc in the offline production queue 
 * These numbers are to avoid memory filling up, if data is not
 * being consumed. These are the size of the queues before sacrificing
 */
#define max_triples_offline 200000
#define max_squares_offline 200000
#define max_bits_offline 200000

/* The max number of triples etc in the sacrifice production queue 
 * where we stop producing stuff.
 * The actual queues may end up being larger, as we dont want to throw
 * good data away
 */
#define max_triples_sacrifice 5000000
#define max_squares_sacrifice 5000000
#define max_bits_sacrifice 5000000
// Following is per player 
#define max_IO_sacrifice 1000

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
#define sacrifice_stat_sec 40

/* Number of macs stat_sec.
 * This defines how many macs we use in the full threshold case
 * per data item, again need to divide by log_2 p.
 * So for log_2 p>macs_stat_sec this then makes 1 MAC
 *   - So it is basically log_2 p unless p is small
 */
#define macs_stat_sec 40

/* The ZKPoK stat_sec.
 * Basically gives the soundness error of the ZKPoKs
 * in the full threshold case. This is the one which
 * affects offline runtime the most
 */
#define ZK_stat_sec 40

/* The Distributed Decryption stat_sec
 * This defines the closeness of the distribution produced
 * during distributed decryption to uniform in the full
 * threshold case.
 */
#define DD_stat_sec 40


/* Note stat security parameter for fixed/floating point numbers
 * is defined in the compiler. Do alter this from the default
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

#define MEMORY_DIR "Data/"

#endif
