#include "SysInclude.h"
#include "audio_main.h"
#include "audio_file_access.h"

#ifdef A_CORE_DECODE
#ifdef HIFI_FlAC_DECODE

#include <stdio.h>
#pragma arm section code = "FlacHDecCode", rodata = "FlacHDecCode", rwdata = "FlacHDecData", zidata = "FlacHDecBss"

#include "Hifi_flac_dec.h"
#include "hifi.h"
#include "Hw_hifi.h"
#include "flacdec.h"
#include "../hifi_get_bits.h"
#include "codec.h"


unsigned char inbuf[in_buf_size + 8];
//unsigned char outbuf[out_buf_size];


AVPacket flac_avpkt;
FLACContext s_flac_str;
extern FILE *flac_file_handle;
extern FLACStreaminfo s_FLAC_INFO;
extern int ID3_len;
int metsize = 0;
int g_hf_FFW_FFD = 0;
int FLAC_decode_init()
{
    {
        int buf_size;
        unsigned char *buf ;
        s_FLAC_INFO.samples_decoded = 0;
        RKFIO_FRead(inbuf, in_buf_size, flac_file_handle);
        flac_avpkt.data = inbuf;
        flac_avpkt.size = in_buf_size;
        buf = flac_avpkt.data;
        buf_size = flac_avpkt.size;
#ifdef HIFI_ACC
        Hifi_Set_ACC_XFER_Disable(0, 0, HIfi_ACC_TYPE_FLAC); //开始传输配置数据和初始化系数(不往fifo送)
        Hifi_Set_ACC_clear(0);//fpga 内部已实现
        Hifi_Set_ACC_Dmacr(0);
        Hifi_Set_ACC_Intcr(0);
#endif

        /* check for  header */
        /* check that there is at least the smallest decodable amount of data.
        this amount corresponds to the smallest valid FLAC frame possible.
        FF F8 69 02 00 00 9A 00 00 34 46 */
        if (AV_RB32(buf) == MKBETAG('f', 'L', 'a', 'C'))
        {
            parse_streaminfo(&s_flac_str, buf, buf_size);
            metsize = get_metadata_size(buf, buf_size);
            // Hifi_Flac_Printf("metadata_size =%d \n",metsize);
        }

        if (s_FLAC_INFO.channels > 2)
        {
            Hifi_Flac_Printf("channels %d > 2.Unsupport CHN\n", s_FLAC_INFO.channels);
            return -1;
        }

        if (s_FLAC_INFO.max_blocksize > FLAC_MAX_BLOCKSIZE )
        {
            Hifi_Flac_Printf("blocksize %d > %d\n", s_FLAC_INFO.max_blocksize, FLAC_MAX_BLOCKSIZE);
            return -1;
        }

        s_FLAC_INFO.bitrate = (((long long )RKFIO_FLength(flac_file_handle) - metsize - ID3_len) * 1000 ) / (((long long)s_FLAC_INFO.samples * 1000 / s_FLAC_INFO.samplerate) >> 3);

        RKFIO_FSeek(metsize + ID3_len, SEEK_SET, flac_file_handle);
        RKFIO_FRead(inbuf, in_buf_size, flac_file_handle);
        /* decode frame */
        init_get_bits(&s_flac_str.gb, buf, (buf_size) * 8);
    }
    return 0;
}

int FLAC_frame_decode(unsigned char*outbuf, int *out_size)
{
    int bytes_read;
    int i;

    {
        *out_size = out_buf_size;

        bytes_read = flac_decode_frame(outbuf, out_size, &flac_avpkt);

        if (bytes_read < 0)
        {
            return -1 ;
        }

        flac_avpkt.size -= bytes_read;
        flac_avpkt.data += bytes_read;

        if (flac_avpkt.size < in_buf_size)
        {
            if (flac_avpkt.size < 0)
            {
                flac_avpkt.size = 0;
            }

            MemMov2(inbuf, flac_avpkt.data, flac_avpkt.size);
            flac_avpkt.data = inbuf;

            bytes_read = RKFIO_FRead(flac_avpkt.data + flac_avpkt.size, in_buf_size - flac_avpkt.size, flac_file_handle);

            if (bytes_read > 0)
            {
                flac_avpkt.size += bytes_read;
            }

            return flac_avpkt.size ;
        }
    }

}
int Flac_deblock(char* dest, int *decoded[2], int channel_mode, int bps, int blocksize) //alac 由于可能存在额外数据，因此先不做处理
{
    int ch_mode = channel_mode;
    int left, right, a, b, i, j, is32;
    int sample_shift;
    int channels = s_FLAC_INFO.channels;
    char *samples_24 = (char *) dest;
    int *samples_32 = (int *)dest;
    short *samples_16 = (short *)dest;

    if (bps > 16)
    {
        sample_shift = 32 - bps;
        is32 = 1;
    }
    else
    {
        sample_shift = 16 - bps;
        is32 = 0;
    }

    switch (ch_mode)
    {
        case CODEC_CHMODE_INDEPENDENT:

            for (j = 0; j < blocksize; j++)
            {
                if (channels == 1)
                {
                    {
                        if (is32)
                        {
                            if (sample_shift) //24bit
                            {
                                *(int *)samples_24 = decoded[0][j];
                                samples_24 += 3;
                                *(int *)samples_24 = decoded[0][j];
                                samples_24 += 3;
                            }
                            else//32bit
                            {
                                *samples_32++ = decoded[0][j];
                                *samples_32++ = decoded[0][j];
                            }
                        }
                        else //16bit
                        {
                            *samples_16++ = decoded[0][j] ;
                            *samples_16++ = decoded[0][j] ;

                        }
                    }
                }
                else
                {
                    for (i = 0; i < channels; i++)
                    {
                        if (is32)
                        {
                            if (sample_shift) //24bit
                            {
                                *(int *)samples_24 = decoded[i][j];
                                samples_24 += 3;
                            }
                            else//32bit
                            {
                                *samples_32++ = decoded[i][j];
                            }
                        }
                        else //16bit
                        {
                            *samples_16++ = decoded[i][j] ;
                        }
                    }
                }
            }

            break;

        case CODEC_CHMODE_LEFT_SIDE:


            for (i = 0; i < blocksize; i++)
            {
                a = decoded[0][i];
                b = decoded[1][i];
                left = a;
                right = a - b;

                if (is32)
                {
                    if (sample_shift) //24bit
                    {
                        *(int *)samples_24 = left;
                        samples_24 += 3;
                        *(int *)samples_24 = right;
                        samples_24 += 3;
                    }
                    else//32bit
                    {
                        *samples_32++ = left;
                        *samples_32++ = right;
                    }
                }
                else //16bit
                {
                    *samples_16++ = left;
                    *samples_16++ = right;
                }
            }

            break;

        case CODEC_CHMODE_RIGHT_SIDE:

            for (i = 0; i < blocksize; i++)
            {
                a = decoded[0][i];
                b = decoded[1][i];
                left = a + b;
                right = b;

                if (is32)
                {
                    if (sample_shift) //24bit
                    {
                        *(int *)samples_24 = left;
                        samples_24 += 3;
                        *(int *)samples_24 = right;
                        samples_24 += 3;
                    }
                    else//32bit
                    {
                        *samples_32++ = left;
                        *samples_32++ = right;
                    }

                }
                else //16bit
                {
                    *samples_16++ = left;
                    *samples_16++ = right;
                }
            }

            break;

        case CODEC_CHMODE_MID_SIDE:

            for (i = 0; i < blocksize; i++)
            {
                a = decoded[0][i];
                b = decoded[1][i];
                right = a - (b >> 1);
                left = right + b;

                if (is32)
                {
                    if (sample_shift) //24bit
                    {
                        *(int *)samples_24 = left;
                        samples_24 += 3;
                        *(int *)samples_24 = right;
                        samples_24 += 3;
                    }
                    else//32bit
                    {
                        *samples_32++ = left;
                        *samples_32++ = right;
                    }

                }
                else //16bit
                {
                    *samples_16++ = left;
                    *samples_16++ = right;
                }
            }

            break;

    }

    if (is32)
    {
        if (sample_shift) //24bit
            return blocksize * channels * 3;
        else
            return blocksize * channels * 4;
    }
    else
    {
        //  fwrite(dest,4,blocksize,outfile);
        return blocksize * channels * 2;
    }

}



/*****************************************************
buffer_in    熵解码后的残差信号
buffer_out   经过线性预测后的信号
lpc系数用倒序,这样数据源就不需倒序乘
*********************************************************/
void  Flac_Decode_Lpc( int32_t *buffer_in, int32_t *buffer_out,
                       int nb_samples, int bps, int32_t *lpc_coefs, int16_t *adapt_coefs,
                       int lpc_order, int lpc_quant, int16_t *delay, int* extra, int version) //int extra for ape = f->avg
{

//if          adapt_coefs != NULL    ---->APE (ape的delay 可以使用buffer_in，但是注意数据类型)
//else if     buffer_in   == NULL    ---->FLAC
//else                               ---->ALAC

    long long  sum;
    int i, j;

    {
        //线性预测，不更新系数,同一般的fir 滤波器相同       系数 32bit
        int *history ;
        int *decoded = buffer_out;

        for (i = lpc_order; i < nb_samples; i++)
        {
            sum = 0;

            history = &decoded[i - lpc_order];

            for (j = 0; j < lpc_order; j++)
                sum += (long long )lpc_coefs[j] * (*(history++));

            decoded[i] += sum >> lpc_quant;
        }

    }

}

extern unsigned char sync_code;
extern  int sample_size_table[];
extern int ff_flac_sample_rate_table[16];
extern int ff_flac_blocksize_table[16];
int sync_bytes = 0;
int frame_header(GetBitContext *gb, FLACFrameInfo *fi )//查找帧头
{
    int bs_code, sr_code, bps_code, crc8;
    int sync_flag = 0;
    unsigned char buf[2];
    int ret = 0;
    int find_sync_bytes = 0;

    while (1)
    {
        if (find_sync_bytes >= in_buf_size)
        {
            sync_bytes = find_sync_bytes;
            return -1;
        }

        while (sync_flag == 0)
        {
            ret = FLAC_get_more_data(gb, 16);

            if (ret == 0)
            {
                sync_bytes = find_sync_bytes;
                return  -1;
            }

            buf[0] = get_bits(gb, 8);
            find_sync_bytes++;

            if (buf[0] == 0xff)
            {
                buf[1] = get_bits(gb, 8);
                find_sync_bytes++;

                if (buf[1] == sync_code)
                {
                    sync_flag = 1;
                }
            }

            if (0 == s_FLAC_INFO.max_framesize)
            {
                s_FLAC_INFO.max_framesize = 4608;
            }

            if (find_sync_bytes > s_FLAC_INFO.max_framesize)
            {
                sync_bytes = find_sync_bytes;
                Hifi_Flac_Printf("find_sync_bytes out =%d\n", find_sync_bytes);
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

        if (fi->ch_mode < FLAC_MAX_CHANNELS)
        {
            fi->channels = fi->ch_mode + 1;
            fi->ch_mode = FLAC_CHMODE_INDEPENDENT;
        }
        else if (fi->ch_mode <= FLAC_MAX_CHANNELS + FLAC_CHMODE_MID_SIDE)
        {
            fi->channels = 2;
        }
        else
        {

            //  printf("invalid channel mode: %d\n", fi->ch_mode);
            //  printf("invalid channel mode: %d\n", count);
            sync_flag = 0;
            continue;
            //return -1;
        }

        //   Hifi_Flac_Printf("text 408 \n");
        /* bits per sample */
        bps_code = get_bits(gb, 3);

        // Hifi_Flac_Printf("text 411 \n");
        if (bps_code == 3 || bps_code == 7)
        {
            //  printf("invalid sample size code (%d)\n", bps_code);
            sync_flag = 0;
            continue;
            //return -1;
        }

        fi->bps = sample_size_table[bps_code];

        //  fi->frame_or_sample_num = get_utf8(gb);
        //  printf("pause from frame head bps = %d\n",fi->bps);
        /* reserved bit */
        //  get_bits1(gb);
        if (get_bits1(gb))
        {
            sync_flag = 0;
            continue;
            //  return -1;
        }

        //  Hifi_Flac_Printf("text 428 \n");
        /* sample or frame count */
        fi->frame_or_sample_num = get_utf8(gb);

        // Hifi_Flac_Printf("text 431 \n");
        if (fi->frame_or_sample_num < 0)
        {
            sync_flag = 0;
            continue;
            //return -1;
        }

        /* blocksize */
        if (bs_code == 0)
        {
            sync_flag = 0;
            continue;
            //return -1;
        }
        else if (bs_code == 6)
        {
            fi->blocksize = get_bits(gb, 8) + 1;
            find_sync_bytes ++;
        }
        else if (bs_code == 7)
        {
            fi->blocksize = get_bits(gb, 16) + 1;
            find_sync_bytes +=  2;
        }
        else
        {
            fi->blocksize = ff_flac_blocksize_table[bs_code];
        }

//        Hifi_Flac_Printf("text 459 \n");
        /* sample rate */
        if (sr_code < 12)
        {
            fi->samplerate = ff_flac_sample_rate_table[sr_code];
        }
        else if (sr_code == 12)
        {
            fi->samplerate = get_bits(gb, 8) * 1000;
            find_sync_bytes ++;
        }
        else if (sr_code == 13)
        {
            fi->samplerate = get_bits(gb, 16);
            find_sync_bytes +=  2;
        }
        else if (sr_code == 14)
        {
            fi->samplerate = get_bits(gb, 16) * 10;
            find_sync_bytes +=  2;
        }
        else
        {
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
int CheckValidHeader_FlacSeek(GetBitContext *gb)
{
    //GetBitContext *gb = &s->gb;
    FLACFrameInfo fi;
    unsigned char *buf = inbuf;
    int buf_size = in_buf_size;
    init_get_bits(gb, buf, (buf_size) * 8);

    if (frame_header(gb, &fi) < 0)
    {
        return -1;
    }
    else
    {
        return fi.frame_or_sample_num;
    }
}

#endif
#if 1
int hifi_flac_seek(int seconds)
{
    unsigned long target_sample = seconds * s_FLAC_INFO.samplerate;//目标采样
    unsigned long current_pos = 0, last_pos;                  //当前位置
    unsigned long FileSize = RKFIO_FLength(flac_file_handle);                         //文件总长度
    unsigned long FFWOffset = 0;
    int blocksize = s_FLAC_INFO.max_blocksize;
    int approx_bytes_per_frame = 0;
//  FLACContext *s = &s_flac_str;
    int the_frame_num = 0, last_frame = -1;
    long long current_sample;
    unsigned char *buf = inbuf;
    int buf_size = in_buf_size ;
    int delta = 0, delta0 = 0;
//  GetBitContext *gb = &s->gb;
    GetBitContext *gb;

    Hifi_Flac_Printf("FileSize = %lu\n", FileSize);

    if(s_FLAC_INFO.max_framesize == 0)
        approx_bytes_per_frame = 4608 ;
    else if(s_FLAC_INFO.max_framesize > 0)
        approx_bytes_per_frame = (s_FLAC_INFO.max_framesize + s_FLAC_INFO.min_framesize) / 2  + 1;
    {
        FFWOffset = seconds * s_FLAC_INFO.samplerate / s_FLAC_INFO.max_blocksize * approx_bytes_per_frame + metsize + ID3_len; //初始估计位置

        if (RKFIO_FSeek(FFWOffset, SEEK_SET , flac_file_handle) != 0)
        {
            goto flac_seek_fail;
        }
        else
        {
            current_pos = RKFIO_FTell(flac_file_handle);
        }

        RKFIO_FRead(buf , in_buf_size ,  flac_file_handle );
        init_get_bits(gb, buf, (buf_size) * 8);
        the_frame_num = CheckValidHeader_FlacSeek(gb);

        if (the_frame_num >= 0)
        {
            current_sample = blocksize * the_frame_num;
        }
        else
        {
            RKFIO_FRead(buf , in_buf_size ,  flac_file_handle );
            current_pos += buf_size;
        }

        //  approx_bytes_per_frame = approx_bytes_per_frame/2;//steplen
    }

    delta = (abs(target_sample - current_sample)) / blocksize;

    //校正
    while (1)
    {
        //Hifi_Flac_Printf("the_frame_num = %d\n",the_frame_num );
        delta0 = (abs(target_sample - current_sample)) / blocksize;
        //Hifi_Flac_Printf("delta = %d\n",delta);
        // Hifi_Flac_Printf("delta0 = %d\n",delta0);

        if ((0 <= (delta0 - delta)) && ((delta0 - delta) <= 35) && (approx_bytes_per_frame > s_FLAC_INFO.min_framesize))
        {
            approx_bytes_per_frame = approx_bytes_per_frame / 2 + 1;
            delta = delta0;
            last_frame = the_frame_num;
        }
        else if (delta0 < delta)
        {
            delta = delta0;
            last_frame = the_frame_num;
            //last_pos = current_pos;
        }
        else
        {
            the_frame_num = last_frame;
            current_sample = last_frame * blocksize;
            current_pos = last_pos + sync_bytes + 2;
        }


        // Hifi_Flac_Printf("approx_bytes_per_frame = %d\n",approx_bytes_per_frame);

        if (0 <= delta && delta <= 12 ) //误差为12帧
        {
            goto flac_seek_ok;
        }
        else if (target_sample > current_sample)//向前查找
        {

            // Hifi_Flac_Printf("#########FFD#########");
            FFWOffset = (unsigned long)(current_pos + (delta * approx_bytes_per_frame));

            // Hifi_Flac_Printf("FFWOffset = %ld\n",FFWOffset);
            if (RKFIO_FSeek(FFWOffset, SEEK_SET , flac_file_handle) != 0)
            {
                Hifi_Flac_Printf("seek fail\n");
                goto flac_seek_fail;
            }
            else
            {
                last_pos = current_pos;
                current_pos = FFWOffset;
            }

            RKFIO_FRead(buf , in_buf_size ,  flac_file_handle) ;
            /* if( RKFIO_FRead(buf , in_buf_size ,  flac_file_handle) != buf_size)
             {
                  Hifi_Flac_Printf("read data err\n");
                 goto flac_seek_fail;
             }*/
            the_frame_num = CheckValidHeader_FlacSeek(gb);
            current_sample = blocksize * the_frame_num;
        }
        else if (target_sample < current_sample) //向后查找
        {
            FFWOffset = (long long)(current_pos - (delta * approx_bytes_per_frame));

            if (RKFIO_FSeek(FFWOffset, SEEK_SET , flac_file_handle) != 0)
            {
                Hifi_Flac_Printf("seek fail\n");
                goto flac_seek_fail;
            }
            else
            {
                last_pos = current_pos;
                current_pos = FFWOffset;
            }

            RKFIO_FRead(buf , in_buf_size ,  flac_file_handle);
            /* if( RKFIO_FRead(buf , in_buf_size ,  flac_file_handle) != buf_size)
                 {
                     Hifi_Flac_Printf("read data err\n");
                     goto flac_seek_fail;
                     }*/
            the_frame_num = CheckValidHeader_FlacSeek(gb);
            current_sample = blocksize * the_frame_num;
        }
        else
        {
            goto flac_seek_fail;
        }

        //  Hifi_Flac_Printf("the_frame_num = %ld\n",the_frame_num);
    }

    if (FFWOffset > FileSize)
    {
        Hifi_Flac_Printf("FileSize = %lu\n", FileSize);
        Hifi_Flac_Printf("FFWOffset > FileSize\n");
        goto flac_seek_fail;
    }

flac_seek_ok:
    RKFIO_FSeek((FFWOffset + sync_bytes - 10), SEEK_SET , flac_file_handle);
    RKFIO_FRead(buf , in_buf_size ,  flac_file_handle );
    flac_avpkt.data = inbuf;
    flac_avpkt.size = in_buf_size;
    s_FLAC_INFO.samples_decoded = current_sample;
    return 0;
flac_seek_fail:
    Hifi_Flac_Printf("#######_seek_fail_#########");
    return -1;
}
#pragma arm section code
#endif
#endif
#endif
//extern unsigned char sync_code;
/*unsigned char CheckValidHeader_FlacSeek(unsigned char *header)
{
    unsigned char buf_data[8];
    unsigned char bs_code, sr_code, bps_code,ch_mode;
    unsigned char sz_table[] = { 0, 8, 12, 0, 16, 20, 24, 0 };
    unsigned int src_table[] = {0, 88200, 176400, 192000, 8000, 16000, 22050, 24000, 32000, 44100, 48000, 96000};
    unsigned int src_temp;

    buf_data[0] = *header++;

    if (buf_data[0] == 0xff)
    {
        buf_data[1] =  *header++;

        if((buf_data[1]) == sync_code)        //sync_code = 0xfff8 ? 0xfff9
        {
            buf_data[2] =  *header++;
            buf_data[3] =  *header++;

            //Hifi_Flac_Printf("sync code = 0x%02x sync = 0x%02x, %02x, %02x, %02x",sync_code, buf_data[0], buf_data[1], buf_data[2], buf_data[3]);

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
                if(src_temp != s_FLAC_INFO.samplerate)
                {
                    //error
                    //Hifi_Flac_Printf("sync: error! s_FLAC_INFO.samplerate= %d, src_temp= %d", s_FLAC_INFO.samplerate, src_temp);
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

           // ch_mode  = (buf_data[3] >> 4) + 1;
             ch_mode  = (buf_data[3] >> 4) ;
            bps_code = (buf_data[3] >> 1) & 0x7;
            if((buf_data[3] & 0x1))         //0 : mandatory value    1 : reserved for future use
            {
                //Hifi_Flac_Printf("sync: mandatory value error!");
                return 0;
            }

            //if(ch_mode > 10)

            if(ch_mode>12)
                return 0;
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
                    if(bps_code != s_FLAC_INFO.bps)
                    {
                        //Hifi_Flac_Printf("sync: error! s_FLAC_INFO.bps= %d, bps_code= %d", s_FLAC_INFO.bps, bps_code);
                        return 0;
                    }
                }
            }
            return 1;
        }
     }
     return 0;
}

int hifi_flac_seek(int seconds)
{
    unsigned long FFWOffset,FFWSampleNumber;
    int i;
    int max_FFW_FFD_time= 3;//seconds
    unsigned long max_FFW_FFD_offset;
    int FileSize = RKFIO_FLength(flac_file_handle);
    int cur_pos = RKFIO_FTell(flac_file_handle);
    int approx_bytes_per_frame = 0;
    Hifi_Flac_Printf("seconds = %d",seconds);
    FFWOffset = ((long long )seconds * s_FLAC_INFO.bitrate>>3) + metsize + ID3_len;
    FFWSampleNumber = (long long)seconds * s_FLAC_INFO.samplerate;
    approx_bytes_per_frame = (16+s_FLAC_INFO.max_framesize)/2+1 ;
    max_FFW_FFD_offset = (max_FFW_FFD_time*s_FLAC_INFO.samplerate/s_FLAC_INFO.max_blocksize)*approx_bytes_per_frame;
  //  FFWOffset = (((long long)seconds * s_FLAC_INFO.samplerate) / s_FLAC_INFO.max_blocksize)* approx_bytes_per_frame;
    Hifi_Flac_Printf("cur_pos =%d\n",cur_pos);
    Hifi_Flac_Printf("FFWOffset =%d\n",FFWOffset);
    if((FFWOffset > cur_pos) && (FFWOffset - cur_pos)> (max_FFW_FFD_offset))
    {
        FFWOffset = cur_pos+max_FFW_FFD_offset;
        Hifi_Flac_Printf("\n#######ffD#########\n");
      }
    if((FFWOffset < cur_pos) && (cur_pos-FFWOffset) > (max_FFW_FFD_offset))
    {
        FFWOffset = cur_pos-max_FFW_FFD_offset;
        Hifi_Flac_Printf("\########nffw######\n");
        }

    if (FFWOffset > FileSize)
    {
        goto flac_seek_fail;
    }

    if (RKFIO_FSeek(FFWOffset, SEEK_SET , flac_file_handle) != 0)
    {
        goto flac_seek_fail;
    }

    //Hifi_Flac_Printf("seek 0x%08x  metsize = %d sync_code = 0x%02x",FFWOffset,metsize,sync_code);
    {
        unsigned char *buf;
        int ret;
        int pos = 0;
        int sync_flag = 0;
        int seekfix_counter = 0;

        buf = inbuf;
        RKFIO_FRead(buf , in_buf_size ,  flac_file_handle );

        while(1)
        {
            ret = CheckValidHeader_FlacSeek(&buf[pos]);
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
                cbRead = RKFIO_FRead(buf , in_buf_size ,  flac_file_handle );
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

    if (RKFIO_FSeek(FFWOffset, SEEK_SET , flac_file_handle) != 0)
    {
        goto flac_seek_fail;
    }

flac_seek_ok:
    RKFIO_FRead(inbuf, in_buf_size, flac_file_handle);

    if (FFWSampleNumber < s_FLAC_INFO.samples_decoded)
    {
        g_hf_FFW_FFD = 1;
    }
    flac_avpkt.data = inbuf;
    flac_avpkt.size = in_buf_size;
    s_FLAC_INFO.samples_decoded = FFWSampleNumber;
    return 0;

flac_seek_fail:
    return -1;
}
#pragma arm section code
#endif
#endif
*/

