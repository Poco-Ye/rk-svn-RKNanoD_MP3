/*------------------------------------------------------------
   FLAC DECODER
  ------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "audio_main.h"

#ifdef A_CORE_DECODE
#ifdef FLAC_DEC_INCLUDE

#include "ordinals.h"
#include "flac_decoder_info.h"
#include "flacbuffer.h"

#include "audio_file_access.h"
//#include"Stream_decoder_i.h"


//#include <stdio.h>
//#include <stdlib.h> /* for flac_malloc() */
//#include <string.h> /* for memset/memcpy() */
//#include "assert.h"
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
#include "flac_get_bits.h"

_ATTR_FLACDEC_DATA_
static int flac_seek_time_dummy = (-1);

_ATTR_FLACDEC_BSS_
FLAC_Decoder_Info g_flac_decoder_info;

_ATTR_FLACDEC_BSS_
unsigned long g_CurrFileOffset; //the current file offset

_ATTR_FLACDEC_BSS_
int g_num_of_seek_table_points;

_ATTR_FLACDEC_BSS_
int g_cur_used_seek_table_points; //the start point to FFW or FFD

/* Seek Points Table ,put it to iram , by Vincent @ Apr 9 , 2009 */
_ATTR_FLACDEC_TEXT_
FLAC__StreamMetadata_SeekPoint g_seek_table_points[MAX__SEEK_POINTS_NAMBLE];

_ATTR_FLACDEC_BSS_
int g_f_FFW_FFD;// = false; //the flag of FFW or FFD

_ATTR_FLACDEC_BSS_
int g_pFLAC__BitBuffer; //point the frist address of g_FLAC__BitBuffer.

_ATTR_FLACDEC_BSS_
volatile int g_Flac_Buf_idx;// = 0;

_ATTR_FLACDEC_BSS_
FILE *g_hFlacFile, *g_hFlacFileBake;

_ATTR_FLACDEC_BSS_
FILE *g_hWaveFile;

_ATTR_FLACDEC_BSS_
FLAC__bool isPostHalfFrame, isNeedDecByHalfFrmae; // = 0;
_ATTR_FLACDEC_BSS_
int g_isGotFrame, g_SeekFileOffset, fail_cnt;

_ATTR_FLACDEC_BSS_
int isRightAfterSeek;
_ATTR_FLACDEC_BSS_
unsigned long FFWSampleNumber;
extern unsigned int g_read_frame_header_error_cnt;
extern int Flac_decoder_init() ;
extern int Flac_decoder_decode_frame() ;
extern int Flac_decoder_finish() ;
extern int init_get_bits(GetBitContext *s, unsigned char *buffer, int bit_size);
extern unsigned int get_bits(GetBitContext *s, int n);
extern int get_bits_left(GetBitContext *gb);
extern int my_bui_clz(unsigned int a);
extern int unaligned32_be( void *v);
extern unsigned long swap32(unsigned long value);
extern int unaligned32_be( void *v);
#ifdef FLAC_TABLE_ROOM_VERIFY
extern void flac_table_room_init(void);
extern long long CheckValidHeader_FlacSeek(GetBitContext *gb,int buf_len,int find_first_frame);

#endif

extern  unsigned long SRC_Num_Forehead; //for src
extern int metadata_size;
extern int ID3_len ;
extern int flac_output_length;

/*check sync code*/
unsigned char sync_code = 0xf8;
int sync_bytes = 0;
unsigned char inbuf[in_buf_size + 8];


_ATTR_FLACDEC_TEXT_
int flac_open_dec()
{
   /*define the last frame position*/
   unsigned long file_data_offset = 0;
   unsigned long file_size = RKFIO_FLength(pRawFileCache);
   long cur_pos = 0;
   unsigned char *buf = inbuf;
   int buf_size = in_buf_size;
   GetBitContext gb;
   int total_frame = 0;
   int buf_len = 0;
   int current_frame = 0;
   unsigned long data_pos = 0;
   /*define the last frame position end*/
    g_hFlacFile = pRawFileCache;
    g_hFlacFileBake = pFlacFileHandleBake;
    isRightAfterSeek = 0;
    g_Flac_Buf_idx = 0;
    g_read_frame_header_error_cnt = 0;
    g_f_FFW_FFD = false;
    isPostHalfFrame = 0;
    flac_output_length = 0;

    flac_MemSet(&g_FlacCodecBuffer[0][0], 0, FLAC__PCMBUFFER_DEFAULT_CAPACITY);
    flac_MemSet(&g_FlacCodecBuffer[1][0], 0, FLAC__PCMBUFFER_DEFAULT_CAPACITY);


    // RKFIO_FSeek(0, SEEK_SET, g_hFlacFile);
    //memset(g_FlacCodecBuffer, 0 , sizeof(g_FlacCodecBuffer));

    g_CurrFileOffset = 0; //add by fsh

#ifdef FLAC_TABLE_ROOM_VERIFY
    flac_table_room_init();
#endif

    /*decode initialization. */
    if (! Flac_decoder_init()) {
        flac_DEBUG("Flac_deocder_init failed!\n");
        goto ErrorExit;
    }

    //  printf("flac block size = %d\n", g_flac_decoder_info.blocksize);

    if (g_flac_decoder_info.blocksize > FLAC__PCMBUFFER_DEFAULT_CAPACITY ||
        g_flac_decoder_info.channels  > 2 ||
        g_flac_decoder_info.bits_per_sample != 16) {
        flac_DEBUG("can not play this file blocksize %d \n", g_flac_decoder_info.blocksize );
        goto ErrorExit;
    }

    g_flac_decoder_info.audio_data_offset = g_CurrFileOffset - FLAC__BITBUFFER_DEFAULT_CAPACITY + *(int*)g_pFLAC__BitBuffer;

    /*flac_DEBUG the id3 information.*/
    //flac_DEBUG("Album  = %s\n", g_flac_decoder_info.Album);
    //flac_DEBUG("Title  = %s\n", g_flac_decoder_info.Title);
    //flac_DEBUG("Artist = %s\n", g_flac_decoder_info.Artist);

    g_flac_decoder_info.total_play_time = ((long long)g_flac_decoder_info.total_samples * 1000) / g_flac_decoder_info.sample_rate;

    //compute bps.
    g_flac_decoder_info.bits_per_second = ((long long)(RKFIO_FLength(pRawFileCache)) * 1000 ) / (g_flac_decoder_info.total_play_time >> 3);
    // g_flac_decoder_info.bits_per_second *= 8;
    //flac_DEBUG("g_flac_decoder_info.total_samples = %d",g_flac_decoder_info.total_samples);
    //flac_DEBUG("g_flac_decoder_info.blocksize = %d",g_flac_decoder_info.blocksize);

    isPostHalfFrame = 0;

    /*calculate the last frame position by cdd 20170605*/
    data_pos = RKFIO_FTell(g_hFlacFile);
   // flac_DEBUG("%d",data_pos);
    init_get_bits(&gb, buf, (buf_size) * 8);
    cur_pos = file_size - buf_size;

    while(1)
    {
        RKFIO_FSeek(cur_pos, SEEK_SET, g_hFlacFile);
        buf_len = RKFIO_FRead(buf, buf_size, g_hFlacFile);
        //flac_DEBUG("cur_pos =%d",cur_pos);
        if(!buf_len)
        {
            flac_DEBUG("\nfind last frame fail\n");
            goto ErrorExit;
        }
        current_frame = (int)CheckValidHeader_FlacSeek(&gb, buf_len,0);
        //flac_DEBUG("current_frame= %d",current_frame);
       // flac_DEBUG("sync_bytes = %d",sync_bytes);
        if(current_frame >= 0)
        {
           g_flac_decoder_info.last_frame_pos = cur_pos + sync_bytes;
           g_flac_decoder_info.last_frame = current_frame;
           //flac_DEBUG("g_flac_decoder_info.last_frame_pos = 0x%x",g_flac_decoder_info.last_frame_pos);
          // flac_DEBUG("g_flac_decoder_info.last_frame = %d",g_flac_decoder_info.last_frame);
          // flac_DEBUG("version 1");
           break;
        }
        else
        {
            cur_pos -= buf_size;

            if(cur_pos <= 0)
            {
                flac_DEBUG("\nfind last frame fail\n");
                goto ErrorExit;
            }
        }
    }
    RKFIO_FSeek(data_pos, SEEK_SET, g_hFlacFile);
    return 1;
ErrorExit:
    return 0;
}

_ATTR_FLACDEC_TEXT_
int flac_close_dec()
{
    /*decode over*/
    Flac_decoder_finish();
    return 1;
}

_ATTR_FLACDEC_TEXT_
int flac_decode()
{
    FLAC__StreamDecoderState DecodeState;

    //g_isGotFrame = 0;
    //isRightAfterSeek = 0;
    flac_seek_time_dummy = (-1);

    if (g_f_FFW_FFD) {
      //  memset(g_FlacCodecBuffer, 0 , sizeof(g_FlacCodecBuffer));
    }


    DecodeState = (FLAC__StreamDecoderState)Flac_decoder_decode_frame();
#if 0
    if (isRightAfterSeek) {
        if (g_isGotFrame) {
            isPostHalfFrame ^= 1;
            isRightAfterSeek = 0;
        }
    } else {
        isPostHalfFrame ^= 1;
    }
#else
    if (isRightAfterSeek) {
        g_flac_decoder_info.samples_decoded = FFWSampleNumber;
    }

    if (g_isGotFrame) {
        isRightAfterSeek = 0;
        if (isNeedDecByHalfFrmae)
            isPostHalfFrame ^= 1;
    }
#endif
    if (g_flac_decoder_info.bits_per_second) {
        //g_flac_decoder_info.curr_play_time = ((long long)g_CurrFileOffset * 8 * 1000/ g_flac_decoder_info.bits_per_second);
        g_flac_decoder_info.curr_play_time = (int)((long long)g_flac_decoder_info.samples_decoded * 1000 / g_flac_decoder_info.sample_rate);

    }


    if (DecodeState == FLAC__STREAM_DECODER_END_OF_STREAM) {

        goto DecodeFailed; //decode finish normally.
    }

    if (DecodeState == FLAC__STREAM_DECODER_ABORTED || 50 == g_read_frame_header_error_cnt) {
        goto DecodeFailed; //decode finish abnormally.
    }



    //if(!isRightAfterSeek)
    {
        //flac_seek_time_dummy = (-1);
    }
    return 1;

DecodeFailed:
    return 0;

}

_ATTR_FLACDEC_TEXT_
int flac_get_samplerate()
{
    return g_flac_decoder_info.sample_rate;
}

_ATTR_FLACDEC_TEXT_
int flac_get_channels()
{
    //  return 2;   /* TODO , but outside don't care. by Vincent @ Apr 13 , 2009 */
    //flac_DEBUG("channels = %d\n",g_flac_decoder_info.channels);
    return g_flac_decoder_info.channels;
}

_ATTR_FLACDEC_TEXT_
int flac_get_bitrate()
{
    return (g_flac_decoder_info.bits_per_second + 500);
}

_ATTR_FLACDEC_TEXT_
int flac_get_bps()
{
    return g_flac_decoder_info.bits_per_sample;
}
int flac_get_length()
{
    return g_flac_decoder_info.total_play_time ;
}


_ATTR_FLACDEC_TEXT_
int flac_get_timepos()
{
    //if (flac_seek_time_dummy != (-1))
    //return flac_seek_time_dummy;
    //else
    return g_flac_decoder_info.curr_play_time;
}

extern int flac_output_length;
_ATTR_FLACDEC_TEXT_
int flac_get_buffer(int *ptr, int *length)
{
    *ptr = (int) &g_FlacCodecBuffer[g_Flac_Buf_idx][SRC_Num_Forehead];

    if (flac_output_length == 0)
        flac_output_length = 2304;
    *length = flac_output_length;

    {
        if (g_Flac_Buf_idx == 0)
            g_Flac_Buf_idx = 1;
        else
            g_Flac_Buf_idx = 0;
    }
    //  printf("buf = %x， len = %d\n", *ptr, flac_output_length);
    return 1;
}






int FLAC_get_more_data(GetBitContext *gb, int bits_needed,int buf_len)
{
    int ret;
    int index ;
    int left_byte ;
    int bits_left = get_bits_left(gb);
    if (bits_left < 0) {
        bits_left = 0;
    }
    if (bits_needed > bits_left ) {
        index = gb->index >> 3;
        left_byte = (bits_left + 7) >> 3;
        memmove(gb->buffer, &gb->buffer[index], left_byte);
        ret = RKFIO_FRead(&gb->buffer[left_byte], buf_len - left_byte , g_hFlacFile);
        gb->index  = gb->index % 8;

        if (ret == 0) {
            return 0;
        }
    }
    return 1 ;
}
long long int get_utf8(GetBitContext *gb)
{
    long long int val;
    GET_UTF8(val, get_bits(gb, 8), return -1;)
    return val;
}

int frame_header(GetBitContext *gb, FLACFrameInfo *fi, int buf_len)//查找帧头
{
    int bs_code, sr_code, bps_code, crc8;
    int sample_size_table[] = { 0, 8, 12, 0, 16, 20, 24, 0 };
    int ff_flac_sample_rate_table[16] = {
        0, 88200, 176400, 192000,
        8000, 16000, 22050, 24000,
        32000, 44100, 48000, 96000,
        0, 0, 0, 0
    };
    int ff_flac_blocksize_table[16] = {
        0, 192, 576 << 0, 576 << 1, 576 << 2, 576 << 3,      0,      0,
        256 << 0, 256 << 1, 256 << 2, 256 << 3, 256 << 4, 256 << 5, 256 << 6, 256 << 7
    };
    int sync_flag = 0;
    unsigned char buf[2];
    int ret = 0;
    int find_sync_bytes = 0;

    while (1) {

      /*  if (find_sync_bytes >= buf_len) {
            //flac_DEBUG("find_sync_bytes = %d",find_sync_bytes);
            sync_bytes = find_sync_bytes;
            return -1;
        }*/
        while (sync_flag == 0) {
           // flac_DEBUG();
            ret = FLAC_get_more_data(gb, 16,buf_len);
            if (ret == 0) {
                return  -1;
            }
            buf[0] = get_bits(gb, 8);
            find_sync_bytes++;
            if (buf[0] == 0xff) {
                buf[1] = get_bits(gb, 8);
               // flac_DEBUG("0x%x",buf[1]);
                find_sync_bytes++;
                if (buf[1] == 0xf8) {
                    //flac_DEBUG();
                    sync_flag = 1;
                }
            }
            if (0 == g_flac_decoder_info.max_framesize) {
                g_flac_decoder_info.max_framesize = 4608;
            }
            if (find_sync_bytes > buf_len) {
                //flac_DEBUG("find_sync_bytes = %d",find_sync_bytes);
                //flac_DEBUG();
                return  -1;//返回一帧帧长
            }
        }
        //flac_DEBUG();
        FLAC_get_more_data(gb, 64,buf_len);
        find_sync_bytes +=  8;
        /* block size and sample rate codes */
        bs_code = get_bits(gb, 4);//块大小
        sr_code = get_bits(gb, 4);

        /* channels and decorrelation */
        fi->ch_mode = get_bits(gb, 4);
        if (fi->ch_mode < 2) {
            fi->channels = fi->ch_mode + 1;
            fi->ch_mode = FLAC_CHMODE_INDEPENDENT;
        } else if (fi->ch_mode <= 2 + FLAC_CHMODE_MID_SIDE) {
            fi->channels = 2;
        } else {
            sync_flag = 0;
            //flac_DEBUG("find_sync_bytes = %d",find_sync_bytes);
            get_bits(gb,4);
            continue;
        }
        /* bits per sample */
        bps_code = get_bits(gb, 3);
        if (bps_code == 3 || bps_code == 7) {
            get_bits(gb,1);
            sync_flag = 0;
            //flac_DEBUG();
            continue;
            //return -1;
        }
        fi->bps = sample_size_table[bps_code];

        if (fi->bps != g_flac_decoder_info.bits_per_sample) {
            get_bits(gb,1);
            sync_flag = 0;
            //flac_DEBUG("find_sync_bytes = %d",find_sync_bytes);
           // flac_DEBUG("fi->bps = %d",fi->bps);
            //flac_DEBUG("g_flac_decoder_info.bits_per_sample = %d",g_flac_decoder_info.bits_per_sample);
            continue;
        }
        if (get_bits1(gb)) {
            sync_flag = 0;
            //flac_DEBUG();
            continue;
        }
        fi->frame_or_sample_num = get_utf8(gb);
        if (fi->frame_or_sample_num < 0) {
            sync_flag = 0;
            //flac_DEBUG();
            continue;
        }

        /* blocksize */
        if (bs_code == 0) {
            sync_flag = 0;
            //flac_DEBUG();
            continue;
        } else if (bs_code == 6) {
            fi->blocksize = get_bits(gb, 8) + 1;
            find_sync_bytes ++;
        } else if (bs_code == 7) {
            fi->blocksize = get_bits(gb, 16) + 1;
            find_sync_bytes +=  2;
        } else {
            fi->blocksize = ff_flac_blocksize_table[bs_code];
        }
        if (sr_code < 12) {
            fi->samplerate = ff_flac_sample_rate_table[sr_code];
        } else if (sr_code == 12) {
            fi->samplerate = get_bits(gb, 8) * 1000;
            find_sync_bytes ++;
        } else if (sr_code == 13) {
            fi->samplerate = get_bits(gb, 16);
            find_sync_bytes +=  2;
        } else if (sr_code == 14) {
            fi->samplerate = get_bits(gb, 16) * 10;
            find_sync_bytes +=  2;
        } else {
            sync_flag = 0;
            //flac_DEBUG();
            continue;
        }
        if ((fi->samplerate != g_flac_decoder_info.sample_rate)) {
            //flac_DEBUG("fi->samplerate = %d",fi->samplerate);
            sync_flag = 0;
            continue;
        }
        sync_bytes = find_sync_bytes - 10;
        //flac_DEBUG();
        return 0;

    }
    //

}
#if 1
/*find_first_frame : is it check the first frame during the buffer ,only 0 or 1*/
long long CheckValidHeader_FlacSeek(GetBitContext *gb,int buf_in_len,int find_first_frame)
{
    FLACFrameInfo fi;
    unsigned char *buf = inbuf;
    int buf_size = buf_in_len;

#if 1
    int frame_num = -1;
    int header_flag ;
    int tmp = 0;
    int frame_sync_bytes = 0;
    int ret;
    int i = 0;
    if (find_first_frame == 1)//check the first frame during the buffer
    {
         memset(&fi, 0, sizeof(fi));
        init_get_bits(gb, buf, (buf_size) * 8);
        if (frame_header(gb,&fi,buf_size) < 0)
            return -1;
        else
            return fi.frame_or_sample_num;
    }
    else if (find_first_frame == 0)//check the last frame during the buffer
    {
        while(1)
        {
            memset(&fi, 0, sizeof(FLACFrameInfo));
            init_get_bits(gb, (buf + frame_sync_bytes), (buf_size) * 8);
            header_flag = frame_header(gb, &fi,buf_size);
            if(header_flag < 0)
            {
                sync_bytes = frame_sync_bytes - 16;
                return frame_num;
            }
            else
            {
                if (fi.frame_or_sample_num > 0)
                frame_num = fi.frame_or_sample_num;
                frame_sync_bytes += sync_bytes + 10;
                buf_size = buf_in_len - frame_sync_bytes;

                if(buf_size <= 10)
                {
                   sync_bytes = frame_sync_bytes - 16;
                    return frame_num;
                }
            }
        }

    }
    else
    {
        flac_DEBUG("check frame header parameter error!!!\n");
        return -1;
    }

#else

    memset(&fi, 0, sizeof(fi));
    init_get_bits(gb, buf, (buf_size) * 8);
    if (frame_header(gb, &fi,buf_len) < 0)
        return -1;
    else
        return fi.frame_or_sample_num;

 #endif
}

int flac_abs(int a)
{
    return a >= 0 ? a : (0 - a);
}

#define FRAME_PIECE 1
int flac_seek(int mseconds)
{

    unsigned long  target_sample = 0,current_sample = 0;

    long  target_frame = 0,current_frame = 0,max_frame = 0,min_frame = 0;
    unsigned long  target_pos = 0,current_pos = 0,max_pos = 0,max_pos1 = 0,min_pos = 0,mid_pos = 0;
    unsigned long total_samples =  g_flac_decoder_info.total_samples;
    unsigned long FileSize = RKFIO_FLength(pRawFileCache);
    unsigned long last_pos = g_flac_decoder_info.last_frame_pos;
    int last_frame = g_flac_decoder_info.last_frame;
    long  FFWOffset = 0;
    int  approx_bytes_per_frame = 0;
    int blocksize = g_flac_decoder_info.blocksize;
    int ff_flag = 0;
    unsigned char *buf = inbuf;
    int buf_size = in_buf_size;
    int delta = 0, delta0 = 0;
    GetBitContext gb;
    int buf_len = 0;
    unsigned long seconds = mseconds / 1000, ms = mseconds % 1000;

    target_sample = seconds * g_flac_decoder_info.sample_rate + ms * g_flac_decoder_info.sample_rate / 1000;
    target_frame = target_sample / blocksize;
    //flac_DEBUG("mseconds = %d",mseconds);
   // flac_DEBUG("target_frame = %d",target_frame);
    if (target_sample == 0) {
        FFWOffset = 0;
        RKFIO_FSeek(0, SEEK_SET, g_hFlacFile);
        current_sample = 0;
        goto flac_seek_ok;
    }
    else if(target_frame >= last_frame)
    {
        FFWOffset = last_pos;
        RKFIO_FSeek(FFWOffset, SEEK_SET, g_hFlacFile);
        current_sample = last_frame * blocksize;
        goto flac_seek_ok;
    }
    if (g_flac_decoder_info.max_framesize == 0)
        g_flac_decoder_info.max_framesize = 4608;


    approx_bytes_per_frame = g_flac_decoder_info.max_framesize;
    buf_size = g_flac_decoder_info.max_framesize;
    if (buf_size > in_buf_size)
        buf_size = in_buf_size;

    init_get_bits(&gb, buf, (buf_size) * 8);

    /* 查找当前位置帧号 start*/
    current_pos = RKFIO_FTell(g_hFlacFile);
    if (current_pos > last_pos)
    {
        RKFIO_FSeek(last_pos, SEEK_SET, g_hFlacFile);
        current_pos = last_pos;
    }
    buf_len = RKFIO_FRead(buf, buf_size, g_hFlacFile);
    if(buf_len == 0)
        while(1)
            flac_DEBUG("find current frame fail!!!\n");
    while (1) {
        current_frame = (int)CheckValidHeader_FlacSeek(&gb, buf_len,1);
        //flac_DEBUG("current_frame = %d",current_frame);
        if (current_frame >= 0) {
            current_pos = current_pos + sync_bytes;
            break;
        } else {
            buf_len = RKFIO_FRead(buf, buf_size, g_hFlacFile);
            if(buf_len == 0)
                while(1)
                    flac_DEBUG("find current frame fail!!!\n");
            current_pos += buf_len;
            if (current_pos >= last_pos) {
                FFWOffset = last_pos;
                current_sample = last_frame * blocksize;
               // flac_DEBUG();
                goto flac_seek_ok;
            }
        }
    }
     /* 查找当前位置帧号 end*/

    delta =fabs(target_frame - current_frame);
     ///flac_DEBUG("delta  = %d",delta);
     if(delta <= FRAME_PIECE)
     {
        current_sample = current_frame * blocksize;
        FFWOffset = current_pos;
        RKFIO_FSeek(FFWOffset, SEEK_SET, g_hFlacFile);
        //flac_DEBUG();
        goto flac_seek_ok;
     }
     if (target_frame > current_frame){ //快进
         ff_flag = 0x01;
         FFWOffset = (long)(current_pos + (delta * approx_bytes_per_frame));
         if(FFWOffset > last_pos)
         {
            FFWOffset = last_pos;
            max_frame = last_frame;
            max_pos = last_pos;
         }
         else {
            RKFIO_FSeek(FFWOffset, SEEK_SET, g_hFlacFile);
            max_pos = RKFIO_FTell(g_hFlacFile);
            max_pos = FFWOffset;
            buf_len = RKFIO_FRead(buf, buf_size, g_hFlacFile);
            if(buf_len == 0)
                while(1)
                    flac_DEBUG("cannot read more data!!!\n");
            while(1){
                max_frame = (int)CheckValidHeader_FlacSeek(&gb, buf_len,1);
                //flac_DEBUG("max_frame = %d",max_frame);
                if (max_frame >= 0) {
                    max_pos = max_pos + sync_bytes;
                     break;
                } else {
                  buf_len = RKFIO_FRead(buf, buf_size, g_hFlacFile);
                  max_pos += buf_len;
                  if (!buf_len) {
                    FFWOffset = last_pos;
                    max_frame = last_frame;
                    max_pos = last_pos;
                    break;
                   }
               }
            }
        }
         min_frame = current_frame;
         min_pos = current_pos;
     } else { //快退
        ff_flag = 0x02;
         FFWOffset = (long)(current_pos - (delta * approx_bytes_per_frame));
         if(FFWOffset < 0){
           FFWOffset = 0;
           min_frame = 0;
           min_pos = 0;
        }
         else
        {
            RKFIO_FSeek(FFWOffset, SEEK_SET, g_hFlacFile);
            min_pos = RKFIO_FTell(g_hFlacFile);
            buf_len = RKFIO_FRead(buf, buf_size, g_hFlacFile);
            if(buf_len == 0)
                while(1)
                    flac_DEBUG("cannot read more data!!!\n");
            while(1){
                min_frame = (int)CheckValidHeader_FlacSeek(&gb, buf_len,1);
                //flac_DEBUG("min_frame = %d",min_frame);
                if (min_frame >= 0) {
                    min_pos = min_pos + sync_bytes;
                     break;
                } else {
                  buf_len = RKFIO_FRead(buf, buf_size, g_hFlacFile);
                  min_pos += buf_len;
               }
            }
        }
       max_frame = current_frame;
       max_pos = current_pos;
     }

     if((fabs(min_frame - target_frame)<= FRAME_PIECE))
     {
        current_sample = min_frame * blocksize;
        FFWOffset = min_pos;
        RKFIO_FSeek(FFWOffset, SEEK_SET, g_hFlacFile);
       // flac_DEBUG();
        goto flac_seek_ok;
     } else if(fabs(max_frame - target_frame)<=FRAME_PIECE){
        current_sample = max_frame * blocksize;
        FFWOffset = max_pos;
        RKFIO_FSeek(FFWOffset, SEEK_SET, g_hFlacFile);
        //flac_DEBUG();
        goto flac_seek_ok;
     }

    while (1) {
        //flac_DEBUG("min_frame = %d,max_frame = %d,target_frame = %d\n",min_frame,max_frame,target_frame);
       // flac_DEBUG("min_pos = %d,max_pos = %d\n",min_pos,max_pos);
        FFWOffset = min_pos + ((max_pos - min_pos) / 2);
        RKFIO_FSeek(FFWOffset, SEEK_SET, g_hFlacFile);
        current_pos = RKFIO_FTell(g_hFlacFile);
        mid_pos = current_pos;
        buf_len = RKFIO_FRead(buf, buf_size, g_hFlacFile);
        if(buf_len == 0)
            while(1)
                flac_DEBUG("cannot read more data!!!\n");
        while(1){
          // flac_DEBUG("current_pos = %d",current_pos);
           current_frame = (int)CheckValidHeader_FlacSeek(&gb, buf_len,1);
          // flac_DEBUG("current_frame = %d",current_frame);
          // flac_DEBUG("sync_bytes = %d",sync_bytes);
           if (current_frame >= 0) {
               current_pos = current_pos + sync_bytes;
               break;
           } else {
               buf_len = RKFIO_FRead(buf, buf_size, g_hFlacFile);
               if (buf_len == 0)
                  while(1)
                    flac_DEBUG("cannot read more data!!!\n");
               //flac_DEBUG("buf_len = %d",buf_len);
               current_pos += buf_len;
           }
        }

        if(fabs(current_frame - target_frame)<=FRAME_PIECE)
        {
            current_sample = current_frame * blocksize;
            FFWOffset = current_pos;
             RKFIO_FSeek(FFWOffset, SEEK_SET, g_hFlacFile);
            goto flac_seek_ok;
        }
         else if((current_frame >= max_frame) ||(current_frame <= min_frame))//容错
        {
          //flac_DEBUG("ff_flag = %d",ff_flag);
           if(ff_flag == 0x01)
           {
               current_sample = max_frame * blocksize;
               FFWOffset = max_pos1;
               RKFIO_FSeek(FFWOffset, SEEK_SET, g_hFlacFile);
               //flac_DEBUG();
               goto flac_seek_ok;
           }
           else if(ff_flag == 0x02)
           {
                current_sample = min_frame * blocksize;
                FFWOffset = min_pos;
                RKFIO_FSeek(FFWOffset, SEEK_SET, g_hFlacFile);
                //flac_DEBUG();
                goto flac_seek_ok;
           }
        }
        else  if(current_frame > target_frame) {
            max_frame = current_frame;
            max_pos = mid_pos;
            max_pos1 = current_pos;
        } else if(current_frame < target_frame) {
            min_frame = current_frame;
            min_pos = current_pos;
        }
    }

flac_seek_ok:
    FFWSampleNumber = current_sample;
    g_flac_decoder_info.samples_decoded = current_sample;
    g_flac_decoder_info.curr_play_time  = (int)((unsigned long long)(g_flac_decoder_info.samples_decoded) * 1000 / g_flac_decoder_info.sample_rate);
    //flac_DEBUG("g_flac_decoder_info.curr_play_time = %d",g_flac_decoder_info.curr_play_time);
    //flac_DEBUG("current_sample = %d",current_sample);
    g_f_FFW_FFD = true;
    isPostHalfFrame  = 0;
    isRightAfterSeek = 1;
    g_CurrFileOffset = FFWOffset;
    g_SeekFileOffset = FFWOffset;
   // flac_DEBUG("#######_seek_ok_#########");
    return 0;
//flac_seek_fail:
   // flac_DEBUG("#######_seek_fail_#########");
   // return -1;
}

#endif
#if 0
_ATTR_FLACDEC_TEXT_
unsigned char CheckValidHeader_FlacSeek(unsigned char *header)
{
    unsigned char buf_data[8];
    unsigned char bs_code, sr_code, bps_code, ch_mode;
    unsigned char sz_table[] = { 0, 8, 12, 0, 16, 20, 24, 0 };
    unsigned int src_table[] = {0, 88200, 176400, 192000, 8000, 16000, 22050, 24000, 32000, 44100, 48000, 96000};
    unsigned int src_temp;
    long long int val;

    buf_data[0] = *header++;

    if (buf_data[0] == 0xff) {
        buf_data[1] =  *header++;

        //if((buf_data[1] & 0xfe) == sync_code)        //sync_code = 0xfff8 ? 0xfff9
        if (buf_data[1] == sync_code) {
            buf_data[2] =  *header++;
            buf_data[3] =  *header++;
            buf_data[4] =  *header++;
            //bb_printf1("sync code = 0x%02x sync = 0x%02x, %02x, %02x, %02x",sync_code, buf_data[0], buf_data[1], buf_data[2], buf_data[3]);

#if 1
            bs_code = buf_data[2] >> 4;     //samples
            sr_code = buf_data[2] & 0xf;    //sample rate
            if (sr_code == 0xf) {
                // invalid, to prevent sync-fooling string of 1s
                //Hifi_Flac_Printf("sync: sample rate error!");
                return 0;
            } else if ((0x00 < sr_code) && (sr_code < 0x0c)) {
                src_temp = src_table[sr_code];
                if (src_temp != g_flac_decoder_info.sample_rate) {
                    //error
                    //bb_printf1("sync: error! s_FLAC_INFO.samplerate= %d, src_temp= %d", g_flac_decoder_info.sample_rate, src_temp);
                    return 0;
                }
            } else {
                //0x00: get from STREAMINFO metadata block
                //0x0c: get 8 bit sample rate (in kHz) from end of header
                //0x0d: get 16 bit sample rate (in Hz) from end of header
                //0d0e: get 16 bit sample rate (in tens of Hz) from end of header
            }
#endif

#if 1
            //ch_mode  = (buf_data[3] >> 4) + 1;

            ch_mode  = (buf_data[3] >> 4);
            bps_code = (buf_data[3] >> 1) & 0x7;
            if ((buf_data[3] & 0x1)) {      //0 : mandatory value    1 : reserved for future use
                //Hifi_Flac_Printf("sync: mandatory value error!");
                return 0;
            }
#endif

#if 1
            if (ch_mode > 12) {
                return 0;
            }
#endif

#if 1
            if (bps_code != 0) {
                if ((bps_code == 3) || (bps_code == 7)) { // reserved
                    //Hifi_Flac_Printf("sync: bits per sampl error! bps_code = %d", bps_code);
                    return 0;
                } else {      //get from STREAMINFO metadata block
                    bps_code = sz_table[bps_code];
                    if (bps_code != g_flac_decoder_info.bits_per_sample) {
                        //bb_printf1("sync: error! s_FLAC_INFO.bps= %d, bps_code= %d", g_flac_decoder_info.bits_per_sample, bps_code);
                        return 0;
                    }
                }
            }
#endif

            GET_UTF8(val, buf_data[4], return -1;)
            Hifi_Flac_Printf("val = %ld\n ", val);
            return 1;
        }
    }

    return 0;
}

#define in_buf_size 16384
unsigned char inbuf[in_buf_size + 8];

_ATTR_FLACDEC_TEXT_
int flac_seek(int mseconds)
{
    unsigned long FFWOffset, realFFWOffset;
    unsigned long seconds, ms;
    int i;
    int FileSize = RKFIO_FLength(pRawFileCache);

    //bb_printf1("ms = %d, g_flac_decoder_info.curr_play_time = %d", mseconds, g_flac_decoder_info.curr_play_time);
#if 1
    seconds = mseconds / 1000;
    ms      = mseconds % 1000;
    FFWOffset = (unsigned long)((unsigned long long)(seconds) * (g_flac_decoder_info.bits_per_second / 8) + (ms * (g_flac_decoder_info.bits_per_second / 8)) / 1000);
    FFWSampleNumber = (unsigned long)((unsigned long long)(seconds) * g_flac_decoder_info.sample_rate + (ms * g_flac_decoder_info.sample_rate) / 1000);

    if (FFWOffset > FileSize) {
        goto flac_seek_fail;
    }

    if (RKFIO_FSeek(FFWOffset, SEEK_SET , g_hFlacFile) != 0) {
        goto flac_seek_fail;
    }

    {
        unsigned char *buf;
        int ret;
        int pos = 0;
        int sync_flag = 0;
        int seekfix_counter = 0;

        buf = inbuf;
        RKFIO_FRead(buf , in_buf_size ,  g_hFlacFile );

        while (1) {
            ret = CheckValidHeader_FlacSeek(&buf[pos]);
            Hifi_Flac_Printf("val = %ld\n ", ret);
            if (ret) {
                break;
            }

            pos += 1;

            if (pos >= in_buf_size) {
                size_t cbRead;
                buf = inbuf;
                pos = 0;
                cbRead = RKFIO_FRead(buf , in_buf_size ,  g_hFlacFile );
                if ((cbRead != in_buf_size) || (seekfix_counter++ >= 2048)) {      //读取32m BYTE
                    //Hifi_Flac_Printf("seekfix_counter = %d ",seekfix_counter);
                    break;
                }
                //break;
            }
        }

        //FFWOffset += pos ;
        FFWOffset += (pos + in_buf_size * seekfix_counter);
        //Hifi_Flac_Printf("seek校正 0x%x 0x%x ",FFWOffset,buf[pos+1]);
    }

    if (RKFIO_FSeek(FFWOffset, SEEK_SET , g_hFlacFile) != 0) {
        goto flac_seek_fail;
    }

    g_CurrFileOffset = FFWOffset;
    g_SeekFileOffset = FFWOffset;
    FFWSampleNumber  = (unsigned long)((unsigned long long)(FFWOffset) * 8  * g_flac_decoder_info.sample_rate / g_flac_decoder_info.bits_per_second);

    g_f_FFW_FFD = true;

    isPostHalfFrame  = 0;
    isRightAfterSeek = 1;

flac_seek_ok:
    //flac_seek_time_dummy =  FFWOffset * 1000 / (g_flac_decoder_info.bits_per_second / 8);//seconds * 1000;
    g_flac_decoder_info.samples_decoded = (unsigned long)((unsigned long long)(FFWOffset) * 8  * g_flac_decoder_info.sample_rate / g_flac_decoder_info.bits_per_second);
    g_flac_decoder_info.curr_play_time  = (unsigned long)((unsigned long long)(g_flac_decoder_info.samples_decoded) * 1000 / g_flac_decoder_info.sample_rate);

    //bb_printf1("ms111 = %d", g_flac_decoder_info.curr_play_time);

    return 0;

#else

    //assert(g_flac_decoder_info.total_samples < 0x1fffffff);
    //assert(FileSize < 0x1fffffff);

    // 计算bps
    //g_flac_decoder_info.bits_per_second = (FileSize << 3) / ((g_flac_decoder_info.total_samples << 3) / g_flac_decoder_info.sample_rate);
    //g_flac_decoder_info.bits_per_second *= 8;

    FFWOffset = seconds * (g_flac_decoder_info.bits_per_second / 8);
    FFWSampleNumber = (unsigned long)seconds * g_flac_decoder_info.sample_rate;


    if (FFWOffset > FileSize) {
        goto flac_seek_fail;
    }


    realFFWOffset = 0;
#if 0
    if (g_num_of_seek_table_points != 0) {
        for (i = 0; i < g_num_of_seek_table_points - 1; i++) {
            unsigned long first_point, second_point;

            first_point = g_seek_table_points[i].stream_offset + g_flac_decoder_info.audio_data_offset;
            second_point = g_seek_table_points[i + 1].stream_offset + g_flac_decoder_info.audio_data_offset;
            if ((FFWOffset > first_point) && (FFWOffset < second_point)) {
                realFFWOffset = (FFWOffset - first_point) > (second_point - FFWOffset) ? first_point : second_point;
                g_cur_used_seek_table_points = i;
                break;
            }
        }

        if (FFWOffset < g_seek_table_points[0].stream_offset + g_flac_decoder_info.audio_data_offset) {
            realFFWOffset = g_flac_decoder_info.audio_data_offset;
            g_cur_used_seek_table_points = 0;
        }

        if (FFWOffset > g_seek_table_points[g_num_of_seek_table_points - 1].stream_offset + g_flac_decoder_info.audio_data_offset) {
            realFFWOffset = g_seek_table_points[g_num_of_seek_table_points - 1].stream_offset + g_flac_decoder_info.audio_data_offset;
            g_cur_used_seek_table_points = g_num_of_seek_table_points - 1;
        }
    } else
#endif
    {
        //realFFWOffset = (g_flac_decoder_info.bits_pe了、
        // 根据样点数来seek
        for (i = 0; i < g_num_of_seek_table_points - 1; i++) {
            unsigned long first_point, second_point;

            first_point = g_seek_table_points[i].sample_number;
            second_point = g_seek_table_points[i + 1].sample_number;
            if ((FFWSampleNumber > first_point) && (FFWSampleNumber < second_point)) {
                realFFWOffset = g_seek_table_points[i].stream_offset + g_flac_decoder_info.audio_data_offset;
                g_cur_used_seek_table_points = i;
                break;
            }
            * /
        }

        if (RKFIO_FSeek(realFFWOffset, SEEK_SET , g_hFlacFile) != 0) {
            goto flac_seek_fail;
        }

        g_CurrFileOffset = realFFWOffset;
        g_SeekFileOffset = realFFWOffset;

        g_f_FFW_FFD = true;

        isPostHalfFrame = 0;
        isRightAfterSeek = 1;

    flac_seek_ok:
        flac_seek_time_dummy = seconds * 1000;
        g_flac_decoder_info.samples_decoded = FFWSampleNumber;
        g_flac_decoder_info.curr_play_time = (int)((long long)g_flac_decoder_info.samples_decoded * 1000 / g_flac_decoder_info.sample_rate);
        //if (g_flac_decoder_info.bits_per_second)
        {
            //updata current time
            //g_flac_decoder_info.curr_play_time = 0;//(int)((long long)(g_flac_decoder_info.samples_decoded + FFWSampleNumber)*1000/ g_flac_decoder_info.sample_rate);
            //g_flac_decoder_info.samples_decoded  = 0;

        }
        //memset(g_FlacCodecBuffer,0,sizeof(g_FlacCodecBuffer));
        return 0;
#endif

flac_seek_fail:
    return -1;
}
#endif

#endif
#endif
