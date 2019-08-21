/* Copyright (C) 2009 ROCK-CHIPS FUZHOU . All Rights Reserved. */
/*
File    : \Audio\pMP3.h
Desc    : the flow chart of mp3 decode control.

Author  : Vincent Hsiung (xw@rock-chips.com)
Date    : Mar 10 , 2009
Notes   : 

$Log    : 
* 
*
*/
/****************************************************************/

#include <stdio.h>
#include <string.h>	//for memcpy(),memmove()

#ifndef _MP2_H_
#define _MP2_H_

//-----------------------------------------------------------------
typedef struct
{
    long samplerate ;
    long bitrate ;
    long channels ;
    long length ;
    long outlength ;
	unsigned char ucIsVBR;
	unsigned char ucChannels;
	unsigned short usSampleRate;
	unsigned long ulFirstFrame;
	unsigned long ulLength;
	unsigned long ulBitRate;
	unsigned long ulTimePos;
	unsigned long ulTimeLength;
	unsigned long ulOutputLength;
}tMP2;

extern short * mp2_get_outbuffer();
extern long mp2_get_outlength();

#endif
