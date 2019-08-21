//*@@@+++@@@@******************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// Contains functions needed for WMA Std, WMA Pro, and WMA Lossless.
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
#pragma arm section zidata = "WmaDecoderBss",rwdata = "WmaDecoderData"


#define MIN_FRAME_SIZE_V2 (128)
#define SHRT_MAX      32767 

//*****************************************************************************************
// Forward Function Declarations
//*****************************************************************************************

Void prvSetAdjustedValues(CAudioObject *pau, I16 cSubbandAdjusted);
Void prvSetAdjustedValuesPC(CAudioObject *pau, PerChannelInfo *ppcinfo);
Void prvFFT4DCT(CoefType data[], const Int nLog2np);
//--------global variables-----------------------------
CAudioObject gAudioObj;//added by hxd 20080724
I32 gPcmInHis[2];
I32 gDisSilen[2];
CBT gInterInput[2];
CBT gInterInputT[2];
#ifdef SATURERATE_AFTER_FFT 
CBT gRgiCoefRecon[2][2048 + 2048/4];
#else 
CBT gRgiCoefRecon[2][1024*5];
#endif
Int gvalidBarkBand[5];
Int gRgiBarkIndexOrg[5*(NUM_BARK_BAND + 1)];
Int grgcSubWoofer[5];
Int gRgiMaskQ[2*NUM_BARK_BAND];
Int gRgiFstNoiseBand[5];
U8  gRgbandNotCod[NUM_BARK_BAND*2];//repaired by hxd 20080812 纠正原来空间分配(17*2)不够的bug
Int gRgiNoisePower[NUM_BARK_BAND*2];
FastFloat gRgffltSqrt[NUM_BARK_BAND*2];//最大bark band数目位28
CoefType gRgfltCoef[2];
CoefType* gRgpfltCoef[2];
I16  gRgiChInTile[2];
U16  gRgcSampReq[2];

#ifdef  WMAINITIALIZE
#pragma arm section code = "WmaOpenCodecCode"
void RandStateClear(tRandState* ptRandState)
{
    ptRandState->iPrior = 0;
    ptRandState->uiRand = 0;
}

CAudioObject* auNew()
{
    CAudioObject* pau = (CAudioObject*) & gAudioObj;//auMalloc (sizeof (CAudioObject));

    if (pau == NULL)
    {
        TraceResult(WMA_E_OUTOFMEMORY);
        return pau;
    }

    memset(pau, 0, sizeof(CAudioObject));

    memset(&pau->m_qstQuantStep, 0, sizeof(pau->m_qstQuantStep)); // May be struct or float/double
    // memset(pau->m_rgiQuantStepFract, 0, sizeof(pau->m_rgiQuantStepFract));
    pau->m_iPacketCurr = 0;
    pau->m_cBitPackedFrameSize = 0;
    pau->m_cBitPacketLength = 0;
    pau->m_cRunOfZeros = 0;
    pau->m_iLevel = 0;
    pau->m_iSign = 0;

    pau->m_cRunOfMaskZeros    = 0;
    pau->m_iMaskLevel         = 0;
    pau->m_iCurrReconMaskBand = 0;

    pau->m_fNoiseSub = WMAB_FALSE;
    pau->m_fltBitsPerSample = 0;
    pau->m_fltWeightedBitsPerSample = 0;

    pau->m_iMaxEscSize = 9;
    pau->m_iMaxEscLevel = (1 << pau->m_iMaxEscSize) - 1;

    pau->m_iVersion   = 0;
    pau->m_iEncodeOpt = 0;
    pau->m_codecStatus     = CODEC_NULL;
    pau->m_fSeekAdjustment = WMAB_FALSE;
    pau->m_iSamplingRate  = 0;
    pau->m_cBytePerSec    = 0;
    pau->m_cChannel       = 0;
    pau->m_nBytePerSample = 2;
    pau->m_nBitsPerSample = 16;
    pau->m_nValidBitsPerSample = 16;
    pau->m_iBitDepthSelector   = BITDEPTH_16;
    pau->m_nSubWooferChannel   = -1;

    // V3 RTM bits
    //pau->m_bV3RTM   = WMAB_FALSE;

    // V3 lossless LPC
    //pau->m_bEnforcedUnifiedLLM = WMAB_FALSE;
    //pau->m_fEnableUnifiedLLM = WMAB_TRUE;
    //pau->m_bUnifiedLLM = WMAB_FALSE; // Lossless is not the default choice. ChaoHe 8-20-2001

//    pau->m_bMLLMUsePLLMPrevFrm = WMAB_FALSE;
    //pau->m_bUnifiedPureLLMCurrFrm = WMAB_FALSE;
    //pau->m_bUnifiedPureLLMNextFrm = WMAB_FALSE;

    //pau->m_bFirstUnifiedPureLLMFrm = WMAB_FALSE;
    //pau->m_bSecondUnifiedPureLLMFrm = WMAB_FALSE;
    //pau->m_bLastUnifiedPureLLMFrm = WMAB_FALSE;

    //pau->m_bPureLosslessMode  = WMAB_FALSE;
    //pau->m_bDoInterChDecorr   = WMAB_FALSE;
    //pau->m_bDoMCLMS           = WMAB_FALSE;
    //pau->m_bSeekable          = WMAB_FALSE;
    //pau->m_iPCMTrailZeros     = 0;
    //pau->m_bOutputRawPCM      = WMAB_FALSE;
    //pau->m_cGetPCMSamplesDone = 0;

    pau->m_fAllowSuperFrame = WMAB_FALSE;
    pau->m_fAllowSubFrame   = WMAB_FALSE;
    pau->m_fV5Lpc           = WMAB_FALSE;

    pau->m_iCurrReconCoef          = 0;
    pau->m_iSubFrameSizeWithUpdate = 0;
    pau->m_iMaxSubFrameDiv         = 1;
    pau->m_cMinSubFrameSample      = 0;
    pau->m_cMinSubFrameSampleHalf  = 0;
    pau->m_cMinSubFrameSampleQuad  = 0;
    pau->m_cPossibleWinSize = 0;

    pau->m_cBitsSubbandMax  = 0;
    pau->m_cFrameSample     = 0;
    pau->m_cFrameSampleHalf = 0;
    pau->m_cFrameSampleQuad = 0;

    pau->m_cLowCutOff      = 0;
    pau->m_cHighCutOff     = 0;
    pau->m_cLowCutOffLong  = 0;
    pau->m_cHighCutOffLong = 0;

    pau->m_iWeightingMode  = LPC_MODE;
    pau->m_stereoMode      = STEREO_LEFTRIGHT;
    pau->m_iEntropyMode    = 0;
    pau->m_fltDitherLevel  = 0.04F;
    pau->m_iQuantStepSize  = (MIN_QUANT + MAX_QUANT - 1) / 2;

    //pau->m_fltDctScale           = 0;
    pau->m_cValidBarkBand        = 0;
    pau->m_rgiBarkIndex          = NULL;
    pau->m_cSubWooferCutOffIndex = 0;
    pau->m_rgiCoefQ              = NULL;

    pau->m_rgpcinfo              = NULL;

    pau->m_rgiCoefReconOrig      = NULL;
    //pau->m_rgiCoefReconMLLMOrig  = NULL;

    pau->m_rgiMaskQ              = NULL;
    pau->m_rgiMaskQResampled     = NULL;
    pau->m_rgcValidBarkBand      = NULL;
    pau->m_rgiBarkIndexOrig      = NULL;
    pau->m_rgrgrgnBarkResampleMatrix = NULL;
    pau->m_rgcSubWooferCutOffIndex   = NULL;

    pau->m_rgiPCMInHistory = NULL;
    pau->m_rgiDiscardSilence = NULL;
    pau->m_rgiInterlacedInput = NULL;
    pau->m_rgiInterlacedInputT = NULL;
    pau->m_fLeadingSilence = WMAB_FALSE;
    pau->m_u32LeadingSize = 0;
    pau->m_u32TrailingSize = 0;
    pau->m_iPCMReconStart = 0;
    pau->m_iPCMReconEnd = 0;

//#ifdef ENABLE_ALL_ENCOPT
    pau->m_fltFirstNoiseFreq = 0;
    pau->m_iFirstNoiseBand = 0;
    pau->m_iFirstNoiseIndex = 0;
    pau->m_iNoisePeakIgnoreBand = 1;

    pau->m_rgiFirstNoiseBand = NULL;
    pau->m_rgbBandNotCoded = NULL;
    pau->m_rgffltSqrtBWRatio = NULL;
    pau->m_rgiNoisePower = NULL;

//#endif  // ENABLE_ALL_ENCOPT

    pau->m_rgfltWeightFactor = NULL;
    pau->m_rguiWeightFactor  = NULL;

    pau->m_iFrameNumber = 0;

    // transform domain resampling (by 2^n or 1/2^n, where n is (+) integer)
    //pau->m_fHalfTransform         = WMAB_FALSE;
    //pau->m_fPad2XTransform        = WMAB_FALSE;
    //pau->m_iXformSamplingRate     = 0;
    pau->m_iAdjustSizeShiftFactor = 0;
    //pau->m_fWMAProHalfTransform   = WMAB_FALSE;

    pau->m_cFrameSampleAdjusted     = 0;
    pau->m_cFrameSampleHalfAdjusted = 0;
    pau->m_cHighCutOffAdjusted      = 0;
//#ifdef WMAMIDRATELOWRATE
    //pau->aupfnInverseQuantize = prvInverseQuantizeMidRate;
//#endif
    pau->prvpfnInverseTransformMono = NULL; // Not currently used
    pau->aupfnGetNextRun = NULL;

//    pau->aupfnReconSample = auReconSample;
//    pau->m_pfnPostProcess = NULL;

//    pau->aupfnDctIV       = auDctIV;
//    pau->aupfnFFT         = prvFFT4DCT;

    pau->m_pfnSetSample   = NULL;
    pau->m_pfnGetSample   = NULL;


    RandStateClear(&(pau->m_tRandState));



    pau->m_rgrgrgfltMultiXIDCT = NULL;
    pau->m_rgfltCoefDst        = NULL;
    pau->m_rgpfltCoefGrpSrc    = NULL;
    pau->m_rgfltGivensTmp0     = NULL;
    pau->m_rgfltGivensTmp1     = NULL;

    //pau->m_ucDrcFrameScaleFactor = 0;
    pau->m_pDrc = NULL;

    pau->m_cChannelGroup    = 0;
    pau->m_rgChannelGrpInfo = NULL;
    pau->m_cChInTile        = 0;
    pau->m_rgiChInTile      = NULL;

    pau->m_fUseVecCoder = WMAB_FALSE;

    pau->m_fWriteFrameSize = WMAB_FALSE;
    pau->m_fExtraBitsInPktHeader = WMAB_FALSE;
    pau->m_fByteAlignFrame = WMAB_FALSE;
    pau->m_cComprFrameSizeBits = 0;

    pau->m_fGenerateDrcParams = WMAB_FALSE;

    pau->m_rgcSamplesREQ  = NULL;



    return pau;
} // auNew
#pragma arm section code
#endif
//*****************************************************************************************
//
// auDelete
// delete a CAudioObject
//
//*****************************************************************************************
//Void    auDelete (CAudioObject* pau)
//{
//    if (NULL == pau)
//        return;
//
//    //V3 LLMB Chao 03-20-02
////    prvFreeMCLMSPredictor(&pau->m_MCLMSPredictor);
//
//    //V3 LLM-B. Chao. Add. 03-15-02 in the common PerChannelInfo we have some auMalloced memory.
//    //delete them before delete pau->m_rgpcinfo;
////    auDeletePcInfoCommon (pau, pau->m_rgpcinfo);
//
//    DELETE_ARRAY (pau->m_rgiPCMInHistory);
//    DELETE_ARRAY (pau->m_rgiDiscardSilence);
//    //freeAligned(pau->m_rgiInterlacedInput);
//    //freeAligned(pau->m_rgiInterlacedInputT);
//
//    //freeAligned(pau->m_rgiCoefReconOrig);
//    //freeAligned(pau->m_rgiCoefReconMLLMOrig);
//
//    DELETE_ARRAY (pau->m_rguiWeightFactor);
//
//    pau->m_rgfltWeightFactor = NULL;
//
//    DELETE_ARRAY (pau->m_rgcValidBarkBand);
//    DELETE_ARRAY (pau->m_rgiBarkIndexOrig);
//
////    prvDeleteBarkResampleMatrix(pau);
//
//    DELETE_ARRAY (pau->m_rgcSubWooferCutOffIndex);
//    DELETE_ARRAY (pau->m_rgiMaskQ);
//    DELETE_ARRAY (pau->m_rgiMaskQResampled);
//
//    // V3 LLM
////  DELETE_ARRAY (pau->m_rgiLLMContextProb);
//
////#ifdef ENABLE_ALL_ENCOPT
//    {
//        DELETE_ARRAY (pau->m_rgiFirstNoiseBand);
//        DELETE_ARRAY (pau->m_rgbBandNotCoded);
//        DELETE_ARRAY (pau->m_rgffltSqrtBWRatio);
//        DELETE_ARRAY (pau->m_rgiNoisePower);
//    }
////#endif //ENABLE_ALL_ENCOPT
//
//    DELETE_ARRAY (pau->m_rgiCoefQ);
//
//
//
//    DELETE_ARRAY(pau->m_rgfltCoefDst);
//    DELETE_ARRAY(pau->m_rgpfltCoefGrpSrc);
//    DELETE_ARRAY(pau->m_rgfltGivensTmp0);
//    DELETE_ARRAY(pau->m_rgfltGivensTmp1);
//
//
//    DELETE_PTR(pau->m_pDrc);
//    DELETE_ARRAY (pau->m_rgiChInTile);
//
//    DELETE_ARRAY (pau->m_rgcSamplesREQ);
//    //auFree (pau);
//
////    DUMP_MONITOR_RANGES(0);
//} // auDelete


//Void auDeletePcInfoCommon (CAudioObject* pau, PerChannelInfo* rgpcinfo)
//{
//    //V3 LLM-B. Chao. Add. 03-15-02 in the common PerChannelInfo we have some auMalloced memory.
//    //delete them before delete pau->m_rgpcinfo;
//    I16 iCh;
//    if (rgpcinfo)
//    {
//        for (iCh = 0; iCh < pau->m_cChannel; iCh++)
//        {
////            U16 i;
//            PerChannelInfo * ppcinfo = rgpcinfo + iCh;
//            //for (i = 0; i < LLMB_CLMSFLT_TTL_MAX; i++)
//           // {
//           //     prvFreeLMSPredictor(ppcinfo->m_rgLMSPredictors + i);
//           // }
//        }
//        DELETE_ARRAY (rgpcinfo);
//    }
//} // auDeletePcInfoCommon


//***************************************************************************
//***************************************************************************
#ifdef  WMAINITIALIZE
#pragma arm section code = "WmaOpenCodecCode"

WMARESULT prvInitCommonParameters(CAudioObject *pau,
                                  Int           iVersionNumber,
                                  Int           cSubband,
                                  Int           cSamplePerSec,
                                  U16           cChannel,
                                  U32           nBytePerSample,
                                  U16           nValidBitsPerSample,
                                  U32           nChannelMask,
                                  Int           cBytePerSec,
                                  Int           cbPacketLength,
                                  U16           iEncodeOpt,
                                  Int           iOutputSamplingRate,
                                  WMAPlayerInfo *pWmaPlayerInfo)
{
    WMARESULT   wmaResult = WMA_OK;
    U16         iNonSupportedPlayerOpt;
    U16         iPlayerOpt = (pWmaPlayerInfo ? pWmaPlayerInfo->nPlayerOpt : 0);

//    static const char fOKOptions[16] = {
    //        // WinCE Player Option Combinations
    //        WMAB_TRUE,  // 0: normal
    //        WMAB_TRUE,  // 1: Device that does not support 32kHz sampling -> interpolated downsample to 22kHz
    //        WMAB_TRUE,  // 2: Background HalfTransform mode to save CPU cycles
    //        WMAB_TRUE,  // 3: Background HalfTransform mode on device that does not support 32kHz sampling, ends up at 11kHz
    //        WMAB_TRUE,  // 4: A slow CPU which does not support F0 but does support 2*F0
    //        WMAB_TRUE,  // 5: Device that does not support 32kHz sample nor 22kHz playback, plays 32kHz data via 22kHz quality at 44kHz
    //        WMAB_TRUE,  // 6: Background HalfTransform mode for device that does not support half sampling rate
    //        WMAB_TRUE,  // 7: Background with downsampling - why?
    //        WMAB_TRUE,  // 8: Hide HP-430's lack of a low-pass filter for 22kHz output
    //        WMAB_FALSE, // 9: not appropriate - would need to interpolate to 44kHz if appropriate
    //        WMAB_TRUE,  // A: Background HalfTransform mode on a HP-430 at 22kHz, decode as normal (neither half nor doubled)
    //        WMAB_TRUE,  // B: why but why not allow??
    //        WMAB_FALSE, // C: not appropriate
    //        WMAB_FALSE, // D: not appropriate
    //        WMAB_FALSE, // E: not appropriate
    //        WMAB_FALSE  // F: not appropriate
    //    };

//    assert(PLAYOPT_HALFTRANSFORM == 2 && PLAYOPT_PAD2XTRANSFORM == 8);

    pau->m_iVersion = iVersionNumber;
    pau->m_cFrameSampleHalf  = cSubband;
    pau->m_iSamplingRate = cSamplePerSec;
    pau->m_cChannel = cChannel;
    pau->m_nBytePerSample = nBytePerSample;
    pau->m_nValidBitsPerSample = nValidBitsPerSample;
    //pau->m_iPCMSampleMin = PCMSAMPLE_MIN(pau->m_nValidBitsPerSample);
    //pau->m_iPCMSampleMax = PCMSAMPLE_MAX(pau->m_nValidBitsPerSample);
    pau->m_nChannelMask = nChannelMask;
    pau->m_cBytePerSec = cBytePerSec;
    pau->m_cBitPacketLength = cbPacketLength * BITS_PER_BYTE;
    pau->m_iEncodeOpt = iEncodeOpt;
    pau->m_cBytePacketLength = (pau->m_cBitPacketLength >> 3);
    assert((pau->m_cBytePacketLength << 3) == pau->m_cBitPacketLength);
    pau->m_cBitsBytePacketLength = LOG2(pau->m_cBytePacketLength) + 1;
    pau->m_cBitsBitsPacketLength = LOG2(pau->m_cBitPacketLength) + 1;
    assert(pau->m_cBytePacketLength < (1 << pau->m_cBitsBytePacketLength));
    assert(pau->m_cBitPacketLength < (1 << pau->m_cBitsBitsPacketLength));

    // --- Handle player options ---
    // First, figure out if the player has asked us for an option that we
    // didn't build
    iNonSupportedPlayerOpt = ~0; // Turn everything on
//    if (pau->m_bPureLosslessMode == WMAB_TRUE)
    //    {
    //        assert(pau->m_iVersion > 2);
    //        // LLM doesn't support many options.
    //        iNonSupportedPlayerOpt &= ~(PLAYOPT_DYNAMICRANGECOMPR);
    //    }
    //    else

//    {
    //        iNonSupportedPlayerOpt &= ~(PLAYOPT_HALFTRANSFORM);
    //        iNonSupportedPlayerOpt &= ~(PLAYOPT_PAD2XTRANSFORM);
    //        iNonSupportedPlayerOpt &= ~(PLAYOPT_DYNAMICRANGECOMPR);
    //
    ////        if (pau->m_iVersion > 2)
    //        //        {
    //        //            iNonSupportedPlayerOpt &= ~(PLAYOPT_LTRT);
    //        //        }
    //
    //    }


//    if ( (iNonSupportedPlayerOpt & iPlayerOpt) || !fOKOptions[iPlayerOpt&0xF] )
    //    {
    //        // Player has requested something that we didn't build
    //        // or wants an unsupported combination of options
    //        wmaResult = TraceResult(WMA_E_NOTSUPPORTED);
    //        goto exit;
    //    }


//    if ( (iPlayerOpt&(PLAYOPT_HALFTRANSFORM|PLAYOPT_PAD2XTRANSFORM))==(PLAYOPT_HALFTRANSFORM|PLAYOPT_PAD2XTRANSFORM) )
    //    {
    //        // if in pad2X mode and a shift into the background sets half transform mode, do neither
    //        iPlayerOpt &= ~(PLAYOPT_HALFTRANSFORM|PLAYOPT_PAD2XTRANSFORM);
    //    }


    //pau->m_iXformSamplingRate = pau->m_iSamplingRate;
    pau->m_iAdjustSizeShiftFactor = 0;

    // PLAYOPT_XXXTRANSFORM are for testing the 2x/half-transform, so honor them first
//    if (iPlayerOpt & PLAYOPT_HALFTRANSFORM)
    //    {
    //      pau->m_iAdjustSizeShiftFactor++;
    //      pau->m_iXformSamplingRate >>= 1;
    //    }
    //    else if (iPlayerOpt & PLAYOPT_PAD2XTRANSFORM)
    //    {
    //      pau->m_iAdjustSizeShiftFactor--;
    //      pau->m_iXformSamplingRate <<= 1;
    //    }
    //    else if (iOutputSamplingRate!=pau->m_iSamplingRate)
    //    {
    //        pau->m_iAdjustSizeShiftFactor =
    //          (Int)(log((Double)pau->m_iSamplingRate/iOutputSamplingRate)/log(2.0) + 0.5);
    //        if (pau->m_iAdjustSizeShiftFactor > 0)
    //          pau->m_iXformSamplingRate = pau->m_iSamplingRate >> pau->m_iAdjustSizeShiftFactor;
    //        else
    //          pau->m_iXformSamplingRate = pau->m_iSamplingRate << (-pau->m_iAdjustSizeShiftFactor);
    //    }


    //pau->m_fHalfTransform = WMAB_FALSE;
    //pau->m_fPad2XTransform = WMAB_FALSE;
    //pau->m_fWMAProHalfTransform = WMAB_FALSE;

//    if (pau->m_iAdjustSizeShiftFactor < 0)
    //    {
    //      pau->m_iAdjustSizeShiftFactor = -pau->m_iAdjustSizeShiftFactor;
    //      pau->m_fPad2XTransform = WMAB_TRUE;
    //    }
    //    else if (pau->m_iAdjustSizeShiftFactor > 0)
    //    {
    //      pau->m_fHalfTransform = WMAB_TRUE;
    //    }





//#ifdef BUILD_INTEGER
    pau->m_cLeftShiftBitsFixedPre  = COEF_FRAC_BITS;
    pau->m_cLeftShiftBitsFixedPost = COEF_FRAC_BITS;
//    if (pau->m_iVersion == 3)
    //    {
    //        // it is essential not to left shift the input by any amount when
    //        // coefficients coming in since max coef is 31 bits.
    //        pau->m_cLeftShiftBitsFixedPre  = 0;
    //        // having a fixed shift before overlap add reduces pcm mismatches by
    //        // 1 or 2, since pcm input is never more than 24 bits (even after
    //        // quantization shouldn't be very much larger) a shift of 2 is okay.
    //        pau->m_cLeftShiftBitsFixedPost = 2;
    //        // On 24 bit implementations, we should change this value to 0,
    //        // and take the hit of 1 or 2 in pcm mismatch, else we risk overflowing
    //        // or should check both subframes before overlapp-add to see if
    //        // overflow will occur, which may result in slowdown.
    //        // if number of bits less than 28, set fixed shift to 0.
    //        if (MAX32BITVAL <= 0x7ffffff)
    //            pau->m_cLeftShiftBitsFixedPost = 0;
    //    }

//#endif

//exit:

    return wmaResult;
} // prvInitCommonParameters



WMARESULT prvInitCommonSecondaryFixed(CAudioObject *pau)
{
    WMARESULT   wmaResult = WMA_OK;

    if (pau->m_iVersion > 2)
        pau->m_fAllowSuperFrame = WMAB_TRUE;

    // m_iBitDepthSelector is a one-step way to consider both bytePerSample/validBitsPerSample
    assert(pau->m_nBytePerSample > 0 && pau->m_nBytePerSample <= 4);
    assert(pau->m_nValidBitsPerSample > 0 && pau->m_nValidBitsPerSample <= 32);
    pau->m_iBitDepthSelector = (BITDEPTH)((pau->m_nBytePerSample - 1) |
                                          ((pau->m_nValidBitsPerSample - 1) << 2));

    pau->m_nBitsPerSample = pau->m_nBytePerSample * 8;
    pau->m_fltBitsPerSample  = (Float)(pau->m_cBytePerSec * 8.0F /
                                       (pau->m_iSamplingRate * pau->m_cChannel));
    pau->m_fltWeightedBitsPerSample = pau->m_fltBitsPerSample;
    // With the same QuantStep, the stereo is equivant to the mono with 1.6 times the bitrate/ch.
    if (pau->m_cChannel == 2)
        pau->m_fltWeightedBitsPerSample *= (Float) MSA_STEREO_WEIGHT;
    else if (pau->m_cChannel > 2)
        pau->m_fltWeightedBitsPerSample *= (Float) MSA_MULTICH_WEIGHT;

    // Identify subwoofer position
    //prvGetChannelNumAtSpeakerPosition (pau->m_nChannelMask, SPEAKER_LOW_FREQUENCY,
    //                                    &(pau->m_nSubWooferChannel));
    //pau->m_nSubWooferChannel = 5; // Temporary hardcoding for some old content.
    pau->m_cFrameSample      = 2 * pau->m_cFrameSampleHalf;
    pau->m_cFrameSampleQuad  = pau->m_cFrameSampleHalf / 2;

    //if (pau->m_iVersion <= 2)
    {
        pau->m_iWeightingMode    = (pau->m_iEncodeOpt & ENCOPT_BARK) ? BARK_MODE : LPC_MODE;
        pau->m_fV5Lpc = (pau->m_iEncodeOpt & ENCOPT_V5LPC) ? WMAB_TRUE : WMAB_FALSE;

        pau->m_fAllowSuperFrame  = !!(pau->m_iEncodeOpt & ENCOPT_SUPERFRAME);
        pau->m_fAllowSubFrame    = pau->m_fAllowSuperFrame && !!(pau->m_iEncodeOpt & ENCOPT_SUBFRAME);

        if (pau->m_fAllowSubFrame)
        {
            pau->m_iMaxSubFrameDiv = ((pau->m_iEncodeOpt & ENCOPT_SUBFRAMEDIVMASK) >>
                                      ENCOPT_SUBFRAMEDIVSHR);
            if (pau->m_cBytePerSec / pau->m_cChannel >= 4000)
                pau->m_iMaxSubFrameDiv = (8 << pau->m_iMaxSubFrameDiv);
            else
                pau->m_iMaxSubFrameDiv = (2 << pau->m_iMaxSubFrameDiv);
        }
        else
        {
            pau->m_iMaxSubFrameDiv = 1;
        }

        if (pau->m_iMaxSubFrameDiv > pau->m_cFrameSample / MIN_FRAME_SIZE_V2 / 2)
            pau->m_iMaxSubFrameDiv = pau->m_cFrameSample / MIN_FRAME_SIZE_V2 / 2;
    }
//    else
    //    {
    //        //These may not be the best for very low bitrate;
    //        //but since V3 doesn't allow them anyway, we have to revisit
    //        //when working on that.
    //        pau->m_iWeightingMode    = BARK_MODE; //retiring the LPC mode
    //        pau->m_fV5Lpc = WMAB_TRUE;            //irrelavent
    //        pau->m_fAllowSuperFrame  = WMAB_TRUE; //always true for V3; we can achieve low delay with this as well just more work
    //        pau->m_iMaxSubFrameDiv = 1 << ((pau->m_iEncodeOpt & ENCOPT3_SUBFRM_DIV) >> 3);
    //        if (pau->m_iMaxSubFrameDiv > 1)
    //            pau->m_fAllowSubFrame = WMAB_TRUE;
    //        else
    //            pau->m_fAllowSubFrame = WMAB_FALSE;
    //
    ////        if (pau->m_cFrameSampleHalf <= 1024)
    ////            pau->m_iMaxSubFrameDiv = 4;
    //    }

    if (pau->m_iVersion == 1)
        pau->m_cPossibleWinSize = 1;
    else
        pau->m_cPossibleWinSize = LOG2(pau->m_iMaxSubFrameDiv) + 1;

    pau->m_cMinSubFrameSample    = pau->m_cFrameSampleHalf * 2 / pau->m_iMaxSubFrameDiv;
    pau->m_cMinSubFrameSampleHalf = pau->m_cMinSubFrameSample / 2;
    pau->m_cMinSubFrameSampleQuad = pau->m_cMinSubFrameSampleHalf / 2;

    // When using noise substitution for uncoded bark or frequency bands,
    // m_fltDitherLevel sets an overall "gain" of the substitution noise
//#ifdef WMAMIDRATELOWRATE
    if (pau->m_iWeightingMode == LPC_MODE)
        pau->m_fltDitherLevel = 0.04F;
    else // BARK_MODE
//#endif
        pau->m_fltDitherLevel = 0.02F;

//#ifdef WMAMIDRATELOWRATE
    // find the region to apply noise subsitution, in the frequencies...
    TRACEWMA_EXIT(wmaResult, prvInitNoiseSubSecondaryFixed(pau));
//#endif

    pau->m_cBitsSubbandMax = LOG2((U32)pau->m_cFrameSampleHalf);

    if (pau->m_iVersion == 1)
        pau->m_cLowCutOffLong    = LOW_CUTOFF_V1;                     //need investigation
    else
        pau->m_cLowCutOffLong    = LOW_CUTOFF;                        //need investigation

    if (pau->m_iVersion < 3)
        pau->m_cHighCutOffLong       = pau->m_cFrameSampleHalf - 9 * pau->m_cFrameSampleHalf / 100; //need investigation
    else
        pau->m_cHighCutOffLong       = pau->m_cFrameSampleHalf;

    //default
    pau->m_cLowCutOff            = pau->m_cLowCutOffLong;
    pau->m_cHighCutOff           = pau->m_cHighCutOffLong;

    if (pau->m_iVersion <= 2)
    {
        //set up some global coding condtions based on bitrate
        pau->m_iEntropyMode = SIXTEENS_OB;                     //default

        if (pau->m_fltWeightedBitsPerSample < 0.72f)
        {
            if (pau->m_iSamplingRate >= 32000)
                pau->m_iEntropyMode = FOURTYFOURS_QB;
        }
        else if (pau->m_fltWeightedBitsPerSample < 1.16f)
        {
            if (pau->m_iSamplingRate >= 32000)
            {
                pau->m_iEntropyMode = FOURTYFOURS_OB;
            }
        }
    }
//    else
    //    {
    //        pau->m_iEntropyMode = FOURTYFOURS_OB;                  //default
    //    }


    // Adjust frame sizes for half/double transform
    prvSetAdjustedValues(pau, 0);

    //initialize constants for packetization
    if (pau->m_iVersion <= 2)
    {
        pau->m_cBitPackedFrameSize = LOG2((U32)ROUNDF(pau->m_fltBitsPerSample *
                                          pau->m_cFrameSampleHalf / 8.0F)) + 2;
    }
//    else    //variable
    //    {
    // /*
    //        U32 n = LOG2 (pau->m_cBitPacketLength); //really should - NBITS_PACKET_CNT but be safe
    //        assert (pau->m_cBitPacketLength >= (1<<n) &&
    //                pau->m_cBitPacketLength <= ((1<<(n+1)) - 1));
    //
    //        pau->m_cBitPackedFrameSize = LOG2 (n);
    //        if (n > (0xFFFFFFFF >> (BITS_PER_DWORD - pau->m_cBitPackedFrameSize))) //round up
    //            pau->m_cBitPackedFrameSize++;
    //*/
    //        pau->m_cBitPackedFrameSize = LOG2 (pau->m_cBitPacketLength) + 1;
    //    }


    //for v2 it has to be dynamically decided
    //if (pau->m_iVersion == 1)
    //    pau->m_fltDctScale = (Float) sqrt (2.0 / pau->m_cFrameSampleHalfAdjusted);


//#ifdef WMAMIDRATELOWRATE
    prvInitInverseQuadRootTable(pau);
//#endif

//    if (pau->m_iVersion >= 3)
//    {
//        if (pau->m_iEncodeOpt & ENCOPT3_UNSUPPORTED_OPTS)
//            TRACEWMA_EXIT(wmaResult, WMA_E_NOTSUPPORTED);
//    }
//
//    if (pau->m_iVersion >= 3)
//    {
//        pau->m_fUseVecCoder = WMAB_TRUE; // by default use vector huffman code
//    }
//
//    if (pau->m_iVersion >= 3)
//    {
//        if (pau->m_iEncodeOpt & ENCOPT3_WRITE_FRAMESIZE_IN_HDR)
//            pau->m_fWriteFrameSize = WMAB_TRUE;
//    }
//
//    pau->m_bMonoV3 = WMAB_FALSE; // This flag is switched on/off in WMA Voice for old content.
//
//    if (pau->m_iVersion >= 3)
//    {
//        if (pau->m_iEncodeOpt &  ENCOPT3_GENERATE_DRC_PARAMS)
//            pau->m_fGenerateDrcParams = WMAB_TRUE;
//        if (pau->m_cChannel == 1)
//         pau->m_bMonoV3 = WMAB_TRUE;
//        if (pau->m_iEncodeOpt & ENCOPT3_RTMBITS)
//            pau->m_bV3RTM = WMAB_TRUE;
//        if (pau->m_iEncodeOpt &  ENCOPT3_EXTRABITS_IN_PKTHDR)
//            pau->m_fExtraBitsInPktHeader = WMAB_TRUE;
//    }



exit:
    return wmaResult;
} // prvInitCommonSecondaryFixed

WMARESULT prvAllocatePcInfoCommon(CAudioObject* pau, PerChannelInfo* rgpcinfo)
{
    WMARESULT   wmaResult = WMA_OK;
    I16 iCh;

    //V3 LLM-B. Chao. Add. 03-15-02. We initialize LMS filters here.
    for (iCh = 0; iCh < pau->m_cChannel; iCh++)
    {
//        U16   i;

        U8 * pMem = NULL;
        I32 cbMemTtl = 0;
        I32 cbMemUsed = 0;

        PerChannelInfo * ppcinfo = rgpcinfo + iCh;
        // ppcinfo->m_cLMSPredictors = LLMB_CLMSFLT_TTL_MAX;

        //for (i = 0; i < LLMB_CLMSFLT_TTL_MAX; i++)
        //{
        //TRACEWMA_EXIT(wmaResult,
        //     prvInitLMSPredictor(ppcinfo->m_rgLMSPredictors + i, LLMB_CLMSFLT_MAX_ORDER, pMem, &cbMemUsed, cbMemTtl));
        //    TRACEWMA_EXIT(wmaResult,
//                    prvResetLMSPredictor(pau, ppcinfo->m_rgLMSPredictors + i));
        //}
    }

//exit:
    return wmaResult;
} // prvAllocatePcInfoCommon

//#endif
//#ifdef  WMAINITIALIZE
WMARESULT prvInitCommonAllocate(CAudioObject *pau)
{
    WMARESULT   wmaResult = WMA_OK;
    I32 iSize, iSizeCBT;

    pau->m_iSampleMaxValue = (1 << (pau->m_nBitsPerSample - 1)) - 1;
    pau->m_iSampleMinValue = -(1 << (pau->m_nBitsPerSample - 1));


//    TRACEWMA_EXIT(wmaResult, prvInitMCLMSPredictor(pau, &pau->m_MCLMSPredictor, LLMB_MCLMS_MAX_ORDER_EACHCH));
    //prvMCLMSPredictorReset(pau, &pau->m_MCLMSPredictor);

    iSize = sizeof(I32) * pau->m_cChannel;
    iSizeCBT = sizeof(CBT) * pau->m_cChannel;
    pau->m_rgiPCMInHistory = (I32 *)gPcmInHis;//auMalloc(iSize);
    if (NULL == pau->m_rgiPCMInHistory)
    {
        wmaResult = TraceResult(WMA_E_OUTOFMEMORY);
        goto exit;
    }
    memset(pau->m_rgiPCMInHistory, 0, iSize);

    iSize = sizeof(I32) * pau->m_cChannel;
    pau->m_rgiDiscardSilence = (I32 *)gDisSilen;//auMalloc(iSize);
    if (NULL == pau->m_rgiDiscardSilence)
    {
        wmaResult = TraceResult(WMA_E_OUTOFMEMORY);
        goto exit;
    }
    memset(pau->m_rgiDiscardSilence, 0, iSize);

    pau->m_rgiInterlacedInput = (CBT *)gInterInput;//mallocAligned (iSizeCBT, 32);
    if (NULL == pau->m_rgiInterlacedInput)
    {
        wmaResult = TraceResult(WMA_E_OUTOFMEMORY);
        goto exit;
    }
    memset(pau->m_rgiInterlacedInput, 0, iSizeCBT);

    pau->m_rgiInterlacedInputT = (CBT *)gInterInputT;//mallocAligned (iSizeCBT, 32);
    if (NULL == pau->m_rgiInterlacedInputT)
    {
        wmaResult = TraceResult(WMA_E_OUTOFMEMORY);
        goto exit;
    }
    memset(pau->m_rgiInterlacedInputT, 0, iSizeCBT);
    /*pau->m_rgiCoefReconOrig需要的空间为(samplesperframe/2)*3/2 * Channels*sizeof(int)*/
    pau->m_rgiCoefReconOrig     = (CBT*) gRgiCoefRecon;//mallocAligned (sizeof (CBT) *
    //AU_HALF_OR_DOUBLE(pau->m_fWMAProHalfTransform,pau->m_fPad2XTransform,pau->m_cFrameSampleHalf*3/2) * pau->m_cChannel, 32);
    if (pau->m_rgiCoefReconOrig == NULL)
    {
        wmaResult = TraceResult(WMA_E_OUTOFMEMORY);
        goto exit;
    }

//    if (pau->m_fWMAProHalfTransform)
    //    {
    //        // Special one-channel worth of memory for Half transforming MLLM
    //        pau->m_rgiCoefReconMLLMOrig     = (CBT*) mallocAligned (sizeof (CBT) *
    //            pau->m_cFrameSampleHalf, 32);
    //        if (pau->m_rgiCoefReconMLLMOrig == NULL)
    //        {
    //            wmaResult = TraceResult(WMA_E_OUTOFMEMORY);
    //            goto exit;
    //        }
    //    }


//#if defined (BUILD_WMASTD) || defined (BUILD_WMAPRO)
    pau->m_rgcValidBarkBand  = (Int*) gvalidBarkBand;//auMalloc (sizeof (Int) * pau->m_cPossibleWinSize);
    if (pau->m_rgcValidBarkBand == NULL)
    {
        wmaResult = TraceResult(WMA_E_OUTOFMEMORY);
        goto exit;
    }

    pau->m_rgiBarkIndexOrig = (Int*) gRgiBarkIndexOrg;//auMalloc (sizeof (Int) * (NUM_BARK_BAND + 1) * pau->m_cPossibleWinSize); //+1 : including bottom and top end
    if (pau->m_rgiBarkIndexOrig == NULL)
    {
        wmaResult = TraceResult(WMA_E_OUTOFMEMORY);
        goto exit;
    }
    prvSetBarkIndex(pau);
//#endif // BUILD_WMASTD || BUILD_WMAPRO

    // Technically prvSetBarkIndex is an initialization but I have no choice but to call it,
    // as some of the following allocs require m_cValidBarkBand which is non-trivial to
    // set outside of prvSetBarkIndex.


    // Allocate mask resample matrices.
//    TRACEWMA_EXIT(wmaResult, prvAllocateBarkResampleMatrix(pau));

    pau->m_rgcSubWooferCutOffIndex  = (Int*) grgcSubWoofer;//auMalloc (sizeof (Int) * pau->m_cPossibleWinSize);
    if (pau->m_rgcSubWooferCutOffIndex == NULL)
    {
        wmaResult = TraceResult(WMA_E_OUTOFMEMORY);
        goto exit;
    }

//#if defined (BUILD_WMASTD) || defined (BUILD_WMAPRO)
    pau->m_rgiMaskQ         = (Int*) gRgiMaskQ;//auMalloc (sizeof (Int) * NUM_BARK_BAND * pau->m_cChannel);
    if (pau->m_rgiMaskQ == NULL)
    {
        wmaResult = TraceResult(WMA_E_OUTOFMEMORY);
        goto exit;
    }
//#endif // BUILD_WMASTD || BUILD_WMAPRO



//#if defined (ENABLE_ALL_ENCOPT) && defined (BUILD_WMASTD)
    //noise sub stuff
    pau->m_rgiFirstNoiseBand = (Int*) gRgiFstNoiseBand;//auMalloc (sizeof (Int) * pau->m_cPossibleWinSize);
    if (pau->m_rgiFirstNoiseBand == NULL)
        return TraceResult(WMA_E_OUTOFMEMORY);
    pau->m_rgbBandNotCoded  = (U8*) gRgbandNotCod;//auMalloc (pau->m_cValidBarkBand * pau->m_cChannel);
    if (pau->m_rgbBandNotCoded == NULL)
        return TraceResult(WMA_E_OUTOFMEMORY);
    pau->m_rgiNoisePower    = (Int*) gRgiNoisePower;//auMalloc (sizeof (Int) * pau->m_cValidBarkBand * pau->m_cChannel);
    if (pau->m_rgiNoisePower == NULL)
        return TraceResult(WMA_E_OUTOFMEMORY);
    pau->m_rgffltSqrtBWRatio    = (FastFloat*) gRgffltSqrt;//auMalloc (sizeof (FastFloat) * pau->m_cValidBarkBand * pau->m_cChannel);
    if (pau->m_rgffltSqrtBWRatio == NULL)
        return TraceResult(WMA_E_OUTOFMEMORY);
//#endif // ENABLE_ALL_ENCOPT && BUILD_WMASTD

//    TRACEWMA_EXIT(wmaResult, prvMultiXIDCTAllocate(pau));

    pau->m_rgfltCoefDst = (CoefType*) gRgfltCoef;//auMalloc(sizeof(CoefType) * pau->m_cChannel);
    if (pau->m_rgfltCoefDst == NULL)
        return TraceResult(WMA_E_OUTOFMEMORY);

    pau->m_rgpfltCoefGrpSrc = (CoefType**) gRgpfltCoef;//auMalloc(sizeof(CoefType*) * pau->m_cChannel);
    if (pau->m_rgpfltCoefGrpSrc == NULL)
        return TraceResult(WMA_E_OUTOFMEMORY);

    pau->m_rgiChInTile  = (I16*) gRgiChInTile;//auMalloc(sizeof(I16) * pau->m_cChannel);
    if (pau->m_rgiChInTile == NULL)
        return TraceResult(WMA_E_OUTOFMEMORY);

    pau->m_rgcSamplesREQ = (U16*) gRgcSampReq;//auMalloc(sizeof(U16) * pau->m_cChannel);
    if (pau->m_rgcSamplesREQ == NULL)
        return TraceResult(WMA_E_OUTOFMEMORY);

//#ifdef WMAMIDRATELOWRATE //midrate and lowrate 's samplesperframe is less than 1024,且rgiCoefRecon开辟的空间也只要4*2*1024*3/2,所以可以共用gRgiCoefRecon这个全局Buf的空间.若sampleperframe为2048时,暂时不支持,空间不够
    if (pau->m_iWeightingMode == LPC_MODE && pau->m_rguiWeightFactor == NULL)
    {
#ifdef SATURERATE_AFTER_FFT
        pau->m_rguiWeightFactor = (UInt*)((unsigned char*)gRgiCoefRecon + 1024/4*5*2*4);
#else
		/*pau->m_rguiWeightFactor需要的空间为samplesperframe/2 * Channels*sizeof(int)*/
        pau->m_rguiWeightFactor = (UInt*)((unsigned char*)gRgiCoefRecon + 1024*24);//接在pau->m_rgiCoefReconOrig之后，并且还剩余4096个字节
#endif        
        //pau->m_rguiWeightFactor = (UInt*) malloc (sizeof (UInt) * DOUBLE(pau->m_fPad2XTransform,pau->m_cFrameSampleHalf) * pau->m_cChannel);
        //modified by evan wu,low rate wma file is not supported when samples per frame is larger than 1024
        if (pau->m_rguiWeightFactor == NULL)
	 //   if(pau->m_cFrameSampleHalf > 1024)
        {
            wmaResult = TraceResult(WMA_E_OUTOFMEMORY);
            goto exit;
        }
    }
//#endif
exit:
    return wmaResult;
} // prvInitCommonAllocate


WMARESULT prvResetPcInfoCommon(CAudioObject* pau, PerChannelInfo* rgpcinfo)
{
    WMARESULT wmaResult = WMA_OK;
    I16 iCh;
    // Initialize quantizer modifiers.
    //for (iCh = 0; iCh < pau->m_cChannel; iCh++)
    //{
    //    rgpcinfo[iCh].m_ucQuantStepModifierIndex = 0;
//        rgpcinfo[iCh].m_qstQuantStepModifier = qstCalcQuantStepModifier(0, 0);
    // }

    // Initialize mask value setup
    for (iCh = 0; iCh < pau->m_cChannel; iCh++)
    {
        // These pointers will be correctly assigned in encoder and decoder
        rgpcinfo[iCh].m_rgiMaskQResampled    = NULL;
        rgpcinfo[iCh].m_fAnchorMaskAvailable = WMAB_FALSE;
        rgpcinfo[iCh].m_iMaskQuantMultiplier = 1;
    }
    // Reset previous subframe size array
    for (iCh = 0; iCh < pau->m_cChannel; iCh++)
    {
        // These pointers will be correctly assigned in encoder and decoder
        rgpcinfo[iCh].m_iSizePrev = (I16)pau->m_cFrameSampleHalf / 2;
        rgpcinfo[iCh].m_iSizeCurr = rgpcinfo[iCh].m_iSizePrev;
        rgpcinfo[iCh].m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameSize[0] = (I16) pau->m_cFrameSampleHalf;
        rgpcinfo[iCh].m_iCurrCoefPosition = 0;
    }
//exit:
    return wmaResult;
} // prvResetPcInfoCommon


WMARESULT prvInitCommonSecondaryAllocated(CAudioObject *pau)
{
    WMARESULT   wmaResult = WMA_OK;

    // Initialize mask resample matrices.
//#if !defined (DEBUG_PRINT_BARK_NMR)
//    if (pau->m_iVersion > 2)
//#endif
//    {
//        auInitBarkResampleMatrix(pau);
//    }

//    prvSetSubWooferCutOffs (pau);
//    TRACEWMA_EXIT(wmaResult, prvMultiXIDCTInit(pau));

    TRACEWMA_EXIT(wmaResult, prvInitNoiseSubSecondaryAllocated(pau));
exit:
    return wmaResult;
} // prvInitCommonSecondaryAllocated
#pragma arm section code
#endif
#ifdef WMAINITIALIZE_HIGH_MID_ALL_USED
#pragma arm section code = "WmaCommonCode"

WMARESULT prvInitCommonResetOperational(CAudioObject *pau)
{
    WMARESULT   wmaResult = WMA_OK;

    pau->m_iPacketCurr = 0;
    pau->m_iFrameNumber = 0;

    memset(pau->m_rgiPCMInHistory, 0, sizeof(*pau->m_rgiPCMInHistory) * pau->m_cChannel);
#ifdef SATURERATE_AFTER_FFT
    memset(pau->m_rgiCoefReconOrig, 0, sizeof(CBT) *pau->m_cFrameSampleHalf*5 / 4 * pau->m_cChannel);
#else
    memset(pau->m_rgiCoefReconOrig, 0, sizeof(CBT) *pau->m_cFrameSampleHalf*3 / 2 * pau->m_cChannel);
#endif
    //if (pau->m_fWMAProHalfTransform)
    //    {
    //        memset (pau->m_rgiCoefReconMLLMOrig, 0, sizeof (CBT) *
    //            pau->m_cFrameSampleHalf);
    //    }

    pau->m_iPCMReconStart = 0;
    pau->m_iPCMReconEnd = 0;
    // Reset silence variables
    pau->m_fLeadingSilence = WMAB_FALSE;
    memset(pau->m_rgiDiscardSilence, 0, sizeof(pau->m_rgiDiscardSilence[0]) * pau->m_cChannel);
    memset(pau->m_rgiInterlacedInput, 0, sizeof(pau->m_rgiInterlacedInput[0]) * pau->m_cChannel);
    memset(pau->m_rgiInterlacedInputT, 0, sizeof(pau->m_rgiInterlacedInputT[0]) * pau->m_cChannel);
    pau->m_iQuantStepSize = (MIN_QUANT + MAX_QUANT - 1) / 2;
//exit:
    return wmaResult;
} // prvInitCommonResetOperational
#pragma arm section code

#endif
#ifdef WMAINITIALIZE
#pragma arm section code = "WmaOpenCodecCode"

void prvInitGetSetFn(PFNGETSAMPLE *ppfnGetSample,
                     PFNSETSAMPLE *ppfnSetSample,
                     const BITDEPTH iBitDepthSelector)
{
    switch (iBitDepthSelector)
    {
        case BITDEPTH_24:
            //*ppfnSetSample = prvSetSample24;
            //*ppfnGetSample = prvGetSample24;
            break;

        case BITDEPTH_2024:
            //*ppfnSetSample = prvSetSample2024;
            //*ppfnGetSample = prvGetSample2024;
            break;

        case BITDEPTH_16:
            *ppfnSetSample = prvSetSample16;
            *ppfnGetSample = prvGetSample16;
            break;

        default:
            //*ppfnSetSample = prvSetSample;
            //*ppfnGetSample = prvGetSample;
            break;
    }
} // prvInitGetSetFn

WMARESULT prvInitCommonSetFunctionPtrs(CAudioObject *pau)
{
    WMARESULT   wmaResult = WMA_OK;

    prvInitGetSetFn(&pau->m_pfnGetSample, &pau->m_pfnSetSample,
                    pau->m_iBitDepthSelector);

    // Set up DCT and FFT
//    pau->aupfnDctIV = auDctIV;
//    pau->aupfnFFT = prvFFT4DCT;


    // Default reconstruct function
//    pau->aupfnReconSample = auReconSample;

    // --- initialize LMS (MCLMS and CDLMS) function pointers --- //
    if (pau->m_nValidBitsPerSample <= 16)
    {
        assert(pau->m_nValidBitsPerSample > 0);
//        pau->pprvMCLMSPred = prvMCLMSPredictorPred_I16_C;
//        pau->pprvMCLMSUpdate = prvMCLMSPredictorUpdate_I16_C;
//        pau->pprvCDLMSPred  = prvLMSPredictorPred_I16_C;
//        pau->pprvCDLMSUpdate = prvLMSPredictorUpdate_I16_C;
    }
    else
    {
//        pau->pprvMCLMSPred = prvMCLMSPredictorPred_I32_C;
//        pau->pprvMCLMSUpdate = prvMCLMSPredictorUpdate_I32_C;
//        pau->pprvCDLMSPred  = prvLMSPredictorPred_I32_C;
//        pau->pprvCDLMSUpdate = prvLMSPredictorUpdate_I32_C;
    }


//#ifdef ENABLE_ALL_ENCOPT

//    if (pau->m_iWeightingMode == BARK_MODE)
    //    {
    //        if (!pau->m_fNoiseSub)
    //            pau->aupfnInverseQuantize = NULL;
    ////#ifdef WMAMIDRATELOWRATE
    //        else
    //            pau->aupfnInverseQuantize = prvInverseQuantizeMidRate;
    ////#endif
    //    }
    ////#ifdef WMAMIDRATELOWRATE
    //    else
    //        pau->aupfnInverseQuantize = prvInverseQuantizeLowRate;

//#endif
//#endif  // ENABLE_ALL_ENCOPT

    return wmaResult;
} // prvInitCommonSetFunctionPtrs
//*****************************************************************************************
//
// auInit
// initialize a CAudioObject based on information from input file or stream
//
//*****************************************************************************************
WMARESULT auInit(CAudioObject* pau,
                 Int iVersionNumber,
                 Int cSubband,
                 Int cSamplePerSec,
                 U16 cChannel,
                 U32 nBytePerSample,
                 U16 nValidBitsPerSample,
                 U32 nChannelMask,
                 Int cBytePerSec,
                 Int cbPacketLength,
                 U16 iEncodeOpt,
                 Int iOutputSamplingRate,
                 WMAPlayerInfo *pWmaPlayerInfo)
{
    WMARESULT   wmaResult = WMA_OK;

    // Check and store the input parameters into member variables
    TRACEWMA_EXIT(wmaResult, prvInitCommonParameters(pau, iVersionNumber, cSubband, cSamplePerSec,
                  cChannel, nBytePerSample, nValidBitsPerSample, nChannelMask, cBytePerSec,
                  cbPacketLength, iEncodeOpt, iOutputSamplingRate, pWmaPlayerInfo));

    // Set member variables based on input parameters
    TRACEWMA_EXIT(wmaResult, prvInitCommonSecondaryFixed(pau));

    // Select optimized functions based on current platform and/or CPU capabilities
    TRACEWMA_EXIT(wmaResult, prvInitCommonSetFunctionPtrs(pau));

    // Allocate memory buffers, then initialize them
    TRACEWMA_EXIT(wmaResult, prvInitCommonAllocate(pau));
//#ifdef WMAMIDRATELOWRATE
    TRACEWMA_EXIT(wmaResult, prvInitCommonSecondaryAllocated(pau));
//#endif

    // Reset operational variables
    TRACEWMA_EXIT(wmaResult, prvInitCommonResetOperational(pau));

exit:
    return wmaResult;
} // auInit
#pragma arm section code

#endif

//***************************************************************************
// Function: prvGetFramePCM
//
// Purpose:
//   This function intends to return how much PCM is available for retrieval
//   for the entire frame.
//***************************************************************************
#if defined(WMAHIGHRATE) || defined(WMAMIDRATELOWRATE)
#pragma arm section code = "WmaHighLowCommonCode"
void prvGetFramePCM(CAudioObject* pau, Int *piFramePCM)
{
    Int iCh;
    Int iMinSizeOutput;

    iMinSizeOutput = SHRT_MAX; // just a init number
    for (iCh = 0; iCh < pau->m_cChannel; iCh++)
    {
        Int iSubframeNum, iSizeCurr, iSizeNext, iSizeOutput;
        I16 iQ3, iQ4;

        PerChannelInfo* ppcinfo = pau->m_rgpcinfo + iCh;

        iSubframeNum = ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_cSubFrame;
        iSizeCurr = ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameSize[iSubframeNum-1];

//        if (pau->m_fHalfTransform)
        //        {
        //            iSizeCurr >>= pau->m_iAdjustSizeShiftFactor;
        //        }
        //        else if (pau->m_fPad2XTransform)
        //        {
        //            iSizeCurr <<= pau->m_iAdjustSizeShiftFactor;
        //        }


        //if( pau->m_iVersion <= 2 )
        {

            iSizeNext = ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameSize[iSubframeNum];

            //if (pau->m_fHalfTransform)
            //            {
            //                iSizeNext >>= pau->m_iAdjustSizeShiftFactor;
            //            }
            //            else if (pau->m_fPad2XTransform)
            //            {
            //                iSizeNext <<= pau->m_iAdjustSizeShiftFactor;
            //            }


            prvCalcQ3Q4(pau, WMAB_TRUE, (I16)iSizeCurr, (I16)iSizeNext, (I16)iSizeCurr, &iQ3, &iQ4);

            iSizeOutput = pau->m_cFrameSampleHalfAdjusted + iQ3 - iSizeCurr * 3 / 2;

        }
//        else
        //        {
        //
        //            // v3 and above we don't know the size of next subframe
        //            iSizeOutput = pau->m_cFrameSampleHalfAdjusted - iSizeCurr / 2;
        //
        //        }


        // consider history
        iSizeOutput += pau->m_cFrameSampleHalfAdjusted / 2;
        if (iSizeOutput < iMinSizeOutput)
            iMinSizeOutput = iSizeOutput;
    }

    *piFramePCM = iMinSizeOutput - pau->m_iPCMReconStart;//
} // prvGetFramePCM


void prvShiftPCMBuffer(CAudioObject* pau)
{
    Int iCh, iCopySize, iFrameSize;
    pcmType*pcfPCMBuf;

    // if m_iPCMReconStart go beyond frame boundary
    if (pau->m_iPCMReconStart >= pau->m_cFrameSampleHalfAdjusted)
    {

        iFrameSize = pau->m_cFrameSampleHalf;

        iCopySize = pau->m_cFrameSampleHalfAdjusted >> 1;

        for (iCh = 0; iCh < pau->m_cChannel; iCh++)
        {
#ifdef SATURERATE_AFTER_FFT
            pcfPCMBuf = (pcmType*)((CoefType *)pau->m_rgiCoefReconOrig + (iFrameSize * 5 / 4) * iCh) + (iFrameSize >> 1) - iCopySize;
#else
            pcfPCMBuf = (pcmType*)((CoefType *)pau->m_rgiCoefReconOrig + (iFrameSize * 3 / 2) * iCh) + (iFrameSize >> 1) - iCopySize;
#endif

            memcpy(pcfPCMBuf, pcfPCMBuf + pau->m_cFrameSampleHalfAdjusted, iCopySize*sizeof(pcmType));
        }
        pau->m_iPCMReconStart -= pau->m_cFrameSampleHalfAdjusted;
        pau->m_iPCMReconEnd -= pau->m_cFrameSampleHalfAdjusted;
    }
} // prvShiftPCMBuffer

//***************************************************************************
// Function: prvCountAlignedPCM
//
// Purpose:
//   This function returns how much aligned PCM is available for retrieval,
//     after silence is discarded.
//
// Arguments:
//   CAudioObject *pau [in] - CAudioObject
//   I32 *piAlignedPCM [out] - The amount of aligned PCM currently available
//     for reconstruction and return to user, minus discarded silence: eg, if
//     all aligned PCM is to be discarded, the value returned here will be zero.
//   Bool fDiscardSilence [in] - if TRUE, this function pre-discards silence
//     (simulates a call to auGetPCM, but omits the actual reconstruction work).
//     Regardless of the value of this argument, silence is always discarded
//     from the *piAlignedPCM value.
//   Bool fEntireFrame [in] - if TRUE, this function counts the aligned PCM
//     which the entire frame should return. Otherwise only operates on the
//     current (subframe) data.
//***************************************************************************
//void prvCountAlignedPCM(CAudioObject *pau,
//                        U16 *piAlignedPCM,
//                        Bool fDiscardSilence,
//                        Bool fEntireFrame,
//                        Bool fSPDIF)
//{
//    Int iCh;
//    U16 iAlignedPCMMin = SHRT_MAX;
//
//    // There are likely issues with v2 concerning prvInitDiscardSilence (m_iActualPower
//    // not set yet). Fix those before enabling this function for frame duration in v2.
//    // wchen: temporarilly enable v2. tian, pleae make sure this is okay.
////    assert(WMAB_FALSE == fEntireFrame || pau->m_iVersion > 2);
//
//    // In calculating the amount of aligned PCM available, we need to subtract
//    // silence PCM, because in v3, silence is not necessarily aligned across channels.
//    if (CODEC_BEGIN == pau->m_codecStatus)
//        prvInitDiscardSilence(pau, fSPDIF);
//
//    for (iCh = 0; iCh < pau->m_cChannel; iCh++)
//    {
//        PerChannelInfo* ppcinfo = pau->m_rgpcinfo + iCh;
//        U16 iSamplesAvail = (U16)pau->m_rgiPCMInHistory[iCh];
//
//        if (fEntireFrame)
//        {
//            Int i = 0;
//            Int iNominalSubfrSum = 0;
//
//            do
//            {
//                I16 iQ1, iQ2, iQ3, iQ4;
//                const I16 iCurrSubFrameSizeHalf = (I16) ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].
//                    m_rgiSubFrameSize[i];
//
//                // Obtain reconstruction width of current subframe
//                prvCalcQ1Q2(pau, WMAB_TRUE,
//                    (I16)ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameSize[i - 1], // -1 is valid index
//                    iCurrSubFrameSizeHalf,
//                    &iQ1, &iQ2);
//
//                // There might be issues with half/double transform, because
//                // cSubFrameSampleHalfAdjusted might not == iCurrSubFrameSizeHalf.
//                // Verify later, but for now assert to call attention to this.
//                assert(WMAB_FALSE == pau->m_fHalfTransform);
//                assert(WMAB_FALSE == pau->m_fPad2XTransform);
//                prvCalcQ3Q4(pau, WMAB_TRUE, iCurrSubFrameSizeHalf,
//                    (I16)ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameSize[i + 1],
//                    iCurrSubFrameSizeHalf, &iQ3, &iQ4);
//
//                if( pau->m_iVersion <= 2 )
//                    iSamplesAvail += iQ3 - iQ1;
//                else
//                    iSamplesAvail += (U16)( ( iCurrSubFrameSizeHalf +
//                                              ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameSize[i-1] )/2 );
//
//                // We'll know we're finished with current frame when we hit cFrameSampleHalf
//                iNominalSubfrSum += iCurrSubFrameSizeHalf;
//
//                // Advance index
//                i += 1;
//
//            } while(iNominalSubfrSum < pau->m_cFrameSampleHalf);
//            assert(pau->m_cFrameSampleHalf == iNominalSubfrSum);
//        }
//        else
//        {
//            if (pau->m_bPureLosslessMode == WMAB_FALSE &&
//                CURRGETPCM_INVALID != ppcinfo->m_iCurrGetPCM_SubFrame)
//            {
//                if( pau->m_iVersion <= 2 )
//                {
//                    assert(ppcinfo->m_iCurrGetPCM_SubFrame >= ppcinfo->m_iCoefRecurQ1);
//                    iSamplesAvail += ppcinfo->m_iCoefRecurQ3 - ppcinfo->m_iCoefRecurQ1;
//                }
//                else
//                {
//                    iSamplesAvail += (ppcinfo->m_iSizeCurr + ppcinfo->m_iSizePrev) / 2;
//                }
//            }
//        }
//
//        if( fEntireFrame )
//        {
//            // Subtract any silence which must be discarded
//            if (pau->m_rgiDiscardSilence[iCh] > 0)
//            {
//                U16 iSilenceToDiscard;
//
//                iSilenceToDiscard = (U16) min(iSamplesAvail, pau->m_rgiDiscardSilence[iCh]);
//                iSamplesAvail -= iSilenceToDiscard;
//            }
//        }
//        else
//        {
//            // Subtract any silence which must be discarded
//            if (pau->m_rgiDiscardSilence[iCh] > 0)
//            {
//                U16 iSilenceToDiscard;
//
//                iSilenceToDiscard = (U16) min(iSamplesAvail, pau->m_rgiDiscardSilence[iCh]);
//                iSamplesAvail -= iSilenceToDiscard;
//
//                pau->m_rgiDiscardSilence[iCh] -= iSilenceToDiscard;
//            }
//
//   // we simulate the reconstruction only in subframe mode
//   pau->m_rgiPCMInHistory[iCh] = iSamplesAvail;
//  }
//
//        // Calculate minimum PCM output available
//        if (iSamplesAvail < iAlignedPCMMin)
//            iAlignedPCMMin = iSamplesAvail;
//    }
//
//    // Lossless mode has some exceptions: apply them now
//    if (pau->m_bPureLosslessMode)
//    {
//        if (fEntireFrame)
//            iAlignedPCMMin = (I16)pau->m_cFrameSampleHalf;
//        iAlignedPCMMin = (I16)min(iAlignedPCMMin, pau->m_u32ValidSamples);
//        if (fEntireFrame == WMAB_FALSE)
//            iAlignedPCMMin -= pau->m_cGetPCMSamplesDone;
//    }
//
//    *piAlignedPCM = (U16)iAlignedPCMMin;
//} // prvCountAlignedPCM


/****************************************************************************
**
** Function:        auPreGetPCM
**
** Description:     Compute home many samples can be generated for each subframe
**
** Return:          None
**
** Note: Now this is no need to setup for real PCM reconstruction
**
*****************************************************************************/
//Void auPreGetPCM (CAudioObject* pau, U16* pcSampleDecoded,
//                  I16* pcSamplesSeekAdj,
//                  Bool fSPDIF)
//{
//    I16 iCh, j = 0;
//
//    if (pau->m_bPureLosslessMode == WMAB_TRUE)
//    {
//        prvCountAlignedPCM(pau, pcSampleDecoded, CAP_NODISCARDSILENCE, CAP_SUBFRAMELVL, fSPDIF);
//        if (pcSamplesSeekAdj != NULL)
//            *pcSamplesSeekAdj = 0;
//        return;
//    }
//
//    for (iCh = 0; iCh < pau->m_cChannel; iCh++)
//    {
//        PerChannelInfo* ppcinfo = pau->m_rgpcinfo + iCh;
//        U16 iSamplesAvail = 0;
//
//        ppcinfo->m_iCurrGetPCM_SubFrame = CURRGETPCM_INVALID;
//    }
//
//    for (iCh = 0; iCh < pau->m_cChInTile; iCh++)
//    {
//        I16 iChSrc = pau->m_rgiChInTile [iCh];
//        PerChannelInfo* ppcinfo = pau->m_rgpcinfo + iChSrc;
//
//        ppcinfo->m_iCurrGetPCM_SubFrame = (I16) ppcinfo->m_iCoefRecurQ1;
//    }
//
//    prvCountAlignedPCM(pau, pcSampleDecoded, CAP_DISCARDSILENCE,
//        CAP_SUBFRAMELVL, fSPDIF);
//
//} // auPreGetPCM



/****************************************************************************
**
** Function:        auGetPCM
**
** Description:     Generate PCM samples for output
**
** Return:          WMARESULT
**
*****************************************************************************/
WMARESULT auGetPCM(CAudioObject* pau,   // [in]  the audio structure
                   //Void* pHandle,      // [in]  the caller (encoder/decoder)
                   U16* pcSample,      // [in/out] the number of samples for output
                   short* ppbDst,        // [in/out] output buffer
                   U32 cbDstLength)    // [in] the output buffer length
{
    WMARESULT hr = WMA_OK;
    U16 *rgcSamplesREQ = NULL;
    U16 cSampleREQmax;
    I16 iCh;
    U16 iPCMRequested;
    I32         u32MaxSamplesReq;
    U16         iRetrieve;
    Int         iFrameSize;
    //U8          *pbOutput;
    PerChannelInfo* ppcinfo;
    short       i;//hxd

    // pau->m_nBytePerSample is destination bytes per sample, so this is okay.
    U32 cMaxSamplesInDst = (cbDstLength / pau->m_nBytePerSample / pau->m_cChannel);
    //CoefType *pcfPCMBuf;
    pcmType *pcfPCMBuf,*pcfPCMBuf2;

    rgcSamplesREQ = pau->m_rgcSamplesREQ;

    // ======== Limit user request to maximum aligned PCM available ========
    iPCMRequested = *pcSample;

    // ======== For each channel setup target number of output samples ========
    //for (iCh = 0; iCh < pau->m_cChannel; iCh++)
    {
        //PerChannelInfo* ppcinfo = pau->m_rgpcinfo + iCh;
        //ppcinfo->m_cSamplesLeft = iPCMRequested;

    }

    // ======== Do PCM reconstruction ========

    // ---- Check if we do have samples for output ----
    iRetrieve = (U16)(pau->m_iPCMReconEnd - pau->m_iPCMReconStart);

    u32MaxSamplesReq = min(iPCMRequested, cMaxSamplesInDst);
    u32MaxSamplesReq = min(u32MaxSamplesReq, SHRT_MAX);

    if (u32MaxSamplesReq > iRetrieve)
        u32MaxSamplesReq = iRetrieve;

    iFrameSize = pau->m_cFrameSampleHalf;
    iRetrieve = (U16)u32MaxSamplesReq;

    if (iRetrieve == 0)
    {
        *pcSample = cSampleREQmax = 0;
    }
    else
    {
        // Setup number of samples requested for each channel
        // Now start to output, we simplify it a little bit.
        // we use ppbDst[0], and do interleave by ourself
        //pbOutput = ppbDst[0];

        // first setup the buffer pointer for each channel
        for (iCh = 0; iCh < pau->m_cChannel; iCh++)
        {
            ppcinfo = pau->m_rgpcinfo + iCh;
#ifdef SATURERATE_AFTER_FFT
            ppcinfo->m_rgiPCMBuffer = (long*)((pcmType*)(pau->m_rgiCoefReconOrig + (iFrameSize * 5 / 4) * iCh ) + pau->m_iPCMReconStart);
#else
			ppcinfo->m_rgiPCMBuffer = pau->m_rgiCoefReconOrig + (iFrameSize * 3 / 2) * iCh +
									  (iFrameSize >> 1) -
									  (pau->m_cFrameSampleHalfAdjusted >> 1) +
									  pau->m_iPCMReconStart;
#endif
			while( iFrameSize!= pau->m_cFrameSampleHalfAdjusted);//add by evan wu
        }

        // Post process whatever is possible before getting pcm samples
        //if (NULL != pau->m_pfnPostProcess)
        //{
        //    TRACEWMA_EXIT(hr, pau->m_pfnPostProcess(pHandle, &iRetrieve, NULL, 0));
        //}

        // now read each channel
        //TRACEWMA_EXIT(hr, pau->aupfnReconSample(pau, (PCMSAMPLE*)pbOutput,
        //  iRetrieve));
        ppcinfo = pau->m_rgpcinfo;
        pcfPCMBuf = (pcmType*)ppcinfo->m_rgiPCMBuffer;

        ppcinfo = pau->m_rgpcinfo + 1;
        pcfPCMBuf2 = (pcmType *)ppcinfo->m_rgiPCMBuffer;
		if(1 == pau->m_cChannel)
		{
			 for (i = 0; i < iRetrieve; i++)
			 {
				pcmType data = *pcfPCMBuf++;
#ifndef SATURERATE_AFTER_FFT		
				if (data < -32768)
					data = -32768;
				else if (data > 32767)
					data = 32767;
#endif
           		*ppbDst++ =  (short)data;
				*ppbDst++ =  (short)data;
			 }
		}
		else
		{
	        for (i = 0; i < iRetrieve; i++)
	        {
#ifdef SATURERATE_AFTER_FFT
	                *ppbDst++ =  *pcfPCMBuf++;
	                *ppbDst++ =  *pcfPCMBuf2++;
#else
					long        cfPCMData;
	                cfPCMData = pcfPCMBuf[i];
					
	                if (cfPCMData < -32768)
	                   cfPCMData = -32768;
	                else if(cfPCMData > 32767)
	                   cfPCMData = 32767;
	               
	                *ppbDst++ = (short)cfPCMData;   

	                cfPCMData = pcfPCMBuf2[i];
					
	                if (cfPCMData < -32768)
	                    cfPCMData = -32768;	                
	                else if (cfPCMData > 32767)
	                    cfPCMData = 32767;	                
	               
	                *ppbDst++ = (short)cfPCMData;
#endif				
	        }
		}


        //assert( iRetrieve <= (U16)(pau->m_iPCMReconEnd - pau->m_iPCMReconStart));

        pau->m_iPCMReconStart += iRetrieve;
        *pcSample = iRetrieve;

        // ******** test if m_iPCMReconStart go beyond frame boundary ********
        if (pau->m_iPCMReconStart >= pau->m_cFrameSampleHalfAdjusted)
        {
            // ==== here we need to shift PCM buffer ====
            u32MaxSamplesReq = (pau->m_cFrameSampleHalfAdjusted >> 1);

            pau->m_iPCMReconStart -= pau->m_cFrameSampleHalfAdjusted;
            pau->m_iPCMReconEnd -= pau->m_cFrameSampleHalfAdjusted;

//			while(pau->m_iPCMReconStart != 0);//add by evan wu

            if (pau->m_iPCMReconStart < 0 || pau->m_iPCMReconStart >= pau->m_cFrameSampleHalfAdjusted / 2)
            {
                hr = WMA_E_FAIL;
                goto exit;
            }

            for (iCh = 0; iCh < pau->m_cChannel; iCh++)
            {
#ifdef SATURERATE_AFTER_FFT
				pcfPCMBuf = (pcmType*)((CoefType *)pau->m_rgiCoefReconOrig + (iFrameSize * 5 / 4) * iCh) +
							(iFrameSize >> 1) -
							u32MaxSamplesReq;
#else
                pcfPCMBuf = (pcmType*)((CoefType *)pau->m_rgiCoefReconOrig + (iFrameSize * 3 / 2) * iCh) +
                            (iFrameSize >> 1) -
                            u32MaxSamplesReq;
#endif
                /* ================================================================
                for( i = 0; i < (pau->m_cFrameSampleHalfAdjusted >> 1); i++ )
                    pcfPCMBuf[i] = pcfPCMBuf[i + pau->m_cFrameSampleHalfAdjusted];
                =================================================================== */              
                memcpy(pcfPCMBuf + pau->m_iPCMReconStart,
                       pcfPCMBuf + pau->m_cFrameSampleHalfAdjusted + pau->m_iPCMReconStart,
                       (u32MaxSamplesReq - pau->m_iPCMReconStart)*sizeof(pcmType));
            }
        }

    } // end of if( u32MaxSamplesReq != 0 )

    //while(pau->m_iPCMReconStart != 0);
    if (CODEC_BEGIN == pau->m_codecStatus)
    {
        pau->m_codecStatus = CODEC_STEADY;
    }

    //FUNCTION_PROFILE_STOP(&fp);
exit:

    return hr;
} // auGetPCM

void prvSetAdjustedValuesPC(CAudioObject *pau, PerChannelInfo *ppcinfo)
{
    //if (pau->m_fHalfTransform)
    //    {
    //        ppcinfo->m_cSubbandAdjusted = ppcinfo->m_cSubband>>pau->m_iAdjustSizeShiftFactor;
    //        ppcinfo->m_cSubFrameSampleHalfAdjusted = ppcinfo->m_cSubFrameSampleHalf>>pau->m_iAdjustSizeShiftFactor;
    //        ppcinfo->m_iSizePrev >>= pau->m_iAdjustSizeShiftFactor;
    //        ppcinfo->m_iSizeCurr >>= pau->m_iAdjustSizeShiftFactor;
    //        ppcinfo->m_iSizeNext >>= pau->m_iAdjustSizeShiftFactor;
    //    }
    //    else if (pau->m_fPad2XTransform)
    //    {
    //        ppcinfo->m_cSubbandAdjusted = ppcinfo->m_cSubband<<pau->m_iAdjustSizeShiftFactor;
    //        ppcinfo->m_cSubFrameSampleHalfAdjusted = ppcinfo->m_cSubFrameSampleHalf<<pau->m_iAdjustSizeShiftFactor;
    //        ppcinfo->m_iSizePrev <<= pau->m_iAdjustSizeShiftFactor;
    //        ppcinfo->m_iSizeCurr <<= pau->m_iAdjustSizeShiftFactor;
    //        ppcinfo->m_iSizeNext <<= pau->m_iAdjustSizeShiftFactor;
    //    }
    //    else

    {
        ppcinfo->m_cSubbandAdjusted = ppcinfo->m_cSubband;
        ppcinfo->m_cSubFrameSampleHalfAdjusted = ppcinfo->m_cSubFrameSampleHalf;
    }
}
#pragma arm section code

#endif

#ifdef WMAINITIALIZE_HIGH_MID_ALL_USED
#pragma arm section code = "WmaCommonCode"

void prvSetAdjustedValues(CAudioObject *pau, I16 cSubbandAdjusted)
{
    //if (pau->m_fHalfTransform)
    //    {
    //        pau->m_cFrameSampleAdjusted        = pau->m_cFrameSample>>pau->m_iAdjustSizeShiftFactor;
    //        pau->m_cFrameSampleHalfAdjusted    = pau->m_cFrameSampleHalf>>pau->m_iAdjustSizeShiftFactor;
    //        pau->m_cHighCutOffAdjusted         = min(pau->m_cHighCutOff, cSubbandAdjusted); // unchanged unless actual number of bands is fewer
    //    }
    //    else if (pau->m_fPad2XTransform)
    //    {
    //        pau->m_cFrameSampleAdjusted        = pau->m_cFrameSample<<pau->m_iAdjustSizeShiftFactor;
    //        pau->m_cFrameSampleHalfAdjusted    = pau->m_cFrameSampleHalf<<pau->m_iAdjustSizeShiftFactor;
    //        pau->m_cHighCutOffAdjusted         = pau->m_cHighCutOff;    // unchanged
    //    }
    //    else

    {
        pau->m_cFrameSampleAdjusted        = pau->m_cFrameSample;
        pau->m_cFrameSampleHalfAdjusted    = pau->m_cFrameSampleHalf;
        pau->m_cHighCutOffAdjusted         = pau->m_cHighCutOff;
    }
}
#pragma arm section code

#endif


//*****************************************************************************************
//
// auAdaptToSubFrameConfig
// setup paramters for handling and transisting between varying size subframes
//
//*****************************************************************************************
#if defined(WMAHIGHRATE) || defined(WMAMIDRATELOWRATE)
#pragma arm section code = "WmaHighLowCommonCode"

WMARESULT auAdaptToSubFrameConfig(CAudioObject* pau)
{
    I16 i, iTotal, iSizeCurr, iCh, iIncr;
    WMARESULT   wmaResult = WMA_OK;


    //if (pau->m_bPureLosslessMode == WMAB_TRUE)
    //    {
    //        for (iCh = 0; iCh < pau->m_cChInTile; iCh++)
    //        {
    //            I16 iChSrc = pau->m_rgiChInTile [iCh];
    //            PerChannelInfo* ppcinfo = pau->m_rgpcinfo + iChSrc;
    //            SubFrameConfigInfo* psubfrmconfigCurr = &(ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM]);
    //            assert (psubfrmconfigCurr->m_cSubFrame <= 16);
    //            ppcinfo->m_iSizeCurr = iSizeCurr = (I16) psubfrmconfigCurr->m_rgiSubFrameSize [ppcinfo->m_iCurrSubFrame];
    //            ppcinfo->m_cSubFrameSampleHalf   = iSizeCurr;
    //            // Next two function can be removed since in lossless mode, there is no adjusted values needed.
    //            // prvSetAdjustedValues(pau);
    //            // prvSetAdjustedValuesPC(pau, ppcinfo);
    //        }
    //    }
    //    else if (pau->m_bPureLosslessMode == WMAB_FALSE)

    {
        for (iCh = 0; iCh < pau->m_cChInTile; iCh++)
        {
            I16 iChSrc = pau->m_rgiChInTile [iCh];
            PerChannelInfo* ppcinfo = pau->m_rgpcinfo + iChSrc;
            SubFrameConfigInfo* psubfrmconfigCurr = &(ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM]);
            assert(psubfrmconfigCurr->m_cSubFrame <= 16);
            ppcinfo->m_iSizeCurr = iSizeCurr = (I16) psubfrmconfigCurr->m_rgiSubFrameSize [ppcinfo->m_iCurrSubFrame];
            //to decide the current window shape; look at sizes on the left and right
            ppcinfo->m_iSizePrev = (I16) psubfrmconfigCurr->m_rgiSubFrameSize [ppcinfo->m_iCurrSubFrame - 1];
            ppcinfo->m_iSizeNext = (I16) psubfrmconfigCurr->m_rgiSubFrameSize [ppcinfo->m_iCurrSubFrame + 1];

            if (psubfrmconfigCurr->m_cSubFrame > 1)
            {
                iIncr = (I16) LOG2((U32)(pau->m_cFrameSampleHalf / iSizeCurr));
                if (iIncr >= pau->m_cPossibleWinSize)
                {
                    REPORT_BITSTREAM_CORRUPTION();
                    wmaResult = TraceResult(WMA_E_BROKEN_FRAME);
                    goto exit;
                }
                i = 0;
                iTotal = 0;
                while (i < iIncr)
                {
                    iTotal += (I16)(pau->m_cFrameSampleQuad >> i);
                    i++;
                }

                pau->m_cValidBarkBand = pau->m_rgcValidBarkBand [iIncr];
                pau->m_rgiBarkIndex   = pau->m_rgiBarkIndexOrig + iIncr * (NUM_BARK_BAND + 1);
                pau->m_cSubWooferCutOffIndex = pau->m_rgcSubWooferCutOffIndex [iIncr];
            }
            else
            {
                iIncr = 0;
                pau->m_cValidBarkBand       = pau->m_rgcValidBarkBand [0];
                pau->m_rgiBarkIndex         = pau->m_rgiBarkIndexOrig;
                pau->m_cSubWooferCutOffIndex = pau->m_rgcSubWooferCutOffIndex [0];
            }
            pau->m_cLowCutOff  = iSizeCurr * pau->m_cLowCutOffLong / pau->m_cFrameSampleHalf;    //proportional
            pau->m_cHighCutOff = iSizeCurr * pau->m_cHighCutOffLong / pau->m_cFrameSampleHalf;    //proportional

            ppcinfo->m_cSubFrameSampleHalf   = iSizeCurr;

            //init; could be modified by noise sub
            ppcinfo->m_cSubbandActual = pau->m_cHighCutOff - pau->m_cLowCutOff;

#ifdef WMAMIDRATELOWRATE
            //update first noise index
            if (pau->m_fNoiseSub == WMAB_TRUE)
            {
                pau->m_iFirstNoiseIndex = (Int)(0.5F + pau->m_fltFirstNoiseFreq * ppcinfo->m_cSubFrameSampleHalf * 2
                                                / ((Float) pau->m_iSamplingRate));  //open end
                if (pau->m_iFirstNoiseIndex > ppcinfo->m_cSubband)
                    pau->m_iFirstNoiseIndex = ppcinfo->m_cSubband;

                // use precalculated values
                pau->m_iFirstNoiseBand = pau->m_rgiFirstNoiseBand[iIncr];
            }
#endif

            prvSetAdjustedValuesPC(pau, ppcinfo);
            if (iCh == 0) prvSetAdjustedValues(pau, ppcinfo->m_cSubbandAdjusted);

            prvCalcQ1Q2(pau, WMAB_TRUE, ppcinfo->m_iSizePrev, ppcinfo->m_iSizeCurr, &ppcinfo->m_iCoefRecurQ1,
                        &ppcinfo->m_iCoefRecurQ2);

            prvCalcQ3Q4(pau, WMAB_TRUE, ppcinfo->m_iSizeCurr, ppcinfo->m_iSizeNext,
                        ppcinfo->m_cSubFrameSampleHalfAdjusted, &ppcinfo->m_iCoefRecurQ3,
                        &ppcinfo->m_iCoefRecurQ4);
            ppcinfo++;
        }
        prvAdaptTrigToSubframeConfig(pau);
    }
exit:
//#ifdef PROFILE
    //FunctionProfileStop(&fp);
//#endif
    return wmaResult;
}
#pragma arm section code

#endif


//*****************************************************************************************
//
// VERIFY_DECODED_COEFFICENT
//
// define VERIFY_DECODED_COEFS and set fltCoefIntFloatThreshold
//
//*****************************************************************************************
//#define VERIFY_DECODED_COEFS
#ifdef WMAMIDRATELOWRATE
#pragma arm section code = "WmaLowRateCode"

// looks like an encoder function, but is needed by decoder.
void SetActualPower(const I32 *piCoefQ, const Int iCount,
                    PerChannelInfo *ppcinfo, const Status codecStatus)
{
    Int i;

    ppcinfo->m_iActualPower = 0;
    if (CODEC_BEGIN != codecStatus || 0 == ppcinfo->m_iPower || NULL == piCoefQ)
        return;

    for (i = 0; i < iCount; i++)
    {
        if (0 != piCoefQ[i])
        {
            ppcinfo->m_iActualPower = 1;
            break;
        }
    }
}
#pragma arm section code

#endif

#ifdef WMAHIGHRATE
#pragma arm section code = "WmaHighRateCode"

// looks like an encoder function, but is needed by decoder.
void SetActualPowerHighRate(const CBT *piCoefRecon, const int iCount,
                            PerChannelInfo *ppcinfo, const Status codecStatus)
{
    int i;

    ppcinfo->m_iActualPower = 0;
    if (CODEC_BEGIN != codecStatus || 0 == ppcinfo->m_iPower || NULL == piCoefRecon)
        return;

    for (i = 0; i < iCount; i++)
    {
        if (0 != piCoefRecon[i])
        {
            ppcinfo->m_iActualPower = 1;
            break;
        }
    }
}
#pragma arm section code

#endif

//***************************************************************************
// 24-bit Encoding
//***************************************************************************

//PCMSAMPLE prvGetSample(const PCMSAMPLE *pCurrentPos,
//                       const Int nBytePerSample,
//                       const Int nValidBitsPerSample,
//                       const Int iOffset)
//{
//    PCMSAMPLE   iResult = 0;
//    U8 *pbSrc = (U8*) pCurrentPos;
//    U8 *pbDst = (U8*) &iResult;
//    const Int iBytesPerSample = nBytePerSample;
//    const Int iSignbits = BITS_PER_BYTE * (sizeof(PCMSAMPLE) - iBytesPerSample);
//    const Int iPadBits = (8 * nBytePerSample) - nValidBitsPerSample;
//    Int i;
//
//    assert(iBytesPerSample <= sizeof(PCMSAMPLE));
//
//    // Read the sample in one byte at a time. Slow but easy to implement. We'll fix later.
//    pbSrc += iBytesPerSample * iOffset; // Skip iOffset worth of samples
//    for (i = 0; i < iBytesPerSample; i++)
//    {
//        *pbDst = *pbSrc;
//
//        // Advance pointers
//        pbSrc += 1;
//        pbDst += 1;
//    }
//
//    assert(0 == (iResult & ((1 << iPadBits) - 1))); // Verify that pad bits are zero
//
//    // Extend sign bits
//#ifndef BIG_ENDIAN
//    iResult <<= iSignbits;
//#endif
//    iResult >>= (iSignbits + iPadBits); // Dump LSB's (should all be zero)
//
//    return iResult;
//} // prvGetSample


//WMARESULT auReconSample(CAudioObject* pau, PCMSAMPLE* piOutput, U16 cSamples)
//{
//    WMARESULT hr = WMA_OK;
//    I16 i, iCh;
//    Int offset;
//    CoefType cfPCMData, *pcfPCMBuf;
//    PCMSAMPLE   iPCMData;
//
//    offset = 0;
//    for( i = 0; i < cSamples; i++ )
//    {
//
//        for( iCh = 0; iCh < pau->m_cChannel; iCh++ )
//        {
//
//            PerChannelInfo* ppcinfo = pau->m_rgpcinfo + iCh;
//
//            pcfPCMBuf = (CoefType *)ppcinfo->m_rgiPCMBuffer;
//
//            cfPCMData = pcfPCMBuf[i];
//   //DEBUG("pcfPCMBuf=0x%x,\n",&pcfPCMBuf[i]);//hxd
//
//#if defined(BUILD_INTEGER) && defined(COEF64BIT)
//            ROUND_AND_CHECK_RANGE( iPCMData, (PCMSAMPLE)cfPCMData,
//                                   PCMSAMPLE_MIN(pau->m_nValidBitsPerSample),
//                                   PCMSAMPLE_MAX(pau->m_nValidBitsPerSample) );
//#else
//            ROUND_AND_CHECK_RANGE( iPCMData, cfPCMData,
//                                   PCMSAMPLE_MIN(pau->m_nValidBitsPerSample),
//                                   PCMSAMPLE_MAX(pau->m_nValidBitsPerSample) );
//#endif
//
//            pau->m_pfnSetSample(iPCMData, piOutput, pau, offset);
//            offset++;
//        }
//    }
//
//    return hr;
//} // auReconSample

#ifdef WMAINITIALIZE
#pragma arm section code = "WmaOpenCodecCode"

PCMSAMPLE prvGetSample16(const PCMSAMPLE *pCurrentPos,
                         const Int nBytePerSample,
                         const Int nValidBitsPerSample,
                         const Int iOffset)
{
    U8 *pbSrc = (U8*) pCurrentPos;

    pbSrc += 2 * iOffset; // Skip iOffset worth of samples
    return *((I16*)pbSrc);
} // prvGetSample16



//PCMSAMPLE prvGetSample24(const PCMSAMPLE *pCurrentPos,
//                         const Int nBytePerSample,
//                         const Int nValidBitsPerSample,
//                         const Int iOffset)
//{
//    U8 *pbSrc = (U8*) pCurrentPos;
//    I32 iResult;
//
//    pbSrc += 3 * iOffset; // Skip iOffset worth of samples
//#ifdef BIG_ENDIAN
// iResult = (*(I32*)pbSrc) >> 8;
//#else
//    iResult = (*((U16*)pbSrc) | (*((I8*)(pbSrc + 2)) << 16));
//#endif
//
//    return iResult;
//} // prvGetSample24
//
//
//
//PCMSAMPLE prvGetSample2024(const PCMSAMPLE *pCurrentPos,
//                           const Int nBytePerSample,
//                           const Int nValidBitsPerSample,
//                           const Int iOffset)
//{
//    U8 *pbSrc = (U8*) pCurrentPos;
//    I32 iResult;
//
//    pbSrc += 3 * iOffset; // Skip iOffset worth of samples
//    iResult = (*((U16*)pbSrc) | (*((I8*)(pbSrc + 2)) << 16));
//
//    assert(0 == (iResult & 0x0F)); // Verify that 4 least signficant bits always zero
//
//    iResult >>= 4; // Dump the 4 least-significant bits (should always be zero)
//    return iResult;
//} // prvGetSample2024
//
//
//
//void prvSetSample(const PCMSAMPLE iValue,
//                  PCMSAMPLE *pCurrentPos,
//                  const CAudioObject *pau,
//                  const Int iOffset)
//{
//    PCMSAMPLE iNewValue;
//    U8 *pbDst = (U8*) pCurrentPos;
//    U8 *pbSrc = (U8*) &iNewValue;
//    Int i;
//    const Int iBytesPerSample = pau->m_nBytePerSample;
//    const Int iPadBits = pau->m_nBitsPerSample - pau->m_nValidBitsPerSample;
//
//#ifdef BIG_ENDIAN
//    assert(pau->m_nBytePerSample == 2); //only support 16bits sample size for Mac
//    pbSrc += 2;
//#endif
//
//    // We used to check if new value fit in valid bits: don't, because auSaveHistory
//    // can feed us values > valid bits. Expectation is that we truncate to valid bits.
//
//    // Pad to fit within container, MSB-justified (LSB's are zero)
//    iNewValue = (iValue << iPadBits);
//
//    // Write the sample one byte at a time. Slow but easy to implement. We'll fix later.
//    pbDst += iBytesPerSample * iOffset; // Skip iOffset worth of samples
//    for (i = 0; i < iBytesPerSample; i++)
//    {
//        *pbDst = *pbSrc;
//
//        // Advance pointers
//        pbSrc += 1;
//        pbDst += 1;
//    }
//} // prvSetSample


void prvSetSample16(const PCMSAMPLE iValue,
                    PCMSAMPLE *pCurrentPos,
                    const CAudioObject *pau,
                    const Int iOffset)
{
    U8 *pbDst = (U8*) pCurrentPos;
    U8 *pbSrc = (U8*) & iValue;

    pbDst += 2 * iOffset; // Skip iOffset worth of samples
#ifdef BIG_ENDIAN
    assert(pau->m_nBytePerSample == 2); //only support 16bits sample size for Mac

#    ifdef EARLIER_SWITCH
    *((I8*)(pbDst)) = *((I8*)(pbSrc + 3));
    *((I8*)(pbDst + 1)) = *((I8*)(pbSrc + 2));
#    else
    *((I16*)pbDst) = *((I16*)(pbSrc + 2));
#    endif // #ifdef EARLIER_SWITCH
#else
    *((I16*)pbDst) = *((I16*)pbSrc);
#endif // #ifdef BIG_ENDIAN
} // prvSetSample16


//void prvSetSample24(const PCMSAMPLE iValue,
//                    PCMSAMPLE *pCurrentPos,
//                    const CAudioObject *pau,
//                    const Int iOffset)
//{
//    U8 *pbDst = (U8*) pCurrentPos;
//    U8 *pbSrc = (U8*) &iValue;
//
//    pbDst += 3 * iOffset; // Skip iOffset worth of samples
//#ifdef BIG_ENDIAN
//    *((I8*)pbDst) = *((I8*)pbSrc + 1);
//    *((I8*)(pbDst + 1)) = *((I8*)(pbSrc + 2));
//    *((I8*)(pbDst + 2)) = *((I8*)(pbSrc + 3));
//#else
//    *((I16*)pbDst) = *((I16*)pbSrc);
//    *((I8*)(pbDst + 2)) = *((I8*)(pbSrc + 2));
//#endif
//} // prvSetSample24
//
//
//
//void prvSetSample2024(const PCMSAMPLE iValue,
//                      PCMSAMPLE *pCurrentPos,
//                      const CAudioObject *pau,
//                      const Int iOffset)
//{
//    I32 iNewValue;
//    U8 *pbDst = (U8*) pCurrentPos;
//    U8 *pbSrc = (U8*) &iNewValue;
//
//    // We used to check if incoming value was < 20-bit: don't, because auSaveHistory
//    // can feed us values > 20-bit. Expectation is that we truncate to 20 bits.
//    iNewValue = (iValue << 4);
//
//    pbDst += 3 * iOffset; // Skip iOffset worth of samples
//    *((I16*)pbDst) = *((I16*)pbSrc);
//    *((I8*)(pbDst + 2)) = *((I8*)(pbSrc + 2));
//} // prvSetSample2024
//
//
//
//Int prvCountSamples(const PCMSAMPLE *pCurrentPos,
//                    const PCMSAMPLE *pBasePos,
//                    const CAudioObject *pau,
//                    const Int iChannels)
//{
//    Int iTotalSamples;
//
//    iTotalSamples = (Int) ((U8*) pCurrentPos - (U8*) pBasePos);
//    assert(0 == (iTotalSamples % pau->m_nBytePerSample));
//    iTotalSamples /= pau->m_nBytePerSample;
//
//    assert(0 != iChannels);
//    assert(0 == iTotalSamples % iChannels);
//    return iTotalSamples / iChannels;
//} // prvCountSamples
#pragma arm section code
#endif
#endif
#endif


