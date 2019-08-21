/*
********************************************************************************
*                   Copyright (c) 2008, Rockchip
*                         All rights reserved.
*
* File Name£º   decoder.c
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             Vincent Hsiung     2009-1-14         1.0
*    desc:    ORG.
********************************************************************************
*/

#include "../include/audio_main.h"

#ifdef A_CORE_DECODE
#ifdef MP3_DEC_INCLUDE

# include "mp3_global.h"
# include "mp3_stream.h"
# include "mp3_frame.h"
# include "mp3_synth.h"
# include "mp3_decoder.h"

# include <stdlib.h>

// Decalre a sync structure because no malloc and free are not implemented here
_ATTR_MP3DEC_BSS_
struct sync_t Sync;

	_ATTR_MP3DEC_DATA_
		static  char mp3version[] = "Version:0.0.1 \nDate:2012.3.23 \nLib:mp3_dec_lib" ;
	_ATTR_MP3DEC_TEXT_
		 char * Mp3DecVersion()
		{
			
				return mp3version;
		}

/*
 * NAME:	decoder->init()
 * DESCRIPTION:	initialize a decoder object with callback routines
 */
_ATTR_MP3DEC_TEXT_
void mad_decoder_init(struct mad_decoder *decoder, void *data,
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

_ATTR_MP3DEC_TEXT_
int mad_decoder_finish(struct mad_decoder *decoder)
{
# if defined(USE_ASYNC)
  if (decoder->mode == MAD_DECODER_MODE_ASYNC && decoder->async.pid) {
    pid_t pid;
    int status;

    close(decoder->async.in);

    do
      pid = waitpid(decoder->async.pid, &status, 0);
    while (pid == -1 && errno == EINTR);

    decoder->mode = -1;

    close(decoder->async.out);

    decoder->async.pid = 0;
    decoder->async.in  = -1;
    decoder->async.out = -1;

    if (pid == -1)
      return -1;

    return (!WIFEXITED(status) || WEXITSTATUS(status)) ? -1 : 0;
  }
# endif

  return 0;
}


_ATTR_MP3DEC_TEXT_
static
enum mad_flow error_default_0(void *data, struct mad_stream *stream,
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

_ATTR_MP3DEC_TEXT_
static
int run_sync(struct mad_decoder *decoder)
{
  enum mad_flow (*error_func)(void *, struct mad_stream *, struct mad_frame *);
  void *error_data;
  int bad_last_frame = 0;
  struct mad_stream *stream;
  struct mad_frame *frame;
  struct mad_synth *synth;
  int result = 0;

  if (decoder->input_func == 0)
    return 0;

  if (decoder->error_func) {
    error_func = decoder->error_func;
    error_data = decoder->cb_data;
  }
  else {
    error_func = error_default_0;
    error_data = &bad_last_frame;
  }

  stream = &decoder->sync->stream;
  frame  = &decoder->sync->frame;
  synth  = &decoder->sync->synth;

  mad_stream_init(stream);
  mad_frame_init(frame);
  mad_synth_init(synth);

  mad_stream_options(stream, decoder->options);

  do {
    switch (decoder->input_func(decoder->cb_data, stream)) {
    case MAD_FLOW_STOP:
      goto done;
    case MAD_FLOW_BREAK:
      goto fail;
    case MAD_FLOW_IGNORE:
      continue;
    case MAD_FLOW_CONTINUE:
      break;
    }

    while (1) {
# if defined(USE_ASYNC)
      if (decoder->mode == MAD_DECODER_MODE_ASYNC) {
	switch (check_message(decoder)) {
	case MAD_FLOW_IGNORE:
	case MAD_FLOW_CONTINUE:
	  break;
	case MAD_FLOW_BREAK:
	  goto fail;
	case MAD_FLOW_STOP:
	  goto done;
	}
      }
# endif

      if (decoder->header_func) {
	if (mad_header_decode(&frame->header, stream) == -1) {
	  if (!MAD_RECOVERABLE(stream->error))
	    break;

	  switch (error_func(error_data, stream, frame)) {
	  case MAD_FLOW_STOP:
	    goto done;
	  case MAD_FLOW_BREAK:
	    goto fail;
	  case MAD_FLOW_IGNORE:
	  case MAD_FLOW_CONTINUE:
	  default:
	    continue;
	  }
	}

	switch (decoder->header_func(decoder->cb_data, &frame->header)) {
	case MAD_FLOW_STOP:
	  goto done;
	case MAD_FLOW_BREAK:
	  goto fail;
	case MAD_FLOW_IGNORE:
	  continue;
	case MAD_FLOW_CONTINUE:
	  break;
	}
      }

      if (mad_frame_decode(frame, stream) == -1) {
	if (!MAD_RECOVERABLE(stream->error))
	  break;

	switch (error_func(error_data, stream, frame)) {
	case MAD_FLOW_STOP:
	  goto done;
	case MAD_FLOW_BREAK:
	  goto fail;
	case MAD_FLOW_IGNORE:
	  break;
	case MAD_FLOW_CONTINUE:
	default:
	  continue;
	}
      }
      else
	bad_last_frame = 0;

      if (decoder->filter_func) {
	switch (decoder->filter_func(decoder->cb_data, stream, frame)) {
	case MAD_FLOW_STOP:
	  goto done;
	case MAD_FLOW_BREAK:
	  goto fail;
	case MAD_FLOW_IGNORE:
	  continue;
	case MAD_FLOW_CONTINUE:
	  break;
	}
      }

//By Vincent
#if 1
      mad_synth_frame(synth, frame);
#endif 

      if (decoder->output_func) {
	switch (decoder->output_func(decoder->cb_data,
				     &frame->header, &synth->pcm)) {
	case MAD_FLOW_STOP:
	  goto done;
	case MAD_FLOW_BREAK:
	  goto fail;
	case MAD_FLOW_IGNORE:
	case MAD_FLOW_CONTINUE:
	  break;
	}
      }
    }
  }
  while (stream->error == MAD_ERROR_BUFLEN);

 fail:
  result = -1;

 done:
  mad_synth_finish(synth);
  mad_frame_finish(frame);
  mad_stream_finish(stream);

  return result;
}

/*
 * NAME:	decoder->run()
 * DESCRIPTION:	run the decoder thread either synchronously or asynchronously
 */
_ATTR_MP3DEC_TEXT_
int mad_decoder_run(struct mad_decoder *decoder, enum mad_decoder_mode mode)
{
  int result;
  int (*run)(struct mad_decoder *) = 0;

  switch (decoder->mode = mode) {
  case MAD_DECODER_MODE_SYNC:
    run = run_sync;
    break;

  case MAD_DECODER_MODE_ASYNC:
# if defined(USE_ASYNC)
    run = run_async;
# endif
    break;
  }

  if (run == 0)
    return -1;

  decoder->sync = &Sync;

  if (decoder->sync == 0)
    return -1;

  result = run(decoder);

  decoder->sync = 0;

  return result;
}

/*
 * NAME:	decoder->message()
 * DESCRIPTION:	send a message to and receive a reply from the decoder process
 */
_ATTR_MP3DEC_TEXT_
int mad_decoder_message(struct mad_decoder *decoder,
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
#endif

