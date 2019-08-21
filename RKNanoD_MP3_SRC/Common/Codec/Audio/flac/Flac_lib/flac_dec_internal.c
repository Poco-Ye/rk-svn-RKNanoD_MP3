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
FILE *g_hFlacFile,*g_hFlacFileBake;

_ATTR_FLACDEC_BSS_
FILE *g_hWaveFile;

_ATTR_FLACDEC_BSS_
FLAC__bool isPostHalfFrame,isNeedDecByHalfFrmae;// = 0;
_ATTR_FLACDEC_BSS_
int g_isGotFrame,g_SeekFileOffset,fail_cnt;

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
#endif

extern  unsigned long SRC_Num_Forehead; //for src
extern int metadata_size;
extern int ID3_len ;
extern int flac_output_length;

_ATTR_FLACDEC_TEXT_
int flac_open_dec()
{
    g_hFlacFile = pRawFileCache;
    g_hFlacFileBake = pFlacFileHandleBake;
    isRightAfterSeek = 0;
    g_Flac_Buf_idx = 0;
    g_read_frame_header_error_cnt = 0;
    g_f_FFW_FFD = false;
    isPostHalfFrame = 0;
    flac_output_length = 0;

    flac_MemSet(&g_FlacCodecBuffer[0][0],0,FLAC__PCMBUFFER_DEFAULT_CAPACITY);
    flac_MemSet(&g_FlacCodecBuffer[1][0],0,FLAC__PCMBUFFER_DEFAULT_CAPACITY);


   // RKFIO_FSeek(0, SEEK_SET, g_hFlacFile);
    //memset(g_FlacCodecBuffer, 0 , sizeof(g_FlacCodecBuffer));

    g_CurrFileOffset=0;//add by fsh

    #ifdef FLAC_TABLE_ROOM_VERIFY
    flac_table_room_init();
    #endif

    /*decode initialization. */
    if (! Flac_decoder_init())
    {
        flac_DEBUG("Flac_deocder_init failed!\n");
        goto ErrorExit;
    }

  //  printf("flac block size = %d\n", g_flac_decoder_info.blocksize);

    if (g_flac_decoder_info.blocksize > FLAC__PCMBUFFER_DEFAULT_CAPACITY ||
            g_flac_decoder_info.channels  > 2 ||
            g_flac_decoder_info.bits_per_sample != 16)
    {
        flac_DEBUG("can not play this file blocksize %d \n",g_flac_decoder_info.blocksize );
        goto ErrorExit;
    }

    g_flac_decoder_info.audio_data_offset = g_CurrFileOffset - FLAC__BITBUFFER_DEFAULT_CAPACITY + *(int*)g_pFLAC__BitBuffer;

    /*flac_DEBUG the id3 information.*/
    //flac_DEBUG("Album  = %s\n", g_flac_decoder_info.Album);
    //flac_DEBUG("Title  = %s\n", g_flac_decoder_info.Title);
    //flac_DEBUG("Artist = %s\n", g_flac_decoder_info.Artist);

    g_flac_decoder_info.total_play_time =( (long long )g_flac_decoder_info.total_samples *1000)/ g_flac_decoder_info.sample_rate;

    //compute bps.
    g_flac_decoder_info.bits_per_second = ((long long )(RKFIO_FLength(pRawFileCache)-metadata_size-ID3_len)*1000 )/ (g_flac_decoder_info.total_play_time>>3);
   // g_flac_decoder_info.bits_per_second *= 8;

    isPostHalfFrame = 0;

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

    if (g_f_FFW_FFD)
    {
        //memset(g_FlacCodecBuffer, 0 , sizeof(g_FlacCodecBuffer));
    }


    DecodeState = (FLAC__StreamDecoderState)Flac_decoder_decode_frame();
#if 0
    if(isRightAfterSeek)
    {
        if(g_isGotFrame)
        {
           isPostHalfFrame ^= 1;
           isRightAfterSeek = 0;
        }
    }
    else
    {
        isPostHalfFrame ^= 1;
    }
#else
    if(isRightAfterSeek)
        {
    g_flac_decoder_info.samples_decoded = FFWSampleNumber;
        }

    if(g_isGotFrame)
    {
       isRightAfterSeek = 0;
       if(isNeedDecByHalfFrmae)
          isPostHalfFrame ^= 1;
    }
#endif
    if (g_flac_decoder_info.bits_per_second)
    {
        //g_flac_decoder_info.curr_play_time = ((long long)g_CurrFileOffset * 8 * 1000/ g_flac_decoder_info.bits_per_second);
        g_flac_decoder_info.curr_play_time = (int)((long long)g_flac_decoder_info.samples_decoded *1000/ g_flac_decoder_info.sample_rate);

    }


    if (DecodeState == FLAC__STREAM_DECODER_END_OF_STREAM)
    {

        goto DecodeFailed; //decode finish normally.
    }

    if (DecodeState == FLAC__STREAM_DECODER_ABORTED||50 == g_read_frame_header_error_cnt)
    {
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
    flac_DEBUG("channels = %d\n",g_flac_decoder_info.channels);
    return g_flac_decoder_info.channels;
}

_ATTR_FLACDEC_TEXT_
int flac_get_bitrate()
{
    return (g_flac_decoder_info.bits_per_second +500);
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
int flac_get_buffer(int *ptr,int *length)
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


unsigned char sync_code = 0xf8;
int sync_bytes = 1;
unsigned char inbuf[in_buf_size + 8];


 int FLAC_get_more_data(GetBitContext *gb,int bits_needed )
    {
        int ret;
        int index ;
        int left_byte ;
        int bits_left = get_bits_left(gb);
        if(bits_left < 0)
        {
          // printf("bits_left 负%d\n",bits_left);
           bits_left = 0;
        }
        if(bits_needed > bits_left )
        {
            index = gb->index >>3;
            left_byte = (bits_left+7)>>3;
            memmove(gb->buffer, &gb->buffer[index], left_byte);
            //printf("file offset %x\n",ftell(infile));
          //  ret = fread(&gb->buffer[left_byte],1, in_buf_size-left_byte, infile);
            ret = RKFIO_FRead(&gb->buffer[left_byte], in_buf_size-left_byte , g_hFlacFile);
            gb->index  = gb->index%8;

            if(ret == 0)
            {
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

int frame_header(GetBitContext *gb, FLACFrameInfo *fi )//查找帧头
{
    int bs_code, sr_code, bps_code, crc8;
    int sample_size_table[] = { 0, 8, 12, 0, 16, 20, 24, 0 };
    int ff_flac_sample_rate_table[16] =
        { 0,
        88200, 176400, 192000,
        8000, 16000, 22050, 24000, 32000, 44100, 48000, 96000,
        0, 0, 0, 0 };
    int ff_flac_blocksize_table[16] = {
         0,    192, 576<<0, 576<<1, 576<<2, 576<<3,      0,      0,
        256<<0, 256<<1, 256<<2, 256<<3, 256<<4, 256<<5, 256<<6, 256<<7
        };
      int sync_flag = 0;
      unsigned char buf[2];
      int ret = 0;
    int find_sync_bytes = 0;
      while (1){
        if(find_sync_bytes>=in_buf_size)
        {
            sync_bytes = find_sync_bytes;
            return -1;
        }
        while (sync_flag == 0)
        {

             ret = FLAC_get_more_data(gb, 16);
             if (ret == 0)
             {
                   return  -1;
             }

            buf[0] = get_bits(gb, 8);
            find_sync_bytes++;

            if (buf[0] == 0xff)
            {
                 buf[1] = get_bits(gb, 8);
                 find_sync_bytes++;
                 if (buf[1]== sync_code)
                 {
                      sync_flag = 1;
                 }
            }

            if (0 == g_flac_decoder_info.max_framesize)
            {
                 g_flac_decoder_info.max_framesize = 4608;
            }
//
            if (find_sync_bytes > g_flac_decoder_info.max_framesize)
            {
          sync_bytes = find_sync_bytes;
                  return  -1;//返回一帧帧长
            }
        }
        //Hifi_Flac_Printf("find_sync_bytes =%d\n",find_sync_bytes);
        FLAC_get_more_data(gb, 64);
        find_sync_bytes +=  8;
        /* block size and sample rate codes */
        bs_code = get_bits(gb, 4);//块大小
        sr_code = get_bits(gb, 4);

        /* channels and decorrelation */
        fi->ch_mode = get_bits(gb, 4);
        if (fi->ch_mode < 2) {
            fi->channels = fi->ch_mode + 1;
            fi->ch_mode = FLAC_CHMODE_INDEPENDENT;
        }
        else if (fi->ch_mode <= 2+ FLAC_CHMODE_MID_SIDE) {
            fi->channels = 2;
        }
        else {
            sync_flag = 0;
            continue;
            //return -1;
        }
     //   Hifi_Flac_Printf("text 408 \n");
        /* bits per sample */
        bps_code = get_bits(gb, 3);
       // Hifi_Flac_Printf("text 411 \n");
        if (bps_code == 3 || bps_code == 7) {
        //  printf("invalid sample size code (%d)\n", bps_code);
            sync_flag = 0;
            continue;
            //return -1;
        }
        fi->bps = sample_size_table[bps_code];

    if(fi->bps != g_flac_decoder_info.bits_per_sample)
    {
       sync_flag = 0;
             continue;
    }
        if (get_bits1(gb))
        {
             sync_flag = 0;
             continue;
        }
        fi->frame_or_sample_num = get_utf8(gb);
        if (fi->frame_or_sample_num < 0)
        {
             sync_flag = 0;
             continue;
        }

        /* blocksize */
        if (bs_code == 0)
        {
            sync_flag = 0;
            continue;
            //return -1;
        }
        else if (bs_code == 6) {
            fi->blocksize = get_bits(gb, 8) + 1;
            find_sync_bytes ++;
        }
        else if (bs_code == 7) {
            fi->blocksize = get_bits(gb, 16) + 1;
            find_sync_bytes +=  2;
        }
        else {
            fi->blocksize = ff_flac_blocksize_table[bs_code];
        }
        if (sr_code < 12) {
            fi->samplerate = ff_flac_sample_rate_table[sr_code];
        }
        else if (sr_code == 12) {
            fi->samplerate = get_bits(gb, 8) * 1000;
            find_sync_bytes ++;
        }
        else if (sr_code == 13) {
            fi->samplerate = get_bits(gb, 16);
            find_sync_bytes +=  2;
        }
        else if (sr_code == 14) {
            fi->samplerate = get_bits(gb, 16) * 10;
            find_sync_bytes +=  2;
        }
        else {
            sync_flag = 0;
            continue;
            //return -1;
        }
  //       Hifi_Flac_Printf("find_sync_bytes = %ld\n",find_sync_bytes);
    sync_bytes = find_sync_bytes;
        return 0;
    }
    //
}
#if 1
long long CheckValidHeader_FlacSeek(GetBitContext *gb)
{
    //GetBitContext *gb = &s->gb;
      FLACFrameInfo fi;
      unsigned char *buf =inbuf;
      int buf_size = in_buf_size;
        init_get_bits(gb, buf, (buf_size)* 8);
        if (frame_header(gb, &fi) < 0)
        {
            return -1;
        }
        else
        {
            return fi.frame_or_sample_num;
        }
}

//extern FILE *g_hFlacFile;
//extern int sync_bytes;
int flac_abs(int a)
{
    return a >= 0 ? a : (0-a);
}
int flac_seek(int mseconds)
{

    unsigned long current_sample,target_sample = 0,current_pos =0,last_pos= 0,FFWOffset = 0;//mseconds * g_flac_decoder_info.sample_rate/1000;//目标采样                //当前位置
    unsigned long FileSize = RKFIO_FLength(pRawFileCache);                         //文件总长度
    int  approx_bytes_per_frame = 0;
    int blocksize = g_flac_decoder_info.blocksize;
    int the_frame_num = -1,the_last_frame = -1;
   // int the_total_frame = (int)(g_flac_decoder_info.total_samples / g_flac_decoder_info.blocksize);
    unsigned char *buf = inbuf;
    int buf_size = in_buf_size ;
    int delta = 0,delta0 = 0,ffw_flag = 0;
    GetBitContext *gb;
    unsigned long seconds = mseconds/1000,ms = mseconds % 1000;

    init_get_bits(gb, buf, (buf_size)* 8);

    target_sample = seconds * g_flac_decoder_info.sample_rate + ms * g_flac_decoder_info.sample_rate/1000;
    if(g_flac_decoder_info.max_framesize > 0)
    {
        approx_bytes_per_frame =  (g_flac_decoder_info.max_framesize + g_flac_decoder_info.min_framesize) / 2+1;
    }

    //查找当前位置采样点

    current_pos = RKFIO_FTell(g_hFlacFile);
    RKFIO_FRead(buf , buf_size , g_hFlacFile);
    while (1)
    {
        the_frame_num = (int)CheckValidHeader_FlacSeek(gb);
       // flac_DEBUG("the_frame_num= %d\n",the_frame_num);
       if (the_frame_num >= 0)
        {
            current_sample = blocksize * the_frame_num;
            current_pos = current_pos + sync_bytes;
            break;
         }
       else
        {
            RKFIO_FRead(buf , buf_size , g_hFlacFile );
            current_pos += buf_size;
        }
    }
     FFWOffset = current_pos;
     delta = (flac_abs(target_sample - current_sample)) / blocksize;
     the_last_frame = the_frame_num;
     //flac_DEBUG("delta = %d\n",delta);
    /* if(target_sample > current_sample)//判定快进还是快退
        ffw_flag = 1;*/
    //校正
    while (1)
    {
        delta0 = (flac_abs(target_sample - current_sample)) / blocksize;
        // flac_DEBUG("delta0 = %d\n",delta0);
        if((0 <= (delta0 - delta))&&((delta0 - delta) <= 10) && (approx_bytes_per_frame > g_flac_decoder_info.min_framesize))
        {
            approx_bytes_per_frame = approx_bytes_per_frame / 2;
            delta = delta0;
            the_last_frame = the_frame_num;
        }
        else if(delta0 < delta)
        {
           delta = delta0;
           the_last_frame = the_frame_num;
        }
        else
        {
           current_pos = (unsigned long)(last_pos + sync_bytes);
           current_sample =(unsigned long)( blocksize * the_last_frame);
        }
        if(0<=delta && delta <= 3)
        //if(0 <= real_delta && real_delta <= 5)
        {
            break;
        }
        else if (target_sample > current_sample)//快进
        {
            FFWOffset = (unsigned long)(current_pos + (delta * approx_bytes_per_frame));
            if(FFWOffset >= FileSize)
            {
                flac_DEBUG("FFWOffset = %lu\n",FFWOffset);
                FFWOffset = FileSize;
                break;

            }

        }

        else //快退
        {

            FFWOffset = (unsigned long)(current_pos -(delta * approx_bytes_per_frame) - buf_size);
        }
        RKFIO_FSeek(FFWOffset, SEEK_SET , g_hFlacFile);
        last_pos = current_pos;
        current_pos = FFWOffset;
        RKFIO_FRead(buf , buf_size ,  g_hFlacFile);
        the_frame_num = CheckValidHeader_FlacSeek(gb);
        current_sample =(unsigned long)( blocksize * the_frame_num);

    }

    if(FFWOffset > FileSize )
    {

        FFWOffset = FileSize;
    }

    RKFIO_FSeek((FFWOffset + sync_bytes -2), SEEK_SET , g_hFlacFile);
    FFWSampleNumber = current_sample ;
    g_CurrFileOffset = (FFWOffset  + sync_bytes - 2);
    g_SeekFileOffset = (FFWOffset  + sync_bytes - 2);
    g_f_FFW_FFD = true;
    isPostHalfFrame  = 0;
    isRightAfterSeek = 1;

flac_seek_ok:
   // flac_DEBUG("#######_seek_ok_#########");
    g_flac_decoder_info.samples_decoded = current_sample;
    g_flac_decoder_info.curr_play_time  = (unsigned long)((unsigned long long)(g_flac_decoder_info.samples_decoded) * 1000 / g_flac_decoder_info.sample_rate);
    //flac_DEBUG("accurry_play_time = %d\n",(g_flac_decoder_info.curr_play_time));
   // flac_DEBUG("#######_seek_ok_#########");
    return 0;
flac_seek_fail:
    flac_DEBUG("#######_seek_fail_#########");
    return -1;
}

#endif
#if 0
_ATTR_FLACDEC_TEXT_
unsigned char CheckValidHeader_FlacSeek(unsigned char *header)
{
    unsigned char buf_data[8];
    unsigned char bs_code, sr_code, bps_code,ch_mode;
    unsigned char sz_table[] = { 0, 8, 12, 0, 16, 20, 24, 0 };
    unsigned int src_table[] = {0, 88200, 176400, 192000, 8000, 16000, 22050, 24000, 32000, 44100, 48000, 96000};
    unsigned int src_temp;
    long long int val;

    buf_data[0] = *header++;

    if (buf_data[0] == 0xff)
    {
        buf_data[1] =  *header++;

        //if((buf_data[1] & 0xfe) == sync_code)        //sync_code = 0xfff8 ? 0xfff9
        if(buf_data[1] == sync_code)
        {
            buf_data[2] =  *header++;
            buf_data[3] =  *header++;
            buf_data[4] =  *header++;
            //bb_printf1("sync code = 0x%02x sync = 0x%02x, %02x, %02x, %02x",sync_code, buf_data[0], buf_data[1], buf_data[2], buf_data[3]);

            #if 1
            bs_code = buf_data[2] >> 4;     //samples
            sr_code = buf_data[2] & 0xf;    //sample rate
            if (sr_code == 0xf)
            {
                // invalid, to prevent sync-fooling string of 1s
                //Hifi_Flac_Printf("sync: sample rate error!");
                return 0;
            }
            else if ((0x00 < sr_code) && (sr_code < 0x0c))
            {
                src_temp = src_table[sr_code];
                if(src_temp != g_flac_decoder_info.sample_rate)
                {
                    //error
                    //bb_printf1("sync: error! s_FLAC_INFO.samplerate= %d, src_temp= %d", g_flac_decoder_info.sample_rate, src_temp);
                    return 0;
                }
            }
            else
            {
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
            if((buf_data[3] & 0x1))         //0 : mandatory value    1 : reserved for future use
            {
                //Hifi_Flac_Printf("sync: mandatory value error!");
                return 0;
            }
            #endif

            #if 1
            if(ch_mode > 12)
            {
                return 0;
            }
            #endif

            #if 1
            if (bps_code != 0)
            {
                if((bps_code == 3) || (bps_code == 7))  // reserved
                {
                    //Hifi_Flac_Printf("sync: bits per sampl error! bps_code = %d", bps_code);
                    return 0;
                }
                else          //get from STREAMINFO metadata block
                {
                    bps_code = sz_table[bps_code];
                    if(bps_code != g_flac_decoder_info.bits_per_sample)
                    {
                        //bb_printf1("sync: error! s_FLAC_INFO.bps= %d, bps_code= %d", g_flac_decoder_info.bits_per_sample, bps_code);
                        return 0;
                    }
                }
            }
            #endif

             GET_UTF8(val, buf_data[4], return -1;)
             Hifi_Flac_Printf("val = %ld\n ",val);
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

    if (FFWOffset > FileSize)
    {
        goto flac_seek_fail;
    }

    if (RKFIO_FSeek(FFWOffset, SEEK_SET , g_hFlacFile) != 0)
    {
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

         while(1)
         {
             ret = CheckValidHeader_FlacSeek(&buf[pos]);
             Hifi_Flac_Printf("val = %ld\n ",ret);
             if(ret)
             {
                break;
             }

             pos+=1;

             if(pos >= in_buf_size)
             {
                 size_t cbRead;
                 buf = inbuf;
                 pos = 0;
                 cbRead = RKFIO_FRead(buf , in_buf_size ,  g_hFlacFile );
                 if ((cbRead != in_buf_size) || (seekfix_counter++ >= 2048))        //读取32m BYTE
                 {
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

    if (RKFIO_FSeek(FFWOffset, SEEK_SET , g_hFlacFile) != 0)
    {
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


    if (FFWOffset > FileSize)
    {
        goto flac_seek_fail;
    }


    realFFWOffset = 0;
#if 0
    if (g_num_of_seek_table_points != 0)
    {
        for (i = 0; i < g_num_of_seek_table_points - 1; i++)
        {
            unsigned long first_point, second_point;

            first_point = g_seek_table_points[i].stream_offset + g_flac_decoder_info.audio_data_offset;
            second_point = g_seek_table_points[i+1].stream_offset + g_flac_decoder_info.audio_data_offset;
            if ((FFWOffset > first_point) && (FFWOffset < second_point))
            {
                realFFWOffset = (FFWOffset - first_point) > (second_point - FFWOffset) ? first_point : second_point;
                g_cur_used_seek_table_points = i;
                break;
            }
        }

        if (FFWOffset < g_seek_table_points[0].stream_offset + g_flac_decoder_info.audio_data_offset)
        {
            realFFWOffset = g_flac_decoder_info.audio_data_offset;
            g_cur_used_seek_table_points = 0;
        }

        if (FFWOffset > g_seek_table_points[g_num_of_seek_table_points-1].stream_offset + g_flac_decoder_info.audio_data_offset)
        {
            realFFWOffset = g_seek_table_points[g_num_of_seek_table_points-1].stream_offset + g_flac_decoder_info.audio_data_offset;
            g_cur_used_seek_table_points = g_num_of_seek_table_points - 1;
        }
    }
    else
#endif
    {
        //realFFWOffset = (g_flac_decoder_info.bits_pe了、
        // 根据样点数来seek
        for (i = 0; i < g_num_of_seek_table_points - 1; i++)
        {
            unsigned long first_point, second_point;

            first_point = g_seek_table_points[i].sample_number;
            second_point = g_seek_table_points[i+1].sample_number;
            if ((FFWSampleNumber > first_point) && (FFWSampleNumber < second_point))
            {
                realFFWOffset = g_seek_table_points[i].stream_offset + g_flac_decoder_info.audio_data_offset;
                g_cur_used_seek_table_points = i;
                break;
            }
*/
    }

    if (RKFIO_FSeek(realFFWOffset, SEEK_SET , g_hFlacFile) != 0)
    {
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
    g_flac_decoder_info.curr_play_time = (int)((long long)g_flac_decoder_info.samples_decoded *1000/ g_flac_decoder_info.sample_rate);
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
