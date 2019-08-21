/*
********************************************************************************
*                   Copyright (c) 2008, Rockchip
*                         All rights reserved.
*
* File Name£º   fixed.c
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             Vincent Hsiung     2009-1-14         1.0
*    desc:    ORG.
********************************************************************************
*/

/*

 1.É¾³ý mad_f_div()º¯ÊýÌå.
 
 by Vincent Hsiung.
 
*/

#include "../include/audio_main.h"

#ifdef A_CORE_DECODE
#ifdef MP3_DEC_INCLUDE

# include "mp3_global.h"
# include "mp3_fixed.h"

/*
 * NAME:	fixed->abs()
 * DESCRIPTION:	return absolute value of a fixed-point number
 */
_ATTR_MP3DEC_TEXT_
mad_fixed_t mad_f_abs(mad_fixed_t x)
{
  return x < 0 ? -x : x;
}

/*
 * NAME:	fixed->div()
 * DESCRIPTION:	perform division using fixed-point math
 */
//By Vincent , REMOVED, Don't use mad_f_div().
_ATTR_MP3DEC_TEXT_
mad_fixed_t mad_f_div(mad_fixed_t x, mad_fixed_t y)
{
  return 0;
}

#endif
#endif

