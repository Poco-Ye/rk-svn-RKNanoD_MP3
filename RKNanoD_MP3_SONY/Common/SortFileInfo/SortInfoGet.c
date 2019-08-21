/*
********************************************************************************
*          Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                             All Rights Reserved
*                                    V1.00
* FileName   : SortInfoGet.c
* Author     :
* Description: ý���uiʱ����
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
  Description   : ��ȡ·����Ϣ�����ļ��ţ��Լ����ļ���

  Input         : pPathBuffer -- ��ȡ��·����Ϣ���buf
                  ulFullInfoSectorAddr --- flash�д����ϸ�ļ���Ϣ����ʼ��ַ
                  ulSortSectorAddr --- flash�д�ŷ���������Ϣ����ʼ��ַ
                  uiSortId -- ��Ҫ��ȡ���ļ��ڷ�����Ϣ�е������
                  Filenum -- ��ǰҪ��ȡ���ļ��������ļ��е����(�� 1 ��ʼ)���������ղؼ�������ļ����ж��Ƿ��ظ����
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
  Description   : ��ȡý�����ʾ��Ŀ����Ϣ(����������������ר���������ļ�����ID3Tilte)

  Input         : AddrInfo -- �ļ���Ϣ��ŵ�ַ�ṹ�����
                : uiSortId -- �ļ���
                : uiCharNum --- Ҫ��ȡ���ֽ���
                : uiCurDeep --- ͨ���ñ���ȷ��ƫ�Ƶ�ַ
                : Flag --- �ж��Ƿ��ȡ�ļ�����ID3Title Ϊ1��ʾ�Ƕ�ȡ�ļ�����ID3Title��Ϊ0��ʾ��ȡ����ID3��Ϣ
  Return        : pFileName Ҫ��ȡ���ļ����ļ�����ָ��

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
    UINT8 	FileInfoBuf[MEDIA_ID3_SAVE_CHAR_NUM *2];//���ļ�����ID3����ϢҪ������ѡ���ļ��ĳ��ȿ��ռ䲻���������Խ��

   SORTINFO_STRUCT *Subinfo = (SORTINFO_STRUCT *)ucBufTemp;

   if(Flag)
   {
        MDReadData(DataDiskID,(AddrInfo.ulFileSortInfoSectorAddr<<9)+(unsigned long)(uiSortId*2), 2, ucBufTemp);
        temp1 = (ucBufTemp[0]&0xff)+((ucBufTemp[1]&0xff)<<8); // ��ö�Ӧ���ļ������
   }
   else
   {
        //��ȡһ���������Ϣ�ṹ SORTINFO_STRUCT
        MDReadData(DataDiskID,(AddrInfo.ulSortSubInfoSectorAddr[uiCurDeep]<<9)+(unsigned long)(uiSortId*sizeof(SORTINFO_STRUCT)), sizeof(SORTINFO_STRUCT), ucBufTemp);
        //ͨ����Ϣ�ṹ�е� BaseID ������ȡ��Ҫ���ļ���
        MDReadData(DataDiskID,(AddrInfo.ulFileSortInfoSectorAddr<<9)+Subinfo->BaseID*2, 2, ucBufTemp1);
        temp1 = (ucBufTemp1[0]&0xff)+((ucBufTemp1[1]&0xff)<<8); // ��ö�Ӧ���ļ������
   }

    AddrOffset = (UINT32)(temp1)*BYTE_NUM_SAVE_PER_FILE + AddrInfo.uiSortInfoAddrOffset[uiCurDeep]; // ͨ��ƫ��λ����ȷ����ȡ���ǳ��ļ�����Ϣ����ID3Title��Ϣ

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
  Description   : ��ȡID3������Ϣ����Ŀ(ר���µ��ļ�����������ͬһ���������µ��ļ�����)

  Input         : ulSumSectorAddr ID3��Ϣ����洢��ַ
                : uiSumId  -- ��Ŀid
                : uiFindSumType -- ��ȡ��Ŀ��Ϣ�����ͣ�(��Ŀ�������ǣ���ʼ��Ŀ��ţ����Ǿ����ID3��Ϣ)
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ָ����Ӧ��Ϣ��flash�е�sec��ַ
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

    if(uiFindSumType==FIND_SUM_STARTFILEID)//��������Ϣ
    {
        uiSumInfo = Subinfo.BaseID;
    }
    else if(uiFindSumType==FIND_SUM_SORTSTART)//��ʼ�ļ���
    {
        uiSumInfo = Subinfo.ItemBaseID;
    }
    else if(uiFindSumType==FIND_SUM_ITEMNUM)//��Ŀ��Ϣ
    {
        uiSumInfo = Subinfo.ItemNum;
 	 //Rk Aaron.sun
 	 if((SortInfoAddr.uiSortInfoAddrOffset[MusicDirTreeInfo.MusicDirDeep] == ID3_ALBUM_SAVE_ADDR_OFFSET) &&  (MusicDirTreeInfo.MusicDirDeep != 0))
 	 {
		uiSumInfo++;  // for All Album
	 }
    }
    else if (uiFindSumType==FIND_SUM_FILENUM)//�ļ�����
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

