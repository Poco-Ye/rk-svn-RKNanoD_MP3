/*
********************************************************************************
*                   Copyright (c) 2008,CHENFEN
*                         All rights reserved.
*
* File Name:  M3uWinSub.h
* 
* Description: 
*
* History:      <author>          <time>        <version>
*              
*    desc:      ORG.
********************************************************************************
*/

#ifndef _M3U_CORE_H_
#define _M3U_CORE_H_

#undef  EXT
#ifdef _IN_M3U_CORE_
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

#define _ATTR_M3U_CORE_CODE_         __attribute__((section("M3uCoreCode")))
#define _ATTR_M3U_CORE_DATA_         __attribute__((section("M3uCoreData")))
#define _ATTR_M3U_CORE_BSS_          __attribute__((section("M3uCoreBss"),zero_init))


#ifndef PATH_MAXLEN
    #define PATH_MAXLEN                 (3 + (MAX_DIR_DEPTH-1)*12 +1)
#endif


/*
*-------------------------------------------------------------------------------
*  
*                           Struct define
*  
*-------------------------------------------------------------------------------
*/

//J}>]=a99
typedef struct _M3uFileInfo
{
    FileType FileType;                                          //file type
    UINT8   *FileExtName;                                       //file extension
    
}M3uFileInfo;


typedef struct _FolderIndexInfo
{
    
    UINT16  StartIndex;
    UINT16  CurIndex;
//    int32u  DirClus;

} FolderIndexInfo;


typedef struct _FileListInfo
{
    UINT32 DirClus; 
    UINT32 Index; 
} FileListInfo;


typedef struct _FolderInfoStruct
{
    
    UINT16 TotalItems;                                         //total item number.
    UINT16 TotalFiles;                                         //total file number,
    UINT16  TotalFolders;                                       //total direction number,
    UINT32  DirClus;                                            //directory cluster
    
} FolderInfoStruct;


typedef struct _M3uFileStruct {
    
    struct _M3uFileStruct   *pPrev;
	struct _M3uFileStruct   *pNext;
    struct _FIND_DATA           FileLocInfo;                    //it used to locating of file
    FileType                    FileType;
    FDT                         FileFDT;                         //pointer
    UINT16                      LongFileName[MAX_FILENAME_LEN];
    
} M3uFileStruct;


typedef struct _M3uBrowserDataStruct
{
	//list head for _M3uFileStruct 
    struct _M3uFileStruct   *pM3uFile;   //File list
    uint16                      CurDiskSelect;                  //current disk select
    uint16                      PreDiskSelect;                  //it used for disk switch
    uint16                      TotleDisk;                      //all disk
    
    FolderInfoStruct            FolderInfo[MAX_DIR_DEPTH];
    UINT16                      CurStartIndex[MAX_DIR_DEPTH];   // start Index in current directory;maybe many page -- 8 item a page
    UINT16                      CurPointerBack[MAX_DIR_DEPTH];  //current page display start Index
    UINT16                      CurPointer;                     //cursor postion in current dir
    UINT16                      PrePointer;                     //last cursor postion

    M3uFileInfo                 *pSearchFileInfo;   
    
    UINT16                      M3uTitleId ;            //display titile address
    UINT16                      ScrollBar;                    
    
} M3uBrowserDataStruct;



typedef struct _M3uPlayListDataStruct
{
	//list head for _M3uFileStruct 
    struct _PlayListEntryInfoStruct   *pPlayList;   //File list
    
    UINT16             totalFiles;
    UINT16             playlistDirDeep;                        //directory deepth
    UINT16             curStartIndex[MAX_DIR_DEPTH];   // start Index in current directory;maybe many page -- 8 item a page. will be set to 0 when enter a folder
    UINT16             curPointerBack[MAX_DIR_DEPTH];  //current page display start Index
    UINT16             index;                     //cursor postion in playlist
    UINT16             lastIndex;                     //last cursor postion

    M3uFileInfo                 *pSearchFileInfo;   
    
   // UINT16                      M3uTitleId ;            //display titile address
    UINT16                      ScrollBar;                    
    
} M3uPlayListDataStruct;


/*
*-------------------------------------------------------------------------------
*  
*                           Variable define
*  
*-------------------------------------------------------------------------------
*/
#define   gM3uFileReadBufSize 500

_ATTR_M3U_CORE_BSS_     EXT M3uBrowserDataStruct   gM3uBrowserData;   //include file list / index / pointer for m3u
_ATTR_M3U_CORE_BSS_     EXT M3uFileStruct       gBrowserFileArray[M3U_ITEM_PER_PAGE];  //file array ,just for init a file list as  M3uData.pM3uFile  in ValueInit()

  
#ifdef _IN_M3U_CORE_
_ATTR_M3U_CORE_DATA_    EXT M3uFileInfo     gM3uFileInfo[] = 
{
//this array is for compare ext name , can't miss any of the type, other wise it will crash--by norton
    {FileTypeAudio,   (UINT8*)AudioFileExtString    },
    {FileTypeVideo,   (UINT8*)VideoFileExtString    },
    {FileTypePicture, (UINT8*)PictureFileExtString  },
    {FileTypeText,    (UINT8*)TextFileExtString     },
	{FileTypeM3u,     (UINT8*)M3UFileExtString      },
    {FileTypeALL,     (UINT8*)ALLFileExtString      },
};
#else
_ATTR_M3U_CORE_DATA_    EXT M3uFileInfo     gM3uFileInfo[];
#endif


/*
--------------------------------------------------------------------------------
  
   Functon Declaration 
  
--------------------------------------------------------------------------------
*/
M3uFileInfo *M3uGetSearchFileInfo(FileType FileType);
M3uFileInfo *M3uGetFileInfo( uint8 * FileName );

void M3uCreateGlobalNumList(HANDLE handle);
int16 M3uGetFileNumInFloderByShortName(UINT8 *shortName, UINT32 DirClus , FileType FileType);
int16 M3uTransCodeFromUTF8ToUnicode(UINT16 *longName);
int M3uGetUTF8Size(uint16 pInput);
 
/*
********************************************************************************
*
*                         End of file
*
********************************************************************************
*/
#endif

