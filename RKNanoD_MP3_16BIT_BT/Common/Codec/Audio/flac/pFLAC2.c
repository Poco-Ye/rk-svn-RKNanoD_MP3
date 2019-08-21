/* Copyright (C) 2009 ROCK-CHIPS FUZHOU . All Rights Reserved. */
/*
File    : \Audio\pFLAC.c
Desc    : floe chart of FLAC decode

Author  : Vincent Hsiung (xw@rock-chips.com)
Date    : Apr 10 , 2009
Notes   :

$Log    :
*
*
*/
/****************************************************************/

#include "../include/audio_main.h"
#include "../include/audio_globals.h"
#include "../include/audio_file_access.h"

#ifdef FLAC_DEC_INCLUDE

#include <stdio.h>
#include <string.h> //for memcpy(),memmove()

//*************************************************************************************************************//
//the achievement of functions.£º
//SUBFN_CODEC_GETNAME  :   get decoder name
//SUBFN_CODEC_GETARTIST:   get artist name.
//SUBFN_CODEC_GETTITLE :   get song title.
//SUBFN_CODEC_GETBITRATE:  get bit rate.
//SUBFN_CODEC_GETSAMPLERATE: get sample rate.
//SUBFN_CODEC_GETCHANNELS: get channel number.
//SUBFN_CODEC_GETLENGTH :  get total play time [unit:ms]
//SUBFN_CODEC_GETTIME  :   get current play time.[unit:ms].note:this time get by timestamp,there may be error if file is been demage..
//SUBFN_CODEC_OPEN_DEC :   open deooder(initialization.)
//SUBFN_CODEC_DECODE   :   deocode.
//SUBFN_CODEC_ENCODE   :   not support.
//SUBFN_CODEC_SEEK     :   location by time directly.[unit:ms]
//SUBFN_CODEC_CLOSE    :   close decoder.
//SUBFN_CODEC_SETBUFFER:   set cache area,point out the position to put save result.
/******************************************************
Name:
Desc:
Param: ulIoctl child function number.
    ulParam1 child function parameter 1.
    ulParam2 child function parameter 2.
    ulParam3 child function parameter 3.
    ulParam4 child function parameter 4.

Return:
Global:
Note:
Author:
Log:
******************************************************/
_ATTR_FLACDEC_TEXT_
static unsigned int CheckID3V2Tag(unsigned  char *pucBuffer)
{
    // The first three bytes of the tag should be "ID3".
    if ((pucBuffer[0] !=    'I') || (pucBuffer[1] != 'D') || (pucBuffer[2] != '3'))
    {
        return(0);
    }

    // The next byte should be the value 3 (i.e. we support ID3v2.3.0).
    //if(pucBuffer[3]   != 3)
    if (pucBuffer[3]    <2  && pucBuffer[3]> 4)
    {
        return(0);
    }

    // The next byte should be less than 0xff.
    if (pucBuffer[4]    == 0xff)
    {
        return(0);
    }

    // We don't care about the next byte.  The following four bytes should be
    // less than 0x80.
    if ((pucBuffer[6] >=    0x80) || (pucBuffer[7] >= 0x80) ||
            (pucBuffer[8] >=    0x80) || (pucBuffer[9] >= 0x80))
    {
        return(0);
    }

    // Return the length of the ID3v2 tag.
    return((pucBuffer[6] << 21) | (pucBuffer[7] << 14) |
           (pucBuffer[8] <<  7) |  pucBuffer[9]);
}


_ATTR_FLACDEC_DATA_ int ID3_len = 0;

_ATTR_FLACDEC_TEXT_
unsigned long FLACDecFunction2(unsigned long ulIoctl, unsigned long ulParam1,
            unsigned long ulParam2, unsigned long ulParam3)
{
    switch (ulIoctl)
    {
/* put these to ID3?
        case SUBFN_CODEC_GETNAME:
            {
                return(1);
            }
        case SUBFN_CODEC_GETARTIST:
            {
                return(1);
            }

        case SUBFN_CODEC_GETTITLE:
            {
                return(1);
            }
*/
        case SUBFN_CODEC_OPEN_DEC:
            {
                unsigned char flag[20];

                FILE *rawfile=(FILE*)pRawFileCache;
                RKFIO_FSeek(0,0 ,rawfile);
                RKFIO_FRead(flag,20,rawfile);
                ID3_len = CheckID3V2Tag(flag);
                if (ID3_len == 0)
                {
                    RKFIO_FSeek(0,0 ,rawfile);

                }
                else
                {
                    ID3_len += 10;
                    RKFIO_FSeek(ID3_len,0 ,rawfile);

                }
                return flac_open_dec();
            }

        case SUBFN_CODEC_GETBUFFER:
            {
                flac_get_buffer(ulParam1,ulParam2);
                return 1;
            }

        case SUBFN_CODEC_DECODE:
            {
                return flac_decode();
            }

        case SUBFN_CODEC_GETSAMPLERATE:
            {
                *(int *)ulParam1 = flac_get_samplerate();
                return(1);
            }

        case SUBFN_CODEC_GETCHANNELS:
            {
                *(int *)ulParam1 = flac_get_channels();
                return(1);
            }

        case SUBFN_CODEC_GETBITRATE:
            {
                *(int *)ulParam1 = flac_get_bitrate();
                return(1);
            }
        case SUBFN_CODEC_GETBPS:
            {
                *(int *)ulParam1 = flac_get_bps();
                return(1);
            }

        case SUBFN_CODEC_GETLENGTH:
            {
                *(int *)ulParam1 = flac_get_length();
                return 1;
            }

        case SUBFN_CODEC_GETTIME:
            {
               *(int *)ulParam1 = flac_get_timepos();
                return 1;
            }

        case SUBFN_CODEC_SEEK:
            {
                flac_seek( ulParam1); /* seconds */
                return 1;
            }

        case SUBFN_CODEC_CLOSE:
            {
                flac_close_dec();
                return 1;
            }

        default:
            {
                return 0;
            }
    }
    return -1;
}

#endif
