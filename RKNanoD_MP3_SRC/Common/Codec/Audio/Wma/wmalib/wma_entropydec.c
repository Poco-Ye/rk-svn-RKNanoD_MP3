//*@@@+++@@@@******************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// Contains the logic common to WMA Std, WMA Pro & WMA Lossless decoders.
// Also contains a few stub functions, if actual implementations are not needed.
//*@@@---@@@@******************************************************************
#include "../include/audio_main.h"
#include "..\wmaInclude\msaudiodec.h"
#include "..\wmaInclude\huffdec.h"
#include "..\wmaInclude\AutoProfile.h"
#include "..\wmaInclude\entropydec.h"

#ifdef A_CORE_DECODE
#ifdef WMA_DEC_INCLUDE
//****************************************************************************
// floor of log base 2 of a number which is a power of 2
//****************************************************************************
#ifdef WMAINITIALIZE_HIGH_MID_ALL_USED
#pragma arm section code = "WmaCommonCode"
I32 LOG2(U32 i)
{   // returns n where n = log2(2^n) = log2(2^(n+1)-1)
    U32 iLog2 = 0;
    assert(i != 0);

    while ((i >> iLog2) > 1)
        iLog2++;

    return iLog2;
}
#pragma arm section code
#endif

//*****************************************************************************************
//
// prvDecodeFrameHeader
//
//*****************************************************************************************
#if defined(WMAHIGHRATE) || defined(WMAMIDRATELOWRATE)
#pragma arm section code = "WmaHighLowCommonCode"

WMARESULT prvDecodeFrameHeader(CAudioObjectDecoder* paudec)
{
    WMARESULT wmaResult     = WMA_OK;
    CAudioObject* pau       = paudec->pau;
    PerChannelInfo* ppcinfo = NULL;
    SubFrameConfigInfo* psubfrmconfig = NULL;

    I16 iCh;
    U32 iResult;
    I16 iSizePrev = 0, iSizeCurr = 0;
//    I16 iSizeTile;
    I32 cBitSize  = 0;
    I32 cBitSize0 = 0;
    I32 cBitSize1 = 0;

//#ifdef PROFILE
//    FunctionProfile fp;
//    FunctionProfileStart(&fp,DECODE_FRAME_HEADER_PROFILE);
//#endif

    if (pau->m_fAllowSubFrame && pau->m_iMaxSubFrameDiv <= 1)        //avoid log(0) below
    {
        REPORT_BITSTREAM_CORRUPTION();
        wmaResult = WMA_E_BROKEN_FRAME;
        CHECKWMA_EXIT(wmaResult);
    }

    //if (pau->m_bPureLosslessMode == WMAB_TRUE) {
    //        pau->m_u32TrailingSize = 0;
    //    }

    while (paudec->m_fhdrdecsts != FHDR_DONE)
    {
        switch (paudec->m_fhdrdecsts)//FHdrDecodeStatus
        {
            case FHDR_PREV:
                if (pau->m_fAllowSubFrame && pau->m_iVersion <= 2)
                {
                    //peek enough bits for this "case" if not enough will be on hold
                    Int cBitsNeed = LOG2((U32) LOG2((U32)pau->m_iMaxSubFrameDiv)) + 1;
                    TRACEWMA_EXIT(wmaResult, ibstrmLookForBits(&paudec->m_ibstrm, cBitsNeed * 2));
                    TRACEWMA_EXIT(wmaResult, ibstrmGetBits(&paudec->m_ibstrm, cBitsNeed, &iResult));
                    iSizePrev = (I16)(pau->m_cFrameSampleHalf / (1 << iResult));

                    //current
                    TRACEWMA_EXIT(wmaResult, ibstrmGetBits(&paudec->m_ibstrm, cBitsNeed, &iResult));
                    iSizeCurr = (I16)(pau->m_cFrameSampleHalf / (1 << iResult));

                    assert(iSizePrev != 0 && iSizeCurr != 0);
                    if (iSizePrev  < pau->m_cMinSubFrameSampleHalf || iSizePrev > pau->m_cFrameSampleHalf ||
                            iSizeCurr  < pau->m_cMinSubFrameSampleHalf || iSizeCurr > pau->m_cFrameSampleHalf)
                    {
                        REPORT_BITSTREAM_CORRUPTION();
                        return TraceResult(WMA_E_BROKEN_FRAME);//如果不满足上述条件表示不是标准wma文件(128<=isizeprev/isizecurr<=2048)
                    }

                    for (iCh = 0; iCh < pau->m_cChannel; iCh++)
                    {
                        ppcinfo = pau->m_rgpcinfo + iCh;
                        //only useful in case of packet loss and prev info lost or first in sequece
                        assert(ppcinfo->m_iCurrSubFrame == 0);
                        ppcinfo->m_rgsubfrmconfig [CONFIG_CURRFRM].m_rgiSubFrameSize [-1] = iSizePrev;

                        //first frame in super and first sub frame; init
                        ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_cSubFrame = 0;
                        ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameSize [0] = iSizeCurr;
                        ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameStart[0] = 0;
                        ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_cSubFrame++;
                    }

                    paudec->m_fhdrdecsts = FHDR_ADJUST_TIMESTAMP;
                    break;

                } //else if (pau->m_iVersion > 2) {   // Version 3 or above
                //
                //                paudec->m_fhdrdecsts = FHDR_SIZE;
                //                //prvInitDecodeTileHdr(paudec, CONFIG_CURRFRM);
                //                break;
                //            }

                else            //non - superframe
                {
                    paudec->m_fhdrdecsts = FHDR_ADJUST_TIMESTAMP;
                    break;
                }

            case FHDR_SIZE:
//            if (pau->m_fWriteFrameSize)
                //            {
                //                assert(pau->m_iVersion > 2);
                //                TRACEWMA_EXIT(wmaResult, ibstrmGetBits(&paudec->m_ibstrm, pau->m_cBitsBitsPacketLength, (U32 *)&pau->m_cComprFrameSizeBits));
                //                //WMAPrintf("Actual Frame Size: %d\n", pau->m_cComprFrameSizeBits);
                //                if (paudec->m_fSPDIF)
                //                {
                //                    ibstrmSetGap(&paudec->m_ibstrm, pau->m_cComprFrameSizeBits - pau->m_cBitsBitsPacketLength);
                //                }
                //            }

                paudec->m_fhdrdecsts = FHDR_CURR;

            case  FHDR_CURR:
                //if (pau->m_iVersion > 2)
                //{
                //    TRACEWMA_EXIT(wmaResult, prvDecodeTileHdr(paudec, CONFIG_CURRFRM));
                //}
                paudec->m_ppxformsts = PPXFORM_BEGIN;
                paudec->m_fhdrdecsts = FHDR_PPXFORM;

            case  FHDR_PPXFORM:
                //assert(pau->m_iVersion == 3);
                //TRACEWMA_EXIT(wmaResult, prvEntropyDecodePostProcXform(paudec));
                //paudec->m_fhdrdecsts = FHDR_DRC_PARAM;

            case  FHDR_DRC_PARAM:
                //if (pau->m_fGenerateDrcParams)
                //{
                assert(pau->m_iVersion > 2);
//                TRACEWMA_EXIT(wmaResult, ibstrmGetBits(&paudec->m_ibstrm, BITS_PER_BYTE, &iResult));
                //                pau->m_ucDrcFrameScaleFactor = (U8)iResult;

                //WMAFprintf(stdout, "Fac=%d\n", pau->m_ucDrcFrameScaleFactor);
                //}
                paudec->m_fhdrdecsts = FHDR_SILENCE;
                break;

            case FHDR_SILENCE:
                // Leading/trailing silence for v3 only
                iResult = WMAB_FALSE;
                /*if (pau->m_iVersion > 2)
                {
                    pau->m_fLeadingSilence = WMAB_FALSE;

                    TRACEWMA_EXIT(wmaResult, ibstrmGetBits (&paudec->m_ibstrm, 1, &iResult));
                }*/

                if (WMAB_FALSE == iResult)
                {
                    // No leading or trailing silence
                    paudec->m_fhdrdecsts = FHDR_ADJUST_TIMESTAMP;
                }
                else
                {
                    // Leading or trailing silence: find out lead/trail or both
                    paudec->m_fhdrdecsts = FHDR_SILENCE_LEAD_FLAG;
                }
                break;

            case FHDR_SILENCE_LEAD_FLAG:
                // We know we need to look at 1 more bits: reserve them, go on hold if not avail
                // Look for leading silence indicator
                TRACEWMA_EXIT(wmaResult, ibstrmGetBits(&paudec->m_ibstrm, 1, &iResult));
                pau->m_fLeadingSilence = iResult;

                if (iResult)
                {
                    // Leading silence: we now need to read the size of leading silence
                    paudec->m_fhdrdecsts = FHDR_SILENCE_LEAD;
                }
                else
                {
                    paudec->m_fhdrdecsts = FHDR_SILENCE_TRAIL_FLAG;
                }

                break;

            case FHDR_SILENCE_LEAD:
                {
                    const I32 cBit = LOG2(pau->m_cFrameSample);
                    iResult = 0;

                    // Read in the amount of leading silence to discard
                    TRACEWMA_EXIT(wmaResult, ibstrmGetBits(&paudec->m_ibstrm, cBit, &iResult));
                    pau->m_u32LeadingSize = iResult;
                }
                paudec->m_fhdrdecsts = FHDR_SILENCE_TRAIL_FLAG;

                break;

            case FHDR_SILENCE_TRAIL_FLAG:
                // Next, check for trailing silence
                TRACEWMA_EXIT(wmaResult, ibstrmGetBits(&paudec->m_ibstrm, 1, &iResult));
                if (iResult)
                {
                    // Trailing silence: we now need to read the end-of-PCM marker
                    paudec->m_fhdrdecsts = FHDR_SILENCE_TRAIL;
                }
                else
                    paudec->m_fhdrdecsts = FHDR_ADJUST_TIMESTAMP;

                break;

            case FHDR_SILENCE_TRAIL:
                {
                    const I32 cBit = LOG2(pau->m_cFrameSample);
                    iResult = 0;

                    // Read in the amount of trailing silence to discard
                    TRACEWMA_EXIT(wmaResult, ibstrmGetBits(&paudec->m_ibstrm, cBit, &iResult));
                    assert(pau->m_u32TrailingSize == 0);
                    pau->m_u32TrailingSize = iResult;
                }
                paudec->m_fhdrdecsts = FHDR_ADJUST_TIMESTAMP;
                break;

            case FHDR_ADJUST_TIMESTAMP:
                paudec->m_fhdrdecsts = FHDR_DONE;
                break;
        }
    }

    //if (pau->m_bPureLosslessMode == WMAB_TRUE)
    //        pau->m_u32ValidSamples = pau->m_cFrameSampleHalf - pau->m_u32TrailingSize;

exit:

    return wmaResult;
} // prvDecodeFrameHeader

WMARESULT prvDecodeSubFrameHeader(CAudioObjectDecoder* paudec)
{
    CAudioObject* pau = paudec->pau;
    PerChannelInfo* ppcinfo = NULL;
    PerChannelInfoDEC* ppcinfoDEC = NULL;
    SubFrameConfigInfo* psubfrmconfig = NULL;

    Bool fSkipAll;
    Bool fUpdateMask = WMAB_FALSE;
    I16  *piChannel = &paudec->m_iChannel;
    Int iMaskQPrev;
    Int iSizeNext = pau->m_cFrameSampleHalf;//isizenext等于帧样本数目
    I16 iCh, iChSrc;

    WMARESULT   wmaResult = WMA_OK;
    U32         iResult;


    while (paudec->m_hdrdecsts != HDR_DONE)
    {
        switch (paudec->m_hdrdecsts)
        {
            case HDR_SIZE:
                //if (pau->m_iVersion <= 2)
                {
                    if (pau->m_fAllowSubFrame)
                    {
                        //peek enough bits for this "case" if not enough will be on hold
                        Int cBitsNeed = LOG2((U32) LOG2((U32)pau->m_iMaxSubFrameDiv)) + 1;
                        TRACEWMA_EXIT(wmaResult, ibstrmGetBits(&paudec->m_ibstrm, cBitsNeed, &iResult));
                        iSizeNext = pau->m_cFrameSampleHalf / (1 << iResult);
                        if (iSizeNext  < pau->m_cMinSubFrameSampleHalf || iSizeNext > pau->m_cFrameSampleHalf)
                        {
                            REPORT_BITSTREAM_CORRUPTION();
                            return TraceResult(WMA_E_BROKEN_FRAME);//isizenext属于[128,2048]
                        }
                    }
                    pau->m_cChInTile = pau->m_cChannel;
                    for (iCh = 0; iCh < pau->m_cChannel; iCh++)
                    {
                        pau->m_rgiChInTile [iCh] = iCh;
                    }
                    wmaResult = prvUpdateSubFrameConfig(paudec, iSizeNext);
                    if (WMA_FAILED(wmaResult))
                    {
                        TraceResult(wmaResult);
                        REPORT_BITSTREAM_CORRUPTION();
                        goto exit;
                    }
                }
                //else
                //{
                //                //search for tile starting position
                //                I32 cSampleLeft = pau->m_cFrameSampleHalf * pau->m_cChannel;
                //                ppcinfo = pau->m_rgpcinfo;
                //                iStartMin = (I16) pau->m_cFrameSampleHalf;
                //                iSizeMin  = (I16) pau->m_cFrameSampleHalf;
                //                for (iCh = 0; iCh < pau->m_cChannel; iCh++)
                //                {
                ////                    I16* piStart = ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameStart + ppcinfo->m_iCurrSubFrame;
                //                    I16* piStart = ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameStart;
                //                    I16* piSize  = ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameSize + ppcinfo->m_iCurrSubFrame;
                //                    if (iStartMin > *piStart)
                //                    {
                //                        iStartMin = (I16) *piStart;
                //                        iSizeMin  = (I16) *piSize;
                //                    }
                //                    ppcinfo++;
                //                }
                //
                //                pau->m_cChInTile = 0;
                //                ppcinfo   = pau->m_rgpcinfo;
                //                for (iCh = 0; iCh < pau->m_cChannel; iCh++)
                //                {
                //                    I16* rgiSize;
                //                    I16* rgiStart;
                //                    I16* piSubFrmCurr;
                //
                //                    psubfrmconfig= ppcinfo->m_rgsubfrmconfig; //curr
                //                    rgiSize      = psubfrmconfig->m_rgiSubFrameSize;
                //                    rgiStart     = psubfrmconfig->m_rgiSubFrameStart;
                //                    piSubFrmCurr = &ppcinfo->m_iCurrSubFrame;
                //
                //                    cSampleLeft -= rgiStart [0]; //covered so far
                //                    //possibly a ch in tile
                ////                    if (iStartMin == rgiStart [*piSubFrmCurr])
                //                    if (iStartMin == *rgiStart)
                //                    {
                //                        if (iSizeMin == rgiSize [*piSubFrmCurr])
                //                        {
                //                            pau->m_rgiChInTile [pau->m_cChInTile] = iCh;
                //                            pau->m_cChInTile++;
                //                            ppcinfo->m_iSizeNext = (I16) rgiSize [*piSubFrmCurr + 1];
                //                            ppcinfo->m_iSizeCurr = (I16) rgiSize [*piSubFrmCurr];
                //                            ppcinfo->m_iSizePrev = (I16) rgiSize [*piSubFrmCurr - 1];
                //                            cSampleLeft -= rgiSize [*piSubFrmCurr];
                //                            *rgiStart += rgiSize [*piSubFrmCurr];
                //                        }
                //                    }
                //                    ppcinfo++;
                //                }
                ////              assert (pau->m_cChInTile <= pau->m_cChannel && pau->m_cChInTile > 0);
                ////              assert (cSampleLeft >= 0);
                //                if (pau->m_cChInTile > pau->m_cChannel || pau->m_cChInTile <= 0 || cSampleLeft < 0) {
                //                    REPORT_BITSTREAM_CORRUPTION();
                //                    wmaResult = TraceResult(WMA_E_BROKEN_FRAME);
                //                    goto exit;
                //                }
                //
                //                paudec->m_fLastSubFrame = (cSampleLeft == 0);
                //            }


                ASSERTWMA_EXIT(wmaResult, auAdaptToSubFrameConfig(pau));
                ASSERTWMA_EXIT(wmaResult, auAdaptToSubFrameConfigDEC(pau));

#ifdef ENABLE_EQUALIZER
                prvAdaptEqToSubFrame(paudec);
#endif    //ENABLE_EQUALIZER


                //if (pau->m_iVersion <= 2)
                {
                    paudec->m_hdrdecsts = HDR_V2_POWER;
                    break;
                }
//            else
                //            {
                //                // V3 LLM. Chao. Add. Coordinate with AnchorMask.
                //
                //                for (iCh = 0; iCh < pau->m_cChInTile; iCh++)
                //                {
                //                    iChSrc = pau->m_rgiChInTile [iCh];
                //                    ppcinfo = pau->m_rgpcinfo + iChSrc;
                //
                //                    // Initializations for mask updates
                //                    if (ppcinfo->m_iCurrSubFrame == 0)
                //                    {
                //                        ppcinfo->m_fAnchorMaskAvailable = WMAB_FALSE;
                //                        ppcinfo->m_iMaskQuantMultiplier = 1;
                //                        memset(ppcinfo->m_rgiMaskQ, 0, sizeof(Int) * NUM_BARK_BAND);
                //                        memset(ppcinfo->m_rgiMaskQResampled, 0, sizeof(Int) * NUM_BARK_BAND);
                //                        ppcinfo->m_iMaxMaskQ = 0;
                //                    }
                //
                //                    prvSetDecTable  (paudec, ppcinfo, 0);
                //                }
                //                paudec->m_hdrdecsts = HDR_V3_ESCAPE;
                //            }

                break;

            case HDR_V3_ESCAPE:
                // V3 Subframe Escape
//            TRACEWMA_EXIT(wmaResult, ibstrmLookForBits(&paudec->m_ibstrm, 1+2+4+15));
//            TRACEWMA_EXIT(wmaResult, ibstrmGetBits(&paudec->m_ibstrm, 1, &iResult));
//            if (0 == iResult)
//                paudec->m_hdrdecsts = HDR_V3_LLM;
//            else
//            {
//                // Find out how many data bits are coming
//                TRACEWMA_EXIT(wmaResult, ibstrmGetBits(&paudec->m_ibstrm, 2, &iResult));
//                if (iResult > 0)
//                {
//                    assert(iResult <= 3);
//                    paudec->m_iSubfrmEscDataBits = iResult;
//                    paudec->m_hdrdecsts = HDR_V3_ESCAPE_SKIPDATA;
//                }
//                else
//                {
//                    I32 iSubframeEscBitsLog2;
//
//                    // First, read in the width (# bits) of next item
//                    TRACEWMA_EXIT(wmaResult, ibstrmGetBits(&paudec->m_ibstrm, 4,
//                        (U32 *)&iSubframeEscBitsLog2));
//
//                    // Read in the number of data bits coming our way
//                    TRACEWMA_EXIT(wmaResult, ibstrmGetBits(&paudec->m_ibstrm,
//                        iSubframeEscBitsLog2, &iResult));
//
//                    // Since zero is invalid # data bits, encoder maps "1" to zero.
//                    // Add that 1 back.
//                    paudec->m_iSubfrmEscDataBits = iResult + 1;
//                    paudec->m_hdrdecsts = HDR_V3_ESCAPE_SKIPDATA;
//                }
//
//            }

                break;


            case HDR_V3_ESCAPE_SKIPDATA:
                // Read 24 bits at a time (LookForBits cannot check for > 24)
//            while (paudec->m_iSubfrmEscDataBits > 0)
                //            {
                //                I32 iCurrReq = min(24, paudec->m_iSubfrmEscDataBits);
                //
                //                TRACEWMA_EXIT(wmaResult, ibstrmGetBits(&paudec->m_ibstrm,
                //                    iCurrReq, &iResult));
                //
                //                paudec->m_iSubfrmEscDataBits -= iCurrReq;
                //            }
                //
                //            assert(0 == paudec->m_iSubfrmEscDataBits); // Done skipping bits
                //            paudec->m_hdrdecsts = HDR_V3_LLM;

                break;

            case HDR_V3_LLM:
//            assert(pau->m_iVersion == 3);
//
//            //peek enough bits for this "case" if not enough will be on hold
//            TRACEWMA_EXIT(wmaResult, ibstrmGetBits (&paudec->m_ibstrm, 1, &iResult));
//            pau->m_bUnifiedLLM = (Bool)iResult;
//            pau->m_bUnifiedPureLLMCurrFrm = WMAB_FALSE;
//
//
//            // Save m_bUnifiedLLM information to every channel.
//            for (iCh = 0; iCh < pau->m_cChInTile; iCh++)
//            {
//                iChSrc = pau->m_rgiChInTile [iCh];
//                ppcinfo = pau->m_rgpcinfo + iChSrc;
////                ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_bUnifiedLLM[ppcinfo->m_iCurrSubFrame] = (U8) pau->m_bUnifiedLLM;
//            }
//
//            if (pau->m_bUnifiedLLM == WMAB_FALSE) {
//                paudec->m_hdrdecsts = HDR_V3_CHXFORM;
//                paudec->m_chxformsts = CHXFORM_BEGIN;
//                pau->m_bSeekable = WMAB_TRUE; // all lossy frames are seekable.
//            }
//            else {
//                pau->m_bSeekable = WMAB_TRUE; // if not in PURE_IN_MLLM mode, all MLLM frames are seekable.
//                paudec->m_hdrdecsts = HDR_PURE_IN_MLLM;
//            }

                break;

            case  HDR_V3_CHXFORM:
                // Multi-channel transform data
//            assert(pau->m_iVersion == 3);
                ////            TRACEWMA_EXIT(wmaResult, prvEntropyDecodeChannelXform(paudec));
                //            paudec->m_hdrdecsts  = HDR_V3_POWER;


            case HDR_V3_POWER:

                // paudec->m_hdrdecsts = HDR_V3_VECCODER_POSEXIST;
                break;

            case HDR_PURE_IN_MLLM:


            case HDR_PURE_IN_MLLM_FIRST_FRM:

                paudec->m_hdrdecsts = HDR_PURE_IN_MLLM_LAST_FRM;

            case HDR_PURE_IN_MLLM_LAST_FRM:

                paudec->m_hdrdecsts  = HDR_PURE_IN_MLLM_SEEKABLE;

            case HDR_PURE_IN_MLLM_SEEKABLE:

            case HDR_V3_RAWPCM_LLM:
                paudec->m_hdrdecsts = HDR_V3_POWER_LLM;

            case HDR_V3_POWER_LLM:
                {
                    paudec->m_hdrdecsts = HDR_ENTROPY_CODING;
                }
                //            break;

            case HDR_ENTROPY_CODING:

                paudec->m_hdrdecsts = HDR_V3_LPC_UNIFIED;

            case HDR_V3_LPC_UNIFIED:

                paudec->m_hdrdecsts = HDR_V3_LPC_ORDER;

            case HDR_V3_LPC_ORDER:

                paudec->m_hdrdecsts = HDR_V3_LPC_UNIFIED_SCALING;

            case HDR_V3_LPC_UNIFIED_SCALING:

                paudec->m_hdrdecsts = HDR_V3_LPC_UNIFIED_INTBITS;

            case HDR_V3_LPC_UNIFIED_INTBITS:

                paudec->m_hdrdecsts = HDR_MLLM_QUANT_STEPSIZE;
//            break;

            case HDR_MLLM_QUANT_STEPSIZE:

                // MLLMUsePLLM doesn't need mask.
                // Mixed lossless must decode mask if it is in the bitstream.
                //if (pau->m_bUnifiedPureLLMCurrFrm == WMAB_TRUE) {
                //                paudec->m_hdrdecsts = HDR_INTERCH_DECORR;
                //            }
                //            else
                {
                    paudec->m_hdrdecsts = HDR_MSKUPD;
                }
                break;

            case HDR_INTERCH_DECORR:

                paudec->m_hdrdecsts = HDR_INTERCH_DECORR_MCLMS;

            case HDR_INTERCH_DECORR_MCLMS:

                paudec->m_hdrdecsts = HDR_CDLMS_SEND;

            case HDR_CDLMS_SEND:

                paudec->m_iChannel = 0;
                paudec->m_hdrdecsts = HDR_FILTERS_PARA;
                paudec->m_hdrdecFilterParasts = HDRFLT_CLMS_AMOUNT;

            case HDR_FILTERS_PARA:
                assert(pau->m_cChInTile == pau->m_cChannel);

                paudec->m_hdrdecsts = HDR_DONE;
                break;

            case HDR_V2_POWER:
                assert(pau->m_cChannel <= 2);
                TRACEWMA_EXIT(wmaResult, ibstrmLookForBits(&paudec->m_ibstrm, pau->m_cChannel + 1));
                if (pau->m_cChannel == 1)
                {
                    TRACEWMA_EXIT(wmaResult, ibstrmGetBits(&paudec->m_ibstrm, 1, &iResult));
                    pau->m_rgpcinfo [0].m_iPower = iResult;

                    fSkipAll = (pau->m_rgpcinfo [0].m_iPower == 0);

                    pau->m_rgpcinfo->m_stereoMode = STEREO_LEFTRIGHT;
                }
                else
                {
                    TRACEWMA_EXIT(wmaResult, ibstrmGetBits(&paudec->m_ibstrm, 1, &iResult));

                    pau->m_rgpcinfo->m_stereoMode = (StereoMode) iResult;//get iStereoMode value=getbits(1)
                    (pau->m_rgpcinfo + 1)->m_stereoMode = (StereoMode) iResult;

                    TRACEWMA_EXIT(wmaResult, ibstrmGetBits(&paudec->m_ibstrm, 1, &iResult));
                    pau->m_rgpcinfo [0].m_iPower = iResult;//get iPower0 value=getbits(1)

                    TRACEWMA_EXIT(wmaResult, ibstrmGetBits(&paudec->m_ibstrm, 1, &iResult));
                    pau->m_rgpcinfo [1].m_iPower = iResult;//get iPower1 value=getbits(1)

                    fSkipAll = (pau->m_rgpcinfo [0].m_iPower == 0) & (pau->m_rgpcinfo [1].m_iPower == 0);
                    if (pau->m_rgpcinfo->m_stereoMode == STEREO_SUMDIFF)
                        prvSetDecTable(paudec,  pau->m_rgpcinfo + 1, 1);  //STEREO_SUMDIFF mode search table 1
                    else
                        prvSetDecTable(paudec,  pau->m_rgpcinfo + 1, 0);  //non-STEREO_SUMDIFF mode search table 0
                }
                // shift it back by the minimum for decode or in case of exit
                // V3 does not come here
                pau->m_iQuantStepSize = MIN_QUANT;//get QuantStepSize value=1+getbits(7)

                if (fSkipAll)
                {
                    // no bits will be read
#ifdef WMAMIDRATELOWRATE
                    if (pau->m_iWeightingMode == LPC_MODE)
                    {
                        // Make sure there is a rightly defined process for dither decoding
                        // Since no bits are read, we do not need additional states for this
                        for (paudec->m_iChannel = 0;
                                paudec->m_iChannel < pau->m_cChInTile;
                                paudec->m_iChannel++)
                        {
                            iChSrc =  pau->m_rgiChInTile [paudec->m_iChannel];
                            ppcinfo = pau->m_rgpcinfo + iChSrc;

                            if (ppcinfo->m_iCurrSubFrame > 0)
                            {
                                // Anchor mask is most likely already available: resample it
                                ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgfMaskUpdate [0] = WMAB_FALSE;
                            }
                            else
                            {
                                // Anchor mask is not available: reset the mask
                                ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgfMaskUpdate [0] = WMAB_TRUE;
                            }
                        }
                        // Set correct channel# for prvDecodeSubFrameHeaderLpc to work.
                        paudec->m_iChannel = 0;
                        TRACEWMA_EXIT(wmaResult, prvDecodeSubFrameHeaderLpc(paudec));
                    }
#endif

                    paudec->m_hdrdecsts = HDR_DONE;
                    goto exit;
                }
                paudec->m_hdrdecsts = HDR_QUANT;
                break;

            case HDR_V3_VECCODER_POSEXIST:
                //pau->m_fExpVecsInNonRLMode = WMAB_FALSE;
//            if (pau->m_iVersion == 3)
                //            {
                //                TRACEWMA_EXIT(wmaResult, ibstrmGetBits(&paudec->m_ibstrm, 1, &iResult));
                //                if (iResult != 0) pau->m_fExpVecsInNonRLMode = WMAB_TRUE;
                //            }

//            *piChannel = 0;
                //            paudec->m_hdrdecsts = HDR_V3_VECCODER_POS;


            case HDR_V3_VECCODER_POS:
//            if (pau->m_fExpVecsInNonRLMode == WMAB_TRUE) {
                //                Int nBits;
                //                for (; *piChannel < pau->m_cChInTile; (*piChannel)++) {
                //                    iChSrc = pau->m_rgiChInTile [*piChannel];
                //                    ppcinfo = pau->m_rgpcinfo + iChSrc;
                //                    nBits = (Int)LOG2((ppcinfo->m_cSubbandActual+3)>>2) + 1;
                //                    TRACEWMA_EXIT(wmaResult, ibstrmGetBits (&paudec->m_ibstrm, nBits, &iResult));
                //                    ppcinfo->m_iVecsInNonRLMode = iResult;
                //                }
                //            }
                //            paudec->m_hdrdecsts = HDR_QUANT;


            case HDR_QUANT:
                //if (pau->m_iVersion <= 2)
                {
                    TRACEWMA_EXIT(wmaResult, prvDecodeQuantStepV2(paudec));//calculate QuantStepsize value
                }
                //else
                //{
                //TRACEWMA_EXIT(wmaResult, prvDecodeQuantStepV3(paudec));
                //}
                auUpdateMaxEsc(pau, pau->m_iQuantStepSize);
                *piChannel  = -1;
                // Some sensible initialization for quant modifiers (needed in V3 and up)
                for (iCh = 0; iCh < pau->m_cChInTile; iCh++)
                {
                    iChSrc = pau->m_rgiChInTile [iCh];
                    ppcinfo = pau->m_rgpcinfo + iChSrc;
                    //ppcinfo->m_ucQuantStepModifierIndex = 0;
                    // ppcinfo->m_qstQuantStepModifier = qstCalcQuantStepModifier(0, 0);
                }
                //paudec->m_cBitQuantStepModifierIndex = 0;
                paudec->m_hdrdecsts = HDR_V3_QUANT_MODIFIER;

            case HDR_V3_QUANT_MODIFIER:
                //TRACEWMA_EXIT(wmaResult, prvDecodeQuantStepModifiers(paudec));

//#ifdef ENABLE_ALL_ENCOPT
                paudec->m_iBand     = (I16) pau->m_iFirstNoiseBand;;
//            #endif // ENABLE_ALL_ENCOPT
                *piChannel  = 0;
                paudec->m_hdrdecsts = HDR_NOISE1;

            case HDR_NOISE1 : // Fall into
            case HDR_NOISE2 : // Fall into
//#           ifdef ENABLE_ALL_ENCOPT
#ifdef WMAMIDRATELOWRATE
                if (pau->m_fNoiseSub == WMAB_TRUE)
                {
                    TRACEWMA_EXIT(wmaResult, prvDecodeSubFrameHeaderNoiseSub(paudec));
                }
#endif
//#           endif //ENABLE_ALL_ENCOPT
                paudec->m_hdrdecsts = HDR_MSKUPD;
            case HDR_MSKUPD :
                //if (pau->m_iVersion <= 2)  //v3 is done as part of mask
                {
                    fUpdateMask = WMAB_TRUE;//fmaskupdate=firstsubframe?TRUE:getbits(1)
                    TRACEWMA_EXIT(wmaResult, ibstrmLookForBits(&paudec->m_ibstrm, 1));
                    if (pau->m_fAllowSubFrame && pau->m_rgpcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_cSubFrame > 1)
                    {
                        TRACEWMA_EXIT(wmaResult, ibstrmGetBits(&paudec->m_ibstrm, 1, &iResult));
                        fUpdateMask = iResult;
                        // First subframe must have fUpdateMask. Otherwise it will cause
                        // problem in prvGetBandWeightMidRate (v2, 16000hz) if there is bs corruption.
                        // (assert( iMaskBand == iRsmpBand || (iMaskBand+1) == iRsmpBand )
                        // because the ratio is 4.)
                        if ((pau->m_rgpcinfo->m_iCurrSubFrame == 0) && (fUpdateMask != 1))
                        {
                            REPORT_BITSTREAM_CORRUPTION_AND_EXIT(wmaResult);
                        }
                    }
                }

                //Setup the pointer to the quantized coefficents. This must be done after
                //prvDecodeFrameHeaderNoiseSub since it changes the value of m_cSubbandActual
                for (iCh = 0; iCh < pau->m_cChInTile; iCh++)
                {
                    iChSrc = pau->m_rgiChInTile [iCh];
                    ppcinfo = pau->m_rgpcinfo + iChSrc;
                    ppcinfoDEC = paudec->m_rgpcinfoDEC + iChSrc;
                    /*
                                    ppcinfo->m_rgiCoefQ       = ((I32*) (pau->m_rgiCoefReconOrig
                                                              + DOUBLE(pau->m_fPad2XTransform, (iChSrc + 1) * pau->m_cFrameSampleHalf)))
                                                              - DOUBLE(pau->m_fPad2XTransform, ppcinfo->m_cSubbandActual);
                    */
                    // Offset with ppcinfo->m_cSubFrameSampleHalf- ppcinfo->m_cSubbandActual is necessary
                    // for noise-sub: entropy decoded symbols must be expanded to fit full subframe. To avoid
                    // overwriting shared rgiCoefQ and rgfltCoefRecon (aka rgiCoefRecon): Naveen
                    ppcinfo->m_rgiCoefQ = (I32 *)(ppcinfo->m_rgiCoefRecon +
                                                  (ppcinfo->m_cSubFrameSampleHalf)) -
                                          (ppcinfo->m_cSubbandActual);

                    memset(ppcinfo->m_rgiCoefQ, 0,  sizeof(I32) * (ppcinfo->m_cSubbandActual));

                    // V3 and up: One bit per channel mask presence information
                    ppcinfoDEC->m_fMaskHeaderDone = WMAB_FALSE;
                    ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgfMaskUpdate [0] = (U8) fUpdateMask;
                }

                *piChannel = 0;
                paudec->m_iBand = 0;
                paudec->m_hdrdecsts = HDR_BARK;

            case HDR_BARK ://
                if (pau->m_iWeightingMode == BARK_MODE) //calculate iMaskQ and MaxMaskQ
                {
                    for (; *piChannel < pau->m_cChInTile; (*piChannel)++)
                    {
                        iChSrc = pau->m_rgiChInTile [*piChannel];
                        ppcinfo = pau->m_rgpcinfo + iChSrc;
                        ppcinfoDEC = paudec->m_rgpcinfoDEC + iChSrc;
                        //if (pau->m_iVersion > 2 && ppcinfoDEC->m_fMaskHeaderDone == WMAB_FALSE)
                        //  TRACEWMA_EXIT(wmaResult, prvDecodeMaskHeaderV3_Channel(paudec, iChSrc));

                        fUpdateMask = ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgfMaskUpdate [0];

                        if (ppcinfo->m_iPower != 0 || pau->m_iVersion > 2)
                        {
                            Int* rgiMaskQ = ppcinfo->m_rgiMaskQ;//calculate iMaskq


#ifdef WMA_TABLE_ROOM_VERIFY
                            const U16 *pDecodeTable = (const U16 *)p_g_rgiHuffDecTblMsk; //(pau->m_iVersion <= 2) ? (const U16 *)p_g_rgiHuffDecTblMsk : g_rgunHuffDecTblMaskVLCV3;
#else
                            const U16 *pDecodeTable = g_rgiHuffDecTblMsk;//(pau->m_iVersion <= 2) ? g_rgiHuffDecTblMsk : g_rgunHuffDecTblMaskVLCV3;
#endif
                            if (fUpdateMask == WMAB_TRUE)
                            {
                                {
                                    // Perform simple VLC decoding.
                                    if (pau->m_iVersion == 1)
                                    {
                                        TRACEWMA_EXIT(wmaResult, ibstrmGetBits(&paudec->m_ibstrm,
                                                                               NBITS_FIRST_MASKQUANT, &iResult));//when version=1;calculate QuantStepsize
                                        rgiMaskQ [0] = iResult + MIN_MASKQ_IN_DB_V1;//calculate iMaskQ
                                        paudec->m_iBand++;
                                    }

                                    for (; paudec->m_iBand < pau->m_cValidBarkBand; paudec->m_iBand++)//for each iband to cvalidbarkband {...}
                                    {
                                        Int iDiff;
                                        TRACEWMA_EXIT(wmaResult, huffDecGet(pDecodeTable,
                                                                            &paudec->m_ibstrm, &iResult, (unsigned long *)&iDiff, (unsigned long *)0));

                                        TRACEWMA_EXIT(wmaResult, ibstrmFlushBits(&paudec->m_ibstrm, iResult));   //flush bits used by huffDecGet
                                        iDiff -= MAX_MASKQUANT_DIFF;
                                        assert(iDiff >= -MAX_MASKQUANT_DIFF);
                                        assert(pau->m_iVersion <= 2 || ppcinfo->m_fAnchorMaskAvailable == WMAB_FALSE);

                                        //if (pau->m_iVersion > 2 && paudec->m_iBand == 0)
                                        //    iMaskQPrev = (Int)(FIRST_V3_MASKQUANT / ppcinfo->m_iMaskQuantMultiplier);
                                        //else
                                        iMaskQPrev = (paudec->m_iBand == 0) ? FIRST_V2_MASKQUANT : rgiMaskQ [paudec->m_iBand - 1];

                                        rgiMaskQ [paudec->m_iBand] = iDiff + iMaskQPrev;
//                                    MONITOR_RANGE(gMR_rgiMaskQ, rgiMaskQ[paudec->m_iBand]);
                                    }
                                }
                                pau->m_iSubFrameSizeWithUpdate = ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameSize [ppcinfo->m_iCurrSubFrame];
                                paudec->m_iBand = 0;
                            }

                            //for IntFloat this gets done inside ReconWeightFactor()
                            //Scan For Max Weight
                            if (fUpdateMask)
                            {
                                Int iMax = rgiMaskQ[0];
                                I16 iBand;
                                for (iBand = 1; iBand < pau->m_cValidBarkBand; iBand++)
                                {
                                    if (rgiMaskQ[iBand] > iMax)
                                        iMax = rgiMaskQ[iBand];
                                }
                                ppcinfo->m_iMaxMaskQ = iMax;//calculate MaxMaskQ
                                // Now we have an mask anchor for this channel for temporal delta
                                // decoding in future subframes.
                                ppcinfo->m_fAnchorMaskAvailable = WMAB_TRUE;
                            }
                        }
                        else if (fUpdateMask == WMAB_TRUE)
                        {   //else artifically set to constants since nothing got sent; see comments in msaudioenc.c
                            //0 db = 1.0 for weightfactor
                            memset(ppcinfo->m_rgiMaskQ, 0, pau->m_cValidBarkBand*sizeof(Int));
                            ppcinfo->m_iMaxMaskQ = 0;
                        }
                        /*if (pau->m_iVersion > 2)
                        {
                            if (fUpdateMask == WMAB_TRUE)
                            {
                                // Now that the mask has Make a note of when the mask got updated for this channel
                                ppcinfo->m_cSubFrameSampleHalfWithUpdate = ppcinfo->m_cSubband;
                                ppcinfo->m_cValidBarkBandLatestUpdate = pau->m_cValidBarkBand;
                            }
                        }*/
                    }
                }
                else
                {
#ifdef WMAMIDRATELOWRATE
                    TRACEWMA_EXIT(wmaResult, prvDecodeSubFrameHeaderLpc(paudec));

                    if (pau->m_fNoiseSub == WMAB_TRUE)
                        prvGetBandWeightLowRate(paudec);
#endif
                }

                paudec->m_hdrdecsts = HDR_DONE;//after decoding iMaskQ/MaxMaskQ,decoding of subframe header is end
                break;
        }
    }

exit:

    return wmaResult;
} // prvDecodeSubFrameHeader
#pragma arm section code

#endif

#endif
#endif

