//*@@@+++@@@@******************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// Contains functions needed for WMA Std. These are not needed by WMA Pro or Lossless.
//*@@@---@@@@******************************************************************
/*************************************************************************

Copyright (C) Microsoft Corporation, 1996 - 1999

Module Name:

MsAudio.cpp

Abstract:

Implementation of public member functions for CAudioObject.

Author:

Wei-ge Chen (wchen) 11-March-1998

Revision History:


*************************************************************************/
#include "../include/audio_main.h"
// Print out the target we're building for
#define COMMONMACROS_OUTPUT_TARGET

#ifdef A_CORE_DECODE
#ifdef WMA_DEC_INCLUDE
#if !defined (_WIN32_WCE) && !defined (HITACHI)
//#include <time.h>
#endif  // _WIN32_WCE

//#include <math.h>
//#include <limits.h>
//#include <stdio.h>
#include "..\wmaInclude\msaudio.h"
#include "..\wmaInclude\AutoProfile.h"
#include "..\wmaInclude\macros.h"
//#include "float.h"
//#include "..\wmaInclude\wavfileexio.h"
//#include "..\wmaInclude\drccommon.h"


//#if defined (BUILD_WMASTD)
//*****************************************************************************************
//
// prvWeightedQuantization
// calculate 10^( (MaskQ-MaxMaskQ)*2.5*0.5/20 ) * 10^( QuantStepSize/20 )
//         = (10^(1/16))^( MaskQ-MaxMaskQ )     * (10^(1/20)^QuantStepSize
//*****************************************************************************************
#if defined(WMAHIGHRATE) || defined(WMAMIDRATELOWRATE)
#pragma arm section code = "WmaHighLowCommonCode"
void Norm4FastFloat(FastFloat* pfflt)
{   // use the faster Norm4FastFloatU when you know the value is positive
    register UInt uiF = abs(pfflt->iFraction);
    register Int iFB = 0;
    if (uiF == 0)
    {
        pfflt->iFracBits = 0;
        return;
    }
    while (uiF < 0x1FFFFFFF)
    {
        uiF <<= 2;
        iFB +=  2;
    }
    if (uiF < 0x3FFFFFFF)
    {
        iFB +=  1;
    }
    pfflt->iFraction <<= iFB;
    pfflt->iFracBits += iFB;
}

QuantFloat prvWeightedQuantization(CAudioObject *pau, PerChannelInfo *ppcinfo, Int iBark)
{
    Int iIndex = ppcinfo->m_iMaxMaskQ - ppcinfo->m_rgiMaskQ[iBark];
    QuantFloat qfltRMS;
    assert(0 <= iIndex);

    if (iIndex >= MASK_MINUS_POWER_TABLE_SIZE)
        iIndex = MASK_MINUS_POWER_TABLE_SIZE - 1;
    {
        Int uiFraction, iFracBits;
        //uiFraction = rgiMaskMinusPower10[ iIndex ];     // with MASK_POWER_FRAC_BITS==28 fractional bits
#ifdef WMA_TABLE_ROOM_VERIFY
        uiFraction = ((const MaskPowerType *)p_rgiMaskMinusPower10)[ iIndex ];
#else
        uiFraction = rgiMaskMinusPower10[ iIndex ];  // with MASK_POWER_FRAC_BITS==28 fractional bits
#endif
        iFracBits = MASK_POWER_FRAC_BITS + (iIndex >> 2);
//        MONITOR_RANGE(gMR_WeightRatio,(float)uiFraction/pow(2,iFracBits));

        qfltRMS.iFraction = MULT_HI(pau->m_qstQuantStep.iFraction, uiFraction);
        qfltRMS.iFracBits = pau->m_qstQuantStep.iFracBits + iFracBits - 31;
        Norm4FastFloat(&qfltRMS);
    }



    return qfltRMS;
}
#pragma arm section code

#endif // defined(WMAHIGHRATE) || defined(WMAMIDRATELOWRATE)
//#endif
#endif
#endif
