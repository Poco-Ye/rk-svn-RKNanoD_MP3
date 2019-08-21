#include "SysInclude.h"
#include "audio_main.h"

#ifdef A_CORE_DECODE
#ifdef HIFI_AlAC_DECODE
#pragma arm section code = "AlacHDecCode", rodata = "AlacHDecCode", rwdata = "AlacHDecData", zidata = "AlacHDecBss"
#define _IN_MOVFILE_H
#define ASSERT(expr) 
#ifndef BOARD
    #include <stdio.h>
  //  #include <memory.h>
    #include <stdlib.h>

#endif
#include "alac.h"


#include "hifi_alac_MovFile.h"
//#include "movparserinterface.h"
#include "audio_file_access.h"
#ifdef BOARD
#define abs     Abs_h
#else
#include <math.h>
#endif
extern   unsigned char alac_info[];
#define movMain     main

//static uint32 preVideoSampleNo = 0;
//static uint32 preVideoBeginSampleNo = 0;
//static uint32 preVideoChunkSize = 0;

static uint32 preAudioSampleNo = 0;
static uint32 preAudioBeginSampleNo = 0;
static uint32 preAudioChunkSize = 0;

STATIC uint32 sVideoTimeScale = 0x1;
STATIC uint32 sUint32VideoDuration = 0x0;
STATIC MovDuration sUint64VideoDuration = {0x0, 0x0};

MovFileinf gMovFile_h;

//uint8 *gH264DataBuff;
DWORD IsTheTrackVideo_h = -1;
SKIPSTATE gBeingSkip_h = NO_SKIP;
//SKIPSTATE gVideoBeingSkip = NO_SKIP;

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
const int sampRateTab_h[12] = {
    96000, 88200, 64000, 48000, 44100, 32000, 
    24000, 22050, 16000, 12000, 11025,  8000
};
const int channelMapTab_h[8] = {
    -1, 1, 2, 3, 4, 5, 6, 8
};

#endif


uint32 movTimerCount_h;
/*
SODB gSodb;
uint32 *sStartAddress;
EXT uint32 *sEndAddress;
int movLastSeekTime;
int gSkipFlag;

LONG  sMovFilledBlockNum;
uint32 skipToSampleNo;

int movaudfirst = 1;
int movvidfirst = 1;*/

MovMem movMem_h;

void MovMemInit_h(void)
{
    movMem_h.sizeused = 0;
}

int MovMemLeft_h(void)
{
    return MOV_MEM_SIZE - movMem_h.sizeused;
}

void* MovMemAlloc_h(int size)
{
    void *ret;

    if (size > MovMemLeft_h() || size <= 0)
    {
        return 0;
    }

    ret = movMem_h.buf + movMem_h.sizeused;
    movMem_h.sizeused += size + (4 - size & 0x3); /* 4 bytes align */

    return ret;
}

void MovMemFree_h(void *mem)
{
 
}


int movBufInit_h(MovBuf *movbuf, FILE *file)
{
    if (file == (FILE *)-1 || movbuf == NULL)
    {
        return -1;
    }
    movbuf->rdmax = movbuf->rdpos = 0;
    movbuf->st = file;

    return 0;
}

void movBufdeinit_h(MovBuf *movbuf)
{
    if (movbuf)
    {
        if (movbuf->st)
        {
        
            movbuf->st = NULL;
        }

        movbuf->rdmax = movbuf->rdpos = 0;
    }
}

int movBufLeft_h(MovBuf *movbuf)
{
    return movbuf->rdmax - movbuf->rdpos;
}

long movBufTell_h(MovBuf *movbuf)
{
    return MovTell(movbuf->st) - movBufLeft_h(movbuf);
}

int movBufSeek_h(MovBuf *movbuf, int seeksize, int mode)
{
    int fseeksize = 0;
    if (SEEK_CUR == mode)
    {
        if (seeksize == 0)
        {
            return 0;
        }

        if ((seeksize > 0 && abs(seeksize) > movBufLeft_h(movbuf)) 
            || (seeksize < 0 && abs(seeksize) > movbuf->rdpos))
        {
            fseeksize = seeksize - movBufLeft_h(movbuf);
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

int movBufRead_h(uint8 *buf, int size, int cnt, MovBuf *movbuf)
{
    int rdsize = 0;
    int ndsize = size * cnt;
    int rdfsize = 0;

    ASSERT(buf);

LOOP:
    rdsize = ndsize > movBufLeft_h(movbuf) ? movBufLeft_h(movbuf) : ndsize;

    Hifi_Alac_Memcpy(buf, movbuf->buf + movbuf->rdpos, rdsize);
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

int MovIndexCreate_h(FILE *aac_raw_file, MovIndex **index)
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

void MovIndexDestroy_h(MovIndex *index)
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

void MovIndexInit_h(int indexpos, MovIndex *index)
{
    if (index == NULL)
    {
        return;
    }
    
    index->indexpos = indexpos;
    MovSTseek(index->indexbuf, indexpos, SEEK_SET);
    index->curpos = 0;
}

uint32 MovIndexRead_h(int indexno, MovIndex *index)
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
long Abs_h(long num)
{
    return num < 0 ? 0-num : num;
}

void movFillEndData_h(char *buf, int size)
{
    int fillsize = size & 0x3;
    char pHeader[32];

    while (fillsize-- > 0)
    {
        *(buf++) = 0;
        size--;
    }

    MovGenerateMovFrmHeader_h(pHeader, 0, 0);

    if (VIDEO_CODEC_LIB_MP4V == gMovFile_h.vFormat)
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
        Hifi_Alac_Memcpy(buf, pHeader, 32);
        size -= 32;
        buf += 32;
    }
}
/* add by wujiangrui for analysize ESDS box  */
   int   read_byte_h=0;
  int read_mp4_descr_length_h(MOV_ST *videoFile)
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
     read_byte_h+=numBytes;
     return length;
 }
 int MovReadESDS_h(MOV_ST *videoFile, int *sampleRate_config,int *channel_count_config)
{
    char temp_tag = 0x0;
    int temp_len;
    int channel_configuration_index;
    int samplerate_index;
    char buf1,buf2;
     read_byte_h=0;
    MovSTseek(videoFile, FOURBYTE, SEEK_CUR);
    read_byte_h +=FOURBYTE;
    if (MovSTread((unsigned char *)&temp_tag, 1, sizeof(char), videoFile) < sizeof(char))
    {
        return MOV_READ_DATA_FAILED;
    }
    read_byte_h+=1;
    if(temp_tag == 0x03)
    {
        temp_len = read_mp4_descr_length_h(videoFile);
        
        MovSTseek(videoFile, 0x3, SEEK_CUR);
        read_byte_h+=3;
    }
    else
    {
        MovSTseek(videoFile, 0x2, SEEK_CUR);
        read_byte_h+=2;
    }
    if (MovSTread((unsigned char *)&temp_tag, 1, sizeof(char), videoFile) < sizeof(char))
    {
        return MOV_READ_DATA_FAILED;
    }
    read_byte_h+=1;
    if(temp_tag != 0x04)
    {
        return MOV_UNSUPPORTED_STREAM;
    }
    temp_len = read_mp4_descr_length_h( videoFile);
    if(temp_len < 13)
    {
        return MOV_UNSUPPORTED_STREAM;
    }
    MovSTseek(videoFile, 13, SEEK_CUR);//可获得最大比特率及平均比特率
    read_byte_h+=13;
    if (MovSTread((unsigned char *)&temp_tag, 1, sizeof(char), videoFile) < sizeof(char))
    {
        return MOV_READ_DATA_FAILED;
    }
        read_byte_h+=1;
    if(temp_tag != 0x05)
    {
        return MOV_UNSUPPORTED_STREAM;
    }
    temp_len = read_mp4_descr_length_h( videoFile);
    
    if (MovSTread((unsigned char *)&buf1, 1, sizeof(char), videoFile) < sizeof(char))
    {
        return MOV_READ_DATA_FAILED;
    }
        read_byte_h+=1;
    if (MovSTread((unsigned char *)&buf2, 1, sizeof(char), videoFile) < sizeof(char))
    {
        return MOV_READ_DATA_FAILED;
    }
    read_byte_h+=1;
    samplerate_index = ((buf1 & 0x7)<<1)|(buf2 >>7);
    channel_configuration_index = (buf2 >> 3)& 0xf;
    *sampleRate_config = sampRateTab_h[samplerate_index];
    *channel_count_config = channelMapTab_h[channel_configuration_index];
     MovSTseek(videoFile, -read_byte_h, SEEK_CUR);
    return MOV_FILE_BOX_PARSE_SUCCESS ;
    
}

/************end**************/

 int MovReadStsd_h(MOV_ST *videoFile, MovFileinf *pFileinf, LONG boxSize)
{
    int sampleRate_config, channel_count_config;
    ASSERT((videoFile != NULL)
           && (pFileinf != NULL));

    if (IsTheTrackVideo_h == SIGN_OTHER)
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

            if (IsTheTrackVideo_h == SIGN_VIDEO)
            {
                switch (codeTag)
                {
                case VIDEOCODE_MP4V :
                    pFileinf->vFormat = VIDEO_CODEC_LIB_MP4V;
                   strncpy(gMovFile_h.VideoDec, "M4V",4);
                    break;

                case VIDEOCODE_S263 :
                case VIDEOCODE_H263 :
                    pFileinf->vFormat = VIDEO_CODEC_LIB_H263;
                    strncpy(gMovFile_h.VideoDec, "263",4);
                    break;

                case VIDEOCODE_AVC1 :
                    pFileinf->vFormat = VIDEO_CODEC_LIB_H264;
                    strncpy(gMovFile_h.VideoDec, "264",4);
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
                        return MOV_FILE_RESOLUTION_UNSUPPORTED ;
                    }
                }
                else
                {
                    if ((pFileinf->width > MOV_MAX_FRAME_WIDTH)
                        || (pFileinf->height > MOV_MAX_FRAME_HEIGHT))
                    {
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
                    strncpy(gMovFile_h.AudioDec, "AMR",16);
                    break;

                case AUDIOCODE_MP4A :
                    pFileinf->aFormat = AUDIO_CODEC_LIB_MP4A;
                    strncpy(gMovFile_h.AudioDec, "VAC",16);
                    break;

                case AUDIOCODE_MP3:
                    pFileinf->aFormat = AUDIO_CODEC_LIB_MP3;
                    strncpy(gMovFile_h.AudioDec, "VM3",16);
                    break;
                case AUDIOCODE_ALAC:
                    pFileinf->aFormat = AUDIO_CODEC_LIB_ALAC;
                    strncpy(gMovFile_h.AudioDec, "VAC",16);//alac
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
            
                #if 1 /*add by jiangrui.wu  for analysise ESDS*/    
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

                    if (tag == SIGN_ALAC)
                    {   MovSTseek(videoFile, -(FOURBYTE * 0x2), SEEK_CUR);
                        MovSTread(alac_info,1,size,videoFile);
                        MovSTseek(videoFile, -size, SEEK_CUR);
                        MovSTseek(videoFile, entrySize - (FOURBYTE * 0x9), SEEK_CUR);                   
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

 int MovReadStts_h(MOV_ST *videoFile, MovFileinf *pFileinf, LONG boxSize)
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

    switch (IsTheTrackVideo_h)
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

 int MovReadStsc_h(MOV_ST *videoFile, MovFileinf *pFileinf, LONG boxSize)
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

    switch (IsTheTrackVideo_h)
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

 int MovReadStsz_h(MOV_ST *videoFile, MovFileinf *pFileinf, LONG boxSize)
{
    uint32  sampleSize =0x0;
    uint32  sampleCount = 0x0;

    ASSERT((videoFile != NULL)
           && (pFileinf != NULL));

    MovSTseek(videoFile, FOURBYTE, SEEK_CUR);
    if (MovSTread((unsigned char *)&sampleSize, 1, sizeof(uint32), videoFile) < sizeof(uint32))
    {
        return MOV_READ_DATA_FAILED;
    }
    sampleSize = BYTESWAP(sampleSize);

    switch (IsTheTrackVideo_h)
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

    if (MovSTread((unsigned char *)&sampleCount, 1, sizeof(uint32), videoFile) < sizeof(uint32))
    {
        return MOV_READ_DATA_FAILED;
    }
    sampleCount = BYTESWAP(sampleCount);

    switch (IsTheTrackVideo_h)
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

            MovIndexInit_h(MovSTtell(videoFile), sVideoSampleSizeIndex);
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
            MovIndexInit_h(MovSTtell(videoFile), sAudioSampleSizeIndex);
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

 int MovReadStco_h(MOV_ST *videoFile, MovFileinf *pFileinf, LONG boxSize)
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

    switch (IsTheTrackVideo_h)
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
        MovIndexInit_h(MovSTtell(videoFile), sVideoChunkOffsetIndex);
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
        MovIndexInit_h(MovSTtell(videoFile), sAudioChunkOffsetIndex);
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

 int MovReadStss_h(MOV_ST *videoFile, MovFileinf *pFileinf, LONG boxSize)
{
    uint32 entryCount = 0x0;

    ASSERT((videoFile != NULL)
           && (pFileinf != NULL));

    switch (IsTheTrackVideo_h)
    {
    case SIGN_VIDEO:
        MovSTseek(videoFile, FOURBYTE, SEEK_CUR);

        /* read the number of random access video samples */
        if (MovSTread((unsigned char *)&entryCount, 1, sizeof(uint32), videoFile) < sizeof(uint32))
        {
            return MOV_READ_DATA_FAILED;
        }
        entryCount = BYTESWAP(entryCount);
        pFileinf->videoSyncSampleNo = entryCount;

        if (pFileinf->videoSyncSampleNo > 0x0)
        {
            sVideoSyncSampleIndex = (uint32 *)MALLOC(sizeof(uint32) * pFileinf->videoSyncSampleNo);

            if (sVideoSyncSampleIndex == NULL)
            {
                return MALLOC_FAILED;
            }

            if (MovSTread((unsigned char *)sVideoSyncSampleIndex, 1, sizeof(uint32) * pFileinf->videoSyncSampleNo, videoFile)
                < (sizeof(uint32) * pFileinf->videoSyncSampleNo))
            {
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

 int MovReadStbl_h(MOV_ST *videoFile, MovFileinf *pFileinf, LONG boxSize)
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
            case SIGN_STSD :
                if (MovReadStsd_h(videoFile, pFileinf, length))
                {
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

            case SIGN_STTS :
                if (MovReadStts_h(videoFile, pFileinf, length))
                {
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
                if (MovReadStsc_h(videoFile, pFileinf, length))
                {
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

            case SIGN_STSZ :
                if (MovReadStsz_h(videoFile, pFileinf, length))
                {
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
                if (MovReadStco_h(videoFile, pFileinf, length))
                {
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
                if (MovReadStss_h(videoFile, pFileinf, length))
                {
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

 int MovReadMdhd_h(MOV_ST *videoFile, MovFileinf *pFileinf, LONG boxSize)
{
    DWORD vsfl = 0x0;
    uint32  timeScale = 0x0;
    uint32 tempVar = 0x0;

    ASSERT((videoFile != NULL)
           && (pFileinf != NULL));

    if (MovSTread((unsigned char *)&vsfl, 1, sizeof(DWORD), videoFile) < sizeof(DWORD))
    {
        return MOV_READ_DATA_FAILED;
    }
    vsfl = BYTESWAP(vsfl);

    if (vsfl & 0xff000000)
    {
        MovSTseek(videoFile, FOURBYTE << 2, SEEK_CUR);
        if (MovSTread((unsigned char *)&timeScale, 1, sizeof(uint32), videoFile) < sizeof(uint32))
        {
            return MOV_READ_DATA_FAILED;
        }
        timeScale = BYTESWAP(timeScale);

        if (IsTheTrackVideo_h == SIGN_AUDIO)
        {
            uint32 tmp;

            pFileinf->audioTimeScale = timeScale;

            /**
             * Ceva or VC does not support 64 bits data; for 
             */
            /*if (MovFread(&pFileinf->uint64AudioDuration, 1, sizeof(uint64), videoFile) < sizeof(uint64))*/
            if (MovSTread((unsigned char *)&pFileinf->AudioDuration.low_part, 1, sizeof(uint32), videoFile) < sizeof(uint32));
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
        }
        else
        {
            if (IsTheTrackVideo_h == SIGN_VIDEO)
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
    else
    {
        MovSTseek(videoFile, FOURBYTE << 1, SEEK_CUR);
        if (MovSTread((unsigned char *)&timeScale, 1, sizeof(uint32), videoFile) < sizeof(uint32))
        {
            return MOV_READ_DATA_FAILED;
        }
        timeScale = BYTESWAP(timeScale);

        if (IsTheTrackVideo_h == SIGN_AUDIO)
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
            if (IsTheTrackVideo_h == SIGN_VIDEO)
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

 int MovReadHdlr_h(MOV_ST *videoFile, MovFileinf *pFileinf, LONG boxSize)
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
        IsTheTrackVideo_h = SIGN_VIDEO;
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
            if (IsTheTrackVideo_h == -1)
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
            IsTheTrackVideo_h = SIGN_AUDIO;
            pFileinf->audioFlag = TRUE;
        }
        else
        {
            IsTheTrackVideo_h = SIGN_OTHER;
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

 int MovReadMinf_h(MOV_ST *videoFile, MovFileinf *pFileinf, LONG boxSize)
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
                if (MovReadStbl_h(videoFile, pFileinf, length))
                {
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

 int MovReadTkhd_h(MOV_ST *videoFile, MovFileinf *pFileinf, LONG boxSize)
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
        IsTheTrackVideo_h = SIGN_AUDIO;
    }
    else
    {
        IsTheTrackVideo_h = SIGN_OTHER;
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

 int MovReadMdia_h(MOV_ST *videoFile, MovFileinf *pFileinf, LONG boxSize)
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
            case SIGN_MDHD :
                if (MovReadMdhd_h(videoFile, pFileinf, length))
                {
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

            case SIGN_HDLR :
                if (MovReadHdlr_h(videoFile, pFileinf, length))
                {
                    return MOV_FILE_BOX_PARSE_ERR;
                }

                break;

            case SIGN_MINF :
                if (MovReadMinf_h(videoFile, pFileinf, length))
                {
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
 int MovReadTrak_h(MOV_ST *videoFile, MovFileinf *pFileinf, LONG boxSize)
{
    uint32  length = 0x0;
    DWORD tag = 0x0;
    uint32 sumLength = 0x0;

    ASSERT((videoFile != NULL)
           && (pFileinf != NULL));

    IsTheTrackVideo_h = -1;
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
            case SIGN_TKHD :
                if (MovReadTkhd_h(videoFile, pFileinf, length))
                {
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
                if (MovReadMdia_h(videoFile, pFileinf, length))
                {
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

 int MovReadMoov_h(MOV_ST *videoFile, MovFileinf *pFileinf, LONG boxSize)
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
                if (MovReadTrak_h(videoFile, pFileinf, length))
                {
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
STATIC uint32 MovGetAudioSampleSize_h(MOV_ST *videoFile,
                                    MovFileinf *pFileinf,
                                    uint32 audioSampleNo)
{
    uint32 sampleSize = 0x0;

    ASSERT(pFileinf != NULL);

    if (pFileinf->audioSampleSize == 0x0)
    {
        //sampleSize = sAudioSampleSizeIndex[audioSampleNo - 1];
        sampleSize = MovIndexRead_h(audioSampleNo, sAudioSampleSizeIndex);
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
STATIC uint32 MovGetAudioChunkOffset_h(MOV_ST *videoFile,
                                     MovFileinf *pFileinf,
                                     uint32 audioChunkNo)
{
    uint32 chunkOffset = 0x0;

    /*chunkOffset = sAudioChunkOffsetIndex[audioChunkNo - 1];*/
    chunkOffset = MovIndexRead_h(audioChunkNo, sAudioChunkOffsetIndex);
    chunkOffset = BYTESWAP(chunkOffset);
    return chunkOffset;
}



STATIC uint32 MovGetAudioChunkSize_h(MOV_ST *videoFile,
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
            sampleSize = MovIndexRead_h(*endSampleNo, sAudioSampleSizeIndex);
            sampleSize = BYTESWAP(sampleSize);
            chunkSize = preAudioChunkSize + sampleSize;
            goto ret;
        }

        for (count = *beginSampleNo; count <= *endSampleNo; count++)
        {
            /*sampleSize = sAudioSampleSizeIndex[count - 1];*/
            sampleSize = MovIndexRead_h(count, sAudioSampleSizeIndex);
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
STATIC void MovAudioGetSampleToChunk_h(MOV_ST *videoFile,
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
STATIC void MovGetAudioSampleinfChunk_h(MOV_ST *videoFile,
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
uint32 MovGetAudioSampleTime_h(MOV_ST *audioFile,
                            MovFileinf *pFileinf,
                            uint32 sampleNo)
{
    uint32 loop = 0x1;
    uint32 sampleCount = 0x0;
    uint32 sampleDelta = 0x0;
    uint32 sampleSum = 0x0;
    uint32 durationSum = 0x0;
    int duration = 0x0;

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

STATIC uint32 MovGetAudioFrameNoToTime_h(MOV_ST *videoFile,
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
STATIC int MovAudioGetNextSampleInfo_h(MOV_ST *videoFile, MovFileinf *pFileinf)
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

    MovAudioGetSampleToChunk_h(videoFile,
                             pFileinf,
                             pFileinf->curAudioSampleNo,
                             &chunkNo,
                             &index);

    chunkOffset = MovGetAudioChunkOffset_h(videoFile, pFileinf, chunkNo);

    MovGetAudioSampleinfChunk_h(videoFile,
                              pFileinf,
                              &chunkNo,
                              &totalSample,
                              &beginSampleNo,
                              &endSampleNo);

    tempVar = beginSampleNo + index - 2;

    size = MovGetAudioChunkSize_h(videoFile,
                                pFileinf,
                                &beginSampleNo,
                                &tempVar);

    pFileinf->audioSample.sampleOffset = chunkOffset + size;

    pFileinf->audioSample.curSampleSize
        = MovGetAudioSampleSize_h(videoFile, pFileinf, pFileinf->curAudioSampleNo);

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
void MovGenerateMovFrmHeader_h(char *pHeader, uint32 frmLength, uint32 msTime)
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
STATIC int MovReadDescrLen_h(MOV_ST *videoFile)
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
STATIC int MovReadDescr_h(MOV_ST *videoFile, BYTE *tag)
{
    int len;
    if (MovSTread(tag, 1, sizeof(BYTE), videoFile) < sizeof(BYTE))
    {
        return MOV_READ_DATA_FAILED;
    }
    len = MovReadDescrLen_h(videoFile);
    return len;
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
//#define     bool                            BOOL




extern  HANDLE FileOpen(uint8 * shortname, int32 DirClus, int32 Index, int FsType, uint8 *Type);
extern uint16 FileRead(uint8 *Buf, uint16 Size, HANDLE Handle);
extern uint8 FileSeek(int32 offset, uint8 Whence, HANDLE Handle);
extern bool FileEof(HANDLE Handle);
extern uint8 FileClose(HANDLE Handle);
extern unsigned long RKFTell(FILE *in);




/*FILE * fopen_hl(const char * path,const char * mode)
{
        M4a_Name *p = (M4a_Name *)path;
         return ((FILE*)FileOpen((unsigned char *)p->name, p->Clus, p->Index, 1, "R"));
        

}*/


unsigned short  fread_hl_h(void*buffer,size_t size,size_t count,FILE*stream)
    {

        int num  = size*count ;
        if(num >=(1U<<16))
                num = (1u<<16)-1;
        
        return RKFIO_FRead(buffer,(unsigned short)num,stream);
    
    
    }
    unsigned char fseek_hl_h(FILE *stream, long offset, int fromwhere)
        {
        return RKFIO_FSeek((unsigned int )offset,(unsigned char )fromwhere,stream);          

    }
        



unsigned  long ftell_hl_h(FILE *stream)
{
    return  RKFIO_FTell(stream);
}

unsigned char  fclose_hl_h(FILE *stream)
{
    return RKFIO_FClose( stream);
}

#endif 




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


int MovFileInit_h(FILE* alac_raw_file)
{
   aac_MemSet(&movMem_h,0,sizeof(movMem_h));
/*
    if (0 > MovSTopen(&gMovFile.movVidBuf, MovOpen(pathName, "rb")))
    {
        return MOV_FILE_OPEN_ERR;
    }
*/
  if (0 > MovSTopen(&gMovFile_h.movAudBuf, alac_raw_file))
    {
        return MOV_FILE_OPEN_ERR;
    }
    
 //   MovSTseek(&gMovFile.movVidBuf, 0, SEEK_SET);
    MovSTseek(&gMovFile_h.movAudBuf, 0, SEEK_SET);
/*
    if (0 > MovIndexCreate((char *)pathName, &sVideoSampleSizeIndex))
    {
        return MOV_FILE_OPEN_ERR;
    }
*/
    if (0 > MovIndexCreate_h(pAacFileHandleSize, &sAudioSampleSizeIndex))
    {
        return MOV_FILE_OPEN_ERR;
    }
/*
    if (0 > MovIndexCreate((char *)pathName, &sVideoChunkOffsetIndex))
    {
        return MOV_FILE_OPEN_ERR;
    }
    */

    if (0 > MovIndexCreate_h(pAacFileHandleOffset, &sAudioChunkOffsetIndex))
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
extern int ALAC_filelen;
int alac_data_size = 0;
int MDATA_offset ;
int MovFileParsing_h(MOV_ST *videoFile)
{

    uint32 boxSize = 0x0;
    DWORD tag = 0x0;
    int rdsize;
    
    int data_offset = 0;
    ASSERT(videoFile != NULL);
    MovSTseek(videoFile, 0, SEEK_SET);

    for (; ;)
    {
        /* read the size of the box */
        if (MovSTread((unsigned char *)&boxSize, 1, sizeof(uint32), videoFile) < sizeof(uint32))//if parse to the end
        {
            return MOV_FILE_PARSING_OK;
        }

        boxSize = BYTESWAP(boxSize);
        data_offset += boxSize;     

        if (MovSTread((unsigned char *)&tag, 1, sizeof(DWORD), videoFile) < sizeof(DWORD))
        {
            return MOV_READ_DATA_FAILED;
        }

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
            if (MovReadMoov_h(videoFile, &gMovFile_h, boxSize))
            {
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
            /*case SIGN_MDAT ://alac
                return MOV_FILE_PARSING_OK;
                break;*/

        default :
            if ((tag == SIGN_FTYP) || (tag == SIGN_PDIN) || (tag == SIGN_MOOF)
                || (tag == SIGN_MFRA) || (tag == SIGN_FREE) || (tag == SIGN_MDAT)
                || (tag == SIGN_SKIP) || (tag == SIGN_META) || (tag == SIGN_WIDE)
                || (tag == SIGN_UDTA) || (tag == SIGN_UUID))
            {
               if(tag == SIGN_MDAT)
                {
                   if (alac_data_size <= 512) //当出现两个SIGN_MDAT标志时
                   {
                      MDATA_offset = data_offset -boxSize+8;
                      alac_data_size = boxSize;
                   }
                }
                MovSTseek(videoFile, boxSize - (FOURBYTE << 1), SEEK_CUR);
            }
            else
            {
                uint32 curpos = MovSTtell(videoFile);
                if(( ALAC_filelen - curpos )<512)
                {
                    Hifi_Alac_Printf("非正常结束\n");
                    
                    return  MOV_FILE_PARSING_OK;
                }
                else
                {
                    Hifi_Alac_Printf("0x%x pos unkown box",curpos);
                    return MOV_FILE_BOX_PARSE_ERR;
                }
            }
            break;
        }
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

void MovFileClose_h(void)
{
   // MovSTclose(&gMovFile.movVidBuf);
    MovSTclose(&gMovFile_h.movAudBuf);

    //MovIndexDestroy(sVideoSampleSizeIndex);
    MovIndexDestroy_h(sAudioSampleSizeIndex);

    //MovIndexDestroy(sVideoChunkOffsetIndex);
    MovIndexDestroy_h(sAudioChunkOffsetIndex);
}



/** 
 * ljf@20100416
 * Mov Audio Seek To Time
 */
int MovAudioSeekToTime_h(uint32 timems)
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

    audioDuration = ((timems / 1000) * gMovFile_h.audioTimeScale) +
                    (((timems % 1000) * gMovFile_h.audioTimeScale) / 1000);

    if ((gMovFile_h.aFormat == AUDIO_CODEC_LIB_MP4A)
        ||(gMovFile_h.aFormat == AUDIO_CODEC_LIB_ALAC))
    {
        audioFrameNo = MovGetAudioFrameNoToTime_h(&gMovFile_h.movAudBuf, &gMovFile_h, audioDuration);

        /* stop the audio */
        if (audioFrameNo == (gMovFile_h.audioSampleNum + 0x1))
        {
            gMovFile_h.curAudioSampleNo = audioFrameNo;
            gMovFile_h.audioSample.readSize = 0x0;
            gMovFile_h.audioSample.curSampleSize = 0x0;
            //VideoAudioStop(0x0);
            movTimerCount_h = timems;
            //MovSetAudioPlayEnd();
        }
        else
        {
            /* reboot the codec of audio */
            if (0/*isAudioPlayEnd == TRUE*/)
            {
                gMovFile_h.curAudioSampleNo = 0x0;
                gMovFile_h.audioSample.readSize = 0x0;
                gMovFile_h.audioSample.curSampleSize = 0x0;
                //VideoAudioStop(0x0);
                //MovAudioStart();
            }

            gMovFile_h.curAudioSampleNo = audioFrameNo;
            gMovFile_h.audioSample.readSize = 0x0;
            
            MovAudioGetSampleToChunk_h(&gMovFile_h.movAudBuf,
                                     &gMovFile_h,
                                     gMovFile_h.curAudioSampleNo,
                                     &chunkNo,
                                     &index);

            chunkOffset = MovGetAudioChunkOffset_h(&gMovFile_h.movAudBuf, &gMovFile_h, chunkNo);

            MovGetAudioSampleinfChunk_h(&gMovFile_h.movAudBuf,
                                      &gMovFile_h,
                                      &chunkNo,
                                      &totalSample,
                                      &beginSampleNo,
                                      &endSampleNo);

            tempVar = beginSampleNo + index - 2;
            size = MovGetAudioChunkSize_h(&gMovFile_h.movAudBuf,
                                        &gMovFile_h,
                                        &beginSampleNo,
                                        &tempVar);


            gMovFile_h.audioSample.sampleOffset = chunkOffset + size;

            gMovFile_h.audioSample.curSampleSize = MovGetAudioSampleSize_h(&gMovFile_h.movAudBuf,
                                                                       &gMovFile_h,
                                                                       gMovFile_h.curAudioSampleNo);

            msTime = ((audioDuration / gMovFile_h.audioTimeScale) * 1000) +
                     (((audioDuration % gMovFile_h.audioTimeScale) * 1000) / gMovFile_h.audioTimeScale);

            movTimerCount_h = msTime;
          
        }/*end if (gMovFile.curAudioSampleNo == (gMovFile.audioSampleNum + 0x1))*/
    }
    else
    {
        if ((gMovFile_h.aFormat == AUDIO_CODEC_LIB_SAMR)
            || (gMovFile_h.aFormat == AUDIO_CODEC_LIB_MP3))
        {
            audioFrameNo = MovGetAudioFrameNoToTime_h(&gMovFile_h.movAudBuf,
                                                    &gMovFile_h,
                                                    audioDuration);

            /* the audio end */
            if (audioFrameNo == (gMovFile_h.audioSampleNum + 0x1))
            {
                gMovFile_h.curAudioSampleNo = audioFrameNo;
                gMovFile_h.audioSample.readSize = 0x0;
                gMovFile_h.audioSample.curSampleSize = 0x0;
                //VideoAudioStop(0x0);
                movTimerCount_h = msTime;
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

/**
 * Following Functions are Mov Parser Interface.
 */

int MovIF_SynAudio2Video_h(unsigned int timems)
{
    MovAudioSeekToTime_h(timems);
    MovAudioSeekToFILE_h(&gMovFile_h.movAudBuf);
    gBeingSkip_h = RESUME;
    return 0;
}

unsigned int MovIF_AudioGetCurrentTime_h(void)
{
    uint32 duration;
    uint32 msTime;
    uint32 estTime;
   
    duration = MovGetAudioSampleTime_h(&gMovFile_h.movAudBuf, &gMovFile_h, gMovFile_h.curAudioSampleNo);
    msTime = ((duration / gMovFile_h.audioTimeScale) * 1000) 
        + (((duration % gMovFile_h.audioTimeScale) * 1000) / gMovFile_h.audioTimeScale);

    if ((gMovFile_h.aFormat == AUDIO_CODEC_LIB_MP4A)||((gMovFile_h.aFormat == AUDIO_CODEC_LIB_ALAC)))
    {
        estTime = 0;
    }
    else
    {
        estTime = 180;
    }

    return msTime > estTime ? msTime - estTime : msTime;
}




/*********************************************************************************************************
**                            End Of File
********************************************************************************************************/
#pragma arm section code
#endif
#endif
