/*
********************************************************************************
*                   Copyright (c) 2008,ZhengYongzhi
*                         All rights reserved.
*
* File Name#:   PicBroWin.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*               anzhiguo      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _PICBROWIN_H_
#define _PICBROWIN_H_

#undef  EXT
#ifdef  _IN_PICBRO_WIN_
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
#define _ATTR_PICBROWIN_CODE_         __attribute__((section("PicBroCode")))
#define _ATTR_PICBROWIN_DATA_         __attribute__((section("PicBroData")))
#define _ATTR_PICBROWIN_BSS_          __attribute__((section("PicBroBss"),zero_init))

//3uJ<;/4zBk
#define _ATTR_PICBROWIN_INIT_CODE_    __attribute__((section("PicBroInitCode")))
#define _ATTR_PICBROWIN_INIT_DATA_    __attribute__((section("PicBroInitData")))
#define _ATTR_PICBROWIN_INIT_BSS_     __attribute__((section("PicBroInitBss"),zero_init))

//743uJ<;/4zBk
#define _ATTR_PICBROWIN_DEINIT_CODE_  __attribute__((section("PicBroDeInitCode")))
#define _ATTR_PICBROWIN_DEINIT_DATA_  __attribute__((section("PicBroDeInitData")))
#define _ATTR_PICBROWIN_DEINIT_BSS_   __attribute__((section("PicBroDeInitBss"),zero_init))

//?I5w6H4zBk
#define _ATTR_PICBROWIN_SERVICE_CODE_  __attribute__((section("PicBroServiceCode")))
#define _ATTR_PICBROWIN_SERVICE_DATA_ __attribute__((section("PicBroServiceData")))
#define _ATTR_PICBROWIN_SERVICE_BSS_   __attribute__((section("PicBroServiceBss"),zero_init))
//
//LuD?PEO";qH!6N
#define _ATTR_PICBRO_SORTGET_CODE_         __attribute__((section("PicBroSortGetCode")))
#define _ATTR_PICBRO_SORTGET_DATA_         __attribute__((section("PicBroSortGetData")))
#define _ATTR_PICBRO_SORTGET_BSS_          __attribute__((section("PicBroSortGetBss"),zero_init))

//#define _ATTR_PICBROSUBWIN_CODE_         __attribute__((section("PicBroSubCode")))
//#define _ATTR_PICBROSUBWIN_DATA_         __attribute__((section("PicBroSubData")))
//#define _ATTR_PICBROSUBWIN_BSS_          __attribute__((section("PicBroSubBss"),zero_init))

//#define _ATTR_MEDIAFAVOSUBWIN_CODE_         __attribute__((section("MediaFavoSubCode")))
//#define _ATTR_MEDIAFAVOSUBWIN_DATA_         __attribute__((section("MediaFavoSubData")))
//#define _ATTR_MEDIAFAVOSUBWIN_BSS_          __attribute__((section("MediaFavoSubBss"),zero_init))



#define     PICBRO_TITLE_TXT_X            34
#define     PICBRO_TITLE_TXT_Y            146 
#define     PICBRO_TITLE_TXT_XSIZE        61
#define     PICBRO_TITLE_TXT_YSIZE        12

#define     ALL_BROITEM_UPDATE_JPEG   0
#define     UP_UPDATE_JPEG    1
#define     DOWN_UPDATE_JPEG  2

#define     PATH_SIZE_JPEG                  (3 + (MAX_DIR_DEPTH - 1) * 12 + 1)
#define     SHORT_NAME_SIZE_JPEG            11

//#define     PATH_ADDR_OFFSET_JPEG           400 //related with media saved information.


#define  MAX_ITEM_NUM_PICBRO_DISP      6 //<---- sanshin //  //the max item number that one screen can display.




/*
*-------------------------------------------------------------------------------
*  
*                           Struct define
*  
*-------------------------------------------------------------------------------
*/
#define		  MAX_MUSIC_DIR_DEPTH		4

typedef struct _PICBRO_STRUCT{
    
    struct _PICBRO_STRUCT *pPrev;
	struct _PICBRO_STRUCT *pNext;
	
	UINT16  FileType;
    UINT16  ItemNumber;
	UINT16 LongFileName[MAX_FILENAME_LEN];
	
}PICBRO_STRUCT;

typedef struct _JPEG_DIR_TREE_STRUCT{

	UINT16 JpegDirTotalItem;
	UINT16 JpegDirBaseSortId[MAX_MUSIC_DIR_DEPTH];
//	UINT16 JpegDirDeep;
	UINT16 JpegCurId[MAX_MUSIC_DIR_DEPTH];
	UINT16 ItemStar;//FAD;6%2?5D5ZR;LuPr:E
    UINT16 KeyCounter;//9b1j5DN;VCBp?
    UINT16 PreCounter;//IOR;8v4N9b1jKyTZN;VC
    UINT16 DispTotalItem;
    PICBRO_STRUCT *pPicBro;
}JPEG_DIR_TREE_STRUCT;//TestTreeStruct;



#if 1
typedef struct _JPEGSORT_INFO_ADDR_STRUCT{ //sch120411 for medialib
    
    UINT32 ulFileFullInfoSectorAddr;    // 1#4fND<~OjO8PEO"5DFpJ<sector5XV7
  
    //UINT16  uiSortInfoAddrOffset[3];       // M,R;8vND<~1#4fPEO"VP5D2;M,DZH]5DF+RF5XV7,5%N;Byte
    UINT16  uiJpegSortInfoAddrOffset[4]; // Rk Aaron.sun
    UINT32 ulFileSortInfoSectorAddr;    // 1#4fND<~EEPrPEO"5DFpJ<sector5XV7(EE:C5DND<~:EPrAP1m5D5XV7#,ui2c6AH!PEO"J1#,OH6A8C1m;qH!ND<~:E
                                        //TZM(9};qH!5DND<~:E#,UR5=6TS&5DND<~OjO8PEO"TZflashVP5D5XV7:s#,M(9}F+RF5XV7
                                        //uiSortInfoAddrOffset;qH!O`S&5DOTJ>PEO")
    UINT32 ulSortSubInfoSectorAddr[3];     // ID3PEO"9i@`1#4f5XV7
  
}JPEGSORT_INFO_ADDR_STRUCT;
#endif
//#define  MAX_ITEM_NUM_MEDIA_DISP      4 //R;FA?IRTOTJ>5DWn4sLuD?J}




/*
*-------------------------------------------------------------------------------
*  
*                           Variable define
*  
*-------------------------------------------------------------------------------
*/

EXT _ATTR_PICBROWIN_BSS_  JPEGSORT_INFO_ADDR_STRUCT   JpegSortInfoAddr;//16
EXT _ATTR_PICBROWIN_BSS_ PICBRO_STRUCT          PicBroItem[MAX_ITEM_NUM_PICBRO_DISP];//552
EXT _ATTR_PICBROWIN_BSS_ JPEG_DIR_TREE_STRUCT    JpegDirTreeInfo;//28
EXT _ATTR_PICBROWIN_BSS_ JPEG_DIR_TREE_STRUCT    JpegDirThumbTreeInfo;//<-----sanshin_20150625
//EXT _ATTR_PICBROWIN_BSS_ UINT32 	    CurMusicDirClusBak;
//EXT _ATTR_PICBROWIN_BSS_ UINT16       MusicBroDispFlag;

//EXT _ATTR_PICBROWIN_BSS_ UINT16       MusicPrevScrollY;

//EXT _ATTR_PICBROWIN_BSS_ UINT16       DialogAddDelAdjustValue;
//EXT _ATTR_PICBROWIN_BSS_ UINT16	    DialogAddDelAllAdjustValue;

//EXT _ATTR_PICBROWIN_BSS_ FIND_DATA    FindDataMusicBro;

EXT _ATTR_PICBROWIN_BSS_ FIND_DATA 		stFindDataJpeg;

EXT _ATTR_PICBROWIN_BSS_  UINT16       *PicBroPrintfBuf;//4
EXT _ATTR_PICBROWIN_BSS_  UINT16       PicBroTitle;//4


/*
--------------------------------------------------------------------------------
  
   Functon Declaration 
  
--------------------------------------------------------------------------------
*/
EXT void PicBroInit(void *pArg);
EXT void PicBroDeInit(void);

EXT UINT32 PicBroService(void);
EXT UINT32 PicBroKey(void);
EXT void   PicBroDisplay(void);




/*
--------------------------------------------------------------------------------
  
  Description:  window sturcture definition
  
--------------------------------------------------------------------------------
*/
#ifdef _IN_PICBRO_WIN_
_ATTR_PICBROWIN_DATA_ WIN PicBroWin = {
    
    NULL,
    NULL,
    
    PicBroService,               //window service handle function.
    PicBroKey,                   //window key service handle function.
    PicBroDisplay,               //window display service handle function.
    
    PicBroInit,                  //window initial handle function.
    PicBroDeInit                 //window auti-initial handle function.
    
};

#else 
_ATTR_PICBROWIN_DATA_ EXT WIN PicBroWin;
#endif

/*
********************************************************************************
*
*                         End of MedialibWin.h
*
********************************************************************************
*/
#endif

