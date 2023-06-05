/*
 * prng.c
 *
 */

#include "parameters.h"
#include "crypto_chacha12.h"
#include "ecrypt-sync.h"
#include "prng.h"
#include <string.h>

/* Nonce for ChaCha12 */
static unsigned char nonce[CHACHA12_NONCE_BYTES] = {0};

void prng(unsigned char * out, unsigned long long const outlen, unsigned char const seed[SEED_BYTES])
{
	ECRYPT_ctx tmp;
	ECRYPT_keysetup(&tmp, seed, 8*CHACHA12_KEY_BYTES, 8*CHACHA12_NONCE_BYTES);
	ECRYPT_ivsetup(&tmp, nonce);
	ECRYPT_keystream_bytes(&tmp, out, outlen);
}
