//*@@@+++@@@@******************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// Contains entropy decoding functions needed exclusively by WMA Std.
// They are not needed for WMA Pro or WMA Lossless.
#include "../include/audio_main.h"
#include "..\wmaInclude\msaudiodec.h"
#include "..\wmaInclude\huffdec.h"
#include "..\wmaInclude\AutoProfile.h"
#include "..\wmaInclude\entropydec.h"
#include "..\wmaInclude\predefine.h"

#ifdef A_CORE_DECODE
#ifdef WMA_DEC_INCLUDE
#if defined(WMAHIGHRATE) || defined(WMAMIDRATELOWRATE)
#pragma arm section code = "WmaHighLowCommonCode"


WMARESULT prvDecodeQuantStepV2(CAudioObjectDecoder* paudec)
{
    WMARESULT wmaResult = WMA_OK;
    CAudioObject* pau = paudec->pau;
    U32         iResult;

    TRACEWMA_EXIT(wmaResult, ibstrmLookForBits(&paudec->m_ibstrm, 3 * NUM_BITS_QUANTIZER));
    while (1)
    {
        Int iStepSize;

        TRACEWMA_EXIT(wmaResult, ibstrmGetBits(&paudec->m_ibstrm,
                                               NUM_BITS_QUANTIZER, &iResult));//iresult=getbits(7),then quantStepSize=1+iresult
        iStepSize = iResult;

        assert(iStepSize <= MSA_MAX_QUANT_ESC);
//        DEBUG_TRACE_QUANT(pau->m_iFrameNumber, pau->m_iCurrSubFrame, iStepSize);
        if (iStepSize != MSA_MAX_QUANT_ESC)
        {
            // the last one...
            pau->m_iQuantStepSize += iStepSize;//quantStepSize=1+istepsize(iresult)
//            MONITOR_RANGE(gMR_iQuantStepSize,pau->m_iQuantStepSize);
            break;
        }
        // escape code, so on to the next level...
        pau->m_iQuantStepSize += MSA_MAX_QUANT_ESC;
    }
exit:
    return wmaResult;
} // prvDecodeQuantStepV2
#pragma arm section code

#endif
//*****************************************************************************************
//
// prvDecodeCoefficientMono
//
//*****************************************************************************************
#ifdef WMAMIDRATELOWRATE
#pragma arm section code = "WmaLowRateCode"

WMARESULT prvDecodeCoefficientMono(CAudioObjectDecoder* paudec,
                                   PerChannelInfo* ppcinfo)
{
    WMARESULT   wmaResult = WMA_OK;



    if (ppcinfo->m_iPower == 0)
        goto exit;

    TRACEWMA_EXIT(wmaResult, prvDecodeRunLevel(paudec, ppcinfo, 0, ppcinfo->m_cSubbandActual));

exit:

    return wmaResult;
} // prvDecodeCoefficientMono


//*****************************************************************************************
//
// prvDecodeCoefficientStereo
//
//*****************************************************************************************
WMARESULT prvDecodeCoefficientStereo(CAudioObjectDecoder* paudec,
                                     PerChannelInfo* ppcinfo)
{
    WMARESULT   wmaResult = WMA_OK;
    CAudioObject* pau = paudec->pau;


    for (; paudec->m_iChannel < pau->m_cChInTile; paudec->m_iChannel++)
    {
        I16 iChSrc = pau->m_rgiChInTile [paudec->m_iChannel];
        ppcinfo = pau->m_rgpcinfo + iChSrc;
        TRACEWMA_EXIT(wmaResult, prvDecodeCoefficientMono(paudec, ppcinfo));

        if (pau->m_iVersion == 1)
        {
//            DEBUG_ONLY(paudec->m_ibstrm.m_cFrmBitCnt += ibstrmBitsLeft(&paudec->m_ibstrm) % 8;)
            ibstrmFlush(&paudec->m_ibstrm);         //no need to support pause mode (always has fraction bytes)
        }
        pau->m_iCurrReconCoef = 0;
        prvResetRunLevelState(paudec); // reset run level decoding
    }

exit:

    return wmaResult;
} // prvDecodeCoefficientStereo
#pragma arm section code

#endif


//*****************************************************************************************
//
// prvGetNextRunDEC
//
//*****************************************************************************************
extern Int g_cBitPut;
#define NANO_C
#ifdef WMAINITIALIZE_HIGH_MID_ALL_USED
#pragma arm section code = "WmaCommonCode"
WMARESULT prvGetNextRunDEC(Void* pobjin,PerChannelInfo* ppcinfo)
{
    U32 uSign  = 0;     //1 for positive 0 for nonpositive
    I32 iSign  = 0;
    U32 iEntry = 0;     //index to the Vlc table entry
    U32 iResult;
    U16 cBitsTotal;
    WMARESULT     wmaResult = WMA_OK;
    CAudioObjectDecoder* paudec  = (CAudioObjectDecoder*) pobjin;
    CAudioObject*        pau = paudec->pau;
    CWMAInputBitStream*  pibstrm = &(paudec->m_ibstrm);
//    DEBUG_ONLY(Bool fPrint = (pau->m_iFrameNumber  == 19 && pau->m_rgpcinfo->m_iCurrSubFrame == 1);)


    switch (paudec->m_rlsts)
    {
        case TBL_INDEX:
            assert(paudec->pau->m_iVersion > 2);
            TRACEWMA_EXIT(wmaResult, ibstrmGetBits(pibstrm, 1, &iResult));

            prvSetDecTable(paudec,  ppcinfo, (I16) iResult);

            paudec->m_rlsts = VLC;
            // Caution: Fall into VLC

        case VLC:
//#if defined(WMA_ENTROPY_TEST) && defined(ARITH_TIME)
//        if (!g_maskTest) { TSTART; }
//#endif
            TRACEWMA_EXIT(wmaResult,
                          huffDecGet(ppcinfo->m_rgiHuffDecTbl, pibstrm,
                                     &iResult, &iEntry, &uSign));
//#if defined(WMA_ENTROPY_TEST) && defined(ARITH_TIME)
//        if (!g_maskTest) { TSTOP; }
//        if (!g_maskTest) { g_huffTime += TGET(); }
//#endif
            TRACEWMA_EXIT(wmaResult, ibstrmFlushBits(pibstrm, iResult));   //flush bits used by huffDecGet
            if (iEntry == 0)
            {
                //if (pau->m_iVersion <= 2)
                {
                    //paudec->m_rlsts = ESCAPE;
                    paudec->m_rlsts = ESCAPE_LEVEL;
                    //           goto esc;
                    goto escl;
                }
//            else
                //            {
                //                paudec->m_rlsts = ESCAPE_LEVEL;
                //                pau->m_iSign = 0;
                //                goto escl;
                //            }

            }
            if (iEntry == 1)
            {
                /*
                // [1], end of coefs for this subFrame
                *pcRunOfZeros = ppcinfo->m_cSubbandActual - iCoef - 1;
                *piCurrLevel = 0;
                wmaResult = WMA_OK;
                */
                pau->m_iLevel = 0;
                pau->m_cRunOfZeros = (I16)(ppcinfo->m_cSubbandActual - pau->m_iCurrReconCoef - 1);
            }
            else
            {
                assert(iEntry >= 2);
                TRACEWMA_EXIT(wmaResult, ibstrmFlushBits(pibstrm, 1));   //flush the sign bit also
	#ifdef NANO_C
		      pau->m_cRunOfZeros = ppcinfo->m_getRunValue(ppcinfo->m_rgiRunEntry, iEntry - 2) & 0x7FFFFFFF;
		      assert (pau->m_cRunOfZeros >= 0);
	        	pau->m_iLevel = ppcinfo->m_getLevelValue(ppcinfo->m_rgiLevelEntry, iEntry - 2);
	#else
		  pau->m_cRunOfZeros = ppcinfo->m_rgiRunEntry [iEntry - 2] & 0x7FFFFFFF;
		  assert (pau->m_cRunOfZeros >= 0);
		  pau->m_iLevel = ppcinfo->m_rgiLevelEntry [iEntry - 2];
	#endif

                pau->m_iSign = (I32)(uSign >> 31) - 1;
            }
            break;

//    case ESCAPE:
//esc:
//        cBitsTotal = (U16) ( pau->m_iMaxEscSize + pau->m_cBitsSubbandMax + 1 ); //<=27=13+13+1
//        assert (cBitsTotal <= 27);
//
//        TRACEWMA_EXIT(wmaResult, ibstrmGetBits (pibstrm, cBitsTotal, &iResult));
//
//        cBitsTotal = (U16) ( paudec->pau->m_cBitsSubbandMax + 1 );
//        pau->m_iLevel = iResult >> cBitsTotal;
//        pau->m_iSign = (I32) (iResult & 0x00000001) - 1;
//        //*piCurrLevel = (I16) ((*piCurrLevel ^ iSign) - iSign);
//
//        pau->m_cRunOfZeros = (I16) (iResult & (0xFFFFFFFF >> (32 - cBitsTotal))) >> 1;
//        pau->m_cRunOfZeros &= 0x7FFFFFFF;
//        paudec->m_rlsts = VLC;
//        break;

        case ESCAPE_LEVEL:
escl:
            //if (pau->m_iVersion <= 2)
            {
                cBitsTotal = (U16)(pau->m_iMaxEscSize);    //<=13
                TRACEWMA_EXIT(wmaResult, ibstrmGetBits(pibstrm, cBitsTotal, &iResult));

                pau->m_iLevel = iResult;
                paudec->m_rlsts = ESCAPE_RUN;

            }//else{
            //        if (pau->m_iSign == 0)
            //        {
            //            U32 iMaskBit = 0;
            //            //need to generalize the decoder
            //            TRACEWMA_EXIT(wmaResult, ibstrmPeekBits(pibstrm, 4, &iResult));
            //
            //            //reuse two vars
            //            pau->m_iLevel = 0; //upper bound; need to be persistent
            //            pau->m_iSign = 4;  //size;        need to be persistent
            //            if ((iResult & (0x80000000 >> iMaskBit)) != 0)
            //            {
            //                pau->m_iLevel += 1 << pau->m_iSign;
            //                pau->m_iSign += 4;
            //                iMaskBit++;
            //                while ((iResult & (0x80000000 >> iMaskBit)) != 0)
            //                {
            //                    pau->m_iLevel += 1 << pau->m_iSign;
            //                    pau->m_iSign += 8;
            //                    iMaskBit++;
            //                }
            //            }
            //            TRACEWMA_EXIT(wmaResult, ibstrmFlushBits(pibstrm, iMaskBit+1));
            //        }
            //        TRACEWMA_EXIT(wmaResult, ibstrmGetBits (pibstrm, pau->m_iSign + 1, &iResult));
            //        pau->m_iLevel += (iResult >> 1) + 1;
            //        pau->m_iSign = (I32) (iResult & 0x00000001) - 1;
            //        paudec->m_rlsts = ESCAPE_RUN;
            //        } // version > 2

        case ESCAPE_RUN:

            //if (pau->m_iVersion <= 2)
            {
                cBitsTotal = (U16)(pau->m_cBitsSubbandMax + 1);   //<=13+1
                TRACEWMA_EXIT(wmaResult, ibstrmGetBits(pibstrm, cBitsTotal, &iResult));

                cBitsTotal = (U16)(paudec->pau->m_cBitsSubbandMax + 1);

                pau->m_iSign = (I32)(iResult & 0x00000001) - 1;
                //*piCurrLevel = (I16) ((*piCurrLevel ^ iSign) - iSign);

                pau->m_cRunOfZeros = (I16)(iResult & (0xFFFFFFFF >> (32 - cBitsTotal))) >> 1;
                pau->m_cRunOfZeros &= 0x7FFFFFFF;
                paudec->m_rlsts = VLC;
                break;
            }//else{
            //
            //        //TRACEWMA_EXIT(wmaResult,
            //        //  prvEscapeDecodeRunV3((Int*)&iResult, (Int)paudec->m_cMaxRun,
            //       //                        pibstrm));
            //        pau->m_cRunOfZeros = (I16) iResult;
            //        if (pau->m_cRunOfZeros == 0) pau->m_iLevel += paudec->m_iMaxLevelInVlc;
            //        paudec->m_rlsts = VLC;
            //        }

    }


exit:


    //DBG_RUNLEVEL_DEC(g_cBitGet-21,pau->m_cRunOfZeros,pau->m_iLevel,fPrint);
    return wmaResult;
} // prvGetNextRunDEC
#pragma arm section code
#endif



//*****************************************************************************************
//
// prvDecodeRunLevel
//
//*****************************************************************************************

#ifdef WMAMIDRATELOWRATE
#pragma arm section code = "WmaLowRateCode"

WMARESULT prvDecodeRunLevel(CAudioObjectDecoder* paudec,
                            PerChannelInfo* ppcinfo,
                            Int iCoefFirst,
                            Int iCoefLim)
{
    I32* rgiCoefQ       = ppcinfo->m_rgiCoefQ;

  #ifdef NANO_C
    const U8* rgiRunEntry    = ppcinfo->m_rgiRunEntry;
    const U8* rgiLevelEntry  = ppcinfo->m_rgiLevelEntry;
   #else
    const U16* rgiRunEntry    = ppcinfo->m_rgiRunEntry;
    const U16* rgiLevelEntry  = ppcinfo->m_rgiLevelEntry;
   #endif
    CAudioObject* pau = paudec->pau;
    I16* piCoef = &pau->m_iCurrReconCoef;

    WMARESULT   wmaResult = WMA_OK;

    // This function is too frequently called to be profiled
    //FUNCTION_PROFILE(fp);
    //FUNCTION_PROFILE_START(&fp,DECODE_RUN_LEVEL_PROFILE);
    paudec->m_cMaxRun = (I16) LOG2(iCoefLim - 1) + 1;
    while (*piCoef < iCoefLim)
    {
        TRACEWMA_EXIT(wmaResult, prvGetNextRunDEC(paudec, ppcinfo));
        pau->m_iLevel = ((pau->m_iLevel ^ pau->m_iSign) - pau->m_iSign);
//        DBG_RUNLEVEL_DEC(g_cBitGet-21,pau->m_cRunOfZeros,pau->m_iLevel,fPrint);
        if ((*piCoef + pau->m_cRunOfZeros) >= iCoefLim)
        {
            REPORT_BITSTREAM_CORRUPTION();
            wmaResult = TraceResult(WMA_E_BROKEN_FRAME);
            goto exit;
        }
//        memset (rgiCoefQ + *piCoef, 0, pau->m_cRunOfZeros * sizeof (I16));
        *piCoef += (I16) pau->m_cRunOfZeros;
        assert(pau->m_iLevel < (1 << 16));
        rgiCoefQ [*piCoef] = (U32)pau->m_iLevel;
//        MONITOR_RANGE(gMR_iCoefQ,rgiCoefQ [*piCoef]);
//        MONITOR_COUNT_CONDITIONAL(pau->m_cRunOfZeros>0,gMC_0CoefQ,pau->m_cRunOfZeros);
        (*piCoef)++;
        paudec->m_rlsts = VLC;
    }

exit:

    //FUNCTION_PROFILE_STOP(&fp);
    return wmaResult;
} // prvDecodeRunLevel


#if 0
extern const U16 gRun44ssQb_cmp[];
extern const U16 gRun44smQb_cmp[];
extern const U16 gRun44ssOb_cmp[];
extern const U16 gLevel44smOb_cmp[];

void VerifyRunLevelTbl()
{
	I32 i;

	for(i = 0; i < 553; i++)
	{
		if(gRun44ssQb_cmp[i] != getRunLevelValue44ssQb(gRun44ssQb, i))
		{
			DEBUG("getRunLevelValue44ssQb error.\n");
			assert(0);
		}
	}

	for(i = 0; i < 664; i++)
	{
		if(gRun44smQb_cmp[i] != getRunLevelValue44smQb(gRun44smQb, i))
		{
			DEBUG("getRunLevelValue44smQb error.\n");
			assert(0);
		}
	}

	for(i = 0; i < 1070; i++)
	{
		if(gRun44ssOb_cmp[i] != getRunLevelValue44ssOb(gRun44ssOb, i))
		{
			DEBUG("getRunLevelValue44ssOb error.\n");
			assert(0);
		}
	}

	for(i = 0; i < 1334; i++)
	{
		if(gLevel44smOb_cmp[i] != getRunLevelValue44smOb(gLevel44smOb, i))
		{
			DEBUG("getRunLevelValue44smOb error.\n");
			assert(0);
		}
	}
}
#endif


#pragma arm section code

#endif


//*****************************************************************************************
//
// prvSetDetTable
//
//*****************************************************************************************
#ifdef WMAINITIALIZE_HIGH_MID_ALL_USED
#pragma arm section code = "WmaCommonCode"

I32 getRunLevelValue(const U8 *tbl, U32 idx)
{
	return tbl[idx];
}

I32 getRunLevelValue44ssQb(const U8 *tbl, U32 idx)
{
	if(idx < 256)
		return tbl[idx];
	else if(idx > 310)
		return tbl[idx];
	else
		return tbl[idx] + 256;
}

I32 getRunLevelValue44smQb(const U8 *tbl, U32 idx)
{
	if(idx < 256)
		return tbl[idx];
	else if(idx > 316)
		return tbl[idx];
	else
		return tbl[idx] + 256;
}

I32 getRunLevelValue44ssOb(const U8 *tbl, U32 idx)
{
	if(idx < 256)
		return tbl[idx];
	else if(idx > 350)
		return tbl[idx];
	else
		return tbl[idx] + 256;
}

I32 getRunLevelValue44smOb(const U8 *tbl, U32 idx)
{
	if(idx < 1249)
		return tbl[idx];
	else
		return tbl[idx] + 256;
}


Void prvSetDecTable(CAudioObjectDecoder* paudec,  PerChannelInfo* ppcinfo, I16 iSet)
{
    CAudioObject* pau = paudec->pau;
    /*** less than 0.5%
    #ifdef PROFILE
        //FunctionProfile fp;
        //FunctionProfileStart(&fp,SET_DET_TABLE_PROFILE);
    #endif
    *///
    //pau->m_iEntropyMode how to be computed?,the value determinted to choose which huffman table.
    if (pau->m_iEntropyMode == SIXTEENS_OB)
    {
        if (iSet == 1)
        {
#ifdef WMA_TABLE_ROOM_VERIFY
#ifdef NANO_C
	     ppcinfo->m_rgiHuffDecTbl = g_rgiHuffDecTbl16ssOb;
            ppcinfo->m_rgiRunEntry = p_gRun16ssOb;    
            ppcinfo->m_rgiLevelEntry = p_gLevel16ssOb; 
#else
	     ppcinfo->m_rgiHuffDecTbl = (const U16*)p_g_rgiHuffDecTbl16ssOb;
	     ppcinfo->m_rgiRunEntry = (const U16*)p_gRun16ssOb;    
            ppcinfo->m_rgiLevelEntry = (const U16*)p_gLevel16ssOb; 
#endif

#else
            ppcinfo->m_rgiHuffDecTbl = g_rgiHuffDecTbl16ssOb;//huffman-RLC-16-Diff's sets ofsymbol value and length
            ppcinfo->m_rgiRunEntry = gRun16ssOb;   //huffman-RLC-16-Diff's Zero Run value table
            ppcinfo->m_rgiLevelEntry = gLevel16ssOb;//huffman-RLC-16-Diff's abs(level) value  table
#endif
#ifdef NANO_C
	  ppcinfo->m_getRunValue = getRunLevelValue;
	  ppcinfo->m_getLevelValue = getRunLevelValue;
#endif

            paudec->m_iMaxLevelInVlc = MAX_RUNLEVEL_16SS_OB;

        }
        else
        {
#ifdef WMA_TABLE_ROOM_VERIFY
#ifdef NANO_C
	    ppcinfo->m_rgiHuffDecTbl = g_rgiHuffDecTbl16smOb; 
           ppcinfo->m_rgiRunEntry = gRun16smOb;    
            ppcinfo->m_rgiLevelEntry = gLevel16smOb; 
#else
	    ppcinfo->m_rgiHuffDecTbl = (const U16*)p_g_rgiHuffDecTbl16smOb; 
	    ppcinfo->m_rgiRunEntry = (const U16*)p_gRun16smOb;    
            ppcinfo->m_rgiLevelEntry = (const U16*)p_gLevel16smOb; 
#endif

#else
            ppcinfo->m_rgiHuffDecTbl = g_rgiHuffDecTbl16smOb;
            ppcinfo->m_rgiRunEntry = gRun16smOb;       //huffman-RLC-16-Mono's Zero Run value table
            ppcinfo->m_rgiLevelEntry = gLevel16smOb;   //huffman-RLC-16-Mono's abs(level) value table
#endif
#ifdef NANO_C
				ppcinfo->m_getRunValue = getRunLevelValue;
				ppcinfo->m_getLevelValue = getRunLevelValue;
#endif

            paudec->m_iMaxLevelInVlc = MAX_RUNLEVEL_16SM_OB;

        }
    }
//#ifdef ENABLE_ALL_ENCOPT
    else if (pau->m_iEntropyMode == FOURTYFOURS_QB)
    {
        if (iSet == 1)
        {
#ifdef WMA_TABLE_ROOM_VERIFY
#ifdef NANO_C
			ppcinfo->m_rgiHuffDecTbl = g_rgiHuffDecTbl44ssQb;
            ppcinfo->m_rgiRunEntry = (const U8*)p_gRun44ssQb;    
            ppcinfo->m_rgiLevelEntry = (const U8*)p_gLevel44ssQb;
#else
			ppcinfo->m_rgiHuffDecTbl = (const U16*)p_g_rgiHuffDecTbl44ssQb;
			ppcinfo->m_rgiRunEntry = (const U16*)p_gRun44ssQb;    
            ppcinfo->m_rgiLevelEntry = (const U16*)p_gLevel44ssQb;
#endif

#else
            ppcinfo->m_rgiHuffDecTbl = g_rgiHuffDecTbl44ssQb;
            ppcinfo->m_rgiRunEntry = gRun44ssQb;
            ppcinfo->m_rgiLevelEntry = gLevel44ssQb;
#endif
#ifdef NANO_C
				ppcinfo->m_getRunValue = getRunLevelValue44ssQb;
				ppcinfo->m_getLevelValue = getRunLevelValue;
#endif

            paudec->m_iMaxLevelInVlc = MAX_RUNLEVEL_44SS_QB;

        }
        else
        {
#ifdef WMA_TABLE_ROOM_VERIFY
#ifdef NANO_C
	   ppcinfo->m_rgiHuffDecTbl = g_rgiHuffDecTbl44smQb; 
            ppcinfo->m_rgiRunEntry = p_gRun44smQb;    
            ppcinfo->m_rgiLevelEntry = p_gLevel44smQb;
#else
		ppcinfo->m_rgiHuffDecTbl = (const U16*)p_g_rgiHuffDecTbl44smQb; 
		ppcinfo->m_rgiRunEntry = (const U16*)p_gRun44smQb;    
            ppcinfo->m_rgiLevelEntry = (const U16*)p_gLevel44smQb;
#endif

#else
            ppcinfo->m_rgiHuffDecTbl = g_rgiHuffDecTbl44smQb;
            ppcinfo->m_rgiRunEntry = gRun44smQb;
            ppcinfo->m_rgiLevelEntry = gLevel44smQb;
#endif
#ifdef NANO_C
				ppcinfo->m_getRunValue = getRunLevelValue44smQb;
				ppcinfo->m_getLevelValue = getRunLevelValue;
#endif	

            paudec->m_iMaxLevelInVlc = MAX_RUNLEVEL_44SM_QB;

        }
    }
    else if (pau->m_iEntropyMode == FOURTYFOURS_OB)
    {
        if (iSet == 1)
        {
#ifdef WMA_TABLE_ROOM_VERIFY
#ifdef NANO_C
					ppcinfo->m_rgiHuffDecTbl = g_rgiHuffDecTbl44ssOb;
					ppcinfo->m_rgiRunEntry = p_gRun44ssOb;
					ppcinfo->m_rgiLevelEntry = p_gLevel44ssOb;
		#else
					ppcinfo->m_rgiHuffDecTbl = (const U16*)p_g_rgiHuffDecTbl44ssOb;
					ppcinfo->m_rgiRunEntry = (const U16*)p_gRun44ssOb;
					ppcinfo->m_rgiLevelEntry = (const U16*)p_gLevel44ssOb;
		#endif
#else
            ppcinfo->m_rgiHuffDecTbl = g_rgiHuffDecTbl44ssOb;
            ppcinfo->m_rgiRunEntry = gRun44ssOb;
            ppcinfo->m_rgiLevelEntry = gLevel44ssOb;
#endif
#ifdef NANO_C
				ppcinfo->m_getRunValue = getRunLevelValue44ssOb;
				ppcinfo->m_getLevelValue = getRunLevelValue;
#endif

            paudec->m_iMaxLevelInVlc = MAX_RUNLEVEL_44SS_OB;

        }
        else
        {
#ifdef WMA_TABLE_ROOM_VERIFY
         	#ifdef NANO_C
			ppcinfo->m_rgiHuffDecTbl = (const U16*)p_g_rgiHuffDecTbl44smOb;
            ppcinfo->m_rgiRunEntry = (const U8*)p_gRun44smOb;
            ppcinfo->m_rgiLevelEntry = (const U8*)p_gLevel44smOb;
			#else
			ppcinfo->m_rgiHuffDecTbl = (const U16*)p_g_rgiHuffDecTbl44smOb;
			ppcinfo->m_rgiRunEntry = (const U16*)p_gRun44smOb;
            ppcinfo->m_rgiLevelEntry = (const U16*)p_gLevel44smOb;
			#endif
#else
            ppcinfo->m_rgiHuffDecTbl = g_rgiHuffDecTbl44smOb;
            ppcinfo->m_rgiRunEntry = gRun44smOb;
            ppcinfo->m_rgiLevelEntry = gLevel44smOb;
#endif
	#ifdef NANO_C
						ppcinfo->m_getRunValue = getRunLevelValue;
						ppcinfo->m_getLevelValue = getRunLevelValue44smOb;
		#endif		

            paudec->m_iMaxLevelInVlc = MAX_RUNLEVEL_44SM_OB;

        }
    }
//#endif // ENABLE_ALL_ENCOPT
    /***
    #ifdef PROFILE
        //FunctionProfileStop(&fp);
    #endif
    *///
}
#pragma arm section code
#endif

//#if defined(ENABLE_ALL_ENCOPT) && defined(ENABLE_LPC)


#ifdef WMAMIDRATELOWRATE
#pragma arm section code = "WmaLowRateCode"

WMARESULT prvDecodeSubFrameHeaderLpc(CAudioObjectDecoder* paudec)
{
    CAudioObject* pau = paudec->pau;
    I16* piChannel = &paudec->m_iChannel;
    PerChannelInfo *ppcinfo;
    Bool fUpdateMask;
    I16 iChSrc;
    // U8 rgiLspFreqQ[LPCORDER];  ONHOLD will lose this, use WeightFactor instead
    I16* piFreq = &paudec->m_iBand;  // preserve this across ONHOLD
    Int cBitAlloc;
    WMARESULT   wmaResult = WMA_OK;
    LpType rgLpcCoef [LPCORDER];
    //FUNCTION_PROFILE(fp);
    //FUNCTION_PROFILE_START(&fp,FRAME_HEADER_LPC_PROFILE);

//    DEBUG_BREAK_AT_FRAME_LPC_WEIGHTS;

    for (; *piChannel < pau->m_cChInTile; (*piChannel)++)
    {
        iChSrc =  pau->m_rgiChInTile [*piChannel];
        ppcinfo = pau->m_rgpcinfo + iChSrc;
        fUpdateMask = ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgfMaskUpdate [0];
        if (ppcinfo->m_iPower != 0)
        {
            if (fUpdateMask == WMAB_TRUE)
            {
                // these 10 bytes are saved in the WeightFactor array in case we go ONHOLD.
                // WeightFactor will be set by rgiLspFreqQ
                U8* rgiLspFreqQ = (U8*)(INTEGER_OR_INT_FLOAT(ppcinfo->m_rguiWeightFactor, ppcinfo->m_rgfltWeightFactor));
                for (; *piFreq < LPCORDER; (*piFreq)++)
                {
                    U32         iResult;
                    cBitAlloc = (*piFreq == 0 || *piFreq == 8 || *piFreq == 9) ? 3 : 4;

                    TRACEWMA_EXIT(wmaResult, ibstrmGetBits(&paudec->m_ibstrm,
                                                           cBitAlloc, &iResult));
                    rgiLspFreqQ[*piFreq] = (U8) iResult;

//                    MONITOR_RANGE(gMR_rgiLspFreqQ, rgiLspFreqQ[*piFreq]);
                }
                auLsp2lpc(pau, rgiLspFreqQ, rgLpcCoef, LPCORDER);
                TRACEWMA_EXIT(wmaResult, prvLpcToSpectrum(pau,
                              rgLpcCoef, ppcinfo));
//                SHOW_OWF(*piChannel,0);
            }
            else if (ppcinfo->m_iCurrSubFrame > 0)
            {
                auResampleWeightFactorLPC(pau, ppcinfo);
//                SHOW_OWF(*piChannel,1);
            }
        }
        else if (fUpdateMask == WMAB_TRUE)
        {   //artifically set to constants since nothing got sent for this channel; see comments in msaudioenc.c
            FinalWeightType* pwtWeightFactor = (FinalWeightType*) ppcinfo->m_rguiWeightFactor;
            I16 iFreq;
            ppcinfo->m_wtMaxWeight = FINALWEIGHT_FROM_FLOAT(1.0F);
            for (iFreq = (I16) ppcinfo->m_cSubband; iFreq > 0; iFreq--)
                *pwtWeightFactor++ = FINALWEIGHT_FROM_FLOAT(1.0F);
//            SHOW_OWF(*piChannel,2);
        }
        else if (ppcinfo->m_iCurrSubFrame > 0)
        {   // resample even if zero power, these weights might be used in a later subframe with non-zero power
            auResampleWeightFactorLPC(pau, ppcinfo);
//            SHOW_OWF(*piChannel,3);
        }
        *piFreq = 0;
    }

exit:
    //FUNCTION_PROFILE_STOP(&fp);
    return wmaResult;
}

//#ifdef ENABLE_ALL_ENCOPT
WMARESULT prvDecodeSubFrameHeaderNoiseSub(CAudioObjectDecoder* paudec)
{
    U8* rgbBandNotCoded;
    Int iStart, iEnd;
    Int iCode;
    U32 cBitUsed;
    I16* piChannel = &paudec->m_iChannel;
    I16* piBand = &paudec->m_iBand;
    CAudioObject* pau = paudec->pau;

    WMARESULT   wmaResult = WMA_OK;
    U32         iResult;
    //FUNCTION_PROFILE(fp);
    //FUNCTION_PROFILE_START(&fp,FRAME_HEADER_NOISE_SUB_PROFILE);

    switch (paudec->m_hdrdecsts)
    {
        case HDR_NOISE1:
            //noise band indicator
            for (; *piChannel < pau->m_cChannel; (*piChannel)++)
            {
                PerChannelInfo* ppcinfo = pau->m_rgpcinfo + *piChannel;
                TRACEWMA_EXIT(wmaResult, ibstrmLookForBits(&paudec->m_ibstrm,
                              pau->m_cValidBarkBand - pau->m_iFirstNoiseBand));
                if (ppcinfo->m_iPower != 0)
                {
                    rgbBandNotCoded = ppcinfo->m_rgbBandNotCoded;
                    *rgbBandNotCoded = 0;
                    ppcinfo->m_cSubbandActual = pau->m_iFirstNoiseIndex - pau->m_cLowCutOff;
                    iStart = 0, iEnd = 0;
                    while (WMAB_TRUE)
                    {
                        iStart = max(pau->m_iFirstNoiseIndex, pau->m_rgiBarkIndex [*piBand]);
                        if (iStart >= pau->m_cHighCutOff)
                            break;
                        iEnd   = min(pau->m_cHighCutOff, pau->m_rgiBarkIndex [*piBand + 1]);

                        TRACEWMA_EXIT(wmaResult, ibstrmGetBits(&paudec->m_ibstrm, 1, &iResult));
                        rgbBandNotCoded [*piBand] = (U8) iResult;

                        if (rgbBandNotCoded [*piBand] != 0)
                            rgbBandNotCoded [0]++;
                        else
                            ppcinfo->m_cSubbandActual += iEnd - iStart;
                        (*piBand)++;
                    }
                    if (*piBand < pau->m_cValidBarkBand)
                        rgbBandNotCoded [*piBand] = (U8) 0;     // protect from debug verify code looking one band past end
                }
                else
                    ppcinfo->m_cSubbandActual = 0;
                assert(ppcinfo->m_cSubbandActual >= 0 &&
                       ppcinfo->m_cSubbandActual <= pau->m_cHighCutOff - pau->m_cLowCutOff);
                *piBand = (I16) pau->m_iFirstNoiseBand;
            }
            paudec->m_hdrdecsts = HDR_NOISE2;
            *piChannel = 0;
            //I guess
            *piBand = 0;

        case HDR_NOISE2:
            //noise power
            for (; *piChannel < pau->m_cChannel; (*piChannel)++)
            {
                PerChannelInfo* ppcinfo = pau->m_rgpcinfo + *piChannel;
                if (ppcinfo->m_iPower != 0)
                {
                    Int* rgiNoisePower   = ppcinfo->m_rgiNoisePower;

                    rgbBandNotCoded      = ppcinfo->m_rgbBandNotCoded;
                    if (rgbBandNotCoded [0] > 0)
                    {
                        if (*piBand == 0)
                        {
                            TRACEWMA_EXIT(wmaResult, ibstrmGetBits(&paudec->m_ibstrm, 7, &iResult));
                            rgiNoisePower [0] = iResult - 19;
                            (*piBand)++;
                        }

                        for (; *piBand < rgbBandNotCoded [0]; (*piBand)++)
                        {
#ifdef WMA_TABLE_ROOM_VERIFY
                            TRACEWMA_EXIT(wmaResult, huffDecGet((const U16 *)p_g_rgiHuffDecTblNoisePower,
                                                                &paudec->m_ibstrm, &cBitUsed, (unsigned long *)&iCode, (unsigned long *)0));
#else
                            TRACEWMA_EXIT(wmaResult, huffDecGet(g_rgiHuffDecTblNoisePower,
                                                                &paudec->m_ibstrm, &cBitUsed, (unsigned long *)&iCode, (unsigned long *)0));
#endif
                            TRACEWMA_EXIT(wmaResult, ibstrmFlushBits(&paudec->m_ibstrm, cBitUsed));     //flush bits used by huffDecGet
                            rgiNoisePower [*piBand] = iCode - 18 + rgiNoisePower [*piBand - 1];

//                            MONITOR_RANGE(gMR_rgiNoisePower,rgiNoisePower[*piBand]);
                        }
                    }
                }
                *piBand = 0;
            }
    }

exit:
//    FUNCTION_PROFILE_STOP(&fp);
    return wmaResult;
}
#pragma arm section code

#endif
//******************************************************************************************
#ifdef WMAMIDRATELOWRATE
#pragma arm section code = "WmaLowRateCode"

#define U64_SQUARE_I32(x) (((U64)(x)) * ((U64)(x)))

FastFloat ffltAvgSquareWeights(WeightType* pwtWeightFactor, Int cN)
{
    U64 u64Sum = 0;
    FastFloat fflt;
    Int i = cN;
    for (; i > 0; i--, pwtWeightFactor++)
    {
        u64Sum += U64_SQUARE_I32(*pwtWeightFactor);
    }
    // convert to a fastfloat and then divide.  This avoids an int64 division which is
    // really slow on the sh 3 and quite slow on other risc/dsp machines
    // cN varies from 1 to 328 so this can lose 8.3 bits of precision.
    // but we are comparing to float implementation which has only 24 bits of mantissa.
    fflt = FASTFLOAT_FROM_U64(u64Sum, (WEIGHTFACTOR_FRACT_BITS << 1));
    fflt.iFraction /= cN;
    Norm4FastFloatU(&fflt);
    return fflt;
}
#   define FFLT_AVG_SQUARE_WEIGHTS( pwtWeightFactor, cN ) ffltAvgSquareWeights( pwtWeightFactor, cN )
//#endif




Void prvGetBandWeightLowRate(CAudioObjectDecoder* paudec)
{
    U8* rgbBandNotCoded;
    Int iBand;
    Int iStart, iEnd;
    Int iChannel;
    U8 cNoiseBand;

    //FUNCTION_PROFILE(fp);
    //FUNCTION_PROFILE_START(&fp,GET_BAND_WEIGHTS_PROFILE);

    //calculating band weight
    for (iChannel = 0; iChannel < paudec->pau->m_cChannel; iChannel++)
    {
        if (paudec->pau->m_rgpcinfo [iChannel].m_iPower != 0)
        {
            FastFloat rgfltBandWeight[25];
            FastFloat* rgffltSqrtBWRatio = paudec->pau->m_rgpcinfo[iChannel].m_rgffltSqrtBWRatio;
            FinalWeightType* rgxWeightFactor  = paudec->pau->m_rgpcinfo[iChannel].INTEGER_OR_INT_FLOAT(m_rguiWeightFactor, m_rgfltWeightFactor);

            rgbBandNotCoded = paudec->pau->m_rgpcinfo[iChannel].m_rgbBandNotCoded;
            cNoiseBand = 0;
            iStart = 0, iEnd = 0;
            iBand = paudec->pau->m_iFirstNoiseBand;
            while (WMAB_TRUE)
            {
                iStart = max(paudec->pau->m_iFirstNoiseIndex, paudec->pau->m_rgiBarkIndex [iBand]);
                if (iStart >= paudec->pau->m_cHighCutOff)
                    break;
                assert(iBand < paudec->pau->m_cValidBarkBand);
                iEnd   = min(paudec->pau->m_cHighCutOff, paudec->pau->m_rgiBarkIndex [iBand + 1]);
                assert(iEnd >= iStart);

                if (rgbBandNotCoded [iBand] == 1)
                {
                    // This may be slower than straight divide, but we must do this to
                    // remain consistent with prvGetNoiseLikeBandsMono
                    rgfltBandWeight [cNoiseBand] = FFLT_AVG_SQUARE_WEIGHTS(rgxWeightFactor + iStart, iEnd - iStart);
                    //// rgfltBandWeight[cNoiseBand].iFraction*1.0F/(1<<rgfltBandWeight[cNoiseBand].iFracBits)
//                    MONITOR_RANGE(gMR_rgfltBandWeight,FLOAT_FROM_FASTFLOAT(rgfltBandWeight[cNoiseBand]));
//                    MONITOR_COUNT(gMC_GBW_floats,1+(iEnd-iStart)*2 );
                    cNoiseBand++;
                }
                //SHOW_NOISE_BANDS_ONCE;
                iBand++;
            }
            assert(cNoiseBand < paudec->pau->m_cValidBarkBand);
            assert(paudec->pau->m_iFirstNoiseBand > 0);
            for (iBand = 0; iBand < (cNoiseBand - 1); iBand++)
            {
                // This calculation is not ideally suited to FastFloat, leave in float for time being
                // But note that limited range of the result probably means it can be done more quickly
                rgffltSqrtBWRatio[iBand] = FFLT_SQRT_RATIO(rgfltBandWeight[iBand], rgfltBandWeight[cNoiseBand-1]);
                //// float SqrtBWRatio = fflt.iFraction*1.0F/(1<<fflt.iFracBits)
//                MONITOR_RANGE(gMR_rgffltSqrtBWRatio,FLOAT_FROM_FASTFLOAT(rgffltSqrtBWRatio[iBand]));
//                MONITOR_COUNT(gMC_GBW_floats,14);       // count sqrt as 10.
            }
            if (cNoiseBand > 0)
            {   // last band has a ratio of 1.0
//#if defined(BUILD_INTEGER)
                rgffltSqrtBWRatio[cNoiseBand-1].iFraction = 0x40000000;
                rgffltSqrtBWRatio[cNoiseBand-1].iFracBits = 30;
//                MONITOR_RANGE(gMR_rgffltSqrtBWRatio,1.0F);
            }
            rgbBandNotCoded [0] = cNoiseBand; //used as cNoiseBand
        }
    }
    //FUNCTION_PROFILE_STOP(&fp);
}
#pragma arm section code

#endif
#endif
#endif

