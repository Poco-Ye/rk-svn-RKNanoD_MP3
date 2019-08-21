/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：  SysService.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-9-13          1.0
*    desc:
********************************************************************************
*/
#define _IN_SYSSERVICE_

#include "SysInclude.h"
#include "UsbAdapterProbe.h"

#include "FunUSBInterface.h"
#include "FsInclude.h"
#include "Mainmenu.h"
#include "RecordWinInterface.h"
#include "FMControl.h"
#include "AudioControl.h"
#include "VideoControl.h"
#include "PowerOn_Off.h"
#include "Fade.h"
#include "MediaBroWin.h"
#include "rockcodec.h"

extern void AudioSbcEncodeInit(void);
extern BOOLEAN VideoPause(void);

#if(CODEC_CONFIG == CODEC_ROCKC)
    _ATTR_SYS_DATA_ DMA_CFGX AudioControlDmaCfg  = {DMA_CTLL_I2S0_TX, DMA_CFGL_I2S0_TX, DMA_CFGH_I2S0_TX,0};
#else   //other codec( which not in i2s channel 0) dma config
    _ATTR_SYS_DATA_ DMA_CFGX AudioControlDmaCfg  = {DMA_CTLL_I2S1_TX, DMA_CFGL_I2S1_TX, DMA_CFGH_I2S1_TX,0};
#endif

////////////////////////////////////////////////////////////////////////////////
_ATTR_OVERLAY_CODE_
void GetBeepSourceInf(uint32 ModuleNum, uint32 *baseAddr, uint32 *moduleLen)
{
    uint32 i, Len;
    uint32 CodeInfoAddr;
    uint32 LoadStartBase;
    uint32 LoadBase;
    uint32 ImageBase;
    uint32 ImageLength;
    uint8  FlashBuf[512];
    uint8  *pBss;
    uint8  tempIsr;
    CODE_INFO_T Module;

    FIRMWARE_INFO_T *pFirmwareModuleInfo;

    //先读取固件相对起始地址
    MDReadData(SysDiskID, CodeLogicAddress, 512, FlashBuf);
    pFirmwareModuleInfo = (FIRMWARE_INFO_T *)FlashBuf;
    LoadStartBase = pFirmwareModuleInfo -> LoadStartBase;

    //读取模块信息CODE_INFO_T
    CodeInfoAddr  = CodeLogicAddress + sizeof(pFirmwareModuleInfo -> LoadStartBase);
    CodeInfoAddr  = CodeInfoAddr + sizeof(pFirmwareModuleInfo -> ModuleInfo.ModuleNum) + ModuleNum * sizeof(CODE_INFO_T);
    MDReadData(SysDiskID, CodeInfoAddr, sizeof(CODE_INFO_T), FlashBuf);
    Module = *(CODE_INFO_T *)FlashBuf;


    *baseAddr    = Module.CodeLoadBase - LoadStartBase + CodeLogicAddress;
    *moduleLen   = Module.CodeImageLength;

}

/*
--------------------------------------------------------------------------------
  Function name : Functions for beep
  Author        :
  Description   :
  Input         : null
  Return        : null
  History       : <author>         <time>         <version>

  desc          :
--------------------------------------------------------------------------------
*/
#ifdef _BEEP_

_ATTR_SYS_DATA_   __align(4) uint8  BeepBuffer[2048];
_ATTR_SYS_DATA_   uint32 BeepBaseAddr = 0;
_ATTR_SYS_DATA_   uint32 BeepLen = 0;
_ATTR_SYS_DATA_   uint32 BeepOffset = 0;
_ATTR_SYS_DATA_   uint32 CurBuferBank = 0;
_ATTR_SYS_DATA_   int    BeepLoopCnt = 0;
_ATTR_SYS_BSS_    uint8  gbMusicFade;
_ATTR_SYS_BSS_    uint8  gbMusicFadeOk;
_ATTR_SYS_BSS_    uint8  gbFadeDeep;

_ATTR_SYS_CODE_
void BeepInit(void)
{
    BeepPlayState = Voice_STOP;
    BeepPlayerState = Voice_STOP;

    AudioPlayState = AUDIO_STATE_STOP;
    AudioPlayerState = AUDIO_STATE_STOP;

    BeepSampleRate = FS_44100Hz;
    gbMusicFade = 0;
    gbFadeDeep = 1;


    I2SInit(I2S_CH, I2S_PORT, I2S_MODE,
            BeepSampleRate, I2S_FORMAT,
            I2S_DATA_WIDTH16, I2S_NORMAL_MODE);
    Codec_SetMode(Codec_DACoutHP, ACodec_I2S_DATA_WIDTH16);
    Codec_SetSampleRate(BeepSampleRate);

    IntPendingClear(FAULT_ID14_PENDSV);
    /*-------set the priority-----------*/
    IntPrioritySet(FAULT_ID14_PENDSV, 0xE0);

    /*----------register interupt callback--------------*/
    IntRegister(FAULT_ID14_PENDSV, AudioDecoding);

    /*----------interruption enable--------------*/
    IntEnable(FAULT_ID14_PENDSV);
}


_ATTR_SYS_CODE_
void BeepStop(void)
{
    uint32 timeout;

    if (BeepPlayerState != Voice_STOP)
    {
        if (BeepPlayState != Voice_STOP)
        {
            UserIsrDisable();
            BeepLoopCnt = 0;
            UserIsrEnable();

            while (BeepPlayState != Voice_STOP)
            {
                DelayMs(1);
            }
        }

        UserIsrDisable();
        //BeepPlayState = Voice_STOP;
        if (ThreadCheck(pMainThread, &MusicThread) == TRUE)
        {
            if (AudioPlayState != AUDIO_STATE_PLAY)
            {
                timeout = 200;
                while (DmaGetState(AUDIO_DMACHANNEL_IIS) == DMA_BUSY)
                {
                    DelayMs(1);
                    if (--timeout == 0)
                    {
                        break;
                    }
                }

                #ifdef _BEEP_NOT_MIXED_
                {
                    //Codec_SetSampleRate(pAudioRegKey->samplerate);
                }
                #endif
                //Codec_SetVolumet(AudioPlayInfo.VolumeCnt);
                DmaTransting = 0;
                AudioDecodeing = 0;
            }
            #ifdef _BEEP_NOT_MIXED_
            else
            {
                //Codec_SetSampleRate(pAudioRegKey->samplerate);
                if (gbMusicFade)
                {
                    gbMusicFade = 0;
                    gbFadeDeep = 1;
                }
                //Codec_SetVolumet(AudioPlayInfo.playVolume);
                AudioErrorFrameNum = 2;
            }
            #endif
        }
        else
        {
            timeout = 200;
            while (DmaGetState(AUDIO_DMACHANNEL_IIS) == DMA_BUSY)
            {
                DelayMs(1);
                if (--timeout == 0)
                {
                    break;
                }
            }
            I2SStop(I2S_CH,I2S_START_DMA_TX);
            I2SDeInit(I2S_CH, I2S_PORT);

            if (ThreadCheck(pMainThread, &FMThread) != TRUE)
            {
                Codec_ExitMode(Codec_DACoutHP);
            }
            else
            {
                I2SInit(I2S_CH, I2S_PORT, I2S_MODE,
                        BeepSampleRate_FM, I2S_FORMAT,
                        I2S_DATA_WIDTH16, I2S_NORMAL_MODE);
                Codec_SetSampleRate(BeepSampleRate_FM);
                DelayMs(5);
                Codec_SetMode(Codec_Line1in,BeepDatawidth_FM);
                DelayMs(5);
                Codec_SetMode(Codec_DACoutHP,BeepDatawidth_FM);
                Codec_DACUnMute();

                #ifdef _BEEP_NOT_MIXED_
                {
                    if (FmStandbyFlag == FALSE)//HJ
                    {
                        FM_Codec_SetVolumet(gSysConfig.OutputVolume);
                        FmDriverTable.Tuner_MuteControl(0);
                    }
                    else
                    {
                        FM_Codec_SetVolumet(gSysConfig.OutputVolume);
                        FmDriverTable.Tuner_MuteControl(1);//HJ
                    }
                }
                #endif
            }

            DmaTransting = 0;
            AudioDecodeing = 0;
            //ScuClockGateCtr(CLOCK_GATE_I2S, 0);
        }

        BeepPlayerState = Voice_STOP;

        //Rk Aaron.sun for Beep Crash Voice Start
        if (gbMusicFade)
        {
            gbMusicFade = 0;
            gbFadeDeep = 1;
        }
        //Rk Aaron.sun for Beep Crash Voice end

        UserIsrEnable();
        FREQ_ExitModule(FREQ_BEEP);
    }

}

_ATTR_SYS_CODE_
void BeepSourceInit(uint32 Samplerate, uint8 BeepID, int LoopCnt)
{

    BeepOffset = 0;
    BeepBaseAddr = 0;
    BeepLen = 0;
    BeepLoopCnt = LoopCnt - 1;

    //DEBUG("Beep Samplerate = %d, BeepID = %d", Samplerate, BeepID);

    switch (Samplerate)
    {
        case FS_8000Hz:
            Samplerate = 0;
            break;

        case FS_11025Hz:
            Samplerate = 1;
            break;

        case FS_12KHz:
            Samplerate = 2;
            break;

        case FS_16KHz:
            Samplerate = 3;
            break;

        case FS_22050Hz:
            Samplerate = 4;
            break;

        case FS_24KHz:
            Samplerate = 5;
            break;

        case FS_32KHz:
            Samplerate = 6;
            break;

        case FS_44100Hz:
            Samplerate = 7;
            break;

        case FS_48KHz:
            Samplerate = 8;
            break;

    }

    GetBeepSourceInf(MODULE_ID_SEID0000_8K +  Samplerate * BEEP_NUM + BeepID, &BeepBaseAddr, &BeepLen);
}

_ATTR_SYS_CODE_
uint16 BeepRead(uint8 *Buf, uint16 Size)
{
    uint16 readlen;


    if (BeepOffset + Size > BeepLen)
    {
        if (BeepOffset == BeepLen)
        {
            if (BeepLoopCnt == -2)
            {
                BeepOffset = 0;
            }
            else if (BeepLoopCnt > 0)
            {
                BeepLoopCnt--;
                BeepOffset = 0;
            }
            else
            {
                return 0;
            }
            readlen = Size;
        }
        else
        {
            if (BeepLoopCnt == -2)
            {
                readlen = BeepLen - BeepOffset;
                ModuleOverlayLoadData(BeepBaseAddr + BeepOffset, (uint32)Buf, readlen);
                BeepOffset = 0;
                ModuleOverlayLoadData(BeepBaseAddr + BeepOffset, (uint32)Buf + readlen, Size - readlen);
                BeepOffset += (Size - readlen);

                return Size;
            }
            else if (BeepLoopCnt > 0)
            {
                BeepLoopCnt--;
                readlen = BeepLen - BeepOffset;
                ModuleOverlayLoadData(BeepBaseAddr + BeepOffset, (uint32)Buf, readlen);
                BeepOffset = 0;
                ModuleOverlayLoadData(BeepBaseAddr + BeepOffset, (uint32)Buf + readlen, Size - readlen);
                BeepOffset += (Size - readlen);

                return Size;
            }
            else
            {
                readlen = BeepLen - BeepOffset;
                ModuleOverlayLoadData(BeepBaseAddr + BeepOffset, (uint32)Buf, readlen);
                BeepOffset += readlen;

                return readlen;
            }
        }
    }
    else
    {
        readlen = Size;
    }

    ModuleOverlayLoadData(BeepBaseAddr + BeepOffset, (uint32)Buf, readlen);
    BeepOffset += readlen;

    return readlen;
}



_ATTR_SYS_CODE_
void BeepPlay(uint8 BeepId, int LoopCnt, uint8 BeepVolume, uint8 MusicFade, uint8 MusicFadeDeep)
{
    uint16 readlen;
    uint32 timeout;
    short *pBuffer1, * pBuffer2, * pBuffer3;
    int   k = 0;
    if (BeepPlayerState != Voice_PLAY)
    {
        if (AudioPlayerState == AUDIO_STATE_PLAY)
        {
            if (!CheckMsg(MSG_MUSIC_FADE_OK))
            {
                return;
            }
        }

        FREQ_EnterModule(FREQ_BEEP);

        if (MusicFade)
        {
            gbFadeDeep = MusicFadeDeep;
            gbMusicFade = 1;
            gbMusicFadeOk = 0;
            timeout = 0xfffff;
            while (gbMusicFadeOk <= 3)
            {
                timeout--;
                if (timeout <= 10)
                {
                    break;
                }
            }
        }

        BeepSourceInit(BeepSampleRate, BeepId, LoopCnt);

        UserIsrDisable();
        BeepPlayerState = Voice_PLAY;

        if (ThreadCheck(pMainThread, &MusicThread) == TRUE)
        {
            if (AudioPlayState != AUDIO_STATE_PLAY)
            {
                CurBuferBank = CurBuferBank? 0:1;

                AudioPtr = (uint32)(BeepBuffer + 1024 * CurBuferBank);

                AudioLen = 256;

                readlen = BeepRead((unsigned char*)AudioPtr, AudioLen * 4);

                pBuffer3 = (unsigned short*)AudioPtr;
                if (readlen < (AudioLen * 4))
                {
                    memset(((unsigned char*)AudioPtr) + readlen, 0x00,  AudioLen * 4 - readlen);
                }
                else
                {
                    for (k = 0; k < AudioLen * 2; k++)
                    {
                        pBuffer3[k] = pBuffer3[k] / 2;
                    }
                }

                ScuSoftResetCtr(I2S0_SRST, 0);
                I2SInit(I2S_CH0,I2S_SLAVE_MODE,BeepSampleRate,I2S_FORMAT,I2S_DATA_WIDTH16,I2S_NORMAL_MODE);

                ACodec_Set_I2S_Mode(TFS_TX_I2S_MODE,ACodec_I2S_DATA_WIDTH16,IBM_TX_BUS_MODE_NORMAL,I2S_MST_MASTER);
                Codec_SetSampleRate(BeepSampleRate);
                Codec_SetMode(Codec_DACoutHP,BeepSampleRate);

                #ifdef _BEEP_NOT_MIXED_
                {
                    Codec_SetVolumet(BeepVolume);
                }
                #else
                {
                    Codec_SetVolumet(gSysConfig.OutputVolume);
                }
                #endif
                DelayMs(10);

                BeepPlayState = Voice_PLAY;
                DmaTransting = 1;
                I2SStart(I2S_CH0,I2S_START_DMA_TX);
                DmaStart(AUDIO_DMACHANNEL_IIS, (UINT32)AudioPtr, (uint32)(&(I2s_Reg->I2S_TXDR)),
                         AudioLen, &AudioControlDmaCfg, AudioDmaIsrHandler);

                AudioDecodeing = 1;
                UserIsrRequest();
            }
            else
            {
                BeepPlayState = Voice_PLAY;

                #ifdef _BEEP_NOT_MIXED_
                {
                    if (MusicFade)
                    {
                        gbFadeDeep = MusicFadeDeep;
                        gbMusicFade = 1;
                    }
                    Codec_SetVolumet(BeepVolume);
                }
                #endif
            }
        }
        else
        {
            CurBuferBank = CurBuferBank? 0:1;
            AudioPtr = (uint32)(BeepBuffer + 1024 * CurBuferBank);
            AudioLen = 256;

            readlen = BeepRead((unsigned char*)AudioPtr, AudioLen * 4);
            pBuffer3 = (unsigned short*)AudioPtr;
            if (readlen < (AudioLen * 4))
            {
                memset(((unsigned char*)AudioPtr) + readlen, 0x00,  AudioLen * 4 - readlen);
            }
            else
            {
                for (k = 0; k < AudioLen * 2; k++)
                {
                    pBuffer3[k] = pBuffer3[k] / 2;
                }
            }
            #ifdef _BEEP_NOT_MIXED_
            {
                #ifdef _RADIO_
                if (ThreadCheck(pMainThread, &FMThread) == TRUE)
                {
                    FmDriverTable.Tuner_MuteControl(1);
                }
                #endif
                ACodec_Set_I2S_Mode(TFS_TX_I2S_MODE,ACodec_I2S_DATA_WIDTH16,IBM_TX_BUS_MODE_NORMAL,I2S_MST_MASTER);
                Codec_SetSampleRate(BeepSampleRate);
                Codec_SetMode(Codec_DACoutHP,BeepSampleRate);
            }
            #else
            {
                #ifdef _RADIO_
                if (ThreadCheck(pMainThread, &FMThread) == TRUE)
                {
                    Codec_SetSampleRate(BeepSampleRate);
                    Codec_SetMode(Codec_Line2in,BeepSampleRate);
                }
                else
                #endif
                {
                    ACodec_Set_I2S_Mode(TFS_TX_I2S_MODE,ACodec_I2S_DATA_WIDTH16,IBM_TX_BUS_MODE_NORMAL,I2S_MST_MASTER);
                    Codec_SetSampleRate(BeepSampleRate);
                    Codec_SetMode(Codec_DACoutHP,BeepSampleRate);
                }
            }
            #endif

            ScuSoftResetCtr(I2S0_SRST, 0);
            I2SInit(I2S_CH0,I2S_SLAVE_MODE,BeepSampleRate,I2S_FORMAT,I2S_DATA_WIDTH16,I2S_NORMAL_MODE);

            timeout = 200;
            while (DmaGetState(AUDIO_DMACHANNEL_IIS) == DMA_BUSY)
            {
                DelayMs(1);
                if (--timeout == 0)
                {
                    break;
                }
            }

            #ifdef _BEEP_NOT_MIXED_
            {
                Codec_SetVolumet(BeepVolume);
            }
            #else
            {
                Codec_SetVolumet(gSysConfig.OutputVolume);
            }
            #endif

            BeepPlayState = Voice_PLAY;
            DmaTransting = 1;
            DmaStart(AUDIO_DMACHANNEL_IIS, (UINT32)AudioPtr, (uint32)(&(I2s_Reg->I2S_TXDR)),
                     AudioLen, &AudioControlDmaCfg, AudioDmaIsrHandler);

            AudioDecodeing = 1;
            UserIsrRequest();
            I2SStart(I2S_CH0,I2S_START_DMA_TX);
        }
        UserIsrEnable();
    }
}

_ATTR_SYS_CODE_
void BeepSever(void)
{
    uint8 VolumeCnt;

    if (GetMsg(MSG_BEEP_END))
    {
        BeepStop();
    }
}

#else //_BEEP_
void unBeepInit(void)
{
    AudioPlayState = AUDIO_STATE_STOP;
    AudioPlayerState = AUDIO_STATE_STOP;

    IntPendingClear(FAULT_ID14_PENDSV);
    /*-------set the priority-----------*/
    IntPrioritySet(FAULT_ID14_PENDSV, 0xE0);

    /*----------register interupt callback--------------*/
    IntRegister(FAULT_ID14_PENDSV, AudioDecoding);


    /*----------interruption enable--------------*/
    IntEnable(FAULT_ID14_PENDSV);
}
#endif

/*
--------------------------------------------------------------------------------
  Function name : AudioDmaIsrHandler
  Author        :
  Description   :
  Input         : null
  Return        : null
  History       : <author>         <time>         <version>

  desc          :
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void AudioDmaIsrHandler(void)
{
    if (AudioDecodeing == 1)
    {
        printf("\nAudioDmaIsrHandler AudioDecodeing = 1");
        DmaTransting = 0;
        return;
    }

    #ifdef _BEEP_
    if (BeepPlayState == Voice_PLAY)
    {
        DmaStart(AUDIO_DMACHANNEL_IIS, (UINT32)AudioPtr, (uint32)(&(I2s_Reg->I2S_TXDR)),  AudioLen, &AudioControlDmaCfg, AudioDmaIsrHandler);
        AudioDecodeing = 1;
        UserIsrRequest();
    }
    else
    #endif
    {
        if (AUDIO_STATE_PLAY == AudioPlayState)
        {
            DmaStart(AUDIO_DMACHANNEL_IIS, (UINT32)AudioPtr, (uint32)(&(I2s_Reg->I2S_TXDR)),  AudioLen, &AudioControlDmaCfg, AudioDmaIsrHandler);
            AudioDecodeing = 1;
            UserIsrRequest();
        }
        else
        {
            //printf("\n DMA end");
        }
    }
}

/*
--------------------------------------------------------------------------------
  Function name : Bit_Convertor
  Author        :
  Description   :
  Input         : null
  Return        : null
  History       : <author>         <time>         <version>

  desc          :
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void Bit_Convertor(short *ppsBuffer, long *plLength,int bps)//直接输出，低位补0
{
    int i = 0;
    int offset = 0 ;

    #ifdef CODEC_24BIT
    {
        if (bps == 16)
        {
            short *pOut = ppsBuffer;
            offset = *plLength*2;
            for (i=offset-1; i >= 0 ; i--)
            {
                pOut[2*i+1] = pOut[i];
                pOut[2*i]   = 0;
            }
            *plLength = offset;
        }
        else if (bps == 24)
        {
            char *pOut =(char *) ppsBuffer;
            offset = *plLength*2;
            for (i=offset; i > 0 ; i--)
            {
                pOut[4*i-1]  = pOut[3*i-1];
                pOut[4*i-2]  = pOut[3*i-2];
                pOut[4*i-3]  = pOut[3*i-3];
                pOut[4*i-4]  = 0;
            }
            *plLength = *plLength*2;
        }
        else if (bps == 32)
        {
            *plLength = *plLength*2;
        }
    }
    #else
    {
        if (bps == 16)
        {
        }
        else if (bps == 24)
        {
            char *pOut = (char *)ppsBuffer;
            offset = *plLength*2;
            for (i=0; i < offset ; i++)
            {
                pOut[2*i]  = pOut[3*i+1];
                pOut[2*i+1]= pOut[3*i+2];
            }
        }
        else if (bps == 32)
        {
            char *pOut = (char *)ppsBuffer;
            offset = *plLength*2;
            for (i=0; i < offset ; i++)
            {
                pOut[2*i]  = pOut[4*i+2];
                pOut[2*i+1]= pOut[4*i+3];
            }
        }
    }
    #endif
}

/*
--------------------------------------------------------------------------------
  Function name : Bit_Convertor_DEC
  Author        :
  Description   : Convert to 24bit
  Input         : null
  Return        : null
  History       : <author>         <time>         <version>

  desc          : Convert to low 24bit for EQ, fade process...;
                  paired with Bit_Convertor_shift() after EQ process to 32bit
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void Bit_Convertor_DEC(short *ppsBuffer, long *plLength,int bps)//紧接解码器输出，高位补0，用于EQ等运算
{
    int i = 0;
    int offset = 0;

    #ifdef CODEC_24BIT
    {
        if (bps == 16)
        {
            short *pOut = ppsBuffer;
            long *pOut32 =(long *) ppsBuffer;
            offset = *plLength*2;
            for (i=offset-1; i >= 0 ; i--)
            {
                pOut32[i] = (pOut[i]<<16)>>8;
            }
        }
        else if (bps == 24)
        {
            char *pOut =(char *) ppsBuffer;
            long *pOut32 =(long *) ppsBuffer;
            offset = *plLength*2;
            for (i=offset; i > 0 ; i--)
            {
                pOut32[i-1]  =( pOut[3*i-1]<<24 | pOut[3*i-2]<<16| pOut[3*i-3]<<8)>>8;
            }
        }
        else if (bps == 32)
        {
            long *pOut =(long *) ppsBuffer;
            offset = *plLength*2;
            for (i=offset-1; i >= 0 ; i--)
            {
                pOut[i] = pOut[i] >> 8;
            }
        }
    }
    #else
    {
        if (bps == 16)
        {
        }
        else if (bps == 24)
        {
            char *pOut = (char *)ppsBuffer;
            offset = *plLength*2;
            for (i=0; i < offset ; i++)
            {
                pOut[2*i]  = pOut[3*i+1];
                pOut[2*i+1]= pOut[3*i+2];
            }
        }
        else if (bps == 32)
        {
            char *pOut = (char *)ppsBuffer;
            offset = *plLength*2;
            for (i=0; i < offset ; i++)
            {
                pOut[2*i]  = pOut[4*i+2];
                pOut[2*i+1]= pOut[4*i+3];
            }
        }
    }
    #endif
}

/*
--------------------------------------------------------------------------------
  Function name : Bit_Convertor_shift
  Author        :
  Description   : shift 24bit to 32bit
  Input         : null
  Return        : null
  History       : <author>         <time>         <version>

  desc          : shift 24bit to 32bit
                  paired with Bit_Convertor_DEC() after EQ process to 32bit
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void Bit_Convertor_shift(short *ppsBuffer, long *plLength,int bps)//经过EQ等处理后的数据传送
{
    int i = 0;
    int offset = 0;

    #ifdef CODEC_24BIT
    {
        long *pOut =(long *) ppsBuffer;
        offset = *plLength * 2;
        for (i=offset-1; i >= 0 ; i--)
        {
            pOut[i] = pOut[i] << 8;
        }

        *plLength = *plLength*2;
    }
    #endif
}

extern FILE *pRawFileCache,*pFlacFileHandleBake;
/*
--------------------------------------------------------------------------------
  Function name : BOOLEAN AudioPlay(void)
  Author        : zs
  Description   : audio decode,decode next frame.
  Input         : null
  Return        : null
  History       : <author>         <time>         <version>
                   zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void AudioDecoding(void)
{
    short *pBuffer1, * pBuffer2, * pBuffer3;
    int  i,j,k = 0;
    int32 DecodeErr;
    uint32 readlen, totalRead;
    int32 DataTemp;

    #if (2 == FW_IN_DEV)
    {
        if (CheckCard() == 0)
        {
            //printf("\nNO Card audio decoding!!!");
            return;
        }
    }
    #endif

    AudioDecodeCnt++;

    if (AUDIO_STATE_PLAY == AudioPlayState)
    {
        if (AudioCodecOpenErr)
        {
            AudioPlayState = AUDIO_STATE_STOP;
            SendMsg(MSG_AUDIO_FILE_ERROR);
            goto BEEP;
        }

        #ifdef A_CORE_DECODE
        {
            AudioFileInput(pRawFileCache);
            DecodeErr = CodecDecode();

            #ifdef MP3_DEC_INCLUDE
            if (CurrentCodec == CODEC_MP3_DEC)
            {
                mp3_wait_synth();
            }
            #endif
            CodecGetCaptureBuffer((short*)&AudioPtr, &AudioLen);
        }
        #else
        {
            DecodeErr = CodecGetCaptureBuffer((short*)&AudioPtr, &AudioLen);
        }
        #endif

        if (0 == DecodeErr)   // Decoding end or error
        {
            if (FileInfo[(uint32)pRawFileCache].Offset <= FileInfo[(uint32)pRawFileCache].FileSize / 2
                && (gSysConfig.MusicConfig.RepeatMode != AUDIO_REPEAT)
                && (gSysConfig.MusicConfig.RepeatMode != AUDIO_REPEAT1))
            {
                AudioPlayState = AUDIO_STATE_STOP;
                memset((uint8*)AudioPtr, 0x00, AudioLen * 4);
                SendMsg(MSG_AUDIO_FILE_ERROR);
            }
            else
            {
                AudioPlayState = AUDIO_STATE_STOP;
                memset((uint8*)AudioPtr, 0x00, AudioLen * 4);
                SendMsg(MSG_AUDIO_DECODE_END);
            }
            AudioDecodeing = 0;
            return;
        }

        #ifdef _RK_CUE_
        if(AudioFileInfo.FindData.IsCue == 1)
        {
            if(pAudioRegKey->CurrentTime >= AudioFileInfo.FindData.CueEndTime)
            {
                AudioPlayState = AUDIO_STATE_STOP;
                memset((uint8*)AudioPtr, 0x00, AudioLen * 4);
                SendMsg(MSG_AUDIO_DECODE_END);

                AudioDecodeing = 0;
                return;
            }
        }
        #endif

        #ifndef A_CORE_DECODE
        /*else*/ if (2 == DecodeErr)
        {
            AudioNeedDecode = 1;
            return;
        }
        #endif

       /* #ifdef _A2DP_SOUCRE_
        {
            AudioSbcEncodeInit();
        }
        #endif*/

        Bit_Convertor_DEC((short*)AudioPtr, &AudioLen,  pAudioRegKey->bps);

        // fade in init
        if (AudioErrorFrameNum < 4)
        {
            if (++AudioErrorFrameNum >= 4)
            {
                //MusicNextFile = 1;
                SendMsg(MSG_MUSIC_FADE_IN);
                #ifdef _FADE_PROCESS_
                AudioPlayInfo.VolumeCnt = AudioPlayInfo.playVolume;
                Codec_SetVolumet(AudioPlayInfo.playVolume);
                FadeInit(pAudioRegKey->samplerate,pAudioRegKey->samplerate / 32,FADE_IN);
                SendMsg(MSG_MUSIC_FADE_OK);
                AudioEndFade = 0;
                #endif
            }
            else
            {
                #ifdef CODEC_24BIT
                {
                    memset((uint8*)AudioPtr, 0x00, AudioLen * 8);
                }
                #else
                {
                    memset((uint8*)AudioPtr, 0x00, AudioLen * 4);
                }
                #endif
            }
        }

        // last 500ms for Fadeout
        #ifdef _FADE_PROCESS_
        {
            #ifdef _RK_CUE_
            if(AudioFileInfo.FindData.IsCue == 1)
            {
                if ((AudioFileInfo.FindData.CueEndTime - pAudioRegKey->CurrentTime) <= 300)
                {
                    if (FadeIsFinished())
                    {
                        if (AudioEndFade == 0)
                        {
                            FadeInit(0,pAudioRegKey->samplerate/4,FADE_OUT);
                            AudioEndFade = 1;
                        }
                    }
                }
            }
            else
            #endif
            {
                if ((pAudioRegKey->TotalTime - pAudioRegKey->CurrentTime) <= 300)
                {
                    if (FadeIsFinished())
                    {
                        if (AudioEndFade == 0)
                        {
                            FadeInit(pAudioRegKey->samplerate,pAudioRegKey->samplerate/4,FADE_OUT);
                            AudioEndFade = 1;
                        }
                    }
                }
            }
        }
        #endif

        #ifdef _RK_EQ_
        {
            #ifdef NEW_EQ
            {
                EffectProcess((EQ_TYPE *)AudioPtr, AudioLen);
            }
            #else
            if(EQ_NOR != AudioIOBuf.EffectCtl.Mode)
            {
                /*audio effect process.*/
                if(pAudioRegKey->samplerate <= 96000)
                {
                    EffectProcess((EQ_TYPE *)AudioPtr, AudioLen);
                }
            }
            #endif
        }
        #endif

        #ifdef _FADE_PROCESS_
        {
            if (!FadeIsFinished())
            {
                #ifdef CODEC_24BIT
                {
                    FadeProcess((long*)AudioPtr,AudioLen);
                }
                #else
                {
                    FadeProcess((short*)AudioPtr,AudioLen);
                }
                #endif
            }
            else if (AudioEndFade == 1)
            {
                #ifdef CODEC_24BIT
                {
                    memset((uint8*)AudioPtr, 0x00, AudioLen * 8);
                }
                #else
                {
                    memset((uint8*)AudioPtr, 0x00, AudioLen * 4);
                }
                #endif
            }
        }
        #endif

        #ifdef _RK_SPECTRUM_
        if (FALSE == CheckMsg(MSG_BL_OFF))
        {
            short Spectrum_data[128];
            if (TRUE == GetMusicUIState())//backgroud need not to compute the spectrum.
            {
                if (SpectrumLoop == 0)
                {
                    if (SpectrumCnt == 0)
                        memset(SpectrumOut, 0, SPECTRUM_LINE_M*sizeof(char));


                    if ((AUDIO_STATE_PLAY == AudioPlayState) && (SpectrumCnt < 3))
                    {
                        #ifdef CODEC_24BIT
                        {
                           int i = 0;
                           long *ptr = (long*)AudioPtr;
                           for(i=0;i<128;i++)
                            {
                              Spectrum_data[i] = (short)((*ptr) >>8);
                              ptr++;
                            }
                        }
                        #else
                        {
                            memcpy(Spectrum_data, (short*)AudioPtr, 256);
                        }
                        #endif
                        DoSpectrum((short*)Spectrum_data,&SpectrumOut[0]);

                        SpectrumCnt++;
                        if (SpectrumCnt >= 3)
                        {
                            SpectrumLoop = SpectrumLoopTime;
                            SendMsg(MUSIC_UPDATESPECTRUM);
                            SpectrumCnt = 0;
                        }
                    }
                }

                if (SpectrumLoop > 0)
                {
                    SpectrumLoop-- ;
                }
            }
        }
        #endif
    }

BEEP:
    #ifdef _BEEP_
    {
        if (gbMusicFade && (BeepPlayState != Voice_PLAY))
        {
            pBuffer1 = (short*)AudioPtr;
            for (i = 0; i < (AudioLen * 2); i++)
            {
                pBuffer1[i]  >>= gbFadeDeep;
            }
            gbMusicFadeOk++;
        }

        if (BeepPlayState == Voice_PLAY)
        {
            if ((AUDIO_STATE_PLAY == AudioPlayState) && CheckMsg(MSG_MUSIC_FADE_OK))
            {
                totalRead = 0;
                pBuffer1 = (short*)AudioPtr;

                while (totalRead < AudioLen * 4)
                {
                    pBuffer2 = (short*)BeepBuffer;

                    if ((AudioLen * 4 - totalRead) < 2048)
                    {
                        readlen = (AudioLen * 4 - totalRead);
                    }
                    else
                    {
                        readlen = 2048;
                    }

                    readlen = BeepRead((unsigned char*)pBuffer2, readlen);

                    if ((readlen == 0) && (totalRead == 0))
                    {
                        BeepPlayState = Voice_STOP;
                        //BeepPlayerState = Voice_STOP;
                        SendMsg(MSG_BEEP_END);
                        break;
                    }

                    if (readlen == 0)
                    {
                        for (i = 0; i < (AudioLen * 4 - totalRead) / 2; i++)
                        {
                            pBuffer1[i]  >>= gbFadeDeep;
                        }
                        break;
                    }

                    for (i = 0; i < readlen / 2; i++)
                    {
                        DataTemp = ((uint32)pBuffer1[i] >> gbFadeDeep) + ((uint32)pBuffer2[i] >> 1);
                        pBuffer1[i] = DataTemp;
                    }

                    pBuffer1  += readlen / 2;
                    totalRead += readlen;
                }
            }
            else if (AUDIO_STATE_PLAY == AudioPlayState)
            {
                /*
                //AudioLen = 256;
                readlen = BeepRead((unsigned char*)AudioPtr, AudioLen * 4, NULL);

                if(readlen < (AudioLen * 4))
                    memset(((unsigned char*)AudioPtr) + readlen, 0x00,  AudioLen * 4 - readlen);

                printf("readlenX = %d, wantread = %d\n", readlen, AudioLen);

                if(readlen == 0)
                {
                    //BeepPlayState = Voice_STOP;
                    BeepOffset = 0;
                    readlen = BeepRead((unsigned char*)AudioPtr, AudioLen * 4, NULL);
                }
                */
            }
            else
            {
                CurBuferBank = CurBuferBank? 0:1;

                AudioPtr = (uint32)(BeepBuffer + 1024 * CurBuferBank);

                AudioLen = 256;

                readlen = BeepRead((unsigned char*)AudioPtr, AudioLen * 4);
                pBuffer3 = (unsigned short*)AudioPtr;
                if (readlen < (AudioLen * 4))
                    memset(((unsigned char*)AudioPtr) + readlen, 0x00,  AudioLen * 4 - readlen);
                else
                {
                    for (k = 0; k < AudioLen * 2; k++)
                    {
                        pBuffer3[k] = pBuffer3[k] / 2;
                    }
                }

                if (readlen == 0)
                {
                    #ifdef CODEC_24BIT
                    {
                        memset((uint8*)AudioPtr, 0x00, AudioLen * 8);
                    }
                    #else
                    {
                        memset((uint8*)AudioPtr, 0x00, AudioLen * 4);
                    }
                    #endif
                    BeepPlayState = Voice_STOP;
                    SendMsg(MSG_BEEP_END);
                }
            }
        }
    }
    #endif

    Bit_Convertor_shift((short*)AudioPtr, &AudioLen,  pAudioRegKey->bps);

    /* modified by cdd 20171101  */
    #ifdef _A2DP_SOUCRE_
    {
        AudioSbcEncodeInit();
    }
    #endif

    IntDisable(INT_ID_DMA);
    AudioDecodeing = 0;

    if (DmaTransting == 0)
    {
        AudioDmaIsrHandler();
    }

    DmaTransting = 1;
    IntEnable(INT_ID_DMA);
}



#ifdef _USB_
/*
--------------------------------------------------------------------------------
  Function name : UINT32 SysService(void)
  Author        : ZHengYongzhi
  Description   : 系统服务程序，该程序调用系统消息、系统线程、外设检测等后台需要
                  完成的工作

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
extern THREAD USBControlThread;

_ATTR_SYS_BSS_ uint32 USBDebounceCount;
_ATTR_SYS_BSS_ uint8  USBVbusStatus;
_ATTR_SYS_BSS_ uint8  VbusDetStatus;

#ifdef _USB_HOST_

_ATTR_SYS_CODE_
uint32 USBHost_Reinit(void)
{
    USBHostReInit();
}


_ATTR_SYS_CODE_
uint32 USBHostStatusDetect(void)
{
    uint32 retval = RETURN_OK;
    TASK_ARG TaskArg;
    UHC_CHN_INFO *pUsbHC = NULL;

    pUsbHC = (UHC_CHN_INFO *)HostGetChannelInfo();

    if (pUsbHC->ConnectStatus != 1)
    {
        gSysConfig.UsbHost = 0;
        gSysConfig.Listen_UsbHost = 0;
        USBHost_Reinit();
        if (MemorySelect == USBHOST_FLASH)
        {
            #ifdef _RADIO_
            if (TRUE != ThreadCheck(pMainThread, &FMThread))
            #endif

            #ifdef _MUSIC_
            {
                ThreadDelete(&pMainThread, &MusicThread);
            }

            if (Task.TaskID == TASK_ID_MUSIC)
            {
                retval = RETURN_FAIL;
                TaskArg.MainMenu.MenuID = MAINMENU_ID_MUSIC;
                TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
                //DEBUG ("HostDetect:0 MemorySelect = FLASH0");
                MemorySelect = FLASH0;
                FileSysSetup(MemorySelect);//USB host退出默认安装Flash0
                ClearMsg(MSG_MEDIA_NOW_PLAY);
                SendMsg(MSG_USBHOT_UPDATE);
                return(RETURN_FAIL);
            }
            #endif
            //DEBUG ("HostDetect:1 MemorySelect = FLASH0");
            MemorySelect = FLASH0;
            FileSysSetup(MemorySelect);//USB host退出默认安装Flash0
            SendMsg(MSG_USBHOT_UPDATE);
        }
    }
    else
    {
        //todo...
    }

    return (retval);
}
#endif

extern uint32 powerOnChargeFlag;
_ATTR_SYS_CODE_
uint32 USBStatusDetect(void)
{
    uint32 RetVal = 0;
    int32  UsbAdapterRet = 0;
    TASK_ARG TaskArg;
    uint8 VbusStatus = 0;
    uint8 VbusChanged = 0;

    VbusStatus = CheckVbus();
    if (USBVbusStatus != VbusStatus)
    {
        USBVbusStatus = VbusStatus;
        VbusChanged = 1;
    }

    if (VbusChanged)
    {
        VbusDetStatus = 0;
        if (VbusStatus)
        {
            VbusDetStatus = 1;
        }
        USBDebounceCount = SysTickCounter;
    }
    else
    {
        if (VbusStatus)
        {
            if (1 == VbusDetStatus)
            {
                if ((SysTickCounter-USBDebounceCount) > 50) // 500ms 防抖 有溢出风险
                {
                    if (FALSE == CheckMsg(MSG_VBUS_INSERT))
                    {
                        USBDEBUG("VBUS INSET");
                        //Vbus 从拔出到插入,若是USB HOST存在则反初始化USB HOST
                        #ifdef _USB_HOST_
                        USBHost_Reinit();
                        #endif
                        SendMsg(MSG_VBUS_INSERT);

                        AutoPowerOffDisable();
                        ThreadDeleteAll(&pMainThread);
                        #ifdef _BEEP_
                        BeepStop();
                        #endif

                        UsbAdpterProbeStart();
                        VbusDetStatus = 2;
                    }
                }
            }
            else if (2 == VbusDetStatus)
            {
                UsbAdapterRet = UsbAdpterProbe();

                if (1 == UsbAdapterRet)
                {
                    VbusDetStatus = 0;

                    if (GetMsg(MSG_MES_FIRMWAREUPGRADE))
                    {
                        TaskArg.Usb.FunSel = USB_CLASS_TYPE_UPGRADE;
                    }
                    else
                    {
                        #if 0
                        TaskArg.Usb.FunSel = USB_CLASS_TYPE_SERIAL;
                        #else
                        if (gSysConfig.UsbDeviceClass == 0)
						{
							TaskArg.Usb.FunSel = USB_CLASS_TYPE_MSC;
						}
						else if (gSysConfig.UsbDeviceClass == 1)
						{
							TaskArg.Usb.FunSel = USB_CLASS_TYPE_AUDIO;
						}
                        #endif
                    }
                    powerOnChargeFlag = 0;
                    TaskSwitch(TASK_ID_USB, &TaskArg);
                    RetVal = RETURN_FAIL;
                }
                else if (2 == UsbAdapterRet)
                {
                    //AC
                    SendMsg(MSG_CHARGE_ENABLE);

                    BatteryChargeInit();
                    SendMsg(MSG_CHARGE_START);      //使能充电

                    SendMsg(MSG_SYS_RESUME);

                    if (Task.TaskID != TASK_ID_CHARGE)
                    {
                        if (powerOnChargeFlag == 1)
                        {
                            powerOnChargeFlag = 0;
                            TaskSwitch(TASK_ID_CHARGE, NULL);
                            RetVal = RETURN_FAIL;
                        }
                    }
                }
            }
        }
        else
        {
            //if ((SysTickCounter-USBDebounceCount) > 20) // 200ms 防抖 有溢出风险
            {
                if (CheckMsg(MSG_VBUS_INSERT))
                {
                    USBDEBUG("VBUS REMOVE");
                    ClearMsg(MSG_VBUS_INSERT);
                    SendMsg(MSG_SYS_RESUME);

                    UsbAdpterProbeStop();

                    AutoPowerOffEnable();

                    BatteryChargeDeInit();

                    ClearMsg(MSG_CHARGE_ENABLE);
                    ClearMsg(MSG_CHARGE_START);
                }
            }
        }
    }

    return (RetVal);
}

#endif

#ifdef _SDCARD_
/*
--------------------------------------------------------------------------------
  Function name : UINT32 SysService(void)
  Author        : ZHengYongzhi
  Description   : 系统服务程序，该程序调用系统消息、系统线程、外设检测等后台需要
                  完成的工作

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void SDCardEnable(void)
{
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 SysService(void)
  Author        : ZHengYongzhi
  Description   : 系统服务程序，该程序调用系统消息、系统线程、外设检测等后台需要
                  完成的工作

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void SDCardDisable(void)
{
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 SysService(void)
  Author        : ZHengYongzhi
  Description   : 系统服务程序，该程序调用系统消息、系统线程、外设检测等后台需要
                  完成的工作

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
uint32 SDCardConter = 0;
uint32 SDStatus = 0;

_ATTR_SYS_CODE_
bool IsSDCardRemoved(void)
{
    if (gSysConfig.Memory == CARD)
    {
        if (TRUE == CheckMsg(MSG_SDCARD_EJECT))
        {
            //已经检测到卡拔出
            return TRUE;
        }
        
        if ((CheckCard() == 0) && (TRUE == CheckMsg(MSG_SDCARD_INSERT)))
        {
            //还未来得及检测到卡拔出，但是却检测到卡状态拔出
            return TRUE;
        }
    }
    
    return FALSE;
}

_ATTR_SYS_CODE_
uint32 SDCardStatusDetect(void)
{
    uint32 RetVal = 0;
    TASK_ARG TaskArg;

    if (CheckCard() == 0)
    {
        if (SDStatus != 0)
        {
            SDStatus = 0;
            DEBUG("SD Eject!!!");

            //if (FALSE == CheckMsg(MSG_SDCARD_EJECT))
            {
                FREQ_EnterModule(FREQ_BLON);
                SendMsg(MSG_SYS_RESUME);
                
                SendMsg(MSG_SDCARD_EJECT);                
                ClearMsg(MSG_SDCARD_INSERT);
                    
                SendMsg(MSG_SDCARD_UPDATE);
                SendMsg(MSG_SDCARD_MEM_UPDATE);

                #if (2 != FW_IN_DEV)
                if (gSysConfig.Memory == CARD)
                #endif
                {
                    #ifdef _RADIO_
                    if (TRUE != ThreadCheck(pMainThread, &FMThread))
                    #endif
                    {
                        #ifdef _MUSIC_
                        {
                            ThreadDelete(&pMainThread, &MusicThread);
                        }
                        #endif

                        #ifdef _RECORD_
                        {
                            ThreadDelete(&pMainThread, &RecordThread);
                        }
                        #endif
                    }

                    #ifdef _MUSIC_
                    if (Task.TaskID == TASK_ID_MUSIC)
                    {
                        RetVal = RETURN_FAIL;
                        TaskArg.MainMenu.MenuID = MAINMENU_ID_MUSIC;
                        TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
                    }
                    #endif

                    #ifdef _VIDEO_//ylz++
                    if (Task.TaskID == TASK_ID_VIDEO)
                    {
                        RetVal = RETURN_FAIL;
                        TaskArg.MainMenu.MenuID = MAINMENU_ID_VIDEO;
                        TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
                    }
                    #endif

                    #ifdef _RECORD_
                    if (Task.TaskID == TASK_ID_RADIO)
                    {
                        RetVal = RETURN_FAIL;
                        TaskArg.MainMenu.MenuID = MAINMENU_ID_RADIO;
                        TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
                    }
                    #endif

                    #ifdef _PICTURE_//ylz++
                    if (Task.TaskID == TASK_ID_PICTURE)
                    {
                        RetVal = RETURN_FAIL;
                        TaskArg.MainMenu.MenuID = MAINMENU_ID_PICTURE;
                        TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
                    }
                    #endif

                    #if (FW_IN_DEV != 2)
                    {
                        ClearMsg(MSG_MEDIA_NOW_PLAY);
                        gSysConfig.Memory = FLASH0;
                        FileSysSetup(gSysConfig.Memory);//dgl second filesystem
                    }
                    #endif
                }

                SDCardDisable();
                SDCardDeInit();//add by dgl
                
                ClearMsg(MSG_SDCARD_EJECT);
            }
        }
        else //if (SDStatus == 0)
        {
            //DEBUG("SD222 Eject!!!");
            //ClearMsg(MSG_SDCARD_INSERT);
        }
    }
    else
    {    
        if (SDStatus == 0)
        {
            SDStatus = 1;
            SDCardConter = 0;
        }
        else if (SDStatus == 1)
        {
            SDCardConter++;
            if (SDCardConter > 10)
            {
                SDStatus = 2;
                DEBUG("SD Insert!!!");
                
                //if (CheckMsg(MSG_SDCARD_EJECT))
                {
                    ClearMsg(MSG_SDCARD_EJECT);
                    SendMsg(MSG_SDCARD_INSERT);
                    
                    SendMsg(MSG_SDCARD_UPDATE);
                    SendMsg(MSG_SDCARD_MEM_UPDATE);
                    SendMsg(MSG_SYS_RESUME);
                    FREQ_EnterModule(FREQ_BLON);

                    SDCardEnable();
                    
                }
            }
        }
        else //if (SDStatus == 2)
        {
            //DEBUG("SD222 Insert!!!");
        }
    }

    return (RetVal);
}
#endif

/*
--------------------------------------------------------------------------------
  Function name : AdcKey_SysTimer_Handler()
  Author        : anzhiguo
  Description   : ADC Key Timer Isr

  Input         : null

  Return        : null

  History:     <author>         <time>         <version>
               anzhiguo        2009-1-14         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void AutoPowerDownTimerISR(void)
{
    SendMsg(MSG_POWER_DOWN);
}

_ATTR_SYS_CODE_
void SetPowerDownTimerISR(void)
{
    SetPowerOffFlag = 0;
    SendMsg(MSG_POWER_DOWN);
}

_ATTR_SYS_DATA_
SYSTICK_LIST AutoPowerDownTimer =
{
    NULL,
    0,
    10 * 60 * 100,
    1,
    AutoPowerDownTimerISR,
};

_ATTR_SYS_DATA_
SYSTICK_LIST SetPowerDownTimer =
{
    NULL,
    0,
    0,
    1,
    SetPowerDownTimerISR,
};

/*
--------------------------------------------------------------------------------
  Function name : void PowerOffDetec(void)
  Author        : ZHengYongzhi
  Description   : 关机检测

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void AutioPowerOffTimerRest(void)
{
    AutoPowerDownTimer.Counter = 0;
}

/*
--------------------------------------------------------------------------------
  Function name : void PowerOffDetec(void)
  Author        : ZHengYongzhi
  Description   : 关机检测

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void AutoPowerOffEnable(void)
{
    if (AutoPowerOffDisableCounter > 0)
    {
        AutoPowerOffDisableCounter--;

        if (AutoPowerOffDisableCounter == 0)
        {
            SystickTimerStart(&AutoPowerDownTimer);
        }
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void PowerOffDetec(void)
  Author        : ZHengYongzhi
  Description   : 关机检测

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void AutoPowerOffDisable(void)
{
    if (AutoPowerOffDisableCounter == 0)
    {
        SystickTimerStop(&AutoPowerDownTimer);
    }

    AutoPowerOffDisableCounter++;
}

/*
--------------------------------------------------------------------------------
  Function name : void PowerOffDetec(void)
  Author        : ZHengYongzhi
  Description   : 关机检测

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void SetPowerOffTimerEnable(void)
{
    uint32 PowerOffBuff[6] = {0, 10 * 60 * 100, 15 * 60 * 100, 30 * 60 * 100, 60 * 60 * 100, 120 * 60 * 100};

    if (gSysConfig.ShutTime)
    {
        SetPowerDownTimer.Counter = 0;
        SetPowerDownTimer.Reload = PowerOffBuff[gSysConfig.ShutTime];

        if (SetPowerOffFlag == 0)
        {
            SetPowerOffFlag = 1;
            SystickTimerStart(&SetPowerDownTimer);
        }
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void PowerOffDetec(void)
  Author        : ZHengYongzhi
  Description   : 关机检测

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void SetPowerOffTimerDisable(void)
{
    if (SetPowerOffFlag == 1)
    {
        SetPowerOffFlag = 0;
        SystickTimerStop(&SetPowerDownTimer);
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void PowerOffDetec(void)
  Author        : ZHengYongzhi
  Description   : 关机检测

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
uint32 PowerOffDetec(void)
{
    uint32 RetVal = RETURN_OK;
    TASK_ARG TaskArg;

    //--------------------------------------------------------------------------
    if (GetMsg(MSG_POWER_DOWN))
    {
        #if 0 //zyz: ???? #ifdef _MUSIC_#ifdef _MUSIC_
        if (TRUE == ThreadCheck(pMainThread, &MusicThread))
        {
            gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 1;
        }
        else
        {
            gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0;//HoldOnPlayInfo.HoldMusicGetSign ;
        }
        #endif

        #ifdef _VIDEO_
        {
            if (TRUE == ThreadCheck(pMainThread, &VideoThread))
            {
                gSysConfig.VideoConfig.HoldOnPlaySaveFlag = 1;
            }
            else
            {
                gSysConfig.VideoConfig.HoldOnPlaySaveFlag = VideoHoldOnPlayInfo.HoldVideoGetSign ;
            }
        }
        #endif

        #ifdef _RADIO_
        {
            if ((TRUE == ThreadCheck(pMainThread, &FMThread)) || (FmStandbyFlag == TRUE))
            {
                if (gpRadioplayerRegKey->FmState == FM_State_StepStation)
                {
                    gSysConfig.RadioConfig.HoldOnPlaySaveFlag = 2;
                }
                else
                {
                    gSysConfig.RadioConfig.HoldOnPlaySaveFlag = 1;
                }
            }
            else
            {
                gSysConfig.RadioConfig.HoldOnPlaySaveFlag = 0;
            }
        }
        #endif

        #ifdef _RECORD_
        if (TRUE == ThreadCheck(pMainThread, &RecordThread))
        {
            ThreadDelete(&pMainThread, &RecordThread);   //delete current thread,
        }
        #endif

        ThreadDeleteAll(&pMainThread);

        if (0)//((CheckVbus()) && (0 == Scu_DCout_Issue_State_Get()))
        {
            //TaskArg.Usb.FunSel = FUN_USB_CHARGE;
            TaskSwitch(TASK_ID_CHARGE, &TaskArg);
            RetVal = RETURN_FAIL;
        }
        else
        {
            SaveSysInformation(1);
            PowerOff();
        }
    }

    return(RetVal);
}

/*
--------------------------------------------------------------------------------
  Function name : void PowerOffDetec(void)
  Author        : ZHengYongzhi
  Description   : 关机检测

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void SysResume(void)
{
    if (TRUE == GetMsg(MSG_SYS_RESUME))
    {
        //KeyReset();
        BL_Resume();
        AutioPowerOffTimerRest();
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void SysBatteryStateChk(void)

  Author        : yangwenjie
  Description   : shutdown check

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void SysBatteryStateCheck(void)
{
    WIN    * PowerOffWin;

    if (CheckAdcState(ADC_CHANEL_BATTERY))
    {
        Battery_GetLevel();

        //low voltage check,
        if (TRUE == GetMsg(MSG_LOW_POWER) && (CheckVbus( ) == 0) )
        {
            SendMsg(MSG_SYS_RESUME);

            #ifdef _VIDEO_//ylz++
            if (TRUE == ThreadCheck(pMainThread, &VideoThread))
            {
                VideoPause();
            }
            #endif

            PowerOffWin = WinGetFocus(pMainWin);
            WinCreat(PowerOffWin, &LowPowerWin,NULL);
        }
    }
}

/*
--------------------------------------------------------------------------------
  Function name : FunUSBSysReboot
  Author        : ZhengYongzhi
  Description   : USB get system version number

  Input         :
  Return        : null

  History:     <author>         <time>         <version>
             ZhengYongzhi      2008-1-15          Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
//_ATTR_OVERLAY_CODE_

void SysReboot(uint32 addr, uint8 flag)
{
    void(*pgUpgrade)(void);
    uint32 temp, i;

    DEBUG("Reboot, addr = 0x%x, flag = %d", addr, flag);

    ThreadDeleteAll(&pMainThread);

    #ifdef _WATCH_DOG_
    {
        WatchDogDisableInt();
        WatchDogDeInit();
    }
    #endif

    BL_Off();
    LcdStandby();
    AdcPowerDown();
    Codec_DeInitial();

    if (flag & 0x01)
    {
        SaveSysInformation(1);
    }

    MDDeInitAll();
    DelayMs(5);

    // N   a  n   o   C  R   e  b    o  o  t   F    l    a   g
    // 4e 61 6e 6f, 43 52 65 62, 6f 6f 74 46, 6c 61 67
    RebootTag[0] = 0x6f6e614e;
    RebootTag[1] = 0x62655243;
    RebootTag[2] = 0x46746f6f;
    RebootTag[3] = 0x0067616c;

    SetPllDefault();

    //disable all interrupt
    nvic->SysTick.Ctrl     &= ~NVIC_SYSTICKCTRL_TICKINT;
    for (i = 0; i < 32; i++)
    {
        nvic->Irq.Disable[i]    = 0xffffffff;
        nvic->Irq.ClearPend[i]  = 0xffffffff;
    }

    for (i = 0; i < 10; i++)
    {
        CRU->CRU_CLKGATE_CON[i] = 0xffff0000;
    }
    ScuClockGateCtr(PCLK_WDT_GATE, FALSE);
    DelayUs(50);
    for (i = 0; i < 4; i++)
    {
        CRU->CRU_SOFTRST[i] = 0xffff0000;
    }

    for (i = 0; i < 4; i++)
    {
        Grf->GPIO_IO0MUX[i]  = 0xffff0000;
        Grf->GPIO_IO1MUX[i]  = 0xffff0000;
        Grf->GPIO_IO2MUX[i]  = 0xffff0000;

        Grf->GPIO_IO0PULL[i] = 0xffff0000;
        Grf->GPIO_IO1PULL[i] = 0xffff0000;
        Grf->GPIO_IO2PULL[i] = 0xffff0000;
    }

    {
        Gpio_SetPortDirec(GPIO_CH0, 0x00000000);
        Gpio_SetPortDirec(GPIO_CH1, 0x00000000);
        Gpio_SetPortDirec(GPIO_CH2, 0x00000000);
    }

    Grf_NOC_Remap_Sel(NOC_BOOT_ROM);

    nvic->VectorTableOffset = (UINT32)0x00000000;

    if (addr == 0x0000008c)//firmware update.
    {
        *((UINT32 *)0X03050000) = chip_freq.armclk;
        pgUpgrade = (void *)(*((uint32 *)addr));

        Grf_otgphy_suspend(0);
        (*pgUpgrade)();
    }
    else
    {
        if (flag & 0x02) //enter loader usb
        {
            //Grf->SOFT_DBG0 = (0x18AF|0xFFFF0000);
            Pmu_Reg->PMU_SYS_REG3 = (0x18AF|0xFFFF0000);
        }

        nvic->APIntRst = 0x05fa0007;       //system reseet.
    }

    while (1);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 SysService(void)
  Author        : ZHengYongzhi
  Description   : 系统服务程序，该程序调用系统消息、系统线程、外设检测等后台需要
                  完成的工作

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_INIT_CODE_
void SysServiceInit(void)
{
    DEBUG("system service initial...");

    #ifdef _USB_
    {
        USBVbusStatus = 0xff; //-1
    }
    #endif

    LcdStandbyEnable();
    BLOffEnable();

    AutoPowerOffDisableCounter = 1;
    AutoPowerOffEnable();

    SetPowerOffTimerEnable();
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 SysService(void)
  Author        : ZHengYongzhi
  Description   : system service progarm,this program will call system backgroud message,system thread
                  and devices check etc to finish work.,

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
extern uint32 keybackup;
_ATTR_SYS_CODE_
UINT32 SysService(void)
{
    uint32 retval;
#ifdef _USB_HOST_
    UHC_CHN_INFO *pUsbHC = NULL;
    pUsbHC = (UHC_CHN_INFO *)HostGetChannelInfo();
#endif

    BBDebug();

    #ifdef _WATCH_DOG_
    {
        WatchDogReload();
    }
    #endif

    #ifdef _USE_SHELL_
    {
        ShellTask();
    }
    #endif

    BacklightDetec();

    #ifdef _SDCARD_
    if (gSysConfig.SDEnable)
    {
        if (RETURN_FAIL == SDCardStatusDetect())
        {
            return(RETURN_FAIL);
        }
    }
    #endif

#ifdef _USB_
    #ifdef _USB_HOST_
    {
        if (gSysConfig.Listen_UsbHost == 1)
        {
            if (RETURN_FAIL == USBHostStatusDetect())
            {
                return(RETURN_FAIL);
            }
        }
    }
    if (pUsbHC->ConnectStatus != 1)
    #endif
    {
        if (RETURN_FAIL == USBStatusDetect())
        {
            return(RETURN_FAIL);
        }
    }

#else
    SendMsg(MSG_CHARGE_ENABLE);
#endif

#if(CODEC_CONFIG == CODEC_ROCKC)
#ifdef HP_DET_CONFIG
    Codec_HP_Detect();
#endif

#else
    #endif

    if (RETURN_FAIL == PowerOffDetec())
    {
        return(RETURN_FAIL);
    }

    SysBatteryStateCheck();

    SysResume();

    #ifdef _BEEP_
    {
        BeepSever();
    }
    #endif

    if (TRUE == CheckMsg(MSG_BL_OFF) && (TRUE != ThreadCheck(pMainThread, &RecordThread)))
    {
        __WFI();
    }

    return (RETURN_OK);
}

/*
********************************************************************************
*
*                         End of SysService.c
*
********************************************************************************
*/

