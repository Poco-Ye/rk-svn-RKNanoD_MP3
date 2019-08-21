/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��  HoldonPlay.h
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
    UINT32      CurrentFileNum;                                 // ��ǰ�ļ����
    UINT32      Video_Current_FrameNum;                         // ���浱ǰ֡��
    UINT8       HoldVideoGetSign;                               // �Ƿ񱣴�ϵ��־ 1:�жϵ� 0:û�жϵ�
    
}VIDEO_HOLDON_PLAY_INFO;


/*
********************************************************************************
*
*                         End of HoldonPlay.h
*
********************************************************************************
*/
#endif


