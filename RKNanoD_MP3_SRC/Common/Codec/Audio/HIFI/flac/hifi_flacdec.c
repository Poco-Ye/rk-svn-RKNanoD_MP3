/*
 * FLAC (Free Lossless Audio Codec) decoder
 * @author Alex Beregszaszi
 *
 * For more information on the FLAC format, visit:
 *  http://flac.sourceforge.net/
 *
 * This decoder can be used in 1 of 2 ways: Either raw FLAC data can be fed
 * through, starting from the initial 'fLaC' signature; or by passing the
 * 34-byte streaminfo structure through avctx->extradata[_size] followed
 * by data starting with the 0xFFF8 marker.
 */
#include "SysInclude.h"
#include "audio_main.h"

#ifdef A_CORE_DECODE
#ifdef HIFI_FlAC_DECODE
#include <limits.h>
#include <stdio.h>
#include "audio_file_access.h"
#pragma arm section code = "FlacHDecCode", rodata = "FlacHDecCode", rwdata = "FlacHDecData", zidata = "FlacHDecBss"

#include "flacdec.h"
#include "hifi.h"
#include "Hw_hifi.h"

FLACStreaminfo s_FLAC_INFO;
extern FLACContext s_flac_str;
extern FILE *flac_file_handle;
extern int metsize ;
extern unsigned char inbuf[in_buf_size + 8];
#define max(a,b) ((a) > (b) ? (a) : (b))
#define AV_RB8(x)     (((const unsigned char*)(x))[0])
#define AV_RB32(x)                                \
    (((unsigned int)((const unsigned char*)(x))[0] << 24) |    \
     (((const unsigned char*)(x))[1] << 16) |    \
     (((const unsigned char*)(x))[2] <<  8) |    \
     ((const unsigned char*)(x))[3])
#define AV_RB24(x)                           \
    ((((const unsigned char*)(x))[0] << 16) |         \
     (((const unsigned char*)(x))[1] <<  8) |         \
     ((const unsigned char*)(x))[2])
#define AV_RL32(x)                                \
    (((unsigned int)((const unsigned char*)(x))[3] << 24) |    \
     (((const unsigned char*)(x))[2] << 16) |    \
     (((const unsigned char*)(x))[1] <<  8) |    \
     ((const unsigned char*)(x))[0])

void allocate_buffers(FLACContext *s);
int sample_size_table[] = { 0, 8, 12, 0, 16, 20, 24, 0 };

int ff_flac_sample_rate_table[16] =
{
    0,
    88200, 176400, 192000,
    8000, 16000, 22050, 24000, 32000, 44100, 48000, 96000,
    0, 0, 0, 0
};

int ff_flac_blocksize_table[16] =
{
    0,    192, 576 << 0, 576 << 1, 576 << 2, 576 << 3,      0,      0,
    256 << 0, 256 << 1, 256 << 2, 256 << 3, 256 << 4, 256 << 5, 256 << 6, 256 << 7
};


unsigned char table_crc8[256] =
{
    0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15,
    0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d,
    0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65,
    0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d,
    0xe0, 0xe7, 0xee, 0xe9, 0xfc, 0xfb, 0xf2, 0xf5,
    0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd,
    0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85,
    0xa8, 0xaf, 0xa6, 0xa1, 0xb4, 0xb3, 0xba, 0xbd,
    0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2,
    0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea,
    0xb7, 0xb0, 0xb9, 0xbe, 0xab, 0xac, 0xa5, 0xa2,
    0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a,
    0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32,
    0x1f, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0d, 0x0a,
    0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42,
    0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a,
    0x89, 0x8e, 0x87, 0x80, 0x95, 0x92, 0x9b, 0x9c,
    0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4,
    0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec,
    0xc1, 0xc6, 0xcf, 0xc8, 0xdd, 0xda, 0xd3, 0xd4,
    0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c,
    0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44,
    0x19, 0x1e, 0x17, 0x10, 0x05, 0x02, 0x0b, 0x0c,
    0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
    0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b,
    0x76, 0x71, 0x78, 0x7f, 0x6a, 0x6d, 0x64, 0x63,
    0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b,
    0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13,
    0xae, 0xa9, 0xa0, 0xa7, 0xb2, 0xb5, 0xbc, 0xbb,
    0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8d, 0x84, 0x83,
    0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb,
    0xe6, 0xe1, 0xe8, 0xef, 0xfa, 0xfd, 0xf4, 0xf3
};

/*int my_bui_clz(unsigned int a)
{
    int i;
    unsigned int tmp_index = 1 << 31;
    if(a == 0)
        return 31;
    for(i=0;i<32;i++)
    {
        if((a & tmp_index)==tmp_index)
            return i;
        else tmp_index = tmp_index >> 1;
    }
}*/

long long int get_utf8(GetBitContext *gb)
{
    long long int val;
    GET_UTF8(val, get_bits(gb, 8), return -1;)
    return val;
}

void dump_headers( FLACStreaminfo *s)
{
    Hifi_Flac_Printf("Max Blocksize: %d\n", s->max_blocksize);
    Hifi_Flac_Printf("Min Framesize: %d\n", s->min_framesize);
    Hifi_Flac_Printf("Max Framesize: %d\n", s->max_framesize);
    Hifi_Flac_Printf("samples: %lld\n", s->samples);
    Hifi_Flac_Printf("Samplerate: %d\n", s->samplerate);
    Hifi_Flac_Printf("Channels: %d\n", s->channels);
    Hifi_Flac_Printf("Bits: %d\n", s->bps);
}

void ff_flac_parse_streaminfo(FLACStreaminfo *s, unsigned char *buffer)
{
    GetBitContext gb;
    init_get_bits(&gb, buffer, FLAC_STREAMINFO_SIZE * 8);
    skip_bits(&gb, 16); /* skip min blocksize */
    s->max_blocksize = get_bits(&gb, 16);

    if (s->max_blocksize < FLAC_MIN_BLOCKSIZE)
    {
        s->max_blocksize = 16;
    }

    //skip_bits(&gb, 24); /* skip min frame size */
    s->min_framesize = get_bits_long(&gb, 24);
    s->max_framesize = get_bits_long(&gb, 24);
    s->samplerate = get_bits_long(&gb, 20);
    s->channels = get_bits(&gb, 3) + 1;
    s->bps = get_bits(&gb, 5) + 1;
    s->samples  = get_bits_long(&gb, 32) << 4;
    s->samples |= get_bits(&gb, 4);
    dump_headers(s);
    skip_bits_long(&gb, 64); /* md5 sum */
    skip_bits_long(&gb, 64); /* md5 sum */
    s_FLAC_INFO.max_blocksize = s->max_blocksize ;
    s_FLAC_INFO.samplerate = s->samplerate;
    s_FLAC_INFO.channels = s->channels;
    s_FLAC_INFO.bps = s->bps;
    s_FLAC_INFO.samples = s->samples;
    s_FLAC_INFO.min_framesize = s->min_framesize ;
    s_FLAC_INFO.max_framesize = s->max_framesize ;
}


unsigned int bytestream_get_byte(const unsigned char **b)
{
    (*b) += 1;
    return AV_RB8(*b - 1);
}

unsigned int bytestream_get_be24(const unsigned char **b)
{
    (*b) += 3;
    return AV_RB24(*b - 3);
}
void ff_flac_parse_block_header(const unsigned char *block_header,
                                int *last, int *type, int *size)
{
    int tmp = bytestream_get_byte(&block_header);

    if (last)
        *last = tmp & 0x80;

    if (type)
        *type = tmp & 0x7F;

    if (size)
        *size = bytestream_get_be24(&block_header);
}

/**
 * Parse the STREAMINFO from an  header.
 * @param s the flac decoding context
 * @param buf input buffer, starting with the "fLaC" marker
 * @param buf_size buffer size
 * @return non-zero if metadata is invalid
 */
int parse_streaminfo(FLACContext *s,  unsigned char *buf, int buf_size)
{
    int metadata_type, metadata_size;

    if (buf_size < FLAC_STREAMINFO_SIZE + 8)
    {
        /* need more data */
        return 0;
    }

    ff_flac_parse_block_header(&buf[4], NULL, &metadata_type, &metadata_size);

    if (metadata_type != FLAC_METADATA_TYPE_STREAMINFO || metadata_size != FLAC_STREAMINFO_SIZE)
    {
        return AVERROR_INVALIDDATA;
    }

    ff_flac_parse_streaminfo((FLACStreaminfo *)s, &buf[8]);
    //allocate_buffers(s);
    s->got_streaminfo = 1;
    return 0;
}

/**
 * Determine the size of an  header.
 * @param buf input buffer, starting with the "fLaC" marker
 * @param buf_size buffer size
 * @return number of bytes in the header, or 0 if more data is needed
 */
extern FILE *flac_file_handle;;
int get_metadata_size( unsigned char *buf, int buf_size)
{
    int metadata_last, metadata_size;
    unsigned char *buf_end = buf + buf_size;
    unsigned char *read_buf = buf;
    int read_byte = 0;
    buf += 4;
    read_byte += 4;

    do
    {
        ff_flac_parse_block_header(buf, &metadata_last, NULL, &metadata_size);
        buf += 4;
        read_byte += 4;
        read_byte += metadata_size;

        if (buf + metadata_size > buf_end)
        {
            RKFIO_FSeek(metadata_size - (buf_end - buf), SEEK_CUR, flac_file_handle);
            RKFIO_FRead(read_buf,  buf_size, flac_file_handle);
            buf = read_buf;
        }
        else
        {
            buf += metadata_size;
        }
    }
    while (!metadata_last);

    return read_byte;
}

/**
 * read unsigned golomb rice code (jpegls).
 */
int get_ur_golomb_jpegls(GetBitContext *gb, int k, int limit, int esc_len)
{
    unsigned int buf;
    int log;
    OPEN_READER(re, gb);
    UPDATE_CACHE(re, gb);
    buf = GET_CACHE(re, gb);
    log = av_log2(buf);

    if(log - k >= 32 - MIN_CACHE_BITS + (MIN_CACHE_BITS == 32) && 32 - log < limit)
    {
        buf >>= log - k;
        buf += (30 - log) << k;
        LAST_SKIP_BITS(re, gb, 32 + k - log);
        CLOSE_READER(re, gb);
        return buf;
    }
    else
    {
        int i;

        for(i = 0; i < limit && SHOW_UBITS(re, gb, 1) == 0; i++)
        {
            LAST_SKIP_BITS(re, gb, 1);
            UPDATE_CACHE(re, gb);
        }

        SKIP_BITS(re, gb, 1);

        if(i < limit - 1)
        {
            if(k)
            {
                buf = SHOW_UBITS(re, gb, k);
                LAST_SKIP_BITS(re, gb, k);
            }
            else
            {
                buf = 0;
            }

            CLOSE_READER(re, gb);
            return buf + (i << k);
        }
        else if(i == limit - 1)
        {
            buf = SHOW_UBITS(re, gb, esc_len);
            LAST_SKIP_BITS(re, gb, esc_len);
            CLOSE_READER(re, gb);
            return buf + 1;
        }
        else
            return -1;
    }
}

/**
 * read signed golomb rice code (flac).
 */
int get_sr_golomb_flac(GetBitContext *gb, int k, int limit, int esc_len)
{
    int v = get_ur_golomb_jpegls(gb, k, limit, esc_len);
    return (v >> 1) ^ -(v & 1);
}
int decode_residuals(FLACContext *s, int channel, int pred_order)
{
    int i, tmp, partition, method_type, rice_order;
    int sample = 0, samples;
    FLAC_get_more_data(&s->gb, 6);
    method_type = get_bits(&s->gb, 2);

    if (method_type > 1)
    {
        return -1;
    }

    rice_order = get_bits(&s->gb, 4);
    samples = s->blocksize >> rice_order;

    if (pred_order > samples)
    {
        return -1;
    }

    sample = i = pred_order;

    for (partition = 0; partition < (1 << rice_order); partition++)
    {
        FLAC_get_more_data(&s->gb, 10);
        tmp = get_bits(&s->gb, method_type == 0 ? 4 : 5);

        if (tmp == (method_type == 0 ? 15 : 31))
        {
            tmp = get_bits(&s->gb, 5);

            for (; i < samples; i++, sample++)
            {
                FLAC_get_more_data(&s->gb, tmp);
                s->decoded[channel][sample] = get_sbits_long(&s->gb, tmp);
            }
        }
        else
        {
            for (; i < samples; i++, sample++)
            {
                FLAC_get_more_data(&s->gb, 256);
                s->decoded[channel][sample] = get_sr_golomb_flac(&s->gb, tmp, INT_MAX, 0);
            }
        }

        i = 0;
    }

    return 0;
}

int decode_subframe_fixed(FLACContext *s, int channel, int pred_order)
{
    const int blocksize = s->blocksize;
    int *decoded = s->decoded[channel];
    int a, b, c, d, i;
    /* warm up samples */
    FLAC_get_more_data(&s->gb, 32 * 32);

    for (i = 0; i < pred_order; i++)
    {
        decoded[i] = get_sbits(&s->gb, s->curr_bps);
    }

    if (decode_residuals(s, channel, pred_order) < 0)
        return -1;

    if (pred_order > 0)
        a = decoded[pred_order - 1];

    if (pred_order > 1)
        b = a - decoded[pred_order - 2];

    if (pred_order > 2)
        c = b - decoded[pred_order - 2] + decoded[pred_order - 3];

    if (pred_order > 3)
        d = c - decoded[pred_order - 2] + 2 * decoded[pred_order - 3] - decoded[pred_order - 4];

    switch (pred_order)
    {
        case 0:
            break;

        case 1:
            for (i = pred_order; i < blocksize; i++)
                decoded[i] = a += decoded[i];

            break;

        case 2:
            for (i = pred_order; i < blocksize; i++)
                decoded[i] = a += b += decoded[i];

            break;

        case 3:
            for (i = pred_order; i < blocksize; i++)
                decoded[i] = a += b += c += decoded[i];

            break;

        case 4:
            for (i = pred_order; i < blocksize; i++)
                decoded[i] = a += b += c += d += decoded[i];

            break;

        default:
            return -1;
    }

    return 0;
}
int flag = 0;
int flac_lpc_cnt = 0;
int flac_max_order = 0;
int decode_subframe_lpc(FLACContext *s, int channel, int pred_order)
{
    int i, j;
    int coeff_prec, qlevel;
    int coeffs[32];
    int *decoded = s->decoded[channel];

    if(flag == 0)
    {
        flag = 1;
    }

    /* warm up samples */
    FLAC_get_more_data(&s->gb, 32 * 64 + 9);

    for (i = 0; i < pred_order; i++)
    {
        decoded[i] = get_sbits(&s->gb, s->curr_bps);
    }

    coeff_prec = get_bits(&s->gb, 4) + 1;//系数精度

    if (coeff_prec == 16)
    {
        return -1;
    }

    qlevel = get_sbits(&s->gb, 5);//量化位数

    if (qlevel < 0)
    {
        return -1;
    }

#if 1

    for (i = 0; i < pred_order; i++)
    {
        coeffs[pred_order - 1 - i] = get_sbits(&s->gb, coeff_prec);
    }

#else

    //对应(*(--history))
    for (i = 0; i < pred_order; i++)
    {
        coeffs[i] = get_sbits(&s->gb, coeff_prec);
    }

#endif

    if (decode_residuals(s, channel, pred_order) < 0)
        return -1;

#if 1
#ifndef HIFI_ACC
    Flac_Decode_Lpc( decoded, decoded,
                     s->blocksize, s->bps, coeffs, NULL,
                     pred_order,  qlevel , NULL, 0);
#else

    if((pred_order == 1) || (pred_order == 2))
    {
        Flac_Decode_Lpc( decoded, decoded,
                         s->blocksize, s->bps, coeffs, NULL,
                         pred_order,  qlevel , NULL, 0);
    }
    else
    {
        Hifi_Set_ACC_XFER_Disable(0, s->blocksize, HIfi_ACC_TYPE_FLAC); //开始传输配置数据和初始化系数(不往fifo送)
        Hifi_Enable_FUN_DONE_FLAG(0);
        Flac_Set_CFG(0, pred_order, qlevel);
        HIFITranData((uint32*)coeffs, (uint32*)FLAC_COEF_ADD, pred_order);
        Hifi_Set_ACC_XFER_Start(0, s->blocksize, HIfi_ACC_TYPE_FLAC); //可以开始往fifo传数据，并且可以取数据
        HIFI_DMA_TO_ACC(decoded, (uint32*)TX_FIFO, s->blocksize, (uint32*)RX_FIFO, decoded);

        while(Hifi_Get_ACC_Intsr(0, Function_done_interrupt_active) != Function_done_interrupt_active) ;

        Hifi_Clear_FUN_DONE_FLAG(0);
    }

#endif
#else

    if (s->bps > 16)
    {
        long long int sum;
        int *history ;

        for (i = pred_order; i < s->blocksize; i++)
        {
            sum = 0;
            history = &decoded[i];

            for (j = 0; j < pred_order; j++)
                sum += (long long )coeffs[j] * (*(--history));

            decoded[i] += sum >> qlevel;
        }
    }
    else
    {
        for (i = pred_order; i < s->blocksize - 1; i += 2)
        {
            int c;
            int d = decoded[i - pred_order];
            int s0 = 0, s1 = 0;

            for (j = pred_order - 1; j > 0; j--)
            {
                c = coeffs[j];
                s0 += c * d;
                d = decoded[i - j];
                s1 += c * d;
            }

            c = coeffs[0];
            s0 += c * d;
            d = decoded[i] += s0 >> qlevel;
            s1 += c * d;
            decoded[i + 1] += s1 >> qlevel;
        }

        if (i < s->blocksize)
        {
            int sum = 0;

            for (j = 0; j < pred_order; j++)
                sum += coeffs[j] * decoded[i - j - 1];

            decoded[i] += sum >> qlevel;
        }
    }

#endif
    return 0;
}

int decode_subframe(FLACContext *s, int channel)
{
    int type, wasted = 0;
    int i, tmp;
    s->curr_bps = s->bps;

    if (channel == 0)
    {
        if (s->ch_mode == FLAC_CHMODE_RIGHT_SIDE)
            s->curr_bps++;
    }
    else
    {
        if (s->ch_mode == FLAC_CHMODE_LEFT_SIDE || s->ch_mode == FLAC_CHMODE_MID_SIDE)
        {
            s->curr_bps++;
        }
    }

    FLAC_get_more_data(&s->gb, 64);

    if (get_bits1(&s->gb))
    {
        Hifi_Flac_Printf("invalid subframe padding\n");
        return -1;
        // frm_cnt++;
    }

    type = get_bits(&s->gb, 6);

    if (get_bits1(&s->gb))
    {
        wasted = 1;

        while (!get_bits1(&s->gb))
            wasted++;

        s->curr_bps -= wasted;
    }

    if (s->curr_bps > 32)
    {
        Hifi_Flac_Printf("s->curr_bps > 32\n");
        return -1;
    }

    if (type == 0)
    {
        tmp = get_sbits_long(&s->gb, s->curr_bps);

        for (i = 0; i < s->blocksize; i++)
            s->decoded[channel][i] = tmp;
    }
    else if (type == 1)
    {
        for (i = 0; i < s->blocksize; i++)
        {
            FLAC_get_more_data(&s->gb, 64);
            s->decoded[channel][i] = get_sbits_long(&s->gb, s->curr_bps);
        }
    }
    else if ((type >= 8) && (type <= 12))
    {
        if (decode_subframe_fixed(s, channel, type & ~0x8) < 0)
            return -1;
    }
    else if (type >= 32)
    {
        if (decode_subframe_lpc(s, channel, (type & ~0x20) + 1) < 0)
            return -1;
    }
    else
    {
        Hifi_Flac_Printf("invalid coding type\n");
        return -1;
    }

    /* {
         int i ;
         Hifi_Flac_Printf("****%d 帧******\n",frm_cnt++);
         for(i = 0;i < s->blocksize/16;i++)
         {
            Hifi_Flac_Printf("\n %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d ",
              s->decoded[channel][16*i],s->decoded[channel][16*i+1],s->decoded[channel][16*i+2],s->decoded[channel][16*i+3],
              s->decoded[channel][16*i+4],s->decoded[channel][16*i+5],s->decoded[channel][16*i+6],s->decoded[channel][16*i+7],
              s->decoded[channel][16*i+8],s->decoded[channel][16*i+9],s->decoded[channel][16*i+10],s->decoded[channel][16*i+11],
              s->decoded[channel][16*i+12],s->decoded[channel][16*i+13],s->decoded[channel][16*i+14],s->decoded[channel][16*i+15]);
          }

     }*/

    if (wasted)
    {
        int i;

        for (i = 0; i < s->blocksize; i++)
            s->decoded[channel][i] <<= wasted;
    }

    return 0;
}

int get_crc8(const unsigned char *buf, int count)
{
    int crc = 0;
    int i;

    for(i = 0; i < count; i++)
    {
        crc = table_crc8[crc ^ buf[i]];
    }

    return crc;
}
unsigned char sync_code = 0xf8;
int ff_flac_decode_frame_header(GetBitContext *gb, FLACFrameInfo *fi, int log_level_offset)
{
    int bs_code, sr_code, bps_code, crc8;
    int sync_flag = 0;
    int find_sync_bytes = 0;
    unsigned char buf[2];
    int ret = 0;

    /* frame sync code */
    while(sync_flag == 0)
    {
        ret = FLAC_get_more_data(gb, 16);

        if(ret == 0)
        {
            return  -1;
        }

        buf[0] = get_bits(gb, 8);
        find_sync_bytes ++;

        if (buf[0] == 0xff)
        {
            buf[1] = get_bits(gb, 8);
            find_sync_bytes ++;

            if(buf[1]  == sync_code)
            {
                /*fi->is_var_size = (buf[1] & 0x1);

                if(fi->is_var_size )
                {
                    Hifi_Flac_Printf("var blocksize not surport\n");
                    return  -1;
                }
                */
                sync_flag = 1;
            }
        }

        if ((s_FLAC_INFO.max_framesize != 0) && (find_sync_bytes > in_buf_size))//s_FLAC_INFO.max_framesize))
        {
            //Hifi_Flac_Printf("invalid sync code\n");
            return  -2;
        }
    }

    FLAC_get_more_data(gb, 64);
    /* block size and sample rate codes */
    bs_code = get_bits(gb, 4);
    sr_code = get_bits(gb, 4);
    /* channels and decorrelation */
    fi->ch_mode = get_bits(gb, 4);

    if (fi->ch_mode < FLAC_MAX_CHANNELS)
    {
        fi->channels = fi->ch_mode + 1;
        fi->ch_mode = FLAC_CHMODE_INDEPENDENT;
    }
    else if (fi->ch_mode <= FLAC_MAX_CHANNELS + FLAC_CHMODE_MID_SIDE)
    {
        fi->channels = 2;
    }
    else
    {
        Hifi_Flac_Printf("invalid channel mode: %d\n", fi->ch_mode);
        return -1;
    }

    /* bits per sample */
    bps_code = get_bits(gb, 3);

    if (bps_code == 3 || bps_code == 7)
    {
        Hifi_Flac_Printf("invalid sample size code (%d)\n", bps_code);
        return -1;
    }

    fi->bps = sample_size_table[bps_code];

    /* reserved bit */
    if (get_bits1(gb))
    {
        Hifi_Flac_Printf("broken stream, invalid padding\n");
        return -1;
    }

    /* sample or frame count */
    fi->frame_or_sample_num = get_utf8(gb);

    if (fi->frame_or_sample_num < 0)
    {
        Hifi_Flac_Printf("sample/frame number invalid; utf8 fscked\n");
        return -1;
    }

    if(fi->frame_or_sample_num == 0)
    {
        sync_code  = buf[1];
        Hifi_Flac_Printf("sync code 0xFF 0x%x\n", sync_code);
    }

    /* blocksize */
    if (bs_code == 0)
    {
        Hifi_Flac_Printf("reserved blocksize code: 0\n");
        return -1;
    }
    else if (bs_code == 6)
    {
        fi->blocksize = get_bits(gb, 8) + 1;
    }
    else if (bs_code == 7)
    {
        fi->blocksize = get_bits(gb, 16) + 1;
    }
    else
    {
        fi->blocksize = ff_flac_blocksize_table[bs_code];
    }

    /* sample rate */
    if (sr_code < 12)
    {
        fi->samplerate = ff_flac_sample_rate_table[sr_code];
    }
    else if (sr_code == 12)
    {
        fi->samplerate = get_bits(gb, 8) * 1000;
    }
    else if (sr_code == 13)
    {
        fi->samplerate = get_bits(gb, 16);
    }
    else if (sr_code == 14)
    {
        fi->samplerate = get_bits(gb, 16) * 10;
    }
    else
    {
        Hifi_Flac_Printf("illegal sample rate code %d\n", sr_code);
        return -1;
    }

    {
        extern int g_hf_FFW_FFD ;

        if(g_hf_FFW_FFD)
        {
            g_hf_FFW_FFD = 0;
            s_FLAC_INFO.samples_decoded = (fi->frame_or_sample_num) * fi->blocksize;
            //Hifi_Flac_Printf("444samples_decoded = %d", s_FLAC_INFO.samples_decoded);
        }
    }

    skip_bits(gb, 8);
    get_crc8(gb->buffer, get_bits_count(gb) / 8);
    return 0;
}

int decode_frame(FLACContext *s)
{
    int i,ret;
    GetBitContext *gb = &s->gb;
    unsigned char *buf = inbuf;
    FLACFrameInfo fi;
#if 0
    if (ff_flac_decode_frame_header( gb, &fi, 0) < 0)
    {
        return -1;
    }
#endif
    while (1)
    {
        ret = ff_flac_decode_frame_header(gb, &fi, 0);

        if (ret == -2)
        {
            RKFIO_FRead(inbuf , in_buf_size ,  flac_file_handle );
            //buf = inbuf;
            init_get_bits(gb, inbuf, (in_buf_size)* 8);

        }
        else if (ret < 0)
            return -1;
        else
        {
            break;
        }

    }

    s->channels = fi.channels;
    s->ch_mode = fi.ch_mode;

    if (!fi.bps)
    {
        fi.bps = s->bps;
    }

    s->bps = fi.bps;

    if (s->bps > 16)
    {
        s->sample_shift = 32 - s->bps;
        s->is32 = 1;
    }
    else
    {
        s->sample_shift = 16 - s->bps;
        s->is32 = 0;
    }

    s->max_blocksize = FLAC_MAX_BLOCKSIZE;

    if (fi.blocksize > s->max_blocksize)
    {
        Hifi_Flac_Printf("blocksize %d > %d\n", fi.blocksize, s->max_blocksize);
        return -1;
    }

    s->blocksize = fi.blocksize;
    s->samplerate = fi.samplerate;

    /* if (!s->got_streaminfo) {
         allocate_buffers(s);
         s->got_streaminfo = 1;
         dump_headers((FLACStreaminfo *)s);
     }*/

    /* subframes */
    for (i = 0; i < s->channels; i++)
    {
        if (decode_subframe(s, i) < 0)
            return -1;
    }

    align_get_bits(gb);
    FLAC_get_more_data(&s->gb, 16);
    /* frame footer */
    skip_bits(gb, 16); /* data crc */
    return 0;
}

int flac_decode_frame(void *data, int *data_size, AVPacket *avpkt)
{
    unsigned char *buf = avpkt->data;
    int buf_size = avpkt->size;
    FLACContext *s = &s_flac_str;
    int i, j = 0, bytes_read = 0;
    int alloc_data_size = *data_size;
    int output_size;
    unsigned int metsize;
    *data_size = 0;
//  printf("------------decode frame------------\n");

    if (buf_size < FLAC_MIN_FRAME_SIZE)
    {
        return buf_size;
    }

    init_get_bits(&s->gb, buf, (buf_size) * 8); //直接跳头

    if (decode_frame(s) < 0)
    {
        return -1;
    }

    bytes_read = ((get_bits_count(&s->gb) + 7) / 8);
    /* check if allocated data size is large enough for output */
    output_size = s->blocksize * s->channels * (s->is32 ? 4 : 2);
    s_FLAC_INFO.samples_decoded += s->blocksize;
    //Hifi_Flac_Printf("2222samples_decoded = %d", s_FLAC_INFO.samples_decoded);

    if (output_size > alloc_data_size)
    {
        Hifi_Flac_Printf("output data size is larger than allocated data size\n");
        return -1;
    }

    {
        int *decoded[2];
        decoded[0] = s->decoded[0];
        decoded[1] = s->decoded[1];
        *data_size = Flac_deblock(data, decoded, max(0, (s->ch_mode - 7)), s->bps, s->blocksize);
        *data_size = s->blocksize;//获取采样点
    }

    return bytes_read;
}

int FLAC_get_more_data(GetBitContext *gb, int bits_needed )
{
    int ret;
    int index ;
    int left_byte ;
    int bits_left = get_bits_left(gb);

    if(bits_left < 0)
    {
        Hifi_Flac_Printf("bits_left 负%d\n", bits_left);
        bits_left = 0;
    }

    if(bits_needed > bits_left )
    {
        index = gb->index >> 3;
        left_byte = (bits_left + 7) >> 3;
        MemMov2(gb->buffer, &gb->buffer[index], left_byte);
        ret = RKFIO_FRead(&gb->buffer[left_byte], in_buf_size - left_byte, flac_file_handle);
        gb->index  = gb->index % 8;

        if(ret == 0)
        {
            return 0;
        }
    }

    return 1;
}
#pragma arm section code
#endif
#endif

