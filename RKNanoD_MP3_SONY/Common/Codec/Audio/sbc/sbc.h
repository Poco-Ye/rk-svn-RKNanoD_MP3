/*
 *
 *  Bluetooth low-complexity, subband codec (SBC) library
 *
 *  Copyright (C) 2008-2010  Nokia Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2004-2005  Henryk Ploetz <henryk@ploetzli.ch>
 *  Copyright (C) 2005-2006  Brad Midgley <bmidgley@xmission.com>
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef __SBC_H
#define __SBC_H

//#include <stdint.h>
#include "types.h"

#define SBC_SYNCWORD	0x9C

/* sampling frequency */
#define SBC_FREQ_16000		0x00
#define SBC_FREQ_32000		0x01
#define SBC_FREQ_44100		0x02
#define SBC_FREQ_48000		0x03

/* blocks */
#define SBC_BLK_4		0x00
#define SBC_BLK_8		0x01
#define SBC_BLK_12		0x02
#define SBC_BLK_16		0x03

/* channel mode */
#define SBC_MODE_MONO		0x00
#define SBC_MODE_DUAL_CHANNEL	0x01
#define SBC_MODE_STEREO		0x02
#define SBC_MODE_JOINT_STEREO	0x03

/* allocation method */
#define SBC_AM_LOUDNESS		0x00
#define SBC_AM_SNR		0x01

/* subbands */
#define SBC_SB_4		0x00
#define SBC_SB_8		0x01

/* Data endianess */
#define SBC_LE			0x00
#define SBC_BE			0x01

struct sbc_struct {
	unsigned long flags;

	uint8_t frequency;
	uint8_t blocks;
	uint8_t subbands;
	uint8_t mode;
	uint8_t allocation;
	uint8_t bitpool;
	uint8_t endian;

	void *priv;
	void *priv_alloc_base;
};

typedef struct sbc_struct sbc_t;

/* This structure contains an unpacked SBC frame.
   Yes, there is probably quite some unused space herein */
struct sbc_frame {
	uint8_t frequency;
	uint8_t block_mode;
	uint8_t blocks;
	enum {
		MONO		= SBC_MODE_MONO,
		DUAL_CHANNEL	= SBC_MODE_DUAL_CHANNEL,
		STEREO		= SBC_MODE_STEREO,
		JOINT_STEREO	= SBC_MODE_JOINT_STEREO
	} mode;
	uint8_t channels;
	enum {
		LOUDNESS	= SBC_AM_LOUDNESS,
		SNR		= SBC_AM_SNR
	} allocation;
	uint8_t subband_mode;
	uint8_t subbands;
	uint8_t bitpool;
	uint16_t codesize;
	uint8_t length;

	/* bit number x set means joint stereo has been used in subband x */
	uint8_t joint;

	/* only the lower 4 bits of every element are to be used */
	uint32_t /*SBC_ALIGNED*/ scale_factor[2][8];

	/* raw integer subband samples in the frame */
	int32_t /*SBC_ALIGNED*/ sb_sample_f[16][2][8];

	/* modified subband samples */
	int32_t /*SBC_ALIGNED*/ sb_sample[16][2][8];

	/* original pcm audio samples */
	int16_t /*SBC_ALIGNED*/ pcm_sample[2][16*8];
};

struct sbc_decoder_state {
	int subbands;
	int32_t V[2][170];
	int offset[2][16];
};

struct sbc_priv {
	int init;
	struct /*SBC_ALIGNED*/ sbc_frame frame;
	struct /*SBC_ALIGNED*/ sbc_decoder_state dec_state;
	//struct SBC_ALIGNED sbc_encoder_state enc_state;
};

int sbc_init(sbc_t *sbc, unsigned long flags);
//int sbc_reinit(sbc_t *sbc, unsigned long flags);

ssize_t sbc_parse(sbc_t *sbc, const void *input, size_t input_len);

/* Decodes ONE input block into ONE output block */
ssize_t sbc_decode(sbc_t *sbc, const void *input, size_t input_len,
			void *output, size_t output_len, size_t *written);

double sbc_get_bit_rate(sbc_t *sbc);
int calc_frame_len(sbc_t *sbc);

/* Returns the input block size in bytes */
size_t sbc_get_codesize(sbc_t *sbc);

//const char *sbc_get_implementation_info(sbc_t *sbc);
void sbc_finish(sbc_t *sbc);

#endif /* __SBC_H */
