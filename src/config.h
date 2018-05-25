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
#define sz_offline_batch 10000
#define sz_IO_batch 1000


/* This are the maximum batch sizes for sacrificing per call
 * 
 * Note we do not need many squares in the end so this max
 * is set quite low
 */

#define sz_triples_sacrifice 20000
#define sz_squares_sacrifice 500
#define sz_bits_sacrifice 20000

/* The max number of triples etc in the offline production queue 
 * These numbers are to avoid memory filling up, if data is not
 * being consumed
 */
#define max_triples_offline 100000
#define max_squares_offline 100000
#define max_bits_offline 100000

/* The max number of triples etc in the sacrifice production queue 
 * where we stop producing stuff.
 * The actual queues may end up being larger, as we dont want to throw
 * good data away
 */
#define max_triples_sacrifice 200000
#define max_squares_sacrifice 200000
#define max_bits_sacrifice 200000
// Following is per player 
#define max_IO_sacrifice 1000

/* The amount of amortization in sacrifice and offline (when we do it) */
#define amortize 512

/* Number of openings between a OpenCheck call */
#define open_check_gap 10000

/* Stat sec parameter */
#define stat_sec 40

/* Computational security parameter for FHE schemes
 *   - Valid values are 80, 128 and 256
 *   - For large plaintext space it might be best to select 80
 */   
#define comp_sec 128

#define MEMORY_DIR "Data/"

#define IO_FUNCTIONALITY Input_Output_Simple
#endif
