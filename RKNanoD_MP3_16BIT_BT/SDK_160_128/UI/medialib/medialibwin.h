/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：   medialibwin.h
*
* Description:
*
* History:      <author>          <time>        <version>
*               anzhiguo      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _MEDIALIBWIN_H_
#define _MEDIALIBWIN_H_

#undef  EXT
#ifdef _IN_MEDIALIBWIN_
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
//常驻代码
#define _ATTR_MEDIALIBWIN_CODE_         __attribute__((section("MediaWinCode")))
#define _ATTR_MEDIALIBWIN_DATA_         __attribute__((section("MediaWinData")))
#define _ATTR_MEDIALIBWIN_BSS_          __attribute__((section("MediaWinBss"),zero_init))

//初始化代码
#define _ATTR_MEDIALIBWIN_INIT_CODE_    __attribute__((section("MediaWinInitCode")))
#define _ATTR_MEDIALIBWIN_INIT_DATA_    __attribute__((section("MediaWinInitData")))
#define _ATTR_MEDIALIBWIN_INIT_BSS_     __attribute__((section("MediaWinInitBss"),zero_init))

//反初始化代码
#define _ATTR_MEDIALIBWIN_DEINIT_CODE_  __attribute__((section("MediaWinDeInitCode")))
#define _ATTR_MEDIALIBWIN_DEINIT_DATA_  __attribute__((section("MediaWinDeInitData")))
#define _ATTR_MEDIALIBWIN_DEINIT_BSS_   __attribute__((section("MediaWinDeInitBss"),zero_init))

//可调度代码
#define _ATTR_MEDIALIBWIN_SERVICE_CODE_  __attribute__((section("MediaWinServiceCode")))
#define _ATTR_MEDIALIBWIN_SERVICE_DATA_ __attribute__((section("MediaWinServiceData")))
#define _ATTR_MEDIALIBWIN_SERVICE_BSS_   __attribute__((section("MediaWinServiceBss"),zero_init))

#define     MEDIALIB_TITLE_TXT_X            34
#define     MEDIALIB_TITLE_TXT_Y            146
#define     MEDIALIB_TITLE_TXT_XSIZE        61
#define     MEDIALIB_TITLE_TXT_YSIZE        12

#define     MEDIA_ITEM_X                    20  //x direction start point
#define     MEDIA_ITEM_Y                    5  //y direction start point
#define     MEDIA_ITEM_XSIZE                77  //item x size
#define     MEDIA_ITEM_YSIZE                17  //item y size
#define     MEDIA_ITEM_CHARYSIZE            12  //the height of item
/*
*-------------------------------------------------------------------------------
*
*                           Struct define
*
*-------------------------------------------------------------------------------
*/
#ifdef _MEDIAUPDATE_
#define  MAX_TOTLE_ITEM_NUM             11
#elif defined(_RECORD_)
#define  MAX_TOTLE_ITEM_NUM             6
#else
#define  MAX_TOTLE_ITEM_NUM             6
#endif
#define  MAX_ITEM_NUM_MEDIA_DISP       6//	3	//the max display item number that one screeen display

typedef struct _MedialibWinDataStruct
{

    UINT8                      TotalItem;                      //total items.
    UINT8                      CurStartIndex;                  //the start index in current page
    UINT8                      CurPointer;                     //cursor location.
    UINT8                      PrePointer;                     //last time cursor location
    UINT8                      TotalDispItem;                  //one screen item number
    UINT8                      CurNum;                         //cursor item index

} MedialibWinDataStruct;

/*
*-------------------------------------------------------------------------------
*
*                           Variable define
*
*-------------------------------------------------------------------------------
*/

//#define MainMenuBatteryLevel       gBattery.Batt_Level
//#define MainmenuHoldState          0

_ATTR_MEDIALIBWIN_BSS_ EXT MedialibWinDataStruct    MedialibWinItemData;

#ifdef _IN_MEDIALIBWIN_

#ifdef _MEDIAUPDATE_
EXT _ATTR_MEDIALIBWIN_DATA_ UINT16 MediaItem[MAX_TOTLE_ITEM_NUM][2] ={
                                                    {IMG_ID_NOWPLAY,SID_PLAYING}, //playing
                                                    {IMG_ID_ALLMUSIC,SID_ALL_MUISC_FILE},//all music
                                                    {IMG_ID_SINGER,SID_ARTIST}, //artist
                                                    {IMG_ID_ABLUM,SID_ALBUM},//album
                                                    {IMG_ID_GENRE,SID_GENRE},  //genre
                                                    {NULL,SID_PLAY_LIST},  //play list
                                                    {NULL,SID_DIR_LIST},//目录列表
                                                    {NULL,SID_RECORE_FILE}, //录音文件
                                                    {NULL,SID_RECORE_FILE}, //录音文件
                                                    {NULL,SID_MY_FAVORITE}, //我的收藏夹
                                                    {NULL,SID_UPDATE_MEDIALIB}, //更新媒体库
                                                    //{NULL,SID_HOLDON_PLAY} //断点播放

                                                   };
#elif defined(_RECORD_)
EXT _ATTR_MEDIALIBWIN_DATA_ UINT16 MediaItem[MAX_TOTLE_ITEM_NUM][2] ={
                                                    {IMG_ID_NOWPLAY,SID_PLAYING}, //playing
                                                    {IMG_ID_ALLMUSIC,SID_ALL_MUISC_FILE},//all music
                                                    {IMG_ID_SINGER,SID_ARTIST}, //artist
                                                    {IMG_ID_ABLUM,SID_ALBUM},//album
                                                    {IMG_ID_GENRE,SID_GENRE},  //genre
                                                    {IMG_ID_LIST,SID_DIR_LIST},//direction list

                                                    //{NULL,SID_PLAY_LIST},  //play list
                                                    //{NULL,SID_RECORE_FILE}, //录音文件
                                                    //{NULL,SID_RECORE_FILE}, //录音文件
                                                    //{NULL,SID_MY_FAVORITE}, //我的收藏夹
                                                    //{NULL,SID_UPDATE_MEDIALIB}, //更新媒体库
                                                    //{NULL,SID_HOLDON_PLAY} //断点播放

                                                   };
#else
EXT _ATTR_MEDIALIBWIN_DATA_ UINT16 MediaItem[MAX_TOTLE_ITEM_NUM][2] ={
                                                    {IMG_ID_NOWPLAY,SID_PLAYING}, //playing
                                                    {IMG_ID_ALLMUSIC,SID_ALL_MUISC_FILE},//all music
                                                    {IMG_ID_SINGER,SID_ARTIST}, //artist
                                                    {IMG_ID_ABLUM,SID_ALBUM},//album
                                                    {IMG_ID_GENRE,SID_GENRE},  //genre
                                                    {IMG_ID_LIST,SID_DIR_LIST},//direction list
                                                    //{NULL,SID_PLAY_LIST},  //play list
                                                    //{NULL,SID_MY_FAVORITE}, //我的收藏夹
                                                    //{NULL,SID_UPDATE_MEDIALIB}, //更新媒体库
                                                    //{NULL,SID_HOLDON_PLAY}, //断点播放
                                                   };
#endif
#else
EXT _ATTR_MEDIALIBWIN_DATA_ UINT16 MediaItem[MAX_TOTLE_ITEM_NUM][2];
#endif

/*
--------------------------------------------------------------------------------

   Functon Declaration

--------------------------------------------------------------------------------
*/
extern void MedialibWinInit(void *pArg);
extern void MedialibWinDeInit(void);

extern UINT32 MedialibWinService(void);
extern UINT32 MedialibWinKey(void);
extern void   MedialibWinDisplay(void);


/*
--------------------------------------------------------------------------------

  Description:  window sturcture definition

--------------------------------------------------------------------------------
*/
#ifdef _IN_MEDIALIBWIN_
_ATTR_MEDIALIBWIN_DATA_ WIN MedialibWin = {

    NULL,
    NULL,

    MedialibWinService,               //window service handle function.
    MedialibWinKey,                   //window key service handle function.
    MedialibWinDisplay,               //window display service handle function.

    MedialibWinInit,                  //window initial handle function.
    MedialibWinDeInit                 //window auti-initial handle function.

};
#else
_ATTR_MEDIALIBWIN_DATA_ EXT WIN MedialibWin;
#endif

/*
********************************************************************************
*
*                         End of MedialibWin.h
*
********************************************************************************
*/
#endif


