/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��   Dir.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-10-21          1.0
*    desc:    ORG.
********************************************************************************
*/
#define  IN_DIR

#include "FsInclude.h"


#ifdef FIND_FILE
/*********************************************************************************************************
** Name        :BuildDirInfo
** Description :create direction information
** Input       :expand name related file.
** Output      :file number
** global      :SubDirInfo[],CurDirDeep,CurDirClus
** call module : null
********************************************************************************************************/
_ATTR_FAT_FIND_CODE_
uint16 BuildDirInfo(uint8* ExtName, FS_TYPE FsType)
{
    uint16 TotalFiles = 0;

    GotoCurDir(ExtName, FsType);

    while (1)
    {
        #ifdef _WATCH_DOG_
        WatchDogReload();
        #endif

        TotalFiles += SubDirInfo[CurDirDeep].TotalFile;

        //DEBUG("total file = %d, FsType = %d\n", TotalFiles, FsType);

        do
        {
            #ifdef _WATCH_DOG_
            WatchDogReload();
            #endif

            GotoNextDir(ExtName, FsType);           // ������һ��Ŀ¼��������Ŀ¼����ͬ��Ŀ¼
        }
        while ((SubDirInfo[CurDirDeep].TotalFile == 0) && (CurDirDeep != 0));

        if (CurDirDeep == 0)
        {
            break;
        }
    }

    return (TotalFiles);
}


/*********************************************************************************************************
** Name         :GetCurFileNum
** Description  :get current direction file index pointer from global file index pointer.
** Input        :global file pointer FileNum, file type ExtName
** Output       :the file index pointer of current direction.return 0 if happen err.
** global       :CurDirDeep
** call module  :GotoRootDir, GotoNextDir
** explain     :it will enter the direction automatically after call it.
********************************************************************************************************/
_ATTR_FAT_FIND_CODE_
uint16 GetCurFileNum(uint16 FileNum, FIND_DATA * FindData, uint8 *ExtName, FS_TYPE FsType)
{
    uint16 temp;
    FILE_TREE_BASIC FileTreeBasic;

    //GotoRootDir(ExtName, FsType);
    GotoCurDir(ExtName, FsType);

    while (FileNum > SubDirInfo[CurDirDeep].TotalFile)
    {
        FileNum -= SubDirInfo[CurDirDeep].TotalFile;

        do
        {
            GotoNextDir(ExtName, FsType);           // ������һ��Ŀ¼��������Ŀ¼����ͬ��Ŀ¼

        }
        while ((SubDirInfo[CurDirDeep].TotalFile == 0) && (CurDirDeep != 0));

        if (CurDirDeep == 0)
        {
            FileNum = 0;
            break;
        }
    }
    FindData->Clus = SubDirInfo[CurDirDeep].DirClus;
    FindData->TotalItem = SubDirInfo[CurDirDeep].TotalFile + SubDirInfo[CurDirDeep].TotalSubDir;

    return (FileNum);
}


/*********************************************************************************************************
** Name        : GotoNextDir
** Description : turn to next direction
** Input       : extension related files.
** Output      : null
** global      : SubDirInfo[],CurDirDeep,CurDirClus
** call module : null
********************************************************************************************************/
_ATTR_FAT_FIND_CODE_
void GotoNextDir(uint8* ExtName, FS_TYPE FsType)
{
    if ((SubDirInfo[CurDirDeep].TotalSubDir == 0) || (CurDirDeep == (MAX_DIR_DEPTH - 1)))    //��Ŀ¼��û��Ŀ¼��ΪҶ���,Ҫ�Ҹ�Ŀ¼��ͬ��Ŀ¼
    {
        while (1)
        {
            #ifdef _WATCH_DOG_
            WatchDogReload();
            #endif

            if (CurDirDeep == 0)                    //�ҵ���Ŀ¼�˲�����������
            {
                return;
            }

            CurDirDeep--;                       //��ָ����һ��Ŀ¼
            if (SubDirInfo[CurDirDeep].CurDirNum < SubDirInfo[CurDirDeep].TotalSubDir)
            {
                SubDirInfo[CurDirDeep].CurDirNum++;
                CurDirClus = SubDirInfo[CurDirDeep].DirClus;
                CurDirDeep++;
                SubDirInfo[CurDirDeep].DirNum = TotalAllDir;
                CurDirClus = ChangeDir(SubDirInfo[CurDirDeep - 1].Index, FsType);
                break;
            }
        }
    }
    else            //��Ŀ¼�»�����Ŀ¼,Ҫ�����ĵ�һ����Ŀ¼
    {
        SubDirInfo[CurDirDeep].CurDirNum = 0;
        SubDirInfo[CurDirDeep].Index = 0;
        SubDirInfo[CurDirDeep].CurDirNum++;
        SubDirInfo[CurDirDeep].DirClus = CurDirClus;
        CurDirDeep++;
        SubDirInfo[CurDirDeep].DirNum = TotalAllDir;
        CurDirClus = ChangeDir(0, FsType);
    }

    SubDirInfo[CurDirDeep].TotalFile = GetTotalFiles(CurDirClus, ExtName, FsType);   //add by lxs @2005.02.24

    SubDirInfo[CurDirDeep].TotalSubDir = GetTotalSubDir(CurDirClus, FsType);     //��ȡ��Ŀ¼�µ���Ŀ¼��
    TotalAllDir += SubDirInfo[CurDirDeep].TotalSubDir;

}



/*********************************************************************************************************
** Name        : GotoNextDir
** Description : turn to next direction
** Input       : extension related files.
** Output      : null
** global      : SubDirInfo[],CurDirDeep,CurDirClus
** call module : null
********************************************************************************************************/
_ATTR_FAT_FIND_CODE_
uint32 GetCurDir(uint32 DirClus, uint32 index, FS_TYPE FsType)
{
    FILE_TREE_BASIC FileTreeBasic;
    UINT32 cluster;
    UINT16 temp;

    if (FsType == MUSIC_DB)
    {
        //rk_printf("DirClus = %d, index =%d", DirClus, index);
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + MUSIC_TREE_SORT_INFO_SECTOR_START)<<9) + 2 * (UINT32)(DirClus + index), 2, (uint8 *)&temp);
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + MUSIC_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(DirClus + temp), sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);
        if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_DIR)
        {
            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + MUSIC_TREE_INFO_SECTOR_START + MEDIAINFO_BLOCK_SIZE * 2)<<9) + sizeof(FILE_TREE_EXTEND)*(UINT32)(FileTreeBasic.dwExtendTreeID), 4, (uint8 *)&cluster);
        }
        else
        {
            cluster = 0;
        }
    }
    else if (FsType == RECORD_DB)
    {
        //printf("DirClus = %d, index =%d\n", DirClus, index);
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + RECORD_TREE_SORT_INFO_SECTOR_START)<<9) + 2 * (UINT32)(DirClus + index), 2, (uint8 *)&temp);
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + RECORD_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(DirClus + temp), sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);
        if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_DIR)
        {
            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + RECORD_TREE_INFO_SECTOR_START + MEDIAINFO_BLOCK_SIZE * 6)<<9) + sizeof(FILE_TREE_EXTEND)*(UINT32)(FileTreeBasic.dwExtendTreeID), 4, (uint8 *)&cluster);
            //printf("son = %d\n", cluster);
        }
        else
        {
            cluster = 0;
        }
    }
#ifdef PIC_MEDIA                                                                                                                                                                                                    /*<--sanshin 0612*/
    else if(FsType == JPEG_DB)                                                                                                                                                                                      /*<--sanshin 0612*/
    {                                                                                                                                                                                                               /*<--sanshin 0612*/
         //printf("DirClus = %d, index =%d\n", DirClus, index);                                                                                                                                                       /*<--sanshin 0612*/
         MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + JPEG_TREE_SORT_INFO_SECTOR_START)<<9) + 2 * (UINT32)(DirClus + index), 2, (uint8 *)&temp);                                                                 /*<--sanshin 0612*/
         MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + JPEG_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(DirClus + temp), sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);                    /*<--sanshin 0612*/
         if(FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_DIR)                                                                                                                                                        /*<--sanshin 0612*/
         {                                                                                                                                                                                                          /*<--sanshin 0612*/
             MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + JPEG_TREE_INFO_SECTOR_START + MEDIAINFO_BLOCK_SIZE * 6)<<9) + sizeof(FILE_TREE_EXTEND)*(UINT32)(FileTreeBasic.dwExtendTreeID), 4, (uint8 *)&cluster);  /*<--sanshin 0612*/
             //printf("son = %d\n", cluster);                                                                                                                                                                         /*<--sanshin 0612*/
         }                                                                                                                                                                                                          /*<--sanshin 0612*/
         else                                                                                                                                                                                                       /*<--sanshin 0612*/
         {                                                                                                                                                                                                          /*<--sanshin 0612*/
            cluster = 0;                                                                                                                                                                                            /*<--sanshin 0612*/
         }                                                                                                                                                                                                          /*<--sanshin 0612*/
    }                                                                                                                                                                                                               /*<--sanshin 0612*/
#endif                                                                                                                                                                                                              /*<--sanshin 0612*/
    else
    {

    }

    return cluster;
}



/*********************************************************************************************************
** Name        : GotoRootDir
** Description : goto boot direction.
** Input       : extension related file
** Output      : null
** global      : SubDirInfo[],CurDirDeep,CurDirClus
** call module : null
********************************************************************************************************/
_ATTR_FAT_FIND_CODE_
void GotoRootDir(uint8* ExtName, FS_TYPE FsType)
{
    CurDirDeep = 0;                       //direction deepth.

    if (FsType == MUSIC_DB)
    {
        CurDirClus = 0;
    }
    else if (FsType == RECORD_DB)
    {
        CurDirClus = 0;
    }
#ifdef PIC_MEDIA                /*<--sanshin 0612*/
    else if(FsType == JPEG_DB)  /*<--sanshin 0612*/
    {                           /*<--sanshin 0612*/
    CurDirClus = 0;             /*<--sanshin 0612*/
    }                           /*<--sanshin 0612*/
#endif                          /*<--sanshin 0612*/
    else
    {
        // FS_FAT, FS_FAT_EX_VOICE
        CurDirClus = BootSector.BPB_RootClus; //direction cluster boot direction.
    }

    SubDirInfo[CurDirDeep].TotalSubDir = GetTotalSubDir(CurDirClus, FsType);
    SubDirInfo[CurDirDeep].TotalFile   = GetTotalFiles(CurDirClus, ExtName, FsType);
    SubDirInfo[CurDirDeep].DirClus = CurDirClus;
    SubDirInfo[CurDirDeep].Index = 0;
    SubDirInfo[CurDirDeep].DirNum = 0;
    TotalAllDir = SubDirInfo[CurDirDeep].TotalSubDir;
}


/*********************************************************************************************************
** Name        : GotoRootDir
** Description : goto boot direction.
** Input       : extension related file
** Output      : null
** global      : SubDirInfo[],CurDirDeep,CurDirClus
** call module : null
********************************************************************************************************/
_ATTR_FAT_FIND_CODE_
void GotoCurDir(uint8* ExtName, FS_TYPE FsType)
{
    CurDirDeep = 0;                       //direction deepth.

    CurDirClus = gwSaveDirClus;

    SubDirInfo[CurDirDeep].TotalSubDir = GetTotalSubDir(CurDirClus, FsType);
    SubDirInfo[CurDirDeep].TotalFile   = GetTotalFiles(CurDirClus, ExtName, FsType);
    SubDirInfo[CurDirDeep].DirClus = gwSaveDirClus;
    SubDirInfo[CurDirDeep].Index = 0;
    SubDirInfo[CurDirDeep].DirNum = 0;
    TotalAllDir = SubDirInfo[CurDirDeep].TotalSubDir;
}

/*********************************************************************************************************
** Name :GetRootDirClus
** Description :
** Input   :
** Output         : null
** call module : null
********************************************************************************************************/
_ATTR_FAT_FIND_CODE_
long GetRootDirClus(FS_TYPE FsType)
{
    if (FsType == MUSIC_DB)
    {
        return 0;
    }
    else if (FsType == RECORD_DB)
    {
        return 0;
    }
#ifdef PIC_MEDIA                /*<--sanshin 0612*/
    else if(FsType == JPEG_DB)  /*<--sanshin 0612*/
    {                           /*<--sanshin 0612*/
        return 0;               /*<--sanshin 0612*/
    }                           /*<--sanshin 0612*/
#endif                          /*<--sanshin 0612*/
    else
    {
        return(BootSector.BPB_RootClus);
    }
}

/*********************************************************************************************************
** Name         :ChangeDir
** Description  :change direction,turn to child direction point by index of current direction.
** Input        :SubDirIndex
** Output       :cluster number of index child direction.
** global       :CurDirClus
** call module  : null
********************************************************************************************************/
_ATTR_FAT_FIND_CODE_
uint32 ChangeDir(uint16 SubDirIndex, FS_TYPE FsType)
{
    FDT Rt;
    uint32 cluster;
    uint32 index;
    uint16 temp;
    FILE_TREE_BASIC FileTreeBasic;


    index= SubDirIndex;
    cluster = 0;

    if (FsType == MUSIC_DB)
    {
        while (1)
        {
            #ifdef _WATCH_DOG_
            WatchDogReload();
            #endif

            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + MUSIC_TREE_SORT_INFO_SECTOR_START)<<9) + 2 * (UINT32)(CurDirClus + index), 2, (uint8 *)&temp);
            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + MUSIC_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(CurDirClus + temp), sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);
            index++;
            if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_DIR)
            {
                MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + MUSIC_TREE_INFO_SECTOR_START + MEDIAINFO_BLOCK_SIZE * 2)<<9) + sizeof(FILE_TREE_EXTEND)*(UINT32)(FileTreeBasic.dwExtendTreeID), 4, (uint8 *)&cluster);
                break;
            }
        }

    }
    else if (FsType == RECORD_DB)
    {
        while (1)
        {
            #ifdef _WATCH_DOG_
            WatchDogReload();
            #endif

            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + RECORD_TREE_SORT_INFO_SECTOR_START)<<9) + 2 * (UINT32)(CurDirClus + index), 2, (uint8 *)&temp);
            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + RECORD_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(CurDirClus + temp), sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);
            index++;
            if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_DIR)
            {
                MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + RECORD_TREE_INFO_SECTOR_START + MEDIAINFO_BLOCK_SIZE * 6)<<9) + sizeof(FILE_TREE_EXTEND)*(UINT32)(FileTreeBasic.dwExtendTreeID), 4, (uint8 *)&cluster);
                break;
            }
        }

    }

#ifdef PIC_MEDIA                                                                                                                                                                                                                /*<--sanshin 0612*/
   else if(FsType == JPEG_DB)                                                                                                                                                                                                   /*<--sanshin 0612*/
    {                                                                                                                                                                                                                           /*<--sanshin 0612*/
        while(1)                                                                                                                                                                                                                /*<--sanshin 0612*/
        {
            #ifdef _WATCH_DOG_
            WatchDogReload();
            #endif

            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + JPEG_TREE_SORT_INFO_SECTOR_START)<<9) + 2 * (UINT32)(CurDirClus + index), 2, (uint8 *)&temp);                                                                       /*<--sanshin 0612*/
            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + JPEG_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(CurDirClus + temp), sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);                          /*<--sanshin 0612*/
            index++;                                                                                                                                                                                                            /*<--sanshin 0612*/
            if(FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_DIR)                                                                                                                                                                 /*<--sanshin 0612*/
            {                                                                                                                                                                                                                   /*<--sanshin 0612*/
                MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + JPEG_TREE_INFO_SECTOR_START + MEDIAINFO_BLOCK_SIZE * 6)<<9) + sizeof(FILE_TREE_EXTEND)*(UINT32)(FileTreeBasic.dwExtendTreeID), 4, (uint8 *)&cluster);           /*<--sanshin 0612*/
                break;                                                                                                                                                                                                          /*<--sanshin 0612*/
            }                                                                                                                                                                                                                   /*<--sanshin 0612*/
        }                                                                                                                                                                                                                       /*<--sanshin 0612*/
                                                                                                                                                                                                                                /*<--sanshin 0612*/
    }                                                                                                                                                                                                                           /*<--sanshin 0612*/
#endif                                                                                                                                                                                                                          /*<--sanshin 0612*/
    else
    {
        while (1)
        {
            #ifdef _WATCH_DOG_
            WatchDogReload();
            #endif

            if (RETURN_OK != GetFDTInfo(&Rt, CurDirClus, index++))
            {
                break;
            }

            if (Rt.Name[0] == 0x00)         //null mean end
            {
                break;
            }

            if (Rt.Name[0] == FILE_DELETED) //had deletec
            {
                continue;
            }

            if (Rt.Name[0] == 0x2e)
            {
                continue;
            }

            if (Rt.Attr & ATTR_HIDDEN)
            {
                continue;
            }

            if ((Rt.Attr & ATTR_DIRECTORY) == ATTR_DIRECTORY)
            {
                if (CurDirDeep == 1)
                {
                    if (FsType == FS_FAT_EX_VOICE)
                    {
                        if (memcmp(Rt.Name, "RECORD      ", 11) == 0)
                        {
                            continue;
                        }
                    }
                }

                cluster   = Rt.FstClusHI;
                cluster <<= 16;
                cluster  |= Rt.FstClusLO;
                break;

            }
        }
    }

    SubDirInfo[CurDirDeep].DirClus = cluster;
    SubDirInfo[CurDirDeep-1].Index = index;
    return (cluster);
}


/*********************************************************************************************************
** Name        :GetTotalSubDir
** Description :get the total number of child direction.
** Input       :Path
** Output      :child direction number
** global      :CurDirClus
** call module : null
********************************************************************************************************/
_ATTR_FAT_FIND_CODE_
uint16 GetTotalSubDir(uint32 DirClus, FS_TYPE FsType)
{
    FDT Rt;
    uint32 index;
    uint16 TotSubDir=0;
    //uint8 buf[512];

    FILE_TREE_BASIC  FileTreeBasic;

    if (FsType == MUSIC_DB)
    {
        if (DirClus != 0xffffffff)
        {
            for (index = DirClus; ; index++)
            {
            #ifdef _WATCH_DOG_
                WatchDogReload();
            #endif
                /*
                if(index % (512 / sizeof(FILE_TREE_BASIC)) == 0)
                {
                    MDRead(DataDiskID, MediaInfoAddr + MUSIC_TREE_INFO_SECTOR_START + index / (512 / sizeof(FILE_TREE_BASIC)), 1, buf);
                    FileTreeBasic = (FILE_TREE_BASIC *)buf;
                }
                */

                MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + MUSIC_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(index), sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);

                if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_DIR)
                {
                    TotSubDir++;
                }

                if (FileTreeBasic.dwNextBrotherID == 0xffffffff)
                {
                    break;
                }
                //FileTreeBasic++;

            }
        }
    }
    else if (FsType == RECORD_DB)
    {
        if (DirClus != 0xffffffff)
        {
            for (index = DirClus; ; index++)
            {
            #ifdef _WATCH_DOG_
                WatchDogReload();
            #endif
                /*
                if(index % (512 / sizeof(FILE_TREE_BASIC)) == 0)
                {
                    MDRead(DataDiskID, MediaInfoAddr + MUSIC_TREE_INFO_SECTOR_START + index / (512 / sizeof(FILE_TREE_BASIC)), 1, buf);
                    FileTreeBasic = (FILE_TREE_BASIC *)buf;
                }
                */

                MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + RECORD_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(index), sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);

                if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_DIR)
                {
                    TotSubDir++;
                }

                if (FileTreeBasic.dwNextBrotherID == 0xffffffff)
                {
                    break;
                }
                //FileTreeBasic++;

            }
        }
    }
#ifdef PIC_MEDIA                                                                                                                                                                                        /*<--sanshin 0612*/
    else if(FsType == JPEG_DB)                                                                                                                                                                          /*<--sanshin 0612*/
    {                                                                                                                                                                                                   /*<--sanshin 0612*/
       if(DirClus != 0xffffffff)                                                                                                                                                                        /*<--sanshin 0612*/
          {                                                                                                                                                                                             /*<--sanshin 0612*/
            for(index = DirClus; ; index++)                                                                                                                                                             /*<--sanshin 0612*/
            {                                                                                                                                                                                           /*<--sanshin 0612*/
            #ifdef _WATCH_DOG_
                WatchDogReload();
            #endif

                MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + JPEG_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(index), sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);          /*<--sanshin 0612*/
                                                                                                                                                                                                        /*<--sanshin 0612*/
                if(FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_DIR)                                                                                                                                     /*<--sanshin 0612*/
                {                                                                                                                                                                                       /*<--sanshin 0612*/
                    TotSubDir++;                                                                                                                                                                        /*<--sanshin 0612*/
                }                                                                                                                                                                                       /*<--sanshin 0612*/
                                                                                                                                                                                                        /*<--sanshin 0612*/
                if(FileTreeBasic.dwNextBrotherID == 0xffffffff)                                                                                                                                         /*<--sanshin 0612*/
                {                                                                                                                                                                                       /*<--sanshin 0612*/
                    break;                                                                                                                                                                              /*<--sanshin 0612*/
                }                                                                                                                                                                                       /*<--sanshin 0612*/
                //FileTreeBasic++;                                                                                                                                                                      /*<--sanshin 0612*/
                                                                                                                                                                                                        /*<--sanshin 0612*/
            }                                                                                                                                                                                           /*<--sanshin 0612*/
        }                                                                                                                                                                                               /*<--sanshin 0612*/
    }                                                                                                                                                                                                   /*<--sanshin 0612*/
#endif                                                                                                                                                                                                  /*<--sanshin 0612*/
    else
    {
        if (DirClus == BAD_CLUS)
        {
            return (0);
        }

        for (index = 0; ; index++)
        {
        #ifdef _WATCH_DOG_
            WatchDogReload();
        #endif
            if (RETURN_OK != GetFDTInfo(&Rt, DirClus, index))
            {
                break;
            }

            if (Rt.Name[0] == FILE_NOT_EXIST)   //�ҵ��յ���Ϊ����
            {
                break;
            }
            else if (Rt.Name[0] == FILE_DELETED)     //��ɾ��
            {
                continue;
            }
            else if (Rt.Name[0] == 0x2e) // removed . and ..
            {
                continue;
            }
            else if ((FsType == FS_FAT_EX_VOICE) && (Rt.Attr & ATTR_DIRECTORY))
            {
                if (CurDirDeep == 0)
                {
                    if ((memcmp(Rt.Name, "RECORD     ", 11) == 0) )
                    {
                        //printf("find a record\n");
                        continue;
                    }
                }
            }

            if (Rt.Attr & ATTR_HIDDEN)
            {
                continue;
            }

            if ((Rt.Attr & ATTR_DIRECTORY) == ATTR_DIRECTORY)
            {
                TotSubDir++;
            }
        }
    }

    return (TotSubDir);
}
#endif

/*********************************************************************************************************
** Name        : GetDirClusIndex
** Description : get start cluster number of sepecified direction.
** Input       : Path:(no include file name)
** Output      : start cluster number.
** global      : CurDirClus
** call module : FindFDTInfo
** explain     : '\' can not be the path end,suppot 1~11 path name.
********************************************************************************************************/
IRAM_FAT
uint32 GetDirClusIndex(uint8 *Path)
{
    uint8  i;
    uint32 DirClusIndex;
    FDT    temp;
    uint8  PathName[12];

    DirClusIndex = BAD_CLUS;

    if (Path != NULL)       //null pointer
    {
//***********************************************************************
//֧���̷���A:
//***********************************************************************
        if (Path[1] == ':')
        {
            Path += 2;
        }

        DirClusIndex = BootSector.BPB_RootClus; //��Ŀ¼
//***********************************************************************
//A:TEMP��TEMP��.\TEMP����ָ��ǰĿ¼�µ�TEMP��Ŀ¼
        if (Path[0] != '\\')            //* ����Ŀ¼�ָ�����,��������ǵ�ǰ·��
        {
            DirClusIndex = CurDirClus;
        }
        else
        {
            Path++;
        }

        if (Path[0] == '.')             // '\.'��������ǵ�ǰ·��
        {
            DirClusIndex = CurDirClus;
            if (Path[1] == '\0' || Path[1] == '\\')     //case "." or ".\"
            {
                Path++;
            }
        }

        while (Path[0] != '\0')
        {
            if (Path[0] == ' ')         //�׸��ַ�������Ϊ�ո�
            {
                DirClusIndex = BAD_CLUS;
                break;
            }

            for (i = 0; i < 11; i++)        //Ŀ¼����ո�
            {
                PathName[i] = ' ';
            }

            for (i = 0; i < 12; i++)
            {
                if (*Path == '\0')      //��·������
                {
                    break;
                }

                PathName[i] = *Path++;
            }

            if (FindFDTInfo(&temp, DirClusIndex, PathName) != RETURN_OK)    //��ȡFDT��Ϣ
            {
                DirClusIndex = BAD_CLUS;
                break;
            }

            if ((temp.Attr & ATTR_DIRECTORY) == 0)  //FDT�Ƿ���Ŀ¼
            {
                DirClusIndex = BAD_CLUS;
                break;
            }

            DirClusIndex = ((uint32)(temp.FstClusHI) << 16) + temp.FstClusLO;
        }
    }

    return (DirClusIndex);

}


#ifdef LONG_DIR_PATH

IRAM_FAT
uint8 FSRealname(uint8 *dname, uint16 *pDirName)
{
    uint8 i;
    uint8 *pdname = dname;
    uint16 len;
    uint8 NTRes = 0;

    if ((*pDirName >= 'a') && (*pDirName <= 'z'))
    {
        NTRes |= 0x08;
    }


    for (i = 0; i < 11; i++)        //Ŀ¼����ո�
    {
        dname[i] = ' ';
    }

    for (i = 0; i < 13; i++)   // 11 + '.' + '\0' = 13
    {
        if (*pDirName == '\0' || *pDirName == '\\' )      //��·������,
        {
            break;
        }

        if (*pDirName == '.') //�ļ���Ҫȥ����׺����.
        {
            pDirName++;
            pdname = &dname[8];

            if ((*pDirName >= 'a') && (*pDirName <= 'z'))
            {
                NTRes |= 0x10;
            }
            continue;
        }

        if ((*pDirName >= 'a') && (*pDirName <= 'z'))
        {
            *pdname = *pDirName - ('a' - 'A');
        }
        else
        {
            *pdname = *pDirName;
        }
        pdname++;
        pDirName++;

    }

    return NTRes;

}


IRAM_FAT
uint8 FSIsLongName(uint16 *dname, uint32 len)
{
    uint32  i = len;
    uint16 *p = dname;
    uint16 dot = 0;
    uint8 UpCase, LowCase;

    if (len > 12)
    {
        return 1;
    }

    while (i--)
    {
        if ( (*p > 127)
                ||(*p == '+')
                ||(*p == '=')
                ||(*p == ',')
                ||(*p == ';')
                ||(*p == '[')
                ||(*p == ']')
                ||(*p == ' ')) //��"+ = , ; [ ]"��Ҳ�ǳ��ļ���
        {
            return 1;
        }
        else if (*p == '.')
        {
            dot++;
            if (dot > 1) /* С�������1�����ǳ��ļ��� */
            {
                return 1;
            }

            if ((len-i-1) > 8)  /* �ļ���������8���ǳ��ļ��� */
            {
                return 1;
            }

            if (i > 3)   /* ��չ������3���ǳ��ļ��� */
            {
                return 1;
            }
        }
        p++;
    }

    if ((len == 12) && (dot == 0))
    {
        return 1;
    }

    UpCase = 0;
    LowCase = 0;
    p = dname;

    for (i = 0; i < 8; i++)
    {
        if ((*p >='a') && (*p <='z'))
        {
            LowCase = 1;
        }
        else if ((*p >='A') && (*p <='Z'))
        {
            UpCase = 1;
        }

        //printf("%c\n", *p);
        p++;
        if (*p == '.')
        {
            p++;
            break;
        }
        else if ((*p == 0) || (*p == '\\'))
        {
            if ((LowCase == 1) && (UpCase == 1))
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }

    }

    if ((LowCase == 1) && (UpCase == 1))
    {
        //printf("xxx\n");
        return 1;
    }

    UpCase = 0;
    LowCase = 0;

    for (i = 0; i < 3; i++)
    {
        if ((*p >='a') && (*p <='z'))
        {
            LowCase = 1;
        }
        else if ((*p >='A') && (*p <='Z'))
        {
            UpCase = 1;
        }
        else if ((*p == 0) || (*p == '\\'))
        {
            break;
        }

        p++;
    }

    if (*p != 0)
    {
        return 1;
    }

    if ((LowCase == 1) && (UpCase == 1))
    {
        //printf("yyyyy\n");
        return 1;
    }

    return 0;
}

/*********************************************************************************************************
********************************************************************************************************/
IRAM_FAT
uint32 GetDirClusIndexLong(uint16 *Path, uint16 len)
{
    uint16  i;
    uint32 DirClusIndex, Index;
    FDT    temp;
    uint8   SName[13];
    uint16 *start;
    uint8 ret;

    DirClusIndex = BAD_CLUS;

    if (Path != NULL)       //null pointer
    {
//***********************************************************************
//֧���̷���A:
//***********************************************************************
        if (Path[1] == ':')
        {
            Path += 2;
            len -= 2;
        }

        DirClusIndex = BootSector.BPB_RootClus; //��Ŀ¼
//***********************************************************************
//A:TEMP��TEMP��.\TEMP����ָ��ǰĿ¼�µ�TEMP��Ŀ¼
        if (Path[0] != '\\')            //* ����Ŀ¼�ָ�����,��������ǵ�ǰ·��
        {
            DirClusIndex = CurDirClus;
        }
        else
        {
            Path++;
            len--;
        }

        if (Path[0] == '.')             // '\.'��������ǵ�ǰ·��
        {
            DirClusIndex = CurDirClus;
            if (Path[1] == '\0' || Path[1] == '\\')     //case "." or ".\"
            {
                Path++;
                len--;
            }
        }

        while (len > 0)
        {
            if (Path[0] == ' ')         //�׸��ַ�������Ϊ�ո�
            {
                DirClusIndex = BAD_CLUS;
                break;
            }

            start = Path;
            for (i = 1; i < 256; i++)
            {
                Path++;
                len--;
                if (*Path == '\\')
                {
                    Path++;
                    len--;
                    break;
                }
            }

            if (FSIsLongName(start, i))
            {
                *(start + i) = L'\0';
                ret = FindFDTInfoLong(&temp, DirClusIndex, &Index, start);
                *(start + i) = L'\\';
            }
            else
            {
                FSRealname(SName, start);
                ret = FindFDTInfo(&temp, DirClusIndex, SName);
            }

            if (ret != RETURN_OK)    //��ȡFDT��Ϣ
            {
                DirClusIndex = BAD_CLUS;
                break;
            }


            if ((temp.Attr & ATTR_DIRECTORY) == 0)  //FDT�Ƿ���Ŀ¼
            {
                DirClusIndex = BAD_CLUS;
                break;
            }

            DirClusIndex = ((uint32)(temp.FstClusHI) << 16) + temp.FstClusLO;
            // printf("long: DirClus = %d\n", DirClusIndex);
        }
    }

    return (DirClusIndex);

}

#endif

#ifdef ENCODE
/*********************************************************************************************************
** Name        :DirDelete
** Description :Delete a dir.
** Input       :DirClus

** Output      :RETURN_OK��success, RETURN_FAIL: Fail
********************************************************************************************************/
IRAM_ENCODE
uint8 DirDelete(uint32 DirClus, uint32 DirIndex)
{
    uint32 gSaveClus,TotalFile;
    FDT fdt;
    uint8  Rt;
    uint32 ClusIndex1;

    uint32 TotalDir, TotalDeleFile;

    gSaveClus = gwSaveDirClus;

    //get dir info
    printf("dir clus = %d, Index = %d\n", DirClus, DirIndex);

    if(GetFDTInfo(&fdt, DirClus, DirIndex) != RETURN_OK)
    {
        printf("dir error\n");
        return RETURN_FAIL;
    }

    gwSaveDirClus = fdt.FstClusLO + ((uint32)(fdt.FstClusHI) << 16);

    GotoCurDir(ALLFileExtString, FS_FAT);

    TotalDir = 0;
    TotalDeleFile = 0;

    do
    {
		uint32 Index;

        Index = 0;
        TotalFile = SubDirInfo[CurDirDeep].TotalFile;

        printf("SubDirInfo[%d].DirClus = %d\n", CurDirDeep, SubDirInfo[CurDirDeep].DirClus);

        while(RETURN_OK == GetFDTInfo(&fdt, SubDirInfo[CurDirDeep].DirClus, Index++))		    //�ҵ�Ŀ¼��
        {
            printf("x");

            if(fdt.Name[0] == 0)
            {
                break;
            }

            if(fdt.Name[0] == 0xe5)
            {
                continue;
            }

            if(fdt.Name[0] == '.')
            {
                continue;
            }

            if ((fdt.Attr & (ATTR_DIRECTORY | ATTR_LFN_ENTRY | ATTR_VOLUME_ID)) == 0)  		                // ���ļ���ɾ��
     		{
     			Rt = FILE_LOCK;
     			if (FindOpenFile1(SubDirInfo[CurDirDeep].DirClus, Index - 1) >= MAX_OPEN_FILES)	//�ļ�û�д򿪲�ɾ��
     			{
     				ClusIndex1 = fdt.FstClusLO + ((uint32)(fdt.FstClusHI) << 16);

                     FATDelClusChain(ClusIndex1);

                     Rt = DelFDT(SubDirInfo[CurDirDeep].DirClus, fdt.Name);

                     TotalDeleFile++;

                     TotalFile--;
                     printf("Delete File Clus = %d, Index = %d\n", SubDirInfo[CurDirDeep].DirClus, Index);
                     if(TotalFile == 0)
                     {
                        break;
                     }
     			}
                else
                {
                    gwSaveDirClus = gSaveClus;
                    return Rt;
                }
             }

            #ifdef _WATCH_DOG_
            if(TotalFile % 100)
                WatchDogReload();
            #endif

        }

        #ifdef _WATCH_DOG_
            WatchDogReload();
        #endif


        if (SubDirInfo[CurDirDeep].TotalSubDir == 0)    //��Ŀ¼��û��Ŀ¼��ΪҶ���,Ҫ�Ҹ�Ŀ¼��ͬ��Ŀ¼
        {
            while(1)
            {
                if (CurDirDeep == 0)                    //�ҵ���Ŀ¼�˲�����������
                {
                    if(RETURN_OK != GetFDTInfo(&fdt, DirClus, DirIndex))
                    {
                        gwSaveDirClus = gSaveClus;
                         printf("dir error\n");
                        return RETURN_FAIL;
                    }
                    printf("delete0 dir DirClus = %d, Index = %d",DirClus, DirIndex);

                    ClusIndex1 = fdt.FstClusLO + ((uint32)(fdt.FstClusHI) << 16);

                    FATDelClusChain(ClusIndex1);

                    Rt = DelFDT(DirClus, fdt.Name);

                    TotalDir++;

                    goto END;
                }

                CurDirDeep--;                       //��ָ����һ��Ŀ¼

                printf("delete1 dir DirClus = %d, Index = %d\n",SubDirInfo[CurDirDeep].DirClus, SubDirInfo[CurDirDeep].Index - 1);

                if(RETURN_OK != GetFDTInfo(&fdt, SubDirInfo[CurDirDeep].DirClus, SubDirInfo[CurDirDeep].Index - 1))
                {
                    gwSaveDirClus = gSaveClus;
                     printf("dir error\n");
                    return RETURN_FAIL;
                }

                ClusIndex1 = fdt.FstClusLO + ((uint32)(fdt.FstClusHI) << 16);

                FATDelClusChain(ClusIndex1);

                Rt = DelFDT(SubDirInfo[CurDirDeep].DirClus, fdt.Name);

                TotalDir++;


                if(SubDirInfo[CurDirDeep].CurDirNum < SubDirInfo[CurDirDeep].TotalSubDir)
                {
                    SubDirInfo[CurDirDeep].CurDirNum++;
                    CurDirClus = SubDirInfo[CurDirDeep].DirClus;
                    CurDirDeep++;
                    SubDirInfo[CurDirDeep].DirNum = TotalAllDir;
                    CurDirClus = ChangeDir(SubDirInfo[CurDirDeep - 1].Index, FS_FAT);
                    break;
                }

            }

        }
        else if(CurDirDeep == (MAX_DIR_DEPTH - 1))
        {
              gwSaveDirClus = gSaveClus;
              printf("path too deep\n");
              return RETURN_FAIL;
        }
        else            //��Ŀ¼�»�����Ŀ¼,Ҫ�����ĵ�һ����Ŀ¼
        {
            SubDirInfo[CurDirDeep].CurDirNum = 0;
            SubDirInfo[CurDirDeep].Index = 0;
            SubDirInfo[CurDirDeep].CurDirNum++;
            SubDirInfo[CurDirDeep].DirClus = CurDirClus;
            CurDirDeep++;
            SubDirInfo[CurDirDeep].DirNum = TotalAllDir;
            CurDirClus = ChangeDir(0, FS_FAT);
        }

        SubDirInfo[CurDirDeep].TotalFile = GetTotalFiles(CurDirClus, ALLFileExtString, FS_FAT);   //add by lxs @2005.02.24
        SubDirInfo[CurDirDeep].TotalSubDir = GetTotalSubDir(CurDirClus, FS_FAT);     //��ȡ��Ŀ¼�µ���Ŀ¼��
        TotalAllDir += SubDirInfo[CurDirDeep].TotalSubDir;

 END:

    }while(CurDirDeep != 0);


    printf("deletedir totoaldir = %d, totalfile = %d\n", TotalDir, TotalDeleFile);

    gwSaveDirClus = gSaveClus;
    return RETURN_OK;

}


/*********************************************************************************************************
** Name        :ClearClus
** Description :clear the specified cluster to zero.
** Input       :Path
** Output      :RETURN_OK��success
** other reference:the return value explain in file fat.h�й��ڷ���ֵ��˵��
** global :FileInfo
** call module :AddFDT, GetDirClusIndex
********************************************************************************************************/
IRAM_ENCODE
uint8 ClearClus(uint32 Index)
{
    uint16 i;
    uint32 SecIndex;
    uint8  buf[512];

    memset(buf, 0, 512);

    if (Index < (BootSector.CountofClusters + 2))
    {
        SecIndex = ((Index - 2) << LogSecPerClus) + BootSector.FirstDataSector;

        for (i = 0; i < BootSector.BPB_SecPerClus; i++)
        {
            FATWriteSector(SecIndex++, buf);
        }
        return (RETURN_OK);
    }
    else
    {
        return (CLUSTER_NOT_IN_DISK);
    }

}

/*********************************************************************************************************
** Name        :MakeDir
** Description :create direction
** Input       :Path: DirFileName the path name use the 8.3 format.
** Output      :RETURN_OK���ɹ�
** other reference the return value explain in file fat.h
** global :FileInfo
** call module :AddFDT, GetDirClusIndex
********************************************************************************************************/
IRAM_ENCODE
uint8 MakeDir(uint8 *Path, uint8 *DirFileName)
{
    uint8 Rt;
    uint8 i;
    uint32 ClusIndex, Temp1;
    FDT temp;
    uint32 index;

    ClusIndex = GetDirClusIndex(Path);

    Rt = PATH_NOT_FIND;
    if (ClusIndex != BAD_CLUS)
    {
        for (i = 0; i < 11; i++)        //Ŀ¼����ո�
        {
            temp.Name[i] = ' ';
        }

        for (i = 0; i < 11; i++)
        {
            if (*DirFileName == '\0')   //��·������
            {
                break;
            }

            temp.Name[i] = *DirFileName++;
        }

        /* FDT�Ƿ���� */
        Rt = FDTIsLie(ClusIndex, temp.Name);
        if (Rt == NOT_FIND_FDT)
        {
            /* ������ */
            Temp1 = FATAddClus(0);                  /* ��ȡĿ¼������̿ռ� */
            Rt    = DISK_FULL;                      /* û�п��пռ� */

            if ((Temp1 > EMPTY_CLUS_1) && (Temp1 < BAD_CLUS))
            {
                ClearClus(Temp1);                       /* ��մ� */

                /* ����FDT���� */
                temp.Attr         = ATTR_DIRECTORY;
                temp.FileSize     = 0;
                temp.NTRes        = 0;
                temp.CrtTimeTenth = 0;
                temp.CrtTime      = 0;
                temp.CrtDate      = 0;
                temp.LstAccDate   = 0;
                temp.WrtTime      = 0;
                temp.WrtDate      = 0;
                temp.FstClusLO    = Temp1 & 0xffff;
                temp.FstClusHI    = Temp1 / 0x10000;

                Rt = AddFDT(ClusIndex, &temp, &index);       /* ����Ŀ¼�� */

                if (Rt == RETURN_OK)
                {
                    /* ����'.'Ŀ¼ */
                    temp.Name[0] = '.';
                    for (i = 1; i < 11; i++)
                    {
                        temp.Name[i] = ' ';
                    }
                    AddFDT(Temp1, &temp, &index);

                    /* ����'..'Ŀ¼ */
                    temp.Name[1] = '.';
                    if (ClusIndex == BootSector.BPB_RootClus)
                    {
                        ClusIndex = 0;
                    }

                    temp.FstClusLO = ClusIndex & 0xffff;
                    temp.FstClusHI = ClusIndex / 0x10000;
                    Rt = AddFDT(Temp1, &temp, &index);
                }
                else
                {
                    FATDelClusChain(Temp1);
                }

                Rt = RETURN_OK;
            }
        }
    }

    ///////////////////////////////////////////////////////////
    //��дFAT Cache
    if (gFatCache.Sec!=0 && gFatCache.Flag!=0)
    {
        gFatCache.Flag=0;
        FATWriteSector(gFatCache.Sec, gFatCache.Buf);
    }
    ///////////////////////////////////////////////////////////

    return (Rt);
}
#endif
/*
********************************************************************************
*
*                         End of Dir.c
*
********************************************************************************
*/

