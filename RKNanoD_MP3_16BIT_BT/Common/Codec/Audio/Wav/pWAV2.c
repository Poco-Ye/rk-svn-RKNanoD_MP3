/* Copyright (C) 2007 ROCK-CHIPS FUZHOU . All Rights Reserved. */
/*
File : \Audio\ADPCM
Desc : WAV���롣����PCM WAV , IMA-ADPCM WAV , MS-ADPCM WAV ��

Author : FSH , Vincent Hisung
Date : 2007-08-xx
Notes :

$Log :
* vincent     2007/08/xx �������ļ�
*
*/
/****************************************************************/

#include "../include/audio_main.h"

#ifdef WAV_DEC_INCLUDE

#include "../include/audio_globals.h"
#include "WAV_LIB/sf_wav.h"



#include "../include/audio_file_access.h"
#include "PCM.H"

#include <stdio.h>
#include <string.h>

#define INFO_BYTE_SIZE 32

typedef struct
{
    char Artist[INFO_BYTE_SIZE];
    char Name[INFO_BYTE_SIZE];
    char Product[INFO_BYTE_SIZE];
    char Genre[INFO_BYTE_SIZE];
}tINFO;
_ATTR_WAVDEC_DATA_ int total_samples;
#define _ATTR_BB_SYS_CODE_          __attribute__((section("BBSysCode")))
#define _ATTR_BB_SYS_DATA_          __attribute__((section("BBSysData")))
#define _ATTR_BB_SYS_BSS_           __attribute__((section("BBSysBss"), zero_init))

__align(4)
_ATTR_BB_SYS_BSS_  char PcmOutputBuff[WAV_IMAMAX_PCM_LENGTH*8];
_ATTR_BB_SYS_BSS_  tPCM pcm_s;

extern SF_PRIVATE sf;
extern  unsigned long SRC_Num_Forehead; //for src
extern  int CodecBufSize2;

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

_ATTR_WAVDEC_TEXT_
unsigned long
PCMFunction2(unsigned long ulIoctl, unsigned long ulParam1,
             unsigned long ulParam2, unsigned long ulParam3
            )
{
    switch (ulIoctl)
    {
        case SUBFN_CODEC_OPEN_DEC:
            {
                tPCM *pPCM;
                tINFO pINFO;

                int i;
                int isErr;

                unsigned long ulPos;

                pPCM = &pcm_s;

                MemSet2(&pINFO , 0 , sizeof(pINFO));
                MemSet2(&pcm_s , 0 , sizeof(tPCM));

                pPCM->N_remain = 0;
                pPCM->N_GeneratedSample = 0;

                isErr = InitPCMDecoder(pPCM);

                if (pPCM->wFormatTag == WAVE_FORMAT_ADPCM)
                {
                    sf.sf.channels = pcm_s.ucChannels;
                    sf.sf.samplerate = pcm_s.usSampleRate;
                    sf.sf.format = (SF_FORMAT_WAV | SF_FORMAT_MS_ADPCM);
                    sf.mode = SFM_READ;

                    if (pcm_s.ulLength == 0xffffffff)
                        pcm_s.ulLength = 0x7fffffff;

                    sf.datalength = pcm_s.ulLength;

                    /* wav_w64_msadpcm_init */
                    msadpcm_dec_init (&sf, pcm_s.usBytesPerBlock, pcm_s.usSamplesPerBlock);
                }

                //if (pPCM->usByteRate != 0)
                //    pPCM->ulTimeLength = 1000 * ((long long)(RKFIO_FLength(pRawFileCache)) / (long long) pPCM->usByteRate);

                //----------------------------------------

                //set cache buffer area.
                {
                    unsigned long size;

//                    pPCM->pOutput = (BufferState *) & pPCM->sPlayBuffer;

                    if (pPCM->wFormatTag == WAVE_FORMAT_DVI_ADPCM)
                        size = WAV_IMAMAX_PCM_LENGTH;
                    else
                        if (pPCM->wFormatTag == WAVE_FORMAT_PCM)
                            size = pPCM->usSamplesPerBlock;
                        else
                            size = pPCM->usSamplesPerBlock;

                    pPCM->msadpcm_outsize = size * 2;

                }

                total_samples = (long long)pPCM->ulTimeLength*pPCM->usSampleRate/1000;


                pPCM->OutLength = 1152 ; // pPCM->usSamplesPerBlock;

                ulPos = 0 ;

                pPCM->ulTimePos = ulPos * pPCM->usSamplesPerBlock;

                if (pPCM->wFormatTag == 1)
                {
                    ulPos = pPCM->ulDataStart + (ulPos * pPCM->usBytesPerBlock) * pPCM->usSamplesPerBlock; //(PCM Seek )
                }

                else //else if(pPCM->sWaveFormat.wFormatTag == 2)
                {
                    ulPos = pPCM->ulDataStart + (ulPos * pPCM->usBytesPerBlock); //(ADPCM Seek )
                }

                if (ulPos > pPCM->ulLength)
                    return 0;

                if (pPCM->wFormatTag == WAVE_FORMAT_ADPCM)
                {
                    RKFIO_FSeek( (int)ulPos , 0 , pRawFileCache );
                }
                else if (pPCM->wFormatTag == WAVE_FORMAT_IMA_ADPCM)
                {
                    RKFIO_FSeek( (int)(ulPos&~511), 0 , pRawFileCache );
                    pPCM->usValid = RKFIO_FRead( pPCM->pucEncodedData, 1024 , pRawFileCache );

                    if (pPCM->usValid < 1024)
                        return 0;
                }
                else
                {
                    RKFIO_FSeek( (int)(pPCM->ulDataStart), 0 , pRawFileCache );
                }

                pPCM->usOffset = (unsigned short)(ulPos & 511);

                pPCM->ulDataValid = pPCM->ulLength +  pPCM->ulDataStart - (ulPos & ~511);


                return(isErr);
            }
            /*
                    case SUBFN_CODEC_GETARTIST:
                        {
                            char **ppcName;
                            ppcName = (char **)ulParam2;
                            *ppcName = 0;
                            return(1);
                        }

                    case SUBFN_CODEC_GETTITLE:
                        {
                            char **ppcName;
                            ppcName = (char **)ulParam2;
                            *ppcName = 0;

                            return(1);
                        }
            */

        case SUBFN_CODEC_GETBITRATE:
            {
                unsigned long *pulBitRate;
                tPCM *pPCM;

                pPCM = &pcm_s;

                pulBitRate = (unsigned long *)ulParam1;
                *pulBitRate = pPCM->usByteRate * 8;

#ifndef CODEC_24BIT
                if((pPCM->usByteRate * 8) > 1600000)    //codec 16bit
                {
                    return 0;
                }
                else
                    return(1);
#else
                 return(1);
#endif
            }

        case SUBFN_CODEC_GETSAMPLERATE:
            {
                unsigned long *pulSampleRate;
                tPCM *pPCM;

                pPCM = &pcm_s;

                pulSampleRate = (unsigned long *)ulParam1;
                *pulSampleRate = pPCM->usSampleRate;

                return(1);
            }

        case SUBFN_CODEC_GETCHANNELS:
            {
                unsigned long *pulChannels;
                tPCM *pPCM;

                pPCM = &pcm_s;

                pulChannels = (unsigned long *)ulParam1;
                *pulChannels = pPCM->ucChannels;

                return(1);
            }

        case SUBFN_CODEC_GETLENGTH:
            {
                unsigned long *pulLength;
                tPCM *pPCM;

                pPCM = &pcm_s;
                pulLength = (unsigned long *)ulParam1;
                *pulLength = pPCM->ulTimeLength;
                return(1);
            }

        case SUBFN_CODEC_GETTIME:
            {
                unsigned long *pulTime;
                tPCM *pPCM;

                pPCM = &pcm_s;
                pulTime = (unsigned long *)ulParam1;
                *pulTime = (((__int64)pPCM->ulTimePos * 1000 / pPCM->usSampleRate)) +
                           (((((__int64)pPCM->ulTimePos * 1000 % pPCM->usSampleRate)) / pPCM->usSampleRate));

                return(1);
            }

        case SUBFN_CODEC_GETBUFFER:
            {
                //ulParam1 = buf ..
                //ulParam2 = length ..

                tPCM *pPCM = &pcm_s;
                *(short**)ulParam1 = &pPCM->OutputBuff[pPCM->CurDecodeIdx][SRC_Num_Forehead];
                if (pPCM->CurDecodeIdx==0)
                    pPCM->CurDecodeIdx = 1;
                else
                    pPCM->CurDecodeIdx = 0;

                *(long*)ulParam2 = pPCM->OutLength;

                return(1);
            }

        case SUBFN_CODEC_DECODE:
            {
                tPCM *pPCM;
                short *psLeft, *psRight;
                short *pOut;
                char  *pOut_24;
                long lLength, lBlockSize;
                long ret;
                short *ptr;
                char *char_ptr;
                unsigned int Counter = 0;

                pPCM = &pcm_s;

                switch (pPCM->wFormatTag)
                {
                    case WAVE_FORMAT_ADPCM:
                        {
                            pPCM->OutLength = msadpcm_read_s (&sf, &pPCM->OutputBuff[pPCM->CurDecodeIdx][SRC_Num_Forehead] , /* pPCM->usSamplesPerBlock */ 1152 )
                                              / pcm_s.ucChannels ;

                            pPCM->ulTimePos += pPCM->OutLength;

                            if (pPCM->OutLength == 0)
                                return 0;


                            return 1;
                        }

                    case WAVE_FORMAT_IMA_ADPCM:
                        {
                            int i;

                            //--------------------------------
                            if ((pPCM->ulLength) == 0)
                                return 0;

                            if ((pPCM->usOffset) >= (pPCM->ulLength))
                                return 0;

                            if (pPCM->usValid < (pPCM->usOffset & ~3))
                                return 0;
                            //--------------------------------

                            ret = IMAADPCM_FORMAT(pPCM);

                            pPCM->ulTimePos += ret;

                            if (ret > 0)
                                pPCM->OutLength = ret;



                            if (ret != 0)
                                ret = 1;
                            else
                                ret = 0;

                            return(ret);
                        }

                    case WAVE_FORMAT_PCM:
                        {
                            int i;

                            if ((pPCM->ulLength) == 0)
                                return 0;

                            if ((pPCM->usOffset) >= (pPCM->ulLength))
                                return 0;

                            if (pPCM->usValid == 0)
                                return 0;

                            pOut = &pPCM->OutputBuff[pPCM->CurDecodeIdx][SRC_Num_Forehead];
                            pOut_24 = (char *)&pPCM->OutputBuff[pPCM->CurDecodeIdx][SRC_Num_Forehead];

                            if (pPCM->uBitsPerSample == 16)
                            {
                                lBlockSize = (pPCM->usSamplesPerBlock << pPCM->ucChannels);
                            }
                            else if (pPCM->uBitsPerSample == 8)
                            {
                                lBlockSize = (pPCM->usSamplesPerBlock * pPCM->ucChannels);
                            }
                            else if (pPCM->uBitsPerSample == 24)
                            {
                                lBlockSize = (pPCM->usSamplesPerBlock * pPCM->ucChannels*3);

                            }
                            else if (pPCM->uBitsPerSample == 32)
                            {
                                lBlockSize = (pPCM->usSamplesPerBlock << (pPCM->ucChannels+1));
                            }
                            else
                                return 0;
                            if ((pPCM->ucChannels == 2) && (pPCM->uBitsPerSample >8))
                            {
                                pPCM->usValid = RKFIO_FRead( pOut, lBlockSize , pRawFileCache );
                            }
                            else
                            {
                                pPCM->usValid = RKFIO_FRead(pPCM->pucEncodedData, lBlockSize , pRawFileCache );

                                //16 bit quantity
                                if (pPCM->uBitsPerSample == 16)
                                {
                                    ptr = (short *)pPCM->pucEncodedData;
                                    if (pPCM->ucChannels == 1)
                                    {
                                        for (i = 0; i < lBlockSize / 2 ; i++)
                                        {
                                            *pOut++  = *ptr   ;	// L
                                            *pOut++  = *ptr++ ;	// R
                                        }
                                    }

                                }
                                else if (pPCM->uBitsPerSample == 8) //8 bit quantity
                                {
                                    char_ptr = (char *)pPCM->pucEncodedData;

                                    if (pPCM->ucChannels == 1)  //mono
                                    {

                                        for (i = 0; i < lBlockSize ; i++)
                                        {
                                            //L
                                            *pOut++   = (short)(((*char_ptr) - 0x80) << 8);//Convert 8BIT(per sample) to 16BIT(per sample)
                                            //R
                                            *pOut++   = (short)(((*char_ptr) - 0x80) << 8);//Convert 8BIT(per sample) to 16BIT(per sample)

                                            char_ptr++ ;
                                        }
                                    }
                                    else  //stereo
                                    {
                                        for (i = 0 ; i < lBlockSize ; i++)
                                        {
                                            // L,R,L,R
                                            if (i % 2 == 0)
                                            {
                                                *pOut++  = (short)(((*char_ptr) - 0x80) << 8);
                                                char_ptr++ ;	//LLL
                                            }
                                            else
                                            {
                                                *pOut++  = (short)(((*char_ptr) - 0x80) << 8);
                                                char_ptr++ ;	//RRR
                                            }
                                        }
                                    }
                                }
                                else if (pPCM->uBitsPerSample == 24)
                                {
                                    int temp ;
                                    if (pPCM->ucChannels == 1)
                                    {
                                        for (i = 0; i < lBlockSize / 3 ; i++)
                                        {
                                            temp = pPCM->pucEncodedData[3*i+2]<<16|pPCM->pucEncodedData[3*i+1]<<8
                                                   |pPCM->pucEncodedData[3*i];
                                            *(int *)pOut_24  = temp;	// L
                                            pOut_24 += 3;
                                            *(int *)pOut_24  = temp;	// R
                                            pOut_24 += 3;

                                        }
                                    }

                                }
                                else if (pPCM->uBitsPerSample == 32)
                                {
                                    int temp ;
                                    if (pPCM->ucChannels == 1)
                                    {
                                        for (i = 0; i < (lBlockSize >>2) ; i++)
                                        {
                                            temp = pPCM->pucEncodedData[4*i+3]<<24|pPCM->pucEncodedData[4*i+2]<<16
                                                   |pPCM->pucEncodedData[4*i+1]<<8|pPCM->pucEncodedData[4*i];
                                            *(int *)pOut_24  = temp;	// L
                                            pOut_24 += 4;
                                            *(int *)pOut_24  = temp;	// R
                                            pOut_24 += 4;

                                        }
                                    }

                                }

                            else  //here mean the quantity bits do not support.
                                return 0;



                            }

                                   if ( pPCM->usValid < lBlockSize )
                                       return 0;

                            pPCM->usOffset += pPCM->usValid;
                            pPCM->OutLength = pPCM->usSamplesPerBlock;
                            //--------------------------------------------

                            if (pPCM->uBitsPerSample == 16)
                            {
                                pPCM->ulTimePos += (lBlockSize >> pPCM->ucChannels);
                            }
                            else if (pPCM->uBitsPerSample == 8)
                            {
                                pPCM->ulTimePos += (lBlockSize >> (pPCM->ucChannels - 1));
                            }
                            else if (pPCM->uBitsPerSample == 24)
                            {
                                pPCM->ulTimePos += pPCM->usSamplesPerBlock;
                            }
                            else if (pPCM->uBitsPerSample == 32)
                            {
                                pPCM->ulTimePos += pPCM->usSamplesPerBlock;
                            }

                            //decoding is finished,if it had reached the end of file.
                            if (pPCM->usValid < lBlockSize)
                            {
                                return 0;
                            }
                            if (pPCM->ulTimePos > total_samples )
                            {
                                return 0;
                            }

                            return 1;
                        }
                    default:
                        return 0; //error.
                }
            }

        case SUBFN_CODEC_SEEK:
            {
                tPCM *pPCM;
                unsigned long ulPos;

                pPCM = &pcm_s;

                if (ulParam1 > pPCM->ulTimeLength)
                {
                    ulParam1 = pPCM->ulTimeLength;
                }

                ulPos = (((ulParam1 / 1000) * pPCM->usSampleRate) / pPCM->usSamplesPerBlock) +
                        (((ulParam1 % 1000) * pPCM->usSampleRate) / (pPCM->usSamplesPerBlock * 1000));

                pPCM->ulTimePos = ulPos * pPCM->usSamplesPerBlock;

                msadpcm_seek_set(&sf,pPCM->usSamplesPerBlock * ulPos);

                if (pPCM->wFormatTag == 1)
                {
                    ulPos = pPCM->ulDataStart + (ulPos * pPCM->usBytesPerBlock) * pPCM->usSamplesPerBlock; //(PCM Seek )
                }

                else //else if(pPCM->sWaveFormat.wFormatTag == 2)
                {
                    ulPos = pPCM->ulDataStart + (ulPos * pPCM->usBytesPerBlock); //(ADPCM Seek )
                }

                if (ulPos > pPCM->ulLength)
                    return 0;

                if (pPCM->wFormatTag == WAVE_FORMAT_ADPCM)
                {
                    RKFIO_FSeek( (int)ulPos , 0 , pRawFileCache );
                }
                else if (pPCM->wFormatTag == WAVE_FORMAT_IMA_ADPCM)
                {
                    RKFIO_FSeek( (int)(ulPos&~511), 0 , pRawFileCache );
                    pPCM->usValid = RKFIO_FRead( pPCM->pucEncodedData, 1024 , pRawFileCache );

                    if (pPCM->usValid < 1024)
                        return 0;
                }
                else if (pPCM->wFormatTag == WAVE_FORMAT_PCM)
                {
                    RKFIO_FSeek( (int)(ulPos), 0 , pRawFileCache );
                }


                pPCM->usOffset = (unsigned short)(ulPos & 511);

                pPCM->ulDataValid = pPCM->ulLength +  pPCM->ulDataStart - (ulPos & ~511);

                return(1);
            }
        case SUBFN_CODEC_GETBPS:
            {
                tPCM *pPCM;
                unsigned long *bps;
                pPCM = &pcm_s;
                bps = (unsigned long *)ulParam1;
                *bps = pPCM->uBitsPerSample;
                if (*bps <= 16)
                {
                    *bps  = 16;
                }
                return 1;
            }
        case SUBFN_CODEC_CLOSE:
            {
                tPCM *pPCM;

                return 1;
            }

        default:
            {
                return 0;
            }
    }
}

#endif
