/*
********************************************************************************
*                   Copyright (c) 2008, Rockchip
*                         All rights reserved.
*
* File Name£º   global.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             Vincent Hsiung     2009-1-14         1.0
*    desc:    ORG.
********************************************************************************
*/

#include "../include/audio_globals.h"

# ifndef LIBMAD_GLOBAL_H
# define LIBMAD_GLOBAL_H

/*
//section define
#define _ATTR_MP3DEC_TEXT_     __attribute__((section("MP3DEC_CODE_SEG")))
#define _ATTR_MP3DEC_DATA_     __attribute__((section("MP3DEC_DATA_SEG")))
#define _ATTR_MP3DEC_BSS_      __attribute__((section("MP3DEC_BSS_SEG"),zero_init))
*/
#ifdef VIDEO_MP2_DECODE
    #define MP2_INCLUDE
#endif

#define _ATTR_MP2DEC_TEXT_     __attribute__((section("Mp2Code"/*"MP3DEC_CODE_SEG"*/)))
#define _ATTR_MP2DEC_DATA_     __attribute__((section("Mp2Data"/*"MP3DEC_DATA_SEG"*/)))
#define _ATTR_MP2DEC_BSS_      __attribute__((section("Mp2Bss"/*"MP3DEC_BSS_SEG"*/),zero_init))

extern void abort(void);

#define FPM_DEFAULT
#define OPT_SSO
#define OPT_SPEED

/* conditional debugging */

# if defined(DEBUG) && defined(NDEBUG)
#  error "cannot define both DEBUG and NDEBUG"
# endif

# if defined(DEBUG)
#  include <stdio.h>
# endif

/* conditional features */

# if defined(OPT_SPEED) && defined(OPT_ACCURACY)
#  error "cannot optimize for both speed and accuracy"
# endif

# if defined(OPT_SPEED) && !defined(OPT_SSO)
#  define OPT_SSO
# endif

# if defined(HAVE_UNISTD_H) && defined(HAVE_WAITPID) &&  \
    defined(HAVE_FCNTL) && defined(HAVE_PIPE) && defined(HAVE_FORK)
#  define USE_ASYNC
# endif


#define assert(x)	/* nothing */

# endif
