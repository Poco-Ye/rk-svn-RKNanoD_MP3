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

#include <stdio.h>
#include <stdlib.h> /* for qsort() */
#include "assert.h"
#include "format.h"
#include "format_i.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef min
#undef min
#endif
#define min(a,b) ((a)<(b)?(a):(b))

/* adjust for compilers that can't understand using LLU suffix for uint64_t literals */
#ifdef _MSC_VER
#define FLAC__U64L(x) x
#else
#define FLAC__U64L(x) x##LLU
#endif
#define VERSION "1.1.2"

_ATTR_FLACDEC_BSS_
unsigned long g_parameters[(1 << 8)];
_ATTR_FLACDEC_BSS_
unsigned long g_raw_bits[(1 << 8)];


/* VERSION should come from configure */
_ATTR_FLACDEC_DATA_
FLAC_API char *FLAC__VERSION_STRING = VERSION;

//#if defined _MSC_VER || defined __MINW32__
/* yet one more hack because of MSVC6: */
_ATTR_FLACDEC_DATA_
FLAC_API char *FLAC__VENDOR_STRING = "reference libFLAC 1.1.2 20050205";
//#else
//FLAC_API char *FLAC__VENDOR_STRING = "reference libFLAC " VERSION " 20050205";
//#endif

_ATTR_FLACDEC_DATA_
FLAC_API FLAC__byte FLAC__STREAM_SYNC_STRING[4] = { 'f', 'L', 'a', 'C' };
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_SYNC = 0x664C6143;
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_SYNC_LEN = 32; /* bits */;

_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_STREAMINFO_MIN_BLOCK_SIZE_LEN = 16; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_STREAMINFO_MAX_BLOCK_SIZE_LEN = 16; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_STREAMINFO_MIN_FRAME_SIZE_LEN = 24; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_STREAMINFO_MAX_FRAME_SIZE_LEN = 24; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_STREAMINFO_SAMPLE_RATE_LEN = 20; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_STREAMINFO_CHANNELS_LEN = 3; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_STREAMINFO_BITS_PER_SAMPLE_LEN = 5; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_STREAMINFO_TOTAL_SAMPLES_LEN = 36; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_STREAMINFO_MD5SUM_LEN = 128; /* bits */

_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_APPLICATION_ID_LEN = 32; /* bits */

_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_SEEKPOINT_SAMPLE_NUMBER_LEN = 64; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_SEEKPOINT_STREAM_OFFSET_LEN = 64; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_SEEKPOINT_FRAME_SAMPLES_LEN = 16; /* bits */

_ATTR_FLACDEC_DATA_
FLAC_API FLAC__uint64 FLAC__STREAM_METADATA_SEEKPOINT_PLACEHOLDER = FLAC__U64L(0xffffffffffffffff);

_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_VORBIS_COMMENT_ENTRY_LENGTH_LEN = 32; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_VORBIS_COMMENT_NUM_COMMENTS_LEN = 32; /* bits */

_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_CUESHEET_INDEX_OFFSET_LEN = 64; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_CUESHEET_INDEX_NUMBER_LEN = 8; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_CUESHEET_INDEX_RESERVED_LEN = 3 * 8; /* bits */

_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_CUESHEET_TRACK_OFFSET_LEN = 64; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_CUESHEET_TRACK_NUMBER_LEN = 8; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_CUESHEET_TRACK_ISRC_LEN = 12 * 8; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_CUESHEET_TRACK_TYPE_LEN = 1; /* bit */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_CUESHEET_TRACK_PRE_EMPHASIS_LEN = 1; /* bit */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_CUESHEET_TRACK_RESERVED_LEN = 6 + 13 * 8; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_CUESHEET_TRACK_NUM_INDICES_LEN = 8; /* bits */

_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_CUESHEET_MEDIA_CATALOG_NUMBER_LEN = 128 * 8; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_CUESHEET_LEAD_IN_LEN = 64; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_CUESHEET_IS_CD_LEN = 1; /* bit */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_CUESHEET_RESERVED_LEN = 7 + 258 * 8; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_CUESHEET_NUM_TRACKS_LEN = 8; /* bits */

_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_IS_LAST_LEN = 1; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_TYPE_LEN = 7; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__STREAM_METADATA_LENGTH_LEN = 24; /* bits */

_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__FRAME_HEADER_SYNC = 0x3ffe;
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__FRAME_HEADER_SYNC_LEN = 14; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__FRAME_HEADER_RESERVED_LEN = 2; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__FRAME_HEADER_BLOCK_SIZE_LEN = 4; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__FRAME_HEADER_SAMPLE_RATE_LEN = 4; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__FRAME_HEADER_CHANNEL_ASSIGNMENT_LEN = 4; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__FRAME_HEADER_BITS_PER_SAMPLE_LEN = 3; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__FRAME_HEADER_ZERO_PAD_LEN = 1; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__FRAME_HEADER_CRC_LEN = 8; /* bits */

_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__FRAME_FOOTER_CRC_LEN = 16; /* bits */

_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__ENTROPY_CODING_METHOD_TYPE_LEN = 2; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE_ORDER_LEN = 4; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE_PARAMETER_LEN = 4; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE_RAW_LEN = 5; /* bits */

_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE_ESCAPE_PARAMETER = 15; /* == (1<<FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE_PARAMETER_LEN)-1 */

_ATTR_FLACDEC_DATA_
FLAC_API char * FLAC__EntropyCodingMethodTypeString[] =
{
    "PARTITIONED_RICE"
};

_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__SUBFRAME_LPC_QLP_COEFF_PRECISION_LEN = 4; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__SUBFRAME_LPC_QLP_SHIFT_LEN = 5; /* bits */

_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__SUBFRAME_ZERO_PAD_LEN = 1; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__SUBFRAME_TYPE_LEN = 6; /* bits */
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__SUBFRAME_WASTED_BITS_FLAG_LEN = 1; /* bits */

_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__SUBFRAME_TYPE_CONSTANT_BYTE_ALIGNED_MASK = 0x00;
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__SUBFRAME_TYPE_VERBATIM_BYTE_ALIGNED_MASK = 0x02;
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__SUBFRAME_TYPE_FIXED_BYTE_ALIGNED_MASK = 0x10;
_ATTR_FLACDEC_DATA_
FLAC_API unsigned FLAC__SUBFRAME_TYPE_LPC_BYTE_ALIGNED_MASK = 0x40;

_ATTR_FLACDEC_DATA_
FLAC_API char * FLAC__SubframeTypeString[] =
{
    "CONSTANT",
    "VERBATIM",
    "FIXED",
    "LPC"
};

_ATTR_FLACDEC_DATA_
FLAC_API char * FLAC__ChannelAssignmentString[] =
{
    "INDEPENDENT",
    "LEFT_SIDE",
    "RIGHT_SIDE",
    "MID_SIDE"
};

_ATTR_FLACDEC_DATA_
FLAC_API char * FLAC__FrameNumberTypeString[] =
{
    "FRAME_NUMBER_TYPE_FRAME_NUMBER",
    "FRAME_NUMBER_TYPE_SAMPLE_NUMBER"
};

_ATTR_FLACDEC_DATA_
FLAC_API char * FLAC__MetadataTypeString[] =
{
    "STREAMINFO",
    "PADDING",
    "APPLICATION",
    "SEEKTABLE",
    "VORBIS_COMMENT",
    "CUESHEET"
};

_ATTR_FLACDEC_TEXT_
void FLAC__format_entropy_coding_method_partitioned_rice_contents_init(FLAC__EntropyCodingMethod_PartitionedRiceContents *object)
{
    FLAC__ASSERT(0 != object);

    object->parameters = 0;
    object->raw_bits = 0;
    object->capacity_by_order = 0;
}

_ATTR_FLACDEC_TEXT_
FLAC__bool FLAC__format_entropy_coding_method_partitioned_rice_contents_ensure_size(FLAC__EntropyCodingMethod_PartitionedRiceContents *object, unsigned max_partition_order)
{
    FLAC__ASSERT(0 != object);

    FLAC__ASSERT(object->capacity_by_order > 0 || (0 == object->parameters && 0 == object->raw_bits));

    /*if(object->capacity_by_order < max_partition_order) {
     if(0 == (object->parameters = (unsigned*)realloc(object->parameters, sizeof(unsigned)*(1 << max_partition_order))))
      return false;
     if(0 == (object->raw_bits = (unsigned*)realloc(object->raw_bits, sizeof(unsigned)*(1 << max_partition_order))))
      return false;
     object->capacity_by_order = max_partition_order;
    }*/
    object->parameters = (unsigned*)g_parameters;
    object->raw_bits = (unsigned*)g_raw_bits;
    object->capacity_by_order = max_partition_order;
    FLAC__ASSERT(max_partition_order <= 8); // 不然的话我们分配的空间不够

    return true;
}

#endif
#endif
