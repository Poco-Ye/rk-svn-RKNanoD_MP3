/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��   audio_file_access.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             Vincent Hsiung     2009-1-8          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _AUDIO_FILE_ACCESS_H_
#define _AUDIO_FILE_ACCESS_H_

#include <stdio.h>

#define RKNANO_FS

#define SEEK_SET 0 /* start of stream (see fseek) */
#define SEEK_CUR 1 /* current position in stream (see fseek) */
#define SEEK_END 2 /* end of stream (see fseek) */
#define NULL     0

#define AUDIO_FILE_PIPO_BUF_SIZE         (1024 * 22)     //WMA output buffer 12k
#define HIFI_AUDIO_FILE_PIPO_BUF_SIZE    (1024 * 22)     // File data buffer 8K
                                                        // WMA output buffer 12k
                                                        // DcFilter need (64+128+320) *4 = 2k byte

#define AUDIO_BUF_SIZE              (2 * AUDIO_FILE_PIPO_BUF_SIZE)
#define HIFI_AUDIO_BUF_SIZE         (2 * HIFI_AUDIO_FILE_PIPO_BUF_SIZE)

#ifdef WAV_DEC_INCLUDE
#include  "pcm.h"
#define     WAV_AD_PIPO_BUFFER_SIZE             6144
#endif

typedef unsigned int size_t;

extern signed char   (*RKFIO_FOpen)();
extern unsigned long (*RKFIO_FRead)(void * /*buffer*/, size_t /*length*/,FILE *) ;
extern unsigned char (*RKFIO_FSeek)(long int /*offset*/, int /*whence*/ ,FILE * /*stream*/);
extern long int      (*RKFIO_FTell)(FILE * /*stream*/);
extern unsigned long (*RKFIO_FWrite)(void * /*buffer*/, size_t /*length*/,FILE * /*stream*/);
extern unsigned long (*RKFIO_FLength)(FILE *in /*stream*/);
extern unsigned char (*RKFIO_FClose)(FILE * /*stream*/);
extern int (*RKFIO_FEof)(FILE *);

extern int FLAC_FileSeekFast(int offset, int clus, FILE *in);
extern int FLAC_FileGetSeekInfo(int *pOffset, int *pClus, FILE *in);


extern FILE *pRawFileCache,*pFlacFileHandleBake,*pAacFileHandleSize,*pAacFileHandleOffset;
#endif
