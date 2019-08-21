/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   audio_globals.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             Vincent Hsiung     2009-1-8          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _AUDIO_MP2CODEC_H_

#define _AUDIO_MP2CODEC_H_

#include "SysConfig.h"
#include <stdio.h>

#ifdef VIDEO_MP2_DECODE
    #define MP2_INCLUDE
#endif

#define _ATTR_MP2DEC_TEXT_     __attribute__((section("Mp2Code"/*"MP3DEC_CODE_SEG"*/)))
#define _ATTR_MP2DEC_DATA_     __attribute__((section("Mp2Data"/*"MP3DEC_DATA_SEG"*/)))
#define _ATTR_MP2DEC_BSS_      __attribute__((section("Mp2Bss"/*"MP3DEC_BSS_SEG"*/),zero_init))


typedef unsigned int size_t;

extern size_t   (*AVI_RKFIO_FRead)(void * /*buffer*/, size_t /*length*/,FILE *) ;
extern int      (*AVI_RKFIO_FSeek)(long int /*offset*/, int /*whence*/ ,FILE * /*stream*/);
extern long int (*AVI_RKFIO_FTell)(FILE * /*stream*/);
extern size_t   (*AVI_RKFIO_FWrite)(void * /*buffer*/, size_t /*length*/,FILE * /*stream*/);
extern unsigned long (*AVI_RKFIO_FLength)(FILE *in /*stream*/);
extern int      (*AVI_RKFIO_FClose)(FILE * /*stream*/);

extern FILE *AVI_pRawFileCache;

#define RKFIO_FRead	AVI_RKFIO_FRead
#define RKFIO_FSeek	AVI_RKFIO_FSeek
#define RKFIO_FLength AVI_RKFIO_FLength




//****************************************************************************
// The following values are the IOCTLs which are sent to the individual codec
// drivers.
//****************************************************************************

enum
{
    MP2_CODEC_GETNAME,
    MP2_CODEC_GETARTIST,
    MP2_CODEC_GETTITLE,
    MP2_CODEC_GETBITRATE,
    MP2_CODEC_GETSAMPLERATE,
    MP2_CODEC_GETCHANNELS,
    MP2_CODEC_GETLENGTH,
    MP2_CODEC_GETTIME,
    MP2_CODEC_OPEN_DEC,
    MP2_CODEC_OPEN_ENC,
    MP2_CODEC_GETBUFFER,
    MP2_CODEC_SETBUFFER,
    MP2_CODEC_DECODE,
    MP2_CODEC_ENCODE,
    MP2_CODEC_SEEK,
    MP2_CODEC_CLOSE,
    MP2_CODEC_ZOOM
};

//****************************************************************************
// The following are the flags passed to CodecOpen.
//****************************************************************************

//****************************************************************************
// The following are the flags passed to CodecSeek.
//****************************************************************************
#define CODEC_SEEK_FF           0x00000000
#define CODEC_SEEK_REW          0x00000001
#define CODEC_SEEK_OTHER        0x00000002

//****************************************************************************
// Function prototypes and global variables.
//****************************************************************************

// pMP2.C
extern unsigned long MP2Function(unsigned long ulSubFn, unsigned long ulParam1,
                                 unsigned long ulParam2,unsigned long ulParam3);


// From codec.c
///////////////////////////////////////////////////////////////////////////////
extern unsigned long Mp2CodecOpen(unsigned long ulCodec, unsigned long ulFlags);
extern unsigned long Mp2CodecGetSampleRate(unsigned long *pulSampleRate);
extern unsigned long Mp2CodecGetChannels(unsigned long *pulChannels);
extern unsigned long Mp2CodecGetLength(unsigned long *pulLength);
extern unsigned long Mp2CodecGetTime(unsigned long *pulTime);
extern unsigned long Mp2CodecGetCaptureBuffer(short *ppsBuffer, long *plLength);
extern unsigned long Mp2CodecSetBuffer(short *psBuffer);
extern unsigned long Mp2CodecDecode(void);
extern unsigned long Mp2CodecSeek(unsigned long ulTime, unsigned long ulSeekType);
extern unsigned long Mp2CodecClose(void);
extern unsigned long Mp2CodecGetBitrate(unsigned long *pulBitrate);


 extern int mp2_open(int isExact);
 extern int mp2_decode();
 extern int mp2_get_buffer(short** ulParam1 , int *ulParam2);
 extern int mp2_get_samplerate();
 extern int  mp2_get_bitrate();
 extern int mp2_get_length();
 extern int mp2_get_channels();


//------------------------------------------------------------




#endif

