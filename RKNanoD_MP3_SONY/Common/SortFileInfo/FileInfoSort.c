/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Nameˇ   FileInfoSort.C
*
* Description:
*
* History:      <author>          <time>        <version>
*               anzhiguo      2009-06-03          1.0
*    desc:      ORG.
********************************************************************************
*/
#define  _IN_FILEINFOSORT_
#include "SysInclude.h"
#include "FsInclude.h"
#include "FileInfo.h"

#include "AddrSaveMacro.h"

#include "MDBBuildWin.h"//PAGE

#ifdef MEDIA_UPDATE

#define   SORT_TYPE_ITEM_NUM  4   /* 定义排序类型个数 */

#define     SORT_FILENAME_LEN               (4)  /* 从文件名中截取的参与排序字符的个数，目前的16k的排序空间最大可以保存8*1024个文件的排序信息*/

#define     CHILD_CHAIN_NUM             72 /* 确定排序子链表的个数 */

//#define   FILE_NAME_TYPE    0       //这个顺序必须和AddrSaveMacro.h中定义的存储顺序一致
#define   ID3_TITLE_TYPE    0
#define   ID3_ALBUM_TYPE    1
#define   ID3_ARTIST_TYPE   2
#define   ID3_GENRE_TYPE    3


typedef struct _FILE_INFO_ADD_STRUCT
{

    UINT32  add1;//第一层排序信息写Flash地址
    UINT32  add2;//第二层排序信息写Flash地址
    UINT32  add3;//第三层排序信息写Flash地址
    UINT32  add4;//第四层排序信息写Flash地址

}FILE_INFO_ADD_STRUCT; /* 文件待排序信息,define by phc*/

typedef __packed struct _FILE_INFO_INDEX_STRUCT
{

    struct  _FILE_INFO_INDEX_STRUCT   *pNext;
    UINT16  SortFileName[SORT_FILENAME_LEN + 1];

}FILE_INFO_INDEX_STRUCT; /* 文件待排序信息,define by phc*/


_FILE_INFO_SORT_CODE_   UINT16 gChildChainDivValue[CHILD_CHAIN_NUM];//72*2字节  存放各个子链表的首个元属的第一个字母ˇ要比较的unicode 通过与该数组元属对比找个对应的子链表
_FILE_INFO_SORT_BSS_    FILE_INFO_INDEX_STRUCT  *pChildChainHead[CHILD_CHAIN_NUM]; //72*4字节 各排序个子链表的头节点的指针
_FILE_INFO_SORT_BSS_    FILE_INFO_INDEX_STRUCT  gChildChainHead[CHILD_CHAIN_NUM];  //72*8字节 各排序个子链表的头结点

//sanshin...>
_FILE_INFO_SORT_CODE_   FILE_INFO_INDEX_STRUCT  *pChildChainLast;  //72*8字节 各排序个子链表的头结点
_FILE_INFO_SORT_CODE_   FILE_INFO_INDEX_STRUCT  *pChildChainLast2;  //72*8字节 各排序个子链表的头结点
//<...sanshin


/* 注意ˇ以下两个数组物理地址必须ˇ连ˇ具体实ˇ可将其紧挨在一起定义 */
_FILE_INFO_SORT_BSS_    FILE_INFO_INDEX_STRUCT  gSortNameBuffer0[SORT_FILENUM_DEFINE];//1536*8字节 保存截取的文件名信息ˇ常驻于内存ˇ排序时调用, Buffer0
_FILE_INFO_SORT_BSS_   UINT32 gFileSortBufferIndex;
_FILE_INFO_SORT_CODE_  __align(4) UINT8 gFileSortBuffer[MEDIAINFO_PAGE_SIZE];

_FILE_INFO_SORT_CODE_  UINT16 TempBuf1[MEDIA_ID3_SAVE_CHAR_NUM];
_FILE_INFO_SORT_CODE_  UINT16 TempBuf2[MEDIA_ID3_SAVE_CHAR_NUM];
_FILE_INFO_SORT_CODE_  UINT32 gBkMDReadAdrs1;
_FILE_INFO_SORT_CODE_  UINT32 gBkMDReadAdrs2;
_FILE_INFO_SORT_DATA_   UINT32 gwSectorOffset;
_FILE_INFO_SORT_DATA_   UINT32 gwDataBaseAddr;

_FILE_INFO_SORT_CODE_ __align(4) UINT8 SingerBuffer[SORT_FILENUM_DEFINE * sizeof(SORTINFO_STRUCT)]; //16k 字节 歌手分类信息buf 因为歌手数ˇ制为2048所以这个buf开16k
_FILE_INFO_SORT_CODE_ __align(4) UINT8  GerneAblumBuffer[SORT_FILENUM_DEFINE * sizeof(SORTINFO_STRUCT)]; //8k  流派专辑共用buf 记录分类信息ˇ因为专辑数ˇ制为1024所以这个buf开8k
_FILE_INFO_SORT_BSS_ __align(4) UINT8   FileIDBuffer[2 * SORT_FILENUM_DEFINE]; //记录排序文件号 2010.05.17 由于28时发ˇ媒体库排序buff空间被写flash冲掉ˇ修改脚本后引起空间不足而修改
_FILE_INFO_SORT_BSS_ __align(4) UINT8   FileIDBuffer1[2 * SORT_FILENUM_DEFINE]; //记录排序文件号

_FILE_INFO_SORT_CODE_   UINT32  MediaInfoSaveAdd[4];
_FILE_INFO_SORT_CODE_   UINT32  MediaInfoReadAdd[4];

//---->sanshin_20151008
_FILE_INFO_SORT_DATA_   UINT16  SonyTable_CmpCode00C0[]={
    0x0041,0x0041,0x0041,0x0041,    //0xC0~0xC3
    0x0041,0x0041,0x0041,0x0043,    //0xC4~0xC7
    0x0045,0x0045,0x0045,0x0045,    //0xC8~0xCB
    0x0049,0x0049,0x0049,0x0049,    //0xCC~0xCF

    0x0044,0x004E,0x004F,0x004F,    //0xD0~0xD3
    0x004F,0x004F,0x004F,0x00D7,    //0xD4~0xD7
    0x004F,0x0055,0x0055,0x0055,    //0xD8~0xDB
    0x0055,0x0059,0x00DE,0x0053,    //0xDC~0xDF

    0x0041,0x0041,0x0041,0x0041,    //0xE0~0xE3
    0x0041,0x0041,0x0041,0x0043,    //0xE4~0xE7
    0x0045,0x0045,0x0045,0x0045,    //0xE8~0xEB
    0x0049,0x0049,0x0049,0x0049,    //0xEC~0xEF

    0x0044,0x004E,0x004F,0x004F,    //0xF0~0xF3
    0x004F,0x004F,0x004F,0x00F7,    //0xF4~0xF7
    0x004F,0x0055,0x0055,0x0055,    //0xF8~0xFB
    0x0055,0x0059,0x00DE,0x0059,    //0xFC~0xFF
};
_FILE_INFO_SORT_DATA_   UINT16  SonyTable_CmpCode0430[]={
    0x0410,0x0411,0x0412,0x0413,    //0x30~0x33
    0x0414,0x0415,0x0416,0x0417,    //0x34~0x37
    0x0418,0x0419,0x041A,0x041B,    //0x38~0x3B
    0x041C,0x041D,0x041E,0x041F,    //0x3C~0x3F

    0x0420,0x0421,0x0422,0x0423,    //0x40~0x43
    0x0424,0x0425,0x0426,0x0427,    //0x44~0x47
    0x0428,0x0429,0x042A,0x042B,    //0x48~0x4B
    0x042C,0x042D,0x042E,0x042F,    //0x4C~0x4F

};
_FILE_INFO_SORT_DATA_   UINT16  SonyTable_CmpCode2160[]={
    0x0031,0x0032,0x0033,0x0034,    //0x60~0x63
    0x0035,0x0036,0x0037,0x0038,    //0x64~0x67
    0x0039,0x0031,0x0031,0x0031,    //0x68~0x6B
    0x216C,0x216D,0x216E,0x216F,    //0x6C~0x6F

    0x0031,0x0032,0x0033,0x0034,    //0x70~0x73
    0x0035,0x0036,0x0037,0x0038,    //0x74~0x77
    0x0039,0x0031,0x217A,0x217B,    //0x78~0x7B
    0x217C,0x217D,0x217E,0x217F,    //0x7C~0x7F

};

_FILE_INFO_SORT_DATA_   UINT16  SonyTable_CmpCode2460[]={
    0x0031,0x0032,0x0033,0x0034,    //0x60~0x63
    0x0035,0x0036,0x0037,0x0038,    //0x64~0x67
    0x0039,0x0031,0x0031,0x0031,    //0x68~0x6B
    0x0031,0x0031,0x0031,0x0031,    //0x6C~0x6F

    0x0031,0x0031,0x0031,0x0032,    //0x70~0x73
    0x0031,0x0032,0x0033,0x0034,    //0x74~0x77
    0x0035,0x0036,0x0037,0x0038,    //0x78~0x7B
    0x0039,0x0031,0x0031,0x0031,    //0x7C~0x7F

    0x0031,0x0031,0x0031,0x2483,    //0x80~0x83
    0x2484,0x2485,0x2486,0x2487,    //0x84~0x87
    0x0031,0x0032,0x0033,0x0034,    //0x88~0x8B
    0x0035,0x0036,0x0037,0x0038,    //0x8C~0x8F

    0x0039,                         //0x90

};
_FILE_INFO_SORT_DATA_   UINT16  SonyTable_CmpCode24D0[]={
    0x0041,0x0042,0x0043,0x0044,    //0xD0~0xD3
    0x0045,0x0046,0x0047,0x0048,    //0xD4~0xD7
    0x0049,0x004A,0x004B,0x004C,    //0xD8~0xDB
    0x004D,0x004E,0x004F,0x0050,    //0xDC~0xDF

    0x0051,0x0052,0x0053,0x0054,    //0xE0~0xE3
    0x0055,0x0056,0x0057,0x0058,    //0xE4~0xE7
    0x0059,0x005A,0x24EA,0x24EB,    //0xE8~0xEB
    0x24EC,0x24ED,0x24EE,0x24EF,    //0xEC~0xEF

};
_FILE_INFO_SORT_DATA_   UINT16  SonyTable_CmpCodeFF10[]={
    0x0030,0x0031,0x0032,0x0033,    //0x10~0x13
    0x0034,0x0035,0x0036,0x0037,    //0x14~0x17
    0x0038,0x0039,0xFF1A,0xFF1B,    //0x18~0x1B
    0xFF1C,0xFF1D,0xFF1E,0xFF1F,    //0x1C~0x1F

    0xFF20,0x0041,0x0042,0x0043,    //0x20~0x23
    0x0044,0x0045,0x0046,0x0047,    //0x24~0x27
    0x0048,0x0049,0x004A,0x004B,    //0x28~0x2B
    0x004C,0x004D,0x004E,0x004F,    //0x2C~0x2F

    0x0050,0x0051,0x0052,0x0053,    //0x30~0x33
    0x0054,0x0055,0x0056,0x0057,    //0x34~0x37
    0x0058,0x0059,0x005A,0xFF3B,    //0x38~0x3B
    0xFF3C,0xFF3D,0xFF3E,0xFF3F,    //0x3C~0x3F

    0xFF40,0x0041,0x0042,0x0043,    //0x40~0x43
    0x0044,0x0045,0x0046,0x0047,    //0x44~0x47
    0x0048,0x0049,0x004A,0x004B,    //0x48~0x4B
    0x004C,0x004D,0x004E,0x004F,    //0x4C~0x4F

    0x0050,0x0051,0x0052,0x0053,    //0x50~0x53
    0x0054,0x0055,0x0056,0x0057,    //0x54~0x57
    0x0058,0x0059,0x005A,0xFF5B,    //0x58~0x5B
    0xFF5C,0xFF5D,0xFF5E,0xFF5F,    //0x5C~0x5F


};
_FILE_INFO_SORT_DATA_   UINT16  SonyTable_CmpCode0140[]={
    0x004C,0x004C,0x004C,0x0143,    //0x40~0x43
    0x004E,0x0145,0x0146,0x0147,    //0x44~0x47
    0x004E,0x0149,0x004E,0x004E,    //0x48~0x4B
    0x014C,0x004F,0x014E,0x014F,    //0x4C~0x4F
};
_FILE_INFO_SORT_DATA_   UINT16  SonyTable_CmpCode01D0[]={
    0x0049,0x01D1,0x004F,0x01D3,    //0x40~0x43
    0x0055,0x01D5,0x0055,0x01D7,    //0x44~0x47
    0x0055,0x01D9,0x0055,0x01DB,    //0x48~0x4B
    0x0055,0x01DD,0x01DE,0x01DF,    //0x4C~0x4F
};
//<----sanshin_20151008

extern UINT32 SaveSortInfo(UINT16 Deep, UINT8* SubInfoBuffer,UINT16 StartID,UINT8* FileIDBuf,UINT16 Flag);

//extern UINT32 SaveSortInfo(UINT32 uiSaveType, UINT32 ulSortSectorAddr,UINT32 MediaInfoAddress);
/*
--------------------------------------------------------------------------------
  Function name : UINT32 GetPYCode(UINT32 wch)
  Author        : anzhiguo
  Description   : 获取汉字字符的排序号

  Input         : wchˇ汉字字符的unicode值
  Return        : pinCode:字符拼音排序号

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
#ifdef  PYSORT_ENABLE
_FILE_INFO_SORT_CODE_
UINT32 GetPYCode(UINT16 wch)
{
    UINT32 pinCode;

    extern UINT32 ReadDataFromIRAM(UINT32 addr);

    pinCode = ReadDataFromIRAM(wch- UNICODE_BEGIN+BASE_PYTABLE_ADDR_IN_IRM); // 从调入IRAM的拼音排序表得到汉字的拼音排序值
    pinCode += UNICODE_BEGIN;

    return (pinCode);
}

#endif
/*
--------------------------------------------------------------------------------
  Function name : UINT32 GetCmpResult(UINT32 wch)
  Author        : anzhiguo
  Description   : 获取汉字字符的排序号

  Input         : wchˇ汉字字符的unicode值
  Return        : pinCode:字符拼音排序号

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_FILE_INFO_SORT_CODE_

UINT16 GetSonySortCode(UINT16 cmpCode); //<----sanshin_20151008

UINT32 GetCmpResult(UINT16 wch)
{
    UINT32 cmpCode;

#ifdef  PYSORT_ENABLE
    if ((wch >= UNICODE_BEGIN) && (wch <= UNICODE_END))
    {
        cmpCode = GetPYCode(wch);
    }
    else
#endif

        if (wch>=97&&wch<=122) // 97 = 'a', 122 = 'z'  小写字母全部转换成大写字母
        {
            cmpCode = wch-32;
        }
        else
        {
            cmpCode = wch;
        }

    cmpCode = GetSonySortCode(cmpCode); //<----sanshin_20151008


    return (cmpCode);
}

//---->sanshin_20151008
_FILE_INFO_SORT_CODE_

/*
--------------------------------------------------------------------------------
*/
UINT16 GetSonySortCode(UINT16 cmpCode)
{
    UINT16  NewCmpCode, tmpCmpidx, tmp;

    NewCmpCode = cmpCode;

//  DEBUG("--- cmpCode = %x ",cmpCode);
    tmp = cmpCode & 0xff00;

    switch (tmp)
    {

        case 0x0000:
            if(cmpCode >= 0x00C0){
                tmpCmpidx = cmpCode - 0x00C0;
                NewCmpCode = SonyTable_CmpCode00C0[tmpCmpidx];
            }

        break;

        case 0x0400:
            if(
                (cmpCode >= 0x0430) &&
                (cmpCode <= 0x044F)
            ){
                tmpCmpidx = cmpCode - 0x0430;
                NewCmpCode = SonyTable_CmpCode0430[tmpCmpidx];
            }

            break;

        case 0x2100:
            if(
                (cmpCode >= 0x2160) &&
                (cmpCode <= 0x2179)
            ){
                tmpCmpidx = cmpCode - 0x2160;
                NewCmpCode = SonyTable_CmpCode2160[tmpCmpidx];
            }
            break;

        case 0x2400:
            if(
                (cmpCode >= 0x2460) &&
                (cmpCode <= 0x2490)
            ){
                tmpCmpidx = cmpCode - 0x2460;
                NewCmpCode = SonyTable_CmpCode2460[tmpCmpidx];
            }
            else if(
                (cmpCode >= 0x24D0) &&
                (cmpCode <= 0x24E9)
            ){
                tmpCmpidx = cmpCode - 0x24D0;
                NewCmpCode = SonyTable_CmpCode24D0[tmpCmpidx];
            }
            break;

        case 0xFF00:
            if(
                (cmpCode >= 0xFF10) &&
                (cmpCode <= 0xFF5A)
            ){
                tmpCmpidx = cmpCode - 0xFF10;
                NewCmpCode = SonyTable_CmpCodeFF10[tmpCmpidx];
            }
            break;

        case 0x0100:

            if(cmpCode == 0x0101){
                NewCmpCode = 0x0041;

            }else if(cmpCode == 0x0111){
                NewCmpCode = 0x0044;

            }else if(cmpCode == 0x01CE){
                NewCmpCode = 0x0041;

            }else if(cmpCode == 0x01F9){
                NewCmpCode = 0x004E;

            }else if(
                (cmpCode >= 0x0113) &&
                (cmpCode <= 0x017E)
            ){
                tmp = cmpCode & 0x00F0;

                switch (tmp)
                {

                    case 0x0010:
                        if(cmpCode == 0x0113){
                            NewCmpCode = 0x0045;

                        }else if(cmpCode == 0x011B){
                            NewCmpCode = 0x0045;
                        }
                        break;

                    case 0x0020:
                        if(cmpCode == 0x0126){
                            NewCmpCode = 0x0048;
                        }else if(cmpCode == 0x0127){
                            NewCmpCode = 0x0048;
                        }else if(cmpCode == 0x012B){
                            NewCmpCode = 0x0049;
                        }
                        break;

                    case 0x0030:
                        if(cmpCode != 0x0130){
                            if(cmpCode <= 0x0133){
                                NewCmpCode = 0x0049;
                            }else if(cmpCode == 0x0138){
                                NewCmpCode = 0x0051;
                            }else if(cmpCode == 0x013F){
                                NewCmpCode = 0x004C;
                            }
                        }

                        break;
                    case 0x0040:
                        tmpCmpidx = cmpCode - 0x0140;
                        NewCmpCode = SonyTable_CmpCode0140[tmpCmpidx];

                        break;

                    case 0x0050:
                        if(cmpCode == 0x0152){
                            NewCmpCode = 0x004F;

                        }else if(cmpCode == 0x0153){
                            NewCmpCode = 0x004F;

                        }

                        break;
                    case 0x0060:
                        if(cmpCode <= 0x0161){
                            NewCmpCode = 0x0053;
                        }else if(cmpCode == 0x0166){
                            NewCmpCode = 0x0054;
                        }else if(cmpCode == 0x0167){
                            NewCmpCode = 0x0054;
                        }else if(cmpCode == 0x016B){
                            NewCmpCode = 0x0055;
                        }

                        break;
                    case 0x0070:
                        if(cmpCode == 0x0178){
                            NewCmpCode = 0x0059;
                        }else if(cmpCode == 0x017D){
                            NewCmpCode = 0x005A;
                        }else if(cmpCode == 0x017E){
                            NewCmpCode = 0x005A;
                        }

                        break;

                    default:
                        break;

                }

            }else if(
                (cmpCode >= 0x01D0) &&
                (cmpCode <= 0x01DC)
            ){
                tmpCmpidx = cmpCode - 0x01D0;
                NewCmpCode = SonyTable_CmpCode01D0[tmpCmpidx];
            }
            break;

        default:
            break;
    }

//  DEBUG("--- CmpCode >> NewCmpCode : 0x%x >> 0x%x",cmpCode,NewCmpCode);
    return(NewCmpCode);
}
//<----sanshin_20151008

/*
--------------------------------------------------------------------------------
  Function name : int PinyinCharCmp(UINT32 wch1, UINT32 wch2)
  Author        : anzhiguo
  Description   : 按拼音排序ˇ比较两个字符的码大小

  Input         : wch1ˇ字符1的unicode值   wch2ˇ字符2的unicode值
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

_FILE_INFO_SORT_CODE_
int PinyinCharCmp(UINT16 wch1, UINT16 wch2)
{
    return (GetCmpResult(wch1) - GetCmpResult(wch2));
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 PinyinStrnCmp(UINT16 *str1, UINT16 *str2, UINT32 length)
  Author        : anzhiguo
  Description   : 按拼音排序ˇ比较两个字符串大小

  Input         : str1ˇ字符串1   str2ˇ字符串2
  Return        : 0 str1 < str2
                  1 str1 = str2
                  2 str1 > str2
  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_FILE_INFO_SORT_CODE_

UINT32 PinyinStrnCmp(UINT16 *str1, UINT16 *str2/*, UINT32 length*/)
{
    UINT32  i = 0;
    UINT16 * pBkMDReadAdrs1, * pBkMDReadAdrs2;
    UINT16 * longStr1, *longStr2;

    UINT32  BkMDReadAdrs1, BkMDReadAdrs2;


//    if (str1 == NULL)
//    {
//        return 0;
//    }
//
//    if (str2 == NULL)
//    {
//          return 2;
//    }
//
//    for (i = 0; i < length; i++)
//    {
//          if ((str1[i] == NULL) || (str2[i] == NULL) || (/*GetCmpResult*/(str1[i]) != /*GetCmpResult*/(str2[i])))
//          {
//          break;
//          }
//    }
//
//    if(i==length)  return 1;

//      if((str1[i]) > (str2[i]))
//         return 2;
//      else if((str1[i]) == (str2[i]))
//         return 1;  //ˇ等
//      else
//         return 0;



    //if(NULL == (str1[0] | str2[0]))   return 1;

    if (str1[0] > str2[0])   return 2;
    if (str1[0] < str2[0])   return 0;
    if (str1[1] > str2[1])   return 2;
    if (str1[1] < str2[1])   return 0;
    if (str1[2] > str2[2])   return 2;
    if (str1[2] < str2[2])   return 0;
    if (str1[3] > str2[3])   return 2;
    if (str1[3] < str2[3])   return 0;
    //if(str1[4] > str2[4]) return 2;
    //if(str1[4] < str2[4]) return 0;
    //if(str1[5] > str2[5]) return 2;
    //if(str1[5] < str2[5]) return 0;

#if 1
    //---->sanshin_20151026
    //if (str1[3] == NULL)
    //{
    //    //printf("error2\n");
    //    return 1;
    //}
    //<----sanshin_20151026

    pBkMDReadAdrs1 = (uint16 *)(&str1[SORT_FILENAME_LEN]);
    pBkMDReadAdrs2 = (uint16 *)(&str2[SORT_FILENAME_LEN]);

    BkMDReadAdrs1 = ((UINT32)(MediaInfoAddr+gwDataBaseAddr)<<9) + BYTE_NUM_SAVE_PER_FILE * (UINT32)(*pBkMDReadAdrs1)+gwSectorOffset;
    BkMDReadAdrs2 = ((UINT32)(MediaInfoAddr+gwDataBaseAddr)<<9) + BYTE_NUM_SAVE_PER_FILE * (UINT32)(*pBkMDReadAdrs2)+gwSectorOffset;

#if 1
    if (BkMDReadAdrs1 == gBkMDReadAdrs1)
    {
        longStr1 = TempBuf1;
    }
    else
    {
        MDReadData(DataDiskID,BkMDReadAdrs1, MEDIA_ID3_SAVE_CHAR_NUM*2, TempBuf1);
        longStr1 = TempBuf1;
        gBkMDReadAdrs1 = BkMDReadAdrs1;
#if 0
        {
            uint32 i;
            for (i = 0; i < 41; i++)
            {
                printf("%c ", TempBuf1[i]);
            }
            printf("\n");
        }
#endif
    }
#else
    MDReadData(DataDiskID,*pBkMDReadAdrs1, MEDIA_ID3_SAVE_CHAR_NUM*2, TempBuf1);
    longStr1 = TempBuf1;
#if 0
    {
        uint32 i;
        for (i = 0; i < 41; i++)
        {
            printf("%x ", TempBuf1[i]);
        }
        printf("\n");
    }
#endif
#endif

#if 1

    if (BkMDReadAdrs2 == gBkMDReadAdrs2)
    {
        longStr2 = TempBuf2;
    }
    else
    {
        MDReadData(DataDiskID,BkMDReadAdrs2, MEDIA_ID3_SAVE_CHAR_NUM*2, TempBuf2);
        longStr2 = TempBuf2;
        gBkMDReadAdrs2 = BkMDReadAdrs2;
    }
#else
    MDReadData(DataDiskID,*pBkMDReadAdrs2, MEDIA_ID3_SAVE_CHAR_NUM*2, TempBuf2);
    longStr2 = TempBuf2;
#endif


//    for (i = 4; i < MEDIA_ID3_SAVE_CHAR_NUM; i++)
    for (i = 0; i < MEDIA_ID3_SAVE_CHAR_NUM; i++)       //<----sanshin_20151026
    {
        if (longStr1[i] > longStr2[i])   return 2;
        if (longStr1[i] < longStr2[i])   return 0;
        if (longStr1[i] == NULL)
        {
            //printf("i = %d\n", i);
            return 1;
        }
    }
#endif

    //printf("errror\n");
    return 1;



}
/*
--------------------------------------------------------------------------------
  Function name : UINT8 BrowserListInsertBySort(FILE_INFO_INDEX_STRUCT *head, FILE_INFO_INDEX_STRUCT *pNode)
  Author        : anzhiguo
  Description   : 初始化双ˇ链表

  Input         : *head   插入链表头指针
                  *pFileSaveTemp   待插入结点结构
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_FILE_INFO_SORT_CODE_

UINT8 BrowserListInsertBySort(FILE_INFO_INDEX_STRUCT *head, FILE_INFO_INDEX_STRUCT *pNode)
{
    FILE_INFO_INDEX_STRUCT *p,*q;
    UINT32 i;

    //UINT32 test_cnt = 0;  //<...sanshin test

    p = head;

    //sanshin...>
    //printf(" head p = %p \n",p);
    if ( (pChildChainLast->pNext!=NULL) ||
            (PinyinStrnCmp((uint16*)(pChildChainLast->SortFileName), (uint16*)(pNode->SortFileName)/*, SORT_FILENAME_LEN*/)==2) //顺序扫描链表,将新结点插入到比它大的结点前
       )
    {
        //<...sanshin

        //sanshin...>
        if (
            (pChildChainLast2->pNext!=NULL) &&
            (PinyinStrnCmp((uint16*)(pChildChainLast2->SortFileName), (uint16*)(pNode->SortFileName)/*, SORT_FILENAME_LEN*/)==2)
        )
        {
            //printf(" bk reset : pChildChainLast2 = %p  \n", p );
        }
        else
        {
            //printf(" bk non reverse : p = %p  \n", p );
            p = pChildChainLast2;

        }
        //<...sanshin

        while (p->pNext!=NULL)
        {
            #ifdef _WATCH_DOG_
            WatchDogReload();
            #endif

            q = p->pNext;
            // 2 表示 q->SortFileName > pNode->SortFileName
            if (PinyinStrnCmp((uint16*)(q->SortFileName), (uint16*)(pNode->SortFileName)/*, SORT_FILENAME_LEN*/)==2) //顺序扫描链表,将新结点插入到比它大的结点前
            {
                pNode->pNext = q;
                p->pNext = pNode;

                //sanshin...>
                pChildChainLast2 = pNode;
//              printf(" sort exec : test_cnt = %d p = %p  \n", test_cnt , p );
                //<...sanshin

                return 1;
            }
            p = q;

            //test_cnt++;       //<...sanshin test

        }

        //sanshin...>
    }
    else
    {
        p = pChildChainLast;
    }

    //printf(" NULL exec : test_cnt = %d p = %p pChildChainLast = %p \n",test_cnt,p,pChildChainLast);
    //<...sanshin

    if (p->pNext==NULL) // 若到了链尾,说明插入的结点为当前链表最大值,将其插入到链尾
    {
        pNode->pNext = NULL;
        p->pNext = pNode;

        //sanshin...>
        pChildChainLast = pNode;
        //<...sanshin

        return 1;
    }

    return 0;
}
/*
--------------------------------------------------------------------------------
  Function name : void SortPageBufferInit(UINT8 *pBuffer)
  Author        : anzhiguo
  Description   :

  Input         : *pBuffer   要初始化的buf指针

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_FILE_INFO_SORT_CODE_

void SortPageBufferInit(UINT8 *pBuffer)
{
    UINT32  i;

    if (pBuffer == GerneAblumBuffer)
    {
        //memset(GerneAblumBuffer,0,8*1024);
        memset(GerneAblumBuffer,0,sizeof(GerneAblumBuffer));
    }

    if (pBuffer == SingerBuffer)
    {
        //memset(SingerBuffer,0,16*1024);
        memset(SingerBuffer,0,sizeof(SingerBuffer));
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void ChildChainInit(void)
  Author        : anzhiguo
  Description   : 初始化双ˇ链表

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         gChildChainDivValue[i] 与 pChildChainHead[i] 一一对应
--------------------------------------------------------------------------------
*/
_FILE_INFO_SORT_CODE_
void ChildChainInit(UINT16 Flag)
{
    UINT32 i,j;
    for (i=0;i<CHILD_CHAIN_NUM;i++) // 初始化各链表头节点
    {

        //pChildChainHead 是一个指针数组ˇ它的每个元素均为一个FILE_INFO_INDEX_STRUCT类型的指针ˇ
        //gChildChainHead 是一个结构体数组ˇ它的每个元素均为一个FILE_INFO_INDEX_STRUCT类型的结构体变量ˇ
        pChildChainHead[i] = &gChildChainHead[i];
        pChildChainHead[i]->pNext = NULL;
    }

    //多次调度时ˇ此处下面的代码只需在进入排序模块时执行一次就可以ˇ因此可以考虑传一个参数用于判断是不是第一次进入
    if (Flag)
    {
        gChildChainDivValue[0]=  0x0000; // 排序字符为空的放置于此
        gChildChainDivValue[1]=   0x41; // 小于英文字母的字符ˇ包括数字及其它符号  0x41 A字母的ascall 码

        j = 2;
        for (i='A';i<'Z'+1;i++)
        {
            gChildChainDivValue[j] = i+1; // 英文字符,A~Z
            j++;
        }

        gChildChainDivValue[j++] = 0x4DFF; // 英文字符到汉字之间的unicode字符

        j = 29;
        for (i=j;i<CHILD_CHAIN_NUM-1;i++)
        {
            gChildChainDivValue[i] = gChildChainDivValue[i-1]+(0x51a5)/(CHILD_CHAIN_NUM-30); // 对汉字部分字符进行42等分
        }

        gChildChainDivValue[CHILD_CHAIN_NUM-1] = 0xffff; // 大于汉字的其它字符全部放至此链表
    }
    /*for(i=0;i<CHILD_CHAIN_NUM;i++)
    {
       DisplayTestDecNum((i%6)*50,(i/6)*15,gChildChainDivValue[i]);
    }
    while(1);*/
    /* 以下设置是为了在拼音排序时ˇ按拼音字母分隔排序链表 */
    /*gChildChainDivValue[0]=   0x0000;
    gChildChainDivValue[1]=   0x4DFF; //0x0000~0x4DFF; //  0~9,字母,其它符号
    gChildChainDivValue[2] =  0x4EC3; //0x4E00~0x4EC3; //  A, 吖
    gChildChainDivValue[3] =  0x5235; //0x4EC4~0x5235; //  Bˇ八
    gChildChainDivValue[4] =  0x576a; //0x5236~0x576a; //  C, 嚓
    gChildChainDivValue[5] =  0x5b40; //0x576b~0x5b40; //  D, ˇ
    gChildChainDivValue[6] =  0x5bf0; //0x5b41~0x5bf0; //  Eˇ酬
    gChildChainDivValue[7] =  0x5e55; //0x5bf1~0x5e55; //  F, 发
    gChildChainDivValue[8] =  0x6198; //0x5e56~0x6198; //  G, 旮
    gChildChainDivValue[9] =  0x65b3; //0x6199~0x65b3; //  H, 承,ha
    gChildChainDivValue[10] = 0x6c00; //0x65b4~0x6c00; //  J, 丌
    gChildChainDivValue[11] = 0x6e30; //0x6c01~0x6e30; //  K, 咔
    gChildChainDivValue[12] = 0x7448; //0x6e31~0x7448; //  L, 垃
    gChildChainDivValue[13] = 0x7810; //0x7449~0x7810; //  M, 
    gChildChainDivValue[14] = 0x7953; //0x7811~0x7953; //  N, ˇ
    gChildChainDivValue[15] = 0x7978; //0x7954~0x7978; //  O, 噢
    gChildChainDivValue[16] = 0x7be2; //0x7979~0x7be2; //  P, 秤
    gChildChainDivValue[17] = 0x7fe9; //0x7be3~0x7fe9; //  q, 七,qi
    gChildChainDivValue[18] = 0x8131; //0x7fea~0x8131; //  r, ◎,ra
    gChildChainDivValue[19] = 0x8650; //0x8132~0x8650; //  s, 仨,sa
    gChildChainDivValue[20] = 0x89d0; //0x8651~0x89d0; //  t, 他
    gChildChainDivValue[21] = 0x8c43; //0x89d1~0x8c43; //  w, 服
    gChildChainDivValue[22] = 0x9169; //0x8c44~0x9169; //  x, 夕
    gChildChainDivValue[23] = 0x9904; //0x916a~0x9904; //  y, 丫
    gChildChainDivValue[24] = 0x9fa4; //0x9905~0x9fa4; //  z, */
}

/*
--------------------------------------------------------------------------------
  Function name : void GetSortName(UINT32 SectorOffset)
  Author        : anzhiguo
  Description   : 从flash中读取所有文件的长文件名ˇ并存放到 gSortNameBuffer0和 gSortNameBuffer1 中
                  这两块空间是连续的ˇ每个可以存放的文件数是系统允许的做大文件数的一半

  Input         : SectorOffsetˇ待排序字符在文件信息中的偏移地址

  Return        : Flag 判断读取文件的顺序,Flag =0ˇ按FileIDBuffer中的顺序读取文件

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         从保存在Flash中的文件信息读取需要排序的字符(文件名、ID3信息)ˇ并且存与gSortNameBuffer0[].SortFileName中
                将所有文件的信息(需要排序的字符信息)读取
--------------------------------------------------------------------------------
*/
_FILE_INFO_SORT_CODE_
void GetSortName(UINT32 SectorOffset,UINT16 FileNum,UINT16 Flag)
{
    UINT16  i,j;
    UINT16  FileCount;
    UINT8  TempBuffer[SORT_FILENAME_LEN*2];
    FILE_INFO_INDEX_STRUCT *pTemp;
    UINT16  *FileID;
    uint16 * BkMDReadAdrs;
    pTemp = gSortNameBuffer0;
    FileCount = FileNum;


    FileID = (UINT16 *)FileIDBuffer;

    BkMDReadAdrs = (uint16 *)(pTemp->SortFileName + SORT_FILENAME_LEN);

    gwSectorOffset = SectorOffset;
    gwDataBaseAddr = MUSIC_SAVE_INFO_SECTOR_START;

    for (i=0;i<FileCount;i++)
    {
        //从flash中读取截取的长文件名信息  //这里地址的计算在4k page的flash中会出问题
        #ifdef _WATCH_DOG_
        WatchDogReload();
        #endif
        
        if (Flag)
        {
            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr+MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(i)+SectorOffset, SORT_FILENAME_LEN*2, TempBuffer);
            //*BkMDReadAdrs = ((UINT32)(MediaInfoAddr+MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(i)+SectorOffset;
            *BkMDReadAdrs = i;
            //printf("*BkMDReadAdrs = %d\n", *BkMDReadAdrs);
        }
        else
        {
            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr+MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileID[i])+SectorOffset, SORT_FILENAME_LEN*2, TempBuffer);
            //*BkMDReadAdrs = ((UINT32)(MediaInfoAddr+MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileID[i])+SectorOffset;
            *BkMDReadAdrs = FileID[i];
            //printf("*BkMDReadAdrs = %d\n", *BkMDReadAdrs);
        }

        for (j=0;j<SORT_FILENAME_LEN;j++)
        {
            pTemp->SortFileName[j] = (TempBuffer[j*2]&0xff)+((TempBuffer[j*2+1]&0xff)<<8);
            if (
                (97 <=pTemp->SortFileName[j])
                && (122 >=pTemp->SortFileName[j])
            )
            {
                pTemp->SortFileName[j] = pTemp->SortFileName[j] - 32;
            }

            pTemp->SortFileName[j] = GetSonySortCode(pTemp->SortFileName[j]);   //<----sanshin_20151008

        }

        pTemp++;
        BkMDReadAdrs = (uint16 *)(pTemp->SortFileName + SORT_FILENAME_LEN);
    }

}


_FILE_INFO_SORT_CODE_
void GetDirSortName(UINT32 SectorOffset,UINT32 StartID, UINT16 * FileNum, UINT16 Flag, UINT8 DirSort)
{
    UINT16  i,j;
    UINT16  FileCount;
    UINT8  TempBuffer[SORT_FILENAME_LEN*2];
    FILE_TREE_BASIC stFileTreeBasic;
    uint32 TreeSectStart, SaveInfoStart;

    FILE_INFO_INDEX_STRUCT *pTemp;
    UINT16  *FileID;
    uint16 * BkMDReadAdrs;

    pTemp = gSortNameBuffer0;
    FileID = (UINT16 *)FileIDBuffer;

    BkMDReadAdrs = (uint16 *)(pTemp->SortFileName + SORT_FILENAME_LEN);

    if (DirSort == 1)
    {
        TreeSectStart = MUSIC_TREE_INFO_SECTOR_START;
        SaveInfoStart = MUSIC_SAVE_INFO_SECTOR_START;
    }
    else if (DirSort == 2)
    {
        TreeSectStart = RECORD_TREE_INFO_SECTOR_START;
        SaveInfoStart = RECORD_SAVE_INFO_SECTOR_START;
    }
#ifdef PIC_MEDIA                                        /*<-- sanshin 0612 */
    else if(DirSort == 4)                               /*<-- sanshin 0612 */
    {                                                   /*<-- sanshin 0612 */
        TreeSectStart = JPEG_TREE_INFO_SECTOR_START;    /*<-- sanshin 0612 */
        SaveInfoStart = JPEG_SAVE_INFO_SECTOR_START;    /*<-- sanshin 0612 */
    }                                                   /*<-- sanshin 0612 */
#endif                                                  /*<-- sanshin 0612 */
    else
    {
        *FileNum = 0;
        return;
    }

    gwSectorOffset = SectorOffset;
    gwDataBaseAddr = SaveInfoStart;

    if (Flag)
    {
        //first get info need count totalnum
        FileCount = 0;
        do
        {
        #ifdef _WATCH_DOG_
            WatchDogReload();
        #endif

            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + TreeSectStart)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(StartID + FileCount), sizeof(FILE_TREE_BASIC), (uint8 *)&stFileTreeBasic);
            FileCount++;

            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr+SaveInfoStart)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(stFileTreeBasic.dwBasicInfoID)+SectorOffset, SORT_FILENAME_LEN*2, TempBuffer);
            //*BkMDReadAdrs = ((UINT32)(MediaInfoAddr+SaveInfoStart)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(stFileTreeBasic.dwBasicInfoID)+SectorOffset;
            *BkMDReadAdrs = stFileTreeBasic.dwBasicInfoID;

            for (j=0;j<SORT_FILENAME_LEN;j++)
            {
                pTemp->SortFileName[j] = (TempBuffer[j*2]&0xff)+((TempBuffer[j*2+1]&0xff)<<8);
                if (
                    (97 <=pTemp->SortFileName[j])
                    && (122 >=pTemp->SortFileName[j])
                )
                {
                    pTemp->SortFileName[j] = pTemp->SortFileName[j] - 32;
                }
                pTemp->SortFileName[j] = GetSonySortCode(pTemp->SortFileName[j]);   //<----sanshin_20151008
            }

            pTemp++;
            BkMDReadAdrs = (uint16 *)(pTemp->SortFileName + SORT_FILENAME_LEN);
        }
        while (stFileTreeBasic.dwNextBrotherID != 0xffffffff);

        *FileNum = FileCount;
        //printf("StartID = %d, SubNum = %d\n", StartID, *FileNum);

    }
    else
    {
        FileCount = *FileNum;

        for (i=0;i<FileCount;i++)
        {
            //从flash中读取截取的长文件名信息  //这里地址的计算在4k page的flash中会出问题
        #ifdef _WATCH_DOG_
            WatchDogReload();
        #endif

            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + TreeSectStart)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(StartID + FileID[i]), sizeof(FILE_TREE_BASIC), (uint8 *)&stFileTreeBasic);

            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr+SaveInfoStart)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(stFileTreeBasic.dwBasicInfoID)+SectorOffset, SORT_FILENAME_LEN*2, TempBuffer);
            // *BkMDReadAdrs = ((UINT32)(MediaInfoAddr+SaveInfoStart)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(stFileTreeBasic.dwBasicInfoID)+SectorOffset;
            *BkMDReadAdrs = stFileTreeBasic.dwBasicInfoID;

            for (j=0;j<SORT_FILENAME_LEN;j++)
            {
                pTemp->SortFileName[j] = (TempBuffer[j*2]&0xff)+((TempBuffer[j*2+1]&0xff)<<8);
                if (
                    (97 <=pTemp->SortFileName[j])
                    && (122 >=pTemp->SortFileName[j])
                )
                {
                    pTemp->SortFileName[j] = pTemp->SortFileName[j] - 32;
                }
                pTemp->SortFileName[j] = GetSonySortCode(pTemp->SortFileName[j]);   //<----sanshin_20151008
            }

            pTemp++;
            BkMDReadAdrs = (uint16 *)(pTemp->SortFileName + SORT_FILENAME_LEN);
        }
        //printf("Secnum = %d\n",FileCount);
    }

}

#ifdef PIC_MEDIA
//sanshin----> //

_FILE_INFO_SORT_CODE_
void GetSortNameJpeg(UINT32 SectorOffset,UINT16 FileNum,UINT16 Flag)
{
    UINT16  i,j;
    UINT16  FileCount;
    UINT8  TempBuffer[SORT_FILENAME_LEN*2];
    FILE_INFO_INDEX_STRUCT *pTemp;
    UINT16  *FileID;
    uint16 * BkMDReadAdrs;
    pTemp = gSortNameBuffer0;
    FileCount = FileNum;


    FileID = (UINT16 *)FileIDBuffer;

    BkMDReadAdrs = (uint16 *)(pTemp->SortFileName + SORT_FILENAME_LEN);

    gwSectorOffset = SectorOffset;
    gwDataBaseAddr = JPEG_SAVE_INFO_SECTOR_START;

    for (i=0;i<FileCount;i++)
    {
    #ifdef _WATCH_DOG_
        WatchDogReload();
    #endif
    
        //4SflashVP6AH!=XH!5D3$ND<~C{PEO"  //Ub@o5XV75D<FKcTZ4k page5DflashVP;a3vNJLb
        if (Flag)
        {
            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr+JPEG_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(i)+SectorOffset, SORT_FILENAME_LEN*2, TempBuffer);
            //*BkMDReadAdrs = ((UINT32)(MediaInfoAddr+MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(i)+SectorOffset;
            *BkMDReadAdrs = i;
            //printf("*BkMDReadAdrs = %d\n", *BkMDReadAdrs);
        }
        else
        {
            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr+JPEG_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileID[i])+SectorOffset, SORT_FILENAME_LEN*2, TempBuffer);
            //*BkMDReadAdrs = ((UINT32)(MediaInfoAddr+MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileID[i])+SectorOffset;
            *BkMDReadAdrs = FileID[i];
            //printf("*BkMDReadAdrs = %d\n", *BkMDReadAdrs);
        }

        for (j=0;j<SORT_FILENAME_LEN;j++)
        {
            pTemp->SortFileName[j] = (TempBuffer[j*2]&0xff)+((TempBuffer[j*2+1]&0xff)<<8);
            if (
                (97 <=pTemp->SortFileName[j])
                && (122 >=pTemp->SortFileName[j])
            )
            {
                pTemp->SortFileName[j] = pTemp->SortFileName[j] - 32;
            }
            pTemp->SortFileName[j] = GetSonySortCode(pTemp->SortFileName[j]);   //<----sanshin_20151008
        }

        pTemp++;
        BkMDReadAdrs = (uint16 *)(pTemp->SortFileName + SORT_FILENAME_LEN);
    }

}
//<---- sanshin //

#endif


_FILE_INFO_SORT_CODE_                                                                                                                                                                   //<----sanshin_20150616
void GetSortNameM3u(UINT32 SectorOffset,UINT16 FileNum,UINT16 Flag)                                                                                                                     //<----sanshin_20150616
{                                                                                                                                                                                       //<----sanshin_20150616
    UINT16  i,j;                                                                                                                                                                        //<----sanshin_20150616
    UINT16  FileCount;                                                                                                                                                                  //<----sanshin_20150616
    UINT8  TempBuffer[SORT_FILENAME_LEN*2];                                                                                                                                             //<----sanshin_20150616
    FILE_INFO_INDEX_STRUCT *pTemp;                                                                                                                                                      //<----sanshin_20150616
    UINT16  *FileID;                                                                                                                                                                    //<----sanshin_20150616
    uint16 * BkMDReadAdrs;                                                                                                                                                              //<----sanshin_20150616
    pTemp = gSortNameBuffer0;                                                                                                                                                           //<----sanshin_20150616
    FileCount = FileNum;                                                                                                                                                                //<----sanshin_20150616
                                                                                                                                                                                        //<----sanshin_20150616
    FileID = (UINT16 *)FileIDBuffer;                                                                                                                                                    //<----sanshin_20150616
                                                                                                                                                                                        //<----sanshin_20150616
    BkMDReadAdrs = (uint16 *)(pTemp->SortFileName + SORT_FILENAME_LEN);                                                                                                                 //<----sanshin_20150616
                                                                                                                                                                                        //<----sanshin_20150616
    gwSectorOffset = SectorOffset;                                                                                                                                                      //<----sanshin_20150616
    gwDataBaseAddr = M3U_SAVE_INFO_SECTOR_START;                                                                                                                                        //<----sanshin_20150616
                                                                                                                                                                                        //<----sanshin_20150616
    for(i=0;i<FileCount;i++)                                                                                                                                                            //<----sanshin_20150616
    {                                                                                                                                                                                   //<----sanshin_20150616
    #ifdef _WATCH_DOG_
        WatchDogReload();
    #endif
    
        if(Flag)                                                                                                                                                                        //<----sanshin_20150616
        {                                                                                                                                                                               //<----sanshin_20150616
            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr+M3U_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(i)+SectorOffset, SORT_FILENAME_LEN*2, TempBuffer);            //<----sanshin_20150616
            *BkMDReadAdrs = i;                                                                                                                                                          //<----sanshin_20150616
        }                                                                                                                                                                               //<----sanshin_20150616
        else                                                                                                                                                                            //<----sanshin_20150616
        {                                                                                                                                                                               //<----sanshin_20150616
            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr+M3U_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileID[i])+SectorOffset, SORT_FILENAME_LEN*2, TempBuffer);    //<----sanshin_20150616
            *BkMDReadAdrs = FileID[i];                                                                                                                                                  //<----sanshin_20150616
        }                                                                                                                                                                               //<----sanshin_20150616
                                                                                                                                                                                        //<----sanshin_20150616
        for(j=0;j<SORT_FILENAME_LEN;j++)                                                                                                                                                //<----sanshin_20150616
        {                                                                                                                                                                               //<----sanshin_20150616
            pTemp->SortFileName[j] = (TempBuffer[j*2]&0xff)+((TempBuffer[j*2+1]&0xff)<<8);                                                                                              //<----sanshin_20150616
            if(                                                                                                                                                                         //<----sanshin_20150616
                   (97 <=pTemp->SortFileName[j])                                                                                                                                        //<----sanshin_20150616
                && (122 >=pTemp->SortFileName[j])                                                                                                                                       //<----sanshin_20150616
            )                                                                                                                                                                           //<----sanshin_20150616
            {                                                                                                                                                                           //<----sanshin_20150616
                pTemp->SortFileName[j] = pTemp->SortFileName[j] - 32;                                                                                                                   //<----sanshin_20150616
            }                                                                                                                                                                           //<----sanshin_20150616
            pTemp->SortFileName[j] = GetSonySortCode(pTemp->SortFileName[j]);   //<----sanshin_20151008
        }                                                                                                                                                                               //<----sanshin_20150616
        pTemp++;                                                                                                                                                                        //<----sanshin_20150616
        BkMDReadAdrs = (uint16 *)(pTemp->SortFileName + SORT_FILENAME_LEN);                                                                                                             //<----sanshin_20150616
    }                                                                                                                                                                                   //<----sanshin_20150616
}                                                                                                                                                                                       //<----sanshin_20150616

/*
--------------------------------------------------------------------------------
  Function name : UINT32 SaveSortInfo(UINT32 uiSaveType, UINT32 ulSortSectorAddr)
  Author        : anzhiguo
  Description   : 保存分类信息

  Input         : uiSaveType 保存信息的类型 : ID3_TITLE_TYPE :FILE_NAME_TYPE:ID3_ARTIST_TYPE:ID3_ALBUM_TYPE 四种类型
                  ulSortSectorAddr 信息保存的地址ˇ物理地址还是sec地址 ?
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_FILE_INFO_SORT_CODE_
UINT32 SaveSortInfo(UINT16 Deep, UINT8* SubInfoBuffer,UINT16 StartID,UINT8* FileIDBuf,UINT16 Flag)
{
    UINT32  i,j;

    UINT32 uiFileCount = 0;
    UINT32 uiFileSubCount = 0;  // 文件个数计数
    UINT32 uiSameCount = 0;     // ID3信息归类时ˇˇ同类型计数
    UINT32 uiFileIndex = 0;     // Flash Page控制计数
    UINT16 uiFileSortTemp = 0;  // 文件排序号临时变量

    UINT32 uiSubCount = 0;      // 用于每条子链的文件ID3信息分类计数
    UINT32 uiSubCountTotal = 0; // 用于ID3信息分类的所有文件计数

    UINT32 ulID3SubSectorAddr = 0; // ID3信息归类信息保存地址

    UINT16 StartfileID = StartID; // 分类信息起始文件号
    UINT16 PreSortFileName[SORT_FILENAME_LEN + 2];

    FILE_INFO_INDEX_STRUCT  *pTemp = NULL; // 排序链表头指针
    if (!Flag)
    {
        //memcpy(FileIDBuf,FileIDBuffer,4096);
        memcpy(FileIDBuf, FileIDBuffer, 2 * SORT_FILENUM_DEFINE);
    }
    ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL);//调用写flash代码

    switch (Deep)
    {
        case 1:
            // case FILE_NAME_TYPE:
            // case ID3_TITLE_TYPE:
            i = 1;
            break;
        case 2:
            // case ID3_ARTIST_TYPE:
            i = 0;
            break;
            // case ID3_ALBUM_TYPE:
        case 3:
            i = 0;
            break;
            // case ID3_GENRE_TYPE:
        case 4:
            i = 0;
            break;
        default:
            break;

    }

    for (;i<CHILD_CHAIN_NUM;i++)
    {
        pTemp = pChildChainHead[i];

#ifdef _WATCH_DOG_
        WatchDogReload();
#endif
        while (pTemp->pNext!=NULL)
        {
            pTemp = pTemp->pNext;
            uiFileSortTemp = (UINT16)(pTemp - &gSortNameBuffer0[0]); //计算当前的文件在所有文件中的排序号

            //计算当前要保存的信息与gSortNameBuffer0[0]的偏移地址ˇ因为pChildChainHead链表的信息来源于gSortNameBuffer0
            if (Flag)
            {
                FileIDBuffer[StartID*2 +uiFileIndex++] =  uiFileSortTemp & 0xFF;     //保存文件排序后的序号
                FileIDBuffer[StartID*2 +uiFileIndex++] = (uiFileSortTemp>>8) & 0xFF;//保存文件排序后的序号

                //printf("uiFileSortTemp = %d = %d\n", uiFileSortTemp, StartID);
            }
            else
            {

                FileIDBuffer[StartID*2 +uiFileIndex++] =  FileIDBuf[uiFileSortTemp*2] & 0xFF;     //保存文件排序后的序号
                FileIDBuffer[StartID*2 +uiFileIndex++] =  FileIDBuf[uiFileSortTemp*2+1] & 0xFF;//保存文件排序后的序号

                //printf("Start = %d\n", StartID);
            }

            //printf("id = %d\n", *(uint16 *)(FileIDBuffer + StartID*2 + uiFileIndex - 2));
#ifdef _WATCH_DOG_
            WatchDogReload();
#endif

            switch (Deep)
            {
                case 1://一层深度时ˇ只要最终的文件号顺序链表ˇ不需要sub子ˇ的信息
                    // case FILE_NAME_TYPE:
                    // case ID3_TITLE_TYPE:
                    uiFileSubCount++;
                    break;
                    // case ID3_ARTIST_TYPE://同一个艺术家 或专辑有几个文件ˇ保存其第一个文件的偏移地址ˇ并保存该专辑下的文件个数
                    //  case ID3_ALBUM_TYPE :
                    // case ID3_GENRE_TYPE   :
                case 2:
                case 3:
                case 4:
                    {
                        if (0 == uiFileSubCount)
                        {
                            SubInfoBuffer[uiSubCount++] = StartfileID&0xFF; // 起始文件号
                            SubInfoBuffer[uiSubCount++] = (StartfileID>>8)&0xFF;// 起始文件号

                            SubInfoBuffer[uiSubCount++] = 0;//子ˇ的起始位置ˇ更新完下一级时才写入数据
                            SubInfoBuffer[uiSubCount++] = 0;
                            // printf("uiFileSubCount = %d\n", uiFileSubCount);
                            uiFileSubCount++;
                        }
                        //PinyinStrnCmp 返回 1 表示 两个字符窜ˇ等ˇ同一个艺术家或同一个专辑
                        else if (1==PinyinStrnCmp((uint16*)(pTemp->SortFileName), PreSortFileName/*, SORT_FILENAME_LEN*/)) //(pTemp->SortFileName)等于(pNodePrev->SortFileName)
                        {
                            //printf("same name\n");
                            uiSameCount++;
                        }
                        else // (pTemp->SortFileName)大于(pNodePrev->SortFileName),不会有小于的情况ˇ因为已经排过序的了
                        {
                            StartfileID += (uiSameCount+1);
                            SubInfoBuffer[uiSubCount++] = 0; // 保留空间, 使每次保存大小为2指数倍
                            SubInfoBuffer[uiSubCount++] = 0; // 保留空间, 使每次保存大小为2指数倍

                            //当前这个流派 ˇ专辑ˇ歌手下有多少个文件
                            SubInfoBuffer[uiSubCount++] = (uiSameCount+1)&0xFF; // 记录上一个Item包含的元素个数
                            SubInfoBuffer[uiSubCount++] = ((uiSameCount+1)>>8)& 0xFF;


                            //记录下一个分类ˇ的起始文件位置
                            SubInfoBuffer[uiSubCount++] = StartfileID&0xFF;
                            SubInfoBuffer[uiSubCount++] = (StartfileID>>8)&0xFF;

                            SubInfoBuffer[uiSubCount++] = 0; // //子ˇ的起始位置ˇ更新完下一级时才写入数据
                            SubInfoBuffer[uiSubCount++] = 0; // //子ˇ的起始位置ˇ更新完下一级时才写入数据


                            uiFileSubCount++;

                            uiSameCount = 0;
                        }
                    }
                    break;

                default:
                    break;

            }

            for (j=0;j<(SORT_FILENAME_LEN + 2);j++)
            {
                PreSortFileName[j] = pTemp->SortFileName[j];
            }
            uiFileCount++;
        }

    }
#if 1
    //  if (uiSubCount)//// 保存排序信息不足2K的部分
    //  {

    SubInfoBuffer[uiSubCount++] = 0; // 保留空间, 使每次保存大小为2指数倍
    SubInfoBuffer[uiSubCount++] = 0; // 保留空间, 使每次保存大小为2指数倍

    SubInfoBuffer[uiSubCount++] = (uiSameCount+1)&0xFF; // 统计最后一个Item的个数
    SubInfoBuffer[uiSubCount++] = ((uiSameCount+1)>>8)&0xFF;

    // }
    //printf("uiFileSubCount = %d\n", uiFileSubCount);
#endif

    return uiFileSubCount;


}

/*
--------------------------------------------------------------------------------
  Function name : void SortUpdateFun(void)
  Author        : anzhiguo
  Description   : 对指定范围内的文件进行排序

  Input         : StartNum--排序文件的起始文件号
                  FileNum --排序的文件个数
                  SubInfoBuffer  --存放本次排序后的sub信息
                  PreSubInfoBuffer -- 存放上一级排序的sbu信息ˇ为本级排序提供一些必要的参数
                  Deep    -- 判断当前排序的内容
  Return        : 返回当前范围内排序后分ˇ个数

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_FILE_INFO_SORT_CODE_
UINT32 SingleTypeSortFunction(UINT16 StartNum,UINT16 FileNum,UINT16 *SubInfoBuffer,UINT16 Deep,UINT16 Flag)
{
    UINT32  i,j,SortSubNum;
    UINT16  filenum;
    UINT32 uiFirsCharSortVal=0; // 当前插入结点排序名字的首字符的拼音排序值
    //UINT32 firstpTempNext = 0;                //<...sanshin test

    FILE_INFO_INDEX_STRUCT *pHeadBk = NULL;     //<...sanshin
    FILE_INFO_INDEX_STRUCT *pTemp;
    // if(FileNum==1)//只有一个文件的时候不排序
    //  {
    //     return 1;
    //  }
    pTemp = &gSortNameBuffer0[StartNum];

    /*
    if(StartNum < SORT_FILE_NAME_BUF_SIZE )
        pTemp = &gSortNameBuffer0[StartNum];
    else
        pTemp = &gSortNameBuffer1[StartNum - SORT_FILE_NAME_BUF_SIZE];

    */

    filenum = FileNum;
    //printf("filenum = %d\n", filenum);
//  printf(" %s filenum =0x%x\n",__FUNCTION__,filenum);     //<...sanshin test
    /* 对所有文件的长文件名进行排序 */


    for (i=0;i<filenum;i++)
    {
#ifdef _WATCH_DOG_
        if(i % 200 == 0)
        {
            WatchDogReload();
        }
#endif
        uiFirsCharSortVal = /*GetCmpResult*/(pTemp->SortFileName[0]);
        j = 0;
        while (uiFirsCharSortVal>gChildChainDivValue[j]) // 通过长文件的第一个字符来判断需要在哪个子链表中排序
            j++;
        //sanshin...>
        //if(0 == firstpTempNext){                                                  /////////////
        //  //if(NULL == pChildChainHead[j]->pNext){                                /////////////
        //      printf(" First ->pNext = NULL 0x%x\n",pChildChainHead[j]->pNext);   /////////////
        //      pChildChainLast = pChildChainHead[j];                               /////////////
        //      pHeadBk = pChildChainHead[j];                                       /////////////
        //  //}
        //  //else{                                                             /////////////
        //  //  printf(" First ->pNext = Address 0x%x\n",pChildChainHead[j]->pNext);/////////////
        //  //}                                                                     /////////////
        //  firstpTempNext = 1;                                                     /////////////
        //}
        if (pHeadBk != pChildChainHead[j])
        {
            pHeadBk = pChildChainHead[j];                                       /////////////
            pChildChainLast = pChildChainHead[j];
            pChildChainLast2 = pChildChainHead[j];
        }
        //<...sanshin
        BrowserListInsertBySort(pChildChainHead[j], pTemp); // 将gSortNameBuffer0中的文件信息逐个插入链表pChildChainHead中
        pTemp++;
    }

    //SortSubNum 带回排序后的分类sub数目
    SortSubNum = SaveSortInfo(Deep, (UINT8*)SubInfoBuffer, StartNum, FileIDBuffer1, Flag); //保存文件的排序号ˇ以及分类信息ˇ并返回该类文件的数量

    return SortSubNum;
}

/*
--------------------------------------------------------------------------------
  Function name : void GetSortName(UINT32 SectorOffset)
  Author        : anzhiguo
  Description   : 文件排序功能函数

  Input         : uiSaveAddrOffsetˇ待排序字符(文件名ˇid3titleˇid3singerˇid3ablum)在文件信息中的偏移地址
                  FileNum---排序的文件总数
                  Deep  --- 排序的深度(目录层次)
                 // uiSortType ---排序的类型
                  SortSubNum ---排序后生成的子ˇ个数
                  buffer1   --- 排序信息存放buff
                  buffer2   --- 排序中用到上级排序的信息buf
                  Flag  --- 读取文件信息的方式
                  SubNum --- 用于分级排序时ˇ确定外层循环次数
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         将gSortNameBuffer0和gSortNameBuffer1 中的信息进行分类排列ˇ即分派到对应子链表pChildChainHead的适当的位子上
--------------------------------------------------------------------------------
*/
_FILE_INFO_SORT_CODE_
void SortFunction(UINT32* uiSaveAddrOffset,UINT16 * FileNum,UINT16 Deep,UINT32 *SortSubNum,UINT16 Flag,UINT16*buffer1,UINT16*buffer2,UINT16 SubNum, UINT32 StartID, UINT8 DirSort)
{
    UINT32  i,j;

    UINT16* subbuf1,subbuf2;
    UINT32 uiFirsCharSortVal=0; // 当前插入结点排序名字的首字符的拼音排序值
    UINT16  filenum = 0;
    SORTINFO_STRUCT * pSortInfo1;
    SORTINFO_STRUCT * pSortInfo2;

    FILE_INFO_INDEX_STRUCT *pTemp;

    pTemp = gSortNameBuffer0;

    memset(&gSortNameBuffer0[0], 0, sizeof(FILE_INFO_INDEX_STRUCT) * SORT_FILENUM_DEFINE);

    //DEBUG("FileNum = %d", FileNum);
    gBkMDReadAdrs1 = 0xffffffff;
    gBkMDReadAdrs2 = 0xffffffff;

#ifdef PIC_MEDIA
//sanshin----> //

    //printf("filenum = %d, deep = %d Flag = %d DirSort = %d StartID = %d\n", *FileNum, Deep, Flag, DirSort, StartID);
    if (DirSort == 3)
    {
        GetSortNameJpeg(uiSaveAddrOffset[Deep-1], *FileNum, Flag);     // 8y>]51G05DEEPrIn6H!&;qH!PhR*5DEEPrPEO"(KySPND<~)5=gSortNameBuffer0VP
    }
    else if(DirSort == 5)                                               //<----sanshin_20150616
    {                                                                   //<----sanshin_20150616
        GetSortNameM3u(uiSaveAddrOffset[Deep-1], *FileNum, Flag);       //<----sanshin_20150616
    }                                                                   //<----sanshin_20150616
    else
#endif
    {
//<---- sanshin //

        if (DirSort)
        {
            GetDirSortName(uiSaveAddrOffset[Deep-1],StartID, FileNum, Flag, DirSort);
        	if(8192 < *FileNum){		//<----ton_20151111
        		*FileNum = 8192;		//<----ton_20151111
        	}							//<----ton_20151111
        }
        else
        {
//---->sanshin_20151008
            GetSortName(uiSaveAddrOffset[Deep-1], *FileNum, Flag);     // 根据当前的排序深度ˇ获取需要的排序信息(所有文件)到gSortNameBuffer0中
//<----sanshin_20151008
        }
//sanshin----> //

    }
//<---- sanshin //

    //  DisplayTestDecNum(0, 0,2);    //4K4&6O5c?IRT2i?4  gSortNameBuffer0 VPJG7qSPJ}>]6AHk M,J1R2?IRT?4FileIDbuffVP5DV5
    if (Flag)
    {
        ChildChainInit(1);  // 对各子链表的头节点进行初始化ˇ并给定义每个链表的分段关键字

        //SortSubNum获取最外一层排序后的分类子ˇ个数
        //从gSortNameBuffer0的第0个位置的文件开始ˇ排序FileNum个文件ˇ排序后的分类信息以结构存放在buffer1中
        //buffer2 存放上一级排序的信息ˇ在内层排序时ˇ提供一些参数
//      printf("SingleTypeSortFunction 0 ------------------------------------------------------\n");        //<... sanshin test
        filenum = SingleTypeSortFunction(0,*FileNum,buffer1,Deep,Flag);
        *SortSubNum  = filenum;
    }
    else
    {
        UINT16 IDnum ,basenum;
        pSortInfo1 = (SORTINFO_STRUCT *)buffer2 ;
        pSortInfo1->ItemBaseID = 0 ;
        pSortInfo1->ItemNum = 0;
        basenum = 0;
        pSortInfo2 = (SORTINFO_STRUCT *)buffer1 ;

        for (i=0; i < SubNum; i++)
        {   //获取分类子ˇ个数
            ChildChainInit(0);
            //if(DirSort)
            //printf("BaseID = %d, FileNum = %d\n", pSortInfo1->BaseID, pSortInfo1->FileNum);
//          printf("SingleTypeSortFunction subNum ------------------------------------------------------\n");       //<... sanshin test
            //printf("BaseID = %d, FileNum = %d\n", pSortInfo1->BaseID, pSortInfo1->FileNum);
            IDnum = SingleTypeSortFunction(pSortInfo1->BaseID,pSortInfo1->FileNum,(UINT16*)pSortInfo2,Deep,Flag);
            //偏移存放排序后sub的头指针ˇ为下次排序准备,这里是否需要加保护措施?????

            pSortInfo2 += IDnum;

            //numtemp = pSortInfo1->ItemNum;
            //保存分类子ˇ个数
            basenum += IDnum ;
            pSortInfo1->ItemNum = IDnum;
            //结构指针自加ˇ指ˇ一个sub块的信息结构ˇ并保存其起始id
            if (i < (SORT_FILENUM_DEFINE - 1))
            {
                pSortInfo1 ++;

                pSortInfo1->ItemBaseID = basenum ;
            }
            filenum +=IDnum;

        }
        //在这里ˇ上层排序完成ˇ需要写buffer2到flash中以保存信息

        ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL);

        if (DirSort == 0)
        {
            MDWrite(DataDiskID, MediaInfoSaveAdd[Deep], SORT_FILENUM_DEFINE * sizeof(SORTINFO_STRUCT) / SECTOR_BYTE_SIZE, buffer2);
            //printf("MediaInfoSaveAdd[%d] = %d\n", Deep,MediaInfoSaveAdd[Deep]);

#if 0
            {
                uint16 i, j;

                for (i = 0; i < 4096; i++)
                {
                    printf("\n");
                    for (j = 0; j < 4; j++)
                    {
                        printf("%d ", buffer2[i * 4 + j]);
                    }
                }
            }
#endif
        }
        SortPageBufferInit((UINT8*)buffer2);
    }

    Deep--;

    if (Deep == 0)
    {
        //将fileidbuffer 写flashˇ以及排序sub信息buffer
        ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL);

#if 0
        {
            uint16 i;
            printf("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n\n\n");

            for (i = 0; i < 4096; i++)
            {
                printf("%d\n", ((uint16 *)FileIDBuffer)[i]);
            }
        }
#endif

        if (DirSort == 0)
        {
            MDWrite(DataDiskID, MediaInfoSaveAdd[0], (2 * SORT_FILENUM_DEFINE) / SECTOR_BYTE_SIZE, FileIDBuffer);
            //printf("MediaInfoSaveAdd[0] = %d\n", MediaInfoSaveAdd[0]);
#if 0
            {
                uint16 i;

                for (i = 0; i < 4096; i++)
                {
                    printf("%d\n", ((uint16 *)FileIDBuffer)[i]);
                }
            }
#endif
        }
        else
        {

            //printf("xxx\n");
            for (i = 0; i < (*FileNum * 2); i++)
            {
                gFileSortBuffer[gFileSortBufferIndex++] = FileIDBuffer[i];
                if (gFileSortBufferIndex == MEDIAINFO_PAGE_SIZE)
                {
                    MDWrite(DataDiskID, MediaInfoSaveAdd[0], MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE, gFileSortBuffer);
#if 0
                    {
                        uint16 i;

                        for (i = 0; i < 2048; i++)
                        {
                            printf("%d ", ((uint16 *)gFileSortBuffer)[i]);
                        }
                    }
#endif
                    gFileSortBufferIndex = 0;
                    memset(gFileSortBuffer, 0, MEDIAINFO_PAGE_SIZE);
                    MediaInfoSaveAdd[0] += MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE;
                }
            }
        }
        return;
    }
    else
    {
        //printf("SortFunction ------------------------------------------------------\n");      //<... sanshin test
        SortFunction(uiSaveAddrOffset, FileNum, Deep, SortSubNum,0,buffer2,buffer1,filenum, StartID, DirSort);
    }

}


/*
--------------------------------------------------------------------------------
  Function name : void SortUpdateFun(void)
  Author        : anzhiguo
  Description   : 对文件名信息进行排序ˇ并在指定Flash中存储排序信息

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_FILE_INFO_SORT_CODE_
void SortUpdateFun(MEDIALIB_CONFIG * Sysfilenum ,UINT32 MediaInfoAddress)
{
    UINT16   PathDeep;//媒体库排序深的(目录层数)
    UINT16   *buffer1;
    UINT16   *buffer2;
    UINT32   uiSortTypeCount = 0;            // 排序类型选择

    UINT32   uiCountTemp[SORT_TYPE_ITEM_NUM];
    UINT16   FileNum;
    UINT32   StartID;

    //Gpio_SetPinLevel(BACKLIGHT_PIN, 0);               //<... sanshin test

    if (Sysfilenum->gMusicFileNum != 0)
    {
        buffer1 = (UINT16*)GerneAblumBuffer;
        buffer2 = (UINT16*)SingerBuffer;

        for (uiSortTypeCount=0;uiSortTypeCount<SORT_TYPE_ITEM_NUM;uiSortTypeCount++)
            //uiSortTypeCount = ID3_ARTIST_TYPE;
        {
#ifdef _WATCH_DOG_
            WatchDogReload();
#endif
            switch (uiSortTypeCount)//设置ˇ应的地址信息
            {
                    /*
                    case FILE_NAME_TYPE:
                    //printf("FILE_NAME_TYPE ------------------------------------------------------\n");        //<... sanshin test
                        //排序结束后ˇ排序结果保存到flash中的地址
                        MediaInfoSaveAdd[0] = MediaInfoAddress + FILENAME_SORT_INFO_SECTOR_START;
                        //排序开始前ˇ需要从该地址处读取需要的排序信息
                        MediaInfoReadAdd[0] = FILE_NAME_SAVE_ADDR_OFFSET;

                        PathDeep = 1;
                        uiSortType = SORT_TYPE1;
                        break;
                    */
                case ID3_ALBUM_TYPE:

                    //printf("ID3_ALBUM_TYPE ------------------------------------------------------\n");        //<... sanshin test
                    MediaInfoReadAdd[0] = TRACKID_SAVE_ADDR_OFFSET;
                    MediaInfoReadAdd[1] = ID3_ALBUM_SAVE_ADDR_OFFSET;

                    MediaInfoSaveAdd[0] = MediaInfoAddress + ID3ALBUM_SORT_INFO_SECTOR_START;
                    MediaInfoSaveAdd[1] = MediaInfoAddress + ID3ALBUM_SORT_SUB_SECTOR_START;

                    PathDeep = 2;
                    break;


                case ID3_TITLE_TYPE:
                    //printf("ID3_TITLE_TYPE ------------------------------------------------------\n");        //<... sanshin test
                    MediaInfoReadAdd[0] = ID3_TITLE_SAVE_ADDR_OFFSET;//保存的文件信息的偏移地址(ˇ对于保存文件信息起始地址)
                    MediaInfoSaveAdd[0] = MediaInfoAddress + ID3TITLE_SORT_INFO_SECTOR_START;

                    PathDeep = 1;
                    break;

                case ID3_ARTIST_TYPE:
                    //printf("ID3_ARTIST_TYPE ------------------------------------------------------\n");       //<... sanshin test
                    MediaInfoReadAdd[0] = TRACKID_SAVE_ADDR_OFFSET;
                    MediaInfoReadAdd[1] = ID3_ALBUM_SAVE_ADDR_OFFSET;
                    MediaInfoReadAdd[2] = ID3_SINGLE_SAVE_ADDR_OFFSET;

                    MediaInfoSaveAdd[0] = MediaInfoAddress + ID3ARTIST_SORT_INFO_SECTOR_START;
                    MediaInfoSaveAdd[1] = MediaInfoAddress + ID3ARTIST_ALBUM_SORT_SUB_SECTOR_START;
                    MediaInfoSaveAdd[2] = MediaInfoAddress + ID3ARTIST_SORT_SUB_SECTOR_START;
                    //printf("adddr = %d\n", MediaInfoAddress + ID3ARTIST_SORT_INFO_SECTOR_START);
                    PathDeep = 3;

                    buffer2 = (UINT16*)GerneAblumBuffer;
                    buffer1 = (UINT16*)SingerBuffer;
                    break;


                case ID3_GENRE_TYPE:
                    //printf("ID3_GENRE_TYPE ------------------------------------------------------\n");        //<... sanshin test
                    MediaInfoReadAdd[0] = TRACKID_SAVE_ADDR_OFFSET;
                    MediaInfoReadAdd[1] = ID3_ALBUM_SAVE_ADDR_OFFSET;
                    MediaInfoReadAdd[2] = ID3_SINGLE_SAVE_ADDR_OFFSET;
                    MediaInfoReadAdd[3] = ID3_GENRE_SAVE_ADDR_OFFSET;
                    //MediaInfoReadAdd[2] = ID3_GENRE_SAVE_ADDR_OFFSET; //<... sanshin test

                    MediaInfoSaveAdd[0] = MediaInfoAddress + ID3GENRE_SORT_INFO_SECTOR_START;
                    MediaInfoSaveAdd[1] = MediaInfoAddress + ID3GENRE_ALBUM_SORT_SUB_SECTOR_START;
                    MediaInfoSaveAdd[2] = MediaInfoAddress + ID3GENRE_ARTIST_SORT_SUB_SECTOR_START;
                    MediaInfoSaveAdd[3] = MediaInfoAddress + ID3GENRE_SORT_SUB_SECTOR_START;
                    //MediaInfoSaveAdd[2] = MediaInfoAddress + ID3GENRE_SORT_SUB_SECTOR_START;  //<... sanshin test

                    buffer1 = (UINT16*)GerneAblumBuffer;
                    buffer2 = (UINT16*)SingerBuffer;
                    PathDeep = 4;
                    //PathDeep = 3; //<... sanshin test
                    break;

            }
            //开始一类(流派ˇ专辑ˇ歌手ˇ文件名)信息的排序
            memset(FileIDBuffer,0,sizeof(FileIDBuffer));           // 4 * SORT_FILE_NAME_BUF_SIZE
            memset(GerneAblumBuffer,0,sizeof(GerneAblumBuffer));   // 8*SORT_FILE_NAME_BUF_SIZE
            memset(SingerBuffer,0,sizeof(SingerBuffer));           // 16*SORT_FILE_NAME_BUF_SIZE

            SortFunction(MediaInfoReadAdd,(uint16 *)&(Sysfilenum->gMusicFileNum),PathDeep,&uiCountTemp[uiSortTypeCount],1,buffer1,buffer2,0, 0, 0);//获取对应的文件信息ˇ并排序ˇ一次只能是一种信息

            //if(FILE_NAME_TYPE == uiSortTypeCount){    //<...sanshin test
            //  uiSortTypeCount++;                      //<...sanshin test
            //}                                         //<...sanshin test

            // uiCountTemp[uiSortTypeCount] = SaveSortInfo(uiSortTypeCount, ulFileSortInfoSectorAddr,MediaInfoAddress); //保存文件的排序号ˇ以及分类信息ˇ并返回该类文件的数量
        }

        Sysfilenum->gID3TitleFileNum =  uiCountTemp[0]; // 得到具有ID3 Title信息的文件个数   uiCountTemp[0] 是所有的音乐文件个数
        Sysfilenum->gID3AlbumFileNum =  uiCountTemp[1]; // 得到具有ID3 Album信息的文件个数
        Sysfilenum->gID3ArtistFileNum = uiCountTemp[2]; // 得到具有ID3 Artist信息的文件个数
        Sysfilenum->gID3GenreFileNum =  uiCountTemp[3]; // 得到具有ID3 Genre信息的文件个数

    }



    if (Sysfilenum->gTotalFileNum != 0)
    {
        buffer1 = (UINT16*)GerneAblumBuffer;
        buffer2 = (UINT16*)SingerBuffer;
        MediaInfoReadAdd[0] = FILE_NAME_SAVE_ADDR_OFFSET;
        MediaInfoReadAdd[1] = ATTR_SAVE_ADDR_OFFSET;
        MediaInfoSaveAdd[0] = MediaInfoAddress + MUSIC_TREE_SORT_INFO_SECTOR_START;
        gFileSortBufferIndex = 0;
        memset(gFileSortBuffer, 0, MEDIAINFO_PAGE_SIZE);

        StartID = 0;
        FileNum = 0;

        //printf("totalnum = %d\n", Sysfilenum->gTotalFileNum);
        while (StartID < Sysfilenum->gTotalFileNum)
        {
#ifdef _WATCH_DOG_
            WatchDogReload();
#endif
            MedialibUpdateDisplayHook();//PAGE
            memset(FileIDBuffer,0,sizeof(FileIDBuffer));           // 4 * SORT_FILE_NAME_BUF_SIZE
            memset(GerneAblumBuffer,0,sizeof(GerneAblumBuffer));   // 8*SORT_FILE_NAME_BUF_SIZE
            memset(SingerBuffer,0,sizeof(SingerBuffer));           // 16*SORT_FILE_NAME_BUF_SIZE
            PathDeep = 2;
            //printf("StartID = %d\n", StartID);
            SortFunction(MediaInfoReadAdd,&FileNum,PathDeep,&uiCountTemp[0],1,buffer1,buffer2,0, StartID, 1);
            StartID += FileNum;
            FileNum = 0;
        }

        if (gFileSortBufferIndex)
        {
#if 0
            {
                uint16 i, j;
                for (i = 0; i < 32; i++)
                {
                    printf("\n");
                    for (j = 0; j < 16; j++)
                    {
                        printf("%02x ", gFileSortBuffer[i * 16 + j]);
                    }

                }
                printf("\n");
            }
#endif

            MDWrite(DataDiskID, MediaInfoSaveAdd[0], MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE, gFileSortBuffer);
        }

        //Gpio_SetPinLevel(BACKLIGHT_PIN, 1);           //<... sanshin test
    }

    if (Sysfilenum->gRecordFmFileNum != 0)
    {
        buffer1 = (UINT16*)GerneAblumBuffer;
        buffer2 = (UINT16*)SingerBuffer;
        MediaInfoReadAdd[0] = FILE_NAME_SAVE_ADDR_OFFSET;
        MediaInfoReadAdd[1] = ATTR_SAVE_ADDR_OFFSET;
        MediaInfoSaveAdd[0] = MediaInfoAddress + RECORD_TREE_SORT_INFO_SECTOR_START;
        gFileSortBufferIndex = 0;
        memset(gFileSortBuffer, 0, MEDIAINFO_PAGE_SIZE);

        StartID = 0;
        FileNum = 0;

        //printf("totalnum = %d\n", Sysfilenum->gRecordFmFileNum);
        while (StartID < Sysfilenum->gRecordFmFileNum)
        {
#ifdef _WATCH_DOG_
            WatchDogReload();
#endif
            memset(FileIDBuffer,0,sizeof(FileIDBuffer));           // 4 * SORT_FILE_NAME_BUF_SIZE
            memset(GerneAblumBuffer,0,sizeof(GerneAblumBuffer));   // 8*SORT_FILE_NAME_BUF_SIZE
            memset(SingerBuffer,0,sizeof(SingerBuffer));           // 16*SORT_FILE_NAME_BUF_SIZE
            PathDeep = 2;
            //printf("StartID = %d\n", StartID);
            SortFunction(MediaInfoReadAdd,&FileNum,PathDeep,&uiCountTemp[0],1,buffer1,buffer2,0, StartID, 2);
            StartID += FileNum;
            //printf("FileNum = %d\n", FileNum);
            FileNum = 0;
        }

        if (gFileSortBufferIndex)
        {
#if 0
            {
                uint16 i, j;
                for (i = 0; i < 32; i++)
                {
                    printf("\n");
                    for (j = 0; j < 16; j++)
                    {
                        printf("%02x ", gFileSortBuffer[i * 16 + j]);
                    }

                }
                printf("\n");
            }
#endif

            MDWrite(DataDiskID, MediaInfoSaveAdd[0], MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE, gFileSortBuffer);
        }

        //Gpio_SetPinLevel(BACKLIGHT_PIN, 1);           //<... sanshin test
    }

#ifdef PIC_MEDIA

//sanshin -------------------------------

    if (Sysfilenum->gJpegFileNum/*gJpegTotalFileNum*/ != 0)
    {

        //("%s 0 Sysfilenum->gJpegTotalFileNum = %d \n",__FUNCTION__,Sysfilenum->gJpegTotalFileNum);
        //DEBUG("%s 1 \n",__FUNCTION__);


        buffer1 = (UINT16*)GerneAblumBuffer;
        buffer2 = (UINT16*)SingerBuffer;
        MediaInfoReadAdd[0] = FILE_NAME_SAVE_ADDR_OFFSET;
        MediaInfoReadAdd[1] = ATTR_SAVE_ADDR_OFFSET;
        MediaInfoSaveAdd[0] = MediaInfoAddress + JPEG_TREE_ALL_SORT_INFO_SECTOR_START;  /*<--sanshin 0612*/
        gFileSortBufferIndex = 0;
        memset(gFileSortBuffer, 0, MEDIAINFO_PAGE_SIZE);

        StartID = 0;
//       FileNum = 0;
        FileNum = Sysfilenum->gJpegFileNum;
        //printf("totalnum = %d\n", Sysfilenum->gJpegFileNum);
        while (StartID < Sysfilenum->gJpegFileNum/*gJpegTotalFileNum*/)
        {
#ifdef _WATCH_DOG_
            WatchDogReload();
#endif
            //MedialibUpdateDisplayHook();//PAGE
            memset(FileIDBuffer,0,sizeof(FileIDBuffer));           // 4 * SORT_FILE_NAME_BUF_SIZE
            memset(GerneAblumBuffer,0,sizeof(GerneAblumBuffer));   // 8*SORT_FILE_NAME_BUF_SIZE
            memset(SingerBuffer,0,sizeof(SingerBuffer));           // 16*SORT_FILE_NAME_BUF_SIZE
            PathDeep = 1;
            //printf("StartID = %d\n", StartID);
//            SortFunction(MediaInfoReadAdd,&FileNum,PathDeep,&uiCountTemp[0],0,buffer1,buffer2,0, StartID, 3);
            SortFunction(MediaInfoReadAdd,&FileNum,PathDeep,&uiCountTemp[0],1,buffer1,buffer2,0, StartID, 3);

            StartID += FileNum;
            FileNum = 0;
        }

        if (gFileSortBufferIndex)
        {

            //DEBUG("%s 3 \n",__FUNCTION__);
            MDWrite(DataDiskID, MediaInfoSaveAdd[0], MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE, gFileSortBuffer);
        }

        //Gpio_SetPinLevel(BACKLIGHT_PIN, 1);           //<... sanshin test
    }
    //
    // folder
    //

       buffer1 = (UINT16*)GerneAblumBuffer;                                                                             /*<--sanshin 0612*/
       buffer2 = (UINT16*)SingerBuffer;                                                                                 /*<--sanshin 0612*/
       MediaInfoReadAdd[0] = FILE_NAME_SAVE_ADDR_OFFSET;                                                                /*<--sanshin 0612*/
       MediaInfoReadAdd[1] = ATTR_SAVE_ADDR_OFFSET;                                                                     /*<--sanshin 0612*/
       MediaInfoSaveAdd[0] = MediaInfoAddress + JPEG_TREE_SORT_INFO_SECTOR_START;                                       /*<--sanshin 0612*/
       gFileSortBufferIndex = 0;                                                                                        /*<--sanshin 0612*/
       memset(gFileSortBuffer, 0, MEDIAINFO_PAGE_SIZE);                                                                 /*<--sanshin 0612*/
                                                                                                                        /*<--sanshin 0612*/
       StartID = 0;                                                                                                     /*<--sanshin 0612*/
//       FileNum = 0;                                                                                                   /*<--sanshin 0612*/
    //FileNum = Sysfilenum->gJpegFileNum;                                                                               /*<--sanshin 0612*/
       //printf("totalnum = %d\n", Sysfilenum->gJpegTotalFileNum);                                                    /*<--sanshin 0612*/
       while(StartID < Sysfilenum->gJpegTotalFileNum)                                                                   /*<--sanshin 0612*/
       {                                                                                                                /*<--sanshin 0612*/
#ifdef _WATCH_DOG_
            WatchDogReload();
#endif
            //MedialibUpdateDisplayHook();//PAGE                                                                        /*<--sanshin 0612*/
            memset(FileIDBuffer,0,sizeof(FileIDBuffer));           // 4 * SORT_FILE_NAME_BUF_SIZE                       /*<--sanshin 0612*/
            memset(GerneAblumBuffer,0,sizeof(GerneAblumBuffer));   // 8*SORT_FILE_NAME_BUF_SIZE                         /*<--sanshin 0612*/
            memset(SingerBuffer,0,sizeof(SingerBuffer));           // 16*SORT_FILE_NAME_BUF_SIZE                        /*<--sanshin 0612*/
            PathDeep = 2;                                                                                               /*<--sanshin 0612*/
            //printf("StartID = %d\n", StartID);                                                                        /*<--sanshin 0612*/
//            SortFunction(MediaInfoReadAdd,&FileNum,PathDeep,&uiCountTemp[0],0,buffer1,buffer2,0, StartID, 3);         /*<--sanshin 0612*/
            SortFunction(MediaInfoReadAdd,&FileNum,PathDeep,&uiCountTemp[0],1,buffer1,buffer2,0, StartID, 4);           /*<--sanshin 0612*/
                                                                                                                        /*<--sanshin 0612*/
            StartID += FileNum;                                                                                         /*<--sanshin 0612*/
            FileNum = 0;                                                                                                /*<--sanshin 0612*/
       }                                                                                                                /*<--sanshin 0612*/
                                                                                                                        /*<--sanshin 0612*/
       if(gFileSortBufferIndex)                                                                                         /*<--sanshin 0612*/
       {                                                                                                                /*<--sanshin 0612*/
           #if 0                                                                                                        /*<--sanshin 0612*/
           {                                                                                                            /*<--sanshin 0612*/
                uint16 i, j;                                                                                            /*<--sanshin 0612*/
                for(i = 0; i < 32; i++)                                                                                 /*<--sanshin 0612*/
                {                                                                                                       /*<--sanshin 0612*/
                    printf("\n");                                                                                       /*<--sanshin 0612*/
                    for(j = 0; j < 16; j++)                                                                             /*<--sanshin 0612*/
                    {                                                                                                   /*<--sanshin 0612*/
                        printf("%02x ", gFileSortBuffer[i * 16 + j]);                                                   /*<--sanshin 0612*/
                    }                                                                                                   /*<--sanshin 0612*/
                                                                                                                        /*<--sanshin 0612*/
                }                                                                                                       /*<--sanshin 0612*/
                printf("\n");                                                                                           /*<--sanshin 0612*/
           }                                                                                                            /*<--sanshin 0612*/
           #endif                                                                                                       /*<--sanshin 0612*/
                                                                                                                        /*<--sanshin 0612*/
    //DEBUG("%s 3 \n",__FUNCTION__);                                                                                    /*<--sanshin 0612*/
           MDWrite(DataDiskID, MediaInfoSaveAdd[0], MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE, gFileSortBuffer);             /*<--sanshin 0612*/
       }                                                                                                                /*<--sanshin 0612*/


/*<--sanshin 0612*/

//sanshin -------------------------------

#endif

//-----------------------------------------------------------------------
//---->sanshin_20150616
#ifdef _M3U_
    if(Sysfilenum->gM3uFileNum != 0)                                                                                    //<----sanshin_20150616
    {                                                                                                                   //<----sanshin_20150616
        buffer1 = (UINT16*)GerneAblumBuffer;                                                                            //<----sanshin_20150616
        buffer2 = (UINT16*)SingerBuffer;                                                                                //<----sanshin_20150616
        MediaInfoReadAdd[0] = FILE_NAME_SAVE_ADDR_OFFSET;                                                               //<----sanshin_20150616
        MediaInfoReadAdd[1] = ATTR_SAVE_ADDR_OFFSET;                                                                    //<----sanshin_20150616
        MediaInfoSaveAdd[0] = MediaInfoAddress + M3U_TREE_SORT_INFO_SECTOR_START;                                       //<----sanshin_20150616
        gFileSortBufferIndex = 0;                                                                                       //<----sanshin_20150616
        memset(gFileSortBuffer, 0, MEDIAINFO_PAGE_SIZE);                                                                //<----sanshin_20150616
                                                                                                                        //<----sanshin_20150616
        StartID = 0;                                                                                                    //<----sanshin_20150616
        FileNum = Sysfilenum->gM3uFileNum;                                                                              //<----sanshin_20150616
        while(StartID < Sysfilenum->gM3uFileNum)                                                                        //<----sanshin_20150616
        {                                                                                                               //<----sanshin_20150616
#ifdef _WATCH_DOG_
            WatchDogReload();
#endif
            memset(FileIDBuffer,0,sizeof(FileIDBuffer));           // 4 * SORT_FILE_NAME_BUF_SIZE                       //<----sanshin_20150616
            memset(GerneAblumBuffer,0,sizeof(GerneAblumBuffer));   // 8*SORT_FILE_NAME_BUF_SIZE                         //<----sanshin_20150616
            memset(SingerBuffer,0,sizeof(SingerBuffer));           // 16*SORT_FILE_NAME_BUF_SIZE                        //<----sanshin_20150616
            PathDeep = 1;                                                                                               //<----sanshin_20150616
            SortFunction(MediaInfoReadAdd,&FileNum,PathDeep,&uiCountTemp[0],1,buffer1,buffer2,0, StartID, 5);           //<----sanshin_20150616
                                                                                                                        //<----sanshin_20150616
            StartID += FileNum;                                                                                         //<----sanshin_20150616
            FileNum = 0;                                                                                                //<----sanshin_20150616
        }                                                                                                               //<----sanshin_20150616
                                                                                                                        //<----sanshin_20150616
        if(gFileSortBufferIndex)                                                                                        //<----sanshin_20150616
        {                                                                                                               //<----sanshin_20150616
            MDWrite(DataDiskID, MediaInfoSaveAdd[0], MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE, gFileSortBuffer);            //<----sanshin_20150616
        }                                                                                                               //<----sanshin_20150616
   }                                                                                                                    //<----sanshin_20150616
#endif
//<----sanshin_20150616
//-----------------------------------------------------------------------
}


#endif

/*
********************************************************************************
*
*                         End of FileInfoSort.c
********************************************************************************
*/
