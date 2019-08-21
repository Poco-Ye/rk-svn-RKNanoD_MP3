//*@@@+++@@@@******************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// Contains functions needed for WMA Std, WMA Pro, and WMA Lossless.
//*@@@---@@@@******************************************************************

//#include <math.h>
//#include <limits.h>
//#include <stdio.h>
#include "../include/audio_main.h"
#include "..\wmaInclude\msaudio.h"

#ifdef A_CORE_DECODE
#ifdef WMA_DEC_INCLUDE
//*****************************************************************************************
//
// msaudioGetSamplePerFrame
//
//*****************************************************************************************
#ifdef WMAINITIALIZE
#pragma arm section code = "WmaOpenCodecCode"
I32 msaudioGetSamplePerFrame(Int   cSamplePerSec,
                             U32   dwBitPerSec,
                             Int   cNumChannels,
                             Int   iVersion,
                             U16   wEncOpt)
{
    //return NEW samples coming into a frame; actual samples in a frame
    //should be * 2 due to 50% overlapping window
    I32 cSamplePerFrame;
//    U32 dwBytesPerFrame;

    //don't know what to do
    if ((dwBitPerSec == 0 && iVersion < 3) || iVersion > 3)
        return 0;

    //if (cSamplePerSec <= 8000)
    //    cSamplePerFrame = 512;
    //else if (cSamplePerSec <= 11025)
    //    cSamplePerFrame = 512;
    /*else*/
    if (cSamplePerSec <= 16000)
    {
        cSamplePerFrame = 512;
    }
    else if (cSamplePerSec <= 22050)
    {
        cSamplePerFrame = 1024;
    }
    else if (cSamplePerSec <= 32000)
    {
        if (iVersion == 1)
            cSamplePerFrame = 1024;
        else
            cSamplePerFrame = 2048;
    }
    else if (cSamplePerSec <= 44100)
        cSamplePerFrame = 2048;
    else if (cSamplePerSec <= 48000)
        cSamplePerFrame = 2048;
    //else if (cSamplePerSec <= 96000)
    //    cSamplePerFrame = 4096;
    //else if (cSamplePerSec <= 192000)
    //    cSamplePerFrame = 8192;
    // else
    //    cSamplePerFrame = 8192; // cap to 8192


    // Since V3 permits one frame spans over multiple packets, obsolete comments are removed.

    /*if (iVersion == 3)
    {
        U16 iFrmSizeModifier = wEncOpt & ENCOPT3_FRM_SIZE_MOD;
        if (iFrmSizeModifier == 2)
        {
            cSamplePerFrame *= 2;
        }
        else if (iFrmSizeModifier == 4)
        {
            cSamplePerFrame /= 2;
        }
        else if (iFrmSizeModifier == 6)
        {
            cSamplePerFrame /= 4;
        }
    }*/

    //if (iVersion < 3)
    //    {   /* compute bytesperframe corresponding to csampleperframe*/
    //        dwBytesPerFrame = (U32) (((cSamplePerFrame*dwBitPerSec + cSamplePerSec/2)/cSamplePerSec + 7)/8);
    //        if ( dwBytesPerFrame==0 && (cSamplePerFrame*dwBitPerSec) == 0 )
    //        {
    //            // this can happen when garbage data sets dwBitsPerSec to a very large number
    //            // avoid an infinite loop below
    //            dwBitPerSec = cSamplePerSec;
    //            dwBytesPerFrame = (U32) (((cSamplePerFrame*dwBitPerSec + cSamplePerSec/2)/cSamplePerSec + 7)/8);
    //        }
    //        if (dwBytesPerFrame <= 1)
    //        {   //silence mode
    //            while (dwBytesPerFrame == 0)
    //            {
    //                cSamplePerFrame *= 2;           //save more bits; quartz can't take too big a value
    //                dwBytesPerFrame = (U32) (((cSamplePerFrame*dwBitPerSec + cSamplePerSec/2)/cSamplePerSec + 7)/8);
    //            }
    //        }
    //    }


    return cSamplePerFrame;
} // msaudioGetSamplePerFrame
#pragma arm section code
#endif
#endif
#endif
