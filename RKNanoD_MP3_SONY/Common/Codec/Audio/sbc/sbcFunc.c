/* Copyright (C) 2007 ROCK-CHIPS FUZHOU . All Rights Reserved. */
/*
File	: \Audio\APEDec
Desc	: APE decode

Author	: huangxd , Vincent Hisung
Date	: 2007-08-xx
Notes	:

$Log	: 
* huangxd . create the file at 08.xx.2007
*	 
* vincent .	amendment at 08.xx.2007.
* 
*/
/****************************************************************/

#include "audio_main.h"


#ifdef SBC_INCLUDE

#include "audio_globals.h"
#include "audio_file_access.h"
#include "sbc_interface.h"

_ATTR_SBCDEC_TEXT_
unsigned long SbcDecFunction(unsigned long ulIoctl, unsigned long ulParam1,
        unsigned long ulParam2, unsigned long ulParam3)
{

switch (ulIoctl)
{

        case SUBFN_CODEC_OPEN_DEC:
            {
                return sbc_open();
            }

        case SUBFN_CODEC_GETBUFFER:
            {
                sbc_get_buffer(ulParam1,ulParam2);
                return 1;
            }

        case SUBFN_CODEC_DECODE:
            {
                return sbc_dec();
            }

        case SUBFN_CODEC_GETSAMPLERATE:
            {
                *(int *)ulParam1 = sbc_get_samplerate();
                return(1);
            }

        case SUBFN_CODEC_GETCHANNELS:
            {
                *(int *)ulParam1 = sbc_get_channels();
                return(1);
            }

        case SUBFN_CODEC_GETBITRATE:
            {
                *(int *)ulParam1 = sbc_get_bitrate();
                return(1);
            }

        case SUBFN_CODEC_GETLENGTH:
            {
                *(int *)ulParam1 = sbc_get_length();
                return 1;
            }

        case SUBFN_CODEC_GETTIME:
            {
               *(int *)ulParam1 = (long long) sbc_get_timepos() * 1000 / sbc_get_samplerate();
                return 1;
            }

        case SUBFN_CODEC_SEEK:
            {
                sbc_seek( ulParam1 / 1000 ); /* seconds */
                return 1;
            }

        case SUBFN_CODEC_CLOSE:
            {
                sbc_close();
                return 1;
            }
        case SUBFN_CODEC_SETBUFFER:

            return 1;

        default:
            {
                return 0;
            }
    }
    return -1;
}

#endif

