/**
 * \file subtreeconstr_atk.c
 *
 * \brief SPHINCS-2556 subtree construction simulation for fault attack.
 *
 * Copyright (c) 2011-2016 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#define WITH_SOFTWARE_FAULT

#include "delay.h"
#include "asf.h"
#include "stdio_serial.h"
#include "conf_board.h"
#include "conf_clock.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "parameters.h"
#include "prf.h"
#include "hash.h"
#include "trees.h"
#include "wotsp.h"
#ifdef WITH_SOFTWARE_FAULT
# include <stdlib.h>
#endif /* WITH_SOFTWARE_FAULT */

/* UART communication */
#define WAITING_CHAR 0xaa
#define SENDING_CHAR 0xbb

static unsigned char readchar[2] = {'\0', '\0'};

#define READ_SHORT(target) do { \
	readchar[0] = fgetc(stdin); \
	readchar[1] = fgetc(stdin); \
	target = (((uint16_t) readchar[0]) << 8) | (readchar[1]); \
} while (0)

#define SEND_SHORT(target) do { \
	readchar[0] = (unsigned char) (target >> 8); \
	printf("%c", readchar[0]); \
	readchar[1] = (unsigned char) (target & 0x00ff); \
	printf("%c", readchar[1]); \
} while (0)

#define READ_BYTE(target) do { \
	readchar[0] = fgetc(stdin); \
	target = readchar[0]; \
} while (0)

#define SEND_BYTE(target) do { \
	readchar[0] = (unsigned char) (target); \
	printf("%c", readchar[0]); \
} while (0)

#define WAIT_FOR_CHAR do { \
	while ((readchar[0] = fgetc(stdin)) != WAITING_CHAR); \
	printf("%c", SENDING_CHAR); \
} while (0)

/* GPIO toggle */
#define PIO_TRIGGER PIO_PA14_IDX
#define PIO_TOGGLE PIO_PA7_IDX

static int toggled = 0;
static int triggered = 0;

#define TOGGLE do { \
	if (toggled) { \
		ioport_set_pin_level(PIO_TOGGLE, 1); \
		ioport_set_pin_level(PIO_TOGGLE, 0); \
		toggled = 0; \
	} else { \
		ioport_set_pin_level(PIO_TOGGLE, 0); \
		ioport_set_pin_level(PIO_TOGGLE, 1); \
		toggled = 1; \
	} \
} while (0)

#define TRIGGER do { \
	if (triggered) { \
		ioport_set_pin_level(PIO_TRIGGER, 1); \
		ioport_set_pin_level(PIO_TRIGGER, 0); \
		triggered = 0; \
	} else { \
		ioport_set_pin_level(PIO_TRIGGER, 0); \
		ioport_set_pin_level(PIO_TRIGGER, 1); \
		triggered = 1; \
	} \
} while (0)


/* Delay in between targeted operation (time domain isolation) */
#define OP_DELAY_US 1

/**
 *  Configures UART console.
 */

static void configure_console(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
	#ifdef CONF_UART_CHAR_LENGTH
		.charlength = CONF_UART_CHAR_LENGTH,
	#endif
		.paritytype = CONF_UART_PARITY,
	#ifdef CONF_UART_STOP_BITS
		.stopbits = CONF_UART_STOP_BITS,
	#endif
	};

	/* Configures console UART. */
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
	stdio_serial_init(CONF_UART, &uart_serial_options);
}

unsigned char const constant_masks[MASKS_BYTES] =
{
	0x50, 0x41, 0x4C, 0x50, 0x41, 0x54, 0x49, 0x4E, 
	0x45, 0x3A, 0x20, 0x28, 0x63, 0x6F, 0x6E, 0x74, 
	0x69, 0x6E, 0x75, 0x69, 0x6E, 0x67, 0x29, 0x20, 
	0x44, 0x69, 0x64, 0x20, 0x79, 0x6F, 0x75, 0x20, 

	0x65, 0x76, 0x65, 0x72, 0x20, 0x68, 0x65, 0x61, 
	0x72, 0x20, 0x74, 0x68, 0x65, 0x20, 0x74, 0x72, 
	0x61, 0x67, 0x65, 0x64, 0x79, 0x20, 0x6F, 0x66, 
	0x20, 0x44, 0x61, 0x72, 0x74, 0x68, 0x20, 0x50, 

	0x6C, 0x61, 0x67, 0x75, 0x65, 0x69, 0x73, 0x20, 
	0x22, 0x74, 0x68, 0x65, 0x20, 0x77, 0x69, 0x73, 
	0x65, 0x22, 0x3F, 0x0A, 0x0A, 0x41, 0x4E, 0x41, 
	0x4B, 0x49, 0x4E, 0x3A, 0x20, 0x4E, 0x6F, 0x2E, 

	0x0A, 0x0A, 0x50, 0x41, 0x4C, 0x50, 0x41, 0x54, 
	0x49, 0x4E, 0x45, 0x3A, 0x20, 0x49, 0x20, 0x74, 
	0x68, 0x6F, 0x75, 0x67, 0x68, 0x74, 0x20, 0x6E, 
	0x6F, 0x74, 0x2E, 0x20, 0x49, 0x74, 0x27, 0x73, 

	0x20, 0x6E, 0x6F, 0x74, 0x20, 0x61, 0x20, 0x73, 
	0x74, 0x6F, 0x72, 0x79, 0x20, 0x74, 0x68, 0x65, 
	0x20, 0x4A, 0x65, 0x64, 0x69, 0x20, 0x77, 0x6F, 
	0x75, 0x6C, 0x64, 0x20, 0x74, 0x65, 0x6C, 0x6C, 

	0x20, 0x79, 0x6F, 0x75, 0x2E, 0x20, 0x49, 0x74, 
	0x27, 0x73, 0x20, 0x61, 0x20, 0x53, 0x69, 0x74, 
	0x68, 0x20, 0x6C, 0x65, 0x67, 0x65, 0x6E, 0x64, 
	0x2E, 0x20, 0x44, 0x61, 0x72, 0x74, 0x68, 0x20, 

	0x50, 0x6C, 0x61, 0x67, 0x75, 0x65, 0x69, 0x73, 
	0x20, 0x77, 0x61, 0x73, 0x20, 0x61, 0x20, 0x44, 
	0x61, 0x72, 0x6B, 0x20, 0x4C, 0x6F, 0x72, 0x64, 
	0x20, 0x6F, 0x66, 0x20, 0x74, 0x68, 0x65, 0x20, 

	0x53, 0x69, 0x74, 0x68, 0x2C, 0x20, 0x73, 0x6F, 
	0x20, 0x70, 0x6F, 0x77, 0x65, 0x72, 0x66, 0x75, 
	0x6C, 0x20, 0x61, 0x6E, 0x64, 0x20, 0x73, 0x6F, 
	0x20, 0x77, 0x69, 0x73, 0x65, 0x20, 0x68, 0x65, 

	0x20, 0x63, 0x6F, 0x75, 0x6C, 0x64, 0x20, 0x75, 
	0x73, 0x65, 0x20, 0x74, 0x68, 0x65, 0x20, 0x46, 
	0x6F, 0x72, 0x63, 0x65, 0x20, 0x74, 0x6F, 0x20, 
	0x69, 0x6E, 0x66, 0x6C, 0x75, 0x65, 0x6E, 0x63, 

	0x65, 0x20, 0x74, 0x68, 0x65, 0x20, 0x6D, 0x69, 
	0x64, 0x69, 0x2D, 0x63, 0x68, 0x6C, 0x6F, 0x72, 
	0x69, 0x61, 0x6E, 0x73, 0x20, 0x74, 0x6F, 0x20, 
	0x63, 0x72, 0x65, 0x61, 0x74, 0x65, 0x20, 0x6C, 

	0x69, 0x66, 0x65, 0x20, 0x2E, 0x2E, 0x2E, 0x20, 
	0x48, 0x65, 0x20, 0x68, 0x61, 0x64, 0x20, 0x73, 
	0x75, 0x63, 0x68, 0x20, 0x61, 0x20, 0x6B, 0x6E, 
	0x6F, 0x77, 0x6C, 0x65, 0x64, 0x67, 0x65, 0x20, 

	0x6F, 0x66, 0x20, 0x74, 0x68, 0x65, 0x20, 0x64, 
	0x61, 0x72, 0x6B, 0x20, 0x73, 0x69, 0x64, 0x65, 
	0x20, 0x74, 0x68, 0x61, 0x74, 0x20, 0x68, 0x65, 
	0x20, 0x63, 0x6F, 0x75, 0x6C, 0x64, 0x20, 0x65, 

	0x76, 0x65, 0x6E, 0x20, 0x6B, 0x65, 0x65, 0x70, 
	0x20, 0x74, 0x68, 0x65, 0x20, 0x6F, 0x6E, 0x65, 
	0x73, 0x20, 0x68, 0x65, 0x20, 0x63, 0x61, 0x72, 
	0x65, 0x64, 0x20, 0x61, 0x62, 0x6F, 0x75, 0x74, 

	0x20, 0x66, 0x72, 0x6F, 0x6D, 0x20, 0x64, 0x79, 
	0x69, 0x6E, 0x67, 0x2E, 0x0A, 0x0A, 0x41, 0x4E, 
	0x41, 0x4B, 0x49, 0x4E, 0x3A, 0x20, 0x48, 0x65, 
	0x20, 0x63, 0x6F, 0x75, 0x6C, 0x64, 0x20, 0x61, 

	0x63, 0x74, 0x75, 0x61, 0x6C, 0x6C, 0x79, 0x20, 
	0x73, 0x61, 0x76, 0x65, 0x20, 0x70, 0x65, 0x6F, 
	0x70, 0x6C, 0x65, 0x20, 0x66, 0x72, 0x6F, 0x6D, 
	0x20, 0x64, 0x65, 0x61, 0x74, 0x68, 0x3F, 0x0A, 

	0x0A, 0x50, 0x41, 0x4C, 0x50, 0x41, 0x54, 0x49, 
	0x4E, 0x45, 0x3A, 0x20, 0x54, 0x68, 0x65, 0x20, 
	0x64, 0x61, 0x72, 0x6B, 0x20, 0x73, 0x69, 0x64, 
	0x65, 0x20, 0x6F, 0x66, 0x20, 0x74, 0x68, 0x65, 

	0x20, 0x46, 0x6F, 0x72, 0x63, 0x65, 0x20, 0x69, 
	0x73, 0x20, 0x61, 0x20, 0x70, 0x61, 0x74, 0x68, 
	0x77, 0x61, 0x79, 0x20, 0x74, 0x6F, 0x20, 0x6D, 
	0x61, 0x6E, 0x79, 0x20, 0x61, 0x62, 0x69, 0x6C, 

	0x69, 0x74, 0x69, 0x65, 0x73, 0x20, 0x73, 0x6F, 
	0x6D, 0x65, 0x20, 0x63, 0x6F, 0x6E, 0x73, 0x69, 
	0x64, 0x65, 0x72, 0x20, 0x74, 0x6F, 0x20, 0x62, 
	0x65, 0x20, 0x75, 0x6E, 0x6E, 0x61, 0x74, 0x75, 

	0x72, 0x61, 0x6C, 0x2E, 0x0A, 0x0A, 0x41, 0x4E, 
	0x41, 0x4B, 0x49, 0x4E, 0x3A, 0x20, 0x57, 0x68, 
	0x61, 0x74, 0x20, 0x68, 0x61, 0x70, 0x70, 0x65, 
	0x6E, 0x65, 0x64, 0x20, 0x74, 0x6F, 0x20, 0x68, 

	0x69, 0x6D, 0x3F, 0x0A, 0x0A, 0x50, 0x41, 0x4C, 
	0x50, 0x41, 0x54, 0x49, 0x4E, 0x45, 0x3A, 0x20, 
	0x48, 0x65, 0x20, 0x62, 0x65, 0x63, 0x61, 0x6D, 
	0x65, 0x20, 0x73, 0x6F, 0x20, 0x70, 0x6F, 0x77, 

	0x65, 0x72, 0x66, 0x75, 0x6C, 0x20, 0x2E, 0x20, 
	0x2E, 0x20, 0x2E, 0x20, 0x74, 0x68, 0x65, 0x20, 
	0x6F, 0x6E, 0x6C, 0x79, 0x20, 0x74, 0x68, 0x69, 
	0x6E, 0x67, 0x20, 0x68, 0x65, 0x20, 0x77, 0x61, 

	0x73, 0x20, 0x61, 0x66, 0x72, 0x61, 0x69, 0x64, 
	0x20, 0x6F, 0x66, 0x20, 0x77, 0x61, 0x73, 0x20, 
	0x6C, 0x6F, 0x73, 0x69, 0x6E, 0x67, 0x20, 0x68, 
	0x69, 0x73, 0x20, 0x70, 0x6F, 0x77, 0x65, 0x72, 

	0x2C, 0x20, 0x77, 0x68, 0x69, 0x63, 0x68, 0x20, 
	0x65, 0x76, 0x65, 0x6E, 0x74, 0x75, 0x61, 0x6C, 
	0x6C, 0x79, 0x2C, 0x20, 0x6F, 0x66, 0x20, 0x63, 
	0x6F, 0x75, 0x72, 0x73, 0x65, 0x2C, 0x20, 0x68, 

	0x65, 0x20, 0x64, 0x69, 0x64, 0x2E, 0x20, 0x55, 
	0x6E, 0x66, 0x6F, 0x72, 0x74, 0x75, 0x6E, 0x61, 
	0x74, 0x65, 0x6C, 0x79, 0x2C, 0x20, 0x68, 0x65, 
	0x20, 0x74, 0x61, 0x75, 0x67, 0x68, 0x74, 0x20, 

	0x68, 0x69, 0x73, 0x20, 0x61, 0x70, 0x70, 0x72, 
	0x65, 0x6E, 0x74, 0x69, 0x63, 0x65, 0x20, 0x65, 
	0x76, 0x65, 0x72, 0x79, 0x74, 0x68, 0x69, 0x6E, 
	0x67, 0x20, 0x68, 0x65, 0x20, 0x6B, 0x6E, 0x65, 

	0x77, 0x2C, 0x20, 0x74, 0x68, 0x65, 0x6E, 0x20, 
	0x68, 0x69, 0x73, 0x20, 0x61, 0x70, 0x70, 0x72, 
	0x65, 0x6E, 0x74, 0x69, 0x63, 0x65, 0x20, 0x6B, 
	0x69, 0x6C, 0x6C, 0x65, 0x64, 0x20, 0x68, 0x69, 

	0x6D, 0x20, 0x69, 0x6E, 0x20, 0x68, 0x69, 0x73, 
	0x20, 0x73, 0x6C, 0x65, 0x65, 0x70, 0x2E, 0x20, 
	0x28, 0x73, 0x6D, 0x69, 0x6C, 0x65, 0x73, 0x29, 
	0x20, 0x50, 0x6C, 0x61, 0x67, 0x75, 0x65, 0x69, 

	0x73, 0x20, 0x6E, 0x65, 0x76, 0x65, 0x72, 0x20, 
	0x73, 0x61, 0x77, 0x20, 0x69, 0x74, 0x20, 0x63, 
	0x6F, 0x6D, 0x69, 0x6E, 0x67, 0x2E, 0x20, 0x49, 
	0x74, 0x27, 0x73, 0x20, 0x69, 0x72, 0x6F, 0x6E, 

	0x69, 0x63, 0x20, 0x68, 0x65, 0x20, 0x63, 0x6F, 
	0x75, 0x6C, 0x64, 0x20, 0x73, 0x61, 0x76, 0x65, 
	0x20, 0x6F, 0x74, 0x68, 0x65, 0x72, 0x73, 0x20, 
	0x66, 0x72, 0x6F, 0x6D, 0x20, 0x64, 0x65, 0x61, 

	0x74, 0x68, 0x2C, 0x20, 0x62, 0x75, 0x74, 0x20, 
	0x6E, 0x6F, 0x74, 0x20, 0x68, 0x69, 0x6D, 0x73, 
	0x65, 0x6C, 0x66, 0x2E, 0x0A, 0x0A, 0x41, 0x4E, 
	0x41, 0x4B, 0x49, 0x4E, 0x3A, 0x20, 0x49, 0x73, 

	0x20, 0x69, 0x74, 0x20, 0x70, 0x6F, 0x73, 0x73, 
	0x69, 0x62, 0x6C, 0x65, 0x20, 0x74, 0x6F, 0x20, 
	0x6C, 0x65, 0x61, 0x72, 0x6E, 0x20, 0x74, 0x68, 
	0x69, 0x73, 0x20, 0x70, 0x6F, 0x77, 0x65, 0x72, 

	0x3F, 0x0A, 0x0A, 0x50, 0x41, 0x4C, 0x50, 0x41, 
	0x54, 0x49, 0x4E, 0x45, 0x3A, 0x20, 0x4E, 0x6F, 
	0x74, 0x20, 0x66, 0x72, 0x6F, 0x6D, 0x20, 0x61, 
	0x20, 0x4A, 0x65, 0x64, 0x69, 0x2E, 0x0A, 0x0A
};

static void subtree_constr(unsigned char const sk1[SEED_BYTES],
                           unsigned char const addr_in[SPHINCS_ADDRESS_BYTES],
                           unsigned char const masks[MASKS_BYTES])
{
	int h = 0, i = 0, j = 0, l = 0;
	unsigned char a = 0;
	unsigned char leafidx = 0;
	unsigned char seed[SEED_BYTES];
	unsigned char path[MSS_TREE_HEIGHT*SPHINCS_BYTES];
	unsigned char leaves[MSS_TOTAL_LEAVES*SPHINCS_BYTES];
	unsigned char wots_pk[WOTS_L*SPHINCS_BYTES];
	unsigned char addr[SPHINCS_ADDRESS_BYTES];

	/* Copies addr_in into addr */
	memcpy(addr, addr_in, SPHINCS_ADDRESS_BYTES);
	a = (addr[0] >> 4) & 0x0f;

	/* Recovers the leaf index which will sign the previous root */
	leafidx = addr[SPHINCS_ADDRESS_BYTES - 1] & 0x1f;

	TOGGLE;
	/* Recovers all the leaves from subtree */
	for (l = 0; l < MSS_TOTAL_LEAVES; ++l)
	{
		/* Recovers seed at address given */
		addr[SPHINCS_ADDRESS_BYTES - 1] = (addr[SPHINCS_ADDRESS_BYTES - 1] & 0xe0) | l;
		prf_64(seed, addr, sk1);

		/* Generates WOTS+ public key and compresses it to the root of an L-tree */
		wotsp_keygen(wots_pk, seed, masks);
		TREE_CONSTRUCTION_MASK(i, j, h, WOTS_L, wots_pk, masks);

		/* Stores WOTS+ compressed key as leaf of top tree */
		for (i = 0; i < SPHINCS_BYTES; ++i)
		{
			leaves[l*SPHINCS_BYTES + i] = wots_pk[0*SPHINCS_BYTES + i];
		}
	}

	/* Outputs MSS signature */
	for (i = 0; i < MSS_TREE_HEIGHT; ++i)
	{
		/* Computes authentication id in leafidx */
		if ((leafidx & 1) == 0)
		{
			leafidx += 1;
		}
		else
		{
			leafidx -= 1;
		}

		/* Stores authentication path */
		for (j = 0; j < SPHINCS_BYTES; ++j)
		{
			path[i*SPHINCS_BYTES + j] = leaves[leafidx*SPHINCS_BYTES + j];
		}

		/* Constructs tree */
		for (j = 0; j < (1 << (MSS_TREE_HEIGHT - i)); j += 2)
		{
			if (i == 2 && j == 2)
			{
				TRIGGER;
#ifdef WITH_SOFTWARE_FAULT
        unsigned char mask[2*SPHINCS_BYTES];
        memcpy(mask, masks + 2*(WOTS_LOG_L + i)*SPHINCS_BYTES, 2*SPHINCS_BYTES);
        if (rand() % 2 == 1) {
          mask[2*SPHINCS_BYTES-1] = mask[2*SPHINCS_BYTES-1] ^ (rand() & 0xff);
        }
        hash_nn_n_mask(leaves + (j/2)*SPHINCS_BYTES, leaves + (j)*SPHINCS_BYTES,
                       leaves + (j+1)*SPHINCS_BYTES, mask);
#else
        hash_nn_n_mask(leaves + (j/2)*SPHINCS_BYTES, leaves + (j)*SPHINCS_BYTES,
                       leaves + (j+1)*SPHINCS_BYTES, masks + 2*(WOTS_LOG_L + i)*SPHINCS_BYTES);
#endif /* WITH_SOFTWARE_FAULT */
				TRIGGER;
			}
			else {
				
				hash_nn_n_mask(leaves + (j/2)*SPHINCS_BYTES, leaves + (j)*SPHINCS_BYTES,
				               leaves + (j+1)*SPHINCS_BYTES, masks + 2*(WOTS_LOG_L + i)*SPHINCS_BYTES);
			}
		}

		/* Authentication node from next layer */
		leafidx /= 2;
	}
	TOGGLE;

	/* Prepares for W-OTS+ signature */
	for (i = SPHINCS_ADDRESS_BYTES - 1; i > 1; --i)
	{
		addr[i] = ((addr[i - 1] & 0x1f) << 3) | ((addr[i] & 0xe0) >> 5);
	}
	addr[1] = ((addr[0] & 0x0f) << 3) | ((addr[1] & 0xe0) >> 5);
	addr[0] = ((a+1) << 4) & 0xf0;
	prf_64(seed, addr, sk1);

	/* Prints authentication path */
	for (i = 0; i < SPHINCS_BYTES*MSS_TREE_HEIGHT; ++i)
	{
		printf("%c", path[i]);
	}

	/* Prints W-OTS+ signature */
	wotsp_sign(leaves + 0*SPHINCS_BYTES, seed, masks);
}

/**
 *  Simulates SPHINCS-256 first subtree construction.
 */

static void subtree_attack(uint16_t count, uint16_t del_ms)
{
	int i = 0;
	unsigned char sk1[SEED_BYTES];
	unsigned char addr[SPHINCS_ADDRESS_BYTES];

	for (i = 0; i < SEED_BYTES; ++i)
	{
		READ_BYTE(sk1[i]);
	}

	for (i = 0; i < SPHINCS_ADDRESS_BYTES; ++i)
	{
		READ_BYTE(addr[i]);
	}

	for (i = 0; i < SEED_BYTES; ++i)
	{
		SEND_BYTE(sk1[i]);
	}

	for (i = 0; i < SPHINCS_ADDRESS_BYTES; ++i)
	{
		SEND_BYTE(addr[i]);
	}

	for (i = 0; i < count; ++i)
	{
		subtree_constr(sk1, addr, constant_masks);

		delay_ms(del_ms);
	}
}

/**
 *  Simulation entry point.
 *
 *  \return Unused (ANSI-C compatibility).
 */

int main(void)
{
	/* Function to run */
	void (*run_func)(uint16_t, uint16_t) = subtree_attack;

	/* Initializes the SAM system */
	sysclk_init();
	board_init();
	configure_console();
	ioport_init();

	/* Directs pin and initializes to zero */
	ioport_set_pin_dir(PIO_TRIGGER, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(PIO_TRIGGER, 0);
	ioport_set_pin_dir(PIO_TOGGLE, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(PIO_TRIGGER, 0);

	/* Tests trigger */
	TRIGGER;
	TOGGLE;
	delay_us(OP_DELAY_US);
	TRIGGER;
	TOGGLE;

	while (1)
	{
		uint16_t count = 0;
		uint16_t del_ms = 0;

		/* Waits for 0xaa and responds with 0xbb */
		WAIT_FOR_CHAR;

		READ_SHORT(count);
		SEND_SHORT(count);
		READ_SHORT(del_ms);
		SEND_SHORT(del_ms);

		(run_func)(count, del_ms);
	}

	return 0;
}
