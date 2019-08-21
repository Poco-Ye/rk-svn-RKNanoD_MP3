/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：   audio_file_access.c
* 
* Description:  Audio File Operation Interface
*
* History:      <author>          <time>        <version>       
*             Vincent Hsiung    2009-01-08         1.0
*    desc:    ORG.
********************************************************************************
*/

#pragma arm section code = "AviDecCode", rodata = "AviDecCode", rwdata = "AviDecData", zidata = "AviDecBss"

#include <stdio.h>
#include <string.h>

#include "FsInclude.h"
#include "File.h"
#include "SysConfig.h"

#ifdef _VIDEO_

#define NANO_FILE_SYS
//#define PC_FILE_SYS

//#define SIMULATION
//#define	MEMFILE_SIM
FILE *AVI_pRawFileCache;



typedef unsigned int size_t;

size_t   (*AVI_RKFIO_FRead)(void * /*buffer*/, size_t /*length*/,FILE *) ;

int      (*AVI_RKFIO_FSeek)(long int /*offset*/, int /*whence*/ , FILE * /*stream*/);

long int (*AVI_RKFIO_FTell)(FILE * /*stream*/);

size_t   (*AVI_RKFIO_FWrite)(void * /*buffer*/, size_t /*length*/,FILE * /*stream*/);

unsigned long (*AVI_RKFIO_FLength)(FILE *in /*stream*/);
int      (*AVI_RKFIO_FClose)(FILE * /*stream*/);

#ifdef PC_FILE_SYS

unsigned int AVI_FRead(void * buffer, size_t length,FILE * file) 
{
	return fread(buffer, 1,length,file) ;
	
}

unsigned int AVI_FWrite(void * buffer, size_t length,FILE * file) 
{
	return fwrite(buffer, length,1,file) ;
	
}

int AVI_FSeek(long int offset, int whence , FILE * stream)
{
	return fseek(stream,offset,whence);
}

long int AVI_FTell(FILE * stream)
{
	return ftell(stream);
}

int AVI_FOpen( const char *filename, const char *Type)
{
	return fopen(filename,Type);
}

int AVI_FClose(FILE * stream)
{
	return fclose(stream);
}

int AVI_FileEof(FILE * stream)
{
	return feof(stream);
}


#endif

#ifdef NANO_FILE_SYS

#include "SysInclude.h"
#include "FsInclude.h"

unsigned int AVI_FRead(void * buffer, size_t length, UINT32 file) 
{
	return FileRead(buffer, length, (HANDLE)file);
}

unsigned int AVI_FWrite(void * buffer, size_t length, UINT32 file) 
{

}

int AVI_FSeek(long int offset, int whence , UINT32 file)
{
	return FileSeek(offset, whence, (HANDLE)file);
}

long int AVI_FTell(UINT32 file)
{
	return FileInfo[file].Offset;
}

int AVI_FOpen(char *filename, char *Type)
{
	return FileOpenA("U:\\", filename, Type);
}

int AVI_FClose(UINT32 file)
{
	FileClose((HANDLE)file);
}

int AVI_FileEof(UINT32 file)
{
	return FileEof((HANDLE)file);
}

//FILE *pRawFileCache;
unsigned int AVI_FReadAudioData(void * buffer, size_t length, UINT32 file) 
{
   return AviGetAudioData( buffer, length);
}

unsigned long AVI_Audio_RKFLength(FILE *in)
{
    return 0x7FFFFFFF;
}

int AVI_Audio_FSeek(long int offset, int whence , UINT32 file)
{
#if 0
	if(offset != 0)
	{
		while(1); // 目前还不支持
	}
	if(whence != 0)
	{
		while(1); // 目前还不支持
	}
#endif	
	return 0;
}

unsigned long AVI_Audio_RKFTell(FILE *in)
{
    return 0;
}

int AVI_Audio_FClose(UINT32 file)
{
	return 0;
}

#endif



unsigned long AVI_RKFLength(FILE *in)
{
    return (FileInfo[(int)in].FileSize); // modified by huweiguo, 09/04/11

    //return 0x851121;
}


unsigned long AVI_RKFTell(FILE *in)
{
    return (FileInfo[(int)in].Offset);
}


void Avi_Audio_FileFuncInit(void)
{
	AVI_RKFIO_FLength = AVI_Audio_RKFLength;

	AVI_RKFIO_FRead = AVI_FReadAudioData;//AVI_FRead;
	AVI_RKFIO_FSeek = AVI_Audio_FSeek;
	AVI_RKFIO_FTell = AVI_Audio_RKFTell;
	AVI_RKFIO_FClose = AVI_Audio_FClose;

}
#endif 
#pragma arm section code

