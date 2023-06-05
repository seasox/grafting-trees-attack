/*
 * trees.h
 *
 */

#include "parameters.h"
#include <string.h>

#ifndef TREES_H_
#define TREES_H_

#define INT_CEIL(x, y) 1 + ((x - 1) / y)

#define TREE_CONSTRUCTION_MASK(i, j, h, total, leaves, masks) do { \
	for (i = total, h = 0; i > 1; i = INT_CEIL(i, 2), h++) { \
		for (j = 0; j < i; j += 2) { \
			if (j + 2 > i) { \
				memcpy(leaves + (j/2)*SPHINCS_BYTES, leaves + (j)*SPHINCS_BYTES, SPHINCS_BYTES); \
			} else { \
				hash_nn_n_mask(leaves + (j/2)*SPHINCS_BYTES, leaves + (j)*SPHINCS_BYTES, \
				               leaves + (j+1)*SPHINCS_BYTES, \
				               masks + (2*h)*SPHINCS_BYTES); \
			} \
		} \
	} \
} while(0)

#endif /* TREES_H_ */
