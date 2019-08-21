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
#include <stdlib.h> /* for flac_malloc() */
#include <string.h> /* for memset/memcpy() */
#include "assert.h"
#include "stream_decoder.h"
#include "bitbuffer.h"
#include "bitmath.h"
#include "cpu.h"
#include "crc.h"
#include "fixed.h"
#include "format.h"
#include "lpc.h"
//#include "private/memory.h"
#include "replacer.h"
#include "flacbuffer.h"
#include "flac_decoder_info.h"
#include "FLACTab.h"

extern FLAC_Decoder_Info g_flac_decoder_info;
extern unsigned long g_CurrFileOffset; // 当前文件偏移量
extern FILE *g_hFlacFile,*g_hFlacFileBake;


extern unsigned int flac_fread(void *buffer, unsigned int size, unsigned int count, FILE *stream);

extern int flac_fseek(FILE *stream, long offset, int origin);

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef max
#undef max
#endif
#define max(a,b) ((a)>(b)?(a):(b))

/* adjust for compilers that can't understand using LLU suffix for uint64_t literals */
#ifdef _MSC_VER
#define FLAC__U64L(x) x
#else
#define FLAC__U64L(x) x##LLU
#endif
/*
*/
#if defined(FLAC_MINUS_ZI_DATA)||defined(HALF_FRAME_BY_HALF_FRAME)
unsigned gFLACchannel = 2;//intionally initialized as a invalid value
#endif
#ifdef HALF_FRAME_BY_HALF_FRAME
unsigned g_half_block_size = 0;
#endif
_ATTR_FLACDEC_DATA_
unsigned int g_read_frame_header_error_cnt;

/***********************************************************************
 *
 * Private static data
 *
 ***********************************************************************/
_ATTR_FLACDEC_DATA_
static FLAC__byte ID3V2_TAG_[3] = { 'I', 'D', '3' };

/***********************************************************************
 *
 * Private class method prototypes
 *
 ***********************************************************************/

static void set_defaults_(FLAC__StreamDecoder *decoder);
static FLAC__bool allocate_output_(FLAC__StreamDecoder *decoder, unsigned size, unsigned channels);
static FLAC__bool has_id_filtered_(FLAC__StreamDecoder *decoder, FLAC__byte *id);
static FLAC__bool find_metadata_(FLAC__StreamDecoder *decoder);
static FLAC__bool read_metadata_(FLAC__StreamDecoder *decoder);
static FLAC__bool read_metadata_streaminfo_(FLAC__StreamDecoder *decoder, FLAC__bool is_last, unsigned length);
static FLAC__bool read_metadata_seektable_(FLAC__StreamDecoder *decoder, FLAC__bool is_last, unsigned length);
static FLAC__bool read_metadata_vorbiscomment_(FLAC__StreamDecoder *decoder, FLAC__StreamMetadata_VorbisComment *obj);
static FLAC__bool read_metadata_cuesheet_(FLAC__StreamDecoder *decoder, FLAC__StreamMetadata_CueSheet *obj);
static FLAC__bool skip_id3v2_tag_(FLAC__StreamDecoder *decoder);
static FLAC__bool frame_sync_(FLAC__StreamDecoder *decoder);
static FLAC__bool read_frame_(FLAC__StreamDecoder *decoder, FLAC__bool *got_a_frame, FLAC__bool do_full_decode);
static FLAC__bool read_frame_header_(FLAC__StreamDecoder *decoder);
static FLAC__bool read_subframe_(FLAC__StreamDecoder *decoder, unsigned channel, unsigned bps, FLAC__bool do_full_decode);
static FLAC__bool read_subframe_constant_(FLAC__StreamDecoder *decoder, unsigned channel, unsigned bps, FLAC__bool do_full_decode);
static FLAC__bool read_subframe_fixed_(FLAC__StreamDecoder *decoder, unsigned channel, unsigned bps, const unsigned order, FLAC__bool do_full_decode);
static FLAC__bool read_subframe_lpc_(FLAC__StreamDecoder *decoder, unsigned channel, unsigned bps, const unsigned order, FLAC__bool do_full_decode);
static FLAC__bool read_subframe_verbatim_(FLAC__StreamDecoder *decoder, unsigned channel, unsigned bps, FLAC__bool do_full_decode);
static FLAC__bool read_residual_partitioned_rice_(FLAC__StreamDecoder *decoder, unsigned predictor_order, unsigned partition_order, FLAC__EntropyCodingMethod_PartitionedRiceContents *partitioned_rice_contents, FLAC__int32 *residual);
static FLAC__bool read_zero_padding_(FLAC__StreamDecoder *decoder);
static FLAC__bool read_callback_(FLAC__byte buffer[], unsigned *bytes, void *client_data);

#ifdef HALF_FRAME_BY_HALF_FRAME
static FLAC__bool post_read_frame_(FLAC__StreamDecoder *decoder, FLAC__bool *got_a_frame, FLAC__bool do_full_decode);
static FLAC__bool post_read_subframe_(FLAC__StreamDecoder *decoder, unsigned channel, unsigned bps, FLAC__bool do_full_decode);
static FLAC__bool post_read_subframe_constant_(FLAC__StreamDecoder *decoder, unsigned channel, unsigned bps, FLAC__bool do_full_decode);
static FLAC__bool post_read_subframe_fixed_(FLAC__StreamDecoder *decoder, unsigned channel, unsigned bps, unsigned order, FLAC__bool do_full_decode);
static FLAC__bool post_read_subframe_lpc_(FLAC__StreamDecoder *decoder, unsigned channel, unsigned bps, unsigned order, FLAC__bool do_full_decode);
static FLAC__bool post_read_subframe_verbatim_(FLAC__StreamDecoder *decoder, unsigned channel, unsigned bps, FLAC__bool do_full_decode);
static FLAC__bool post_read_residual_partitioned_rice_(FLAC__StreamDecoder *decoder, unsigned predictor_order, unsigned partition_order, FLAC__EntropyCodingMethod_PartitionedRiceContents *partitioned_rice_contents, FLAC__int32 *residual);
#endif


/***********************************************************************
 *
 * Private class data
 *
 ***********************************************************************/

typedef struct FLAC__StreamDecoderPrivate
{
    FLAC__StreamDecoderReadCallback read_callback;
    FLAC__StreamDecoderWriteCallback write_callback;
    FLAC__StreamDecoderMetadataCallback metadata_callback;
    FLAC__StreamDecoderErrorCallback error_callback;
    /* generic 32-bit datapath: */
    void (*local_lpc_restore_signal)(const FLAC__int32 residual[], unsigned data_len, const FLAC__int32 qlp_coeff[], unsigned order, int lp_quantization, FLAC__int32 data[]);
    /* generic 64-bit datapath: */
    void (*local_lpc_restore_signal_64bit)(const FLAC__int32 residual[], unsigned data_len, const FLAC__int32 qlp_coeff[], unsigned order, int lp_quantization, FLAC__int32 data[]);
    /* for use when the signal is <= 16 bits-per-sample, or <= 15 bits-per-sample on a side channel (which requires 1 extra bit): */
    void (*local_lpc_restore_signal_16bit)(const FLAC__int32 residual[], unsigned data_len, const FLAC__int32 qlp_coeff[], unsigned order, int lp_quantization, FLAC__int32 data[]);
    /* for use when the signal is <= 16 bits-per-sample, or <= 15 bits-per-sample on a side channel (which requires 1 extra bit), AND order <= 8: */
    void (*local_lpc_restore_signal_16bit_order8)(const FLAC__int32 residual[], unsigned data_len, const FLAC__int32 qlp_coeff[], unsigned order, int lp_quantization, FLAC__int32 data[]);
    void *client_data;
    FLAC__BitBuffer *input;
    FLAC__int32 *output[FLAC__MAX_CHANNELS];
    FLAC__int32 *residual[FLAC__MAX_CHANNELS]; /* WATCHOUT: these are the aligned pointers; the real pointers that should be flac_free()'d are residual_unaligned[] below */
    FLAC__EntropyCodingMethod_PartitionedRiceContents partitioned_rice_contents[FLAC__MAX_CHANNELS];
    unsigned output_capacity, output_channels;
    FLAC__uint32 last_frame_number;
    FLAC__uint32 last_block_size;
    FLAC__uint64 samples_decoded;
    FLAC__bool has_stream_info, has_seek_table;
    FLAC__StreamMetadata stream_info;
    FLAC__StreamMetadata seek_table;
    FLAC__bool metadata_filter[128]; /* MAGIC number 128 == total number of metadata block types == 1 << 7 */
    FLAC__byte *metadata_filter_ids;
    unsigned metadata_filter_ids_count, metadata_filter_ids_capacity; /* units for both are IDs, not bytes */
    FLAC__Frame frame;
    FLAC__bool cached; /* true if there is a byte in lookahead */
    FLAC__CPUInfo cpuinfo;
    FLAC__byte header_warmup[2]; /* contains the sync code and reserved bits */
    FLAC__byte lookahead; /* temp storage when we need to look ahead one byte in the stream */
    /* unaligned (original) pointers to allocated data */
    FLAC__int32 *residual_unaligned[FLAC__MAX_CHANNELS];
} FLAC__StreamDecoderPrivate;


#ifdef _DEBUG
extern int g_local_lpc_restore_signal_16bit_order8_been_called_times;
extern int g_local_lpc_restore_signal_16bit_been_called_times;
extern int g_local_lpc_restore_signal_been_called_times;
extern int g_local_lpc_restore_signal_64bit_been_called_times;
#endif

_ATTR_FLACDEC_BSS_
FLAC__StreamDecoder g_FLAC__StreamDecoder;
_ATTR_FLACDEC_BSS_
FLAC__StreamDecoderProtected g_FLAC__StreamDecoderProtected;
_ATTR_FLACDEC_BSS_
FLAC__StreamDecoderPrivate g_FLAC__StreamDecoderPrivate;
//FLAC__byte g_metadata_filter_ids[(FLAC__STREAM_METADATA_APPLICATION_ID_LEN/8)*16];
_ATTR_FLACDEC_BSS_
FLAC__byte g_metadata_filter_ids[(32/8)*16];

extern int g_f_FFW_FFD;
extern int g_num_of_seek_table_points;
extern int g_cur_used_seek_table_points;
extern FLAC__StreamMetadata_SeekPoint g_seek_table_points[MAX__SEEK_POINTS_NAMBLE];

/***********************************************************************
 *
 * Public static class data
 *
 ***********************************************************************/
_ATTR_FLACDEC_BSS_
FLAC_API char ** FLAC__StreamDecoderStateString;
/*
_ATTR_FLACDEC_DATA_
FLAC_API char * FLAC__StreamDecoderStateString[] =
{
    "FLAC__STREAM_DECODER_SEARCH_FOR_METADATA",
    "FLAC__STREAM_DECODER_READ_METADATA",
    "FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC",
    "FLAC__STREAM_DECODER_READ_FRAME",
    "FLAC__STREAM_DECODER_END_OF_STREAM",
    "FLAC__STREAM_DECODER_ABORTED",
    "FLAC__STREAM_DECODER_UNPARSEABLE_STREAM",
    "FLAC__STREAM_DECODER_MEMORY_ALLOCATION_ERROR",
    "FLAC__STREAM_DECODER_ALREADY_INITIALIZED",
    "FLAC__STREAM_DECODER_INVALID_CALLBACK",
    "FLAC__STREAM_DECODER_UNINITIALIZED"
};
*/

_ATTR_FLACDEC_BSS_
FLAC_API char ** FLAC__StreamDecoderReadStatusString;
/*
_ATTR_FLACDEC_DATA_
FLAC_API char * FLAC__StreamDecoderReadStatusString[] =
{
    "FLAC__STREAM_DECODER_READ_STATUS_CONTINUE",
    "FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM",
    "FLAC__STREAM_DECODER_READ_STATUS_ABORT"
};
*/

_ATTR_FLACDEC_BSS_
FLAC_API char ** FLAC__StreamDecoderWriteStatusString;
/*
_ATTR_FLACDEC_DATA_
FLAC_API char * FLAC__StreamDecoderWriteStatusString[] =
{
    "FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE",
    "FLAC__STREAM_DECODER_WRITE_STATUS_ABORT"
};
*/

_ATTR_FLACDEC_BSS_
FLAC_API char ** FLAC__StreamDecoderErrorStatusString;
/*
_ATTR_FLACDEC_DATA_
FLAC_API char * FLAC__StreamDecoderErrorStatusString[] =
{
    "FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC",
    "FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER",
    "FLAC__STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH"
};
*/
/**********************************************************************
*            global variables
************************************************************************/
#ifdef HALF_FRAME_BY_HALF_FRAME
_ATTR_FLACDEC_BSS_
FrameInfo SubframeInfo[FLAC__MAX_CHANNEL];
#endif

/***********************************************************************
 *
 * Class constructor/destructor
 *
 ***********************************************************************/
#if 0
FLAC_API FLAC__StreamDecoder *FLAC__stream_decoder_new()
{
    FLAC__StreamDecoder *decoder;
    unsigned i;

    FLAC__ASSERT(sizeof(int) >= 4); /* we want to die right away if this is not true */

    decoder = (FLAC__StreamDecoder*)calloc(1, sizeof(FLAC__StreamDecoder));
    if (decoder == 0)
    {
        return 0;
    }

    decoder->protected_ = (FLAC__StreamDecoderProtected*)calloc(1, sizeof(FLAC__StreamDecoderProtected));
    if (decoder->protected_ == 0)
    {
        flac_free(decoder);
        return 0;
    }

    decoder->private_ = (FLAC__StreamDecoderPrivate*)calloc(1, sizeof(FLAC__StreamDecoderPrivate));
    if (decoder->private_ == 0)
    {
        flac_free(decoder->protected_);
        flac_free(decoder);
        return 0;
    }

    decoder->private_->input = FLAC__bitbuffer_new();
    if (decoder->private_->input == 0)
    {
        flac_free(decoder->private_);
        flac_free(decoder->protected_);
        flac_free(decoder);
        return 0;
    }

    decoder->private_->metadata_filter_ids_capacity = 16;
    if (0 == (decoder->private_->metadata_filter_ids = (FLAC__byte*)flac_malloc((FLAC__STREAM_METADATA_APPLICATION_ID_LEN / 8) * decoder->private_->metadata_filter_ids_capacity)))
    {
        FLAC__bitbuffer_delete(decoder->private_->input);
        flac_free(decoder->private_);
        flac_free(decoder->protected_);
        flac_free(decoder);
        return 0;
    }

    for (i = 0; i < FLAC__MAX_CHANNELS; i++)
    {
        decoder->private_->output[i] = 0;
        decoder->private_->residual_unaligned[i] = decoder->private_->residual[i] = 0;
    }

    decoder->private_->output_capacity = 0;
    decoder->private_->output_channels = 0;
    decoder->private_->has_seek_table = false;

    for (i = 0; i < FLAC__MAX_CHANNELS; i++)
        FLAC__format_entropy_coding_method_partitioned_rice_contents_init(&decoder->private_->partitioned_rice_contents[i]);

    set_defaults_(decoder);

    decoder->protected_->state = FLAC__STREAM_DECODER_UNINITIALIZED;

    return decoder;
}
#else
_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__StreamDecoder *FLAC__stream_decoder_new()
{
    FLAC__StreamDecoder *decoder;
    unsigned i;

    FLAC__ASSERT(sizeof(int) >= 4); /* we want to die right away if this is not true */

    decoder = (FLAC__StreamDecoder*) & g_FLAC__StreamDecoder;
    memset(&g_FLAC__StreamDecoder, 0, sizeof(FLAC__StreamDecoder));

    decoder->protected_ = (FLAC__StreamDecoderProtected*) & g_FLAC__StreamDecoderProtected;
    memset(&g_FLAC__StreamDecoderProtected, 0, sizeof(FLAC__StreamDecoderProtected));

    decoder->private_ = (FLAC__StreamDecoderPrivate*) & g_FLAC__StreamDecoderPrivate;
    memset(&g_FLAC__StreamDecoderPrivate, 0, sizeof(FLAC__StreamDecoderPrivate));

    decoder->private_->input = FLAC__bitbuffer_new();

    decoder->private_->metadata_filter_ids_capacity = 16;
    decoder->private_->metadata_filter_ids = (FLAC__byte*)g_metadata_filter_ids;

    for (i = 0; i < FLAC__MAX_CHANNELS; i++)
    {
        decoder->private_->output[i] = 0;
        decoder->private_->residual_unaligned[i] = decoder->private_->residual[i] = 0;
    }

    decoder->private_->output_capacity = 0;
    decoder->private_->output_channels = 0;
    decoder->private_->has_seek_table = false;

    for (i = 0; i < FLAC__MAX_CHANNELS; i++)
        FLAC__format_entropy_coding_method_partitioned_rice_contents_init(&decoder->private_->partitioned_rice_contents[i]);

    set_defaults_(decoder);

    decoder->protected_->state = FLAC__STREAM_DECODER_UNINITIALIZED;

    return decoder;
}
#endif

/***********************************************************************
 *
 * Public class methods
 *
 ***********************************************************************/
_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__StreamDecoderState FLAC__stream_decoder_init(FLAC__StreamDecoder *decoder)
{
    FLAC__ASSERT(0 != decoder);

    if (decoder->protected_->state != FLAC__STREAM_DECODER_UNINITIALIZED)
        return decoder->protected_->state = FLAC__STREAM_DECODER_ALREADY_INITIALIZED;

    if (0 == decoder->private_->read_callback || 0 == decoder->private_->write_callback || 0 == decoder->private_->metadata_callback || 0 == decoder->private_->error_callback)
        return decoder->protected_->state = FLAC__STREAM_DECODER_INVALID_CALLBACK;

    if (!FLAC__bitbuffer_init(decoder->private_->input))
        return decoder->protected_->state = FLAC__STREAM_DECODER_MEMORY_ALLOCATION_ERROR;

    decoder->private_->last_frame_number = 0;
    decoder->private_->last_block_size = 0;
    decoder->private_->samples_decoded = 0;
    decoder->private_->has_stream_info = false;
    decoder->private_->cached = false;

    /*
     * get the CPU info and set the function pointers
     */
    //FLAC__cpu_info(&decoder->private_->cpuinfo);
    decoder->private_->cpuinfo.type = FLAC__CPUINFO_TYPE_UNKNOWN;
    decoder->private_->cpuinfo.use_asm = false;

    decoder->private_->local_lpc_restore_signal = FLAC__lpc_restore_signal;
    decoder->private_->local_lpc_restore_signal_64bit = FLAC__lpc_restore_signal_wide;
    decoder->private_->local_lpc_restore_signal_16bit = FLAC__lpc_restore_signal;
    decoder->private_->local_lpc_restore_signal_16bit_order8 = FLAC__lpc_restore_signal;

    if (!FLAC__stream_decoder_reset(decoder))
        return decoder->protected_->state = FLAC__STREAM_DECODER_MEMORY_ALLOCATION_ERROR;

    return decoder->protected_->state;
}

#if 0
FLAC_API void FLAC__stream_decoder_finish(FLAC__StreamDecoder *decoder)
{
    unsigned i;
    FLAC__ASSERT(0 != decoder);
    if (decoder->protected_->state == FLAC__STREAM_DECODER_UNINITIALIZED)
        return;
    if (0 != decoder->private_->seek_table.data.seek_table.points)
    {
        flac_free(decoder->private_->seek_table.data.seek_table.points);
        decoder->private_->seek_table.data.seek_table.points = 0;
        decoder->private_->has_seek_table = false;
    }
    FLAC__bitbuffer_free(decoder->private_->input);
    for (i = 0; i < FLAC__MAX_CHANNELS; i++)
    {
        /* WATCHOUT:
         * FLAC__lpc_restore_signal_asm_ia32_mmx() requires that the
         * output arrays have a buffer of up to 3 zeroes in front
         * (at negative indices) for alignment purposes; we use 4
         * to keep the data well-aligned.
         */
        if (0 != decoder->private_->output[i])
        {
            flac_free(decoder->private_->output[i] - 4);
            decoder->private_->output[i] = 0;
        }
        if (0 != decoder->private_->residual_unaligned[i])
        {
            flac_free(decoder->private_->residual_unaligned[i]);
            decoder->private_->residual_unaligned[i] = decoder->private_->residual[i] = 0;
        }
    }
    decoder->private_->output_capacity = 0;
    decoder->private_->output_channels = 0;

    set_defaults_(decoder);

    decoder->protected_->state = FLAC__STREAM_DECODER_UNINITIALIZED;
}
#else
_ATTR_FLACDEC_TEXT_
FLAC_API void FLAC__stream_decoder_finish(FLAC__StreamDecoder *decoder)
{
    //unsigned i;
    FLAC__ASSERT(0 != decoder);
    //if(decoder->protected_->state == FLAC__STREAM_DECODER_UNINITIALIZED)
    // return;
    decoder->protected_->state = FLAC__STREAM_DECODER_UNINITIALIZED;
}
#endif

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__bool FLAC__stream_decoder_set_read_callback(FLAC__StreamDecoder *decoder, FLAC__StreamDecoderReadCallback value)
{
    FLAC__ASSERT(0 != decoder);
    FLAC__ASSERT(0 != decoder->private_);
    FLAC__ASSERT(0 != decoder->protected_);
    if (decoder->protected_->state != FLAC__STREAM_DECODER_UNINITIALIZED)
        return false;
    decoder->private_->read_callback = value;
    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__bool FLAC__stream_decoder_set_write_callback(FLAC__StreamDecoder *decoder, FLAC__StreamDecoderWriteCallback value)
{
    FLAC__ASSERT(0 != decoder);
    FLAC__ASSERT(0 != decoder->private_);
    FLAC__ASSERT(0 != decoder->protected_);
    if (decoder->protected_->state != FLAC__STREAM_DECODER_UNINITIALIZED)
        return false;
    decoder->private_->write_callback = value;
    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__bool FLAC__stream_decoder_set_metadata_callback(FLAC__StreamDecoder *decoder, FLAC__StreamDecoderMetadataCallback value)
{
    FLAC__ASSERT(0 != decoder);
    FLAC__ASSERT(0 != decoder->private_);
    FLAC__ASSERT(0 != decoder->protected_);
    if (decoder->protected_->state != FLAC__STREAM_DECODER_UNINITIALIZED)
        return false;
    decoder->private_->metadata_callback = value;
    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__bool FLAC__stream_decoder_set_error_callback(FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorCallback value)
{
    FLAC__ASSERT(0 != decoder);
    FLAC__ASSERT(0 != decoder->private_);
    FLAC__ASSERT(0 != decoder->protected_);
    if (decoder->protected_->state != FLAC__STREAM_DECODER_UNINITIALIZED)
        return false;
    decoder->private_->error_callback = value;
    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__bool FLAC__stream_decoder_set_client_data(FLAC__StreamDecoder *decoder, void *value)
{
    FLAC__ASSERT(0 != decoder);
    FLAC__ASSERT(0 != decoder->private_);
    FLAC__ASSERT(0 != decoder->protected_);
    if (decoder->protected_->state != FLAC__STREAM_DECODER_UNINITIALIZED)
        return false;
    decoder->private_->client_data = value;
    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__bool FLAC__stream_decoder_set_metadata_respond(FLAC__StreamDecoder *decoder, FLAC__MetadataType type)
{
    FLAC__ASSERT(0 != decoder);
    FLAC__ASSERT(0 != decoder->private_);
    FLAC__ASSERT(0 != decoder->protected_);
    FLAC__ASSERT((unsigned)type <= FLAC__MAX_METADATA_TYPE_CODE);
    /* double protection */
    //if((unsigned)type > FLAC__MAX_METADATA_TYPE_CODE)
    // return false;
    //if(decoder->protected_->state != FLAC__STREAM_DECODER_UNINITIALIZED)
    // return false;
    decoder->private_->metadata_filter[type] = true;
    if (type == FLAC__METADATA_TYPE_APPLICATION)
        decoder->private_->metadata_filter_ids_count = 0;
    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__StreamDecoderState FLAC__stream_decoder_get_state(const FLAC__StreamDecoder *decoder)
{
    FLAC__ASSERT(0 != decoder);
    FLAC__ASSERT(0 != decoder->protected_);
    return decoder->protected_->state;
}

_ATTR_FLACDEC_TEXT_
FLAC_API const char *FLAC__stream_decoder_get_resolved_state_string(const FLAC__StreamDecoder *decoder)
{
    return FLAC__StreamDecoderStateString[decoder->protected_->state];
}

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__bool FLAC__stream_decoder_flush(FLAC__StreamDecoder *decoder)
{
    FLAC__ASSERT(0 != decoder);
    FLAC__ASSERT(0 != decoder->private_);
    FLAC__ASSERT(0 != decoder->protected_);

    if (!FLAC__bitbuffer_clear(decoder->private_->input))
    {
        decoder->protected_->state = FLAC__STREAM_DECODER_MEMORY_ALLOCATION_ERROR;
        return false;
    }
    decoder->private_->last_frame_number = 0;
    decoder->private_->last_block_size = 0;
    decoder->protected_->state = FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;

    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__bool FLAC__stream_decoder_reset(FLAC__StreamDecoder *decoder)
{
    FLAC__ASSERT(0 != decoder);
    FLAC__ASSERT(0 != decoder->private_);
    FLAC__ASSERT(0 != decoder->protected_);

    if (!FLAC__stream_decoder_flush(decoder))
    {
        decoder->protected_->state = FLAC__STREAM_DECODER_MEMORY_ALLOCATION_ERROR;
        return false;
    }
    decoder->protected_->state = FLAC__STREAM_DECODER_SEARCH_FOR_METADATA;

    decoder->private_->samples_decoded = 0;

    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__bool FLAC__stream_decoder_process_until_end_of_metadata(FLAC__StreamDecoder *decoder)
{
    FLAC__ASSERT(0 != decoder);
    FLAC__ASSERT(0 != decoder->protected_);

    if (!judge_file_type(decoder))    //add by evan wu
        return false; /* above function sets the status for us */

    while (1)
    {
        switch (decoder->protected_->state)
        {
            case FLAC__STREAM_DECODER_SEARCH_FOR_METADATA:
                if (!find_metadata_(decoder))
                    return false; /* above function sets the status for us */
                break;
            case FLAC__STREAM_DECODER_READ_METADATA:
                if (!read_metadata_(decoder))
                    return false; /* above function sets the status for us */
                break;
            case FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC:
            case FLAC__STREAM_DECODER_READ_FRAME:
            case FLAC__STREAM_DECODER_END_OF_STREAM:
            case FLAC__STREAM_DECODER_ABORTED:
                return true;
            default:
                FLAC__ASSERT(0);
                return false;
        }
    }
}


extern int  g_iFlacStreamDecoderState ;
_ATTR_FLACDEC_BSS_
int cnt;

extern int  g_isGotFrame;

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__bool FLAC__stream_decoder_process_until_end_of_stream(FLAC__StreamDecoder *decoder)
{
    FLAC__bool dummy;


    FLAC__ASSERT(0 != decoder);
    FLAC__ASSERT(0 != decoder->protected_);

    while (1)
    {
        //
        g_iFlacStreamDecoderState = decoder->protected_->state ;

        switch (decoder->protected_->state)
        {
            case FLAC__STREAM_DECODER_SEARCH_FOR_METADATA:
                if (!find_metadata_(decoder))
                    return false; /* above function sets the status for us */
                break;
            case FLAC__STREAM_DECODER_READ_METADATA:
                if (!read_metadata_(decoder))
                    return false; /* above function sets the status for us */
                break;
            case FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC:
                if (!frame_sync_(decoder))
                {
                    g_iFlacStreamDecoderState = decoder->protected_->state ;
                    return true; /* above function sets the status for us */
                }
                break;
            case FLAC__STREAM_DECODER_READ_FRAME:
                if (!read_frame_(decoder, &dummy, /*do_full_decode=*/true))
                {
                    g_iFlacStreamDecoderState = decoder->protected_->state ;

                    //BY VINCENT HSIUNG , APR 28 , 2009
                    g_isGotFrame = dummy;

                    return false; /* above function sets the status for us */
                }

                //done
                else
                {
                    g_iFlacStreamDecoderState = decoder->protected_->state ;

                    //BY VINCENT HSIUNG , APR 28 , 2009
                    g_isGotFrame = dummy;

                    return true ;
                }


                break;
            case FLAC__STREAM_DECODER_END_OF_STREAM:
            case FLAC__STREAM_DECODER_ABORTED:
                return true;
            default:
                FLAC__ASSERT(0);
                return false;
        }
    }
}
/*************************************************************/
_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__bool post_FLAC__stream_decoder_process_until_end_of_stream(FLAC__StreamDecoder *decoder)
{
    FLAC__bool dummy;
    if (!post_read_frame_(decoder, &dummy, /*do_full_decode=*/true))
    {
        g_iFlacStreamDecoderState = decoder->protected_->state ;
        return false; /* above function sets the status for us */
    }
    return true;
}

/***********************************************************************
 *
 * Protected class methods
 *
 ***********************************************************************/


/***********************************************************************
 *
 * Private class methods
 *
 ***********************************************************************/
_ATTR_FLACDEC_TEXT_
void set_defaults_(FLAC__StreamDecoder *decoder)
{
    decoder->private_->read_callback = 0;
    decoder->private_->write_callback = 0;
    decoder->private_->metadata_callback = 0;
    decoder->private_->error_callback = 0;
    decoder->private_->client_data = 0;

    memset(decoder->private_->metadata_filter, 0, sizeof(decoder->private_->metadata_filter));
    decoder->private_->metadata_filter[FLAC__METADATA_TYPE_STREAMINFO] = true;
    decoder->private_->metadata_filter_ids_count = 0;
}

#if 0
FLAC__bool allocate_output_(FLAC__StreamDecoder *decoder, unsigned size, unsigned channels)
{
    unsigned i;
    FLAC__int32 *tmp;

    if (size <= decoder->private_->output_capacity && channels <= decoder->private_->output_channels)
        return true;

    /* simply using realloc() is not practical because the number of channels may change mid-stream */

    for (i = 0; i < FLAC__MAX_CHANNELS; i++)
    {
        if (0 != decoder->private_->output[i])
        {
            flac_free(decoder->private_->output[i] - 4);
            decoder->private_->output[i] = 0;
        }
        if (0 != decoder->private_->residual_unaligned[i])
        {
            flac_free(decoder->private_->residual_unaligned[i]);
            decoder->private_->residual_unaligned[i] = decoder->private_->residual[i] = 0;
        }
    }

    for (i = 0; i < channels; i++)
    {
        /* WATCHOUT:
         * FLAC__lpc_restore_signal_asm_ia32_mmx() requires that the
         * output arrays have a buffer of up to 3 zeroes in front
         * (at negative indices) for alignment purposes; we use 4
         * to keep the data well-aligned.
         */
        tmp = (FLAC__int32*)flac_malloc(sizeof(FLAC__int32) * (size + 4));
        if (tmp == 0)
        {
            decoder->protected_->state = FLAC__STREAM_DECODER_MEMORY_ALLOCATION_ERROR;
            return false;
        }
        memset(tmp, 0, sizeof(FLAC__int32)*4);
        decoder->private_->output[i] = tmp + 4;

        /* WATCHOUT:
         * minimum of quadword alignment for PPC vector optimizations is REQUIRED:
         */
        if (!FLAC__memory_alloc_aligned_int32_array(size, &decoder->private_->residual_unaligned[i], &decoder->private_->residual[i]))
        {
            decoder->protected_->state = FLAC__STREAM_DECODER_MEMORY_ALLOCATION_ERROR;
            return false;
        }
    }

    decoder->private_->output_capacity = size;
    decoder->private_->output_channels = channels;

    return true;
}
#else
_ATTR_FLACDEC_TEXT_
FLAC__bool allocate_output_(FLAC__StreamDecoder *decoder, unsigned size, unsigned channels)
{
    unsigned i;

    if (size > FLAC__PCMBUFFER_DEFAULT_CAPACITY)
    {
        //DEBUG("size = %d\n", size);
        FLAC__ASSERT(size <= FLAC__PCMBUFFER_DEFAULT_CAPACITY);
        return false;
    }

    /* PCMout Buffer 与 residual Buffer 共用 */

    for (i = 0; i < channels; i++)
    {
        decoder->private_->output[i] = (FLAC__int32 *)(g_FlacOutputBuffer[i] + FLAC__MAX_LPC_ORDER);

        //decoder->private_->residual_unaligned[i] = decoder->private_->output[i];
        decoder->private_->residual[i] = decoder->private_->output[i];
    }


    decoder->private_->output_capacity = FLAC__PCMBUFFER_DEFAULT_CAPACITY;
    decoder->private_->output_channels = channels;

    return true;
}
#endif

_ATTR_FLACDEC_TEXT_
FLAC__bool has_id_filtered_(FLAC__StreamDecoder *decoder, FLAC__byte *id)
{
    unsigned i;

    FLAC__ASSERT(0 != decoder);
    FLAC__ASSERT(0 != decoder->private_);

    for (i = 0; i < decoder->private_->metadata_filter_ids_count; i++)
        if (0 == memcmp(decoder->private_->metadata_filter_ids + i * (FLAC__STREAM_METADATA_APPLICATION_ID_LEN / 8), id, (FLAC__STREAM_METADATA_APPLICATION_ID_LEN / 8)))
            return true;

    return false;
}
//add by evan wu
_ATTR_FLACDEC_TEXT_
FLAC__bool judge_file_type(FLAC__StreamDecoder *decoder)
{
    FLAC__uint32 x;
    unsigned i;

    FLAC__ASSERT(FLAC__bitbuffer_is_consumed_byte_aligned(decoder->private_->input));

    for (i = 0; i < 4;i++)
    {
        if (decoder->private_->cached)
        {
            x = (FLAC__uint32)decoder->private_->lookahead;
            decoder->private_->cached = false;
        }
        else
        {
            if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &x, 8, read_callback_, decoder))
                return false; /* the read_callback_ sets the state for us */
        }
        if (x == FLAC__STREAM_SYNC_STRING[i])
        {
            continue;
        }
        else
        {
            return false;
        }
    }
    decoder->protected_->state = FLAC__STREAM_DECODER_READ_METADATA;
    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC__bool find_metadata_(FLAC__StreamDecoder *decoder)
{
    FLAC__uint32 x;
    unsigned i, id;
    FLAC__bool first = false;

    FLAC__ASSERT(FLAC__bitbuffer_is_consumed_byte_aligned(decoder->private_->input));

    for (i = id = 0; i < 4;)
    {
        if (decoder->private_->cached)
        {
            x = (FLAC__uint32)decoder->private_->lookahead;
            decoder->private_->cached = false;
        }
        else
        {
            if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &x, 8, read_callback_, decoder))
                return false; /* the read_callback_ sets the state for us */
        }
        if (x == FLAC__STREAM_SYNC_STRING[i])
        {
            first = true;
            i++;
            id = 0;
            continue;
        }
        /*if (x == ID3V2_TAG_[id])
        {
            flac_DEBUG("%c\n",x);
            id++;
            i = 0;
            if (id == 3)
            {
               if (!skip_id3v2_tag_(decoder))
                  id = 0;//{flac_DEBUG();return false;} // the read_callback_ sets the state for us

            }
            continue;
        }*/
        if (x == 0xff)  /* MAGIC NUMBER for the first 8 frame sync bits */
        {
            decoder->private_->header_warmup[0] = (FLAC__byte)x;
            if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &x, 8, read_callback_, decoder))
                return false; /* the read_callback_ sets the state for us */

            /* we have to check if we just read two 0xff's in a row; the second may actually be the beginning of the sync code */
            /* else we have to check if the second byte is the end of a sync code */
            if (x == 0xff)  /* MAGIC NUMBER for the first 8 frame sync bits */
            {
                decoder->private_->lookahead = (FLAC__byte)x;
                decoder->private_->cached = true;
            }
            else if (x >> 2 == 0x3e)  /* MAGIC NUMBER for the last 6 sync bits */
            {
                decoder->private_->header_warmup[1] = (FLAC__byte)x;
                decoder->protected_->state = FLAC__STREAM_DECODER_READ_FRAME;
                return true;
            }
        }
        i = 0;
        if (first)
        {
            decoder->private_->error_callback(decoder, FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC, decoder->private_->client_data);
            first = false;
        }
    }

    decoder->protected_->state = FLAC__STREAM_DECODER_READ_METADATA;
    return true;
}

_ATTR_FLACDEC_DATA_ int metadata_size = 0;
_ATTR_FLACDEC_TEXT_
FLAC__bool read_metadata_(FLAC__StreamDecoder *decoder)
{
    FLAC__bool is_last;
    FLAC__uint32 x, type, length;

    FLAC__ASSERT(FLAC__bitbuffer_is_consumed_byte_aligned(decoder->private_->input));

    // read 1 bit,
    // 1:表示这是最后一个metadata，接下来将是Audio data
    // 0:表示接下来还有metadata
    if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &x, FLAC__STREAM_METADATA_IS_LAST_LEN, read_callback_, decoder))
        return false; /* the read_callback_ sets the state for us */
    is_last = x ? true : false;

    // read 7 bits, metadata的类型
    if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &type, FLAC__STREAM_METADATA_TYPE_LEN, read_callback_, decoder))
        return false; /* the read_callback_ sets the state for us */

    // read 24 bits, metadata的长度
    if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &length, FLAC__STREAM_METADATA_LENGTH_LEN, read_callback_, decoder))
        return false; /* the read_callback_ sets the state for us */
    metadata_size += length;

    if (type == FLAC__METADATA_TYPE_STREAMINFO)
    {
        if (!read_metadata_streaminfo_(decoder, is_last, length))
            return false;

        decoder->private_->has_stream_info = true;
        if (decoder->private_->metadata_filter[FLAC__METADATA_TYPE_STREAMINFO])
            decoder->private_->metadata_callback(decoder, &decoder->private_->stream_info, decoder->private_->client_data);
    }
    else if (type == FLAC__METADATA_TYPE_SEEKTABLE)
    {
        if (!read_metadata_seektable_(decoder, is_last, length))
        {
           //return false;
           //DEBUG("seektable is too large or not exist\n");
           decoder->protected_->state = FLAC__STREAM_DECODER_SEARCH_FOR_METADATA;
           return true;
        }


        decoder->private_->has_seek_table = true;
        if (decoder->private_->metadata_filter[FLAC__METADATA_TYPE_SEEKTABLE])
            decoder->private_->metadata_callback(decoder, &decoder->private_->seek_table, decoder->private_->client_data);
    }
    else
    {
        FLAC__bool skip_it = !decoder->private_->metadata_filter[type];
        unsigned real_length = length;
        FLAC__StreamMetadata block;

        block.is_last = is_last;
        block.type = (FLAC__MetadataType)type;
        block.length = length;

#if 0
        if (type == FLAC__METADATA_TYPE_APPLICATION)
        {
            if (!FLAC__bitbuffer_read_byte_block_aligned_no_crc(decoder->private_->input, block.data.application.id, FLAC__STREAM_METADATA_APPLICATION_ID_LEN / 8, read_callback_, decoder))
                return false; /* the read_callback_ sets the state for us */

            real_length -= FLAC__STREAM_METADATA_APPLICATION_ID_LEN / 8;

            if (decoder->private_->metadata_filter_ids_count > 0 && has_id_filtered_(decoder, block.data.application.id))
                skip_it = !skip_it;
        }
#endif

        if (type == FLAC__METADATA_TYPE_VORBIS_COMMENT)
        {
            skip_it = 0; // 不能跳过这个meta，需要从中读取 ID3 信息
        }

        if (skip_it)
        {
            if (!FLAC__bitbuffer_read_byte_block_aligned_no_crc(decoder->private_->input, 0, real_length, read_callback_, decoder))
                return false; /* the read_callback_ sets the state for us */
        }
        else
        {
            switch (type)
            {
#if 0
                case FLAC__METADATA_TYPE_PADDING:
                    /* skip the padding bytes */
                    if (!FLAC__bitbuffer_read_byte_block_aligned_no_crc(decoder->private_->input, 0, real_length, read_callback_, decoder))
                        return false; /* the read_callback_ sets the state for us */
                    break;
                case FLAC__METADATA_TYPE_APPLICATION:
                    /* remember, we read the ID already */
                    if (real_length > 0)
                    {
                        if (0 == (block.data.application.data = (FLAC__byte*)flac_malloc(real_length)))
                        {
                            decoder->protected_->state = FLAC__STREAM_DECODER_MEMORY_ALLOCATION_ERROR;
                            return false;
                        }
                        if (!FLAC__bitbuffer_read_byte_block_aligned_no_crc(decoder->private_->input, block.data.application.data, real_length, read_callback_, decoder))
                            return false; /* the read_callback_ sets the state for us */
                    }
                    else
                        block.data.application.data = 0;
                    break;
#endif
                case FLAC__METADATA_TYPE_VORBIS_COMMENT:
                    if (!read_metadata_vorbiscomment_(decoder, &block.data.vorbis_comment))
                        return false;
                    skip_it = 1;
                    break;
#if 0
                case FLAC__METADATA_TYPE_CUESHEET:
                    if (!read_metadata_cuesheet_(decoder, &block.data.cue_sheet))
                        return false;
                    break;
                case FLAC__METADATA_TYPE_STREAMINFO:
                case FLAC__METADATA_TYPE_SEEKTABLE:
                    FLAC__ASSERT(0);
                    break;
                default:
                    if (real_length > 0)
                    {
                        if (0 == (block.data.unknown.data = (FLAC__byte*)flac_malloc(real_length)))
                        {
                            decoder->protected_->state = FLAC__STREAM_DECODER_MEMORY_ALLOCATION_ERROR;
                            return false;
                        }
                        if (!FLAC__bitbuffer_read_byte_block_aligned_no_crc(decoder->private_->input, block.data.unknown.data, real_length, read_callback_, decoder))
                            return false; /* the read_callback_ sets the state for us */
                    }
                    else
                        block.data.unknown.data = 0;
                    break;
#else
                default:
                    break;
#endif
            }
        }
    }

    if (is_last)
        decoder->protected_->state = FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;

    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC__bool read_metadata_streaminfo_(FLAC__StreamDecoder *decoder, FLAC__bool is_last, unsigned length)
{
    FLAC__uint32 x;
    unsigned bits, used_bits = 0;

    FLAC__ASSERT(FLAC__bitbuffer_is_consumed_byte_aligned(decoder->private_->input));

    decoder->private_->stream_info.type = FLAC__METADATA_TYPE_STREAMINFO;
    decoder->private_->stream_info.is_last = is_last;
    decoder->private_->stream_info.length = length;

    // 最小blocksize
    bits = FLAC__STREAM_METADATA_STREAMINFO_MIN_BLOCK_SIZE_LEN;
    if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &x, bits, read_callback_, decoder))
        return false; /* the read_callback_ sets the state for us */
    decoder->private_->stream_info.data.stream_info.min_blocksize = x;
    used_bits += bits;

    // 最大blocksize
    bits = FLAC__STREAM_METADATA_STREAMINFO_MAX_BLOCK_SIZE_LEN;
    if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &x, FLAC__STREAM_METADATA_STREAMINFO_MAX_BLOCK_SIZE_LEN, read_callback_, decoder))
        return false; /* the read_callback_ sets the state for us */
    decoder->private_->stream_info.data.stream_info.max_blocksize = x;
    used_bits += bits;

    if (decoder->private_->stream_info.data.stream_info.min_blocksize != decoder->private_->stream_info.data.stream_info.max_blocksize)
    {
        // 我们只支持固定的blocksize
        return false;
    }
    g_flac_decoder_info.blocksize = x;

    // 最小framesize
    bits = FLAC__STREAM_METADATA_STREAMINFO_MIN_FRAME_SIZE_LEN;
    if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &x, FLAC__STREAM_METADATA_STREAMINFO_MIN_FRAME_SIZE_LEN, read_callback_, decoder))
        return false; /* the read_callback_ sets the state for us */
    decoder->private_->stream_info.data.stream_info.min_framesize = x;
    g_flac_decoder_info.min_framesize = x;
    used_bits += bits;

    // 最大framesize
    bits = FLAC__STREAM_METADATA_STREAMINFO_MAX_FRAME_SIZE_LEN;
    if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &x, FLAC__STREAM_METADATA_STREAMINFO_MAX_FRAME_SIZE_LEN, read_callback_, decoder))
        return false; /* the read_callback_ sets the state for us */
    decoder->private_->stream_info.data.stream_info.max_framesize = x;
     g_flac_decoder_info.max_framesize = x;
    used_bits += bits;

    // 采样率
    bits = FLAC__STREAM_METADATA_STREAMINFO_SAMPLE_RATE_LEN;
    if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &x, FLAC__STREAM_METADATA_STREAMINFO_SAMPLE_RATE_LEN, read_callback_, decoder))
        return false; /* the read_callback_ sets the state for us */
    decoder->private_->stream_info.data.stream_info.sample_rate = x;
    g_flac_decoder_info.sample_rate = x;
    used_bits += bits;

    // channel
    bits = FLAC__STREAM_METADATA_STREAMINFO_CHANNELS_LEN;
    if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &x, FLAC__STREAM_METADATA_STREAMINFO_CHANNELS_LEN, read_callback_, decoder))
        return false; /* the read_callback_ sets the state for us */
    decoder->private_->stream_info.data.stream_info.channels = x + 1;
    g_flac_decoder_info.channels = x + 1;
    used_bits += bits;

    if (decoder->private_->stream_info.data.stream_info.channels > 2)
    {
        return false;
    }

    // 每个样点的bit 数
    bits = FLAC__STREAM_METADATA_STREAMINFO_BITS_PER_SAMPLE_LEN;
    if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &x, FLAC__STREAM_METADATA_STREAMINFO_BITS_PER_SAMPLE_LEN, read_callback_, decoder))
        return false; /* the read_callback_ sets the state for us */
    decoder->private_->stream_info.data.stream_info.bits_per_sample = x + 1;
    g_flac_decoder_info.bits_per_sample = x + 1;
    used_bits += bits;

    if (decoder->private_->stream_info.data.stream_info.bits_per_sample != 16)
    {
        return false;
    }

    // 总的样点数
    bits = FLAC__STREAM_METADATA_STREAMINFO_TOTAL_SAMPLES_LEN;
    //if(!FLAC__bitbuffer_read_raw_uint64(decoder->private_->input, &decoder->private_->stream_info.data.stream_info.total_samples, FLAC__STREAM_METADATA_STREAMINFO_TOTAL_SAMPLES_LEN, read_callback_, decoder))
    // return false; /* the read_callback_ sets the state for us */
    //used_bits += bits;

    // 先读 4 bits
    if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &decoder->private_->stream_info.data.stream_info.total_samples, 4, read_callback_, decoder))
        return false; /* the read_callback_ sets the state for us */

    if (decoder->private_->stream_info.data.stream_info.total_samples > 0)
    {
        // 数据太大，32 位表示不下来
        FLAC__ASSERT(0);
        return false;
    }

    //再读剩下的 32 bits
    if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &decoder->private_->stream_info.data.stream_info.total_samples, FLAC__STREAM_METADATA_STREAMINFO_TOTAL_SAMPLES_LEN - 4, read_callback_, decoder))
        return false; /* the read_callback_ sets the state for us */
    g_flac_decoder_info.total_samples = decoder->private_->stream_info.data.stream_info.total_samples;
    used_bits += bits;

    // md5 值
    if (!FLAC__bitbuffer_read_byte_block_aligned_no_crc(decoder->private_->input, decoder->private_->stream_info.data.stream_info.md5sum, 16, read_callback_, decoder))
        return false; /* the read_callback_ sets the state for us */
    used_bits += 16 * 8;

    /* skip the rest of the block */
    FLAC__ASSERT(used_bits % 8 == 0);
    length -= (used_bits / 8);
    if (!FLAC__bitbuffer_read_byte_block_aligned_no_crc(decoder->private_->input, 0, length, read_callback_, decoder))
        return false; /* the read_callback_ sets the state for us */

    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC__bool read_metadata_seektable_(FLAC__StreamDecoder *decoder, FLAC__bool is_last, unsigned length)
{
    FLAC__uint32 i, x;
    FLAC__uint64 xx;

    FLAC__ASSERT(FLAC__bitbuffer_is_consumed_byte_aligned(decoder->private_->input));

    decoder->private_->seek_table.type = FLAC__METADATA_TYPE_SEEKTABLE;
    decoder->private_->seek_table.is_last = is_last;
    decoder->private_->seek_table.length = length;

    decoder->private_->seek_table.data.seek_table.num_points = length / FLAC__STREAM_METADATA_SEEKPOINT_LENGTH;

    /* use realloc since we may pass through here several times (e.g. after seeking) */
    /*if(0 == (decoder->private_->seek_table.data.seek_table.points = (FLAC__StreamMetadata_SeekPoint*)realloc(decoder->private_->seek_table.data.seek_table.points, decoder->private_->seek_table.data.seek_table.num_points * sizeof(FLAC__StreamMetadata_SeekPoint)))) {
     decoder->protected_->state = FLAC__STREAM_DECODER_MEMORY_ALLOCATION_ERROR;
     return false;
    }*/

    if (decoder->private_->seek_table.data.seek_table.num_points > MAX__SEEK_POINTS_NAMBLE)
    {
        //DEBUG("num_points = %d\n", decoder->private_->seek_table.data.seek_table.num_points);
        decoder->private_->seek_table.data.seek_table.num_points = MAX__SEEK_POINTS_NAMBLE;
        //FLAC__ASSERT(decoder->private_->seek_table.data.seek_table.num_points <= MAX__SEEK_POINTS_NAMBLE);
    }

    g_num_of_seek_table_points = 0;//decoder->private_->seek_table.data.seek_table.num_points;
    decoder->private_->seek_table.data.seek_table.points = (FLAC__StreamMetadata_SeekPoint*)g_seek_table_points;

    for (i = 0; i < decoder->private_->seek_table.data.seek_table.num_points; i++)
    {
        if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &xx, FLAC__STREAM_METADATA_SEEKPOINT_SAMPLE_NUMBER_LEN / 2, read_callback_, decoder))
            return false; /* the read_callback_ sets the state for us */

        /*if (xx&0xffffffff > 0)
        {
            FLAC__ASSERT(0); // 数据太大，32位表示不下
            return false;
        }*/

        if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &xx, FLAC__STREAM_METADATA_SEEKPOINT_SAMPLE_NUMBER_LEN / 2, read_callback_, decoder))
            return false; /* the read_callback_ sets the state for us */
        decoder->private_->seek_table.data.seek_table.points[i].sample_number = xx;

        if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &xx, FLAC__STREAM_METADATA_SEEKPOINT_STREAM_OFFSET_LEN / 2, read_callback_, decoder))
            return false; /* the read_callback_ sets the state for us */

       /* if (xx&0xffffffff > 0)
        {
            FLAC__ASSERT(0); // 数据太大，32位表示不下
            return false;
        }*/

        if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &xx, FLAC__STREAM_METADATA_SEEKPOINT_STREAM_OFFSET_LEN / 2, read_callback_, decoder))
            return false; /* the read_callback_ sets the state for us */
        decoder->private_->seek_table.data.seek_table.points[i].stream_offset = xx;

        if ((xx > 0) || (i == 0))
        {
            // seek_table 里面的数据可能为零，因此我们不必去读这些数据
            g_num_of_seek_table_points++;
        }

        if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &x, FLAC__STREAM_METADATA_SEEKPOINT_FRAME_SAMPLES_LEN, read_callback_, decoder))
            return false; /* the read_callback_ sets the state for us */
        //decoder->private_->seek_table.data.seek_table.points[i].frame_samples = x;
    }

    if (g_num_of_seek_table_points > MAX__SEEK_POINTS_NAMBLE)
        g_num_of_seek_table_points = MAX__SEEK_POINTS_NAMBLE;

    length -= (decoder->private_->seek_table.data.seek_table.num_points * FLAC__STREAM_METADATA_SEEKPOINT_LENGTH);
    /* if there is a partial point left, skip over it */
    if (length > 0)
    {
        /*@@@ do an error_callback() here?  there's an argument for either way */
        if (!FLAC__bitbuffer_read_byte_block_aligned_no_crc(decoder->private_->input, 0, length, read_callback_, decoder))
            return false; /* the read_callback_ sets the state for us */
    }

    return true;
}

#if 0
FLAC__bool read_metadata_vorbiscomment_(FLAC__StreamDecoder *decoder, FLAC__StreamMetadata_VorbisComment *obj)
{
    FLAC__uint32 i;

    FLAC__ASSERT(FLAC__bitbuffer_is_consumed_byte_aligned(decoder->private_->input));

    /* read vendor string */
    FLAC__ASSERT(FLAC__STREAM_METADATA_VORBIS_COMMENT_ENTRY_LENGTH_LEN == 32);
    if (!FLAC__bitbuffer_read_raw_uint32_little_endian(decoder->private_->input, &obj->vendor_string.length, read_callback_, decoder))
        return false; /* the read_callback_ sets the state for us */
    if (obj->vendor_string.length > 0)
    {
        if (0 == (obj->vendor_string.entry = (FLAC__byte*)flac_malloc(obj->vendor_string.length + 1)))
        {
            decoder->protected_->state = FLAC__STREAM_DECODER_MEMORY_ALLOCATION_ERROR;
            return false;
        }
        if (!FLAC__bitbuffer_read_byte_block_aligned_no_crc(decoder->private_->input, obj->vendor_string.entry, obj->vendor_string.length, read_callback_, decoder))
            return false; /* the read_callback_ sets the state for us */
        obj->vendor_string.entry[obj->vendor_string.length] = '\0';
    }
    else
        obj->vendor_string.entry = 0;

    /* read num comments */
    FLAC__ASSERT(FLAC__STREAM_METADATA_VORBIS_COMMENT_NUM_COMMENTS_LEN == 32);
    if (!FLAC__bitbuffer_read_raw_uint32_little_endian(decoder->private_->input, &obj->num_comments, read_callback_, decoder))
        return false; /* the read_callback_ sets the state for us */

    /* read comments */
    if (obj->num_comments > 0)
    {
        if (0 == (obj->comments = (FLAC__StreamMetadata_VorbisComment_Entry*)flac_malloc(obj->num_comments * sizeof(FLAC__StreamMetadata_VorbisComment_Entry))))
        {
            decoder->protected_->state = FLAC__STREAM_DECODER_MEMORY_ALLOCATION_ERROR;
            return false;
        }
        for (i = 0; i < obj->num_comments; i++)
        {
            FLAC__ASSERT(FLAC__STREAM_METADATA_VORBIS_COMMENT_ENTRY_LENGTH_LEN == 32);
            if (!FLAC__bitbuffer_read_raw_uint32_little_endian(decoder->private_->input, &obj->comments[i].length, read_callback_, decoder))
                return false; /* the read_callback_ sets the state for us */
            if (obj->comments[i].length > 0)
            {
                if (0 == (obj->comments[i].entry = (FLAC__byte*)flac_malloc(obj->comments[i].length + 1)))
                {
                    decoder->protected_->state = FLAC__STREAM_DECODER_MEMORY_ALLOCATION_ERROR;
                    return false;
                }
                if (!FLAC__bitbuffer_read_byte_block_aligned_no_crc(decoder->private_->input, obj->comments[i].entry, obj->comments[i].length, read_callback_, decoder))
                    return false; /* the read_callback_ sets the state for us */
                obj->comments[i].entry[obj->comments[i].length] = '\0';
            }
            else
                obj->comments[i].entry = 0;
        }
    }
    else
    {
        obj->comments = 0;
    }

    return true;
}
#else
#define MAX_COMMENT_LENGTH 64
_ATTR_FLACDEC_TEXT_
FLAC__bool read_metadata_vorbiscomment_(FLAC__StreamDecoder *decoder, FLAC__StreamMetadata_VorbisComment *obj)
{
    FLAC__uint32 i;
    FLAC__uint32 comment_length;
    FLAC__byte strings[MAX_COMMENT_LENGTH+1];
    FLAC__uint32 disp_length;

    FLAC__ASSERT(FLAC__bitbuffer_is_consumed_byte_aligned(decoder->private_->input));

    /* read vendor string */
    FLAC__ASSERT(FLAC__STREAM_METADATA_VORBIS_COMMENT_ENTRY_LENGTH_LEN == 32);
    if (!FLAC__bitbuffer_read_raw_uint32_little_endian(decoder->private_->input, &obj->vendor_string.length, read_callback_, decoder))
        return false; /* the read_callback_ sets the state for us */
    if (obj->vendor_string.length > 0)
    {
        if (!FLAC__bitbuffer_read_byte_block_aligned_no_crc(decoder->private_->input, NULL, obj->vendor_string.length, read_callback_, decoder))
            return false; /* the read_callback_ sets the state for us */
    }

    /* read num comments */
    FLAC__ASSERT(FLAC__STREAM_METADATA_VORBIS_COMMENT_NUM_COMMENTS_LEN == 32);
    if (!FLAC__bitbuffer_read_raw_uint32_little_endian(decoder->private_->input, &obj->num_comments, read_callback_, decoder))
        return false; /* the read_callback_ sets the state for us */

    /* read comments */
    if (obj->num_comments > 0)
    {
        for (i = 0; i < obj->num_comments; i++)
        {
            FLAC__ASSERT(FLAC__STREAM_METADATA_VORBIS_COMMENT_ENTRY_LENGTH_LEN == 32);
            if (!FLAC__bitbuffer_read_raw_uint32_little_endian(decoder->private_->input, &comment_length, read_callback_, decoder))
                return false; /* the read_callback_ sets the state for us */
            if (comment_length > 0)
            {
                if (comment_length > MAX_COMMENT_LENGTH)
                {
                    if (!FLAC__bitbuffer_read_byte_block_aligned_no_crc(decoder->private_->input, strings, MAX_COMMENT_LENGTH, read_callback_, decoder))
                        return false; /* the read_callback_ sets the state for us */
                    strings[MAX_COMMENT_LENGTH] = '\0';

                    if (!FLAC__bitbuffer_read_byte_block_aligned_no_crc(decoder->private_->input, NULL, comment_length - MAX_COMMENT_LENGTH, read_callback_, decoder))
                        return false; /* the read_callback_ sets the state for us */

                    comment_length = MAX_COMMENT_LENGTH;
                    //strings[0] = 0;
                }
                else
                {
                    if (!FLAC__bitbuffer_read_byte_block_aligned_no_crc(decoder->private_->input, strings, comment_length, read_callback_, decoder))
                        return false; /* the read_callback_ sets the state for us */
                    strings[comment_length] = '\0';
                }

                /* 读取 ID3 信息 */

                if ((strings[0] == 'a' || strings[0] == 'A') && (strings[1] == 'r' || strings[1] == 'R') && (strings[2] == 't' || strings[2] == 'T') && (strings[3] == 'i' || strings[3] == 'I') && (strings[4] == 's' || strings[4] == 'S') && (strings[5] == 't' || strings[5] == 'T'))
                {
                    disp_length = (comment_length - 7) > 30 ? 30 : (comment_length - 7);
                    memcpy(g_flac_decoder_info.Artist, strings + 7, sizeof(FLAC__byte)*disp_length);
                    g_flac_decoder_info.Artist[30-1] = 0;
                }
                if ((strings[0] == 't' || strings[0] == 'T') && (strings[1] == 'i' || strings[1] == 'I') && (strings[2] == 't' || strings[2] == 'T') && (strings[3] == 'l' || strings[3] == 'L') && (strings[4] == 'e' || strings[4] == 'E'))
                {
                    disp_length = (comment_length - 6) > 30 ? 30 : (comment_length - 6);
                    memcpy(g_flac_decoder_info.Title, strings + 6, sizeof(FLAC__byte)*disp_length);
                    g_flac_decoder_info.Title[30-1] = 0;
                }
                if ((strings[0] == 'a' || strings[0] == 'A') && (strings[1] == 'l' || strings[1] == 'L') && (strings[2] == 'b' || strings[2] == 'B') && (strings[3] == 'u' || strings[3] == 'U') && (strings[4] == 'm' || strings[4] == 'M'))
                {
                    disp_length = (comment_length - 6) > 30 ? 30 : (comment_length - 6);
                    memcpy(g_flac_decoder_info.Album, strings + 6, sizeof(FLAC__byte)*disp_length);
                    g_flac_decoder_info.Album[30-1] = 0;
                }
            }
        }
    }
    else
    {
        obj->comments = 0;
    }

    return true;
}
#endif

_ATTR_FLACDEC_TEXT_
FLAC__bool skip_id3v2_tag_(FLAC__StreamDecoder *decoder)
{
    FLAC__uint32 x;
    unsigned i, skip;

    /* skip the version and flags bytes */
    if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &x, 24, read_callback_, decoder))
        return false; /* the read_callback_ sets the state for us */
    /* get the size (in bytes) to skip */
    skip = 0;
    for (i = 0; i < 4; i++)
    {
        if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &x, 8, read_callback_, decoder))
            return false; /* the read_callback_ sets the state for us */
        skip <<= 7;
        skip |= (x & 0x7f);
    }
    /* skip the rest of the tag */
    if (!FLAC__bitbuffer_read_byte_block_aligned_no_crc(decoder->private_->input, 0, skip, read_callback_, decoder))
        return false; /* the read_callback_ sets the state for us */
    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC__bool frame_sync_(FLAC__StreamDecoder *decoder)
{
    FLAC__uint32 x;
    FLAC__bool first = true;

    //if (g_f_FFW_FFD)
    //{
    //    decoder->private_->samples_decoded = g_seek_table_points[g_cur_used_seek_table_points].sample_number;
    //}

    /* If we know the total number of samples in the stream, stop if we've read that many. */
    /* This will stop us, for example, from wasting time trying to sync on an ID3V1 tag. */
    if (decoder->private_->has_stream_info && decoder->private_->stream_info.data.stream_info.total_samples)
    {
        if (decoder->private_->samples_decoded >= decoder->private_->stream_info.data.stream_info.total_samples)
        {
            decoder->protected_->state = FLAC__STREAM_DECODER_END_OF_STREAM;
            return true;
        }
    }

    g_flac_decoder_info.samples_decoded = decoder->private_->samples_decoded;

    //???? why ?? by Vincent
    //if (!g_f_FFW_FFD)
    {
        /* make sure we're byte aligned */
        if (!FLAC__bitbuffer_is_consumed_byte_aligned(decoder->private_->input))
        {
            if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &x, FLAC__bitbuffer_bits_left_for_byte_alignment(decoder->private_->input), read_callback_, decoder))
                return false; /* the read_callback_ sets the state for us */
        }
    }

    while (1)
    {
        if (decoder->private_->cached)
        {
            x = (FLAC__uint32)decoder->private_->lookahead;
            decoder->private_->cached = false;
        }
        else
        {
            if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &x, 8, read_callback_, decoder))
            {
                //while(1);//by Vincent for test
                return false; /* the read_callback_ sets the state for us */
            }
        }
        if (x == 0xff)  /* MAGIC NUMBER for the first 8 frame sync bits */
        {
            decoder->private_->header_warmup[0] = (FLAC__byte)x;
            if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &x, 8, read_callback_, decoder))
            {
                //while(1);//by Vincent for test
                return false; /* the read_callback_ sets the state for us */
            }

            /* we have to check if we just read two 0xff's in a row; the second may actually be the beginning of the sync code */
            /* else we have to check if the second byte is the end of a sync code */
            if (x == 0xff)  /* MAGIC NUMBER for the first 8 frame sync bits */
            {
                decoder->private_->lookahead = (FLAC__byte)x;
                decoder->private_->cached = true;
            }
            else if (x >> 2 == 0x3e)  /* MAGIC NUMBER for the last 6 sync bits */
            {
                decoder->private_->header_warmup[1] = (FLAC__byte)x;
                decoder->protected_->state = FLAC__STREAM_DECODER_READ_FRAME;
                return true;
            }
        }
        if (first)
        {
            decoder->private_->error_callback(decoder, FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC, decoder->private_->client_data);
            first = false;
        }
    }

    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC__bool read_frame_(FLAC__StreamDecoder *decoder, FLAC__bool *got_a_frame, FLAC__bool do_full_decode)
{
    unsigned channel;
    unsigned i;
    FLAC__int32 mid, side, left, right;
    FLAC__uint16 frame_crc; /* the one we calculate from the input stream */
    FLAC__uint32 x;

    unsigned block_size;

    *got_a_frame = false;

    /* init the CRC */
    frame_crc = 0;
    FLAC__CRC16_UPDATE(decoder->private_->header_warmup[0], frame_crc);
    FLAC__CRC16_UPDATE(decoder->private_->header_warmup[1], frame_crc);
    FLAC__bitbuffer_reset_read_crc16(decoder->private_->input, frame_crc);

    if (!read_frame_header_(decoder))
    {
        g_read_frame_header_error_cnt++;
        decoder->protected_->state = FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
        return false;
    }
    g_read_frame_header_error_cnt = 0;
    if (decoder->protected_->state == FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC)
        return true;

    block_size = decoder->private_->frame.header.blocksize;
    if(block_size > FLAC__PCMBUFFER_DEFAULT_CAPACITY/2)
    {
       // printf("block size = %d\n", block_size);
        isNeedDecByHalfFrmae = 1;
       #ifdef HALF_FRAME_BY_HALF_FRAME
        g_half_block_size = block_size = block_size/2;
       #endif
    }
    else
        isNeedDecByHalfFrmae = 0;
//#ifdef HALF_FRAME_BY_HALF_FRAME
//    g_half_block_size = block_size = decoder->private_->frame.header.blocksize/2;
//#else
//    block_size = decoder->private_->frame.header.blocksize;
//#endif
    if (!allocate_output_(decoder, block_size, decoder->private_->frame.header.channels))
        return false;
    for (channel = 0; channel < decoder->private_->frame.header.channels; channel++)
    {
        /*
         * first figure the correct bits-per-sample of the subframe
         */
        unsigned bps = decoder->private_->frame.header.bits_per_sample;
        switch (decoder->private_->frame.header.channel_assignment)
        {
            case FLAC__CHANNEL_ASSIGNMENT_INDEPENDENT:
                /* no adjustment needed */
                break;
            case FLAC__CHANNEL_ASSIGNMENT_LEFT_SIDE:
                FLAC__ASSERT(decoder->private_->frame.header.channels == 2);
                if (channel == 1)
                    bps++;
                break;
            case FLAC__CHANNEL_ASSIGNMENT_RIGHT_SIDE:
                FLAC__ASSERT(decoder->private_->frame.header.channels == 2);
                if (channel == 0)
                    bps++;
                break;
            case FLAC__CHANNEL_ASSIGNMENT_MID_SIDE:
                FLAC__ASSERT(decoder->private_->frame.header.channels == 2);
                if (channel == 1)
                    bps++;
                break;
            default:
                FLAC__ASSERT(0);
        }
        /*
         * now read it
         */
        if (!read_subframe_(decoder, channel, bps, do_full_decode))
            return false;
        if (decoder->protected_->state != FLAC__STREAM_DECODER_READ_FRAME)
        {
            decoder->protected_->state = FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
            return true;
        }
    }
    if (!read_zero_padding_(decoder))
        return false;

    /*
     * Read the frame CRC-16 from the footer and check
     */
#ifndef  HALF_FRAME_BY_HALF_FRAME//如果按半帧解码，不做crc，crc在下半帧解码完毕之后做
  // if(0 == isNeedDecByHalfFrmae)
    {
       frame_crc = FLAC__bitbuffer_get_read_crc16(decoder->private_->input);
       if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &x, FLAC__FRAME_FOOTER_CRC_LEN, read_callback_, decoder))
           return false; /* the read_callback_ sets the state for us */
    }

#endif
		//flac_DEBUG("x = %d ",x);
		if(frame_crc == (FLAC__uint16)x||x == 0)//isNeedDecByHalfFrmae)
   //

    {
        //恢复出左右声道数据
        if (do_full_decode)
        {

            FLAC__int32* pmidOrLeft = (FLAC__int32*)(decoder->private_->output[0]);

            /* Undo any special channel coding */
            switch (decoder->private_->frame.header.channel_assignment)
            {
                    /*
                     * 左右声道数据重构
                     *
                     * mid  = (left + right) / 2
                     * side = left - right
                     */
                case FLAC__CHANNEL_ASSIGNMENT_INDEPENDENT:
                    /* do nothing */
                    break;
                case FLAC__CHANNEL_ASSIGNMENT_LEFT_SIDE:
                    FLAC__ASSERT(decoder->private_->frame.header.channels == 2);
                    for (i = 0; i < block_size; i++)
                        //decoder->private_->output[1][i] = decoder->private_->output[0][i] - decoder->private_->output[1][i];
                        decoder->private_->output[1][i] = (FLAC__int32)(pmidOrLeft[i]) - decoder->private_->output[1][i];
                    break;
                case FLAC__CHANNEL_ASSIGNMENT_RIGHT_SIDE:
                    FLAC__ASSERT(decoder->private_->frame.header.channels == 2);
                    for (i = 0; i < block_size; i++)
                        pmidOrLeft[i] += decoder->private_->output[1][i];
                    break;
                case FLAC__CHANNEL_ASSIGNMENT_MID_SIDE:
                    FLAC__ASSERT(decoder->private_->frame.header.channels == 2);
                    for (i = 0; i < block_size; i++)
                    {
                        mid = pmidOrLeft[i];
                        side = decoder->private_->output[1][i];
                        mid <<= 1;
                        if (side & 1) /* i.e. if 'side' is odd... */
                            mid++;
                        left = mid + side;
                        right = mid - side;

                        pmidOrLeft[i] = left >> 1;
                        decoder->private_->output[1][i] = right >> 1;
                    }
                    break;
                default:
                    FLAC__ASSERT(0);
                    break;
            }
        }
    }
    else
    {
        /* Bad frame, emit error and zero the output signal */
#ifdef _DEBUG
        decoder->private_->error_callback(decoder, FLAC__STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH, decoder->private_->client_data);
#endif
        /* crc检验出错,将输出Buffer清零,继续下一帧解码 */
        if (do_full_decode)
        {
            for (channel = 0; channel < decoder->private_->frame.header.channels; channel++)
            {
                memset(decoder->private_->output[channel], 0, sizeof(FLAC__int32) * block_size);
            }
        }
    }

    *got_a_frame = true;

    /* put the latest values into the public section of the decoder instance */
    decoder->protected_->channels = decoder->private_->frame.header.channels;
    decoder->protected_->channel_assignment = decoder->private_->frame.header.channel_assignment;
    decoder->protected_->bits_per_sample = decoder->private_->frame.header.bits_per_sample;
    decoder->protected_->sample_rate = decoder->private_->frame.header.sample_rate;
    decoder->protected_->blocksize = decoder->private_->frame.header.blocksize;

    FLAC__ASSERT(decoder->private_->frame.header.number_type == FLAC__FRAME_NUMBER_TYPE_SAMPLE_NUMBER);
    decoder->private_->samples_decoded = decoder->private_->frame.header.number.sample_number + block_size;

    /* write it */
    if (do_full_decode)
    {
        if (decoder->private_->write_callback(decoder, &decoder->private_->frame, (const FLAC__int32 * const *)decoder->private_->output, decoder->private_->client_data) != FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE)
            return false;
    }
//----------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
  #ifdef HALF_FRAME_BY_HALF_FRAME
    if(0 == isNeedDecByHalfFrmae)
  #endif
       decoder->protected_->state = FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
    return true;
}
/*********************************************************

**********************************************************/
_ATTR_FLACDEC_TEXT_
FLAC__bool post_read_frame_(FLAC__StreamDecoder *decoder, FLAC__bool *got_a_frame, FLAC__bool do_full_decode)
{
    unsigned channel;
    unsigned i;
    FLAC__int32 mid, side, left, right;
    FLAC__uint16 frame_crc; /* the one we calculate from the input stream */
    FLAC__uint32 x;

    unsigned block_size;
    #if 0
//return true;
    *got_a_frame = false;
#ifdef HALF_FRAME_BY_HALF_FRAME
     g_half_block_size = block_size = decoder->private_->frame.header.blocksize/2;
#else
     block_size = decoder->private_->frame.header.blocksize;
#endif

    //------------------------decode post-half frame-----------------------------------
#ifdef HALF_FRAME_BY_HALF_FRAME

        for (channel = 0; channel < decoder->private_->frame.header.channels; channel++)
        {
            unsigned bps = decoder->private_->frame.header.bits_per_sample;

            if (!post_read_subframe_(decoder, channel, bps, do_full_decode))
                return false;
            if (decoder->protected_->state != FLAC__STREAM_DECODER_READ_FRAME)
            {
                decoder->protected_->state = FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
                return true;
            }
        }

        if (!read_zero_padding_(decoder))
            return false;

        /*
         * Read the frame CRC-16 from the footer and check
         */
       frame_crc = FLAC__bitbuffer_get_read_crc16(decoder->private_->input);
       if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &x, FLAC__FRAME_FOOTER_CRC_LEN, read_callback_, decoder))
           return false; /* the read_callback_ sets the state for us */
      // flac_DEBUG("%d, %d",frame_crc,(FLAC__uint16)x);
        if(frame_crc == (FLAC__uint16)x ||(x == 0))
        {
            //恢复出左右声道数据
            if (do_full_decode)
            {
#ifdef FLAC_MINUS_ZI_DATA
                FLAC__int16* pmidOrLeft = (FLAC__int16*)(decoder->private_->output[0]);
#else
                FLAC__int32* pmidOrLeft = (FLAC__int32*)(decoder->private_->output[0]);
#endif
                /* Undo any special channel coding */
                switch (decoder->private_->frame.header.channel_assignment)
                {
                        /*
                         * 左右声道数据重构
                         *
                         * mid  = (left + right) / 2
                         * side = left - right
                         */
                    case FLAC__CHANNEL_ASSIGNMENT_INDEPENDENT:
                        /* do nothing */
                        break;
                    case FLAC__CHANNEL_ASSIGNMENT_LEFT_SIDE:
                        FLAC__ASSERT(decoder->private_->frame.header.channels == 2);
                        for (i = 0; i < block_size; i++)
                            //decoder->private_->output[1][i] = decoder->private_->output[0][i] - decoder->private_->output[1][i];
                            decoder->private_->output[1][i] = (FLAC__int32)(pmidOrLeft[i]) - decoder->private_->output[1][i];
                        break;
                    case FLAC__CHANNEL_ASSIGNMENT_RIGHT_SIDE:
                        FLAC__ASSERT(decoder->private_->frame.header.channels == 2);
                        for (i = 0; i < block_size; i++)
                            pmidOrLeft[i] += decoder->private_->output[1][i];
                        break;
                    case FLAC__CHANNEL_ASSIGNMENT_MID_SIDE:
                        FLAC__ASSERT(decoder->private_->frame.header.channels == 2);
                        for (i = 0; i < block_size; i++)
                        {
                            mid = pmidOrLeft[i];
                            side = decoder->private_->output[1][i];
                            mid <<= 1;
                            if (side & 1) /* i.e. if 'side' is odd... */
                                mid++;
                            left = mid + side;
                            right = mid - side;

                            pmidOrLeft[i] = left >> 1;
                            decoder->private_->output[1][i] = right >> 1;
                        }
                        break;
                    default:
                        FLAC__ASSERT(0);
                        break;
                }
            }
        }
        else
        {
            /* Bad frame, emit error and zero the output signal */
#ifdef _DEBUG
            decoder->private_->error_callback(decoder, FLAC__STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH, decoder->private_->client_data);
#endif
            /* crc检验出错,将输出Buffer清零,继续下一帧解码 */
            if (do_full_decode)
            {
#ifdef FLAC_MINUS_ZI_DATA
                memset(decoder->private_->output[0], 0, sizeof(FLAC__int16) * block_size);
                memset(decoder->private_->output[1], 0, sizeof(FLAC__int32) * block_size);

#else
                for (channel = 0; channel < decoder->private_->frame.header.channels; channel++)
                {
                    memset(decoder->private_->output[channel], 0, sizeof(FLAC__int32) * block_size);
                }
#endif
            }
        }

        *got_a_frame = true;

        /* put the latest values into the public section of the decoder instance */
        decoder->protected_->channels = decoder->private_->frame.header.channels;
        decoder->protected_->channel_assignment = decoder->private_->frame.header.channel_assignment;
        decoder->protected_->bits_per_sample = decoder->private_->frame.header.bits_per_sample;
        decoder->protected_->sample_rate = decoder->private_->frame.header.sample_rate;
        decoder->protected_->blocksize = decoder->private_->frame.header.blocksize;

        FLAC__ASSERT(decoder->private_->frame.header.number_type == FLAC__FRAME_NUMBER_TYPE_SAMPLE_NUMBER);
        decoder->private_->samples_decoded = decoder->private_->frame.header.number.sample_number + block_size;

        /* write it */
        if (do_full_decode)
        {
            if (decoder->private_->write_callback(decoder, &decoder->private_->frame, (const FLAC__int32 * const *)decoder->private_->output, decoder->private_->client_data) != FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE)
                return false;
        }

#endif
    //------------------------decode post-half frame end-------------------------------
    #endif
    decoder->protected_->state = FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC__bool read_frame_header_(FLAC__StreamDecoder *decoder)
{
    FLAC__uint32 x;
    FLAC__uint64 xx;
    unsigned i, blocksize_hint = 0, sample_rate_hint = 0;
    FLAC__byte crc8, raw_header[16]; /* MAGIC NUMBER based on the maximum frame header size, including CRC */
    unsigned raw_header_len;
    FLAC__bool is_unparseable = false;
    const FLAC__bool is_known_variable_blocksize_stream = (decoder->private_->has_stream_info && decoder->private_->stream_info.data.stream_info.min_blocksize != decoder->private_->stream_info.data.stream_info.max_blocksize);
    const FLAC__bool is_known_fixed_blocksize_stream = (decoder->private_->has_stream_info && decoder->private_->stream_info.data.stream_info.min_blocksize == decoder->private_->stream_info.data.stream_info.max_blocksize);

    FLAC__ASSERT(FLAC__bitbuffer_is_consumed_byte_aligned(decoder->private_->input));

    /* init the raw header with the saved bits from synchronization */
    raw_header[0] = decoder->private_->header_warmup[0];
    raw_header[1] = decoder->private_->header_warmup[1];
    raw_header_len = 2;

    /*
     * check to make sure that the reserved bits are 0
     */
    if (raw_header[1] & 0x03)  /* MAGIC NUMBER */
    {
        is_unparseable = true;
    }

    /*
     * Note that along the way as we read the header, we look for a sync
     * code inside.  If we find one it would indicate that our original
     * sync was bad since there cannot be a sync code in a valid header.
     */

    /*
     * read in the raw header as bytes so we can CRC it, and parse it on the way
     */
    for (i = 0; i < 2; i++)
    {
        if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &x, 8, read_callback_, decoder))
            return false; /* the read_callback_ sets the state for us */
        if (x == 0xff)  /* MAGIC NUMBER for the first 8 frame sync bits */
        {
            /* if we get here it means our original sync was erroneous since the sync code cannot appear in the header */
            decoder->private_->lookahead = (FLAC__byte)x;
            decoder->private_->cached = true;
            decoder->private_->error_callback(decoder, FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER, decoder->private_->client_data);
            decoder->protected_->state = FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
            return true;
        }
        raw_header[raw_header_len++] = (FLAC__byte)x;
    }

    switch (x = raw_header[2] >> 4)
    {
        case 0:
            if (is_known_fixed_blocksize_stream)
                decoder->private_->frame.header.blocksize = decoder->private_->stream_info.data.stream_info.min_blocksize;
            else
                is_unparseable = true;
            break;
        case 1:
            decoder->private_->frame.header.blocksize = 192;
            break;
        case 2:
        case 3:
        case 4:
        case 5:
            decoder->private_->frame.header.blocksize = 576 << (x - 2);
            break;
        case 6:
        case 7:
            blocksize_hint = x;
            break;
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
            decoder->private_->frame.header.blocksize = 256 << (x - 8);
            break;
        default:
            FLAC__ASSERT(0);
            break;
    }


    if (decoder->private_->frame.header.blocksize != g_flac_decoder_info.blocksize)
    {
        //return false;
    }

    switch (x = raw_header[2] & 0x0f)
    {
        case 0:
            if (decoder->private_->has_stream_info)
                decoder->private_->frame.header.sample_rate = decoder->private_->stream_info.data.stream_info.sample_rate;
            else
                is_unparseable = true;
            break;
        case 1:
        case 2:
        case 3:
            is_unparseable = true;
            break;
        case 4:
            decoder->private_->frame.header.sample_rate = 8000;
            break;
        case 5:
            decoder->private_->frame.header.sample_rate = 16000;
            break;
        case 6:
            decoder->private_->frame.header.sample_rate = 22050;
            break;
        case 7:
            decoder->private_->frame.header.sample_rate = 24000;
            break;
        case 8:
            decoder->private_->frame.header.sample_rate = 32000;
            break;
        case 9:
            decoder->private_->frame.header.sample_rate = 44100;
            break;
        case 10:
            decoder->private_->frame.header.sample_rate = 48000;
            break;
        case 11:
            decoder->private_->frame.header.sample_rate = 96000;
            break;
        case 12:
        case 13:
        case 14:
            sample_rate_hint = x;
            break;
        case 15:
            decoder->private_->error_callback(decoder, FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER, decoder->private_->client_data);
            decoder->protected_->state = FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
            return true;
        default:
            FLAC__ASSERT(0);
    }

    if (decoder->private_->frame.header.sample_rate != g_flac_decoder_info.sample_rate)
    {
        //return false;
    }

    x = (unsigned)(raw_header[3] >> 4);
    if (x & 8)
    {
        decoder->private_->frame.header.channels = 2;
        switch (x & 7)
        {
            case 0:
                decoder->private_->frame.header.channel_assignment = FLAC__CHANNEL_ASSIGNMENT_LEFT_SIDE;
                break;
            case 1:
                decoder->private_->frame.header.channel_assignment = FLAC__CHANNEL_ASSIGNMENT_RIGHT_SIDE;
                break;
            case 2:
                decoder->private_->frame.header.channel_assignment = FLAC__CHANNEL_ASSIGNMENT_MID_SIDE;
                break;
            default:
                is_unparseable = true;
                break;
        }
    }
    else
    {
        decoder->private_->frame.header.channels = (unsigned)x + 1;
        decoder->private_->frame.header.channel_assignment = FLAC__CHANNEL_ASSIGNMENT_INDEPENDENT;
    }

    //FLAC__ASSERT(decoder->private_->frame.header.channels <= 2);// 我们支持的声道最大为 2 个

    if (decoder->private_->frame.header.channels != g_flac_decoder_info.channels)
    {
        return false;
    }

    switch (x = (unsigned)(raw_header[3] & 0x0e) >> 1)
    {
        case 0:
            if (decoder->private_->has_stream_info)
                decoder->private_->frame.header.bits_per_sample = decoder->private_->stream_info.data.stream_info.bits_per_sample;
            else
                is_unparseable = true;
            break;
        case 1:
            decoder->private_->frame.header.bits_per_sample = 8;
            break;
        case 2:
            decoder->private_->frame.header.bits_per_sample = 12;
            break;
        case 4:
            decoder->private_->frame.header.bits_per_sample = 16;
            break;
        case 5:
            decoder->private_->frame.header.bits_per_sample = 20;
            break;
        case 6:
            decoder->private_->frame.header.bits_per_sample = 24;
            break;
        case 3:
        case 7:
            is_unparseable = true;
            break;
        default:
            FLAC__ASSERT(0);
            break;
    }

    //FLAC__ASSERT(decoder->private_->frame.header.bits_per_sample == 16); // 我们只支持 bits_per_sample = 16;

    if (decoder->private_->frame.header.bits_per_sample != g_flac_decoder_info.bits_per_sample)
    {
        return false;
    }

    if (raw_header[3] & 0x01)  /* this should be a zero padding bit */
    {
        decoder->private_->error_callback(decoder, FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER, decoder->private_->client_data);
        decoder->protected_->state = FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
        return true;
    }

    /*
     * Now we get to the regrettable consequences of not knowing for sure
     * whether we got a frame number or a sample number.  There are no
     * encoders that do variable-blocksize encoding so unless we know from
     * the STREAMINFO that it is variable-blocksize we will assume it is
     * fixed-blocksize.  The trouble comes when we have no STREAMINFO; again
     * we will guess that is fixed-blocksize.  Where this can go wrong: 1) a
     * variable-blocksize stream with no STREAMINFO; 2) a fixed-blocksize
     * stream that was edited such that one or more frames before or
     * including this one do not have the same number of samples as the
     * STREAMINFO's min and max blocksize.
     */
    if (is_known_variable_blocksize_stream)
    {

        FLAC__ASSERT(0);// 目前我们不支持variable_blocksize
        return false;

        if (blocksize_hint)
        {
            if (!FLAC__bitbuffer_read_utf8_uint64(decoder->private_->input, &xx, read_callback_, decoder, raw_header, &raw_header_len))
                return false; /* the read_callback_ sets the state for us */
            if (xx == FLAC__U64L(0xffffffffffffffff))  /* i.e. non-UTF8 code... */
            {
                decoder->private_->lookahead = raw_header[raw_header_len-1]; /* back up as much as we can */
                decoder->private_->cached = true;
                decoder->private_->error_callback(decoder, FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER, decoder->private_->client_data);
                decoder->protected_->state = FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
                return true;
            }
            decoder->private_->frame.header.number_type = FLAC__FRAME_NUMBER_TYPE_SAMPLE_NUMBER;
            decoder->private_->frame.header.number.sample_number = xx;
        }
        else
            is_unparseable = true;
    }
    else
    {
        if (!FLAC__bitbuffer_read_utf8_uint32(decoder->private_->input, &x, read_callback_, decoder, raw_header, &raw_header_len))
            return false; /* the read_callback_ sets the state for us */
        if (x == 0xffffffff)  /* i.e. non-UTF8 code... */
        {
            decoder->private_->lookahead = raw_header[raw_header_len-1]; /* back up as much as we can */
            decoder->private_->cached = true;
            decoder->private_->error_callback(decoder, FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER, decoder->private_->client_data);
            decoder->protected_->state = FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
            return true;
        }
        decoder->private_->last_frame_number = x;
        decoder->private_->frame.header.number_type = FLAC__FRAME_NUMBER_TYPE_SAMPLE_NUMBER;
        if (decoder->private_->has_stream_info)
        {
            FLAC__ASSERT(decoder->private_->stream_info.data.stream_info.min_blocksize == decoder->private_->stream_info.data.stream_info.max_blocksize);
            decoder->private_->frame.header.number.sample_number = (FLAC__uint64)decoder->private_->stream_info.data.stream_info.min_blocksize * (FLAC__uint64)x;
            decoder->private_->last_block_size = decoder->private_->frame.header.blocksize;
        }
        else if (blocksize_hint)
        {
            if (decoder->private_->last_block_size)
                decoder->private_->frame.header.number.sample_number = (FLAC__uint64)decoder->private_->last_block_size * (FLAC__uint64)x;
            else
                is_unparseable = true;
        }
        else
        {
            decoder->private_->frame.header.number.sample_number = (FLAC__uint64)decoder->private_->frame.header.blocksize * (FLAC__uint64)x;
            decoder->private_->last_block_size = decoder->private_->frame.header.blocksize;
        }
        FLAC__ASSERT(decoder->private_->frame.header.number.sample_number < 0xffffffff);
    }

    if (blocksize_hint)
    {
        if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &x, 8, read_callback_, decoder))
            return false; /* the read_callback_ sets the state for us */
        raw_header[raw_header_len++] = (FLAC__byte)x;
        if (blocksize_hint == 7)
        {
            FLAC__uint32 _x;
            if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &_x, 8, read_callback_, decoder))
                return false; /* the read_callback_ sets the state for us */
            raw_header[raw_header_len++] = (FLAC__byte)_x;
            x = (x << 8) | _x;
        }
        decoder->private_->frame.header.blocksize = x + 1;
    }

    if (sample_rate_hint)
    {
        if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &x, 8, read_callback_, decoder))
            return false; /* the read_callback_ sets the state for us */
        raw_header[raw_header_len++] = (FLAC__byte)x;
        if (sample_rate_hint != 12)
        {
            FLAC__uint32 _x;
            if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &_x, 8, read_callback_, decoder))
                return false; /* the read_callback_ sets the state for us */
            raw_header[raw_header_len++] = (FLAC__byte)_x;
            x = (x << 8) | _x;
        }
        if (sample_rate_hint == 12)
            decoder->private_->frame.header.sample_rate = x * 1000;
        else if (sample_rate_hint == 13)
            decoder->private_->frame.header.sample_rate = x;
        else
            decoder->private_->frame.header.sample_rate = x * 10;
    }

    /* read the CRC-8 byte */
    if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &x, 8, read_callback_, decoder))
        return false; /* the read_callback_ sets the state for us */
    crc8 = (FLAC__byte)x;

    if (FLAC__crc8(raw_header, raw_header_len) != crc8)
    {
        decoder->private_->error_callback(decoder, FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER, decoder->private_->client_data);
        decoder->protected_->state = FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
        return true;
    }

    if (is_unparseable)
    {
        decoder->protected_->state = FLAC__STREAM_DECODER_UNPARSEABLE_STREAM;
        return false;
    }

    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC__bool read_subframe_(FLAC__StreamDecoder *decoder, unsigned channel, unsigned bps, FLAC__bool do_full_decode)
{
    FLAC__uint32 x;
    FLAC__bool wasted_bits;
//#ifdef HALF_FRAME_BY_HALF_FRAME
//  unsigned block_size = decoder->private_->frame.header.blocksize/2;
//#else
    unsigned block_size = decoder->private_->frame.header.blocksize;
//#endif
#ifdef HALF_FRAME_BY_HALF_FRAME
    block_size >>= isNeedDecByHalfFrmae;
#endif
   //flac_DEBUG("%d ",block_size);
#if defined(FLAC_MINUS_ZI_DATA)||defined(HALF_FRAME_BY_HALF_FRAME)
    gFLACchannel = channel;
#endif
    if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &x, 8, read_callback_, decoder)) /* MAGIC NUMBER */
        return false; /* the read_callback_ sets the state for us */

    wasted_bits = (x & 1);
    x &= 0xfe;

    if (wasted_bits)
    {
        unsigned u;
        if (!FLAC__bitbuffer_read_unary_unsigned(decoder->private_->input, &u, read_callback_, decoder))
            return false; /* the read_callback_ sets the state for us */
        decoder->private_->frame.subframes[channel].wasted_bits = u + 1;
        bps -= decoder->private_->frame.subframes[channel].wasted_bits;
    }
    else
        decoder->private_->frame.subframes[channel].wasted_bits = 0;

    /*
     * Lots of magic numbers here
     */
    // 预测类型
    if (x & 0x80)
    {
        decoder->private_->error_callback(decoder, FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC, decoder->private_->client_data);
        decoder->protected_->state = FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
        return true;
    }
    else if (x == 0)
    {
        if (!read_subframe_constant_(decoder, channel, bps, do_full_decode))
            return false;
    }
    else if (x == 2)
    {
        if (!read_subframe_verbatim_(decoder, channel, bps, do_full_decode))
            return false;
    }
    else if (x < 16)
    {
        decoder->protected_->state = FLAC__STREAM_DECODER_UNPARSEABLE_STREAM;
        return false;
    }
    else if (x <= 24)
    {
        if (!read_subframe_fixed_(decoder, channel, bps, (x >> 1)&7, do_full_decode))
            return false;
    }
    else if (x < 64)
    {
        decoder->protected_->state = FLAC__STREAM_DECODER_UNPARSEABLE_STREAM;
        return false;
    }
    else
    {
        if (!read_subframe_lpc_(decoder, channel, bps, ((x >> 1)&31) + 1, do_full_decode))
            return false;
    }
    #ifdef HALF_FRAME_BY_HALF_FRAME
    SubframeInfo[channel].judge_wasted_bits = wasted_bits;
    #endif
    if (wasted_bits && do_full_decode)
    {
        unsigned i;
        x = decoder->private_->frame.subframes[channel].wasted_bits;
#ifdef HALF_FRAME_BY_HALF_FRAME
       SubframeInfo[channel].wasted_bits = x;
#endif

        for (i = 0; i < block_size;i++)
            decoder->private_->output[channel][i] <<= x;
    }

    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC__bool read_subframe_constant_(FLAC__StreamDecoder *decoder, unsigned channel, unsigned bps, FLAC__bool do_full_decode)
{
    FLAC__Subframe_Constant *subframe = &decoder->private_->frame.subframes[channel].data.constant;
    FLAC__int32 x;
    unsigned i;
    FLAC__int32 *output = decoder->private_->output[channel];
//#ifdef HALF_FRAME_BY_HALF_FRAME
//  unsigned block_size = decoder->private_->frame.header.blocksize/2;
//#else
    unsigned block_size = decoder->private_->frame.header.blocksize;
//#endif
    #ifdef HALF_FRAME_BY_HALF_FRAME
    block_size >>= isNeedDecByHalfFrmae;
    #endif


    decoder->private_->frame.subframes[channel].type = FLAC__SUBFRAME_TYPE_CONSTANT;

    if (!FLAC__bitbuffer_read_raw_int32(decoder->private_->input, &x, bps, read_callback_, decoder))
        return false; /* the read_callback_ sets the state for us */

    subframe->value = x;

    /* decode the subframe */
    if (do_full_decode)
    {
        for (i = 0; i < block_size; i++)
            output[i] = x;
    }


#ifdef HALF_FRAME_BY_HALF_FRAME
if(isNeedDecByHalfFrmae)
{
    //记录信息供下半帧解码
    //1 子帧分支和constant
    SubframeInfo[channel].subframeType = FLAC__SUBFRAME_TYPE_CONSTANT;
    SubframeInfo[channel].constant = x;

    //2 输入buffer信息
    SubframeInfo[channel].fileOffset = g_CurrFileOffset - FLAC__BITBUFFER_DEFAULT_CAPACITY;

    //3 bit buffer 的信息
    memcpy(&(SubframeInfo[channel].bitBuffer),decoder->private_->input,sizeof(FLAC__BitBuffer));

    //4 用于做预测的order个数据
}
#endif
    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC__bool read_subframe_fixed_(FLAC__StreamDecoder *decoder, unsigned channel, unsigned bps, const unsigned order, FLAC__bool do_full_decode)
{
    FLAC__Subframe_Fixed *subframe = &decoder->private_->frame.subframes[channel].data.fixed;
    FLAC__int32 i32;
    FLAC__uint32 u32;
    unsigned u;
    FLAC__int32* ptemp,*pOutput;

//#ifdef HALF_FRAME_BY_HALF_FRAME
//  unsigned block_size = decoder->private_->frame.header.blocksize/2;
//#else
    unsigned block_size = decoder->private_->frame.header.blocksize;
//#endif
  #ifdef HALF_FRAME_BY_HALF_FRAME
    block_size >>= isNeedDecByHalfFrmae;
  #endif

    decoder->private_->frame.subframes[channel].type = FLAC__SUBFRAME_TYPE_FIXED;

    subframe->residual = decoder->private_->residual[channel];
    subframe->order = order;

    ptemp = decoder->private_->residual[channel] + order;


    /* read warm-up samples */
    for (u = 0; u < order; u++)
    {
        if (!FLAC__bitbuffer_read_raw_int32(decoder->private_->input, &i32, bps, read_callback_, decoder))
            return false; /* the read_callback_ sets the state for us */
        subframe->warmup[u] = i32;
    }

    /* read entropy coding method info */
    if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &u32, FLAC__ENTROPY_CODING_METHOD_TYPE_LEN, read_callback_, decoder))
        return false; /* the read_callback_ sets the state for us */
    subframe->entropy_coding_method.type = (FLAC__EntropyCodingMethodType)u32;
    switch (subframe->entropy_coding_method.type)
    {
        case FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE:
            if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &u32, FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE_ORDER_LEN, read_callback_, decoder))
                return false; /* the read_callback_ sets the state for us */
            subframe->entropy_coding_method.data.partitioned_rice.order = u32;
            subframe->entropy_coding_method.data.partitioned_rice.contents = &decoder->private_->partitioned_rice_contents[channel];
            break;
        default:
            decoder->protected_->state = FLAC__STREAM_DECODER_UNPARSEABLE_STREAM;
            return false;
    }

    /* read residual */
    switch (subframe->entropy_coding_method.type)
    {
        case FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE:
            //if(!read_residual_partitioned_rice_(decoder, order, subframe->entropy_coding_method.data.partitioned_rice.order, &decoder->private_->partitioned_rice_contents[channel], decoder->private_->residual[channel]+order)) // 留出 order 个 int32 来放 warmup
            if (!read_residual_partitioned_rice_(decoder, order, subframe->entropy_coding_method.data.partitioned_rice.order, &decoder->private_->partitioned_rice_contents[channel], ptemp)) // 留出 order 个 int32 来放 warmup
                return false;
            break;
        default:
            FLAC__ASSERT(0);
    }

    /* decode the subframe */
    if (do_full_decode)
    {
        pOutput = decoder->private_->output[channel] + order;
        memcpy(decoder->private_->output[channel], subframe->warmup, sizeof(FLAC__int32) * order);

        //FLAC__fixed_restore_signal(decoder->private_->residual[channel]+order, decoder->private_->frame.header.blocksize-order, order, decoder->private_->output[channel]+order);
        FLAC__fixed_restore_signal(ptemp, block_size - order, order, pOutput);
    }

#ifdef HALF_FRAME_BY_HALF_FRAME
if(isNeedDecByHalfFrmae)
{
    //1 subframe type
    SubframeInfo[channel].subframeType = FLAC__SUBFRAME_TYPE_FIXED;

    //2 data for prediction
    for(u = 0;u < order;u++)
    {
        SubframeInfo[channel].qlp_predictor.predictor[u] = pOutput[block_size - order+u];
    }
    SubframeInfo[channel].order = order;

    //保留order个数据供下半帧解码
    memcpy(g_FlacOutputBuffer[channel] + (FLAC__MAX_LPC_ORDER - order),pOutput + block_size - order - order,order*sizeof(FLAC__int32));
}
#endif

    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC__bool read_subframe_lpc_(FLAC__StreamDecoder *decoder, unsigned channel, unsigned bps, const unsigned order, FLAC__bool do_full_decode)
{
    FLAC__Subframe_LPC *subframe = &decoder->private_->frame.subframes[channel].data.lpc;
    FLAC__int32 i32;
    FLAC__uint32 u32;
    unsigned u;
    FLAC__int32 *ptemp,* pOutputAddOrder;
    unsigned block_size;

//#ifdef HALF_FRAME_BY_HALF_FRAME
//    block_size = decoder->private_->frame.header.blocksize/2;
//#else
    block_size = decoder->private_->frame.header.blocksize;
//#endif
#ifdef HALF_FRAME_BY_HALF_FRAME
    block_size >>= isNeedDecByHalfFrmae;
#endif
    decoder->private_->frame.subframes[channel].type = FLAC__SUBFRAME_TYPE_LPC;

    subframe->residual = decoder->private_->residual[channel];
    subframe->order = order;


    ptemp = decoder->private_->residual[channel] + order;


    /* read warm-up samples */
    for (u = 0; u < order; u++)
    {
        if (!FLAC__bitbuffer_read_raw_int32(decoder->private_->input, &i32, bps, read_callback_, decoder))
            return false; /* the read_callback_ sets the state for us */
        subframe->warmup[u] = i32;
    }

    /* read qlp coeff precision */
    // 量化比特数
    if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &u32, FLAC__SUBFRAME_LPC_QLP_COEFF_PRECISION_LEN, read_callback_, decoder))
        return false; /* the read_callback_ sets the state for us */
    if (u32 == (1u << FLAC__SUBFRAME_LPC_QLP_COEFF_PRECISION_LEN) - 1)
    {
        decoder->private_->error_callback(decoder, FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC, decoder->private_->client_data);
        decoder->protected_->state = FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
        return true;
    }
    subframe->qlp_coeff_precision = u32 + 1;

    /* read qlp shift */
    // 是 Q 多少表示的
    if (!FLAC__bitbuffer_read_raw_int32(decoder->private_->input, &i32, FLAC__SUBFRAME_LPC_QLP_SHIFT_LEN, read_callback_, decoder))
        return false; /* the read_callback_ sets the state for us */
    subframe->quantization_level = i32;

    /* read quantized lp coefficiencts */
    for (u = 0; u < order; u++)
    {
        if (!FLAC__bitbuffer_read_raw_int32(decoder->private_->input, &i32, subframe->qlp_coeff_precision, read_callback_, decoder))
            return false; /* the read_callback_ sets the state for us */
        subframe->qlp_coeff[u] = i32;
    }

    /* read entropy coding method info */
    if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &u32, FLAC__ENTROPY_CODING_METHOD_TYPE_LEN, read_callback_, decoder))
        return false; /* the read_callback_ sets the state for us */
    subframe->entropy_coding_method.type = (FLAC__EntropyCodingMethodType)u32;
    switch (subframe->entropy_coding_method.type)
    {
        case FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE:
            if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &u32, FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE_ORDER_LEN, read_callback_, decoder))
                return false; /* the read_callback_ sets the state for us */
            subframe->entropy_coding_method.data.partitioned_rice.order = u32;
            subframe->entropy_coding_method.data.partitioned_rice.contents = &decoder->private_->partitioned_rice_contents[channel];
            break;
        default:
            decoder->protected_->state = FLAC__STREAM_DECODER_UNPARSEABLE_STREAM;
            return false;
    }

#ifdef HALF_FRAME_BY_HALF_FRAME
if(isNeedDecByHalfFrmae)
{
    SubframeInfo[channel].subframeType = FLAC__SUBFRAME_TYPE_LPC;
    SubframeInfo[channel].quantization_level = subframe->quantization_level;
    memcpy(SubframeInfo[channel].qlp_predictor.qlp_coeff,subframe->qlp_coeff,sizeof(subframe->qlp_coeff));
    SubframeInfo[channel].order = order;
}
#endif

    /* read residual */
    switch (subframe->entropy_coding_method.type)
    {
        case FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE:
            //if(!read_residual_partitioned_rice_(decoder, order, subframe->entropy_coding_method.data.partitioned_rice.order, &decoder->private_->partitioned_rice_contents[channel], decoder->private_->residual[channel]+order))// 留出 order 个 int32 来放 warmup 样值
            if (!read_residual_partitioned_rice_(decoder, order, subframe->entropy_coding_method.data.partitioned_rice.order, &decoder->private_->partitioned_rice_contents[channel], ptemp))// 留出 order 个 int32 来放 warmup 样值
                return false;
            break;
        default:
            FLAC__ASSERT(0);
    }

    /* decode the subframe */
    if (do_full_decode)
    {

        pOutputAddOrder = decoder->private_->output[channel] + order;
        memcpy(decoder->private_->output[channel], subframe->warmup, sizeof(FLAC__int32) * order);

        if (bps + subframe->qlp_coeff_precision + FLAC__bitmath_ilog2(order) <= 32)
        {
            if (bps <= 16 && subframe->qlp_coeff_precision <= 16)
            {
                if (order <= 8)
                {
                    //decoder->private_->local_lpc_restore_signal_16bit_order8(decoder->private_->residual[channel]+order, decoder->private_->frame.header.blocksize-order, subframe->qlp_coeff, order, subframe->quantization_level, decoder->private_->output[channel]+order);
                    decoder->private_->local_lpc_restore_signal_16bit_order8(ptemp, block_size - order, subframe->qlp_coeff, order, subframe->quantization_level, pOutputAddOrder);
#ifdef HALF_FRAME_BY_HALF_FRAME
                    SubframeInfo[channel].restore_signal_type = FLAC__RESTORE_SIGNAL_TYPE_16BITS_ORDER8;
#endif
                }
                else
                {
                    //decoder->private_->local_lpc_restore_signal_16bit(decoder->private_->residual[channel]+order, decoder->private_->frame.header.blocksize-order, subframe->qlp_coeff, order, subframe->quantization_level, decoder->private_->output[channel]+order);
                    decoder->private_->local_lpc_restore_signal_16bit(ptemp, block_size - order, subframe->qlp_coeff, order, subframe->quantization_level, pOutputAddOrder);
#ifdef HALF_FRAME_BY_HALF_FRAME
                    SubframeInfo[channel].restore_signal_type = FLAC__RESTORE_SIGNAL_TYPE_16BITS;
#endif
                }
            }
            else
            {
                //decoder->private_->local_lpc_restore_signal(decoder->private_->residual[channel]+order, decoder->private_->frame.header.blocksize-order, subframe->qlp_coeff, order, subframe->quantization_level, decoder->private_->output[channel]+order);
                decoder->private_->local_lpc_restore_signal(ptemp, block_size - order, subframe->qlp_coeff, order, subframe->quantization_level, pOutputAddOrder);
#ifdef HALF_FRAME_BY_HALF_FRAME
                SubframeInfo[channel].restore_signal_type = FLAC__RESTORE_SIGNAL_TYPE_DEFAULT;
#endif
            }
        }
        else
        {
            //decoder->private_->local_lpc_restore_signal_64bit(decoder->private_->residual[channel]+order, decoder->private_->frame.header.blocksize-order, subframe->qlp_coeff, order, subframe->quantization_level, decoder->private_->output[channel]+order);
            decoder->private_->local_lpc_restore_signal_64bit(ptemp, block_size - order, subframe->qlp_coeff, order, subframe->quantization_level, pOutputAddOrder);
#ifdef HALF_FRAME_BY_HALF_FRAME
            SubframeInfo[channel].restore_signal_type = FLAC__RESTORE_SIGNAL_TYPE_64BITS;
#endif
        }
    }
#ifdef HALF_FRAME_BY_HALF_FRAME
//保留order个数据供下半帧解码
    if(isNeedDecByHalfFrmae)
         memcpy(g_FlacOutputBuffer[channel] + (FLAC__MAX_LPC_ORDER - order),pOutputAddOrder + block_size - order - order,order*sizeof(FLAC__int32));
#endif

    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC__bool read_subframe_verbatim_(FLAC__StreamDecoder *decoder, unsigned channel, unsigned bps, FLAC__bool do_full_decode)
{
    //FLAC__Subframe_Verbatim *subframe = &decoder->private_->frame.subframes[channel].data.verbatim;
    FLAC__int32 x, *residual = decoder->private_->residual[channel];
    unsigned i;

//#ifdef HALF_FRAME_BY_HALF_FRAME
//  unsigned block_size = decoder->private_->frame.header.blocksize/2;
//#else
    unsigned block_size = decoder->private_->frame.header.blocksize;
//#endif
#ifdef HALF_FRAME_BY_HALF_FRAME
    block_size >>= isNeedDecByHalfFrmae;
#endif

    decoder->private_->frame.subframes[channel].type = FLAC__SUBFRAME_TYPE_VERBATIM;

    //subframe->data = residual;

    for (i = 0; i < block_size; i++)
    {
        if (!FLAC__bitbuffer_read_raw_int32(decoder->private_->input, &x, bps, read_callback_, decoder))
            return false; /* the read_callback_ sets the state for us */
        residual[i] = x;
    }
#ifdef HALF_FRAME_BY_HALF_FRAME
if(isNeedDecByHalfFrmae)
{
    //-----------------record frame information for post-half frame decoding----------
    //1 子帧分支和constant
    SubframeInfo[channel].subframeType = FLAC__SUBFRAME_TYPE_VERBATIM;
    SubframeInfo[channel].constant = x;

    //2 输入buffer信息
    SubframeInfo[channel].fileOffset = g_CurrFileOffset - FLAC__BITBUFFER_DEFAULT_CAPACITY;

    //3 bit buffer 的信息
    memcpy(&(SubframeInfo[channel].bitBuffer),decoder->private_->input,sizeof(FLAC__BitBuffer));

    //4 用于做预测的order个数据

    //-------------------record end-----------------------------------------------------

    //skip post-half frame
    for (i = 0; i < block_size; i++)
    {
        if (!FLAC__bitbuffer_read_raw_int32(decoder->private_->input, &x, bps, read_callback_, decoder))
            return false; /* the read_callback_ sets the state for us */
    }
}
#endif




    FLAC__ASSERT(0);

    /* decode the subframe */
    //if(do_full_decode)
    // memcpy(decoder->private_->output[channel], subframe->data, sizeof(FLAC__int32) * decoder->private_->frame.header.blocksize);
    //record information for posthalf frame decoding
    /*to do*/
    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC__bool read_residual_partitioned_rice_(FLAC__StreamDecoder *decoder, unsigned predictor_order, unsigned partition_order, FLAC__EntropyCodingMethod_PartitionedRiceContents *partitioned_rice_contents, FLAC__int32 *residual)
{
    FLAC__uint32 rice_parameter;
    int i;
    unsigned partition, sample, u;
    const unsigned partitions = 1u << partition_order;
    const unsigned partition_samples = partition_order > 0 ? decoder->private_->frame.header.blocksize >> partition_order : decoder->private_->frame.header.blocksize - predictor_order;
    FLAC__int32* tmpResidual;
#ifdef HALF_FRAME_BY_HALF_FRAME
    const unsigned half_block_size = decoder->private_->frame.header.blocksize/2;
#endif

    /* sanity checks */
    if (partition_order == 0)
    {
        if (decoder->private_->frame.header.blocksize < predictor_order)
        {
            decoder->private_->error_callback(decoder, FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC, decoder->private_->client_data);
            decoder->protected_->state = FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
            return true;
        }
    }
    else
    {
        if (partition_samples < predictor_order)
        {
            decoder->private_->error_callback(decoder, FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC, decoder->private_->client_data);
            decoder->protected_->state = FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
            return true;
        }
    }

    if (!FLAC__format_entropy_coding_method_partitioned_rice_contents_ensure_size(partitioned_rice_contents, max(6, partition_order)))
    {
        decoder->protected_->state = FLAC__STREAM_DECODER_MEMORY_ALLOCATION_ERROR;
        return false;
    }

    sample = 0;
#ifdef HALF_FRAME_BY_HALF_FRAME
if(isNeedDecByHalfFrmae)
{
    //---------------record frame information-----------
    FLAC__ASSERT(gFLACchannel >= 0);
    FLAC__ASSERT(gFLACchannel <= 1);
    //SubframeInfo[channel].constant = x;

    //2 输入buffer信息
    SubframeInfo[gFLACchannel].fileOffset = g_CurrFileOffset - FLAC__BITBUFFER_DEFAULT_CAPACITY;

    //3 bit buffer 的信息
    memcpy(&(SubframeInfo[gFLACchannel].bitBuffer),decoder->private_->input,sizeof(FLAC__BitBuffer));
    SubframeInfo[gFLACchannel].partition_order = partition_order;
}
#endif
    for (partition = 0; partition < partitions; partition++)
    {
        if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &rice_parameter, FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE_PARAMETER_LEN, read_callback_, decoder))
            return false; /* the read_callback_ sets the state for us */
        partitioned_rice_contents->parameters[partition] = rice_parameter;
        if (rice_parameter < FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE_ESCAPE_PARAMETER)
        {
            u = (partition_order == 0 || partition > 0) ? partition_samples : partition_samples - predictor_order;

            tmpResidual = residual + sample;

            //if(!FLAC__bitbuffer_read_rice_signed_block(decoder->private_->input, residual + sample, u, rice_parameter, read_callback_, decoder))
            if (!FLAC__bitbuffer_read_rice_signed_block(decoder->private_->input, tmpResidual, u, rice_parameter, read_callback_, decoder,sample + predictor_order))
                return false; /* the read_callback_ sets the state for us */
            sample += u;
        }
        else
        {
            if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &rice_parameter, FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE_RAW_LEN, read_callback_, decoder))
                return false; /* the read_callback_ sets the state for us */
            partitioned_rice_contents->raw_bits[partition] = rice_parameter;
            for (u = (partition_order == 0 || partition > 0) ? 0 : predictor_order; u < partition_samples; u++, sample++)
            {
                if (!FLAC__bitbuffer_read_raw_int32(decoder->private_->input, &i, rice_parameter, read_callback_, decoder))
                    return false; /* the read_callback_ sets the state for us */

#ifdef HALF_FRAME_BY_HALF_FRAME
                if((0 == isNeedDecByHalfFrmae) || ((sample + predictor_order)< half_block_size))
#endif
                   residual[sample] = i;
            }
        }
    }

    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC__bool read_zero_padding_(FLAC__StreamDecoder *decoder)
{
    if (!FLAC__bitbuffer_is_consumed_byte_aligned(decoder->private_->input))
    {
        FLAC__uint32 zero = 0;
        if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &zero, FLAC__bitbuffer_bits_left_for_byte_alignment(decoder->private_->input), read_callback_, decoder))
            return false; /* the read_callback_ sets the state for us */
        if (zero != 0)
        {
            decoder->private_->error_callback(decoder, FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC, decoder->private_->client_data);
            decoder->protected_->state = FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
        }
    }
    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC__bool read_callback_(FLAC__byte buffer[], unsigned *bytes, void *client_data)
{
    FLAC__StreamDecoder *decoder = (FLAC__StreamDecoder *)client_data;
    FLAC__StreamDecoderReadStatus status;

    status = decoder->private_->read_callback(decoder, buffer, bytes, decoder->private_->client_data);
    if (status == FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM)
        decoder->protected_->state = FLAC__STREAM_DECODER_END_OF_STREAM;
    else if (status == FLAC__STREAM_DECODER_READ_STATUS_ABORT)
        decoder->protected_->state = FLAC__STREAM_DECODER_ABORTED;
    return status == FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

#ifdef HALF_FRAME_BY_HALF_FRAME
_ATTR_FLACDEC_TEXT_
FLAC__bool post_read_subframe_(FLAC__StreamDecoder *decoder, unsigned channel, unsigned bps, FLAC__bool do_full_decode)
{
    FLAC__uint32 x,judge_wash_bits;
    unsigned block_size = decoder->private_->frame.header.blocksize/2;
    gFLACchannel = channel;
#if 0
    if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &x, 8, read_callback_, decoder)) /* MAGIC NUMBER */
        return false; /* the read_callback_ sets the state for us */

    wasted_bits = (x & 1);
    x &= 0xfe;

    if (wasted_bits)
    {
        unsigned u;
        if (!FLAC__bitbuffer_read_unary_unsigned(decoder->private_->input, &u, read_callback_, decoder))
            return false; /* the read_callback_ sets the state for us */
        decoder->private_->frame.subframes[channel].wasted_bits = u + 1;
        bps -= decoder->private_->frame.subframes[channel].wasted_bits;
    }
    else
        decoder->private_->frame.subframes[channel].wasted_bits = 0;

    /*
     * Lots of magic numbers here
     */
    // 预测类型
    if (x & 0x80)
    {
        decoder->private_->error_callback(decoder, FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC, decoder->private_->client_data);
        decoder->protected_->state = FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
        return true;
    }
    else if (x == 0)
    {
        if (!read_subframe_constant_(decoder, channel, bps, do_full_decode))
            return false;
    }
    else if (x == 2)
    {
        if (!read_subframe_verbatim_(decoder, channel, bps, do_full_decode))
            return false;
    }
    else if (x < 16)
    {
        decoder->protected_->state = FLAC__STREAM_DECODER_UNPARSEABLE_STREAM;
        return false;
    }
    else if (x <= 24)
    {
        if (!read_subframe_fixed_(decoder, channel, bps, (x >> 1)&7, do_full_decode))
            return false;
    }
    else if (x < 64)
    {
        decoder->protected_->state = FLAC__STREAM_DECODER_UNPARSEABLE_STREAM;
        return false;
    }
    else
    {
        if (!read_subframe_lpc_(decoder, channel, bps, ((x >> 1)&31) + 1, do_full_decode))
            return false;
    }
#endif
    switch(SubframeInfo[channel].subframeType)
    {
        case FLAC__SUBFRAME_TYPE_CONSTANT:
            if (!post_read_subframe_constant_(decoder, channel, bps, do_full_decode))
                return false;
            break;
        case FLAC__SUBFRAME_TYPE_VERBATIM:
            if (!post_read_subframe_verbatim_(decoder, channel, bps, do_full_decode))
                return false;
            break;
        case FLAC__SUBFRAME_TYPE_FIXED:
            if (!post_read_subframe_fixed_(decoder, channel, bps, (x >> 1)&7, do_full_decode))
                return false;
            break;
        case FLAC__SUBFRAME_TYPE_LPC:
            if (!post_read_subframe_lpc_(decoder, channel, bps, ((x >> 1)&31) + 1, do_full_decode))
                return false;
            break;
        default:
            decoder->protected_->state = FLAC__STREAM_DECODER_UNPARSEABLE_STREAM;
            return false;
    }

    judge_wash_bits = SubframeInfo[channel].judge_wasted_bits;
    if (judge_wash_bits && do_full_decode)
    {
        unsigned i;
        x = SubframeInfo[channel].wasted_bits;

        for (i = 0; i < block_size;i++)
            decoder->private_->output[channel][i] <<= x;
    }

    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC__bool post_read_subframe_constant_(FLAC__StreamDecoder *decoder, unsigned channel, unsigned bps, FLAC__bool do_full_decode)
{
//    FLAC__Subframe_Constant *subframe = &decoder->private_->frame.subframes[channel].data.constant;
    FLAC__int32 x = SubframeInfo[channel].constant;
    unsigned i;
    FLAC__int32 *output = decoder->private_->output[channel];
    unsigned block_size = decoder->private_->frame.header.blocksize/2;


//    if (!FLAC__bitbuffer_read_raw_int32(decoder->private_->input, &x, bps, read_callback_, decoder))
//        return false; /* the read_callback_ sets the state for us */

//    subframe->value = x;
#ifdef FLAC_MINUS_ZI_DATA
    if (0 == channel)
    {
        FLAC__int16* pOutput = (FLAC__int16*)output;
        if (do_full_decode)
        {
            for (i = 0; i < block_size; i++)
                pOutput[i] = x;
        }
    }
    else if (1 == channel)
    {
        if (do_full_decode)
        {
            for (i = 0; i < block_size; i++)
                output[i] = x;
        }
    }
    else
    {
        while (1);
    }

#else
    /* decode the subframe */
    if (do_full_decode)
    {
        for (i = 0; i < block_size; i++)
            output[i] = x;
    }
#endif
    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC__bool post_read_subframe_fixed_(FLAC__StreamDecoder *decoder, unsigned channel, unsigned bps, unsigned order, FLAC__bool do_full_decode)
{
    FLAC__Subframe_Fixed *subframe = &decoder->private_->frame.subframes[channel].data.fixed;
//    FLAC__int32 i32;
//    FLAC__uint32 u32;
    unsigned u;
    FLAC__int32* ptemp,*pOutput,*ppredictor;

    unsigned block_size = decoder->private_->frame.header.blocksize/2;

    //recover part of information
    order = SubframeInfo[channel].order;

#ifdef FLAC_MINUS_ZI_DATA
    if (channel == 0)
        ptemp = decoder->private_->residual[channel];
    else if (channel == 1)
        ptemp = decoder->private_->residual[channel];// + order;
#else
    ptemp = decoder->private_->residual[channel];// + order;
#endif
    /* read residual */
//    switch (subframe->entropy_coding_method.type)
    {
 //       case FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE:
            //if(!read_residual_partitioned_rice_(decoder, order, subframe->entropy_coding_method.data.partitioned_rice.order, &decoder->private_->partitioned_rice_contents[channel], decoder->private_->residual[channel]+order)) // 留出 order 个 int32 来放 warmup
            if (!post_read_residual_partitioned_rice_(decoder, order, subframe->entropy_coding_method.data.partitioned_rice.order, &decoder->private_->partitioned_rice_contents[channel], ptemp))
                return false;
//            break;
//        default:
//            FLAC__ASSERT(0);

    }

    /* decode the subframe */
    if (do_full_decode)
    {
#if 0//def FLAC_MINUS_ZI_DATA
        if (0 == channel)
        {
            pOutput = (FLAC__int32*)((FLAC__int16*)(decoder->private_->output[channel]) + order);
            memcpy(decoder->private_->output[0], subframe->warmup, sizeof(FLAC__int16) * order);
        }
        else if (1 == channel)
        {
            pOutput = decoder->private_->output[channel] + order;
            memcpy(decoder->private_->output[1], subframe->warmup, sizeof(FLAC__int32) * order);
        }
#else
        //order = SubframeInfo[channel].order;
        ptemp = decoder->private_->residual[channel];
        pOutput = decoder->private_->output[channel];// + order;
        //memcpy(decoder->private_->output[channel], subframe->warmup, sizeof(FLAC__int32) * order);
        #if 0
        //先获取用作预测的order个数据
        ppredictor = SubframeInfo[channel].qlp_predictor.predictor;
        switch (SubframeInfo[channel].order)
        {
            case 0:
                break;
            case 1:
                pOutput[0] = ptemp[0] + ppredictor[0];
                break;
            case 2:
                    /* == residual[i] + 2*data[i-1] - data[i-2] */
                    //data[i] = residual[i] + (data[i-1] << 1) - data[i-2];
                pOutput[0] = ptemp[0] + (ppredictor[1]<<1) - ppredictor[0];
                pOutput[1] = ptemp[1] + (pOutput[0]<<1) - ppredictor[1];
                break;
            case 3:
                ///for (i = 0; i < idata_len; i++)
                {
                    /* residual[i] + 3*data[i-1] - 3*data[i-2]) + data[i-3] */
                    //data[i] = residual[i] + (((data[i-1] - data[i-2]) << 1) + (data[i-1] - data[i-2])) + data[i-3];
                    pOutput[0] = ptemp[0] + (((ppredictor[2] - ppredictor[1]) << 1) + (ppredictor[2] - ppredictor[1])) + ppredictor[0];
                    pOutput[1] = ptemp[1] + (((pOutput[0] - ppredictor[2]) << 1) + (pOutput[0] - ppredictor[2])) + ppredictor[1];
                    pOutput[2] = ptemp[2] + (((pOutput[1] - pOutput[0]) << 1) + (pOutput[1] - pOutput[0])) + ppredictor[2];
                }
                break;
            case 4:
               // for (i = 0; i < idata_len; i++)
                {
                    /* == residual[i] + 4*data[i-1] - 6*data[i-2] + 4*data[i-3] - data[i-4] */
                    //pDataTmp[i] = pResidual[i] + ((pDataTmp[i-1] + pDataTmp[i-3]) << 2) - ((pDataTmp[i-2] << 2) + (pDataTmp[i-2] << 1)) - pDataTmp[i-4];
                    pOutput[0] = ptemp[0] + ((ppredictor[3] + ppredictor[1]) << 2) - ((ppredictor[2] << 2) + (ppredictor[2] << 1)) - ppredictor[0];
                    pOutput[1] = ptemp[1] + ((pOutput[0] + ppredictor[2]) << 2) - ((ppredictor[3] << 2) + (ppredictor[3] << 1)) - ppredictor[1];
                    pOutput[2] = ptemp[2] + ((pOutput[1] + ppredictor[3]) << 2) - ((pOutput[0] << 2) + (pOutput[0] << 1)) - ppredictor[2];
                    pOutput[3] = ptemp[3] + ((pOutput[2] + pOutput[0]) << 2) - ((pOutput[1] << 2) + (pOutput[1] << 1)) - ppredictor[3];
                }
                break;
            default:
                FLAC__ASSERT(0);
        }
        #endif
//      pOutput += order;
//        ptemp += order;
#endif
        //FLAC__fixed_restore_signal(decoder->private_->residual[channel]+order, decoder->private_->frame.header.blocksize-order, order, decoder->private_->output[channel]+order);
        FLAC__fixed_restore_signal(ptemp, block_size, order, pOutput);

    }

#if 0//def HALF_FRAME_BY_HALF_FRAME
    //1 subframe type
    SubframeInfo[channel].subframeType = FLAC__SUBFRAME_TYPE_FIXED;

    //2 data for prediction
    for(u = 0;u < order;u++)
    {
        SubframeInfo[channel].qlp_predictor.predictor[u] = pOutput[block_size - order+u];
    }

#endif
    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC__bool post_read_subframe_lpc_(FLAC__StreamDecoder *decoder, unsigned channel, unsigned bps, unsigned order, FLAC__bool do_full_decode)
{
    FLAC__Subframe_LPC *subframe = &decoder->private_->frame.subframes[channel].data.lpc;
    FLAC__int32 i32;
    FLAC__uint32 u32;
    unsigned u;
    FLAC__int32 *ptemp;
    unsigned block_size = decoder->private_->frame.header.blocksize/2;


    subframe->residual = decoder->private_->residual[channel];
    order = subframe->order = SubframeInfo[channel].order;

    ptemp = decoder->private_->residual[channel];// + order;

#if 1//def HALF_FRAME_BY_HALF_FRAME
     //recover information
//  SubframeInfo[channel].subframeType = FLAC__SUBFRAME_TYPE_LPC;
    subframe->quantization_level = SubframeInfo[channel].quantization_level;
    memcpy(subframe->qlp_coeff,SubframeInfo[channel].qlp_predictor.qlp_coeff,sizeof(subframe->qlp_coeff));
#endif
//while(0 == subframe->entropy_coding_method.data.partitioned_rice.order);
    /* read residual */
    switch (subframe->entropy_coding_method.type)
    {
        case FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE:
            //if(!read_residual_partitioned_rice_(decoder, order, subframe->entropy_coding_method.data.partitioned_rice.order, &decoder->private_->partitioned_rice_contents[channel], decoder->private_->residual[channel]+order))// 留出 order 个 int32 来放 warmup 样值
            if (!post_read_residual_partitioned_rice_(decoder, order, subframe->entropy_coding_method.data.partitioned_rice.order, &decoder->private_->partitioned_rice_contents[channel], ptemp))// 留出 order 个 int32 来放 warmup 样值
                return false;
            break;
        default:
            FLAC__ASSERT(0);
    }

    /* decode the subframe */
    if (do_full_decode)
    {
        FLAC__int32* pOutputAddOrder;

        pOutputAddOrder = decoder->private_->output[channel];// + order;

        switch(SubframeInfo[channel].restore_signal_type)
        {
            case FLAC__RESTORE_SIGNAL_TYPE_16BITS_ORDER8:
                decoder->private_->local_lpc_restore_signal_16bit_order8(ptemp, block_size, subframe->qlp_coeff, order, subframe->quantization_level, pOutputAddOrder);
                break;
            case FLAC__RESTORE_SIGNAL_TYPE_16BITS:
                decoder->private_->local_lpc_restore_signal_16bit(ptemp, block_size, subframe->qlp_coeff, order, subframe->quantization_level, pOutputAddOrder);
                break;
            case FLAC__RESTORE_SIGNAL_TYPE_DEFAULT:
                decoder->private_->local_lpc_restore_signal(ptemp, block_size, subframe->qlp_coeff, order, subframe->quantization_level, pOutputAddOrder);
                break;
            case FLAC__RESTORE_SIGNAL_TYPE_64BITS:
                decoder->private_->local_lpc_restore_signal_64bit(ptemp, block_size, subframe->qlp_coeff, order, subframe->quantization_level, pOutputAddOrder);
                break;
            default:
                FLAC__ASSERT(0);
        }
    }

    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC__bool post_read_subframe_verbatim_(FLAC__StreamDecoder *decoder, unsigned channel, unsigned bps, FLAC__bool do_full_decode)
{
    FLAC__int32 x, *residual = decoder->private_->residual[channel];
    unsigned i;

    unsigned block_size = decoder->private_->frame.header.blocksize/2;
#ifdef FLAC_MINUS_ZI_DATA
    if (0 == channel)
    {
        FLAC__int16* presidual = (FLAC__int16*)residual;
        for (i = 0; i < block_size; i++)
        {
            if (!FLAC__bitbuffer_read_raw_int32(decoder->private_->input, &x, bps, read_callback_, decoder))
                return false; /* the read_callback_ sets the state for us */
            presidual[i] = x;
        }
    }
    else if (1 == channel)
    {
        for (i = 0; i < block_size; i++)
        {
            if (!FLAC__bitbuffer_read_raw_int32(decoder->private_->input, &x, bps, read_callback_, decoder))
                return false; /* the read_callback_ sets the state for us */
            residual[i] = x;
        }
    }
    else
    {
        while (1);
    }

#else
    {
        FLAC__BitBuffer* pBitBuffer = &(SubframeInfo[gFLACchannel].bitBuffer);
        //recover subframe information
        x = SubframeInfo[channel].constant;

        //recover input buffer

        //if(0 == channel)
#ifdef FLAC_FSEEK_OPT
        {
            int Offset,Clus;
            flac_fseek(g_hFlacFileBake, SubframeInfo[gFLACchannel].fileOffset, 0);
            FLAC_FileGetSeekInfo(&Offset, &Clus, g_hFlacFileBake);
            FLAC_FileSeekFast(Offset, Clus, g_hFlacFile);
        }
#else
        flac_fseek(g_hFlacFile, SubframeInfo[gFLACchannel].fileOffset, 0);
#endif
    //  flac_fread(g_FlacInputBuffer, 1, FLAC__BITBUFFER_DEFAULT_CAPACITY, g_hFlacFile);
        flac_fread(pBitBuffer->buffer + pBitBuffer->blurbs - pBitBuffer->capacity, 1, FLAC__BITBUFFER_DEFAULT_CAPACITY, g_hFlacFile);
    //  g_CurrFileOffset += FLAC__BITBUFFER_DEFAULT_CAPACITY;

        //3 recover bit buffer
        memcpy(decoder->private_->input,&(SubframeInfo[gFLACchannel].bitBuffer),sizeof(FLAC__BitBuffer));
    }

    for (i = 0; i < block_size; i++)
    {
        if (!FLAC__bitbuffer_read_raw_int32(decoder->private_->input, &x, bps, read_callback_, decoder))
            return false; /* the read_callback_ sets the state for us */
        residual[i] = x;
    }
#endif


    FLAC__ASSERT(0);

    /* decode the subframe */
    //if(do_full_decode)
    // memcpy(decoder->private_->output[channel], subframe->data, sizeof(FLAC__int32) * decoder->private_->frame.header.blocksize);

    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC__bool post_read_residual_partitioned_rice_(FLAC__StreamDecoder *decoder, unsigned predictor_order, unsigned partition_order, FLAC__EntropyCodingMethod_PartitionedRiceContents *partitioned_rice_contents, FLAC__int32 *residual)
{
    FLAC__uint32 rice_parameter;
    int i;
    unsigned partition, sample, u;
    const unsigned partitions = 1u << SubframeInfo[gFLACchannel].partition_order;
    const unsigned partition_samples = SubframeInfo[gFLACchannel].partition_order > 0 ? decoder->private_->frame.header.blocksize >> (SubframeInfo[gFLACchannel].partition_order) : decoder->private_->frame.header.blocksize - predictor_order;
    FLAC__int32* tmpResidual;
    const unsigned half_block_size = decoder->private_->frame.header.blocksize/2;

    FLAC__ASSERT(gFLACchannel >= 0);
    FLAC__ASSERT(gFLACchannel <= 1);

    partition_order = SubframeInfo[gFLACchannel].partition_order;
    predictor_order = SubframeInfo[gFLACchannel].order;


    /* sanity checks */
    if (partition_order == 0)
    {
        if (decoder->private_->frame.header.blocksize < predictor_order)
        {
            decoder->private_->error_callback(decoder, FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC, decoder->private_->client_data);
            decoder->protected_->state = FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
            return true;
        }
    }
    else
    {
        if (partition_samples < predictor_order)
        {
            decoder->private_->error_callback(decoder, FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC, decoder->private_->client_data);
            decoder->protected_->state = FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
            return true;
        }
    }

    if (!FLAC__format_entropy_coding_method_partitioned_rice_contents_ensure_size(partitioned_rice_contents, max(6, partition_order)))
    {
        decoder->protected_->state = FLAC__STREAM_DECODER_MEMORY_ALLOCATION_ERROR;
        return false;
    }

    sample = 0;
#if 1//def HALF_FRAME_BY_HALF_FRAME
    //---------------record frame information-----------
    {
        FLAC__BitBuffer* pBitBuffer = &(SubframeInfo[gFLACchannel].bitBuffer);
        //SubframeInfo[channel].constant = x;

        //2 恢复input buffer
        //if(0 == gFLACchannel)
//#ifdef FLAC_FSEEK_OPT
#ifdef 0
        {
            int Offset,Clus;
            flac_fseek(g_hFlacFileBake, SubframeInfo[gFLACchannel].fileOffset, 0);
            FLAC_FileGetSeekInfo(&Offset, &Clus, g_hFlacFileBake);
            FLAC_FileSeekFast(Offset, Clus, g_hFlacFile);
        }
#else
        flac_fseek(g_hFlacFile, SubframeInfo[gFLACchannel].fileOffset, 0);
#endif
    //    flac_fread(g_FlacInputBuffer, 1, FLAC__BITBUFFER_DEFAULT_CAPACITY, g_hFlacFile);
        flac_fread(pBitBuffer->buffer + pBitBuffer->blurbs - pBitBuffer->capacity, 1, FLAC__BITBUFFER_DEFAULT_CAPACITY, g_hFlacFile);


        //3 bit buffer 的信息
        memcpy(decoder->private_->input,&(SubframeInfo[gFLACchannel].bitBuffer),sizeof(FLAC__BitBuffer));
    }
#endif

    for (partition = 0; partition < partitions; partition++)
    {
        if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &rice_parameter, FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE_PARAMETER_LEN, read_callback_, decoder))
            return false; /* the read_callback_ sets the state for us */
        partitioned_rice_contents->parameters[partition] = rice_parameter;
        if (rice_parameter <= FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE_ESCAPE_PARAMETER)
        {
            u = (partition_order == 0 || partition > 0) ? partition_samples : partition_samples - predictor_order;
#ifdef FLAC_MINUS_ZI_DATA
            if (0 == gFLACchannel)
                tmpResidual = (FLAC__int32*)((FLAC__int16*)residual + sample);
            else if (1 == gFLACchannel)
                tmpResidual = residual + sample;
#else
            tmpResidual = residual + sample + predictor_order - half_block_size;
#endif
                //  printf("L3346 %d block size %d %d %d\n",partition,decoder->private_->frame.header.blocksize,decoder->private_->frame.header.channels,partitions);

            //if(!FLAC__bitbuffer_read_rice_signed_block(decoder->private_->input, residual + sample, u, rice_parameter, read_callback_, decoder))
            if (!post_FLAC__bitbuffer_read_rice_signed_block(decoder->private_->input, tmpResidual, u, rice_parameter, read_callback_, decoder,sample + predictor_order))
                return false; /* the read_callback_ sets the state for us */
            //   printf("L3350 block size %d %d\n",decoder->private_->frame.header.blocksize,decoder->private_->frame.header.channels);

            sample += u;
        }
        else
        {
            if (!FLAC__bitbuffer_read_raw_uint32(decoder->private_->input, &rice_parameter, FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE_RAW_LEN, read_callback_, decoder))
                return false; /* the read_callback_ sets the state for us */
            partitioned_rice_contents->raw_bits[partition] = rice_parameter;

            for (u = (partition_order == 0 || partition > 0) ? 0 : predictor_order; u < partition_samples; u++, sample++)
            {
                if (!FLAC__bitbuffer_read_raw_int32(decoder->private_->input, &i, rice_parameter, read_callback_, decoder))
                    return false; /* the read_callback_ sets the state for us */
#ifdef FLAC_MINUS_ZI_DATA
                if (0 == gFLACchannel)
                    *((FLAC__int16*)residual + sample) = (FLAC__int16)i;
                else if (1 == gFLACchannel)
                    residual[sample] = i;
#else
                if(sample+predictor_order >= half_block_size)
                residual[sample] = i;

#endif
            }

        }
    }

    return true;
}


#endif

#endif
#endif
