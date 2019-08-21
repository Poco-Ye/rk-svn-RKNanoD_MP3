/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º  AudioControl.C
*
* Description:
*
* History:      <author>          <time>        <version>
*                 ZS      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#include "SysInclude.h"

#ifdef VIDEO_MP2_DECODE

#include "pMp2codec.h"
#include "AviFile.h"
#include "VideoControl.h"

uint32              Video_AudioOutptr;
long                Video_AudioOutLength;
_ATTR_VideoControl_DATA_ DMA_CFGX VideoAudioControlDmaCfg  = {DMA_CTLL_I2S0_TX, DMA_CFGL_I2S0_TX, DMA_CFGH_I2S0_TX,0};

void Video_AudioDecoding(void);
extern void mp2_synth_handler();

/*
--------------------------------------------------------------------------------
  Function name : void Video_Audio_I2SInit(void)
  Author        : zs
  Description   :audio i2s initial
  Input         : ÎÞ
  Return        : ÎÞ
  History       :  <author>         <time>         <version>
                      zs            2009/02/20         Ver1.0
  desc          :         ORG
--------------------------------------------------------------------------------
*/
_ATTR_VideoControl_TEXT_
void Video_Audio_I2SInit(void)
{
    //I2S_PowerOnInit(I2S_SlaveMode);
    //Codec_SetMode(Codec_DACoutHP,FS_8000Hz);
}

/*
--------------------------------------------------------------------------------
  Function name : void Video_AudioHWInit(void)
  Author        : zs
  Description   :
  Input         : null
  Return        : null
  History       : <author>         <time>         <version>
                    zs            2009/02/20         Ver1.0
  desc          :         ORG
--------------------------------------------------------------------------------
*/
_ATTR_VideoControl_TEXT_
void Video_AudioHWInit(void)
{
#ifdef MP2_INCLUDE
        mp2_AcceleratorHWInit();
#endif

}

/*
--------------------------------------------------------------------------------
  Function name :void Video_AudioDmaIsrHandler(void)
  Author        : zs
  Description   : the interruption callback of audio DMA0
  Input         : null
  Return        : null
  History       : <author>         <time>         <version>
                    zs            2009/02/20         Ver1.0
  desc          :         ORG
--------------------------------------------------------------------------------
*/
_ATTR_VideoControl_TEXT_
void Video_AudioDmaIsrHandler(void)
{
    if (VIDEO_STATE_PLAY == VideoPlayState)
    {
        DmaReStart(DMA_CHN2, (UINT32)Video_AudioOutptr, (uint32)(&(I2s_Reg->I2S_TXDR)),  Video_AudioOutLength, &VideoAudioControlDmaCfg, Video_AudioDmaIsrHandler);
        UserIsrRequest();
    }
}

/*
--------------------------------------------------------------------------------
  Function name :void Video_AudioStart(void)
  Author        : zs
  Description   : Call related initialization,start audio module.
  Input         : NULL
  Return        : 0:success,-1:Fail
  History       :<author>         <time>         <version>
                    zs            2009/02/20         Ver1.0
  desc          :  ORG
--------------------------------------------------------------------------------
*/
_ATTR_VideoControl_TEXT_
int  Video_AudioStart(void)
{
    ModuleOverlay(MODULE_ID_MP2_DECODE, MODULE_OVERLAY_ALL);

    IntPendingClear(FAULT_ID14_PENDSV);
    IntPrioritySet(FAULT_ID14_PENDSV,0xE0);
    IntRegister(FAULT_ID14_PENDSV, Video_AudioDecoding);
    IntEnable(FAULT_ID14_PENDSV);

#ifdef MP2_INCLUDE
    Video_AudioHWInit();
    IntPendingClear(INT_ID_SYNTH);
    IntRegister(INT_ID_SYNTH, mp2_synth_handler);
    IntEnable(INT_ID_SYNTH);
#endif

    Video_Audio_I2SInit();
    Avi_Audio_FileFuncInit();
    if (0 == Mp2CodecOpen(MP2_CODEC_OPEN_DEC,0))
    {
        return -1;
    }

    I2SInit(I2S_CH,I2S_PORT,I2S_MODE, I2S_FS_8000Hz,I2S_FORMAT,I2S_DATA_WIDTH16,I2S_NORMAL_MODE);
    Codec_SetMode(Codec_DACoutHP,ACodec_I2S_DATA_WIDTH16);
    Codec_SetSampleRate(AviStreamInfo.AudioSamplingRate);


    Video_AudioOutLength = 1152;
    Mp2CodecGetCaptureBuffer((short*)&Video_AudioOutptr,&Video_AudioOutLength);
    DmaStart(DMA_CHN2, (UINT32)Video_AudioOutptr, (uint32)(&(I2s_Reg->I2S_TXDR)),  Video_AudioOutLength, &VideoAudioControlDmaCfg, Video_AudioDmaIsrHandler);
    I2SStart(I2S_CH,I2S_START_DMA_TX);

    return 0;
}
/*
--------------------------------------------------------------------------------
  Function name : BOOLEAN Video_AudioDecoding(void)
  Author        : zs
  Description   : audio decode,decode next frame.
  Input         : null
  Return        : null
  History       : <author>         <time>         <version>
                   zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_VideoControl_TEXT_
void Video_AudioDecoding(void)
{

    if (VIDEO_STATE_PLAY != VideoPlayState)
        return ;

    if (0 == Mp2CodecDecode())
    {
        Mp2CodecGetCaptureBuffer((short*)&Video_AudioOutptr,&Video_AudioOutLength);
        memset((char *)Video_AudioOutptr, 0, Video_AudioOutLength*4);
        //must clear this memory to zero.as video maybe is playing.so DMA is sending voice,if do not clear,it will cause noise.
        return ;
    }

    Mp2CodecGetCaptureBuffer((short*)&Video_AudioOutptr,&Video_AudioOutLength);

}

/*
--------------------------------------------------------------------------------
  Function name :  BOOLEAN Video_AudioStop(UINT16 ReqType);
  Author        :  zs
  Description   :  audio decode end,
  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_VideoControl_TEXT_
BOOLEAN Video_AudioStop(UINT16 ReqType)
{
    uint32 timeout = 100;
/*************************************************************/
//interruption auti-initialization.

    /*---------clear interrupt enable bit------------*/
    IntDisable(INT_ID_IMDCT);
    IntDisable(INT_ID_SYNTH);

    /*---------clear interruption pending.---------*/
    IntPendingClear(INT_ID_IMDCT);
    IntPendingClear(INT_ID_SYNTH);
    IntPendingClear(FAULT_ID14_PENDSV);

    /*-----------unload registerd callback functions--------------*/
    IntUnregister(INT_ID_IMDCT);
    IntUnregister(INT_ID_SYNTH);
    IntUnregister(FAULT_ID14_PENDSV);

    /*--------clear the interruption flag of dma----------*/
    //Codec_DACMute();
    Codec_SetVolumet(0);
    while(DmaGetState(DMA_CHN2) == DMA_BUSY)
    {
        DelayMs(1);
        if (--timeout == 0)
        {
            break;
        }
    }
    Codec_ExitMode(Codec_DACoutHP);

    I2SStop(I2S_CH,I2S_START_DMA_TX);
    I2SDeInit(I2S_CH, I2S_PORT);

    Mp2CodecClose();

    return TRUE;
}

/*
--------------------------------------------------------------------------------
  Function name : BOOLEAN AudioResume(void)
  Author        :  zs
  Description   : audio decode resume.
                  change play status.
                  send decode message.
                  start dma.
  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_VideoControl_TEXT_
BOOLEAN Video_AudioResume(void)
{
    uint32 timeout = 100;

    Mp2CodecSeek(0, 0);

    Mp2CodecGetCaptureBuffer((short*)&Video_AudioOutptr,&Video_AudioOutLength);

    while(DmaGetState(DMA_CHN2) == DMA_BUSY)
    {
        DelayMs(1);
        if (--timeout == 0)
        {
            break;
        }
    }
    DmaStart(DMA_CHN2, (UINT32)Video_AudioOutptr, (uint32)(&(I2s_Reg->I2S_TXDR)),  Video_AudioOutLength, &VideoAudioControlDmaCfg, Video_AudioDmaIsrHandler);

    return TRUE;
}


#endif
//#pragma arm section code

/*
********************************************************************************
*
*                         End of Video_AudioControl.c
*
********************************************************************************
*/
