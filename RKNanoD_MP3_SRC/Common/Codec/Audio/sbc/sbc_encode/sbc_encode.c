/*
 *
 *  Bluetooth low-complexity, subband codec (SBC) library
 *
 *  Copyright (C) 2004-2008  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2004-2005  Henryk Ploetz <henryk@ploetzli.ch>
 *  Copyright (C) 2005-2008  Brad Midgley <bmidgley@xmission.com>
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

/* todo items:

  use a log2 table for byte integer scale factors calculation (sum log2 results
  for high and low bytes) fill bitpool by 16 bits instead of one at a time in
  bits allocation/bitpool generation port to the dsp

*/


#include <string.h>
#include <stdlib.h>
#include "typedef.h"


#include "sbc_enc_tables.h"

#include "sbc_enc.h"

#ifdef _SBC_ENCODE_

#define SBC_SYNCWORD    0x9C

#define __LITTLE_ENDIAN__

/* This structure contains an unpacked SBC frame.
   Yes, there is probably quite some unused space herein */
/*

enum ech_mode{
        MONO        = SBC_CM_MONO,
        DUAL_CHANNEL    = SBC_CM_DUAL_CHANNEL,
        STEREO      = SBC_CM_STEREO,
        JOINT_STEREO    = SBC_CM_JOINT_STEREO
    } ;

enum en_allocation_method{
        LOUDNESS    = SBC_AM_LOUDNESS,
        SNR     = SBC_AM_SNR
    } ;
*/
struct sbc_frame {
    uint8 frequency;
    uint8 block_mode;
    uint8 blocks;

    enum  ech_mode {
        MONO        = SBC_MODE_MONO,
        DUAL_CHANNEL    = SBC_MODE_DUAL_CHANNEL,
        STEREO      = SBC_MODE_STEREO,
        JOINT_STEREO    = SBC_MODE_JOINT_STEREO
    } mode;

//  enum ech_mode  mode;
    uint8 channels;

    enum  en_allocation_method{
        LOUDNESS    = SBC_AM_LOUDNESS,
        SNR     = SBC_AM_SNR
    } allocation;

    // enum en_allocation_method  allocation;
    uint8 subband_mode;
    uint8 subbands;
    uint8 bitpool;
    uint8 codesize;
    uint8 length;

    /* bit number x set means joint stereo has been used in subband x */
    uint8 joint;

    /* only the lower 4 bits of every element are to be used */
    uint8 scale_factor[2][8];

    /* raw integer subband samples in the frame */

    int32 sb_sample_f[16][2][8];
    int32 sb_sample[16][2][8];  /* modified subband samples */
    int16 pcm_sample[2][16*8];  /* original pcm audio samples */
};
#if 0
struct sbc_decoder_state {
    int subbands;
    int32 V[2][170];
    int offset[2][16];
};
#endif
struct sbc_encoder_state {
    int subbands;
    int position[2];
    int32 X[2][160];
};

struct sbc_priv {
    int init;
    struct sbc_frame frame;
    //struct sbc_decoder_state dec_state;
    struct sbc_encoder_state enc_state;
};


#ifndef __RUN_ON_OS__
_ATTR_AUDIO_SBC_ENCODE_TEXT_ static sbc_t         sbcdecexc;
_ATTR_AUDIO_SBC_ENCODE_TEXT_ static struct sbc_priv sbcpriv;
#endif

/*
 * Calculates the CRC-8 of the first len bits in data
 */
_ATTR_AUDIO_SBC_ENCODE_TEXT_
static  uint8 crc_table[256] = {
    0x00, 0x1D, 0x3A, 0x27, 0x74, 0x69, 0x4E, 0x53,
    0xE8, 0xF5, 0xD2, 0xCF, 0x9C, 0x81, 0xA6, 0xBB,
    0xCD, 0xD0, 0xF7, 0xEA, 0xB9, 0xA4, 0x83, 0x9E,
    0x25, 0x38, 0x1F, 0x02, 0x51, 0x4C, 0x6B, 0x76,
    0x87, 0x9A, 0xBD, 0xA0, 0xF3, 0xEE, 0xC9, 0xD4,
    0x6F, 0x72, 0x55, 0x48, 0x1B, 0x06, 0x21, 0x3C,
    0x4A, 0x57, 0x70, 0x6D, 0x3E, 0x23, 0x04, 0x19,
    0xA2, 0xBF, 0x98, 0x85, 0xD6, 0xCB, 0xEC, 0xF1,
    0x13, 0x0E, 0x29, 0x34, 0x67, 0x7A, 0x5D, 0x40,
    0xFB, 0xE6, 0xC1, 0xDC, 0x8F, 0x92, 0xB5, 0xA8,
    0xDE, 0xC3, 0xE4, 0xF9, 0xAA, 0xB7, 0x90, 0x8D,
    0x36, 0x2B, 0x0C, 0x11, 0x42, 0x5F, 0x78, 0x65,
    0x94, 0x89, 0xAE, 0xB3, 0xE0, 0xFD, 0xDA, 0xC7,
    0x7C, 0x61, 0x46, 0x5B, 0x08, 0x15, 0x32, 0x2F,
    0x59, 0x44, 0x63, 0x7E, 0x2D, 0x30, 0x17, 0x0A,
    0xB1, 0xAC, 0x8B, 0x96, 0xC5, 0xD8, 0xFF, 0xE2,
    0x26, 0x3B, 0x1C, 0x01, 0x52, 0x4F, 0x68, 0x75,
    0xCE, 0xD3, 0xF4, 0xE9, 0xBA, 0xA7, 0x80, 0x9D,
    0xEB, 0xF6, 0xD1, 0xCC, 0x9F, 0x82, 0xA5, 0xB8,
    0x03, 0x1E, 0x39, 0x24, 0x77, 0x6A, 0x4D, 0x50,
    0xA1, 0xBC, 0x9B, 0x86, 0xD5, 0xC8, 0xEF, 0xF2,
    0x49, 0x54, 0x73, 0x6E, 0x3D, 0x20, 0x07, 0x1A,
    0x6C, 0x71, 0x56, 0x4B, 0x18, 0x05, 0x22, 0x3F,
    0x84, 0x99, 0xBE, 0xA3, 0xF0, 0xED, 0xCA, 0xD7,
    0x35, 0x28, 0x0F, 0x12, 0x41, 0x5C, 0x7B, 0x66,
    0xDD, 0xC0, 0xE7, 0xFA, 0xA9, 0xB4, 0x93, 0x8E,
    0xF8, 0xE5, 0xC2, 0xDF, 0x8C, 0x91, 0xB6, 0xAB,
    0x10, 0x0D, 0x2A, 0x37, 0x64, 0x79, 0x5E, 0x43,
    0xB2, 0xAF, 0x88, 0x95, 0xC6, 0xDB, 0xFC, 0xE1,
    0x5A, 0x47, 0x60, 0x7D, 0x2E, 0x33, 0x14, 0x09,
    0x7F, 0x62, 0x45, 0x58, 0x0B, 0x16, 0x31, 0x2C,
    0x97, 0x8A, 0xAD, 0xB0, 0xE3, 0xFE, 0xD9, 0xC4
};

_ATTR_AUDIO_SBC_ENCODE_TEXT_
static uint8 sbc_crc_bit8(const uint8 *data, size_t len)
{
    uint8 crc = 0x0f;
    size_t i;
    uint8 octet;

    for (i = 0; i < len / 8; i++)
        crc = crc_table[crc ^ data[i]];

    octet = data[i];
    for (i = 0; i < len % 8; i++) {
        char bit = ((octet ^ crc) & 0x80) >> 7;

        crc = ((crc & 0x7f) << 1) ^ (bit ? 0x1d : 0);

        octet = octet << 1;
    }

    return crc;
}

/*
 * Code straight from the spec to calculate the bits array
 * Takes a pointer to the frame in question, a pointer to the bits array and
 * the sampling frequency (as 2 bit integer)
 */
 _ATTR_AUDIO_SBC_ENCODE_TEXT_
static void sbc_get_frame_bits(const struct sbc_frame *frame, int (*bits)[8])
{
    uint8 sf = frame->frequency;

    if (frame->mode == SBC_MODE_MONO || frame->mode == SBC_MODE_DUAL_CHANNEL) {
        int bitneed[2][8], loudness, max_bitneed, bitcount, slicecount, bitslice;
        int ch, sb;

        for (ch = 0; ch < frame->channels; ch++) {
            max_bitneed = 0;
            if (frame->allocation == SBC_AM_SNR) {
                for (sb = 0; sb < frame->subbands; sb++) {
                    bitneed[ch][sb] = frame->scale_factor[ch][sb];
                    if (bitneed[ch][sb] > max_bitneed)
                        max_bitneed = bitneed[ch][sb];
                }
            } else {
                for (sb = 0; sb < frame->subbands; sb++) {
                    if (frame->scale_factor[ch][sb] == 0)
                        bitneed[ch][sb] = -5;
                    else {
                        if (frame->subbands == 4)
                            loudness = frame->scale_factor[ch][sb] - sbc_offset4[sf][sb];
                        else
                            loudness = frame->scale_factor[ch][sb] - sbc_offset8[sf][sb];
                        if (loudness > 0)
                            bitneed[ch][sb] = loudness / 2;
                        else
                            bitneed[ch][sb] = loudness;
                    }
                    if (bitneed[ch][sb] > max_bitneed)
                        max_bitneed = bitneed[ch][sb];
                }
            }

            bitcount = 0;
            slicecount = 0;
            bitslice = max_bitneed + 1;
            do {
                bitslice--;
                bitcount += slicecount;
                slicecount = 0;
                for (sb = 0; sb < frame->subbands; sb++) {
                    if ((bitneed[ch][sb] > bitslice + 1) && (bitneed[ch][sb] < bitslice + 16))
                        slicecount++;
                    else if (bitneed[ch][sb] == bitslice + 1)
                        slicecount += 2;
                }
            } while (bitcount + slicecount < frame->bitpool);

            if (bitcount + slicecount == frame->bitpool) {
                bitcount += slicecount;
                bitslice--;
            }

            for (sb = 0; sb < frame->subbands; sb++) {
                if (bitneed[ch][sb] < bitslice + 2)
                    bits[ch][sb] = 0;
                else {
                    bits[ch][sb] = bitneed[ch][sb] - bitslice;
                    if (bits[ch][sb] > 16)
                        bits[ch][sb] = 16;
                }
            }

            for (sb = 0; bitcount < frame->bitpool && sb < frame->subbands; sb++) {
                if ((bits[ch][sb] >= 2) && (bits[ch][sb] < 16)) {
                    bits[ch][sb]++;
                    bitcount++;
                } else if ((bitneed[ch][sb] == bitslice + 1) && (frame->bitpool > bitcount + 1)) {
                    bits[ch][sb] = 2;
                    bitcount += 2;
                }
            }

            for (sb = 0; bitcount < frame->bitpool && sb < frame->subbands; sb++) {
                if (bits[ch][sb] < 16) {
                    bits[ch][sb]++;
                    bitcount++;
                }
            }

        }

    } else if (frame->mode == SBC_MODE_STEREO || frame->mode == SBC_MODE_JOINT_STEREO) {
        int bitneed[2][8], loudness, max_bitneed, bitcount, slicecount, bitslice;
        int ch, sb;

        max_bitneed = 0;
        if (frame->allocation == SBC_AM_SNR) {
            for (ch = 0; ch < 2; ch++) {
                for (sb = 0; sb < frame->subbands; sb++) {
                    bitneed[ch][sb] = frame->scale_factor[ch][sb];
                    if (bitneed[ch][sb] > max_bitneed)
                        max_bitneed = bitneed[ch][sb];
                }
            }
        } else {
            for (ch = 0; ch < 2; ch++) {
                for (sb = 0; sb < frame->subbands; sb++) {
                    if (frame->scale_factor[ch][sb] == 0)
                        bitneed[ch][sb] = -5;
                    else {
                        if (frame->subbands == 4)
                            loudness = frame->scale_factor[ch][sb] - sbc_offset4[sf][sb];
                        else
                            loudness = frame->scale_factor[ch][sb] - sbc_offset8[sf][sb];
                        if (loudness > 0)
                            bitneed[ch][sb] = loudness / 2;
                        else
                            bitneed[ch][sb] = loudness;
                    }
                    if (bitneed[ch][sb] > max_bitneed)
                        max_bitneed = bitneed[ch][sb];
                }
            }
        }

        bitcount = 0;
        slicecount = 0;
        bitslice = max_bitneed + 1;
        do {
            bitslice--;
            bitcount += slicecount;
            slicecount = 0;
            for (ch = 0; ch < 2; ch++) {
                for (sb = 0; sb < frame->subbands; sb++) {
                    if ((bitneed[ch][sb] > bitslice + 1) && (bitneed[ch][sb] < bitslice + 16))
                        slicecount++;
                    else if (bitneed[ch][sb] == bitslice + 1)
                        slicecount += 2;
                }
            }
        } while (bitcount + slicecount < frame->bitpool);

        if (bitcount + slicecount == frame->bitpool) {
            bitcount += slicecount;
            bitslice--;
        }

        for (ch = 0; ch < 2; ch++) {
            for (sb = 0; sb < frame->subbands; sb++) {
                if (bitneed[ch][sb] < bitslice + 2) {
                    bits[ch][sb] = 0;
                } else {
                    bits[ch][sb] = bitneed[ch][sb] - bitslice;
                    if (bits[ch][sb] > 16)
                        bits[ch][sb] = 16;
                }
            }
        }

        ch = 0;
        sb = 0;
        while (bitcount < frame->bitpool) {
            if ((bits[ch][sb] >= 2) && (bits[ch][sb] < 16)) {
                bits[ch][sb]++;
                bitcount++;
            } else if ((bitneed[ch][sb] == bitslice + 1) && (frame->bitpool > bitcount + 1)) {
                bits[ch][sb] = 2;
                bitcount += 2;
            }
            if (ch == 1) {
                ch = 0;
                sb++;
                if (sb >= frame->subbands) break;
            } else
                ch = 1;
        }

        ch = 0;
        sb = 0;
        while (bitcount < frame->bitpool) {
            if (bits[ch][sb] < 16) {
                bits[ch][sb]++;
                bitcount++;
            }
            if (ch == 1) {
                ch = 0;
                sb++;
                if (sb >= frame->subbands) break;
            } else
                ch = 1;
        }

    }

}

_ATTR_AUDIO_SBC_ENCODE_TEXT_
static void sbc_encode_init(struct sbc_encoder_state *state,
                const struct sbc_frame *frame)
{
    memset(&state->X, 0, sizeof(state->X));
    state->subbands = frame->subbands;
    state->position[0] = state->position[1] = 9 * frame->subbands;
}

/*
_sbc_analyze_four
*/
_ATTR_AUDIO_SBC_ENCODE_TEXT_
static  void _sbc_process_4bytes(const uint32 *in, uint32 *out)
{
    sbc_fixed_t t[8], s[5];

    t[0] = SCALE4_STAGE1( /* Q8 */
        MULA(_sbc_proto_4[0], in[8] - in[32], /* Q18 */
        MUL( _sbc_proto_4[1], in[16] - in[24])));

    t[1] = SCALE4_STAGE1(
        MULA(_sbc_proto_4[2], in[1],
        MULA(_sbc_proto_4[3], in[9],
        MULA(_sbc_proto_4[4], in[17],
        MULA(_sbc_proto_4[5], in[25],
        MUL( _sbc_proto_4[6], in[33]))))));

    t[2] = SCALE4_STAGE1(
        MULA(_sbc_proto_4[7], in[2],
        MULA(_sbc_proto_4[8], in[10],
        MULA(_sbc_proto_4[9], in[18],
        MULA(_sbc_proto_4[10], in[26],
        MUL( _sbc_proto_4[11], in[34]))))));

    t[3] = SCALE4_STAGE1(
        MULA(_sbc_proto_4[12], in[3],
        MULA(_sbc_proto_4[13], in[11],
        MULA(_sbc_proto_4[14], in[19],
        MULA(_sbc_proto_4[15], in[27],
        MUL( _sbc_proto_4[16], in[35]))))));

    t[4] = SCALE4_STAGE1(
        MULA(_sbc_proto_4[17], in[4] + in[36],
        MULA(_sbc_proto_4[18], in[12] + in[28],
        MUL( _sbc_proto_4[19], in[20]))));

    t[5] = SCALE4_STAGE1(
        MULA(_sbc_proto_4[16], in[5],
        MULA(_sbc_proto_4[15], in[13],
        MULA(_sbc_proto_4[14], in[21],
        MULA(_sbc_proto_4[13], in[29],
        MUL( _sbc_proto_4[12], in[37]))))));

    /* don't compute t[6]... this term always multiplies
     * with cos(pi/2) = 0 */

    t[7] = SCALE4_STAGE1(
        MULA(_sbc_proto_4[6], in[7],
        MULA(_sbc_proto_4[5], in[15],
        MULA(_sbc_proto_4[4], in[23],
        MULA(_sbc_proto_4[3], in[31],
        MUL( _sbc_proto_4[2], in[39]))))));

    s[0] = MUL( _anamatrix4[0], t[0] + t[4]);
    s[1] = MUL( _anamatrix4[2], t[2]);
    s[2] = MULA(_anamatrix4[1], t[1] + t[3],
        MUL(_anamatrix4[3], t[5]));
    s[3] = MULA(_anamatrix4[3], t[1] + t[3],
        MUL(_anamatrix4[1], -t[5] + t[7]));
    s[4] = MUL( _anamatrix4[3], t[7]);

    out[0] = SCALE4_STAGE2( s[0] + s[1] + s[2] - s[4]); /* Q0 */
    out[1] = SCALE4_STAGE2(-s[0] + s[1] + s[3]);
    out[2] = SCALE4_STAGE2(-s[0] + s[1] - s[3]);
    out[3] = SCALE4_STAGE2( s[0] + s[1] - s[2] + s[4]);
}

/*
sbc_analyze_four
*/
_ATTR_AUDIO_SBC_ENCODE_TEXT_
static  void sbc_process_4bytes(struct sbc_encoder_state *state,
                struct sbc_frame *frame, int ch, int blk)
{
    int32 *x = &state->X[ch][state->position[ch]];
    int16 *pcm = &frame->pcm_sample[ch][blk * 4];

    /* Input 4 Audio Samples */
    x[40] = x[0] = pcm[3];
    x[41] = x[1] = pcm[2];
    x[42] = x[2] = pcm[1];
    x[43] = x[3] = pcm[0];

    _sbc_process_4bytes(x, frame->sb_sample_f[blk][ch]);

    state->position[ch] -= 4;
    if (state->position[ch] < 0)
        state->position[ch] = 36;
}

_ATTR_AUDIO_SBC_ENCODE_TEXT_
static  void _sbc_process_8bytes(const uint32 *in, uint32 *out)
{
    sbc_fixed_t t[8], s[10];

    t[2] = SCALE8_STAGE1(
        MULA(_sbc_proto_8[6], in[2],
        MULA(_sbc_proto_8[7], in[18],
        MULA(_sbc_proto_8[8], in[34],
        MULA(_sbc_proto_8[9], in[50],
        MULA(_sbc_proto_8[10], in[66],
        MULA(_sbc_proto_8[26], in[6],
        MULA(_sbc_proto_8[27], in[22],
        MULA(_sbc_proto_8[28], in[38],
        MULA(_sbc_proto_8[29], in[54],
        MUL( _sbc_proto_8[30], in[70])))))))))));

    t[4] = SCALE8_STAGE1(
        MULA(_sbc_proto_8[2], in[4],
        MULA(_sbc_proto_8[3], in[20],
        MULA(_sbc_proto_8[4], in[36],
        MULA(_sbc_proto_8[5], in[52],
        MUL(_sbc_proto_8[39], in[68]))))));

    t[3] = SCALE8_STAGE1(
        MULA(_sbc_proto_8[16], in[3],
        MULA(_sbc_proto_8[17], in[19],
        MULA(_sbc_proto_8[18], in[35],
        MULA(_sbc_proto_8[19], in[51],
        MULA(_sbc_proto_8[20], in[67],
        MULA(_sbc_proto_8[21], in[5],
        MULA(_sbc_proto_8[22], in[21],
        MULA(_sbc_proto_8[23], in[37],
        MULA(_sbc_proto_8[24], in[53],
        MUL( _sbc_proto_8[25], in[69])))))))))));

    t[1] = SCALE8_STAGE1(
        MULA( _sbc_proto_8[11], in[1],
        MULA( _sbc_proto_8[12], in[17],
        MULA( _sbc_proto_8[13], in[33],
        MULA( _sbc_proto_8[14], in[49],
        MULA( _sbc_proto_8[15], in[65],
        MULA( _sbc_proto_8[31], in[7],
        MULA( _sbc_proto_8[32], in[23],
        MULA( _sbc_proto_8[33], in[39],
        MULA( _sbc_proto_8[34], in[55],
        MUL(  _sbc_proto_8[35], in[71])))))))))));

    t[0] = SCALE8_STAGE1(
        MULA( _sbc_proto_8[0], (in[16] - in[64]),
        MULA( _sbc_proto_8[1], (in[32] - in[48]),
        MULA( _sbc_proto_8[36],(in[8] + in[72]),
        MULA( _sbc_proto_8[37],(in[24] + in[56]),
        MUL(  _sbc_proto_8[38], in[40]))))));

    t[5] = SCALE8_STAGE1(
        MULA( _sbc_proto_8[35], in[9],
        MULA( _sbc_proto_8[34], in[25],
        MULA( _sbc_proto_8[33], in[41],
        MULA( _sbc_proto_8[32], in[57],
        MULA( _sbc_proto_8[31], in[73],
        MULA(-_sbc_proto_8[15], in[15],
        MULA(-_sbc_proto_8[14], in[31],
        MULA(-_sbc_proto_8[13], in[47],
        MULA(-_sbc_proto_8[12], in[63],
        MUL( -_sbc_proto_8[11], in[79])))))))))));

    t[6] = SCALE8_STAGE1(
        MULA( _sbc_proto_8[30], in[10],
        MULA( _sbc_proto_8[29], in[26],
        MULA( _sbc_proto_8[28], in[42],
        MULA( _sbc_proto_8[27], in[58],
        MULA( _sbc_proto_8[26], in[74],
        MULA(-_sbc_proto_8[10], in[14],
        MULA(-_sbc_proto_8[9], in[30],
        MULA(-_sbc_proto_8[8], in[46],
        MULA(-_sbc_proto_8[7], in[62],
        MUL( -_sbc_proto_8[6], in[78])))))))))));

    t[7] = SCALE8_STAGE1(
        MULA( _sbc_proto_8[25], in[11],
        MULA( _sbc_proto_8[24], in[27],
        MULA( _sbc_proto_8[23], in[43],
        MULA( _sbc_proto_8[22], in[59],
        MULA( _sbc_proto_8[21], in[75],
        MULA(-_sbc_proto_8[20], in[13],
        MULA(-_sbc_proto_8[19], in[29],
        MULA(-_sbc_proto_8[18], in[45],
        MULA(-_sbc_proto_8[17], in[61],
        MUL( -_sbc_proto_8[16], in[77])))))))))));


        s[0] = MUL(_anamatrix8[7],t[4]);

        s[1] = MUL(_anamatrix8[6],t[0]);

        s[2] = MUL(_anamatrix8[0],t[2]);  // 0 2

        s[3] = MUL(_anamatrix8[1],t[6]);  // 1 6

        s[8] = MUL(_anamatrix8[1],t[2]); //  1 2

        s[9] = MUL(_anamatrix8[0],t[6]); // 0 6

        s[4] = MULA( _anamatrix8[2], t[3],
                    MULA( _anamatrix8[3], t[1],
                    MULA( _anamatrix8[4], t[5],
                    MUL(  _anamatrix8[5], t[7]))));

        s[5] = MULA(-_anamatrix8[2], t[5],
                    MULA( _anamatrix8[3], t[3],
                    MULA(-_anamatrix8[4], t[7],
                    MUL( -_anamatrix8[5], t[1]))));

        s[6] = MULA( _anamatrix8[4], t[3],
                    MULA( -_anamatrix8[2], t[1],
                    MULA( _anamatrix8[5], t[5],
                    MUL(  _anamatrix8[3], t[7]))));

        s[7] = MULA(-_anamatrix8[2], t[7],
                    MULA( _anamatrix8[3], t[5],
                    MULA(-_anamatrix8[4], t[1],
                    MUL(  _anamatrix8[5], t[3]))));
        //uint32 cut[8];
        out[0] = SCALE8_STAGE2( (s[0] + s[1]) + (s[2] + s[3]) + s[4]);

        out[1] = SCALE8_STAGE2( (s[0] - s[1]) + (s[8] - s[9]) + s[5]);

        out[2] = SCALE8_STAGE2( (s[0] - s[1]) - s[8] + (s[9] + s[6]));

        out[3] = SCALE8_STAGE2( (s[0] + s[1]) - (s[2] + s[3]) + s[7]);

        out[4] = SCALE8_STAGE2( (s[0] + s[1]) - (s[2] + s[3]) - s[7]);

        out[5] = SCALE8_STAGE2( (s[0] - s[1]) - s[8] + (s[9] - s[6]));

        out[6] = SCALE8_STAGE2( (s[0] - s[1]) + (s[8] - s[9]) - s[5]);

        out[7] = SCALE8_STAGE2( (s[0] + s[1]) + (s[2] + s[3]) - s[4] );
}

/*
sbc_analyze_eight
*/
_ATTR_AUDIO_SBC_ENCODE_TEXT_
static  void sbc_process_8bytes(struct sbc_encoder_state *state,
                    struct sbc_frame *frame, int ch,
                    int blk)
{
    int32 *x = &state->X[ch][state->position[ch]];
    int16 *pcm = &frame->pcm_sample[ch][blk * 8];

    /* Input 8 Audio Samples */
    x[80] = x[0] = pcm[7];
    x[81] = x[1] = pcm[6];
    x[82] = x[2] = pcm[5];
    x[83] = x[3] = pcm[4];
    x[84] = x[4] = pcm[3];
    x[85] = x[5] = pcm[2];
    x[86] = x[6] = pcm[1];
    x[87] = x[7] = pcm[0];


    _sbc_process_8bytes(x, frame->sb_sample_f[blk][ch]);

    state->position[ch] -= 8;
    if (state->position[ch] < 0)
        state->position[ch] = 72;
}

/*
sbc_analyze_audio
*/
_ATTR_AUDIO_SBC_ENCODE_TEXT_
static int sbc_process_frame(struct sbc_encoder_state *state,
                struct sbc_frame *frame)
{
    int ch, blk;

    switch (frame->subbands) {
    case 4:
        for (ch = 0; ch < frame->channels; ch++)
            for (blk = 0; blk < frame->blocks; blk++)
                sbc_process_4bytes(state, frame, ch, blk);
        return frame->blocks * 4;

    case 8:
        for (ch = 0; ch < frame->channels; ch++)
            for (blk = 0; blk < frame->blocks; blk++)
                sbc_process_8bytes(state, frame, ch, blk);
        return frame->blocks * 8;

    default:
        return -1;
    }
}

/*
 * Packs the SBC frame from frame into the memory at data. At most len
 * bytes will be used, should more memory be needed an appropriate
 * error code will be returned. Returns the length of the packed frame
 * on success or a negative value on error.
 *
 * The error codes are:
 * -1 Not enough memory reserved
 * -2 Unsupported sampling rate
 * -3 Unsupported number of blocks
 * -4 Unsupported number of subbands
 * -5 Bitpool value out of bounds
 * -99 not implemented
 */


_ATTR_AUDIO_SBC_ENCODE_TEXT_
static int sbc_pack_frame_into_buff(uint8 *data, struct sbc_frame *frame, size_t len)
{
    int produced;
    /* Will copy the header parts for CRC-8 calculation here */
    uint8 crc_header[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    int crc_pos = 0;

    uint16 audio_sample;

    int ch, sb, blk, bit;   /* channel, subband, block and bit counters */
    int bits[2][8];     /* bits distribution */
    int levels[2][8];   /* levels are derived from that */

    uint32 scalefactor[2][8];   /* derived from frame->scale_factor */

    data[0] = SBC_SYNCWORD;

    data[1] = (frame->frequency & 0x03) << 6;

    data[1] |= (frame->block_mode & 0x03) << 4;

    data[1] |= (frame->mode & 0x03) << 2;

    data[1] |= (frame->allocation & 0x01) << 1;

    switch (frame->subbands) {
    case 4:
        /* Nothing to do */
        break;
    case 8:
        data[1] |= 0x01;
        break;
    default:
        return -4;
        break;
    }

    data[2] = frame->bitpool;

    if ((frame->mode == SBC_MODE_MONO || frame->mode == SBC_MODE_DUAL_CHANNEL) &&
            frame->bitpool > frame->subbands << 4)
        return -5;

    if ((frame->mode == SBC_MODE_STEREO || frame->mode == SBC_MODE_JOINT_STEREO) &&
            frame->bitpool > frame->subbands << 5)
        return -5;

    /* Can't fill in crc yet */

    produced = 32;

    crc_header[0] = data[1];
    crc_header[1] = data[2];
    crc_pos = 16;

    for (ch = 0; ch < frame->channels; ch++) {
        for (sb = 0; sb < frame->subbands; sb++) {
            frame->scale_factor[ch][sb] = 0;
            scalefactor[ch][sb] = 2;
            for (blk = 0; blk < frame->blocks; blk++) {
                while (scalefactor[ch][sb] < fabs(frame->sb_sample_f[blk][ch][sb])) {
                    frame->scale_factor[ch][sb]++;
                    scalefactor[ch][sb] *= 2;
                }
            }
        }
    }

    if (frame->mode == SBC_MODE_JOINT_STEREO) {
        /* like frame->sb_sample but joint stereo */
        int32 sb_sample_j[16][2];
        /* scalefactor and scale_factor in joint case */
        uint32 scalefactor_j[2];
        uint8 scale_factor_j[2];

        frame->joint = 0;

        for (sb = 0; sb < frame->subbands - 1; sb++) {
            scale_factor_j[0] = 0;
            scalefactor_j[0] = 2;
            scale_factor_j[1] = 0;
            scalefactor_j[1] = 2;

            for (blk = 0; blk < frame->blocks; blk++) {
                /* Calculate joint stereo signal */
                sb_sample_j[blk][0] =
                    (frame->sb_sample_f[blk][0][sb] +
                        frame->sb_sample_f[blk][1][sb]) >> 1;
                sb_sample_j[blk][1] =
                    (frame->sb_sample_f[blk][0][sb] -
                        frame->sb_sample_f[blk][1][sb]) >> 1;

                /* calculate scale_factor_j and scalefactor_j for joint case */
                while (scalefactor_j[0] < fabs(sb_sample_j[blk][0])) {
                    scale_factor_j[0]++;
                    scalefactor_j[0] *= 2;
                }
                while (scalefactor_j[1] < fabs(sb_sample_j[blk][1])) {
                    scale_factor_j[1]++;
                    scalefactor_j[1] *= 2;
                }
            }

            /* decide whether to join this subband */
            if ((scalefactor[0][sb] + scalefactor[1][sb]) >
                    (scalefactor_j[0] + scalefactor_j[1]) ) {
                /* use joint stereo for this subband */
                frame->joint |= 1 << sb;
                frame->scale_factor[0][sb] = scale_factor_j[0];
                frame->scale_factor[1][sb] = scale_factor_j[1];
                scalefactor[0][sb] = scalefactor_j[0];
                scalefactor[1][sb] = scalefactor_j[1];
                for (blk = 0; blk < frame->blocks; blk++) {
                    frame->sb_sample_f[blk][0][sb] =
                            sb_sample_j[blk][0];
                    frame->sb_sample_f[blk][1][sb] =
                            sb_sample_j[blk][1];
                }
            }
        }

        data[4] = 0;
        for (sb = 0; sb < frame->subbands - 1; sb++)
            data[4] |= ((frame->joint >> sb) & 0x01) << (frame->subbands - 1 - sb);

        crc_header[crc_pos >> 3] = data[4];

        produced += frame->subbands;
        crc_pos += frame->subbands;
    }

    for (ch = 0; ch < frame->channels; ch++) {
        for (sb = 0; sb < frame->subbands; sb++) {
            data[produced >> 3] <<= 4;
            crc_header[crc_pos >> 3] <<= 4;
            data[produced >> 3] |= frame->scale_factor[ch][sb] & 0x0F;
            crc_header[crc_pos >> 3] |= frame->scale_factor[ch][sb] & 0x0F;

            produced += 4;
            crc_pos += 4;
        }
    }

    /* align the last crc byte */
    if (crc_pos % 8)
        crc_header[crc_pos >> 3] <<= 8 - (crc_pos % 8);

    data[3] = sbc_crc_bit8(crc_header, crc_pos);

    sbc_get_frame_bits(frame, bits);

    for (ch = 0; ch < frame->channels; ch++) {
        for (sb = 0; sb < frame->subbands; sb++)
            levels[ch][sb] = (1 << bits[ch][sb]) - 1;

    }

    for (blk = 0; blk < frame->blocks; blk++) {

        for (ch = 0; ch < frame->channels; ch++) {

            for (sb = 0; sb < frame->subbands; sb++) {

                if (levels[ch][sb] > 0) {
                audio_sample =
                    (uint16) ((((frame->sb_sample_f[blk][ch][sb]*levels[ch][sb]) >>
                                    (frame->scale_factor[ch][sb] + 1)) +
                                levels[ch][sb]) >> 1);

                    audio_sample <<= 16 - bits[ch][sb];
                    for (bit = 0; bit < bits[ch][sb]; bit++) {
                        data[produced >> 3] <<= 1;
                        if (audio_sample & 0x8000)
                            data[produced >> 3] |= 0x1;
                        audio_sample <<= 1;
                        produced++;
                    }
                }
            }
        }
    }

    /* align the last byte */
    if (produced % 8) {
        data[produced >> 3] <<= 8 - (produced % 8);
    }

    return (produced + 7) >> 3;
}

uint8 avdtp_get_sbc_bitpool(void);


_ATTR_AUDIO_SBC_ENCODE_TEXT_
static void sbc_virables_init(sbc_t *sbc, unsigned long flags)
{
    sbc->frequency = SBC_FREQ_44100;
    sbc->mode = SBC_MODE_JOINT_STEREO;  //SBC_MODE_MONO SBC_MODE_STEREO
    sbc->subbands = SBC_SB_8;    //8
    sbc->blocks = SBC_BLK_16;
    //sbc->bitpool = 53;           // 53
    sbc->bitpool = sbc_get_sbc_bitpool_max();
#ifdef __LITTLE_ENDIAN__
    sbc->endian = SBC_LE;
#else
    sbc->endian = SBC_BE;
#endif
}

//void *  sbc_init(UINT32 nSampleRate, UINT32 nBitRate, UINT32 nChannels)

_ATTR_AUDIO_SBC_ENCODE_TEXT_
void *  sbc_init()
{
    sbc_t *sbc;
    unsigned long flags=0;

    sbc = &sbcdecexc ;
    memset(sbc, 0, sizeof(sbc_t));

#if __RUN_ON_OS__
    sbc->priv = malloc(sizeof(struct sbc_priv));
    if (!sbc->priv)
        return -ENOMEM;
#else
    sbc->priv = &sbcpriv;
#endif
    memset(sbc->priv, 0, sizeof(struct sbc_priv));

    sbc_virables_init(sbc, flags);

    return sbc;
}


_ATTR_AUDIO_SBC_ENCODE_TEXT_
int sbc_encode_frame(void * hEnc, UINT32* pnFrameSize, UINT8 * pFrame, UINT32 nRawLen, short *pRawData)
{

    struct sbc_priv *priv;
    char *ptr;
    int i, ch, framelen, samples;
    sbc_t *sbc;
    int output_len;
    output_len = 500 ;
    sbc = (sbc_t *)hEnc ;
    if (!sbc && !pRawData)
        return -1;

    priv = (struct sbc_priv *)sbc->priv;

    if (pnFrameSize)
        *pnFrameSize = 0;

    if (!priv->init) {
        priv->frame.frequency = sbc->frequency;
        priv->frame.mode = /*(enum  sbc_frame::ech_mode)*/sbc->mode;
        priv->frame.channels = sbc->mode == SBC_MODE_MONO ? 1 : 2;
        priv->frame.allocation = /*(enum  sbc_frame::en_allocation_method)*/sbc->allocation;
        priv->frame.subband_mode = sbc->subbands;
        priv->frame.subbands = sbc->subbands ? 8 : 4;
        priv->frame.block_mode = sbc->blocks;
        priv->frame.blocks = 4 + (sbc->blocks * 4);
        priv->frame.bitpool = sbc->bitpool;
        priv->frame.codesize = sbc_get_encode_size(sbc);
        priv->frame.length = sbc_get_frameLen(sbc);
        sbc_encode_init(&priv->enc_state, &priv->frame);
        priv->init = 1;
    }

    /* input must be large enough to encode a complete frame */
    if (nRawLen < priv->frame.codesize)
        return 0;

    /* output must be large enough to receive the encoded frame */
    if (!pFrame || output_len < priv->frame.length)
        return -1;

    ptr = (char *)pRawData;

    for (i = 0; i < priv->frame.subbands * priv->frame.blocks; i++) {
        for (ch = 0; ch < priv->frame.channels; ch++) {
            int16 s;

#ifdef  __LITTLE_ENDIAN__
            s = (ptr[0] & 0xff) | (ptr[1] & 0xff) << 8;
#else
            s = (ptr[0] & 0xff) << 8 | (ptr[1] & 0xff);
#endif

            ptr += 2;
            priv->frame.pcm_sample[ch][i] = s;
        }
    }

    samples = sbc_process_frame(&priv->enc_state, &priv->frame);

    framelen = sbc_pack_frame_into_buff((unsigned char *)pFrame, &priv->frame, output_len);

    if (pnFrameSize)
        *pnFrameSize = framelen;
    return samples * priv->frame.channels * 2;
}


_ATTR_AUDIO_SBC_ENCODE_TEXT_
int sbc_get_frameLen(sbc_t *sbc)
{
    int ret;
    uint8 subbands, channels, blocks, joint;
    struct sbc_priv *priv;

    priv = (struct sbc_priv *)sbc->priv;
    if (!priv->init) {
        subbands = sbc->subbands ? 8 : 4;
        blocks = 4 + (sbc->blocks * 4);
        channels = sbc->mode == SBC_MODE_MONO ? 1 : 2;
        joint = sbc->mode == SBC_MODE_JOINT_STEREO ? 1 : 0;
    } else {
        subbands = priv->frame.subbands;
        blocks = priv->frame.blocks;
        channels = priv->frame.channels;
        joint = priv->frame.joint;
    }

    ret = 4 + (4 * subbands * channels) / 8;

    /* This term is not always evenly divide so we round it up */
    if (channels == 1)
        ret += ((blocks * channels * sbc->bitpool) + 7) / 8;
    else
        ret += (((joint ? subbands : 0) + blocks * sbc->bitpool) + 7)
            / 8;

    return ret;
}


_ATTR_AUDIO_SBC_ENCODE_TEXT_
int sbc_get_encode_size(sbc_t *sbc)
{
    uint8 subbands, channels, blocks;
    struct sbc_priv *priv;

    priv = (struct sbc_priv *)sbc->priv;
    if (!priv->init) {
        subbands = sbc->subbands ? 8 : 4;
        blocks = 4 + (sbc->blocks * 4);
        channels = sbc->mode == SBC_MODE_MONO ? 1 : 2;
    } else {
        subbands = priv->frame.subbands;
        blocks = priv->frame.blocks;
        channels = priv->frame.channels;
    }

    return subbands * blocks * channels * 2;
}

#endif
