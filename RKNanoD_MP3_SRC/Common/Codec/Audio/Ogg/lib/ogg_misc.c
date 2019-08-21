/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2002             *
 * by the XIPHOPHORUS Company http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************/

#define HEAD_ALIGN 32
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define MISC_C
#include "misc.h"
#include "audio_main.h"

#ifdef A_CORE_DECODE
#ifdef OGG_DEC_INCLUDE

_ATTR_OGGDEC_TEXT_
char DecCode_ogg_table[17900];//只用于 s->dec_table ;
_ATTR_OGGDEC_TEXT_
char alloca_buff[1024*12];// 只为floor0的lsp_to_curve 提供内存分配
#pragma arm section code = "OggDecCode", rodata = "OggDecCode", rwdata = "OggDecData", zidata = "OggDecBss"


long mem_cnt = 0;
long mem_aft_free_cnt = 0;	
long sum = 0;






static void **pointers=NULL;
static long *insertlist=NULL; /* We can't embed this in the pointer list;
			  a pointer can have any value... */

static char **files=NULL;
static long *file_bytes=NULL;
static int  filecount=0;

static int ptop=0;
static int palloced=0;
static int pinsert=0;
int malloc_ogg_pos=0;
typedef struct {
  char *file;
  long line;
  long ptr;
  long bytes;
} head;

long global_bytes=0;
long start_time=-1;


//extern char*alloca_buff;
static long alloca_pos = 0;
void RK_ogg_alloca_exit()
{
	alloca_pos = 0;
}

void *RK_malloc(int n)
{
   malloc_ogg_pos +=n;
   if(malloc_ogg_pos >= 17900)
   {
   //	printf("码表内存不够\n");
   }
   return &DecCode_ogg_table[malloc_ogg_pos-n];
}
void *RK_ogg_alloca(long n)
{
	///return test_malloc(n);
	if (n&3)
		{
			n = n - (n&3) + 4;
		}
	//*/
	if (alloca_pos + n < 1024*12)
	{		
		
		alloca_pos += n;
		return &alloca_buff[alloca_pos-n];		
	}
	else
	{
		alloca_pos = n;
		
		return &alloca_buff[0];
	}
}


#pragma arm section code

#endif
#endif

