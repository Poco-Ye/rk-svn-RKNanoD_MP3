/*
********************************************************************************
*          Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                             All Rights Reserved
*                                    V1.00
* FileName   : SortInfoGet.c
* Author     :
* Description: 媒体库ui时调用
* History    :
*           <author>        <time>     <version>       <desc>
*            azg            06/08/09       1.0            ORG
*
********************************************************************************
*/


#include "SysInclude.h"

#include "FsInclude.h"

#include "AddrSaveMacro.h"
#include "FileInfo.h"
#include "MediaBroWin.h"
#include "PicBroWin.h"
#include "M3uBroWin.h"//<----sanshin_20150616

#include "SortInfoGetMacro.h"
#include "AudioControl.h"
#ifdef _MEDIA_MODULE_

_ATTR_MEDIABROWIN_BSS_ SORTINFO_STRUCT Subinfo;

/*
--------------------------------------------------------------------------------
  Function name : void GetSavedMusicPath(UINT8 *pPathBuffer, UINT32 ulFullInfoSectorAddr, UINT32 ulSortSectorAddr, UINT16 uiSortId)

  Author        : anzhiguo
  Description   : 获取路径信息，，文件号，以及长文件名

  Input         : pPathBuffer -- 获取的路径信息存放buf
                  ulFullInfoSectorAddr --- flash中存放详细文件信息的起始地址
                  ulSortSectorAddr --- flash中存放分类排序信息的起始地址
                  uiSortId -- 需要获取的文件在分类信息中的排序号
                  Filenum -- 当前要获取的文件在所有文件中的序号(从 1 开始)，用于在收藏夹中添加文件是判断是否重复添加
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void GetSavedMusicDir(FIND_DATA * pFindData, UINT32 ulFullInfoSectorAddr, UINT32 ulSortSectorAddr, UINT16 uiSortId )
{
    UINT32 temp1;
    UINT8 ucBufTemp[2];
    MDReadData(DataDiskID,(ulSortSectorAddr<<9)+((UINT32)uiSortId)*2, 2, ucBufTemp);
    temp1 = (UINT16)ucBufTemp[0]+(((UINT16)ucBufTemp[1])<<8);
    MDReadData(DataDiskID,(ulFullInfoSectorAddr<<9)+(UINT32)(temp1)*BYTE_NUM_SAVE_PER_FILE+(UINT32)DIR_CLUS_SAVE_ADDR_OFFSET, 8 , (uint8 *)pFindData);
}
/*
--------------------------------------------------------------------------------
  Function name :
  void GetSavedMusicFileName(unsigned char *pFileName, SORT_INFO_ADDR_STRUCT AddrInfo, unsigned int uiSortId, unsigned int uiCharNum, unsigned int uiCurDeep)
  Author        : anzhiguo
  Description   : 获取媒体库显示条目的信息(流派名，歌手名，专辑名，长文件名，ID3Tilte)

  Input         : AddrInfo -- 文件信息存放地址结构体变量
                : uiSortId -- 文件号
                : uiCharNum --- 要读取的字节数
                : uiCurDeep --- 通过该变量确定偏移地址
                : Flag --- 判断是否读取文件名或ID3Title 为1表示是读取文件名或ID3Title，为0表示读取其他ID3信息
  Return        : pFileName 要获取的文件长文件名的指针

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_MEDIABROWIN_CODE_
void GetMediaItemInfo(UINT16 *pFileName, SORT_INFO_ADDR_STRUCT AddrInfo, UINT16 uiSortId, UINT16 uiCharNum, UINT16 uiCurDeep, UINT16 Flag)
{
    UINT16 i;
    UINT16 temp1;
    UINT8   ucBufTemp[8],ucBufTemp1[2];
    UINT32  AddrOffset;
    UINT8 	FileInfoBuf[MEDIA_ID3_SAVE_CHAR_NUM *2];//长文件名比ID3的信息要长，故选择长文件的长度开空间不会出现数组越界

   SORTINFO_STRUCT *Subinfo = (SORTINFO_STRUCT *)ucBufTemp;

   if(Flag)
   {
        MDReadData(DataDiskID,(AddrInfo.ulFileSortInfoSectorAddr<<9)+(unsigned long)(uiSortId*2), 2, ucBufTemp);
        temp1 = (ucBufTemp[0]&0xff)+((ucBufTemp[1]&0xff)<<8); // 获得对应的文件保存号
   }
   else
   {
        //获取一个子项的信息结构 SORTINFO_STRUCT
        MDReadData(DataDiskID,(AddrInfo.ulSortSubInfoSectorAddr[uiCurDeep]<<9)+(unsigned long)(uiSortId*sizeof(SORTINFO_STRUCT)), sizeof(SORTINFO_STRUCT), ucBufTemp);
        //通过信息结构中的 BaseID 参数获取需要的文件号
        MDReadData(DataDiskID,(AddrInfo.ulFileSortInfoSectorAddr<<9)+Subinfo->BaseID*2, 2, ucBufTemp1);
        temp1 = (ucBufTemp1[0]&0xff)+((ucBufTemp1[1]&0xff)<<8); // 获得对应的文件保存号
   }

    AddrOffset = (UINT32)(temp1)*BYTE_NUM_SAVE_PER_FILE + AddrInfo.uiSortInfoAddrOffset[uiCurDeep]; // 通过偏移位置来确定读取的是长文件名信息还是ID3Title信息

    MDReadData(DataDiskID,(AddrInfo.ulFileFullInfoSectorAddr<<9)+AddrOffset, uiCharNum*2, FileInfoBuf);

    for(i=0;i<uiCharNum;i++)
    {
        *pFileName++ = (UINT16)FileInfoBuf[2*i]+((UINT16)FileInfoBuf[2*i+1]<<8);
    }

}


/*
--------------------------------------------------------------------------------
  Function name : unsigned int GetSummaryInfo(unsigned long ulSumSectorAddr, unsigned int uiSumId, unsigned int uiFindSumType)
{
  Author        : anzhiguo
  Description   : 获取ID3归类信息的条目(专辑下的文件个数，或是同一个艺术家下的文件个数)

  Input         : ulSumSectorAddr ID3信息归类存储地址
                : uiSumId  -- 条目id
                : uiFindSumType -- 获取条目信息的类型，(条目总数还是，起始条目序号，还是具体的ID3信息)
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         指到对应信息在flash中的sec地址
--------------------------------------------------------------------------------
*/
_ATTR_MEDIABROWIN_CODE_
UINT16 GetSummaryInfo(UINT32 ulSumSectorAddr, UINT16 uiSumId, UINT16 uiFindSumType)
{
    UINT16 uiSumInfo;
    UINT8 	*ucBufTemp;
    UINT32 uiSumTypeOffset;
   ucBufTemp = (UINT8 *)(&Subinfo);

    MDReadData(DataDiskID,(ulSumSectorAddr<<9)+(UINT32)uiSumId*8, 8, ucBufTemp);

    if(uiFindSumType==FIND_SUM_STARTFILEID)//完整的信息
    {
        uiSumInfo = Subinfo.BaseID;
    }
    else if(uiFindSumType==FIND_SUM_SORTSTART)//起始文件号
    {
        uiSumInfo = Subinfo.ItemBaseID;
    }
    else if(uiFindSumType==FIND_SUM_ITEMNUM)//条目信息
    {
        uiSumInfo = Subinfo.ItemNum;
 	 //Rk Aaron.sun
 	 if((SortInfoAddr.uiSortInfoAddrOffset[MusicDirTreeInfo.MusicDirDeep] == ID3_ALBUM_SAVE_ADDR_OFFSET) &&  (MusicDirTreeInfo.MusicDirDeep != 0))
 	 {
		uiSumInfo++;  // for All Album
	 }
    }
    else if (uiFindSumType==FIND_SUM_FILENUM)//文件总数
    {
        uiSumInfo = Subinfo.FileNum;
    }

    return uiSumInfo;
}




#ifdef PIC_MEDIA

/////sanshin----->

_ATTR_PICBROWIN_BSS_ SORTINFO_STRUCT JpegSubinfo;

/*
--------------------------------------------------------------------------------
  Function name : void GetSavedMusicPath(UINT8 *pPathBuffer, UINT32 ulFullInfoSectorAddr, UINT32 ulSortSectorAddr, UINT16 uiSortId)

  Author        : anzhiguo
  Description   : ;qH!B7>6PEO"#,#,ND<~:E#,RT<03$ND<~C{

  Input         : pPathBuffer -- ;qH!5DB7>6PEO"4f7Ebuf
                  ulFullInfoSectorAddr --- flashVP4f7EOjO8ND<~PEO"5DFpJ<5XV7
                  ulSortSectorAddr --- flashVP4f7E7V@`EEPrPEO"5DFpJ<5XV7
                  uiSortId -- PhR*;qH!5DND<~TZ7V@`PEO"VP5DEEPr:E
                  Filenum -- 51G0R*;qH!5DND<~TZKySPND<~VP5DPr:E(4S 1 ?*J<)#,SCSZTZJU2X<PVPLm<SND<~JGEP6OJG7qVX84Lm<S
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void PicBroGetSavedJpegDir(FIND_DATA * pFindData, UINT32 ulFullInfoSectorAddr, UINT32 ulSortSectorAddr, UINT16 uiSortId )
{
    UINT32 temp1;
    UINT8 ucBufTemp[2];
    MDReadData(DataDiskID,(ulSortSectorAddr<<9)+((UINT32)uiSortId)*2, 2, ucBufTemp);
    temp1 = (UINT16)ucBufTemp[0]+(((UINT16)ucBufTemp[1])<<8);
    MDReadData(DataDiskID,(ulFullInfoSectorAddr<<9)+(UINT32)(temp1)*BYTE_NUM_SAVE_PER_FILE+(UINT32)DIR_CLUS_SAVE_ADDR_OFFSET, 8 , (uint8 *)pFindData);
}
/*
--------------------------------------------------------------------------------
  Function name :
  void GetSavedMusicFileName(unsigned char *pFileName, SORT_INFO_ADDR_STRUCT AddrInfo, unsigned int uiSortId, unsigned int uiCharNum, unsigned int uiCurDeep)
  Author        : anzhiguo
  Description   : ;qH!C=Le?bOTJ>LuD?5DPEO"(AwEIC{#,8hJVC{#,W(<-C{#,3$ND<~C{#,ID3Tilte)

  Input         : AddrInfo -- ND<~PEO"4f7E5XV7=a99Le1dA?
                : uiSortId -- ND<~:E
                : uiCharNum --- R*6AH!5DWV=ZJ}
                : uiCurDeep --- M(9}8C1dA?H76(F+RF5XV7
                : Flag --- EP6OJG7q6AH!ND<~C{;rID3Title N*11mJ>JG6AH!ND<~C{;rID3Title#,N*01mJ>6AH!FdK{ID3PEO"
  Return        : pFileName R*;qH!5DND<~3$ND<~C{5DV8Uk

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_PICBROWIN_CODE_
void PicBroGetMediaItemInfo(UINT16 *pFileName, SORT_INFO_ADDR_STRUCT AddrInfo, UINT16 uiSortId, UINT16 uiCharNum, UINT16 uiCurDeep, UINT16 Flag)
{
    UINT16 i;
    UINT16 temp1;
    UINT8   ucBufTemp[8],ucBufTemp1[2];
    UINT32  AddrOffset;
    UINT8 	FileInfoBuf[MEDIA_ID3_SAVE_CHAR_NUM *2];//3$ND<~C{1HID35DPEO"R*3$#,9JQ!Tq3$ND<~5D3$6H?*?U<d2;;a3vOVJ}WiT==g

   SORTINFO_STRUCT *jSubinfo = (SORTINFO_STRUCT *)ucBufTemp;

   if(Flag)
   {
        //printf("AddrOffset1 = %x\n", AddrInfo.ulFileSortInfoSectorAddr);
        //printf("AxxxxAddr = %x\n",  SortInfoAddr.ulFileSortInfoSectorAddr);
        MDReadData(DataDiskID,(AddrInfo.ulFileSortInfoSectorAddr<<9)+(unsigned long)(uiSortId*2), 2, ucBufTemp);
        temp1 = (ucBufTemp[0]&0xff)+((ucBufTemp[1]&0xff)<<8); // ;q5C6TS&5DND<~1#4f:E
   }
   else
   {
        //;qH!R;8vWSOn5DPEO"=a99 SORTINFO_STRUCT
        MDReadData(DataDiskID,(AddrInfo.ulSortSubInfoSectorAddr[uiCurDeep]<<9)+(unsigned long)(uiSortId*sizeof(SORTINFO_STRUCT)), sizeof(SORTINFO_STRUCT), ucBufTemp);
        //M(9}PEO"=a99VP5D BaseID 2NJ};qH!PhR*5DND<~:E
        MDReadData(DataDiskID,(AddrInfo.ulFileSortInfoSectorAddr<<9)+jSubinfo->BaseID*2, 2, ucBufTemp1);
        temp1 = (ucBufTemp1[0]&0xff)+((ucBufTemp1[1]&0xff)<<8); // ;q5C6TS&5DND<~1#4f:E
   }

    AddrOffset = (UINT32)(temp1)*BYTE_NUM_SAVE_PER_FILE + AddrInfo.uiSortInfoAddrOffset[uiCurDeep]; // M(9}F+RFN;VC@4H76(6AH!5DJG3$ND<~C{PEO";9JGID3TitlePEO"
    //printf("num = %d\n", temp1);
    //printf("offset = %d, deep = %d\n", AddrInfo.uiSortInfoAddrOffset[uiCurDeep], uiCurDeep);
    //printf("fulladdr = %d\n", AddrInfo.ulFileFullInfoSectorAddr);

    MDReadData(DataDiskID,(AddrInfo.ulFileFullInfoSectorAddr<<9)+AddrOffset, uiCharNum*2, FileInfoBuf);

    for(i=0;i<uiCharNum;i++)
    {
        *pFileName++ = (UINT16)FileInfoBuf[2*i]+((UINT16)FileInfoBuf[2*i+1]<<8);
    }

}


/*
--------------------------------------------------------------------------------
  Function name : unsigned int GetSummaryInfo(unsigned long ulSumSectorAddr, unsigned int uiSumId, unsigned int uiFindSumType)
{
  Author        : anzhiguo
  Description   : ;qH!ID39i@`PEO"5DLuD?(W(<-OB5DND<~8vJ}#,;rJGM,R;8vRUJu<ROB5DND<~8vJ})

  Input         : ulSumSectorAddr ID3PEO"9i@`4f4"5XV7
                : uiSumId  -- LuD?id
                : uiFindSumType -- ;qH!LuD?PEO"5D@`PM#,(LuD?W\J};9JG#,FpJ<LuD?Pr:E#,;9JG>_Le5DID3PEO")
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         V85=6TS&PEO"TZflashVP5Dsec5XV7
--------------------------------------------------------------------------------
*/
_ATTR_PICBROWIN_CODE_
UINT16 PicBroGetSummaryInfo(UINT32 ulSumSectorAddr, UINT16 uiSumId, UINT16 uiFindSumType)
{
    UINT16 uiSumInfo;
    UINT8 	*ucBufTemp;
    UINT32 uiSumTypeOffset;
   // SORTINFO_STRUCT *Subinfo = (SORTINFO_STRUCT *)ucBufTemp;
   ucBufTemp = (UINT8 *)(&JpegSubinfo);

    MDReadData(DataDiskID,(ulSumSectorAddr<<9)+(UINT32)uiSumId*8, 8, ucBufTemp);
    //uiSumInfo = (ucBufTemp[0]&0xff)+((ucBufTemp[1]&0xff)<<8);

    if(uiFindSumType==FIND_SUM_STARTFILEID)//MjU{5DPEO"
    {
        uiSumInfo = JpegSubinfo.BaseID;
    }
    else if(uiFindSumType==FIND_SUM_SORTSTART)//FpJ<ND<~:E
    {
        uiSumInfo = JpegSubinfo.ItemBaseID;
    }
    else if(uiFindSumType==FIND_SUM_ITEMNUM)//LuD?PEO"
    {
        uiSumInfo = JpegSubinfo.ItemNum;
	 //Rk Aaron.sun
	 if((SortInfoAddr.uiSortInfoAddrOffset[MusicDirTreeInfo.MusicDirDeep] == ID3_ALBUM_SAVE_ADDR_OFFSET) &&  (MusicDirTreeInfo.MusicDirDeep != 0))
	 {
		uiSumInfo++;  // for All Album
	 }
    }
    else if (uiFindSumType==FIND_SUM_FILENUM)//ND<~W\J}
    {
        uiSumInfo = JpegSubinfo.FileNum;
    }

    return uiSumInfo;
}


/////<-----sanshin
#endif

//---->sanshin_20150616
_ATTR_M3UBROWIN_BSS_ SORTINFO_STRUCT M3uSubinfo;																															//<----sanshin_20150616
                                                                                                                                                                            //<----sanshin_20150616
/*                                                                                                                                                                          //<----sanshin_20150616
--------------------------------------------------------------------------------                                                                                            //<----sanshin_20150616
  Function name : void  M3uBroGetSavedM3uDir(FIND_DATA * pFindData, UINT32 ulFullInfoSectorAddr, UINT32 ulSortSectorAddr, UINT16 uiSortId )                                 //<----sanshin_20150616
                                                                                                                                                                            //<----sanshin_20150616
  Author        : anzhiguo                                                                                                                                                  //<----sanshin_20150616
  Description   : ;qH!B7>6PEO"#,#,ND<~:E#,RT<03$ND<~C{                                                                                                                      //<----sanshin_20150616
                                                                                                                                                                            //<----sanshin_20150616
  Input         : pPathBuffer -- ;qH!5DB7>6PEO"4f7Ebuf                                                                                                                      //<----sanshin_20150616
                  ulFullInfoSectorAddr --- flashVP4f7EOjO8ND<~PEO"5DFpJ<5XV7                                                                                                //<----sanshin_20150616
                  ulSortSectorAddr --- flashVP4f7E7V@`EEPrPEO"5DFpJ<5XV7                                                                                                    //<----sanshin_20150616
                  uiSortId -- PhR*;qH!5DND<~TZ7V@`PEO"VP5DEEPr:E                                                                                                            //<----sanshin_20150616
                  Filenum -- 51G0R*;qH!5DND<~TZKySPND<~VP5DPr:E(4S 1 ?*J<)#,SCSZTZJU2X<PVPLm<SND<~JGEP6OJG7qVX84Lm<S                                                        //<----sanshin_20150616
  Return        :                                                                                                                                                           //<----sanshin_20150616
                                                                                                                                                                            //<----sanshin_20150616
  History:     <author>         <time>         <version>                                                                                                                    //<----sanshin_20150616
                anzhiguo     2009/06/02         Ver1.0                                                                                                                      //<----sanshin_20150616
  desc:                                                                                                                                                                     //<----sanshin_20150616
--------------------------------------------------------------------------------                                                                                            //<----sanshin_20150616
*/                                                                                                                                                                          //<----sanshin_20150616
_ATTR_SYS_CODE_                                                                                                                                                             //<----sanshin_20150616
void  M3uBroGetSavedM3uDir(FIND_DATA * pFindData, UINT32 ulFullInfoSectorAddr, UINT32 ulSortSectorAddr, UINT16 uiSortId )                                                   //<----sanshin_20150616
{                                                                                                                                                                           //<----sanshin_20150616
    UINT32 temp1;                                                                                                                                                           //<----sanshin_20150616
    UINT8 ucBufTemp[2];                                                                                                                                                     //<----sanshin_20150616
    MDReadData(DataDiskID,(ulSortSectorAddr<<9)+((UINT32)uiSortId)*2, 2, ucBufTemp);                                                                                        //<----sanshin_20150616
    temp1 = (UINT16)ucBufTemp[0]+(((UINT16)ucBufTemp[1])<<8);                                                                                                               //<----sanshin_20150616
    MDReadData(DataDiskID,(ulFullInfoSectorAddr<<9)+(UINT32)(temp1)*BYTE_NUM_SAVE_PER_FILE+(UINT32)DIR_CLUS_SAVE_ADDR_OFFSET, 8 , (uint8 *)pFindData);                      //<----sanshin_20150616
}                                                                                                                                                                           //<----sanshin_20150616
/*                                                                                                                                                                          //<----sanshin_20150616
--------------------------------------------------------------------------------                                                                                            //<----sanshin_20150616
  Function name :                                                                                                                                                           //<----sanshin_20150616
  void GetM3uItemInfo(UINT16 *pFileName, SORT_INFO_ADDR_STRUCT AddrInfo, UINT16 uiSortId, UINT16 uiCharNum, UINT16 uiCurDeep, UINT16 Flag)                                  //<----sanshin_20150616
  Author        : anzhiguo                                                                                                                                                  //<----sanshin_20150616
  Description   : ;qH!C=Le?bOTJ>LuD?5DPEO"(AwEIC{#,8hJVC{#,W(<-C{#,3$ND<~C{#,ID3Tilte)                                                                                      //<----sanshin_20150616
                                                                                                                                                                            //<----sanshin_20150616
  Input         : AddrInfo -- ND<~PEO"4f7E5XV7=a99Le1dA?                                                                                                                    //<----sanshin_20150616
                : uiSortId -- ND<~:E                                                                                                                                        //<----sanshin_20150616
                : uiCharNum --- R*6AH!5DWV=ZJ}                                                                                                                              //<----sanshin_20150616
                : uiCurDeep --- M(9}8C1dA?H76(F+RF5XV7                                                                                                                      //<----sanshin_20150616
                : Flag --- EP6OJG7q6AH!ND<~C{;rID3Title N*11mJ>JG6AH!ND<~C{;rID3Title#,N*01mJ>6AH!FdK{ID3PEO"                                                               //<----sanshin_20150616
  Return        : pFileName R*;qH!5DND<~3$ND<~C{5DV8Uk                                                                                                                      //<----sanshin_20150616
                                                                                                                                                                            //<----sanshin_20150616
  History:     <author>         <time>         <version>                                                                                                                    //<----sanshin_20150616
                anzhiguo     2009/06/02         Ver1.0                                                                                                                      //<----sanshin_20150616
  desc:                                                                                                                                                                     //<----sanshin_20150616
--------------------------------------------------------------------------------                                                                                            //<----sanshin_20150616
*/                                                                                                                                                                          //<----sanshin_20150616
_ATTR_M3UBROWIN_CODE_                                                                                                                                                       //<----sanshin_20150616
void M3uBroGetMediaItemInfo(UINT16 *pFileName, SORT_INFO_ADDR_STRUCT AddrInfo, UINT16 uiSortId, UINT16 uiCharNum, UINT16 uiCurDeep, UINT16 Flag)                            //<----sanshin_20150616
{                                                                                                                                                                           //<----sanshin_20150616
    UINT16 i;                                                                                                                                                               //<----sanshin_20150616
    UINT16 temp1;                                                                                                                                                           //<----sanshin_20150616
    UINT8   ucBufTemp[8],ucBufTemp1[2];                                                                                                                                     //<----sanshin_20150616
    UINT32  AddrOffset;                                                                                                                                                     //<----sanshin_20150616
    UINT8 	FileInfoBuf[MEDIA_ID3_SAVE_CHAR_NUM *2];                                                                                                                        //<----sanshin_20150616
                                                                                                                                                                            //<----sanshin_20150616
   SORTINFO_STRUCT *jSubinfo = (SORTINFO_STRUCT *)ucBufTemp;                                                                                                                //<----sanshin_20150616
                                                                                                                                                                            //<----sanshin_20150616
   if(Flag)                                                                                                                                                                 //<----sanshin_20150616
   {                                                                                                                                                                        //<----sanshin_20150616
        MDReadData(DataDiskID,(AddrInfo.ulFileSortInfoSectorAddr<<9)+(unsigned long)(uiSortId*2), 2, ucBufTemp);                                                            //<----sanshin_20150616
        temp1 = (ucBufTemp[0]&0xff)+((ucBufTemp[1]&0xff)<<8);                                                                                                               //<----sanshin_20150616
   }                                                                                                                                                                        //<----sanshin_20150616
   else                                                                                                                                                                     //<----sanshin_20150616
   {                                                                                                                                                                        //<----sanshin_20150616
        MDReadData(DataDiskID,(AddrInfo.ulSortSubInfoSectorAddr[uiCurDeep]<<9)+(unsigned long)(uiSortId*sizeof(SORTINFO_STRUCT)), sizeof(SORTINFO_STRUCT), ucBufTemp);      //<----sanshin_20150616
        MDReadData(DataDiskID,(AddrInfo.ulFileSortInfoSectorAddr<<9)+jSubinfo->BaseID*2, 2, ucBufTemp1);                                                                    //<----sanshin_20150616
        temp1 = (ucBufTemp1[0]&0xff)+((ucBufTemp1[1]&0xff)<<8);                                                                                                             //<----sanshin_20150616
   }                                                                                                                                                                        //<----sanshin_20150616
                                                                                                                                                                            //<----sanshin_20150616
    AddrOffset = (UINT32)(temp1)*BYTE_NUM_SAVE_PER_FILE + AddrInfo.uiSortInfoAddrOffset[uiCurDeep];                                                                         //<----sanshin_20150616
    MDReadData(DataDiskID,(AddrInfo.ulFileFullInfoSectorAddr<<9)+AddrOffset, uiCharNum*2, FileInfoBuf);                                                                     //<----sanshin_20150616
                                                                                                                                                                            //<----sanshin_20150616
    for(i=0;i<uiCharNum;i++)                                                                                                                                                //<----sanshin_20150616
    {                                                                                                                                                                       //<----sanshin_20150616
        *pFileName++ = (UINT16)FileInfoBuf[2*i]+((UINT16)FileInfoBuf[2*i+1]<<8);                                                                                            //<----sanshin_20150616
    }                                                                                                                                                                       //<----sanshin_20150616
                                                                                                                                                                            //<----sanshin_20150616
}                                                                                                                                                                           //<----sanshin_20150616
                                                                                                                                                                            //<----sanshin_20150616
                                                                                                                                                                            //<----sanshin_20150616
/*                                                                                                                                                                          //<----sanshin_20150616
--------------------------------------------------------------------------------                                                                                            //<----sanshin_20150616
  Function name : UINT16 M3uGetSummaryInfo(UINT32 ulSumSectorAddr, UINT16 uiSumId, UINT16 uiFindSumType)                                                                    //<----sanshin_20150616
{                                                                                                                                                                           //<----sanshin_20150616
  Author        : anzhiguo                                                                                                                                                  //<----sanshin_20150616
  Description   : ;qH!ID39i@`PEO"5DLuD?(W(<-OB5DND<~8vJ}#,;rJGM,R;8vRUJu<ROB5DND<~8vJ})                                                                                     //<----sanshin_20150616
                                                                                                                                                                            //<----sanshin_20150616
  Input         : ulSumSectorAddr ID3PEO"9i@`4f4"5XV7                                                                                                                       //<----sanshin_20150616
                : uiSumId  -- LuD?id                                                                                                                                        //<----sanshin_20150616
                : uiFindSumType -- ;qH!LuD?PEO"5D@`PM#,(LuD?W\J};9JG#,FpJ<LuD?Pr:E#,;9JG>_Le5DID3PEO")                                                                      //<----sanshin_20150616
  Return        :                                                                                                                                                           //<----sanshin_20150616
                                                                                                                                                                            //<----sanshin_20150616
  History:     <author>         <time>         <version>                                                                                                                    //<----sanshin_20150616
                anzhiguo     2009/06/02         Ver1.0                                                                                                                      //<----sanshin_20150616
  desc:         V85=6TS&PEO"TZflashVP5Dsec5XV7                                                                                                                              //<----sanshin_20150616
--------------------------------------------------------------------------------                                                                                            //<----sanshin_20150616
*/                                                                                                                                                                          //<----sanshin_20150616
_ATTR_M3UBROWIN_CODE_                                                                                                                                                       //<----sanshin_20150616
UINT16 M3uGetSummaryInfo(UINT32 ulSumSectorAddr, UINT16 uiSumId, UINT16 uiFindSumType)                                                                                      //<----sanshin_20150616
{                                                                                                                                                                           //<----sanshin_20150616
    UINT16 uiSumInfo;                                                                                                                                                       //<----sanshin_20150616
    UINT8  *ucBufTemp;                                                                                                                                                      //<----sanshin_20150616
    UINT32 uiSumTypeOffset;                                                                                                                                                 //<----sanshin_20150616
                                                                                                                                                                            //<----sanshin_20150616
	ucBufTemp = (UINT8 *)(&M3uSubinfo);                                                                                                                                     //<----sanshin_20150616
                                                                                                                                                                            //<----sanshin_20150616
    MDReadData(DataDiskID,(ulSumSectorAddr<<9)+(UINT32)uiSumId*8, 8, ucBufTemp);                                                                                            //<----sanshin_20150616
                                                                                                                                                                            //<----sanshin_20150616
    if(uiFindSumType==FIND_SUM_STARTFILEID)                                                                                                                                 //<----sanshin_20150616
    {                                                                                                                                                                       //<----sanshin_20150616
        uiSumInfo = M3uSubinfo.BaseID;                                                                                                                                      //<----sanshin_20150616
    }                                                                                                                                                                       //<----sanshin_20150616
    else if(uiFindSumType==FIND_SUM_SORTSTART)                                                                                                                              //<----sanshin_20150616
    {                                                                                                                                                                       //<----sanshin_20150616
        uiSumInfo = M3uSubinfo.ItemBaseID;                                                                                                                                  //<----sanshin_20150616
    }                                                                                                                                                                       //<----sanshin_20150616
    else if(uiFindSumType==FIND_SUM_ITEMNUM)                                                                                                                                //<----sanshin_20150616
    {                                                                                                                                                                       //<----sanshin_20150616
        uiSumInfo = M3uSubinfo.ItemNum;                                                                                                                                     //<----sanshin_20150616
                                                                                                                                                                            //<----sanshin_20150616
		if((SortInfoAddr.uiSortInfoAddrOffset[MusicDirTreeInfo.MusicDirDeep] == ID3_ALBUM_SAVE_ADDR_OFFSET) &&  (MusicDirTreeInfo.MusicDirDeep != 0))                       //<----sanshin_20150616
		{                                                                                                                                                                   //<----sanshin_20150616
			uiSumInfo++;  // for All Album                                                                                                                                  //<----sanshin_20150616
		}                                                                                                                                                                   //<----sanshin_20150616
    }                                                                                                                                                                       //<----sanshin_20150616
    else if (uiFindSumType==FIND_SUM_FILENUM)                                                                                                                               //<----sanshin_20150616
    {                                                                                                                                                                       //<----sanshin_20150616
        uiSumInfo = M3uSubinfo.FileNum;                                                                                                                                     //<----sanshin_20150616
    }                                                                                                                                                                       //<----sanshin_20150616
}																																											//<----sanshin_20150616
//<----sanshin_20150616

#endif

