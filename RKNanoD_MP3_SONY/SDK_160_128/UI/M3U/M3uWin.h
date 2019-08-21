/*
********************************************************************************
*                   Copyright (c) 2008,CHENFEN
*                         All rights reserved.
*
* File Name£º  M3uWin.h
* 
* Description: 
*
* History:      <author>          <time>        <version>
*               sanshin           15/06/16       1.0
*    desc:      ORG.
********************************************************************************
*/
#include	"BrowserUI.h"
#ifndef _M3U_UI_H
#define _M3U_UI_H

#undef EXT
#ifdef _IN_M3U_UI_
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
//m3u window permanent code.
#define _ATTR_M3U_UI_CODE_         __attribute__((section("M3uUICode")))
#define _ATTR_M3U_UI_DATA_         __attribute__((section("M3uUIData")))
#define _ATTR_M3U_UI_BSS_          __attribute__((section("M3uUIBss"),zero_init))

//------------------------------------------------------------------------------
//related interface
#define     M3U_ITEM_PER_PAGE          8  // 3  added by chenz
#define     M3U_SCROLL_BAR_COUNT       123

//------------------------------------------------------------------------------
#define     M3U_TITLE_TXT_X            34
#define     M3U_TITLE_TXT_Y            146 
#define     M3U_TITLE_TXT_XSIZE        61
#define     M3U_TITLE_TXT_YSIZE        12

//------------------------------------------------------------------------------
//display mark
#define     M3U_DIS_NULL               0x0000
#define     M3U_DIS_BAR                0x0001
#define     M3U_DIS_ALL_ITEM           0x0002
#define     M3U_DIS_SELE_ITEM          0x0004
#define     M3U_DIS_BATT               0x0008
#define     M3U_DIS_HOLD               0x0010
#define     M3U_DIS_MEM_SEL            0x0020
#define     M3U_DIS_SCLLO_ITEM         0x0040
#define     M3U_DIS_ALL                (M3U_DIS_BAR|M3U_DIS_ALL_ITEM|M3U_DIS_BATT|M3U_DIS_HOLD)

//------------------------------------------------------------------------------
#define     MSG_MODULE_OK               0x0000
#define     MSG_MODULE_EXIT             0x0001
#define     MSG_MODULE_ERROR            0x0002
//------------------------------------------------------------------------------

#define  MAX_ITEM_NUM_M3UWIN_DISP			8 //the max item number that one screen can display.

#define  CODE_TYPE_DEFAULT   0
#define  CODE_TYPE_UTF_8     1

/*
*-------------------------------------------------------------------------------
*  
*                           Struct define
*  
*-------------------------------------------------------------------------------
*/

typedef enum _M3U_TYPE
{
    M3U_FILE_FROM_DATABASE = 0,
    M3U_FILE_FROM_FS,
}M3U_TYPE;

//---->sanshin_M3U_hoshi
typedef struct _M3U_MUSIC_STRUCT
{
	UINT16 LongFileName[MAX_FILENAME_LEN];
	//UINT16 FileNameLen;
	//UINT16 PathOffset;
	//INT32  Clus;
	//INT32  Index;
	//UINT16 globalFileNum;
}M3U_MUSIC_STRUCT;


typedef struct _M3U_FILE_MANAGE_STRUCT
{
	M3U_MUSIC_STRUCT M3u_Music[MAX_ITEM_NUM_M3UWIN_DISP];

}M3U_FILE_MANAGE_STRUCT;

EXT _ATTR_M3U_UI_BSS_ M3U_FILE_MANAGE_STRUCT M3uManage;
//<----sanshin_M3U_hoshi

/*
*-------------------------------------------------------------------------------
*  
*                           Variable define
*  
*-------------------------------------------------------------------------------
*/
//extenal variables interface
#define M3uHoldState                     0
#define M3uBatteryLevel                  gBattery.Batt_Level
#define M3U_LIST_FILE_MAX                1000

//system permanent variables
_ATTR_M3U_UI_BSS_        EXT FileType    gM3uFindFileType;
_ATTR_M3U_UI_BSS_        EXT INT16       gM3uFildFileNum;
_ATTR_M3U_UI_BSS_        EXT INT16       gM3uFsType;

//UI variables
_ATTR_M3U_UI_BSS_       EXT UINT16      *M3uPrintfBuf;

_ATTR_M3U_UI_BSS_       EXT UINT16       gM3uGlobalFileNumBuf[M3U_LIST_FILE_MAX];
_ATTR_M3U_UI_BSS_       EXT UINT16       gM3uGlobalFileCnt;

/*
--------------------------------------------------------------------------------
  
   Functon Declaration 
  
--------------------------------------------------------------------------------
*/
void M3uWinInit(void *pArg);
void M3uWinDeInit(void);
uint32 M3uWinService(void);
UINT32 M3uWinKey(void);
void M3uWinPaint(void);
void M3uWinScrollInit(LCD_RECT *pRect, uint16 ImageID, uint16 *pstr, uint16 Speed);

void M3uGetLongFileNameEraseSp( int32 DirClus, int32 Index, FS_TYPE FsType, uint16* lfName );	//<----sanshin_20150707

void M3uValueInit(UINT16 NeedInitAll);

void M3uKeyDownProc(void);
void M3uKeyUpProc(void);
int16 M3uKeyEnterPro(void);
int16 M3uKeyExitPro(void);

/*
--------------------------------------------------------------------------------
  
  Description:  window structure definition
  
--------------------------------------------------------------------------------
*/



#ifdef _IN_M3U_UI_
_ATTR_M3U_UI_DATA_ WIN M3uWin = {
    
    NULL,
    NULL,
        
    M3uWinService,               //window service handle
    M3uWinKey,                   //window key handle
    M3uWinPaint,                 //window display handle
        
    M3uWinInit,                  //window initial handle
    M3uWinDeInit                 //window auti-initial handle
    
};
#else
_ATTR_M3U_UI_DATA_ EXT  WIN M3uWin;
#endif


/*
********************************************************************************
*
*                         End of file
*
********************************************************************************
*/

#endif //_M3U_UI_H

