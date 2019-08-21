/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name： VideoControl.C
*
* Description:
*
* History:      <author>          <time>        <version>
*                 ZS      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#define _IN_VIDEO_CONTROL_

#include "SysInclude.h"

#ifdef _VIDEO_

#include "SysFindFile.h"
#include "AviFile.h"

#include "xvid_dec_main.h"

#include "VideoControl.h"

#ifdef _MEDIA_MODULE_
#include "MediaBroWin.h"
#endif

#define AVI_FFD_FFW_STEP    (5*15)  //the step of avi FFD or FFW,It is equal with frarme rate multiply time.

_ATTR_VideoControl_DATA_ UINT8  VideoTimerFlag = 1;
_ATTR_VideoControl_DATA_ UINT8  VideoDisplayTime = VIDEOUITIME;
_ATTR_VideoControl_DATA_ UINT8  VideDisplayFlag = 0;
_ATTR_VideoControl_BSS_  UINT32 VideoPlayFileNum;
_ATTR_VideoControl_BSS_  bool   VideoUiFlag;
_ATTR_VideoControl_BSS_  UINT32 VideoPlayErrorNum;

/*
--------------------------------------------------------------------------------
  Function name : void VideoTimerIsr(void)
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
void VideoTimerIsr(void)
{
    TimerClearIntFlag(TIMER0);  //clear interrupt flag.
    VideoTimerFlag = 0;

    //****************use for ui display.****************************//
    VideDisplayFlag ++;

    if (10 == VideDisplayFlag)
    {
        VideDisplayFlag = 0;
        if (VideoDisplayTime)
        {
            VideoDisplayTime --;
            if (0 == VideoDisplayTime)
            {
                VideoUiFlag = 0;
            }
        }
    }
}
/*
--------------------------------------------------------------------------------
  Function name : void VideoGetNextMovie(void)
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
BOOLEAN VideoGetNextMovie(UINT32 msg)
{

    //ModuleOverlay(MODULE_ID_FILE_FIND, MODULE_OVERLAY_ALL);

    if ((VideoPlayFileNum >= VideoFileInfo.TotalFiles)&&(VideoStopMode != Video_Stop_Force))
    {
        VideoPlayFileNum = 0;
//          VideoPlayState = VIDEO_STATE_STOP;// by zs 06.09
//          SendMsg(MSG_VIDEO_EXIT_BROWSER);
//          return TRUE;
    }
//    else
    {
        SysFindFile(&VideoFileInfo,msg);
    }

    if (VideoPlayFileNum >= VideoFileInfo.TotalFiles)
    {
        VideoPlayFileNum = 0;
    }

    if (0 != VideoControlProc(MSG_XVID_START,NULL))
    {
        VideoPlayState = VIDEO_STATE_STOP;// by zs 06.09
        SendMsg(MSG_VIDEO_FILENOTSUPPORT);
        return FALSE;
    }

    return TRUE;

}
/*
--------------------------------------------------------------------------------
  Function name : void VideoStart(void)
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
int VideoStart(void)
{
    //VideoPlayState = VIDEO_STATE_PLAY;

    if (VideoPlayStateBack == VIDEO_STATE_PAUSE)
    {
        VideoPlayState = VIDEO_STATE_PAUSE;
    }
    else
    {
        VideoPlayState = VIDEO_STATE_PLAY;
    }

    VideoDisplayTime = VIDEOUITIME;
    VideoPlayFileNum ++;

#ifdef VIDEO_HOLDON_PLAY
    if (((VideoHoldOnPlayInfo.HoldVideoGetSign == 1)||(gSysConfig.VideoConfig.HoldOnPlaySaveFlag == 1))&&(VideoHoldOnPlayInfo.CurrentFileNum == gbVideoFileNum))
    {
        VideoHoldOnStart();
    }
#endif

    ModuleOverlay(MODULE_ID_AVI_DECODE, MODULE_OVERLAY_ALL);
#ifdef VIDEO_AVI
    if (VideoDecodeInit() != 0)
    {
        return(-1);
    }
#endif

#ifdef VIDEO_HOLDON_PLAY
    if (((VideoHoldOnPlayInfo.HoldVideoGetSign == 1)||(gSysConfig.VideoConfig.HoldOnPlaySaveFlag == 1))&&(VideoHoldOnPlayInfo.CurrentFileNum == gbVideoFileNum))
    {
        AviVideoSeek( VideoHoldOnPlayInfo.Video_Current_FrameNum,1);
        Video_chunk_info.Video_Current_FrameNum =  VideoHoldOnPlayInfo.Video_Current_FrameNum;
        SyncAudio2Video();
        VideoHoldOnPlayInfo.HoldVideoGetSign = 0;
        gSysConfig.VideoConfig.HoldOnPlaySaveFlag = 0;
    }
#endif

#ifdef VIDEO_MP2_DECODE
    if (Video_AudioStart()!= 0)
    {
        return(-1);
    }
#endif

#ifdef VIDEO_AVI
    VideoControlProc(MSG_XVID_OPEN,NULL);

    IntRegister(INT_ID_TIMER0, VideoTimerIsr);
    IntPendingClear(INT_ID_TIMER0);
    IntEnable(INT_ID_TIMER0);
    //TimerPeriodSet(TIMER0, (Play_Frame_Rate/1000), 0);
    TimerPeriodSet(TIMER0, Play_Frame_Rate, 0);
#endif

    //Codec_DACUnMute();
    Codec_SetVolumet(gSysConfig.OutputVolume);

    VideoPlayErrorNum = 0;
    VedioDirection = 0;   //ylz++

    SendMsg(MSG_XVID_DISPLAY_ALL);

    return 0;
}
/*
--------------------------------------------------------------------------------
  Function name : void VideoPlay(void)
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
void VideoPlay(void)
{
    if (VIDEO_STATE_PLAY != VideoPlayState)
        return ;

#ifdef VIDEO_AVI
    if (VideoTimerFlag == 0)
    {
        VideoTimerFlag = 1;

        if (xvid_dec_frame()!= 0)//if decoding is failure or finished,then find next file.
        {
            VideoControlProc(MSG_XVID_STOP,Video_Stop_Normal);
            return ;
        }

        if (SyncVideo2Audio() != 2)
        {
            xvid_dec_reset();
        }

        if (VideoPlayStateBack == VIDEO_STATE_PAUSE)
        {
            VideoPlayState = VIDEO_STATE_PAUSE;
            SendMsg(MSG_XVID_DISPFLAG_STATUS);
        }
        else
        {
            VideoPlayState = VIDEO_STATE_PLAY;
        }
    }
#endif
}
/*
--------------------------------------------------------------------------------
  Function name : void VideoPause(void)
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
BOOLEAN VideoPause(void)
{

    if (VIDEO_STATE_PLAY == VideoPlayState)
    {
        VideoPlayState = VIDEO_STATE_PAUSE;
        VideoPlayStateBack = VIDEO_STATE_PAUSE;
        //Codec_DACMute();
        Codec_SetVolumet(0);
    }
    else
    {
        VideoPlayStateBack = VIDEO_STATE_PLAY;
    }

    return TRUE;
}
/*
--------------------------------------------------------------------------------
  Function name : void VideoResume(void)
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
BOOLEAN VideoResume(void)
{
    if ((VIDEO_STATE_PAUSE != VideoPlayState) && (VIDEO_STATE_FFD != VideoPlayState) && (VIDEO_STATE_FFW != VideoPlayState))
        return FALSE;

    //Codec_DACUnMute();
    Codec_SetVolumet(VideoOutputVol);

    SyncAudio2Video();

#ifdef VIDEO_MP2_DECODE
    Video_AudioResume();
#endif

#ifdef VIDEO_AVI
    xvid_dec_reset();
#endif

    if (VideoPlayStateBack == VIDEO_STATE_PAUSE)
    {
        VideoPlayState = VIDEO_STATE_PAUSE;
    }
    else
    {
        VideoPlayState = VIDEO_STATE_PLAY;
    }

    return TRUE;
}
/*
--------------------------------------------------------------------------------
  Function name : void VideoStop(void)
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
BOOLEAN VideoStop(UINT16 ReqType)
{
    if (VIDEO_STATE_STOP == VideoPlayState)
    {
        return FALSE;
    }
    VideoPlayState = VIDEO_STATE_STOP;

    VideoControlProc(MSG_XVID_CLOSE,NULL);
#ifdef VIDEO_MP2_DECODE
    Video_AudioStop();
#endif

    IntDisable(INT_ID_TIMER0);
    IntPendingClear(INT_ID_TIMER0);
    IntUnregister(INT_ID_TIMER0);

#ifdef _SDCARD_ //by zs 05.18 solve the bug:crash when poll out sd card.
    if ((CheckCard() == 0)&&(gSysConfig.Memory == CARD))
    {
        SendMsg(MSG_VIDEO_CARD_CHECK);
        gSysConfig.Memory = FLASH0;
        FileSysSetup(gSysConfig.Memory);
        SDCardDisable();
        return FALSE;
    }
#endif

    /*************************************************************/
    //Flash Refresh zyz
#ifdef _RECORD_
    FREQ_EnterModule(FREQ_MAX);
    ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL);
    FREQ_ExitModule(FREQ_MAX);
#endif
    /*************************************************************/

    if (Video_Stop_Normal == ReqType)
    {
        VideoControlProc(MSG_XVID_NEXTFILE,VIDEONEXTFILE);
        return FALSE;
    }

    return TRUE;
}
/*
--------------------------------------------------------------------------------
  Function name :  BOOLEAN VideoFFW(unsigned long StepLen)
  Author        :  zs
  Description   :
  Input         :
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_VideoControl_TEXT_
BOOLEAN VideoFFW(unsigned long StepLen)
{
    if (VIDEO_STATE_STOP == VideoPlayState)
        return TRUE;

    if (VIDEO_STATE_FFW != VideoPlayState)
    {
        //Codec_DACMute();
        Codec_SetVolumet(0);
        VideoPlayState = VIDEO_STATE_FFW;
    }

    if (AviVideoSeek(AVI_FFD_FFW_STEP, -1) != 0) //go forward to seek 10 frame each time.
    {
        return FALSE;
    }
#ifdef VIDEO_AVI
    //when in FFW procession,then decode the current frame and display to screen.
    xvid_dec_reset();
    xvid_dec_frame();
#endif
    return TRUE;
}

/*
--------------------------------------------------------------------------------
  Function name :  BOOLEAN VideoFFD(unsigned long StepLen)
  Author        :  zs
  Description   :
  Input         :
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_VideoControl_TEXT_
BOOLEAN VideoFFD(unsigned long StepLen)
{
    if (VIDEO_STATE_STOP == VideoPlayState)
        return TRUE;

    if (VIDEO_STATE_FFD != VideoPlayState)
    {
        //Codec_DACMute();
        Codec_SetVolumet(0);
        VideoPlayState = VIDEO_STATE_FFD;
    }

    if (AviVideoSeek(AVI_FFD_FFW_STEP, 1) != 0) //go backward to seek 10 frame each time.
    {
        return FALSE; //if seek to the last frame,do not decode,it maybe display wrong.
    }

#ifdef VIDEO_AVI
    //when in FFD procession,then decode the current frame and display to screen.
    xvid_dec_reset();
    xvid_dec_frame();
#endif
    return TRUE;
}


/*
--------------------------------------------------------------------------------
  Function name :  void VideoControlProc(void *pArg)
  Author        :  zs
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_VideoControl_TEXT_
int  VideoControlProc(MSG_ID id,void * msg)
{
    BOOLEAN ret = 0;

    switch (id)
    {
        case MSG_XVID_INIT:
            if (VideoDecodeInit() != 0)
            {
                return(-1);
            }
            break;

#ifdef VIDEO_AVI
        case MSG_XVID_OPEN:
            ret = xvid_dec_open();    /* 0: success; <0: fail*/
            if (ret)
            {
                VideoControlProc(MSG_XVID_CLOSE,NULL);
            }
            break;
#endif

        case MSG_XVID_START:
            ret =  VideoStart();//solve the bug,format do not support,but it can not exit to explorer!!
            break;

        case MSG_XVID_DECODE:
            VideoPlay();
            break;

#ifdef VIDEO_AVI
        case MSG_XVID_CLOSE:
            {
                xvid_dec_close();
                VideoDecodeUninit();
                break;
            }
#endif

        case MSG_XVID_STOP:
            if (VideoStop((int)msg))
            {
#ifdef VIDEO_HOLDON_PLAY
                if (gSysConfig.Memory == FLASH0)
                {
                    VideoHoldOnInforSave(&VideoFileInfo);
                }
#endif
            }
            break;

        case MSG_XVID_PAUSE:
            VideoPause();
            break;

        case MSG_XVID_RESUME:
            VideoResume();
            break;

        case MSG_XVID_NEXTFILE:
            if ((VIDEO_STATE_STOP != VideoPlayState)||(VideoPlayState == Video_Stop_Force))
            {
                VideoStop(Video_Stop_Force);
            }
            VideoGetNextMovie((UINT32)msg);
            break;

        case MSG_XVID_VOLUMESET:
            Codec_SetVolumet((unsigned int)msg);
            break;

        case MSG_XVID_FFD:
            VideoFFD(0);
            break;

        case MSG_XVID_FFW:
            VideoFFW(0);
            break;

        default:
            break;
    }
    return (ret);
}

/*
--------------------------------------------------------------------------------
  Function name :  void VideoThreadInit(void *pArg)
  Author        :  zs
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_VideoControl_TEXT_
void VideoThreadInit(void *pArg)
{
    INT16 FrameID;

    FREQ_EnterModule(FREQ_AVI);

    //ModuleOverlay(MODULE_ID_FILE_FIND, MODULE_OVERLAY_ALL);
    gbVideoFileNum = ((VIDEO_THREAD_ARG*)pArg)->FileNum;

    if (gbVideoFileNum == 0xFFFF)
    {
        gbVideoFileNum = 1;
    }

    //在有媒体库的时候这个值需要设置为资源管理器
    //否则视频找下一个文件时会出错
#ifdef _MEDIA_MODULE_
    VideoFileInfo.ucSelPlayType = SORT_TYPE_SEL_BROWSER;
#endif

    gwSaveDirClus = 0;  //tiantian
    SysFindFileInit(&VideoFileInfo, gbVideoFileNum, FIND_FILE_RANGE_ALL ,NULL ,(UINT8*)VideoFileExtString);

    if (0 == VideoFileInfo.TotalFiles)
    {
        SendMsg(MSG_XVID_NOFILE);
        return;
    }


    ScuClockGateCtr(HCLK_SYNTH_GATE,1);
    ScuClockGateCtr(HCLK_IMDCT_GATE,1);
    ScuSoftResetCtr(IMDCT_SRST, 0);
    ScuSoftResetCtr(SYNTH_SRST, 0);


    if (0 != VideoControlProc(MSG_XVID_START,NULL))
    {
        VideoPlayState = VIDEO_STATE_STOP;// by zs 06.09
        SendMsg(MSG_VIDEO_FILENOTSUPPORT);
        return;
    }
}

/*
--------------------------------------------------------------------------------
  Function name :  UINT32 VideoThreadService(void)
  Author        :  zs
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_VideoControl_TEXT_
UINT32 VideoThreadService(void)
{
    int ret = 0;

    VideoControlProc(MSG_XVID_DECODE,NULL);

    return ret;
}

/*
--------------------------------------------------------------------------------
  Function name :  void VideoThreadDeInit(void)
  Author        :  zs
  Description   :
  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_VideoControl_TEXT_
void VideoThreadDeInit(void)
{
    VideoControlProc(MSG_XVID_STOP,(void*)Video_Stop_Force);

    Codec_ExitMode(Codec_DACoutHP);

    ScuClockGateCtr(HCLK_SYNTH_GATE,0);
    ScuClockGateCtr(HCLK_IMDCT_GATE,0);
    ScuSoftResetCtr(IMDCT_SRST, 1);
    ScuSoftResetCtr(SYNTH_SRST, 1);
    //ScuClockGateCtr(CLOCK_GATE_IMDCT, 0);

    FREQ_ExitModule(FREQ_AVI);

    return;
}


#ifndef _FRAME_BUFFER_
/*
**whether use dma for video refresh.
*/

_ATTR_VideoControl_TEXT_
void LCD_DrawBmpVideoWithDMA(UINT16 x0, UINT16 y0, UINT16 xsize, UINT16 ysize, UINT16 Pixel, UINT16 *pData, UINT16 width)
{
    INT16  x1,y1;

    if (xsize < 0 || ysize < 0)
    {
        return;
    }

    switch (Pixel)
    {

        case 16:
            {
                x1   = xsize;
                y1   = ysize;

#if (LCD_DIRECTION == LCD_HORIZONTAL)
                if (x1>LCD_WIDTH-1) x1 = LCD_WIDTH-1;
                if (y1>LCD_HEIGHT-1) y1 = LCD_HEIGHT-1;
#endif

                x1 += 1;
                y1 += 1;
                //Lcdchang(pData,(x1-x0)*(y1-y0)*2); // we do swap in yuv2rgb

                Lcd_DMATranfer(x0,y0,x1,y1,pData);
            }
            break;

        default:
            break;
    }
}

#endif

_ATTR_VideoControl_TEXT_
void LCD_DrawBmpVideo(UINT16 x0, UINT16 y0, UINT16 xsize, UINT16 ysize, UINT16 Pixel, UINT16 *pData, UINT16 width)
{
    UINT16 i,j;
    INT16  x, y;
    INT16  x1,y1;
    INT16  BytesPerLine;
    UINT16 BitOffset =0;
    UINT16 Xpos,Ypos;
    UINT8 bPage,bColNum;
    UINT16 *p;

    UINT16 x_temp,y_temp;
    //x1 = x0+xsize-1;
    //y1 = y0+ysize-1;

    if (xsize < 0 || ysize < 0)
    {
        return;
    }

    switch (Pixel)
    {

        case 16:
            {
                Xpos = x0;
                Ypos = y0;
                x1   = xsize;
                y1   = ysize;

#if (LCD_DIRECTION == LCD_HORIZONTAL)
                if (x1>LCD_WIDTH-1) x1 = LCD_WIDTH-1;
                if (y1>LCD_HEIGHT-1) y1 = LCD_HEIGHT-1;
#endif

#ifdef _DMA_LCD_
                Lcdchang(pData,(x1-x0)*(y1-y0+1)*2);
                Lcd_DMATranfer(x0,y0-1,x1,y1,pData);
#else
                Lcd_SetWindow(x0, y0, x1, y1);

                for (j = y0; j < y1+1; j++)
                {
                    p = pData + (j-y0)*width;
                    for (i = x0; i < x1+1; i++)
                    {
                        Lcd_SendData (*p++);
                    }
                    Ypos++;
                    Lcd_SetWindow(Xpos, Ypos,x1,y1);
                }
#endif//_DMA_LCD_
            }
            break;

        default:
            break;
    }
}

#endif

