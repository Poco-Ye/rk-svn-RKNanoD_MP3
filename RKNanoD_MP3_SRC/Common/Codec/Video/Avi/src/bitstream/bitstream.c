/*****************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  - Bitstream reader/writer -
 *
 *  Copyright (C) 2001-2003 Peter Ross <pross@xvid.org>
 *                     2003 Cristoph Lampert <gruel@web.de>
 *
 *  This program is free software ; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation ; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY ; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program ; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * $Id: bitstream.c,v 1.55.2.1 2006/07/10 15:05:30 Isibaar Exp $
 *
 ****************************************************************************/

//#include <string.h>
//#include <stdio.h>

#include "bitstream.h"
#include "zigzag.h"
//#include "../dequant/dequant_matrix.h"
#include "mbcoding.h"

#ifdef XVID_INCLUDE

#ifdef MDK_PLATFORM
#pragma arm section code = "XvidDecCode", rodata = "XvidDecCode", rwdata = "XvidDecData", zidata = "XvidDecBss"
#endif

/*****************************************************************************
 * Bitstream
 ****************************************************************************/

/* Input buffer should be readable as full chunks of 8bytes, including
the end of the buffer. Padding might be appropriate. If only chunks
of 4bytes are applicable, define XVID_SAFE_BS_TAIL. Note that this will
slow decoding, so consider this as a last-resort solution */
//#define XVID_SAFE_BS_TAIL

/* initialise bitstream structure */

 void /*__inline*/
BitstreamInit(Bitstream *  bs,
              void * bitstream,
              uint32_t length)
{
    uint32_t tmp;
    size_t bitpos;
    ptr_t adjbitstream = (ptr_t)bitstream;

    /*
     * Start the stream on a uint32_t boundary, by rounding down to the
     * previous uint32_t and skipping the intervening bytes.
     */
    bitpos = ((sizeof(uint32_t) - 1) & (size_t)bitstream);
    adjbitstream = adjbitstream - bitpos;  // 把指针调整到uint32对齐
    //bs->start = bs->tail = (uint32_t *) adjbitstream;

    //modified by evan wu,
    bs->start = (uint32_t *) adjbitstream;
    bs->tail = bs->start + 2;

	XVID_ASSERT(bitpos == 0); // 必须保证4字节对齐

    tmp = *bs->start;
    BSWAP(tmp);
    bs->bufa = tmp;

    tmp = *(bs->start + 1);
    BSWAP(tmp);
    bs->bufb = tmp;

    bs->pos = bs->initpos = bitpos * 8;
    /* preserve the intervening bytes */

    if (bs->initpos > 0)
        bs->buf = bs->bufa & (0xffffffff << (32 - bs->initpos));
    else
        bs->buf = 0;

	XVID_ASSERT(length >= 2*sizeof(uint32_t));

	if(length >= 2*sizeof(uint32_t))
    	bs->length = length - 2*sizeof(uint32_t);
}

#if 0
/* reset bitstream state */

 void /*__inline*/
BitstreamReset(Bitstream *  bs)
{
    uint32_t tmp;

    bs->tail = bs->start;

    tmp = *bs->start;
    BSWAP(tmp);
    bs->bufa = tmp;

    tmp = *(bs->start + 1);
    BSWAP(tmp);
    bs->bufb = tmp;

    /* preserve the intervening bytes */

    if (bs->initpos > 0)
        bs->buf = bs->bufa & (0xffffffff << (32 - bs->initpos));
    else
        bs->buf = 0;

    bs->pos = bs->initpos;
}
#endif

/* reads n bits from bitstream without changing the stream pos */

 uint32_t /*__inline*/
BitstreamShowBits(Bitstream *  bs,
                   uint32_t bits)
{
    int nbit = (bits + bs->pos) - 32;

    if (nbit > 0)
    {
        return ((bs->bufa & (0xffffffff >> bs->pos)) << nbit) | (bs->bufb >> (32 - nbit));
    }
    else
    {
        return (bs->bufa & (0xffffffff >> bs->pos)) >> (32 - bs->pos - bits);
    }
}


/* skip n bits forward in bitstream */

 /*__inline*/ void
BitstreamSkip(Bitstream *  bs,
               uint32_t bits)
{
    bs->pos += bits;

    if (bs->pos >= 32)
    {
        uint32_t tmp;
        int end_frame;

        bs->bufa = bs->bufb;
        if ( (bs->length <= sizeof(uint32_t)) &&
			 (bs->end_frame != 1) ) // 如果当前stream buf里包含一帧结尾的数据，那这些数据够解完一帧了，不再取新的数据
        {
        	int cbGet = 0;
			
        	/* bitstream里数据不够，需要从文件中读取新的码流 */

            if (bs->length)
                //memcpy(bs->start, bs->tail + 2, bs->length);//把剩下的＜＝4个字节的数据移至buffer的最前面。
                memcpy(bs->start, bs->tail, bs->length);//把剩下的＜＝4个字节的数据移至buffer的最前面。

            cbGet = xvid_bs_read((char *)bs->start + bs->length, XVID_BS_BUF_SIZE, &end_frame);
			bs->end_frame = end_frame;
            if(cbGet == -1)
            {
                cbGet = 0; // -1为非法值，将其修改成0
            }
            else if(cbGet == 0)
            {
            	bs->end_frame = 1; // 数据已经读完，不要再去读了
            }

			//XVID_ASSERT(cbGet != 0); // TODO:如果这些时读不到码流，也就是说这一帧数据读完了，怎么办?

			bs->length += cbGet;
			if(bs->length < sizeof(uint32_t))
				bs->length = sizeof(uint32_t);
			
            //bs->tail = bs->start - 2;
            bs->tail = bs->start;
        }

        //tmp = *((uint32_t *) bs->tail + 2);
        tmp = *bs->tail++;

#if 0
        if (bs->length >= sizeof(uint32_t))
        {
            bs->length -= sizeof(uint32_t);
        }
        else
        {
            bs->length = 0;
        }

#else
        bs->length -= sizeof(uint32_t);

#endif


        BSWAP(tmp);

        bs->bufb = tmp;

        //bs->tail++;

        bs->pos -= 32;
    }
}
void
BitstreamFillBitsBuf(Bitstream *  bs)
{
	int cbGet = 0;

	/* bitstreamà?êy?Y2?1?￡?Dèòa′ó???t?D?áè?D?μ???á÷ */

	if (bs->length)
		memcpy(bs->start, bs->tail, bs->length);

	cbGet = xvid_bs_read((char *)bs->start + bs->length, XVID_BS_BUF_SIZE, &bs->end_frame);

	//bs->end_frame = end_frame;
#if 0
	if (cbGet == -1)
	{
		cbGet = 0; // -1?a·?·¨?μ￡?????DT??3é0
	}
	else if (cbGet == 0)
	{
		bs->end_frame = 1; // êy?Yò??-?áíê￡?2?òa?ùè￥?áá?
	}
#else
    while(cbGet == -1);
#endif

	//XVID_ASSERT(cbGet != 0); // TODO:è?1??aD?ê±?á2?μ???á÷￡?ò2?íê??μ?aò???êy?Y?áíêá?￡????′°ì?

	bs->length += cbGet;

	if (bs->length < sizeof(uint32_t))
		bs->length = sizeof(uint32_t);

	//bs->tail = bs->start - 2;
	bs->tail = bs->start;
}


/* number of bits to next byte alignment */
 /*__inline*/ uint32_t
BitstreamNumBitsToByteAlign(Bitstream *bs)
{
    uint32_t n = (32 - bs->pos) % 8;
    return n == 0 ? 8 : n;
}


/* show nbits from next byte alignment */
 /*__inline*/ uint32_t
BitstreamShowBitsFromByteAlign(Bitstream *bs, int bits)
{
    int bspos = bs->pos + BitstreamNumBitsToByteAlign(bs);
    int nbit = (bits + bspos) - 32;

    if (bspos >= 32)
    {
        return bs->bufb >> (32 - nbit);
    }
    else if (nbit > 0)
    {
        return ((bs->bufa & (0xffffffff >> bspos)) << nbit) | (bs->
                bufb >> (32 -
                         nbit));
    }
    else
    {
        return (bs->bufa & (0xffffffff >> bspos)) >> (32 - bspos - bits);
    }

}



/* move forward to the next byte boundary */

 /*__inline*/ void
BitstreamByteAlign(Bitstream *  bs)
{
    uint32_t remainder = bs->pos % 8;

    if (remainder)
    {
        BitstreamSkip(bs, 8 - remainder);
    }
}

/* move backward to the next byte boundary */
 /*__inline*/ void 
BitstreamByteAlignBackward(Bitstream *  bs)
{
    uint32_t remainder = bs->pos % 8;

    if (remainder)
    {
        bs->pos -= remainder;
    }
}


/* bitstream length (unit bits) */

 uint32_t //__inline
BitstreamPos( Bitstream *  bs) 
{
    //return((uint32_t)(8*((ptr_t)bs->tail - (ptr_t)bs->start) + bs->pos - bs->initpos));
    return 0; // 由于修改了bitstream流程，此函数已经无效, 另外些函数也不需要了
}


/*
 * flush the bitstream & return length (unit bytes)
 * NOTE: assumes no futher bitstream functions will be called.
 */
#if 0
 uint32_t //__inline
BitstreamLength(Bitstream *  bs)
{
    uint32_t len = (uint32_t)((ptr_t)bs->tail - (ptr_t)bs->start);

    if (bs->pos)
    {
        uint32_t b = bs->buf;

#ifndef ARCH_IS_BIG_ENDIAN
        BSWAP(b);
#endif
        *bs->tail = b;

        len += (bs->pos + 7) / 8;
    }

    /* initpos is always on a byte boundary */
    if (bs->initpos)
        len -= bs->initpos / 8;

    return len;
}

/* move bitstream position forward by n bits and write out buffer if needed */

 void //__inline
BitstreamForward(Bitstream *  bs,
                  uint32_t bits)
{
    bs->pos += bits;

    if (bs->pos >= 32)
    {
        uint32_t b = bs->buf;

#ifndef ARCH_IS_BIG_ENDIAN
        BSWAP(b);
#endif
        *bs->tail++ = b;
        bs->buf = 0;
        bs->pos -= 32;
    }
}
#endif

/* read n bits from bitstream */

 uint32_t //__inline
BitstreamGetBits(Bitstream *  bs,
                  uint32_t n)
{
    uint32_t ret = BitstreamShowBits(bs, n);

    BitstreamSkip(bs, n);
    return ret;
}


/* read single bit from bitstream */

 uint32_t //__inline
BitstreamGetBit(Bitstream *  bs)
{
    return BitstreamGetBits(bs, 1);
}

#if 0
/* write single bit to bitstream */

 void //__inline
BitstreamPutBit(Bitstream *  bs,
                 uint32_t bit)
{
    if (bit)
        bs->buf |= (0x80000000 >> bs->pos);

    BitstreamForward(bs, 1);
}


/* write n bits to bitstream */

 void //__inline
BitstreamPutBits(Bitstream *  bs,
                  uint32_t value,
                  uint32_t size)
{
    uint32_t shift = 32 - bs->pos - size;

    if (shift <= 32)
    {
        bs->buf |= value << shift;
        BitstreamForward(bs, size);
    }
    else
    {
        uint32_t remainder;

        shift = size - (32 - bs->pos);
        bs->buf |= value >> shift;
        BitstreamForward(bs, size - shift);
        remainder = shift;

        shift = 32 - shift;

        bs->buf |= value << shift;
        BitstreamForward(bs, remainder);
    }
}

 int stuffing_codes[8] =
{
    /* nbits     stuffing code */
    0,  /* 1          0 */
    1,  /* 2          01 */
    3,  /* 3          011 */
    7,  /* 4          0111 */
    0xf, /* 5          01111 */
    0x1f, /* 6          011111 */
    0x3f,   /* 7          0111111 */
    0x7f, /* 8          01111111 */
};

/* pad bitstream to the next byte boundary */

 void //__inline
BitstreamPad(Bitstream *  bs)
{
    int bits = 8 - (bs->pos % 8);

    if (bits < 8)
        BitstreamPutBits(bs, stuffing_codes[bits - 1], bits);
}


/*
 * pad bitstream to the next byte boundary
 * alway pad: even if currently at the byte boundary
 */

 void //__inline
BitstreamPadAlways(Bitstream *  bs)
{
    int bits = 8 - (bs->pos % 8);
    BitstreamPutBits(bs, stuffing_codes[bits - 1], bits);
}
#endif

 uint8_t log2_tab_16[16] =  { 0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4 };

 uint32_t log2bin(uint32_t value)
{
    int n = 0;

    if (value & 0xffff0000)
    {
        value >>= 16;
        n += 16;
    }

    if (value & 0xff00)
    {
        value >>= 8;
        n += 8;
    }

    if (value & 0xf0)
    {
        value >>= 4;
        n += 4;
    }

    return n + log2_tab_16[value];
}

 uint32_t intra_dc_threshold_table[] =
{
    32,       /* never use */
    13,
    15,
    17,
    19,
    21,
    23,
    1,
};


 void
bs_get_matrix(Bitstream * bs,
              uint8_t * matrix)
{
    int i = 0;
    int last, value = 0;

    do
    {
        last = value;
        value = BitstreamGetBits(bs, 8);
        matrix[scan_tables[0][i++]] = value;
    }
    while (value != 0 && i < 64);

    if (value != 0) return;

    i--;

    while (i < 64)
    {
        matrix[scan_tables[0][i++]] = last;
    }
}

#if 0

/*
 * for PVOP addbits == fcode - 1
 * for BVOP addbits == max(fcode,bcode) - 1
 * returns mbpos
 */
int
read_video_packet_header(Bitstream *bs,
                         DECODER * dec,
                           int addbits,
                         int * quant,
                         int * fcode_forward,
                         int  * fcode_backward,
                         int * intra_dc_threshold)
{
    int startcode_bits = NUMBITS_VP_RESYNC_MARKER + addbits;
    int mbnum_bits = log2bin(dec->mb_width *  dec->mb_height - 1);
    int mbnum;
    int hec = 0;

    BitstreamSkip(bs, BitstreamNumBitsToByteAlign(bs));
    BitstreamSkip(bs, startcode_bits);

    DPRINTF(XVID_DEBUG_STARTCODE, "<video_packet_header>\n");

    if (dec->shape != VIDOBJLAY_SHAPE_RECTANGULAR)
    {
        hec = BitstreamGetBit(bs);  /* header_extension_code */

        if (hec && !(dec->sprite_enable == SPRITE_STATIC /* && current_coding_type = I_VOP */))
        {
            BitstreamSkip(bs, 13);   /* vop_width */
            READ_MARKER();
            BitstreamSkip(bs, 13);   /* vop_height */
            READ_MARKER();
            BitstreamSkip(bs, 13);   /* vop_horizontal_mc_spatial_ref */
            READ_MARKER();
            BitstreamSkip(bs, 13);   /* vop_vertical_mc_spatial_ref */
            READ_MARKER();
        }
    }

    mbnum = BitstreamGetBits(bs, mbnum_bits);  /* macroblock_number */

    DPRINTF(XVID_DEBUG_HEADER, "mbnum %i\n", mbnum);

    if (dec->shape != VIDOBJLAY_SHAPE_BINARY_ONLY)
    {
        *quant = BitstreamGetBits(bs, dec->quant_bits); /* quant_scale */
        DPRINTF(XVID_DEBUG_HEADER, "quant %i\n", *quant);
    }

    if (dec->shape == VIDOBJLAY_SHAPE_RECTANGULAR)
        hec = BitstreamGetBit(bs);  /* header_extension_code */


    DPRINTF(XVID_DEBUG_HEADER, "header_extension_code %i\n", hec);

    if (hec)
    {
        int time_base;
        int time_increment;
        int coding_type;

        for (time_base = 0; BitstreamGetBit(bs) != 0; time_base++);  /* modulo_time_base */

        READ_MARKER();

        if (dec->time_inc_bits)
            time_increment = (BitstreamGetBits(bs, dec->time_inc_bits)); /* vop_time_increment */

        READ_MARKER();

        DPRINTF(XVID_DEBUG_HEADER, "time %i:%i\n", time_base, time_increment);

        coding_type = BitstreamGetBits(bs, 2);

        DPRINTF(XVID_DEBUG_HEADER, "coding_type %i\n", coding_type);

        if (dec->shape != VIDOBJLAY_SHAPE_RECTANGULAR)
        {
            BitstreamSkip(bs, 1); /* change_conv_ratio_disable */

            if (coding_type != I_VOP)
                BitstreamSkip(bs, 1); /* vop_shape_coding_type */
        }

        if (dec->shape != VIDOBJLAY_SHAPE_BINARY_ONLY)
        {
            *intra_dc_threshold = intra_dc_threshold_table[BitstreamGetBits(bs, 3)];

            if (dec->sprite_enable == SPRITE_GMC && coding_type == S_VOP &&
                    dec->sprite_warping_points > 0)
            {
                /* TODO: sprite trajectory */
            }

            if (dec->reduced_resolution_enable &&
                    dec->shape == VIDOBJLAY_SHAPE_RECTANGULAR &&
                    (coding_type == P_VOP || coding_type == I_VOP))
            {
                BitstreamSkip(bs, 1); /* XXX: vop_reduced_resolution */
            }

            if (coding_type != I_VOP && fcode_forward)
            {
                *fcode_forward = BitstreamGetBits(bs, 3);
                DPRINTF(XVID_DEBUG_HEADER, "fcode_forward %i\n", *fcode_forward);
            }

            if (coding_type == B_VOP && fcode_backward)
            {
                *fcode_backward = BitstreamGetBits(bs, 3);
                DPRINTF(XVID_DEBUG_HEADER, "fcode_backward %i\n", *fcode_backward);
            }
        }
    }

    if (dec->newpred_enable)
    {
        int vop_id;
        int vop_id_for_prediction;

        vop_id = BitstreamGetBits(bs, MIN(dec->time_inc_bits + 3, 15));
        DPRINTF(XVID_DEBUG_HEADER, "vop_id %i\n", vop_id);

        if (BitstreamGetBit(bs)) /* vop_id_for_prediction_indication */
        {
            vop_id_for_prediction = BitstreamGetBits(bs, MIN(dec->time_inc_bits + 3, 15));
            DPRINTF(XVID_DEBUG_HEADER, "vop_id_for_prediction %i\n", vop_id_for_prediction);
        }

        READ_MARKER();
    }

    return mbnum;
}

#endif

#if 0
/* vol estimation header */
 void
read_vol_complexity_estimation_header(Bitstream * bs, DECODER * dec)
{
    ESTIMATION * e = &dec->estimation;

    e->method = BitstreamGetBits(bs, 2); /* estimation_method */
    DPRINTF(XVID_DEBUG_HEADER, "+ complexity_estimation_header; method=%i\n", e->method);

    if (e->method == 0 || e->method == 1)
    {
        if (!BitstreamGetBit(bs))  /* shape_complexity_estimation_disable */
        {
            e->opaque = BitstreamGetBit(bs);  /* opaque */
            e->transparent = BitstreamGetBit(bs);  /* transparent */
            e->intra_cae = BitstreamGetBit(bs);  /* intra_cae */
            e->inter_cae = BitstreamGetBit(bs);  /* inter_cae */
            e->no_update = BitstreamGetBit(bs);  /* no_update */
            e->upsampling = BitstreamGetBit(bs);  /* upsampling */
        }

        if (!BitstreamGetBit(bs)) /* texture_complexity_estimation_set_1_disable */
        {
            e->intra_blocks = BitstreamGetBit(bs);  /* intra_blocks */
            e->inter_blocks = BitstreamGetBit(bs);  /* inter_blocks */
            e->inter4v_blocks = BitstreamGetBit(bs);  /* inter4v_blocks */
            e->not_coded_blocks = BitstreamGetBit(bs);  /* not_coded_blocks */
        }
    }

    READ_MARKER();

    if (!BitstreamGetBit(bs))  /* texture_complexity_estimation_set_2_disable */
    {
        e->dct_coefs = BitstreamGetBit(bs);  /* dct_coefs */
        e->dct_lines = BitstreamGetBit(bs);  /* dct_lines */
        e->vlc_symbols = BitstreamGetBit(bs);  /* vlc_symbols */
        e->vlc_bits = BitstreamGetBit(bs);  /* vlc_bits */
    }

    if (!BitstreamGetBit(bs))  /* motion_compensation_complexity_disable */
    {
        e->apm = BitstreamGetBit(bs);  /* apm */
        e->npm = BitstreamGetBit(bs);  /* npm */
        e->interpolate_mc_q = BitstreamGetBit(bs);  /* interpolate_mc_q */
        e->forw_back_mc_q = BitstreamGetBit(bs);  /* forw_back_mc_q */
        e->halfpel2 = BitstreamGetBit(bs);  /* halfpel2 */
        e->halfpel4 = BitstreamGetBit(bs);  /* halfpel4 */
    }

    READ_MARKER();

    if (e->method == 1)
    {
        if (!BitstreamGetBit(bs)) /* version2_complexity_estimation_disable */
        {
            e->sadct = BitstreamGetBit(bs);  /* sadct */
            e->quarterpel = BitstreamGetBit(bs);  /* quarterpel */
        }
    }
}

/* vop estimation header */
 void
read_vop_complexity_estimation_header(Bitstream * bs, DECODER * dec, int coding_type)
{
    ESTIMATION * e = &dec->estimation;

    if (e->method == 0 || e->method == 1)
    {
        if (coding_type == I_VOP)
        {
            if (e->opaque)  BitstreamSkip(bs, 8); /* dcecs_opaque */

            if (e->transparent) BitstreamSkip(bs, 8); /* */

            if (e->intra_cae) BitstreamSkip(bs, 8); /* */

            if (e->inter_cae) BitstreamSkip(bs, 8); /* */

            if (e->no_update) BitstreamSkip(bs, 8); /* */

            if (e->upsampling) BitstreamSkip(bs, 8); /* */

            if (e->intra_blocks) BitstreamSkip(bs, 8); /* */

            if (e->not_coded_blocks) BitstreamSkip(bs, 8); /* */

            if (e->dct_coefs) BitstreamSkip(bs, 8); /* */

            if (e->dct_lines) BitstreamSkip(bs, 8); /* */

            if (e->vlc_symbols) BitstreamSkip(bs, 8); /* */

            if (e->vlc_bits) BitstreamSkip(bs, 8); /* */

            if (e->sadct)  BitstreamSkip(bs, 8); /* */
        }

        if (coding_type == P_VOP)
        {
            if (e->opaque) BitstreamSkip(bs, 8);  /* */

            if (e->transparent) BitstreamSkip(bs, 8); /* */

            if (e->intra_cae) BitstreamSkip(bs, 8); /* */

            if (e->inter_cae) BitstreamSkip(bs, 8); /* */

            if (e->no_update) BitstreamSkip(bs, 8); /* */

            if (e->upsampling) BitstreamSkip(bs, 8); /* */

            if (e->intra_blocks) BitstreamSkip(bs, 8); /* */

            if (e->not_coded_blocks) BitstreamSkip(bs, 8); /* */

            if (e->dct_coefs) BitstreamSkip(bs, 8); /* */

            if (e->dct_lines) BitstreamSkip(bs, 8); /* */

            if (e->vlc_symbols) BitstreamSkip(bs, 8); /* */

            if (e->vlc_bits) BitstreamSkip(bs, 8); /* */

            if (e->inter_blocks) BitstreamSkip(bs, 8); /* */

            if (e->inter4v_blocks) BitstreamSkip(bs, 8); /* */

            if (e->apm)   BitstreamSkip(bs, 8); /* */

            if (e->npm)   BitstreamSkip(bs, 8); /* */

            if (e->forw_back_mc_q) BitstreamSkip(bs, 8); /* */

            if (e->halfpel2) BitstreamSkip(bs, 8); /* */

            if (e->halfpel4) BitstreamSkip(bs, 8); /* */

            if (e->sadct)  BitstreamSkip(bs, 8); /* */

            if (e->quarterpel) BitstreamSkip(bs, 8); /* */
        }

        if (coding_type == B_VOP)
        {
            if (e->opaque)  BitstreamSkip(bs, 8); /* */

            if (e->transparent) BitstreamSkip(bs, 8); /* */

            if (e->intra_cae) BitstreamSkip(bs, 8); /* */

            if (e->inter_cae) BitstreamSkip(bs, 8); /* */

            if (e->no_update) BitstreamSkip(bs, 8); /* */

            if (e->upsampling) BitstreamSkip(bs, 8); /* */

            if (e->intra_blocks) BitstreamSkip(bs, 8); /* */

            if (e->not_coded_blocks) BitstreamSkip(bs, 8); /* */

            if (e->dct_coefs) BitstreamSkip(bs, 8); /* */

            if (e->dct_lines) BitstreamSkip(bs, 8); /* */

            if (e->vlc_symbols) BitstreamSkip(bs, 8); /* */

            if (e->vlc_bits) BitstreamSkip(bs, 8); /* */

            if (e->inter_blocks) BitstreamSkip(bs, 8); /* */

            if (e->inter4v_blocks) BitstreamSkip(bs, 8); /* */

            if (e->apm)   BitstreamSkip(bs, 8); /* */

            if (e->npm)   BitstreamSkip(bs, 8); /* */

            if (e->forw_back_mc_q) BitstreamSkip(bs, 8); /* */

            if (e->halfpel2) BitstreamSkip(bs, 8); /* */

            if (e->halfpel4) BitstreamSkip(bs, 8); /* */

            if (e->interpolate_mc_q) BitstreamSkip(bs, 8); /* */

            if (e->sadct)  BitstreamSkip(bs, 8); /* */

            if (e->quarterpel) BitstreamSkip(bs, 8); /* */
        }

        if (coding_type == S_VOP && dec->sprite_enable == SPRITE_STATIC)
        {
            if (e->intra_blocks) BitstreamSkip(bs, 8); /* */

            if (e->not_coded_blocks) BitstreamSkip(bs, 8); /* */

            if (e->dct_coefs) BitstreamSkip(bs, 8); /* */

            if (e->dct_lines) BitstreamSkip(bs, 8); /* */

            if (e->vlc_symbols) BitstreamSkip(bs, 8); /* */

            if (e->vlc_bits) BitstreamSkip(bs, 8); /* */

            if (e->inter_blocks) BitstreamSkip(bs, 8); /* */

            if (e->inter4v_blocks) BitstreamSkip(bs, 8); /* */

            if (e->apm)   BitstreamSkip(bs, 8); /* */

            if (e->npm)   BitstreamSkip(bs, 8); /* */

            if (e->forw_back_mc_q) BitstreamSkip(bs, 8); /* */

            if (e->halfpel2) BitstreamSkip(bs, 8); /* */

            if (e->halfpel4) BitstreamSkip(bs, 8); /* */

            if (e->interpolate_mc_q) BitstreamSkip(bs, 8); /* */
        }
    }
}
#endif




/*
decode headers
returns coding_type, or -1 if error
*/

#define VIDOBJ_START_CODE_MASK  0x0000001f
#define VIDOBJLAY_START_CODE_MASK 0x0000000f

int
BitstreamReadHeaders(Bitstream * bs,
                     DECODER * dec,
                     uint32_t * rounding,
                     uint32_t * quant,
                     uint32_t * fcode_forward,
                     uint32_t * fcode_backward,
                     uint32_t * intra_dc_threshold,
                     WARPPOINTS *gmc_warp)
{
    uint32_t vol_ver_id;
    uint32_t coding_type;
    uint32_t start_code;
    uint32_t time_incr = 0;
    int32_t time_increment = 0;
    int resize = 0;

    //while ((BitstreamPos(bs) >> 3) + 4 <= bs->length)
    while(bs->length > 0)
    {

        BitstreamByteAlign(bs);
        start_code = BitstreamShowBits(bs, 32);

        if (start_code == VISOBJSEQ_START_CODE)
        {

            int profile;

            DPRINTF(XVID_DEBUG_STARTCODE, "<visual_object_sequence>\n");

            BitstreamSkip(bs, 32); /* visual_object_sequence_start_code */
            profile = BitstreamGetBits(bs, 8); /* profile_and_level_indication */

            DPRINTF(XVID_DEBUG_HEADER, "profile_and_level_indication %i\n", profile);

        }
        else if (start_code == VISOBJSEQ_STOP_CODE)
        {

            BitstreamSkip(bs, 32); /* visual_object_sequence_stop_code */

            DPRINTF(XVID_DEBUG_STARTCODE, "</visual_object_sequence>\n");

        }
        else if (start_code == VISOBJ_START_CODE)
        {
            DPRINTF(XVID_DEBUG_STARTCODE, "<visual_object>\n");

            BitstreamSkip(bs, 32); /* visual_object_start_code */

            if (BitstreamGetBit(bs)) /* is_visual_object_identified */
            {
                dec->ver_id = BitstreamGetBits(bs, 4); /* visual_object_ver_id */
                DPRINTF(XVID_DEBUG_HEADER, "visobj_ver_id %i\n", dec->ver_id);
                BitstreamSkip(bs, 3); /* visual_object_priority */
            }
            else
            {
                dec->ver_id = 1;
            }

            if (BitstreamShowBits(bs, 4) != VISOBJ_TYPE_VIDEO) /* visual_object_type */
            {
                DPRINTF(XVID_DEBUG_ERROR, "visual_object_type != video\n");
                return -1;
            }

            BitstreamSkip(bs, 4);

            /* video_signal_type */

            if (BitstreamGetBit(bs)) /* video_signal_type */
            {
                DPRINTF(XVID_DEBUG_HEADER, "+ video_signal_type\n");
                BitstreamSkip(bs, 3); /* video_format */
                BitstreamSkip(bs, 1); /* video_range */

                if (BitstreamGetBit(bs)) /* color_description */
                {
                    DPRINTF(XVID_DEBUG_HEADER, "+ color_description");
                    BitstreamSkip(bs, 8); /* color_primaries */
                    BitstreamSkip(bs, 8); /* transfer_characteristics */
                    BitstreamSkip(bs, 8); /* matrix_coefficients */
                }
            }
        }
        else if ((start_code & ~VIDOBJ_START_CODE_MASK) == VIDOBJ_START_CODE)
        {

            DPRINTF(XVID_DEBUG_STARTCODE, "<video_object>\n");
            DPRINTF(XVID_DEBUG_HEADER, "vo id %i\n", start_code & VIDOBJ_START_CODE_MASK);

            BitstreamSkip(bs, 32); /* video_object_start_code */

        }
        else if ((start_code & ~VIDOBJLAY_START_CODE_MASK) == VIDOBJLAY_START_CODE)
        {

            DPRINTF(XVID_DEBUG_STARTCODE, "<video_object_layer>\n");
            DPRINTF(XVID_DEBUG_HEADER, "vol id %i\n", start_code & VIDOBJLAY_START_CODE_MASK);

            BitstreamSkip(bs, 32); /* video_object_layer_start_code */
            BitstreamSkip(bs, 1); /* random_accessible_vol */

            BitstreamSkip(bs, 8);   /* video_object_type_indication */

            if (BitstreamGetBit(bs)) /* is_object_layer_identifier */
            {
                DPRINTF(XVID_DEBUG_HEADER, "+ is_object_layer_identifier\n");
                vol_ver_id = BitstreamGetBits(bs, 4); /* video_object_layer_verid */
                DPRINTF(XVID_DEBUG_HEADER, "ver_id %i\n", vol_ver_id);
                BitstreamSkip(bs, 3); /* video_object_layer_priority */
            }
            else
            {
                vol_ver_id = dec->ver_id;
            }

            dec->aspect_ratio = BitstreamGetBits(bs, 4);

            if (dec->aspect_ratio == VIDOBJLAY_AR_EXTPAR) /* aspect_ratio_info */
            {
                DPRINTF(XVID_DEBUG_HEADER, "+ aspect_ratio_info\n");
                dec->par_width = BitstreamGetBits(bs, 8); /* par_width */
                dec->par_height = BitstreamGetBits(bs, 8); /* par_height */
            }

            if (BitstreamGetBit(bs)) /* vol_control_parameters */
            {
                DPRINTF(XVID_DEBUG_HEADER, "+ vol_control_parameters\n");
                BitstreamSkip(bs, 2); /* chroma_format */
                dec->low_delay = BitstreamGetBit(bs); /* low_delay */
                DPRINTF(XVID_DEBUG_HEADER, "low_delay %i\n", dec->low_delay);

                if (BitstreamGetBit(bs)) /* vbv_parameters */
                {
                    unsigned int bitrate;
                    unsigned int buffer_size;
                    unsigned int occupancy;

                    DPRINTF(XVID_DEBUG_HEADER, "+ vbv_parameters\n");

                    bitrate = BitstreamGetBits(bs, 15) << 15; /* first_half_bit_rate */
                    READ_MARKER();
                    bitrate |= BitstreamGetBits(bs, 15); /* latter_half_bit_rate */
                    READ_MARKER();

                    buffer_size = BitstreamGetBits(bs, 15) << 3; /* first_half_vbv_buffer_size */
                    READ_MARKER();
                    buffer_size |= BitstreamGetBits(bs, 3);  /* latter_half_vbv_buffer_size */

                    occupancy = BitstreamGetBits(bs, 11) << 15; /* first_half_vbv_occupancy */
                    READ_MARKER();
                    occupancy |= BitstreamGetBits(bs, 15); /* latter_half_vbv_occupancy */
                    READ_MARKER();

                    DPRINTF(XVID_DEBUG_HEADER, "bitrate %d (unit=400 bps)\n", bitrate);
                    DPRINTF(XVID_DEBUG_HEADER, "buffer_size %d (unit=16384 bits)\n", buffer_size);
                    DPRINTF(XVID_DEBUG_HEADER, "occupancy %d (unit=64 bits)\n", occupancy);
                }
            }
            else
            {
                dec->low_delay = dec->low_delay_default;
            }

            dec->shape = BitstreamGetBits(bs, 2); /* video_object_layer_shape */

            DPRINTF(XVID_DEBUG_HEADER, "shape %i\n", dec->shape);

            if (dec->shape != VIDOBJLAY_SHAPE_RECTANGULAR)
            {
                DPRINTF(XVID_DEBUG_ERROR, "non-rectangular shapes are not supported\n");
            }

            if (dec->shape == VIDOBJLAY_SHAPE_GRAYSCALE && vol_ver_id != 1)
            {
                BitstreamSkip(bs, 4); /* video_object_layer_shape_extension */
            }

            READ_MARKER();

            /********************** for decode B-frame time ***********************/
            dec->time_inc_resolution = BitstreamGetBits(bs, 16); /* vop_time_increment_resolution */
            DPRINTF(XVID_DEBUG_HEADER, "vop_time_increment_resolution %i\n", dec->time_inc_resolution);

            if (dec->time_inc_resolution > 0)
            {
                dec->time_inc_bits = MAX(log2bin(dec->time_inc_resolution - 1), 1);
            }
            else
            {
                /* for "old" xvid compatibility, set time_inc_bits = 1 */
                dec->time_inc_bits = 1;
            }

            READ_MARKER();

            if (BitstreamGetBit(bs)) /* fixed_vop_rate */
            {
                DPRINTF(XVID_DEBUG_HEADER, "+ fixed_vop_rate\n");
                BitstreamSkip(bs, dec->time_inc_bits); /* fixed_vop_time_increment */
            }

            if (dec->shape != VIDOBJLAY_SHAPE_BINARY_ONLY)
            {

                if (dec->shape == VIDOBJLAY_SHAPE_RECTANGULAR)
                {
                    uint32_t width, height;

                    READ_MARKER();
                    width = BitstreamGetBits(bs, 13); /* video_object_layer_width */
                    READ_MARKER();
                    height = BitstreamGetBits(bs, 13); /* video_object_layer_height */
                    READ_MARKER();

                    DPRINTF(XVID_DEBUG_HEADER, "width %i\n", width);
                    DPRINTF(XVID_DEBUG_HEADER, "height %i\n", height);

                    if (dec->width != width || dec->height != height)
                    {
                        if (dec->fixed_dimensions)
                        {
                            DPRINTF(XVID_DEBUG_ERROR, "decoder width/height does not match bitstream\n");
                            return -1;
                        }

                        resize = 1;

                        dec->width = width;
                        dec->height = height;
                    }
                }

                dec->interlacing = BitstreamGetBit(bs);
				if(dec->interlacing)
				{
					XVID_ASSERT(0);
				}

                DPRINTF(XVID_DEBUG_HEADER, "interlacing %i\n", dec->interlacing);

                if (!BitstreamGetBit(bs)) /* obmc_disable */
                {
                    DPRINTF(XVID_DEBUG_ERROR, "obmc_disabled==false not supported\n");
                    /* TODO */
                    /* fucking divx4.02 has this enabled */
                }

                dec->sprite_enable = BitstreamGetBits(bs, (vol_ver_id == 1 ? 1 : 2)); /* sprite_enable */

                if (dec->sprite_enable == SPRITE_STATIC || dec->sprite_enable == SPRITE_GMC)
                {
                    int low_latency_sprite_enable;

                    if (dec->sprite_enable != SPRITE_GMC)
                    {
                        int sprite_width;
                        int sprite_height;
                        int sprite_left_coord;
                        int sprite_top_coord;
                        sprite_width = BitstreamGetBits(bs, 13);  /* sprite_width */
                        READ_MARKER();
                        sprite_height = BitstreamGetBits(bs, 13); /* sprite_height */
                        READ_MARKER();
                        sprite_left_coord = BitstreamGetBits(bs, 13); /* sprite_left_coordinate */
                        READ_MARKER();
                        sprite_top_coord = BitstreamGetBits(bs, 13); /* sprite_top_coordinate */
                        READ_MARKER();
                    }

                    dec->sprite_warping_points = BitstreamGetBits(bs, 6);  /* no_of_sprite_warping_points */

                    dec->sprite_warping_accuracy = BitstreamGetBits(bs, 2);  /* sprite_warping_accuracy */
                    dec->sprite_brightness_change = BitstreamGetBits(bs, 1);  /* brightness_change */

                    if (dec->sprite_enable != SPRITE_GMC)
                    {
                        low_latency_sprite_enable = BitstreamGetBits(bs, 1);  /* low_latency_sprite_enable */
                    }
                }

                if (vol_ver_id != 1 &&
                        dec->shape != VIDOBJLAY_SHAPE_RECTANGULAR)
                {
                    BitstreamSkip(bs, 1); /* sadct_disable */
                }

                if (BitstreamGetBit(bs)) /* not_8_bit */
                {
                    DPRINTF(XVID_DEBUG_HEADER, "not_8_bit==true (ignored)\n");
                    dec->quant_bits = BitstreamGetBits(bs, 4); /* quant_precision */
                    BitstreamSkip(bs, 4); /* bits_per_pixel */
                }
                else
                {
                    dec->quant_bits = 5;
                }

                if (dec->shape == VIDOBJLAY_SHAPE_GRAYSCALE)
                {
                    BitstreamSkip(bs, 1); /* no_gray_quant_update */
                    BitstreamSkip(bs, 1); /* composition_method */
                    BitstreamSkip(bs, 1); /* linear_composition */
                }

                dec->quant_type = BitstreamGetBit(bs); /* quant_type */

                DPRINTF(XVID_DEBUG_HEADER, "quant_type %i\n", dec->quant_type);

                if (dec->quant_type)
                {
                	XVID_ASSERT(0);
//                    if (BitstreamGetBit(bs)) /* load_intra_quant_mat */
//                    {
//                        uint8_t matrix[64];
//
//                        DPRINTF(XVID_DEBUG_HEADER, "load_intra_quant_mat\n");
//
//                        bs_get_matrix(bs, matrix);
//                        set_intra_matrix(dec->mpeg_quant_matrices, matrix);
//                    }
//                    else
//                        set_intra_matrix(dec->mpeg_quant_matrices, get_default_intra_matrix());
//
//                    if (BitstreamGetBit(bs)) /* load_inter_quant_mat */
//                    {
//                        uint8_t matrix[64];
//
//                        DPRINTF(XVID_DEBUG_HEADER, "load_inter_quant_mat\n");
//
//                        bs_get_matrix(bs, matrix);
//                        set_inter_matrix(dec->mpeg_quant_matrices, matrix);
//                    }
//                    else
//                        set_inter_matrix(dec->mpeg_quant_matrices, get_default_inter_matrix());
//
//                    if (dec->shape == VIDOBJLAY_SHAPE_GRAYSCALE)
//                    {
//                        DPRINTF(XVID_DEBUG_ERROR, "greyscale matrix not supported\n");
//                        return -1;
//                    }

                }


                if (vol_ver_id != 1)
                {
                    dec->quarterpel = BitstreamGetBit(bs); /* quarter_sample */
                    DPRINTF(XVID_DEBUG_HEADER, "quarterpel %i\n", dec->quarterpel);
                }
                else
                    dec->quarterpel = 0;


                dec->complexity_estimation_disable = BitstreamGetBit(bs); /* complexity estimation disable */

                if (!dec->complexity_estimation_disable)
                {
                	XVID_ASSERT(0);
                    //read_vol_complexity_estimation_header(bs, dec);
                }

                BitstreamSkip(bs, 1); /* resync_marker_disable */

                if (BitstreamGetBit(bs)) /* data_partitioned */
                {
                    DPRINTF(XVID_DEBUG_ERROR, "data_partitioned not supported\n");
                    BitstreamSkip(bs, 1); /* reversible_vlc */
                }

                if (vol_ver_id != 1)
                {
                    dec->newpred_enable = BitstreamGetBit(bs);

                    if (dec->newpred_enable) /* newpred_enable */
                    {
                        DPRINTF(XVID_DEBUG_HEADER, "+ newpred_enable\n");
                        BitstreamSkip(bs, 2); /* requested_upstream_message_type */
                        BitstreamSkip(bs, 1); /* newpred_segment_type */
                    }

                    dec->reduced_resolution_enable = BitstreamGetBit(bs); /* reduced_resolution_vop_enable */

                    DPRINTF(XVID_DEBUG_HEADER, "reduced_resolution_enable %i\n", dec->reduced_resolution_enable);
                }
                else
                {
                    dec->newpred_enable = 0;
                    dec->reduced_resolution_enable = 0;
                }

                dec->scalability = BitstreamGetBit(bs); /* scalability */

                if (dec->scalability)
                {
                    DPRINTF(XVID_DEBUG_ERROR, "scalability not supported\n");
                    BitstreamSkip(bs, 1); /* hierarchy_type */
                    BitstreamSkip(bs, 4); /* ref_layer_id */
                    BitstreamSkip(bs, 1); /* ref_layer_sampling_direc */
                    BitstreamSkip(bs, 5); /* hor_sampling_factor_n */
                    BitstreamSkip(bs, 5); /* hor_sampling_factor_m */
                    BitstreamSkip(bs, 5); /* vert_sampling_factor_n */
                    BitstreamSkip(bs, 5); /* vert_sampling_factor_m */
                    BitstreamSkip(bs, 1); /* enhancement_type */

                    if (dec->shape == VIDOBJLAY_SHAPE_BINARY /* && hierarchy_type==0 */)
                    {
                        BitstreamSkip(bs, 1); /* use_ref_shape */
                        BitstreamSkip(bs, 1); /* use_ref_texture */
                        BitstreamSkip(bs, 5); /* shape_hor_sampling_factor_n */
                        BitstreamSkip(bs, 5); /* shape_hor_sampling_factor_m */
                        BitstreamSkip(bs, 5); /* shape_vert_sampling_factor_n */
                        BitstreamSkip(bs, 5); /* shape_vert_sampling_factor_m */
                    }

                    return -1;
                }
            }
            else    /* dec->shape == BINARY_ONLY */
            {
                if (vol_ver_id != 1)
                {
                    dec->scalability = BitstreamGetBit(bs); /* scalability */

                    if (dec->scalability)
                    {
                        DPRINTF(XVID_DEBUG_ERROR, "scalability not supported\n");
                        BitstreamSkip(bs, 4); /* ref_layer_id */
                        BitstreamSkip(bs, 5); /* hor_sampling_factor_n */
                        BitstreamSkip(bs, 5); /* hor_sampling_factor_m */
                        BitstreamSkip(bs, 5); /* vert_sampling_factor_n */
                        BitstreamSkip(bs, 5); /* vert_sampling_factor_m */
                        return -1;
                    }
                }

                BitstreamSkip(bs, 1); /* resync_marker_disable */

            }

            return (resize ? -3 : -2 ); /* VOL */

        }
        else if (start_code == GRPOFVOP_START_CODE)
        {

            DPRINTF(XVID_DEBUG_STARTCODE, "<group_of_vop>\n");

            BitstreamSkip(bs, 32);
            {
                int hours, minutes, seconds;

                hours = BitstreamGetBits(bs, 5);
                minutes = BitstreamGetBits(bs, 6);
                READ_MARKER();
                seconds = BitstreamGetBits(bs, 6);

                DPRINTF(XVID_DEBUG_HEADER, "time %ih%im%is\n", hours, minutes, seconds);
            }

            BitstreamSkip(bs, 1); /* closed_gov */
            BitstreamSkip(bs, 1); /* broken_link */

        }
        else if (start_code == VOP_START_CODE)
        {
            DPRINTF(XVID_DEBUG_STARTCODE, "<vop>\n");

            BitstreamSkip(bs, 32); /* vop_start_code */

            coding_type = BitstreamGetBits(bs, 2); /* vop_coding_type */
            DPRINTF(XVID_DEBUG_HEADER, "coding_type %i\n", coding_type);

            /*********************** for decode B-frame time ***********************/

            while (BitstreamGetBit(bs) != 0) /* time_base */
                time_incr++;

            READ_MARKER();

            if (dec->time_inc_bits)
            {
                time_increment = (BitstreamGetBits(bs, dec->time_inc_bits)); /* vop_time_increment */
            }

            DPRINTF(XVID_DEBUG_HEADER, "time_base %i\n", time_incr);

            DPRINTF(XVID_DEBUG_HEADER, "time_increment %i\n", time_increment);

            DPRINTF(XVID_DEBUG_TIMECODE, "%c %i:%i\n",
                    coding_type == I_VOP ? 'I' : coding_type == P_VOP ? 'P' : coding_type == B_VOP ? 'B' : 'S',
                    time_incr, time_increment);

            if (coding_type != B_VOP)
            {
                dec->last_time_base = dec->time_base;
                dec->time_base += time_incr;
                dec->time = dec->time_base * dec->time_inc_resolution + time_increment;
                dec->time_pp = (int32_t)(dec->time - dec->last_non_b_time);
                dec->last_non_b_time = dec->time;
            }
            else
            {
                dec->time = (dec->last_time_base + time_incr) * dec->time_inc_resolution + time_increment;
                dec->time_bp = dec->time_pp - (int32_t)(dec->last_non_b_time - dec->time);
            }

            DPRINTF(XVID_DEBUG_HEADER, "time_pp=%i\n", dec->time_pp);

            DPRINTF(XVID_DEBUG_HEADER, "time_bp=%i\n", dec->time_bp);

            READ_MARKER();

            if (!BitstreamGetBit(bs)) /* vop_coded */
            {
                DPRINTF(XVID_DEBUG_HEADER, "vop_coded==false\n");
                return N_VOP;
            }

            if (dec->newpred_enable)
            {
                int vop_id;
                int vop_id_for_prediction;

                vop_id = BitstreamGetBits(bs, MIN(dec->time_inc_bits + 3, 15));
                DPRINTF(XVID_DEBUG_HEADER, "vop_id %i\n", vop_id);

                if (BitstreamGetBit(bs)) /* vop_id_for_prediction_indication */
                {
                    vop_id_for_prediction = BitstreamGetBits(bs, MIN(dec->time_inc_bits + 3, 15));
                    DPRINTF(XVID_DEBUG_HEADER, "vop_id_for_prediction %i\n", vop_id_for_prediction);
                }

                READ_MARKER();
            }



            /* fix a little bug by MinChen <chenm002@163.com> */
            if ((dec->shape != VIDOBJLAY_SHAPE_BINARY_ONLY) &&
                    ( (coding_type == P_VOP) || (coding_type == S_VOP && dec->sprite_enable == SPRITE_GMC) ) )
            {
                *rounding = BitstreamGetBit(bs); /* rounding_type */
                DPRINTF(XVID_DEBUG_HEADER, "rounding %i\n", *rounding);
            }

            if (dec->reduced_resolution_enable &&
                    dec->shape == VIDOBJLAY_SHAPE_RECTANGULAR &&
                    (coding_type == P_VOP || coding_type == I_VOP))
            {

                if (BitstreamGetBit(bs));

                DPRINTF(XVID_DEBUG_ERROR, "RRV not supported (anymore)\n");
            }

            if (dec->shape != VIDOBJLAY_SHAPE_RECTANGULAR)
            {
                if (!(dec->sprite_enable == SPRITE_STATIC && coding_type == I_VOP))
                {

                    uint32_t width, height;
                    uint32_t horiz_mc_ref, vert_mc_ref;

                    width = BitstreamGetBits(bs, 13);
                    READ_MARKER();
                    height = BitstreamGetBits(bs, 13);
                    READ_MARKER();
                    horiz_mc_ref = BitstreamGetBits(bs, 13);
                    READ_MARKER();
                    vert_mc_ref = BitstreamGetBits(bs, 13);
                    READ_MARKER();

                    DPRINTF(XVID_DEBUG_HEADER, "width %i\n", width);
                    DPRINTF(XVID_DEBUG_HEADER, "height %i\n", height);
                    DPRINTF(XVID_DEBUG_HEADER, "horiz_mc_ref %i\n", horiz_mc_ref);
                    DPRINTF(XVID_DEBUG_HEADER, "vert_mc_ref %i\n", vert_mc_ref);
                }

                BitstreamSkip(bs, 1); /* change_conv_ratio_disable */

                if (BitstreamGetBit(bs)) /* vop_constant_alpha */
                {
                    BitstreamSkip(bs, 8); /* vop_constant_alpha_value */
                }
            }

            if (dec->shape != VIDOBJLAY_SHAPE_BINARY_ONLY)
            {

                if (!dec->complexity_estimation_disable)
                {
                	XVID_ASSERT(0);
                    //read_vop_complexity_estimation_header(bs, dec, coding_type);
                }

                /* intra_dc_vlc_threshold */
                *intra_dc_threshold =
                    intra_dc_threshold_table[BitstreamGetBits(bs, 3)];

                dec->top_field_first = 0;

                dec->alternate_vertical_scan = 0;

                if (dec->interlacing)
                {
                    dec->top_field_first = BitstreamGetBit(bs);
                    DPRINTF(XVID_DEBUG_HEADER, "interlace top_field_first %i\n", dec->top_field_first);
                    dec->alternate_vertical_scan = BitstreamGetBit(bs);
                    DPRINTF(XVID_DEBUG_HEADER, "interlace alternate_vertical_scan %i\n", dec->alternate_vertical_scan);

                }
            }

            if ((dec->sprite_enable == SPRITE_STATIC || dec->sprite_enable == SPRITE_GMC) && coding_type == S_VOP)
            {
				XVID_ASSERT(0);
//                int i;
//
//                for (i = 0 ; i < dec->sprite_warping_points; i++)
//                {
//                    int length;
//                    int x = 0, y = 0;
//
//                    /* sprite code borowed from ffmpeg; thx Michael Niedermayer <michaelni@gmx.at> */
//                    length = bs_get_spritetrajectory(bs);
//
//                    if (length)
//                    {
//                        x = BitstreamGetBits(bs, length);
//
//                        if ((x >> (length - 1)) == 0) /* if MSB not set it is negative*/
//                            x = - (x ^ ((1 << length) - 1));
//                    }
//
//                    READ_MARKER();
//
//                    length = bs_get_spritetrajectory(bs);
//
//                    if (length)
//                    {
//                        y = BitstreamGetBits(bs, length);
//
//                        if ((y >> (length - 1)) == 0) /* if MSB not set it is negative*/
//                            y = - (y ^ ((1 << length) - 1));
//                    }
//
//                    READ_MARKER();
//
//                    gmc_warp->duv[i].x = x;
//                    gmc_warp->duv[i].y = y;
//
//                    DPRINTF(XVID_DEBUG_HEADER, "sprite_warping_point[%i] xy=(%i,%i)\n", i, x, y);
//                }
//
//                if (dec->sprite_brightness_change)
//                {
//                    /* XXX: brightness_change_factor() */
//                }
//
//                if (dec->sprite_enable == SPRITE_STATIC)
//                {
//                    /* XXX: todo */
//                }

            }

            if ((*quant = BitstreamGetBits(bs, dec->quant_bits)) < 1) /* vop_quant */
                *quant = 1;

            DPRINTF(XVID_DEBUG_HEADER, "quant %i\n", *quant);

            if (coding_type != I_VOP)
            {
                *fcode_forward = BitstreamGetBits(bs, 3); /* fcode_forward */
                DPRINTF(XVID_DEBUG_HEADER, "fcode_forward %i\n", *fcode_forward);
            }

            if (coding_type == B_VOP)
            {
                *fcode_backward = BitstreamGetBits(bs, 3); /* fcode_backward */
                DPRINTF(XVID_DEBUG_HEADER, "fcode_backward %i\n", *fcode_backward);
            }

            if (!dec->scalability)
            {
                if ((dec->shape != VIDOBJLAY_SHAPE_RECTANGULAR) &&
                        (coding_type != I_VOP))
                {
                    BitstreamSkip(bs, 1); /* vop_shape_coding_type */
                }
            }

            return coding_type;

        }
        else if (start_code == USERDATA_START_CODE)
        {
            char tmp[256];
            int i;//, version, build;
//            char packed;

            BitstreamSkip(bs, 32); /* user_data_start_code */

            memset(tmp, 0, 256);
            tmp[0] = BitstreamShowBits(bs, 8);

            for (i = 1; i < 256; i++)
            {
                tmp[i] = (BitstreamShowBits(bs, 16) & 0xFF);

                if (tmp[i] == 0)
                    break;

                BitstreamSkip(bs, 8);
            }

            DPRINTF(XVID_DEBUG_STARTCODE, "<user_data>: %s\n", tmp);
#if 0
            /* read xvid bitstream version */

            if (strncmp(tmp, "XviD", 4) == 0)
            {
                if (tmp[strlen(tmp)-1] == 'C')
                {
                    sscanf(tmp, "XviD%dC", &dec->bs_version);
                    dec->cartoon_mode = 1;
                }
                else
                    sscanf(tmp, "XviD%d", &dec->bs_version);

                DPRINTF(XVID_DEBUG_HEADER, "xvid bitstream version=%i\n", dec->bs_version);
            }

            /* divx detection */
            i = sscanf(tmp, "DivX%dBuild%d%c", &version, &build, &packed);

            if (i < 2)
                i = sscanf(tmp, "DivX%db%d%c", &version, &build, &packed);

            if (i >= 2)
            {
                dec->packed_mode = (i == 3 && packed == 'p');
                DPRINTF(XVID_DEBUG_HEADER, "divx version=%i, build=%i packed=%i\n",
                        version, build, dec->packed_mode);
            }
#endif
        }
        else     /* start_code == ? */
        {
            if (BitstreamShowBits(bs, 24) == 0x000001)
            {
                DPRINTF(XVID_DEBUG_STARTCODE, "<unknown: %x>\n", BitstreamShowBits(bs, 32));
            }

            BitstreamSkip(bs, 8);
        }
    }

#if 0
    DPRINTF("*** WARNING: no vop_start_code found");

#endif
    return -1;     /* ignore it */
}

#ifdef MDK_PLATFORM
#pragma arm section code
#endif

#endif

