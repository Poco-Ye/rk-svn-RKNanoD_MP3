/*
********************************************************************************
*                   Copyright (c) 2008, Rockchip
*                         All rights reserved.
*
* File Name：   decoder.c
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             Vincent Hsiung     2009-1-14         1.0
*    desc:    ORG.
********************************************************************************
*/

#include "../include/audio_main.h"
#ifdef MP2_INCLUDE

# include "mp2_global.h"
# include "mp2_stream.h"
# include "mp2_frame.h"
# include "mp2_synth.h"
# include "mp2_decoder.h"
# include "mp2_accelerator_hal.h"

# include <stdlib.h>

_ATTR_MP2DEC_BSS_
unsigned short * mp2_crc_table ;
_ATTR_MP2DEC_BSS_
unsigned long  (*mp2_bitrate_table)[15];
_ATTR_MP2DEC_BSS_
unsigned int  *mp2_samplerate_table;

_ATTR_MP2DEC_BSS_
struct  sbquant_table_define{
  unsigned int sblimit;
  unsigned char const offsets[30];
}  *mp2_sbquant_table ;

_ATTR_MP2DEC_BSS_
struct  bitalloc_table_define{
  unsigned short nbal;
  unsigned short offset;
}  *mp2_bitalloc_table ;

_ATTR_MP2DEC_BSS_
unsigned char  (*mp2_offset_table)[15] ;

_ATTR_MP2DEC_BSS_
struct quantclass {
  unsigned short nlevels;
  unsigned char group;
  unsigned char bits;
  mad_fixed_t C;
  mad_fixed_t D;
}  *mp2_qc_table ;

_ATTR_MP2DEC_BSS_
struct quantclass  *mp2_qc_table_2 ;

_ATTR_MP2DEC_BSS_
mad_fixed_t  *mp2_sf_table ;


// Decalre a sync structure because no malloc and free are not implemented here
_ATTR_MP2DEC_BSS_
struct sync_t mp2_Sync;

/*
 * NAME:	decoder->init()
 * DESCRIPTION:	initialize a decoder object with callback routines
 */
_ATTR_MP2DEC_TEXT_
void mp2_mad_decoder_init(struct mad_decoder *decoder, void *data,
		      enum mad_flow (*input_func)(void *,
						  struct mad_stream *),
		      enum mad_flow (*header_func)(void *,
						   struct mad_header const *),
		      enum mad_flow (*filter_func)(void *,
						   struct mad_stream const *,
						   struct mad_frame *),
		      enum mad_flow (*output_func)(void *,
						   struct mad_header const *,
						   struct mad_pcm *),
		      enum mad_flow (*error_func)(void *,
						  struct mad_stream *,
						  struct mad_frame *),
		      enum mad_flow (*message_func)(void *,
						    void *, unsigned int *))
{
  decoder->mode         = -1;

  decoder->options      = 0;

  decoder->async.pid    = 0;
  decoder->async.in     = -1;
  decoder->async.out    = -1;

  decoder->sync         = 0;

  decoder->cb_data      = data;

  decoder->input_func   = input_func;
  decoder->header_func  = header_func;
  decoder->filter_func  = filter_func;
  decoder->output_func  = output_func;
  decoder->error_func   = error_func;
  decoder->message_func = message_func;
}

_ATTR_MP2DEC_TEXT_
int mp2_mad_decoder_finish(struct mad_decoder *decoder)
{
  return 0;
}

_ATTR_MP2DEC_TEXT_
static void mad_frame_mute()
{
  //TODO: synth模块中FIFO清零动作
 // memset((void *)0x62045000,0,1024*sizeof(long));SYNTH_BASEADDR
    memset((void *)(SYNTH_BASEADDR+0x1000),0,1024*sizeof(long))	;
}

_ATTR_MP2DEC_TEXT_
static
enum mad_flow mp2_error_default_0(void *data, struct mad_stream *stream,
			    struct mad_frame *frame)
{
  int *bad_last_frame = data;

  switch (stream->error) {
  case MAD_ERROR_BADCRC:
    if (*bad_last_frame)
      mad_frame_mute();
    else
      *bad_last_frame = 1;

    return MAD_FLOW_IGNORE;

  default:
    return MAD_FLOW_CONTINUE;
  }
}

/*
 * NAME:	decoder->message()
 * DESCRIPTION:	send a message to and receive a reply from the decoder process
 */
_ATTR_MP2DEC_TEXT_
int mp2_mad_decoder_message(struct mad_decoder *decoder,
			void *message, unsigned int *len)
{
# if defined(USE_ASYNC)
  if (decoder->mode != MAD_DECODER_MODE_ASYNC ||
      send(decoder->async.out, message, *len) != MAD_FLOW_CONTINUE ||
      receive(decoder->async.in, &message, len) != MAD_FLOW_CONTINUE)
    return -1;

  return 0;
# else
  return -1;
# endif
}

#endif

