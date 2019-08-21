/*****************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  - Portable macros, types and inlined assembly -
 *
 *  Copyright(C) 2002      Michael Militzer <isibaar@xvid.org>
 *               2002-2003 Peter Ross <pross@xvid.org>
 *               2002-2003 Edouard Gomez <ed.gomez@free.fr>
 *
 *  This program is free software ; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation ; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY ; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program ; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * $Id: portab.h,v 1.54.2.1 2006/07/10 15:05:30 Isibaar Exp $
 *
 ****************************************************************************/

#ifndef _PORTAB_H_
#define _PORTAB_H_

/*****************************************************************************
 *  Platform Defines
 ****************************************************************************/

#include "../xvid_dec_main.h"


/*****************************************************************************
 *  Global Defines
 ****************************************************************************/

#define ARCH_IS_IA32

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/*****************************************************************************
 *  Common things
 ****************************************************************************/

/* Buffer size for msvc implementation because it outputs to DebugOutput */
//#if defined(_DEBUG)
//extern unsigned int xvid_debug;
//#define DPRINTF_BUF_SZ  1024
//#endif

/*****************************************************************************
 *  Types used in XviD sources
 ****************************************************************************/

#define int8_t   char
#define uint8_t  unsigned char
#define int16_t  short
#define uint16_t unsigned short
#define int32_t  int
#define uint32_t unsigned int
#define int64_t  __int64
#define uint64_t unsigned __int64 


/*****************************************************************************
 *  Some things that are only architecture dependant
 ****************************************************************************/

#define CACHE_LINE 64 
#define ptr_t uint32_t
#define intptr_t int32_t

/*****************************************************************************
 *  Things that must be sorted by compiler and then by architecture
 ****************************************************************************/

/*
 * Ok we know nothing about the compiler, so we fallback to ANSI C
 * features, so every compiler should be happy and compile the code.
 *
 * This is (mostly) equivalent to ARCH_IS_GENERIC.
 */

#ifdef VC_PLATFORM
#define DEBUG DEBUG
#define DPRINTF

#define XVID_ASSERT assert
#endif

#ifdef MDK_PLATFORM
#define DEBUG
#define DPRINTF

//#define XVID_ASSERT(x) while(x == 0) 
#define XVID_ASSERT(x) (xvid_assert_fail = !(x))
#endif



#define BSWAP(a) \
    ((a) = (((a) & 0xff) << 24)  | (((a) & 0xff00) << 8) | \
           (((a) >> 8) & 0xff00) | (((a) >> 24) & 0xff))

#define DECLARE_ALIGNED_MATRIX(name,sizex,sizey,type,alignment) \
    type name[(sizex)*(sizey)]



#endif /* PORTAB_H */
