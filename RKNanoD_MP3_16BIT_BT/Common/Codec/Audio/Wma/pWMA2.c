/* Copyright (C) 2009 ROCK-CHIPS FUZHOU . All Rights Reserved. */
/*
File    : \Audio\pMP3.c
Desc    :

Author  :
Date    :
Notes   :

$Log    :
*
*
*/
/****************************************************************/

#include "../include/audio_main.h"
#include "../include/audio_globals.h"
#include "../include/audio_file_access.h"

#ifdef WMA_DEC_INCLUDE
#include "pWMA.h"
//#include <stdio.h>
//#include <string.h> //for memcpy(),memmove()
#include ".\wmaInclude\predefine.h"
#if 1//ndef WMAAPI_NO_DRM//USB ����ã��ʷ���ϵͳ��
const unsigned char g_pmid[] =
{
    0x32, 0x33, 0x44, 0x45, 0x38, 0x45, 0x32, 0x41, 0x39, 0x34,
    0x45, 0x44, 0x43, 0x43, 0x30, 0x46
};
#endif
#pragma arm section zidata = "WmaDecoderBss",rwdata = "WmaDecoderData"
//------------------global varibles----------------------------------
static WMA_DEC_INFO gWMADecInfo;

static unsigned int bytesCnt = MAX_SAMPLES_OF_ONE_CHANNEL * 2;
//unsigned int g_DecodedSamples;
//unsigned int g_cCountGetPCM;
static unsigned long g_TotalSamples;
static  unsigned int g_wmaBufIndex;

static long wma_frame_cnt = 0;

//extern tWMAFileContDesc *g_pdesc;
//extern tWMAFileHeader g_hdr;
short *gWmaOutputPtr[2];
extern  unsigned long SRC_Num_Forehead; //for src

//static FILE *g_fpWMA;
//-----------------------------------------------------------------
extern tWMAFileStatus wma_decoder_dec(WMA_DEC_INFO* dec_info);
extern tWMAFileStatus wma_decoder_init(WMA_DEC_INFO* dec_info);
extern tWMAFileStatus wma_decoder_seek(WMA_DEC_INFO* dec_info);

//*************************************************************************************************************//
//the achievement of functions.��
//SUBFN_CODEC_GETNAME  :   get decoder name
//SUBFN_CODEC_GETARTIST:   get artist name.
//SUBFN_CODEC_GETTITLE :   get song title.
//SUBFN_CODEC_GETBITRATE:  get bit rate.
//SUBFN_CODEC_GETSAMPLERATE: get sample rate.
//SUBFN_CODEC_GETCHANNELS: get channel number.
//SUBFN_CODEC_GETLENGTH :  get total play time [unit:ms]
//SUBFN_CODEC_GETTIME  :   get current play time.[unit:ms].note:this time get by timestamp,there may be error if file is been demage..
//SUBFN_CODEC_OPEN_DEC :   open deooder(initialization.)
//SUBFN_CODEC_DECODE   :   deocode.
//SUBFN_CODEC_ENCODE   :   not support.
//SUBFN_CODEC_SEEK     :   location by time directly.[unit:ms]
//SUBFN_CODEC_CLOSE    :   close decoder.
//SUBFN_CODEC_SETBUFFER:   set cache area,point out the position to put save result.
/******************************************************
Name:
Desc:
Param: ulIoctl child function number.
    ulParam1 child function parameter 1.
    ulParam2 child function parameter 2.
    ulParam3 child function parameter 3.
    ulParam4 child function parameter 4.

Return:
Global:
Note:
Author:
Log:
******************************************************/
#pragma arm section code = "WmaCommonCode"
unsigned long WMAFunction2(unsigned long ulIoctl, unsigned long ulParam1,
                           unsigned long ulParam2, unsigned long ulParam3)
{
    switch (ulIoctl)
    {
#if 1

        case SUBFN_CODEC_GETNAME:
        {
            char **ppcName;
            ppcName = (char **)ulParam1;
            *ppcName = "WMA Windows Media(tm) Audio";
            return 1;
        }

        case SUBFN_CODEC_GETARTIST:
        {
            //char **ppcName = (char **)ulParam1;
            if (gWMADecInfo.pdesc)
            {
                *((unsigned int*)ulParam1) = (unsigned int)(gWMADecInfo.pdesc->pAuthor);
                return 1;
            }

            return 0;
        }

        case SUBFN_CODEC_GETTITLE:
        {
            //char **ppcName;

            //ppcName = (char **)ulParam1;
            if (gWMADecInfo.pdesc)
            {
                *((unsigned int*)ulParam1) = (unsigned int)(gWMADecInfo.pdesc->pTitle);
                return 1;
            }

            return 0;
        }

        case SUBFN_CODEC_GETBITRATE:
        {
            //unsigned long *pulBitRate;
            //pulBitRate = ;
            *((unsigned long *)ulParam1) = (unsigned long)(gWMADecInfo.hdr->bitrate);
            return 1;
        }

        case SUBFN_CODEC_GETBPS:
        {
            *((unsigned long *)ulParam1) = (unsigned long)(gWMADecInfo.hdr->valid_bits_per_sample);
            return 1;
        }

        case SUBFN_CODEC_GETSAMPLERATE:
        {
            unsigned long *pulSampleRate;
            pulSampleRate = (unsigned long *)ulParam1;
            *pulSampleRate = (unsigned long)(gWMADecInfo.hdr->sample_rate);
            return 1;
        }

        case SUBFN_CODEC_GETCHANNELS:
        {
            unsigned long *pulChannels;
            pulChannels = (unsigned long *)ulParam1;
            *pulChannels = (unsigned long)(gWMADecInfo.hdr->num_channels);
            return 1;
        }

        case SUBFN_CODEC_GETLENGTH:
        {
            unsigned long *pulLength;
            pulLength = (unsigned long *)ulParam1;
            *pulLength = (unsigned long)(gWMADecInfo.hdr->duration);
            return 1;
        }

        case SUBFN_CODEC_GETTIME:
        {
            unsigned long *pulTime;
            unsigned int decode_samples = gWMADecInfo.decoded_samples;
            unsigned long sample_rate = gWMADecInfo.hdr->sample_rate;
            pulTime = (unsigned long *)ulParam1;
            //*pulTime = (decode_samples / sample_rate * 1000) + ((decode_samples % sample_rate) * 1000  / sample_rate);
            *pulTime = (unsigned long)(((unsigned long long)decode_samples / sample_rate * 1000)
                + (((unsigned long long)decode_samples % sample_rate) * 1000  / sample_rate));
            return 1;
        }

        case SUBFN_CODEC_OPEN_ENC:
        {
            return 1;
        }

#endif

        case SUBFN_CODEC_OPEN_DEC:
        {
#ifdef WMAINITIALIZE
            int rc;
            wma_frame_cnt = 0;
            gWMADecInfo.fhandle = (unsigned long)pRawFileCache;
            rc = wma_decoder_init(&gWMADecInfo);

//          if(gWMADecInfo.hdr->bits_per_sample == 16
//                && gWMADecInfo.hdr -> bitrate == 320000
//                && gWMADecInfo.hdr->sample_rate == 24000
//              && gWMADecInfo.hdr->duration == 189354
//              && gWMADecInfo.hdr->num_channels == 1
//              && gWMADecInfo.hdr->packet_size == 3200
//              && gWMADecInfo.hdr->first_packet_offset == 672
//              && gWMADecInfo.hdr->last_packet_offset == 14199072)
//          {
//              return 0;
//          }

            if (rc != cWMA_NoErr)
            {
                //DEBUG("WMA Decoder Init Failed!\n");
                //goto ErrorExit;
                return 0;
            }

            if ((gWMADecInfo.hdr->bitrate == 22000) && (gWMADecInfo.hdr->sample_rate == 32000) && (gWMADecInfo.hdr->num_channels == 2))//�˸�ʽ��֧��,ԭ���ĵ�
            {
                //DEBUG("Don't support format!\n");
                //goto ErrorExit;
                return 0;
            }

            if(gWMADecInfo.isHighRate == 1)
                //if (isWMAHighRate())
            {
                //DEBUG("wma is highrate!\n");
#ifndef WMAHIGHRATE
                //DEBUG("wma highrate declib is close!\n");
                //goto ErrorExit;
                return 0;
#endif
            }
            else
            {
                //DEBUG("wma is midrate or lowrate!\n");
#ifndef WMAMIDRATELOWRATE
                //DEBUG("wma midrate/lowrate declib is close!\n");
                //goto ErrorExit;
                return 0;
#endif
            }

            g_TotalSamples = (gWMADecInfo.hdr->duration / 1000) * gWMADecInfo.hdr->sample_rate + (gWMADecInfo.hdr->duration % 1000) * gWMADecInfo.hdr->sample_rate / 1000;
            g_wmaBufIndex = 0;
            gWMADecInfo.curPtr = (unsigned long)&gWmaOutputPtr[g_wmaBufIndex][SRC_Num_Forehead];
            memset((void*)gWMADecInfo.curPtr, 0, MAX_SAMPLES_OF_ONE_CHANNEL * sizeof(short));
#endif
            return 1;
        }

        case SUBFN_CODEC_GETBUFFER:
        {
            // return the buffer address
            //*(unsigned int *)ulParam1 = (unsigned int)(gWMADecInfo.curPtr);
            *(unsigned int *)ulParam1 = (unsigned int)(&gWmaOutputPtr[g_wmaBufIndex][SRC_Num_Forehead]);
            g_wmaBufIndex ^= 1;
            *(unsigned int *)ulParam2 = (bytesCnt >> 2);
            //(short*)ulParam1 = (uint32)(gWmaOutputPtr[g_wmaBufIndex]);
            return 1;
        }

        case SUBFN_CODEC_DECODE:
        {
#if defined(WMAHIGHRATE) || defined(WMAMIDRATELOWRATE)
            tWMAFileStatus rc;
            /* decode loop */
            // while(1)
            gWMADecInfo.curPtr = (unsigned long)&gWmaOutputPtr[g_wmaBufIndex][SRC_Num_Forehead];
            {
                rc = wma_decoder_dec(&gWMADecInfo);

                if (rc != cWMA_NoErr)
                {
                    memset((void*)gWMADecInfo.curPtr, 0, MAX_SAMPLES_OF_ONE_CHANNEL * sizeof(short));
                    wma_DEBUG("ret = %d Error", rc);
                    return 0;
                }
                else
                {
                    bytesCnt = gWMADecInfo.frameLen * 2 * gWMADecInfo.hdr->valid_bits_per_sample / 8;

                    if (wma_frame_cnt < 6) //(gWmaIsRightAfterSeek)
                    {
                    }
                    else
                    {
                        //g_wmaBufIndex ^= 1;
                    }

                    wma_frame_cnt ++;
                    return 1;
                }
            }
#endif
        }

        case SUBFN_CODEC_SEEK:
        {
#if defined(WMAHIGHRATE) || defined(WMAMIDRATELOWRATE)
            tWMAFileStatus rc;
            wma_frame_cnt = 0;
            //reset dma,synth,imdct
            /*
            *((volatile unsigned long *) 0x40010014) = 0x00000070;
            *((volatile unsigned long *) 0x40010014) = 0x00000000;
            */
            gWMADecInfo.curPtr = (unsigned long)&gWmaOutputPtr[g_wmaBufIndex ^ 1][SRC_Num_Forehead];
            gWMADecInfo.msSeekTo = ulParam1;
            rc = wma_decoder_seek(&gWMADecInfo);
#endif
            memset(&gWmaOutputPtr[0][SRC_Num_Forehead], 0, (MAX_SAMPLES_OF_ONE_CHANNEL)*sizeof(short));
            memset(&gWmaOutputPtr[1][SRC_Num_Forehead], 0, (MAX_SAMPLES_OF_ONE_CHANNEL)*sizeof(short));

            if (rc != cWMA_NoErr)
                return 0;

            return 1;
        }

        case SUBFN_CODEC_CLOSE:
        {
            return 1;
        }

#if 0

        case SUBFN_CODEC_ENCODE:
        {
            return 1;
        }

#endif

        default:
        {
            return 0;
        }
    }
}
#pragma arm section code
#endif
