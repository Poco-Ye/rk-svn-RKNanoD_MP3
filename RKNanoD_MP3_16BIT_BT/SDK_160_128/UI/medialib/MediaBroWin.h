/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��   MediaBroWin.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*               anzhiguo      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _MEDIABROWIN_H_
#define _MEDIABROWIN_H_

#undef  EXT
#ifdef  _IN_MEDIABRO_WIN_
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
#define _ATTR_MEDIABROWIN_CODE_         __attribute__((section("MediaBroCode")))
#define _ATTR_MEDIABROWIN_DATA_         __attribute__((section("MediaBroData")))
#define _ATTR_MEDIABROWIN_BSS_          __attribute__((section("MediaBroBss"),zero_init))

//��ʼ������
#define _ATTR_MEDIABROWIN_INIT_CODE_    __attribute__((section("MediaBroInitCode")))
#define _ATTR_MEDIABROWIN_INIT_DATA_    __attribute__((section("MediaBroInitData")))
#define _ATTR_MEDIABROWIN_INIT_BSS_     __attribute__((section("MediaBroInitBss"),zero_init))

//����ʼ������
#define _ATTR_MEDIABROWIN_DEINIT_CODE_  __attribute__((section("MediaBroDeInitCode")))
#define _ATTR_MEDIABROWIN_DEINIT_DATA_  __attribute__((section("MediaBroDeInitData")))
#define _ATTR_MEDIABROWIN_DEINIT_BSS_   __attribute__((section("MediaBroDeInitBss"),zero_init))

//�ɵ��ȴ���
#define _ATTR_MEDIABROWIN_SERVICE_CODE_  __attribute__((section("MediaBroServiceCode")))
#define _ATTR_MEDIABROWIN_SERVICE_DATA_ __attribute__((section("MediaBroServiceData")))
#define _ATTR_MEDIABROWIN_SERVICE_BSS_   __attribute__((section("MediaBroServiceBss"),zero_init))
//
//��Ŀ��Ϣ��ȡ��
#define _ATTR_MEDIABRO_SORTGET_CODE_         __attribute__((section("MediaBroSortGetCode")))
#define _ATTR_MEDIABRO_SORTGET_DATA_         __attribute__((section("MediaBroSortGetData")))
#define _ATTR_MEDIABRO_SORTGET_BSS_          __attribute__((section("MediaBroSortGetBss"),zero_init))

#define _ATTR_MEDIABROSUBWIN_CODE_         __attribute__((section("MediaBroSubCode")))
#define _ATTR_MEDIABROSUBWIN_DATA_         __attribute__((section("MediaBroSubData")))
#define _ATTR_MEDIABROSUBWIN_BSS_          __attribute__((section("MediaBroSubBss"),zero_init))

#define _ATTR_MEDIAFAVOSUBWIN_CODE_         __attribute__((section("MediaFavoSubCode")))
#define _ATTR_MEDIAFAVOSUBWIN_DATA_         __attribute__((section("MediaFavoSubData")))
#define _ATTR_MEDIAFAVOSUBWIN_BSS_          __attribute__((section("MediaFavoSubBss"),zero_init))



#define     MEDIABRO_TITLE_TXT_X            34
#define     MEDIABRO_TITLE_TXT_Y            146 
#define     MEDIABRO_TITLE_TXT_XSIZE        61
#define     MEDIABRO_TITLE_TXT_YSIZE        12

#define     ALL_BROITEM_UPDATE   0
#define     UP_UPDATE    1
#define     DOWN_UPDATE  2

#if(MAX_DIR_DEPTH == 4) 
#define     PATH_SIZE                    MAX_FILENAME_LEN
#define     SHORT_NAME_SIZE              11
#else
#define     PATH_SIZE                   (3 + (MAX_DIR_DEPTH - 1) * 12 + 1)
#define     SHORT_NAME_SIZE              11
#endif

#define     PATH_ADDR_OFFSET            400 //related with media saved information.

#define	    SORT_TYPE_SEL_NOW_PLAY	    0
#define     SORT_TYPE_SEL_ID3TITLE      1
#define     SORT_TYPE_SEL_ID3ALBUM      3
#define     SORT_TYPE_SEL_ID3SINGER     2

#define     SORT_TYPE_SEL_GENRE         4//����
#define     SORT_TYPE_PLAY_LIST         5
#define     SORT_TYPE_SEL_FOLDER		6//�ļ������

#ifdef _RECORD_

#define     MUSIC_TYPE_SEL_RECORDFILE   7
#define     MUSIC_TYPE_SEL_FMFILE       8
#define     MUSIC_TYPE_SEL_MYFAVORITE   9
#define	    MEDIA_SORT_UPDATE_SEL		10
#define		MEDIA_MUSIC_BREAKPOINT	    11
#define     MUSIC_TYPE_SEL_RECORDFILE_DEL 12

#else
#define     MUSIC_TYPE_SEL_MYFAVORITE     7
#define	    MEDIA_SORT_UPDATE_SEL         8
#define     MEDIA_MUSIC_BREAKPOINT        9
#endif

#ifdef _M3U_							//<----sanshin_20150619
#define SORT_TYPE_SEL_M3U_BROWSER 98	//<----sanshin_20150619
#endif									//<----sanshin_20150619

#define SORT_TYPE_SEL_BROWSER  99
#define SORT_TYPE_SEL_FILENAME 100


#define     MEDIALIBTYPE          0     

#define  MAX_ITEM_NUM_MEDIABRO_DISP      8 //the max item number that one screen can display.

//----->sanshin_20150618
#define  MEDIABRO_ALBUM_OK 1			//<-----sanshin_20150618
#define  MEDIABRO_ALBUM_NG 0			//<-----sanshin_20150618
//<-----sanshin_20150618

#ifdef _IN_MEDIABRO_WIN_
EXT _ATTR_MEDIABROWIN_DATA_ UINT8 RecordPathString[] = {'U',':','\\','R','E','C','O','R','D',' ',' ',' ',' ',' ','\\','F','M',0};
#else
EXT _ATTR_MEDIABROWIN_DATA_ UINT8 RecordPathString[];
#endif

EXT _ATTR_MEDIABROWIN_DATA_ UINT32 RecordDirClus;
    
#ifdef _IN_MEDIABRO_WIN_
EXT _ATTR_MEDIABROSUBWIN_DATA_ UINT16 AddFileToFavoString[] = {SID_ADD_FAVORITE,SID_EXIT};	
#else
EXT _ATTR_MEDIABROSUBWIN_DATA_ UINT16 AddFileToFavoString[];
#endif

#ifdef _IN_MEDIABRO_WIN_
EXT _ATTR_MEDIAFAVOSUBWIN_DATA_ UINT16 DeletFileFromFavoString[] = {SID_DELETE_FILE,SID_CLEAR_FAVORITE,SID_EXIT};	
#else
EXT _ATTR_MEDIAFAVOSUBWIN_DATA_ UINT16 DeletFileFromFavoString[];
#endif

/*
*-------------------------------------------------------------------------------
*  
*                           Struct define
*  
*-------------------------------------------------------------------------------
*/
#define		  MAX_MUSIC_DIR_DEPTH		4

typedef struct _MUSICBRO_STRUCT{
    
    struct _MUSICBRO_STRUCT *pPrev;
	struct _MUSICBRO_STRUCT *pNext;
	
	UINT16  FileType;
    UINT16  ItemNumber;
	UINT16 LongFileName[MAX_FILENAME_LEN];
	
}MUSICBRO_STRUCT;

typedef struct _MUSIC_DIR_TREE_STRUCT{

	UINT16 MusicDirTotalItem;
	UINT16 MusicDirBaseSortId[MAX_MUSIC_DIR_DEPTH];
	UINT16 MusicDirDeep;
	UINT16 CurId[MAX_MUSIC_DIR_DEPTH];
	UINT16 ItemStar;//��Ļ�����ĵ�һ�����
    UINT16 KeyCounter;//����λ����?
    UINT16 PreCounter;//��һ���ι������λ��
    UINT16 DispTotalItem;
    MUSICBRO_STRUCT *pMusicBro;
}MUSIC_DIR_TREE_STRUCT;//TestTreeStruct;


typedef struct _MEDIA_SUBDATA_STRUCT{
 
	UINT8 TotalItem;
	UINT8 CurId;
	UINT8 ItemStar;//the frist index of screen top.

    UINT8 KeyCounter;
    UINT8 PreCounter;//last time cursor location
    UINT8 DispTotalItem;
    
}MEDIA_SUBDATA_STRUCT;

#if 1
typedef struct _SORT_INFO_ADDR_STRUCT{ //sch120411 for medialib
    
    UINT32 ulFileFullInfoSectorAddr;    // �����ļ���ϸ��Ϣ����ʼsector��ַ
  
    //UINT16  uiSortInfoAddrOffset[3];       // ͬһ���ļ�������Ϣ�еĲ�ͬ���ݵ�ƫ�Ƶ�ַ,��λByte
    UINT16  uiSortInfoAddrOffset[4]; // Rk Aaron.sun
    UINT32 ulFileSortInfoSectorAddr;    // �����ļ�������Ϣ����ʼsector��ַ(�źõ��ļ������б�ĵ�ַ��ui���ȡ��Ϣʱ���ȶ��ñ��ȡ�ļ���
                                        //��ͨ����ȡ���ļ��ţ��ҵ���Ӧ���ļ���ϸ��Ϣ��flash�еĵ�ַ��ͨ��ƫ�Ƶ�ַ
                                        //uiSortInfoAddrOffset��ȡ��Ӧ����ʾ��Ϣ)
    UINT32 ulSortSubInfoSectorAddr[3];     // ID3��Ϣ���ౣ���ַ
  
}SORT_INFO_ADDR_STRUCT;
#endif
//#define  MAX_ITEM_NUM_MEDIA_DISP      4 //һ��������ʾ�������Ŀ��




/*
*-------------------------------------------------------------------------------
*  
*                           Variable define
*  
*-------------------------------------------------------------------------------
*/

EXT _ATTR_MEDIABROWIN_BSS_  SORT_INFO_ADDR_STRUCT    SortInfoAddr;//16
EXT _ATTR_MEDIABROWIN_BSS_ MUSICBRO_STRUCT          MusicBroItem[MAX_ITEM_NUM_MEDIABRO_DISP];//552
EXT _ATTR_MEDIABROWIN_BSS_ MUSIC_DIR_TREE_STRUCT    MusicDirTreeInfo;//28
EXT _ATTR_MEDIABROWIN_BSS_ MUSIC_DIR_TREE_STRUCT    MusicDirThumbTreeInfo;//<----sanshin_20150625
EXT _ATTR_MEDIABROWIN_BSS_ UINT32 	    CurMusicDirClusBak;
EXT _ATTR_MEDIABROWIN_BSS_ UINT16       MusicBroDispFlag;

EXT _ATTR_MEDIABROWIN_BSS_ UINT16       MusicPrevScrollY;

EXT _ATTR_MEDIABROWIN_BSS_ UINT16       DialogAddDelAdjustValue;
EXT _ATTR_MEDIABROWIN_BSS_ UINT16	    DialogAddDelAllAdjustValue;

EXT _ATTR_MEDIABROWIN_BSS_ FIND_DATA    FindDataMusicBro;

EXT _ATTR_MEDIABROWIN_BSS_ FIND_DATA stFindData;

EXT _ATTR_MEDIABROWIN_BSS_  UINT16       *MediaBroPrintfBuf;//4
EXT _ATTR_MEDIABROWIN_BSS_  UINT16       MediaBroTitle;//4

EXT _ATTR_MEDIABROWIN_BSS_ MEDIA_SUBDATA_STRUCT MediaSubData;//ý��������Ӵ��ڹ���ͬһ���ṹ����� //6

/*
--------------------------------------------------------------------------------
  
   Functon Declaration 
  
--------------------------------------------------------------------------------
*/
EXT void MediaBroInit(void *pArg);
EXT void MediaBroDeInit(void);

EXT UINT32 MediaBroService(void);
EXT UINT32 MediaBroKey(void);
EXT void   MediaBroDisplay(void);


EXT void MediaBroSubInit(void *pArg);
EXT void MediaBroSubDeInit(void);

EXT UINT32 MediaBroSubService(void);
EXT UINT32 MediaBroSubKey(void);
EXT void   MediaBroSubDisplay(void);

#if 1
EXT void MediaFavoSubInit(void *pArg);
EXT void MediaFavoSubDeInit(void);

EXT UINT32 MediaFavoSubService(void);
EXT UINT32 MediaFavoSubKey(void);
EXT void   MediaFavoSubDisplay(void);

#endif

/*
--------------------------------------------------------------------------------
  
  Description:  window sturcture definition
  
--------------------------------------------------------------------------------
*/
#ifdef _IN_MEDIABRO_WIN_
_ATTR_MEDIABROWIN_DATA_ WIN MediaBroWin = {
    
    NULL,
    NULL,
    
    MediaBroService,               //window service handle function.
    MediaBroKey,                   //window key service handle function.
    MediaBroDisplay,               //window display service handle function.
    
    MediaBroInit,                  //window initial handle function.
    MediaBroDeInit                 //window auti-initial handle function.
    
};
#else 
_ATTR_MEDIABROWIN_DATA_ EXT WIN MediaBroWin;
#endif

/*
********************************************************************************
*
*                         End of MedialibWin.h
*
********************************************************************************
*/
#endif

