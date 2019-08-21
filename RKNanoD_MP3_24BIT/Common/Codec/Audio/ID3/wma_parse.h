/******************************************************************************
*
*  Copyright (C),2007, Fuzhou Rockchip Co.,Ltd.
*
*  File name :     wma_parse.h
*  Description:   get the id3 information of wma song.
*  Remark:
*
*  History:
*           <author>      <time>     <version>       <desc>
*           Huweiguo     07/11/05      1.0
*
*******************************************************************************/

#ifndef __WMA_PARSE__
#define __WMA_PARSE__

#include "sysinclude.h"
#include "AddrSaveMacro.h"

#if 1   //if FILE_INFO_SAVE_DATA beyond systemdata size use this define
typedef struct
{
    short Title[MEDIA_ID3_SAVE_CHAR_NUM/2];
    short Album[MEDIA_ID3_SAVE_CHAR_NUM/2];
    short Author[MEDIA_ID3_SAVE_CHAR_NUM/2];
    short Genre[MEDIA_ID3_SAVE_CHAR_NUM/2];
    short Track[MEDIA_ID3_SAVE_CHAR_NUM/2];

} ID3;
#else
typedef struct
{
    short Title[MEDIA_ID3_SAVE_CHAR_NUM];
    short Album[MEDIA_ID3_SAVE_CHAR_NUM];
    short Author[MEDIA_ID3_SAVE_CHAR_NUM];
    short Genre[MEDIA_ID3_SAVE_CHAR_NUM];
    short Track[MEDIA_ID3_SAVE_CHAR_NUM];

} ID3;
#endif


#define AP_FILE int

#define AP_SUCESS               0
#define AP_FAIL                 1

#define STREAM_BUF_SIZE    512

typedef struct
{
    char Stream[STREAM_BUF_SIZE];
    long cbOffset;
    long cbSize;
} sByteStreamCtl;

int WMA_ParseAsf_Header(AP_FILE fhandle, ID3 *id3);

#endif
