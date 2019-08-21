/* libFLAC - Free Lossless Audio Codec library
 * Copyright (C) 2000,2001,2002,2003,2004,2005  Josh Coalson
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * - Neither the name of the Xiph.org Foundation nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "../../include/audio_main.h"

#ifdef A_CORE_DECODE
#ifdef FLAC_DEC_INCLUDE

#include <stdlib.h> /* for flac_malloc() */
#include <string.h> /* for memcpy(), memset() */
#include "bitbuffer.h"
#include "bitmath.h"
#include "crc.h"
#include "assert.h"
#include "replacer.h"
#include "flacbuffer.h"
#include "FLACTab.h"
/*
 * Along the way you will see two versions of some functions, selected
 * by a FLAC__NO_MANUAL_INLINING macro.  One is the simplified, more
 * readable, and slow version, and the other is the same function
 * where crucial parts have been manually inlined and are much faster.
 *
 */

/*
 * Some optimization strategies are slower with older versions of MSVC
 */
//#if defined _MSC_VER && _MSC_VER <= 1200
//#define FLAC__OLD_MSVC_FLAVOR
//#endif

/*
 * This should be at least twice as large as the largest number of blurbs
 * required to represent any 'number' (in any encoding) you are going to
 * read.  With FLAC this is on the order of maybe a few hundred bits.
 * If the buffer is smaller than that, the decoder won't be able to read
 * in a whole number that is in a variable length encoding (e.g. Rice).
 *
 * The number we are actually using here is based on what would be the
 * approximate maximum size of a verbatim frame at the default block size,
 * for CD audio (4096 sample * 4 bytes per sample), plus some wiggle room.
 * 32kbytes sounds reasonable.  For kicks we subtract out 64 bytes for any
 * alignment or flac_malloc overhead.
 *
 * Increase this number to decrease the number of read callbacks, at the
 * expense of using more memory.  Or decrease for the reverse effect,
 * keeping in mind the limit from the first paragraph.
 */
//static const unsigned FLAC__BITBUFFER_DEFAULT_CAPACITY = ((65536 - 64) * 8) / FLAC__BITS_PER_BLURB; /* blurbs */
//static const unsigned FLAC__BITBUFFER_DEFAULT_CAPACITY = 4096*2;
#ifndef FLAC_TABLE_ROOM_VERIFY
#ifndef FLAC__OLD_MSVC_FLAVOR
static const unsigned char byte_to_unary_table[] =
{
    8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
#endif
#endif
#if FLAC__BITS_PER_BLURB == 8
#define FLAC__BITS_PER_BLURB_LOG2 3
#define FLAC__BYTES_PER_BLURB 1
#define FLAC__BLURB_ALL_ONES ((FLAC__byte)0xff)
#define FLAC__BLURB_TOP_BIT_ONE ((FLAC__byte)0x80)
#define BLURB_BIT_TO_MASK(b) (((FLAC__blurb)'\x80') >> (b))
#define CRC16_UPDATE_BLURB(bb, blurb, crc) FLAC__CRC16_UPDATE((blurb), (crc));
#ifndef FLAC__OLD_MSVC_FLAVOR

#ifdef FLAC_TABLE_ROOM_VERIFY
#define FLAC__ALIGNED_BLURB_UNARY(blurb) (*(p_byte_to_unary_table + blurb))
#else
#define FLAC__ALIGNED_BLURB_UNARY(blurb) (byte_to_unary_table[blurb])
#endif

#endif
#elif FLAC__BITS_PER_BLURB == 32
#define FLAC__BITS_PER_BLURB_LOG2 5
#define FLAC__BYTES_PER_BLURB 4
#define FLAC__BLURB_ALL_ONES ((FLAC__uint32)0xffffffff)
#define FLAC__BLURB_TOP_BIT_ONE ((FLAC__uint32)0x80000000)
#define BLURB_BIT_TO_MASK(b) (((FLAC__blurb)0x80000000) >> (b))
#define CRC16_UPDATE_BLURB(bb, blurb, crc) crc16_update_blurb((bb), (blurb));
#ifndef FLAC__OLD_MSVC_FLAVOR

#ifdef FLAC_TABLE_ROOM_VERIFY
#define FLAC__ALIGNED_BLURB_UNARY(blurb) ((blurb) <= 0xff ? (*(p_byte_to_unary_table + blurb)) + 24 : ((blurb) <= 0xffff ? (*(p_byte_to_unary_table + ((blurb) >> 8))) + 16 : ((blurb) <= 0xffffff ? (*(p_byte_to_unary_table + ((blurb) >> 16))) + 8 : (*(p_byte_to_unary_table + ((blurb) >> 24))))))
#else
#define FLAC__ALIGNED_BLURB_UNARY(blurb) ((blurb) <= 0xff ? byte_to_unary_table[blurb] + 24 : ((blurb) <= 0xffff ? byte_to_unary_table[(blurb) >> 8] + 16 : ((blurb) <= 0xffffff ? byte_to_unary_table[(blurb) >> 16] + 8 : byte_to_unary_table[(blurb) >> 24])))
#endif

#endif
#else
/* ERROR, only sizes of 8 and 32 are supported */
#endif

#define FLAC__BLURBS_TO_BITS(blurbs) ((blurbs) << FLAC__BITS_PER_BLURB_LOG2)

#ifdef min
#undef min
#endif
#define min(x,y) ((x)<(y)?(x):(y))
#ifdef max
#undef max
#endif
#define max(x,y) ((x)>(y)?(x):(y))

/* adjust for compilers that can't understand using LLU suffix for uint64_t literals */
#ifdef _MSC_VER
#define FLAC__U64L(x) x
#else
#define FLAC__U64L(x) x##LLU
#endif

#ifndef FLaC__INLINE
#define FLaC__INLINE
#endif
#if 0
struct FLAC__BitBuffer
{
    /* consumed_blurbs 必须为结构体的第一个成员, huweiguo-07-05-09 */
    //----------------------------------------
    unsigned consumed_blurbs, consumed_bits;
    //----------------------------------------
    FLAC__blurb *buffer;
    unsigned capacity; /* in blurbs */
    unsigned blurbs, bits;
    unsigned total_bits; /* must always == FLAC__BITS_PER_BLURB*blurbs+bits */
    //unsigned consumed_blurbs, consumed_bits;
    unsigned total_consumed_bits; /* must always == FLAC__BITS_PER_BLURB*consumed_blurbs+consumed_bits */
    FLAC__uint16 read_crc16;
#if FLAC__BITS_PER_BLURB == 32
    unsigned crc16_align;
#endif
    FLAC__blurb save_head, save_tail;
};
#endif

extern int g_f_FFW_FFD;

_ATTR_FLACDEC_BSS_
FLAC__BitBuffer g_FLAC__BitBuffer;
extern int g_pFLAC__BitBuffer;

#if FLAC__BITS_PER_BLURB == 32
_ATTR_FLACDEC_TEXT_
static void crc16_update_blurb(FLAC__BitBuffer *bb, FLAC__blurb blurb)
{
    if (bb->crc16_align == 0)
    {
        FLAC__CRC16_UPDATE(blurb >> 24, bb->read_crc16);
        FLAC__CRC16_UPDATE((blurb >> 16) & 0xff, bb->read_crc16);
        FLAC__CRC16_UPDATE((blurb >> 8) & 0xff, bb->read_crc16);
        FLAC__CRC16_UPDATE(blurb & 0xff, bb->read_crc16);
    }
    else if (bb->crc16_align == 8)
    {
        FLAC__CRC16_UPDATE((blurb >> 16) & 0xff, bb->read_crc16);
        FLAC__CRC16_UPDATE((blurb >> 8) & 0xff, bb->read_crc16);
        FLAC__CRC16_UPDATE(blurb & 0xff, bb->read_crc16);
    }
    else if (bb->crc16_align == 16)
    {
        FLAC__CRC16_UPDATE((blurb >> 8) & 0xff, bb->read_crc16);
        FLAC__CRC16_UPDATE(blurb & 0xff, bb->read_crc16);
    }
    else if (bb->crc16_align == 24)
    {
        FLAC__CRC16_UPDATE(blurb & 0xff, bb->read_crc16);
    }
    bb->crc16_align = 0;
}
#endif

_ATTR_FLACDEC_TEXT_
static FLAC__bool bitbuffer_read_from_client_(FLAC__BitBuffer *bb, FLAC__bool(*read_callback)(FLAC__byte buffer[], unsigned *bytes, void *client_data), void *client_data)
{
    unsigned bytes;
    FLAC__byte *target;

    /* first shift the unconsumed buffer data toward the front as much as possible */
    if (bb->total_consumed_bits >= FLAC__BITS_PER_BLURB)
    {
#if 1//FLAC__BITS_PER_BLURB == 8
        /*
         * memset and memcpy are usually implemented in assembly language
         * by the system libc, and they can be much faster
         */
        const unsigned r_end = bb->blurbs + (bb->bits ? 1 : 0);
        const unsigned r = bb->consumed_blurbs, l = r_end - r;
        memmove(&bb->buffer[0], &bb->buffer[r], l);
        memset(&bb->buffer[l], 0, r);
#elif FLAC__BITS_PER_BLURB == 32
        /* still needs optimization */
        const unsigned r_end = bb->blurbs + (bb->bits ? 1 : 0);
        unsigned l = 0, r = bb->consumed_blurbs;
        for (; r < r_end; l++, r++)
            bb->buffer[l] = bb->buffer[r];
        for (; l < r_end; l++)
            bb->buffer[l] = 0;
#else
        FLAC__ASSERT(false); /* ERROR, only sizes of 8 and 32 are supported */
#endif /* FLAC__BITS_PER_BLURB == 32 or 8 */

        bb->blurbs -= bb->consumed_blurbs;
        bb->total_bits -= FLAC__BLURBS_TO_BITS(bb->consumed_blurbs);
        bb->consumed_blurbs = 0;
        bb->total_consumed_bits = bb->consumed_bits;
    }

    /* grow if we need to */
    /*if(bb->capacity <= 1) {
     if(!bitbuffer_resize_(bb, 16))
      return false;
    }*/

    /* set the target for reading, taking into account blurb alignment */
#if FLAC__BITS_PER_BLURB == 8

    /* blurb == byte, so no gyrations necessary: */
    target = bb->buffer + bb->blurbs;
    //bytes = bb->capacity - bb->blurbs;

    FLAC__ASSERT(bb->blurbs <= FLAC__BITBUFFER_MAX_REMAINDER);

    //@@@ 每次读 FLAC__BITBUFFER_DEFAULT_CAPACITY 数据，这样可以提高读取的效率
    bytes = FLAC__BITBUFFER_DEFAULT_CAPACITY;

#elif FLAC__BITS_PER_BLURB == 32
    /* @@@ WATCHOUT: code currently only works for big-endian: */
    FLAC__ASSERT((bb->bits & 7) == 0);
    target = (FLAC__byte*)(bb->buffer + bb->blurbs) + (bb->bits >> 3);
    bytes = ((bb->capacity - bb->blurbs) << 2) - (bb->bits >> 3); /* i.e. (bb->capacity - bb->blurbs) * FLAC__BYTES_PER_BLURB - (bb->bits / 8) */
#else
    FLAC__ASSERT(false); /* ERROR, only sizes of 8 and 32 are supported */
#endif

    /* finally, read in some data */
    if (!read_callback(target, &bytes, client_data))
        return false;

    /* now we have to handle partial blurb cases: */
#if FLAC__BITS_PER_BLURB == 8
    /* blurb == byte, so no gyrations necessary: */
    bb->blurbs += bytes;
    bb->total_bits += FLAC__BLURBS_TO_BITS(bytes);
#elif FLAC__BITS_PER_BLURB == 32
    /* @@@ WATCHOUT: code currently only works for big-endian: */
    {
        const unsigned aligned_bytes = (bb->bits >> 3) + bytes;
        bb->blurbs += (aligned_bytes >> 2); /* i.e. aligned_bytes / FLAC__BYTES_PER_BLURB */
        bb->bits = (aligned_bytes & 3u) << 3; /* i.e. (aligned_bytes % FLAC__BYTES_PER_BLURB) * 8 */
        bb->total_bits += (bytes << 3);
    }
#else
    FLAC__ASSERT(false); /* ERROR, only sizes of 8 and 32 are supported */
#endif
    return true;
}

/***********************************************************************
 *
 * Class constructor/destructor
 *
 ***********************************************************************/
 _ATTR_FLACDEC_TEXT_
#if 0
FLAC__BitBuffer *FLAC__bitbuffer_new()
{
    FLAC__BitBuffer *bb = (FLAC__BitBuffer*)calloc(1, sizeof(FLAC__BitBuffer));

    /* calloc() implies:
     memset(bb, 0, sizeof(FLAC__BitBuffer));
     bb->buffer = 0;
     bb->capacity = 0;
     bb->blurbs = bb->bits = bb->total_bits = 0;
     bb->consumed_blurbs = bb->consumed_bits = bb->total_consumed_bits = 0;
    */
    return bb;
}
#else
FLAC__BitBuffer *FLAC__bitbuffer_new()
{
    FLAC__BitBuffer *bb = (FLAC__BitBuffer*) & g_FLAC__BitBuffer;

    g_pFLAC__BitBuffer = (int) & g_FLAC__BitBuffer;

    memset(&g_FLAC__BitBuffer, 0, sizeof(FLAC__BitBuffer));

    return bb;
}
#endif

/***********************************************************************
 *
 * Public class methods
 *
 ***********************************************************************/
_ATTR_FLACDEC_TEXT_
FLAC__bool FLAC__bitbuffer_init(FLAC__BitBuffer *bb)
{
    FLAC__ASSERT(0 != bb);

    bb->buffer = 0;
    bb->capacity = 0;
    bb->blurbs = bb->bits = bb->total_bits = 0;
    bb->consumed_blurbs = bb->consumed_bits = bb->total_consumed_bits = 0;

    return FLAC__bitbuffer_clear(bb);
}

_ATTR_FLACDEC_TEXT_
#if 0
FLAC__bool FLAC__bitbuffer_clear(FLAC__BitBuffer *bb)
{
    if (bb->buffer == 0)
    {
        bb->capacity = FLAC__BITBUFFER_DEFAULT_CAPACITY;
        bb->buffer = (FLAC__blurb*)calloc(bb->capacity, sizeof(FLAC__blurb));
        if (bb->buffer == 0)
            return false;
    }
    else
    {
        memset(bb->buffer, 0, bb->blurbs + (bb->bits ? 1 : 0));
    }
    bb->blurbs = bb->bits = bb->total_bits = 0;
    bb->consumed_blurbs = bb->consumed_bits = bb->total_consumed_bits = 0;
    return true;
}
#else
FLAC__bool FLAC__bitbuffer_clear(FLAC__BitBuffer *bb)
{
    bb->capacity = FLAC__BITBUFFER_DEFAULT_CAPACITY;
    bb->buffer = (FLAC__blurb *)g_FlacInputBuffer;

    bb->blurbs = bb->bits = bb->total_bits = 0;
    bb->consumed_blurbs = bb->consumed_bits = bb->total_consumed_bits = 0;
    return true;
}
#endif

_ATTR_FLACDEC_TEXT_
void FLAC__bitbuffer_reset_read_crc16(FLAC__BitBuffer *bb, FLAC__uint16 seed)
{
    FLAC__ASSERT(0 != bb);
    FLAC__ASSERT(0 != bb->buffer);
    FLAC__ASSERT((bb->consumed_bits & 7) == 0);

    bb->read_crc16 = seed;
#if FLAC__BITS_PER_BLURB == 8
    /* no need to do anything */
#elif FLAC__BITS_PER_BLURB == 32
    bb->crc16_align = bb->consumed_bits;
#else
    FLAC__ASSERT(false); /* ERROR, only sizes of 8 and 32 are supported */
#endif
}

_ATTR_FLACDEC_TEXT_
FLAC__uint16 FLAC__bitbuffer_get_read_crc16(FLAC__BitBuffer *bb)
{
    FLAC__ASSERT(0 != bb);
    FLAC__ASSERT(0 != bb->buffer);
    FLAC__ASSERT((bb->bits & 7) == 0);
    FLAC__ASSERT((bb->consumed_bits & 7) == 0);

#if FLAC__BITS_PER_BLURB == 8
    /* no need to do anything */
#elif FLAC__BITS_PER_BLURB == 32
    /*@@@ BUG: even though this probably can't happen with FLAC, need to fix the case where we are called here for the very first blurb and crc16_align is > 0 */
    if (bb->bits == 0 || bb->consumed_blurbs < bb->blurbs)
    {
        if (bb->consumed_bits == 8)
        {
            const FLAC__blurb blurb = bb->buffer[bb->consumed_blurbs];
            FLAC__CRC16_UPDATE(blurb >> 24, bb->read_crc16);
        }
        else if (bb->consumed_bits == 16)
        {
            const FLAC__blurb blurb = bb->buffer[bb->consumed_blurbs];
            FLAC__CRC16_UPDATE(blurb >> 24, bb->read_crc16);
            FLAC__CRC16_UPDATE((blurb >> 16) & 0xff, bb->read_crc16);
        }
        else if (bb->consumed_bits == 24)
        {
            const FLAC__blurb blurb = bb->buffer[bb->consumed_blurbs];
            FLAC__CRC16_UPDATE(blurb >> 24, bb->read_crc16);
            FLAC__CRC16_UPDATE((blurb >> 16) & 0xff, bb->read_crc16);
            FLAC__CRC16_UPDATE((blurb >> 8) & 0xff, bb->read_crc16);
        }
    }
    else
    {
        if (bb->consumed_bits == 8)
        {
            const FLAC__blurb blurb = bb->buffer[bb->consumed_blurbs];
            FLAC__CRC16_UPDATE(blurb >> (bb->bits - 8), bb->read_crc16);
        }
        else if (bb->consumed_bits == 16)
        {
            const FLAC__blurb blurb = bb->buffer[bb->consumed_blurbs];
            FLAC__CRC16_UPDATE(blurb >> (bb->bits - 8), bb->read_crc16);
            FLAC__CRC16_UPDATE((blurb >> (bb->bits - 16)) & 0xff, bb->read_crc16);
        }
        else if (bb->consumed_bits == 24)
        {
            const FLAC__blurb blurb = bb->buffer[bb->consumed_blurbs];
            FLAC__CRC16_UPDATE(blurb >> (bb->bits - 8), bb->read_crc16);
            FLAC__CRC16_UPDATE((blurb >> (bb->bits - 16)) & 0xff, bb->read_crc16);
            FLAC__CRC16_UPDATE((blurb >> (bb->bits - 24)) & 0xff, bb->read_crc16);
        }
    }
    bb->crc16_align = bb->consumed_bits;
#else
    FLAC__ASSERT(false); /* ERROR, only sizes of 8 and 32 are supported */
#endif
    return bb->read_crc16;
}

_ATTR_FLACDEC_TEXT_
FLAC__bool FLAC__bitbuffer_is_byte_aligned(const FLAC__BitBuffer *bb)
{
    return ((bb->bits & 7) == 0);
}

_ATTR_FLACDEC_TEXT_
FLAC__bool FLAC__bitbuffer_is_consumed_byte_aligned(const FLAC__BitBuffer *bb)
{
    return ((bb->consumed_bits & 7) == 0);
}

_ATTR_FLACDEC_TEXT_
unsigned FLAC__bitbuffer_bits_left_for_byte_alignment(const FLAC__BitBuffer *bb)
{
    return 8 - (bb->consumed_bits & 7);
}

_ATTR_FLACDEC_TEXT_
FLAC__bool FLAC__bitbuffer_peek_bit(FLAC__BitBuffer *bb, unsigned *val, FLAC__bool(*read_callback)(FLAC__byte buffer[], unsigned *bytes, void *client_data), void *client_data)
{
    /* to avoid a drastic speed penalty we don't:
    FLAC__ASSERT(0 != bb);
    FLAC__ASSERT(0 != bb->buffer);
    FLAC__ASSERT(bb->bits == 0);
    */

    while (1)
    {
        if (bb->total_consumed_bits < bb->total_bits)
        {
            *val = (bb->buffer[bb->consumed_blurbs] & BLURB_BIT_TO_MASK(bb->consumed_bits)) ? 1 : 0;
            return true;
        }
        else
        {
            if (!bitbuffer_read_from_client_(bb, read_callback, client_data))
                return false;
        }
    }
}

_ATTR_FLACDEC_TEXT_
FLAC__bool FLAC__bitbuffer_read_bit(FLAC__BitBuffer *bb, unsigned *val, FLAC__bool(*read_callback)(FLAC__byte buffer[], unsigned *bytes, void *client_data), void *client_data)
{
    /* to avoid a drastic speed penalty we don't:
    FLAC__ASSERT(0 != bb);
    FLAC__ASSERT(0 != bb->buffer);
    FLAC__ASSERT(bb->bits == 0);
    */

    while (1)
    {
        if (bb->total_consumed_bits < bb->total_bits)
        {
            *val = (bb->buffer[bb->consumed_blurbs] & BLURB_BIT_TO_MASK(bb->consumed_bits)) ? 1 : 0;
            bb->consumed_bits++;
            if (bb->consumed_bits == FLAC__BITS_PER_BLURB)
            {
                CRC16_UPDATE_BLURB(bb, bb->buffer[bb->consumed_blurbs], bb->read_crc16);
                bb->consumed_blurbs++;
                bb->consumed_bits = 0;
            }
            bb->total_consumed_bits++;
            return true;
        }
        else
        {
            if (!bitbuffer_read_from_client_(bb, read_callback, client_data))
                return false;
        }
    }
}

_ATTR_FLACDEC_TEXT_
FLaC__INLINE FLAC__bool FLAC__bitbuffer_ffw_ffd_init(FLAC__BitBuffer *bb)
{
    bb->blurbs = 0;
    bb->bits = 0;
    bb->total_bits = 0;
    bb->consumed_blurbs = 0;
    bb->consumed_bits = 0;
    bb->total_consumed_bits = 0;
    bb->read_crc16 = 0;

    return true;
}

_ATTR_FLACDEC_TEXT_
FLaC__INLINE FLAC__bool FLAC__bitbuffer_read_raw_uint32(FLAC__BitBuffer *bb, FLAC__uint32 *val, const unsigned bits, FLAC__bool(*read_callback)(FLAC__byte buffer[], unsigned *bytes, void *client_data), void *client_data)
{
    unsigned i, bits_ = bits;
    FLAC__uint32 v = 0;

    FLAC__ASSERT(0 != bb);
    FLAC__ASSERT(0 != bb->buffer);

    FLAC__ASSERT(bits <= 32);
    FLAC__ASSERT((bb->capacity*FLAC__BITS_PER_BLURB) * 2 >= bits);

    if (bits == 0)
    {
        *val = 0;
        return true;
    }

    if (g_f_FFW_FFD)
    {
        if((bb->consumed_bits & 7) != 0)
        {
            bits_ = 0;
        }

        FLAC__bitbuffer_ffw_ffd_init(bb);
        g_f_FFW_FFD = false;
    }

    while (bb->total_consumed_bits + bits > bb->total_bits)
    {
        if (!bitbuffer_read_from_client_(bb, read_callback, client_data))
            return false;
    }
    if (bb->consumed_bits)
    {
        i = FLAC__BITS_PER_BLURB - bb->consumed_bits;
        if (i <= bits_)
        {
            v = bb->buffer[bb->consumed_blurbs] & (FLAC__BLURB_ALL_ONES >> bb->consumed_bits);
            bits_ -= i;
            CRC16_UPDATE_BLURB(bb, bb->buffer[bb->consumed_blurbs], bb->read_crc16);
            bb->consumed_blurbs++;
            bb->consumed_bits = 0;
            /* we hold off updating bb->total_consumed_bits until the end */
        }
        else
        {
            *val = (bb->buffer[bb->consumed_blurbs] & (FLAC__BLURB_ALL_ONES >> bb->consumed_bits)) >> (i - bits_);
            bb->consumed_bits += bits_;
            bb->total_consumed_bits += bits_;
            return true;
        }
    }
    while (bits_ >= FLAC__BITS_PER_BLURB)
    {
        v <<= FLAC__BITS_PER_BLURB;
        v |= bb->buffer[bb->consumed_blurbs];
        bits_ -= FLAC__BITS_PER_BLURB;
        CRC16_UPDATE_BLURB(bb, bb->buffer[bb->consumed_blurbs], bb->read_crc16);
        bb->consumed_blurbs++;
        /* bb->consumed_bits is already 0 */
        /* we hold off updating bb->total_consumed_bits until the end */
    }
    if (bits_ > 0)
    {
        v <<= bits_;
        v |= (bb->buffer[bb->consumed_blurbs] >> (FLAC__BITS_PER_BLURB - bits_));
        bb->consumed_bits = bits_;
        /* we hold off updating bb->total_consumed_bits until the end */
    }
    bb->total_consumed_bits += bits;
    *val = v;

    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC__bool FLAC__bitbuffer_read_raw_int32(FLAC__BitBuffer *bb, FLAC__int32 *val, const unsigned bits, FLAC__bool(*read_callback)(FLAC__byte buffer[], unsigned *bytes, void *client_data), void *client_data)
{
    unsigned i, bits_ = bits;
    FLAC__uint32 v = 0;

    FLAC__ASSERT(0 != bb);
    FLAC__ASSERT(0 != bb->buffer);

    FLAC__ASSERT(bits <= 32);
    FLAC__ASSERT((bb->capacity*FLAC__BITS_PER_BLURB) * 2 >= bits);

    if (bits == 0)
    {
        *val = 0;
        return true;
    }

    if (g_f_FFW_FFD)
    {
        if((bb->consumed_bits & 7) != 0)
        {
            bits_ = 0;
        }

        FLAC__bitbuffer_ffw_ffd_init(bb);
        g_f_FFW_FFD = false;
    }

    while (bb->total_consumed_bits + bits > bb->total_bits)
    {
        if (!bitbuffer_read_from_client_(bb, read_callback, client_data))
            return false;
    }
#if FLAC__BITS_PER_BLURB > 8
    if (bb->bits == 0 || bb->consumed_blurbs < bb->blurbs)  /*@@@ comment on why this is here*/
    {
#endif
        if (bb->consumed_bits)
        {
            i = FLAC__BITS_PER_BLURB - bb->consumed_bits;
            if (i <= bits_)
            {
                v = bb->buffer[bb->consumed_blurbs] & (FLAC__BLURB_ALL_ONES >> bb->consumed_bits);
                bits_ -= i;
                CRC16_UPDATE_BLURB(bb, bb->buffer[bb->consumed_blurbs], bb->read_crc16);
                bb->consumed_blurbs++;
                bb->consumed_bits = 0;
                /* we hold off updating bb->total_consumed_bits until the end */
            }
            else
            {
                /* bits_ must be < FLAC__BITS_PER_BLURB-1 if we get to here */
                v = (bb->buffer[bb->consumed_blurbs] & (FLAC__BLURB_ALL_ONES >> bb->consumed_bits));
                v <<= (32 - i);
                *val = (FLAC__int32)v;
                *val >>= (32 - bits_);
                bb->consumed_bits += bits_;
                bb->total_consumed_bits += bits_;
                return true;
            }
        }
#if FLAC__BITS_PER_BLURB == 32
        /* note that we know bits_ cannot be > 32 because of previous assertions */
        if (bits_ == FLAC__BITS_PER_BLURB)
        {
            v = bb->buffer[bb->consumed_blurbs];
            bits_ = 0;
            CRC16_UPDATE_BLURB(bb, v, bb->read_crc16);
            bb->consumed_blurbs++;
            /* bb->consumed_bits is already 0 */
            /* we hold off updating bb->total_consumed_bits until the end */
        }
#else
        while (bits_ >= FLAC__BITS_PER_BLURB)
        {
            v <<= FLAC__BITS_PER_BLURB;
            v |= bb->buffer[bb->consumed_blurbs];
            bits_ -= FLAC__BITS_PER_BLURB;
            CRC16_UPDATE_BLURB(bb, bb->buffer[bb->consumed_blurbs], bb->read_crc16);
            bb->consumed_blurbs++;
            /* bb->consumed_bits is already 0 */
            /* we hold off updating bb->total_consumed_bits until the end */
        }
#endif
        if (bits_ > 0)
        {
            v <<= bits_;
            v |= (bb->buffer[bb->consumed_blurbs] >> (FLAC__BITS_PER_BLURB - bits_));
            bb->consumed_bits = bits_;
            /* we hold off updating bb->total_consumed_bits until the end */
        }
        bb->total_consumed_bits += bits;
#if FLAC__BITS_PER_BLURB > 8
    }
    else
    {
        for (i = 0; i < bits; i++)
        {
            if (!FLAC__bitbuffer_read_bit_to_uint32(bb, &v, read_callback, client_data))
                return false;
        }
    }
#endif

    /* fix the sign */
    i = 32 - bits;
    if (i)
    {
        v <<= i;
        *val = (FLAC__int32)v;
        *val >>= i;
    }
    else
        *val = (FLAC__int32)v;

    return true;
}

_ATTR_FLACDEC_TEXT_
FLaC__INLINE FLAC__bool FLAC__bitbuffer_read_raw_uint32_little_endian(FLAC__BitBuffer *bb, FLAC__uint32 *val, FLAC__bool(*read_callback)(FLAC__byte buffer[], unsigned *bytes, void *client_data), void *client_data)
{
    FLAC__uint32 x8, x32 = 0;

    /* this doesn't need to be that fast as currently it is only used for vorbis comments */

    if (!FLAC__bitbuffer_read_raw_uint32(bb, &x32, 8, read_callback, client_data))
        return false;

    if (!FLAC__bitbuffer_read_raw_uint32(bb, &x8, 8, read_callback, client_data))
        return false;
    x32 |= (x8 << 8);

    if (!FLAC__bitbuffer_read_raw_uint32(bb, &x8, 8, read_callback, client_data))
        return false;
    x32 |= (x8 << 16);

    if (!FLAC__bitbuffer_read_raw_uint32(bb, &x8, 8, read_callback, client_data))
        return false;
    x32 |= (x8 << 24);

    *val = x32;
    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC__bool FLAC__bitbuffer_skip_bits_no_crc(FLAC__BitBuffer *bb, unsigned bits, FLAC__bool(*read_callback)(FLAC__byte buffer[], unsigned *bytes, void *client_data), void *client_data)
{
    /*
     * @@@ a slightly faster implementation is possible but
     * probably not that useful since this is only called a
     * couple of times in the metadata readers.
     */
    FLAC__ASSERT(0 != bb);
    FLAC__ASSERT(0 != bb->buffer);

    if (bits > 0)
    {
        const unsigned n = bb->consumed_bits & 7;
        unsigned m;
        FLAC__uint32 x;

        if (n != 0)
        {
            m = min(8 - n, bits);
            if (!FLAC__bitbuffer_read_raw_uint32(bb, &x, m, read_callback, client_data))
                return false;
            bits -= m;
        }
        m = bits / 8;
        if (m > 0)
        {
            if (!FLAC__bitbuffer_read_byte_block_aligned_no_crc(bb, 0, m, read_callback, client_data))
                return false;
            bits %= 8;
        }
        if (bits > 0)
        {
            if (!FLAC__bitbuffer_read_raw_uint32(bb, &x, bits, read_callback, client_data))
                return false;
        }
    }

    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC__bool FLAC__bitbuffer_read_byte_block_aligned_no_crc(FLAC__BitBuffer *bb, FLAC__byte *val, unsigned nvals, FLAC__bool(*read_callback)(FLAC__byte buffer[], unsigned *bytes, void *client_data), void *client_data)
{
    FLAC__ASSERT(0 != bb);
    FLAC__ASSERT(0 != bb->buffer);
    FLAC__ASSERT(FLAC__bitbuffer_is_byte_aligned(bb));
    FLAC__ASSERT(FLAC__bitbuffer_is_consumed_byte_aligned(bb));
#if FLAC__BITS_PER_BLURB == 8
    while (nvals > 0)
    {
        unsigned chunk = min(nvals, bb->blurbs - bb->consumed_blurbs);
        if (chunk == 0)
        {
            if (!bitbuffer_read_from_client_(bb, read_callback, client_data))
                return false;
        }
        else
        {
            if (0 != val)
            {
                memcpy(val, bb->buffer + bb->consumed_blurbs, FLAC__BYTES_PER_BLURB * chunk);
                val += FLAC__BYTES_PER_BLURB * chunk;
            }
            nvals -= chunk;
            bb->consumed_blurbs += chunk;
            bb->total_consumed_bits = (bb->consumed_blurbs << FLAC__BITS_PER_BLURB_LOG2);
        }
    }
#else
    @@@ need to write this still
    FLAC__ASSERT(0);
#endif

    return true;
}

_ATTR_FLACDEC_TEXT_
FLaC__INLINE FLAC__bool FLAC__bitbuffer_read_unary_unsigned(FLAC__BitBuffer *bb, unsigned *val, FLAC__bool(*read_callback)(FLAC__byte buffer[], unsigned *bytes, void *client_data), void *client_data)
#ifdef FLAC__NO_MANUAL_INLINING
{
    unsigned bit, val_ = 0;

    FLAC__ASSERT(0 != bb);
    FLAC__ASSERT(0 != bb->buffer);

    while (1)
    {
        if (!FLAC__bitbuffer_read_bit(bb, &bit, read_callback, client_data))
            return false;
        if (bit)
            break;
        else
            val_++;
    }
    *val = val_;
    return true;
}
#else
{
    unsigned i, val_ = 0;
    unsigned total_blurbs_ = (bb->total_bits + (FLAC__BITS_PER_BLURB - 1)) / FLAC__BITS_PER_BLURB;
    FLAC__blurb b;

    FLAC__ASSERT(0 != bb);
    FLAC__ASSERT(0 != bb->buffer);

#if FLAC__BITS_PER_BLURB > 8
    if (bb->bits == 0 || bb->consumed_blurbs < bb->blurbs)  /*@@@ comment on why this is here*/
    {
#endif
        if (bb->consumed_bits)
        {
            b = bb->buffer[bb->consumed_blurbs] << bb->consumed_bits;
            if (b)
            {
                for (i = 0; !(b & FLAC__BLURB_TOP_BIT_ONE); i++)
                    b <<= 1;
                *val = i;
                i++;
                bb->consumed_bits += i;
                bb->total_consumed_bits += i;
                if (bb->consumed_bits == FLAC__BITS_PER_BLURB)
                {
                    CRC16_UPDATE_BLURB(bb, bb->buffer[bb->consumed_blurbs], bb->read_crc16);
                    bb->consumed_blurbs++;
                    bb->consumed_bits = 0;
                }
                return true;
            }
            else
            {
                val_ = FLAC__BITS_PER_BLURB - bb->consumed_bits;
                CRC16_UPDATE_BLURB(bb, bb->buffer[bb->consumed_blurbs], bb->read_crc16);
                bb->consumed_blurbs++;
                bb->consumed_bits = 0;
                bb->total_consumed_bits += val_;
            }
        }
        while (1)
        {
            if (bb->consumed_blurbs >= total_blurbs_)
            {
                if (!bitbuffer_read_from_client_(bb, read_callback, client_data))
                    return false;
                total_blurbs_ = (bb->total_bits + (FLAC__BITS_PER_BLURB - 1)) / FLAC__BITS_PER_BLURB;
            }
            b = bb->buffer[bb->consumed_blurbs];
            if (b)
            {
                for (i = 0; !(b & FLAC__BLURB_TOP_BIT_ONE); i++)
                    b <<= 1;
                val_ += i;
                i++;
                bb->consumed_bits = i;
                *val = val_;
                if (i == FLAC__BITS_PER_BLURB)
                {
                    CRC16_UPDATE_BLURB(bb, bb->buffer[bb->consumed_blurbs], bb->read_crc16);
                    bb->consumed_blurbs++;
                    bb->consumed_bits = 0;
                }
                bb->total_consumed_bits += i;
                return true;
            }
            else
            {
                val_ += FLAC__BITS_PER_BLURB;
                CRC16_UPDATE_BLURB(bb, 0, bb->read_crc16);
                bb->consumed_blurbs++;
                /* bb->consumed_bits is already 0 */
                bb->total_consumed_bits += FLAC__BITS_PER_BLURB;
            }
        }
#if FLAC__BITS_PER_BLURB > 8
    }
    else
    {
        while (1)
        {
            if (!FLAC__bitbuffer_read_bit(bb, &i, read_callback, client_data))
                return false;
            if (i)
                break;
            else
                val_++;
        }
        *val = val_;
        return true;
    }
#endif
}
#endif

#ifdef HALF_FRAME_BY_HALF_FRAME
extern unsigned g_half_block_size;
#endif

_ATTR_FLACDEC_TEXT_
FLAC__bool FLAC__bitbuffer_read_rice_signed_block(FLAC__BitBuffer *bb, int vals[], unsigned nvals, unsigned parameter, FLAC__bool(*read_callback)(FLAC__byte buffer[], unsigned *bytes, void *client_data), void *client_data,unsigned decoded_samples)
{
    const FLAC__blurb *buffer = bb->buffer;

    unsigned i, j, val_i = nvals;
    unsigned cbits = 0, uval = 0, msbs = 0, lsbs_left = 0;
    FLAC__blurb blurb, save_blurb;
    unsigned state = 0; /* 0 = getting unary MSBs, 1 = getting binary LSBs */
    //long values;//
#ifdef HALF_FRAME_BY_HALF_FRAME
    const unsigned half_block_size = g_half_block_size;
#endif
    FLAC__ASSERT(0 != bb);
    FLAC__ASSERT(0 != bb->buffer);
    FLAC__ASSERT(parameter <= 31);

    if (nvals == 0)
        return true;

    cbits = bb->consumed_bits;
    i = bb->consumed_blurbs;

    while (val_i != 0)
    {
        for (; i < bb->blurbs; i++)
        {
            blurb = (save_blurb = buffer[i]) << cbits;
            blurb &= 0xff;
            while (1)
            {
                if (state == 0)
                {
                    if (blurb)
                    {
                        j = FLAC__ALIGNED_BLURB_UNARY(blurb);
                        msbs += j;
                        j++;
                        cbits += j;

                        uval = 0;
                        lsbs_left = parameter;
                        state++;
                        if (cbits == FLAC__BITS_PER_BLURB)
                        {
                            cbits = 0;
                            CRC16_UPDATE_BLURB(bb, save_blurb, bb->read_crc16);
                            break;
                        }
                        blurb <<= j;
                        blurb &= 0xff;
                    }
                    else
                    {
                        msbs += FLAC__BITS_PER_BLURB - cbits;
                        cbits = 0;
                        CRC16_UPDATE_BLURB(bb, save_blurb, bb->read_crc16);
                        break;
                    }
                }
                else
                {
                    const unsigned available_bits = FLAC__BITS_PER_BLURB - cbits;
                    if (lsbs_left >= available_bits)
                    {
                        uval <<= available_bits;
                        uval |= (blurb >> cbits);
                        cbits = 0;
                        CRC16_UPDATE_BLURB(bb, save_blurb, bb->read_crc16);

                        if (lsbs_left == available_bits)
                        {
                            /* compose the value */
                            uval |= (msbs << parameter);
#ifdef HALF_FRAME_BY_HALF_FRAME
                            if((0 == isNeedDecByHalfFrmae) || (decoded_samples < half_block_size))
#endif
                            *vals = (int)(uval >> 1 ^ -(int)(uval & 1));

                           decoded_samples++;

                            --val_i;
                            if (val_i == 0)
                            {
                                i++;
                                goto break2;
                            }

                            ++vals;

                            msbs = 0;
                            state = 0;
                        }

                        lsbs_left -= available_bits;
                        break;
                    }
                    else
                    {
                        cbits += lsbs_left;
                        uval <<= lsbs_left;
                        uval |= (blurb >> (FLAC__BITS_PER_BLURB - lsbs_left));
                        blurb <<= lsbs_left;
                        blurb &= 0xff;

                        /* compose the value */
                        uval |= (msbs << parameter);
#ifdef HALF_FRAME_BY_HALF_FRAME
                        if((0 == isNeedDecByHalfFrmae) || (decoded_samples < half_block_size))
#endif
                        *vals = (int)(uval >> 1 ^ -(int)(uval & 1));
#ifdef HALF_FRAME_BY_HALF_FRAME
                        decoded_samples++;
#endif

                        --val_i;
                        if (val_i == 0)
                            goto break2;

                        ++vals;

                        msbs = 0;
                        state = 0;
                    }
                }
            }
        }
break2:
        bb->consumed_blurbs = i;
        bb->consumed_bits = cbits;
        bb->total_consumed_bits = (i << FLAC__BITS_PER_BLURB_LOG2) | cbits;
        if (val_i != 0)
        {
            if (!bitbuffer_read_from_client_(bb, read_callback, client_data))
                return false;
            /* these must be zero because we can only get here if we got to the end of the buffer */
            FLAC__ASSERT(bb->consumed_blurbs == 0);
            FLAC__ASSERT(bb->consumed_bits == 0);
            i = 0;
        }
    }


    return true;
}

/* on return, if *val == 0xffffffff then the utf-8 sequence was invalid, but the return value will be true */
_ATTR_FLACDEC_TEXT_
FLAC__bool FLAC__bitbuffer_read_utf8_uint32(FLAC__BitBuffer *bb, FLAC__uint32 *val, FLAC__bool(*read_callback)(FLAC__byte buffer[], unsigned *bytes, void *client_data), void *client_data, FLAC__byte *raw, unsigned *rawlen)
{
    FLAC__uint32 v = 0;
    FLAC__uint32 x;
    unsigned i;

    if (!FLAC__bitbuffer_read_raw_uint32(bb, &x, 8, read_callback, client_data))
        return false;
    if (raw)
        raw[(*rawlen)++] = (FLAC__byte)x;
    if (!(x & 0x80))  /* 0xxxxxxx */
    {
        v = x;
        i = 0;
    }
    else if (x & 0xC0 && !(x & 0x20))  /* 110xxxxx */
    {
        v = x & 0x1F;
        i = 1;
    }
    else if (x & 0xE0 && !(x & 0x10))  /* 1110xxxx */
    {
        v = x & 0x0F;
        i = 2;
    }
    else if (x & 0xF0 && !(x & 0x08))  /* 11110xxx */
    {
        v = x & 0x07;
        i = 3;
    }
    else if (x & 0xF8 && !(x & 0x04))  /* 111110xx */
    {
        v = x & 0x03;
        i = 4;
    }
    else if (x & 0xFC && !(x & 0x02))  /* 1111110x */
    {
        v = x & 0x01;
        i = 5;
    }
    else
    {
        *val = 0xffffffff;
        return true;
    }
    for (; i; i--)
    {
        if (!FLAC__bitbuffer_read_raw_uint32(bb, &x, 8, read_callback, client_data))
            return false;
        if (raw)
            raw[(*rawlen)++] = (FLAC__byte)x;
        if (!(x & 0x80) || (x & 0x40))  /* 10xxxxxx */
        {
            *val = 0xffffffff;
            return true;
        }
        v <<= 6;
        v |= (x & 0x3F);
    }
    *val = v;
    return true;
}

/* on return, if *val == 0xffffffffffffffff then the utf-8 sequence was invalid, but the return value will be true */
_ATTR_FLACDEC_TEXT_
FLAC__bool FLAC__bitbuffer_read_utf8_uint64(FLAC__BitBuffer *bb, FLAC__uint64 *val, FLAC__bool(*read_callback)(FLAC__byte buffer[], unsigned *bytes, void *client_data), void *client_data, FLAC__byte *raw, unsigned *rawlen)
{
    FLAC__uint64 v = 0;
    FLAC__uint32 x;
    unsigned i;

    if (!FLAC__bitbuffer_read_raw_uint32(bb, &x, 8, read_callback, client_data))
        return false;
    if (raw)
        raw[(*rawlen)++] = (FLAC__byte)x;
    if (!(x & 0x80))  /* 0xxxxxxx */
    {
        v = x;
        i = 0;
    }
    else if (x & 0xC0 && !(x & 0x20))  /* 110xxxxx */
    {
        v = x & 0x1F;
        i = 1;
    }
    else if (x & 0xE0 && !(x & 0x10))  /* 1110xxxx */
    {
        v = x & 0x0F;
        i = 2;
    }
    else if (x & 0xF0 && !(x & 0x08))  /* 11110xxx */
    {
        v = x & 0x07;
        i = 3;
    }
    else if (x & 0xF8 && !(x & 0x04))  /* 111110xx */
    {
        v = x & 0x03;
        i = 4;
    }
    else if (x & 0xFC && !(x & 0x02))  /* 1111110x */
    {
        v = x & 0x01;
        i = 5;
    }
    else if (x & 0xFE && !(x & 0x01))  /* 11111110 */
    {
        v = 0;
        i = 6;
    }
    else
    {
        *val = FLAC__U64L(0xffffffffffffffff);
        return true;
    }
    for (; i; i--)
    {
        if (!FLAC__bitbuffer_read_raw_uint32(bb, &x, 8, read_callback, client_data))
            return false;
        if (raw)
            raw[(*rawlen)++] = (FLAC__byte)x;
        if (!(x & 0x80) || (x & 0x40))  /* 10xxxxxx */
        {
            *val = FLAC__U64L(0xffffffffffffffff);
            return true;
        }
        v <<= 6;
        v |= (x & 0x3F);
    }
    *val = v;
    return true;
}

#ifdef HALF_FRAME_BY_HALF_FRAME
_ATTR_FLACDEC_TEXT_
FLAC__bool post_FLAC__bitbuffer_read_rice_signed_block(FLAC__BitBuffer *bb, int vals[], unsigned nvals, unsigned parameter, FLAC__bool(*read_callback)(FLAC__byte buffer[], unsigned *bytes, void *client_data), void *client_data,unsigned decoded_samples)
{
    const FLAC__blurb *buffer = bb->buffer;

    unsigned i, j, val_i = nvals;
    unsigned cbits = 0, uval = 0, msbs = 0, lsbs_left = 0;
    FLAC__blurb blurb, save_blurb;
    unsigned state = 0; /* 0 = getting unary MSBs, 1 = getting binary LSBs */
    //long values;//

    const unsigned half_block_size = g_half_block_size;

    FLAC__ASSERT(0 != bb);
    FLAC__ASSERT(0 != bb->buffer);
    FLAC__ASSERT(parameter <= 31);

    if (nvals == 0)
        return true;

    cbits = bb->consumed_bits;
    i = bb->consumed_blurbs;

    while (val_i != 0)
    {
        for (; i < bb->blurbs; i++)
        {
            blurb = (save_blurb = buffer[i]) << cbits;
            blurb &= 0xff;
            while (1)
            {
                if (state == 0)
                {
                    if (blurb)
                    {
                        j = FLAC__ALIGNED_BLURB_UNARY(blurb);
                        msbs += j;
                        j++;
                        cbits += j;

                        uval = 0;
                        lsbs_left = parameter;
                        state++;
                        if (cbits == FLAC__BITS_PER_BLURB)
                        {
                            cbits = 0;
                            CRC16_UPDATE_BLURB(bb, save_blurb, bb->read_crc16);
                            break;
                        }
                        blurb <<= j;
                        blurb &= 0xff;
                    }
                    else
                    {
                        msbs += FLAC__BITS_PER_BLURB - cbits;
                        cbits = 0;
                        CRC16_UPDATE_BLURB(bb, save_blurb, bb->read_crc16);
                        break;
                    }
                }
                else
                {
                    const unsigned available_bits = FLAC__BITS_PER_BLURB - cbits;
                    if (lsbs_left >= available_bits)
                    {
                        uval <<= available_bits;
                        uval |= (blurb >> cbits);
                        cbits = 0;
                        CRC16_UPDATE_BLURB(bb, save_blurb, bb->read_crc16);

                        if (lsbs_left == available_bits)
                        {
                            /* compose the value */
                            uval |= (msbs << parameter);

                            if(decoded_samples >= half_block_size)

                            *vals = (int)(uval >> 1 ^ -(int)(uval & 1));

                           decoded_samples++;

                            /*values = (long)(uval >> 1 ^ -(long)(uval & 1));//
                            if(values > (long)0x00007fff)
                            {
                             values = 0x00007fff;
                            }
                            else if(values < (long)0xffff8000)
                            {
                             values = 0xffff8000;
                            }*/

                            --val_i;
                            if (val_i == 0)
                            {
                                i++;
                                goto break2;
                            }

                            ++vals;

                            msbs = 0;
                            state = 0;
                        }

                        lsbs_left -= available_bits;
                        break;
                    }
                    else
                    {
                        cbits += lsbs_left;
                        uval <<= lsbs_left;
                        uval |= (blurb >> (FLAC__BITS_PER_BLURB - lsbs_left));
                        blurb <<= lsbs_left;
                        blurb &= 0xff;

                        /* compose the value */
                        uval |= (msbs << parameter);
#ifdef HALF_FRAME_BY_HALF_FRAME
                        if(decoded_samples >= half_block_size)
#endif
                        *vals = (int)(uval >> 1 ^ -(int)(uval & 1));
#ifdef HALF_FRAME_BY_HALF_FRAME
                        decoded_samples++;
#endif

                        /*values = (long)(uval >> 1 ^ -(long)(uval & 1));//
                        if(values > (long)0x00007fff)
                        {
                         values = 0x00007fff;
                        }
                        else if(values < (long)0xffff8000)
                        {
                         values = 0xffff8000;
                        }*/

                        --val_i;
                        if (val_i == 0)
                            goto break2;

                        ++vals;

                        msbs = 0;
                        state = 0;
                    }
                }
            }
        }
break2:
        bb->consumed_blurbs = i;
        bb->consumed_bits = cbits;
        bb->total_consumed_bits = (i << FLAC__BITS_PER_BLURB_LOG2) | cbits;
        if (val_i != 0)
        {
            if (!bitbuffer_read_from_client_(bb, read_callback, client_data))
                return false;
            /* these must be zero because we can only get here if we got to the end of the buffer */
            FLAC__ASSERT(bb->consumed_blurbs == 0);
            FLAC__ASSERT(bb->consumed_bits == 0);
            i = 0;
        }
    }

    return true;
}
#endif

#endif
#endif
