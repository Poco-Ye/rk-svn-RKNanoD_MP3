/* Copyright (C) 2009 ROCK-CHIPS FUZHOU . All Rights Reserved. */
/*
File    : \Audio\Common\pCODECS.c
Desc    :

Author  : Vincent Hsiung
Date    : 2009-01-08
Notes   :

$Log    :
*
*
*/
/****************************************************************/

#include "../AudioConfig.h"
#include "../include/audio_globals.h"

//global variables
_ATTR_AUDIO_BSS_
int CurrentCodec;

//be sure to keep consistent with array order of IOCTL enum that is defined in audio_globals.h.
_ATTR_AUDIO_DATA_
static unsigned long (*CodecPFn[NUMCODECS])(unsigned long ulSubFn,
                                                   unsigned long ulParam1,
                                                   unsigned long ulParam2,
                                                   unsigned long ulParam3) =
{
    #ifdef MP3_DEC_INCLUDE
    MP3Function,
    #else
    0,
    #endif

    #ifdef WMA_DEC_INCLUDE
    WMAFunction,
    #else
    0,
    #endif

    #ifdef AAC_DEC_INCLUDE
    AACDecFunction,
    #else
    0,
    #endif

    #ifdef WAV_DEC_INCLUDE
    PCMFunction,
    #endif

    #ifdef REV4_DEC_INCLUDE
    REV4DecFunction,
    #else
    0,
    #endif

    #ifdef FLAC_DEC_INCLUDE
    FLACDecFunction,
    #else
    0,
    #endif

    #ifdef REV6_DEC_INCLUDE
    REV6DecFunction,
    #else
    0,
    #endif

    #ifdef REV73_DEC_INCLUDE
    REV7DecFunction,
    #else
    0,
    #endif
};

_ATTR_AUDIO_TEXT_
unsigned long CodecOpen(unsigned long ulCodec, unsigned long ulFlags)
{
    unsigned long ulRet;

    if(CurrentCodec == 0xff)
        return -1;

    // Pass the open request to the entry point for the codec.
    ulRet = (CodecPFn[CurrentCodec])(SUBFN_CODEC_OPEN_DEC, 0, 0, 0);

    // Return the result to the caller.
    return(ulRet);
}

_ATTR_AUDIO_TEXT_
unsigned long CodecDecode(void)
{
    if(CurrentCodec == 0xff)
        return -1;

    return((CodecPFn[CurrentCodec])(SUBFN_CODEC_DECODE, 0, 0, 0));
}

_ATTR_AUDIO_TEXT_
unsigned long CodecSeek(unsigned long ulTime, unsigned long ulSeekType)
{
    // Pass the seek request to the entry point for the specified codec.
    unsigned long ret;

    if(CurrentCodec == 0xff)
        return -1;
    ret=((CodecPFn[CurrentCodec])(SUBFN_CODEC_SEEK, ulTime, ulSeekType,0));
    return ret;
}

_ATTR_AUDIO_TEXT_
unsigned long CodecGetTime(unsigned long *pulTime)
{
    // Pass the time request to the entry point for the specified codec.
    unsigned long ret;
    if(CurrentCodec == 0xff)
        return -1;

    ret=((CodecPFn[CurrentCodec])(SUBFN_CODEC_GETTIME, (unsigned long)pulTime, 0, 0));

    return ret;
}

_ATTR_AUDIO_TEXT_
unsigned long CodecGetBitrate(unsigned long *pulBitrate)
{
   // Pass the bitrate request to the entry point for the specified codec.
   unsigned long ret;
   if(CurrentCodec == 0xff)
        return -1;


   ret=((CodecPFn[CurrentCodec])(SUBFN_CODEC_GETBITRATE, (unsigned long)pulBitrate, 0, 0));

   return ret;
}

_ATTR_AUDIO_TEXT_
unsigned long CodecGetSampleRate(unsigned long *pulSampleRate)
{
    // Pass the sample rate request to the entry point for the specified codec.
    unsigned long ret;
    if(CurrentCodec == 0xff)
        return -1;

    ret=((CodecPFn[CurrentCodec])(SUBFN_CODEC_GETSAMPLERATE, (unsigned long)pulSampleRate, 0, 0));

    return ret;
}

_ATTR_AUDIO_TEXT_
unsigned long CodecGetChannels(unsigned long *pulChannels)
{
    // Pass the channels request to the entry point for the specified codec.
    unsigned long ret;
    if(CurrentCodec == 0xff)
        return -1;

    ret=((CodecPFn[CurrentCodec])(SUBFN_CODEC_GETCHANNELS, (unsigned long)pulChannels, 0, 0));

    return ret;
}

_ATTR_AUDIO_TEXT_
unsigned long CodecGetLength(unsigned long *pulLength)
{
    // Pass the length request to the entry point for the specified codec.
    unsigned long ret;
    if(CurrentCodec == 0xff)
        return -1;

    ret=((CodecPFn[CurrentCodec])(SUBFN_CODEC_GETLENGTH, (unsigned long)pulLength, 0, 0));


    return ret;
}

_ATTR_AUDIO_TEXT_
unsigned long CodecSetBuffer(short* psBuffer)
{
    // Pass the set buffer request to the entry point for the specified codec.
    unsigned long ret;
    if(CurrentCodec == 0xff)
        return -1;

    ret=((CodecPFn[CurrentCodec])(SUBFN_CODEC_SETBUFFER, (unsigned long)psBuffer, 0, 0));
    return ret;
}

_ATTR_AUDIO_TEXT_
unsigned long CodecClose(void)
{
    unsigned long ulRet;

    if(CurrentCodec == 0xff)
        return -1;

    // Pass the close request to the entry point for the specified codec.
    ulRet = (CodecPFn[CurrentCodec])(SUBFN_CODEC_CLOSE, CODEC_OPEN_ENCODE, 0, 0);

    return(ulRet);
}

_ATTR_AUDIO_TEXT_
unsigned long CodecGetCaptureBuffer(short *ppsBuffer, long *plLength)
{
    // Pass the get capture buffer request to the entry point for the specified
    // codec.
    if(CurrentCodec == 0xff)
        return -1;

    return((CodecPFn[CurrentCodec])(SUBFN_CODEC_GETBUFFER,(unsigned long)ppsBuffer,
            (unsigned long)plLength, 0));
}

_ATTR_AUDIO_TEXT_
unsigned long CodecEncode(void)
{
    // Pass the encode request to the entry point for the specified codec.
    if(CurrentCodec == 0xff)
        return -1;

    return((CodecPFn[CurrentCodec])(SUBFN_CODEC_ENCODE, 0, 0, 0));
}

_ATTR_AUDIO_TEXT_
unsigned long CodecGetBps(long *audioBps)
{
    unsigned long ulRet;

    if(CurrentCodec == 0xff)
        return -1;
    // Pass the encode request to the entry point for the specified codec.

    ulRet = ((CodecPFn[CurrentCodec])(SUBFN_CODEC_GETBPS, (unsigned long)audioBps, 0, 0));

    return ulRet;
}
