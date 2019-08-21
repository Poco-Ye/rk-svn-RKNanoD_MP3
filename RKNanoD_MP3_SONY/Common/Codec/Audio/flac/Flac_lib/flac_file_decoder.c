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

#if 0
#include <stdio.h>
#include <stdlib.h> /* for flac_malloc() */
#include <string.h> /* for strcmp() */
//#include <sys/stat.h> /* for stat() */
#if defined _MSC_VER || defined __MINGW32__
#include <io.h> /* for _setmode() */
#include <fcntl.h> /* for _O_BINARY */
#elif defined __CYGWIN__
#include <io.h> /* for setmode(), O_BINARY */
#include <fcntl.h> /* for _O_BINARY */
#endif

#endif

#include "assert.h"
#include "file_decoder.h"
#include "seekable_stream_decoder.h"

#define _REPLACER_FILE
#include "replacer.h"

extern FILE *g_hFlacFile,*g_hFlacFileBake;
//extern FILE *g_hWaveFile;

/***********************************************************************
 *
 * Private class method prototypes
 *
 ***********************************************************************/

static void set_defaults_(FLAC__FileDecoder *decoder);
static FILE *get_binary_stdin_();
static FLAC__SeekableStreamDecoderReadStatus read_callback_(const FLAC__SeekableStreamDecoder *decoder, FLAC__byte buffer[], unsigned *bytes, void *client_data);
static FLAC__SeekableStreamDecoderSeekStatus seek_callback_(const FLAC__SeekableStreamDecoder *decoder, FLAC__uint64 absolute_byte_offset, void *client_data);
static FLAC__SeekableStreamDecoderTellStatus tell_callback_(const FLAC__SeekableStreamDecoder *decoder, FLAC__uint64 *absolute_byte_offset, void *client_data);
static FLAC__SeekableStreamDecoderLengthStatus length_callback_(const FLAC__SeekableStreamDecoder *decoder, FLAC__uint64 *stream_length, void *client_data);
static FLAC__bool eof_callback_(const FLAC__SeekableStreamDecoder *decoder, void *client_data);
static FLAC__StreamDecoderWriteStatus write_callback_(const FLAC__SeekableStreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data);
static void metadata_callback_(const FLAC__SeekableStreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data);
static void error_callback_(const FLAC__SeekableStreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data);

/***********************************************************************
 *
 * Private class data
 *
 ***********************************************************************/

typedef struct FLAC__FileDecoderPrivate
{
    FLAC__FileDecoderWriteCallback write_callback;
    FLAC__FileDecoderMetadataCallback metadata_callback;
    FLAC__FileDecoderErrorCallback error_callback;
    void *client_data;
    FILE *file;
    char *filename; /* == NULL if stdin */
    FLAC__SeekableStreamDecoder *seekable_stream_decoder;
} FLAC__FileDecoderPrivate;

_ATTR_FLACDEC_BSS_
FLAC__FileDecoder g_FLAC__FileDecoder;
_ATTR_FLACDEC_BSS_
FLAC__FileDecoderProtected g_FLAC__FileDecoderProtected;
_ATTR_FLACDEC_BSS_
FLAC__FileDecoderPrivate g_FLAC__FileDecoderPrivate;

/***********************************************************************
 *
 * Public static class data
 *
 ***********************************************************************/
 _ATTR_FLACDEC_BSS_
FLAC_API char ** FLAC__FileDecoderStateString;
/*
_ATTR_FLACDEC_DATA_
FLAC_API char * FLAC__FileDecoderStateString[] =
{
    "FLAC__FILE_DECODER_OK",
    "FLAC__FILE_DECODER_END_OF_FILE",
    "FLAC__FILE_DECODER_ERROR_OPENING_FILE",
    "FLAC__FILE_DECODER_MEMORY_ALLOCATION_ERROR",
    "FLAC__FILE_DECODER_SEEK_ERROR",
    "FLAC__FILE_DECODER_SEEKABLE_STREAM_DECODER_ERROR",
    "FLAC__FILE_DECODER_ALREADY_INITIALIZED",
    "FLAC__FILE_DECODER_INVALID_CALLBACK",
    "FLAC__FILE_DECODER_UNINITIALIZED"
};
*/

/***********************************************************************
 *
 * Class constructor/destructor
 *
 ***********************************************************************/
_ATTR_FLACDEC_TEXT_
#if 0
FLAC_API FLAC__FileDecoder *FLAC__file_decoder_new()
{
    FLAC__FileDecoder *decoder;

    FLAC__ASSERT(sizeof(int) >= 4); /* we want to die right away if this is not true */

    decoder = (FLAC__FileDecoder*)calloc(1, sizeof(FLAC__FileDecoder));
    if (decoder == 0)
    {
        return 0;
    }

    decoder->protected_ = (FLAC__FileDecoderProtected*)calloc(1, sizeof(FLAC__FileDecoderProtected));
    if (decoder->protected_ == 0)
    {
        flac_free(decoder);
        return 0;
    }

    decoder->private_ = (FLAC__FileDecoderPrivate*)calloc(1, sizeof(FLAC__FileDecoderPrivate));
    if (decoder->private_ == 0)
    {
        flac_free(decoder->protected_);
        flac_free(decoder);
        return 0;
    }

    decoder->private_->seekable_stream_decoder = FLAC__seekable_stream_decoder_new();
    if (0 == decoder->private_->seekable_stream_decoder)
    {
        flac_free(decoder->private_);
        flac_free(decoder->protected_);
        flac_free(decoder);
        return 0;
    }

    decoder->private_->file = 0;

    set_defaults_(decoder);

    decoder->protected_->state = FLAC__FILE_DECODER_UNINITIALIZED;

    return decoder;
}
#else
FLAC_API FLAC__FileDecoder *FLAC__file_decoder_new()
{
    FLAC__FileDecoder *decoder;

    FLAC__ASSERT(sizeof(int) >= 4); /* we want to die right away if this is not true */

    decoder = (FLAC__FileDecoder*) & g_FLAC__FileDecoder;
    memset(&g_FLAC__FileDecoder, 0, sizeof(FLAC__FileDecoder));

    decoder->protected_ = (FLAC__FileDecoderProtected*) & g_FLAC__FileDecoderProtected;
    memset(&g_FLAC__FileDecoderProtected, 0, sizeof(FLAC__FileDecoderProtected));

    decoder->private_ = (FLAC__FileDecoderPrivate*) & g_FLAC__FileDecoderPrivate;
    memset(&g_FLAC__FileDecoderPrivate, 0, sizeof(FLAC__FileDecoderPrivate));

    decoder->private_->seekable_stream_decoder = FLAC__seekable_stream_decoder_new();

    decoder->private_->file = 0;

    set_defaults_(decoder);

    decoder->protected_->state = FLAC__FILE_DECODER_UNINITIALIZED;

    return decoder;
}
#endif


/***********************************************************************
 *
 * Public class methods
 *
 ***********************************************************************/
_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__FileDecoderState FLAC__file_decoder_init(FLAC__FileDecoder *decoder)
{
    FLAC__ASSERT(0 != decoder);

    if (decoder->protected_->state != FLAC__FILE_DECODER_UNINITIALIZED)
        return decoder->protected_->state = FLAC__FILE_DECODER_ALREADY_INITIALIZED;

    if (0 == decoder->private_->write_callback || 0 == decoder->private_->metadata_callback || 0 == decoder->private_->error_callback)
        return decoder->protected_->state = FLAC__FILE_DECODER_INVALID_CALLBACK;

    //if(0 == decoder->private_->filename)
    // decoder->private_->file = get_binary_stdin_();
    //else
    // decoder->private_->file = flac_fopen(decoder->private_->filename, "rb");
    decoder->private_->file = (FILE*)g_hFlacFile;

/* do not check file pointer : on rknano 0 is a correctly file number , by Vincent*/
#if 0
    if (decoder->private_->file == 0)
    {
        //DEBUG("Open flac file failed!\n");
        return decoder->protected_->state = FLAC__FILE_DECODER_ERROR_OPENING_FILE;
    }
#endif

    FLAC__seekable_stream_decoder_set_read_callback(decoder->private_->seekable_stream_decoder, read_callback_);
    FLAC__seekable_stream_decoder_set_seek_callback(decoder->private_->seekable_stream_decoder, seek_callback_);
    FLAC__seekable_stream_decoder_set_tell_callback(decoder->private_->seekable_stream_decoder, tell_callback_);
    FLAC__seekable_stream_decoder_set_length_callback(decoder->private_->seekable_stream_decoder, length_callback_);
    FLAC__seekable_stream_decoder_set_eof_callback(decoder->private_->seekable_stream_decoder, eof_callback_);
    FLAC__seekable_stream_decoder_set_write_callback(decoder->private_->seekable_stream_decoder, write_callback_);
    FLAC__seekable_stream_decoder_set_metadata_callback(decoder->private_->seekable_stream_decoder, metadata_callback_);
    FLAC__seekable_stream_decoder_set_error_callback(decoder->private_->seekable_stream_decoder, error_callback_);
    FLAC__seekable_stream_decoder_set_client_data(decoder->private_->seekable_stream_decoder, decoder);

    if (FLAC__seekable_stream_decoder_init(decoder->private_->seekable_stream_decoder) != FLAC__SEEKABLE_STREAM_DECODER_OK)
        return decoder->protected_->state = FLAC__FILE_DECODER_SEEKABLE_STREAM_DECODER_ERROR;

    return decoder->protected_->state = FLAC__FILE_DECODER_OK;
}

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__bool FLAC__file_decoder_set_write_callback(FLAC__FileDecoder *decoder, FLAC__FileDecoderWriteCallback value)
{
    FLAC__ASSERT(0 != decoder);
    FLAC__ASSERT(0 != decoder->private_);
    FLAC__ASSERT(0 != decoder->protected_);
    if (decoder->protected_->state != FLAC__FILE_DECODER_UNINITIALIZED)
        return false;
    decoder->private_->write_callback = value;
    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__bool FLAC__file_decoder_set_metadata_callback(FLAC__FileDecoder *decoder, FLAC__FileDecoderMetadataCallback value)
{
    FLAC__ASSERT(0 != decoder);
    FLAC__ASSERT(0 != decoder->private_);
    FLAC__ASSERT(0 != decoder->protected_);
    if (decoder->protected_->state != FLAC__FILE_DECODER_UNINITIALIZED)
        return false;
    decoder->private_->metadata_callback = value;
    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__bool FLAC__file_decoder_set_error_callback(FLAC__FileDecoder *decoder, FLAC__FileDecoderErrorCallback value)
{
    FLAC__ASSERT(0 != decoder);
    FLAC__ASSERT(0 != decoder->private_);
    FLAC__ASSERT(0 != decoder->protected_);
    if (decoder->protected_->state != FLAC__FILE_DECODER_UNINITIALIZED)
        return false;
    decoder->private_->error_callback = value;
    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__bool FLAC__file_decoder_set_client_data(FLAC__FileDecoder *decoder, void *value)
{
    FLAC__ASSERT(0 != decoder);
    FLAC__ASSERT(0 != decoder->private_);
    FLAC__ASSERT(0 != decoder->protected_);
    if (decoder->protected_->state != FLAC__FILE_DECODER_UNINITIALIZED)
        return false;
    decoder->private_->client_data = value;
    return true;
}

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__FileDecoderState FLAC__file_decoder_get_state(const FLAC__FileDecoder *decoder)
{
    FLAC__ASSERT(0 != decoder);
    FLAC__ASSERT(0 != decoder->protected_);
    return decoder->protected_->state;
}

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__SeekableStreamDecoderState FLAC__file_decoder_get_seekable_stream_decoder_state(const FLAC__FileDecoder *decoder)
{
    FLAC__ASSERT(0 != decoder);
    FLAC__ASSERT(0 != decoder->private_);
    return FLAC__seekable_stream_decoder_get_state(decoder->private_->seekable_stream_decoder);
}

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__StreamDecoderState FLAC__file_decoder_get_stream_decoder_state(const FLAC__FileDecoder *decoder)
{
    FLAC__ASSERT(0 != decoder);
    FLAC__ASSERT(0 != decoder->private_);
    return FLAC__seekable_stream_decoder_get_stream_decoder_state(decoder->private_->seekable_stream_decoder);
}

_ATTR_FLACDEC_TEXT_
FLAC_API const char *FLAC__file_decoder_get_resolved_state_string(const FLAC__FileDecoder *decoder)
{
    if (decoder->protected_->state != FLAC__FILE_DECODER_SEEKABLE_STREAM_DECODER_ERROR)		
        return FLAC__FileDecoderStateString[decoder->protected_->state];
    else
        return FLAC__seekable_stream_decoder_get_resolved_state_string(decoder->private_->seekable_stream_decoder);
}

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__bool FLAC__file_decoder_process_until_end_of_metadata(FLAC__FileDecoder *decoder)
{
    FLAC__bool ret;
    FLAC__ASSERT(0 != decoder);

    if (decoder->private_->seekable_stream_decoder->protected_->state == FLAC__SEEKABLE_STREAM_DECODER_END_OF_STREAM)
        decoder->protected_->state = FLAC__FILE_DECODER_END_OF_FILE;

    if (decoder->protected_->state == FLAC__FILE_DECODER_END_OF_FILE)
        return true;

    FLAC__ASSERT(decoder->protected_->state == FLAC__FILE_DECODER_OK);

    ret = FLAC__seekable_stream_decoder_process_until_end_of_metadata(decoder->private_->seekable_stream_decoder);
    if (!ret)
        decoder->protected_->state = FLAC__FILE_DECODER_SEEKABLE_STREAM_DECODER_ERROR;

    return ret;
}

_ATTR_FLACDEC_TEXT_
FLAC_API FLAC__bool FLAC__file_decoder_process_until_end_of_file(FLAC__FileDecoder *decoder)
{
    FLAC__bool ret;
    FLAC__ASSERT(0 != decoder);

    if (decoder->private_->seekable_stream_decoder->protected_->state == FLAC__SEEKABLE_STREAM_DECODER_END_OF_STREAM)
        decoder->protected_->state = FLAC__FILE_DECODER_END_OF_FILE;

    if (decoder->protected_->state == FLAC__FILE_DECODER_END_OF_FILE)
        return true;

    //FLAC__ASSERT(decoder->protected_->state == FLAC__FILE_DECODER_OK);

    ret = FLAC__seekable_stream_decoder_process_until_end_of_stream(decoder->private_->seekable_stream_decoder);
    if (!ret)
        decoder->protected_->state = FLAC__FILE_DECODER_SEEKABLE_STREAM_DECODER_ERROR;

    return ret;
}

/***********************************************************************
 *
 * Private class methods
 *
 ***********************************************************************/
_ATTR_FLACDEC_TEXT_
void set_defaults_(FLAC__FileDecoder *decoder)
{
    FLAC__ASSERT(0 != decoder);
    FLAC__ASSERT(0 != decoder->private_);

    decoder->private_->filename = 0;
    decoder->private_->write_callback = 0;
    decoder->private_->metadata_callback = 0;
    decoder->private_->error_callback = 0;
    decoder->private_->client_data = 0;
}


_ATTR_FLACDEC_TEXT_
FLAC__SeekableStreamDecoderReadStatus read_callback_(const FLAC__SeekableStreamDecoder *decoder, FLAC__byte buffer[], unsigned *bytes, void *client_data)
{
    FLAC__FileDecoder *file_decoder = (FLAC__FileDecoder *)client_data;
    (void)decoder;

    if (*bytes > 0)
    {
        *bytes = (unsigned)flac_fread(buffer, /*sizeof(FLAC__byte)*/1, *bytes, file_decoder->private_->file);
        return FLAC__SEEKABLE_STREAM_DECODER_READ_STATUS_OK;
    }
    else
        return FLAC__SEEKABLE_STREAM_DECODER_READ_STATUS_ERROR; /* abort to avoid a deadlock */
}

_ATTR_FLACDEC_TEXT_
FLAC__SeekableStreamDecoderSeekStatus seek_callback_(const FLAC__SeekableStreamDecoder *decoder, FLAC__uint64 absolute_byte_offset, void *client_data)
{
    FLAC__FileDecoder *file_decoder = (FLAC__FileDecoder *)client_data;
    (void)decoder;

    if (flac_fseek(file_decoder->private_->file, (long)absolute_byte_offset, SEEK_SET) < 0)
        return FLAC__SEEKABLE_STREAM_DECODER_SEEK_STATUS_ERROR;
    else
        return FLAC__SEEKABLE_STREAM_DECODER_SEEK_STATUS_OK;
}

_ATTR_FLACDEC_TEXT_
FLAC__SeekableStreamDecoderTellStatus tell_callback_(const FLAC__SeekableStreamDecoder *decoder, FLAC__uint64 *absolute_byte_offset, void *client_data)
{
    FLAC__FileDecoder *file_decoder = (FLAC__FileDecoder *)client_data;
    long pos;
    (void)decoder;

    if ((pos = flac_ftell(file_decoder->private_->file)) < 0)
        return FLAC__SEEKABLE_STREAM_DECODER_TELL_STATUS_ERROR;
    else
    {
        *absolute_byte_offset = (FLAC__uint64)pos;
        return FLAC__SEEKABLE_STREAM_DECODER_TELL_STATUS_OK;
    }
}

_ATTR_FLACDEC_TEXT_
FLAC__SeekableStreamDecoderLengthStatus length_callback_(const FLAC__SeekableStreamDecoder *decoder, FLAC__uint64 *stream_length, void *client_data)
{
#if 1
    return FLAC__SEEKABLE_STREAM_DECODER_LENGTH_STATUS_OK;
#else

    FLAC__FileDecoder *file_decoder = (FLAC__FileDecoder *)client_data;
    struct stat filestats;
    (void)decoder;

    if (0 == file_decoder->private_->filename || stat(file_decoder->private_->filename, &filestats) != 0)
        return FLAC__SEEKABLE_STREAM_DECODER_LENGTH_STATUS_ERROR;
    else
    {
        *stream_length = (FLAC__uint64)filestats.st_size;
        return FLAC__SEEKABLE_STREAM_DECODER_LENGTH_STATUS_OK;
    }
#endif
}

_ATTR_FLACDEC_TEXT_
FLAC__bool eof_callback_(const FLAC__SeekableStreamDecoder *decoder, void *client_data)
{
    FLAC__FileDecoder *file_decoder = (FLAC__FileDecoder *)client_data;
    (void)decoder;
	//todo by Vincent
    return false;// /*flac_feof*/(file_decoder->private_->file) ? true : false;
}

_ATTR_FLACDEC_TEXT_
FLAC__StreamDecoderWriteStatus write_callback_(const FLAC__SeekableStreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data)
{
    FLAC__FileDecoder *file_decoder = (FLAC__FileDecoder *)client_data;
    (void)decoder;

    return file_decoder->private_->write_callback(file_decoder, frame, buffer, file_decoder->private_->client_data);
}

_ATTR_FLACDEC_TEXT_
void metadata_callback_(const FLAC__SeekableStreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{
    FLAC__FileDecoder *file_decoder = (FLAC__FileDecoder *)client_data;
    (void)decoder;

    file_decoder->private_->metadata_callback(file_decoder, metadata, file_decoder->private_->client_data);
}

_ATTR_FLACDEC_TEXT_
void error_callback_(const FLAC__SeekableStreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
    FLAC__FileDecoder *file_decoder = (FLAC__FileDecoder *)client_data;
    (void)decoder;

    file_decoder->private_->error_callback(file_decoder, status, file_decoder->private_->client_data);
}

#endif
#endif
