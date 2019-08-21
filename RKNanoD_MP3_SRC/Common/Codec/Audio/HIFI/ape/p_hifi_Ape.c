
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
/*
********************************************************************************************
*
*                Copyright (c): 2014 - 2014 + 5, WJR
*                             All rights reserved.
*
* FileName: BBSystem\Codecs\Audio\Decode\ape\pApe.c
* Owner: WJR
* Date: 2014.12.23
* Time: 19:31:51
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    WJR     2014.12.23     19:31:51   1.0
********************************************************************************************
*/

#include "SysInclude.h"
#include "audio_main.h"

#ifdef HIFI_APE_DECODE

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#include "ape.h"
#include "audio_globals.h"



#pragma arm section code = "ApeHDecCode", rodata = "ApeHDecCode", rwdata = "ApeHDecData", zidata = "ApeHDecBss"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define _BBSYSTEM_CODECS_AUDIO_DECODE_APE_PAPE_READ_  __attribute__((section("bbsystem_codecs_audio_decode_ape_pape_read")))
#define _BBSYSTEM_CODECS_AUDIO_DECODE_APE_PAPE_WRITE_ __attribute__((section("bbsystem_codecs_audio_decode_ape_pape_write")))
#define _BBSYSTEM_CODECS_AUDIO_DECODE_APE_PAPE_INIT_  __attribute__((section("bbsystem_codecs_audio_decode_ape_pape_init")))
#define _BBSYSTEM_CODECS_AUDIO_DECODE_APE_PAPE_SHELL_  __attribute__((section("bbsystem_codecs_audio_decode_ape_pape_shell")))


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/

//uint8 gHAPEPingPangBuf[2][4096*4];
u32 APE_out_Length = 1024;
uint8 *gHAPEPingPangBuf[2];
#ifdef A_CORE_DECODE
extern unsigned long SRC_Num_Forehead;
#else
//#define SRC_Num_Forehead 0
#endif
u32 gHAPEPingPangIndex;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/

FILE *ape_file_handle;
extern FILE *pRawFileCache;
APEContext apeobj;


#ifndef A_CORE_DECODE
#include "sysinclude.h"
#include "driverinclude.h"
#include "audio_file_access.h"
extern MediaBlock  gpMediaBlock;
extern unsigned int gDecDone;
extern unsigned int gSeekDone;
extern unsigned int gCloseDone;
extern unsigned int gOpenDone;

extern unsigned int  gError;
extern unsigned char DecDataBuf[2][AUDIO_FILE_PIPO_BUF_SIZE];
extern unsigned char DecBufID;

static UINT16 DmaTranferCallback;

static void DMATranferCallBack(void)
{
    DmaTranferCallback = 1;
}

#endif

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
static unsigned int CheckID3V2Tag(unsigned  char *pucBuffer)
{
    if ((pucBuffer[0] !=    'I') || (pucBuffer[1] != 'D') || (pucBuffer[2] != '3'))
    {
        return (0);
    }

    if (pucBuffer[3]    < 2  && pucBuffer[3] > 4)
    {
        return (0);
    }

    if (pucBuffer[4]    == 0xff)
    {
        return (0);
    }

    if ((pucBuffer[6] >=    0x80) || (pucBuffer[7] >= 0x80) ||
        (pucBuffer[8] >=    0x80) || (pucBuffer[9] >= 0x80))
    {
        return (0);
    }

    return ((pucBuffer[6] << 21) | (pucBuffer[7] << 14) |
            (pucBuffer[8] <<  7) |  pucBuffer[9]);
}
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: APEDecFunction
** Input:unsigned long ulSubFn, unsigned long ulParam1,
** Return: unsigned long
** Owner:WJR
** Date: 2014.12.23
** Time: 19:31:54
*******************************************************************************/
//_ATTR_APEDEC_TEXT_
unsigned long  HIFI_APEDecFunction(unsigned long ulSubFn, unsigned long ulParam1,
                                   unsigned long ulParam2, unsigned long ulParam3)
{
    uint32 timeout;

#ifdef A_CORE_DECODE

    APEContext *pAPE;
    pAPE = (APEContext *)&apeobj;

    switch (ulSubFn)
    {
        case SUBFN_CODEC_OPEN_DEC:
        {
            int ret = 0;
            unsigned char flag[20];
            int ID3_len;
            ape_file_handle = pRawFileCache;
            RKFIO_FSeek(0, 0 , ape_file_handle);
            RKFIO_FRead(flag, 20, ape_file_handle);
            ID3_len = CheckID3V2Tag(flag);

            if (ID3_len == 0)
            {
                RKFIO_FSeek(0, 0 , ape_file_handle);
            }
            else
            {
                ID3_len += 10;
                RKFIO_FSeek(ID3_len, 0 , ape_file_handle);
            }

            init_ape();
            ret = ape_read_header();

#ifdef HIFI_ACC
            HIFI_DMA_TO_register();
#endif

            //rk_printf("ape open ok\n");

            if (ret < 0)
            {
                bb_printf1("ape open FAIL!");
                return 0;
            }

            bb_printf1("ape open OK ");
            return 1;
        }

        case SUBFN_CODEC_DECODE:
        {
            ape_decode(&gHAPEPingPangBuf[gHAPEPingPangIndex][SRC_Num_Forehead], &APE_out_Length);

            if (APE_out_Length <= 0)
            {
                return 0;
            }

            if (pAPE->TimePos >= pAPE->total_blocks)
            {
                return 0;
            }

            return 1;
        }

        case SUBFN_CODEC_GETBUFFER:
        {
            *(unsigned long *)ulParam1 = (unsigned long)&gHAPEPingPangBuf[gHAPEPingPangIndex][SRC_Num_Forehead];
            *(unsigned long *)ulParam2 = (unsigned long) APE_out_Length;

            gHAPEPingPangIndex ^= 1;;
            return (1);
        }


        case SUBFN_CODEC_SEEK:
        {
            unsigned long ulPos;
            int file_offset = 0;

            if (ulParam1 > pAPE->file_time)//the unit is ms for ulParam1,the unit is also ms.
            {
                ulParam1 = pAPE->file_time;
            }

            ulPos = (((ulParam1 / 1000) * pAPE->samplerate/*+pAPE->blocksperframe*/) / pAPE->blocksperframe) +
                    (((ulParam1 % 1000) * pAPE->samplerate ) / (pAPE->blocksperframe * 1000));//the frame number that is equal with current time.

            if (ulPos >= pAPE->totalframes)
            {
                ulPos = pAPE->totalframes ;
                file_offset = (long long) ulPos * pAPE->blocksperframe / pAPE->samplerate * pAPE->bitrate;
                RKFIO_FSeek(file_offset, SEEK_SET, ape_file_handle); //��ȡ��֡λ��
            }

            pAPE->TimePos = ulPos * pAPE->blocksperframe;
            pAPE->APE_Frm_NUM = ulPos;
            pAPE->frm_left_sample = 0;

            return (1);
        }

        case SUBFN_CODEC_GETTIME:
        {
            unsigned long *pulTime;
            pulTime = (unsigned long *)ulParam1;

            if (pAPE->samplerate)
            {
                *pulTime = ((pAPE->TimePos / pAPE->samplerate) * 1000) +
                           (((pAPE->TimePos % pAPE->samplerate) * 1000) /
                            pAPE->samplerate);
            }

            return (1);
        }

        case SUBFN_CODEC_GETSAMPLERATE:
        {
            unsigned long *SampleRate;
            SampleRate = (unsigned long *)ulParam1;
            *SampleRate = pAPE->samplerate;
            return  1;
        }

        case SUBFN_CODEC_GETCHANNELS:
        {
            unsigned long *channels;
            channels = (unsigned long *)ulParam1;
            *channels = pAPE->channels;
            return 1;
        }

        case SUBFN_CODEC_GETBPS:
        {
            unsigned long *bps;
            bps = (unsigned long *)ulParam1;
            *bps = pAPE->bps;
            return 1;
        }

        case SUBFN_CODEC_GETBITRATE:
        {
            unsigned long *bitrate;
            bitrate = (unsigned long *)ulParam1;
            *bitrate = pAPE->bitrate + 500;
            return 1;
        }

        // Return the length (in milliseconds) of the file.
        case SUBFN_CODEC_GETLENGTH:
        {
            unsigned long *TimeLength;
            TimeLength = (unsigned long *)ulParam1;
            *TimeLength = pAPE->file_time;
            return 1;
        }


        // Cleanup after the codec.
        case SUBFN_CODEC_CLOSE:
        {
            APE_free();

#ifdef HIFI_ACC
            HIFI_DMA_TO_Unregister();
#endif

            return 1;
        }

        default:
        {
            // Return a failure.
            return (0);
        }
    }

#else

    switch (ulSubFn)
    {
        case SUBFN_CODEC_OPEN_DEC:
        {
            DEBUG("hifi ape open,A 2 B send dec_open...");
            gOpenDone = 0;
            MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DEC_OPEN, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);

            timeout = 20000000;

            while (!gOpenDone)
            {
                #ifdef _WATCH_DOG_
                WatchDogReload();
                #endif
                
                BBDebug();
                
                //__WFI();
                DelayUs(1);
                if (--timeout == 0)
                {
                    DEBUG("SUBFN_CODEC_OPEN_DEC: timeout!!!");
                    break;
                }
                
                #ifdef _SDCARD_
                if (TRUE == IsSDCardRemoved())
                {
                    break;
                }
                #endif
            }

            gOpenDone = 0;

            if ( gError )   //codec decode open error
                return 0;
            else
                return (1);
        }

        case SUBFN_CODEC_DECODE:
        {
            DEBUG("hifi ape decode,A 2 B send decode...");
            MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DECODE, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            return 1;
        }

        case SUBFN_CODEC_GETBUFFER:
        {
            //DEBUG("hifi ape get buffer...DecodeOver = %d",gpMediaBlock.DecodeOver);
            if (gpMediaBlock.DecodeOver == 1)
            {
                if (gpMediaBlock.DecodeErr == 1)
                {
                    return 0;
                }

                // *(int *)ulParam1 = gpMediaBlock.Outptr;
                *(int *)ulParam2 = gpMediaBlock.OutLength;
                gpMediaBlock.DecodeOver = 0;

                gpMediaBlock.Decoding = 1;
                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DECODE, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);

                //memory copy hram 2 lram.
                if (DecBufID == 0)
                {
#if 1
                    {
                        eDMA_CHN channel;
                        DMA_CFGX DmaCfg = {DMA_CTLL_M2M_WORD, DMA_CFGL_M2M_WORD, DMA_CFGH_M2M_WORD, 0};

                        DmaTranferCallback = 0;

                        channel = DmaGetChannel();

                        if (channel != DMA_FALSE)
                        {
                            DmaStart((uint32)(channel), (UINT32)(gpMediaBlock.Outptr), (uint32)(&DecDataBuf[0][0]), (gpMediaBlock.OutLength * gpMediaBlock.Bps) / 16, &DmaCfg, DMATranferCallBack);

                            while (1)
                            {
                                __WFI();

                                if (DmaTranferCallback == 1)
                                    break;
                            }
                        }
                    }
#else
                    memcpy(&DecDataBuf[0][0], (uint8*)(gpMediaBlock.Outptr) , (gpMediaBlock.OutLength * gpMediaBlock.Bps) / 4);

#endif
                    *(int *)ulParam1 = (uint32)&DecDataBuf[0][0];
                    DecBufID = 1;
                }
                else
                {
#if 1
                    {
                        eDMA_CHN channel;
                        DMA_CFGX DmaCfg = {DMA_CTLL_M2M_WORD, DMA_CFGL_M2M_WORD, DMA_CFGH_M2M_WORD, 0};

                        DmaTranferCallback = 0;

                        channel = DmaGetChannel();

                        if (channel != DMA_FALSE)
                        {
                            DmaStart((uint32)(channel), (UINT32)(gpMediaBlock.Outptr), (uint32)(&DecDataBuf[1][0]), (gpMediaBlock.OutLength * gpMediaBlock.Bps) / 16, &DmaCfg, DMATranferCallBack);

                            while (1)
                            {
                                __WFI();

                                if (DmaTranferCallback == 1)
                                    break;
                            }
                        }
                    }
#else
                    memcpy(&DecDataBuf[1][0], (uint8*)(gpMediaBlock.Outptr) , (gpMediaBlock.OutLength * gpMediaBlock.Bps) / 4);
#endif

                    *(int *)ulParam1 = (uint32)&DecDataBuf[1][0];
                    DecBufID = 0;
                }

                return (1);
            }
            else if (gpMediaBlock.Decoding == 0)
            {
                gpMediaBlock.Decoding = 1;
                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DECODE, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                return 2;
            }
            else if (gpMediaBlock.Decoding == 1)
            {
                return 2;
            }

            return (0);
        }

        case SUBFN_CODEC_SEEK:
        {
            gSeekDone = 0;
            MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DECODE_SEEK, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            MailBoxWriteA2BData(ulParam1, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            timeout = 200000;

            while (!gSeekDone)
            {
                #ifdef _WATCH_DOG_
                WatchDogReload();
                #endif
                
                BBDebug();
                //__WFI();
                DelayUs(1);
                if (--timeout == 0)
                {
                    DEBUG("SUBFN_CODEC_SEEK: timeout!!!");
                    break;
                }
                
                #ifdef _SDCARD_
                if (TRUE == IsSDCardRemoved())
                {
                    break;
                }
                #endif
            }

            gSeekDone = 0;
            return (1);
        }

        case SUBFN_CODEC_GETTIME:
        {
            *(unsigned long *)ulParam1 = gpMediaBlock.CurrentPlayTime;
            return (1);
        }

        case SUBFN_CODEC_GETSAMPLERATE:
        {
            *(int *)ulParam1 = gpMediaBlock.SampleRate;
            return  1;
        }

        case SUBFN_CODEC_GETCHANNELS:
        {
            *(int *)ulParam1 = gpMediaBlock.Channel;
            return 1;
        }

        case SUBFN_CODEC_GETBPS:
        {
            *(int *)ulParam1 = gpMediaBlock.Bps;
            return 1;
        }

        case SUBFN_CODEC_GETBITRATE:
        {
            *(int *)ulParam1 = gpMediaBlock.BitRate;
            return 1;
        }

        // Return the length (in milliseconds) of the file.
        case SUBFN_CODEC_GETLENGTH:
        {
            *(int *)ulParam1 = gpMediaBlock.TotalPlayTime;
            return 1;
        }


        // Cleanup after the codec.
        case SUBFN_CODEC_CLOSE:
        {
            gCloseDone = 0;
            MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DECODE_CLOSE, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            timeout = 3000000;

            while (!gCloseDone)
            {
                #ifdef _WATCH_DOG_
                WatchDogReload();
                #endif
                
                BBDebug();
                
                //__WFI();
                DelayUs(1);
                if (--timeout == 0)
                {
                    DEBUG("SUBFN_CODEC_CLOSE: timeout!!!");
                    break;
                }
                
                #ifdef _SDCARD_
                if (TRUE == IsSDCardRemoved())
                {
                    break;
                }
                #endif
            }

            gCloseDone = 0;
            return 1;
        }

        default:
        {
            // Return a failure.
            return (0);
        }
    }

#endif
}



#pragma arm section code
#endif
