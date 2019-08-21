/********************************************************************************************
*   Copyright (C), 2008, Fuzhou Rockchip Co.,Ltd.
*   All Rights Reserved
*
*   File:
*              MovFile.c
*   Description:
*
*   Author:
*              guosl
*   Note:
*              None.
*   $Log: MovFile.c,v $
*   Revision 1.1.1.1  2009/11/16 06:40:48  zjd
*   20091116洪锦坤提交初始版本
*
*   Revision 1.1.1.1  2009/06/18 09:19:55  zjd
*   20090618 方镇福提交初始版本
*
*   Revision 1.1.1.1  2009/03/31 09:48:13  zjd
*   20090331由方镇福提交
*
*   Revision 1.3  2008/07/18 11:09:37  HZF
*   no message
*
*   Revision 1.2  2008/07/18 09:31:07  HZF
*   no message
*
*   Revision 1.1  2008/07/08 07:29:20  HZF
*   增加3gp和MP4
*
*   Revision 1.1  2008/5/16 14:43:19  guosl
*   no message
*
*
*
*
********************************************************************************************/
#include "../include/audio_main.h"

#ifdef A_CORE_DECODE
#define _IN_MOVFILE_H
#ifndef BOARD
    #include <stdio.h>
  //  #include <memory.h>
    #include <stdlib.h>
#include "aacdec.h"
#endif
#ifdef AAC_DEC_INCLUDE
#pragma arm section code = "AacDecCode", rodata = "AacDecCode", rwdata = "AacDecData", zidata = "AacDecBss"


#include "MovFile.h"
#include "movparserinterface.h"

#ifdef BOARD
#define abs     Abs
#else
#include <math.h>
#endif

#define movMain     main

static uint32 preVideoSampleNo = 0;
static uint32 preVideoBeginSampleNo = 0;
static uint32 preVideoChunkSize = 0;

static uint32 preAudioSampleNo = 0;
static uint32 preAudioBeginSampleNo = 0;
static uint32 preAudioChunkSize = 0;

STATIC uint32 sVideoTimeScale = 0x1;
STATIC uint32 sUint32VideoDuration = 0x0;
STATIC MovDuration sUint64VideoDuration = {0x0, 0x0};

MovFileinf gMovFile;

uint8 *gH264DataBuff;
DWORD IsTheTrackVideo = -1;
SKIPSTATE gBeingSkip = NO_SKIP;
SKIPSTATE gVideoBeingSkip = NO_SKIP;

//STATIC uint32 *sVideoSampleSizeIndex = NULL;
//STATIC uint32 *sAudioSampleSizeIndex = NULL;
//STATIC uint32 *sVideoChunkOffsetIndex = NULL;
//STATIC uint32 *sAudioChunkOffsetIndex = NULL;
/* modify Sample Size Index Store Style for Data Memory space Limitation */
STATIC MovIndex *sVideoSampleSizeIndex = NULL;
STATIC MovIndex *sAudioSampleSizeIndex = NULL;

STATIC uint32 *sVideoSampleToChunkIndex = NULL;
STATIC uint32 *sAudioSampleToChunkIndex = NULL;

STATIC MovIndex *sVideoChunkOffsetIndex = NULL;
STATIC MovIndex *sAudioChunkOffsetIndex = NULL;

STATIC uint32 *sVideoTimeToSampleIndex = NULL;
STATIC uint32 *sAudioTimeToSampleIndex = NULL;

STATIC uint32 *sVideoSyncSampleIndex = NULL;

//static uint8 sH264FrmHeader[H264_FRAME_HEAD_LENGTH]
//= {'H', '2', '6', '4', 'B', 'P', ' '};

/* sample rates (table 4.5.1) */
#if 1
const int sampRateTab[12] = {
    96000, 88200, 64000, 48000, 44100, 32000,
    24000, 22050, 16000, 12000, 11025,  8000
};
const int channelMapTab[8] = {
    -1, 1, 2, 3, 4, 5, 6, 8
};

#endif




SODB gSodb;
uint32 *sStartAddress;
EXT uint32 *sEndAddress;
int movLastSeekTime;
int gSkipFlag;
uint32 movTimerCount;
LONG  sMovFilledBlockNum;
uint32 skipToSampleNo;
extern unsigned long sameheader;
extern int m4aformfileflag;
extern uint32  videosamplesum;
int movaudfirst = 1;
int movvidfirst = 1;

MovMem movMem;

void MovMemInit(void)
{
    movMem.sizeused = 0;
}

int MovMemLeft(void)
{
    return MOV_MEM_SIZE - movMem.sizeused;
}

void* MovMemAlloc(int size)
{
    void *ret;

    if (size > MovMemLeft() || size <= 0)
    {
        aac_printf("MALLOC SIZE TOO little size = %d, left = %d\n", size, MovMemLeft());
        return 0;
    }

    ret = movMem.buf + movMem.sizeused;
    movMem.sizeused += size + (4 - size & 0x3); /* 4 bytes align */

    return ret;
}

void MovMemFree(void *mem)
{
    ;
}

//EXT int MovAudioSeekTime(int msTime);

//EXT void MovAudioStart(void);

//EXT void MovSetAudioPlayEnd(void);

//EXT void MovSetAudioPlayNoEnd(void);

//EXT BOOLEAN VideoAudioStop(uint16 ReqType);

/**
 * Following functions are Mov Buffer Processing;
 */
int movBufInit(MovBuf *movbuf, FILE *file)
{
    if (file == (FILE *)-1 || movbuf == NULL)
    {
        return -1;
    }
    movbuf->rdmax = movbuf->rdpos = 0;
    movbuf->st = file;

    return 0;
}

void movBufdeinit(MovBuf *movbuf)
{
    if (movbuf)
    {
        if (movbuf->st)
        {
            MovClose(movbuf->st);
            movbuf->st = NULL;
        }

        movbuf->rdmax = movbuf->rdpos = 0;
    }
}

int movBufLeft(MovBuf *movbuf)
{
    return movbuf->rdmax - movbuf->rdpos;
}

long movBufTell(MovBuf *movbuf)
{
    return MovTell(movbuf->st) - movBufLeft(movbuf);
}

int movBufSeek(MovBuf *movbuf, int seeksize, int mode)
{
    int fseeksize = 0;
    if (SEEK_CUR == mode)
    {
        if (seeksize == 0)
        {
            return 0;
        }

        if ((seeksize > 0 && abs(seeksize) > movBufLeft(movbuf))
            || (seeksize < 0 && abs(seeksize) > movbuf->rdpos))
        {
            fseeksize = seeksize - movBufLeft(movbuf);
            MovSeek(movbuf->st, fseeksize, SEEK_CUR);
            movbuf->rdmax = movbuf->rdpos = 0;
        }
        else
        {
            movbuf->rdpos += seeksize;
        }
    }
    else if (SEEK_SET == mode || SEEK_END == mode)
    {
        MovSeek(movbuf->st, seeksize, mode);
        movbuf->rdmax = movbuf->rdpos = 0;
    }
    else
    {
        return -1;
    }

    return 0;
}

int movBufRead(uint8 *buf, int size, int cnt, MovBuf *movbuf)
{
    int rdsize = 0;
    int ndsize = size * cnt;
    int rdfsize = 0;

    ASSERT(buf);

LOOP:
    rdsize = ndsize > movBufLeft(movbuf) ? movBufLeft(movbuf) : ndsize;

    memcpy(buf, movbuf->buf + movbuf->rdpos, rdsize);
    movbuf->rdpos += rdsize;
    buf += rdsize;
    ndsize -= rdsize;

    if (ndsize != 0)
    {
        if ((rdfsize = MovRead(movbuf->buf, 1, MOV_BUFFER_SIZE, movbuf->st)) == 0)
        {
            goto ret;
        }
        else
        {
            movbuf->rdpos = 0;
            movbuf->rdmax = rdfsize;
            goto LOOP;
        }
    }

ret:
    return cnt * size - ndsize;
}

int MovIndexCreate(FILE *aac_raw_file, MovIndex **index)
{
    *index = (MovIndex*)MALLOC(sizeof(MovIndex));

    if (*index == NULL)
    {
        return -1;
    }

    (*index)->indexbuf = (MOV_ST*)MALLOC(sizeof(MOV_ST));
    if ((*index)->indexbuf == NULL)
    {
        return -1;
    }
    if (0 > MovSTopen((*index)->indexbuf,aac_raw_file ))
    {
        return -1;
    }

    (*index)->indexpos = 0;
    (*index)->curpos = 0;

    return 0;
}

void MovIndexDestroy(MovIndex *index)
{
    if (index)
    {
        if (index->indexbuf)
        {
            MovSTclose(index->indexbuf);
            FREE(index->indexbuf);
        }
        FREE(index);
    }
}

void MovIndexInit(int indexpos, MovIndex *index)
{
    if (index == NULL)
    {
        return;
    }

    index->indexpos = indexpos;
    MovSTseek(index->indexbuf, indexpos, SEEK_SET);
    index->curpos = 0;
}

uint32 MovIndexRead(int indexno, MovIndex *index)
{
    uint32 rd;

    MovSTseek(index->indexbuf, (indexno - index->curpos - 1) * 4, SEEK_CUR);
    index->curpos = indexno;
    MovSTread((unsigned char *)&rd, 1, 4, index->indexbuf);

    return rd;
}

/**
* Redefine Abs function, for generate library
*/
long Abs(long num)
{
    return num < 0 ? 0-num : num;
}

void movFillEndData(char *buf, int size)
{
    int fillsize = size & 0x3;
    char pHeader[32];

    while (fillsize-- > 0)
    {
        *(buf++) = 0;
        size--;
    }

    MovGenerateMovFrmHeader(pHeader, 0, 0);

    if (VIDEO_CODEC_LIB_MP4V == gMovFile.vFormat)
    {
        pHeader[20] = (char)0xFF;
        pHeader[21] = (char)0xFF;
        pHeader[22] = (char)0xFF;
        pHeader[23] = (char)0xFF;
    }
    else
    {
        pHeader[19] = (char)0xFF;
    }

    while (size >= 32)
    {
        memcpy(buf, pHeader, 32);
        size -= 32;
        buf += 32;
    }
}
/* add by wujiangrui for analysize ESDS box  */
   int   read_byte=0;
  int read_mp4_descr_length(MOV_ST *videoFile)
 {
     char b;
     char numBytes = 0;
     int length = 0;

     do
     {
         if (MovSTread((unsigned char *)&b, 1, sizeof(char), videoFile) < sizeof(char))
        {
            return MOV_READ_DATA_FAILED;
        }
         numBytes++;
         length = (length << 7) | (b & 0x7F);
     } while ((b & 0x80) && numBytes < 4);
     read_byte+=numBytes;
     return length;
 }


//esds 存储媒体序列的最大采样率，平均采样率，通道数索引以及平均采样率索引
int MovReadESDS(MOV_ST *videoFile, int *sampleRate_config,int *channel_count_config)
{
    char temp_tag = 0x0;
    int temp_len;
    int channel_configuration_index;
    int samplerate_index;
    char buf1,buf2;

    read_byte=0;
    MovSTseek(videoFile, FOURBYTE, SEEK_CUR);
    read_byte +=FOURBYTE;
    if (MovSTread((unsigned char *)&temp_tag, 1, sizeof(char), videoFile) < sizeof(char))
    {
        aac_DEBUG();
        return MOV_READ_DATA_FAILED;
    }
    read_byte+=1;

    if(temp_tag == 0x03)
    {
        temp_len = read_mp4_descr_length(videoFile);

        MovSTseek(videoFile, 0x3, SEEK_CUR);
        read_byte+=3;
    }
    else
    {
        MovSTseek(videoFile, 0x2, SEEK_CUR);
        read_byte+=2;
    }
    if (MovSTread((unsigned char *)&temp_tag, 1, sizeof(char), videoFile) < sizeof(char))
    {
         aac_DEBUG();
        return MOV_READ_DATA_FAILED;
    }
    read_byte+=1;
    if(temp_tag != 0x04)
    {
        aac_DEBUG();
        return MOV_UNSUPPORTED_STREAM;
    }
    temp_len = read_mp4_descr_length( videoFile);
    if(temp_len < 13)
    {
        aac_DEBUG();
        return MOV_UNSUPPORTED_STREAM;
    }
    MovSTseek(videoFile, 13, SEEK_CUR);//可获得最大比特率及平均比特率
    read_byte+=13;
    if (MovSTread((unsigned char *)&temp_tag, 1, sizeof(char), videoFile) < sizeof(char))
    {
        aac_DEBUG();
        return MOV_READ_DATA_FAILED;
    }
        read_byte+=1;
    if(temp_tag != 0x05)
    {
        aac_DEBUG();
        return MOV_UNSUPPORTED_STREAM;
    }
    temp_len = read_mp4_descr_length( videoFile);

    if (MovSTread((unsigned char *)&buf1, 1, sizeof(char), videoFile) < sizeof(char))
    {
        aac_DEBUG();
        return MOV_READ_DATA_FAILED;
    }
        read_byte+=1;
    if (MovSTread((unsigned char *)&buf2, 1, sizeof(char), videoFile) < sizeof(char))
    {
        aac_DEBUG();
        return MOV_READ_DATA_FAILED;
    }
    read_byte+=1;
    samplerate_index = ((buf1 & 0x7)<<1)|(buf2 >>7);
    channel_configuration_index = (buf2 >> 3)& 0xf;
    *sampleRate_config = sampRateTab[samplerate_index];
    *channel_count_config = channelMapTab[channel_configuration_index];
    MovSTseek(videoFile, -read_byte, SEEK_CUR);
    return MOV_FILE_BOX_PARSE_SUCCESS ;

}

/************end**************/

 int MovReadStsd(MOV_ST *videoFile, MovFileinf *pFileinf, LONG boxSize)
{
    int sampleRate_config, channel_count_config;
    ASSERT((videoFile != NULL)
           && (pFileinf != NULL));

    if (IsTheTrackVideo == SIGN_OTHER)
    {
        MovSTseek(videoFile, boxSize - (FOURBYTE << 1), SEEK_CUR);
    }
    else
    {
        int entries;
        DWORD codeTag = 0x0;
        int loop;
        int entrySize;

        MovSTseek(videoFile, FOURBYTE, SEEK_CUR);
        if (MovSTread((unsigned char *)&entries, 1, sizeof(int), videoFile) < sizeof(int))
        {
            aac_DEBUG();
            return MOV_READ_DATA_FAILED;
        }
        entries = BYTESWAP(entries);

        for (loop = 0x0; loop < entries; loop++)
        {
            if (MovSTread((unsigned char *)&entrySize, 1, sizeof(int), videoFile) < sizeof(int))
            {
                return MOV_READ_DATA_FAILED;
            }
            entrySize = BYTESWAP(entrySize);
            if (MovSTread((unsigned char *)&codeTag, 1, sizeof(DWORD), videoFile) < sizeof(DWORD))
            {
                return MOV_READ_DATA_FAILED;
            }

            if (IsTheTrackVideo == SIGN_VIDEO)
            {
                switch (codeTag)
                {
                case VIDEOCODE_MP4V :
                    pFileinf->vFormat = VIDEO_CODEC_LIB_MP4V;
                    strncpy(gMovFile.VideoDec, "M4V",4);
                    break;

                case VIDEOCODE_S263 :
                case VIDEOCODE_H263 :
                    pFileinf->vFormat = VIDEO_CODEC_LIB_H263;
                    strncpy(gMovFile.VideoDec, "263",4);
                    break;

                case VIDEOCODE_AVC1 :
                    pFileinf->vFormat = VIDEO_CODEC_LIB_H264;
                    strncpy(gMovFile.VideoDec, "264",4);
                    break;
                default :
                    pFileinf->vFormat = VIDEO_CODEC_LIB_NULL;
                    return MOV_FILE_VIDEO_UNSUPPORTED ;
                    //break;

                }

                MovSTseek(videoFile, FOURBYTE * 6, SEEK_CUR);

                /* read the width of the video */
                if (MovSTread((unsigned char *)&pFileinf->width, 1, sizeof(WORD), videoFile) < sizeof(WORD))
                {
                    return MOV_READ_DATA_FAILED;
                }
                pFileinf->width = WORDSWAP(pFileinf->width);

                /* read the height of the video */
                if (MovSTread((unsigned char *)&pFileinf->height, 1, sizeof(WORD), videoFile) < sizeof(WORD))
                {
                    return MOV_READ_DATA_FAILED;
                }
                pFileinf->height = WORDSWAP(pFileinf->height);

                if (pFileinf->vFormat == VIDEO_CODEC_LIB_H264)
                {
                    if ((pFileinf->width > MOV_H264_MAX_FRAME_WIDTH)
                        || (pFileinf->height > MOV_H264_MAX_FRAME_HEIGHT))
                    {
                        aac_DEBUG();
                        return MOV_FILE_RESOLUTION_UNSUPPORTED ;
                    }
                }
                else
                {
                    if ((pFileinf->width > MOV_MAX_FRAME_WIDTH)
                        || (pFileinf->height > MOV_MAX_FRAME_HEIGHT))
                    {
                        aac_DEBUG();
                        return MOV_FILE_RESOLUTION_UNSUPPORTED ;
                    }
                }

                if ((pFileinf->vFormat == VIDEO_CODEC_LIB_MP4V)
                    || (pFileinf->vFormat == VIDEO_CODEC_LIB_H264))
                {
                    uint32 size = 0x0;
                    DWORD  tag = 0x0;

                    /* read the information of esds*/
                    MovSTseek(videoFile, 0x32, SEEK_CUR);
                    if (MovSTread((unsigned char *)&size, 1, sizeof(uint32), videoFile) < sizeof(uint32))
                    {
                        return MOV_READ_DATA_FAILED;
                    }
                    size = BYTESWAP(size);
                    if (MovSTread((unsigned char *)&tag, 1, sizeof(DWORD), videoFile) < sizeof(DWORD))
                    {
                        return MOV_READ_DATA_FAILED;
                    }

                    if (tag == SIGN_ESDS)
                    {
                        MovSTseek(videoFile, FOURBYTE, SEEK_CUR);
                        pFileinf->esdsInf.length = size - 0xC;
                        //pFileinf->esdsInf.esdsFilePos.clus = videoFile->Clus;
                        pFileinf->esdsInf.esdsFilePos.offset = MovSTtell(videoFile);
                        //MovFseek(videoFile, (size - 0xC), SEEK_CUR);
                        MovSTseek(videoFile, (entrySize - 0x62), SEEK_CUR);
                    }
                    else
                    {
                        if (tag == SIGN_AVCC)
                        {
                            MovSTseek(videoFile, FOURBYTE, SEEK_CUR);
                            pFileinf->avcCinf.length = size - 0xC;
                            //pFileinf->avcCinf.avcCFilePos.clus = videoFile->Clus;
                            pFileinf->avcCinf.avcCFilePos.offset = MovSTtell(videoFile);

                            /* 判断Profile_Idc是否等于66，
                               目前解码库只支持Profile_Idc = 66的码流 */
                            {
                                BYTE profileIdc;

                                MovSTseek(videoFile, 0x5, SEEK_CUR);
                                if (MovSTread(&profileIdc, 1, sizeof(BYTE), videoFile)
                                    < sizeof(BYTE))
                                {
                                    return MOV_READ_DATA_FAILED;
                                }

                                if (profileIdc != 0x42)
                                {
                                    //return MOV_UNSUPPORTED_STREAM;
                                }

                            }

                            MovSTseek(videoFile, (entrySize - 0x68), SEEK_CUR);
                        }
                        else
                        {
                            //MovFseek(videoFile, (size - 0x8), SEEK_CUR);
                            MovSTseek(videoFile, (entrySize - 0x5E), SEEK_CUR);
                        }
                    }
                }
                else
                {
                    MovSTseek(videoFile, entrySize - (FOURBYTE * 0x9), SEEK_CUR);
                }
            }
            else
            {
                switch (codeTag)
                {
                case AUDIOCODE_SAMR :
                    pFileinf->aFormat = AUDIO_CODEC_LIB_SAMR;
                    strncpy(gMovFile.AudioDec, "AMR",16);
                    break;

                case AUDIOCODE_MP4A :
                    pFileinf->aFormat = AUDIO_CODEC_LIB_MP4A;
                    strncpy(gMovFile.AudioDec, "VAC",16);
                    break;

                case AUDIOCODE_MP3:
                    pFileinf->aFormat = AUDIO_CODEC_LIB_MP3;
                    strncpy(gMovFile.AudioDec, "VM3",16);
                    break;

                default :
                    pFileinf->aFormat = AUDIO_CODEC_LIB_NULL;
                    break;
                }

                MovSTseek(videoFile, (FOURBYTE << 0x2), SEEK_CUR);

                /* read the channelCount of the audio*/
                if (MovSTread((unsigned char *)&pFileinf->channelCount, 1, sizeof(WORD), videoFile) < sizeof(WORD))
                {
                    return MOV_READ_DATA_FAILED;
                }
                pFileinf->channelCount = WORDSWAP(pFileinf->channelCount);
                MovSTseek(videoFile, 0x6, SEEK_CUR);

                /* read the sample rate of audio */
                if (MovSTread((unsigned char *)&pFileinf->sampleRate, 1, sizeof(uint32), videoFile) < sizeof(uint32))
                {
                    return MOV_READ_DATA_FAILED;
                }
                pFileinf->sampleRate = BYTESWAP(pFileinf->sampleRate);
                pFileinf->sampleRate = ((pFileinf->sampleRate >> 16) & 0x0000ffff);

                /*add by jiangrui.wu  for analysise ESDS*/
                #if 1
                {
                    uint32 size = 0x0;
                    uint32  tag = 0x0;

                    if (MovSTread((unsigned char *)&size, 1, sizeof(uint32), videoFile) < sizeof(uint32))
                    {
                        return MOV_READ_DATA_FAILED;
                    }
                    size = BYTESWAP(size);

                    if (MovSTread((unsigned char *)&tag, 1, sizeof(uint32), videoFile) < sizeof(uint32))
                    {
                        return MOV_READ_DATA_FAILED;
                    }

                    if (tag == SIGN_ESDS)
                    {
                        if(MovReadESDS(videoFile,&sampleRate_config,&channel_count_config))
                        {
                            aac_DEBUG();
                            return MOV_FILE_BOX_PARSE_ERR;
                        }

                        //若使用MP4A的数据，当MP4A和esds的信息不对应时，会出错,
                        //生成的帧头信息中的采样率索引和通道数索引最好选用ESDS的box提供的数据
                        if(sampleRate_config!= pFileinf->sampleRate ||channel_count_config!=pFileinf->channelCount )
                        {
                             pFileinf->sampleRate = sampleRate_config;
                             pFileinf->channelCount = channel_count_config;

                        }
                        MovSTseek(videoFile, entrySize - (FOURBYTE * 0xB), SEEK_CUR);
                    }
                    else
                    {
                       MovSTseek(videoFile, entrySize - (FOURBYTE * 0xB), SEEK_CUR);
                    }
                  }
                #else
                   MovSTseek(videoFile, entrySize - (FOURBYTE * 0x9), SEEK_CUR);
                #endif
            }
        }
    }

    return MOV_FILE_BOX_PARSE_SUCCESS;
}

/********************************************************************************************
*   Func:
*       MovReadStts()
*   Description:
*
*   Param:
*
*   Return:
*       0:   OK.
*       -1:  Error.
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/

 int MovReadStts(MOV_ST *videoFile, MovFileinf *pFileinf, LONG boxSize)
{
    uint32 entryCount = 0x0;

    ASSERT((videoFile != NULL)
           && (pFileinf != NULL));

    MovSTseek(videoFile, FOURBYTE, SEEK_CUR);
    if (MovSTread((unsigned char *)&entryCount, 1, sizeof(uint32), videoFile) < sizeof(uint32))
    {
        return MOV_READ_DATA_FAILED;
    }
    entryCount = BYTESWAP(entryCount);

    switch (IsTheTrackVideo)
    {
    case SIGN_VIDEO:
        pFileinf->videoTimeToSampleNo = entryCount;

        sVideoTimeToSampleIndex = (uint32 *)MALLOC(sizeof(uint32) * 0x2 * entryCount);

        if (sVideoTimeToSampleIndex == NULL)
        {
            return MALLOC_FAILED;
        }

        if (MovSTread((unsigned char *)sVideoTimeToSampleIndex, 1, sizeof(uint32) * (0x2 * entryCount), videoFile)
            < (sizeof(uint32) * 0x2 * entryCount))
        {
            return MOV_READ_DATA_FAILED;
        }
        break;

    case SIGN_AUDIO:
        pFileinf->audioTimeToSampleNo = entryCount;
        sAudioTimeToSampleIndex = (uint32 *)MALLOC(sizeof(uint32) * 0x2 * entryCount);

        if (sAudioTimeToSampleIndex == NULL)
        {
            return MALLOC_FAILED;
        }

        if (MovSTread((unsigned char *)sAudioTimeToSampleIndex, 1, sizeof(uint32) * (0x2 * entryCount), videoFile)
            < (sizeof(uint32) * 0x2 * entryCount))
        {
            return MOV_READ_DATA_FAILED;
        }
        break;

    default:
        MovSTseek(videoFile, boxSize - (FOURBYTE << 2), SEEK_CUR);
        break;
    }

    return MOV_FILE_BOX_PARSE_SUCCESS;
}

/********************************************************************************************
*   Func:
*       MovReadCtts()
*   Description:
*
*   Param:
*
*   Return:
*       0:   OK.
*       -1:  Error.
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/


/********************************************************************************************
*   Func:
*       MovReadStsc()
*   Description:
*
*   Param:
*
*   Return:
*       0:   OK.
*       -1:  Error.
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/

 int MovReadStsc(MOV_ST *videoFile, MovFileinf *pFileinf, LONG boxSize)
{
    uint32  length = 0x0;

    ASSERT((videoFile != NULL)
           && (pFileinf != NULL));
    MovSTseek(videoFile, FOURBYTE, SEEK_CUR);
    if (MovSTread((unsigned char *)&length, 1, sizeof(uint32), videoFile) < sizeof(uint32))
    {
        return MOV_READ_DATA_FAILED;
    }
    length = BYTESWAP(length);

    switch (IsTheTrackVideo)
    {
    case SIGN_VIDEO:
        pFileinf->videoSampleToChunkNo  = length;
        sVideoSampleToChunkIndex = (uint32 *)MALLOC(sizeof(uint32) * 0x3 * length);

        if (sVideoSampleToChunkIndex == NULL)
        {
            return MALLOC_FAILED;
        }

        if (MovSTread((unsigned char *)sVideoSampleToChunkIndex, 1, sizeof(uint32) * (length * 0x3), videoFile) < (sizeof(uint32) * length * 0x3))
        {
            return MOV_READ_DATA_FAILED;
        }
        break;

    case SIGN_AUDIO:
        pFileinf->audioSampleToChunkNo = length;
        sAudioSampleToChunkIndex = (uint32 *)MALLOC(sizeof(uint32) * 0x3 * length);

        if (sAudioSampleToChunkIndex == NULL)
        {
            return MALLOC_FAILED;
        }

        if (MovSTread((unsigned char *)sAudioSampleToChunkIndex, 1, sizeof(uint32) * (length * 0x3), videoFile)
            < (sizeof(uint32) * length * 0x3))
        {
            return MOV_READ_DATA_FAILED;
        }
        break;

    default:
        MovSTseek(videoFile, boxSize - (FOURBYTE << 2), SEEK_CUR);
        break;

    }

    return MOV_FILE_BOX_PARSE_SUCCESS;
}

/********************************************************************************************
*   Func:
*       MovReadStsz()
*   Description:
*
*   Param:
*
*   Return:
*       0:   OK.
*       -1:  Error.
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/

 int MovReadStsz(MOV_ST *videoFile, MovFileinf *pFileinf, LONG boxSize)
{
    uint32  sampleSize =0x0;
    uint32  sampleCount = 0x0;
    uint32  videosamplesize = 0x0;
    uint32  videotemp = 0x0;
    ASSERT((videoFile != NULL)
           && (pFileinf != NULL));

    MovSTseek(videoFile, FOURBYTE, SEEK_CUR);
    if (MovSTread((unsigned char *)&sampleSize, 1, sizeof(uint32), videoFile) < sizeof(uint32))
    {
        return MOV_READ_DATA_FAILED;
    }
    sampleSize = BYTESWAP(sampleSize);

    switch (IsTheTrackVideo)
    {
    case SIGN_VIDEO:
        pFileinf->videoSampleSize = sampleSize;
        break;

    case SIGN_AUDIO:
        pFileinf->audioSampleSize = sampleSize;
        break;

    default:
        break;

    }

    /*number of enties(samples)*/
    if (MovSTread((unsigned char *)&sampleCount, 1, sizeof(uint32), videoFile) < sizeof(uint32))
    {
        return MOV_READ_DATA_FAILED;
    }
    sampleCount = BYTESWAP(sampleCount);

    #if 1
       if(IsTheTrackVideo == SIGN_VIDEO)  //if track is video
       {
           //get total of video samples size
          for (videotemp = sampleCount; videotemp>0;videotemp--)
          {
              if (MovSTread((unsigned char *)&videosamplesize , 1, sizeof(uint32), videoFile) < sizeof(uint32))
              {
                 return MOV_READ_DATA_FAILED;
              }
              videosamplesize = BYTESWAP(videosamplesize);
              videosamplesum += videosamplesize;
          }
          aac_printf("vidiosamplesum = %d", videosamplesum);
          MovSTseek(videoFile, -sizeof(uint32)* sampleCount, SEEK_CUR);
       }
    #endif

    switch (IsTheTrackVideo)
    {
    case SIGN_VIDEO:
        pFileinf->videoSampleNum = sampleCount;

        if (pFileinf->videoSampleSize == 0x0)
        {
#if 1
            /*sVideoSampleSizeIndex = (uint32 *)MALLOC(sizeof(uint32) * sampleCount);

            if (sVideoSampleSizeIndex == NULL)
                return MALLOC_FAILED;

            MovSTread(sVideoSampleSizeIndex, 1, sizeof(uint32) * sampleCount, videoFile);*/

            MovIndexInit(MovSTtell(videoFile), sVideoSampleSizeIndex);
            MovSTseek(videoFile, sizeof(uint32) * sampleCount, SEEK_CUR);
#else
            if (sEndAddress < (uint32*)((char*)sStartAddress + sampleCount))
            {
                return MALLOC_FAILED;
            }
            else
            {

                sVideoSampleSizeIndex = sStartAddress;

                if (MovFread((unsigned char *)sVideoSampleSizeIndex, 1, sizeof(uint32) * sampleCount, videoFile)
                    < (sizeof(uint32) * sampleCount))
                {
                    return MOV_READ_DATA_FAILED;
                }

                sStartAddress += sampleCount;
            }

#endif
        }

        break;

    case SIGN_AUDIO:
        pFileinf->audioSampleNum = sampleCount;

        if (pFileinf->audioSampleSize == 0x0)
        {
#if 1
            /*sAudioSampleSizeIndex = (uint32 *)MALLOC(sizeof(uint32) * sampleCount);

            if (sAudioSampleSizeIndex == NULL)
                return MALLOC_FAILED;

            //MovSTread(sAudioSampleSizeIndex, sizeof(uint32), sampleCount, videoFile);*/
            MovIndexInit(MovSTtell(videoFile), sAudioSampleSizeIndex);
            MovSTseek(videoFile, sizeof(uint32) * sampleCount, SEEK_CUR);
#else
            if (sEndAddress < (uint32*)((char*)sStartAddress + sampleCount))
            {
                return MALLOC_FAILED;
            }
            else
            {
                sAudioSampleSizeIndex = sStartAddress;

                if (MovFread(sAudioSampleSizeIndex, 1, sizeof(uint32) * sampleCount, videoFile)
                    < (sizeof(uint32) * sampleCount))
                {
                    return MOV_READ_DATA_FAILED;
                }
                sStartAddress += sampleCount;
            }
#endif
        }

        break;

    default:
        MovSTseek(videoFile, boxSize - FOURBYTE * 5, SEEK_CUR);
        break;
    }

    return MOV_FILE_BOX_PARSE_SUCCESS;
}


/********************************************************************************************
*   Func:
*       MovReadStco()
*   Description:
*
*   Param:
*
*   Return:
*       0:   OK.
*       -1:  Error.
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/

 int MovReadStco(MOV_ST *videoFile, MovFileinf *pFileinf, LONG boxSize)
{
    uint32 length = 0x0;

    ASSERT((videoFile != NULL)
           && (pFileinf != NULL));

    MovSTseek(videoFile, FOURBYTE, SEEK_CUR);
    if (MovSTread((unsigned char *)&length, 1, sizeof(uint32), videoFile) < sizeof(uint32))
    {
        return MOV_READ_DATA_FAILED;
    }
    length = BYTESWAP(length);

    switch (IsTheTrackVideo)
    {
    case SIGN_VIDEO:
        pFileinf->videoChunkNum = length;
        /*sVideoChunkOffsetIndex = (uint32 *)MALLOC(sizeof(uint32) * length);

        if (sVideoChunkOffsetIndex == NULL)
        {
            return MALLOC_FAILED;
        }

        if (MovSTread(sVideoChunkOffsetIndex, 1, sizeof(uint32) * length, videoFile)
            < (sizeof(uint32) * length))
        {
            return MOV_READ_DATA_FAILED;
        }*/
        MovIndexInit(MovSTtell(videoFile), sVideoChunkOffsetIndex);
        MovSTseek(videoFile, sizeof(uint32) * length, SEEK_CUR);
        break;

    case SIGN_AUDIO:
        pFileinf->audioChunkNum = length;
        /*sAudioChunkOffsetIndex = (uint32 *)MALLOC(sizeof(uint32) * length);

        if (sAudioChunkOffsetIndex == NULL)
        {
            return MALLOC_FAILED;
        }

        if (MovSTread(sAudioChunkOffsetIndex, 1, sizeof(uint32) * length, videoFile)
            < (sizeof(uint32) * length))
        {
            return MOV_READ_DATA_FAILED;
        }*/
        MovIndexInit(MovSTtell(videoFile), sAudioChunkOffsetIndex);
        MovSTseek(videoFile, sizeof(uint32) * length, SEEK_CUR);
        break;

    default:
        MovSTseek(videoFile, boxSize - (FOURBYTE << 2), SEEK_CUR);
        break;

    }

    return MOV_FILE_BOX_PARSE_SUCCESS;
}


/********************************************************************************************
*   Func:
*       MovReadStss()
*   Description:
*
*   Param:
*
*   Return:
*       0:   OK.
*       -1:  Error.
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/

 int MovReadStss(MOV_ST *videoFile, MovFileinf *pFileinf, LONG boxSize)
{
    uint32 entryCount = 0x0;

    ASSERT((videoFile != NULL)
           && (pFileinf != NULL));

    switch (IsTheTrackVideo)
    {
    case SIGN_VIDEO:
        MovSTseek(videoFile, FOURBYTE, SEEK_CUR);

        /* read the number of random access video samples */
        if (MovSTread((unsigned char *)&entryCount, 1, sizeof(uint32), videoFile) < sizeof(uint32))
        {
            aac_DEBUG();
            return MOV_READ_DATA_FAILED;
        }
        entryCount = BYTESWAP(entryCount);
        pFileinf->videoSyncSampleNo = entryCount;

        if (pFileinf->videoSyncSampleNo > 0x0)
        {
            sVideoSyncSampleIndex = (uint32 *)MALLOC(sizeof(uint32) * pFileinf->videoSyncSampleNo);

            if (sVideoSyncSampleIndex == NULL)
            {
                aac_printf("malloc %d\n",pFileinf->videoSyncSampleNo);
                return MALLOC_FAILED;
            }

            if (MovSTread((unsigned char *)sVideoSyncSampleIndex, 1, sizeof(uint32) * pFileinf->videoSyncSampleNo, videoFile)
                < (sizeof(uint32) * pFileinf->videoSyncSampleNo))
            {
                aac_DEBUG();
                return MOV_READ_DATA_FAILED;
            }
        }

        break;

    default:
        MovSTseek(videoFile, boxSize - (FOURBYTE << 1), SEEK_CUR);
        break;
    }

    return MOV_FILE_BOX_PARSE_SUCCESS;
}



/********************************************************************************************
*   Func:
*       MovReadStbl()
*   Description:
*
*   Param:
*
*   Return:
*       0:   OK.
*       -1:  Error.
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/

 int MovReadStbl(MOV_ST *videoFile, MovFileinf *pFileinf, LONG boxSize)
{
    uint32  length = 0x0;
    DWORD tag = 0x0;
    uint32 sumLength = 0x0;

    ASSERT((videoFile != NULL)
           && (pFileinf != NULL));

    if (boxSize <= 8)
    {
        return MOV_FILE_BOX_PARSE_SUCCESS;
    }
    else
    {
        for (; ;)
        {
            /* 读取这个box的长度 */
            if (MovSTread((unsigned char *)&length, 1, sizeof(uint32), videoFile) < sizeof(uint32))
            {
                aac_DEBUG();
                return MOV_READ_DATA_FAILED;
            }

            length = BYTESWAP(length);
            sumLength += length;

            if (MovSTread((unsigned char *)&tag, 1, sizeof(DWORD), videoFile) < sizeof(DWORD))
            {
                aac_DEBUG();
                return MOV_READ_DATA_FAILED;
            }

            switch (tag)
            {
            case SIGN_STSD :
                if (MovReadStsd(videoFile, pFileinf, length))
                {
                    aac_DEBUG();
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

            case SIGN_STTS :
                if (MovReadStts(videoFile, pFileinf, length))
                {
                   aac_DEBUG();
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

                /*case SIGN_CTTS :
                    if (MovReadCtts(videoFile, pFileinf, length))
                    {
                        return MOV_FILE_BOX_PARSE_ERR;
                    }

                    break;*/

            case SIGN_STSC :
                if (MovReadStsc(videoFile, pFileinf, length))
                {
                    aac_DEBUG();
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

            case SIGN_STSZ :
                if (MovReadStsz(videoFile, pFileinf, length))
                {
                   aac_DEBUG();
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

                /*case SIGN_STZ2 :
                    if (MovReadStz2(videoFile, pFileinf, length))
                    {
                        return MOV_FILE_BOX_PARSE_ERR;
                    }

                    break;*/

            case SIGN_STCO:
                if (MovReadStco(videoFile, pFileinf, length))
                {
                    aac_DEBUG();
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

                /*case SIGN_CO64 :
                    if (MovReadCo64(videoFile, pFileinf, length))
                    {
                        return MOV_FILE_BOX_PARSE_ERR;
                    }

                    break;*/

            case SIGN_STSS :
                if (MovReadStss(videoFile, pFileinf, length))
                {
                   aac_DEBUG();
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

            /*case SIGN_STSH :
                if (MovReadStsh(videoFile, pFileinf, length))
                {
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

            case SIGN_PADB :
                if (MovReadPadb(videoFile, pFileinf, length))
                {
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

            case SIGN_STDP :
                if (MovReadStdp(videoFile, pFileinf, length))
                {
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

            case SIGN_SGPD :
                if (MovReadSgpd(videoFile, pFileinf, length))
                {
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

            case SIGN_SDTP :
                if (MovReadSdtp(videoFile, pFileinf, length))
                {
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

            case SIGN_SBGP :
                if (MovReadSbgp(videoFile, pFileinf, length))
                {
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

            case SIGN_SUBS :
                if (MovReadSubs(videoFile, pFileinf, length))
                {
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;*/
            case SIGN_CO64:
                MovSTseek(videoFile, length - (FOURBYTE << 1), SEEK_CUR);
                break;
            default :
                MovSTseek(videoFile, length - (FOURBYTE << 1), SEEK_CUR);
                break;

            }

            if ((sumLength + (FOURBYTE << 1)) >= boxSize)
            {
                return MOV_FILE_BOX_PARSE_SUCCESS;
            }
        }//for
    }//else

    return MOV_FILE_BOX_PARSE_SUCCESS;
}

/********************************************************************************************
*   Func:
*       MovReadMdhd()
*   Description:
*
*   Param:
*
*   Return:
*       0:   OK.
*       -1:  Error.
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/

 int MovReadMdhd(MOV_ST *videoFile, MovFileinf *pFileinf, LONG boxSize)
{
    DWORD vsfl = 0x0;   //version & flag
    uint32  timeScale = 0x0;
    uint32 tempVar = 0x0;

    ASSERT((videoFile != NULL)
           && (pFileinf != NULL));

    if (MovSTread((unsigned char *)&vsfl, 1, sizeof(DWORD), videoFile) < sizeof(DWORD))
    {
        return MOV_READ_DATA_FAILED;
    }
    vsfl = BYTESWAP(vsfl);

    if (vsfl & 0xff000000)  //version == 1
    {
        MovSTseek(videoFile, FOURBYTE << 2, SEEK_CUR);
        if (MovSTread((unsigned char *)&timeScale, 1, sizeof(uint32), videoFile) < sizeof(uint32))
        {
            return MOV_READ_DATA_FAILED;
        }
        timeScale = BYTESWAP(timeScale);

        if (IsTheTrackVideo == SIGN_AUDIO)
        {
            uint32 tmp;

            pFileinf->audioTimeScale = timeScale;

            /**
             * Ceva or VC does not support 64 bits data; for
             */
            /*if (MovFread(&pFileinf->uint64AudioDuration, 1, sizeof(uint64), videoFile) < sizeof(uint64))*/
            if (MovSTread((unsigned char *)&pFileinf->AudioDuration.low_part, 1, sizeof(uint32), videoFile) < sizeof(uint32))
            {
                return MOV_READ_DATA_FAILED;
            }
            if (MovSTread((unsigned char *)&pFileinf->AudioDuration.high_part, 1, sizeof(uint32), videoFile) < sizeof(uint32))
            {
                return MOV_READ_DATA_FAILED;
            }
            //pFileinf->uint64AudioDuration = UINT64SWAP(pFileinf->uint64AudioDuration);
            pFileinf->AudioDuration.low_part = BYTESWAP(pFileinf->AudioDuration.low_part);
            pFileinf->AudioDuration.high_part = BYTESWAP(pFileinf->AudioDuration.high_part);

            /*Note:here just take high part of 64bit as audioDuration*/
            /*get audio duration of this media*/
            pFileinf->uint32AudioDuration = pFileinf->AudioDuration.high_part;
        }
        else
        {
            if (IsTheTrackVideo == SIGN_VIDEO)
            {
                pFileinf->videoTimeScale = timeScale;
                sVideoTimeScale = timeScale;
                if (MovSTread((unsigned char *)&pFileinf->VideoDuration.low_part, 1, sizeof(uint32), videoFile) < sizeof(uint32))
                {
                    return MOV_READ_DATA_FAILED;
                }
                if (MovSTread((unsigned char *)&pFileinf->VideoDuration.high_part, 1, sizeof(uint32), videoFile) < sizeof(uint32))
                {
                    return MOV_READ_DATA_FAILED;
                }
                //pFileinf->uint64VideoDuration = UINT64SWAP(pFileinf->uint64VideoDuration);
                pFileinf->VideoDuration.low_part = BYTESWAP(pFileinf->VideoDuration.low_part);
                pFileinf->VideoDuration.high_part = BYTESWAP(pFileinf->VideoDuration.high_part);
                sUint64VideoDuration.low_part = pFileinf->VideoDuration.low_part;
                sUint64VideoDuration.high_part = pFileinf->VideoDuration.high_part;

                tempVar = pFileinf->VideoDuration.low_part;// * 1000; /* need to modify after ljf@20100319 */
                pFileinf->length = (uint32)(tempVar / pFileinf->videoTimeScale * 1000)
                    + (((tempVar % pFileinf->videoTimeScale) * 1000) / pFileinf->videoTimeScale);
            }
            else
            {
                sVideoTimeScale = timeScale;
                if (MovSTread((unsigned char *)&pFileinf->VideoDuration.low_part, 1, sizeof(uint32), videoFile) < sizeof(uint32))
                {
                    return MOV_READ_DATA_FAILED;
                }
                if (MovSTread((unsigned char *)&pFileinf->VideoDuration.high_part, 1, sizeof(uint32), videoFile) < sizeof(uint32))
                {
                    return MOV_READ_DATA_FAILED;
                }
                //sUint64VideoDuration = UINT64SWAP(sUint64VideoDuration);
                pFileinf->VideoDuration.low_part = BYTESWAP(pFileinf->VideoDuration.low_part);
                pFileinf->VideoDuration.high_part = BYTESWAP(pFileinf->VideoDuration.high_part);
            }
        }

        MovSTseek(videoFile, boxSize - (FOURBYTE * 10), SEEK_CUR);
    }
    else    //version == 0
    {
        MovSTseek(videoFile, FOURBYTE << 1, SEEK_CUR);
        if (MovSTread((unsigned char *)&timeScale, 1, sizeof(uint32), videoFile) < sizeof(uint32))
        {
            return MOV_READ_DATA_FAILED;
        }
        timeScale = BYTESWAP(timeScale);

        if (IsTheTrackVideo == SIGN_AUDIO)
        {
            pFileinf->audioTimeScale = timeScale;
            if (MovSTread((unsigned char *)&pFileinf->uint32AudioDuration, 1, sizeof(uint32), videoFile) < sizeof(uint32))
            {
                return MOV_READ_DATA_FAILED;
            }
            pFileinf->uint32AudioDuration = BYTESWAP(pFileinf->uint32AudioDuration);
        }
        else
        {
            if (IsTheTrackVideo == SIGN_VIDEO)
            {
                pFileinf->videoTimeScale = timeScale;
                sVideoTimeScale = timeScale;
                if (MovSTread((unsigned char *)&pFileinf->uint32VideoDuration, 1, sizeof(uint32), videoFile) < sizeof(uint32))
                {
                    return MOV_READ_DATA_FAILED;
                }
                pFileinf->uint32VideoDuration = BYTESWAP(pFileinf->uint32VideoDuration);
                sUint32VideoDuration = pFileinf->uint32VideoDuration;
                tempVar = (uint32)pFileinf->uint32VideoDuration * 1000;
                pFileinf->length = (uint32)(tempVar / (uint32)pFileinf->videoTimeScale)
                    + (((tempVar % pFileinf->videoTimeScale) * 1000) / pFileinf->videoTimeScale);
            }
            else
            {
                sVideoTimeScale = timeScale;
                if (MovSTread((unsigned char *)&sUint32VideoDuration, 1, sizeof(uint32), videoFile) < sizeof(uint32))
                {
                    return MOV_READ_DATA_FAILED;
                }
                sUint32VideoDuration = BYTESWAP(sUint32VideoDuration);
            }
        }

        MovSTseek(videoFile, boxSize - (FOURBYTE * 7), SEEK_CUR);
    }

    return MOV_FILE_BOX_PARSE_SUCCESS;
}

/********************************************************************************************
*   Func:
*       MovReadHdlr()
*   Description:
*
*   Param:
*
*   Return:
*       0:   OK.
*       -1:  Error.
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/

 int MovReadHdlr(MOV_ST *videoFile, MovFileinf *pFileinf, LONG boxSize)
{
    DWORD tag =0x0;
    uint32 tempVar = 0x0;

    ASSERT((videoFile != NULL)
           && (pFileinf != NULL));

    MovSTseek(videoFile, FOURBYTE << 1, SEEK_CUR);
    if (MovSTread((unsigned char *)&tag, 1, sizeof(DWORD), videoFile) < sizeof(DWORD))
    {
        return MOV_READ_DATA_FAILED;
    }

    if (tag == HANDLER_VIDE)
    {
        IsTheTrackVideo = SIGN_VIDEO;
        pFileinf->videoTimeScale = sVideoTimeScale;
        pFileinf->videoFlag = TRUE;

        if (sUint32VideoDuration != 0x0)
        {
            pFileinf->uint32VideoDuration = sUint32VideoDuration;
            tempVar = (uint32)sUint32VideoDuration;// * 1000;
            pFileinf->length = (uint32)(tempVar / (uint32)sVideoTimeScale * 1000)
                + (((tempVar % pFileinf->videoTimeScale) * 1000) / sVideoTimeScale);
        }
        else
        {
            if (sUint64VideoDuration.low_part != 0x0 || sUint64VideoDuration.high_part != 0)
            {
                pFileinf->VideoDuration.low_part = sUint64VideoDuration.low_part;
                pFileinf->VideoDuration.high_part = sUint64VideoDuration.high_part;
                tempVar = sUint64VideoDuration.low_part;// * 1000;
                pFileinf->length = (uint32)(tempVar / sVideoTimeScale * 1000)
                    + (((tempVar % pFileinf->videoTimeScale) * 1000) / sVideoTimeScale); /* need to modify after ljf@20100319 */
            }
        }
    }
    else
    {
        if (tag == HANDLER_SOUN)
        {
            if (IsTheTrackVideo == -1)
            {
                pFileinf->audioTimeScale = sVideoTimeScale;

                if (sUint32VideoDuration != 0x0)
                {
                    pFileinf->uint32AudioDuration = sUint32VideoDuration;
                }
                else
                {
                    if (sUint64VideoDuration.low_part != 0x0 || sUint64VideoDuration.high_part != 0)
                    {
                        pFileinf->AudioDuration.low_part = sUint64VideoDuration.low_part;
                        pFileinf->AudioDuration.high_part = sUint64VideoDuration.high_part;
                    }
                }
            }
            IsTheTrackVideo = SIGN_AUDIO;
            pFileinf->audioFlag = TRUE;
        }
        else
        {
            IsTheTrackVideo = SIGN_OTHER;
        }
    }

    MovSTseek(videoFile, boxSize  -FOURBYTE * 5, SEEK_CUR);
    return MOV_FILE_BOX_PARSE_SUCCESS;
}

/********************************************************************************************
*   Func:
*       MovReadMinf()
*   Description:
*
*   Param:
*
*   Return:
*       0:   OK.
*       -1:  Error.
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/

 int MovReadMinf(MOV_ST *videoFile, MovFileinf *pFileinf, LONG boxSize)
{
    uint32  length = 0x0;
    DWORD tag = 0x0;
    uint32 sumLength = 0x0;

    ASSERT((videoFile != NULL)
           && (pFileinf != NULL));

    if (boxSize <= 8)
    {
        return MOV_FILE_BOX_PARSE_SUCCESS;
    }
    else
    {
        for (; ;)
        {
            /* 读取这个box的长度 */
            if (MovSTread((unsigned char *)&length, 1, sizeof(uint32), videoFile) < sizeof(uint32))
            {
               aac_DEBUG();
                return MOV_READ_DATA_FAILED;
            }
            length = BYTESWAP(length);
            sumLength += length;
            if (MovSTread((unsigned char *)&tag, 1, sizeof(DWORD), videoFile) < sizeof(DWORD))
            {
                aac_DEBUG();
                return MOV_READ_DATA_FAILED;
            }

            switch (tag)
            {
            /*case SIGN_VMHD :
                if (MovReadVmhd(videoFile, pFileinf, length))
                {
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

            case SIGN_SMHD :
                if (MovReadSmhd(videoFile, pFileinf, length))
                {
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

            case SIGN_HMHD :
                if (MovReadHmhd(videoFile, pFileinf, length))
                {
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

            case SIGN_NMHD :
                if (MovReadNmhd(videoFile, pFileinf, length))
                {
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

            case SIGN_DINF :
                if (MovReadDinf(videoFile, pFileinf, length))
                {
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;*/

            case SIGN_STBL :
                if (MovReadStbl(videoFile, pFileinf, length))
                {
                    //aac_printf();
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

            default :
                MovSTseek(videoFile, length - (FOURBYTE << 1), SEEK_CUR);
                break;

            }

            if ((sumLength + (FOURBYTE << 1)) >= boxSize)
            {
                return MOV_FILE_BOX_PARSE_SUCCESS;
            }
        }//for
    }//else

    return MOV_FILE_BOX_PARSE_SUCCESS;
}

/********************************************************************************************
*   Func:
*       MovReadTkhd()
*   Description:
*
*   Param:
*
*   Return:
*       0:   OK.
*       -1:  Error.
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/

 int MovReadTkhd(MOV_ST *videoFile, MovFileinf *pFileinf, LONG boxSize)
{
    DWORD vsfl = 0x0;
    WORD  tag  = 0x0;

    ASSERT((videoFile != NULL)
           && (pFileinf != NULL));

    if (MovSTread((unsigned char *)&vsfl, 1, sizeof(DWORD), videoFile) < sizeof(DWORD))
    {
        return MOV_READ_DATA_FAILED;
    }
    vsfl = BYTESWAP(vsfl);

    if (vsfl & 0xff000000)
    {
        MovSTseek(videoFile, FOURBYTE * 11, SEEK_CUR);
    }
    else
    {
        MovSTseek(videoFile, FOURBYTE << 3, SEEK_CUR);
    }

    if (MovSTread((unsigned char *)&tag, 1, sizeof(WORD), videoFile) < sizeof(WORD))
    {
        return MOV_READ_DATA_FAILED;
    }

    if (tag == SIGN_AUDIO)
    {
        IsTheTrackVideo = SIGN_AUDIO;
    }
    else
    {
        IsTheTrackVideo = SIGN_OTHER;
    }

    MovSTseek(videoFile, 46, SEEK_CUR);
    return MOV_FILE_BOX_PARSE_SUCCESS;
}


/********************************************************************************************
*   Func:
*       MovReadMdia()
*   Description:
*
*   Param:
*
*   Return:
*       0:   OK.
*       -1:  Error.
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/

 int MovReadMdia(MOV_ST *videoFile, MovFileinf *pFileinf, LONG boxSize)
{
    uint32  length = 0x0;
    DWORD tag = 0x0;
    uint32 sumLength = 0x0;

    ASSERT((videoFile != NULL)
           && (pFileinf != NULL));

    if (boxSize <= 8)
    {
        return MOV_FILE_BOX_PARSE_SUCCESS;
    }
    else
    {
        for (; ;)
        {
            if (MovSTread((unsigned char *)&length, 1, sizeof(uint32), videoFile) < sizeof(uint32))
            {
                aac_DEBUG();
                return MOV_READ_DATA_FAILED;
            }
            length = BYTESWAP(length);
            sumLength += length;
            if (MovSTread((unsigned char *)&tag, 1, sizeof(DWORD), videoFile) < sizeof(DWORD))
            {
               aac_DEBUG();
                return MOV_READ_DATA_FAILED;
            }

            switch (tag)
            {
            case SIGN_MDHD :
                if (MovReadMdhd(videoFile, pFileinf, length))
                {
                    aac_DEBUG();
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

            case SIGN_HDLR :
                if (MovReadHdlr(videoFile, pFileinf, length))
                {
                    aac_DEBUG();
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

            case SIGN_MINF :
                if (MovReadMinf(videoFile, pFileinf, length))
                {
                    aac_DEBUG();
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

            default :
                MovSTseek(videoFile, length - (FOURBYTE << 1), SEEK_CUR);
                break;

            }

            if ((sumLength + (FOURBYTE << 1)) >= boxSize)
            {
                return MOV_FILE_BOX_PARSE_SUCCESS;
            }
        }//for
    }//else

    return MOV_FILE_BOX_PARSE_SUCCESS;
}


/********************************************************************************************
*   Func:
*       MovReadIpmc()
*   Description:
*
*   Param:
*
*   Return:
*       0:   OK.
*       -1:  Error.
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/
 int MovReadTrak(MOV_ST *videoFile, MovFileinf *pFileinf, LONG boxSize)
{
    uint32  length = 0x0;
    DWORD tag = 0x0;
    uint32 sumLength = 0x0;

    ASSERT((videoFile != NULL)
           && (pFileinf != NULL));

    IsTheTrackVideo = -1;
    sUint32VideoDuration = 0x0;
    sUint64VideoDuration.low_part = 0x0;
    sUint64VideoDuration.high_part = 0x0;
    sVideoTimeScale = 0x1;
    if (boxSize <= 8)
    {
        return MOV_FILE_BOX_PARSE_SUCCESS;
    }
    else
    {
        for (; ;)
        {

            /*read the size of the box */
            if (MovSTread((unsigned char *)&length, 1, sizeof(uint32), videoFile) < sizeof(uint32))
            {
                aac_DEBUG();
                return MOV_READ_DATA_FAILED;
            }
            length = BYTESWAP(length);
            sumLength += length;
            if (MovSTread((unsigned char *)&tag, 1, sizeof(DWORD), videoFile) < sizeof(DWORD))
            {
                aac_DEBUG();
                return MOV_READ_DATA_FAILED;
            }

            switch (tag)
            {
            case SIGN_TKHD :
                if (MovReadTkhd(videoFile, pFileinf, length))
                {
                    aac_DEBUG();
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

                /*case SIGN_TREF :
                    if (MovReadTref(videoFile, pFileinf, length))
                    {
                        return MOV_FILE_BOX_PARSE_ERR;
                    }

                    break;

                case SIGN_EDTS :
                    if (MovReadEdts(videoFile, pFileinf, length))
                    {
                        return MOV_FILE_BOX_PARSE_ERR;
                    }

                    break;*/

            case SIGN_MDIA :
                if (MovReadMdia(videoFile, pFileinf, length))
                {
                    aac_DEBUG();
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

                /*case SIGN_UDTA :
                    if (MovReadUdta(videoFile, pFileinf, length))
                    {
                        return MOV_FILE_BOX_PARSE_ERR;
                    }

                    break;

                case SIGN_META :
                    if (MovReadMeta(videoFile, pFileinf, length))
                    {
                        return MOV_FILE_BOX_PARSE_ERR;
                    }

                    break;*/

            default :
                MovSTseek(videoFile, length - (FOURBYTE << 1), SEEK_CUR);
                break;

            }

            if ((sumLength + (FOURBYTE << 1)) >= boxSize)
            {
                return MOV_FILE_BOX_PARSE_SUCCESS;
            }
        }//for
    }

    return MOV_FILE_BOX_PARSE_SUCCESS;
}

/********************************************************************************************
*   Func:
*       MovReadMoov()
*   Description:
*
*   Param:
*
*   Return:
*       0:   OK.
*       -1:  Error.
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/

 int MovReadMoov(MOV_ST *videoFile, MovFileinf *pFileinf, LONG boxSize)
{
    uint32  length = 0x0;
    DWORD tag = 0x0;
    uint32 sumLength = 0x0;

    ASSERT((videoFile != NULL)
           && (pFileinf != NULL));

    if (boxSize <= 8)
    {
        return MOV_FILE_BOX_PARSE_SUCCESS;
    }
    else
    {
        for (; ;)
        {
            if (MovSTread((unsigned char *)&length, 1, sizeof(uint32), videoFile) < sizeof(uint32))
            {
                aac_DEBUG();
                return MOV_READ_DATA_FAILED;
            }
            length = BYTESWAP(length);
            sumLength += length;
            if (MovSTread((unsigned char *)&tag, 1, sizeof(DWORD), videoFile) < sizeof(DWORD))
            {
                return MOV_READ_DATA_FAILED;
            }

            switch (tag)
            {
            /*case SIGN_MVHD :
                if (MovReadMvhd(videoFile, pFileinf, length))
                {
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

            case SIGN_MVEX :
                if (MovReadMvex(videoFile, pFileinf, length))
                {
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;*/

            case SIGN_TRAK :
                if (MovReadTrak(videoFile, pFileinf, length))
                {
                    aac_DEBUG();
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

                /*case SIGN_IPMC :
                    if (MovReadIpmc(videoFile, pFileinf, length))
                    {
                        return MOV_FILE_BOX_PARSE_ERR;
                    }

                    break;

                case SIGN_UDTA :
                    if (MovReadUdta(videoFile, pFileinf, length))
                    {
                        return MOV_FILE_BOX_PARSE_ERR;
                    }

                    break;

                case SIGN_META :
                    if (MovReadMeta(videoFile, pFileinf, length))
                    {
                        return MOV_FILE_BOX_PARSE_ERR;
                    }

                    break;*/

            default:
                MovSTseek(videoFile, length - (FOURBYTE << 1), SEEK_CUR);
                break;

            }

            if ((sumLength + (FOURBYTE << 1)) >= boxSize)
            {
                //aac_DEBUG();
                return MOV_FILE_BOX_PARSE_SUCCESS;
            }
        }//for
    }//else

    return MOV_FILE_BOX_PARSE_SUCCESS;
}

/********************************************************************************************
*   Func:
*       MovGetAudioSampleSize()
*   Description:
*
*   Param:
*
*   Return:
*
*
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/
STATIC uint32 MovGetAudioSampleSize(MOV_ST *videoFile,
                                    MovFileinf *pFileinf,
                                    uint32 audioSampleNo)
{
    uint32 sampleSize = 0x0;

    ASSERT(pFileinf != NULL);

    if (pFileinf->audioSampleSize == 0x0)
    {
        //sampleSize = sAudioSampleSizeIndex[audioSampleNo - 1];
        sampleSize = MovIndexRead(audioSampleNo, sAudioSampleSizeIndex);
        sampleSize = BYTESWAP(sampleSize);
        return sampleSize;
    }
    else
    {
        return pFileinf->audioSampleSize;
    }
}

/********************************************************************************************
*   Func:
*       MovGetAudioChunkOffset()
*   Description:
*
*   Param:
*
*   Return:
*
*
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/
STATIC uint32 MovGetAudioChunkOffset(MOV_ST *videoFile,
                                     MovFileinf *pFileinf,
                                     uint32 audioChunkNo)
{
    uint32 chunkOffset = 0x0;

    /*chunkOffset = sAudioChunkOffsetIndex[audioChunkNo - 1];*/
    chunkOffset = MovIndexRead(audioChunkNo, sAudioChunkOffsetIndex);
    chunkOffset = BYTESWAP(chunkOffset);
    return chunkOffset;
}



STATIC uint32 MovGetAudioChunkSize(MOV_ST *videoFile,
                                   MovFileinf *pFileinf,
                                   const uint32 *beginSampleNo,
                                   const uint32 *endSampleNo)
{
    uint32 count = 0x0;
    uint32 chunkSize = 0x0;
    uint32 sampleSize = 0x0;

    ASSERT((pFileinf != NULL)
           && (beginSampleNo != NULL)
           && (endSampleNo != NULL));

    if (pFileinf->audioSampleSize == 0x0)
    {
        if (preAudioSampleNo  + 1 == *endSampleNo && preAudioBeginSampleNo == *beginSampleNo)
        {
            sampleSize = MovIndexRead(*endSampleNo, sAudioSampleSizeIndex);
            sampleSize = BYTESWAP(sampleSize);
            chunkSize = preAudioChunkSize + sampleSize;
            goto ret;
        }

        for (count = *beginSampleNo; count <= *endSampleNo; count++)
        {
            /*sampleSize = sAudioSampleSizeIndex[count - 1];*/
            sampleSize = MovIndexRead(count, sAudioSampleSizeIndex);
            sampleSize = BYTESWAP(sampleSize);
            chunkSize += sampleSize;
        }
    }
    else
    {
        chunkSize = (*endSampleNo + 1 - *beginSampleNo) * pFileinf->audioSampleSize;
    }

ret:
    preAudioSampleNo = *endSampleNo;
    preAudioBeginSampleNo = *beginSampleNo;
    preAudioChunkSize = chunkSize;
    return chunkSize;
}



/********************************************************************************************
*   Func:
*       MovAudioGetSampleToChunk()
*   Description:
*
*   Param:
*
*   Return:
*       0:   OK.
*       -1:  Error.
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/
STATIC void MovAudioGetSampleToChunk(MOV_ST *videoFile,
                                     MovFileinf *pFileinf,
                                     uint32 sampleNo,
                                     uint32 *chunkNo,
                                     uint32 *index)
{
    if ((pFileinf->audioSampleNum == pFileinf->audioChunkNum)
        &&(pFileinf->audioSampleToChunkNo == 0x1))
    {
        *chunkNo = sampleNo;
        *index = 1;
    }
    else
    {
        {
            uint32 firstChunkNo = 0x0;
            uint32 samplesPerChunk = 0x0;
            uint32 nextChunkNo = 0x0;
            uint32 nextSamplesPerChunk = 0x0;
            uint32 samplesCount = 0x0;
            uint32 saveChunkNo = 0x0;
            uint32 savesamplesPerChunk = 0x0;
            uint32 loop = 0x1;

            firstChunkNo = sAudioSampleToChunkIndex[((loop - 1) * 0x3)];
            firstChunkNo = BYTESWAP(firstChunkNo);
            samplesPerChunk = sAudioSampleToChunkIndex[((loop - 1) * 0x3) + 0x1];
            samplesPerChunk = BYTESWAP(samplesPerChunk);

            do
            {
                if (loop < pFileinf->audioSampleToChunkNo)
                {
                    loop++;
                    nextChunkNo = sAudioSampleToChunkIndex[((loop - 1) * 0x3)];
                    nextChunkNo = BYTESWAP(nextChunkNo);
                    nextSamplesPerChunk = sAudioSampleToChunkIndex[((loop - 1) * 0x3) + 0x1];
                    nextSamplesPerChunk = BYTESWAP(nextSamplesPerChunk);
                    samplesCount += samplesPerChunk * (nextChunkNo - firstChunkNo);
                    saveChunkNo = firstChunkNo;
                    savesamplesPerChunk = samplesPerChunk;
                    firstChunkNo = nextChunkNo;
                    samplesPerChunk = nextSamplesPerChunk;
                }
                else
                {
                    saveChunkNo = firstChunkNo;
                    savesamplesPerChunk = samplesPerChunk;
                    nextChunkNo = pFileinf->audioChunkNum + 1;
                    samplesCount += samplesPerChunk * (nextChunkNo - firstChunkNo);
                    //break;
                }
            }while (sampleNo > samplesCount);

            *chunkNo = saveChunkNo
                       + ((nextChunkNo - saveChunkNo) - (samplesCount - sampleNo) / savesamplesPerChunk - 1);

            *index = savesamplesPerChunk
                     - (samplesCount - sampleNo) % savesamplesPerChunk;
        }

    }

    return ;
}


/********************************************************************************************
*   Func:
*       MovGetAudioSampleinfChunk()
*   Description:
*
*   Param:
*
*   Return:
*
*
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/
STATIC void MovGetAudioSampleinfChunk(MOV_ST *videoFile,
                                      MovFileinf *pFileinf,
                                      const uint32 *chunkNo,
                                      uint32 *sampleTotal,
                                      uint32 *beginSampleNo,
                                      uint32 *endSampleNo)
{
    if ((pFileinf->audioSampleNum == pFileinf->audioChunkNum)
        && (pFileinf->audioSampleToChunkNo == 0x1))
    {
        *sampleTotal = 0x1;
        *beginSampleNo = *chunkNo;
        *endSampleNo = *chunkNo;
    }
    else
    {
        {
            uint32 firstChunkNo = 0x0;
            uint32 samplesPerChunk = 0x0;
            uint32 nextChunkNo = 0x0;
            uint32 nextSamplesPerChunk = 0x0;
            uint32 samplesCount = 0x0;
            uint32 saveChunkNo = 0x0;
            uint32 savesamplesPerChunk = 0x0;
            uint32 loop = 0x1;

            firstChunkNo = sAudioSampleToChunkIndex[((loop - 1) * 0x3)];
            firstChunkNo = BYTESWAP(firstChunkNo);
            samplesPerChunk = sAudioSampleToChunkIndex[((loop - 1) * 0x3) + 0x1];
            samplesPerChunk = BYTESWAP(samplesPerChunk);

            do
            {
                if (loop < pFileinf->audioSampleToChunkNo)
                {
                    loop++;
                    nextChunkNo = sAudioSampleToChunkIndex[((loop - 1) * 0x3)];
                    nextChunkNo = BYTESWAP(nextChunkNo);
                    nextSamplesPerChunk = sAudioSampleToChunkIndex[((loop - 1) * 0x3) + 0x1];
                    nextSamplesPerChunk = BYTESWAP(nextSamplesPerChunk);
                    samplesCount += samplesPerChunk * (nextChunkNo - firstChunkNo);
                    saveChunkNo = firstChunkNo;
                    savesamplesPerChunk = samplesPerChunk;
                    firstChunkNo = nextChunkNo;
                    samplesPerChunk = nextSamplesPerChunk;
                }
                else
                {
                    saveChunkNo = firstChunkNo;
                    savesamplesPerChunk = samplesPerChunk;
                    nextChunkNo = pFileinf->audioChunkNum + 1;
                    samplesCount += samplesPerChunk * (nextChunkNo - firstChunkNo);
                    firstChunkNo = nextChunkNo;
                }
            }while (*chunkNo > firstChunkNo);

            *sampleTotal = savesamplesPerChunk;
            *beginSampleNo = samplesCount - (firstChunkNo - *chunkNo) * savesamplesPerChunk + 1;
            *endSampleNo = *beginSampleNo + savesamplesPerChunk -1;

        }

    }

    return ;
}



/**
 * Get Current Audio Time
 */
uint32 MovGetAudioSampleTime(MOV_ST *audioFile,
                            MovFileinf *pFileinf,
                            uint32 sampleNo)
{
    uint32 loop = 0x1;
    uint32 sampleCount = 0x0;
    uint32 sampleDelta = 0x0;
    uint32 sampleSum = 0x0;
    uint32 durationSum = 0x0;
    int32 duration = 0x0;

    ASSERT(sampleNo <= pFileinf->audioSampleNum);

    sampleCount = sAudioTimeToSampleIndex[((loop - 0x1) * 0x2)];
    sampleCount = BYTESWAP(sampleCount);
    sampleDelta = sAudioTimeToSampleIndex[((loop - 0x1) * 0x2) + 0x1];
    sampleDelta = BYTESWAP(sampleDelta);
    sampleSum += sampleCount;
    durationSum += sampleCount * sampleDelta;

    while (sampleNo > sampleSum)
    {
        if (loop < pFileinf->audioTimeToSampleNo)
        {
            loop++;
            sampleCount = sAudioTimeToSampleIndex[((loop - 0x1) * 0x2)];
            sampleCount = BYTESWAP(sampleCount);
            sampleDelta = sAudioTimeToSampleIndex[((loop - 0x1) * 0x2) + 0x1];
            sampleDelta = BYTESWAP(sampleDelta);
            sampleSum += sampleCount;
            durationSum += sampleCount * sampleDelta;
        }
        else
        {
            break;
        }
    }

    duration = durationSum - (sampleSum - sampleNo + 1) * sampleDelta;
    if(duration < 0)
    {
      duration = 0;
    }
    return duration;
}

/********************************************************************************************
*   Func:
*       MovGetAudioFrameNoToTime()
*   Description:
*
*   Param:
*
*   Return:
*
*
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/

STATIC uint32 MovGetAudioFrameNoToTime(MOV_ST *videoFile,
                                       MovFileinf *pFileinf,
                                       uint32 duration)
{
    uint32 loop = 0x1;
    uint32 frameNo = 0x0;
    uint32 sampleCount = 0x0;
    uint32 sampleDelta = 0x0;
    uint32 durationSum = 0x0;
    uint32 sampleSum = 0x0;

    if (pFileinf->uint32AudioDuration != 0x0)
    {
        if (duration > pFileinf->uint32AudioDuration)
        {
            //duration = pFileinf->uint32AudioDuration;
            return(pFileinf->audioSampleNum + 0x1);
        }
    }
    else
    {
        if (duration > pFileinf->AudioDuration.low_part) /* need to modify after, ljf@20100319 */
        {
            //duration = pFileinf->uint64AudioDuration;
            return(pFileinf->audioSampleNum + 0x1);
        }
    }

    sampleCount = sAudioTimeToSampleIndex[((loop - 0x1) * 0x2)];
    sampleCount = BYTESWAP(sampleCount);
    sampleDelta = sAudioTimeToSampleIndex[((loop - 0x1) * 0x2) + 0x1];
    sampleDelta = BYTESWAP(sampleDelta);
    sampleSum += sampleCount;
    durationSum += sampleCount * sampleDelta;

    while (duration > durationSum)
    {
        if (loop < pFileinf->audioTimeToSampleNo)
        {
            loop++;
            sampleCount = sAudioTimeToSampleIndex[((loop - 0x1) * 0x2)];
            sampleCount = BYTESWAP(sampleCount);
            sampleDelta = sAudioTimeToSampleIndex[((loop - 0x1) * 0x2) + 0x1];
            sampleDelta = BYTESWAP(sampleDelta);
            sampleSum += sampleCount;
            durationSum += sampleCount * sampleDelta;
        }
        else
        {
            break;
        }
    }

    frameNo = sampleSum - ((durationSum - duration) / sampleDelta) + 1;

    if (frameNo > pFileinf->audioSampleNum)
    {
        frameNo = pFileinf->audioSampleNum;
    }

    return frameNo;
}



/********************************************************************************************
*   Func:
*       MovAudioGetNextSampleInfo()
*   Description:
*
*   Param:
*
*   Return:
*
*
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/
STATIC int MovAudioGetNextSampleInfo(MOV_ST *videoFile, MovFileinf *pFileinf)
{
    uint32 chunkNo = 0x0;
    uint32 chunkOffset = 0x0;
    uint32 index = 0x0;
    uint32 totalSample = 0x0;
    uint32 beginSampleNo = 0x0;
    uint32 endSampleNo = 0x0;
    uint32 size = 0x0;
    uint32 tempVar = 0x0;

    if ((pFileinf->curAudioSampleNo) >= pFileinf->audioSampleNum)
    {
        /* reach the end */
        return -2;
    }

    pFileinf->curAudioSampleNo++;
    pFileinf->audioSample.readSize = 0x0;

    /**
     * ljf@20100415
     * Same To Video
     */
    /*pFileinf->audioSample.curSampleSize
        = MovGetAudioSampleSize(videoFile, pFileinf, pFileinf->curAudioSampleNo);*/

    MovAudioGetSampleToChunk(videoFile,
                             pFileinf,
                             pFileinf->curAudioSampleNo,
                             &chunkNo,
                             &index);

    chunkOffset = MovGetAudioChunkOffset(videoFile, pFileinf, chunkNo);

    MovGetAudioSampleinfChunk(videoFile,
                              pFileinf,
                              &chunkNo,
                              &totalSample,
                              &beginSampleNo,
                              &endSampleNo);

    tempVar = beginSampleNo + index - 2;

    size = MovGetAudioChunkSize(videoFile,
                                pFileinf,
                                &beginSampleNo,
                                &tempVar);

    pFileinf->audioSample.sampleOffset = chunkOffset + size;

    pFileinf->audioSample.curSampleSize
        = MovGetAudioSampleSize(videoFile, pFileinf, pFileinf->curAudioSampleNo);

    return 0;
}

/********************************************************************************************
*   Func:
*       MovGenerateAACFrmHeader()
*   Description:
*
*   Param:
*
*   Return:
*
*
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*  生成adts解码所需的帧头信息
********************************************************************************************/
STATIC int MovGenerateAACFrmHeader(char *pHeader, uint32 FrmLen, uint32 SampleRate, uint32 NumOfChannel)
{
    uint32 len = 0x0;
    uint32 index = 0x0;

    if ((SampleRate > 48000)
        || ((NumOfChannel != 0x1) && (NumOfChannel != 0x2)))
    {
        return 0x1;
    }

    len = FrmLen + 0x7;
    pHeader[0] = (char)0xff;
    pHeader[1] = (char)0xf1;

    switch (SampleRate)
    {
    case 48000:
        index = 0x03;
        break;

    case 44100:
        index = 0x04;
        break;

    case 32000:
        index = 0x05;
        break;

    case 24000:
        index = 0x06;
        break;

    case 22050:
        index = 0x07;
        break;

    case 16000:
        index = 0x08;
        break;

    case 12000:
        index = 0x09;
        break;

    case 11025:
        index = 0x0a;
        break;

    case 8000:
        index = 0x0b;
        break;

    default:
        return 0x1;
    }

    pHeader[2] = ((0x01 << 6) | (index << 2) | 0x00);

    if (NumOfChannel == 2)
    {
        pHeader[3] = (char)0x80;  //双声道？ a=rtpmap:97 mpeg4-generic/44100/2
    }
    else
    {
        pHeader[3] = (char)0x40;    //单声道？ a=rtpmap:97 mpeg4-generic/44800
    }

    pHeader[4] = ((len >> 3) & 0xff);
    pHeader[5] = (((len & 0x07) << 5) | 0x1f);
    pHeader[6] = (char)0xfc;
    return 0;
}

/********************************************************************************************
*   Func:
*       MovGenerateMovFrmHeader()
*   Description:
*
*   Param:
*
*   Return:
*
*
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/
void MovGenerateMovFrmHeader(char *pHeader, uint32 frmLength, uint32 msTime)
{
    pHeader[0] = (char)0x52; //'R'
    pHeader[1] = (char)0x4B; //'K'
    pHeader[2] = (char)0x56; //'V'
    pHeader[3] = (char)0x42; //'B'

    /* the length of video frame */
    pHeader[7] = (char)(frmLength & 0x000000ff);
    pHeader[6] = (char)((frmLength >> 8) & 0x000000ff);
    pHeader[5] = (char)((frmLength >> 16) & 0x000000ff);
    pHeader[4] = (char)((frmLength >> 24) & 0x000000ff);

    /* the time of video frame  */
    pHeader[11] = (char)(msTime & 0x000000ff);
    pHeader[10] = (char)((msTime >> 8) & 0x000000ff);
    pHeader[9] = (char)((msTime >> 16) & 0x000000ff);
    pHeader[8] = (char)((msTime >> 24) & 0x000000ff);
    return;
}





/********************************************************************************************
*   Func:
*       MovReadDescrLen()
*   Description:
*
*   Param:
*
*   Return:
*
*
*   Author:
*       guosl
*   Date:
*       2008-6-16
*   Log:
*
********************************************************************************************/
STATIC int MovReadDescrLen(MOV_ST *videoFile)
{
    int len = 0x0;
    int count = 0x4;
    while (count--)
    {
        BYTE c;
        if (MovSTread(&c, 1, sizeof(BYTE), videoFile) < sizeof(BYTE))
        {
            return MOV_READ_DATA_FAILED;
        }
        len = (len << 0x7) | (c & 0x7f);

        if (!(c & 0x80))
        {
            break;
        }
    }

    return len;
}

/********************************************************************************************
*   Func:
*       MovReadDescr()
*   Description:
*
*   Param:
*
*   Return:
*
*
*   Author:
*       guosl
*   Date:
*       2008-6-16   10:30
*   Log:
*
********************************************************************************************/
STATIC int MovReadDescr(MOV_ST *videoFile, BYTE *tag)
{
    int len;
    if (MovSTread(tag, 1, sizeof(BYTE), videoFile) < sizeof(BYTE))
    {
        return MOV_READ_DATA_FAILED;
    }
    len = MovReadDescrLen(videoFile);
    return len;
}

/********************************************************************************************
*   Func:
*       MovAudioGetSampleDataToBuffer()
*   Description:
*
*   Param:
*
*   Return:
*
*
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/
int MovAudioGetSampleDataToBuffer(MOV_ST *videoFile,
                                  MovFileinf *pFileinf,
                                  char *buffer,
                                  const uint32 *size)
{
    STATIC char sAACFrmHeader[7] = "\0";
    STATIC WORD sIndex = 0x0;
    uint32 remainSize;
    uint32 sizeToGet = *size ;

    if ((gBeingSkip == RESUME)
        || (gBeingSkip == SEEK_TIME))
    {
        MovGenerateAACFrmHeader(sAACFrmHeader,
                                pFileinf->audioSample.curSampleSize,
                                pFileinf->sampleRate,
                                pFileinf->channelCount);
        sIndex = 0x1;
        gBeingSkip = NO_SKIP;
        pFileinf->audioSample.readSize = 0x0;
    }

    while (sizeToGet > 0x0)
    {
        remainSize = pFileinf->audioSample.curSampleSize - pFileinf->audioSample.readSize;

        if (remainSize > 0x0)
        {
            if (sIndex < 0x8)
            {
                if ((0x8 - sIndex) < sizeToGet)
                {
                    {
                        WORD loop = 0x0;
                        WORD tempVar = sIndex - 1;

                        for (loop = sIndex - 1; loop <= 0x6; loop++)
                        {
                            buffer[*size-sizeToGet+loop-tempVar] = sAACFrmHeader[loop];
                        }

                        sizeToGet -= (0x8 - sIndex);
                        sIndex = 0x8;
                    }
                }
                else
                {
                    {
                        WORD loop = 0x0;

                        for (loop = 0x0; loop <= (sizeToGet - 1); loop++)
                        {
                            buffer[*size-sizeToGet+loop] = sAACFrmHeader[loop+sIndex-1];
                        }

                        sIndex += sizeToGet;
                        return *size;
                    }
                }
            }

            if (MovSTtell(videoFile) != pFileinf->audioSample.sampleOffset + pFileinf->audioSample.readSize)
            {
                MovSTseek(videoFile,
                         (long)(pFileinf->audioSample.sampleOffset + pFileinf->audioSample.readSize) - MovSTtell(videoFile), SEEK_CUR);
            }

            if (remainSize < sizeToGet)
            {                   // Read all remain data to SDRAM
                if (MovSTread(buffer+*size-sizeToGet, 1, remainSize, videoFile) < remainSize)
                {
                    return MOV_READ_DATA_FAILED;
                }

                sizeToGet -= remainSize;
                pFileinf->audioSample.readSize = pFileinf->audioSample.curSampleSize;
            }
            else
            {
                if (MovSTread(buffer+*size-sizeToGet, 1, sizeToGet, videoFile) < sizeToGet)
                {
                    return MOV_READ_DATA_FAILED;
                }

                pFileinf->audioSample.readSize += sizeToGet;
                return *size;
            }
        }
        else
        {
            if (MovAudioGetNextSampleInfo(videoFile, pFileinf) != 0x0)
            {
                //return -2;

                if (sizeToGet == *size)
                {
                    return -2;
                }
                else
                {
                    return (*size - sizeToGet);
                }
            }

            MovGenerateAACFrmHeader(sAACFrmHeader,
                                    pFileinf->audioSample.curSampleSize,
                                    pFileinf->sampleRate,
                                    pFileinf->channelCount);

            if (0x7 < sizeToGet)
            {
                {
                    WORD loop = 0x0;

                    for (loop = 0x0; loop <= 0x6; loop++)
                    {
                        buffer[*size-sizeToGet+loop] = sAACFrmHeader[loop];
                    }

                    sizeToGet -= 0x7;
                    sIndex = 0x8;
                }
            }
            else
            {
                {
                    WORD loop = 0x0;

                    for (loop = 0x0; loop <= sizeToGet-1; loop++)
                    {
                        buffer[*size-sizeToGet+loop] = sAACFrmHeader[loop];
                    }

                    sIndex = sizeToGet + 1;

                    return *size;
                }
            }

            if (MovSTtell(videoFile) != pFileinf->audioSample.sampleOffset + pFileinf->audioSample.readSize)
            {
                MovSTseek(videoFile,
                         (long)(pFileinf->audioSample.sampleOffset + pFileinf->audioSample.readSize) - MovSTtell(videoFile), SEEK_CUR);
            }

            if (sizeToGet > pFileinf->audioSample.curSampleSize)
            {
                if (MovSTread(buffer+*size-sizeToGet, 1, pFileinf->audioSample.curSampleSize, videoFile)
                    < pFileinf->audioSample.curSampleSize)
                {
                    return MOV_READ_DATA_FAILED;
                }

                pFileinf->audioSample.readSize = pFileinf->audioSample.curSampleSize;
                sizeToGet -= pFileinf->audioSample.curSampleSize;

            }
            else
            {
                if (MovSTread(buffer+*size-sizeToGet, 1, sizeToGet, videoFile) < sizeToGet)
                {
                    return MOV_READ_DATA_FAILED;
                }

                pFileinf->audioSample.readSize = sizeToGet;
                return *size;
            }
        }//else
    }

    return *size;
}

/********************************************************************************************
*   Func:
*       MovAudioGetChunkDataToBuffer()
*   Description:
*
*   Param:
*
*   Return:
*
*
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/
/*
STATIC int MovAudioGetChunkDataToBuffer(MOV_ST *videoFile,
                                        MovFileinf *pFileinf,
                                        char *buffer,
                                        const uint32 *size)
{
    LONG remainSize;
    LONG sizeToGet = *size;

    //GPIOSetPinLevel(GPIOPortE_Pin1, GPIO_HIGH);

    while (sizeToGet > 0x0)
    {

        remainSize = pFileinf->audioSample.curSampleSize - pFileinf->audioSample.readSize;

        if (remainSize > 0)
        {

            if (MovSTtell(videoFile) != pFileinf->audioSample.sampleOffset + pFileinf->audioSample.readSize)
            {
                MovSTseek(videoFile,
                         (long)(pFileinf->audioSample.sampleOffset + pFileinf->audioSample.readSize) - MovSTtell(videoFile), SEEK_CUR);
            }

            if (remainSize < sizeToGet)
            {                   // Read all remain data to SDRAM
                if (MovSTread(buffer, 1, remainSize, videoFile) < remainSize)
                {
                    return MOV_READ_DATA_FAILED;
                }
                sizeToGet -= remainSize;
                pFileinf->audioSample.readSize = pFileinf->audioSample.curSampleSize;
            }
            else
            {                                           // read size required to SDRAM
                if (MovSTread(buffer, 1, sizeToGet, videoFile) < sizeToGet)
                {
                    return MOV_READ_DATA_FAILED;
                }
                pFileinf->audioSample.readSize += sizeToGet;
                sizeToGet = 0;
                goto ret;
            }
        }
        else
        {
            if (MovAudioGetNextSampleInfo(videoFile, pFileinf) != 0x0)
            {
                //return -2;
                if (sizeToGet == *size)
                {
                    return -2;
                }
                else
                {
                    goto ret;
                }
            }

            if (MovSTtell(videoFile) != pFileinf->audioSample.sampleOffset + pFileinf->audioSample.readSize)
            {
                MovSTseek(videoFile,
                         (long)(pFileinf->audioSample.sampleOffset + pFileinf->audioSample.readSize) - MovSTtell(videoFile), SEEK_CUR);
            }


            if (sizeToGet > pFileinf->audioSample.curSampleSize)
            {
                if (MovSTread(buffer+*size-sizeToGet, 1,  pFileinf->audioSample.curSampleSize, videoFile) < pFileinf->audioSample.curSampleSize)
                {
                    return MOV_READ_DATA_FAILED;
                }

                pFileinf->audioSample.readSize = pFileinf->audioSample.curSampleSize;

                sizeToGet -= pFileinf->audioSample.curSampleSize;
            }
            else
            {
                if (MovSTread(buffer+*size-sizeToGet, 1, sizeToGet, videoFile) < sizeToGet)
                {
                    return MOV_READ_DATA_FAILED;
                }

                pFileinf->audioSample.readSize = sizeToGet;
                sizeToGet = 0;
                goto ret;
            }

        }
    }

ret:
    //GPIOSetPinLevel(GPIOPortE_Pin1, GPIO_LOW);

    return *size - sizeToGet;
}


*/


/********************************************************************************************
*   Func:
*       MovAudioGetDataToSDRAM()
*   Description:
*
*   Param:
*
*   Return:
*
*
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/

#define NANOC 1
#if NANOC
typedef     unsigned char                     HANDLE;

typedef     enum {false = 0, true = !false} BOOL;

#define     bool                            BOOL



extern size_t   (*RKFIO_FOpen)();
extern size_t   (*RKFIO_FRead)(void * /*buffer*/, size_t /*length*/,FILE *) ;
extern int      (*RKFIO_FSeek)(long int /*offset*/, int /*whence*/ , FILE * /*stream*/);
extern long int (*RKFIO_FTell)(FILE * /*stream*/);
extern unsigned long (*RKFIO_FLength)(FILE *in /*stream*/);
extern int      (*RKFIO_FClose)(FILE * /*stream*/);
extern int (*RKFIO_FEof)(FILE *);


FILE * fopen_hl(const char * path,const char * mode)
{
        M4a_Name *p = (M4a_Name *)path;
         return ((FILE*)RKFIO_FOpen((unsigned char *)p->name, p->Clus, p->Index, 1, "R"));
}


unsigned short  fread_hl(void*buffer,size_t size,size_t count,FILE*stream)
    {

        int num  = size*count ;

        if(num >=(1U<<16))
                num = (1u<<16)-1;

        return RKFIO_FRead(buffer,(unsigned short)num,stream);


    }
    unsigned char fseek_hl(FILE *stream, long offset, int fromwhere)
    {

        return RKFIO_FSeek((unsigned int )offset,(unsigned char )fromwhere,stream);


    }
    unsigned  long ftell_hl(FILE *stream)
    {
        return  RKFIO_FTell(stream);

    }

    unsigned char  fclose_hl(FILE *stream)
    {

        return RKFIO_FClose( stream);
    }



#endif
int MovAudioGetDataToSDRAM(MOV_ST *videoFile, MovFileinf *pFileinf, char *buffer, const uint32 *size)
{
    if ((pFileinf->aFormat == AUDIO_CODEC_LIB_SAMR)
        || (pFileinf->aFormat == AUDIO_CODEC_LIB_MP3))
    {
        return 1;
        /*MovAudioGetChunkDataToBuffer(videoFile,
                                            pFileinf,
                                            buffer,
                                            size);*/
    }
    else
    {
        if (pFileinf->aFormat == AUDIO_CODEC_LIB_MP4A)
        {

            return MovAudioGetSampleDataToBuffer(videoFile,
                                                 pFileinf,
                                                 buffer,
                                                 size);
        }
        else
        {
            return -1;
        }
    }

}

/********************************************************************************************
*   Func:
*       MovGlobalVarInit()
*   Description:
*
*   Param:
*
*   Return:
*
*
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/

void MovGlobalVarInit(void)
{
    IsTheTrackVideo = -1;

    gMovFile.curAudioChunkNo = 0x0;
    gMovFile.curVideoChunkNo = 0x0;
    gMovFile.realAudioSampleNo=0x0;

    gMovFile.curAudioSampleNo = 0x0;
    gMovFile.curVideoSampleNo = 0x0;

    gMovFile.audioChunk.readSize = 0x0;
    gMovFile.audioChunk.curChunkSize = 0x0;

    gMovFile.videoChunk.readSize = 0x0;
    gMovFile.videoChunk.curChunkSize = 0x0;

    gMovFile.audioSample.readSize = 0x0;
    gMovFile.audioSample.curSampleSize = 0x0;

    gMovFile.videoSample.readSize = 0x0;
    gMovFile.videoSample.curSampleSize = 0x0;

    gMovFile.audioFlag = FALSE;
    gMovFile.videoFlag = FALSE;

    gMovFile.aFormat = FALSE;
    gMovFile.vFormat = FALSE;

    gMovFile.videoSyncSampleNo = -1;

    gMovFile.uint32AudioDuration = 0x0;
    gMovFile.uint32VideoDuration = 0x0;
    gMovFile.AudioDuration.low_part = 0x0;
    gMovFile.AudioDuration.high_part = 0x0;
    gMovFile.VideoDuration.low_part = 0x0;
    gMovFile.VideoDuration.high_part = 0x0;
    ////aac_printf("movfile =%d\n",sizeof(gMovFile));
    sVideoSampleSizeIndex = NULL;
    sAudioSampleSizeIndex = NULL;
    sVideoSampleToChunkIndex = NULL;
    sAudioSampleToChunkIndex = NULL;
    sVideoChunkOffsetIndex = NULL;
    sAudioChunkOffsetIndex = NULL;
    sVideoTimeToSampleIndex = NULL;
    sAudioTimeToSampleIndex = NULL;
    sVideoSyncSampleIndex = NULL;

    gBeingSkip = NO_SKIP;
    gVideoBeingSkip = NO_SKIP;

    sUint32VideoDuration = 0x0;
    sUint64VideoDuration.low_part = 0x0;
    sUint64VideoDuration.high_part = 0x0;

    sVideoTimeScale = 0x1;
    movLastSeekTime = 0x0;
    gSkipFlag = FALSE;
    //isAudioPlayEnd = FALSE;

    gH264DataBuff = NULL;
    gSodb.sodb_consumed = 0x0;
    gSodb.sodb_length = 0x0;

    preVideoSampleNo = 0;
    preVideoBeginSampleNo = 0;
    preVideoChunkSize = 0;

    preAudioSampleNo = 0;
    preAudioBeginSampleNo = 0;
    preAudioChunkSize = 0;

    sMovFilledBlockNum == 1;
    sameheader=0x0;
    return;
}

/********************************************************************************************
*   Func:
*       MovGlobalVarDspInit()
*   Description:
*
*   Param:
*
*   Return:
*
*
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/
/*
void MovGlobalVarDspInit(void)
{
    if (sVideoSampleSizeIndex != NULL)
    {
        FREE(sVideoSampleSizeIndex);
        sVideoSampleSizeIndex = NULL;
    }

    if (sAudioSampleSizeIndex != NULL)
    {
        FREE(sAudioSampleSizeIndex);
        sAudioSampleSizeIndex = NULL;
    }

    if (sVideoSampleToChunkIndex != NULL)
    {
        FREE(sVideoSampleToChunkIndex);
        sVideoSampleToChunkIndex = NULL;
    }

    if (sAudioSampleToChunkIndex != NULL)
    {
        FREE(sAudioSampleToChunkIndex);
        sAudioSampleToChunkIndex = NULL;
    }

    if (sVideoChunkOffsetIndex != NULL)
    {
        FREE(sVideoChunkOffsetIndex);
        sVideoChunkOffsetIndex = NULL;
    }

    if (sAudioChunkOffsetIndex != NULL)
    {
        FREE(sAudioChunkOffsetIndex);
        sAudioChunkOffsetIndex = NULL;
    }

    if (sVideoTimeToSampleIndex != NULL)
    {
        FREE(sVideoTimeToSampleIndex);
        sVideoTimeToSampleIndex = NULL;
    }

    if (sAudioTimeToSampleIndex != NULL)
    {
        FREE(sAudioTimeToSampleIndex);
        sAudioTimeToSampleIndex = NULL;
    }

    if (sVideoSyncSampleIndex != NULL)
    {
        FREE(sVideoSyncSampleIndex);
        sVideoSyncSampleIndex = NULL;
    }
    sStartAddress = NULL;
}
*/

/********************************************************************************************
*   Func:
*       MovFileInit()
*   Description:
*
*   Param:
*
*   Return:
*       0:   OK.
*       -1:  Error.
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/

extern FILE* pAacFileHandleSize,*pAacFileHandleOffset;


int MovFileInit(FILE* aac_raw_file)
{
    aac_MemSet(&movMem,0,sizeof(movMem));

    if (0 > MovSTopen(&gMovFile.movAudBuf, aac_raw_file))
    {
        return MOV_FILE_OPEN_ERR;
    }

    MovSTseek(&gMovFile.movAudBuf, 0, SEEK_SET);

    if (0 > MovIndexCreate(pAacFileHandleSize, &sAudioSampleSizeIndex))
    {
        return MOV_FILE_OPEN_ERR;
    }

    if (0 > MovIndexCreate(pAacFileHandleOffset, &sAudioChunkOffsetIndex))
    {
        return MOV_FILE_OPEN_ERR;
    }

    return MOV_FILE_OPEN_SUCCESS;
}



/********************************************************************************************
*   Func:
*       MovFileParsing()
*   Description:
*
*   Param:
*
*   Return:
*       0:   OK.
*       -1:  Error.
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/
extern int m4a_filelen;
int data_size = 0;  //解码媒体数据的大小
int MovFileParsing(MOV_ST *videoFile)
{
    uint32 boxSize = 0x0;
    uint64 largeboxSize = 0x0;
    uint32 largeboxSizelow = 0x0;
    uint32 largeboxSizehigh = 0x0;
    DWORD tag = 0x0;
    int rdsize;
    uint8 largesizetag = 0x0;
    uint32 curpos;
    uint32 tagcnt = 0;
    ASSERT(videoFile != NULL);

    MovSTseek(videoFile, 0, SEEK_SET);

    for (; ;)
    {
        /* read the size of the box */
        if (MovSTread((unsigned char *)&boxSize, 1, sizeof(uint32), videoFile) < sizeof(uint32))//if parse to the end
        {
            aac_DEBUG();
            //return MOV_FILE_PARSING_OK;
            goto out;
        }

        boxSize = BYTESWAP(boxSize);

        if (MovSTread((unsigned char *)&tag, 1, sizeof(DWORD), videoFile) < sizeof(DWORD))
        {
            aac_DEBUG();
            //return MOV_READ_DATA_FAILED;
            goto out;
        }

#if 1   //此处是对mdat box的特殊处理，如果boxsize == 1，则是largsize tag，该box的真实size 在tag标签的后面8byte中保存。
        if (tag ==SIGN_MDAT)
        {
            if (boxSize == 0x01)
            {
                /*largebox tag flag*/
                largesizetag = 0x01;

                /*get low 4 bytes in largebox size*/
                if (MovSTread((unsigned char *)&largeboxSizelow, 1, sizeof(uint32), videoFile) < sizeof(uint32))//if parse to the end
                {
                    //return MOV_FILE_PARSING_OK;
                    goto out;
                }
                largeboxSizelow = BYTESWAP(largeboxSizelow);

                /*get high 4 bytes in largebox size*/
                if (MovSTread((unsigned char *)&largeboxSizehigh, 1, sizeof(uint32), videoFile) < sizeof(uint32))//if parse to the end
                {
                    //return MOV_FILE_PARSING_OK;
                    goto out;
                }
                largeboxSizehigh = BYTESWAP(largeboxSizehigh);

                /*get largebox realsize*/
                largeboxSize = (largeboxSizelow << 32) + largeboxSizehigh;
            }

            if (boxSize == 0x00)
            {
                 curpos = MovSTtell(videoFile);
                 boxSize = m4a_filelen - curpos  + 8;
                 data_size += boxSize;
                 MovSTseek(videoFile, boxSize - (FOURBYTE << 1), SEEK_CUR);
                 return MOV_FILE_PARSING_OK;
            }
        }
#endif

        switch (tag)
        {
        /*case SIGN_FTYP :
            if (MovReadFtyp(videoFile, &gMovFile, boxSize))
            {
                return MOV_FILE_BOX_PARSE_ERR;
            }

            break;

        case SIGN_PDIN :
            if (MovReadPdin(videoFile, &gMovFile, boxSize))
            {
                return MOV_FILE_BOX_PARSE_ERR;
            }

            break;*/

        case SIGN_MOOV :
            tagcnt++;
            if (MovReadMoov(videoFile, &gMovFile, boxSize))
            {
                  //aac_DEBUG();
                return MOV_FILE_BOX_PARSE_ERR;
            }
            break;

            /*case SIGN_MOOF :
                if (MovReadMoof(videoFile, &gMovFile, boxSize))
                {
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

            case SIGN_MFRA :
                if (MovReadMfra(videoFile, &gMovFile, boxSize))
                {
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

            case SIGN_FREE :
                if (MovReadFree(videoFile, &gMovFile, boxSize))
                {
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

            case SIGN_MDAT :
                if (MovReadMdat(videoFile, &gMovFile, boxSize))
                {
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

            case SIGN_SKIP :
                if (MovReadSkip(videoFile, &gMovFile, boxSize))
                {
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

            case SIGN_META :
                if (MovReadMeta(videoFile, &gMovFile, boxSize))
                {
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;*/

        default :
            if ((tag == SIGN_FTYP) || (tag == SIGN_PDIN) || (tag == SIGN_MOOF)
                || (tag == SIGN_MFRA) || (tag == SIGN_FREE) || (tag == SIGN_MDAT)
                || (tag == SIGN_SKIP) || (tag == SIGN_META) || (tag == SIGN_WIDE)
                || (tag == SIGN_UDTA) || (tag == SIGN_UUID))
            {
                tagcnt++;
                if(tag == SIGN_MDAT && largesizetag == 0x01)
                {
                    //large size box has 16 bytes
                    MovSTseek(videoFile, largeboxSize - (FOURBYTE << 2), SEEK_CUR);
                }
                else
                {
                    MovSTseek(videoFile, boxSize - (FOURBYTE << 1), SEEK_CUR);
                }

                if(tag == SIGN_MDAT)
                {
                    //此处累加boxsize是为了防止多个mdat tag时的解码错误问题。
                    if (largesizetag == 0x01)
                    {
                        data_size += largeboxSize;
                    }
                    else
                    {
                        data_size += boxSize;
                    }
                }
            }
            else
            {
                goto out;
                #if 0
                uint32 curpos = MovSTtell(videoFile);
                if((m4a_filelen - curpos )<512)
                {
                    aac_printf("非正常结束\n");
                    return  MOV_FILE_PARSING_OK;
                }
                else
                {
                    aac_printf("0x%x pos unkown box",curpos);
                    return MOV_FILE_BOX_PARSE_ERR;
                }
                #endif
            }
            break;
        }
    }
out:
    if (tagcnt > 0)
    {
        //aac_DEBUG("tagcnt = %d", tagcnt);
        return  MOV_FILE_PARSING_OK;
    }
    else
    {
        return MOV_FILE_BOX_PARSE_ERR;
    }
}


/********************************************************************************************
*   Func:
*       MovFileClose()
*   Description:
*
*   Param:
*
*   Return:
*
*
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/

void MovFileClose(void)
{
   // MovSTclose(&gMovFile.movVidBuf);
    MovSTclose(&gMovFile.movAudBuf);

    //MovIndexDestroy(sVideoSampleSizeIndex);
    MovIndexDestroy(sAudioSampleSizeIndex);

    //MovIndexDestroy(sVideoChunkOffsetIndex);
    MovIndexDestroy(sAudioChunkOffsetIndex);
}

/********************************************************************************************
*   Func:
*       MovGetAudioFormat()
*   Description:
*
*   Param:
*
*   Return:
*       0:   OK.
*       -1:  Error.
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/
int MovGetAudioFormat(void)
{
    if (gMovFile.audioFlag == FALSE)
    {
        return -1;
    }
    else
    {
        if (gMovFile.aFormat != 0x0)
        {
            return gMovFile.aFormat;
        }
        else
        {
            return -1;
        }
    }
}


/********************************************************************************************
*   Func:
*       MovAudioSeek()
*   Description:
*
*   Param:
*
*   Return:
*       0:   OK.
*       -1:  Error.
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/

int MovAudioSeek(MOV_ST *videoFile, long offset, int origin)
{
    if ((gMovFile.aFormat == AUDIO_CODEC_LIB_MP4A)
        && (gBeingSkip == NO_SKIP))
    {
        gMovFile.curAudioSampleNo = 0x0;
        gMovFile.realAudioSampleNo=0x0;
        gMovFile.audioSample.curSampleSize = 0x0;
        gMovFile.audioSample.readSize = 0x0;
    }
    else
    {
        if (((gMovFile.aFormat == AUDIO_CODEC_LIB_SAMR)
             || (gMovFile.aFormat == AUDIO_CODEC_LIB_MP3))
            && (gBeingSkip == NO_SKIP))
        {
            gMovFile.curAudioSampleNo = 0x0;
            gMovFile.realAudioSampleNo=0x0;
            gMovFile.audioSample.curSampleSize = 0x0;
            gMovFile.audioSample.readSize = 0x0;
        }
    }

    MovSTseek(videoFile, offset, origin);

    return 0;
}

/********************************************************************************************
*   Func:
*       MovSynAudio2Video()
*   Description:
*
*   Param:
*
*   Return:
*       0:   OK.
*       -1:  Error.
*   Author:
*       guosl
*   Date:
*       2008-5-16   10:30
*   Log:
*
********************************************************************************************/
void MovSynAudio2Video(long videoSampleNo)
{
    uint32 audioDuration = 0x0;
    uint32 audioFrameNo = 0x0;
    uint32 videoDuration = 0x0;
    uint32 chunkNo = 0x0;
    uint32 index = 0x0;
    uint32 chunkOffset = 0x0;
    uint32 totalSample = 0x0;
    uint32 beginSampleNo = 0x0;
    uint32 endSampleNo = 0x0;
    uint32 size = 0x0;
    uint32 tempVar = 0x0;
    uint32 msTime = 0x0;


    if (videoSampleNo < gMovFile.videoSampleNum)
    {
    #ifndef ONLY_AUDIO
        videoDuration = MovGetVideoDurationToFrameNo(&gMovFile.movVidBuf, &gMovFile, (videoSampleNo + 0x1));
    #endif
    }
    else
    {
  #ifndef ONLY_AUDIO
        videoDuration = MovGetVideoDurationToFrameNo(&gMovFile.movVidBuf, &gMovFile, videoSampleNo);
    #endif
    }

    //temp = (uint64)(videoDuration) * (uint64)(gMovFile.audioTimeScale);
    msTime = ((videoDuration / gMovFile.videoTimeScale) * 1000) +
             (((videoDuration % gMovFile.videoTimeScale) * 1000) / gMovFile.videoTimeScale);
    if (gMovFile.audioFlag == FALSE)
    {
        movTimerCount = msTime;
        return;
    }

    //audioDuration = temp /(uint64)gMovFile.videoTimeScale + temp % (uint32)gMovFile.videoTimeScale;

    audioDuration = ((msTime / 1000) * gMovFile.audioTimeScale) +
                    (((msTime % 1000) * gMovFile.audioTimeScale) / 1000);

    if (gMovFile.aFormat == AUDIO_CODEC_LIB_MP4A)
    {
        audioFrameNo = MovGetAudioFrameNoToTime(&gMovFile.movAudBuf, &gMovFile, audioDuration);

        /* stop the audio */
        if (audioFrameNo == (gMovFile.audioSampleNum + 0x1))
        {
            gMovFile.curAudioSampleNo = audioFrameNo;
            gMovFile.realAudioSampleNo = audioFrameNo;
            gMovFile.audioSample.readSize = 0x0;
            gMovFile.audioSample.curSampleSize = 0x0;
            //VideoAudioStop(0x0);
            movTimerCount = msTime;
            //MovSetAudioPlayEnd();
        }
        else
        {
            /* reboot the codec of audio */
            if (0/*isAudioPlayEnd == TRUE*/)
            {
                gMovFile.curAudioSampleNo = 0x0;
                gMovFile.realAudioSampleNo = 0x0;
                gMovFile.audioSample.readSize = 0x0;
                gMovFile.audioSample.curSampleSize = 0x0;
                //VideoAudioStop(0x0);
                //MovAudioStart();
            }

            gMovFile.curAudioSampleNo = audioFrameNo;
            gMovFile.realAudioSampleNo = audioFrameNo;
            gMovFile.audioSample.readSize = 0x0;

            MovAudioGetSampleToChunk(&gMovFile.movAudBuf,
                                     &gMovFile,
                                     gMovFile.curAudioSampleNo,
                                     &chunkNo,
                                     &index);

            chunkOffset = MovGetAudioChunkOffset(&gMovFile.movAudBuf, &gMovFile, chunkNo);

            MovGetAudioSampleinfChunk(&gMovFile.movAudBuf,
                                      &gMovFile,
                                      &chunkNo,
                                      &totalSample,
                                      &beginSampleNo,
                                      &endSampleNo);

            tempVar = beginSampleNo + index - 2;
            size = MovGetAudioChunkSize(&gMovFile.movAudBuf,
                                        &gMovFile,
                                        &beginSampleNo,
                                        &tempVar);


            gMovFile.audioSample.sampleOffset = chunkOffset + size;

            gMovFile.audioSample.curSampleSize = MovGetAudioSampleSize(&gMovFile.movAudBuf,
                                                                       &gMovFile,
                                                                       gMovFile.curAudioSampleNo);

            msTime = ((audioDuration / gMovFile.audioTimeScale) * 1000) +
                     (((audioDuration % gMovFile.audioTimeScale) * 1000) / gMovFile.audioTimeScale);

            movTimerCount = msTime;
            //MovAudioSeekTime(msTime);
            //MovSetAudioPlayNoEnd();
        }/*end if (gMovFile.curAudioSampleNo == (gMovFile.audioSampleNum + 0x1))*/
    }
    else
    {
        if ((gMovFile.aFormat == AUDIO_CODEC_LIB_SAMR)
            || (gMovFile.aFormat == AUDIO_CODEC_LIB_MP3))
        {
            audioFrameNo = MovGetAudioFrameNoToTime(&gMovFile.movVidBuf,
                                                    &gMovFile,
                                                    audioDuration);

            /* the audio end */
            if (audioFrameNo == (gMovFile.audioSampleNum + 0x1))
            {
                gMovFile.curAudioSampleNo = audioFrameNo;
                gMovFile.realAudioSampleNo = audioFrameNo;
                gMovFile.audioSample.readSize = 0x0;
                gMovFile.audioSample.curSampleSize = 0x0;
                //VideoAudioStop(0x0);
                movTimerCount = msTime;
                //MovSetAudioPlayEnd();
            }
            else
            {
                /* reboot the codec of audio */
                if (/*isAudioPlayEnd == TRUE*/0)
                {
                    gMovFile.curAudioSampleNo = 0x0;
                    gMovFile.realAudioSampleNo = 0x0;
                    gMovFile.audioSample.readSize = 0x0;
                    gMovFile.audioSample.curSampleSize = 0x0;
                    //MovAudioStart();
                }

                msTime = ((audioDuration / gMovFile.audioTimeScale) * 1000) +
                         (((audioDuration % gMovFile.audioTimeScale) * 1000) / gMovFile.audioTimeScale);

                movTimerCount = msTime;
                //MovAudioSeekTime(msTime);
                gMovFile.curAudioSampleNo = audioFrameNo;
                gMovFile.realAudioSampleNo = audioFrameNo;
                gMovFile.audioSample.readSize = 0x0;

                MovAudioGetSampleToChunk(&gMovFile.movAudBuf,
                                         &gMovFile,
                                         gMovFile.curAudioSampleNo,
                                         &chunkNo,
                                         &index);

                chunkOffset = MovGetAudioChunkOffset(&gMovFile.movVidBuf, &gMovFile, chunkNo);

                MovGetAudioSampleinfChunk(&gMovFile.movAudBuf,
                                          &gMovFile,
                                          &chunkNo,
                                          &totalSample,
                                          &beginSampleNo,
                                          &endSampleNo);

                tempVar = beginSampleNo + index - 2;
                size = MovGetAudioChunkSize(&gMovFile.movAudBuf,
                                            &gMovFile,
                                            &beginSampleNo,
                                            &tempVar);


                gMovFile.audioSample.sampleOffset = chunkOffset + size;

                gMovFile.audioSample.curSampleSize = MovGetAudioSampleSize(&gMovFile.movAudBuf,
                                                                           &gMovFile,
                                                                           gMovFile.curAudioSampleNo);
                //MovSetAudioPlayNoEnd();
            }

        }
    }

    return;
}

/**
 * ljf@20100416
 * Mov Audio Seek To Time
 */
int MovAudioSeekToTime(uint32 timems)
{
    uint32 audioDuration = 0x0;
    uint32 audioFrameNo = 0x0;
    uint32 videoDuration = 0x0;
    uint32 chunkNo = 0x0;
    uint32 index = 0x0;
    uint32 chunkOffset = 0x0;
    uint32 totalSample = 0x0;
    uint32 beginSampleNo = 0x0;
    uint32 endSampleNo = 0x0;
    uint32 size = 0x0;
    uint32 tempVar = 0x0;
    uint32 msTime = 0x0;

    audioDuration = ((timems / 1000) * gMovFile.audioTimeScale) +
                    (((timems % 1000) * gMovFile.audioTimeScale) / 1000);

    if (gMovFile.aFormat == AUDIO_CODEC_LIB_MP4A)
    {
        audioFrameNo = MovGetAudioFrameNoToTime(&gMovFile.movAudBuf, &gMovFile, audioDuration);

        /* stop the audio */
        if (audioFrameNo == (gMovFile.audioSampleNum + 0x1))
        {
            gMovFile.curAudioSampleNo = audioFrameNo;
            gMovFile.realAudioSampleNo = audioFrameNo;
            gMovFile.audioSample.readSize = 0x0;
            gMovFile.audioSample.curSampleSize = 0x0;
            //VideoAudioStop(0x0);
            movTimerCount = timems;
            //MovSetAudioPlayEnd();
        }
        else
        {
            /* reboot the codec of audio */
            if (0/*isAudioPlayEnd == TRUE*/)
            {
                gMovFile.curAudioSampleNo = 0x0;
                gMovFile.realAudioSampleNo = 0x0;
                gMovFile.audioSample.readSize = 0x0;
                gMovFile.audioSample.curSampleSize = 0x0;
                //VideoAudioStop(0x0);
                //MovAudioStart();
            }

            gMovFile.curAudioSampleNo = audioFrameNo;
            gMovFile.realAudioSampleNo = audioFrameNo;
            gMovFile.audioSample.readSize = 0x0;

            MovAudioGetSampleToChunk(&gMovFile.movAudBuf,
                                     &gMovFile,
                                     gMovFile.curAudioSampleNo,
                                     &chunkNo,
                                     &index);

            chunkOffset = MovGetAudioChunkOffset(&gMovFile.movAudBuf, &gMovFile, chunkNo);

            MovGetAudioSampleinfChunk(&gMovFile.movAudBuf,
                                      &gMovFile,
                                      &chunkNo,
                                      &totalSample,
                                      &beginSampleNo,
                                      &endSampleNo);

            tempVar = beginSampleNo + index - 2;
            size = MovGetAudioChunkSize(&gMovFile.movAudBuf,
                                        &gMovFile,
                                        &beginSampleNo,
                                        &tempVar);


            gMovFile.audioSample.sampleOffset = chunkOffset + size;

            gMovFile.audioSample.curSampleSize = MovGetAudioSampleSize(&gMovFile.movAudBuf,
                                                                       &gMovFile,
                                                                       gMovFile.curAudioSampleNo);

            msTime = ((audioDuration / gMovFile.audioTimeScale) * 1000) +
                     (((audioDuration % gMovFile.audioTimeScale) * 1000) / gMovFile.audioTimeScale);

            movTimerCount = msTime;
            //MovAudioSeekTime(msTime);
            //MovSetAudioPlayNoEnd();
        }/*end if (gMovFile.curAudioSampleNo == (gMovFile.audioSampleNum + 0x1))*/
    }
    else
    {
        if ((gMovFile.aFormat == AUDIO_CODEC_LIB_SAMR)
            || (gMovFile.aFormat == AUDIO_CODEC_LIB_MP3))
        {
            audioFrameNo = MovGetAudioFrameNoToTime(&gMovFile.movAudBuf,
                                                    &gMovFile,
                                                    audioDuration);

            /* the audio end */
            if (audioFrameNo == (gMovFile.audioSampleNum + 0x1))
            {
                gMovFile.curAudioSampleNo = audioFrameNo;
                gMovFile.realAudioSampleNo = audioFrameNo;
                gMovFile.audioSample.readSize = 0x0;
                gMovFile.audioSample.curSampleSize = 0x0;
                //VideoAudioStop(0x0);
                movTimerCount = msTime;
                //MovSetAudioPlayEnd();
            }
            else
            {
            #if 0
                /* reboot the codec of audio */
                if (/*isAudioPlayEnd == TRUE*/0)
                {
                    gMovFile.curAudioSampleNo = 0x0;
                    gMovFile.audioSample.readSize = 0x0;
                    gMovFile.audioSample.curSampleSize = 0x0;
                    //MovAudioStart();
                }

                msTime = ((audioDuration / gMovFile.audioTimeScale) * 1000) +
                         (((audioDuration % gMovFile.audioTimeScale) * 1000) / gMovFile.audioTimeScale);

                movTimerCount = msTime;
                //MovAudioSeekTime(msTime);
                gMovFile.curAudioSampleNo = audioFrameNo;
                gMovFile.realAudioSampleNo = audioFrameNo;
                gMovFile.audioSample.readSize = 0x0;

                MovAudioGetSampleToChunk(&gMovFile.movAudBuf,
                                         &gMovFile,
                                         gMovFile.curAudioSampleNo,
                                         &chunkNo,
                                         &index);

                chunkOffset = MovGetAudioChunkOffset(&gMovFile.movAudBuf, &gMovFile, chunkNo);

                MovGetAudioSampleinfChunk(&gMovFile.movAudBuf,
                                          &gMovFile,
                                          &chunkNo,
                                          &totalSample,
                                          &beginSampleNo,
                                          &endSampleNo);

                tempVar = beginSampleNo + index - 2;
                size = MovGetAudioChunkSize(&gMovFile.movAudBuf,
                                            &gMovFile,
                                            &beginSampleNo,
                                            &tempVar);


                gMovFile.audioSample.sampleOffset = chunkOffset + size;

                gMovFile.audioSample.curSampleSize = MovGetAudioSampleSize(&gMovFile.movAudBuf,
                                                                           &gMovFile,
                                                                           gMovFile.curAudioSampleNo);
                //MovSetAudioPlayNoEnd();
                #endif
            }

        }
    }

    return 0;
}





int GetFormat(unsigned char *buf)
{
    if((buf[0]==0xff)&&((buf[1]>>4)&0xf==0xf))
    {
        return 0;
    }
    else if(buf[4]=='f'&&buf[5]=='t'&&buf[6]=='y'&&buf[7]=='p')
    {
        return 1;
    }

    return -1;
}
/**
 * Following Functions are Mov Parser Interface.
 */

int MovIF_close(void)
{
    MovFileClose();
    return 0;
}


int MovIF_AudioGetStream(char *buff, int size, int mode)
{
    int ndsize = size;
    int rdsize = 0;

    //aac_printf(" gMovFile.aFormat = %d\n",gMovFile.aFormat);

    if (movaudfirst && gMovFile.aFormat == AUDIO_CODEC_LIB_SAMR)
    {
        movaudfirst = 0;
        *buff++ = '#';
        *buff++ = '!';
        *buff++ = 'A';
        *buff++ = 'M';
        *buff++ = 'R';
        *buff++ = (char)0x0A;

        ndsize -= 6;
        rdsize += 6;
    }

    rdsize += MovAudioGetDataToSDRAM(&gMovFile.movAudBuf, &gMovFile, buff, &ndsize);

    return rdsize > 0 ? rdsize : 0;
}

int MovIF_AudioFSeek(long offset, int origin)
{
    return MovAudioSeek(&gMovFile.movAudBuf, offset, origin);
}

int MovIF_SynAudio2Video(unsigned int timems)
{
    MovAudioSeekToTime(timems);
    gBeingSkip = RESUME;
    return 0;
}

unsigned int MovIF_AudioGetCurrentTime(void)
{
    unsigned long duration;
    unsigned long msTime;
    uint32 estTime;

    duration = MovGetAudioSampleTime(&gMovFile.movAudBuf, &gMovFile, gMovFile.realAudioSampleNo);
    msTime = (unsigned long)(((unsigned long long)duration / gMovFile.audioTimeScale) * 1000)
        + (((unsigned long long)duration % gMovFile.audioTimeScale * 1000) / gMovFile.audioTimeScale);

    if (gMovFile.aFormat == AUDIO_CODEC_LIB_MP4A)
    {
        estTime = 0;
    }
    else
    {
        estTime = 180;
    }
    return msTime > estTime ? msTime - estTime : msTime;
}


#pragma arm section code

/*********************************************************************************************************
**                            End Of File
********************************************************************************************************/

#endif
#endif
