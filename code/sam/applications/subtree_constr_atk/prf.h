/*
 * prf.h
 *
 */

#include "parameters.h"

#ifndef PRF_H_
#define PRF_H_

/*
 * Applies ensemble of pseudorandom functions to the concatenation of in1 and
 * in2.
 *
 * The purpose of this function is to derive pseudorandom seeds for the
 * generation of WOTS+ secret values, where in1 is the address of the WOTS+
 * leaf in 64 bits, and in2 the first SPHINCS secret seed sk1.
 */

int prf_64(unsigned char out[SPHINCS_BYTES],
           unsigned char const in1[SPHINCS_ADDRESS_BYTES],
           unsigned char const in2[SEED_BYTES]);

#endif /* PRF_H_ */
