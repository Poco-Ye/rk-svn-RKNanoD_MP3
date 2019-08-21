/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：  HoldonPlay.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*               Chenwei          2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#ifndef _VIDEOHOLDON_PLAY_H_
#define _VIDEOHOLDON_PLAY_H_

#undef  EXT
#ifdef  _IN_VIDEOHOLDON_PLAY_
#define EXT
#else
#define EXT extern
#endif
#include "SysConfig.h"
#include "SysFindFile.h"


/*
********************************************************************************
*
*                        Struct Define 
*
********************************************************************************
*/
typedef struct _VIDEO_HOLDON_PLAY_INFO
{
    UINT32      CurrentFileNum;                                 // 当前文件编号
    UINT32      Video_Current_FrameNum;                         // 保存当前帧数
    UINT8       HoldVideoGetSign;                               // 是否保存断点标志 1:有断点 0:没有断点
    
}VIDEO_HOLDON_PLAY_INFO;


/*
********************************************************************************
*
*                         End of HoldonPlay.h
*
********************************************************************************
*/
#endif


