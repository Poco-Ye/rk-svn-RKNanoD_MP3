/******************************************************************************
*
*  Copyright (C),2007, Fuzhou Rockchip Co.,Ltd.
*
*  File name :     flacbuffer.c
*  Description:    FLAC解码器的输入输出Buffer.
*  Remark:
*
*  History:
*           <author>      <time>     <version>       <desc>
*           Huweiguo      07/04/23      1.0
*
*******************************************************************************/

#include "../../include/audio_main.h"

#ifdef A_CORE_DECODE
#ifdef FLAC_DEC_INCLUDE

#include "flacbuffer.h"
// 输入
//unsigned short g_FlacInputBuffer[FLAC__BITBUFFER_DEFAULT_CAPACITY+FLAC__BITBUFFER_MAX_REMAINDER]; //FLAC__BITBUFFER_MAX_REMAINDER 用于存放buffer中残留的数据
_ATTR_FLACDEC_BSS_
unsigned char g_FlacInputBuffer[FLAC__BITBUFFER_DEFAULT_CAPACITY+FLAC__BITBUFFER_MAX_REMAINDER]; //FLAC__BITBUFFER_MAX_REMAINDER 用于存放buffer中残留的数据




#ifdef HALF_FRAME_BY_HALF_FRAME
_ATTR_FLACDEC_BSS_
unsigned long g_FlacOutputBuffer[FLAC__MAX_CHANNEL][FLAC__PCMBUFFER_DEFAULT_CAPACITY/2+FLAC__MAX_LPC_ORDER+FLAC__MAX_LPC_ORDER];
#else
_ATTR_FLACDEC_BSS_
unsigned long g_FlacOutputBuffer[FLAC__MAX_CHANNEL][FLAC__PCMBUFFER_DEFAULT_CAPACITY+FLAC__MAX_LPC_ORDER+FLAC__MAX_LPC_ORDER];              
#endif


#ifdef HALF_FRAME_BY_HALF_FRAME
/* add ping-pong buffer here , by Vincent @ Apr 9 , 2009 */
_ATTR_FLACDEC_BSS_
//__align (32)
//unsigned short g_FlacCodecBuffer[2][(FLAC__PCMBUFFER_DEFAULT_CAPACITY+FLAC__MAX_LPC_ORDER)];
unsigned short *  g_FlacCodecBuffer[2];
#else
_ATTR_FLACDEC_BSS_
unsigned short g_FlacCodecBuffer[(FLAC__PCMBUFFER_DEFAULT_CAPACITY+FLAC__MAX_LPC_ORDER)*2];
#endif


#endif
#endif
