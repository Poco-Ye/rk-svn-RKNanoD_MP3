/*++

Copyright (c) 2006  xxx

Module Name:

    flac_dec_block.c

Abstract:

    flac decode function

Environment:

    xxxx

Notes:

   Copyright (c) 2006 xxx.  All Rights Reserved.


Revision History:

    Created 7-7-2006, by Fibo

--*/

#include "../../include/audio_main.h"

#ifdef A_CORE_DECODE
#ifdef FLAC_DEC_INCLUDE

#include <ctype.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../../include/audio_file_access.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

//#if !defined _MSC_VER && !defined __MINGW32__
/* unlink is in stdio.h in VC++ */
//#include <unistd.h> /* for unlink() */
//#else
#define strcasecmp stricmp
//#endif
#include "all.h"
//#include "share/grabbag.h"
#include "analyze.h"
#include "decode.h"
//#include "encode.h"
//#include "local_string_utils.h" /* for flac__strlcat() and flac__strlcpy() */
#include "utils.h"
//#include "vorbiscomment.h"


extern unsigned long g_CurrFileOffset;

#if 1//def WIN32

#define ext_malloc  malloc
#define ext_free  free

#define ext_fopen  fopen
#define ext_fread  fread
#define ext_fwrite  fwrite
#define ext_fseek  fseek
#define ext_fclose  fclose
#define ext_feof  feof
#define ext_ftell  ftell

#define ext_ferror  ferror
#define ext_fflush  fflush


#else

#ifdef _FLAC_DEC_TCC

//tcc memory function
extern void *tcc_malloc(unsigned int size) ;
extern void tcc_free(void *memblock) ;

//tcc file function
extern void *tcc_fopen(const char *filename, const char *mode) ;
extern unsigned int tcc_fread(void *buffer, unsigned int size, unsigned int count, void *stream) ;
extern unsigned int tcc_fwrite(const void *buffer, unsigned int size, unsigned int count, FILE *stream) ;
extern int tcc_fseek(void *stream, long offset, int origin) ;
extern int tcc_fclose(void *stream) ;

//extern interface
#define ext_malloc  tcc_malloc
#define ext_free  tcc_free

#define ext_fopen  tcc_fopen
#define ext_fread  tcc_fread
#define ext_fwrite  tcc_fwrite
#define ext_fseek  tcc_fseek
#define ext_fclose  tcc_fclose
#define ext_feof  tcc_feof
#define ext_ftell  tcc_ftell


#endif

#endif

_ATTR_FLACDEC_TEXT_
unsigned int flac_fread( void *buffer, unsigned int size, unsigned int count, FILE *stream )
{
 int ret;

 g_CurrFileOffset += count;
 ret = RKFIO_FRead(buffer, size * count, stream) ;
 return (ret/size);
 //return ext_fread (buffer, size, count, FILE_SUB(stream)) ;
}

_ATTR_FLACDEC_TEXT_
unsigned int flac_fwrite(const void *buffer, unsigned int size, unsigned int count, FILE *stream)
{
#if 0
    //return ext_fwrite(buffer, size, count, FILE_SUB(stream)) ;
    unsigned int ret = ext_fwrite(buffer, size, count, g_hWaveFile) ;
    fflush(stream);
    return ret;
#endif
}

_ATTR_FLACDEC_TEXT_
int flac_fseek(FILE *stream, long offset, int origin)
{
	if(0 == origin)
	  g_CurrFileOffset = offset;
	else
		while(1);
    return RKFIO_FSeek(offset, origin, stream);
	//ext_fseek(stream, offset, FILE_SUB(origin)) ;
}
_ATTR_FLACDEC_TEXT_
int flac_fclose(FILE *stream)
{
    return RKFIO_FClose(stream);//ext_fclose(FILE_SUB(stream)) ;
}

#if 0
int flac_feof(FILE *stream)
{
    return ext_feof(FILE_SUB(stream)) ;
}
#endif


_ATTR_FLACDEC_TEXT_
int flac_ftell(FILE *stream)
{
    return RKFIO_FTell(stream);//ext_ftell(FILE_SUB(stream)) ;
}


_ATTR_FLACDEC_TEXT_
int flac_ferror(FILE *stream)
{
    //return ext_ferror (FILE_SUB(stream)) ;
    return false ;
}

_ATTR_FLACDEC_TEXT_
int flac_fflush(FILE *stream)
{
    //return ext_fflush (FILE_SUB(stream)) ;
    return 0 ;
}

#endif
#endif
