//*@@@+++@@@@******************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//*@@@---@@@@******************************************************************
/*************************************************************************

Copyright (C) Microsoft Corporation, 1996 - 1999

Module Name:

    MsAudioDec.cpp

Abstract:

    Implementation of top level functions of CAudioObjectDecoder.

Author:

    Wei-ge Chen (wchen) 14-July-1998

Revision History:


*************************************************************************/

#include "../include/audio_main.h"
#include "..\wmaInclude\wmaudio.h"
#include "..\wmaInclude\msaudiodec.h"
#include "..\wmaInclude\macros.h"
//#include "..\wmaInclude\drccommon.h"
//#include "wmsdecfunc.h"
#include "..\wmaInclude\buffilt.h"
//#include "WMAGLOBALVARDeclare.h"


#include "..\wmaInclude\AutoProfile.h"
#include "..\wmaInclude\predefine.h"


__attribute__((section("WmaCommonCode")))
PerChannelInfo gPerChInfo[2];//20080724
__attribute__((section("WmaCommonCode")))
CAudioObjectDecoder gAudioObjDec;//20080724

#ifdef A_CORE_DECODE
#ifdef WMA_DEC_INCLUDE
#pragma arm section zidata = "WmaDecoderBss",rwdata = "WmaDecoderData"

#define NEW_DRC_DECODE



#define CCS_START(fnID)
#define CCS_END(fnID)
#define CCS_INIT()
#define CCS_CLOSE()
#define CCS_FRM_BDRY()



int WmaTableOverLay = -1;

#define audecLog(x)


#define INPUT_STATE (paudec->m_ibstrm.m_pfnGetMoreData ? audecStateDecode : audecStateInput)

//#define PRINT_EFFECTIVE_BW
//**************************************************************************
//extern functions
//***************************************************************************
extern unsigned short g_flagHighRate;
extern WMARESULT auDctIV(long* rgiCoef, long fltAfterScaleFactor,//Float fltAfterScaleFactor,
                             const Int cSubbandAdjusted, short nFacExponent);//rgiCoef is Q31

//***************************************************************************
// Local Function Prototypes
//***************************************************************************
Void auInitPcInfoDEC(CAudioObjectDecoder* paudec);
WMARESULT prvDecodeTrailerBit(CAudioObjectDecoder* paudec);
WMARESULT prvReConfig(CAudioObjectDecoder* paudec);
//void prvInitDecodeFrameHeader(CAudioObjectDecoder* paudec);
//*****************************************************************************************
PerChannelInfoDEC gRgpcInfoDec[2];
SubFrameConfigInfo gRgSubFrmCfg[2];
U8 gRgiSizeBuf[2][32*2+2+1];
//PerChannelInfo gPerChInfo[2];//20080724
U8* gRgpiRecon[2];
//*****************************************************************************************
//
//  Outline of decoding process - major function call graph (HighRate is >= 32kbps)
//  Decode
//  ..audecDecode
//  ....prvDecodePacket
//  ......prvDecodeData
//  ........auSaveHistoryMono
//  ........prvDecodeSubFrameHighRate
//  ..........prvDecodeFrameHeader
//  ............prvUpdateSubFrameConfig
//  ............auAdaptToSubFrameConfig
//  ............prvAdaptEqToSubFrame
//  ............prvSetDetTable
//  ............prvReconWeightingFactor
//  ..........qstCalcQuantStep                      ; calc sub frame's quantization step size
//  ..........prvDecodeCoefficentsHighRate<float.c> ; fills rgiCoefRecon[] from bitstream with amplitude as a function of frequency
//  ..........InverseQuantizeHighRate<float.c>
//  ..........dctIV
//  ............FFT
//  ......audecGetPCM
//  ......prvDecodeInfo
//*****************************************************************************************
//*****************************************************************************************

//Void prvDeletePcInfoDEC(CAudioObjectDecoder* paudec, PerChannelInfo* rgpcinfo)
//{
//    CAudioObject* pau = paudec->pau;
//    I16 i;
//
//    DELETE_ARRAY(paudec->m_rgpcinfoDEC);
//
//    if (pau != NULL && rgpcinfo != NULL)
//    {
//        for (i = 0; i < pau->m_cChannel; i++)
//        {
//            PerChannelInfo* ppcinfo = rgpcinfo + i;
//
//            if (ppcinfo)
//            {
//                if (ppcinfo->m_rgsubfrmconfig)
//                {
//                    DELETE_ARRAY(ppcinfo->m_rgsubfrmconfig->m_rgiSizeBuf);
//                }
//                DELETE_ARRAY(ppcinfo->m_rgsubfrmconfig);
//            }
//        }
//    }
//} // prvDeleteAllocPcInfoDEC




//************************************************************
//        prvDecDelete: Deletes members of paudec.
//                      Does not self delete.
//                      Currently always returns WMA_OK.
//************************************************************
//WMARESULT prvDecDelete (CAudioObjectDecoder* paudec)
//{
//    WMARESULT wmaResult = WMA_OK;
//    CAudioObject* pau   = NULL;
//    Int k;
//
//    if (paudec == NULL)
//        goto exit;
//
//    pau = paudec->pau;
//
////    if (pau != NULL)
////        prvDeleteChannelGrpInfo(&(paudec->m_rgChannelGrpInfo), pau->m_cChannel);
//
//
//    if (paudec->m_rgrgfltChDnMixMtx)
//    {
//        for (k = 0; k < paudec->m_cDstChannel; k++)
//        {
//            DELETE_ARRAY(paudec->m_rgrgfltChDnMixMtx[k]);
//        }
//    }
//    DELETE_ARRAY(paudec->m_rgrgfltChDnMixMtx);
//
//    if (paudec->m_rgrgfltChDnMixMtxTmp)
//    {
//        for (k = 0; k < paudec->m_cDstChannel; k++)
//        {
//            DELETE_ARRAY(paudec->m_rgrgfltChDnMixMtxTmp[k]);
//        }
//    }
//    DELETE_ARRAY(paudec->m_rgrgfltChDnMixMtxTmp);
//
//    DELETE_ARRAY(paudec->m_rgpiRecon);
//    DELETE_ARRAY(paudec->m_rgpcmsTemp);
//    DELETE_ARRAY(paudec->m_rgpctTemp);
//
//    prvDeletePcInfoDEC(paudec, paudec->m_rgpcinfo);
//    if (NULL !=pau && NULL !=paudec)
//    {
//        auDeletePcInfoCommon (pau, paudec->m_rgpcinfo);
//        paudec->m_rgpcinfo = NULL;
//    }
//
//
//    if (paudec->m_pLtRtBuf) auFree(paudec->m_pLtRtBuf);
//    if (paudec->m_pLtRtDownmix) {
//        //ltrtDownmixFree(paudec->m_pLtRtDownmix);
//        auFree(paudec->m_pLtRtDownmix);
//    }
//
//    if (paudec->m_iLastSample) auFree(paudec->m_iLastSample);
//    if (paudec->m_iPriorSample) auFree(paudec->m_iPriorSample);
//
//    DELETE_ARRAY (paudec->m_rgfltPostProcXform);
//    DELETE_ARRAY (paudec->m_rgfltPostProcXformPrev);
//    DELETE_ARRAY (paudec->m_rgfltPostProcXformBlend);
//
//
//    //oahc
//
//    if (pau != NULL) {
//        auDelete (paudec->pau);
//        paudec->pau = NULL;
//    }
//exit:
//
//
//    return wmaResult;
//} // prvDecDelete


//*****************************************************************************************
//
// audecDelete
//   free up and delete the CAudioObjectDecoder
//
//*****************************************************************************************

//Void audecDelete (void* pDecHandle)
//{
//    CAudioObjectDecoder* paudec = (CAudioObjectDecoder*)pDecHandle;
//    WMARESULT wmaResult = WMA_OK;
//
//    if (paudec == NULL)
//        return;
//
//    //auMallocSetState(MAS_DELETE, NULL, 0);
//
//    TRACEWMA_EXIT(wmaResult, prvDecDelete (paudec));
//
//exit:
//    CCS_CLOSE();
//    //auFree(paudec);
//} // audecDelete


//*********************************************************************************
//         prvWipeCleanDecoder: Brings paudec to a known-to-be-clean state by
//                              1) deleting any allocated memory inside of paudec.
//                              2) wiping clean the structure
//
//*********************************************************************************
//WMARESULT prvWipeCleanDecoder(CAudioObjectDecoder* paudec)//initilize data in audecinit()
//{
//    WMARESULT wmaResult = WMA_OK;
//    U8  cbWMAV_Version;
//
//    // Delete any allocated memory inside of paudec
//    TRACEWMA_EXIT(wmaResult, prvDecDelete(paudec));
//
//    // Wipe clean entire structure
//
//    cbWMAV_Version = paudec->m_cbWMAV_Version;
//    memset(paudec, 0, sizeof(CAudioObjectDecoder));
//    paudec->m_cbWMAV_Version = cbWMAV_Version;
//
//    // Set member values that must be non-zero.
//    // It is wise to move such initializations to audecInit.
//    // Some of the enums/defines are zeroes, but are still kept here for possible future changes.
//
//    paudec->m_fPacketLoss   = WMAB_TRUE;
//    paudec->m_bGotValidFilterInfo = WMAB_FALSE;
//    paudec->m_decsts        = BEGIN_PACKET;
//    paudec->m_subfrmdecsts  = SUBFRM_HDR;
//    paudec->m_hdrdecsts     = HDR_SIZE;
//    paudec->m_fhdrdecsts    = FHDR_PREV;
//    paudec->m_rlsts         = VLC;
//    paudec->m_fLastSubFrame = WMAB_TRUE;
//    paudec->m_iContinuousDecodeCountdown = 2;
//    paudec->m_externalState = audecStateDone;
//    paudec->m_fNeedHeader   = WMAB_TRUE;
//
//#ifdef ENABLE_EQUALIZER
//    paudec->m_fNoEq         = WMAB_TRUE;
//#endif // ENABLE_EQUALIZER
//
//exit:
//    return wmaResult;
//} // prvWipeCleanDecoder


//CAudioObjectDecoder gAudioObjDec;//20080724
//*****************************************************************************************
//
// audecNew
//   create and initialize a CAudioObjectDecoder object
//
//*****************************************************************************************
#ifdef WMAINITIALIZE
#pragma arm section code = "WmaOpenCodecCode"

void* audecNew(void *pMemBuf, const I32 iMemBufSize)
{
    CAudioObjectDecoder* paudec = NULL;
    WMARESULT wmaResult         = WMA_OK;

    if (pMemBuf != NULL && iMemBufSize < 0)
    {
        wmaResult = TraceResult(WMA_E_INVALIDARG);
        // Cant return wmaResult
        return paudec;
    }

//    auMallocSetState(MAS_ALLOCATE, pMemBuf, iMemBufSize);
    paudec = (CAudioObjectDecoder*) & gAudioObjDec;//auMalloc (sizeof (CAudioObjectDecoder));
//    if(paudec == NULL)
    //    {
    //        wmaResult = TraceResult(WMA_E_OUTOFMEMORY);
    //        // Cant return wmaResult
    //        return paudec;
    //    }


    // Wipe clean the object just allocated
    memset(paudec, 0, sizeof(CAudioObjectDecoder));

//    // Set all non-zero initializations
    //    wmaResult = prvWipeCleanDecoder(paudec);
    //
    //    if (WMA_FAILED(wmaResult)) {
    //        DELETE_ARRAY(paudec);
    //        return NULL;
    //    }
    //
    //    // WMA Voice related
    //    paudec->m_cbWMAV_Version = 0;  // disable WMA Voice
    //

    return (void*)paudec;
} // audecNew

//*****************************************************************************************
//
// prvAllocPcInfoDEC
//   Allocates memory for m_rgpcinfoDEC
//
//*****************************************************************************************


WMARESULT prvAllocPcInfoDEC(CAudioObjectDecoder* paudec, PerChannelInfo* rgpcinfo)
{
    WMARESULT wmaResult = WMA_OK;
    CAudioObject* pau = paudec->pau;
    I16 i;

    assert(paudec->m_rgpcinfoDEC == NULL);
    paudec->m_rgpcinfoDEC = (PerChannelInfoDEC*) gRgpcInfoDec;//auMalloc(sizeof(PerChannelInfoDEC) * pau->m_cChannel);
    if (paudec->m_rgpcinfoDEC == NULL)
    {
        wmaResult = WMA_E_OUTOFMEMORY;
        goto exit;
    }

    for (i = 0; i < pau->m_cChannel; i++)
    {
        PerChannelInfo* ppcinfo = rgpcinfo + i;

        ppcinfo->m_rgsubfrmconfig = (SubFrameConfigInfo*)  & gRgSubFrmCfg[i];//auMalloc (sizeof(SubFrameConfigInfo));
        if (ppcinfo->m_rgsubfrmconfig == NULL)
        {
            wmaResult = TraceResult(WMA_E_OUTOFMEMORY);
            goto exit;
        }
        memset(ppcinfo->m_rgsubfrmconfig, 0, (sizeof(SubFrameConfigInfo)));
#define SIZEBUFD ((pau->m_iMaxSubFrameDiv + 2) * sizeof (I16) + sizeof (I16) + sizeof (U8))
        ppcinfo->m_rgsubfrmconfig->m_rgiSizeBuf = (I16*)  & gRgiSizeBuf[i][0];//auMalloc (SIZEBUFD);
        if (ppcinfo->m_rgsubfrmconfig->m_rgiSizeBuf == NULL)
        {
            wmaResult = TraceResult(WMA_E_OUTOFMEMORY);
            goto exit;
        }
        memset(ppcinfo->m_rgsubfrmconfig->m_rgiSizeBuf, 0, SIZEBUFD);

        ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameSize =
            ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSizeBuf + 1; //for -1 indexing
        ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameStart =
            ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameSize + pau->m_iMaxSubFrameDiv + 1; //for + count indexing
        ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgfMaskUpdate =
            (U8*)(ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameStart + 1);
    }
exit:
    return wmaResult;
} // prvAllocPcInfoDEC

//*****************************************************************************************
//
// audecInit
//   initialize a CAudioObjectDecoder for parameters from input file or stream
//
//*****************************************************************************************
WMARESULT audecCheckInitParams(WMAFormat* pWMAFormat, PCMFormat* pPCMFormat, WMAPlayerInfo *pPlayerInfo, audecInitParams* pParams)
{
    WMARESULT wmaResult = WMA_OK;
    U16 wPlayerOpt = 0;
    U32 nIn, nOut;

    if (!pWMAFormat || !pPCMFormat)
    {
//        ASSERT (WMAB_FALSE);
        wmaResult = TraceResult(WMA_E_INVALIDARG);
        goto exit;
    }

    if (pWMAFormat->wFormatTag < 0x160 ||
        pWMAFormat->wFormatTag > 0x163)
    {
        wmaResult = TraceResult(WMA_E_NOTSUPPORTED);
        goto exit;
    }
    
    //we do not resample lossless content
    if (pWMAFormat->wFormatTag == 0x163 && pWMAFormat->nSamplesPerSec != pPCMFormat->nSamplesPerSec) {
    
        wmaResult = TraceResult(WMA_E_NOTSUPPORTED);
        goto exit;
    }

    if (pWMAFormat->wFormatTag <= 0x161)
    {
        if (pWMAFormat->nSamplesPerSec > 48000)
        {
            // V3 still does not have an upper bound on iSamplingRate
            wmaResult = TraceResult(WMA_E_NOTSUPPORTED);
            goto exit;
        }
        if (pWMAFormat->nChannels > 2)
        {
            wmaResult = TraceResult(WMA_E_NOTSUPPORTED);
            goto exit;
        }
        // Finally overcame the fear to add this
        if (pWMAFormat->nValidBitsPerSample != 16)
        {
            wmaResult = TraceResult(WMA_E_NOTSUPPORTED);
            goto exit;
        }
    }
    else
    {
        // WMA Pro, WMA Lossless
        if (pWMAFormat->nChannels > 32)
        {
            wmaResult = TraceResult(WMA_E_NOTSUPPORTED);
            goto exit;
        }
    }
    
#if !defined (BUILD_WMASTD)
    if (pWMAFormat->wFormatTag <= 0x161)
    {
        wmaResult = TraceResult(WMA_E_NOTSUPPORTED);
        goto exit;
    }
#endif // !BUILD_WMASTD

#if !defined (BUILD_WMAPRO)
    if (pWMAFormat->wFormatTag == 0x162)
    {
        wmaResult = TraceResult(WMA_E_NOTSUPPORTED);
        goto exit;
    }
#endif // !BUILD_WMAPRO

#if !defined (BUILD_WMALSL)
    if (pWMAFormat->wFormatTag == 0x163)
    {
        wmaResult = TraceResult(WMA_E_NOTSUPPORTED);
        goto exit;
    }
#endif // !BUILD_WMALSL

    if (pWMAFormat->nSamplesPerSec <= 0)
    {
      wmaResult = TraceResult(WMA_E_NOTSUPPORTED);
      goto exit;
    }
   
    if (pWMAFormat->nChannels == 0)
    {
        wmaResult = TraceResult(WMA_E_NOTSUPPORTED);
        goto exit;
    }

    if (PCMDataPCM != pPCMFormat->pcmData)
    {
        // This decoder can not produce IEEE Float output
        wmaResult = TraceResult(WMA_E_NOTSUPPORTED);
        goto exit;
    }
    
    if (pPCMFormat->nValidBitsPerSample != 16 &&
        pWMAFormat->nValidBitsPerSample != pWMAFormat->nValidBitsPerSample)
    {
        wmaResult = TraceResult(WMA_E_NOTSUPPORTED);
        goto exit;
    }

    if (pPCMFormat->cbPCMContainerSize != 2 && pPCMFormat->cbPCMContainerSize != 3) 
    {
        // We intend to support decoding into 3 bytes even for V2 & V1.
        wmaResult = TraceResult(WMA_E_NOTSUPPORTED);
        goto exit;
    }
    
    // Check the consistency of container size and player opt
    // We no longer intend to support decoding into 3 bytes even for V2 & V1.
    if (pPCMFormat->nValidBitsPerSample == 16 &&
        pPCMFormat->cbPCMContainerSize != 2)
    {
        //ASSERT (WMAB_FALSE);
        wmaResult = TraceResult(WMA_E_INVALIDARG);
        goto exit;
    }
    
    if (pWMAFormat->nValidBitsPerSample != 16 && 
        pWMAFormat->nValidBitsPerSample != 20 &&
        pWMAFormat->nValidBitsPerSample != 24 &&
        pWMAFormat->nValidBitsPerSample != 32)
    {
        wmaResult = TraceResult(WMA_E_NOTSUPPORTED);
        goto exit;
    }

    if  (pPCMFormat->cbPCMContainerSize < (unsigned)((pPCMFormat->nValidBitsPerSample + (BITS_PER_BYTE - 1))/BITS_PER_BYTE)) {
        wmaResult = TraceResult(WMA_E_NOTSUPPORTED);
        goto exit;
    }
    

    //  U32 nChannelMask
    // Ideally the number of 1's in nChannelMask should match cChannel.
    // However, we permit Channel mask to be 0 (to support 'track' coding in the future).
    // Since channel mask is mainly for presentation (not decoding), we ignore it 
    // currently.

    if (pWMAFormat->nAvgBytesPerSec & 0x80000000) {
        wmaResult = TraceResult(WMA_E_INVALIDARG);
        goto exit;
    }

    if (pWMAFormat->nBlockAlign <= 0) {
        wmaResult = TraceResult(WMA_E_INVALIDARG);
        goto exit;
    }

#define USE_JJ_FOLDDOWN
//#undef USE_JJ_FOLDDOWN // use Corona style pre-defined limited set of fold-down matrices

#if !defined (USE_JJ_FOLDDOWN)

    if (pPCMFormat->nChannels == pWMAFormat->nChannels)
    {
        if (pPCMFormat->nChannelMask != pWMAFormat->nChannelMask)
        {
            wmaResult = TraceResult(WMA_E_NOTSUPPORTED);
            goto exit;
        }
    }
    else
    {
        // We have down-mix capability for some combinations of source & sink
        if (((pPCMFormat->nChannels != 1) && (pPCMFormat->nChannels != 2)) || 
            (pWMAFormat->nChannels < 3) || 
            (pWMAFormat->nChannels > 8))
        {
            wmaResult = TraceResult(WMA_E_NOTSUPPORTED);
            goto exit;
        }
    }
#endif // !USE_JJ_FOLDDOWN

    if (((pPCMFormat->nChannels != pWMAFormat->nChannels) ||
        (pPCMFormat->nChannelMask != pWMAFormat->nChannelMask)) &&
        (pWMAFormat->wFormatTag != 0x162))
    {
        // Fold-down is not supported for WMA Std & WMA Lossless
        wmaResult = TraceResult(WMA_E_NOTSUPPORTED);
        goto exit;
    }
    

    // U16 wEncodeOpt: what can be tested? TODO
    
    if (pPlayerInfo) {
        // Do all kinds of player options tests.
        if (pPlayerInfo->nDRCSetting < 0 || 
            pPlayerInfo->nDRCSetting > 2)
        {
            wmaResult = TraceResult(WMA_E_NOTSUPPORTED);
            goto exit;
        }

        // pPlayerInfo->nPlayerOpt tests are done way deep inside during auInit

        // pPlayerInfo->rgiMixDownMatrix can be null. Currently no check on size of the 
        // array.

        // pPlayerInfo->wPeakAmplitude;

        // pPlayerInfo->wRmsAmplitude;
        wPlayerOpt = pPlayerInfo->nPlayerOpt;
    }
    
    // check resample opt
    nIn  = pWMAFormat->nSamplesPerSec;
    nOut = pPCMFormat->nSamplesPerSec;
    if (wPlayerOpt & PLAYOPT_PAD2XTRANSFORM)
    {
        nIn *= 2;
    }
    if (wPlayerOpt & PLAYOPT_HALFTRANSFORM)
    {
        nIn /= 2;
    }
    
    if (nIn != nOut)
    {
        // No resampling permitted for WMA Lossless
        if (0x163 == pWMAFormat->wFormatTag)
        {
            wmaResult = TraceResult(WMA_E_NOTSUPPORTED);
            goto exit;
        }
        if (nIn > nOut)
        {
            // down-sampling
            if (((0x162 == pWMAFormat->wFormatTag) &&
                (nIn > 2 * nOut)) ||
                (nIn > 4 * nOut))
            {
                // In WMA Pro, allow up to half sample rates due to mixed-lossless limitations
                // For WMA Std, limit to quarter sample rate for quality considerations
                wmaResult = TraceResult(WMA_E_NOTSUPPORTED);
                goto exit;
            }
        }
        else
        {
            // nIn < nOut, up-sampling
            // Upsampling not permitted for WMA Pro, as mixed-lossless doesnt yet support it
            if (0x162 == pWMAFormat->wFormatTag)
            {
                wmaResult = TraceResult(WMA_E_NOTSUPPORTED);
                goto exit;
            }
            else
            {
                // allow arbitrary ratios up to 6x, so that 8KHz content can be played 
                // in 48KHz systems. Plays ugly though.
                if (nIn * 6 < nOut)
                {
                    wmaResult = TraceResult(WMA_E_NOTSUPPORTED);
                    goto exit;
                }
            }
        }
    }

    // U32 dwUserData
    // I32 *piMemUsed: Can be NULL
    
exit:
    return wmaResult;
} // audecCheckInitParams

extern I32 getRunLevelValue(const U8 *tbl, U32 idx);
extern I32 getRunLevelValue44ssQb(const U8 *tbl, U32 idx);
extern I32 getRunLevelValue44smQb(const U8 *tbl, U32 idx);
extern I32 getRunLevelValue44ssOb(const U8 *tbl, U32 idx);
extern I32 getRunLevelValue44smOb(const U8 *tbl, U32 idx);  

WMARESULT audecInit(void* pDecHandle, WMAFormat* pWMAFormat, PCMFormat* pPCMFormat,
                    WMAPlayerInfo *pPlayerInfo, audecState *paudecState,
                    audecInitParams* pParams)
{
    CAudioObjectDecoder* paudec = (CAudioObjectDecoder*)pDecHandle;
    Float fltBitsPerSample = 0.0F;
    CAudioObject* pau = NULL;
    WMARESULT   wmaResult = WMA_OK;
//    Int k;
    Int cSubband;
    Int iVersion;
    Bool fLossless;
    U16 nPlayerOpt = 0;
    I32* rgiMixDownMatrix = NULL;
    U32 nBytePerSample;



    if (NULL == paudec ||
            NULL == paudecState)
    {
        wmaResult = TraceResult(WMA_E_INVALIDARG);
        goto exit;
    }


//    audecLog((paudec, "audecInit(0x%04X, %d, %d, %d, %d, %d, %08X, %04X, %d, %d, %d)",
    //              pWMAFormat->wFormatTag, pWMAFormat->nChannels, pWMAFormat->nSamplesPerSec,
    //              pWMAFormat->nAvgBytesPerSec, pWMAFormat->nBlockAlign,
    //              pWMAFormat->nValidBitsPerSample, pWMAFormat->nChannelMask, pWMAFormat->wEncodeOpt,
    //              pPCMFormat->nSamplesPerSec, pPCMFormat->nValidBitsPerSample, pPCMFormat->cbPCMContainerSize));


    // First, do some sanity checks on input
    TRACEWMA_EXIT(wmaResult, audecCheckInitParams(pWMAFormat, pPCMFormat, pPlayerInfo, pParams));

    // To protect from multiple initializations, bring to a known "new" state.
    // TRACEWMA_EXIT(wmaResult, prvWipeCleanDecoder(paudec));

    paudec->m_fSPDIF = /*pParams ? pParams->fSPDIF :*/ WMAB_FALSE;

    assert(pau == NULL);

    paudec->pau = pau = auNew();
    if (NULL == pau)
    {
        TRACEWMA_EXIT(wmaResult, WMA_E_OUTOFMEMORY);
        goto exit; // to keep Prefast happy
    }

    // Fill in function ptrs with decoder fns
    pau->aupfnGetNextRun = prvGetNextRunDEC;

    if (pau->m_codecStatus == CODEC_BEGIN)
        goto exit;

    fLossless = WMAB_FALSE;
    switch (pWMAFormat->wFormatTag)
    {
        case 0x160:
            iVersion = 1;
            break;
        case 0x161:
            iVersion = 2;
            break;
        case 0x162:
            iVersion = 3;
            break;
        case 0x163:
            iVersion = 3;
            fLossless = WMAB_TRUE;
            break;
        default:
            assert(!"bug in audecCheckInitParams");
            TRACEWMA_EXIT(wmaResult, WMA_E_NOTSUPPORTED);
    }

    //wchen: in the future we should use the one directly from the bitstream.
    //but for now the one in the bistream doesn't represent Sample/Frame
    //instead it represents Sample/Raw Packet that is useless for the decoder
    //other than serving the stupidity in the V4RTM decoder. We can't change the format for now.
    //but it should be changed to reprsent Sample/Frame and that should nullify the following function call.
    cSubband = msaudioGetSamplePerFrame(pWMAFormat->nSamplesPerSec, pWMAFormat->nAvgBytesPerSec * 8, pWMAFormat->nChannels, iVersion, pWMAFormat->wEncodeOpt);



    if (cSubband <= 0)
    {
        TRACEWMA_EXIT(wmaResult, WMA_E_NOTSUPPORTED);
        goto exit; // to keep Prefast happy
    }

    // shallow copy of player info (e.g. matrix not copied)
//    if (pPlayerInfo)
//        memcpy(&paudec->m_wmapi, pPlayerInfo, sizeof(paudec->m_wmapi));
//    else
//        memset(&paudec->m_wmapi, 0, sizeof(paudec->m_wmapi));

//    nPlayerOpt = paudec->m_wmapi.nPlayerOpt;
//    rgiMixDownMatrix = paudec->m_wmapi.rgiMixDownMatrix;

    // Hack
    if (paudec->m_wmapi.nDRCSetting != WMA_DRC_HIGH)
    {
        nPlayerOpt |= PLAYOPT_DYNAMICRANGECOMPR; // used by routines which see which player options are supported
        paudec->m_wmapi.nPlayerOpt |= PLAYOPT_DYNAMICRANGECOMPR;
    }

    // Set the container size that is needed by the current crop of decoders
    {
        U32 nBytePerSampleTmp = ((pWMAFormat->nValidBitsPerSample + (BITS_PER_BYTE - 1)) / BITS_PER_BYTE);
        nBytePerSample = max(pPCMFormat->cbPCMContainerSize, nBytePerSampleTmp);
    }

    // should some day pass fLossless to auInit and get rid of the lossless bit in wEncOpt
    TRACEWMA_EXIT(wmaResult, auInit(pau, iVersion, cSubband, pWMAFormat->nSamplesPerSec,
                                    pWMAFormat->nChannels, nBytePerSample, pWMAFormat->nValidBitsPerSample, pWMAFormat->nChannelMask,
                                    pWMAFormat->nAvgBytesPerSec, pWMAFormat->nBlockAlign, pWMAFormat->wEncodeOpt,
                                    pPCMFormat->nSamplesPerSec, &paudec->m_wmapi));

    //TRACEWMA_EXIT(wmaResult, prvInitDecoderSetFunctionPtrs(paudec));

    paudec->m_rgpcinfo = (PerChannelInfo*) gPerChInfo;//auMalloc (sizeof (PerChannelInfo) * pau->m_cChannel);
    if (paudec->m_rgpcinfo == NULL)
    {
        wmaResult = TraceResult(WMA_E_OUTOFMEMORY);
        goto exit;
    }
    memset(paudec->m_rgpcinfo, 0, sizeof(PerChannelInfo) * pau->m_cChannel);
    //TRACEWMA_EXIT(wmaResult, prvAllocatePcInfoCommon(pau, paudec->m_rgpcinfo));
    TRACEWMA_EXIT(wmaResult, prvAllocPcInfoDEC(paudec, paudec->m_rgpcinfo));
    TRACEWMA_EXIT(wmaResult, prvResetPcInfoCommon(pau, paudec->m_rgpcinfo));
    pau->m_rgpcinfo = paudec->m_rgpcinfo;

    TRACEWMA_EXIT(wmaResult, prvReConfig(paudec));

    if (pau->m_fUseVecCoder)
    {
        paudec->m_decRunLevelVecState.getState = TABLE_INDEX;
        //paudec->m_pfnResetEntropyCoder = prvResetVecCoder;
        //pau->aupfnGetNextRun = prvGetNextRunDECVecTableIndex;
    }

    // Handle player settings
    // Downmixing output
    paudec->m_fChannelFoldDown = WMAB_FALSE;
    paudec->m_fLtRtDownmix    = WMAB_FALSE;
    paudec->m_cDstChannel     = pPCMFormat->nChannels;
    paudec->m_nDstChannelMask = pPCMFormat->nChannelMask;

//    if ((pau->m_cChannel != paudec->m_cDstChannel) ||
    //        (pau->m_nChannelMask != paudec->m_nDstChannelMask))
    //    {
    //        if (pau->m_iVersion <= 2)
    //        {
    //            wmaResult = TraceResult(WMA_E_NOTSUPPORTED);
    //            goto exit;
    //        }
    //        paudec->m_fChannelFoldDown = WMAB_TRUE;
    //    }


//    {
    //        Int iMaxCh = max(pau->m_cChannel, paudec->m_cDstChannel);
    //        Int iMemSize = max(sizeof(PCMSAMPLE), sizeof(CoefType)) * iMaxCh;
    //        paudec->m_rgpcmsTemp = (PCMSAMPLE*) gRgpcmsTmp;//auMalloc(iMemSize);
    //        if (paudec->m_rgpcmsTemp == NULL)
    //        {
    //            wmaResult = TraceResult(WMA_E_OUTOFMEMORY);
    //            goto exit;
    //        }
    //        paudec->m_pctTemp = (CoefType*) paudec->m_rgpcmsTemp;
    //        paudec->m_rgpctTemp = (CoefType**) gRgpcTmp;//auMalloc(sizeof(CoefType*)*iMaxCh);
    //        if (NULL == paudec->m_rgpctTemp)
    //        {
    //            TRACEWMA_EXIT(wmaResult, WMA_E_OUTOFMEMORY);
    //        }
    //        paudec->m_ppcbtTemp = (CBT**) paudec->m_rgpctTemp;
    //    }


    // Requantizing to 16 bits
    paudec->m_fReQuantizeTo16   = WMAB_FALSE;
    paudec->m_nDstBytePerSample = pau->m_nBytePerSample;
    paudec->m_nDstValidBitsPerSample = pWMAFormat->nValidBitsPerSample;
    if (pWMAFormat->nValidBitsPerSample != 16 && pPCMFormat->nValidBitsPerSample == 16)
    {
        assert(pau->m_iVersion > 2); // Just to catch bugs
        paudec->m_fReQuantizeTo16 = WMAB_TRUE;
        paudec->m_nDstValidBitsPerSample = 16;
        paudec->m_nDstBytePerSample = min(2, pau->m_nBytePerSample);
        if (paudec->m_nDstBytePerSample == pau->m_nBytePerSample)   // Why extra steps?
        {
            paudec->m_fReQuantizeTo16 = WMAB_FALSE;
            paudec->m_nDstValidBitsPerSample = pWMAFormat->nValidBitsPerSample;
        }
    }

    paudec->m_iDstSamplingRate = pPCMFormat->nSamplesPerSec;
    paudec->m_fUpsample2x = WMAB_FALSE;
    paudec->m_fResample = WMAB_FALSE;
    paudec->m_fLowPass = WMAB_FALSE;

    //if (paudec->m_iDstSamplingRate == 2 * pau->m_iXformSamplingRate) {
    //    paudec->m_fUpsample2x = WMAB_TRUE;
    //}
    //else if (paudec->m_iDstSamplingRate!=pau->m_iXformSamplingRate)
    //{
    //    paudec->m_fResample = WMAB_TRUE;
    // if downsampling, should use lowpass filter
    //    if (paudec->m_iDstSamplingRate < pau->m_iXformSamplingRate)
    //        paudec->m_fLowPass = WMAB_TRUE;
    //}

    // always initialize these variables, use in audecGetPCM
    //prvInterpolateInit(paudec, pau->m_iXformSamplingRate,
    //                   paudec->m_iDstSamplingRate);
    //paudec->m_iInterpolCurPos = paudec->m_iInterpolDstBlkSize;
    // check resampling rates are reasonable...
    //if (paudec->m_iInterpolSrcBlkSize >= 10000 ||
    //    paudec->m_iInterpolDstBlkSize >= 10000) {
    //    TRACEWMA_EXIT(wmaResult, WMA_E_NOTSUPPORTED);
    //}

    // interpolation resampling done after downmixing, so use m_cDstChannel
//    if (paudec->m_fUpsample2x || paudec->m_fResample) {
    //      paudec->m_iPriorSample =
    //        (I32*)auMalloc(paudec->m_cDstChannel*sizeof(I32));
    //      if (paudec->m_iPriorSample == NULL) {
    //        TRACEWMA_EXIT(wmaResult, WMA_E_OUTOFMEMORY);
    //      }
    //      for (k = 0; k < paudec->m_cDstChannel; k++)
    //        paudec->m_iPriorSample[k] = 0;
    //      paudec->m_iLastSample =
    //        (PCMSAMPLE*)auMalloc(paudec->m_cDstChannel*sizeof(PCMSAMPLE));
    //      if (paudec->m_iLastSample == NULL) {
    //        TRACEWMA_EXIT(wmaResult, WMA_E_OUTOFMEMORY);
    //      }
    //    }


    // set postprocessing function pointer
    //pau->m_pfnPostProcess = audecPostProcessPrePCM;

    auInitPcInfoDEC(paudec);

//#ifdef WMAMIDRATELOWRATE
    //if (pWMAFormat->nChannels == 1)
    //    paudec->m_pfnDecodeCoefficient = &prvDecodeCoefficientMono;
    //else
    //    paudec->m_pfnDecodeCoefficient = &prvDecodeCoefficientStereo;
//#endif

#ifdef ENABLE_EQUALIZER
    wmaResult = audecResetEqualizer(paudec);
    TraceError(wmaResult);
#endif //ENABLE_EQUALIZER

    ibstrmInit(&paudec->m_ibstrm, paudec);


    if (pParams)
    {
        ibstrmSetGetMoreData(&paudec->m_ibstrm, pParams->pfnGetMoreData);
        ibstrmSetUserData(&paudec->m_ibstrm, pParams->dwUser);  //need to change for real streaming mode
    }


    if (pau->m_iEntropyMode == SIXTEENS_OB)
    {
#ifdef WMA_TABLE_ROOM_VERIFY
      	#ifdef NANO_C
		WmaTableOverLay = SIXTEENS_OB;
		pau->m_rgpcinfo [0].m_rgiHuffDecTbl = g_rgiHuffDecTbl16smOb;
		pau->m_rgpcinfo [0].m_rgiRunEntry   = gRun16smOb;
     		pau->m_rgpcinfo [0].m_rgiLevelEntry = gLevel16smOb;
		#else
		pau->m_rgpcinfo [0].m_rgiHuffDecTbl = (const U16 *)p_g_rgiHuffDecTbl16smOb;
        pau->m_rgpcinfo [0].m_rgiRunEntry   = (const U16 *)p_gRun16smOb;
        pau->m_rgpcinfo [0].m_rgiLevelEntry = (const U16 *)p_gLevel16smOb;
		#endif
#else
        pau->m_rgpcinfo [0].m_rgiHuffDecTbl = g_rgiHuffDecTbl16smOb;
        pau->m_rgpcinfo [0].m_rgiRunEntry   = gRun16smOb;
        pau->m_rgpcinfo [0].m_rgiLevelEntry = gLevel16smOb;
#endif
#ifdef NANO_C
		pau->m_rgpcinfo [0].m_getRunValue = getRunLevelValue;
		pau->m_rgpcinfo [0].m_getLevelValue = getRunLevelValue;
#endif	

    }
//#ifdef ENABLE_ALL_ENCOPT
    else if (pau->m_iEntropyMode == FOURTYFOURS_QB)
    {
#ifdef WMA_TABLE_ROOM_VERIFY
    	#ifdef NANO_C
		WmaTableOverLay=(FOURTYFOURS_QB);
		pau->m_rgpcinfo [0].m_rgiHuffDecTbl = g_rgiHuffDecTbl44smQb; 
        pau->m_rgpcinfo [0].m_rgiRunEntry   = p_gRun44smQb; 
        pau->m_rgpcinfo [0].m_rgiLevelEntry = p_gLevel44smQb;
		#else
		pau->m_rgpcinfo [0].m_rgiHuffDecTbl = (const U16 *)p_g_rgiHuffDecTbl44smQb; 
		pau->m_rgpcinfo [0].m_rgiRunEntry   = (const U16 *)p_gRun44smQb; 
        pau->m_rgpcinfo [0].m_rgiLevelEntry = (const U16 *)p_gLevel44smQb;
		#endif
#else
        pau->m_rgpcinfo [0].m_rgiHuffDecTbl = g_rgiHuffDecTbl44smQb;
        pau->m_rgpcinfo [0].m_rgiRunEntry   = gRun44smQb;
        pau->m_rgpcinfo [0].m_rgiLevelEntry = gLevel44smQb;
#endif
#ifdef NANO_C
		pau->m_rgpcinfo [0].m_getRunValue = getRunLevelValue44smQb;
		pau->m_rgpcinfo [0].m_getLevelValue = getRunLevelValue;
#endif	

    }
    else if (pau->m_iEntropyMode == FOURTYFOURS_OB)
    {
#ifdef WMA_TABLE_ROOM_VERIFY
     #ifdef NANO_C
		WmaTableOverLay=(FOURTYFOURS_OB);
		pau->m_rgpcinfo [0].m_rgiHuffDecTbl = (const U16 *)p_g_rgiHuffDecTbl44smOb; 
		pau->m_rgpcinfo [0].m_rgiRunEntry   = (const U8 *)p_gRun44smOb; 
        pau->m_rgpcinfo [0].m_rgiLevelEntry = (const U8 *)p_gLevel44smOb;
		#else
		pau->m_rgpcinfo [0].m_rgiHuffDecTbl = (const U16 *)p_g_rgiHuffDecTbl44smOb; 
        pau->m_rgpcinfo [0].m_rgiRunEntry   = (const U16 *)p_gRun44smOb; 
        pau->m_rgpcinfo [0].m_rgiLevelEntry = (const U16 *)p_gLevel44smOb;
		#endif
#else
        pau->m_rgpcinfo [0].m_rgiHuffDecTbl = g_rgiHuffDecTbl44smOb;
        pau->m_rgpcinfo [0].m_rgiRunEntry   = gRun44smOb;
        pau->m_rgpcinfo [0].m_rgiLevelEntry = gLevel44smOb;
#endif
	#ifdef NANO_C
		pau->m_rgpcinfo [0].m_getRunValue = getRunLevelValue;
		pau->m_rgpcinfo [0].m_getLevelValue = getRunLevelValue44smOb;
	#endif				

    }
//#endif // ENABLE_ALL_ENCOPT
    else
    {
        assert(WMAB_FALSE);
        wmaResult = TraceResult(WMA_E_INVALIDARG);
        goto exit;
    }

    pau->m_codecStatus = CODEC_BEGIN;
    pau->m_iPacketCurr = -2;                // force a Packet Loss to begin


    if (pParams) pParams->iMemBufUsed = 0;//auMallocGetCount();

//    paudec->m_cChannelGroup = 0;

    //TRACEWMA_EXIT(wmaResult,
    //    prvAllocChannelGrpInfo((CChannelGroupInfo**)&(paudec->m_rgChannelGrpInfo), pau->m_cChannel));


    // Set pointers to be used by common part
//    pau->m_rgChannelGrpInfo     = paudec->m_rgChannelGrpInfo;
    pau->m_cChannelGroup = 0;


    // V3 LLM. chao. Add.
    // we have redundant bits to indicate pure lossless mode. One in FormatTag one in EncodeOpt.
    // The problem is we first used EncodeOpt and later moved it up to FormatTag but forget to remove
    // EncodeOpt stuff. So now we accept either or both of them as pure lossles mode flag. The encoder
    // dmo should set all of them but old encapp only set EncodeOpt.
    // must check m_iVersion since V2 and V3 have overlapped Encode Options
//    if ((pau->m_iVersion == 3 && (pau->m_iEncodeOpt&ENCOPT3_PURE_LOSSLESS)) || fLossless)
    //        pau->m_bPureLosslessMode = WMAB_TRUE;
    //    else

    //pau->m_bPureLosslessMode = WMAB_FALSE;

    // We don't know it at the frame level. We have to decode into frames.
    //pau->m_bEnforcedUnifiedLLM = WMAB_FALSE;

    // First subFrm is seekable.
    pau->m_bNextSbFrmCanBeSeekable = WMAB_TRUE;



    //oahc
    paudec->m_rgpiRecon = (U8**)gRgpiRecon; //auMalloc(pau->m_cChannel * sizeof(U8*));
    if (paudec->m_rgpiRecon == NULL)
    {
        TRACEWMA_EXIT(wmaResult, WMA_E_OUTOFMEMORY);
    }
    memset(paudec->m_rgpiRecon, 0, pau->m_cChannel * sizeof(U8*));
//    auMallocSetState(MAS_LOCKED, NULL, 0);

    paudec->m_externalState = INPUT_STATE;
    paudec->m_fNeedHeader = WMAB_TRUE;
    paudec->m_ibstrm.m_fNoMoreInput = WMAB_FALSE;

    paudec->m_fNewTimeBase = WMAB_FALSE;
    paudec->m_iNewTimeBase = ((I64)1 << 63);
    paudec->m_iNewTimeBaseTemp = ((I64)1 << 63);
    paudec->m_iTtlNTBs = 0;

    if (paudecState) *paudecState = paudec->m_externalState;

//#ifdef BUILD_INTEGER
    //used for calling auPreScaleCoeffsV3()
    if (pau->m_cChannel != 1)
        pau->m_iLog2MaxIntvalCh = MAXINTVAL_CH_LOG2 - (LOG2(pau->m_cChannel - 1) + 1);
    else
        pau->m_iLog2MaxIntvalCh = MAXINTVAL_CH_LOG2;
//#endif

    CCS_INIT();

exit:


    return wmaResult;
} // audecInit


//*****************************************************************************************
//
// auInitPcInfoDEC
//
//*****************************************************************************************
Void auInitPcInfoDEC(CAudioObjectDecoder* paudec)
{
    I16 i;
    CAudioObject* pau = paudec->pau;
    I16 cSubband = (I16) pau->m_cFrameSampleHalfAdjusted;

    memset(paudec->m_rgpcinfoDEC, 0, sizeof(PerChannelInfoDEC) * pau->m_cChannel);

    for (i = 0; i < pau->m_cChannel; i++)
    {
        PerChannelInfo* ppcinfo = pau->m_rgpcinfo + i;

//      memset(ppcinfo->m_rgfltMask, 0, sizeof(ppcinfo->m_rgfltMask));
        ppcinfo->m_rgiCoefQ = NULL;
        ppcinfo->m_rgiMaskQ = pau->m_rgiMaskQ + NUM_BARK_BAND * i;
        ppcinfo->m_rgiMaskQResampled = pau->m_rgiMaskQResampled + NUM_BARK_BAND * i;
        ppcinfo->m_rgiHuffDecTbl = NULL;
        ppcinfo->m_rgiRunEntry = NULL;
        ppcinfo->m_rgiLevelEntry = NULL;
        ppcinfo->m_cSubbandActual = pau->m_cHighCutOff - pau->m_cLowCutOff;
        ppcinfo->m_iPower = 0;
        ppcinfo->m_iActualPower = 0;

        //TODO: Actually, LLM doesn't need 3/2 cSubband size.
        //ppcinfo->m_rgiWeightFactor: initialized below
#ifdef SATURERATE_AFTER_FFT
        ppcinfo->m_rgiCoefRecon = pau->m_rgiCoefReconOrig + (cSubband * 5 / 4) * i + (cSubband / 4);
        ppcinfo->dst_rgiCoefRecon = (short*)ppcinfo->m_rgiCoefRecon;        
#else
        ppcinfo->m_rgiCoefRecon = pau->m_rgiCoefReconOrig + (cSubband * 3 / 2) * i + (cSubband / 2);
#endif
        ppcinfo->m_rgfltCoefRecon = (Float *)(ppcinfo->m_rgiCoefRecon); //This will soon be removed

        ppcinfo->m_iMaxMaskQ = 0;
        ppcinfo->m_iCurrGetPCM_SubFrame = CURRGETPCM_INVALID;

        ppcinfo->m_fiSinRampUpStart = 0;
        ppcinfo->m_fiCosRampUpStart = 0;
        ppcinfo->m_fiSinRampUpPrior = 0;
        ppcinfo->m_fiCosRampUpPrior = 0;
        ppcinfo->m_fiSinRampUpStep = 0;
        ppcinfo->m_fiSinRampDownStart = 0;
        ppcinfo->m_fiCosRampDownStart = 0;
        ppcinfo->m_fiSinRampDownPrior = 0;
        ppcinfo->m_fiCosRampDownPrior = 0;
        ppcinfo->m_fiSinRampDownStep = 0;


        ppcinfo->m_iSizePrev = 0;
        ppcinfo->m_iSizeCurr = 0;
        ppcinfo->m_iSizeNext = 0;
        ppcinfo->m_iCoefRecurQ1 = 0;
        ppcinfo->m_iCoefRecurQ2 = 0;
        ppcinfo->m_iCoefRecurQ3 = 0;
        ppcinfo->m_iCoefRecurQ4 = 0;
        ppcinfo->m_iCurrSubFrame = 0;
        ppcinfo->m_cSubFrameSampleHalf = 0;

        //ppcinfo->m_rgfltWeightFactor: initialized below
        ppcinfo->m_wtMaxWeight = 0; // Currently always used because LPC not integerized at encoder
//#ifdef WMAMIDRATELOWRATE
        if (pau->m_iWeightingMode == LPC_MODE)
        { // LPC
            ppcinfo->m_rguiWeightFactor = pau->m_rguiWeightFactor +
                                          (cSubband) * i;
            ppcinfo->m_rgfltWeightFactor = (float*)(pau->m_rguiWeightFactor +
                                                    (cSubband) * i);
        }
        else
//#endif
        {
            ppcinfo->m_rguiWeightFactor = (U32*) pau->m_rgpcinfo [i].m_rgiMaskQ;
            ppcinfo->m_rgfltWeightFactor = (float*)(pau->m_rgpcinfo [i].m_rgiMaskQ);
        }

        ppcinfo->m_rgbBandNotCoded = NULL;
        ppcinfo->m_rgffltSqrtBWRatio = NULL;
        ppcinfo->m_rgiNoisePower = NULL;
//#ifdef ENABLE_ALL_ENCOPT
        //set up default pcinfo for noise sub
        ppcinfo->m_rgbBandNotCoded   = pau->m_rgbBandNotCoded   + pau->m_cValidBarkBand * i;
        ppcinfo->m_rgffltSqrtBWRatio = pau->m_rgffltSqrtBWRatio + pau->m_cValidBarkBand * i;
        ppcinfo->m_rgiNoisePower     = pau->m_rgiNoisePower     + pau->m_cValidBarkBand * i;
//#endif
        ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_cSubFrame = 1;
        ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameSize [0] = (I16) pau->m_cFrameSampleHalf;
        ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameSize [-1] = (I16) pau->m_cFrameSampleHalf;
        ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameStart [0] = 0;

        pau->m_rgpcinfo [i].ppcinfoENC = NULL;
    } // for
} // auInitPcInfoDEC
#pragma arm section code
#endif
#if defined(WMAHIGHRATE) || defined(WMAMIDRATELOWRATE)
#pragma arm section code = "WmaHighLowCommonCode"

WMARESULT audecReset(void* pDecHandle)
{
    WMARESULT wmaResult = WMA_OK;

    CAudioObjectDecoder *paudec = (CAudioObjectDecoder*)pDecHandle;
    CAudioObject* pau;
    Int iCh;

    if (paudec == NULL)
        return WMA_OK;
    pau = paudec->pau;

    if (pau == NULL)
        return WMA_OK;

    //to suppress packet loss check
    ibstrmReset(&paudec->m_ibstrm);
    ibstrmSuppressPacketLoss(&paudec->m_ibstrm);
    ibstrmSetPacketHeader(&paudec->m_ibstrm, 0);
    ibstrmSetPacketHeaderT(&paudec->m_ibstrm, 0);

    // Reset PCM reconstruction variables
    memset(pau->m_rgiPCMInHistory, 0, sizeof(*pau->m_rgiPCMInHistory) * pau->m_cChannel);

    //moved to decodeInfo
    //unlock ourself because there is an error already
    //wouldn't continue to decode the frame, making sure decodeInfo will be called
    pau->m_codecStatus = CODEC_BEGIN;

    //== to codec begin
    pau->m_iPacketCurr = -2;        //make sure we see a packet loss

    // Reset previous subframe size array
    pau->m_iPCMReconStart = 0;
    pau->m_iPCMReconEnd = 0;
    for (iCh = 0; iCh < pau->m_cChannel; iCh++)
    {
        // These pointers will be correctly assigned in encoder and decoder
        pau->m_rgpcinfo[iCh].m_iSizePrev = (I16)(pau->m_cFrameSampleHalf / 2);
        pau->m_rgpcinfo[iCh].m_iSizeCurr = pau->m_rgpcinfo[iCh].m_iSizePrev;
        pau->m_rgpcinfo[iCh].m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameSize [-1] = (I16) pau->m_cFrameSampleHalf;
        pau->m_rgpcinfo[iCh].m_iCurrCoefPosition = 0;
    }

    paudec->m_fPacketLoss = WMAB_TRUE;
    paudec->m_bGotValidFilterInfo = WMAB_FALSE;
    ibstrmSetPrevPacketNum(&paudec->m_ibstrm, -2);
    paudec->m_iCurrPresTime = 0;
    paudec->m_fContinuousDecodeAdj = WMAB_FALSE;
    paudec->m_iContinuousDecodeCountdown = 2;
    paudec->m_iSubfrmEscDataBits = 0;
    paudec->m_externalState = INPUT_STATE;
    paudec->m_fNeedHeader = WMAB_TRUE;
    paudec->m_ibstrm.m_fNoMoreInput = WMAB_FALSE;

    paudec->m_fNewTimeBase = WMAB_FALSE;
    // suppose 1 << 63 is not a valid time stamp.
    paudec->m_iNewTimeBase = ((I64)1 << 63);
    paudec->m_iNewTimeBaseTemp = ((I64)1 << 63);
    paudec->m_iTtlNTBs = 0;

    // We need to reset paudec->m_decsts since in wmadec_s DecodeInfo may termiate a seek test (reach MAX_SEEK_DECODE)
    // with paudec->m_decsts == SEEK_TO_NEXT_PACKET and make the next seek test call DecodeInfo
    // using a wrong state (SEEK_TO_NEXT_PACKET). Please check bug # 2629
    paudec->m_decsts = BEGIN_PACKET;

    // Reset operational variables
    TRACEWMA_EXIT(wmaResult, prvInitCommonResetOperational(pau));

exit:
    return wmaResult;
} // audecReset

WMARESULT audecInput(void* pDecHandle, WMA_U8* pbIn, WMA_U32 cbIn,
                     WMA_Bool  fNewPacket, WMA_Bool fNoMoreInput,
                     WMA_Bool  fTime, WMA_I64 rtTime,
                     audecState* paudecState, audecInputParams* pParams)
{
    CAudioObjectDecoder* paudec = (CAudioObjectDecoder*)pDecHandle;
    audecInputBufferInfo buf;
    WMARESULT hr = WMA_OK;

    if (paudec == NULL      ||
            paudec->pau == NULL ||
            pbIn == NULL && cbIn != 0 ||
            paudecState == NULL)
    {
        TRACEWMA_EXIT(hr, WMA_E_INVALIDARG);
        goto exit;
    }

    assert(paudec->m_externalState == audecStateInput);

    // I think this logic could serve the non-SPDIF case as well, but I am afraid to make the change
//    if (paudec->m_fSPDIF && paudec->m_fNeedHeader && !fNewPacket)
    //    {
    //        if (!paudec->m_ibstrm.m_pfnGetMoreData)
    //        {
    //            paudec->m_externalState = audecStateInput;
    //        }
    //        goto exit;
    //    }


    paudec->m_externalState = audecStateDone; // changed below on success

    memset(&buf, 0, sizeof(buf));
    buf.pbIn = pbIn;   //wma file input
    buf.cbIn = cbIn;
    buf.fTime = fTime;
    buf.rtTime = rtTime;
    buf.fNewPacket = fNewPacket;
    buf.fNoMoreInput = fNoMoreInput;
    if (pParams)
    {
        buf.cSkipBits = pParams->cSkipBits;
    }


    assert(!paudec->m_ibstrm.m_pfnGetMoreData); // mixing callback with audecInput has problems - see ibstrmReset() call in audecDecode()
    TRACEWMA_EXIT(hr, prvNewInputBuffer(paudec, &buf));//enter decode new input data

    paudec->m_externalState = audecStateDecode;

exit:
    if (paudecState && paudec)
    {
        *paudecState = paudec->m_externalState;
    }
    return hr;
}
#pragma arm section code = "WmaHighLowCommonCode"

#endif
#ifdef WMAINITIALIZE_HIGH_MID_ALL_USED
#pragma arm section code = "WmaCommonCode"

WMARESULT prvNewInputBuffer(CAudioObjectDecoder* paudec, audecInputBufferInfo* pBuf)
{
    WMARESULT hr = WMA_OK;
    WMA_U8*   pbIn = pBuf->pbIn;//input data of wma file
    WMA_U32   cbIn = pBuf->cbIn;//length of input data
    WMA_Bool  fNewPacket   = pBuf->fNewPacket;
    WMA_Bool  fNoMoreInput = pBuf->fNoMoreInput;
    WMA_Bool  fTime  = pBuf->fTime;
    WMA_I64   rtTime = pBuf->rtTime;
    WMA_I32   cSkipBits = pBuf->cSkipBits;



//    if (fNewPacket) {
    //        CCS_FRM_BDRY();
    //        if (fTime) {
    //            audecLog((paudec, "audecInput(%d) %d", cbIn, (int)(rtTime / 10000)));
    //        } else {
    //            audecLog((paudec, "audecInput(%d) no time", cbIn));
    //        }
    //    } else {
    //        audecLog((paudec, "audecInput(%d)", cbIn));
    //    }
    //    if (fNoMoreInput) {
    //        audecLog((paudec, "No more input"));
    //    }


//    if (paudec->m_fSPDIF && !paudec->m_ibstrm.m_pfnGetMoreData && !paudec->m_fNeedHeader && fNewPacket)
    //    {
    //        assert(!"previous frame never completed ??"); // unexpected
    //        audecReset(paudec); // not sure this is enough
    //        paudec->m_fNeedHeader = WMAB_TRUE;
    //    }


    if (fNewPacket && fTime)
    {
        // We may have 0 or 1 saved NTB now but definitely no 2.
        if (paudec->m_iTtlNTBs > 1)
        {   // when we see 2 NTB now, there is timestamp error.
            // But we supress it by overwriting the last NTB.
            // assert (WMAB_FALSE);
            paudec->m_iTtlNTBs--;
        }
        // Compare the rtTime with the iNewTimeBase. If they are the same, we think this rtTime
        // is an useless TimeStamp. Because it will be overwritten by another TimeStamp without
        // being processed. (when big frame crossing mutiple packets.)
        // Notice that the iNewTimeBase here may or may not have been processed. If there is
        // one saved valid NTB now, it must be NewTimeBase. Otherwise, NewTimeBase is the last
        // processed NTB.
        // One problem of this fix is it can not detect a timestamp error of a WMA file with
        // same time stamp on each packet.
        if (paudec->m_iNewTimeBase != rtTime)
        {   // We got a new (different) TimeBase. We need to save it.
            if (paudec->m_fNewTimeBase == WMAB_FALSE)
            {   // No NTB in buffer. Load the rtTime to NewTimeBase.
                paudec->m_fNewTimeBase = WMAB_TRUE;
                paudec->m_iNewTimeBase = rtTime;
                paudec->m_iTtlNTBs = 1;
            }
            else
            {   // 1 NTB in buffer. Load the rtTime to NewTimeBaseTemp.
                paudec->m_iNewTimeBaseTemp = rtTime;
                paudec->m_iTtlNTBs ++;  // == 2;
                if (paudec->m_iTtlNTBs != 2)
                {
                    assert(WMAB_FALSE);
                    // Anyway, reset it and continue.
                    paudec->m_fNewTimeBase = 0;
                    paudec->m_iTtlNTBs = 0;

                }
            }
        }
    }
    CCS_START(CCS_ibstrmAttach);
//    if (paudec->m_fSPDIF && fNewPacket)
    //    {
    //        ibstrmReset(&paudec->m_ibstrm);
    //    }
    //    else
    //    {
    //        assert(paudec->m_ibstrm.m_cbBuflen == 0);
    //        assert(paudec->m_ibstrm.m_pBuffer == paudec->m_ibstrm.m_pBufferBegin + paudec->m_ibstrm.m_cbBuflenBegin || !paudec->m_fSPDIF);
    //    }

    TRACEWMA_EXIT(hr, ibstrmAttach(&paudec->m_ibstrm,
                                   pBuf->pbIn, pBuf->cbIn,
                                   pBuf->fNewPacket, pBuf->fNoMoreInput,
                                   paudec->m_fSPDIF, paudec->pau->m_iVersion));//input bitstream decode
    CCS_END(CCS_ibstrmAttach);
    if (hr == WMA_S_LOSTPACKET)
    {
        CAudioObject* pau = paudec->pau;
        audecLog((paudec, "...LOST_PACKET"));
        pau->m_codecStatus = CODEC_HEADER;
        paudec->m_fNeedHeader = WMAB_TRUE;
        TraceResult(hr);
    }

//    if (paudec->m_fSPDIF)
    //    {
    //        if (!fNewPacket && cSkipBits)
    //        {
    //            TRACEWMA_EXIT(hr, WMA_E_INVALIDARG);
    //        }
    //
    //        if (cSkipBits)
    //        {
    //            assert(pBuf->cbIn > 0);
    //            if (cSkipBits >= 8)
    //            {
    //                TRACEWMA_EXIT(hr, WMA_E_INVALIDARG);
    //            }
    //
    //            assert(fNewPacket && (paudec->m_fNeedHeader || paudec->m_ibstrm.m_pfnGetMoreData));// cSkipBits only happens at the beginning of a frame
    //
    //            // The gap location comes from decoding the frame header, so we can't be expecting
    //            // a gap yet (frame headers are degapped in pktpare.c by the SPDIF transmitter
    //            // during depacketization).
    //            assert(!paudec->m_ibstrm.m_fDeferredGap);
    //
    //            // skip the gap
    //            ASSERTWMA_EXIT(hr, ibstrmFlushBits(&paudec->m_ibstrm, cSkipBits));
    //        }
    //    }


exit:
    return hr;
}
#pragma arm section code

#endif

//*****************************************************************************************
//
// audecDecode
//
//*****************************************************************************************
#if defined(WMAHIGHRATE) || defined(WMAMIDRATELOWRATE)
#pragma arm section code = "WmaHighLowCommonCode"

WMARESULT audecDecode(void* pDecHandle, WMA_U32* pcSamplesReady, audecState* paudecState,
                      audecDecodeParams* pParams)
{
    CAudioObjectDecoder* paudec = (CAudioObjectDecoder*)pDecHandle;
    WMARESULT hr = WMA_OK;
    CAudioObject* pau = NULL;
    U16 cSamplesReady;
//#ifdef PROFILE
    //FunctionProfile fp;
    //FunctionProfileStart(&fp,DECODE_PACKET_PROFILE);
//#endif

    //CCS_START(CCS_audecDecode);

    if (paudec == NULL      ||
            paudec->pau == NULL ||
            pcSamplesReady == NULL ||
            paudecState == NULL)
    {
        TRACEWMA_EXIT(hr, WMA_E_INVALIDARG);
        goto exit;
    }

    //audecLog((paudec, "audecDecode()"));

    pau = paudec->pau;

    if (paudec->m_externalState != audecStateDecode)
    {
        assert(!"audecDecode called at an inappropriate time");
        TRACEWMA_EXIT(hr, WMA_E_WRONGSTATE);
    }
    paudec->m_externalState = audecStateDone; // changed below on success
    *pcSamplesReady = 0;

    if (pau->m_codecStatus == CODEC_DONE)
        goto exit; //we are done don't do anything

    if (paudec->m_fNeedHeader)
    {
        I32 cIterations = 0;

        // audecNewInputBuffer() normally does this, but that's too late in callback mode.
        // But we can't reset the bstrm here in non-callback mode because ibstrmAttach()
        // has already happened.  So we do it here if and only if the callback is used.
//        if (paudec->m_fSPDIF && paudec->m_ibstrm.m_pfnGetMoreData)
        //        {
        //            ibstrmReset(&paudec->m_ibstrm);
        //        }


        do
        {
            //audecLog((paudec, "  DecodeInfo()"));
            //CCS_START(CCS_audecDecodeInfo);
            hr = prvDecodeInfo(paudec); //??????
            //CCS_END(CCS_audecDecodeInfo);
            if (hr != WMA_E_BROKEN_FRAME)
                break;

            //audecLog((paudec, "...BROKEN_FRAME"));
            audecReset(paudec);         //codecsts will be begin
            if (cIterations++ > 1000000)
            {
                assert(!"DecodeInfo() appears to be stuck");
                hr = WMA_E_ONHOLD; // force new input
                break;
            }
        }
        while (1);

        if (hr == WMA_E_ONHOLD)
        {
            assert(!paudec->m_fSPDIF); // prvDecodeInfo() does very little in SPDIF mode
            audecLog((paudec, "...ONHOLD"));
            // m_codecStatus unchanged because we still need the header
            paudec->m_externalState = (paudec->m_ibstrm.m_fNoMoreInput && !paudec->m_fSPDIF) ? audecStateDone : INPUT_STATE;
            hr = WMA_OK;
            goto exit;
        }

        CHECKWMA_EXIT(hr);
        paudec->m_fNeedHeader = WMAB_FALSE;
    }

    // If no superframes, WMA_S_NO_MORE_SRCDATA always happens at the end of
    // each frame and should constitute an exit condition. If superframes,
    // WMA_S_NO_MORE_SRCDATA should not exit this loop or we may not decode
    // last frame.
//    HEAP_DEBUG_CHECK;
    cSamplesReady = 0;
    //audecLog((paudec, "  DecodeData()"));
    //CCS_START(CCS_audecDecodeData);
    hr = prvDecodeData(paudec, &cSamplesReady, NULL); //decode audio data
    //CCS_END(CCS_audecDecodeData);
    //HEAP_DEBUG_CHECK;

    *pcSamplesReady = cSamplesReady;

    if(hr == WMA_E_INVALID_OVERLAP||hr == WMA_S_UNDOWN_ERROR)
    {
		//hr = WMA_E_INVALID_OVERLAP;
		goto exit;
    }
    else if (hr == WMA_E_BROKEN_FRAME)
    {
        audecLog((paudec, "...BROKEN_FRAME"));
        audecReset(paudec);         //codecsts will be begin
        TraceResult(hr = WMA_OK);  //map it to okay for the app
    }
    else if (hr == WMA_S_NO_MORE_FRAME)
    {
        audecLog((paudec, "...NO_MORE_FRAME"));
        paudec->m_fNeedHeader = WMAB_TRUE;
        pau->m_codecStatus = CODEC_HEADER;
//        if (paudec->m_fSPDIF && pParams)
        //        {
        //
        //            pParams->cbFrameBytesInLastBuffer = (U32)(paudec->m_ibstrm.m_pBuffer - paudec->m_ibstrm.m_pBufferBegin) - ibstrmBitsInDots(&paudec->m_ibstrm) / 8;
        //
        //        }

    }
    else if (hr == WMA_E_ONHOLD)
    {
        // m_codecStatus unchanged because we are still decoding data
        audecLog((paudec, "...ONHOLD"));
        assert(cSamplesReady == 0);
        paudec->m_externalState = (paudec->m_ibstrm.m_fNoMoreInput && !paudec->m_fSPDIF) ? audecStateDone : INPUT_STATE;
        TraceResult(hr = WMA_OK);  //map it to okay for the app
    }
    else
    {
        pau->m_codecStatus = CODEC_DATA;
    }

    CHECKWMA_EXIT(hr);

exit:

    //FUNCTION_PROFILE_STOP(&fp);
//    HEAP_DEBUG_CHECK;
    if (WMA_FAILED(hr))
    {
        audecLog((paudec, "...%08X", hr));
        if (paudec)
            paudec->m_externalState = audecStateDone;
    }

    CCS_END(CCS_audecDecode);

    if (paudec &&
            paudec->m_fSPDIF &&
            paudec->m_fNeedHeader &&
            paudec->m_externalState == audecStateDecode)
    {
        paudec->m_externalState = INPUT_STATE;
    }

    if (paudec && paudecState)
        *paudecState = paudec->m_externalState;//audecstate


    return hr;
} // audecDecode

//*****************************************************************************************
//
// prvDecodeInfo
//
//*****************************************************************************************
WMARESULT prvDecodeInfo(CAudioObjectDecoder* paudec)
{
    Int cBitLeftOver = 0;
    Int cBitLs, cBitRs;
    WMARESULT   wmaResult = WMA_OK;
    I32  nBitsFrmCnt = 0;
    CAudioObject* pau = NULL;
    I16 i;

    //if (paudec == NULL) {
    //        TRACEWMA_EXIT (wmaResult, WMA_E_INVALIDARG);
    //        goto exit;
    //    }


    pau = paudec->pau;
    //if (pau == NULL) {
    //        TRACEWMA_EXIT (wmaResult, WMA_E_INVALIDARG);
    //        goto exit;
    //    }


    //for v3, do after header decoding
    //if (pau->m_iVersion <= 2)
    {
        nBitsFrmCnt = NBITS_FRM_CNT;
//        TRACEWMA_EXIT (wmaResult, prvReConfig (paudec));
    }

    if (paudec->m_decsts == SEEK_TO_NEXT_PACKET)
        goto seekToNextPacket;

    paudec->m_decsts = BEGIN_PACKET;
    paudec->m_cFrmInPacket = 1;

    //if (paudec->m_fSPDIF)
    // {
    //   wmaResult = WMA_OK;
    //  goto exit;
    //}

    if (pau->m_fAllowSuperFrame || pau->m_iVersion > 2)
    {
        //no left over
//NEXT_HDR:
        while (ibstrmGetPacketHeader(&paudec->m_ibstrm) == 0)
        {
            CWMAInputBitStream* pibs = &paudec->m_ibstrm;

            if (ibstrmGetPacketHeaderT(&paudec->m_ibstrm) == 0)
            {
                ibstrmReset(pibs);
                //theoretically we can be on-hold but not tested as apps don't do this
                TRACEWMA_EXIT(wmaResult, ibstrmGetMoreData(&paudec->m_ibstrm, ModePktHdr, WMA_get_nHdrBits(paudec, 0)));
            }
            else
            {
                ibstrmSetPacketHeader(pibs, ibstrmGetPacketHeaderT(pibs));
                ibstrmSetPacketHeaderT(pibs, 0);
            }
        }

        //if (pau->m_iVersion <= 2)
        {
            cBitLs = NBITS_PACKET_CNT;
            cBitRs = BITS_PER_DWORD - nBitsFrmCnt;
            paudec->m_cFrmInPacket = (U16)((ibstrmGetPacketHeader(&paudec->m_ibstrm)
                                            << cBitLs) >> cBitRs);
            //assert (paudec->m_cFrmInPacket < (1 << nBitsFrmCnt));
            cBitLs = NBITS_PACKET_CNT + nBitsFrmCnt;
            cBitRs = BITS_PER_DWORD - (pau->m_cBitPackedFrameSize + 3);
            cBitLeftOver = (ibstrmGetPacketHeader(&paudec->m_ibstrm) << cBitLs) >> cBitRs;
            paudec->m_cBitSeekToNextPacket = cBitLeftOver;
            paudec->m_cBitsOverCurrPkt = cBitLeftOver;
            // TODO: when we see a cFrmInPacket == 0, should we goto the next hdr directly
            // now? (like V3). Currently, in V2, it is done later when decoder tries
            // to decode the next framehdr.


        }
        //else
        //{
        ////            U8 iVal;
        //            U32 dwHdr = ibstrmGetPacketHeader(&paudec->m_ibstrm);
        //            //get 1 bit
        //            cBitLs = NBITS_PACKET_CNT;
        //            cBitRs = BITS_PER_DWORD - NBITS_PACKET_EMPTYNESS;
        //
        //            paudec->m_cSeekableFrmInPacket = (I16) ((dwHdr << cBitLs) >> cBitRs); // 1 bit
        //            cBitLs += (NBITS_PACKET_EMPTYNESS);
        //            cBitRs = BITS_PER_DWORD - NBITS_FORCE_PACKETLOSS;
        //
        //            /* Forced Packet loss has been (and should be) taken care in ibStrmAttach function.
        //            if (((dwHdr << cBitLs) >> cBitRs) == 1) { // FORCE_PACKETLOSS
        //                // BUGBUG: I am not sure what should we do when we see a FORCE_PACKETLOSS.
        //                TRACEWMA_EXIT (wmaResult, WMA_S_LOSTPACKET);
        //                goto exit;
        //            }
        //            */
        //
        //            cBitLs += (NBITS_FORCE_PACKETLOSS);
        //            cBitRs = BITS_PER_DWORD - pau->m_cBitPackedFrameSize;
        //
        //
        //            cBitLeftOver = ((dwHdr << cBitLs) >> cBitRs);
        //
        //            paudec->m_cBitSeekToNextPacket = cBitLeftOver;
        //            paudec->m_cBitsOverCurrPkt = cBitLeftOver;
        //            cBitLs += pau->m_cBitPackedFrameSize;
        //
        //            if (paudec->m_cSeekableFrmInPacket == 1) {
        //                // Seekable Packet must have a frame starting in it and the offset obviously has a valid value.
        //                paudec->m_cFrmInPacket = 1;
        //                if (!(cBitLeftOver + cBitLs < pau->m_cBitPacketLength)) {
        //                    REPORT_BITSTREAM_CORRUPTION();
        //                    return WMA_E_BROKEN_FRAME;
        //                }
        //            }
        //            else {
        //                // (Removed : In LLM, )we use offset to check if there is frame starting in this packet.
        //                // If the offset value is not valid (overflow, point outside the packet), we know this is
        //                // a packet without a frame starting in it.
        //                if (cBitLeftOver + cBitLs >= pau->m_cBitPacketLength) {
        //                    paudec->m_cFrmInPacket = 0;
        //                }
        //                else {
        //                    paudec->m_cFrmInPacket = 1;
        //                    // After MLLMUsePLLM is added, lossy is not always seekable.
        //                    /*
        //                    // we suppose all frames are seekable in lossy.
        //                    if (!(paudec->pau->m_bPureLosslessMode == WMAB_TRUE)) {
        //                        assert(!"corrupt bitstream");
        //                        return WMA_E_BROKEN_FRAME;
        //                    }
        //                    */
        //                }
        //            }
        //
        ///*
        //            if (paudec->m_cFrmInPacket == 0) {
        //                // no frm staring in this packet; //move the queue
        //                // Means we don't have more frame in this packet to decode, we move to next packet.
        //                if (paudec->m_fPacketLoss == WMAB_TRUE)
        //                    ibstrmSetPrevPacketNum(&paudec->m_ibstrm, -2);
        //                ibstrmSetPacketHeader(&paudec->m_ibstrm, 0);
        //                goto NEXT_HDR;
        //            }
        //            else {
        //                // we see a new frame in this packet.
        //                if (paudec->m_cSeekableFrmInPacket == 0) {
        //                    // But it is not seekable.
        //                    if (paudec->m_fPacketLoss == WMAB_TRUE) {
        //                        // if we are seeking, reset status to packet loss, set packet hdr to 0 and get next packet.
        //                        ibstrmSetPrevPacketNum(&paudec->m_ibstrm, -2);
        //                        ibstrmSetPacketHeader(&paudec->m_ibstrm, 0);
        //                        goto NEXT_HDR;
        //                    }
        //                    //else:
        //                    //  If we are not seeking, we continue decoding
        //
        //                }
        //                //else:
        //                //  it is seekable we continue decoding.
        //            }
        //*/
        //            // A more clear logic:
        //            if (paudec->m_fPacketLoss == WMAB_FALSE) {
        //                // not seeking
        //                if (paudec->m_cFrmInPacket == 0) {
        //                    /*
        //                        2)  description 1) is not accurate. We have queue of two packethdrbuf
        //                            (pibs->m_dwHeaderBuf and pibs->m_dwHeaderBufTemp). When a packet is in,
        //                            its header first go to pibs->m_dwHeaderBufTemp. And the original content in pibs->m_dwHeaderBufTemp
        //                            goes to pibs->m_dwHeaderBuf. Therefore, when we see a FrmInPacket == 0 here, there are
        //                            two possibilities:
        //                            a) pibs->m_dwHeaderBuf IS the header of current packet. The current packet underflows
        //                            b) pibs->m_dwHeaderBuf is acutally the packet header of previous packet, the current
        //                               packet's header is in pibs->m_dwHeaderBufTemp. This case means that one frame uses
        //                               the whole previous packet. Underflowing of previous packet is not possible here. Since
        //                               if it was underflow, we should have decoded previous packet's header in previous DecodeInfo call.
        //                               (Each trailing bit == 0 inits a DecodeInfo call and DecodeInfo call consumes the packetheader
        //                               of current packet. But if one frame overlap multiple packets, some packets do not have chance
        //                               to trigger DecodeInfo Call. Their packet headers are pushed into the queue).
        //                        1) no more frame in packet, move to next packet
        //                    */
        //                    ibstrmSetPacketHeader(&paudec->m_ibstrm, 0);
        //                    goto NEXT_HDR;
        //                }
        //            }
        //            else if (paudec->m_fPacketLoss == WMAB_TRUE) {
        //                // seeking
        //                if (paudec->m_cSeekableFrmInPacket == 0) {
        //                    // no seekable frame in packet, set packet loss status and move to next packet.
        //                    ibstrmSetPrevPacketNum(&paudec->m_ibstrm, -2);
        //                    ibstrmSetPacketHeader(&paudec->m_ibstrm, 0);
        //                    // suppress packetloss when seeking. getmoredata will ignore packetloss signal and move forward.
        //                    ibstrmReset(&paudec->m_ibstrm);
        //                    ibstrmSuppressPacketLoss(&paudec->m_ibstrm);
        //                    goto NEXT_HDR;
        //                }
        //            }
        //
        //////
        //            //no frm staring in this packet; //move the queue
        //            if (cBitLeftOver + cBitLs >= pau->m_cBitPacketLength)
        //            {
        //                // we think this block is redundent with the if (paudec->m_cFrmInPacket == 0) block.
        //                // we think the decoder never enters this block.
        //                // we will remove it if the testing verifies our assumption.
        //                assert(WMAB_FALSE);
        //                if (paudec->m_fPacketLoss == WMAB_TRUE)
        //                    ibstrmSetPrevPacketNum(&paudec->m_ibstrm, -2);
        //                ibstrmSetPacketHeader(&paudec->m_ibstrm, 0);
        //                goto NEXT_HDR;
        //            }
        //
        //        }


        if (cBitLeftOver == 0)
            ibstrmResetPacket(&paudec->m_ibstrm);
        ibstrmSetPacketHeader(&paudec->m_ibstrm, 0);
    } // if (pau->m_fAllowSuperFrame || pau->m_iVersion > 2)
    else
    {
        Int iCurrPrevDiff;
        paudec->m_fPacketLoss = WMAB_FALSE;
        // We should only hit this on very first frame
        if (pau->m_iPacketCurr < 0)
        {
            //In callbackless mode, we've already read the packet header
            //assert(ibstrmGetPacketHeader(&paudec->m_ibstrm) == 0);
            pau->m_iPacketCurr = 0;
            paudec->m_fPacketLoss = WMAB_TRUE;
        }

        iCurrPrevDiff = ibstrmGetPacketHeader(&paudec->m_ibstrm) -
                        pau->m_iPacketCurr;

        // Non-superframe mode can't really lose packets


        // If, while processing the last frame we loaded in the start of next frame,
        // cue bitstream pointer to start of next frame
        if (iCurrPrevDiff != 0)
            ibstrmResetPacket(&paudec->m_ibstrm);

        // Advance to next payload: discard data until we hit next frame
        while (ibstrmGetPacketHeader(&paudec->m_ibstrm) == (U32)pau->m_iPacketCurr)
        {
//            U32 iBufLen;

            ibstrmReset(&paudec->m_ibstrm);
            TRACEWMA_EXIT(wmaResult, WMA_E_ONHOLD);
        }

        pau->m_iPacketCurr = ibstrmGetPacketHeader(&paudec->m_ibstrm);
    } // if (!pau->m_fAllowSuperFRame && pau->m_iVersion <= 2)

//    TRACEWMA_EXIT (wmaResult, prvReConfig (paudec));

    // Do we always have paudec->m_fNewTimeBase == TRUE here? (before this checkin)
    // FALSE means TimeStampe error?
    // If fNewTimeBase is always TRUE here, we can have a much simpler solution
    // by binding the NTB queue and the PktHdr queue. Everytime PktHdr queue moves
    // NTB queue follows it.
    // That method is simple but the current method also has advantage.
    // It can check the timestamp after decodeframe done. No need
    // to proceed to call DecodeInfo to get the correct timestamp.
//    assert(paudec->m_fNewTimeBase == WMAB_TRUE);
    if (paudec->m_fNewTimeBase == WMAB_TRUE)
    {
        if (paudec->m_iTtlNTBs == 1)
        {
            //convert 100ns to samples
            paudec->m_iCurrPresTime = paudec->m_iNewTimeBase *
                                      paudec->m_iDstSamplingRate / 10000000;
            paudec->m_fNewTimeBase = WMAB_FALSE;
            paudec->m_iTtlNTBs--;
        }
        else if (paudec->m_iTtlNTBs == 2)
        {
            paudec->m_iCurrPresTime = paudec->m_iNewTimeBase *
                                      paudec->m_iDstSamplingRate / 10000000;
            paudec->m_iNewTimeBase = paudec->m_iNewTimeBaseTemp;
            paudec->m_iTtlNTBs--;
        }
        else
        {
            // we the fix in audecInput we should not hit this assert.
            assert(WMAB_FALSE);
            // Anyway, reset.
            paudec->m_fNewTimeBase = WMAB_FALSE;
            paudec->m_iTtlNTBs = 0;
        }
        // We crossed a packet boundary. However, we have not read the frame
        // header yet. Once we've read the frame header, then we can apply
        // continuous decode adjustment.
        paudec->m_fContinuousDecodeAdj = WMAB_TRUE;
    }

    /*#if defined(SEEK_DECODE_TEST)
        if (paudec->m_pSaveRandState != NULL ) {
            *paudec->m_pSaveRandState = pau->m_tRandState;
            paudec->m_pSaveRandState = NULL;
        }
    #   endif // SEEK_DECODE_TEST
    */
    if (paudec->m_fPacketLoss == WMAB_FALSE)
    {
        wmaResult = WMA_OK;
        goto exit;
    }
    else
    {
seekToNextPacket:

        paudec->m_decsts = SEEK_TO_NEXT_PACKET;

        //packet loss or a broken frame earlier, seek to next complete frm
        //need to redecode trailer bits
        while (paudec->m_cBitSeekToNextPacket > 24)
        {
            TRACEWMA_EXIT(wmaResult, ibstrmFlushBits(&paudec->m_ibstrm, 24));
            paudec->m_cBitSeekToNextPacket -= 24;
        }
        TRACEWMA_EXIT(wmaResult, ibstrmFlushBits(&paudec->m_ibstrm, paudec->m_cBitSeekToNextPacket));

        for (i = 0; i < pau->m_cChannel; i++)
        {
            PerChannelInfo* ppcinfo = pau->m_rgpcinfo + i;
            ppcinfo->m_iCurrGetPCM_SubFrame = (I16) CURRGETPCM_INVALID;
        }

        pau->m_codecStatus = CODEC_BEGIN;
        paudec->m_decsts = BEGIN_PACKET;

        // Ignore packet loss, we're already set to deal with it
        wmaResult = WMA_OK; // WMA_BROKEN_FRAME;
        goto exit;
    }

exit:
    assert(paudec && paudec->m_cFrmInPacket >= 0);

    return wmaResult;
} // prvDecodeInfo




#define CCDUMP(x, shift)

#ifdef SATURERATE_AFTER_FFT
static void coeffSaturerate(short* dst_rgiCoefRecon,CBT* piReconBuf,int frameSize,const Int shift)
{
	CBT value;
	//short *dstpiReconBuf = (short*)piReconBuf;
	while(frameSize--)
	{
		value = *piReconBuf>>shift;
		WMA_16BITS_SATURATE(value);

		*dst_rgiCoefRecon++ = (short)value;		
		piReconBuf++;
	}
}
#endif
//*****************************************************************************************
//
// prvDecodeData
//
//*****************************************************************************************
WMARESULT prvDecodeData(CAudioObjectDecoder* paudec,
                        U16* pcSampleReady,
                        I16* pcSampleSeekAdj)
{
    CAudioObject* pau = paudec->pau;
    PerChannelInfo* ppcinfo = pau->m_rgpcinfo;
    WMARESULT hr = WMA_OK;
    I16 iCh, iChSrc;
//    U32 iBufLen;
    I16 cSampleSeekAdj;

//#ifdef PROFILE
    //FunctionProfile fp;
    //FunctionProfileStart(&fp,AUDEC_DECODE_DATA_PROFILE);
//#endif

    if (paudec == NULL || pcSampleReady == NULL)
    {
        TRACEWMA_EXIT(hr, WMA_E_INVALIDARG);
        goto exit;
    }
    // pcSampleSeekAdj can be NULL

    pau = paudec->pau;
    ppcinfo = pau->m_rgpcinfo;

    if (pau == NULL)
    {
        TRACEWMA_EXIT(hr, WMA_E_INVALIDARG);
        goto exit;
    }


    assert(pcSampleReady != NULL);
    *pcSampleReady = 0;
    if (pcSampleSeekAdj != NULL)
        *pcSampleSeekAdj = 0;
    //if we are being resumed, we need to get the new data
    while (1)
    {
//        BITRATE_REPORT_VARS;
        switch (paudec->m_decsts)//decode status:
        {
            case BEGIN_SUBFRAME :
                if (!paudec->m_fLastSubFrame)//paudec->m_fLastSubFrame为0表示子帧解码过程，为1表示子帧解码完毕
                {
                    for (iCh = 0; iCh < pau->m_cChInTile; iCh++)
                    {
                        iChSrc = pau->m_rgiChInTile [iCh];
                        ppcinfo = pau->m_rgpcinfo + iChSrc;
                        // We no used to check on the value of ppcinfo->m_iCurrGetPCM_SubFrame
                        // here, but this is no longer valid due to lazy reconstruction and
                        // history buffer in-place reconstruction.

                        // Effectively disable GetPCM in case we find broken frame or packet
                        // (disingenuous caller maps WMA_E_LOSTPACKET to WMA_OK and therefore
                        // will try to retrieve the PCM, in which case we must return nothing)
                        ppcinfo->m_iCurrGetPCM_SubFrame = CURRGETPCM_INVALID;
                    }

//                    BITRATE_REPORT_CHECKPT;
                    paudec->m_decsts = DECODE_SUBFRAME;
                    paudec->m_subfrmdecsts = SUBFRM_HDR;
                    paudec->m_hdrdecsts = HDR_SIZE;
                    paudec->m_iChannel = 0;
                    paudec->m_iBand = 0;

                }
                //else
                //    paudec->m_decsts = END_SUBFRAME2; // This assignment is redundant, and should be removed.

                // Initialize m_cLastCodedIndexV3
                for (iCh = 0; iCh < pau->m_cChInTile; iCh++)
                {
                    iChSrc = pau->m_rgiChInTile [iCh];
                    ppcinfo = pau->m_rgpcinfo + iChSrc;
                    //ppcinfo->m_cLastCodedIndexV3 = 1;
                }
                //pau->m_cLastCodedIndexV3 = 1;

                // Fall into
            case DECODE_SUBFRAME :
                {
                    if (pau->m_iWeightingMode == LPC_MODE || pau->m_fNoiseSub)//判断中低码率是根据LPC_MODE或NoiseSub来确定,而判断每帧样本数目是根据采样率来决定的
                    {
#ifdef WMAMIDRATELOWRATE
                        hr = prvDecodeSubFrame(paudec);
#endif
                    }
                    else
                    {
#ifdef WMAHIGHRATE
                        hr = prvDecodeSubFrameHighRate(paudec);
#endif
                    }
                    //hr = (*(paudec->m_pfnDecodeSubFrame))(paudec);
                    {
                        TRACEWMA_EXIT(hr, hr);
                    }
                }
//                WMAPrintf("%d %d %d\n", pau->m_iFrameNumber, pau->m_rgpcinfo->m_iCurrSubFrame, paudec->m_ibstrm.m_cFrmBitCnt);
//                FFLUSH(stdout);

                //HEAP_DEBUG_CHECK;
                //BITRATE_REPORT_PRINT;
                if (paudec->m_subfrmdecsts == SUBFRM_DONE)//表示子帧解码开始
                {
                    if (paudec->m_fLastSubFrame)  //该标记表示子帧解码结束
                    {

                        /*if(paudec->m_cbWMAV_Version>0){//wma voice version
                            paudec->m_decsts = END_SUBFRAME1;
                            break;
                        }else*/
                        {
                            paudec->m_decsts = DECODE_STUFFINGBITS;
                        }
                    }
                    else
                    {
                        paudec->m_decsts = END_SUBFRAME1;
                        break;
                    }
                }

            case DECODE_STUFFINGBITS:

                paudec->m_decsts = DECODE_TRAILERBIT;

            case DECODE_TRAILERBIT:
                //if (pau->m_iVersion <= 2)
                {
                    paudec->m_cFrmInPacket--;//该值表示一个包中有多少个帧
                }


                paudec->m_decsts = END_SUBFRAME1;

                // Fall-into
            case END_SUBFRAME1 :

                if (pcSampleSeekAdj == NULL)
                    pcSampleSeekAdj = &cSampleSeekAdj;

                // V3 Pure LLM. Problem. Chao.
                // better to make another auPreGetPCM function for Pure LLM.
                /*if (paudec->pau->m_bPureLosslessMode == WMAB_TRUE) {
                    prvCountAlignedPCM(pau, pcSampleReady, CAP_NODISCARDSILENCE, CAP_SUBFRAMELVL, paudec->m_fSPDIF);
                    if (pcSampleSeekAdj != NULL)
                        *pcSampleSeekAdj = 0;
                }else
                */
                {
                    // Change auPreGetPCM to auSubframeRecon; subframe reconstruction from coef to PCM samples
                    if(WMA_S_FALSE == auSubframeRecon(pau))//经过这个模块出来以后的数据就是32位的pcm数据
                    {
						hr = TraceResult(WMA_E_INVALID_OVERLAP);
						goto exit;
                    }
                }

                paudec->m_iCurrTile++;
                for (iCh = 0; iCh < pau->m_cChInTile; iCh++)
                {
                    iChSrc = pau->m_rgiChInTile [iCh];
                    ppcinfo = pau->m_rgpcinfo + iChSrc;
                    if (ppcinfo->m_iCurrSubFrame >= ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_cSubFrame)
                    {
                        // Somewhere we messed up in this frame for this channel
                        hr = TraceResult(WMA_E_BROKEN_FRAME);
                        goto exit;
                    }
                    ppcinfo->m_iCurrSubFrame++;
                }


            case END_SUBFRAME2 :
                // Test if we need discard silence
                //if (paudec->pau->m_bPureLosslessMode == WMAB_FALSE) {
                if (CODEC_BEGIN == pau->m_codecStatus)
                {
                    if (paudec->m_cbWMAV_Version > 0)
                        paudec->pau->m_fLeadingSilence = WMAB_TRUE;
                    prvInitDiscardSilence(pau, paudec->m_fSPDIF);
                }
                //}

                /*#ifdef PRINT_SUBFRAME_SEGMENTS
                                prvPrintSubframeSegments(paudec);
                #endif*/

                if (paudec->m_fLastSubFrame)
                {
                    // check bit count
//                    DEBUG_ONLY(assert(!pau->m_fWriteFrameSize || paudec->m_fSPDIF ||
//                                      pau->m_cComprFrameSizeBits == pau->m_cBitPacketLength ||
//                                      pau->m_cComprFrameSizeBits == paudec->m_ibstrm.m_cFrmBitCnt));

                    //#if defined(DEBUG_ENCODE_FOR_XBOX) && defined(_DEBUG)
                    //    fprintf(stdout, "Compressed Frame %d size %d bits\n", paudec->pau->m_iFrameNumber, paudec->m_ibstrm.m_cFrmBitCnt);
                    //#endif

                    // ====== Now it is the end of a frame ======
                    // ====== NEW PCM RECON! Only allow PCM output at this time ======
                    //if (paudec->pau->m_bPureLosslessMode == WMAB_FALSE)
                    {
                        if (pau->m_iPCMReconEnd > pau->m_iPCMReconStart)
                        {
                            // we still have samples in buffer, output them first
                            *pcSampleReady = (U16)(pau->m_iPCMReconEnd - pau->m_iPCMReconStart);
                            break;
                        }
                        else
                        {
                            Int iMinSizeOutput;   // Min output size for all the channels

                            // the m_iPCMReconStart must wrap around first
                            /*#ifdef SAVE_PCMBUFFER_IN_PLLM
                                                        assert( pau->m_iPCMReconStart <= pau->m_cFrameSampleHalfAdjusted / 2);
                                                        if( pau->m_iPCMReconStart < 0 ||
                                                            pau->m_iPCMReconStart > pau->m_cFrameSampleHalfAdjusted / 2 )
                                                        {
                                                            hr = WMA_E_FAIL;
                                                            goto exit;
                                                        }
                            #else*/
                            assert(pau->m_iPCMReconStart < pau->m_cFrameSampleHalfAdjusted / 2);
                            if (pau->m_iPCMReconStart < 0 ||
                                    pau->m_iPCMReconStart >= pau->m_cFrameSampleHalfAdjusted / 2)
                            {
                                hr = WMA_E_FAIL;
                                goto exit;
                            }

//#endif //SAVE_PCMBUFFER_IN_PLLM

                            // ---- Compute how many samples we have in the buffer ----
                            //if( paudec->m_fSPDIF )  // Tian SPDIF
                            //                            {
                            //                                iMinSizeOutput = pau->m_cFrameSampleHalfAdjusted - pau->m_iPCMReconStart;
                            //                            }
                            //                            else

                            {
                                prvGetFramePCM(pau, &iMinSizeOutput);  //计算送出的pcm数据的数目
                            }

                            // iMinSizeOutput is the number of sample available, will be feed to *pcSampleReady
                            pau->m_iPCMReconEnd = iMinSizeOutput + pau->m_iPCMReconStart;//compute PCMRcondEnc

                            // ---- Discard silence ----
                            if (pau->m_rgiDiscardSilence[0] > 0 && !paudec->m_fSPDIF)
                            {

                                // we need to discard silence samples
                                if (pau->m_rgiDiscardSilence[0] >= iMinSizeOutput)
                                {

                                    pau->m_rgiDiscardSilence[0] -= iMinSizeOutput;
                                    pau->m_iPCMReconStart = pau->m_iPCMReconEnd;

                                }
                                else
                                {

                                    pau->m_iPCMReconStart += pau->m_rgiDiscardSilence[0];
                                    pau->m_rgiDiscardSilence[0] = 0;

                                }

                                // **** here we need to shift PCM buffer like in auGetPCM
                                prvShiftPCMBuffer(pau);

                            }

                            // ---- Discard trailing samples ----


                            // ---- Setup the number of output samples
                            if (pau->m_iPCMReconEnd < pau->m_iPCMReconStart)
                            {
                                REPORT_BITSTREAM_CORRUPTION();
                                hr = WMA_E_BROKEN_FRAME;
                                TraceResult(hr);
                                goto exit;
                            }
                            /* 计算出送出的pcm样本的数目等于pcSamplesReady */
                            *pcSampleReady = (U16)(pau->m_iPCMReconEnd - pau->m_iPCMReconStart);//pau->m_iPCMReconEnd = iMinSizeOutput + pau->m_iPCMReconStart;

                            // ---- adjust the timestamp if needed ----
                            if (pau->m_fSeekAdjustment)
                            {

                                if (pcSampleSeekAdj == NULL)
                                    pcSampleSeekAdj = &cSampleSeekAdj;
                                *pcSampleSeekAdj = 0;

                                if (pau->m_iVersion <= 2)
                                {
                                    // we need to adjust Time-stamp for seeking
                                    // for v2 and earlier, there is a risk
                                    //       to do the adjustment if !m_fAllowSuperFrame
                                    //       since the fStartSteam can't be detected.
                                    //       Now we are always doing it at the expense of
                                    //       screwing the begining of the file where it should not be done.
                                    I16  iSizePrev, iSizeCurr, Q1, Q2;

                                    iSizePrev = (I16)pau->m_rgpcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameSize[-1];
                                    iSizeCurr = (I16)pau->m_rgpcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameSize [0];

                                    //if (pau->m_fHalfTransform) {
                                    //                                        iSizeCurr >>= pau->m_iAdjustSizeShiftFactor;
                                    //                                        iSizePrev >>= pau->m_iAdjustSizeShiftFactor;
                                    //                                    }else if (pau->m_fPad2XTransform) {
                                    //                                        iSizeCurr <<= pau->m_iAdjustSizeShiftFactor;
                                    //                                        iSizePrev <<= pau->m_iAdjustSizeShiftFactor;
                                    //                                    }


                                    prvCalcQ1Q2(pau, WMAB_TRUE, iSizePrev, iSizeCurr, &Q1, &Q2);

                                    *pcSampleSeekAdj += (I16)(Q2 - Q1);
                                }
                                //else
                                //                                {
                                //                                    // V3 or above, we are sending the correct Time-Stamp for seeking
                                //                                    // There is no timestamp issue for SPDIF
                                //                                }


                                pau->m_fSeekAdjustment = WMAB_FALSE;

                                if (pcSampleSeekAdj)
                                    paudec->m_iCurrPresTime += *pcSampleSeekAdj;

                            } // fSeekAdjustment
                        } // pau->m_cGetPCMSamplesDone == 0
                    } // lossy mode

                    //take care the end of a frame
                    if (pau->m_fAllowSuperFrame)
                    {
                        //copy next subfrm config to curr
                        for (iCh = 0; iCh < pau->m_cChannel; iCh++)
                        {
                            SubFrameConfigInfo* psubfrmconfig = (pau->m_rgpcinfo + iCh)->m_rgsubfrmconfig + CONFIG_CURRFRM;
                            I32                     cSizeCurr = psubfrmconfig->m_cSubFrame;
                            if (pau->m_iVersion <= 2)
                            {
                                psubfrmconfig->m_rgiSubFrameSize[-1] = psubfrmconfig->m_rgiSubFrameSize[cSizeCurr-1];
                                psubfrmconfig->m_rgiSubFrameSize[0]  = psubfrmconfig->m_rgiSubFrameSize[cSizeCurr];
                                psubfrmconfig->m_rgiSubFrameStart[0] = 0;
                                psubfrmconfig->m_cSubFrame = 1;
                            }
//                            else
                            //                            {
                            //                                psubfrmconfig->m_rgiSubFrameSize[-1] = psubfrmconfig->m_rgiSubFrameSize[cSizeCurr-1];
                            //                            }

                        }
                    }

                    //WMAPrintf("LL%d %d %d\n", pau->m_iFrameNumber, pau->m_rgpcinfo->m_iCurrSubFrame, paudec->m_ibstrm.m_cFrmBitCnt);
                    //FFLUSH(stdout);

                    //start decoding the next frame
                    paudec->m_decsts = BEGIN_FRAME;
                    paudec->m_fhdrdecsts = FHDR_SIZE;

                    if (paudec->m_cFrmInPacket <= 0)
                    {
                        hr = WMA_S_NO_MORE_FRAME;
                        goto exit;
                    }
                }
                else
                    paudec->m_decsts = BEGIN_SUBFRAME; //goto the start of next subfrm
                goto exit;

            case BEGIN_PACKET:
                assert(paudec->pau->m_fAllowSuperFrame || (paudec->m_cFrmInPacket == 1));
                paudec->m_decsts = BEGIN_FRAME;
                paudec->m_fhdrdecsts = FHDR_PREV;

                // Countdown to first continous decode adjustment
                if (paudec->m_iContinuousDecodeCountdown > 0)
                    paudec->m_iContinuousDecodeCountdown -= 1;

                //prvInitDecodeFrameHeader(paudec);
                break;

            case BEGIN_FRAME:
                //WMAPrintf("Frame %d\n", paudec->pau->m_iFrameNumber);
//#ifdef WMA_ENTROPY_TEST
//                pauInitModelsEntropyTest(pau, NULL, g_maskTest);
//#endif
                paudec->pau->m_iFrameNumber++;
                paudec->m_fLastSubFrame = WMAB_FALSE;
                paudec->m_iCurrTile = 0;
                pau->m_u32TrailingSize = 0;
                paudec->m_ibstrm.m_cFrmBitCnt = 0;

                //if (paudec->pau->m_bPureLosslessMode == WMAB_TRUE)
                //{
                // init for getPCM. (Pure Lossless Mode)
                //   pau->m_cGetPCMSamplesDone = 0;
                //}
                //else
                {
                    // NEW PCM RECON: here we wait for PCM output
                    // until the buffer pointer wrap around
                    if (pau->m_iPCMReconEnd > pau->m_iPCMReconStart)  //判定pcSampleReady值是否更新，主要看buffer指针回绕到起点
                    {
                        *pcSampleReady = (U16)(pau->m_iPCMReconEnd - pau->m_iPCMReconStart);
                        break;
                    }
//                    assert ( pau->m_iPCMReconStart < pau->m_cFrameSampleHalfAdjusted / 2) {
//#ifdef SAVE_PCMBUFFER_IN_PLLM
//                    if ( pau->m_iPCMReconStart > pau->m_cFrameSampleHalfAdjusted / 2) {
//#else
                    if (pau->m_iPCMReconStart >= pau->m_cFrameSampleHalfAdjusted / 2)
                    {
//#endif //SAVE_PCMBUFFER_IN_PLLM
                        pau->m_iPCMReconEnd = 0;
                        pau->m_iPCMReconStart = 0;
                    }
                }

                for (iCh = 0; iCh < pau->m_cChannel; iCh++)
                {
                    ppcinfo = pau->m_rgpcinfo + iCh;
                    ppcinfo->m_iCurrSubFrame = 0;
                    //TODOCHAO: we'd better to check the previous frame output the correct PCM samples.
                    // set iPCMInHistory as 0 in PLLM.
                    //if (pau->m_bPureLosslessMode == WMAB_TRUE)
                    //    pau->m_rgiPCMInHistory[iCh] = 0;
                }
                paudec->m_decsts = FRAME_HDR;
                break;

            case FRAME_HDR:
                if (paudec->m_cFrmInPacket == 0)
                {
                    hr = WMA_S_NO_MORE_FRAME;
                    goto exit;
                }
                TRACEWMA_EXIT(hr, prvDecodeFrameHeader(paudec));
                //WMAPrintf("FH%d %d %d\n", pau->m_iFrameNumber, pau->m_rgpcinfo->m_iCurrSubFrame, paudec->m_ibstrm.m_cFrmBitCnt);
                //FFLUSH(stdout);

                // ======== Setup the coef buffer pointer to the beginning of the buffer ========
                for (iCh = 0; iCh < pau->m_cChannel; iCh++)
                {
                    ppcinfo = pau->m_rgpcinfo + iCh;
                    ppcinfo->m_iCurrCoefPosition = 0;
                }

                paudec->m_decsts = BEGIN_SUBFRAME;
                break;
        }
    }

exit:
    if (pcSampleReady != NULL && *pcSampleReady > 0)//pcSampleReady 控制着 audecstategetpcm的赋值操作
    {
        paudec->m_externalState = audecStateGetPCM;//只要进入这个操作，就说明解完的pcm数据可以送出去写入文件
        paudec->m_cSamplesReady = *pcSampleReady;
        audecLog((paudec, "...%d samples ready", paudec->m_cSamplesReady));
    }
    else
    {
        paudec->m_externalState = audecStateDecode;
    }

    //FUNCTION_PROFILE_STOP(&fp);

//    HEAP_DEBUG_CHECK;

    return hr;
} // prvDecodeData

// Function GCD() is already defined in rsa32.lib (rsa_math.obj) for the CE build
// on ARM platforms.

// Stolen from Algorithms in C, Sedgewick
//static Int  GCD (Int u, Int v)
//{
//    Int t;
//
//    while (u > 0)
//    {
//        if (u < v)
//        {
//            // Swap 'em
//            t = u;
//            u = v;
//            v = t;
//        }
//        u = u - v;
//    }
//
//    return v;
//}
//
//
//void prvInterpolateInit(CAudioObjectDecoder *paudec, Int iXformSampleRate,
//                        Int iDstSampleRate)
//{
//    Int iGCD;
//
//    paudec->m_iInterpolSrcBlkSize = iXformSampleRate;
//    paudec->m_iInterpolDstBlkSize = iDstSampleRate;
//
//    // Calculate the greatest common divisor between iXformSampleRate
//    // and iDstSampleRate and use to reduce iSrcBlkSize, iDstBlkSize
//    iGCD = GCD(iXformSampleRate, iDstSampleRate);
//    if (0 != iGCD)
//    {
//        paudec->m_iInterpolSrcBlkSize /= iGCD;
//        paudec->m_iInterpolDstBlkSize /= iGCD;
//    }
//} // prvInterpolateInit
//
//void auLowPass2(CAudioObjectDecoder *paudec, CoefType *rgCoef, Int iNumCoefs)
//{
//    CoefType    *pEnd = rgCoef + iNumCoefs * 2; // Re and Im coefs (so times 2)
//    CoefType    *pCurr;
//    Int     iPassThruCoefs;
//
//    //return;
//
//    // Figure out how many coefficients will pass through untouched
//    assert(paudec->m_iInterpolDstBlkSize <= 10000);
//    assert(iNumCoefs <= 10000);
//    iPassThruCoefs = paudec->m_iInterpolDstBlkSize * iNumCoefs /
//      paudec->m_iInterpolSrcBlkSize;
//    pCurr = rgCoef + (2 * iPassThruCoefs);   // Re and Im coefs (so times 2)
//    iNumCoefs -= iPassThruCoefs;
//
//    if (iNumCoefs <= 0) return;
//
//    while (iNumCoefs > 0)
//    {
//        *pCurr++ = 0;       // Re coef
//        *pCurr++ = 0;       // Im coef
//        iNumCoefs -= 1;
//    }
//
//    assert(pCurr == pEnd);
//} // auLowPass2


// ===========================


//WMARESULT prvInterpolate2X(CAudioObjectDecoder *paudec,
//                           U8 *pbBuf, U16 *pcSample, Int cbDstLength)
//{
//    // work from back of buffer to front so we do not overwrite any
//    // values we still need to use
//    Int iChannel;
//    Int iSamplesPerChannel = *pcSample;
//    PCMSAMPLE *piInput, *piOutput, *piFirst, *piSecond;
//    PCMSAMPLE iNextInput, iPrevInput, iCurOutput, iLastInput;
//    PCMSAMPLE *piBuf = (PCMSAMPLE*)pbBuf;
//    CAudioObject *pau = paudec->pau;
//    const Int nCh = paudec->m_cDstChannel;
//
//    if ((((cbDstLength/pau->m_nBytePerSample)/nCh)/2) < *pcSample)  // precheck
//    {
//        assert(WMAB_FALSE);
//        return WMA_E_INVALIDARG;
//    }
//
//    for (iChannel = 0; iChannel < nCh; iChannel++)
//    {
//        piInput =
//            PAUPRVADVANCEPCMPTR(piBuf, (iSamplesPerChannel-1)*nCh + iChannel);
//        piOutput =
//            PAUPRVADVANCEPCMPTR(piBuf, (2*iSamplesPerChannel-1)*nCh + iChannel);
//
//        iNextInput = PAUPFNGETSAMPLE0(piInput);
//        iLastInput = iNextInput;
//        piFirst = PAUPRVADVANCEPCMPTR(piBuf, iChannel);
//        piSecond = PAUPRVADVANCEPCMPTR(piBuf, nCh + iChannel);
//        while (piOutput > piSecond)
//        {
//             // 1 ahead of in
//            assert(piOutput >= PAUPRVADVANCEPCMPTR(piInput, nCh));
//
//            PAUPFNSETSAMPLE0(iNextInput, piOutput);
//
//            piOutput = PAUPRVADVANCEPCMPTR(piOutput, -nCh);
//            assert(piOutput >= piSecond); // At least at second sample
//            piInput = PAUPRVADVANCEPCMPTR(piInput, -nCh);
//            assert(piInput >= piFirst); // At least at first sample
//
//            iPrevInput = iNextInput;
//            iNextInput = PAUPFNGETSAMPLE0(piInput);
//
//            iCurOutput = (iPrevInput>>1) + (iNextInput>>1);
//            assert(fabs((double)iCurOutput-
//                        ((double)iPrevInput+(double)iNextInput)/2.0) <= 1.0);
//
//            PAUPFNSETSAMPLE0(iCurOutput, piOutput);
//
//            piOutput = PAUPRVADVANCEPCMPTR(piOutput, -nCh);
//            assert(piOutput >= piSecond);
//        }
//
//        assert(piInput == piFirst);
//        assert(piOutput == piSecond);
//        PAUPFNSETSAMPLE0(iNextInput, piOutput);
//        piOutput = PAUPRVADVANCEPCMPTR(piOutput, -nCh);
//        assert(piOutput == piFirst);
//
//        iPrevInput = iNextInput;
//        iNextInput = paudec->m_iPriorSample[iChannel];
//
//        iCurOutput = (iPrevInput>>1) + (iNextInput>>1);
//        assert(fabs((double)iCurOutput-
//                    ((double)iPrevInput+(double)iNextInput)/2.0) <= 1.0);
//
//        PAUPFNSETSAMPLE0(iCurOutput, piOutput);
//
//        paudec->m_iPriorSample[iChannel] = iLastInput;
//    }
//
//    *pcSample *= 2;
//
//    return WMA_OK;
//} // prvInterpolate2X


//WMARESULT prvInterpolateResample(CAudioObjectDecoder *paudec,
//                                 U8 *pbBuf, U16 *pcSamples, Int cbDstLength)
//{
//    U8 *pbSrc, *pbDst, *pbBuf2;
//    PCMSAMPLE iLeftSample, iRightSample, iCurOutput;
//    CAudioObject *pau = paudec->pau;
//    Int nCh = paudec->m_cDstChannel;
//    Int lastPos, curPos, end, i, incr, shift, lastPosS;
//    const Int srcBlkSize = paudec->m_iInterpolSrcBlkSize;
//    const Int dstBlkSize = paudec->m_iInterpolDstBlkSize;
//
//    assert(paudec->m_iInterpolCurPos != 0);
//
//    if (*pcSamples*dstBlkSize < paudec->m_iInterpolCurPos)
//    {
//        paudec->m_iInterpolCurPos -= *pcSamples*dstBlkSize;
//        if (*pcSamples != 0)
//        {
//            for (i=0; i < nCh; i++)
//              paudec->m_iPriorSample[i] =
//                PAUPFNGETSAMPLE((PCMSAMPLE*)pbBuf, (*pcSamples-1)*nCh+i);
//        }
//        *pcSamples = 0;
//        return WMA_OK;
//    }
//    end = (*pcSamples*dstBlkSize - paudec->m_iInterpolCurPos)/srcBlkSize;
//
//    if (((cbDstLength/(Int)pau->m_nBytePerSample)/nCh) < (end+1))  // precheck
//    {
//        assert(WMAB_FALSE);
//        return WMA_E_INVALIDARG;
//    }
//
//    lastPos = paudec->m_iInterpolCurPos + end*srcBlkSize;
//
//    lastPosS = lastPos/dstBlkSize;
//
//    for (i=0; i < nCh; i++) {
//        paudec->m_iLastSample[i] =
//          PAUPFNGETSAMPLE((PCMSAMPLE*)pbBuf, (*pcSamples-1)*nCh+i);
//    }
//
//    // jump src, dst to end
//    pbSrc = (U8*)PAUPRVADVANCEPCMPTR((PCMSAMPLE*)pbBuf, lastPosS*nCh);
//    pbDst = (U8*)PAUPRVADVANCEPCMPTR((PCMSAMPLE*)pbBuf,
//                                      max(lastPosS,end)*nCh);
//    shift = 0;
//    if (lastPosS > end) shift = lastPosS-end;
//    pbBuf2 = (U8*)PAUPRVADVANCEPCMPTR((PCMSAMPLE*)pbBuf, nCh);
//    curPos = lastPos;
//    curPos -= lastPosS*dstBlkSize;
//    assert(curPos >= 0 && curPos < dstBlkSize);
//    while (pbSrc >= pbBuf2)
//    {
//        assert(pbDst >= pbBuf);
//        for (i=0; i < nCh; i++)
//        {
//            if (curPos != 0)
//              iRightSample = PAUPFNGETSAMPLE((PCMSAMPLE*)pbSrc, 0+i);
//            else
//              iRightSample = 0;
//            iLeftSample = PAUPFNGETSAMPLE((PCMSAMPLE*)pbSrc, -nCh+i);
//            iCurOutput = (iRightSample*curPos+iLeftSample*(dstBlkSize-curPos))/
//              dstBlkSize;
//            PAUPFNSETSAMPLE(iCurOutput, (PCMSAMPLE*)pbDst, i);
//        }
//        curPos -= srcBlkSize;
//        if (curPos <= 0)
//        {
//            incr = (curPos - dstBlkSize + 1)/dstBlkSize;
//            curPos -= incr*dstBlkSize;
//            assert(curPos >= 0 && curPos < dstBlkSize);
//            pbSrc = (U8*)PAUPRVADVANCEPCMPTR((PCMSAMPLE*)pbSrc, incr*nCh);
//        }
//        pbDst = (U8*)PAUPRVADVANCEPCMPTR((PCMSAMPLE*)pbDst, -nCh);
//    }
//
//    if (paudec->m_iInterpolCurPos>0 && paudec->m_iInterpolCurPos<dstBlkSize)
//    {
//        for (i=0; i < nCh; i++)
//        {
//            iRightSample = PAUPFNGETSAMPLE((PCMSAMPLE*)pbBuf, i);
//            iLeftSample = paudec->m_iPriorSample[i];
//            iCurOutput = (iRightSample*curPos+iLeftSample*(dstBlkSize-curPos))/
//              dstBlkSize;
//            PAUPFNSETSAMPLE(iCurOutput, (PCMSAMPLE*)pbDst, i);
//        }
//    }
//    for (i=0; i < nCh; i++)
//      paudec->m_iPriorSample[i] = paudec->m_iLastSample[i];
//
//    lastPos += srcBlkSize - *pcSamples*dstBlkSize;
//    paudec->m_iInterpolCurPos = lastPos;
//
//    if (shift != 0)
//    {
//        memcpy(pbBuf, (U8*)PAUPRVADVANCEPCMPTR((PCMSAMPLE*)pbBuf, shift*nCh),
//               (end+1)*nCh*pau->m_nBytePerSample);
//    }
//
//    *pcSamples = (U16)(end+1);
//
//    return WMA_OK;
//} // prvInterpolateResample


//#define DUMP_AUDEC_OUTPUT

#define POSTPROCESS_PREPCM



//WMARESULT audecGetPCMWrap (void* pDecHandle, WMA_U32 cSamplesRequested, WMA_U32 *pcSamplesReturned,
//                                             WMA_U8* pbDst, WMA_U32 cbDstLength,
//                                             WMA_U32* pcbDstUsed, WMA_I64* prtTime,
//                                             audecState *paudecState, WMAPlayerInfo *pPI,
//                                             audecGetPCMParams* pParams)
//{
//    CAudioObjectDecoder* paudec = (CAudioObjectDecoder*)pDecHandle;
//    WMARESULT wmaResult = WMA_OK;
//    U8**  rgpiRecon = NULL;
//    I16  iCh;
//    CAudioObject* pau = NULL;
//    U32 cbDstLengthAdjusted = cbDstLength;
//    I16 nDRCSetting;
//    U16 cTransformSamples;
//    U16 cSamplesReturned = 0;
//
//    CCS_START(CCS_audecGetPCM);
//
//    audecLog((paudec, "audecGetPCM(%d)", cSamplesRequested));
//
//    if (paudec == NULL        ||
//        paudec->pau == NULL   ||
//        pcSamplesReturned == NULL ||
//        (cSamplesRequested > 0 && pbDst == NULL))
//    {
//        TRACEWMA_EXIT(wmaResult, WMA_E_INVALIDARG);
//        goto exit;
//    }
//
//    if (paudec->m_externalState != audecStateGetPCM) {
//      assert(!"audecGetPCM called at an inappropriate time");
//      TRACEWMA_EXIT(wmaResult, WMA_E_WRONGSTATE);
//    }
//    assert(paudec->m_cSamplesReady > 0);
//
//
//
//    cSamplesReturned = (U16)cSamplesRequested;
//
//    nDRCSetting = pPI ? pPI->nDRCSetting : WMA_DRC_HIGH; // update
//
//    paudec->m_externalState = audecStateDone; // changed below on success
//    pau = paudec->pau;
//    rgpiRecon = paudec->m_rgpiRecon;//
//
//    memset(rgpiRecon, 0, sizeof(U8*) * pau->m_cChannel);
//
//    for (iCh = 0; iCh < pau->m_cChannel; iCh++)
//    {
//        PerChannelInfo* ppcinfo = pau->m_rgpcinfo + iCh;
//        rgpiRecon [iCh] = (U8*) prvAdvancePCMPointer((PCMSAMPLE *)pbDst,
//            pau->m_nBytePerSample, iCh);
//
//    }
//
//    //get pcm only when we are in these states
//    if (paudec->m_decsts == BEGIN_FRAME ||
//        paudec->m_decsts == BEGIN_SUBFRAME ||
//        paudec->m_decsts == END_SUBFRAME1)
//    {
//        // post processing may require more buffer than before post processing
//        // i.e., we are upsampling rate or number of channels.
//        // find out how much of buffer we can use to get samples.
//        // adjust cbDstLength to take care of time-domain player options
//        //   - changing number of channels ("downmix")
//        //   - adjusting sampling rate
//        //   - bitdepth is taken care of since pau->m_nBytePerSample
//        //     is already adjusted to dst.
//        // adjust buffer size here since auGetPCM is common function
//        // and does not have decoder specific member variables available.
//        if (paudec->pau->m_iXformSamplingRate != paudec->m_iDstSamplingRate)
//        {
//            cbDstLengthAdjusted = min(cbDstLength,
//              cbDstLength*paudec->m_iInterpolSrcBlkSize/paudec->m_iInterpolDstBlkSize);
//        }
//        cbDstLengthAdjusted = min(cbDstLengthAdjusted,
//          cbDstLengthAdjusted*pau->m_cChannel/paudec->m_cDstChannel);
//
//        TRACEWMA_EXIT(wmaResult, auGetPCM (pau, (Void*)paudec, &cSamplesReturned, rgpiRecon, cbDstLengthAdjusted, paudec->m_fSPDIF));
//        // these are the number of transform samples that are consumed.
//        // actual number of samples output may be smaller or larger than this.
//        cTransformSamples = cSamplesReturned;
//
//        //RACEWMA_EXIT(wmaResult, audecPostProcessPostPCM(pDecHandle, &cSamplesReturned, pbDst, cbDstLength, nDRCSetting));
//    }
//    else
//    {
//        cSamplesReturned = 0;         //we are in a state of error; don't return anything
//        wmaResult = WMA_E_FAIL;
//        TraceResult(wmaResult);
//        goto exit;
//    }
//
//    if (pcbDstUsed) *pcbDstUsed = cSamplesReturned * paudec->m_nDstBytePerSample * paudec->m_cDstChannel;
//
//
//
//    //convert back to 100ns
//    // timestamps are donw using iCurrPresTime, which are calculated using
//    // destination sampling rate; therefore iCurrPresTime should be updated
//    // using actual number of samples that are output
//    if (prtTime) *prtTime = paudec->m_iCurrPresTime * 10000000 / paudec->m_iDstSamplingRate;
//
//    paudec->m_iCurrPresTime += cSamplesReturned;
//
//
//
//
//    assert(paudec->m_cSamplesReady >= cTransformSamples);
//    paudec->m_cSamplesReady -= cTransformSamples;
//
//
//    audecLog((paudec, "...%d @ %d", cSamplesReturned, paudec->m_iCurrPresTime));
//    if (paudec->m_cSamplesReady > 0)
//      paudec->m_externalState = audecStateGetPCM;
//    else
//      paudec->m_externalState = audecStateDecode;
//
//exit:
//    if (pcSamplesReturned) *pcSamplesReturned = cSamplesReturned;
//    if (paudec->m_fSPDIF && paudec->m_fNeedHeader && paudec->m_externalState == audecStateDecode)
//    {
//        paudec->m_externalState = INPUT_STATE;
//    }
//    if (paudecState) *paudecState = paudec->m_externalState;
//    CCS_END(CCS_audecGetPCM);
//    return wmaResult;
//} // audecGetPCM
//
//WMARESULT audecGetPCM (void* pDecHandle, WMA_U32 cSamplesRequested, WMA_U32 *pcSamplesReturned,
//                                         WMA_U8* pbDst, WMA_U32 cbDstLength,
//                                         WMA_U32* pcbDstUsed, WMA_I64* prtTime,
//                                         audecState *paudecState, WMAPlayerInfo *pPI,
//                                         audecGetPCMParams* pParams)
//{
//    WMA_TRY_AND_EXCEPT_AV(audecGetPCMWrap(pDecHandle, cSamplesRequested, pcSamplesReturned,
//                                         pbDst, cbDstLength, pcbDstUsed, prtTime,
//                                         paudecState, pPI, pParams));



// Translate the Givens angle/sign information into channel transform
//WMARESULT prvDecodeChannelXform(CAudioObjectDecoder* paudec)
//{
//    WMARESULT wmaResult = WMA_OK;
//    Int iChGrp;
//    CChannelGroupInfo* pcgi = NULL;
//    CAudioObject* pau = paudec->pau;
//
//    if (pau->m_iVersion <= 2)
//        return wmaResult;
//
//    for (iChGrp = 0; iChGrp < pau->m_cChannelGroup; iChGrp++) {
//        pcgi = pau->m_rgChannelGrpInfo + iChGrp;
//        if (pcgi->m_fIsPredefinedXform == WMAB_FALSE) {
//            // Reconstruct the inverse matrix from angle/sign information
////            TRACEWMA_EXIT(wmaResult,
//                //                auGetTransformFromGivensFactors(pcgi->m_rgbRotationAngle,
//                //                pcgi->m_rgbRotationSign,
//                //                pcgi->m_cChannelsInGrp,
//                //                pau->m_cChannel,  // only for security
//                //                pcgi->m_rgfltMultiXInverse,
//                //                pau->m_rgfltGivensTmp0,
//                //                pau->m_rgfltGivensTmp1));
//
//        }
//    }
////exit:
//    return wmaResult;
//} // prvDecodeChannelXform


Void prvResetRunLevelState(CAudioObjectDecoder *paudec)
{
    if (paudec->pau->m_iVersion <= 2)
    {
        paudec->m_rlsts = VLC;
    }
}
#pragma arm section code

#endif//WMAHIGHRATE) || WMAMIDRATELOWRATE

//*****************************************************************************************
//
// prvDecodeSubFrame
//
// Code Path:
//  lossy
//          SUBFRM_HDR -> SUBFRM_COEFDEC_LOSSY -----------------------------------------> SUBFRM_REC_LOSSY -> SUBFRM_DONE
//  lossless (mixed)
//          SUBFRM_HDR ----------------------> SUBFRM_COEFDEC_MLLM -> SUBFRM_REC_MLLM ----------------------> SUBFRM_DONE
//
//
//*****************************************************************************************
#ifdef WMAMIDRATELOWRATE
#pragma arm section code = "WmaLowRateCode"

extern tWMAFileHeader g_hdr;
WMARESULT prvDecodeSubFrame(CAudioObjectDecoder* paudec)
{
    WMARESULT hr = WMA_OK;
    I16       iCh, iChSrc;
    CAudioObject* pau = paudec->pau;
    long  scalefactorhxd;//hxd
    PerChannelInfo* ppcinfo;
    Bool fSkipAll;
	int isValid;


    while (paudec->m_subfrmdecsts != SUBFRM_DONE)
    {
        switch (paudec->m_subfrmdecsts)
        {
            case SUBFRM_HDR :
                TRACEWMA_EXIT(hr, prvDecodeSubFrameHeader(paudec));
                assert(paudec->m_hdrdecsts == HDR_DONE);
                /*if (pau->m_bUnifiedLLM == WMAB_FALSE) */
                {
                    paudec->m_subfrmdecsts = SUBFRM_COEFDEC_LOSSY;
                    paudec->pau->m_iCurrReconCoef = 0;             //iRecon is offset by -1 so comarison
                    paudec->m_iChannel = 0;
                    prvResetRunLevelState(paudec); // reset run level decoding
                }
                //else {

                //}

                break;

            case SUBFRM_COEFDEC_LOSSY:
                if (g_hdr.num_channels == 1)
                {
                    TRACEWMA_EXIT(hr, prvDecodeCoefficientMono(paudec, pau->m_rgpcinfo));
                }
                else
                {
                    TRACEWMA_EXIT(hr, prvDecodeCoefficientStereo(paudec, pau->m_rgpcinfo));
                }
                //TRACEWMA_EXIT(hr, (*(paudec->m_pfnDecodeCoefficient))(paudec, pau->m_rgpcinfo));
                paudec->m_subfrmdecsts = SUBFRM_REC_LOSSY;

                break;

            case SUBFRM_COEFDEC_MLLM:

                //paudec->m_subfrmdecsts = SUBFRM_REC_MLLM;
                break;

            case SUBFRM_REC_MLLM:

                //paudec->m_subfrmdecsts = SUBFRM_DONE;
                break;

            case SUBFRM_REC_LOSSY:
                // WMA Timestamps: To detect start-of-stream and discard correct amount of silence,
                // we need to verify claim that m_iPower[*] = 1, ForceMaskUpdate and actual power = 0.
                // If m_rgiCoefQ is all 0, we will accept that as actual power = 0 even though it is
                // theoretically possible for actual power != 0 if all bands inside noise-sub band are
                // substituted.
                if (CODEC_BEGIN == pau->m_codecStatus)
                {
                    for (iCh = 0; iCh < pau->m_cChannel; iCh++)
                    {
                        iChSrc = pau->m_rgiChInTile [iCh];
                        ppcinfo = pau->m_rgpcinfo + iChSrc;
                        if (ppcinfo->m_iPower != 0)
                            SetActualPower(ppcinfo->m_rgiCoefQ, ppcinfo->m_cSubbandActual,
                                           ppcinfo, pau->m_codecStatus);
                    }
                }

                //CCDUMP(0, 10);

                pau->m_qstQuantStep = qstCalcQuantStep(pau->m_iQuantStepSize,&isValid);
				TRACE_QUANT_STEP(isValid);
                for (iCh = 0; iCh < pau->m_cChannel; iCh++)
                {
                    iChSrc = pau->m_rgiChInTile [iCh];
                    ppcinfo = pau->m_rgpcinfo + iChSrc;
                    if (ppcinfo->m_iPower != 0)
                    {
                        if (pau->m_iWeightingMode == BARK_MODE)
                        {
                            if (!pau->m_fNoiseSub)
                                pau->aupfnInverseQuantize = NULL;
                            else
                                TRACEWMA_EXIT(hr, prvInverseQuantizeMidRate(pau, ppcinfo, (I32*)ppcinfo->m_rguiWeightFactor));
                        }
                        else
                            TRACEWMA_EXIT(hr, prvInverseQuantizeLowRate(pau, ppcinfo, (I32*)ppcinfo->m_rguiWeightFactor));
                        //TRACEWMA_EXIT(hr, (*pau->aupfnInverseQuantize)(pau, ppcinfo, (I32*)ppcinfo->m_rguiWeightFactor));
                    }
                    else
                    {
                        memset(ppcinfo->m_rgiCoefRecon, 0, sizeof(CoefType) * (ppcinfo->m_cSubband));
                    }
                }

                //CCDUMP(1, 0);


                // Decode Givens angles & signs into inverse multichannel transform.
                //TRACEWMA_EXIT(hr, prvDecodeChannelXform(paudec));

                // Apply top level hierarchical xform
                auInvChannelXForm(pau, WMAB_TRUE);
                // Apply bottom level hierarchical xform
                auInvChannelXForm(pau, WMAB_FALSE);

                // Check if all channels in this tile are zero (to be used later)
                fSkipAll = WMAB_TRUE;
                for (iCh = 0; iCh < pau->m_cChInTile; iCh++)
                {
                    iChSrc = pau->m_rgiChInTile [iCh];
                    if (pau->m_rgpcinfo[iChSrc].m_iPower)
                    {
                        fSkipAll = WMAB_FALSE;
                        break;
                    }
                }

                // Due to Scrunch Bug #32, we used to always force mask update at
                // the start (all channels). We no longer do this for v3, so we
                // must not call auInvWeightSpectrumV3 for zero-power subframes at the
                // start-of-stream, because the first mask update will come with the
                // first non-zero subframe. Note that if even one channel is non-zero,
                // we must process all channels due to v3 auInvChannelXForm.
                if (pau->m_iVersion > 2 && WMAB_FALSE == fSkipAll)
                {
                    // On each channel, do the mask weighting.
                    for (iCh = 0; iCh < pau->m_cChannel; iCh++)
                    {
                        iChSrc = pau->m_rgiChInTile [iCh];
                        ppcinfo = pau->m_rgpcinfo + iChSrc;

                        // discard subwoofer coeffs.
                        if (iChSrc == pau->m_nSubWooferChannel)
                        {
                            assert(pau->m_cSubWooferCutOffIndex < ppcinfo->m_cSubband);
                            memset((Void*)(ppcinfo->m_rgiCoefRecon + pau->m_cSubWooferCutOffIndex),
                                   0, sizeof(CoefType) * (ppcinfo->m_cSubbandAdjusted - pau->m_cSubWooferCutOffIndex));
                        }

                        pau->m_iCurrReconCoef = -1;
                        //TRACEWMA_EXIT(hr, auInvWeightSpectrumV3 (pau, ppcinfo, ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgfMaskUpdate[0]));
                    }
                }


                if (! pau->m_fNoiseSub)
                {
                    for (iCh = 0; iCh < pau->m_cChannel; iCh++)
                    {
                        Int iHighToBeZeroed;
                        iChSrc = pau->m_rgiChInTile [iCh];
                        ppcinfo = pau->m_rgpcinfo + iChSrc;
                        // V4 only zeroed above HighCutOff when NoiseSubstitution was not in effect - e.g. it only zeros for HighRate
                        iHighToBeZeroed = sizeof(CoefType) * (ppcinfo->m_cSubbandAdjusted - pau->m_cHighCutOffAdjusted);
                        memset(ppcinfo->m_rgiCoefRecon + pau->m_cHighCutOffAdjusted, 0, iHighToBeZeroed);

                    }
                }

#ifdef ENABLE_EQUALIZER
                //equalize
                for (iCh = 0; iCh < pau->m_cChannel; iCh++)
                {
                    iChSrc = pau->m_rgiChInTile [iCh];
                    ppcinfo = pau->m_rgpcinfo + iChSrc;
                    if (ppcinfo->m_iPower != 0)
                    {
                        TRACEWMA_EXIT(hr, prvEqualize(paudec, ppcinfo));

                        if (paudec->m_fComputeBandPower == WMAB_TRUE)
                            prvComputeBandPower(paudec);
                    }
                }
#endif // ENABLE_EQUALIZER

                if (WMAB_FALSE == fSkipAll)
                {
                    for (iCh = 0; iCh < pau->m_cChannel; iCh++)
                    {
                        //Float fltAfterScaleFactor;
                        short nFacExponenthxd;//hxd
                        iChSrc = pau->m_rgiChInTile [iCh];
                        ppcinfo = pau->m_rgpcinfo + iChSrc;
                        //fltAfterScaleFactor = (pau->m_iVersion == 1) ? pau->m_fltDctScale :
                        //   (Float)(2.0f/ppcinfo->m_cSubband);
                        //hxd
                        if (pau->m_iVersion == 1)
                        {
                            if (ppcinfo->m_cSubbandAdjusted == 2048 || ppcinfo->m_cSubbandAdjusted == 512 || ppcinfo->m_cSubbandAdjusted == 128)
                                scalefactorhxd = 2147483647;
                            else
                                scalefactorhxd = 1518500224;
                            //hxd
                            if (ppcinfo->m_cSubbandAdjusted == 2048 || ppcinfo->m_cSubbandAdjusted == 1024)
                                nFacExponenthxd = 4;
                            else
                                if (ppcinfo->m_cSubbandAdjusted == 512 || ppcinfo->m_cSubbandAdjusted == 256)
                                    nFacExponenthxd = 3;
                                else
                                    nFacExponenthxd = 2;
                            //hxd
                        }
                        if (pau->m_iVersion == 2)
                        {
                            scalefactorhxd = 1073741824;
                            nFacExponenthxd = 0;
                        }
                        //hxd
                        // INTERPOLATED_DOWNSAMPLE
//                    if (paudec->m_fLowPass)
                        //                    {
                        //                        auLowPass2(paudec, (CoefType*) ppcinfo->m_rgiCoefRecon,
                        //                            ppcinfo->m_cSubband / 2);
                        //                    }

                        if (ppcinfo->m_iPower != 0 || pau->m_iVersion > 2) //这个dctiv是用于midrate和lowrate
                        {
                            //(*pau->aupfnDctIV)((CoefType*) ppcinfo->m_rgiCoefRecon,
                            //scalefactorhxd, ppcinfo->m_cSubbandAdjusted, nFacExponenthxd);
                            auDctIV((CoefType*) ppcinfo->m_rgiCoefRecon, scalefactorhxd, ppcinfo->m_cSubbandAdjusted, nFacExponenthxd);
#ifdef SATURERATE_AFTER_FFT
							{
								//当前帧的原始位置
								CBT* curFrameCoeffReconOrigin = (pau->m_rgiCoefReconOrig + (pau->m_cFrameSampleHalfAdjusted * 5 / 4) * iCh + (pau->m_cFrameSampleHalfAdjusted / 4));
								long offset = (long)(ppcinfo->m_rgiCoefRecon - curFrameCoeffReconOrigin);//偏移点数
								ppcinfo->dst_rgiCoefRecon = (short*)curFrameCoeffReconOrigin + offset;
							    coeffSaturerate(ppcinfo->dst_rgiCoefRecon,ppcinfo->m_rgiCoefRecon,(int)ppcinfo->m_cSubbandAdjusted,pau->m_cLeftShiftBitsFixedPost);
							}
#endif							
                        }
                    }
                }

                //CCDUMP(2, 0);

                paudec->m_subfrmdecsts = SUBFRM_DONE;
                break;
        }
    }

exit:
    //FUNCTION_PROFILE_STOP(&fp);
    return hr;
} // prvDecodeSubFrame for midrate/lowrate
#pragma arm section code

#endif

//*****************************************************************************************
//
// prvUpdateSubFrameConfig
//
//*****************************************************************************************
#if defined(WMAHIGHRATE) || defined(WMAMIDRATELOWRATE)
#pragma arm section code = "WmaHighLowCommonCode"

WMARESULT prvUpdateSubFrameConfig(CAudioObjectDecoder* paudec, Int iSizeNext)
{
    Int iStartCurr;
    I16 iCh, iChSrc;
    CAudioObject* pau = paudec->pau;
    PerChannelInfo* ppcinfo;
    /*** less than 0.5%
    #ifdef PROFILE
        //FunctionProfile fp;
        //FunctionProfileStart(&fp,UPDATE_SUB_FRAME_CONFIG_PROFILE);
    #endif
    *///

    assert(pau->m_iVersion <= 2);

    //constant size case; not sent
    if (iSizeNext == 0)
    {
//        assert (ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_cSubFrame > 0);
        paudec->m_fLastSubFrame = WMAB_TRUE;
        return WMA_OK;
    }

    for (iCh = 0; iCh < pau->m_cChInTile; iCh++)
    {
        iChSrc = pau->m_rgiChInTile [iCh];
        ppcinfo = pau->m_rgpcinfo + iChSrc;

        iStartCurr = ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameStart [0]
                     + ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameSize  [ppcinfo->m_iCurrSubFrame];
        if (iStartCurr >= pau->m_cFrameSampleHalf)
        {
            //init the next frame
            I16 iCurr = ppcinfo->m_iCurrSubFrame;
            paudec->m_fLastSubFrame = WMAB_TRUE;
            ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameSize  [iCurr + 1] = (I16) iSizeNext;
        }
        else
        {
            //must be within one frame; must have received the first one
            I16 iCurr = ppcinfo->m_iCurrSubFrame;
            assert(ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_cSubFrame == iCurr + 1);
            //assert (iSizeNext < pau->m_cFrameSampleHalf);        //> 1 subfrm
            ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameSize [iCurr + 1] = (I16) iSizeNext;

            ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameStart[0] =
                ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameStart[0] +
                ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameSize[iCurr];
            if (ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameStart[0] +
                    ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameSize [iCurr + 1] > pau->m_cFrameSampleHalf)
            {
                REPORT_BITSTREAM_CORRUPTION();
                return TraceResult(WMA_E_BROKEN_FRAME);
            }

            ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_cSubFrame++;
        }
        ppcinfo++;
    }

    /***
    #ifdef PROFILE
    //FunctionProfileStop(&fp);
    #endif
    *///

    return WMA_OK;
} // prvUpdateSubFrameConfig
#pragma arm section code

#endif
#define AUINV_RECON_CHANNEL
#define AUINV_RECON_CHANNEL_DEC
#include "wma_msaudiotemplate.c"
#undef AUINV_RECON_CHANNEL_DEC
#undef AUINV_RECON_CHANNEL

//*****************************************************************************************
//
// prvDecodeSubFrameHighRate
//
// Code Path:
//  lossy
//          SUBFRM_HDR -> SUBFRM_COEFDEC_LOSSY -----------------------------------------> SUBFRM_REC_LOSSY -> SUBFRM_DONE
//  lossless (mixed)
//          SUBFRM_HDR ----------------------> SUBFRM_COEFDEC_MLLM -> SUBFRM_REC_MLLM ----------------------> SUBFRM_DONE
//
//*****************************************************************************************
#ifdef WMAHIGHRATE
#pragma arm section code = "WmaHighRateCode"

WMARESULT prvDecodeSubFrameHighRate(CAudioObjectDecoder* paudec)
{
    WMARESULT hr = WMA_OK;
    I16     iCh, iChSrc;
    PerChannelInfo* ppcinfo;
    CAudioObject* pau = paudec->pau;
    //Float fltAfterScaleFactor;
    long  scalefactorhxd;//hxd
    Bool fSkipAll;
	int isValid;


    //while (paudec->m_subfrmdecsts != SUBFRM_DONE)
    {

        switch (paudec->m_subfrmdecsts)
        {
            case SUBFRM_HDR :
                TRACEWMA_EXIT(hr, prvDecodeSubFrameHeader(paudec)); //get 3 value:QuantStepsize,ipower,iMaskQ

                pau->m_qstQuantStep = qstCalcQuantStep(pau->m_iQuantStepSize,&isValid);//calculate QuantStep from QuantStepSize
                TRACE_QUANT_STEP(isValid);

                for (iCh = 0; iCh < pau->m_cChInTile; iCh++)
                {
                    iChSrc = pau->m_rgiChInTile [iCh];
                    ppcinfo = pau->m_rgpcinfo + iChSrc;
                    memset(ppcinfo->m_rgiCoefRecon, 0, sizeof(CoefType) * pau->m_cHighCutOffAdjusted);
                }
                paudec->m_subfrmdecsts = SUBFRM_COEFDEC_LOSSY;

                //iRecon is offset by -1 so comarison
                paudec->pau->m_iCurrReconCoef = (I16)(pau->m_cLowCutOff - 1);

                    paudec->m_iChannel = 0;
                prvResetRunLevelState(paudec); // reset run level decoding

            case SUBFRM_COEFDEC_LOSSY:
                //Decode coefficents for sum channel or left & right channels
                for (; paudec->m_iChannel < pau->m_cChInTile; paudec->m_iChannel++)
                {
                    iChSrc = pau->m_rgiChInTile [paudec->m_iChannel];
                    ppcinfo = pau->m_rgpcinfo + iChSrc;
                    paudec->m_cMaxRun = (I16) LOG2(ppcinfo->m_cSubbandActual - 1) + 1;

                    if (ppcinfo->m_iPower != 0)
                    {
                        {
                            TRACEWMA_EXIT(hr, auReconCoefficentsHighRate(pau, paudec, ppcinfo));
                        }
                    }

                    if (pau->m_iVersion == 1)
                    {
                        ibstrmFlush(&paudec->m_ibstrm);
                    }

                    //usually <= V2 should only go up to highCutoff but certain V1 contents do go up as high as m_cSubband
                    assert(paudec->pau->m_iCurrReconCoef <= (I16) ppcinfo->m_cSubband);
                    pau->m_iCurrReconCoef = (I16)(pau->m_cLowCutOff - 1);            //iRecon is offset by -1 so comarison
                    prvResetRunLevelState(paudec); // reset run level decoding
                }

                paudec->m_subfrmdecsts = SUBFRM_REC_LOSSY;
                //break;

            case SUBFRM_REC_LOSSY:
                for (iCh = 0; iCh < pau->m_cChInTile; iCh++)
                {
                    iChSrc = pau->m_rgiChInTile [iCh];
                    ppcinfo = pau->m_rgpcinfo + iChSrc;
                    if (ppcinfo->m_iPower != 0)
                    {
                        // WMA Timestamps: To detect start-of-stream and discard correct amount of silence,
                        // we need to verify claim that m_iPower[*] = 1, ForceMaskUpdate and actual power = 0.
                        ppcinfo->m_iActualPower = 0;
                        if (CODEC_BEGIN == pau->m_codecStatus)
                        {
                            SetActualPowerHighRate(ppcinfo->m_rgiCoefRecon,ppcinfo->m_cSubband, ppcinfo, pau->m_codecStatus);
                        }
                    }
                    else
                    {
                        memset(ppcinfo->m_rgiCoefRecon, 0, sizeof(CoefType) * ppcinfo->m_cSubbandAdjusted);
                        ppcinfo->m_iActualPower = 0;
                    }
                }


                // Check if all channels in this tile are zero (to be used later)
                fSkipAll = WMAB_TRUE;
                for (iCh = 0; iCh < pau->m_cChInTile; iCh++)
                {
                    iChSrc = pau->m_rgiChInTile [iCh];
                    if (pau->m_rgpcinfo[iChSrc].m_iPower)
                    {
                        fSkipAll = WMAB_FALSE;
                        break;
                    }
                }

                // Apply top level hierarchical xform
                auInvChannelXForm(pau, WMAB_TRUE);
                // Apply bottom level hierarchical xform
                auInvChannelXForm(pau, WMAB_FALSE);


                if (WMAB_FALSE == fSkipAll)
                {
                    short nFacExponenthxd;//hxd
                    if (pau->m_iVersion == 1)
                    {
						I16 SubbandAdjusted = ppcinfo->m_cSubbandAdjusted;
                        if (SubbandAdjusted == 2048 || SubbandAdjusted == 512 || SubbandAdjusted == 128)
                            scalefactorhxd = 2147483647;
                        else
                            scalefactorhxd = 1518500224;
                        //hxd
                        if (SubbandAdjusted == 2048 || SubbandAdjusted == 1024)
                            nFacExponenthxd = 4;
                        else if (SubbandAdjusted == 512 || SubbandAdjusted == 256)
                            nFacExponenthxd = 3;
                        else
                            nFacExponenthxd = 2;
                        //hxd
                    }
                    else if (pau->m_iVersion == 2)
                    {
                        scalefactorhxd = 1073741824;
                        nFacExponenthxd = 0;
                    }
                    //hxd
                    for (iCh = 0; iCh < pau->m_cChInTile; iCh++)
                    {
                        iChSrc = pau->m_rgiChInTile [iCh];
                        ppcinfo = pau->m_rgpcinfo + iChSrc;


                        if (ppcinfo->m_iPower != 0 || pau->m_iVersion > 2)
                        {
                            auDctIV((CoefType*) ppcinfo->m_rgiCoefRecon, scalefactorhxd, ppcinfo->m_cSubbandAdjusted, nFacExponenthxd);	

#ifdef SATURERATE_AFTER_FFT
							{
								//当前帧的原始位置
								CBT* curFrameCoeffReconOrigin = (pau->m_rgiCoefReconOrig + (pau->m_cFrameSampleHalfAdjusted * 5 / 4) * iCh + (pau->m_cFrameSampleHalfAdjusted / 4));
								long offset = (long)(ppcinfo->m_rgiCoefRecon - curFrameCoeffReconOrigin);//偏移点数
								ppcinfo->dst_rgiCoefRecon = (short*)curFrameCoeffReconOrigin + offset;
							    coeffSaturerate(ppcinfo->dst_rgiCoefRecon,ppcinfo->m_rgiCoefRecon,(int)ppcinfo->m_cSubbandAdjusted,pau->m_cLeftShiftBitsFixedPost);
							}
#endif							
                        }
                    }
                }

                paudec->m_subfrmdecsts = SUBFRM_DONE;
                break;
        }
    }
exit:


    //if (fOnHold)
        //return WMA_E_ONHOLD;
    return hr;
} //prvDecodeSubFrameHighRate for highrate
#endif
#if defined(WMAHIGHRATE) || defined(WMAMIDRATELOWRATE)
#pragma arm section code = "WmaHighLowCommonCode"

// Decoder-only portion of auAdaptToSubFrameConfig
WMARESULT auAdaptToSubFrameConfigDEC(CAudioObject* pau)
{
    Int iCh, iChSrc, iFrameSize;
    PerChannelInfo* ppcinfo = NULL;

    for (iCh = 0; iCh < pau->m_cChInTile; iCh++)
    {
        iChSrc = pau->m_rgiChInTile [iCh];
        ppcinfo = pau->m_rgpcinfo + iChSrc;

        // Why use the buffer from the end? chao
        /*if (pau->m_bPureLosslessMode == WMAB_FALSE)*/
        {
            // ======== Setup the rgiCoefRecon to the proper place ========

            iFrameSize = (pau->m_cFrameSampleHalf);
#ifdef SATURERATE_AFTER_FFT
            ppcinfo->m_rgiCoefRecon   = pau->m_rgiCoefReconOrig + (iFrameSize * 5 / 4) * iChSrc +
                                        (iFrameSize >> 2) +
                                        ppcinfo->m_iCurrCoefPosition;
#else
            ppcinfo->m_rgiCoefRecon   = pau->m_rgiCoefReconOrig + (iFrameSize * 3 / 2) * iChSrc +
                                        (iFrameSize >> 1) +
                                        ppcinfo->m_iCurrCoefPosition;
#endif
            ppcinfo->m_rgfltCoefRecon = (Float *)(ppcinfo->m_rgiCoefRecon);
            ppcinfo->m_iCurrCoefPosition += (I16)ppcinfo->m_cSubFrameSampleHalfAdjusted;
        }
    }

    return WMA_OK;
} // auAdaptToSubFrameConfigDEC
#pragma arm section code

#endif
#ifdef  WMAINITIALIZE
#pragma arm section code = "WmaOpenCodecCode"

//reconfig the decoder: don't allow memroy allocation here!!!
WMARESULT prvReConfig(CAudioObjectDecoder* paudec)
{
    I16 i;
    CAudioObject* pau = paudec->pau;

    if (pau->m_iWeightingMode == LPC_MODE || pau->m_fNoiseSub)//判断中低码率是根据LPC_MODE或NoiseSub来确定,而判断每帧样本数目是根据采样率来决定的
    {
//#ifdef WMAMIDRATELOWRATE
        //low rate or mid rate
        //paudec->m_pfnDecodeSubFrame = prvDecodeSubFrame;
//#endif
        g_flagHighRate = 0;
    }
    else

    {
//#ifdef WMAHIGHRATE
        //high rate
        //paudec->m_pfnDecodeSubFrame = prvDecodeSubFrameHighRate;
//#endif
        g_flagHighRate = 1;
    }
    for (i = 0; i < pau->m_cChannel; i++)
    {
        PerChannelInfo* ppcinfo = pau->m_rgpcinfo + i;
//#ifdef WMAMIDRATELOWRATE
        if (pau->m_iWeightingMode == LPC_MODE)
        { // LPC
            ppcinfo->m_rguiWeightFactor = pau->m_rguiWeightFactor + (pau->m_cFrameSampleHalf) * i;
            ppcinfo->m_rgfltWeightFactor = (Float*)(pau->m_rguiWeightFactor + (pau->m_cFrameSampleHalf) * i);
        }
        else
//#endif
        {
            ppcinfo->m_rguiWeightFactor = (U32*) pau->m_rgpcinfo [i].m_rgiMaskQ;
            ppcinfo->m_rgfltWeightFactor = (Float*)(pau->m_rgpcinfo [i].m_rgiMaskQ);
        }

    } // for
    return WMA_OK;
} // prvReConfig
#pragma arm section code

#endif
#endif
#endif

