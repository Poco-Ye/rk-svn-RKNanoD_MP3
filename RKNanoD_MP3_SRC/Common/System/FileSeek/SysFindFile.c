


#define  IN_SYS_FINDFILE

#include "SysInclude.h"
#include "FsInclude.h"
#include "SysFindFile.h"
#include "AudioControl.h"
#include "myRandom.h"

#include "MediaBroWin.h"
#include "medialibwin.h"
#include "AddrSaveMacro.h"

/*
--------------------------------------------------------------------------------
  Function name :  UINT16 SysCheckTotalFileNum(uint8 *pExtStr)
  Author        :  zs
  Description   :  check there is the type file in media
  Input         :  uint8 *pExtStr: file name
  Return        :  temp: file total.
  History       :  <author>         <time>         <version>
                     zs            2009/02/27         Ver1.0
  desc          : org
--------------------------------------------------------------------------------
_ATTR_SYS_FINDFILE_TEXT_
UINT16 SysCheckTotalFileNum(uint8 *pExtStr)
{
    UINT16 temp = 0;

    temp = dglBuildDirInfo(pExtStr);
    return(temp);
}

*/

/*
-----------------------------------------------------------------------------------------------
  Function name :  INT16 SysFindFileInit(SYS_FILE_INFO *pSysFileInfo, UINT16
                       GlobalFileNum,UINT16 FindFileRange,UINT16 FindFileMode,uint8 *pExtStr)
  Author        :  zs
  Description   :  initial the search variables when enter module.
  Input         :  SYS_FILE_INFO *pSysFileInfo : return file information.
                   UINT16        GlobalFileNum : Input: global file number.
                   UINT16        FindFileRange : Input: the range to find file
                   UINT16        FindFileMode  : Input: random mode
                   uint8         *pExtStr      : Input: file type

  Return        :  return value:0
  History       :  <author>         <time>         <version>
                     zs            2009/02/27         Ver1.0
  desc          : org
-------------------------------------------------------------------------------------------------
*/
_ATTR_SYS_FINDFILE_TEXT_
INT16 SysFindFileInit(SYS_FILE_INFO *pSysFileInfo,UINT16 GlobalFileNum,UINT16 FindFileRange,UINT16 PlayMode, uint8 *pExtStr)
{
    UINT16 i,j;
    UINT16 tempFileNum;
    INT16  FindFileResult;
    INT16  RetVal = 0;
    FS_TYPE FsType;

    if(pSysFileInfo->ucSelPlayType == SORT_TYPE_SEL_FOLDER)
    {
        FsType = MUSIC_DB;
    }
#ifdef _RECORD_
    else if(pSysFileInfo->ucSelPlayType == MUSIC_TYPE_SEL_FMFILE)
    {
        FsType = RECORD_DB;
    }
#endif
    else
    {
        FsType = FS_FAT;
    }

    pSysFileInfo->TotalFiles = BuildDirInfo(pExtStr, FsType);
#ifdef _M3U_
    if(pSysFileInfo->ucSelPlayType == SORT_TYPE_SEL_M3U_BROWSER)
    {
        pSysFileInfo->TotalFiles = pSysFileInfo->M3uGlobalFileCnt;
    }
#endif
    if (0 == pSysFileInfo->TotalFiles)
    {
        return -1;
    }

    pSysFileInfo->DiskTotalFiles = pSysFileInfo->TotalFiles;
    pSysFileInfo->CurrentFileNum = GlobalFileNum;
    pSysFileInfo->pExtStr        = pExtStr;
    pSysFileInfo->Range          = FindFileRange;
    pSysFileInfo->PlayOrder      = PlayMode;


    if (pSysFileInfo->CurrentFileNum > pSysFileInfo->TotalFiles)
    {
        pSysFileInfo->CurrentFileNum = 1;
    }

    #ifdef _M3U_
    if (AudioFileInfo.ucSelPlayType == SORT_TYPE_SEL_M3U_BROWSER)
    {
        tempFileNum = GetCurFileNum(pSysFileInfo->M3uGlobalFileNumBuf[pSysFileInfo->CurrentFileNum - 1], &pSysFileInfo->FindData, pExtStr, FsType);
    }
    else
    #endif
    {
        tempFileNum = GetCurFileNum(pSysFileInfo->CurrentFileNum,&pSysFileInfo->FindData, pExtStr, FsType);
    }


    FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pExtStr, FsType);

    for(i = 1; i < tempFileNum; i++)
    {
        FindFileResult = FindNextFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pExtStr, FsType);
    }

    if (pSysFileInfo->Range == FIND_FILE_RANGE_DIR)
    {
        pSysFileInfo->CurrentFileNum = tempFileNum;
        pSysFileInfo->TotalFiles = GetTotalFiles((pSysFileInfo->FindData).Clus,pExtStr, FsType);
    }

    pSysFileInfo->PlayedFileNum = pSysFileInfo->CurrentFileNum;

    return 0;
}


/*
--------------------------------------------------------------------------------
  Function name : INT16 SysFindFileExt(SYS_FILE_INFO *pSysFileInfo,INT16 Offset)
  Author        :  zs
  Description   :  search file.
  Input         :  SYS_FILE_INFO *pSysFileInfo:structure to find file.
                   Offset == 0   find current file.
                   Offset > 0    find backward file,the offset is the offset file number relatived to the current file.
                   Offset < 0    find forward file,the offset is the offset file number relatived to the current file.

  Return        :
  History       :  <author>         <time>         <version>
                     zs            2009/02/27         Ver1.0
  desc          : org
--------------------------------------------------------------------------------
*/

_ATTR_SYS_FINDFILE_TEXT_
INT16 SysFindFileExt(SYS_FILE_INFO *pSysFileInfo,INT16 Offset)
{
    UINT16  i, tempFileNum;

    UINT16  uiNeedFindNext = 1;
    INT16   FindFileResult = -1;
    UINT16  seed;

    if(Offset == 0)
    {
        return (RETURN_OK);
    }

    //根据播放顺序和播放模式(目录还是全部)调整当前播放文件的文件号
    if(pSysFileInfo->PlayOrder == AUDIO_RAND) //随机播放   azg 8.18
    {
//        if(GetMsg(MSG_MEDIA_BREAKPOINT_PLAY)==FALSE)

        seed =(SysTickCounter % AudioFileInfo.TotalFiles);

        pSysFileInfo->CurrentFileNum = randomGenerator(Offset, seed) + 1;

        if(pSysFileInfo->CurrentFileNum > pSysFileInfo->TotalFiles)
        {
            pSysFileInfo->CurrentFileNum = 1;
        }
        pSysFileInfo->PlayedFileNum = pSysFileInfo->CurrentFileNum;
    }
    else //顺序播放
    {
        if(Offset > 0) //下一曲
        {
            pSysFileInfo->CurrentFileNum ++ ;

            if (pSysFileInfo->CurrentFileNum > pSysFileInfo->TotalFiles)
            {
                pSysFileInfo->CurrentFileNum = 1;

                if(pSysFileInfo->ucSelPlayType == SORT_TYPE_SEL_BROWSER)// || pSysFileInfo->ucSelPlayType == SORT_TYPE_SEL_M3U_BROWSER)
                {
                    if(pSysFileInfo->Range == FIND_FILE_RANGE_DIR)//目录内循环  //文件查找范围是一个文件夹
                    {
                        //播完最后一曲ˇ返回从头开始找目录中的第一首歌
                        FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, FS_FAT);
                        uiNeedFindNext = 0;
                    }
                    else
                    {
                        tempFileNum = GetCurFileNum(pSysFileInfo->CurrentFileNum,&pSysFileInfo->FindData,pSysFileInfo->pExtStr, FS_FAT);

                        FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, FS_FAT);
                        uiNeedFindNext = FindFileResult;
                    }
                }
                else if(pSysFileInfo->ucSelPlayType == SORT_TYPE_SEL_FOLDER)
                {
                    if(pSysFileInfo->Range == FIND_FILE_RANGE_DIR)//目录内循环  //文件查找范围是一个文件夹
                    {
                        //播完最后一曲ˇ返回从头开始找目录中的第一首歌
                        FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, MUSIC_DB);
                        uiNeedFindNext = 0;
                    }
                    else
                    {
                        tempFileNum = GetCurFileNum(pSysFileInfo->CurrentFileNum,&pSysFileInfo->FindData,pSysFileInfo->pExtStr, MUSIC_DB);

                        FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, MUSIC_DB);
                        uiNeedFindNext = FindFileResult;
                    }
                }
                #ifdef _RECORD_
                else if(pSysFileInfo->ucSelPlayType == MUSIC_TYPE_SEL_RECORDFILE)//媒体库中的录音也需要指回第一个文件
                {
                    FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, RECORD_DB);
                    uiNeedFindNext = 0;
                }
                else if(pSysFileInfo->ucSelPlayType == MUSIC_TYPE_SEL_FMFILE)
                {

                    FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, RECORD_DB);
                    uiNeedFindNext = 0;

                }
                #endif

                #ifdef _M3U_
                else if(pSysFileInfo->ucSelPlayType == SORT_TYPE_SEL_M3U_BROWSER)
                {
                    //tempFileNum = GetCurFileNum(pSysFileInfo->M3uGlobalFileNumBuf[pSysFileInfo->CurrentFileNum-1],&pSysFileInfo->FindData,pSysFileInfo->pExtStr, FS_FAT);
                    //FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, FS_FAT);
                    //uiNeedFindNext = FindFileResult;
                }
                #endif
            }

        }
        else if (Offset < 0) //上一曲
        {

            pSysFileInfo->CurrentFileNum -- ;

            if (pSysFileInfo->CurrentFileNum == 0)
            {
                pSysFileInfo->CurrentFileNum = pSysFileInfo->TotalFiles;
            }
        }
        pSysFileInfo->PlayedFileNum = pSysFileInfo->CurrentFileNum;
    }

    #ifdef _MEDIA_MODULE_
    pSysFileInfo->uiCurId[pSysFileInfo->ucCurDeep]= pSysFileInfo->CurrentFileNum - 1;//7.3 azg add
    #endif

    //根据文件号开始找文件ˇ获取路径和短文件名信息ˇ为下面的打开文件做准备
    if(pSysFileInfo->ucSelPlayType == SORT_TYPE_SEL_BROWSER)
    {
        if((Offset < 0) || (pSysFileInfo->PlayOrder == AUDIO_RAND))
        {

            tempFileNum = pSysFileInfo->CurrentFileNum;

            if (pSysFileInfo->Range != FIND_FILE_RANGE_DIR)//目录内循环
            {
                tempFileNum = GetCurFileNum(pSysFileInfo->CurrentFileNum, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, FS_FAT);
            }

            FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, FS_FAT);

            for (i = 1; i < tempFileNum; i++)
            {
                FindFileResult = FindNextFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, FS_FAT);
            }
            uiNeedFindNext = 0;
        }
        else if ((uiNeedFindNext) && (Offset > 0))
        {
            FindFileResult = FindNextFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, FS_FAT);

            if (FindFileResult == NOT_FIND_FILE) /* 没有发ˇ指定文件*/
            {
                tempFileNum = GetCurFileNum(pSysFileInfo->CurrentFileNum, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, FS_FAT);
                FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, FS_FAT);
            }
        }
    }
    else if(pSysFileInfo->ucSelPlayType == SORT_TYPE_SEL_FOLDER)
    {
        if((Offset < 0) || (pSysFileInfo->PlayOrder == AUDIO_RAND))
        {

            tempFileNum = pSysFileInfo->CurrentFileNum;

            if (pSysFileInfo->Range != FIND_FILE_RANGE_DIR)//目录内循环
            {
                tempFileNum = GetCurFileNum(pSysFileInfo->CurrentFileNum, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, MUSIC_DB);
            }

            FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, MUSIC_DB);

            for (i = 1; i < tempFileNum; i++)
            {
                FindFileResult = FindNextFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, MUSIC_DB);
            }
            uiNeedFindNext = 0;
        }
        else if ((uiNeedFindNext) && (Offset > 0))
        {
            FindFileResult = FindNextFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, MUSIC_DB);

            if (FindFileResult == NOT_FIND_FILE) /* 没有发ˇ指定文件*/
            {
                tempFileNum = GetCurFileNum(pSysFileInfo->CurrentFileNum, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, MUSIC_DB);
                FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, MUSIC_DB);
            }
        }
    }

    #ifdef _RECORD_
    // ----解决录音文件夹内文件ˇ播放前一首歌的问题
    else if(pSysFileInfo->ucSelPlayType == MUSIC_TYPE_SEL_RECORDFILE)
    {
        #if 0
        if(pSysFileInfo->PlayOrder == AUDIO_RAND || Offset < 0)
        {
            tempFileNum = pSysFileInfo->CurrentFileNum;

            FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData,  pSysFileInfo->pExtStr, FS_FAT);
            for (i=1; i<tempFileNum; i++)
            {
                FindFileResult = FindNextFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData,  pSysFileInfo->pExtStr, FS_FAT);
            }
            uiNeedFindNext = 0;

        }
        else if ((uiNeedFindNext) && (Offset > 0))
        {
          FindFileResult = FindNextFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, FS_FAT);
          if (FindFileResult == NOT_FIND_FILE)
          {
             FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, FS_FAT);
          }
        }
        #else
        if((pSysFileInfo->PlayOrder == AUDIO_RAND) || (Offset < 0))
        {
            tempFileNum = pSysFileInfo->CurrentFileNum;

            FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData,  pSysFileInfo->pExtStr, RECORD_DB);
            for (i=1; i<tempFileNum; i++)
            {
                FindFileResult = FindNextFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData,  pSysFileInfo->pExtStr, RECORD_DB);
            }
            uiNeedFindNext = 0;

        }
        else if ((uiNeedFindNext) && (Offset > 0))
        {
          FindFileResult = FindNextFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, RECORD_DB);
          if (FindFileResult == NOT_FIND_FILE)
          {
             FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, RECORD_DB);
          }
        }
        #endif

    }
    else if(pSysFileInfo->ucSelPlayType == MUSIC_TYPE_SEL_FMFILE)
    {
        if(pSysFileInfo->PlayOrder == AUDIO_RAND || Offset < 0)
        {
            tempFileNum = pSysFileInfo->CurrentFileNum;

            FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData,  pSysFileInfo->pExtStr, RECORD_DB);
            for (i=1; i<tempFileNum; i++)
            {
                FindFileResult = FindNextFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData,  pSysFileInfo->pExtStr, RECORD_DB);
            }
            uiNeedFindNext = 0;

        }
        else if ((uiNeedFindNext) && (Offset > 0))
        {
          FindFileResult = FindNextFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, RECORD_DB);
          if (FindFileResult == NOT_FIND_FILE)
          {
             FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, RECORD_DB);
          }
        }

    }
    #endif

    #ifdef _M3U_
    else if(pSysFileInfo->ucSelPlayType == SORT_TYPE_SEL_M3U_BROWSER)
    {
        if(((Offset < 0) || (pSysFileInfo->PlayOrder == AUDIO_RAND)) || ((uiNeedFindNext) && (Offset > 0)))
        {
            tempFileNum = GetCurFileNum(pSysFileInfo->M3uGlobalFileNumBuf[pSysFileInfo->CurrentFileNum - 1], &pSysFileInfo->FindData, pSysFileInfo->pExtStr, FS_FAT);

            FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, FS_FAT);

            for (i = 1; i < tempFileNum; i++)
            {
                FindFileResult = FindNextFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, FS_FAT);
            }
            uiNeedFindNext = 0;
        }
        //else if ((uiNeedFindNext) && (Offset > 0))
        //{
        //    FindFileResult = FindNextFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, FS_FAT);
        //
        //    if (FindFileResult == NOT_FIND_FILE) /* 没有发ˇ指定文件*/
        //    {
        //        tempFileNum = GetCurFileNum(pSysFileInfo->M3uGlobalFileNumBuf[pSysFileInfo->CurrentFileNum - 1], &pSysFileInfo->FindData, pSysFileInfo->pExtStr, FS_FAT);
        //        FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, FS_FAT);
        //    }
        //}
    }
    #endif
    else
    {
        switch(pSysFileInfo->ucSelPlayType)
        {
           case SORT_TYPE_SEL_NOW_PLAY:

           #ifdef _MEDIA_MODULE_
           case SORT_TYPE_SEL_FILENAME:
           case SORT_TYPE_SEL_ID3TITLE:
           case SORT_TYPE_SEL_ID3SINGER:
           case SORT_TYPE_SEL_ID3ALBUM:
           case SORT_TYPE_SEL_GENRE:
                i = 0;
                do
                {
                    UINT8 ucBufTemp[2];
                    UINT16 temp1;

                    MDReadData(DataDiskID, (pSysFileInfo->ulSortInfoSectorAddr << 9) + (UINT32) ((pSysFileInfo->CurrentFileNum + pSysFileInfo->uiBaseSortId[pSysFileInfo->ucCurDeep] - 1) * 2), 2, ucBufTemp);

                    temp1 = (ucBufTemp[0]&0xff)+((ucBufTemp[1]&0xff)<<8); // 获取文件号 (在ˇ细文件信息表中的位置)
                    MDReadData(DataDiskID, (pSysFileInfo->ulFullInfoSectorAddr<<9)+(UINT32)(temp1)*BYTE_NUM_SAVE_PER_FILE + (UINT32)DIR_CLUS_SAVE_ADDR_OFFSET, 8, (uint8 *)&(pSysFileInfo->FindData));

#ifdef _RK_CUE_
                    MDReadData(DataDiskID, (pSysFileInfo->ulFullInfoSectorAddr<<9)+(UINT32)(temp1)*BYTE_NUM_SAVE_PER_FILE + (UINT32)CUE_START_SAVE_ADDR_OFFSET, 4, (uint8 *)&(pSysFileInfo->FindData.CueStartTime));
                    MDReadData(DataDiskID, (pSysFileInfo->ulFullInfoSectorAddr<<9)+(UINT32)(temp1)*BYTE_NUM_SAVE_PER_FILE + (UINT32)CUE_END_SAVE_ADDR_OFFSET, 4, (uint8 *)&(pSysFileInfo->FindData.CueEndTime));

                    if(pSysFileInfo->FindData.CueStartTime != 0 || pSysFileInfo->FindData.CueEndTime != 0)
                    {
                        pSysFileInfo->FindData.IsCue = 1;
                    }
                    else
                    {
                        pSysFileInfo->FindData.IsCue = 0;
                    }
#endif

                    FindFileResult = RETURN_OK;

                    if(FindFileResult==RETURN_OK)
                        break; // 找到可播放歌曲

                    if(i==pSysFileInfo->TotalFiles)
                        break; // 当循环所有歌曲发ˇ没有可播放音乐时退出该程序

                   i++;
                }while(1);

                break;

            case MUSIC_TYPE_SEL_MYFAVORITE:
                pSysFileInfo->TotalFiles = gSysConfig.MedialibPara.gMyFavoriteFileNum; // 防止在播放音乐时删除了收藏夹歌曲

                if(pSysFileInfo->TotalFiles ==0)
                {
                    break;
                }

                i = 0;
                do
                {
                   FindFileResult = GetFavoInfo(&pSysFileInfo->FindData, (pSysFileInfo->CurrentFileNum + pSysFileInfo->uiBaseSortId[pSysFileInfo->ucCurDeep] - 1), MusicLongFileName);

                   if(FindFileResult==RETURN_OK) break; // 找到可播放歌曲

                   if(pSysFileInfo->TotalFiles == i)  break; // 播放收藏夹音乐时可能清空了收藏夹
                   i++;
                }while(1);
                break;

            #endif

            default:
                break;
        }

    }
    return(FindFileResult);
}

/*
--------------------------------------------------------------------------------
  Function name : INT16 SysFindFile(SYS_FILE_INFO *pSysFileInfo,    INT16 Offset)
  Author        :  zs
  Description   :  查找文件
  Description   :  search file.
  Input         :  SYS_FILE_INFO *pSysFileInfo:structure to find file.
                   Offset == 0   find current file.
                   Offset > 0    find backward file,the offset is the offset file number relatived to the current file.
                   Offset < 0    find forward file,the offset is the offset file number relatived to the current file.

  Return        :
  History       :  <author>         <time>         <version>
                     zs            2009/02/27         Ver1.0
  desc          :   org
--------------------------------------------------------------------------------
*/
_ATTR_SYS_FINDFILE_TEXT_
INT16 SysFindFile(SYS_FILE_INFO *pSysFileInfo,    INT16 Offset)
{

    UINT16  i, tempFileNum;
    INT16   FindFileResult = -1;

    if(Offset == 0)
        return (RETURN_OK);

    if(Offset<0)
    {
        while(Offset++)
        {
            FindFileResult =  SysFindFileExt(pSysFileInfo,-1);

            if(FindFileResult != RETURN_OK)
                return FindFileResult;
        }
    }
    else
    {
        while(Offset--)
        {
            FindFileResult =  SysFindFileExt(pSysFileInfo,1);

            if(FindFileResult != RETURN_OK)
                return FindFileResult;
        }
    }
}

/*
********************************************************************************
*  Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*  Function name :  GetDirPath()
*  Author:          ZHengYongzhi
*  Description:     get file full path
*
*  Input:   pPath           —— store the start address of file path.
*  Output:  pPath           —— file path
*  Return:
*  Calls:
*
*  History:     <author>         <time>         <version>
*             ZhengYongzhi     2006/01/01         Ver1.0
*     desc: ORG
********************************************************************************
*/
//_ATTR_SYS_FINDFILE_TEXT_
/*
void GetDirPath(UINT8 *pPath)
{
    UINT16 i,j;

    *pPath++ = 0x55;    //'U';
    *pPath++ = 0x3a;    //':';
    *pPath++ = 0x5c;    //'\\';
    for (i = 1; i <= CurDirDeep; i++)
    {
        for (j = 0; j < 11 ; j++ )
        {
            *pPath++ = SubDirInfo[i].DirName[j];
        }
        *pPath++ = 0x5c;//'\\';
    }
    if (CurDirDeep != 0)
    {
        pPath--;
    }
    *pPath= 0;
}
*/

/*
********************************************************************************
*
*                         End of SysFindFile.c
*
********************************************************************************
*/

