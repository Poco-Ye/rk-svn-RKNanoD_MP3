
/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º VideoControl.C
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*                 ZS            2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _VIDEO_CONTROL_H_
#define _VIDEO_CONTROL_H_

#include "Thread.h"
#include "VideoHoldonPlay.h"

#undef  EXT
#ifdef _IN_VIDEO_CONTROL_
#define EXT
#else
#define EXT extern
#endif



typedef unsigned long       DWORD;

//section define
#define _ATTR_VideoControl_TEXT_     __attribute__((section("VideoControlCode")))
#define _ATTR_VideoControl_DATA_     __attribute__((section("VideoControlData")))
#define _ATTR_VideoControl_BSS_      __attribute__((section("VideoControlBss"),zero_init))



//play status definition.
typedef enum
{
    VIDEO_STATE_PLAY,
    VIDEO_STATE_FFD,
    VIDEO_STATE_FFW,
    VIDEO_STATE_PAUSE,
    VIDEO_STATE_STOP
    
}VIDEOSTATE;
_ATTR_VideoControl_BSS_  EXT bool VedioDirection; 
_ATTR_VideoControl_BSS_  EXT UINT8              VideoPlayStateBack;
_ATTR_VideoControl_BSS_  EXT UINT16             VideoPlayState;
_ATTR_VideoControl_BSS_  EXT int                VideoStopMode;    //normal stop or Force//Audio_Stop_Normal or Audio_Stop_Force 
_ATTR_SYS_BSS_           EXT VIDEO_HOLDON_PLAY_INFO    VideoHoldOnPlayInfo;

#define VIDEONEXTFILE           1
#define VIDEOCURRENTFILE        0
#define VIDEOPREFILE            -1

#define Video_Stop_Normal       0
#define Video_Stop_Force        1
#define VideoOutputVol          gSysConfig.OutputVolume
#define VIDEOUITIME             8


void VideoThreadInit(void *pArg);
UINT32 VideoThreadService(void);
void VideoThreadDeInit(void);

/*
--------------------------------------------------------------------------------
  
  Description:  thread definition.
  
--------------------------------------------------------------------------------
*/
#ifdef _IN_VIDEO_CONTROL_
 _ATTR_VideoControl_DATA_ THREAD VideoThread = {

    NULL,
    NULL,
    
    VideoThreadInit,
    VideoThreadService,
    VideoThreadDeInit,
    
    NULL                           
};
#else
_ATTR_VideoControl_DATA_ EXT THREAD VideoThread;
#endif




#endif 

