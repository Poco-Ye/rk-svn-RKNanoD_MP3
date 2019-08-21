/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��   FileInfo.h
*
* Description:
*
* History:      <author>          <time>        <version>
*                               2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _FILEINFO_H
#define _FILEINFO_H

#undef  EXT
#ifdef _IN_FILEINFOSAVE_
#define EXT
#else
#define EXT extern
#endif


/*
*-------------------------------------------------------------------------------
*
*                           Include File
*
*-------------------------------------------------------------------------------
*/

#define   _FILE_INFO_SAVE_CODE_    __attribute__((section("FileInfoSaveCode")))

#define   _FILE_INFO_SAVE_DATA_    __attribute__((section("FileInfoSaveData")))
#define   _FILE_INFO_SAVE_BSS_     __attribute__((section("FileInfoSaveBss"),zero_init))


#define   _FILE_INFO_SORT_CODE_      __attribute__((section("FileInfoSortCode")))
#define   _FILE_INFO_SORT_DATA_      __attribute__((section("FileInfoSortData")))
#define   _FILE_INFO_SORT_BSS_       __attribute__((section("FileInfoSortBss"),zero_init))


extern void SearchAndSaveMusicInfo(void);

#ifdef PIC_MEDIA
extern void SearchAndSaveJpegInfo(void);//sanshin
#endif

extern void SearchAndSaveM3uInfo(void);//<----sanshin_20150616

extern void SortUpdateFun(MEDIALIB_CONFIG * Sysfilenum ,UINT32 MediaInfoAddress);

/*
********************************************************************************
*
*                         End of FileInfo.h
*
********************************************************************************
*/
#endif


