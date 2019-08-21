/*
********************************************************************************
*                   Copyright (c) 2008,ZhengYongzhi
*                         All rights reserved.
*
* File Name:   M3uBroWin.h
* 
* Description:  
*
* History:      <author>          <time>        <version>
*               
*    desc:      ORG.
********************************************************************************
*/

#ifndef _M3UBROWIN_H_
#define _M3UBROWIN_H_

#undef  EXT
#ifdef  _IN_M3UBRO_WIN_
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
//3#W$4zBk
#define _ATTR_M3UBROWIN_CODE_         __attribute__((section("M3uBroCode")))
#define _ATTR_M3UBROWIN_DATA_         __attribute__((section("M3uBroData")))
#define _ATTR_M3UBROWIN_BSS_          __attribute__((section("M3uBroBss"),zero_init))

//3uJ<;/4zBk
#define _ATTR_M3UBROWIN_INIT_CODE_    __attribute__((section("M3uBroInitCode")))
#define _ATTR_M3UBROWIN_INIT_DATA_    __attribute__((section("M3uBroInitData")))
#define _ATTR_M3UBROWIN_INIT_BSS_     __attribute__((section("M3uBroInitBss"),zero_init))

//743uJ<;/4zBk
#define _ATTR_M3UBROWIN_DEINIT_CODE_  __attribute__((section("M3uBroDeInitCode")))
#define _ATTR_M3UBROWIN_DEINIT_DATA_  __attribute__((section("M3uBroDeInitData")))
#define _ATTR_M3UBROWIN_DEINIT_BSS_   __attribute__((section("M3uBroDeInitBss"),zero_init))

//?I5w6H4zBk
#define _ATTR_M3UBROWIN_SERVICE_CODE_  __attribute__((section("M3uBroServiceCode")))
#define _ATTR_M3UBROWIN_SERVICE_DATA_ __attribute__((section("M3uBroServiceData")))
#define _ATTR_M3UBROWIN_SERVICE_BSS_   __attribute__((section("M3uBroServiceBss"),zero_init))
//
//LuD?PEO";qH!6N
#define _ATTR_M3UBRO_SORTGET_CODE_         __attribute__((section("M3uBroSortGetCode")))
#define _ATTR_M3UBRO_SORTGET_DATA_         __attribute__((section("M3uBroSortGetData")))
#define _ATTR_M3UBRO_SORTGET_BSS_          __attribute__((section("M3uBroSortGetBss"),zero_init))


#define     M3UBRO_TITLE_TXT_X            34
#define     M3UBRO_TITLE_TXT_Y            146 
#define     M3UBRO_TITLE_TXT_XSIZE        61
#define     M3UBRO_TITLE_TXT_YSIZE        12

#define     ALL_BROITEM_UPDATE_M3U			0
#define     UP_UPDATE_M3U					1
#define     DOWN_UPDATE_M3U					2

#define     PATH_SIZE_M3U					(3 + (MAX_DIR_DEPTH - 1) * 12 + 1)
#define     SHORT_NAME_SIZE_M3U				11

#define  MAX_ITEM_NUM_M3UBRO_DISP			8 //the max item number that one screen can display.




/*
*-------------------------------------------------------------------------------
*  
*                           Struct define
*  
*-------------------------------------------------------------------------------
*/
#define		  MAX_MUSIC_DIR_DEPTH		4

typedef struct _M3UBRO_STRUCT{
    
    struct _M3UBRO_STRUCT *pPrev;
	struct _M3UBRO_STRUCT *pNext;
	
	UINT16  FileType;
    UINT16  ItemNumber;
	UINT16  LongFileName[MAX_FILENAME_LEN];
	
}M3UBRO_STRUCT;

typedef struct _M3UBRO_DIR_TREE_STRUCT{

	UINT16 M3uDirTotalItem;
	UINT16 M3uDirBaseSortId[MAX_MUSIC_DIR_DEPTH];
	UINT16 M3uCurId[MAX_MUSIC_DIR_DEPTH];
	UINT16 ItemStar;
    UINT16 KeyCounter;
    UINT16 PreCounter;
    UINT16 DispTotalItem;
    M3UBRO_STRUCT *pM3uBro;
}M3UBRO_DIR_TREE_STRUCT;



#if 1
typedef struct _M3USORT_INFO_ADDR_STRUCT{

    UINT32 ulFileFullInfoSectorAddr;
    UINT16 uiM3uSortInfoAddrOffset[4];
    UINT32 ulFileSortInfoSectorAddr;
    UINT32 ulSortSubInfoSectorAddr[3];
}M3USORT_INFO_ADDR_STRUCT;
#endif



/*
*-------------------------------------------------------------------------------
*  
*                           Variable define
*  
*-------------------------------------------------------------------------------
*/

EXT _ATTR_M3UBROWIN_BSS_ M3USORT_INFO_ADDR_STRUCT   M3uSortInfoAddr;//16
EXT _ATTR_M3UBROWIN_BSS_ M3UBRO_STRUCT              M3uBroItem[MAX_ITEM_NUM_M3UBRO_DISP];//552
EXT _ATTR_M3UBROWIN_BSS_ M3UBRO_DIR_TREE_STRUCT     M3uDirTreeInfo;//28
EXT _ATTR_M3UBROWIN_BSS_ FIND_DATA                  stFindDataM3u;

EXT _ATTR_M3UBROWIN_BSS_  UINT16                    *M3uBroPrintfBuf;//4
EXT _ATTR_M3UBROWIN_BSS_  UINT16                    M3uBroTitle;//4


/*
--------------------------------------------------------------------------------
  
   Functon Declaration 
  
--------------------------------------------------------------------------------
*/
EXT void   M3uBroInit(void *pArg);
EXT void   M3uBroDeInit(void);
EXT UINT32 M3uBroService(void);
EXT UINT32 M3uBroKey(void);
EXT void   M3uBroDisplay(void);




/*
--------------------------------------------------------------------------------
  
  Description:  window sturcture definition
  
--------------------------------------------------------------------------------
*/
#ifdef _IN_M3UBRO_WIN_
_ATTR_M3UBROWIN_DATA_ WIN M3uBroWin = {

    NULL,
    NULL,

    M3uBroService,               //window service handle function.
    M3uBroKey,                   //window key service handle function.
    M3uBroDisplay,               //window display service handle function.

    M3uBroInit,                  //window initial handle function.
    M3uBroDeInit                 //window auti-initial handle function.

};

#else 
_ATTR_M3UBROWIN_DATA_ EXT WIN M3uBroWin;
#endif

/*
********************************************************************************
*
*                         End of MedialibWin.h
*
********************************************************************************
*/
#endif

