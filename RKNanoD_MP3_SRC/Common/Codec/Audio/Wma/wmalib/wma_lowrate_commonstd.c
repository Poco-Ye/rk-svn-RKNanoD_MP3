//*@@@+++@@@@******************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// Contains implementations needed for WMA Standard. Not needed for WMA Pro & WMA Lossless
//*@@@---@@@@******************************************************************
/*************************************************************************

Copyright (C) Microsoft Corporation, 1996 - 1999

Module Name:

    LowRate.c

Abstract:

    Implementation of functions only used by low bitrate or mid bitrates
 e.g. MidRate and LowRate specific code

Author:

    Wei-ge Chen (wchen) 14-July-1998

Revision History:
 Sil Sanders (sils) 17-Dec-1999 - Added MidRate specific functions


*************************************************************************/

//#ifdef ENABLE_ALL_ENCOPT

//#include <math.h>
//#include <limits.h>
//#include "stdio.h"
#include "../include/audio_main.h"
#include "..\wmaInclude\AutoProfile.h"
#include "..\wmaInclude\msaudio.h"

#include "..\wmaInclude\lowrate_common.h"



// DEBUG_BREAK at a particular Frame in prvInverseQuantizeHighRate or prvInverseQuantizeLowRate
//#define INVERSE_QUANTIZE_AT_FRAME 178
// PRINT CoefRecon for all Frames in range (define or undefine both at once)
//#define PRINT_INVERSE_QUANTIZE_AT_FRAME_FIRST 177
//#define PRINT_INVERSE_QUANTIZE_AT_FRAME_LAST  177


//#ifdef BUILD_INTEGER
//Integer Version

#ifdef A_CORE_DECODE
#ifdef WMA_DEC_INCLUDE
#ifdef WMAMIDRATELOWRATE
#pragma arm section code = "WmaLowRateCode"

#define INV_MAX_WEIGHT_FRAC_BITS 30
#define MORE_WF_FRAC_BITS 0
I32 quickRand(tRandState* ptRandState)
{
    const U32 a = 1664525;
    const U32 c = 1013904223;
    I32 iTemp, iTemp1;
    //a*x + c has to be done with unsigned 32 bit
    ptRandState->uiRand =  a * ptRandState->uiRand + c;

    // uiRand values starting from a 0 seed are: 0x3c6ef35f, 0x47502932, 0xd1ccf6e9, 0xaaf95334, 0x6252e503, 0x9f2ec686, 0x57fe6c2d, ...

    // do not change the above - this reference generator has been extensively tested and has excellent randomness properties
    // a truism in the world of random number generator theory and practice is:
    // "any change, no matter how small, can and will change the properties of the generator and must be fully tested"
    // In case you don't know, it can easily take a person-month to fully test a generator.

    // Notwithstanding the above, it is fair to take a function of a random number to shape its range or distribution.
    // This we do below to give it a triangular distrbution between -2.5 and 2.5 to roughly approximate a Guassian distribution.

    // cast and shift to make the range (-1, 1) with Binary Point 3.
    iTemp = ((I32)(ptRandState->uiRand)) >> 2;

    // *1.25 to make the range (-1.25, 1.25)
    iTemp += (iTemp >> 2);

    // Difference of two random numbers gives a triangle distribution and a range of (-2.5, 2.5)
    // it also gives a serial correlation of -0.5 at lag 1.  But all the other lags have normally small correlations.
    iTemp1 = iTemp - ptRandState->iPrior;

    // Save first term of this difference for next time.
    ptRandState->iPrior = iTemp;

    //return -2.5 to 2.5 with Binary Point = 3 with a triangle distribution
    return iTemp1;
}

static  I32 DIVI64BYU32(I64 a, U32 b)
{
    return (I32)((a) / (b));
}

static FastFloat ffltAdd(FastFloat ffltA, FastFloat ffltB)
{
    FastFloat ffltP;

    if (ffltA.iFraction > 0x3FFFFFFF)
    {
        ffltA.iFraction >>= 1;
        ffltA.iFracBits--;
    }
    if (ffltB.iFraction > 0x3FFFFFFF)
    {
        ffltB.iFraction >>= 1;
        ffltB.iFracBits--;
    }

    if (ffltA.iFracBits >= ffltB.iFracBits)
    {
        ffltP.iFracBits = ffltB.iFracBits;
        ffltP.iFraction = ffltB.iFraction + (ffltA.iFraction >> (ffltA.iFracBits - ffltB.iFracBits));
    }
    else
    {
        ffltP.iFracBits = ffltA.iFracBits;
        ffltP.iFraction = ffltA.iFraction + (ffltB.iFraction >> (ffltB.iFracBits - ffltA.iFracBits));
    }
    Norm4FastFloat(&ffltP);
    return ffltP;
}

static  FastFloat ffltMaskPower10(Int iMaskQ)
{ // (10^(1/16)) ^ iMaskQ
    FastFloat fflt;

    // There is, unfortunately, no easy way to keep this assert with our new
    // combined decoder/encoder build.
//#if !defined (ENCODER)
//  assert( -MASK_MINUS_POWER_TABLE_SIZE < iMaskQ && iMaskQ <= MASK_PLUS_POWER_TABLE_SIZE );
//#endif

    if (iMaskQ <= 0)
    {
        if (iMaskQ < -MASK_MINUS_POWER_TABLE_SIZE)
            iMaskQ = -(MASK_MINUS_POWER_TABLE_SIZE - 1);
#ifdef WMA_TABLE_ROOM_VERIFY
        fflt.iFraction = ((const MaskPowerType *)p_rgiMaskMinusPower10)[ -iMaskQ ];
#else
        fflt.iFraction = rgiMaskMinusPower10[ -iMaskQ ]; // with MASK_POWER_FRAC_BITS==28 fractional bits
#endif
        fflt.iFracBits = MASK_POWER_FRAC_BITS + (-iMaskQ >> 2);

    }
    else
    {
        if (iMaskQ >= MASK_PLUS_POWER_TABLE_SIZE)
            iMaskQ = MASK_PLUS_POWER_TABLE_SIZE;
#ifdef WMA_TABLE_ROOM_VERIFY
        fflt.iFraction = ((const MaskPowerType *)p_rgiMaskPlusPower10)[ iMaskQ-1 ];
#else
        fflt.iFraction = rgiMaskPlusPower10[ iMaskQ-1 ]; // with MASK_POWER_FRAC_BITS==28 fractional bits
#endif
        fflt.iFracBits = MASK_POWER_FRAC_BITS - (iMaskQ >> 2);

    }
    return fflt;
}

static Int iUnResampleIndex(Int iResampled, const MaskResampleInfo MRI)

{ // return the reverse of iResampleIndex()
    // that is, convert from resampled indexes of MaskQ or WeightFactor to natural index for the current frame
    if (MRI.iMaskResampleRatioPow > MASKQ_RESAMPLE_OFFSET)
    {
        // Rounding for down shift
        // Although V4 did not round when resampling, this inverse funtion needs to round ???
        return (iResampled + (1 << (MRI.iMaskResampleRatioPow - (MASKQ_RESAMPLE_OFFSET + 1)))) >> (MRI.iMaskResampleRatioPow - MASKQ_RESAMPLE_OFFSET);
        // return iResampled >> (MRI.iMaskResampleRatioPow-MASKQ_RESAMPLE_OFFSET);
    }
    else
    {
        return iResampled << (MASKQ_RESAMPLE_OFFSET - MRI.iMaskResampleRatioPow);
    }
}

static FastFloat ffltMultiply(FastFloat ffltA, FastFloat ffltB)
{
    FastFloat ffltP;
    ffltP.iFraction = MULT_HI(ffltA.iFraction, ffltB.iFraction);
    ffltP.iFracBits = (ffltA.iFracBits + ffltB.iFracBits - 31);
    Norm4FastFloat(&ffltP);
    return ffltP;
}

static FastFloat ffltfltMultiply(FastFloat ffltA, Int B , Int bits)
{
    FastFloat ffltP;
    ffltP.iFracBits = ffltA.iFracBits;
    ffltP.iFraction = MULT_HI32_SHIFT(ffltA.iFraction, B, bits);
    Norm4FastFloat(&ffltP);
    return ffltP;
}

static void NormUInt(UInt* puiValue, Int* pcFracBits, const UInt uiTarget)
{
    const UInt uiTarget2 = uiTarget >> 1;
    register UInt uiV = *puiValue;
    register Int cFB = *pcFracBits;
    assert(uiV > 0);
    if (uiV == 0) return; // useful if asserts are disabled

    while (uiV < uiTarget2)
    {
        uiV <<= 2;
        cFB += 2;
    }
    if (uiV < uiTarget)
    {
        uiV <<= 1;
        cFB += 1;
    }
    *puiValue = uiV;
    *pcFracBits = cFB;
}

static Int Align2FracBits(const Int iValue, const Int cFracBits, const Int cAlignFracBits)
{
    const Int iShift = cFracBits - cAlignFracBits;

    if (iShift < 0)
    {
        return (iValue << -iShift);
    }
    else if (iShift < 32)
    {
        return (iValue >> iShift);
    }
    else
    {
        return 0;
    }
}

// WinCE and embedded compilers have lousy performance for I64 multiplication and division.
// On the SH3, this saves .35% of playtime decoding 16m_16 by speeding up InverseQuadRoot 14%.

UInt uiInverseMaxWeight(WeightType wtMaxWeight)
{
    U32 uiMSF = wtMaxWeight;
    Int iExp = (32 - INV_MAX_WEIGHT_FRAC_BITS) + (32 - WEIGHTFACTOR_FRACT_BITS);  // (32-30)+(32-21)=13
    int iMSF8;
    U32 uiFrac1;

    // Check if inv max weight too large to fit in a U32 @ INV_MAX_WEIGHT_FRAC_BITS
    // We note mwMAX = 2^(21+30)/(2^32 - 1) ~= 2^(21+30)/2^32 = 2^(21+30-32)
    if (uiMSF < ((U32)(1 << (INV_MAX_WEIGHT_FRAC_BITS + WEIGHTFACTOR_FRACT_BITS - 32))))
        return 0xFFFFFFFF; // Return the largest number we've got

    // normalize the fractional part
    while ((uiMSF & 0xF0000000) == 0)
    {
        iExp -= 4;
        uiMSF <<= 4;
    }
    while ((uiMSF & 0x80000000) == 0)
    {
        iExp--;
        uiMSF <<= 1;
    }
    // discard the most significant one bit (it's presence is built into g_InverseFraction)
    iExp--;
    uiMSF <<= 1;
    assert(iExp >= 0);
    if (iExp < 0)
        return 0xFFFFFFFF; // Return the largest number we've got

    // split into top INVERSE_FRACTION_TABLE_LOG2_SIZE==8 bits for fractional lookup and bottom bits for interpolation
    iMSF8 = uiMSF >> (32 - INVERSE_FRACTION_TABLE_LOG2_SIZE);
    uiMSF <<= INVERSE_FRACTION_TABLE_LOG2_SIZE;
    // lookup and interpolate - tables are set up to return correct binary point for WeightType (we hope)
#ifdef WMA_TABLE_ROOM_VERIFY
    uiFrac1  = ((const UInt*)p_g_InverseFraction)[iMSF8++];
    uiFrac1 -= MULT_HI_UDWORD(uiMSF, uiFrac1 - ((const UInt*)p_g_InverseFraction)[iMSF8]);
#else
    uiFrac1  = g_InverseFraction[iMSF8++];
    uiFrac1 -= MULT_HI_UDWORD(uiMSF, uiFrac1 - g_InverseFraction[iMSF8]);
#endif
    return uiFrac1 >> iExp;
} // uiInverseMaxWeight

#   define INVERSE_MAX_WEIGHT(mw) uiInverseMaxWeight(mw)
// For BUILD_INT_FLOAT, use the following
// #   define INVERSE_MAX_WEIGHT(mw) ((UInt)((((I64)1)<<(INV_MAX_WEIGHT_FRAC_BITS+WEIGHTFACTOR_FRACT_BITS))/(mw)))


//**************************************************************************************************
//
// prvInverseQuantizeLowRate handles LowRate inverse quantization
//
//**************************************************************************************************
//#if !WMA_OPT_INVERSQUAN_LOWRATE_ARM

WMARESULT prvInverseQuantizeLowRate(CAudioObject* pau, PerChannelInfo* ppcinfo, Int* rgiWeightFactor)
{
    Int iBark = 0;
    Int iCoefQ = 0;
    Int iRecon = 0;
    Int iShiftCoefQ = 20 + 11 - pau->m_iMaxEscSize;  // avoid overflow when |coefQ| can be >= 2048.
    int isValid;
    UInt uiWeightFactor, uiMaxWeight, uiQuantStepXInvMaxWeight;
    Int QuantStepXMaxWeightXWeightFactor, qrand, iNoise;
    Int cQSIMWFracBits, cFracBits, cFracBits2, cWFFracBits, cMaxWeightFracBits;
    Float fltCoefRecon;
    CBT *rgiCoefRecon = ppcinfo->m_rgiCoefRecon;
    const I32* const rgiCoefQ                = ppcinfo->m_rgiCoefQ;
    const U8* const rgbBandNotCoded          = ppcinfo->m_rgbBandNotCoded;
    const Int* const rgiNoisePower           = ppcinfo->m_rgiNoisePower;
    const FastFloat* const rgffltSqrtBWRatio = ppcinfo->m_rgffltSqrtBWRatio;
    U8  cNoiseBand;
    Int iNoiseBand = 0;
    const Int iDitherFactor = 0x51EB851F;  // LPC  0.04 * 2^35
    UInt uiInvMaxWeight;
    FUNCTION_PROFILE(fp);
    FUNCTION_PROFILE_START(&fp, INVERSE_QUAN_LOW_RATE_PROFILE);
    //assert( uiInvMaxWeight == (UInt)((float)(1<<INV_MAX_WEIGHT_FRAC_BITS)*(float)(1<<WEIGHTFACTOR_FRACT_BITS)/((float)ppcinfo->m_wtMaxWeight) ) );
    //// Float 1/MaxWeight = uiInvMaxWeight/(1.0F*(1<<30))

    if (ppcinfo->m_wtMaxWeight <= 0)
        return TraceResult(WMA_E_BROKEN_FRAME);

    uiInvMaxWeight = INVERSE_MAX_WEIGHT(ppcinfo->m_wtMaxWeight);

// DEBUG_BREAK_AT_FRAME_INV_QUANT;
// MONITOR_COUNT(gMC_IQ,1);

    //Calculate QuantStep X invMaxWeight
    cQSIMWFracBits  = pau->m_qstQuantStep.iFracBits;
    uiQuantStepXInvMaxWeight = MULT_HI_DWORD(pau->m_qstQuantStep.iFraction, uiInvMaxWeight);
    //uiQuantStepXInvMaxWeight = (MULT_HI_DWORD(pau->m_qstQuantStep.iFraction,uiInvMaxWeight>>1)<<1);
    cQSIMWFracBits += (INV_MAX_WEIGHT_FRAC_BITS - 32);
    // Float QSIMWF = uiQuantStepXInvMaxWeight*1.0F/(1<<cQSIMWFracBits)
    NormUInt(&uiQuantStepXInvMaxWeight, &cQSIMWFracBits, 0x3FFFFFFF);
// MONITOR_RANGE(gMR_QuantStepXInvMaxWeight,uiQuantStepXInvMaxWeight/pow(2,cQSIMWFracBits));

    // since all weights are less than MaxWeight, fast scale below by MaxWeight's FracBits
    uiMaxWeight = ppcinfo->m_wtMaxWeight << MORE_WF_FRAC_BITS;;
    cMaxWeightFracBits = MORE_WF_FRAC_BITS;  // really should be WEIGHTFACTOR_FRACT_BITS+MORE_WF_FRAC_BITS but this way is for shift delta

    NormUInt(&uiMaxWeight, &cMaxWeightFracBits, 0x3FFFFFFF);

    if (!pau->m_fNoiseSub)
    {
        // Unusual case, but 8kHz Mono 8kpbs gets here
        memset(rgiCoefRecon, 0, pau->m_cLowCutOff * sizeof(CoefType));
        for (iRecon = pau->m_cLowCutOff; iRecon < pau->m_cHighCutOff; iRecon++, iCoefQ++)
        { // rgfltCoefRecon [iRecon] = Float ((Float) (rgiCoefQ [iCoefQ]) * rgfltWeightFactor [iRecon] * dblQuantStep * fltInvMaxWeight);
            int iCoef;
            assert(rgiWeightFactor[iRecon] <= (0x7FFFFFFF >> cMaxWeightFracBits));
            uiWeightFactor = rgiWeightFactor [iRecon] << cMaxWeightFracBits;
            cWFFracBits = WEIGHTFACTOR_FRACT_BITS + cMaxWeightFracBits;
            //// Float WeightFactor = uiWeightFactor*1.0F/(1<<cWFFracBits)
            assert(uiWeightFactor <= 0x7FFFFFFF);
            NormUInt(&uiWeightFactor, &cWFFracBits, 0x3FFFFFFF);
//   MONITOR_RANGE(gMR_weightFactor,uiWeightFactor/pow(2,cWFFracBits));

            QuantStepXMaxWeightXWeightFactor = MULT_HI(uiQuantStepXInvMaxWeight, uiWeightFactor);
            cFracBits = cQSIMWFracBits + cWFFracBits - 31;
            //// Float QuantStep*WeightFactor/InvMaxWeight = QuantStepXMaxWeightXWeightFactor/(1.0F*(1<<cFracBits))
//   MONITOR_RANGE(gMR_QuantStepXMaxWeightXWeightFactor,QuantStepXMaxWeightXWeightFactor/pow(2,cFracBits));

            assert((rgiCoefQ[iCoefQ] << iShiftCoefQ) == (((I64)rgiCoefQ[iCoefQ]) << iShiftCoefQ));
            iCoef = MULT_HI((Int)rgiCoefQ[iCoefQ] << iShiftCoefQ, QuantStepXMaxWeightXWeightFactor);
            cFracBits += (iShiftCoefQ - 31);
            //// Float Coef = iCoef/(1.0F*(1<<cFracBits))

            rgiCoefRecon[iRecon] = Align2FracBits(iCoef, cFracBits, TRANSFORM_FRACT_BITS);
            //// Float CoefRecon = rgiCoefRecon[iRecon]/32.0F

            //VERIFY_COEF_RECON_LR(iRecon,0,0);
        }
        memset(rgiCoefRecon + pau->m_cHighCutOff, 0, (ppcinfo->m_cSubband - pau->m_cHighCutOff) * sizeof(CoefType));
        FUNCTION_PROFILE_STOP(&fp);
        return WMA_OK;
    }

    cNoiseBand = rgbBandNotCoded [0];

    if (iRecon < pau->m_cLowCutOff)
    {
        // not integerized since cLowCutOff is typically 0, so this is here for compatability with V1
        Double dblQuantStep = DOUBLE_FROM_QUANTSTEPTYPE(pau->m_qstQuantStep);
        while (iRecon < pau->m_cLowCutOff)
        {
            Float fltNoise = pau->m_fltDitherLevel * ((Float) quickRand(&(pau->m_tRandState)) / (Float) 0x20000000); //rgfltNoise [iRecon];
            Float fltWeightFactor = ((float)rgiWeightFactor [pau->m_cLowCutOff]) / (1 << WEIGHTFACTOR_FRACT_BITS);
            // SH4 warning CBE4717 on the next line is ignorable - appraently a compiler mistake
            fltCoefRecon  = (Float)(fltNoise * fltWeightFactor * dblQuantStep * (float)(1 << WEIGHTFACTOR_FRACT_BITS) / ((float)ppcinfo->m_wtMaxWeight));
            rgiCoefRecon [iRecon] = (Int)(fltCoefRecon * (1 << TRANSFORM_FRACT_BITS));
//   MONITOR_RANGE(gMR_CoefRecon,rgiCoefRecon[iRecon]/32.0f);
//   MONITOR_RANGE(gMR_WeightRatio,(fltWeightFactor*(float)(1<<WEIGHTFACTOR_FRACT_BITS)/((float)ppcinfo->m_wtMaxWeight)));
//   MONITOR_COUNT(gMC_IQ_Float,9);
            iRecon++;
        }
    }

    while (iRecon < pau->m_iFirstNoiseIndex)
    {
        int iCoef, iCoefScaled, iCoefRecon, iNoiseScaled, iNoiseQuant;
        assert(TRANSFORM_FRACT_BITS == 5);
        qrand = quickRand(&(pau->m_tRandState));

        if (iRecon >= pau->m_rgiBarkIndex [iBark + 1])
            iBark++;
        assert(iBark < NUM_BARK_BAND);

        // Since weight factors became unsigned, the following assert is not
        // valid. Other wrap-around detection would have to be performed elsewhere.
        assert(rgiWeightFactor[iRecon] <= (0x7FFFFFFF >> cMaxWeightFracBits));
        uiWeightFactor = rgiWeightFactor [iRecon] << cMaxWeightFracBits;
        cWFFracBits = WEIGHTFACTOR_FRACT_BITS + cMaxWeightFracBits;
        //// Float WeightFactor = uiWeightFactor/(1.0F*(1<<cWFFracBits))
        // Since weight factors became unsigned, the following assert is not
        // valid. Other wrap-around detection would have to be performed elsewhere.
        assert(uiWeightFactor <= 0x7FFFFFFF);
        NormUInt(&uiWeightFactor, &cWFFracBits, 0x3FFFFFFF);    // weightFactor with cWFFracBits fractional bits
//  MONITOR_RANGE(gMR_weightFactor,uiWeightFactor/pow(2,cWFFracBits));

        QuantStepXMaxWeightXWeightFactor = MULT_HI(uiQuantStepXInvMaxWeight, uiWeightFactor);
        cFracBits = cQSIMWFracBits + cWFFracBits - 31;
        //// Float QuantStep*WeightFactor/MaxWeight = QuantStepXMaxWeightXWeightFactor/(1.0F*(1<<cFracBits))
//  MONITOR_RANGE(gMR_QuantStepXMaxWeightXWeightFactor,QuantStepXMaxWeightXWeightFactor/pow(2,cFracBits));

        assert((rgiCoefQ[iCoefQ] << iShiftCoefQ) == (((I64)rgiCoefQ[iCoefQ]) << iShiftCoefQ));
        iCoef = MULT_HI((Int)rgiCoefQ[iCoefQ] << iShiftCoefQ, QuantStepXMaxWeightXWeightFactor);
        cFracBits += (iShiftCoefQ - 31);
        //// Float Coef = iCoef/(1.0F*(1<<cFracBits))

        //Rescale to TRANSFORM_FRACT_BITS for outputing to the inverse transform so that (float)iCoefScaled/(1<<TRANSFORM_FRACT_BITS)
        iCoefScaled = Align2FracBits(iCoef, cFracBits, TRANSFORM_FRACT_BITS);
        //// Float CoefScaled = iCoefScaled/32.0F

        iNoise = MULT_HI(iDitherFactor, qrand);
        cFracBits2 = 35 + 29 - 31;       // == 33
        //// Float Noise = iNoise/(1024.0F*(1<<(cFracBits2-10)))
        iNoiseQuant = MULT_HI(iNoise, QuantStepXMaxWeightXWeightFactor);
        cFracBits2 += ((cQSIMWFracBits + cWFFracBits - 31) - 31);
        //// Float NoiseQuant = iNoiseQuant/(1024.0F*(1<<(cFracBits2-10)))
        assert((cFracBits + (33 - iShiftCoefQ)) == cFracBits2);

        //rescale iNoiseQuant so that (float)iNoiseScaled/(1<<TRANSFORM_FRACT_BITS)
        iNoiseScaled = Align2FracBits(iNoiseQuant, cFracBits2, TRANSFORM_FRACT_BITS);
        //// Float NoiseScaled = iNoiseScaled/32.0F

        iCoefRecon = iCoefScaled + iNoiseScaled;
        assert(iCoefRecon == (((I64)iCoefScaled) + ((I64)iNoiseScaled)));
        rgiCoefRecon [iRecon] = iCoefRecon;
        //// Float CoefRecon = rgiCoefRecon [iRecon]/32.0F

//  VERIFY_COEF_RECON_LR(iRecon,qrand,cNoiseBand);

        iRecon++;
        iCoefQ++;
    }

    while (iRecon < pau->m_cHighCutOff)
    {
        if (iRecon >= pau->m_rgiBarkIndex [iBark + 1])
            iBark++;
        assert(iBark < NUM_BARK_BAND);

        if (rgbBandNotCoded [iBark] == 1)
        {
            FastFloat ffltNoisePower;
            UInt uiNoisePowerXinvMaxWeight;
            Int iUBLimitOniRecon = min(pau->m_rgiBarkIndex [iBark + 1], pau->m_cHighCutOff);

            assert(iNoiseBand < cNoiseBand);

            ffltNoisePower = qstCalcQuantStep(rgiNoisePower[iNoiseBand],&isValid);
			TRACE_QUANT_STEP(isValid);
            //// Float Noise Power = ffltNoisePower.iFraction/(1.0F*(1<<ffltNoisePower.iFracBits))

            ffltNoisePower = ffltMultiply(ffltNoisePower, rgffltSqrtBWRatio[iNoiseBand]);
            //// Float Noise Power = ffltNoisePower.iFraction/(1.0F*(1<<ffltNoisePower.iFracBits))

            uiNoisePowerXinvMaxWeight = MULT_HI(ffltNoisePower.iFraction, uiInvMaxWeight >> 1) << 1;
            cFracBits = ffltNoisePower.iFracBits + (INV_MAX_WEIGHT_FRAC_BITS - 31);
            //// Float NoisePower/MaxWeight = uiNoisePowerXinvMaxWeight/(1.0F*(1<<cFracBits))
            NormUInt(&uiNoisePowerXinvMaxWeight, &cFracBits, 0x3FFFFFFF);

            while (iRecon < iUBLimitOniRecon)
            {
                Int iNoiseRand, iNoiseWeighted, iCoefRecon;

                qrand = quickRand(&(pau->m_tRandState));           // FB = 29
                iNoiseRand = MULT_HI(uiNoisePowerXinvMaxWeight, qrand);
                cFracBits2 = cFracBits + 29 - 31;
                //// Float NoiseRand = iNoiseRand/(1.0F*(1<<cFracBits2))

                // Since weight factors became unsigned, the following assert is not
                // valid. Other wrap-around detection would have to be performed elsewhere.
                assert(rgiWeightFactor[iRecon] <= (0x7FFFFFFF >> cMaxWeightFracBits));
                uiWeightFactor = rgiWeightFactor [iRecon] << cMaxWeightFracBits;
                cWFFracBits = WEIGHTFACTOR_FRACT_BITS + cMaxWeightFracBits;
                //// Float WeightFactor =  uiWeightFactor/(1024.0F*(1<<(cWFFracBits-10)))
                // Since weight factors became unsigned, the following assert is not
                // valid. Other wrap-around detection would have to be performed elsewhere.
                assert(uiWeightFactor <= 0x7FFFFFFF);
                NormUInt(&uiWeightFactor, &cWFFracBits, 0x3FFFFFFF);    // uiWeightFactor with cWFFracBits fractional bits
//    MONITOR_RANGE(gMR_weightFactor,uiWeightFactor/pow(2,cWFFracBits));

                iNoiseWeighted = MULT_HI(iNoiseRand, uiWeightFactor);
                cFracBits2 += (cWFFracBits - 31);
                //// Float NoiseWeighted = iNoiseWeighted/(1024.0F*(1<<(cFracBits2-10)))

                iCoefRecon = Align2FracBits(iNoiseWeighted, cFracBits2, TRANSFORM_FRACT_BITS);    //scale so that (float)iCoefRecon/(1<<TRANSFORM_FRACT_BITS)
                rgiCoefRecon [iRecon] = iCoefRecon;
                //// Float CoefRecon = rgiCoefRecon [iRecon]/32.0F

//    VERIFY_COEF_RECON_LR(iRecon,qrand,cNoiseBand);

                iRecon++;
            }

            iNoiseBand++;
        }
        else
        { // This should be the same as the first < FirstNoiseIndex loop
            // Float fltNoise = pau->m_fltDitherLevel * ((Float) quickRand (&(pau->m_tRandState)) / (Float) 0x20000000);
            // rgfltCoefRecon [iRecon] = (Float) ((rgiCoefQ [iCoefQ] + fltNoise) * rgfltWeightFactor[iRecon] * dblQuantStep * fltInvMaxWeight);

            Int iCoef, iNoiseQuant, iCoefScaled, iCoefRecon;

            if (iRecon >= pau->m_rgiBarkIndex [iBark + 1])
                iBark++;
            assert(iBark < NUM_BARK_BAND);

            qrand = quickRand(&(pau->m_tRandState));

            assert(rgiWeightFactor[iRecon] <= (0x7FFFFFFF >> cMaxWeightFracBits));
            uiWeightFactor = rgiWeightFactor [iRecon] << cMaxWeightFracBits;
            cWFFracBits = WEIGHTFACTOR_FRACT_BITS + cMaxWeightFracBits;
            //// Float WeightFactor = uiWeightFactor/(1024.0F*(1<<(cWFFracBits-10)))
            assert(uiWeightFactor <= 0x7FFFFFFF);
            NormUInt(&uiWeightFactor, &cWFFracBits, 0x3FFFFFFF);    // uiWeightFactor with cWFFracBits fractional bits
//   MONITOR_RANGE(gMR_weightFactor,uiWeightFactor/pow(2,cWFFracBits));

            QuantStepXMaxWeightXWeightFactor = MULT_HI(uiQuantStepXInvMaxWeight, uiWeightFactor);
            cFracBits = cQSIMWFracBits + cWFFracBits - 31;
            //// Float QuantStep*WightFactor/MaxWeight = QuantStepXMaxWeightXWeightFactor/(1.0F*(1<<cFracBits))
//   MONITOR_RANGE(gMR_QuantStepXMaxWeightXWeightFactor,QuantStepXMaxWeightXWeightFactor/pow(2,cFracBits));

            iNoise = MULT_HI(iDitherFactor, qrand);
            cFracBits2 = 35 + 29 - 31;   // FP = 33
            //// Float Noise = iNoise/(1024.0F*(1<<(33-10)))

            iNoiseQuant = MULT_HI(iNoise, QuantStepXMaxWeightXWeightFactor);
            cFracBits2 += (cFracBits - 31);
            //// Float NoiseQuant = iNoiseQuant/(1024.0F*(1<<(cFracBits2-10)))

            assert((rgiCoefQ[iCoefQ] << iShiftCoefQ) == (((I64)rgiCoefQ[iCoefQ]) << iShiftCoefQ));
            iCoef = MULT_HI((Int)rgiCoefQ[iCoefQ] << iShiftCoefQ, QuantStepXMaxWeightXWeightFactor);
            cFracBits += (iShiftCoefQ - 31);
            //// Float Coef = iCoef/(1.0F*(1<<cFracBits))

            iCoefScaled = Align2FracBits(iCoef, cFracBits, TRANSFORM_FRACT_BITS);
            //// Float CoefScaled = iCoefScaled/32.0F

            assert(cFracBits2 >= TRANSFORM_FRACT_BITS);
            iCoefRecon = iCoefScaled + (iNoiseQuant >> (cFracBits2 - TRANSFORM_FRACT_BITS));
            assert(iCoefRecon == (((I64)iCoefScaled) + ((I64)(iNoiseQuant >> (cFracBits2 - TRANSFORM_FRACT_BITS)))));
            rgiCoefRecon [iRecon] = iCoefRecon;
            //// Float CoefRecon = rgiCoefRecon [iRecon]/32.0F

//   VERIFY_COEF_RECON_LR(iRecon,qrand,cNoiseBand);

            iRecon++;
            iCoefQ++;
        }
    }

    { //Calculate from highCutOff to m_cSubband
        UInt QuantStepXMaxWeightXWeightFactorXDither;

        // Since weight factors became unsigned, the following assert is not
        // valid. Other wrap-around detection would have to be performed elsewhere.
        assert(rgiWeightFactor[pau->m_cHighCutOff - 1] <= (0x7FFFFFFF >> cMaxWeightFracBits));
        uiWeightFactor = rgiWeightFactor [pau->m_cHighCutOff - 1] << MORE_WF_FRAC_BITS;
        cWFFracBits = WEIGHTFACTOR_FRACT_BITS + MORE_WF_FRAC_BITS;
        //// Float WeightFactor = uiWeightFactor/(1024.0F*(1<<(cWFFracBits-10)))
        NormUInt(&uiWeightFactor, &cWFFracBits, 0x3FFFFFFF);

        QuantStepXMaxWeightXWeightFactor = MULT_HI(uiQuantStepXInvMaxWeight, uiWeightFactor);
        cFracBits = cQSIMWFracBits + cWFFracBits - 31;
        //// Float QuantStep*WeightFactor/MaxWeight = QuantStepXMaxWeightXWeightFactor/(1024.0F*(1<<(cFracBits-10)))
        NormUInt((unsigned long *)&QuantStepXMaxWeightXWeightFactor, &cFracBits, 0x3FFFFFFF);

        QuantStepXMaxWeightXWeightFactorXDither = MULT_HI(QuantStepXMaxWeightXWeightFactor, iDitherFactor);
        cFracBits += (35 - 31);
        //// Float QS * WF/MaxWF * Dither = QuantStepXMaxWeightXWeightFactorXDither/(1024.0F*(1<<(cFracBits-10)))
        NormUInt(&QuantStepXMaxWeightXWeightFactorXDither, &cFracBits, 0x3FFFFFFF);

        while (iRecon < ppcinfo->m_cSubband)
        {
            Int iCoefScaled;
            Int qrand = quickRand(&(pau->m_tRandState));
            Int iCoefRecon = MULT_HI(QuantStepXMaxWeightXWeightFactorXDither, qrand);
            cFracBits2 = cFracBits + 29 - 31;
            //// Float CoefRecon = iCoefRecon/(1.0F*(1<<cFracBits2))

            iCoefScaled = Align2FracBits(iCoefRecon, cFracBits2, TRANSFORM_FRACT_BITS);
            rgiCoefRecon [iRecon] = iCoefScaled;
            //// Float CoefRecon = rgiCoefRecon [iRecon]/32.0F

//   MONITOR_RANGE(gMR_QuantStepXMaxWeightXWeightFactor,QuantStepXMaxWeightXWeightFactor/pow(2,cFracBits));
//   VERIFY_COEF_RECON_LR(pau->m_cHighCutOff - 1,qrand,cNoiseBand);

            iRecon++;
        }
    }

#   if defined(REFERENCE_RAND_24) || defined(REFERENCE_RAND_16)
    // call the random generator one extra time per subframe to improve subband randomness
    quickRand(&(pau->m_tRandState));
#   endif // defined(REFERENCE_RAND_24) || defined(REFERENCE_RAND_16)

    FUNCTION_PROFILE_STOP(&fp);
    return WMA_OK;
} // prvInverseQuantizeLowRate

//#endif //WMA_OPT_INVERSQUAN_LOWRATE_ARM

//#pragma warning (default:4554)
//#endif // BUILD_INTEGER

//****************************************************************************
//****************************************************************************
//
//  MidRate
//
// Functions to implement MidRate
//
//****************************************************************************
//****************************************************************************
//function prototype, used to eliminate Mac warnings
Void prvGetBandWeightMidRate(CAudioObject* pau, PerChannelInfo* ppcinfo, const Int* const rgiBarkIndex, const Int* const rgiBarkIndexResampled, MaskResampleInfo MRI);

//*********************************************************************************
// prvGetBandWeightMidRate
// Calculate a band weight for bands which are not coded
//
// BW = average( 10^(2*MaskQ[iBarkResampled]/16) ) over each linear frequency bin
// For most sampling rates, the bark scale when resampled does not equal the un-resampled bark scale
// But 32000 does have the nice property that the resampled bark scale = un-resampled in the noise substitution region
// So for 32000 Hz to 44099 Hz:
//  BW = 10^((2/16)*MaskQ[iBarkResampled])
// For all other sampling frequencies:
//  BW = ( N1 * 10^(2*MarkQ[iBarkResampled]) + N2 * 10^(2*MarkQ[iBarkResampled+1]/16) ) / (N1 + N2)
//  where N1 is the number of linear frequency bins in the first resampled bark band
//  and   N2 is the number of linear frequency bins in the second resampled bark band
// BandWeights are only used as the sqrt of the ratio to the last noise BandWeight
// So for 32000 Hz to 44099 Hz
//  SqrtBWRatio = 10^((MaskQ[iBarkResampled] - MaskQ[iLast])/16)
// And for other sampling frequencies
//  SqrtBWRatio = sqrt( BW[i] / BW[last] )
// And for all cases where SqrtBWRatio[last] = 1
// Note that log to the base 10^(1/16) of the MaskQ are integers for 32000 but are not integers for non-32000Hz
// So doing all the inverse quantization in the exponent domain does not have the advantage of using integer exponents
// For this reason, SqrtBWRatio is stored and used as a FastFloat.
// TODO:  rgffltSqrtBWRatio seems to have a narrow range between 1/4 and 4. Consider making it a fixed point int.
// TODO:  Avoid recalculating when MaskQ's have not been updated and the resample ratio is the same as the previous subframe
//*********************************************************************************
#if 1//ndef FFLT_SQRT_RATIO
FastFloat ffltSqrtRatio(FastFloat fflt1, FastFloat fflt2)
{
#if 1//defined(BUILD_INTEGER)
    U64 u64Ratio;
    FastFloat fflt;
    Int iMSF8;
    U32 uiMSF;
    U32 uiFrac1;
    Int iExp = 0;
    if (fflt2.iFraction == 0)
    {
        assert(fflt2.iFraction != 0);   // divide by 0
        fflt.iFraction = 0x7FFFFFFF;
        fflt.iFracBits = 0;
        return fflt;
    }
    //// fflt1.iFraction*0.5F/(1<<(fflt1.iFracBits-1))
    //// fflt2.iFraction*0.5F/(1<<(fflt2.iFracBits-1))
    u64Ratio = (((U64)fflt1.iFraction) << 32) / fflt2.iFraction;
    uiMSF = (U32)(u64Ratio >> 32);
    if (uiMSF == 0)
    {
        iExp = 32;
        uiMSF = (U32)u64Ratio;
    }
    assert(uiMSF != 0) ;
    // normalize the most significant fractional part
    while ((uiMSF & 0xF0000000) == 0)
    {
        iExp += 4;
        uiMSF <<= 4;
    }
    while ((uiMSF & 0x80000000) == 0)
    {
        iExp++;
        uiMSF <<= 1;
    }
    // discard the most significant one bit (it's presence is built into g_InvQuadRootFraction)
    iExp++;
    // get all 32 bits from source
#if defined(PLATFORM_OPTIMIZE_MINIMIZE_BRANCHING)
    uiMSF = (U32)((u64Ratio << iExp) >> 32);
#else
    uiMSF = (iExp > 32) ? (U32)(u64Ratio << (iExp - 32)) : (U32)(u64Ratio >> (32 - iExp));
#endif
    // split into top SQRT_FRACTION_TABLE_LOG2_SIZE==8 bits for fractional lookup and bottom bits for interpolation
    iMSF8 = uiMSF >> (32 - SQRT_FRACTION_TABLE_LOG2_SIZE);
    uiMSF <<= SQRT_FRACTION_TABLE_LOG2_SIZE;
    // lookup and interpolate
#ifdef WMA_TABLE_ROOM_VERIFY
    uiFrac1  = ((const UInt*)p_g_SqrtFraction)[iMSF8++];   // BP2
    uiFrac1 += MULT_HI_UDWORD(uiMSF, ((const UInt*)p_g_SqrtFraction)[iMSF8] - uiFrac1);
#else
    uiFrac1  = g_SqrtFraction[iMSF8++];   // BP2
    uiFrac1 += MULT_HI_UDWORD(uiMSF,  g_SqrtFraction[iMSF8] - uiFrac1);
#endif
    // adjust by sqrt(1/2) if expoenent is odd
    if ((iExp + fflt1.iFracBits - fflt2.iFracBits) & 1)
    {
        // multiply by 1/sqrt(2) and adjust fracbits by 1/2
        uiFrac1 = MULT_HI_UDWORD(uiFrac1, UBP0_FROM_FLOAT(0.70710678118654752440084436210485));
        fflt.iFracBits = ((fflt1.iFracBits - (fflt2.iFracBits + 1)) >> 1) + (iExp - 3);
    }
    else
    {
        fflt.iFracBits = ((fflt1.iFracBits - fflt2.iFracBits) >> 1) + (iExp - 3);
    }
    fflt.iFraction = uiFrac1 >> 1;  // make sure sign is positive
    //// fflt.iFraction*0.5F/(1<<(fflt.iFracBits-1))
    Norm4FastFloatU(&fflt);
//#if defined(_DEBUG) && 0
//    {
//        // old way used float
//        Float flt = (Float)sqrt(FloatFromFastFloat(fflt1) / FloatFromFastFloat(fflt2));
//        if (fabs(flt - FloatFromFastFloat(fflt)) > 0.01)
//        {
//            DEBUG_BREAK();
//        }
//    }
//#endif
    return fflt;
#else
    return (Float)sqrt(fflt1 / fflt2);
#endif
}

#endif

Void prvGetBandWeightMidRate(CAudioObject* pau, PerChannelInfo* ppcinfo,
                             const Int* const rgiBarkIndex, const Int* const rgiBarkIndexResampled, MaskResampleInfo MRI)
{
    U8* rgbBandNotCoded    = ppcinfo->m_rgbBandNotCoded;
    FastFloat* rgffltSqrtBWRatio = ppcinfo->m_rgffltSqrtBWRatio;
    Int* rgiMaskQ     = ppcinfo->m_rgiMaskQ;
    U8 cNoiseBand = 0;
    FastFloat fflt;
    Int iCurrStart, iCurrBand, iCurrEnd; // indexes for the current subframe
    Int iMaskStart, iMaskBand, iMaskEnd; // indexes in the subframe where the MaskQ's were last updated
    Int iRsmpStart, iRsmpBand, iRsmpEnd; // indexes for the current subframe resampled to the subframe where the MaskQ's were last updated
    int fAllBandsSynced = MRI.iMaskResampleRatioPow == MASKQ_RESAMPLE_OFFSET
                          || (pau->m_iVersion != 1 && 32000 <= pau->m_iSamplingRate && pau->m_iSamplingRate < 44100);
    Int rgiMaskQ4BandNotCoded[10];   // used if bands Syncronized
    FastFloat ffltBandWeight[10];   // used if bands are no Syncronized
    Int cMaskHighCutOff;
    FUNCTION_PROFILE(fp);
    FUNCTION_PROFILE_START(&fp, GET_BAND_WEIGHTS_PROFILE);

    iCurrBand = pau->m_iFirstNoiseBand;
    iMaskBand = iRsmpBand = 0;
    if (!fAllBandsSynced)
        cMaskHighCutOff = iResampleIndex(ppcinfo->m_cSubband, MRI) * pau->m_cHighCutOffLong / pau->m_cFrameSampleHalf;

    while (WMAB_TRUE)
    {
        assert(iCurrBand <= pau->m_cValidBarkBand);
        iCurrStart = max(rgiBarkIndex [iCurrBand], pau->m_iFirstNoiseIndex);
        if (iCurrStart >= pau->m_cHighCutOff)
            break;

        if (rgbBandNotCoded [iCurrBand] == 1)
        {
            assert(cNoiseBand < 10);    // probably (cNoiseBand < 5) would be OK

            // Determine the band for accessing the possibly resampled MaskQ's
            iMaskStart = iResampleIndex(iCurrStart, MRI);
            while (rgiBarkIndexResampled [iMaskBand+1] <= iMaskStart)
                ++iMaskBand;

            if (fAllBandsSynced)
            { // Current subFrame bands synced to band were MaskQ's were last updated
                rgiMaskQ4BandNotCoded[cNoiseBand] = rgiMaskQ[iMaskBand];
            }
            else
            { // Have to check in detail about whether one or two MaskQ's are used by this BandNotCoded
                iCurrEnd   = min(pau->m_cHighCutOff, rgiBarkIndex [iCurrBand + 1]);
                assert(iCurrEnd >= iCurrStart);
                iRsmpStart = iMaskStart;
                iMaskEnd = min(rgiBarkIndexResampled [ iMaskBand+1 ], cMaskHighCutOff);
                iRsmpEnd = min(iResampleIndex(iCurrEnd, MRI), cMaskHighCutOff);
                while (rgiBarkIndexResampled [iRsmpBand+1] <= (iRsmpEnd - 1))
                    ++iRsmpBand;
                assert(iMaskBand == iRsmpBand || (iMaskBand + 1) == iRsmpBand);
                if (iRsmpBand == iMaskBand)
                { // just a constant MaskQ for the whole NoiseBand
                    fflt = ffltMaskPower10(rgiMaskQ[iMaskBand]);
                    ffltBandWeight[cNoiseBand] = FASTFLOAT_MULT(fflt, fflt);
//     MONITOR_COUNT(gMC_GBW_floats,4);
                }
                else
                { // Two different MaskQ's for this NoiseBand
                    // BW = ( N1 * 10^(2*MarkQ[iBarkResampled]) + N2 * 10^(2*MarkQ[iBarkResampled+1]/16) ) / (N1 + N2)
                    FastFloat ffltP1, ffltP2;
                    Int iUnRsmpEnd = iUnResampleIndex(iMaskEnd, MRI);
                    FinalWeightType wt1, wt2;

                    fflt  = ffltMaskPower10(rgiMaskQ[iMaskBand]);
                    ffltP1 = fflt;
                    fflt  = ffltMaskPower10(rgiMaskQ[iRsmpBand]);
                    ffltP2 = fflt;
                    wt1 = FRAC_FROM_RATIO((FinalWeightType)iUnRsmpEnd - iCurrStart, (U32)(iCurrEnd - iCurrStart), 30);
                    wt2 = FRAC_FROM_RATIO((FinalWeightType)iCurrEnd - iUnRsmpEnd, (U32)(iCurrEnd - iCurrStart), 30);
                    ffltP1 = FASTFLOAT_MULT(ffltP1, ffltP1);
                    ffltP1 = FASTFLOAT_FLOAT_MULT(ffltP1, wt1, 30);
                    ffltP2 = FASTFLOAT_MULT(ffltP2, ffltP2);
                    ffltP2 = FASTFLOAT_FLOAT_MULT(ffltP2, wt2, 30);
                    ffltBandWeight[cNoiseBand] = FASTFLOAT_ADD(ffltP1, ffltP2);

//     MONITOR_COUNT(gMC_GBW_floats,13);
                }
            }
            cNoiseBand++;
        }
        iCurrBand++;
    }
    for (iCurrBand = 0; iCurrBand < (cNoiseBand - 1); iCurrBand++)
    {
        if (fAllBandsSynced)
        {
            fflt = ffltMaskPower10(rgiMaskQ4BandNotCoded[iCurrBand] - rgiMaskQ4BandNotCoded[cNoiseBand-1]);
            rgffltSqrtBWRatio[iCurrBand] = fflt;
            //// float SqrtBWRatio = fflt.iFraction*1.0F/(1<<fflt.iFracBits)
        }
        else
        {
            // Note that limited range of the result probably means it can be done more quickly
            rgffltSqrtBWRatio[iCurrBand] = ffltSqrtRatio(ffltBandWeight[iCurrBand], ffltBandWeight[cNoiseBand-1]);
            //// float SqrtBWRatio = fflt.iFraction*1.0F/(1<<fflt.iFracBits)
        }
//  MONITOR_RANGE(gMR_rgffltSqrtBWRatio,FLOAT_FROM_FASTFLOAT(fflt));
//  MONITOR_COUNT(gMC_GBW_floats,14);  // count sqrt as 10.
    }
    if (cNoiseBand > 0)
    { // last band has a ratio of 1.0

//#if defined(BUILD_INTEGER)
        rgffltSqrtBWRatio[cNoiseBand-1].iFraction = 0x40000000;
        rgffltSqrtBWRatio[cNoiseBand-1].iFracBits = 30;
//#else
//  rgffltSqrtBWRatio[cNoiseBand-1] = 1.0F;
//#endif // BUILD_INTEGER
//  MONITOR_RANGE(gMR_rgffltSqrtBWRatio,1.0F);
    }

    assert(cNoiseBand < pau->m_cValidBarkBand);
    assert(pau->m_iFirstNoiseBand > 0);
    rgbBandNotCoded [0] = cNoiseBand;    // used as cNoiseBand
    FUNCTION_PROFILE_STOP(&fp);
} // prvGetBandWeightMidRate
#pragma arm section code

#endif

//#pragma warning (disable:4554)



// **********************************************************************
// Macros for DecodeCoefsMidRate with combined INTEGER and INT_FLOAT code

//#if defined(BUILD_INTEGER)

#define MASK_X_QUANT(iLevel,ffltQuantizer) MULT_HI(((iLevel)<<16),ffltQuantizer.iFraction)
#define RAND_X_QUANT(rnd,ffltQuantizer) MULT_HI((rnd),ffltQuantizer.iFraction)
#define UNNORMED_MULT(fflt,c,shift) \
 fflt.iFraction = MULT_HI( fflt.iFraction, c ); \
 fflt.iFracBits += shift;

// SCALE_COEF_RECON shifts CoefRecon to give it TRANSFORM_FRACT_BITS==5 fractional bits
#if 0//defined(PLATFORM_OPTIMIZE_MINIMIZE_BRANCHING)
// This SCALE_COEF_RECON macro requires 6 ops and no branches
// This SETUP_FOR_SCALE_COEF_RECON requires 5 ops plus 1 branch.
// SCALE_COEFFICENT gets executed 25x as often as SETUP_FOR_SCALE_COEF_RECON,
// so this method requires 6.2 ops plus 0.04 branches per SCALE_COEFFICENT
# define SCALE_COEF_RECON(iCR) ((((iCR)>>-iShift) & iMask2) | (((iCR)<<iShift) & iMask1))
//# define SETUP_FOR_SCALE_COEF_RECON(iFB) iShift = iFB-TRANSFORM_FRACT_BITS;  \
//              iMask2 = 0xFFFFFFFF ^ (iMask1 = (iShift>=0) ? 0xFFFFFFFF : 0)
// See comment below
# define SETUP_FOR_SCALE_COEF_RECON(fftQ) iShift = fftQ.iFracBits-TRANSFORM_FRACT_BITS;  \
              iMask2 = 0xFFFFFFFF ^ (iMask1 = (iShift>=0) ? 0xFFFFFFFF : 0)

#else // so !PLATFORM_OPTIMIZE_MINIMIZE_BRANCHING
// When branching is not a high penaty activity, do it the simplier way
//   iCoefRecon = (t=iFracBits-5)<0 ? iCoefRecon>>-t : iCoefRecon<<t
// This SCALE_COEF_RECON requires 3 ops plus 1 branch or 2 ops plus 1 branch.
// This SETUP_FOR_SCALE_COEF_RECON requires 2 ops
// SCALE_COEFFICENT gets executed 25x as often as SETUP_FOR_SCALE_COEF_RECON,
// so this method requires 2.58 ops plus 0.04 branches per SCALE_COEFFICENT
// On one test on a 500 MHz Pentium 686, this way saves 1% execution time over masking.
# define SCALE_COEF_RECON(iCR) (iShift<0) ? (iCR)<<-iShift : (iCR)>>iShift
//# define SETUP_FOR_SCALE_COEF_RECON(iFB) iShift=iFB-TRANSFORM_FRACT_BITS
// This more complex setup (with pre-normalization) is required to deal with 56_WMAv2.wma which ends
// with 1.5 seconds of DC bias at -890.  This results in a single large coef at 0 and the rest 0.
// Then in the noise band, iShift tries to be == 33...
# define SETUP_FOR_SCALE_COEF_RECON(fftQ) \
    while( fftQ.iFracBits > (30+TRANSFORM_FRACT_BITS) ) { fftQ.iFracBits--; fftQ.iFraction>>=1; } \
    iShift=fftQ.iFracBits-TRANSFORM_FRACT_BITS;
#endif // PLATFORM_OPTIMIZE_MINIMIZE_BRANCHING

#define COEF_PLUS_NOISE_FRAC_BITS 22
#define DITHER_FRAC_BITS 35
#define RAND_FRAC_BITS 29
#define MORE_WF_FRAC_BITS 0

#define RAND_TO_NOISE(qr) qr
#ifdef WMAMIDRATELOWRATE
#pragma arm section code = "WmaLowRateCode"
// Inverse Quantize for "normal" case of CoefQ and Noise Dithering
static void CoefPlusNoiseInvQuant(const Int iRecon, const Int iCoefQ, I32 qrand,
        const I32* rgiCoefQ, CoefType* rgiCoefRecon, FastFloat ffltQuantizer, Int iShift)
{
    Int iNoise, iCoefPlusNoise, iCoefRecon;

    // Multiply by Dither and align iNoise fractional bits to be COEF_PLUS_NOISE_FRAC_BITS == 22
    iNoise = MULT_HI(cDitherFactorMR, qrand) >> ((DITHER_FRAC_BITS + RAND_FRAC_BITS - 31) - COEF_PLUS_NOISE_FRAC_BITS);
    //// Float Noise = iNoise/(1.0F*(1<<22))
    if (abs(rgiCoefQ[iCoefQ]) >= (1 << (31 - COEF_PLUS_NOISE_FRAC_BITS)))
    { // rare, but it does happen occasionally (e.g. tough_32m_32)
        Int iFB = 0;
        UInt uiCoefQ = abs(rgiCoefQ[iCoefQ]);
        while (uiCoefQ >= (1 << (31 - COEF_PLUS_NOISE_FRAC_BITS)))
        {
            uiCoefQ >>= 1;
            iFB++;
        }
        iCoefPlusNoise = (rgiCoefQ[iCoefQ] << (COEF_PLUS_NOISE_FRAC_BITS - iFB)) + (iNoise >> iFB);
        //// Float Coef+Noise = iCoefPlusNoise/(1.0F*(1<<(22-iFB)))
        iCoefRecon = MULT_HI(iCoefPlusNoise, ffltQuantizer.iFraction);
        //// Float qrand = qrand/(1024.0F*(1<<25))

        ffltQuantizer.iFracBits -= iFB;
        SETUP_FOR_SCALE_COEF_RECON(ffltQuantizer);
        ffltQuantizer.iFracBits += iFB;

        rgiCoefRecon [iRecon] = SCALE_COEF_RECON(iCoefRecon);
        //// Float CoefRecon = rgiCoefRecon[iRecon]/32.0F
        SETUP_FOR_SCALE_COEF_RECON(ffltQuantizer);
    }
    else
    {
        iCoefPlusNoise = (rgiCoefQ[iCoefQ] << COEF_PLUS_NOISE_FRAC_BITS) + iNoise;
        //// Float Coef+Noise = iCoefPlusNoise/(1.0F*(1<<22))
        iCoefRecon = MULT_HI(iCoefPlusNoise, ffltQuantizer.iFraction);
        //// Float qrand = qrand/(1024.0F*(1<<25))
        rgiCoefRecon [iRecon] = SCALE_COEF_RECON(iCoefRecon);
        //// Float CoefRecon = rgiCoefRecon[iRecon]/32.0F
    }
} // CoefPlusNoiseInvQuant
#pragma arm section code

#endif

#ifdef  WMAINITIALIZE
#pragma arm section code = "WmaOpenCodecCode"

WMARESULT prvInitNoiseSubSecondaryFixed(CAudioObject* pau)
{
    pau->m_fNoiseSub = WMAB_TRUE;
    pau->m_fltFirstNoiseFreq = (Float)(pau->m_iSamplingRate * 0.5f);
    // adjust...
    // HongCho: This is related to Bark bands (re-adjust when Bark bands change)
    if (pau->m_iVersion == 1)
    {

        // version 1 was incorrectly using the inequalities...
        // do not change!!!

        if (pau->m_iSamplingRate == 22050)
        {
            // somewhat different parameters...
            if (pau->m_fltWeightedBitsPerSample >= 1.16f)
                pau->m_fNoiseSub = WMAB_FALSE;
            else if (pau->m_fltWeightedBitsPerSample >= 0.72f)
                pau->m_fltFirstNoiseFreq *= (Float)0.70;
            else
                pau->m_fltFirstNoiseFreq *= (Float)0.60;
        }
        else if (pau->m_iSamplingRate == 44100)
        {
            if (pau->m_fltWeightedBitsPerSample >= 0.61f)
                pau->m_fNoiseSub = WMAB_FALSE;
            else
                pau->m_fltFirstNoiseFreq *= (Float) 0.4;
        }
        else if (pau->m_iSamplingRate == 16000)
        {
            if (pau->m_fltBitsPerSample <= 0.5f)
                pau->m_fltFirstNoiseFreq *= (Float) 0.30;
            else
            {
                pau->m_fltFirstNoiseFreq *= (Float) 0.50;
                pau->m_iNoisePeakIgnoreBand = 3;
            }
        }
        else if (pau->m_iSamplingRate == 11025)
        {
            assert(1 == pau->m_iNoisePeakIgnoreBand);
            pau->m_fltFirstNoiseFreq *= (Float) 0.70;
            if (pau->m_fltBitsPerSample >= 0.9f)
                pau->m_iNoisePeakIgnoreBand = 3;
        }
        else if (pau->m_iSamplingRate == 8000)
        {
            assert(1 == pau->m_iNoisePeakIgnoreBand);
            if (pau->m_fltBitsPerSample <= 0.625f)
                pau->m_fltFirstNoiseFreq *= (Float) 0.50;
            else if (pau->m_fltBitsPerSample <= 0.75f)
                pau->m_fltFirstNoiseFreq *= (Float) 0.65;
            else
                pau->m_fNoiseSub = WMAB_FALSE;
        }
        else
        {
            if (pau->m_fltBitsPerSample >= 0.8f)
                pau->m_fltFirstNoiseFreq *= (Float)0.75;
            else if (pau->m_fltBitsPerSample >= 0.6f)
                pau->m_fltFirstNoiseFreq *= (Float)0.60;
            else
                pau->m_fltFirstNoiseFreq *= (Float)0.5;
        }

    }
    //else if (pau->m_iVersion > 2) {
    //        // Some other mechanism to be determined
    //        pau->m_fNoiseSub = WMAB_FALSE;
    //    }

    else
    {

        // for newer versions...  more correct using inequality...

        // Disable noise substitution above 48kHz (I assume these represent
        // high-end targets anyway).
        if (pau->m_iSamplingRate > 48000)
            pau->m_fNoiseSub = WMAB_FALSE;

        else if (pau->m_iSamplingRate >= 44100)
        {
            if (pau->m_fltWeightedBitsPerSample >= 0.61f)
                pau->m_fNoiseSub = WMAB_FALSE;
            else
                pau->m_fltFirstNoiseFreq *= (Float) 0.4;
        }
        else if (pau->m_iSamplingRate >= 22050)
        {
            // somewhat different parameters...
            if (pau->m_fltWeightedBitsPerSample >= 1.16f)
                pau->m_fNoiseSub = WMAB_FALSE;
            else if (pau->m_fltWeightedBitsPerSample >= 0.72f)
                pau->m_fltFirstNoiseFreq *= (Float)0.70;
            else
                pau->m_fltFirstNoiseFreq *= (Float)0.60;
        }
        else if (pau->m_iSamplingRate >= 16000)
        {
            if (pau->m_fltBitsPerSample <= 0.5f)
                pau->m_fltFirstNoiseFreq *= (Float) 0.30;
            else
            {
                pau->m_fltFirstNoiseFreq *= (Float) 0.50;
                pau->m_iNoisePeakIgnoreBand = 3;
            }
        }
        else if (pau->m_iSamplingRate >= 11025)
        {
            assert(1 == pau->m_iNoisePeakIgnoreBand);
            pau->m_fltFirstNoiseFreq *= (Float) 0.70;
            if (pau->m_fltBitsPerSample >= 0.9f)
                pau->m_iNoisePeakIgnoreBand = 3;
        }
        else if (pau->m_iSamplingRate >= 8000)
        {
            assert(1 == pau->m_iNoisePeakIgnoreBand);
            if (pau->m_fltBitsPerSample <= 0.625f)
                pau->m_fltFirstNoiseFreq *= (Float) 0.50;
            else if (pau->m_fltBitsPerSample <= 0.75f)
                pau->m_fltFirstNoiseFreq *= (Float) 0.65;
            else
                pau->m_fNoiseSub = WMAB_FALSE;
        }
        else
        {
            if (pau->m_fltBitsPerSample >= 0.8f)
                pau->m_fltFirstNoiseFreq *= (Float)0.75;
            else if (pau->m_fltBitsPerSample >= 0.6f)
                pau->m_fltFirstNoiseFreq *= (Float)0.60;
            else
                pau->m_fltFirstNoiseFreq *= (Float)0.5;
        }
    }

    return WMA_OK;
} // prvInitNoiseSubSecondaryFixed



WMARESULT prvInitNoiseSubSecondaryAllocated(CAudioObject *pau)
{
    WMARESULT wmaResult = WMA_OK;
    Int iWin, iBand;
    Float fltSamplingPeriod;

    Int *piBarkIndex;
    Int cFrameSample;
    Int iNoiseIndex;

    if (!pau->m_fNoiseSub)
        return wmaResult;

    //calculate index of each bark freq
    fltSamplingPeriod = 1.0F / pau->m_iSamplingRate;
    // wchen: we need to think what to do with the cut off frequencies: not include at all or include zeros.
    //for long window
    piBarkIndex = pau->m_rgiBarkIndexOrig;

    // for the v1 compatibility
    if (pau->m_iVersion == 1)
    {
        // precalculate the first noise bands
        // wchen: who put these two lines here? No use!
        //pau->m_rgiFirstNoiseBand[0] = (Int)(pau->m_fltFirstNoiseFreq*pau->m_cFrameSample*fltSamplingPeriod + 0.5);
        //if(pau->m_rgiFirstNoiseBand[0] > pau->m_cSubband) pau->m_rgiFirstNoiseBand[0] = pau->m_cSubband;
        pau->m_rgiFirstNoiseBand[0] = pau->m_rgcValidBarkBand[0] - 1;// init to max...
        for (iBand = 0; iBand < pau->m_rgcValidBarkBand[0]; iBand++)
        {
#ifdef WMA_TABLE_ROOM_VERIFY
            if (((const U32 *)p_g_rgiBarkFreqV2)[iBand] > (U32)pau->m_fltFirstNoiseFreq)
            {
#else
            if (g_rgiBarkFreqV2[iBand] > (U32)pau->m_fltFirstNoiseFreq)
            {
#endif
                pau->m_rgiFirstNoiseBand[0] = iBand;
                break;
            }
        }
        // if not there, turn it off...
        if (iBand == pau->m_rgcValidBarkBand[0])
            pau->m_fNoiseSub = WMAB_FALSE;
        if (pau->m_rgiFirstNoiseBand[0] <= 0)
        {
            // 0th band can not be noise-substituted.
            // We use rgbBandNotCoded[0] to keep up count.
            wmaResult = TraceResult(WMA_E_NOTSUPPORTED);
            goto exit;
        }
    }
    else
    {
        for (iWin = 0; iWin < pau->m_cPossibleWinSize; iWin++)
        {
            // precalculate the first noise bands
            pau->m_rgiFirstNoiseBand[iWin] = pau->m_rgcValidBarkBand[iWin] - 1;// init to max...
            cFrameSample = pau->m_cFrameSample / (1 << iWin);
            iNoiseIndex = (Int)(pau->m_fltFirstNoiseFreq * cFrameSample * fltSamplingPeriod + 0.5f);
            for (iBand = 1; iBand < pau->m_rgcValidBarkBand[iWin]; iBand++)
            {
                if (piBarkIndex[iBand] > iNoiseIndex)
                {
                    pau->m_rgiFirstNoiseBand[iWin] = iBand - 1;
                    break;
                }
            }
            if (pau->m_rgiFirstNoiseBand[iWin] <= 0)
            {
                // 0th band can not be noise-substituted.
                // We use rgbBandNotCoded[0] to keep up count.
                wmaResult = TraceResult(WMA_E_NOTSUPPORTED);
                goto exit;
            }
            piBarkIndex +=  NUM_BARK_BAND + 1;
        }
    }
    pau->m_iFirstNoiseBand = pau->m_rgiFirstNoiseBand[0];

exit:
    return wmaResult;
} // prvInitNoiseSubSecondaryAllocated
#pragma arm section code

#endif

#ifdef WMAMIDRATELOWRATE
#pragma arm section code = "WmaLowRateCode"
static Int prvScanForNextBarkIndex(const Int iRecon, Int* piBarkResampled, const int iHighLimit,
        const Int* const rgiBarkIndexResampled, const MaskResampleInfo MRI)
{   //Scan for the next resampled bark index
    Int iNextBarkIndex, iBarkResampled, iReconResampled;
    iReconResampled = iResampleIndex(iRecon, MRI);
    while (iReconResampled >= rgiBarkIndexResampled [*piBarkResampled+1])
        ++(*piBarkResampled);
    iBarkResampled = *piBarkResampled;
    if (iBarkResampled >= MRI.cValidBarkBandLatestUpdate)
    {
        assert(iBarkResampled < MRI.cValidBarkBandLatestUpdate);
        return(MRI.cValidBarkBandLatestUpdate);
    }
    iNextBarkIndex = iUnResampleIndex(rgiBarkIndexResampled [iBarkResampled + 1], MRI);
    if (iNextBarkIndex > iHighLimit)
        iNextBarkIndex = iHighLimit;
    return iNextBarkIndex;
}

Void auResampleWeightFactorLPC(CAudioObject* pau, PerChannelInfo* ppcinfo)
{
    Int iRatio;
    Int i, j;

    UInt*  rguiWeightFactor = ppcinfo->m_rguiWeightFactor;
    Int iSizeSrc, iSizeDst;
    I16* rgiCurrSubFrmSize = ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameSize;

    iSizeSrc = rgiCurrSubFrmSize [ppcinfo->m_iCurrSubFrame - 1];
    iSizeDst = rgiCurrSubFrmSize [ppcinfo->m_iCurrSubFrame];
    assert(ppcinfo->m_iCurrSubFrame > 0);
//#ifndef BUILD_INTEGER_LPC
// assert( sizeof(Int) == sizeof(Float) );  // rgfltWegihtFactor == rgiWeightFactor
//#endif

    // This should only be called for LPC mode. Bark mode resamples weight factor
    // from the original weight factor array to avoid losing information.
    assert(LPC_MODE == pau->m_iWeightingMode);

    if (iSizeSrc > iSizeDst)
    {
        //downsample
        iRatio = iSizeSrc / iSizeDst;
        i = 0;
        while (i < ppcinfo->m_cSubband)
        {
            rguiWeightFactor [i] = rguiWeightFactor [i * iRatio];
            i++;
        }
    }
    else if (iSizeSrc < iSizeDst)
    {
        //upsample
        iRatio = iSizeDst / iSizeSrc;
        i = ppcinfo->m_cSubband / iRatio - 1;
        while (i >= 0)
        {
            for (j = 0; j < iRatio; j++)
            {
                rguiWeightFactor [i * iRatio + j] = rguiWeightFactor [i];
            }
            i--;
        }
    }
    //if == don't need to do anything
// WFR_PRINT(iSizeSrc > iSizeDst ? 5 :(iSizeSrc < iSizeDst ? 6 : 7),rguiWeightFactor);
} // auResampleWeightFactorLPC


//**************************************************************************************************
//
// prvInverseQuantizeMidRate handles only MidRate inverse quantization
//
// rgiWeightFactor is not used.
//
//**************************************************************************************************

WMARESULT prvInverseQuantizeMidRate(CAudioObject* pau, PerChannelInfo* ppcinfo, Int* rgiWeightFactor)
{
    Int iBark = 0;     // index barks using natural scale for this cSubband (at this sampling frequency)
    Int iBarkResampled = 0;   // index barks using resampled scale from cSubbands when the MaskQ's were last updated
    Int iCoefQ = 0;
    Int iRecon = 0;
	int isValid;
    const I32* rgiCoefQ    = ppcinfo->m_rgiCoefQ;
    const U8*  rgbBandNotCoded  = ppcinfo->m_rgbBandNotCoded;
    const Int* rgiNoisePower  = ppcinfo->m_rgiNoisePower;
    const FastFloat* rgffltSqrtBWRatio = ppcinfo->m_rgffltSqrtBWRatio;
    const Int* rgiMaskQ    = ppcinfo->m_rgiMaskQ;
    const Int  iMaxMaskQ   = ppcinfo->m_iMaxMaskQ;
    const Int *rgiBarkIndex;
    const Int *rgiBarkIndexResampled;
    CoefType* rgiCoefRecon   = (CoefType*)ppcinfo->m_rgiCoefRecon;
    CoefType iCoefRecon;
    Int qrand, iReconTarget;
    U8  cNoiseBand = rgbBandNotCoded [0];
    Int iNoiseBand = 0;
    QuantFloat qfltQuantizer;
    MaskResampleInfo MRI;
    Int iShift = 0;
    Bool *rgfMaskNeededForBark;
    // A mask value of a bark should be preserved if that bark:
    // (1) is needed for mask resampling,
    // (2) not coded (noise substituted),
    // (3) has a coded coeff with non-zero value,
    // (4) or has the maximum mask value used in normalization.

    //FUNCTION_PROFILE(fp);
    //FUNCTION_PROFILE_START(&fp,INVERSE_QUAN_MID_RATE_PROFILE);

    if (NULL != ppcinfo->ppcinfoENC)
        rgfMaskNeededForBark = ppcinfo->m_rgfMaskNeededForBark;
    else
        rgfMaskNeededForBark = NULL;


// DEBUG_BREAK_AT_FRAME_INV_QUANT;
// MONITOR_COUNT(gMC_IQ,1);
    assert(pau->m_fNoiseSub &&  pau->m_iWeightingMode == BARK_MODE);
    assert(TRANSFORM_FRACT_BITS == 5);
    assert(ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameSize [ppcinfo->m_iCurrSubFrame] != 0);
    assert(ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameSize [ppcinfo->m_iCurrSubFrame] == ppcinfo->m_cSubFrameSampleHalf);
    // assert (pau->m_cFrameSampleHalf <= (1<<MASKQ_RESAMPLE_OFFSET));       // why???
    assert(pau->m_iSubFrameSizeWithUpdate != 0);
    assert(pau->m_cPossibleWinSize <= MASKQ_RESAMPLE_OFFSET);

    //if (pau->m_iSubFrameSizeWithUpdate <= 0 || ppcinfo->m_cSubFrameSampleHalf <= 0)
    //        return TraceResult(WMA_E_BROKEN_FRAME);


    // Initially, no bark-mask needs to be preserved.
    if (NULL != rgfMaskNeededForBark)
        memset(rgfMaskNeededForBark, 0, 25 * sizeof(Bool));

    // handle changing subFrame window sizes by resampling the indexes
    MRI.iMaskResampleRatio = (pau->m_iSubFrameSizeWithUpdate << MASKQ_RESAMPLE_OFFSET) /
                             ppcinfo->m_cSubFrameSampleHalf;

    MRI.iMaskResampleRatioPow = LOG2(MRI.iMaskResampleRatio);
    rgiBarkIndexResampled     = pau->m_rgiBarkIndexOrig + (NUM_BARK_BAND + 1) *
                                LOG2(pau->m_cFrameSampleHalf / pau->m_iSubFrameSizeWithUpdate);
    MRI.cValidBarkBandLatestUpdate = pau->m_rgcValidBarkBand [LOG2(pau->m_cFrameSampleHalf / pau->m_iSubFrameSizeWithUpdate)];

    rgiBarkIndex  = pau->m_rgiBarkIndex;

    prvGetBandWeightMidRate(pau, ppcinfo, rgiBarkIndex, rgiBarkIndexResampled, MRI);

    //Scan for the first bark index = note iRecon is 0 and rgiBarkIndex[1] ==0 for 16000 Hz and 11025 Hz frames with 128 or 64 samples
    prvScanForNextBarkIndex(iRecon, &iBarkResampled, pau->m_cLowCutOff, rgiBarkIndexResampled, MRI);
    while (iRecon >= rgiBarkIndex[iBark+1])
        iBark++;

    if (iRecon < pau->m_cLowCutOff)   //当进入该分支时用0值代替,若以后测试歌曲进入该分支后再恢复原始代码
    {
        // not integerized since cLowCutOff is typically 0, so this is here for compatability with V1
        // Double dblQuantStep = DOUBLE_FROM_QUANTSTEPTYPE(pau->m_qstQuantStep);
        //Float fltCoefRecon;
        while (iRecon < pau->m_cLowCutOff)
        {
            //Float fltNoise = pau->m_fltDitherLevel * ((Float) quickRand (&(pau->m_tRandState)) / (Float) 0x20000000);//rgfltNoise [iRecon];
            //Float fltWeightRatio = (float)pow(10.0f,(rgiMaskQ[iBarkResampled] - iMaxMaskQ)/16.0f);
            //fltCoefRecon  = (Float) (fltNoise * fltWeightRatio * dblQuantStep);
            rgiCoefRecon [iRecon] = 0;//COEF_FROM_FLOAT(fltCoefRecon); 0 replace the value
//   MONITOR_COUNT(gMC_IQ_Float,9);
            iRecon++;
        }
    }

    while (iRecon < pau->m_iFirstNoiseIndex)
    {
        iReconTarget = prvScanForNextBarkIndex(iRecon, &iBarkResampled, pau->m_iFirstNoiseIndex, rgiBarkIndexResampled, MRI);

        qfltQuantizer = prvWeightedQuantization(pau, ppcinfo, iBarkResampled);
        INTEGER_ONLY(qfltQuantizer.iFracBits += COEF_PLUS_NOISE_FRAC_BITS - 31);        // Account for MULT_HI in loop below
        //// Float Quantizer = qfltQuantizer.iFraction/(512.0F*(1<<qfltQuantizer.iFracBits)
        SETUP_FOR_SCALE_COEF_RECON(qfltQuantizer);

        while (iRecon < iReconTarget)
        {
            //  CoefRecon[iR] = (CoefQ[iQ] + rand()*DitherLevel) * (10^(1/16)) ^ (MaskQ[iB]-Max(MaskQ[])) * (10^(1/20)) ^ QuantStepSize
            qrand = quickRand(&(pau->m_tRandState));
            //// Float qrand = qrand/(1024.0F*(1<<25))
            CoefPlusNoiseInvQuant(iRecon, iCoefQ, qrand, rgiCoefQ, rgiCoefRecon, qfltQuantizer, iShift);
            //// Float CoefRecon = rgiCoefRecon[iRecon]/32.0F

//            VERIFY_COEF_RECON_MR(iRecon,qrand,-1,iBarkResampled);

            if (NULL != rgfMaskNeededForBark && rgiCoefQ[iCoefQ])
                rgfMaskNeededForBark[iBark] = WMAB_TRUE;

            iRecon++;
            iCoefQ++;
        }
        if (iResampleIndex(iRecon + 1, MRI) >= rgiBarkIndexResampled [iBarkResampled+1])
            iBarkResampled++;       // normal to increment except when hitting FirstNoiseIndex
        while (iRecon >= rgiBarkIndex[iBark+1])
            iBark++;
    }

    while (iRecon < pau->m_cHighCutOff)
    {   // These Bands may or may not be coded, treat as appropriate

        if ((rgbBandNotCoded [iBark] == 1))
        {
            Int iLoopMax;


            // CoefRecon[iR] = rand() * ((10^(1/16)) ^ (MaskQ[iB]-Max(MaskQ[])) * sqrt( BandWeight[iN]/BandWeight[cN-1] ) * ((10^(1/20)) ^ NoisePower[iN])
            // Note BandsNotCoded span whole "natural" bark bands, which are not resampled and are not limited by m_cHighCutOff
            FastFloat ffltMaskPower, ffltNoisePower;

            if (NULL != rgfMaskNeededForBark)
                rgfMaskNeededForBark[iBark] = WMAB_TRUE;

            assert(iNoiseBand < cNoiseBand);

            // auCalcQuantStep( rgiNoisePower[iNoiseBand], &ffltNoisePower.iFraction, &ffltNoisePower.iFracBits );
            ffltNoisePower = FASTFLOAT_FROM_QUANTSTEPTYPE(qstCalcQuantStep(rgiNoisePower[iNoiseBand],&isValid));
			TRACE_QUANT_STEP(isValid);
            ffltNoisePower = FASTFLOAT_MULT(ffltNoisePower, rgffltSqrtBWRatio[iNoiseBand]);
            //// Float Noise Power = ffltNoisePower.iFraction/(4.0F*(1<<ffltNoisePower.iFracBits))

            iLoopMax = min(rgiBarkIndex[iBark+1], pau->m_cHighCutOff);
            while (iRecon < iLoopMax)
            {
                ffltMaskPower = ffltMaskPower10(rgiMaskQ[iBarkResampled] - iMaxMaskQ);
                //// Float Mask Power = ffltMaskPower.iFraction/(1.0F*(1<<ffltMaskPower.iFracBits))

                qfltQuantizer = FASTFLOAT_MULT(ffltMaskPower, ffltNoisePower);
                INTEGER_ONLY(qfltQuantizer.iFracBits += (RAND_FRAC_BITS - 31));         // Account for MULT_HI in loop below
                //// Float Quantizer = qfltQuantizer.iFraction/(4.0F*(1<<qfltQuantizer.iFracBits))
                SETUP_FOR_SCALE_COEF_RECON(qfltQuantizer);

                iReconTarget = prvScanForNextBarkIndex(iRecon, &iBarkResampled, iLoopMax, rgiBarkIndexResampled, MRI);
                while (iRecon < iReconTarget)
                {
                    qrand = quickRand(&(pau->m_tRandState));
                    //// Float qrand = qrand/(1024.0F*(1<<25))
                    iCoefRecon = RAND_X_QUANT(RAND_TO_NOISE(qrand), qfltQuantizer);
                    //// Float CoefRecon = iCoefRecon/(1.0F*(1<<qfltQuantizer.iFracBits))
                    rgiCoefRecon [iRecon] = SCALE_COEF_RECON(iCoefRecon);
                    //// Float CoefRecon = rgiCoefRecon[iRecon]/32.0F

//                    VERIFY_COEF_RECON_MR(iRecon,qrand,cNoiseBand,iBarkResampled);

                    iRecon++;
                }
                if (iResampleIndex(iRecon + 1, MRI) >= rgiBarkIndexResampled [iBarkResampled+1])
                    iBarkResampled++;       // normal to increment except when hitting end of Bark Band
            }
            iNoiseBand++;
        }
        else
        {   // This Band is Coded (just like those before FirstNoiseIndex)
            // CoefRecon[iR] = (CoefQ[iQ] + rand()*DitherLevel) * 10^(MaskQ[iB]-Max(MaskQ[]))*2.5*0.5/20 * 10^(QuantStepSize/20)
            iReconTarget = prvScanForNextBarkIndex(iRecon, &iBarkResampled, pau->m_cHighCutOff, rgiBarkIndexResampled, MRI);
            if (iReconTarget > rgiBarkIndex [iBark + 1])
                iReconTarget = rgiBarkIndex [iBark + 1];
            qfltQuantizer = prvWeightedQuantization(pau, ppcinfo, iBarkResampled);
            //// Float Quantizer = qfltQuantizer.iFraction/(1.0F*(1<<qfltQuantizer.iFracBits))
            // account for MULT_HI in loop below
            INTEGER_ONLY(qfltQuantizer.iFracBits += COEF_PLUS_NOISE_FRAC_BITS - 31);
            //// Float Quantizer = qfltQuantizer.iFraction/(512.0F*(1<<qfltQuantizer.iFracBits))
            SETUP_FOR_SCALE_COEF_RECON(qfltQuantizer);

            while (iRecon < iReconTarget)
            {
                //  CoefRecon[iR] = (CoefQ[iQ] + rand()*DitherLevel) * (10^(1/16)) ^ (MaskQ[iB]-Max(MaskQ[])) * (10^(1/20)) ^ QuantStepSize
                qrand = quickRand(&(pau->m_tRandState));
                //// Float qrand = qrand/(1024.0F*(1<<25))
                CoefPlusNoiseInvQuant(iRecon, iCoefQ, qrand, rgiCoefQ, rgiCoefRecon, qfltQuantizer, iShift);
                //// Float CoefRecon = rgiCoefRecon[iRecon]/32.0F

                if (NULL != rgfMaskNeededForBark && rgiCoefQ[iCoefQ])
                    rgfMaskNeededForBark[iBark] = WMAB_TRUE;

//                VERIFY_COEF_RECON_MR(iRecon,qrand,-1,iBarkResampled);

                iRecon++;
                iCoefQ++;
            }
        }
        if (iResampleIndex(iRecon + 1, MRI) >= rgiBarkIndexResampled [iBarkResampled+1])
            iBarkResampled++;       // normal to increment except when hitting HighCutOff
        while (iRecon >= rgiBarkIndex[iBark+1])
            iBark++;
    }

    iReconTarget = ppcinfo->m_cSubband;
    if (iRecon < iReconTarget)
    {   //  CoefRecon[iR] = rand() * DitherLevel * (10^(1/16)) ^ (MaskQ[iB for HighCutOff-1]-Max(MaskQ[])) * (10^(1/20)) ^ QuantStepSize
        // We may have scaned past m_cHighCutOff doing a BandNotCoded, so search back to find it.
        while ((((pau->m_cHighCutOff - 1) << MRI.iMaskResampleRatioPow) >> MASKQ_RESAMPLE_OFFSET) < rgiBarkIndexResampled [iBarkResampled])
            --iBarkResampled;

        qfltQuantizer = prvWeightedQuantization(pau, ppcinfo, iBarkResampled);
        //// Float Quantizer = qfltQuantizer.iFraction/(1.0F*(1<<qfltQuantizer.iFracBits))
        UNNORMED_MULT(qfltQuantizer, cDitherFactorMR, (DITHER_FRAC_BITS - 31) + (RAND_FRAC_BITS - 31));
        //// Float Quantizer = qfltQuantizer.iFraction/(4.0F*(1<<qfltQuantizer.iFracBits))
        SETUP_FOR_SCALE_COEF_RECON(qfltQuantizer);

        while (iRecon < iReconTarget)
        {
            qrand = quickRand(&(pau->m_tRandState));
            //// Float qrand = qrand/(1024.0F*(1<<25))
            iCoefRecon = RAND_X_QUANT(RAND_TO_NOISE(qrand), qfltQuantizer);
            //// Float CoefRecon = iCoefRecon/(1.0F*(1<<qfltQuantizer.iFracBits))
            rgiCoefRecon [iRecon] = SCALE_COEF_RECON(iCoefRecon);
            //// Float CoefRecon = rgiCoefRecon[iRecon]/32.0F

//            VERIFY_COEF_RECON_MR(iRecon,qrand,cNoiseBand,iBarkResampled);

            iRecon++;
        }
    }

#   if defined(REFERENCE_RAND_24) || defined(REFERENCE_RAND_16)
    // call the random generator one extra time per subframe to improve subband randomness
    quickRand(&(pau->m_tRandState));
#   endif

    // rgfMaskNeededForBark[bark corresponding to HighCutOff-1 should be made true.
    if (NULL != rgfMaskNeededForBark)
    {
        for (iBark = pau->m_cValidBarkBand - 1; iBark >= 0; iBark--)
        {
            if ((pau->m_cHighCutOff >= rgiBarkIndex[iBark]) && (pau->m_cHighCutOff < rgiBarkIndex[iBark+1]))
            {
                rgfMaskNeededForBark[iBark] = WMAB_TRUE;
                break;
            }
        }
    }

    FUNCTION_PROFILE_STOP(&fp);
    return WMA_OK;
} // prvInverseQuantizeMidRate
#pragma arm section code

#endif
#endif
#endif
