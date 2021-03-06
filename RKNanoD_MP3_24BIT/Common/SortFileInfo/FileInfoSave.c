/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：  SearchAndSaveMusicInfo.C
*
* Description:
*
* History:      <author>          <time>        <version>
*               anzhiguo      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_FILEINFOSAVE_

#include "SysInclude.h"

#include "FsInclude.h"

#include "FileInfo.h"
#include "AddrSaveMacro.h"

#include "SysFindFile.h"
#include "AudioControl.h"
#include "id3.h"

#include "MDBBuildWin.h"

#ifdef _RK_CUE_
#include "cue.h"
#endif

#ifdef MEDIA_UPDATE

#define   ALL_FILENUM_DEFINE  (SORT_FILENUM_DEFINE * 4)
#define   RECORD_FILE_NUM 999
#define   TREE_BASIC_NODE_NUM_PRE_PAGE (MEDIAINFO_PAGE_SIZE / sizeof(FILE_TREE_BASIC))
#define   TREE_EXTEND_NODE_NUM_PRE_PAGE ((MEDIAINFO_PAGE_SIZE * 4) / sizeof(FILE_TREE_EXTEND))


typedef struct _MEDIA_FILE_SAVE_STRUCT
{
    UINT16  LongFileName[MEDIA_ID3_SAVE_CHAR_NUM]; //就是文件系统的长文件名
    UINT16  id3_title[MEDIA_ID3_SAVE_CHAR_NUM];
    UINT16  id3_singer[MEDIA_ID3_SAVE_CHAR_NUM];
    UINT16  id3_album[MEDIA_ID3_SAVE_CHAR_NUM];
    UINT16  Genre[MEDIA_ID3_SAVE_CHAR_NUM];
    UINT32  DirClus;
    UINT32  DirIndex;
    UINT16  TrackID[4];   ////Aaron.sun 2013807
    UINT32  FileOrDir;
#ifdef _RK_CUE_
     UINT32  StartTime;
     UINT32  EndTime;
#endif

}MEDIA_FILE_SAVE_STRUCT;

_FILE_INFO_SAVE_BSS_    FIND_DATA FindDataInfo;
_FILE_INFO_SAVE_BSS_    MEDIA_FILE_SAVE_STRUCT gFileSaveInfo; // 用于保存文件信息的结构体变量
_FILE_INFO_SAVE_BSS_    __align(4) UINT8    gFileInfoBasicBuffer[MEDIAINFO_PAGE_SIZE]; // 用于临时记录一个Page的文件信息
_FILE_INFO_SAVE_BSS_    __align(4) UINT8    gFileInfoExtendBuffer[MEDIAINFO_PAGE_SIZE]; // 用于临时记录一个Page的文件信息
_FILE_INFO_SAVE_BSS_    __align(4) UINT8    gFileTreeBasicBuffer[MEDIAINFO_PAGE_SIZE]; // 用于临时记录一个Page的文件信息
_FILE_INFO_SAVE_BSS_    __align(4) UINT32   gFileTreeExtendBuffer[MEDIAINFO_PAGE_SIZE]; // 用于临时记录一个Page的文件信息


/*
--------------------------------------------------------------------------------
  Function name : void FileSaveStructInit(FILE_SAVE_STRUCT  *pFileSaveTemp)
  Author        : anzhiguo
  Description   : initial the structure variable that save file infomation.

  Input         : pFileSaveTemp :structure variable to keeping file
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_FILE_INFO_SAVE_CODE_
void FileSaveStructInit(MEDIA_FILE_SAVE_STRUCT  *pFileSaveTemp)
{
    UINT32  i;

    for (i=0; i<MEDIA_ID3_SAVE_CHAR_NUM; i++)
    {
        pFileSaveTemp->id3_title[i] = 0;
        pFileSaveTemp->id3_singer[i] = 0;
        pFileSaveTemp->id3_album[i] = 0;
        pFileSaveTemp->Genre[i] = 0;
    }
}
/*
--------------------------------------------------------------------------------
  Function name : void PageWriteBufferInit(void)
  Author        : anzhiguo
  Description   : buffer initialization that be used to write flash page.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_FILE_INFO_SAVE_CODE_
void PageWriteBufferInit(void)
{

    memset(gFileInfoBasicBuffer,0,MEDIAINFO_PAGE_SIZE);
    memset(gFileInfoExtendBuffer,0,MEDIAINFO_PAGE_SIZE);
    memset(gFileTreeBasicBuffer,0,MEDIAINFO_PAGE_SIZE);
    memset(gFileTreeExtendBuffer,0XFF,MEDIAINFO_PAGE_SIZE);
}


/*
--------------------------------------------------------------------------------
  Function name : unsigned char SaveFileInfo(unsigned char *Buffer, FILE_SAVE_STRUCT  *pFileSaveTemp)
  Author        : anzhiguo
  Description   : wirte the file information that need to save to cache buffer,the information include
                  long file name,ID3Title,ID3singer,ID3Album,file path,short file name.

  Input         : Buffer：Buffer to save file information.
                  pFileSaveTemp：the information of current file.
  Return        : return 1

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_FILE_INFO_SAVE_CODE_
UINT8 SaveFileInfo(UINT8 *Buffer, MEDIA_FILE_SAVE_STRUCT  *pFileSaveTemp, UINT8 bSaveID3)
{
    UINT32 i;
    UINT8 *pBuffer,*pPath;

    pBuffer = Buffer + FILE_NAME_SAVE_ADDR_OFFSET;
    for (i=0;i<MEDIA_ID3_SAVE_CHAR_NUM;i++)//保存长文件名信息
    {

        *pBuffer++ = pFileSaveTemp->LongFileName[i] & 0xff;//低字节
        //printf("%02x ", *(pBuffer - 1));
        *pBuffer++ = ((pFileSaveTemp->LongFileName[i])>>8) & 0xff;//高字节

        if (pFileSaveTemp->LongFileName[i] == 0)
        {
            break;
        }
    }

    pBuffer = Buffer + DIR_CLUS_SAVE_ADDR_OFFSET;
    *((uint32 *)pBuffer) = pFileSaveTemp->DirClus;

    pBuffer = Buffer + DIR_INDEX_SAVE_ADDR_OFFSET;
    *((uint32 *)pBuffer) = pFileSaveTemp->DirIndex;

    pBuffer = Buffer + ATTR_SAVE_ADDR_OFFSET;
    if (pFileSaveTemp->FileOrDir)
    {
        *pBuffer++ = 0;
        *pBuffer++ = 'D';
        *pBuffer++ = 0;
        *pBuffer++ = 'I';
        *pBuffer++ = 0;
        *pBuffer++ = 'R';
        *pBuffer++ = 0;
        *pBuffer++ = 0;
    }
    else
    {
        *pBuffer++ = 0;
        *pBuffer++ = 'F';
        *pBuffer++ = 0;
        *pBuffer++ = 'I';
        *pBuffer++ = 0;
        *pBuffer++ = 'L';
        *pBuffer++ = 0;
        *pBuffer++ = 'E';
    }

#ifdef _RK_CUE_
    pBuffer = Buffer + CUE_START_SAVE_ADDR_OFFSET;
    *((uint32 *)pBuffer) = pFileSaveTemp->StartTime;

    pBuffer = Buffer + CUE_END_SAVE_ADDR_OFFSET;
    *((uint32 *)pBuffer) = pFileSaveTemp->EndTime;
#endif

    if (bSaveID3)
    {
        pBuffer = Buffer + ID3_TITLE_SAVE_ADDR_OFFSET;//长文件名保存占用的空间是CHAR_NUM_PER_FILE_NAME的2倍

        for (i = 0; i < MEDIA_ID3_SAVE_CHAR_NUM; i++)//保存id3的title信息
        {

            *pBuffer++ = (pFileSaveTemp->id3_title[i])&0xff;
            *pBuffer++ = (pFileSaveTemp->id3_title[i])>>8;
            if (pFileSaveTemp->id3_title[i] == 0)
            {
                break;
            }
        }

        pBuffer = Buffer + ID3_SINGLE_SAVE_ADDR_OFFSET;

        for (i=0;i < MEDIA_ID3_SAVE_CHAR_NUM ; i++)//保存id3的singer信息
        {

            *pBuffer++ = (pFileSaveTemp->id3_singer[i])&0xff;
            *pBuffer++ = (pFileSaveTemp->id3_singer[i])>>8;
            if (pFileSaveTemp->id3_singer[i] == 0)
            {
                break;
            }
        }

        pBuffer = Buffer + ID3_ALBUM_SAVE_ADDR_OFFSET;
        for (i=0;i<MEDIA_ID3_SAVE_CHAR_NUM;i++)//保存id3的album信息
        {

            *pBuffer++ = (pFileSaveTemp->id3_album[i])&0xff;
            *pBuffer++ = (pFileSaveTemp->id3_album[i])>>8;
            if (pFileSaveTemp->id3_album[i] == 0)
            {
                break;
            }
        }

        pBuffer = Buffer+ID3_GENRE_SAVE_ADDR_OFFSET;
        for (i=0;i<MEDIA_ID3_SAVE_CHAR_NUM;i++)//保存id3的album信息
        {

            *pBuffer++ = (pFileSaveTemp->Genre[i])&0xff;
            *pBuffer++ = (pFileSaveTemp->Genre[i])>>8;
            if (pFileSaveTemp->Genre[i] == 0)
            {
                break;
            }
        }

        pBuffer = Buffer + TRACKID_SAVE_ADDR_OFFSET;

        for (i = 0; i < 4; i++)
        {
            *(((uint16 *)pBuffer) + i) = pFileSaveTemp->TrackID[i]; //Aaron.sun 201387
        }

    }

    return 1;
}

/*
--------------------------------------------------------------------------------
  Function name : HANDLE FileOpenByFileFDT(FDT FileFDT, uint8 *Type)
  Author        : anzhiguo
  Description   : open the file by specified mode.

  Input         : FileFDT:FDF item of current file.
                  Type:open mode
  Return        : Not_Open_FILE:can not open

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
//it is the best to save this to file system,as it is still need in finding next file when playing.
_FILE_INFO_SAVE_CODE_
HANDLE FileOpenByFileFDT(FDT FileFDT, uint8 *Type)
{
    uint8 i;
    MY_FILE *fp;
    HANDLE Rt, OsRt;

    OsRt=NOT_OPEN_FILE;

    // 查找空闲文件登记项
    for (Rt = 0; Rt < MAX_OPEN_FILES; Rt++)
    {
        if (FileInfo[Rt].Flags == 0)
        {
            break;
        }
    }

    if (Rt < MAX_OPEN_FILES)
    {
        fp = FileInfo + Rt;

        for (i=0; i<11; i++)
        {
            fp->Name[i]=FileFDT.Name[i];
        }

        fp->Flags = FILE_FLAGS_READ;
        if (Type[0] == 'W' || Type[1] == 'W')
            fp->Flags |= FILE_FLAGS_WRITE;

        fp->FileSize = FileFDT.FileSize;
        fp->FstClus = FileFDT.FstClusLO | (uint32)FileFDT.FstClusHI << 16;
        fp->Clus = fp->FstClus;
        fp->Offset = 0;
        fp->RefClus = fp->Clus;
        fp->RefOffset = 0;
        OsRt=Rt;

    }

    return (OsRt);
}

#ifdef _RK_CUE_
_FILE_INFO_SAVE_BSS_ CUE_INFO CUEInfo;
#endif

extern  uint32   FlashSec[3];
/*
--------------------------------------------------------------------------------
  Function name : void SearchAndSaveMusicInfo(void)
  Author        : anzhiguo
  Description   : check all file information,and save it to specified flash,at the same time to remember
                  the information sort by file name to memory.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_FILE_INFO_SAVE_CODE_
void SearchAndSaveMusicInfo(void)
{
    UINT32  i, j, t;
    HANDLE hMusicFile;
    MEDIA_FILE_SAVE_STRUCT  *pFileSaveTemp = &gFileSaveInfo; // 用于保存文件信息

    UINT16 uiTotalFile = 0;  // 音乐文件个数计数
    UINT16 uiTotalSubDir = 0;
    UINT16 uiTotalMusic = 0;

    UINT16 uiFindFileResult = 0 ; // 文件检索结果
    UINT16 uiTotalFileInDir = 0;

    UINT16 uiFileInfoBasicIndex = 0;       // Flash Page控制计数     该值等于4时，已经将一个page大小的baffer写满，可以写flash了
    UINT16 uiFileInfoExtendIndex = 0;
    UINT16 uiFileTreeBasicIndex = 0;
    UINT16 uiFileTreeExtendIndex = 0;

    UINT32 ulFileInfoBasicSectorAddr = 0; // 保存文件详细信息的起始sector地址
    UINT32 ulFileInfoExtendSectorAddr = 0;
    UINT32 ulFileTreeBasicSectorAddr = 0;
    UINT32 ulFileTreeExtendSectorAddr = 0;

    FILE_TREE_BASIC * pFileTreeBasic = (FILE_TREE_BASIC *)gFileTreeBasicBuffer;
    //FILE_TREE_EXTEND * pFileTreeExtend = (FILE_TREE_EXTEND *)gFileTreeExtendBuffer;


    UINT16 uiHundredFlag=0;
    UINT16 uiTenFlag=0;
    UINT16 uiCountTemp=0;

    ID3V2X_INFO ID3Info;
    FIND_DATA    FindDataInfo_Local;
    FDT Fdt;

#ifdef _RK_CUE_
    int codec;
    uint32 Index, ret;
    char MusicFileExtString[] = "MP1MP2MP3WMAWAVAPEFLAAACM4AOGGMP43GPDFFDSFCUE";
#endif

    ModuleOverlay(MODULE_ID_AUDIO_ID3, MODULE_OVERLAY_ALL); //调用ID3 解析代码
//    ModuleOverlay(MODULE_ID_FILE_FIND, MODULE_OVERLAY_ALL);
    ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL); //调用写flash代码

    FileSaveStructInit(pFileSaveTemp); // 将用于记录文件保存信息的结构体清0
    PageWriteBufferInit();

    ulFileInfoBasicSectorAddr = MediaInfoAddr + MUSIC_SAVE_INFO_SECTOR_START; //保存详细的文件信息起始地址(sec值)
    ulFileInfoExtendSectorAddr = ulFileInfoBasicSectorAddr + MEDIAINFO_BLOCK_SIZE * 16;

    ulFileTreeBasicSectorAddr = MediaInfoAddr + MUSIC_TREE_INFO_SECTOR_START;
    ulFileTreeExtendSectorAddr = ulFileTreeBasicSectorAddr + MEDIAINFO_BLOCK_SIZE * 2;

    GotoRootDir(MusicFileExtString, FS_FAT_EX_VOICE);
    FindDataInfo_Local.Clus = CurDirClus; // 根目录簇号
    uiFindFileResult = FindFirst(&Fdt, &FindDataInfo_Local, MusicFileExtString, FS_FAT_EX_VOICE);
    uiTotalFileInDir = SubDirInfo[CurDirDeep].TotalFile + SubDirInfo[CurDirDeep].TotalSubDir;

    if (uiTotalFileInDir == 0)
    {
        gSysConfig.MedialibPara.gMusicFileNum = 0;
        gSysConfig.MedialibPara.gTotalFileNum = 0;
        return;
    }

    while (uiTotalFile <= ALL_FILENUM_DEFINE) //total 4 blocks ,media 1 block and no media 3 blocks
    {
#ifdef _WATCH_DOG_
        WatchDogReload();
#endif
        if (uiFindFileResult == RETURN_OK)
        {
            uiTotalFileInDir--;

#ifdef _RK_CUE_
            codec = GetFileType(&Fdt.Name[8],(UINT8 *)MusicFileExtString); //codec = 15: CUE
#endif

            if ((Fdt.Attr & ATTR_DIRECTORY))
            {
                //sub dir
                uiTotalFile++; //include all media file ,no media file and sub dir
                if (uiTotalFileInDir)
                {
                    pFileTreeBasic->dwNextBrotherID = uiTotalFile;
                }
                else
                {
                    pFileTreeBasic->dwNextBrotherID = 0xffffffff;
                }

                pFileTreeBasic->dwNodeFlag = MEDIA_FILE_TYPE_DIR;

                uiTotalSubDir++;
                pFileTreeBasic->dwExtendTreeID = uiTotalSubDir - 1;

                GetLongFileName(FindDataInfo_Local.Clus, FindDataInfo_Local.Index - 1, FS_FAT_EX_VOICE, pFileSaveTemp->LongFileName);

                pFileSaveTemp->DirClus = FindDataInfo_Local.Clus;
                pFileSaveTemp->DirIndex = FindDataInfo_Local.Index;
                pFileSaveTemp->FileOrDir = 1; //dir

                SaveFileInfo(&gFileInfoExtendBuffer[uiFileInfoExtendIndex * BYTE_NUM_SAVE_PER_FILE], pFileSaveTemp, 0);

                uiFileInfoExtendIndex++;
                pFileTreeBasic->dwBasicInfoID = uiTotalSubDir - 1 + (MEDIAINFO_BLOCK_SIZE * 16  * SECTOR_BYTE_SIZE)  / BYTE_NUM_SAVE_PER_FILE;

                if (uiFileInfoExtendIndex == FILE_SAVE_NUM_PER_PAGE)//gFileInfoBasicBuffer 已经存满信息，大小为8k ，就开始写flash
                {

                    uiFileInfoExtendIndex = 0;//文件序号回零

                    MDWrite(DataDiskID, ulFileInfoExtendSectorAddr, (MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE), gFileInfoExtendBuffer);//gPageTempBuffer);//

                    for (i = 0; i < MEDIAINFO_PAGE_SIZE; i++)//gFileInfoBasicBuffer 清零
                    {
                        gFileInfoExtendBuffer[i] = 0;
                    }
                    ulFileInfoExtendSectorAddr  +=  (MEDIAINFO_PAGE_SIZE / SECTOR_BYTE_SIZE);//指向下一个page，为下次写做准备
                }

                pFileTreeBasic++;

                if (uiTotalFile % TREE_BASIC_NODE_NUM_PRE_PAGE == 0)
                {
                    pFileTreeBasic = (FILE_TREE_BASIC *)gFileTreeBasicBuffer;

                    MDWrite(DataDiskID, ulFileTreeBasicSectorAddr, (MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE), gFileTreeBasicBuffer);

                    for (i = 0; i < MEDIAINFO_PAGE_SIZE; i++)//gFileInfoBasicBuffer 清零
                    {
                        gFileTreeBasicBuffer[i] = 0;
                    }
                    ulFileTreeBasicSectorAddr  +=  (MEDIAINFO_PAGE_SIZE / SECTOR_BYTE_SIZE);//指向下一个page，为下次写做准备
                }
            }
#ifdef _RK_CUE_
            else if(codec == 15)
            {
                memset(&CUEInfo, 0, sizeof(CUE_INFO));

                hMusicFile = FileOpenByFileFDT(Fdt, "R");
                if (hMusicFile != NOT_OPEN_FILE)    //打开文件并解析ID3信息
                {
                    ParseCueInfo(hMusicFile, &CUEInfo, 0);
                }
                FileClose(hMusicFile);
                hMusicFile = -1;

                ret = FindFDTInfoLong(&Fdt, FindDataInfo_Local.Clus, &Index, (uint16*)(CUEInfo.file));

                if(ret == RETURN_OK)
                {
                    pFileSaveTemp->DirIndex = Index + 1;
                    pFileSaveTemp->DirClus = FindDataInfo_Local.Clus;
                    pFileSaveTemp->FileOrDir = 0;

                    for (i = 0; i < MEDIA_ID3_SAVE_CHAR_NUM; i++)
                    {
                        pFileSaveTemp->LongFileName[i] = CUEInfo.file[i];
                    }

                    for (i = 0; i < CUEInfo.m_total_Song; i++)
                    {
                        uiTotalFile++; //include all media file ,no media file and sub dir
                        if (uiTotalMusic == (SORT_FILENUM_DEFINE - 1))
                        {
                            pFileTreeBasic->dwNextBrotherID = 0xffffffff;
                        }
                        else if(uiTotalFileInDir || (i != (CUEInfo.m_total_Song - 1)))
                        {
                            pFileTreeBasic->dwNextBrotherID = uiTotalFile;
                        }
                        else
                        {
                            pFileTreeBasic->dwNextBrotherID = 0xffffffff;
                        }

                        pFileTreeBasic->dwNodeFlag = MEDIA_FILE_TYPE_CUE;
                        pFileTreeBasic->dwExtendTreeID = 0xffffffff;

                        //sort meida file
                        for (j = 0; j < MEDIA_ID3_SAVE_CHAR_NUM; j++)
                        {
                            pFileSaveTemp->id3_title[j] = CUEInfo.m_songs[i].title[j];
                            pFileSaveTemp->id3_singer[j] = CUEInfo.m_songs[i].performer[j];
                            pFileSaveTemp->id3_album[j] =  CUEInfo.album[j];  //zyz update
                        }

                        for(j = 0; j < 4; j++)
                        {
                            pFileSaveTemp->TrackID[j] =  CUEInfo.m_songs[i].trackId[j];
                        }

                        pFileSaveTemp->StartTime = CUEInfo.m_songs[i].start;
                        pFileSaveTemp->EndTime = CUEInfo.m_songs[i].end;

                        SaveFileInfo(&gFileInfoBasicBuffer[uiFileInfoBasicIndex * BYTE_NUM_SAVE_PER_FILE], pFileSaveTemp, 1); // 保存文件信息及排序索引信息 把获取的当前文件的信息存于全局变量gPageWriteBuffer的某个位置

                        uiTotalMusic++;  // all sort media file
                        uiFileInfoBasicIndex++;
                        pFileTreeBasic->dwBasicInfoID = uiTotalMusic - 1;

                        if (uiFileInfoBasicIndex == FILE_SAVE_NUM_PER_PAGE)//gFileInfoBasicBuffer 已经存满信息，大小为8k ，就开始写flash
                        {
                            uiFileInfoBasicIndex = 0;//文件序号回零

                            MDWrite(DataDiskID, ulFileInfoBasicSectorAddr, (MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE), gFileInfoBasicBuffer);//gPageTempBuffer);//

                            for (t = 0; t < MEDIAINFO_PAGE_SIZE; t++)//gFileInfoBasicBuffer 清零
                            {
                                gFileInfoBasicBuffer[t] = 0;
                            }
                            ulFileInfoBasicSectorAddr  +=  (MEDIAINFO_PAGE_SIZE / SECTOR_BYTE_SIZE);//指向下一个page，为下次写做准备
                        }

                        pFileTreeBasic++;

                        if(uiTotalFile % TREE_BASIC_NODE_NUM_PRE_PAGE == 0)
                        {
                            pFileTreeBasic = (FILE_TREE_BASIC *)gFileTreeBasicBuffer;

                            MDWrite(DataDiskID, ulFileTreeBasicSectorAddr, (MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE), gFileTreeBasicBuffer);

                            for (t = 0; t < MEDIAINFO_PAGE_SIZE; t++)//gFileInfoBasicBuffer 清零
                            {
                                gFileTreeBasicBuffer[t] = 0;
                            }
                            ulFileTreeBasicSectorAddr  +=  (MEDIAINFO_PAGE_SIZE / SECTOR_BYTE_SIZE);//指向下一个page，为下次写做准备
                        }
                    }
                }
            }
#endif
            else
            {
                //sort meida file
                uiTotalFile++; //include all media file ,no media file and sub dir

                if (uiTotalMusic == (SORT_FILENUM_DEFINE - 1))
                {
                    //printf("All music full\n");
                    pFileTreeBasic->dwNextBrotherID = 0xffffffff;
                }
                else if (uiTotalFileInDir)
                {
                    pFileTreeBasic->dwNextBrotherID = uiTotalFile;
                }
                else
                {
                    pFileTreeBasic->dwNextBrotherID = 0xffffffff;
                }

                pFileTreeBasic->dwNodeFlag = MEDIA_FILE_TYPE_FILE;
                pFileTreeBasic->dwExtendTreeID = 0xffffffff;

                hMusicFile = FileOpenByFileFDT(Fdt, "R"); // 改写的FileOpen函数搜索速度大为提高

                if (hMusicFile != NOT_OPEN_FILE)//成功打开文件//打开文件并解析ID3信息
                {
                    memset((UINT8*)&ID3Info, 0, sizeof(ID3V2X_INFO));

#ifdef _RK_ID3_
                    GetAudioId3Info(hMusicFile , &ID3Info, &Fdt.Name[8]);
#endif
                }//打开文件并解析ID3信息

                FileClose(hMusicFile);
                hMusicFile = -1;
                GetLongFileName(FindDataInfo_Local.Clus, FindDataInfo_Local.Index - 1, FS_FAT_EX_VOICE, pFileSaveTemp->LongFileName); // 获取保存的长文件名信息

                for (i=0; i<MEDIA_ID3_SAVE_CHAR_NUM; i++)
                {
                    pFileSaveTemp->id3_title[i] = ID3Info.id3_title[i]; // 保存歌手信息
                    pFileSaveTemp->id3_singer[i] = ID3Info.id3_singer[i]; // 保存歌手信息
                    pFileSaveTemp->id3_album[i] = ID3Info.id3_album[i];  // 保存专辑信息
                    pFileSaveTemp->Genre[i] = ID3Info.id3_genre[i];// 保存专辑信息
                }

                for (i = 0; i < 4; i++)
                {
                    pFileSaveTemp->TrackID[i] = ID3Info.nTrack[i]; //Aaron.sun 2013807
                }

                pFileSaveTemp->DirClus = FindDataInfo_Local.Clus;
                pFileSaveTemp->DirIndex = FindDataInfo_Local.Index;
                pFileSaveTemp->FileOrDir = 0; //dir

#if 0   //if all song show long file name ,pls open this.

                memcpy(pFileSaveTemp->id3_title,pFileSaveTemp->LongFileName,MEDIA_ID3_SAVE_CHAR_NUM*2);
#else
                if (pFileSaveTemp->id3_title[0] == 0)
                {
                    memcpy(pFileSaveTemp->id3_title,pFileSaveTemp->LongFileName,MEDIA_ID3_SAVE_CHAR_NUM*2);
                }
#endif
#ifdef _RK_CUE_
                pFileSaveTemp->StartTime = 0;
                pFileSaveTemp->EndTime = 0;
#endif

                if (pFileSaveTemp->TrackID[0] == 0x0000)
                {
                    pFileSaveTemp->TrackID[0] = 0x30;
                    pFileSaveTemp->TrackID[1] = 0x30;
                    pFileSaveTemp->TrackID[2] = 0x30;
                    pFileSaveTemp->TrackID[3] = 0x30;
                }
                else
                {
                    if (pFileSaveTemp->TrackID[3] == 0X39)
                    {
                        pFileSaveTemp->TrackID[3] = 0x30;
                        if (pFileSaveTemp->TrackID[2] == 0X39)
                        {
                            pFileSaveTemp->TrackID[2] = 0x30;
                            if (pFileSaveTemp->TrackID[1] == 0X39)
                            {
                                pFileSaveTemp->TrackID[1] = 0x30;
                                if (pFileSaveTemp->TrackID[0] == 0X39)
                                {
                                    pFileSaveTemp->TrackID[0] = 0x39;
                                    pFileSaveTemp->TrackID[1] = 0x39;
                                    pFileSaveTemp->TrackID[2] = 0x39;
                                    pFileSaveTemp->TrackID[3] = 0x39;
                                }
                                else
                                {
                                    pFileSaveTemp->TrackID[0]++;
                                }
                            }
                            else
                            {
                                pFileSaveTemp->TrackID[1]++;
                            }
                        }
                        else
                        {
                            pFileSaveTemp->TrackID[2]++;
                        }

                    }
                    else
                    {
                        pFileSaveTemp->TrackID[3]++;
                    }
                }

                SaveFileInfo(&gFileInfoBasicBuffer[uiFileInfoBasicIndex * BYTE_NUM_SAVE_PER_FILE], pFileSaveTemp, 1); // 保存文件信息及排序索引信息 把获取的当前文件的信息存于全局变量gPageWriteBuffer的某个位置
                uiTotalMusic++;  // all sort media file
                uiFileInfoBasicIndex++;
                pFileTreeBasic->dwBasicInfoID = uiTotalMusic - 1;


                if (uiFileInfoBasicIndex == FILE_SAVE_NUM_PER_PAGE)//gFileInfoBasicBuffer 已经存满信息，大小为8k ，就开始写flash
                {
                    uiFileInfoBasicIndex = 0;//文件序号回零

                    MDWrite(DataDiskID, ulFileInfoBasicSectorAddr, (MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE), gFileInfoBasicBuffer);//gPageTempBuffer);//

                    for (i = 0; i < MEDIAINFO_PAGE_SIZE; i++)//gFileInfoBasicBuffer 清零
                    {
                        gFileInfoBasicBuffer[i] = 0;
                    }
                    ulFileInfoBasicSectorAddr  +=  (MEDIAINFO_PAGE_SIZE / SECTOR_BYTE_SIZE);//指向下一个page，为下次写做准备
                }

                pFileTreeBasic++;

                if (uiTotalFile % TREE_BASIC_NODE_NUM_PRE_PAGE == 0)
                {
                    pFileTreeBasic = (FILE_TREE_BASIC *)gFileTreeBasicBuffer;

                    MDWrite(DataDiskID, ulFileTreeBasicSectorAddr, (MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE), gFileTreeBasicBuffer);

                    for (i = 0; i < MEDIAINFO_PAGE_SIZE; i++)//gFileInfoBasicBuffer 清零
                    {
                        gFileTreeBasicBuffer[i] = 0;
                    }
                    ulFileTreeBasicSectorAddr  +=  (MEDIAINFO_PAGE_SIZE / SECTOR_BYTE_SIZE);//指向下一个page，为下次写做准备
                }
            }

            uiCountTemp = uiTotalFile;

            if ( uiHundredFlag == (uiCountTemp / 20))  //ui显示case ，每找到20个文件刷一张图片
            {
                //MedialibUpdataDisplay(uiTenFlag);
                ++uiTenFlag;

                if (uiTenFlag == 3)
                {
                    uiTenFlag = 0;
                }
                ++uiHundredFlag;
            }

            if (uiTotalMusic >= SORT_FILENUM_DEFINE)
            {
                //printf("exit\n");
                break;
            }

            uiFindFileResult = FindNext(&Fdt, &FindDataInfo_Local, MusicFileExtString, FS_FAT_EX_VOICE);
        }
        else
        {
            do
            {
                GotoNextDir(MusicFileExtString, FS_FAT_EX_VOICE);       //遍历下一个目录，找完子目录再找同级目录

                if ((SubDirInfo[CurDirDeep].TotalFile + SubDirInfo[CurDirDeep].TotalSubDir) == 0)
                {
                    gFileTreeExtendBuffer[SubDirInfo[CurDirDeep - 1].DirNum + SubDirInfo[CurDirDeep - 1].CurDirNum - 1] = 0xffffffff;
                }
                else
                {
                    break;
                }

            }
            while (CurDirDeep != 0);

            if (CurDirDeep == 0)
            {
                break;
            }

            gFileTreeExtendBuffer[SubDirInfo[CurDirDeep - 1].DirNum + SubDirInfo[CurDirDeep - 1].CurDirNum - 1] = uiTotalFile;

            FindDataInfo_Local.Clus = CurDirClus; // 当前目录首簇号
            uiFindFileResult = FindFirst(&Fdt, &FindDataInfo_Local, MusicFileExtString, FS_FAT_EX_VOICE);
            uiTotalFileInDir = SubDirInfo[CurDirDeep].TotalFile + SubDirInfo[CurDirDeep].TotalSubDir;
        }
    }

    if (uiFileInfoBasicIndex) // 保存不足2K的文件信息
    {
        MDWrite(DataDiskID, ulFileInfoBasicSectorAddr, (MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE), gFileInfoBasicBuffer);//gPageTempBuffer);//
    }

    if (uiFileInfoExtendIndex) // 保存不足2K的文件信息
    {
        MDWrite(DataDiskID, ulFileInfoExtendSectorAddr, (MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE), gFileInfoExtendBuffer);//gPageTempBuffer);//
    }

    if (uiTotalFile % TREE_BASIC_NODE_NUM_PRE_PAGE != 0)
    {
        MDWrite(DataDiskID, ulFileTreeBasicSectorAddr, (MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE), gFileTreeBasicBuffer);
    }

    if (uiTotalSubDir)
    {
        MDWrite(DataDiskID, ulFileTreeExtendSectorAddr, (MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE) * 4, gFileTreeExtendBuffer);
    }

    gSysConfig.MedialibPara.gTotalFileNum = uiTotalFile;
    gSysConfig.MedialibPara.gMusicFileNum = uiTotalMusic; // 得到系统全部文件数目，最大为SORT_FILENUM_DEFINE个

    FlashSec[0] = 0xffffffff;
    FlashSec[1] = 0xffffffff;
    FlashSec[2] = 0xffffffff;
}

#ifdef _RECORD_
/*
--------------------------------------------------------------------------------
  Function name : void SearchAndSaveMusicInfo(void)
  Author        : anzhiguo
  Description   : check all file information,and save it to specified flash,at the same time to remember
                  the information sort by file name to memory.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_FILE_INFO_SAVE_CODE_
void SearchAndSaveRecordFmInfo(void)
{
    UINT32  i;
    HANDLE hMusicFile;
    MEDIA_FILE_SAVE_STRUCT  *pFileSaveTemp = &gFileSaveInfo; // 用于保存文件信息

    UINT16 uiTotalSubDir = 0;
    UINT16 uiTotalRecord = 0;

    UINT16 uiFindFileResult = 0 ; // 文件检索结果
    UINT16 uiTotalFileInDir = 0;

    UINT16 uiFileInfoBasicIndex = 0;       // Flash Page控制计数     该值等于4时，已经将一个page大小的baffer写满，可以写flash了
    UINT16 uiFileInfoExtendIndex = 0;
    UINT16 uiFileTreeBasicIndex = 0;
    UINT16 uiFileTreeExtendIndex = 0;

    UINT32 ulFileInfoBasicSectorAddr = 0; // 保存文件详细信息的起始sector地址
    UINT32 ulFileInfoExtendSectorAddr = 0;
    UINT32 ulFileTreeBasicSectorAddr = 0;
    UINT32 ulFileTreeExtendSectorAddr = 0;

    FILE_TREE_BASIC * pFileTreeBasic = (FILE_TREE_BASIC *)gFileTreeBasicBuffer;

    UINT16 uiHundredFlag=0;
    UINT16 uiTenFlag=0;
    UINT16 uiCountTemp=0;

    ID3V2X_INFO ID3Info;
    FIND_DATA	 FindDataInfo_Local;
    FDT Fdt;

    ModuleOverlay(MODULE_ID_AUDIO_ID3, MODULE_OVERLAY_ALL); //调用ID3 解析代码
    ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL); //调用写flash代码

    FileSaveStructInit(pFileSaveTemp); // 将用于记录文件保存信息的结构体清0
    PageWriteBufferInit();

    ulFileInfoBasicSectorAddr = MediaInfoAddr + RECORD_SAVE_INFO_SECTOR_START; //保存详细的文件信息起始地址(sec值)
    ulFileInfoExtendSectorAddr = ulFileInfoBasicSectorAddr + MEDIAINFO_BLOCK_SIZE * 16;

    ulFileTreeBasicSectorAddr = MediaInfoAddr + RECORD_TREE_INFO_SECTOR_START;
    ulFileTreeExtendSectorAddr = ulFileTreeBasicSectorAddr + MEDIAINFO_BLOCK_SIZE * 6;

    gwSaveDirClus = GetDirClusIndex("U:\\RECORD     \\FM");
    GotoCurDir(RecordFileExtString, FS_FAT);
    FindDataInfo_Local.Clus = CurDirClus; // 根目录簇号
    uiFindFileResult = FindFirstFile(&Fdt, &FindDataInfo_Local, RecordFileExtString, FS_FAT);
    uiTotalFileInDir = SubDirInfo[CurDirDeep].TotalFile;
    //printf("root:uiTotalFileInDir = %d\n", uiTotalFileInDir);

    if (uiTotalFileInDir == 0)
    {
        gSysConfig.MedialibPara.gRecordFmFileNum = 0;
        return;
    }

    while (uiTotalRecord < RECORD_FILE_NUM) //total 4 blocks ,media 1 block and no media 3 blocks
    {
#ifdef _WATCH_DOG_
        WatchDogReload();
#endif
        if (uiFindFileResult == RETURN_OK)
        {
            uiTotalFileInDir--;

            uiTotalRecord++;

            if (uiTotalFileInDir && (uiTotalRecord < 999))
            {
                pFileTreeBasic->dwNextBrotherID = uiTotalRecord;
            }
            else
            {
                pFileTreeBasic->dwNextBrotherID = 0xffffffff;
            }

            pFileTreeBasic->dwNodeFlag = MEDIA_FILE_TYPE_FILE;
            pFileTreeBasic->dwExtendTreeID = 0xffffffff;

            GetLongFileName(FindDataInfo_Local.Clus, FindDataInfo_Local.Index - 1, FS_FAT,pFileSaveTemp->LongFileName);
            pFileSaveTemp->DirClus = FindDataInfo_Local.Clus;
            pFileSaveTemp->DirIndex = FindDataInfo_Local.Index;
            pFileSaveTemp->FileOrDir = 0; //dir

            SaveFileInfo(&gFileInfoExtendBuffer[uiFileInfoExtendIndex * BYTE_NUM_SAVE_PER_FILE], pFileSaveTemp, 0);

            uiFileInfoExtendIndex++;
            pFileTreeBasic->dwBasicInfoID = uiTotalRecord - 1 + (MEDIAINFO_BLOCK_SIZE * 16  * SECTOR_BYTE_SIZE)  / BYTE_NUM_SAVE_PER_FILE;

            if (uiFileInfoExtendIndex == FILE_SAVE_NUM_PER_PAGE)//gFileInfoBasicBuffer 已经存满信息，大小为8k ，就开始写flash
            {
                uiFileInfoExtendIndex = 0;//文件序号回零

                MDWrite(DataDiskID, ulFileInfoExtendSectorAddr, (MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE), gFileInfoExtendBuffer);//gPageTempBuffer);//

                for (i = 0; i < MEDIAINFO_PAGE_SIZE; i++)//gFileInfoBasicBuffer 清零
                {
                    gFileInfoExtendBuffer[i] = 0;
                }
                ulFileInfoExtendSectorAddr  +=  (MEDIAINFO_PAGE_SIZE / SECTOR_BYTE_SIZE);//指向下一个page，为下次写做准备
            }


            pFileTreeBasic++;

            if (uiTotalRecord % TREE_BASIC_NODE_NUM_PRE_PAGE == 0)
            {
                pFileTreeBasic = (FILE_TREE_BASIC *)gFileTreeBasicBuffer;

                MDWrite(DataDiskID, ulFileTreeBasicSectorAddr, (MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE), gFileTreeBasicBuffer);

                for (i = 0; i < MEDIAINFO_PAGE_SIZE; i++)//gFileInfoBasicBuffer 清零
                {
                    gFileTreeBasicBuffer[i] = 0;
                }
                ulFileTreeBasicSectorAddr  +=  (MEDIAINFO_PAGE_SIZE / SECTOR_BYTE_SIZE);//指向下一个page，为下次写做准备

            }


            uiCountTemp = uiTotalRecord;

            if ( uiHundredFlag == (uiCountTemp / 20))  //ui显示case ，每找到20个文件刷一张图片
            {
                ++uiTenFlag;

                if (uiTenFlag == 3)
                {
                    uiTenFlag = 0;
                }
                ++uiHundredFlag;
            }

            uiFindFileResult = FindNextFile(&Fdt, &FindDataInfo_Local, RecordFileExtString, FS_FAT);
        }
        else
        {
            break;
        }

    }

    if (uiFileInfoBasicIndex) // 保存不足2K的文件信息
    {
        MDWrite(DataDiskID, ulFileInfoBasicSectorAddr, (MEDIAINFO_PAGE_SIZE/512), gFileInfoBasicBuffer);//gPageTempBuffer);//
    }

    if (uiFileInfoExtendIndex) // 保存不足2K的文件信息
    {
        MDWrite(DataDiskID, ulFileInfoExtendSectorAddr, (MEDIAINFO_PAGE_SIZE/512), gFileInfoExtendBuffer);//gPageTempBuffer);//
    }

    if (uiTotalRecord % TREE_BASIC_NODE_NUM_PRE_PAGE != 0)
    {
        MDWrite(DataDiskID, ulFileTreeBasicSectorAddr, (MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE), gFileTreeBasicBuffer);
    }

    if (uiTotalSubDir)
    {
        MDWrite(DataDiskID, ulFileTreeExtendSectorAddr, (MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE) * 4, gFileTreeExtendBuffer);
    }
    gSysConfig.MedialibPara.gRecordFmFileNum = uiTotalRecord;

    FlashSec[0] = 0xffffffff;
    FlashSec[1] = 0xffffffff;
    FlashSec[2] = 0xffffffff;

}
#endif

#endif

