//*@@@+++@@@@******************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//*@@@---@@@@******************************************************************
#include "../include/audio_main.h"
#include "..\wmaInclude\msaudiotemplate.h"

#ifdef AUINV_RECON_CHANNEL

#ifdef AUINV_RECON_CHANNEL_ENC
#define auGetNextRun prvGetNextRunENC
#endif
#ifdef AUINV_RECON_CHANNEL_DEC
#define auGetNextRun (*pau->aupfnGetNextRun)
#endif

#ifdef A_CORE_DECODE
#ifdef WMA_DEC_INCLUDE
//#if defined (BUILD_WMASTD)
///*****************************************************************************************
//
// auReconCoefficentsHighRate  - Integer or IntFloat Version
//
//*****************************************************************************************
#ifdef WMAHIGHRATE
#pragma arm section code = "WmaHighRateCode"

//wchen: this function is not threadsafe!!
WMARESULT auReconCoefficentsHighRate(CAudioObject* pau, Void* pcaller,PerChannelInfo* ppcinfo)
{
    WMARESULT   wmaResult = WMA_OK;

    CoefType* rgiCoefRecon   = (CoefType*) ppcinfo->m_rgiCoefRecon;
    CoefType ctCoefRecon;
    Int iMaskResampleRatio, cValidBarkBandLatestUpdate;
    Int iBark = 0;
    I16* piRecon = &pau->m_iCurrReconCoef;
    INTEGER_ONLY(Int iShift;)
    Int iMaskResampleRatioPow;
    const Int *rgiBarkIndex;
    I16 iHighCutOff = 0;
    I16 iNextBarkIndex = -1;
    Int iHighToBeZeroed;
    QuantFloat qfltQuantizer;   // either a FastFloat or a Float, as appropriate to the build.
    Bool fPrint = (pau->m_iFrameNumber == 17);
    SubFrameConfigInfo* psubfrmconfigCurr = &(ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM]);
//#ifdef PROFILE
//    FunctionProfile fp;
//    FunctionProfileStart(&fp,DECODE_COEFFICENTS_HIGH_RATE_PROFILE);
//#endif

    assert(pau->m_iVersion <= 2);
    assert(!pau->m_fNoiseSub &&  pau->m_iWeightingMode == BARK_MODE);
    assert(psubfrmconfigCurr->m_rgiSubFrameSize [ppcinfo->m_iCurrSubFrame] == ppcinfo->m_cSubFrameSampleHalf);
    assert(0 != ppcinfo->m_cSubFrameSampleHalf);
    assert(pau->m_cFrameSampleHalf <= (1 << 12));
    DEBUG_BREAK_AT_FRAME_DECODE;

    if (pau->m_iSubFrameSizeWithUpdate <= 0 ||
            ppcinfo->m_cSubFrameSampleHalf <= 0)
    {
        REPORT_BITSTREAM_CORRUPTION();
        wmaResult = WMA_E_BROKEN_FRAME;
        CHECKWMA_EXIT(wmaResult);
    }

    iMaskResampleRatio = (pau->m_iSubFrameSizeWithUpdate << 12) /
                         ppcinfo->m_cSubFrameSampleHalf;
    iMaskResampleRatioPow = LOG2(iMaskResampleRatio);
    rgiBarkIndex       = pau->m_rgiBarkIndexOrig + (NUM_BARK_BAND + 1) *
                         LOG2(pau->m_cFrameSampleHalf / pau->m_iSubFrameSizeWithUpdate);
    cValidBarkBandLatestUpdate = pau->m_rgcValidBarkBand [LOG2(pau->m_cFrameSampleHalf / pau->m_iSubFrameSizeWithUpdate)];

    //// DEBUG NOTES below are preceeded by four slashes and typically allow cut and paste so you can view scaled integers as floats
    //// They are on the line below where the calculation occurs - of course, with BUILD_INT_FLOAT, they are unnecessary

    // zero all coefs so we can just skip the many zero ones as we detect them below
    // note 70% of coefficents are zero in High Rate
    // wchen: moved to outside
    //memset (rgiCoefRecon , 0, sizeof (CoefType) * pau->m_cHighCutOff);//(iRun+pau->m_cLowCutOff));

    iBark = 0;
    //pau->m_iLevel = 0; // remove this line -- since GetNextRun guaranteed to return a value for this on success, on failure, don't care, and causes problems since level gets reset when it should not
    iHighCutOff = (I16)(ppcinfo->m_cSubbandActual - 1);//values need to be offset by -1 too
    iNextBarkIndex = -1;

    //Scan for the first bark index = note iRecon is 0 and rgiBarkIndex[1] ==0 for 16000 Hz and 11025 Hz frames with 128 oir 64 samples
    while ((((*piRecon) * iMaskResampleRatio) >> 12) >= rgiBarkIndex [iBark+1])
        ++iBark;

    TRACEWMA_EXIT(wmaResult, auGetNextRun(pcaller, ppcinfo));  //(run,level,isign)=GetnextrunHighrate(); get triplet from bitstream
//    DBG_RUNLEVEL(g_cBitGet-21,pau->m_cRunOfZeros,pau->m_iLevel,pau->m_iSign,fPrint);//according to getted triplet,we get symbol.
    *piRecon += (I16)(pau->m_cRunOfZeros + 1); //iRecon+=RunOfZeros

    while (*piRecon < iHighCutOff && iBark < cValidBarkBandLatestUpdate)  //for k upto Highcutoff and iB upto cValidBarkBand
    {

        //Search for the next bark index
        while ((((*piRecon) * iMaskResampleRatio) >> 12) >= rgiBarkIndex [iBark+1])
            ++iBark;

        if (iBark >= cValidBarkBandLatestUpdate)
        {
            assert(iBark < cValidBarkBandLatestUpdate);
            break;
        }

        // Get the quantStep * (10^(1/16))^(MaxMaskQ-MaskQ[iRecon])
        // as qfltQuantizer.fraction/(1<<(23-qfltQuantizer.exponent))
        // then scale coefficent to give it five FracBits
        if (*piRecon == iNextBarkIndex)
        {
            //Reconstruct the coefficent before getting the next weighting factor if it lies at the end of a bark band
            ctCoefRecon = MULT_QUANT(pau->m_iLevel, qfltQuantizer);

            //// Unsigned Float CoefRecon = ctCoefRecon/(1.0F*(1<<(qfltQuantizer.iFracBits+16-31)))
            INTEGER_ONLY(ctCoefRecon = SCALE_COEF_RECON(ctCoefRecon));
            //// Unsigned Float CoefRecon = ctCoefRecon/32.0F
            VERIFY_DECODED_COEFFICENT(iBark - 1);
            qfltQuantizer = prvWeightedQuantization(pau, ppcinfo, iBark);

            //// Float Quantizer = qfltQuantizer.iFraction/(1024.0F*(1<<(qfltQuantizer-10)))
            MAKE_MASK_FOR_SCALING(qfltQuantizer.iFracBits);
        }
        else
        {
            //Otherwize get the next weighting factor first
            assert(*piRecon > iNextBarkIndex);
            qfltQuantizer = prvWeightedQuantization(pau, ppcinfo, iBark);

            //// Float Quantizer = qfltQuantizer.iFraction/(1024.0F*(1<<(qfltQuantizer-10)))
            MAKE_MASK_FOR_SCALING(qfltQuantizer.iFracBits);
            ctCoefRecon = MULT_QUANT(pau->m_iLevel, qfltQuantizer);

            //// Unsigned Float CoefRecon = ctCoefRecon/(1.0F*(1<<(qfltQuantizer.iFracBits+16-31)))
            INTEGER_ONLY(ctCoefRecon = SCALE_COEF_RECON(ctCoefRecon));
            //// Unsigned Float CoefRecon = ctCoefRecon/32.0F
            VERIFY_DECODED_COEFFICENT(iBark);
        }

        //Calculate the index of the end of this bark band
        if (iMaskResampleRatioPow > 12)
        {
            iNextBarkIndex = (I16)((rgiBarkIndex [iBark + 1] + (1 << (iMaskResampleRatioPow - 13))) >> (iMaskResampleRatioPow - 12));
        }
        else
        {
            iNextBarkIndex = (I16)(rgiBarkIndex [iBark + 1] << (12 - iMaskResampleRatioPow));
        }
        iNextBarkIndex--; //correct by -1
        if (iNextBarkIndex > iHighCutOff)
            iNextBarkIndex = iHighCutOff;

        do
        {
            rgiCoefRecon [*piRecon] = INTEGER_OR_INT_FLOAT((ctCoefRecon ^ pau->m_iSign) - pau->m_iSign,
                                      pau->m_iSign ? -ctCoefRecon : ctCoefRecon);

            //// Float CoefRecon = rgiCoefRecon [iRecon]/32.0F
//            MONITOR_RANGE(gMR_CoefRecon,FLOAT_FROM_COEF(rgiCoefRecon[*piRecon]));
//            MONITOR_COUNT_CONDITIONAL(rgiCoefRecon[*piRecon]==0,gMC_zeroCoefRecon,pau->m_cRunOfZeros);
            TRACEWMA_EXIT(wmaResult, auGetNextRun(pcaller, ppcinfo));
//            DBG_RUNLEVEL(g_cBitGet-21,pau->m_cRunOfZeros,pau->m_iLevel,pau->m_iSign,fPrint);
            *piRecon += (I16)(pau->m_cRunOfZeros + 1);
            if (*piRecon >= iNextBarkIndex)
                break;
            ctCoefRecon = MULT_QUANT(pau->m_iLevel, qfltQuantizer);

            INTEGER_ONLY(ctCoefRecon = SCALE_COEF_RECON(ctCoefRecon));
            VERIFY_DECODED_COEFFICENT(iBark);
        }
        while (WMAB_TRUE);
        iBark++;
    }
    if (*piRecon == iHighCutOff)
    {
        if (*piRecon >= iNextBarkIndex)
        {   // skipped here via a cRunOfZeros past one or more iBark increments
            while (((iBark - 1) < cValidBarkBandLatestUpdate) && (((*piRecon) * iMaskResampleRatio) >> 12) >= rgiBarkIndex [iBark])
                ++iBark;
            if ((iBark - 1) <= cValidBarkBandLatestUpdate)
            {
                qfltQuantizer = prvWeightedQuantization(pau, ppcinfo, iBark - 1);

                //// Float Quantizer = qfltQuantizer.iFraction/(1024.0F*(1<<(qfltQuantizer-10)))
                MAKE_MASK_FOR_SCALING(qfltQuantizer.iFracBits);
            }
        }
        //else
        //{
        //    assert(WMAB_FALSE);
        //}
        ctCoefRecon = MULT_QUANT(pau->m_iLevel, qfltQuantizer);

        //// Unsigned Float CoefRecon = ctCoefRecon/(1.0F*(1<<(qfltQuantizer.iFracBits+16-31)))
        INTEGER_ONLY(ctCoefRecon = SCALE_COEF_RECON(ctCoefRecon));
        //// Unsigned Float CoefRecon = ctCoefRecon/32.0F
        VERIFY_DECODED_COEFFICENT(iBark - 1);
        rgiCoefRecon [*piRecon] = INTEGER_OR_INT_FLOAT((ctCoefRecon ^ pau->m_iSign) - pau->m_iSign,
                                  pau->m_iSign ? -ctCoefRecon : ctCoefRecon);

        //// Float CoefRecon = rgiCoefRecon [iRecon]/32.0F
//        MONITOR_RANGE(gMR_CoefRecon,FLOAT_FROM_COEF(rgiCoefRecon[*piRecon]));
//        MONITOR_COUNT_CONDITIONAL(rgiCoefRecon[*piRecon]==0,gMC_zeroCoefRecon,pau->m_cRunOfZeros);
    }
    if (*piRecon > ppcinfo->m_cSubband)
    {
        REPORT_BITSTREAM_CORRUPTION();
        wmaResult = WMA_E_BROKEN_FRAME;
    }
    CHECKWMA_EXIT(wmaResult);
    assert(iBark <=  cValidBarkBandLatestUpdate);

    // do low cutoff here so there is less branching in the above loop
    if (pau->m_cLowCutOff > 0)
    {
        memset(rgiCoefRecon, 0, sizeof(Int) * pau->m_cLowCutOff);


    }

    //do high cutoff here
    iHighToBeZeroed = sizeof(CoefType) * (ppcinfo->m_cSubbandAdjusted - pau->m_cHighCutOffAdjusted);
    memset(rgiCoefRecon + pau->m_cHighCutOffAdjusted, 0, iHighToBeZeroed);


exit:

    return wmaResult;
}
#pragma arm section code

#endif


#endif // AUINV_RECON_CHANNEL
#endif
#endif

