/*
********************************************************************************
*                   Copyright (c) 2012,SunChuanHu
*                         All rights reserved.
*
* File Name��   MDBBuildWin.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*               SunChuanHu      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*//*
********************************************************************************
*                   Copyright (c) 2012,SunChuanHu
*                         All rights reserved.
*
* File Name��   MDBBuildWin.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*               SunChuanHu      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#include "SysConfig.h"

#ifndef __MDBBUILDWIN_H_
#define __MDBBUILDWIN_H_

#undef  EXT
#ifdef _IN_MDBBUILDWIN_
#define EXT
#else
#define EXT extern
#endif

/*
*-------------------------------------------------------------------------------
*  
*                           Macro define
*  
*-------------------------------------------------------------------------------
*/
//section define
//��פ����
#define _ATTR_MDBBUILDWIN_CODE_         __attribute__((section("MdbBuildWinCode")))
#define _ATTR_MDBBUILDWIN_DATA_         __attribute__((section("MdbBuildWinData")))
#define _ATTR_MDBBUILDWIN_BSS_          __attribute__((section("MdbBuildWinBss"),zero_init))

#define     MEDIA_UPDATA_TXT_X             0    // the start point of x direction.
#define     MEDIA_UPDATA_TXT_Y             80    //the start point of y direction.
#define     MEDIA_UPDATA_TXT_XSIZE         128   //X size
#define     MEDIA_UPDATA_TXT_YSIZE         12   //Y size

#define     MDB_TITLE_TXT_X            34
#define     MDB_TITLE_TXT_Y            146
#define     MDB_TITLE_TXT_XSIZE        61
#define     MDB_TITLE_TXT_YSIZE        12

/*
--------------------------------------------------------------------------------
  
   Functon Declaration 
  
--------------------------------------------------------------------------------
*/
extern void MdbBuildWinInit(void *pArg);
extern void MdbBuildWinDeInit(void);

extern UINT32 MdbBuildWinService(void);
extern UINT32 MdbBuildWinKey(void);
extern void   MdbBuildWinDisplay(void);

extern void MedialibUpdateDisplayHook(void);

/*
--------------------------------------------------------------------------------
  
  Description:  ���ڽṹ�嶨��
  
--------------------------------------------------------------------------------
*/
#ifdef _IN_MDBBUILDWIN_
_ATTR_MDBBUILDWIN_DATA_ WIN MdbBuildWin = {
    
    NULL,
    NULL,
    
    MdbBuildWinService,               //���ڷ�������
    MdbBuildWinKey,                   //���ڰ����������
    MdbBuildWinDisplay,               //������ʾ�������
    
    MdbBuildWinInit,                  //���ڳ�ʼ������
    MdbBuildWinDeInit                 //���ڷ���ʼ������
    
};
#else 
_ATTR_MDBBUILDWIN_DATA_ EXT WIN MdbBuildWin;
#endif

/*
********************************************************************************
*
*                         End of MedialibWin.h
*
********************************************************************************
*/

#endif

