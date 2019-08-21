
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
* FileName: BBSystem\Codecs\Audio\Decode\alac\pAlac.c
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
#include "audio_file_access.h"
#ifdef HIFI_AlAC_DECODE


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#include "audio_globals.h"
#include "alac.h"
#include "hifi_alac_MovFile.h"

#pragma arm section code = "AlacHDecCode", rodata = "AlacHDecCode", rwdata = "AlacHDecData", zidata = "AlacHDecBss"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define _BBSYSTEM_CODECS_AUDIO_DECODE_AlAC_PALAC_READ_  __attribute__((section("bbsystem_codecs_audio_decode_alac_palac_read")))
#define _BBSYSTEM_CODECS_AUDIO_DECODE_AlAC_PALAC_WRITE_ __attribute__((section("bbsystem_codecs_audio_decode_alac_palac_write")))
#define _BBSYSTEM_CODECS_AUDIO_DECODE_AlAC_PALAC_INIT_  __attribute__((section("bbsystem_codecs_audio_decode_alac_palac_init")))
#define _BBSYSTEM_CODECS_AUDIO_DECODE_AlAC_PALAC_SHELL_  __attribute__((section("bbsystem_codecs_audio_decode_alac_palac_shell")))


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#ifdef A_CORE_DECODE
//uint8 gAlacPingPangBuf[2][4096*8];
unsigned int Alac_out_Length ;
unsigned int  gAlacPingPangIndex = 0;
extern ALACContext alac_con;

extern unsigned long SRC_Num_Forehead;
uint8 *gAlacPingPangBuf[2];
#else
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
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/

FILE *alac_file_handle;
extern FILE *pRawFileCache;
int ALAC_filelen ;
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/



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
READ API unsigned long  HIFI_ALACDecFunction(unsigned long ulSubFn, unsigned long ulParam1,
                                              unsigned long ulParam2, unsigned long ulParam3)
{
    uint32 timeout;

#ifdef A_CORE_DECODE
    switch (ulSubFn)
    {
        case SUBFN_CODEC_OPEN_DEC:
            {
                int ret;
                alac_file_handle=(FILE*)pRawFileCache;
                HIFI_DMA_TO_register();
                ALAC_filelen = RKFIO_FLength(pRawFileCache);
                ret =  Alac_decode_init(alac_file_handle);
                if(ret < 0)
                {
                    return ret;
                }
                Alac_header_parse();
                Alac_out_Length =alac_con.max_samples_per_frame;
                return 1;
            }
        case SUBFN_CODEC_DECODE:
            {
                int out_size;
                Alac_out_Length = Alac_frame_decode(&gAlacPingPangBuf[gAlacPingPangIndex][SRC_Num_Forehead] ,&out_size);
                if(Alac_out_Length <= 0)
                {
                    return 0;
                }
                return 1;
            }
        case SUBFN_CODEC_GETBUFFER:
            {
                *(unsigned long *)ulParam1 = (unsigned long) &gAlacPingPangBuf[gAlacPingPangIndex][SRC_Num_Forehead];
                *(unsigned long *)ulParam2 = (unsigned long) Alac_out_Length;

                 gAlacPingPangIndex ^= 1;;
                 return(1);
            }


        case SUBFN_CODEC_SEEK:
            {
                 MovIF_SynAudio2Video_h((unsigned int )ulParam1 );
                 return(1);
            }

                    // Return the current position (in milliseconds) within the file.
        case SUBFN_CODEC_GETTIME:
            {
                *(int *)ulParam1 = (int ) MovIF_AudioGetCurrentTime_h();
                return(1);
            }

        case SUBFN_CODEC_GETSAMPLERATE:
            {
                unsigned long *SampleRate;
                SampleRate = (unsigned long *)ulParam1;
                *SampleRate = alac_con.samplerate;
                return  1;
            }

        case SUBFN_CODEC_GETCHANNELS:
            {
                unsigned long *channels;
                channels = (unsigned long *)ulParam1;
                *channels = alac_con.channels;
                return 1;
            }
        case SUBFN_CODEC_GETBPS:
            {
                unsigned long *bps;
                bps = (unsigned long *)ulParam1;
                *bps = alac_con.sample_size;
                return 1;
            }

        case SUBFN_CODEC_GETBITRATE:
            {
                unsigned long *bitrate;
                 extern int alac_data_size;
                bitrate = (unsigned long *)ulParam1;
                *bitrate = alac_con.average_bitrate;
                if(*bitrate == 0)
                    *bitrate =((long long )alac_data_size*8*gMovFile_h.audioTimeScale)/(gMovFile_h.uint32AudioDuration);
                *bitrate  += 500;
                return 1;
            }

            // Return the length (in milliseconds) of the file.
        case SUBFN_CODEC_GETLENGTH:
            {
                unsigned long *TimeLength;
                TimeLength = (unsigned long *)ulParam1;
                *TimeLength =  (gMovFile_h.uint32AudioDuration/gMovFile_h.audioTimeScale)*1000;
                return 1;
            }


            // Cleanup after the codec.
        case SUBFN_CODEC_CLOSE:
            {
              HIFI_DMA_TO_Unregister();
              return 1;
            }
        default:
            {
                // Return a failure.
                return(0);
            }
    }
#else
    switch (ulSubFn)
    {
        case SUBFN_CODEC_OPEN_DEC:
            {
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

                if( gError )    //codec decode open error
                    return 0;
                else
                    return(1);
            }
        case SUBFN_CODEC_DECODE:
            {
                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DECODE, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                return 1;
            }
        case SUBFN_CODEC_GETBUFFER:
            {
                if(gpMediaBlock.DecodeOver == 1)
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
                                 DmaStart((uint32)(channel), (UINT32)(gpMediaBlock.Outptr),(uint32)(&DecDataBuf[0][0]),(gpMediaBlock.OutLength * gpMediaBlock.Bps) / 16,&DmaCfg, DMATranferCallBack);

                                 while(1){
                                     __WFI();
                                     if(DmaTranferCallback == 1)
                                         break;
                                 }
                             }
                        }
                        #else

                        memcpy(&DecDataBuf[0][0],(uint8*)(gpMediaBlock.Outptr) ,(gpMediaBlock.OutLength * gpMediaBlock.Bps) / 4);

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
                                 DmaStart((uint32)(channel), (UINT32)(gpMediaBlock.Outptr),(uint32)(&DecDataBuf[1][0]),(gpMediaBlock.OutLength * gpMediaBlock.Bps) / 16,&DmaCfg, DMATranferCallBack);

                                 while(1){
                                     __WFI();
                                     if(DmaTranferCallback == 1)
                                         break;
                                 }

                             }

                        }
                        #else
                        memcpy(&DecDataBuf[1][0],(uint8*)(gpMediaBlock.Outptr) ,(gpMediaBlock.OutLength * gpMediaBlock.Bps) / 4);
                        #endif

                        *(int *)ulParam1 = (uint32)&DecDataBuf[1][0];
                        DecBufID = 0;
                    }

                    return(1);
                }
                else if(gpMediaBlock.Decoding == 0)
                {
                    gpMediaBlock.Decoding = 1;
                    MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DECODE, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                    MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                    return 2;
                }
                else if(gpMediaBlock.Decoding == 1)
                {
                    return 2;
                }

                return(0);
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
                return 1;
            }

        // Return the current position (in milliseconds) within the file.
        case SUBFN_CODEC_GETTIME:
            {
                *(unsigned long *)ulParam1 = gpMediaBlock.CurrentPlayTime;
                return(1);
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
                *(unsigned long *)ulParam1 = gpMediaBlock.TotalPlayTime;
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
                return(0);
            }
    }
#endif
}


#pragma arm section code
#endif

