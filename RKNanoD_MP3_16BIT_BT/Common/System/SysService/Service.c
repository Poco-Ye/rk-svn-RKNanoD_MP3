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
#include "PowerOn_Off.h"
#include "Fade.h"
#include "MediaBroWin.h"
#include "rockcodec.h"

extern CodecMode_en_t Codecmode_Bak;
extern CodecFS_en_t   CodecFS_Bak;
extern eACodecI2sDATA_WIDTH_t CodecDataWidth_Bak;

extern void AudioSbcEncodeInit(void);

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

#ifdef _BEEP_

_ATTR_SYS_DATA_   __align(4) uint8  BeepBuffer[2048];

_ATTR_SYS_DATA_   uint32 BeepBaseAddr = 0;
_ATTR_SYS_DATA_   uint32 BeepLen = 0;
_ATTR_SYS_DATA_   uint32 BeepOffset = 0;
_ATTR_SYS_DATA_   uint32 CurBuferBank = 0;
_ATTR_SYS_DATA_   int    BeepLoopCnt = 0;


_ATTR_SYS_CODE_
void BeepInit(void)
{
    BeepPlayState = Voice_STOP;
    BeepPlayerState = Voice_STOP;

    AudioPlayState = AUDIO_STATE_STOP;
    AudioPlayerState = AUDIO_STATE_STOP;

    Codec_SetMode(Codec_DACoutHP, ACodec_I2S_DATA_WIDTH16);
    Codec_SetSampleRate(FS_44100Hz);

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
        //wait for beep stop
        if (BeepPlayState != Voice_STOP)
        {
            UserIsrDisable();
            BeepLoopCnt = 0;
            UserIsrEnable();

            while (BeepPlayState != Voice_STOP)
            {
                DelayMs(1);
            }

            //wait for last frame beep data transfer out
            timeout = 200;
            while (DmaGetState(AUDIO_DMACHANNEL_IIS) == DMA_BUSY)
            {
                DelayMs(1);
                if (--timeout == 0)
                {
                    break;
                }
            }
        }


        UserIsrDisable();
        if (ThreadCheck(pMainThread, &MusicThread) == TRUE)
        {
            //resume Audio Player
            if (AudioPlayState != AUDIO_STATE_PLAY)
            {
                DmaTransting   = 0;
                AudioDecodeing = 0;
            }
            else
            {
                AudioErrorFrameNum = 2;
            }
        }
        else if (ThreadCheck(pMainThread, &FMThread) == TRUE)
        {
            Codec_SetVolumet(0);
            I2SStop(I2S_CH, I2S_START_DMA_TX);
            I2SDeInit(I2S_CH, I2S_PORT);
            Codec_ExitMode(Codec_DACoutHP);

            Codec_SetMode(BeepCodecmodeBK, I2S_DATA_WIDTH16);
            I2SInit(I2S_CH, I2S_PORT,I2S_MODE, BeepSampleRateBK, I2S_FORMAT,I2S_DATA_WIDTH16, I2S_NORMAL_MODE);
            Codec_SetSampleRate(BeepSampleRateBK);
            FMVol_Resume();
        }
        else
        {
            I2SStop(I2S_CH, I2S_START_DMA_TX);
            I2SDeInit(I2S_CH, I2S_PORT);
            Codec_ExitMode(Codec_DACoutHP);

            DmaTransting = 0;
            AudioDecodeing = 0;
        }

        BeepPlayerState = Voice_STOP;

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
void BeepReadBuf(void )
{
    uint16 readlen;
    short * pBuffer3;
    int   k = 0;

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

}

_ATTR_SYS_CODE_
void BeepPlay(uint8 BeepId, int LoopCnt, uint8 BeepVolume, uint8 MusicFade, uint8 MusicFadeDeep)
{
    uint16 readlen;
    uint32 timeout;
    uint32 tBeepSampleRate;
    short *pBuffer1, * pBuffer2, * pBuffer3;
    int   k = 0;

    if (BeepPlayerState != Voice_PLAY)
    {
        UserIsrDisable();

        BeepPlayerState      = Voice_PLAY;

        BeepSampleRateBK     = CodecFS_Bak;
        BeepCodecmodeBK      = Codecmode_Bak;
        BeepCodecdatawidthBK = CodecDataWidth_Bak;

        tBeepSampleRate      = CodecFS_Bak;      //get current ACODEC sample rate
        BeepSourceInit(tBeepSampleRate, BeepId, LoopCnt);

        FREQ_EnterModule(FREQ_BEEP);

        if (ThreadCheck(pMainThread, &MusicThread) == TRUE)
        {
            if (AudioPlayState == AUDIO_STATE_PLAY)
            {
                BeepPlayState = Voice_PLAY;
                Codec_SetVolumet(BeepVolume);
                UserIsrEnable();
                return ;
            }
        }

        if (ThreadCheck(pMainThread, &FMThread) == TRUE)
        {
            I2SStop(I2S_CH, I2S_START_DMA_RTX);
            Codec_ExitMode(Codecmode_Bak);

            BeepReadBuf();

            Codec_SetMode(Codec_DACoutHP, ACodec_I2S_DATA_WIDTH16);

            I2SInit(I2S_CH, I2S_PORT,I2S_MODE, tBeepSampleRate, I2S_FORMAT,I2S_DATA_WIDTH16, I2S_NORMAL_MODE);
            Codec_SetSampleRate(tBeepSampleRate);
            Codec_SetVolumet(BeepVolume);
            DmaStart(AUDIO_DMACHANNEL_IIS, (UINT32)AudioPtr, (uint32)(&(I2s_Reg->I2S_TXDR)),
                                            AudioLen, &AudioControlDmaCfg, AudioDmaIsrHandler);
            I2SStart(I2S_CH, I2S_START_DMA_TX);

            DmaTransting = 1;
            AudioDecodeing = 1;
            BeepPlayState = Voice_PLAY;

            UserIsrRequest();
            UserIsrEnable();

            //BeepStop();
            return;
        }

        //else common process
        {
            BeepReadBuf();

            Codec_SetMode(Codec_DACoutHP, ACodec_I2S_DATA_WIDTH16);
            I2SInit(I2S_CH, I2S_PORT,I2S_MODE, tBeepSampleRate, I2S_FORMAT,I2S_DATA_WIDTH16, I2S_NORMAL_MODE);
            Codec_SetSampleRate(tBeepSampleRate);
            Codec_SetVolumet(BeepVolume);
            DmaStart(AUDIO_DMACHANNEL_IIS, (UINT32)AudioPtr, (uint32)(&(I2s_Reg->I2S_TXDR)),
                                            AudioLen, &AudioControlDmaCfg, AudioDmaIsrHandler);
            I2SStart(I2S_CH, I2S_START_DMA_TX);

            DmaTransting = 1;
            AudioDecodeing = 1;
            BeepPlayState = Voice_PLAY;

            UserIsrRequest();
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

#else
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

_ATTR_SYS_CODE_
void AudioDmaIsrHandler(void)
{
	if (AudioDecodeing == 1)
    {
        //printf("\nAudioDmaIsrHandler AudioDecodeing = 1");
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
_ATTR_SYS_CODE_
void Bit_Convertor(short *ppsBuffer, long *plLength,int bps)//直接输出，低位补0
{
    int i = 0;
    int offset = 0 ;
    if (bps == 16)
    {
    }
    else if (bps == 24)
    {
        char *pOut = (char *)ppsBuffer;
        offset = *plLength*2;
        for (i=0; i < offset ; i++)
        {
            pOut[2*i]= pOut[3*i+1];
            pOut[2*i+1]= pOut[3*i+2];
        }
    }
    else if (bps == 32)
    {
        char *pOut = (char *)ppsBuffer;
        offset = *plLength*2;
        for (i=0; i < offset ; i++)
        {
            pOut[2*i]= pOut[4*i+2];
            pOut[2*i+1]= pOut[4*i+3];
        }
    }
}
_ATTR_SYS_CODE_
void Bit_Convertor_DEC(short *ppsBuffer, long *plLength,int bps)//紧接解码器输出，高位补0，用于EQ等运算
{
    int i = 0;
    int offset = 0 ;
    if (bps == 16)
    {
    }
    else if (bps == 24)
    {
        char *pOut = (char *)ppsBuffer;
        offset = *plLength*2;
        for (i=0; i < offset ; i++)
        {
            pOut[2*i]= pOut[3*i+1];
            pOut[2*i+1]= pOut[3*i+2];
        }
    }
    else if (bps == 32)
    {
        char *pOut = (char *)ppsBuffer;
        offset = *plLength*2;
        for (i=0; i < offset ; i++)
        {
            pOut[2*i]= pOut[4*i+2];
            pOut[2*i+1]= pOut[4*i+3];
        }
    }
}
_ATTR_SYS_CODE_
void Bit_Convertor_shift(short *ppsBuffer, long *plLength,int bps)//经过EQ等处理后的数据传送
{
    int i = 0;
    int offset = 0 ;
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
    if (CheckCard() == 0)
    {
        //printf("\nNO Card audio decoding!!!");
        return;
    }
#endif

	if(BeepPlayState == Voice_PLAY)
    {
           goto BEEPONLY;
	}

    if (AUDIO_STATE_PLAY == AudioPlayState)
    {
        if (AudioCodecOpenErr)
        {
            AudioPlayState = AUDIO_STATE_STOP;
            SendMsg(MSG_AUDIO_FILE_ERROR);
            goto BEEP;
        }

        DecodeErr = CodecGetCaptureBuffer((short*)&AudioPtr, &AudioLen);

        if (0 == DecodeErr)   // Decoding end or error
        {
            if (FileInfo[(uint32)pRawFileCache].Offset <= FileInfo[(uint32)pRawFileCache].FileSize / 2
                && (gSysConfig.MusicConfig.RepeatMode != AUDIO_REPEAT)
                && (gSysConfig.MusicConfig.RepeatMode != AUDIO_REPEAT1))
            {
                //printf("\n------1 file error------ ");
                AudioPlayState = AUDIO_STATE_STOP;
                if(AUDIO_STATE_PLAY == AudioPlayerState)
                {
                    MusicNextFile = 1;
                }
                memset((uint8*)AudioPtr, 0x00, AudioLen * 4);
                if((AUDIO_STATE_FFD == AudioPlayerState) || (AUDIO_STATE_FFW == AudioPlayerState))
                {
                    SendMsg(MSG_AUDIO_FFW_FFD_END);
                }
                else
                {
                    SendMsg(MSG_AUDIO_FILE_ERROR);
                }
            }
            else
            {
                //printf("\n------2 decode end------ ");
                AudioPlayState = AUDIO_STATE_STOP;
                if(AUDIO_STATE_PLAY == AudioPlayerState)
                {
                    MusicNextFile = 1;
                }
                memset((uint8*)AudioPtr, 0x00, AudioLen * 4);
                if((AUDIO_STATE_FFD == AudioPlayerState) || (AUDIO_STATE_FFW == AudioPlayerState))
                {
                    SendMsg(MSG_AUDIO_FFW_FFD_END);
                }
                else
                {
                    SendMsg(MSG_AUDIO_DECODE_END);
                }
            }
            AudioDecodeing = 0;
            return;
        }
        /*else*/ if (2 == DecodeErr)
        {
            //printf("\nDecodeStatus = 2 wait B decode");
            AudioNeedDecode = 1;
            return;
        }

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
                memset((uint8*)AudioPtr, 0x00, AudioLen * 4);
            }
        }

#ifdef _FADE_PROCESS_
        // last 500ms for Fadeout
        if ((pAudioRegKey->TotalTime - pAudioRegKey->CurrentTime) <= 300)
        {
            if (FadeIsFinished())
            {
                if (AudioEndFade == 0)
                {
                    //FadeInit(0,pAudioRegKey->samplerate/4,FADE_OUT);
                    FadeInit(pAudioRegKey->samplerate,pAudioRegKey->samplerate/4,FADE_OUT);
                    AudioEndFade = 1;
                }
            }
        }
#endif

        #ifdef _RK_EQ_
        EffectProcess((EQ_TYPE *)AudioPtr, AudioLen);
        #endif

        #ifdef _FADE_PROCESS_
        if (!FadeIsFinished())
        {
            FadeProcess((short*)AudioPtr,AudioLen);
        }
        else if (AudioEndFade == 1)
        {
            memset((uint8*)AudioPtr, 0x00, AudioLen * 4);
        }
        #endif

#ifdef _A2DP_SOUCRE_
    	AudioSbcEncodeInit();
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
                         memcpy(Spectrum_data, (short*)AudioPtr, 256);
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
                    SpectrumLoop-- ;
            }
        }
#endif
    }

#ifdef _BEEP_

BEEP:

    if (BeepPlayState == Voice_PLAY)
    {
        {
BEEPONLY:
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

            if (readlen == 0)
            {
                memset(((unsigned char*)AudioPtr), 0x00, AudioLen * 4);
                BeepPlayState = Voice_STOP;
                SendMsg(MSG_BEEP_END);
            }
        }
    }
#endif

    Bit_Convertor_shift((short*)AudioPtr, &AudioLen,  pAudioRegKey->bps);
    IntDisable(INT_ID_DMA);
    AudioDecodeing = 0;

    if (DmaTransting == 0)
    {
        //printf("\nDmaTransting == 0");
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
        //USBDEBUG("VBUS Change: ", VbusStatus);
        VbusChanged = 1;
    }

    if(VbusChanged)
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
                        SendMsg(MSG_VBUS_INSERT);

                        AutoPowerOffDisable();

                        BL_Off();
                        LCD_ClrSrc();
                        IsBackLightOn = TRUE;
                        if(FALSE == CheckMsg(MSG_LCD_STANDBY_DISABLE))
                        {
                            LcdStandby();
                            AdcSleepEnter();
                        }
                        ThreadDeleteAll(&pMainThread);
                        #ifdef _BEEP_
                        BeepStop();
                        #endif

                        if (Task.TaskID != TASK_ID_CHARGE)
                        {
                            TaskSwitch(TASK_ID_CHARGE, NULL);
                            RetVal = RETURN_FAIL;
                        }

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

                    //SendMsg(MSG_SYS_RESUME);

                    if (GetMsg(MSG_MES_FIRMWAREUPGRADE))
                    {
                        TaskArg.Usb.FunSel = USB_CLASS_TYPE_UPGRADE;
                    }
                    else
                    {
                        #if 0
                        TaskArg.Usb.FunSel = USB_CLASS_TYPE_SERIAL;
                        #else
                        TaskArg.Usb.FunSel = USB_CLASS_TYPE_MSC;
                        #endif
                    }
                    TaskSwitch(TASK_ID_USB, &TaskArg);
                    RetVal = RETURN_FAIL;
                }
                else if(2 == UsbAdapterRet)
                {
                    //AC
                    SendMsg(MSG_CHARGE_ENABLE);

                    BatteryChargeInit();

                    SendMsg(MSG_CHARGE_START);      //使能充电

                    SendMsg(MSG_SYS_RESUME);

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
_ATTR_SYS_CODE_
uint32 SDCardStatusDetect(void)
{
    uint32 RetVal = 0;
    TASK_ARG TaskArg;

    if (CheckCard() == 0)
    {
        if (FALSE == CheckMsg(MSG_SDCARD_EJECT))
        {
            SendMsg(MSG_SDCARD_EJECT);
            SendMsg(MSG_SDCARD_UPDATE);
            SendMsg(MSG_SDCARD_MEM_UPDATE);
            SendMsg(MSG_SYS_RESUME);
            FREQ_EnterModule(FREQ_BLON);

#if (2 != FW_IN_DEV)
            if (gSysConfig.Memory == CARD)
#endif
            {
#ifdef _RADIO_
                if (TRUE != ThreadCheck(pMainThread, &FMThread))
#endif
                {
#ifdef _MUSIC_
                    ThreadDelete(&pMainThread, &MusicThread);
#endif
#ifdef _RECORD_
                    ThreadDelete(&pMainThread, &RecordThread);
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
                gSysConfig.Memory = FLASH0;
                FileSysSetup(gSysConfig.Memory);//dgl second filesystem
#endif
            }

            SDCardDisable();
            SDCardDeInit();//add by dgl
        }
    }
    else
    {
        if (CheckMsg(MSG_SDCARD_EJECT))
        {
            ClearMsg(MSG_SDCARD_EJECT);
            SendMsg(MSG_SDCARD_UPDATE);
            SendMsg(MSG_SDCARD_MEM_UPDATE);
            SendMsg(MSG_SYS_RESUME);
            FREQ_EnterModule(FREQ_BLON);

            SDCardEnable();
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
    uint32 PowerOffBuff[6] = {0, 10*60*100, 15*60*100, 30*60*100, 60*60*100, 120*60*100};

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
--------------------------------------------------------------------------------
*/
//PAGE
UINT32 VolumeLimit;
UINT8 FactoryVolumeLimit;
void SoundPresureTimerISR(void);
void SetSoundPresureTimerDisable(void);

//_ATTR_SYS_DATA_
SYSTICK_LIST SoundPresureTimer =
{
    NULL,
    0,
    0,
    1,
    SoundPresureTimerISR,
};

//_ATTR_SYS_CODE_
void SoundPresureTimerISR(void)
{
    if (VolumeLimit > 0)
    {
        VolumeLimit--;
        //printf("Volume Limit:%d\n", VolumeLimit);

        if (VolumeLimit == 0)
        {
            SendMsg(MSG_SOUND_PRESSURE_COUNTER_EXPIRED);
        }
    }
}



//_ATTR_SYS_CODE_
void SetSoundPresureTimerEnable(UINT32 SPTimer)
{
    SoundPresureTimer.Counter = 0;
    SoundPresureTimer.Reload = 6000;
    SoundPresureTimer.Times = SPTimer/6000;
    SystickTimerStart(&SoundPresureTimer);
}

//_ATTR_SYS_CODE_
void SetSoundPresureTimerDisable(void)
{
    SystickTimerStop(&SoundPresureTimer);
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
        if (TRUE == ThreadCheck(pMainThread, &VideoThread))
        {
            gSysConfig.VideoConfig.HoldOnPlaySaveFlag = 1;
        }
        else
        {
            gSysConfig.VideoConfig.HoldOnPlaySaveFlag = VideoHoldOnPlayInfo.HoldVideoGetSign ;
        }
#endif

#ifdef _RADIO_
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
            //        ModuleOverlay(MODULE_ID_SYSRESERVED_OP, MODULE_OVERLAY_ALL);
            SaveSysInformation(1);

#if (NAND_DRIVER == 1)
            FtlClose();
#endif

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
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
uint32 DemoModePauseDetect(void)
{
    UINT16 Count;
    TASK_ARG TaskArg;

    if (DemoModePauseSystickCounterBack != 0)
    {
        Count = SysTickCounter - DemoModePauseSystickCounterBack;

        if (Count > 30 *100)
        {
            gMusicTypeSelID = 1; //SORT_TYPE_SEL_ID3TITLE
            TaskArg.MediaBro.TitleAdd = SID_ALL_MUISC_FILE;
            TaskArg.MediaBro.CurId= 0;
            TaskSwitch(TASK_ID_MEDIABRO, &TaskArg);
            SendMsg(MSG_MEDIABRO_DEMO_ALL_SONGS);

            DemoModePauseSystickCounterBack = 0;
            return(RETURN_FAIL);
        }
    }
    return(RETURN_OK);
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
    WatchDogDisableInt();
    WatchDogDeInit();
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
    USBVbusStatus = 0xff; //-1
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

    BBDebug();

#ifdef _WATCH_DOG_
    WatchDogReload();
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
    if (RETURN_FAIL == USBStatusDetect())
    {
        return(RETURN_FAIL);
    }
#else
    SendMsg(MSG_CHARGE_ENABLE);
#endif

    if (RETURN_FAIL == PowerOffDetec())
    {
        return(RETURN_FAIL);
    }

    SysBatteryStateCheck();

    SysResume();

#ifdef _BEEP_
    BeepSever();
#endif

    if (TRUE == CheckMsg(MSG_BL_OFF))
    {
        __WFI();
    }

    return(RETURN_OK);
}

/*
********************************************************************************
*
*                         End of SysService.c
*
********************************************************************************
*/

