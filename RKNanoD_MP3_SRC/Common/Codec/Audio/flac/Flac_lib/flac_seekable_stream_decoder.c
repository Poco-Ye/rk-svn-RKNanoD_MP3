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
#include <stdlib.h> /* for calloc() */
#include <string.h> /* for memcpy()/memcmp() */
#include "assert.h"
#include "seekable_stream_decoder.h"
#include "stream_decoder.h"
#include "float.h" /* for FLAC__double */
//#include "private/md5.h"
#include "replacer.h"

/* adjust for compilers that can't understand using LLU suffix for uint64_t literals */
#ifdef _MSC_VER
#define FLAC__U64L(x) x
#else
#define FLAC__U64L(x) x##LLU
#endif
extern FLAC__bool isPostHalfFrame;
/***********************************************************************
 *
 * Private class method prototypes
 *
 ***********************************************************************/

static void set_defaults_(FLAC__SeekableStreamDecoder *decoder);
static FLAC__StreamDecoderReadStatus read_callback_(FLAC__StreamDecoder *decoder, FLAC__byte buffer[], unsigned *bytes, void *client_data);
static FLAC__StreamDecoderWriteStatus write_callback_(FLAC__StreamDecoder *decoder, FLAC__Frame *frame, FLAC__int32 * buffer[], void *client_data);
static void metadata_callback_(FLAC__StreamDecoder *decoder, FLAC__StreamMetadata *metadata, void *client_data);
static void error_callback_(FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data);
static FLAC__bool seek_to_absolute_sample_(FLAC__SeekableStreamDecoder *decoder, FLAC__uint64 stream_length, FLAC__uint64 target_sample);

/***********************************************************************
 *
 * Private class data
 *
 ***********************************************************************/

typedef struct FLAC__SeekableStreamDecoderPrivate
{
    FLAC__SeekableStreamDecoderReadCallback read_callback;
    FLAC__SeekableStreamDecoderSeekCallback seek_callback;
    FLAC__SeekableStreamDecoderTellCallback tell_callback;
    FLAC__SeekableStreamDecoderLengthCallback length_callback;
    FLAC__SeekableStreamDecoderEofCallback eof_callback;
    FLAC__SeekableStreamDecoderWriteCallback write_callback;
    FLAC__SeekableStreamDecoderMetadataCallback metadata_callback;
    FLAC__SeekableStreamDecoderErrorCallback error_callback;
    void *client_data;
    FLAC__StreamDecoder *stream_decoder;
    FLAC__bool do_md5_checking; /* initially gets protected_->md5_checking but is turned off after a seek */
// struct FLAC__MD5Context md5context;
    FLAC__byte stored_md5sum[16]; /* this is what is stored in the metadata */
    FLAC__byte computed_md5sum[16]; /* this is the sum we computed from the decoded data */
    /* the rest of these are only used for seeking: */
    FLAC__StreamMetadata_StreamInfo stream_info; /* we keep this around so we can figure out how to seek quickly */
    FLAC__StreamMetadata_SeekTable *seek_table; /* we hold a pointer to the stream decoder's seek table for the same reason */
    /* Since we always want to see the STREAMINFO and SEEK_TABLE blocks at this level, we need some extra flags to keep track of whether they should be passed on up through the metadata_callback */
    FLAC__bool ignore_stream_info_block;
    FLAC__bool ignore_seek_table_block;
    FLAC__Frame last_frame; /* holds the info of the last frame we seeked to */
    FLAC__uint64 target_sample;
} FLAC__SeekableStreamDecoderPrivate;

_ATTR_FLACDEC_BSS_
FLAC__SeekableStreamDecoder g_FLAC__SeekableStreamDecoder;
_ATTR_FLACDEC_BSS_
FLAC__SeekableStreamDecoderProtected g_FLAC__SeekableStreamDecoderProtected;
_ATTR_FLACDEC_BSS_
FLAC__SeekableStreamDecoderPrivate g_FLAC__SeekableStreamDecoderPrivate;

/***********************************************************************
 *
 * Public static class data
 *
 ***********************************************************************/


_ATTR_FLACDEC_BSS_
FLAC_API char ** FLAC__SeekableStreamDecoderStateString;
/*
_ATTR_FLACDEC_DATA_
FLAC_API char * FLAC__SeekableStreamDecoderStateString[] =
{
    "FLAC__SEEKABLE_STREAM_DECODER_OK",
    "FLAC__SEEKABLE_STREAM_DECODER_SEEKING",
    "FLAC__SEEKABLE_STREAM_DECODER_END_OF_STREAM",
    "FLAC__SEEKABLE_STREAM_DECODER_MEMORY_ALLOCATION_ERROR",
    "FLAC__SEEKABLE_STREAM_DECODER_STREAM_DECODER_ERROR",
    "FLAC__SEEKABLE_STREAM_DECODER_READ_ERROR",
    "FLAC__SEEKABLE_STREAM_DECODER_SEEK_ERROR",
    "FLAC__SEEKABLE_STREAM_DECODER_ALREADY_INITIALIZED",
    "FLAC__SEEKABLE_STREAM_DECODER_INVALID_CALLBACK",
    "FLAC__SEEKABLE_STREAM_DECODER_UNINITIALIZED"
};
*/

_ATTR_FLACDEC_BSS_
FLAC_API char ** FLAC__SeekableStreamDecoderReadStatusString;
/*
_ATTR_FLACDEC_DATA_
FLAC_API char * FLAC__SeekableStreamDecoderReadStatusString[] =
{
    "FLAC__SEEKABLE_STREAM_DECODER_READ_STATUS_OK",
    "FLAC__SEEKABLE_STREAM_DECODER_READ_STATUS_ERROR"
};
*/

_ATTR_FLACDEC_BSS_
FLAC_API char ** FLAC__SeekableStreamDecoderSeekStatusString;
/*
_ATTR_FLACDEC_DATA_
FLAC_API char * FLAC__SeekableStreamDecoderSeekStatusString[] =
{
    "FLAC__SEEKABLE_STREAM_DECODER_SEEK_STATUS_OK",
    "FLAC__SEEKABLE_STREAM_DECODER_SEEK_STATUS_ERROR"
};
*/

_ATTR_FLACDEC_BSS_
FLAC_API char ** FLAC__SeekableStreamDecoderTellStatusString;
/*
_ATTR_FLACDEC_DATA_
FLAC_API char * FLAC__SeekableStreamDecoderTellStatusString[] =
{
    "FLAC__SEEKABLE_STREAM_DECODER_TELL_STATUS_OK",
    "FLAC__SEEKABLE_STREAM_DECODER_TELL_STATUS_ERROR"
};
*/

_ATTR_FLACDEC_BSS_
FLAC_API char ** FLAC__SeekableStreamDecoderLengthStatusString;
/*
_ATTR_FLACDEC_DATA_
FLAC_API char * FLAC__SeekableStreamDecoderLengthStatusString[] =
{
    "FLAC__SEEKABLE_STREAM_DECODER_LENGTH_STATUS_OK",
    "FLAC__SEEKABLE_STREAM_DECODER_LENGTH_STATUS_ERROR"
};
*/


/***********************************************************************
 *
 * Class constructor/destructor
 *
 ***********************************************************************/
#if 0
FLAC_API FLAC__SeekableStreamDecoder *FLAC__seekable_stream_decoder_new()
{
    FLAC__SeekableStreamDecoder *decoder;

    FLAC__ASSERT(sizeof(int) >= 4); /* we want to die right away if this is not true */

    decoder = (FLAC__SeekableStreamDecoder*)calloc(1, sizeof(FLAC__SeekableStreamDecoder));
    if (decoder == 0)
    {
        return 0;
    }

    decoder->protected_ = (FLAC__SeekableStreamDecoderProtected*)calloc(1, sizeof(FLAC__SeekableStreamDecoderProtected));
    if (decoder->protected_ == 0)
    {
        flac_free(decoder);
        return 0;
    }

    decoder->private_ = (FLAC__SeekableStreamDecoderPrivate*)calloc(1, sizeof(FLAC__SeekableStreamDecoderPrivate));
    if (decoder->private_ == 0)
    {
        flac_free(decoder->protected_);
        flac_free(decoder);
        return 0;
    }

    decoder->private_->stream_decoder = FLAC__stream_decoder_new();
    if (0 == decoder->private_->stream_decoder)
    {
        flac_free(decoder->private_);
        flac_free(decoder->protected_);
        flac_free(decoder);
        return 0;
    }

    set_defaults_(decoder);

    decoder->protected_->state = FLAC__SEEKABLE_STREAM_DECODER_UNINITIALIZED;

    return decoder;
}
#else
_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__SeekableStreamDecoder *FLAC__seekable_stream_decoder_new()
{
    FLAC__SeekableStreamDecoder *decoder;

    FLAC__ASSERT(sizeof(int) >= 4); /* we want to die right away if this is not true */

    decoder = (FLAC__SeekableStreamDecoder*) & g_FLAC__SeekableStreamDecoder;
    memset(&g_FLAC__SeekableStreamDecoder, 0, sizeof(FLAC__SeekableStreamDecoder));

    decoder->protected_ = (FLAC__SeekableStreamDecoderProtected*) & g_FLAC__SeekableStreamDecoderProtected;
    memset(&g_FLAC__SeekableStreamDecoderProtected, 0, sizeof(FLAC__SeekableStreamDecoderProtected));

    decoder->private_ = (FLAC__SeekableStreamDecoderPrivate*) & g_FLAC__SeekableStreamDecoderPrivate;
    memset(&g_FLAC__SeekableStreamDecoderPrivate, 0, sizeof(FLAC__SeekableStreamDecoderPrivate));

    decoder->private_->stream_decoder = FLAC__stream_decoder_new();

    set_defaults_(decoder);

    decoder->protected_->state = FLAC__SEEKABLE_STREAM_DECODER_UNINITIALIZED;

    return decoder;
}
#endif


/***********************************************************************
 *
 * Public class methods
 *
 ***********************************************************************/
_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__SeekableStreamDecoderState FLAC__seekable_stream_decoder_init(FLAC__SeekableStreamDecoder *decoder)
{
    FLAC__ASSERT(0 != decoder);

    if (decoder->protected_->state != FLAC__SEEKABLE_STREAM_DECODER_UNINITIALIZED)
        return decoder->protected_->state = FLAC__SEEKABLE_STREAM_DECODER_ALREADY_INITIALIZED;

    if (0 == decoder->private_->read_callback || 0 == decoder->private_->seek_callback || 0 == decoder->private_->tell_callback || 0 == decoder->private_->length_callback || 0 == decoder->private_->eof_callback)
        return decoder->protected_->state = FLAC__SEEKABLE_STREAM_DECODER_INVALID_CALLBACK;

    if (0 == decoder->private_->write_callback || 0 == decoder->private_->metadata_callback || 0 == decoder->private_->error_callback)
        return decoder->protected_->state = FLAC__SEEKABLE_STREAM_DECODER_INVALID_CALLBACK;

    decoder->private_->seek_table = 0;

    decoder->private_->do_md5_checking = decoder->protected_->md5_checking;

    /* We initialize the FLAC__MD5Context even though we may never use it.  This
     * is because md5 checking may be turned on to start and then turned off if
     * a seek occurs.  So we always init the context here and finalize it in
     * FLAC__seekable_stream_decoder_finish() to make sure things are always
     * cleaned up properly.
     */
    //FLAC__MD5Init(&decoder->private_->md5context);

    FLAC__stream_decoder_set_read_callback(decoder->private_->stream_decoder, read_callback_);
    FLAC__stream_decoder_set_write_callback(decoder->private_->stream_decoder, write_callback_);
    FLAC__stream_decoder_set_metadata_callback(decoder->private_->stream_decoder, metadata_callback_);
    FLAC__stream_decoder_set_error_callback(decoder->private_->stream_decoder, error_callback_);
    FLAC__stream_decoder_set_client_data(decoder->private_->stream_decoder, decoder);

    /* We always want to see these blocks.  Whether or not we pass them up
     * through the metadata callback will be determined by flags set in our
     * implementation of ..._set_metadata_respond/ignore...()
     */
    FLAC__stream_decoder_set_metadata_respond(decoder->private_->stream_decoder, FLAC__METADATA_TYPE_STREAMINFO);
    FLAC__stream_decoder_set_metadata_respond(decoder->private_->stream_decoder, FLAC__METADATA_TYPE_SEEKTABLE);

    if (FLAC__stream_decoder_init(decoder->private_->stream_decoder) != FLAC__STREAM_DECODER_SEARCH_FOR_METADATA)
        return decoder->protected_->state = FLAC__SEEKABLE_STREAM_DECODER_STREAM_DECODER_ERROR;

    return decoder->protected_->state = FLAC__SEEKABLE_STREAM_DECODER_OK;
}

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__bool FLAC__seekable_stream_decoder_set_read_callback(FLAC__SeekableStreamDecoder *decoder, FLAC__SeekableStreamDecoderReadCallback value)
{
    FLAC__ASSERT(0 != decoder);
    FLAC__ASSERT(0 != decoder->private_);
    FLAC__ASSERT(0 != decoder->protected_);
    if (decoder->protected_->state != FLAC__SEEKABLE_STREAM_DECODER_UNINITIALIZED)
        return false;
    decoder->private_->read_callback = value;
    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__bool FLAC__seekable_stream_decoder_set_seek_callback(FLAC__SeekableStreamDecoder *decoder, FLAC__SeekableStreamDecoderSeekCallback value)
{
    FLAC__ASSERT(0 != decoder);
    FLAC__ASSERT(0 != decoder->private_);
    FLAC__ASSERT(0 != decoder->protected_);
    if (decoder->protected_->state != FLAC__SEEKABLE_STREAM_DECODER_UNINITIALIZED)
        return false;
    decoder->private_->seek_callback = value;
    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__bool FLAC__seekable_stream_decoder_set_tell_callback(FLAC__SeekableStreamDecoder *decoder, FLAC__SeekableStreamDecoderTellCallback value)
{
    FLAC__ASSERT(0 != decoder);
    FLAC__ASSERT(0 != decoder->private_);
    FLAC__ASSERT(0 != decoder->protected_);
    if (decoder->protected_->state != FLAC__SEEKABLE_STREAM_DECODER_UNINITIALIZED)
        return false;
    decoder->private_->tell_callback = value;
    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__bool FLAC__seekable_stream_decoder_set_length_callback(FLAC__SeekableStreamDecoder *decoder, FLAC__SeekableStreamDecoderLengthCallback value)
{
    FLAC__ASSERT(0 != decoder);
    FLAC__ASSERT(0 != decoder->private_);
    FLAC__ASSERT(0 != decoder->protected_);
    if (decoder->protected_->state != FLAC__SEEKABLE_STREAM_DECODER_UNINITIALIZED)
        return false;
    decoder->private_->length_callback = value;
    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__bool FLAC__seekable_stream_decoder_set_eof_callback(FLAC__SeekableStreamDecoder *decoder, FLAC__SeekableStreamDecoderEofCallback value)
{
    FLAC__ASSERT(0 != decoder);
    FLAC__ASSERT(0 != decoder->private_);
    FLAC__ASSERT(0 != decoder->protected_);
    if (decoder->protected_->state != FLAC__SEEKABLE_STREAM_DECODER_UNINITIALIZED)
        return false;
    decoder->private_->eof_callback = value;
    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__bool FLAC__seekable_stream_decoder_set_write_callback(FLAC__SeekableStreamDecoder *decoder, FLAC__SeekableStreamDecoderWriteCallback value)
{
    FLAC__ASSERT(0 != decoder);
    FLAC__ASSERT(0 != decoder->private_);
    FLAC__ASSERT(0 != decoder->protected_);
    if (decoder->protected_->state != FLAC__SEEKABLE_STREAM_DECODER_UNINITIALIZED)
        return false;
    decoder->private_->write_callback = value;
    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__bool FLAC__seekable_stream_decoder_set_metadata_callback(FLAC__SeekableStreamDecoder *decoder, FLAC__SeekableStreamDecoderMetadataCallback value)
{
    FLAC__ASSERT(0 != decoder);
    FLAC__ASSERT(0 != decoder->private_);
    FLAC__ASSERT(0 != decoder->protected_);
    if (decoder->protected_->state != FLAC__SEEKABLE_STREAM_DECODER_UNINITIALIZED)
        return false;
    decoder->private_->metadata_callback = value;
    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__bool FLAC__seekable_stream_decoder_set_error_callback(FLAC__SeekableStreamDecoder *decoder, FLAC__SeekableStreamDecoderErrorCallback value)
{
    FLAC__ASSERT(0 != decoder);
    FLAC__ASSERT(0 != decoder->private_);
    FLAC__ASSERT(0 != decoder->protected_);
    if (decoder->protected_->state != FLAC__SEEKABLE_STREAM_DECODER_UNINITIALIZED)
        return false;
    decoder->private_->error_callback = value;
    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__bool FLAC__seekable_stream_decoder_set_client_data(FLAC__SeekableStreamDecoder *decoder, void *value)
{
    FLAC__ASSERT(0 != decoder);
    FLAC__ASSERT(0 != decoder->private_);
    FLAC__ASSERT(0 != decoder->protected_);
    if (decoder->protected_->state != FLAC__SEEKABLE_STREAM_DECODER_UNINITIALIZED)
        return false;
    decoder->private_->client_data = value;
    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__SeekableStreamDecoderState FLAC__seekable_stream_decoder_get_state(FLAC__SeekableStreamDecoder *decoder)
{
    FLAC__ASSERT(0 != decoder);
    FLAC__ASSERT(0 != decoder->protected_);
    return decoder->protected_->state;
}

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__StreamDecoderState FLAC__seekable_stream_decoder_get_stream_decoder_state(FLAC__SeekableStreamDecoder *decoder)
{
    FLAC__ASSERT(0 != decoder);
    FLAC__ASSERT(0 != decoder->private_);
    return FLAC__stream_decoder_get_state(decoder->private_->stream_decoder);
}

_ATTR_FLACDEC_TEXT_
FLAC_API const char *FLAC__seekable_stream_decoder_get_resolved_state_string(FLAC__SeekableStreamDecoder *decoder)
{
    if (decoder->protected_->state != FLAC__SEEKABLE_STREAM_DECODER_STREAM_DECODER_ERROR)
        return FLAC__SeekableStreamDecoderStateString[decoder->protected_->state];
    else
        return FLAC__stream_decoder_get_resolved_state_string(decoder->private_->stream_decoder);
}

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__bool FLAC__seekable_stream_decoder_process_until_end_of_metadata(FLAC__SeekableStreamDecoder *decoder)
{
    FLAC__bool ret;
    FLAC__ASSERT(0 != decoder);

    if (decoder->private_->stream_decoder->protected_->state == FLAC__STREAM_DECODER_END_OF_STREAM)
        decoder->protected_->state = FLAC__SEEKABLE_STREAM_DECODER_END_OF_STREAM;

    if (decoder->protected_->state == FLAC__SEEKABLE_STREAM_DECODER_END_OF_STREAM)
        return true;

    FLAC__ASSERT(decoder->protected_->state == FLAC__SEEKABLE_STREAM_DECODER_OK);

    ret = FLAC__stream_decoder_process_until_end_of_metadata(decoder->private_->stream_decoder);
    if (!ret)
        decoder->protected_->state = FLAC__SEEKABLE_STREAM_DECODER_STREAM_DECODER_ERROR;

    return ret;
}

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__bool FLAC__seekable_stream_decoder_process_until_end_of_stream(FLAC__SeekableStreamDecoder *decoder)
{
    FLAC__bool ret;
    FLAC__ASSERT(0 != decoder);

    if (decoder->private_->stream_decoder->protected_->state == FLAC__STREAM_DECODER_END_OF_STREAM)
        decoder->protected_->state = FLAC__SEEKABLE_STREAM_DECODER_END_OF_STREAM;

    if (decoder->protected_->state == FLAC__SEEKABLE_STREAM_DECODER_END_OF_STREAM)
        return true;

    //FLAC__ASSERT(decoder->protected_->state == FLAC__SEEKABLE_STREAM_DECODER_OK);
    if(isPostHalfFrame&&isNeedDecByHalfFrmae)//take care:isPostHalfFrame must be initialized as 0
    {
        ret = post_FLAC__stream_decoder_process_until_end_of_stream(decoder->private_->stream_decoder);
    }
    else
    {
        ret = FLAC__stream_decoder_process_until_end_of_stream(decoder->private_->stream_decoder);
    }
    if (!ret)
        decoder->protected_->state = FLAC__SEEKABLE_STREAM_DECODER_STREAM_DECODER_ERROR;

    return ret;
}

/***********************************************************************
 *
 * Private class methods
 *
 ***********************************************************************/
_ATTR_FLACDEC_TEXT_
void set_defaults_(FLAC__SeekableStreamDecoder *decoder)
{
    decoder->private_->read_callback = 0;
    decoder->private_->seek_callback = 0;
    decoder->private_->tell_callback = 0;
    decoder->private_->length_callback = 0;
    decoder->private_->eof_callback = 0;
    decoder->private_->write_callback = 0;
    decoder->private_->metadata_callback = 0;
    decoder->private_->error_callback = 0;
    decoder->private_->client_data = 0;
    /* WATCHOUT: these should match the default behavior of FLAC__StreamDecoder */
    decoder->private_->ignore_stream_info_block = false;
    decoder->private_->ignore_seek_table_block = true;

    decoder->protected_->md5_checking = false;
}

_ATTR_FLACDEC_TEXT_
FLAC__StreamDecoderReadStatus read_callback_(FLAC__StreamDecoder *decoder, FLAC__byte buffer[], unsigned *bytes, void *client_data)
{
    FLAC__SeekableStreamDecoder *seekable_stream_decoder = (FLAC__SeekableStreamDecoder *)client_data;
    unsigned bytesToRead = *bytes;
    (void)decoder;

    if (seekable_stream_decoder->private_->eof_callback(seekable_stream_decoder, seekable_stream_decoder->private_->client_data))
    {
        *bytes = 0;
#if 0
        /*@@@@@@ verify that this is not needed */
        seekable_stream_decoder->protected_->state = FLAC__SEEKABLE_STREAM_DECODER_END_OF_STREAM;
#endif
        return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
    }
    else if (*bytes > 0)
    {
        if (seekable_stream_decoder->private_->read_callback(seekable_stream_decoder, buffer, bytes, seekable_stream_decoder->private_->client_data) != FLAC__SEEKABLE_STREAM_DECODER_READ_STATUS_OK)
        {
            seekable_stream_decoder->protected_->state = FLAC__SEEKABLE_STREAM_DECODER_READ_ERROR;
            return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
        }
       // if (*bytes != bytesToRead)
        if(*bytes == 0)
        {
            return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
        }
        else
        {
            return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
        }
    }
    else
        return FLAC__STREAM_DECODER_READ_STATUS_ABORT; /* abort to avoid a deadlock */
}

_ATTR_FLACDEC_TEXT_
FLAC__StreamDecoderWriteStatus write_callback_(FLAC__StreamDecoder *decoder, FLAC__Frame *frame, FLAC__int32 * buffer[], void *client_data)
{
    FLAC__SeekableStreamDecoder *seekable_stream_decoder = (FLAC__SeekableStreamDecoder *)client_data;
    (void)decoder;

    if (seekable_stream_decoder->protected_->state == FLAC__SEEKABLE_STREAM_DECODER_SEEKING)
    {
        FLAC__uint64 this_frame_sample = frame->header.number.sample_number;
        FLAC__uint64 next_frame_sample = this_frame_sample + (FLAC__uint64)frame->header.blocksize;
        FLAC__uint64 target_sample = seekable_stream_decoder->private_->target_sample;

        FLAC__ASSERT(this_frame_sample < 0xffffffff);
        FLAC__ASSERT(next_frame_sample < 0xffffffff);
        FLAC__ASSERT(target_sample < 0xffffffff);

        FLAC__ASSERT(frame->header.number_type == FLAC__FRAME_NUMBER_TYPE_SAMPLE_NUMBER);

        while (1);//for debug,evan wu
        seekable_stream_decoder->private_->last_frame = *frame; /* save the frame */
        if (this_frame_sample <= target_sample && target_sample < next_frame_sample)  /* we hit our target frame */
        {
            unsigned delta = (unsigned)(target_sample - this_frame_sample);
            /* kick out of seek mode */
            seekable_stream_decoder->protected_->state = FLAC__SEEKABLE_STREAM_DECODER_OK;
            /* shift out the samples before target_sample */
            if (delta > 0)
            {
                unsigned channel;
                FLAC__int32 *newbuffer[FLAC__MAX_CHANNELS];
                for (channel = 0; channel < frame->header.channels; channel++)
                    newbuffer[channel] = buffer[channel] + delta;
                seekable_stream_decoder->private_->last_frame.header.blocksize -= delta;
                seekable_stream_decoder->private_->last_frame.header.number.sample_number += (FLAC__uint64)delta;
                FLAC__ASSERT(seekable_stream_decoder->private_->last_frame.header.number.sample_number < 0xffffffff);
                /* write the relevant samples */
                return seekable_stream_decoder->private_->write_callback(seekable_stream_decoder, &seekable_stream_decoder->private_->last_frame, newbuffer, seekable_stream_decoder->private_->client_data);
            }
            else
            {
                /* write the relevant samples */
                return seekable_stream_decoder->private_->write_callback(seekable_stream_decoder, frame, buffer, seekable_stream_decoder->private_->client_data);
            }
        }
        else
        {
            return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
        }
    }
    else
    {
        /*if(seekable_stream_decoder->private_->do_md5_checking) {
         if(!FLAC__MD5Accumulate(&seekable_stream_decoder->private_->md5context, buffer, frame->header.channels, frame->header.blocksize, (frame->header.bits_per_sample+7) / 8))
          return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
        }*/
        return seekable_stream_decoder->private_->write_callback(seekable_stream_decoder, frame, buffer, seekable_stream_decoder->private_->client_data);
    }
}

_ATTR_FLACDEC_TEXT_
void metadata_callback_(FLAC__StreamDecoder *decoder, FLAC__StreamMetadata *metadata, void *client_data)
{
    FLAC__SeekableStreamDecoder *seekable_stream_decoder = (FLAC__SeekableStreamDecoder *)client_data;
    (void)decoder;

    if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO)
    {
        seekable_stream_decoder->private_->stream_info = metadata->data.stream_info;
        /* save the MD5 signature for comparison later */
        memcpy(seekable_stream_decoder->private_->stored_md5sum, metadata->data.stream_info.md5sum, 16);
        //if(0 == memcmp(seekable_stream_decoder->private_->stored_md5sum, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 16))
        /* 不进行 MD5 校验 */
        seekable_stream_decoder->private_->do_md5_checking = false;
    }
    else if (metadata->type == FLAC__METADATA_TYPE_SEEKTABLE)
    {
        seekable_stream_decoder->private_->seek_table = &metadata->data.seek_table;
    }

    if (seekable_stream_decoder->protected_->state != FLAC__SEEKABLE_STREAM_DECODER_SEEKING)
    {
        FLAC__bool ignore_block = false;
        if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO && seekable_stream_decoder->private_->ignore_stream_info_block)
            ignore_block = true;
        else if (metadata->type == FLAC__METADATA_TYPE_SEEKTABLE && seekable_stream_decoder->private_->ignore_seek_table_block)
            ignore_block = true;
        if (!ignore_block)
            seekable_stream_decoder->private_->metadata_callback(seekable_stream_decoder, metadata, seekable_stream_decoder->private_->client_data);
    }
}

_ATTR_FLACDEC_TEXT_
void error_callback_(FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
    return;
    //FLAC__SeekableStreamDecoder *seekable_stream_decoder = (FLAC__SeekableStreamDecoder *)client_data;
    //(void)decoder;

    //if (seekable_stream_decoder->protected_->state != FLAC__SEEKABLE_STREAM_DECODER_SEEKING)
        //seekable_stream_decoder->private_->error_callback(seekable_stream_decoder, status, seekable_stream_decoder->private_->client_data);
}

#endif
#endif
