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


#include "audio_main.h"


#ifdef MP2_INCLUDE


#include "audio_globals.h"
#include "audio_file_access.h"

#include "mp2_mad.h"


typedef unsigned int size_t;

extern size_t   (*AVI_RKFIO_FRead)(void * /*buffer*/, size_t /*length*/,FILE *) ;
extern int      (*AVI_RKFIO_FSeek)(long int /*offset*/, int /*whence*/ ,FILE * /*stream*/);
extern long int (*AVI_RKFIO_FTell)(FILE * /*stream*/);
extern size_t   (*AVI_RKFIO_FWrite)(void * /*buffer*/, size_t /*length*/,FILE * /*stream*/);
extern unsigned long (*AVI_RKFIO_FLength)(FILE *in /*stream*/);
extern int      (*AVI_RKFIO_FClose)(FILE * /*stream*/);

#define RKFIO_FRead	AVI_RKFIO_FRead
#define RKFIO_FSeek	AVI_RKFIO_FSeek
#define RKFIO_FLength AVI_RKFIO_FLength
#define pRawFileCache AVI_pRawFileCache


#include <stdio.h>
#include <string.h>	//for memcpy(),memmove()

extern FILE *AVI_pRawFileCache;

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
}tMP2;

//-----------------------------------------------------------------
/* input buffer */
#define MP2_INPUT_BUFFER_LENGTH	512*4
struct buffer {
  unsigned char const *start;
  unsigned long length;
};

_ATTR_MP2DEC_BSS_
static FILE  *mp2_file;												//TODO: remove this

_ATTR_MP2DEC_BSS_
static struct buffer buffer;

_ATTR_MP2DEC_BSS_
unsigned char  mp2_stream_buf[MP2_INPUT_BUFFER_LENGTH];		//actually input buffer

/* output buffer manager */
_ATTR_MP2DEC_BSS_
static __align(32) short outbuf[2][2*1152];        //must align, for dma transfer

_ATTR_MP2DEC_BSS_
static short *outptrL;

_ATTR_MP2DEC_BSS_
static short *outptrR;

_ATTR_MP2DEC_BSS_
static short *should_out;

/* decoder internal infomation */
_ATTR_MP2DEC_BSS_
static struct mad_decoder decoder;

_ATTR_MP2DEC_BSS_
static void *error_data;

_ATTR_MP2DEC_BSS_
static int bad_last_frame ;

_ATTR_MP2DEC_BSS_
static struct mad_stream *stream;

_ATTR_MP2DEC_BSS_
static struct mad_frame *frame;

_ATTR_MP2DEC_BSS_
static struct mad_synth *synth;

_ATTR_MP2DEC_BSS_
static struct t_sync Sync;

_ATTR_MP2DEC_BSS_
static enum mad_flow (*error_func)(void *, struct mad_stream *, struct mad_frame *);

/* music property */
_ATTR_MP2DEC_BSS_
static long samplerate;

_ATTR_MP2DEC_BSS_
static long bitrate;

_ATTR_MP2DEC_BSS_
static long channels;

_ATTR_MP2DEC_BSS_
static long length;

_ATTR_MP2DEC_BSS_
static long outlength;

_ATTR_MP2DEC_BSS_
static long timepos;

_ATTR_MP2DEC_BSS_
static tMP2 mp2_object;

extern volatile int mp2_frame_cnt;

//----------------------------------------------------
extern
unsigned short * mp2_crc_table ;

extern
unsigned long  (*mp2_bitrate_table)[15];

extern
unsigned int  *mp2_samplerate_table;

extern
struct sbquant_table_define {
  unsigned int sblimit;
  unsigned char const offsets[30];
}*mp2_sbquant_table ;

extern
struct bitalloc_table_define{
  unsigned short nbal;
  unsigned short offset;
}  *mp2_bitalloc_table ;

extern
unsigned char const (*mp2_offset_table)[15] ;

extern
struct quantclass {
  unsigned short nlevels;
  unsigned char group;
  unsigned char bits;
  mad_fixed_t C;
  mad_fixed_t D;
}  *mp2_qc_table ;

extern
struct quantclass  *mp2_qc_table_2 ;

extern
mad_fixed_t  *mp2_sf_table ;

/*
 * This is the input callback. The purpose of this callback is to (re)fill
 * the stream buffer which is to be decoded. In this example, an entire file
 * has been mapped into memory, so we just call mad_stream_buffer() with the
 * address and length of the mapping. When this callback is called a second
 * time, we are finished decoding.
 */
_ATTR_MP2DEC_TEXT_
enum mad_flow mp2_input(void *data, struct mad_stream *stream)
{
  struct buffer *buffer = data;
  unsigned int lb , rb = 0;

  if (!buffer->length)
    return MAD_FLOW_STOP;

  if (stream->this_frame && stream->next_frame)
  {
    rb = (unsigned int)(buffer->length) - 
         (unsigned int)(stream->next_frame - stream->buffer);
	
	if (stream->buffer == stream->next_frame)
	{
		rb = 0;
		lb = RKFIO_FRead((void *)(stream->buffer),buffer->length,mp2_file);
	}
	else
	{
    memmove((void *)stream->buffer, (void *)stream->next_frame, rb);
    lb = RKFIO_FRead((void *)(stream->buffer + rb),buffer->length - rb,mp2_file);
	}
  }
  else  
  {
    lb = RKFIO_FRead((void *)buffer->start , buffer->length - rb,mp2_file);
  }

  if (lb == 0)
  {	
    buffer->length = 0;
    return MAD_FLOW_STOP;
  }
  else 
    buffer->length = lb + rb;

  mp2_mad_stream_buffer(stream, buffer->start, buffer->length);
  
  return MAD_FLOW_CONTINUE;
}


/*
 * This is the output callback function. It is called after each frame of
 * MPEG audio data has been completely decoded. The purpose of this callback
 * is to output (or play) the decoded PCM audio.
 */
_ATTR_MP2DEC_TEXT_
enum mad_flow mp2_output(void *data,
		     struct mad_header const *header,
		     struct mad_pcm *pcm)
{
  return MAD_FLOW_CONTINUE;
}

_ATTR_MP2DEC_TEXT_
enum mad_flow mp2_error_default(void *data, struct mad_stream *stream,
			    struct mad_frame *frame)
{
  int *bad_last_frame = data;

  switch (stream->error) {
  case MAD_ERROR_BADCRC:
    if (*bad_last_frame)
      mp2_mad_frame_mute();
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
_ATTR_MP2DEC_TEXT_
enum mad_flow mp2_error(void *data,
		    struct mad_stream *stream,
		    struct mad_frame *frame)
{
  // struct buffer *buffer = data;

  // DEBUG("decoding error 0x%04x (%s) at byte offset %u\n",
  //	  stream->error, mad_stream_errorstr(stream),
  // 	  stream->this_frame - buffer->start);

  /* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */

  return MAD_FLOW_CONTINUE;
}

_ATTR_MP2DEC_TEXT_
void mp2_outbufferL_swap()
{
	if (outptrL==&outbuf[0][0])
	{
		outptrL = &outbuf[1][0];
	}
	else
	{
		outptrL = &outbuf[0][0];
	}

	frame->output_ptrL = outptrL;
}

_ATTR_MP2DEC_TEXT_
void mp2_outbufferR_swap()
{	
	if (outptrR==&outbuf[0][0])
	{
		outptrR = &outbuf[1][0];
	}
	else
	{
		outptrR = &outbuf[0][0];
	}	

	frame->output_ptrR = outptrR;
}

extern volatile int mp2_is_synthing;

_ATTR_MP2DEC_TEXT_
void mp2_final_stuff(int endch)
{
	if (endch == 1)
		should_out = outptrR;
	else
		should_out = outptrL;

	mp2_mad_synth_frame(synth, frame);

	mp2_is_synthing = 0;
}
#define NANOC
#ifdef NANOC

#pragma arm section code = "Mp2Data", rodata = "Mp2Data", rwdata = "Mp2Data", zidata = "Mp2Data"
#define _X_(x) ((x)>>16)
#define static
#define mad_fixed_t	long
#define MAD_F(x)		((mad_fixed_t) (x##L))

static unsigned short  mp3_crc_table_hl[256] = {
  0x0000, 0x8005, 0x800f, 0x000a, 0x801b, 0x001e, 0x0014, 0x8011,
  0x8033, 0x0036, 0x003c, 0x8039, 0x0028, 0x802d, 0x8027, 0x0022,
  0x8063, 0x0066, 0x006c, 0x8069, 0x0078, 0x807d, 0x8077, 0x0072,
  0x0050, 0x8055, 0x805f, 0x005a, 0x804b, 0x004e, 0x0044, 0x8041,
  0x80c3, 0x00c6, 0x00cc, 0x80c9, 0x00d8, 0x80dd, 0x80d7, 0x00d2,
  0x00f0, 0x80f5, 0x80ff, 0x00fa, 0x80eb, 0x00ee, 0x00e4, 0x80e1,
  0x00a0, 0x80a5, 0x80af, 0x00aa, 0x80bb, 0x00be, 0x00b4, 0x80b1,
  0x8093, 0x0096, 0x009c, 0x8099, 0x0088, 0x808d, 0x8087, 0x0082,

  0x8183, 0x0186, 0x018c, 0x8189, 0x0198, 0x819d, 0x8197, 0x0192,
  0x01b0, 0x81b5, 0x81bf, 0x01ba, 0x81ab, 0x01ae, 0x01a4, 0x81a1,
  0x01e0, 0x81e5, 0x81ef, 0x01ea, 0x81fb, 0x01fe, 0x01f4, 0x81f1,
  0x81d3, 0x01d6, 0x01dc, 0x81d9, 0x01c8, 0x81cd, 0x81c7, 0x01c2,
  0x0140, 0x8145, 0x814f, 0x014a, 0x815b, 0x015e, 0x0154, 0x8151,
  0x8173, 0x0176, 0x017c, 0x8179, 0x0168, 0x816d, 0x8167, 0x0162,
  0x8123, 0x0126, 0x012c, 0x8129, 0x0138, 0x813d, 0x8137, 0x0132,
  0x0110, 0x8115, 0x811f, 0x011a, 0x810b, 0x010e, 0x0104, 0x8101,

  0x8303, 0x0306, 0x030c, 0x8309, 0x0318, 0x831d, 0x8317, 0x0312,
  0x0330, 0x8335, 0x833f, 0x033a, 0x832b, 0x032e, 0x0324, 0x8321,
  0x0360, 0x8365, 0x836f, 0x036a, 0x837b, 0x037e, 0x0374, 0x8371,
  0x8353, 0x0356, 0x035c, 0x8359, 0x0348, 0x834d, 0x8347, 0x0342,
  0x03c0, 0x83c5, 0x83cf, 0x03ca, 0x83db, 0x03de, 0x03d4, 0x83d1,
  0x83f3, 0x03f6, 0x03fc, 0x83f9, 0x03e8, 0x83ed, 0x83e7, 0x03e2,
  0x83a3, 0x03a6, 0x03ac, 0x83a9, 0x03b8, 0x83bd, 0x83b7, 0x03b2,
  0x0390, 0x8395, 0x839f, 0x039a, 0x838b, 0x038e, 0x0384, 0x8381,

  0x0280, 0x8285, 0x828f, 0x028a, 0x829b, 0x029e, 0x0294, 0x8291,
  0x82b3, 0x02b6, 0x02bc, 0x82b9, 0x02a8, 0x82ad, 0x82a7, 0x02a2,
  0x82e3, 0x02e6, 0x02ec, 0x82e9, 0x02f8, 0x82fd, 0x82f7, 0x02f2,
  0x02d0, 0x82d5, 0x82df, 0x02da, 0x82cb, 0x02ce, 0x02c4, 0x82c1,
  0x8243, 0x0246, 0x024c, 0x8249, 0x0258, 0x825d, 0x8257, 0x0252,
  0x0270, 0x8275, 0x827f, 0x027a, 0x826b, 0x026e, 0x0264, 0x8261,
  0x0220, 0x8225, 0x822f, 0x022a, 0x823b, 0x023e, 0x0234, 0x8231,
  0x8213, 0x0216, 0x021c, 0x8219, 0x0208, 0x820d, 0x8207, 0x0202
};

static
unsigned long  bitrate_table_hl[5][15] = {
  /* MPEG-1 */
  { 0,  32000,  64000,  96000, 128000, 160000, 192000, 224000,  /* Layer I   */
       256000, 288000, 320000, 352000, 384000, 416000, 448000 },
  { 0,  32000,  48000,  56000,  64000,  80000,  96000, 112000,  /* Layer II  */
       128000, 160000, 192000, 224000, 256000, 320000, 384000 },
  { 0,  32000,  40000,  48000,  56000,  64000,  80000,  96000,  /* Layer III */
       112000, 128000, 160000, 192000, 224000, 256000, 320000 },

  /* MPEG-2 LSF */
  { 0,  32000,  48000,  56000,  64000,  80000,  96000, 112000,  /* Layer I   */
       128000, 144000, 160000, 176000, 192000, 224000, 256000 },
  { 0,   8000,  16000,  24000,  32000,  40000,  48000,  56000,  /* Layers    */
        64000,  80000,  96000, 112000, 128000, 144000, 160000 } /* II & III  */
};


static
struct  sbquant_table_define  sbquant_table_hl[5] = {
  /* ISO/IEC 11172-3 Table B.2a */
  { 27, { 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 3, 3, 3, 3, 3,	/* 0 */
	  3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0 } },
  /* ISO/IEC 11172-3 Table B.2b */
  { 30, { 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 3, 3, 3, 3, 3,	/* 1 */
	  3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0 } },
  /* ISO/IEC 11172-3 Table B.2c */
  {  8, { 5, 5, 2, 2, 2, 2, 2, 2 } },				/* 2 */
  /* ISO/IEC 11172-3 Table B.2d */
  { 12, { 5, 5, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 } },		/* 3 */
  /* ISO/IEC 13818-3 Table B.1 */
  { 30, { 4, 4, 4, 4, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1,	/* 4 */
	  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 } }
};
static
struct  bitalloc_table_define   bitalloc_table_hl[8] = {
  { 2, 0 },  /* 0 */
  { 2, 3 },  /* 1 */
  { 3, 3 },  /* 2 */
  { 3, 1 },  /* 3 */
  { 4, 2 },  /* 4 */
  { 4, 3 },  /* 5 */
  { 4, 4 },  /* 6 */
  { 4, 5 }   /* 7 */
};

static
unsigned int  samplerate_table_hl[3] = { 44100, 48000, 32000 };
static
unsigned char  offset_table_hl[6][15] = {
  { 0, 1, 16                                             },  /* 0 */
  { 0, 1,  2, 3, 4, 5, 16                                },  /* 1 */
  { 0, 1,  2, 3, 4, 5,  6, 7,  8,  9, 10, 11, 12, 13, 14 },  /* 2 */
  { 0, 1,  3, 4, 5, 6,  7, 8,  9, 10, 11, 12, 13, 14, 15 },  /* 3 */
  { 0, 1,  2, 3, 4, 5,  6, 7,  8,  9, 10, 11, 12, 13, 16 },  /* 4 */
  { 0, 2,  4, 5, 6, 7,  8, 9, 10, 11, 12, 13, 14, 15, 16 }   /* 5 */
};

static
struct quantclass  qc_table_hl[17] = {
# include "mp3_qc_table.dat"
};

static
mad_fixed_t  sf_table_hl[64] = {
# include "mp3_sf_table.dat"
};




#pragma arm section code

_ATTR_MP2DEC_DATA_
	static	char mp2version[] = "Version:0.0.1 \nDate:2012.3.31 \nLib:mp2_dec_lib" ;
_ATTR_MP2DEC_TEXT_
	 char * Mp2DecVersion()
	{
		
			return mp2version;
	}


#endif




_ATTR_MP2DEC_TEXT_
int mp2_open(int isExact)
{
	int result;
	
	tMP2 *pMP2 = &mp2_object;

	mp2_crc_table = mp3_crc_table_hl;
	mp2_bitrate_table = bitrate_table_hl;
	mp2_sbquant_table = sbquant_table_hl;
	mp2_samplerate_table = samplerate_table_hl;
	mp2_bitalloc_table =bitalloc_table_hl;
	mp2_offset_table = offset_table_hl;
	mp2_qc_table = qc_table_hl;
	mp2_sf_table =sf_table_hl;	

	mp2_frame_cnt = 0;
	
	pMP2->usSampleRate = 0;
	pMP2->ulBitRate = 0;
	pMP2->ucChannels = 0;
	pMP2->ulLength = 0;
	pMP2->ulTimePos = 0;

	memset(&outbuf[0][0],0,2*1152*sizeof(short));
	memset(&outbuf[1][0],0,2*1152*sizeof(short));

	should_out = &outbuf[0][0];

//Todo : by Vincent Hsiung , May 14
#if 0	
	if (isExact == 1)        					
	{
	    if (mp2_GetHeaderPosition(pMP2)==0)
			return 0;
	}
#endif
	
	mp2_file = pRawFileCache;
	
	RKFIO_FSeek(  0 , 0  , pRawFileCache);

	/* initialize our private message structure */				
	buffer.start  = mp2_stream_buf;
	buffer.length = sizeof(mp2_stream_buf);
	
	/* configure input, output, and error functions */				
	mp2_mad_decoder_init(&decoder, 
					&buffer,
					mp2_input, 
					0 /* header */, 
					0 /* filter */,
					mp2_output,
					mp2_error, 
					0 /* message */);
									
	if (decoder.input_func == 0)
		return 0;
	
	if (decoder.error_func) {
		error_func = decoder.error_func;
		error_data = decoder.cb_data;
	}
	else {
		error_func = mp2_error_default;
		error_data = &bad_last_frame;
	}

	decoder.sync = (struct t_sync*)&Sync;
					
	stream = &decoder.sync->stream;
	frame  = &decoder.sync->frame;
	synth  = &decoder.sync->synth;
	
	mp2_mad_stream_init(stream);
	mp2_mad_frame_init(frame);
	mp2_mad_synth_init(synth);
	
	mp2_mad_stream_options(stream, decoder.options);

	decoder.input_func(decoder.cb_data, stream);

	outptrL = &outbuf[0][0];
	outptrR = &outbuf[0][0];

	frame->output_ptrL = outptrL;
	frame->output_ptrR = outptrR;
	
	frame->output_pos[0] = 0;
	frame->output_pos[1] = 0;

	//-------------------------------------------------------
	
	open_done:
	result = 1;								

	{
		struct mad_header *header = &frame->header;					
		extern int mp2_synth_out_samples_limit;

/*
		samplerate = header->samplerate;
		bitrate = header->bitrate;
		channels = ((header)->mode ? 2 : 1);
		if (bitrate > 0)
			length = (long long)RKFIO_FLength(pRawFileCache)*1000*8/bitrate;
		else
			length = 60000;
*/		
		if (!pMP2->usSampleRate)
			pMP2->usSampleRate = samplerate;
		if (!pMP2->ulBitRate)
			pMP2->ulBitRate = bitrate;
		if (!pMP2->ucChannels)
			pMP2->ucChannels = channels;
		if (pMP2->ulBitRate > 0)
		    {
		        if (pMP2->ulLength)
				    pMP2->ulTimeLength = (long long)pMP2->ulLength * 1000 * 8/pMP2->ulBitRate;
				else
				    pMP2->ulTimeLength = (long long)RKFIO_FLength(pRawFileCache) * 1000 * 8/pMP2->ulBitRate;
			}
		    else
				pMP2->ulTimeLength = length = 60000;
				
		if (mp2_synth_out_samples_limit == 0)
			mp2_synth_out_samples_limit = 1152;

		outlength = 1152;//synth_out_samples_limit;
		pMP2->ulOutputLength = outlength;
		pMP2->ulTimePos = 0;
	}
	
	samplerate = pMP2->usSampleRate;
	bitrate = pMP2->ulBitRate;
	channels = pMP2->ucChannels;
	length = pMP2->ulTimeLength;
	timepos = pMP2->ulTimePos;

    return result;				

	open_fail:
	mp2_mad_frame_mute();
	result = 0;
	return result;

	//---------------------------------------------------------

    return 1;
}

_ATTR_MP2DEC_TEXT_
int mp2_get_buffer(short** ulParam1 , int *ulParam2)
{
    // return the buffer address
	outlength = mp2_synth_out_samples_limit ;

    *(short **)ulParam1 = should_out;
    
	*(int *)ulParam2 = outlength;
    
    return 1;
}

_ATTR_MP2DEC_TEXT_
int mp2_decode()
{
	int result = 0;
	/* start decoding */
	frame  = &decoder.sync->frame;

	do{
		//Do input if necessary
		if (stream->error == MAD_ERROR_BUFLEN)
		{
			switch (decoder.input_func(decoder.cb_data, stream)) 
			{
				case MAD_FLOW_STOP:
				  goto decode_fail;//decode_done;
				case MAD_FLOW_BREAK:
				  goto decode_fail;
				case MAD_FLOW_IGNORE:
				  continue;
				case MAD_FLOW_CONTINUE:
				  break;	//do nothing
			}
		}
		
		while(1)
		{
			//Decode			
			if (mp2_mad_frame_decode(frame, stream) == -1) {
				if (stream->error == MAD_ERROR_BUFLEN)
				{
					break;	// go out this while, for do input stuff.
				}
				if (!MAD_RECOVERABLE(stream->error))
					goto decode_fail;
				
				switch (error_func(error_data, stream, frame)) {
					case MAD_FLOW_STOP:
					  goto decode_fail;//decode_done;
					case MAD_FLOW_BREAK:
					  goto decode_fail;
					case MAD_FLOW_IGNORE:
					  continue;
					case MAD_FLOW_CONTINUE:
					  continue;
					default:
					  break;	//do nothing
				}
			}
			else
			{
				bad_last_frame = 0;
			}

			goto decode_done;
		}
	}while (stream->error == MAD_ERROR_BUFLEN);
	
	decode_done:
	timepos += outlength;
	result = 1;								

	mp2_frame_cnt++;

    return result;				

	decode_fail:
	mp2_mad_frame_mute();
	result = 0;
	return result;
}

_ATTR_MP2DEC_TEXT_
int mp2_get_samplerate()
{
    return samplerate;
}

_ATTR_MP2DEC_TEXT_
int mp2_get_channels()
{
    return channels;
}

_ATTR_MP2DEC_TEXT_
int mp2_get_bitrate()
{
    return bitrate;
}

_ATTR_MP2DEC_TEXT_
int mp2_get_length()
{
	return length;
}

_ATTR_MP2DEC_TEXT_
int mp2_get_timepos()
{
	return timepos;
}

_ATTR_MP2DEC_TEXT_
int mp2_close()
{
    struct mad_frame *frame;
    struct mad_stream *stream;
    
    stream = &decoder.sync->stream;
    frame  = &decoder.sync->frame;
    
    /* release the decoder */
    mp2_mad_synth_finish(synth);
    mp2_mad_frame_finish(frame);
    mp2_mad_stream_finish(stream);
    mp2_mad_decoder_finish(&decoder);
    return 1;
}

extern int mp2_synth_hw_ch;
_ATTR_MP2DEC_BSS_
static  unsigned long int_able_tmp;

#define SETENA0_REG	*((volatile unsigned long*)(0xE000E100))
#define CLRENA0_REG	*((volatile unsigned long*)(0xE000E180))
#define CLRPEND0_REG	*((volatile unsigned long*)(0xE000E280))

_ATTR_MP2DEC_TEXT_
int mp2_seek(long time)
{
	long pos;

	int_able_tmp = SETENA0_REG;
	CLRENA0_REG	 = 0x60;//0xffffffff;   //Synth and IMDCT clear

	mp2_frame_cnt = 0;

    //Synth and IMDCT clear
	//*((volatile unsigned long *) 0x40010014) = 0x00000030;
	//*((volatile unsigned long *) 0x40010014) = 0x00000000;
	
	if (time > length)
		time = length;

	pos = ((long long) time * bitrate) / 8000;
                
    RKFIO_FSeek( pos , 0 ,  pRawFileCache );
                
    timepos = (long long)time * samplerate / 1000;


	memset(&outbuf[0][0],0,2*1152*sizeof(short));
	memset(&outbuf[1][0],0,2*1152*sizeof(short));
	should_out = &outbuf[0][0];
	
	decoder.sync = (struct t_sync*)&Sync;
					
	stream = &decoder.sync->stream;
	frame  = &decoder.sync->frame;
	//synth  = &decoder.sync->synth;

	//mad_stream_init(stream);

	outptrL = &outbuf[0][0];
	outptrR = &outbuf[0][0];

	frame->output_ptrL = outptrL;
	frame->output_ptrR = outptrR;

	stream->this_frame = 0;
	stream->next_frame = 0;

	decoder.input_func(decoder.cb_data, stream);
	
	frame->output_pos[0] = 0;
	frame->output_pos[1] = 0;

	frame->phase[0] = 0;
	frame->phase[1] = 0;

	mp2_synth_hw_ch = 0;

	mp2_mad_frame_mute();

    CLRPEND0_REG = 0x60;
    /*
    IntPendingClear(INT_ID17_DMA_TRANS0);
    IntPendingClear(INT_ID19_DMA_TRANS1);
    IntPendingClear(INT_ID21_IMDCT36);
    IntPendingClear(INT_ID22_SYNTHESIZE);
    */

	mp2_is_synthing = 0;

	mp2_AcceleratorHWInit();

	SETENA0_REG = int_able_tmp ;
}
#endif
