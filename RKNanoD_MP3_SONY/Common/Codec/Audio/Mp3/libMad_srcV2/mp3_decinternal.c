/* Copyright (C) 2009 ROCK-CHIPS FUZHOU . All Rights Reserved. */
/*
File    : \Audio\pMP3.c
Desc    : MP3解码流程控制

Author  : Vincent Hsiung (xw@rock-chips.com)
Date    : Jan 10 , 2009
Notes   :

$Log    :
*
*
*/
/****************************************************************/

#include "../include/audio_main.h"
#ifdef A_CORE_DECODE
#ifdef MP3_DEC_INCLUDE

#include "../include/audio_globals.h"
#include "../include/audio_file_access.h"
//#include "./libMad/mp3_mad.h"
#include "mp3_mad.h"
//#include "mp3_fixed.h"
#include "mp3_global.h"
#include "Mp3_synth.h"
#include <stdio.h>
#include <string.h> //for memcpy(),memmove()

//-----------------------------------------------------------------
typedef struct
{
    long samplerate ;
    long bitrate ;
    long channels ;
    long length ;
    long outlength ;
    unsigned char ucIsVBR;
    unsigned char ucChannels;
    unsigned short usSampleRate;
    unsigned long ulFirstFrame;
    unsigned long ulLength;
    unsigned long ulBitRate;
    unsigned long ulTimePos;
    unsigned long ulTimeLength;
    unsigned long ulOutputLength;
} tMP3;


/* input buffer */
#define MP3_INPUT_BUFFER_LENGTH 512*4
struct buffer
{
    unsigned char const *start;
    unsigned long length;
};

_ATTR_MP3DEC_BSS_
static FILE  *mp3_file;                                             //TODO: remove this

_ATTR_MP3DEC_DATA_
static int flag_fail = 0;

_ATTR_MP3DEC_DATA_
static int FILE_total_byte = 0;

_ATTR_MP3DEC_DATA_
static int MP3_total_byte = 0;

_ATTR_MP3DEC_BSS_
static struct buffer buffer;

//_ATTR_MP3DEC_BSS_
//static unsigned char  mp3_stream_buf[MP3_INPUT_BUFFER_LENGTH];        //actually input buffer
extern char pENCODED_DATA[MP3_INPUT_BUFFER_LENGTH];
#define mp3_stream_buf pENCODED_DATA

extern unsigned long SRC_Num_Forehead;

/* output buffer manager */
_ATTR_MP3DEC_BSS_
//extern __align(32) short outbuf[2][2*1152];        //must align, for dma transfer
extern short *outbuf[2];

_ATTR_MP3DEC_BSS_
static short *outptrL;

_ATTR_MP3DEC_BSS_
static short *outptrR;

_ATTR_MP3DEC_BSS_
static short *should_out;
static short *should_out1;

/* decoder internal infomation */
_ATTR_MP3DEC_BSS_
static struct mad_decoder decoder;

_ATTR_MP3DEC_BSS_
static void *error_data;

_ATTR_MP3DEC_BSS_
static int bad_last_frame ;

_ATTR_MP3DEC_BSS_
static struct mad_stream *stream;

_ATTR_MP3DEC_BSS_
static struct mad_frame *frame;

_ATTR_MP3DEC_BSS_
static struct mad_synth *synth;

_ATTR_MP3DEC_BSS_
static struct t_sync Sync;

_ATTR_MP3DEC_BSS_
static enum mad_flow (*error_func)(void *, struct mad_stream *, struct mad_frame *);

/* music property */
_ATTR_MP3DEC_BSS_
static unsigned long samplerate;

_ATTR_MP3DEC_BSS_
static long bitrate;

_ATTR_MP3DEC_BSS_
static long channels;

_ATTR_MP3DEC_BSS_
static long length;

_ATTR_MP3DEC_BSS_
static long outlength;

_ATTR_MP3DEC_BSS_
static unsigned long timepos;

_ATTR_MP3DEC_BSS_
static unsigned int lasttimepos;

_ATTR_MP3DEC_BSS_
static unsigned int lasttimeposflg;

_ATTR_MP3DEC_BSS_
static unsigned long framesizeflg;

#ifdef A_CORE_DECODE
extern unsigned long SRC_Num_Forehead;
#else
//#define SRC_Num_Forehead 0
#endif

_ATTR_MP3DEC_BSS_
static unsigned long lastframedecodeflg;

_ATTR_MP3DEC_BSS_
//static tMP3 mp3_object;
tMP3 mp3_object;

extern volatile int mp3_frame_cnt;

_ATTR_MP3DEC_DATA_
static int mp3_decode_err_cnt = 0;

_ATTR_MP3DEC_DATA_
static long org_pos  = 0;

/*
 * This is the input callback. The purpose of this callback is to (re)fill
 * the stream buffer which is to be decoded. In this example, an entire file
 * has been mapped into memory, so we just call mad_stream_buffer() with the
 * address and length of the mapping. When this callback is called a second
 * time, we are finished decoding.
 */
_ATTR_MP3DEC_TEXT_
enum mad_flow input(void *data, struct mad_stream *stream)
{
    struct buffer *buffer = data;
    unsigned int lb , rb = 0, i;

    if (!buffer->length)
        return MAD_FLOW_STOP;

    if (stream->this_frame && stream->next_frame)
    {
        rb = (unsigned int)(buffer->length) -
             (unsigned int)(stream->next_frame - stream->buffer);

        if (stream->buffer == stream->next_frame)
        {
            rb = 0;
            lb = RKFIO_FRead((void *)(stream->buffer), buffer->length, mp3_file);
        }
        else
        {
            memmove((void *)stream->buffer, (void *)stream->next_frame, rb);
            lb = RKFIO_FRead((void *)(stream->buffer + rb), buffer->length - rb, mp3_file);
        }
    }
    else
    {
        lb = RKFIO_FRead((void *)buffer->start , buffer->length - rb, mp3_file);
    }

    {
        buffer->length = lb + rb;

        if (buffer->length)
        {
            mad_stream_buffer(stream, buffer->start, buffer->length);
            return MAD_FLOW_CONTINUE;
        }
        else
        {
            //mp3_printf("data read over");
            return MAD_FLOW_STOP;
        }
    }
}

/*
 * The following utility routine performs simple rounding, clipping, and
 * scaling of MAD's high-resolution samples down to 16 bits. It does not
 * perform any dithering or noise shaping, which would be recommended to
 * obtain any exceptional audio quality. It is therefore not recommended to
 * use this routine if high-quality output is desired.
 */
extern void outbufferL_swap();
extern void outbufferR_swap();
#define MAD_OVER MAD_F(0X7FFF)
static/*inline*/ signed int scale(mad_fixed_t sample)
{
    int a;
  /* round */
  sample += (1L << (MAD_F_FRACBITS - 16));
  //mp3_printf("round:%d\n",sample);
  /* clip */
  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;
  //mp3_printf("clip:%d\n",sample);
  /* quantize */
  a = sample >> (MAD_F_FRACBITS + 1 - 16);
 // mp3_printf("quantize:%d\n",a);
  return a;//sample >> (MAD_F_FRACBITS + 1 - 16);
}


/*
 * This is the output callback function. It is called after each frame of
 * MPEG audio data has been completely decoded. The purpose of this callback
 * is to output (or play) the decoded PCM audio.
 */
_ATTR_MP3DEC_TEXT_
enum mad_flow output(void *data,
                     struct mad_header const *header,
                     struct mad_pcm *pcm)
{
    #if 0
     int i,ch = 0;
	short *start_pos;
for(ch = 0;ch<2;ch++){
	//if (synth_hw_ch == 0)
		start_pos = &frame->output_ptrL[frame->output_pos[ch]*2] + ch;
	//else
		start_pos = &frame->output_ptrR[frame->output_pos[ch]*2] + ch;

	{
		//if (synth_end_ch_number==1)	//stereo
		{
			for(i = 0; i < 1152; i++)
			{
				start_pos[i*2] = (short)(scale(pcm->samples[ch][i]));
                //if(start_pos[i*2] >= 32767 ||start_pos[i*2] <=-32767 )
                //    mp3_printf("%d ",start_pos[i*2]);
            }
           // frame->output_pos[ch]+=576;
		}
	}

	//frame->output_pos[ch]+=32;

    /*
	  depends frame length, when output_pos is larger than this we could output the
	  decoded data of current channel.

	  output length list :

      layer 1 : 384
      layer 2 : 1152
      layer 3 : 1152
      mpeg 2.5  : 576
	*/

	//if (synth_out_samples_limit == 1152)
	{
		//if (frame->output_pos[synth_hw_ch]>= 1152)
		{
			frame->output_pos[ch] = 0;

			if (ch == 1)
			{
			    //右声道
				outbufferR_swap();
                should_out1 = should_out;
                should_out = outptrR;
			}
			else
			{
			    //左声道
				outbufferL_swap();
                should_out1 = should_out;
               should_out = outptrL;
			}

	        //判断此帧是否能结束
			//if (synth_hw_ch == synth_end_ch_number)
				//final_stuff(synth_hw_ch);
		}
	}

}

 #else if //软解
    unsigned int nchannels, nsamples;
	unsigned int i;
	mad_fixed_t const *left_ch, *right_ch;
	int value;
    short *start_pos;
    int ch = 0;
	/* pcm->samplerate contains the sampling frequency */

	nchannels = pcm->channels;
	nsamples  = pcm->length;
	left_ch   = pcm->samples[0];
	right_ch  = pcm->samples[1];
  //  mp3_printf("nchannels = %d ",nchannels);
   // mp3_printf("nsamples = %d ",nsamples);

	#if 1
	for(ch = 0;ch < channels;ch++)
	{
        if(ch == 0)
           start_pos = &frame->output_ptrL[frame->output_pos[ch]*2] + ch;
        else
           start_pos = &frame->output_ptrR[frame->output_pos[ch]*2] + ch;

        for(i =0 ;i < nsamples;i++)
        {
          start_pos[i*2] = (short)(scale(pcm->samples[ch][i]));
         // mp3_printf("%d %d",(int)(pcm->samples[ch][i]&0xffffffffL),start_pos[i*2]);

          if(channels == 1)//mono
            start_pos[i*2+1] = (short)(scale(pcm->samples[ch][i]));
        }
    {
	   {
			frame->output_pos[ch] = 0;

			if (ch == 1)
			{
			    //右声道
				outbufferR_swap();
                should_out1 = should_out;
                should_out = outptrR;
			}
			else
			{
			    //左声道
				outbufferL_swap();
                should_out1 = should_out;
                should_out = outptrL;
			}
		}
	}

 }
#endif
   #endif
 return MAD_FLOW_CONTINUE;
}

_ATTR_MP3DEC_TEXT_
enum mad_flow error_default(void *data, struct mad_stream *stream,
                            struct mad_frame *frame)
{
    int *bad_last_frame = data;

    switch (stream->error)
    {
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
 * This is the error callback function. It is called whenever a decoding
 * error occurs. The error is indicated by stream->error; the list of
 * possible MAD_ERROR_* errors can be found in the mad.h (or stream.h)
 * header file.
 */
_ATTR_MP3DEC_TEXT_
enum mad_flow error(void *data,
                    struct mad_stream *stream,
                    struct mad_frame *frame)
{
    // struct buffer *buffer = data;
    // mp3_printf("decoding error 0x%04x (%s) at byte offset %u\n",
    //      stream->error, mad_stream_errorstr(stream),
    //      stream->this_frame - buffer->start);
    /* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */
    return MAD_FLOW_CONTINUE;
}

_ATTR_MP3DEC_TEXT_
void outbufferL_swap()
{
    if (outptrL == &outbuf[0][SRC_Num_Forehead])
    {
        outptrL = &outbuf[1][SRC_Num_Forehead];
    }
    else
    {
        outptrL = &outbuf[0][SRC_Num_Forehead];
    }

    frame->output_ptrL = outptrL;
}

_ATTR_MP3DEC_TEXT_
void outbufferR_swap()
{
    //mp3_printf("\n outbufferR_swap outbufferR_swap\n");
    if (outptrR == &outbuf[0][SRC_Num_Forehead])
    {
        outptrR = &outbuf[1][SRC_Num_Forehead];
    }
    else
    {
        outptrR = &outbuf[0][SRC_Num_Forehead];
    }

    frame->output_ptrR = outptrR;
}

extern volatile int is_synthing;

_ATTR_MP3DEC_TEXT_
void final_stuff(int endch)
{
    //mp3_printf("************");
    if (endch == 1)
    {
        should_out1 = should_out;
        should_out = outptrR;
    }
    else
    {
        should_out1 = should_out;
        should_out = outptrL;
    }

    mad_synth_frame(synth, frame);
    is_synthing = 0;
}


extern int firstframe;
_ATTR_MP3DEC_TEXT_
void mute_err()
{
    memset(&outbuf[0][SRC_Num_Forehead], 0, 2 * 1152 * sizeof(short));
    memset(&outbuf[1][SRC_Num_Forehead], 0, 2 * 1152 * sizeof(short));
    mad_frame_mute();
}

_ATTR_MP3DEC_TEXT_
int mp3_open(int isExact)
{
    int result, timeOld, timeNew;
    long int pos, time;
    tMP3 *pMP3 = &mp3_object;
    unsigned int slots_per_frame;
    unsigned int pad_slot, N;
    int vbr_flag = 0;
    mp3_frame_cnt = 0;
    flag_fail = 0;
    mp3_decode_err_cnt = 0;
    lastframedecodeflg = 1;
    pMP3->usSampleRate = 0;
    pMP3->ulBitRate = 0;
    pMP3->ucChannels = 0;
    pMP3->ulLength = 0;
    pMP3->ulTimePos = 0;
    memset(&outbuf[0][SRC_Num_Forehead], 0, 2 * 1152 * sizeof(short));
    memset(&outbuf[1][SRC_Num_Forehead], 0, 2 * 1152 * sizeof(short));
    should_out1 = &outbuf[0][SRC_Num_Forehead];
    should_out = &outbuf[0][SRC_Num_Forehead];
    //modify by helun
    RKFIO_FSeek(  0 , 0  , pRawFileCache);

    if (isExact == 1)
    {
        if (GetHeaderPosition(pMP3) == 0)
        {
           // mp3_DEBUG();
            return 0;  // I DON'T KNOW WHY???? BY VINCENT 2010 0708
        }
    }

    mp3_printf("mp3 start pos =%d", firstframe);
    RKFIO_FSeek(firstframe , 0, pRawFileCache);
    //mp3_printf("pos = %d ,file offset = %d,in %s,line = %d \n",pos,RKFIO_FTell(pRawFileCache),__FUNCTION__,__LINE__);
    mp3_file = pRawFileCache;
    //RKFIO_FSeek(  0 , 0  , pRawFileCache);//zs 0908
    /* initialize our private message structure */
    buffer.start  = mp3_stream_buf;
    buffer.length = sizeof(mp3_stream_buf);
    /* configure input, output, and error functions */
    mad_decoder_init(&decoder,
                     &buffer,
                     input,
                     0 /* header */,
                     0 /* filter */,
                     output,
                     error,
                     0 /* message */);

    if (decoder.input_func == 0)
        return 0;

    if (decoder.error_func)
    {
        error_func = decoder.error_func;
        error_data = decoder.cb_data;
    }
    else
    {
        error_func = error_default;
        error_data = &bad_last_frame;
    }

    decoder.sync = (struct t_sync*)&Sync;
    stream = &decoder.sync->stream;
    frame  = &decoder.sync->frame;
    synth  = &decoder.sync->synth;
    mad_stream_init(stream);
    mad_frame_init(frame);
    mad_synth_init(synth);
    mad_stream_options(stream, decoder.options);
    decoder.input_func(decoder.cb_data, stream);
    outptrL = &outbuf[0][SRC_Num_Forehead];
    outptrR = &outbuf[0][SRC_Num_Forehead];
    frame->output_ptrL = outptrL;
    frame->output_ptrR = outptrR;
    frame->output_pos[0] = 0;
    frame->output_pos[1] = 0;
    //-------------------------------------------------------
#if 1
    {
        int searchcount = 1024;

        do
        {
            //Do input if necessary
            if (stream->error == MAD_ERROR_BUFLEN)
            {
                switch (decoder.input_func(decoder.cb_data, stream))
                {
                    case MAD_FLOW_STOP:
                        goto open_done;

                    case MAD_FLOW_BREAK:
                        goto open_fail;

                    case MAD_FLOW_IGNORE:
                        continue;

                    case MAD_FLOW_CONTINUE:
                        break;    //do nothing
                }
            }

            while (1)
            {
                searchcount--;

                if (!searchcount)
                    goto open_fail;

                //Decode
                if (mad_frame_decode(frame, stream) == -1)
                {
                    //mp3_printf("stream->error = 0x%x \n",stream->error);
                    if (stream->error == MAD_ERROR_BUFLEN)
                    {
                        break;  // go out this while, for do input stuff.
                    }

                    if (!MAD_RECOVERABLE(stream->error))
                        goto open_fail;

                    switch (error_func(error_data, stream, frame))
                    {
                        case MAD_FLOW_STOP:
                            goto open_done;

                        case MAD_FLOW_BREAK:
                            goto open_fail;

                        case MAD_FLOW_IGNORE:
                            continue;

                        case MAD_FLOW_CONTINUE:
                            continue;

                        default:
                            break;    //do nothing
                    }
                }
                else
                {
                    bad_last_frame = 0;
                }

                goto open_done; //正常解出一帧即退出
            }
        }
        while (stream->error == MAD_ERROR_BUFLEN);
    }
#endif
open_done:
    result = 1;
    {
        struct mad_header *header = &frame->header;
        extern int synth_out_samples_limit;
        samplerate = header->samplerate;
        bitrate = header->bitrate;
        channels = ((header)->mode ? 2 : 1);

        if (bitrate > 0)
        {
            length = (long long)RKFIO_FLength(pRawFileCache) * 1000 * 8 / bitrate;
        }
        else
            length = 60000;

        FILE_total_byte = RKFIO_FLength(pRawFileCache);

        //for some song the preparse can't get the correct samplerate, so use decoder's infomation
        //by Vincent , July 21, 2010
        //if (samplerate) //(!pMP3->usSampleRate)
        if (!pMP3->usSampleRate)
            pMP3->usSampleRate = samplerate;

        //for some song the preparse can't get the correct bitrate, so use decoder's infomation
        //by Vincent , July 20, 2010
        //if (bitrate) //(!pMP3->ulBitRate)
        if (!pMP3->ulBitRate)
        {
            pMP3->ulBitRate = bitrate;
        }
        else
        {
            vbr_flag = 1;
            mp3_printf("vbr");
        }

        if (!pMP3->ucChannels)
            pMP3->ucChannels = channels;

        if (pMP3->ulBitRate > 0)
        {
            if (pMP3->ulLength)
                pMP3->ulTimeLength = (long long)pMP3->ulLength * 1000 * 8 / pMP3->ulBitRate;
            else
                pMP3->ulTimeLength = (long long)RKFIO_FLength(pRawFileCache) * 1000 * 8 / pMP3->ulBitRate;
        }
        else
            pMP3->ulTimeLength = length = 60000;

        MP3_total_byte = pMP3->ulLength;
        mp3_printf("length:%d, fileLen:%d ,bitraate:%d", pMP3->ulLength, RKFIO_FLength(pRawFileCache), pMP3->ulBitRate);

        if (synth_out_samples_limit == 0)
            synth_out_samples_limit = 1152;

        outlength = 1152;//synth_out_samples_limit;
        pMP3->ulOutputLength = outlength;
        //pMP3->ulTimePos = 0;
        pMP3->ulTimePos = outlength;
    }
    samplerate = pMP3->usSampleRate;
    bitrate = pMP3->ulBitRate;
    channels = pMP3->ucChannels;
    length = pMP3->ulTimeLength;
    timepos = pMP3->ulTimePos;
    return result;
open_fail:
    mad_frame_mute();
    result = 0;
    return result;
    //---------------------------------------------------------
    return 1;
}

_ATTR_MP3DEC_TEXT_
int mp3_get_buffer(short** ulParam1 , int *ulParam2)
{
    // return the buffer address
    outlength = synth_out_samples_limit ;
    *(short **)ulParam1 = should_out1;
    *(int *)ulParam2 = outlength;
    //mp3_printf("shout_out = %x ,len = %d \n",(unsigned long )should_out,outlength);
    return 1;
}

_ATTR_MP3DEC_TEXT_

int mp3_decode()
{
    int result = 0;
    int dec_error_cnt;
    unsigned long ALLtimepos;
    /* start decoding */
    frame  = &decoder.sync->frame;
    dec_error_cnt = 0;

    do
    {
        if (flag_fail == 1)
            goto decode_fail;

        //Do input if necessary
        if (stream->error == MAD_ERROR_BUFLEN)
        {
            switch (decoder.input_func(decoder.cb_data, stream))
            {
                case MAD_FLOW_STOP:
                    timepos = ((long long)length * samplerate) / 1000;
                    goto decode_fail;//decode_done;

                case MAD_FLOW_BREAK:
                    goto decode_fail;

                case MAD_FLOW_IGNORE:
                    continue;

                case MAD_FLOW_CONTINUE:
                    break;    //do nothing
            }
        }

        while (1)
        {
            //Decode
            if (mad_frame_decode(frame, stream) == -1)//容错机制
            {
                if (stream->error == MAD_ERROR_BUFLEN)
                {
                    break;  // go out this while, for do input stuff.
                }

                if (!MAD_RECOVERABLE(stream->error))
                {
                    goto decode_fail;
                }

                timepos += outlength;
                dec_error_cnt++;
               // mp3_printf("%d\n",dec_error_cnt);

                if (dec_error_cnt > 128)
                {
                    mp3_printf("dec_error_cnt > 128");
                    goto decode_fail;
                }

                //mp3_printf("err_code: = %d", error_func(error_data, stream, frame));
                switch (error_func(error_data, stream, frame))
                {
                    case MAD_FLOW_STOP:
                        goto decode_fail;//decode_done;

                    case MAD_FLOW_BREAK:
                        goto decode_fail;

                    case MAD_FLOW_IGNORE:
                        continue;

                    case MAD_FLOW_CONTINUE:
                        continue;

                    default:
                        break;    //do nothing
                }
            }
            else
            {
                bad_last_frame = 0;
                dec_error_cnt = 0;
                mp3_decode_err_cnt = 0;
            }

            /*if (decoder.filter_func) {
	           switch (decoder.filter_func(decoder.cb_data, stream, frame)) {
	           case MAD_FLOW_STOP:
	                goto decode_done;
	           case MAD_FLOW_BREAK:
	                goto decode_fail;
	           case MAD_FLOW_IGNORE:
	                continue;
	           case MAD_FLOW_CONTINUE:
	                break;
	}
      }*/


            mad_synth_frame(synth, frame);

           // while()
           // frame->output_ptrL = (short *)&synth->pcm.samples[0];
            //frame->output_ptrR =(short *) &synth->pcm.samples[1];

          /* {
                int i = 0;
                for(i=0;i<1152;i++)
                {
                 //  mp3_printf("samples[%d]_H=%d ",i,(int)(synth->pcm.samples[1][i]&0xffffffff00000000L));
                   mp3_printf("samples[%d]_L=%d ",i,(int)(synth->pcm.samples[1][i]&0xffffffffL));
                }
            }*/
           if (decoder.output_func) {
	           switch (decoder.output_func(decoder.cb_data,
				     &frame->header, &synth->pcm)) {
	              case MAD_FLOW_STOP:
	                goto decode_done;
	              case MAD_FLOW_BREAK:
	                goto decode_fail;
	              case MAD_FLOW_IGNORE:
	              case MAD_FLOW_CONTINUE:
	                 break;
	               }
                 }


            goto decode_done;
        }
    }
    while (stream->error == MAD_ERROR_BUFLEN);

decode_done:
    {

        long long real_timepos;

		//mad_synth_finish(synth);
        timepos += outlength;
        real_timepos = ((long long)length * samplerate) / 1000;

        if (timepos > real_timepos)
        {
            timepos = real_timepos;
            flag_fail++;
           // mp3_printf("timepos0: = %d %d", (timepos*1000)/samplerate, (real_timepos*1000)/samplerate);
        }
        else
        {
            if ((real_timepos - timepos) < 1 * samplerate)
            {
               // mp3_printf("timepos1: = %d %d", (timepos*1000)/samplerate, (real_timepos*1000)/samplerate);
            }
        }
        mad_synth_finish(synth);
        mad_frame_finish(frame);
        mad_stream_finish(stream);
    }
    result = 1;
    mp3_frame_cnt++;
    return result;
decode_fail:
   // mad_frame_mute();
    {
        long long real_timepos;
        timepos += outlength;
        real_timepos = ((long long)length * samplerate) / 1000;

        if (timepos > real_timepos)
        {
            timepos = real_timepos;
            //mp3_printf("timepos2: = %d %d", (timepos*1000)/samplerate, (real_timepos*1000)/samplerate);
        }
        else
        {
            if ((real_timepos - timepos) < 1 * samplerate)
            {
                //mp3_printf("timepos3: = %d %d", (timepos*1000)/samplerate, (real_timepos*1000)/samplerate);
            }
        }
    }
    result = 0;
    return result;
}

_ATTR_MP3DEC_TEXT_
int mp3_get_bps()
{
    return 16;
}
_ATTR_MP3DEC_TEXT_
unsigned int mp3_get_samplerate()
{
    return samplerate;
}

_ATTR_MP3DEC_TEXT_
int mp3_get_channels()
{
    return channels;
}

_ATTR_MP3DEC_TEXT_
int mp3_get_bitrate()
{
    return bitrate;
}

_ATTR_MP3DEC_TEXT_
int mp3_get_length()
{
    return length;
}

_ATTR_MP3DEC_TEXT_
unsigned long mp3_get_timepos()
{
    return timepos;
}

_ATTR_MP3DEC_TEXT_
int mp3_close()
{
    struct mad_frame *frame;
    struct mad_stream *stream;
    stream = &decoder.sync->stream;
    frame  = &decoder.sync->frame;
    /* release the decoder */
    mad_synth_finish(synth);
    mad_frame_finish(frame);
    mad_stream_finish(stream);
    mad_decoder_finish(&decoder);
    return 1;
}

extern int synth_hw_ch;
_ATTR_MP3DEC_BSS_
static  unsigned long int_able_tmp;
#define SETENA0_REG *((volatile unsigned long*)(0xE000E100))
#define CLRENA0_REG *((volatile unsigned long*)(0xE000E180))
#define CLRPEND0_REG    *((volatile unsigned long*)(0xE000E280))


extern unsigned int ape_tag_size;
extern unsigned int MP3_FORMAT_FLAG ;
static unsigned int  samplerate_table[3] = { 44100, 48000, 32000 };
_ATTR_MP3DEC_TEXT_
unsigned char CheckValidHeader_Seek(unsigned    char *header)
{
    int mpeg ;
    int layer;
    int fs;

    // Check Frame Sync [ AAAAAAAA AAA ]
    if ( !((header[0] == 0xFF) && ((header[1] & 0xE0) == 0xE0)))
        return 0;

    mpeg  = (header[1] & 0x18) >> 3;

    if (mpeg == 0)
    {
        mpeg = 3;
    }
    else
    {
        mpeg = 4 - mpeg;
    }

    layer = 4 - ((header[1] & 0x06) >> 1);

    if (layer != (MP3_FORMAT_FLAG & 0xF) || (mpeg != ((MP3_FORMAT_FLAG & 0xF0) >> 4)))
    {
        return 0;
    }

    // Check Bitrate Index [ EEEE ]
    if ( ((header[2] & 0xF0) == 0x00 ) ||       // Free 0000
         ((header[2] & 0xF0) == 0xF0 ) )        // Bad  1111
        return 0;

    if ( (header[2] & 0x0C) == 0x0C  ) // Reserved 11
        return 0;

    fs = samplerate_table[(header[2] & 0x0C) >> 2];
    fs = fs >> (mpeg - 1);

    // Check Sampling Rate Frequency [ FF ]
    if ( (fs != samplerate))// Reserved 11
        return 0;

    return 1;
}

_ATTR_MP3DEC_TEXT_
int mp3_seek(unsigned long time)
{
    long pos;
    long pos_t;
    long time_t;
    int_able_tmp = SETENA0_REG;
    CLRENA0_REG  = 0x60;//0xffffffff; //Synth and IMDCT clear
    mp3_frame_cnt = 0;
    mp3_decode_err_cnt = 0;
    flag_fail = 0;

    //Synth and IMDCT Rest
    //*((volatile unsigned long *) 0x40010014) = 0x00000030;
    //*((volatile unsigned long *) 0x40010014) = 0x00000000;

    if (time > length)
        time = length;

//    mp3_printf(" seekTime = %d\n",time);
    pos = ((unsigned long long) time * bitrate) / 8000;

    if (MP3_total_byte != FILE_total_byte)
    {
        pos_t = FILE_total_byte - MP3_total_byte - ape_tag_size ;
        //mp3_printf("\ 字节差 =%d \n",pos_t);
        pos += pos_t;
    }

    RKFIO_FSeek( pos , 0 ,  pRawFileCache );
    {
        char buf[2304];
        int ret;
        char *ptr;
        RKFIO_FRead( buf , 2304 ,  pRawFileCache );
        ptr = buf;

        while (1)
        {
            ret = CheckValidHeader_Seek(ptr);

            if (ret )
            {
                break;
            }

            ptr += 1;
            pos += 1;
        }

        //mp3_printf(" POS =%d \n",pos);
        RKFIO_FSeek( pos , 0 ,  pRawFileCache );
    }
    timepos = (unsigned long long)time * samplerate / 1000;
    memset(&outbuf[0][SRC_Num_Forehead], 0, 2 * 1152 * sizeof(short));
    memset(&outbuf[1][SRC_Num_Forehead], 0, 2 * 1152 * sizeof(short));
    should_out1 = &outbuf[0][SRC_Num_Forehead];
    should_out = &outbuf[0][SRC_Num_Forehead];
    decoder.sync = (struct t_sync*)&Sync;
    stream = &decoder.sync->stream;
    frame  = &decoder.sync->frame;
    //synth  = &decoder.sync->synth;
    //mad_stream_init(stream);
    outptrL = &outbuf[0][SRC_Num_Forehead];
    outptrR = &outbuf[0][SRC_Num_Forehead];
    frame->output_ptrL = outptrL;
    frame->output_ptrR = outptrR;
    stream->this_frame = 0;
    stream->next_frame = 0;
    decoder.input_func(decoder.cb_data, stream);
    stream->error = MAD_ERROR_NONE;
    frame->output_pos[0] = 0;
    frame->output_pos[1] = 0;
    frame->phase[0] = 0;
    frame->phase[1] = 0;
    synth_hw_ch = 0;
    mad_frame_mute();
    CLRPEND0_REG = 0x60;
    /*
    IntPendingClear(INT_ID17_DMA_TRANS0);
    IntPendingClear(INT_ID19_DMA_TRANS1);
    IntPendingClear(INT_ID21_IMDCT36);
    IntPendingClear(INT_ID22_SYNTHESIZE);
    */
    is_synthing = 0;
    AcceleratorHWInit();
    SETENA0_REG = int_able_tmp ;
}
#endif
#endif
