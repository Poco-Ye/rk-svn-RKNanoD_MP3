/*
********************************************************************************
*                   Copyright (c) 2008, Rockchip
*                         All rights reserved.
*
* File Name：   frame.c
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             Vincent Hsiung     2009-5-12         1.0
*    desc:    ORG.
********************************************************************************
*/

/*
    1. 将 bitrate_table[5][15]及samplerate_table[3] 移动到SoC的TableROM中;
    2. 修改mad_frame_mute()函数及接口.(在初始化及出错时应使用此函数清除子带合成模块中的历史数据)
    
    by Vincent Hsiung.
    
*/

#include "../include/audio_main.h"
#ifdef MP2_INCLUDE

# include "mp2_global.h"

# include <stdlib.h>

# include "mp2_bit.h"
# include "mp2_stream.h"
# include "mp2_frame.h"
# include "mp2_layer12.h"

extern unsigned long  (*mp2_bitrate_table)[15];
extern unsigned int  * mp2_samplerate_table ;

_ATTR_MP2DEC_DATA_
long mp2_synth_out_samples_limit = 1152; //by Vincent, Apr 6
_ATTR_MP2DEC_DATA_
long mp2_synth_out_samples_limit_lst = 1152; //by Vincent, May 8
_ATTR_MP2DEC_DATA_
long mp2_synth_out_samples_limit_change_flag = 0; //by Vincent, May 8

_ATTR_MP2DEC_DATA_
long mp2_frame_layer = 2;

_ATTR_MP2DEC_BSS_
mad_fixed_t mp2_sbsample_0[2][36][32];	/* synthesis subband filter samples */ // enlarge to 36 for 2*18 subbands, layer 2

/*
 * NAME:	header->init()
 * DESCRIPTION:	initialize header struct
 */
_ATTR_MP2DEC_TEXT_
void mp2_mad_header_init(struct mad_header *header)
{
  header->layer          = 0;
  header->mode           = 0;
  header->mode_extension = 0;
  header->emphasis       = 0;

  header->bitrate        = 0;
  header->samplerate     = 0;

  header->crc_check      = 0;
  header->crc_target     = 0;

  header->flags          = 0;
  header->private_bits   = 0;
}

_ATTR_MP2DEC_TEXT_
void mp2_frame_sbsample_toggle(struct mad_frame *frame)
{

  if (frame->sbsample == (mad_fixed_t(*)[36][32])&mp2_sbsample_0[0][0])
  {
        frame->sbsample = (mad_fixed_t(*)[36][32])&mp2_sbsample_0[1][0];;
  }
  else
  {
        frame->sbsample = (mad_fixed_t(*)[36][32])&mp2_sbsample_0[0][0];;
  }

}

/*
 * NAME:	frame->init()
 * DESCRIPTION:	initialize frame struct
 */
_ATTR_MP2DEC_TEXT_
void mp2_mad_frame_init(struct mad_frame *frame)
{
  mp2_mad_header_init(&frame->header);

  /* by Vincent */
  //synth_out_samples_limit = 0;

  frame->options = 0;

  frame->sbsample = (mad_fixed_t(*)[36][32])&mp2_sbsample_0[0][0];
  
  frame->overlap = 0;

  //By Vincent @ Oct 20 , 2008
  frame->phase[0] = 0;
  frame->phase[1] = 0;

	//todo for test
  //mp2_mad_frame_mute();
}

/*
 * NAME:	frame->finish()
 * DESCRIPTION:	deallocate any dynamic memory associated with frame
 */
_ATTR_MP2DEC_TEXT_
void mp2_mad_frame_finish(struct mad_frame *frame)
{
  mp2_mad_header_finish(&frame->header);

  if (frame->overlap) {
//  free(frame->overlap);
    frame->overlap = 0;
  }
}

/*
 * NAME:	decode_header()
 * DESCRIPTION:	read header data and following CRC word
 */
_ATTR_MP2DEC_TEXT_
static
int decode_header(struct mad_header *header, struct mad_stream *stream)
{
  unsigned int index;

  header->flags        = 0;
  header->private_bits = 0;

  /* header() */

  /* syncword */
  mp2_mad_bit_skip(&stream->ptr, 11);

  /* MPEG 2.5 indicator (really part of syncword) */
  if (mp2_mad_bit_read(&stream->ptr, 1) == 0)
    header->flags |= MAD_FLAG_MPEG_2_5_EXT;

  /* ID */
  if (mp2_mad_bit_read(&stream->ptr, 1) == 0)
    header->flags |= MAD_FLAG_LSF_EXT;
  else if (header->flags & MAD_FLAG_MPEG_2_5_EXT) {
    stream->error = MAD_ERROR_LOSTSYNC;
    return -1;
  }

  /* layer */
  header->layer = 4 - mp2_mad_bit_read(&stream->ptr, 2);

  if (header->layer == 4) {
    stream->error = MAD_ERROR_BADLAYER;
    return -1;
  }

  /* protection_bit */
  if (mp2_mad_bit_read(&stream->ptr, 1) == 0) {
    header->flags    |= MAD_FLAG_PROTECTION;
    header->crc_check = mp2_mad_bit_crc(stream->ptr, 16, 0xffff);
  }

  /* bitrate_index */
  index = mp2_mad_bit_read(&stream->ptr, 4);

  if (index == 15) {
    stream->error = MAD_ERROR_BADBITRATE;
    return -1;
  }

  if (header->flags & MAD_FLAG_LSF_EXT)
    header->bitrate = mp2_bitrate_table[3 + (header->layer >> 1)][index];
  else
    header->bitrate = mp2_bitrate_table[header->layer - 1][index];

  /* sampling_frequency */
  index = mp2_mad_bit_read(&stream->ptr, 2);

  if (index == 3) {
    stream->error = MAD_ERROR_BADSAMPLERATE;
    return -1;
  }

  header->samplerate = mp2_samplerate_table[index];

  if (header->flags & MAD_FLAG_LSF_EXT) {
    header->samplerate /= 2;

    if (header->flags & MAD_FLAG_MPEG_2_5_EXT)
      header->samplerate /= 2;
  }

  /* padding_bit */
  if (mp2_mad_bit_read(&stream->ptr, 1))
    header->flags |= MAD_FLAG_PADDING;

  /* private_bit */
  if (mp2_mad_bit_read(&stream->ptr, 1))
    header->private_bits |= MAD_PRIVATE_HEADER;

  /* mode */
  header->mode = 3 - mp2_mad_bit_read(&stream->ptr, 2);

  /* mode_extension */
  header->mode_extension = mp2_mad_bit_read(&stream->ptr, 2);

  /* copyright */
  if (mp2_mad_bit_read(&stream->ptr, 1))
    header->flags |= MAD_FLAG_COPYRIGHT;

  /* original/copy */
  if (mp2_mad_bit_read(&stream->ptr, 1))
    header->flags |= MAD_FLAG_ORIGINAL;

  /* emphasis */
  header->emphasis = mp2_mad_bit_read(&stream->ptr, 2);

# if defined(OPT_STRICT)
  /*
   * ISO/IEC 11172-3 says this is a reserved emphasis value, but
   * streams exist which use it anyway. Since the value is not important
   * to the decoder proper, we allow it unless OPT_STRICT is defined.
   */
  if (header->emphasis == MAD_EMPHASIS_RESERVED) {
    stream->error = MAD_ERROR_BADEMPHASIS;
    return -1;
  }
# endif

  /* error_check() */

  /* crc_check */
  if (header->flags & MAD_FLAG_PROTECTION)
    header->crc_target = (unsigned short)mp2_mad_bit_read(&stream->ptr, 16); //only 16-bit, so unsigned short , by Vincent

  return 0;
}

/*
 * NAME:	free_bitrate()
 * DESCRIPTION:	attempt to discover the bitstream's free bitrate
 */
_ATTR_MP2DEC_TEXT_
static
int free_bitrate(struct mad_stream *stream, struct mad_header const *header)
{
  struct mad_bitptr keep_ptr;
  unsigned long rate = 0;
  unsigned int pad_slot, slots_per_frame;
  unsigned char const *ptr = 0;

  keep_ptr = stream->ptr;

  pad_slot = (header->flags & MAD_FLAG_PADDING) ? 1 : 0;
  slots_per_frame = (header->layer == MAD_LAYER_III &&
		     (header->flags & MAD_FLAG_LSF_EXT)) ? 72 : 144;

  while (mp2_mad_stream_sync(stream) == 0) {
    struct mad_stream peek_stream;
    struct mad_header peek_header;

    peek_stream = *stream;
    peek_header = *header;

    if (decode_header(&peek_header, &peek_stream) == 0 &&
	peek_header.layer == header->layer &&
	peek_header.samplerate == header->samplerate) {
      unsigned int N;

      ptr = mp2_mad_bit_nextbyte(&stream->ptr);

      N = ptr - stream->this_frame;

      if (header->layer == MAD_LAYER_I) {
	rate = (unsigned long) header->samplerate *
	  (N - 4 * pad_slot + 4) / 48 / 1000;
      }
      else {
	rate = (unsigned long) header->samplerate *
	  (N - pad_slot + 1) / slots_per_frame / 1000;
      }

      if (rate >= 8)
	break;
    }

    mp2_mad_bit_skip(&stream->ptr, 8);
  }

  stream->ptr = keep_ptr;

  if (rate < 8 || (header->layer == MAD_LAYER_III && rate > 640)) {
    stream->error = MAD_ERROR_LOSTSYNC;
    return -1;
  }

  stream->freerate = rate * 1000;

  return 0;
}

/*
 * NAME:	header->decode()
 * DESCRIPTION:	read the next frame header from the stream
 */
_ATTR_MP2DEC_TEXT_
int mp2_mad_header_decode(struct mad_header *header, struct mad_stream *stream)
{
  register unsigned char const *ptr, *end;
  unsigned int pad_slot, N;

  ptr = stream->next_frame;
  end = stream->bufend;

  if (ptr == 0) {
    stream->error = MAD_ERROR_BUFPTR;
    goto fail;
  }

  /* stream skip */
  if (stream->skiplen) {
    if (!stream->sync)
      ptr = stream->this_frame;

    if (end - ptr < (long) stream->skiplen) {
      stream->skiplen   -= end - ptr;
      stream->next_frame = end;

      stream->error = MAD_ERROR_BUFLEN;
      goto fail;
    }

    ptr += stream->skiplen;
    stream->skiplen = 0;

    stream->sync = 1;
  }

 sync:
  /* synchronize */
  if (stream->sync) {
    if (end - ptr < MAD_BUFFER_GUARD) {
      stream->next_frame = ptr;

      stream->error = MAD_ERROR_BUFLEN;
      goto fail;
    }
    else if (!(ptr[0] == 0xff && (ptr[1] & 0xe0) == 0xe0)) {
      /* mark point where frame sync word was expected */
      stream->this_frame = ptr;
      stream->next_frame = ptr + 1;

      stream->error = MAD_ERROR_LOSTSYNC;
      goto fail;
    }
  }
  else {
    mp2_mad_bit_init(&stream->ptr, ptr);

    if (mp2_mad_stream_sync(stream) == -1) {
      if (end - stream->next_frame >= MAD_BUFFER_GUARD)
	stream->next_frame = end - MAD_BUFFER_GUARD;

      stream->error = MAD_ERROR_BUFLEN;
      goto fail;
    }

    ptr = mp2_mad_bit_nextbyte(&stream->ptr);
  }

  /* begin processing */
  stream->this_frame = ptr;
  stream->next_frame = ptr + 1;  /* possibly bogus sync word */

  mp2_mad_bit_init(&stream->ptr, stream->this_frame);

  if (decode_header(header, stream) == -1)
    goto fail;


  /* calculate free bit rate */
  if (header->bitrate == 0) {
    if ((stream->freerate == 0 || !stream->sync ||
	 (header->layer == MAD_LAYER_III && stream->freerate > 640000)) &&
	free_bitrate(stream, header) == -1)
      goto fail;

    header->bitrate = stream->freerate;
    header->flags  |= MAD_FLAG_FREEFORMAT;
  }

  /* calculate beginning of next frame */
  pad_slot = (header->flags & MAD_FLAG_PADDING) ? 1 : 0;

  if (header->layer == MAD_LAYER_I)
    N = ((12 * header->bitrate / header->samplerate) + pad_slot) * 4;
  else {
    unsigned int slots_per_frame;

    slots_per_frame = (header->layer == MAD_LAYER_III &&
		       (header->flags & MAD_FLAG_LSF_EXT)) ? 72 : 144;

    N = (slots_per_frame * header->bitrate / header->samplerate) + pad_slot;
  }

  /* verify there is enough data left in buffer to decode this frame */
  if ((long)(N + MAD_BUFFER_GUARD) > end - stream->this_frame) {
    stream->next_frame = stream->this_frame;

    stream->error = MAD_ERROR_BUFLEN;
    goto fail;
  }

  stream->next_frame = stream->this_frame + N;

  if (!stream->sync) {
    /* check that a valid frame header follows this frame */

    ptr = stream->next_frame;
    if (!(ptr[0] == 0xff && (ptr[1] & 0xe0) == 0xe0)) {
      ptr = stream->next_frame = stream->this_frame + 1;
      goto sync;
    }

    stream->sync = 1;
  }

  header->flags |= MAD_FLAG_INCOMPLETE;

  return 0;

 fail:
  stream->sync = 0;

  return -1;
}


/*
 * NAME:	frame->decode()
 * DESCRIPTION:	decode a single frame from a bitstream
 */
_ATTR_MP2DEC_TEXT_
int mp2_mad_frame_decode(struct mad_frame *frame, struct mad_stream *stream)
{
  frame->options = stream->options;

  /* header() */
  /* error_check() */

  if (!(frame->header.flags & MAD_FLAG_INCOMPLETE) &&
      mp2_mad_header_decode(&frame->header, stream) == -1)
    goto fail;

  /* audio_data() */

  frame->header.flags &= ~MAD_FLAG_INCOMPLETE;

  //if (decoder_table[frame->header.layer - 1](stream, frame) == -1) {
  if (mp2_mad_layer_II(stream, frame) == -1) {
    if (!MAD_RECOVERABLE(stream->error))
      stream->next_frame = stream->this_frame;

    goto fail;
  }

  /* ancillary_data() */

  if (frame->header.layer != MAD_LAYER_III) {
    struct mad_bitptr next_frame;

    mp2_mad_bit_init(&next_frame, stream->next_frame);

    stream->anc_ptr    = stream->ptr;
    stream->anc_bitlen = mp2_mad_bit_length(&stream->ptr, &next_frame);

    mp2_mad_bit_finish(&next_frame);
  }

  /* how many samples output , by Vincent Hsiung*/
  /*
	  layer 1 : 384
	  layer 2 : 1152
	  layer 3 : 1152
	  mpeg 2.5  : 576
  */
#if 0
  synth_out_samples_limit_lst = synth_out_samples_limit ;
  //if (synth_out_samples_limit == 0)
  {
	  if (frame->header.layer == MAD_LAYER_I)
	  	synth_out_samples_limit = 384; 	/* layer I */
	  else
	  	if (frame->header.layer == MAD_LAYER_III)
		{
			if (frame->header.flags & MAD_FLAG_LSF_EXT)	// mpeg 2.5
				synth_out_samples_limit = 576;	/* layer III , MPEG 2.5 */
			else
				synth_out_samples_limit = 1152;	/* layer III */
		}
		else
		{
			synth_out_samples_limit = 1152;	/* layer III */
		}
  }
#endif
  
    if (mp2_synth_out_samples_limit==0)
        mp2_synth_out_samples_limit = 1152;

#if 0
	if (synth_out_samples_limit_lst != synth_out_samples_limit)
	{
		extern struct mad_frame * synth_hw_frame;
		extern volatile int is_synthing;
		//synth_hw_frame->output_pos[0] = 0;
		//synth_hw_frame->output_pos[1] = 0;
		//is_synthing = 0;
		synth_out_samples_limit_change_flag = 1;
	}
#endif

  return 0;

 fail:
  stream->anc_bitlen = 0;
  return -1;
}

/*
 * NAME:	frame->mute()
 * DESCRIPTION:	zero all subband values so the frame becomes silent
 */
_ATTR_MP2DEC_TEXT_
void mp2_mad_frame_mute()
{
  //extern unsigned long mp2_frame_overlap_buff[2 * 32 * 18];

  //将overlap数据清零
  //memset(mp2_frame_overlap_buff, 0 , 2 * 32 * 18 * sizeof(long));

  //TODO: synth模块中FIFO清零动作
  //modify by helun
  memset(0x60021000UL,0,1024*sizeof(long));
}

#endif

