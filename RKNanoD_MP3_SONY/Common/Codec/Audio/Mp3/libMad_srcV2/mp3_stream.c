/*
********************************************************************************
*                   Copyright (c) 2008, Rockchip
*                         All rights reserved.
*
* File Name��   stream.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             Vincent Hsiung     2009-1-14         1.0
*    desc:    ORG.
********************************************************************************
*/

/*
 1. finish()������free�ռ�Ĳ��ֲ���Ҫ,��ɾ��
 2. todo: mad_stream_errorstr()������nano�����岻��,ռ�ÿռ�,Ӧɾ��.

 by Vincent Hsiung.
*/

#include "../include/audio_main.h"

#ifdef A_CORE_DECODE
#ifdef MP3_DEC_INCLUDE

# include "mp3_global.h"

# include <stdlib.h>

# include "mp3_bit.h"
# include "mp3_stream.h"

// define a global structure for MainData  malloc and free are not implemented here
_ATTR_MP3DEC_BSS_
main_data_t MainData;

/*
 * NAME:	stream->init()
 * DESCRIPTION:	initialize stream struct
 */
_ATTR_MP3DEC_TEXT_
void mad_stream_init(struct mad_stream *stream)
{
  stream->buffer     = 0;
  stream->bufend     = 0;
  stream->skiplen    = 0;

  stream->sync       = 0;
  stream->freerate   = 0;

  stream->this_frame = 0;
  stream->next_frame = 0;
  mad_bit_init(&stream->ptr, 0);

  mad_bit_init(&stream->anc_ptr, 0);
  stream->anc_bitlen = 0;

  stream->main_data  = 0;
  stream->md_len     = 0;

  stream->options    = 0;
  stream->error      = MAD_ERROR_NONE;
}

/*
 * NAME:	stream->finish()
 * DESCRIPTION:	deallocate any dynamic memory associated with stream
 */
_ATTR_MP3DEC_TEXT_
void mad_stream_finish(struct mad_stream *stream)
{
  if (stream->main_data) {
    stream->main_data = 0;
  }

  mad_bit_finish(&stream->anc_ptr);
  mad_bit_finish(&stream->ptr);
}

/*
 * NAME:	stream->buffer()
 * DESCRIPTION:	set stream buffer pointers
 */
_ATTR_MP3DEC_TEXT_
void mad_stream_buffer(struct mad_stream *stream,
		       unsigned char const *buffer, unsigned long length)
{
  stream->buffer = buffer;
  stream->bufend = buffer + length;

  stream->this_frame = buffer;
  stream->next_frame = buffer;

  stream->sync = 1;

  mad_bit_init(&stream->ptr, buffer);
}

/*
 * NAME:	stream->skip()
 * DESCRIPTION:	arrange to skip bytes before the next frame
 */
_ATTR_MP3DEC_TEXT_
void mad_stream_skip(struct mad_stream *stream, unsigned long length)
{
  stream->skiplen += length;
}

/*
 * NAME:	stream->sync()
 * DESCRIPTION:	locate the next stream sync word
 */
_ATTR_MP3DEC_TEXT_
int mad_stream_sync(struct mad_stream *stream)
{
  register unsigned char const *ptr, *end;

  ptr = mad_bit_nextbyte(&stream->ptr);
  end = stream->bufend;

  while (ptr < end - 1 &&
	 !(ptr[0] == 0xff && (ptr[1] & 0xe0) == 0xe0))
    ++ptr;

  if (end - ptr < MAD_BUFFER_GUARD)
   return -1;

  mad_bit_init(&stream->ptr, ptr);

  return 0;
}

/*
 * NAME:	stream->errorstr()
 * DESCRIPTION:	return a string description of the current error condition
 */
_ATTR_MP3DEC_TEXT_
char const *mad_stream_errorstr(struct mad_stream const *stream)
{
  switch (stream->error) {
  case MAD_ERROR_NONE:		 return "no error";

  case MAD_ERROR_BUFLEN:	 return "input buffer too small (or EOF)";
  case MAD_ERROR_BUFPTR:	 return "invalid (null) buffer pointer";

  case MAD_ERROR_NOMEM:		 return "not enough memory";

  case MAD_ERROR_LOSTSYNC:	 return "lost synchronization";
  case MAD_ERROR_BADLAYER:	 return "reserved header layer value";
  case MAD_ERROR_BADBITRATE:	 return "forbidden bitrate value";
  case MAD_ERROR_BADSAMPLERATE:	 return "reserved sample frequency value";
  case MAD_ERROR_BADEMPHASIS:	 return "reserved emphasis value";

  case MAD_ERROR_BADCRC:	 return "CRC check failed";
  case MAD_ERROR_BADBITALLOC:	 return "forbidden bit allocation value";
  case MAD_ERROR_BADSCALEFACTOR: return "bad scalefactor index";
  case MAD_ERROR_BADMODE:	 return "bad bitrate/mode combination";
  case MAD_ERROR_BADFRAMELEN:	 return "bad frame length";
  case MAD_ERROR_BADBIGVALUES:	 return "bad big_values count";
  case MAD_ERROR_BADBLOCKTYPE:	 return "reserved block_type";
  case MAD_ERROR_BADSCFSI:	 return "bad scalefactor selection info";
  case MAD_ERROR_BADDATAPTR:	 return "bad main_data_begin pointer";
  case MAD_ERROR_BADPART3LEN:	 return "bad audio data length";
  case MAD_ERROR_BADHUFFTABLE:	 return "bad Huffman table select";
  case MAD_ERROR_BADHUFFDATA:	 return "Huffman data overrun";
  case MAD_ERROR_BADSTEREO:	 return "incompatible block_type for JS";
  }

  return 0;
}

#endif
#endif
