/*
 * ALAC (Apple Lossless Audio Codec) decoder
 * Copyright (c) 2005 David Hammerton
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * ALAC (Apple Lossless Audio Codec) decoder
 * @author 2005 David Hammerton
 * @see http://crazney.net/programs/itunes/alac.html
 *
 * Note: This decoder expects a 36-byte QuickTime atom to be
 * passed through the extradata[_size] fields. This atom is tacked onto
 * the end of an 'alac' stsd atom and has the following format:
 *
 * 32bit  atom size
 * 32bit  tag                  ("alac")
 * 32bit  tag version          (0)
 * 32bit  samples per frame    (used when not set explicitly in the frames)
 *  8bit  compatible version   (0)
 *  8bit  sample size
 *  8bit  history mult         (40)
 *  8bit  initial history      (10)
 *  8bit  rice param limit     (14)
 *  8bit  channels
 * 16bit  maxRun               (255)
 * 32bit  max coded frame size (0 means unknown)
 * 32bit  average bitrate      (0 means unknown)
 * 32bit  samplerate
 */
#include "SysInclude.h"
#include "audio_main.h"

#ifdef A_CORE_DECODE
#ifdef HIFI_AlAC_DECODE
#pragma arm section code = "AlacHDecCode", rodata = "AlacHDecCode", rwdata = "AlacHDecData", zidata = "AlacHDecBss"

#include "hifi.h"
#include "Hw_hifi.h"
#include "../hifi_get_bits.h"
#include "alac.h"

unsigned int malloc_pos = 0;
char DecCode_table[1024 * 96 + 1024]; //= alac->max_samples_per_frame *4*6
unsigned  char alac_info[sizeof(ALACInfo)];
void *alac_malloc(int n)
{
    malloc_pos += n;

    if (malloc_pos >= (1024 * 96 + 1024))
    {
        Hifi_Alac_Printf("码表内存不够\n");
    }

    return &DecCode_table[malloc_pos - n];
}

unsigned int decode_scalar(GetBitContext *gb, int k, int bps)
{
    unsigned int x = get_unary_0_9(gb);

    if (x > 8)   /* RICE THRESHOLD */
    {
        /* use alternative encoding */
        x = get_bits_long(gb, bps);
    }
    else if (k != 1)
    {
        int extrabits = show_bits(gb, k);

        /* multiply x by 2^k - 1, as part of their strange algorithm */
        x = (x << k) - x;

        if (extrabits > 1)
        {
            x += extrabits - 1;
            skip_bits(gb, k);
        }
        else
            skip_bits(gb, k - 1);
    }

    return x;
}

int rice_decompress(ALACContext *alac, int32_t *output_buffer,
                    int nb_samples, int bps, int rice_history_mult)
{
    int i;
    unsigned int history = alac->rice_initial_history;
    int sign_modifier = 0;

    for (i = 0; i < nb_samples; i++)
    {
        int k;
        unsigned int x;
#if 0
        if (get_bits_left(&alac->gb) <= 0)
            return -1;

#else
        ALAC_get_more_data(&alac->gb, 32);
#endif
        /* calculate rice param and decode next value */
        k = av_log2((history >> 9) + 3);
        k = FFMIN(k, alac->rice_limit);
        x = decode_scalar(&alac->gb, k, bps);
        x += sign_modifier;
        sign_modifier = 0;
        output_buffer[i] = (x >> 1) ^ -(x & 1);

        /* update the history */
        if (x > 0xffff)
            history = 0xffff;
        else
            history +=         x * rice_history_mult -
                               ((history * rice_history_mult) >> 9);

        /* special case: there may be compressed blocks of 0 */
        if ((history < 128) && (i + 1 < nb_samples))
        {
            int block_size;

            /* calculate rice param and decode block size */
            k = 7 - av_log2(history) + ((history + 16) >> 6);
            k = FFMIN(k, alac->rice_limit);
            block_size = decode_scalar(&alac->gb, k, 16);

            if (block_size > 0)
            {
                if (block_size >= nb_samples - i)
                {

                    block_size = nb_samples - i - 1;
                }

                Hifi_Alac_MemSet(&output_buffer[i + 1], 0,
                                 block_size * sizeof(*output_buffer));
                i += block_size;
            }

            if (block_size <= 0xffff)
                sign_modifier = 1;

            history = 0;
        }
    }

    return 0;
}
/*
int sign_only(int v)
{
    return v ? FFSIGN(v) : 0;
}
*/
int alac_lpc_cnt = 0;
int alac_max_order = 0;
void lpc_prediction(int32_t *error_buffer, int32_t *buffer_out,
                    int nb_samples, int bps, int32_t *lpc_coefs,
                    int lpc_order, int lpc_quant)
{
    int i;
    int32_t *pred = buffer_out;

    /* first sample always copies第一个数据直接赋值 */
    *buffer_out = *error_buffer;

    if (nb_samples <= 1)
        return;

    if (!lpc_order)
    {
        Hifi_Alac_Memcpy(&buffer_out[1], &error_buffer[1],
                         (nb_samples - 1) * sizeof(*buffer_out));
        return;
    }

    if (lpc_order == 31)
    {
        /* simple 1st-order prediction */
        for (i = 1; i < nb_samples; i++)
        {
            buffer_out[i] = sign_extend(buffer_out[i - 1] + error_buffer[i],
                                        bps);
        }

        return;
    }


    /* read warm-up samples */
#if 0

    for (i = 1; i <= lpc_order && i < nb_samples; i++)
        buffer_out[i] = sign_extend(buffer_out[i - 1] + error_buffer[i], bps);

#else

    for (i = 1; i <= lpc_order && i < nb_samples; i++)
    {
        buffer_out[i] = sign_extend(buffer_out[i - 1] + error_buffer[i], bps);
        error_buffer[i] = buffer_out[i] ;//wjr 用error_buffer做输入、输出
    }

#endif

    /* NOTE: 4 and 8 are very common cases that could be optimized. */
#if 1

#ifndef HIFI_ACC
    Alac_Decode_Lpc( error_buffer, buffer_out,
                     nb_samples, bps, lpc_coefs, NULL,
                     lpc_order,  lpc_quant , NULL, 0);
#else

    if (lpc_quant >= 12) //lpc_quant=12时采用硬件加速会有噪声
        Alac_Decode_Lpc( error_buffer, buffer_out,
                         nb_samples, bps, lpc_coefs, NULL,
                         lpc_order,  lpc_quant , NULL, 0);
    else
    {
        Hifi_Set_ACC_XFER_Disable(0, nb_samples, HIfi_ACC_TYPE_ALAC); //开始传输配置数据和初始化系数(不往fifo送)
        Hifi_Enable_FUN_DONE_FLAG(0);
        Alac_Set_CFG(0, lpc_order, lpc_quant, bps);
        HIFITranData((UINT32*)lpc_coefs, (uint32*)ALAC_COEF_ADD, lpc_order);
        Hifi_Set_ACC_XFER_Start(0, nb_samples, HIfi_ACC_TYPE_ALAC); //可以开始往fifo传数据，并且可以取数据
        HIFI_DMA_TO_ACC(error_buffer, (uint32*)TX_FIFO, nb_samples, (uint32*)RX_FIFO, buffer_out);

        while (Hifi_Get_ACC_Intsr(0, Function_done_interrupt_active) != Function_done_interrupt_active) ;

        Hifi_Clear_FUN_DONE_FLAG(0);
    }
#endif

#else

    for (i = lpc_order + 1; i < nb_samples; i++)
    {
        int j;
        int val = 0;
        int error_val = error_buffer[i];
        int error_sign;
        int d = *pred++;//输出

        /* LPC prediction */
        for (j = 0; j < lpc_order; j++)
            val += (pred[j] - d) * lpc_coefs[j];

        val = (val + (1 << (lpc_quant - 1))) >> lpc_quant;
        val += d + error_val;
        buffer_out[i] = sign_extend(val, bps);

        /* adapt LPC coefficients */
        error_sign = sign_only(error_val);

        if (error_sign)
        {
            for (j = 0; j < lpc_order && error_val * error_sign > 0; j++)
            {
                int sign;
                val  = d - pred[j];
                sign = sign_only(val) * error_sign;
                lpc_coefs[j] -= sign;
                val *= sign;
                error_val -= (val >> lpc_quant) * (j + 1);
            }
        }
    }

#endif
}

void decorrelate_stereo(int32_t *buffer[2], int nb_samples,
                        int decorr_shift, int decorr_left_weight)
{
    int i;

    for (i = 0; i < nb_samples; i++)
    {
        int32_t a, b;

        a = buffer[0][i];
        b = buffer[1][i];

        a -= (b * decorr_left_weight) >> decorr_shift;
        b += a;

        buffer[0][i] = b;
        buffer[1][i] = a;
    }
}

void append_extra_bits(int32_t *buffer[2], int32_t *extra_bits_buffer[2],
                       int extra_bits, int channels, int nb_samples)
{
    int i, ch;

    for (ch = 0; ch < channels; ch++)
        for (i = 0; i < nb_samples; i++)
            buffer[ch][i] = (buffer[ch][i] << extra_bits) | extra_bits_buffer[ch][i];
}

int decode_element(ALACContext *alac, unsigned char *out_buffer, int ch_index,
                   int channels)
{
    int has_size, bps, is_compressed, decorr_shift, decorr_left_weight, ret;
    uint32_t output_samples;
    int i, ch;

    skip_bits(&alac->gb, 4);  /* element instance tag */
    skip_bits(&alac->gb, 12); /* unused header bits */

    /* the number of output samples is stored in the frame */
    has_size = get_bits1(&alac->gb);

    alac->extra_bits = get_bits(&alac->gb, 2) << 3;
    bps = alac->sample_size - alac->extra_bits + channels - 1;

    if (bps > 32U)
    {
        return -1;
    }

    /* whether the frame is compressed */
    is_compressed = !get_bits1(&alac->gb);

    if (has_size)
        output_samples = get_bits_long(&alac->gb, 32);
    else
        output_samples = alac->max_samples_per_frame;

    if (!output_samples || output_samples > alac->max_samples_per_frame)
    {
        return -1;
    }

    alac->nb_samples = output_samples;

    if (is_compressed)
    {
        int32_t lpc_coefs[2][32];
        int lpc_order[2];
        int prediction_type[2];
        int lpc_quant[2];
        int rice_history_mult[2];

        decorr_shift       = get_bits(&alac->gb, 8);
        decorr_left_weight = get_bits(&alac->gb, 8);

        for (ch = 0; ch < channels; ch++)
        {
            prediction_type[ch]   = get_bits(&alac->gb, 4);
            lpc_quant[ch]         = get_bits(&alac->gb, 4);
            rice_history_mult[ch] = get_bits(&alac->gb, 3);
            lpc_order[ch]         = get_bits(&alac->gb, 5);

            if (lpc_order[ch] >= alac->max_samples_per_frame)
                return -1;

            /* read the predictor table */
            for (i = lpc_order[ch] - 1; i >= 0; i--)
                lpc_coefs[ch][i] = get_sbits(&alac->gb, 16);
        }

        if (alac->extra_bits)
        {
            for (i = 0; i < alac->nb_samples; i++)
            {
                ALAC_get_more_data(&alac->gb, alac->extra_bits * channels);

                for (ch = 0; ch < channels; ch++)
                    alac->extra_bits_buffer[ch][i] = get_bits(&alac->gb, alac->extra_bits);
            }
        }

        for (ch = 0; ch < channels; ch++)
        {
            int ret = rice_decompress(alac, alac->predict_error_buffer[ch],
                                      alac->nb_samples, bps,
                                      rice_history_mult[ch] * alac->rice_history_mult / 4);

            if (ret < 0)
                return ret;

            /* adaptive FIR filter */
            if (prediction_type[ch] == 15)
            {
                /* Prediction type 15 runs the adaptive FIR twice.
                 * The first pass uses the special-case coef_num = 31, while
                 * the second pass uses the coefs from the bitstream.
                 *
                 * However, this prediction type is not currently used by the
                 * reference encoder.
                 */
                lpc_prediction(alac->predict_error_buffer[ch],
                               alac->predict_error_buffer[ch],
                               alac->nb_samples, bps, NULL, 31, 0);
            }
            else if (prediction_type[ch] > 0)
            {

            }

            lpc_prediction(alac->predict_error_buffer[ch],
                           alac->output_samples_buffer[ch], alac->nb_samples,
                           bps, lpc_coefs[ch], lpc_order[ch], lpc_quant[ch]);
        }
    }
    else
    {
        /* not compressed, easy case */
        for (i = 0; i < alac->nb_samples; i++)
        {
            ALAC_get_more_data(&alac->gb, alac->sample_size * channels);

            for (ch = 0; ch < channels; ch++)
            {
                alac->output_samples_buffer[ch][i] =
                    get_sbits_long(&alac->gb, alac->sample_size);
            }
        }

        alac->extra_bits   = 0;
        decorr_shift       = 0;
        decorr_left_weight = 0;
    }

    if (channels == 2 && decorr_left_weight)
    {
        decorrelate_stereo(alac->output_samples_buffer, alac->nb_samples,
                           decorr_shift, decorr_left_weight);
    }

    if (alac->extra_bits)
    {
        append_extra_bits(alac->output_samples_buffer, alac->extra_bits_buffer,
                          alac->extra_bits, channels, alac->nb_samples);
    }


    switch (alac->sample_size)
    {
        case 16:
        {
            int16_t *outbuffer = (int16_t *)out_buffer;

            for (i = 0; i < alac->nb_samples; i++)
            {
                if (channels == 1)
                {
                    *outbuffer++ = alac->output_samples_buffer[0][i];
                    *outbuffer++ = alac->output_samples_buffer[0][i];
                }
                else
                {
                    *outbuffer++ = alac->output_samples_buffer[0][i];
                    *outbuffer++ = alac->output_samples_buffer[1][i];
                }
            }
        }
        break;

        case 24:
        {
            unsigned char *outbuffer = (unsigned char *)out_buffer;

            for (i = 0; i < alac->nb_samples; i++)
            {
                if (channels == 1)
                {
                    *(int *)outbuffer = alac->output_samples_buffer[0][i];
                    outbuffer += 3;
                    *(int *)outbuffer = alac->output_samples_buffer[0][i];
                    outbuffer += 3;
                }
                else
                {
                    for (ch = 0; ch < channels; ch++)
                    {
                        *(int *)outbuffer = alac->output_samples_buffer[ch][i];
                        outbuffer += 3;
                    }
                }

                outbuffer += alac->channels - channels;
            }
        }
        break;

        case 32:
        {
            int32_t *outbuffer = (int32_t *)out_buffer;

            for (i = 0; i < alac->nb_samples; i++)
            {
                if (channels == 1)
                {
                    *outbuffer++ = alac->output_samples_buffer[0][i];
                    *outbuffer++ = alac->output_samples_buffer[0][i];
                }
                else
                {
                    *outbuffer++ = alac->output_samples_buffer[0][i];
                    *outbuffer++ = alac->output_samples_buffer[1][i];
                }

                outbuffer += alac->channels - channels;
            }
        }
        break;
    }

    return 0;
}


int alac_decode_frame(ALACContext *alac, void *data, AVPacket *avpkt)
{
    unsigned char *out_buffer    = data;
    enum AlacRawDataBlockType element;
    int channels;
    int ch, ret, got_end;
    int bytes_read;

    if ((ret = init_get_bits8(&alac->gb, avpkt->data, avpkt->size)) < 0) //初始化读数据的buffer
        return ret;

    got_end = 0;
    alac->nb_samples = 0;
    ch = 0;

    while (get_bits_left(&alac->gb) >= 3)
    {
        ALAC_get_more_data(&alac->gb, in_buf_size);
        element = get_bits(&alac->gb, 3);

        if (element == TYPE_END)
        {
            got_end = 1;
            break;
        }

        if (element > TYPE_CPE && element != TYPE_LFE)
        {
            return -1;
        }

        channels = (element == TYPE_CPE) ? 2 : 1;

        if (ch + channels > alac->channels ||
            ff_alac_channel_layout_offsets[alac->channels - 1][ch] + channels > alac->channels)
        {
            return -1;
        }

        ret = decode_element(alac, out_buffer,
                             ff_alac_channel_layout_offsets[alac->channels - 1][ch],
                             channels);

        if (ret < 0 && get_bits_left(&alac->gb))
            return ret;

        ch += channels;
    }

    if (!got_end)
    {

        return -1;
    }

    if (avpkt->size * 8 - get_bits_count(&alac->gb) > 8)
    {

    }

    bytes_read = ((get_bits_count(&alac->gb) + 7) >> 3);

    return bytes_read;
}

int alac_decode_close(ALACContext *alac)
{
    int ch;
    /* for (ch = 0; ch < FFMIN(alac->channels, 2); ch++) {
         free(&alac->predict_error_buffer[ch]);
         if (!alac->direct_output)
             free(&alac->output_samples_buffer[ch]);
         free(&alac->extra_bits_buffer[ch]);
     }*/

    return 0;
}

int allocate_buffers(ALACContext *alac)
{
    int ch;
    int buf_size = alac->max_samples_per_frame * sizeof(int32_t);

    for (ch = 0; ch < FFMIN(alac->channels, 2); ch++)
    {
        alac->predict_error_buffer[ch] = alac_malloc(buf_size);
        alac->direct_output = 1;
        // if (!alac->direct_output) {

        alac->output_samples_buffer[ch] = alac_malloc(buf_size);

        // }

        alac->extra_bits_buffer[ch] = alac_malloc(buf_size);
    }

    return 0;
buf_alloc_fail:
    alac_decode_close(alac);
    return -1;
}

int alac_set_info(ALACContext *alac)
{
    ALACInfo *alacs = (ALACInfo*)alac_info;

    alacs->size = BYTESWAP(alacs->size);
    alacs->sign = BYTESWAP(alacs->sign);
    alacs->version = BYTESWAP(alacs->version);
    alacs->max_samples_per_frame = BYTESWAP(alacs->max_samples_per_frame);
    alacs->maxRun = WORDSWAP(alacs->maxRun);
    alacs->max_frame_size = BYTESWAP(alacs->max_frame_size);
    alacs->average_bitrate = BYTESWAP(alacs->average_bitrate);
    alacs->samplerate = BYTESWAP(alacs->samplerate);

    Hifi_Alac_Printf("ALAC 最大帧size = %d ", alacs->max_frame_size);
    Hifi_Alac_Printf("每帧最大采样点 = %d", alacs->max_samples_per_frame);
    Hifi_Alac_Printf("bps = %d ", alacs->average_bitrate);
    Hifi_Alac_Printf("fs = %d ", alacs->samplerate);
    Hifi_Alac_Printf("位数 = %d", alacs->sample_size);
    Hifi_Alac_Printf("通道数 = %d", alacs->channels);
    Hifi_Alac_Printf("maxRun = %d", alacs->maxRun);
    Hifi_Alac_Printf("rice_history_mult = %d", alacs->rice_history_mult);
    Hifi_Alac_Printf("rice_initial_history =%d", alacs->rice_initial_history);
    Hifi_Alac_Printf("rice_limit = %d \n", alacs->rice_limit);

    alac->max_samples_per_frame = alacs->max_samples_per_frame;
    alac->sample_size          =  alacs->sample_size;
    alac->rice_history_mult    =  alacs->rice_history_mult;
    alac->rice_initial_history =  alacs->rice_initial_history;
    alac->rice_limit           =  alacs->rice_limit;
    alac->channels             =  alacs->channels;
    alac->average_bitrate      =  alacs->average_bitrate ;
    alac->samplerate      =  alacs->samplerate ;

    if (!alac->max_samples_per_frame ||
        alac->max_samples_per_frame > INT_MAX / sizeof(int32_t))
    {
        return -1;
    }

    return 0;
}

int alac_decode_init(ALACContext *alac)
{
    int ret;

    if (alac_set_info(alac))
    {
        return -1;
    }

    if ((ret = allocate_buffers(alac)) < 0)
    {
        return ret;
    }

    return 0;
}
#pragma arm section code
#endif
#endif
