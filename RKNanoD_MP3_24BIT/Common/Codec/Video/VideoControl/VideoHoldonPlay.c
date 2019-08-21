/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º  HoldonPlay.C
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*               Chenwei          2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#define _IN_VIDEOHOLDON_PLAY_

#include "SysInclude.h"
#include "FsInclude.h"
#include "AviFile.h"
#include "VideoControl.h"

#include "VideoHoldonPlay.h"

#ifdef _VIDEO_

extern VIDEO_CHUNK_INFO        Video_chunk_info;

/*----------------------------------------------------------
*Name  :  VideoHoldOnInforSave
*Desc  :   when  out video playing, save the  video file information
*Params:  VideoFileInfoTemp:video holdon file information pointer, CurrenttimeTemp:the  timer of video playing is end
*return:
-----------------------------------------------------------*/
_ATTR_VideoControl_TEXT_
void VideoHoldOnInforSave(SYS_FILE_INFO *SysFileInfo)
{
    uint32 DirClus;
    //ModuleOverlay(MODULE_ID_FILE_FIND, MODULE_OVERLAY_ALL);
    
    VideoHoldOnPlayInfo.HoldVideoGetSign = 1;
   // gSysConfig.VideoConfig.HoldOnPlaySaveFlag = 1;
   
    SysFileInfo->CurrentFileNum = GetCurFileNum(SysFileInfo->CurrentFileNum, &SysFileInfo->FindData, SysFileInfo->pExtStr, FS_FAT);  //tiantian

	DirClus = GetDirClusIndex(SysFileInfo->Path);
    SysFileInfo->CurrentFileNum = GetGlobeFileNum(SysFileInfo->CurrentFileNum ,DirClus, SysFileInfo->pExtStr, FS_FAT);
    
    VideoHoldOnPlayInfo.CurrentFileNum = SysFileInfo->CurrentFileNum;
    
    VideoHoldOnPlayInfo.Video_Current_FrameNum = Video_chunk_info.Video_Current_FrameNum;
    
    gSysConfig.VideoConfig.FileNum = VideoHoldOnPlayInfo.CurrentFileNum;
    
    gSysConfig.VideoConfig.Video_Current_FrameNum = VideoHoldOnPlayInfo.Video_Current_FrameNum;

}
_ATTR_VideoControl_TEXT_
void VideoHoldOnStart(void)
{
    VideoHoldOnPlayInfo.CurrentFileNum = gSysConfig.VideoConfig.FileNum ; 
    VideoFileInfo.CurrentFileNum = VideoHoldOnPlayInfo.CurrentFileNum;
     
    VideoHoldOnPlayInfo.Video_Current_FrameNum = gSysConfig.VideoConfig.Video_Current_FrameNum;

    gwSaveDirClus = 0;  //tiantian
    SysFindFileInit(&VideoFileInfo, VideoFileInfo.CurrentFileNum, FIND_FILE_RANGE_ALL ,NULL ,(UINT8*)VideoFileExtString);
    
}

#endif

