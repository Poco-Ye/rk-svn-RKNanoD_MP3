//*@@@+++@@@@******************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// Contains functions needed for WMA Std & WMA Pro; These are not needed by WMA Lossless.
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

#ifdef A_CORE_DECODE
#ifdef WMA_DEC_INCLUDE
// Print out the target we're building for
#define COMMONMACROS_OUTPUT_TARGET

//#if !defined (_WIN32_WCE) && !defined (HITACHI)
//#include <time.h>
//#endif  // _WIN32_WCE

//#include <math.h>
//#include <limits.h>
//#include <stdio.h>
#include "..\wmaInclude\msaudio.h"
#include "..\wmaInclude\AutoProfile.h"
#include "..\wmaInclude\macros.h"
//#include "float.h"
//#include "wavfileexio.h"
//#include "drccommon.h"
//#pragma arm section zidata = "WmaDecoderBss",rwdata = "WmaDecoderData",rodate = "WmaCommonCode"
#pragma arm section zidata = "WmaDecoderBss",rwdata = "WmaDecoderData",rodate = "WmaCommonCode"



//#if defined (BUILD_WMASTD) || defined (BUILD_WMAPRO)
//*****************************************************************************************
//
// prvSetBarkIndex
// part of prvAllocate which is part of auInit
//
//*****************************************************************************************
#ifdef  WMAINITIALIZE
#pragma arm section code = "WmaOpenCodecCode"

Void prvSetBarkIndex(CAudioObject* pau)
{
    Int i, iWin;
    Float fltSamplingPeriod;

    Int *piBarkIndex;
    Int cFrameSample;
    Bool bCombined;
    const I32 cMaxBarkBand = (pau->m_iVersion <= 2) ? 25 : NUM_BARK_BAND;
#ifdef WMA_TABLE_ROOM_VERIFY
    const U32* rgiBarkFreq = (const U32*)p_g_rgiBarkFreqV2;
#else
    const U32* rgiBarkFreq = g_rgiBarkFreqV2;
#endif

    //calculate index of each bark freq
    fltSamplingPeriod = 1.0F / pau->m_iSamplingRate;
    // wchen: we need to think what to do with the cut off frequencies: not include at all or include zeros.
    //for long window
    piBarkIndex = pau->m_rgiBarkIndexOrig;

    // for the v1 compatibility
    if (pau->m_iVersion == 1)
    {
        assert(pau->m_cPossibleWinSize == 1);

        // set up the bark index
        piBarkIndex[0] = 0;
        for (i = 0; i < cMaxBarkBand; i++)
        {
            piBarkIndex [i + 1] = (Int)(rgiBarkFreq [i] * pau->m_cFrameSample *
                                        fltSamplingPeriod + 0.5f); //open end
            assert(piBarkIndex [i + 1]);
            if (piBarkIndex [i + 1] > pau->m_cFrameSample / 2)
            {
                piBarkIndex [i + 1] = pau->m_cFrameSample / 2;
                pau->m_rgcValidBarkBand [0] = i + 1;
                break;
            }
        }
    }
    else
    {
        for (iWin = 0; iWin < pau->m_cPossibleWinSize; iWin++)
        {
            piBarkIndex  [0] = 0;
            cFrameSample = pau->m_cFrameSample / (1 << iWin);
            bCombined = WMAB_FALSE;
            if (pau->m_iVersion <= 2)
            {
                if (pau->m_iSamplingRate >= 44100)
                {
                    if (cFrameSample == 1024)  // winsize = 512
                    {
                        bCombined = WMAB_TRUE;
                        pau->m_rgcValidBarkBand[iWin] = 17;
                        piBarkIndex[1]  = 5;
                        piBarkIndex[2]  = 12;
                        piBarkIndex[3]  = 18;
                        piBarkIndex[4]  = 25;
                        piBarkIndex[5]  = 34;
                        piBarkIndex[6]  = 46;
                        piBarkIndex[7]  = 54;
                        piBarkIndex[8]  = 63;
                        piBarkIndex[9]  = 86;
                        piBarkIndex[10] = 102;
                        piBarkIndex[11] = 123;
                        piBarkIndex[12] = 149;
                        piBarkIndex[13] = 179;
                        piBarkIndex[14] = 221;
                        piBarkIndex[15] = 279;
                        piBarkIndex[16] = 360;
                        piBarkIndex[17] = 512;
                    }
                    else if (cFrameSample == 512)  // winsize = 256
                    {
                        bCombined = WMAB_TRUE;
                        pau->m_rgcValidBarkBand[iWin] = 15;
                        piBarkIndex[1]  = 5;
                        piBarkIndex[2]  = 11;
                        piBarkIndex[3]  = 17;
                        piBarkIndex[4]  = 23;
                        piBarkIndex[5]  = 31;
                        piBarkIndex[6]  = 37;
                        piBarkIndex[7]  = 43;
                        piBarkIndex[8]  = 51;
                        piBarkIndex[9]  = 62;
                        piBarkIndex[10] = 74;
                        piBarkIndex[11] = 89;
                        piBarkIndex[12] = 110;
                        piBarkIndex[13] = 139;
                        piBarkIndex[14] = 180;
                        piBarkIndex[15] = 256;
                    }
                    else if (cFrameSample == 256)   // winsize = 128
                    {
                        bCombined = WMAB_TRUE;
                        pau->m_rgcValidBarkBand [iWin] = 12;
                        piBarkIndex[1]  = 4;
                        piBarkIndex[2]  = 9;
                        piBarkIndex[3]  = 12;
                        piBarkIndex[4]  = 16;
                        piBarkIndex[5]  = 21;
                        piBarkIndex[6]  = 26;
                        piBarkIndex[7]  = 37;
                        piBarkIndex[8]  = 45;
                        piBarkIndex[9]  = 55;
                        piBarkIndex[10] = 70;
                        piBarkIndex[11] = 90;
                        piBarkIndex[12] = 128;
                    }
                }
                else if (pau->m_iSamplingRate >= 32000)
                {
                    if (cFrameSample == 1024)  // winsize = 512
                    {
                        bCombined = WMAB_TRUE;
                        pau->m_rgcValidBarkBand[iWin] = 16;
                        piBarkIndex[1]  = 6;
                        piBarkIndex[2]  = 13;
                        piBarkIndex[3]  = 20;
                        piBarkIndex[4]  = 29;
                        piBarkIndex[5]  = 41;
                        piBarkIndex[6]  = 55;
                        piBarkIndex[7]  = 74;
                        piBarkIndex[8]  = 101;
                        piBarkIndex[9]  = 141;
                        piBarkIndex[10] = 170;
                        piBarkIndex[11] = 205;
                        piBarkIndex[12] = 246;
                        piBarkIndex[13] = 304;
                        piBarkIndex[14] = 384;
                        piBarkIndex[15] = 496;
                        piBarkIndex[16] = 512;
                    }
                    else if (cFrameSample == 512)  // winsize = 256
                    {
                        bCombined = WMAB_TRUE;
                        pau->m_rgcValidBarkBand[iWin] = 15;
                        piBarkIndex[1]  = 5;
                        piBarkIndex[2]  = 10;
                        piBarkIndex[3]  = 15;
                        piBarkIndex[4]  = 20;
                        piBarkIndex[5]  = 28;
                        piBarkIndex[6]  = 37;
                        piBarkIndex[7]  = 50;
                        piBarkIndex[8]  = 70;
                        piBarkIndex[9]  = 85;
                        piBarkIndex[10] = 102;
                        piBarkIndex[11] = 123;
                        piBarkIndex[12] = 152;
                        piBarkIndex[13] = 192;
                        piBarkIndex[14] = 248;
                        piBarkIndex[15] = 256;
                    }
                    else if (cFrameSample == 256)   // winsize = 128
                    {
                        bCombined = WMAB_TRUE;
                        pau->m_rgcValidBarkBand [iWin] = 11;
                        piBarkIndex[1]  = 4;
                        piBarkIndex[2]  = 9;
                        piBarkIndex[3]  = 14;
                        piBarkIndex[4]  = 19;
                        piBarkIndex[5]  = 25;
                        piBarkIndex[6]  = 35;
                        piBarkIndex[7]  = 51;
                        piBarkIndex[8]  = 76;
                        piBarkIndex[9]  = 96;
                        piBarkIndex[10] = 124;
                        piBarkIndex[11] = 128;
                    }
                }
                else if (pau->m_iSamplingRate >= 22050)
                {
                    if (cFrameSample == 512)  // winsize = 256
                    {
                        bCombined = WMAB_TRUE;
                        pau->m_rgcValidBarkBand [iWin] = 14;
                        piBarkIndex[1]  = 5;
                        piBarkIndex[2]  = 12;
                        piBarkIndex[3]  = 18;
                        piBarkIndex[4]  = 25;
                        piBarkIndex[5]  = 34;
                        piBarkIndex[6]  = 46;
                        piBarkIndex[7]  = 63;
                        piBarkIndex[8]  = 86;
                        piBarkIndex[9]  = 102;
                        piBarkIndex[10] = 123;
                        piBarkIndex[11] = 149;
                        piBarkIndex[12] = 179;
                        piBarkIndex[13] = 221;
                        piBarkIndex[14] = 256;
                    }
                    else if (cFrameSample == 256)  // winsize = 128
                    {
                        bCombined = WMAB_TRUE;
                        pau->m_rgcValidBarkBand [iWin] = 10;
                        piBarkIndex[1]  = 5;
                        piBarkIndex[2]  = 11;
                        piBarkIndex[3]  = 17;
                        piBarkIndex[4]  = 23;
                        piBarkIndex[5]  = 31;
                        piBarkIndex[6]  = 43;
                        piBarkIndex[7]  = 62;
                        piBarkIndex[8]  = 89;
                        piBarkIndex[9]  = 110;
                        piBarkIndex[10] = 128;
                    }
                }
            }

            if (!bCombined)
            {
                Float fltTemp = cFrameSample * fltSamplingPeriod;
                Int iIndex;
                Int iFreqCurr = 0;
                Int iCurr = 1;
                while (WMAB_TRUE)
                {
                    if (pau->m_iVersion <= 2)
                        iIndex = ((Int)((rgiBarkFreq [iFreqCurr++] * fltTemp + 2.0f) / 4.0f)) * 4; // open end
                    //else
                    //                    {
                    //                        // Integerized version
                    //                        iIndex  = ((rgiBarkFreq [iFreqCurr++]*cFrameSample)/pau->m_iSamplingRate + 2);
                    //                        iIndex -= iIndex % 4; // Make a multiple of 4 through truncation
                    //                    }


                    if (iIndex > piBarkIndex[iCurr - 1])
                        piBarkIndex[iCurr++] = iIndex;

                    // We change > to >= because > has a problem in (v2 fz=2964).
                    // Basically, in the second window size loop we has last BarkIndex as 516, but
                    // in the third window size loop, 516 is mapped to 256 which is exactlly equal
                    // to cFrameSample/2. Then, we get one more BarkBand. But later, we alway assume
                    // the first window size has most BarkBands and we assign pau->m_rgcValidBarkBand [0]
                    // to pau->m_cValidBarkBand. As a result, we will encounter buffer overrun.
                    // This fix may change bitsteam. I verified that it does not change the behavior
                    // of the codec for standard settings like (44100, 48000, etc). To be more safer,
                    // I put an assert inside the block. It should not be hit because at this time
                    // we only encode and decode wma at standard sampling rates.
                    if (iFreqCurr >= cMaxBarkBand ||
                            piBarkIndex[iCurr - 1] >= cFrameSample / 2)
                    {
                        // This assert is put here for safety. It is used to verify the above fix does
                        // not change the behavior of codec for standard sampling rate. Once in future
                        // we permit continuous sampling rate (or in TEST_CORRUPT_BITSTREAM) we should
                        // remove it. But we should notice the previous player may not be able to
                        // decode wma files with some continuous sampling rate correctly if
                        // piBarkIndex[iCurr - 1] == cFrameSample here.
#if !defined TEST_CORRUPT_BITSTREAM
                        assert(piBarkIndex[iCurr - 1] != cFrameSample / 2 || pau->m_iSamplingRate == 24000);
#endif
                        piBarkIndex[iCurr - 1] = cFrameSample / 2;
                        pau->m_rgcValidBarkBand[iWin] = iCurr - 1;
                        break;
                    }
                }
            }
            else
            {
                for (i = 0; i < pau->m_rgcValidBarkBand [iWin]; i++)
                {
                    piBarkIndex [i + 1] = ((piBarkIndex [i + 1] + 2) / 4) * 4;  //rounding
                    assert(piBarkIndex [i + 1] > piBarkIndex [i]);
                }
            }

            piBarkIndex +=  NUM_BARK_BAND + 1;
        }
    }
    //default
    pau->m_rgiBarkIndex = pau->m_rgiBarkIndexOrig;
    pau->m_cValidBarkBand = pau->m_rgcValidBarkBand [0];

    // Check pau->m_rgcValidBarkBand [0] is the biggest.
    for (iWin = 0; iWin < pau->m_cPossibleWinSize; iWin++)
    {
        assert(pau->m_rgcValidBarkBand[iWin] <= pau->m_cValidBarkBand);
    }


} // prvSetBarkIndex
#pragma arm section code

#endif

//#if !defined(PLATFORM_SPECIFIC_SUBFRAMERECON) || !defined(WMA_TARGET_MIPS)
/****************************************************************************
**
** Function:        auSubframeRecon
**
** Description:     subframe reconstruction from coef to PCM samples
**
** Return:          None
**
*****************************************************************************/
#if defined(WMAHIGHRATE) || defined(WMAMIDRATELOWRATE)
#pragma arm section code = "WmaHighLowCommonCode"

#if !WMA_OPT_SUBFRAMERECON_ARM
WMARESULT auSubframeRecon(CAudioObject* pau)
{
    WMARESULT hr = WMA_OK;
    short iCh, iChSrc;
    short iSizePrev, iSizeCurr, iSizePrev2;
    PerChannelInfo *ppcinfo, *ppcinfo2;
    long     i, iOverlapSize;
//    short iPrevCoefRecurQ3, iPrevCoefRecurQ4, iCurrCoefRecurQ1, iCurrCoefRecurQ2, iCurrCoefRecurQ3 = 0, iCurrCoefRecurQ4 = 0;
    long bp2Sin;
    long bp2Cos;
    long bp2Sin1;
    long bp2Cos1;
    long bp2Step;
    long bp2SinT;     /* temp sin value within recurrsion */
    long bp2CosT;     /* temp cos value within recurrsion */
    pcmType *piPrevCoef, *piCurrCoef,*piCurrCoefEnd;
    pcmType cfPrevData, cfCurrData;

    i = WMAB_FALSE;
    if (pau->m_cChInTile == 2)
    {

        // ======== Setup the init condition for PCM reconstruction ========
        ppcinfo = &pau->m_rgpcinfo[pau->m_rgiChInTile[0]];
        ppcinfo2 = &pau->m_rgpcinfo[pau->m_rgiChInTile[1]];

        iSizePrev = ppcinfo->m_iSizePrev;
        iSizePrev2 = ppcinfo2->m_iSizePrev;

        if ((pau->m_iVersion <= 2) || (iSizePrev == iSizePrev2))
            i = WMAB_TRUE;
    }

    // ********** Reconstruction for each channel in the tile **********
    if (i == WMAB_TRUE)  //version 1/2 and stereomode
    {
        pcmType *piPrevCoef2, *piCurrCoef2,*piCurrCoefEnd2;	

        // get previous subframe size
        iSizeCurr = ppcinfo->m_iSizeCurr;

        // Reset trig recursion
        bp2Sin  = ppcinfo->m_fiSinRampUpStart;
        bp2Cos  = ppcinfo->m_fiCosRampUpStart;
        bp2Sin1 = ppcinfo->m_fiSinRampUpPrior;
        bp2Cos1 = ppcinfo->m_fiCosRampUpPrior;
        bp2Step = ppcinfo->m_fiSinRampUpStep;

        //==================================================================
        // ==== In-place Reconstruction between prev and curr subframes ====
        //==================================================================

        // ---- Now move unused coef to the end of current subframe coef buffer ----
#ifdef SATURERATE_AFTER_FFT
		piCurrCoef = (pcmType *)ppcinfo->dst_rgiCoefRecon;   // now reverse the coef buffer channel0[f[n]]
		piCurrCoef2 = (pcmType *)ppcinfo2->dst_rgiCoefRecon;	 // now reverse the coef buffer channel1[f[n]]
#else
        piCurrCoef = (pcmType *)ppcinfo->m_rgiCoefRecon;   // now reverse the coef buffer channel0[f[n]]
        piCurrCoef2 = (pcmType *)ppcinfo2->m_rgiCoefRecon;   // now reverse the coef buffer channel1[f[n]]
#endif        
		if(NULL == piCurrCoef||NULL == piCurrCoef2)//add by evan wu,2009-4-24
		{
			return WMA_OK;
		}
        piCurrCoefEnd = piCurrCoef + iSizeCurr - 1;
		piCurrCoefEnd2 = piCurrCoef2 + iSizeCurr - 1;

        //for (i = 0; i < iSizeCurr / 2; i++)  //以下代码是将dctiv后得到的数据互换位置
        i = iSizeCurr / 2;
		while(i--)
        {
            cfCurrData = *piCurrCoef;//Q31
            *piCurrCoef++ = *piCurrCoefEnd;
            *piCurrCoefEnd-- = cfCurrData;

            cfCurrData = *piCurrCoef2;//Q31
            *piCurrCoef2++ = *piCurrCoefEnd2;
            *piCurrCoefEnd2-- = cfCurrData;
        }

        // ---- Setup the coef buffer pointer ----
#ifdef SATURERATE_AFTER_FFT
        piPrevCoef = (pcmType *)ppcinfo->dst_rgiCoefRecon - iSizePrev / 2;       // go forward 左声道的前一子帧的开始位置
        piPrevCoef2 = (pcmType *)ppcinfo2->dst_rgiCoefRecon - iSizePrev / 2;       // go forward 右声道的前一子帧的开始位置

        piCurrCoef = (pcmType *)ppcinfo->dst_rgiCoefRecon + iSizeCurr / 2 - 1;   // go backward 左声道的当前子帧的结尾位置        
        piCurrCoef2 = (pcmType *)ppcinfo2->dst_rgiCoefRecon + iSizeCurr / 2 - 1;   // go backward 右声道的当前子帧的结尾位置
#else
		piPrevCoef = (pcmType *)ppcinfo->m_rgiCoefRecon - iSizePrev / 2;	   // go forward 左声道的前一子帧的开始位置
		piPrevCoef2 = (pcmType *)ppcinfo2->m_rgiCoefRecon - iSizePrev / 2;		 // go forward 右声道的前一子帧的开始位置

		piCurrCoef = (pcmType *)ppcinfo->m_rgiCoefRecon + iSizeCurr / 2 - 1;   // go backward 左声道的当前子帧的结尾位置		
		piCurrCoef2 = (pcmType *)ppcinfo2->m_rgiCoefRecon + iSizeCurr / 2 - 1;	 // go backward 右声道的当前子帧的结尾位置

#endif
        
	    if (iSizeCurr >= iSizePrev) //subframe_curr_size<=subframe_next_size
	    {
			piCurrCoef -= (iSizeCurr - iSizePrev)/2;			
			piCurrCoef2 -= (iSizeCurr - iSizePrev)/2;
			
			iOverlapSize = iSizePrev/2;

	    }
	    else     //subframe_curr_size>subframe_next_size
	    {
			piPrevCoef += (iSizePrev - iSizeCurr)/2;
            piPrevCoef2 += (iSizePrev - iSizeCurr)/2;
			
			iOverlapSize = iSizeCurr/2;
	    }		

        // ---- Now we do overlap and add here ----
        for (i = 0; i < iOverlapSize; i++) //处理交叠样本的尺寸
        {
			long tempResult;
            cfPrevData = *piPrevCoef;
            cfCurrData = *piCurrCoef;

			//*piPrevCoef++ = ((MULT_BP2(-bp2Sin, cfCurrData) + MULT_BP2(bp2Cos, cfPrevData)) >> pau->m_cLeftShiftBitsFixedPost);
            tempResult = WMA_OVERLAP_SHIFT(MULT_BP2(-bp2Sin, cfCurrData) + MULT_BP2(bp2Cos, cfPrevData));
            WMA_16BITS_SATURATE(tempResult);					
			*piPrevCoef++ = (pcmType)tempResult; 
			
            //*piCurrCoef-- = ((MULT_BP2(bp2Sin, cfPrevData) + MULT_BP2(bp2Cos, cfCurrData)) >> pau->m_cLeftShiftBitsFixedPost);
            tempResult = WMA_OVERLAP_SHIFT(MULT_BP2(bp2Sin, cfPrevData) + MULT_BP2(bp2Cos, cfCurrData));            
            WMA_16BITS_SATURATE(tempResult);			
            *piCurrCoef-- = (pcmType)tempResult;
			
			
            cfPrevData = *piPrevCoef2;
            cfCurrData = *piCurrCoef2;

            //*piPrevCoef2++ = ((MULT_BP2(-bp2Sin, cfCurrData) + MULT_BP2(bp2Cos, cfPrevData)) >> pau->m_cLeftShiftBitsFixedPost);
            tempResult = WMA_OVERLAP_SHIFT(MULT_BP2(-bp2Sin, cfCurrData) + MULT_BP2(bp2Cos, cfPrevData));            
            WMA_16BITS_SATURATE(tempResult);						
            *piPrevCoef2++ = (pcmType)tempResult;	
			
            //*piCurrCoef2-- = ((MULT_BP2(bp2Sin, cfPrevData) + MULT_BP2(bp2Cos, cfCurrData)) >> pau->m_cLeftShiftBitsFixedPost);
            tempResult = WMA_OVERLAP_SHIFT(MULT_BP2(bp2Sin, cfPrevData) + MULT_BP2(bp2Cos, cfCurrData));            
            WMA_16BITS_SATURATE(tempResult);			
            *piCurrCoef2-- = (pcmType)tempResult;
			

            /* sin(a+b) = sin(a-b) + 2*sin(b)*cos(a) */
            /* cos(a+b) = cos(a-b) - 2*sin(b)*sin(a) */
            bp2SinT = bp2Sin1 + MULT_BP2(bp2Step, bp2Cos);
            bp2CosT = bp2Cos1 - MULT_BP2(bp2Step, bp2Sin);
            bp2Sin1 = bp2Sin;
            bp2Sin = bp2SinT;
            bp2Cos1 = bp2Cos;
            bp2Cos = bp2CosT;
        }

    }
    else  //version>2 and mono-mode
    {
        // mono or version 3 and above
        for (iCh = 0; iCh < pau->m_cChInTile; iCh++)
        {

            // ======== Setup the init condition for PCM reconstruction ========
            iChSrc = pau->m_rgiChInTile [iCh];
            ppcinfo = pau->m_rgpcinfo + iChSrc;

            // get previous subframe size
            iSizePrev = ppcinfo->m_iSizePrev;
            iSizeCurr = ppcinfo->m_iSizeCurr;

            // Reset trig recursion
            bp2Sin  = ppcinfo->m_fiSinRampUpStart;//Q30
            bp2Cos  = ppcinfo->m_fiCosRampUpStart;//Q30
            bp2Sin1 = ppcinfo->m_fiSinRampUpPrior;
            bp2Cos1 = ppcinfo->m_fiCosRampUpPrior;
            bp2Step = ppcinfo->m_fiSinRampUpStep;

            //prvCalcQ3Q4(pau, WMAB_FALSE, iSizePrev, iSizeCurr, iSizePrev, &iPrevCoefRecurQ3, &iPrevCoefRecurQ4);
            //prvCalcQ1Q2(pau, WMAB_TRUE, iSizePrev, iSizeCurr, &iCurrCoefRecurQ1, &iCurrCoefRecurQ2);

            //==================================================================
            // ==== In-place Reconstruction between prev and curr subframes ====
            //==================================================================

            // ---- Now move unused coef to the end of current subframe coef buffer ----
#ifdef SATURERATE_AFTER_FFT
            piCurrCoef = (pcmType *)ppcinfo->dst_rgiCoefRecon;	 
			if(NULL == piCurrCoef)
			{
				return WMA_OK;//2009-5-12
			}
#else
            piCurrCoef = (pcmType *)ppcinfo->m_rgiCoefRecon;   
#endif    
            // now reverse the coef buffer
            for (i = 0; i < iSizeCurr / 2; i++)
            {
                cfCurrData = piCurrCoef[i];
                piCurrCoef[i] = piCurrCoef[iSizeCurr - 1 - i];
                piCurrCoef[iSizeCurr - 1 - i] = cfCurrData;
            }
#ifdef SATURERATE_AFTER_FFT
			piPrevCoef = (pcmType *)ppcinfo->dst_rgiCoefRecon - iSizePrev / 2;	   
			piCurrCoef = (pcmType *)ppcinfo->dst_rgiCoefRecon + iSizeCurr / 2 - 1;   
#else
            // ---- Setup the coef buffer pointer ----
            piPrevCoef = (pcmType *)ppcinfo->m_rgiCoefRecon - iSizePrev / 2;       
            piCurrCoef = (pcmType *)ppcinfo->m_rgiCoefRecon + iSizeCurr / 2 - 1;  
#endif            

            // ---- Test which subframe is larger ----
            if (iSizeCurr >= iSizePrev)
            {
				piCurrCoef -= (iSizeCurr - iSizePrev)/2;				
				iOverlapSize = iSizePrev/2;

                // Go from smaller subframe to larger subframe
                //piCurrCoef -= iSizeCurr - iCurrCoefRecurQ2;
                //iOverlapSize = (iCurrCoefRecurQ2 - iCurrCoefRecurQ1) / 2;
            }
            else
            {
                // Go from smaller subframe to larger subframe
                //piPrevCoef += iPrevCoefRecurQ3 - iSizePrev;
                //iOverlapSize = (iCurrCoefRecurQ2 - iCurrCoefRecurQ1) / 2;

				piPrevCoef += (iSizePrev - iSizeCurr)/2;				
				iOverlapSize = iSizeCurr/2;				
            }

            // ---- Now we do overlap and add here ----
            for (i = 0; i < iOverlapSize; i++)
            {
                long tempResult;
                cfPrevData = *piPrevCoef;
                cfCurrData = *piCurrCoef;

                //*piPrevCoef++ = INT_FROM_COEF(MULT_BP2(-bp2Sin, cfCurrData) + MULT_BP2(bp2Cos, cfPrevData));
				tempResult = WMA_OVERLAP_SHIFT(MULT_BP2(-bp2Sin, cfCurrData) + MULT_BP2(bp2Cos, cfPrevData));
                WMA_16BITS_SATURATE(tempResult);
				*piPrevCoef++ = (pcmType)tempResult;

                //*piCurrCoef-- = INT_FROM_COEF(MULT_BP2(bp2Sin, cfPrevData) + MULT_BP2(bp2Cos, cfCurrData));
                tempResult = WMA_OVERLAP_SHIFT(MULT_BP2(bp2Sin, cfPrevData) + MULT_BP2(bp2Cos, cfCurrData));
                WMA_16BITS_SATURATE(tempResult);
				*piCurrCoef-- = (pcmType)tempResult;

                /* sin(a+b) = sin(a-b) + 2*sin(b)*cos(a) */
                /* cos(a+b) = cos(a-b) - 2*sin(b)*sin(a) */
                bp2SinT = bp2Sin1 + MULT_BP2(bp2Step, bp2Cos);
                bp2CosT = bp2Cos1 - MULT_BP2(bp2Step, bp2Sin);
                bp2Sin1 = bp2Sin;
                bp2Sin = bp2SinT;
                bp2Cos1 = bp2Cos;
                bp2Cos = bp2CosT;
            }
        }
    }

#ifndef SATURERATE_AFTER_FFT
    {
        short iSizeNext;
        long iOverlapSizeNext, iEnd;
        for (iCh = 0; iCh < pau->m_cChInTile; iCh++)
        {
            iChSrc = pau->m_rgiChInTile [iCh];
            ppcinfo = pau->m_rgpcinfo + iChSrc;

            iSizePrev = ppcinfo->m_iSizePrev;
            iSizeCurr = ppcinfo->m_iSizeCurr;
            iSizeNext = ppcinfo->m_iSizeNext;

            // In Unified WMA, we must call prvCalcQ1Q2/Q3Q4 to get the window shape.
            // iOverlapSize = min(iSizePrev, iSizeCurr) / 2;

            iOverlapSize = (iSizePrev >= iSizeCurr)?iSizeCurr/2 : iSizePrev/2;
            iOverlapSizeNext = (iSizeCurr >= iSizeNext)?iSizeNext/2 : iSizeCurr/2;
			

            if (pau->m_iVersion < 3)
            {
                iEnd = iSizeCurr - iOverlapSizeNext;
            }

            piCurrCoef = ppcinfo->m_rgiCoefRecon;

			//对当前帧内，既不和前一帧数据做叠加，也不和后一帧数据做叠加的数据进行处理
            for (i = iOverlapSize; i < iEnd; i++)
            {
                long tempResult = INT_FROM_COEF(piCurrCoef[i]);
				
                WMA_16BITS_SATURATE(tempResult);				
				piCurrCoef[i] = tempResult;
            }			
        }
    }
#endif

    return WMA_OK;
} // auSubframeRecon
#endif //WMA_OPT_SUBFRAMERECON_ARM
//#endif

//*****************************************************************************************
//
// prvInverseTransformMono
//  transform, window and combine an overlapping subframe of audio data
//  called repeatedly for each subframe in a frame
//
//  using abbreviated names, the process in outline form is as follows:
//
//      pSrc points to someplace in current or previous time-domain Output array
//      pDst points to someplace in current or previous time-domain Output array
//      between each "upto" step below, these two points may change
//
//      Coef is an array of frequency-domain coefs
//
//      call DCT and transform Coef to time-domain
//      pCoef set to middle of the now time-domain coeffiecents
//
//      Window and combine the transformed Coef and pSrc into pDst as follows:
//
//      Window
//                  ________________
//                 /                \
//                /                  \
//               /                    \
//      ________/                      \_________
//      |       | | |       |      | | |         |
//      0       Q Q Q       H      Q 3 Q         F
//              1 u 2       a      3 Q 4         u
//                a         l        u           l
//                d         f        a           l
//                                   d
//
//      upto Q1 (eg 0):
//          Dst <- Src                              [[++pDst, ++pCoef, ++pSrc]]
//
//      upto cSubFrameSampleQuad (eg 1024):
//          Dst <- ( sin(a) * Coef + Src<<5 ) >> 5  [[++pDst, ++pCoef, ++pSrc, ++a]]
//
//      upto Q2 (eg 2048):
//          Dst <- ( sin(a) * Coef + Src<<5 ) >> 5  [[++pDst, --pCoef, ++pSrc, ++a]]
//
//      upto cSubFrameSampleHalf (eg 2048):
//          Dst <- Coef >> 5                        [[++pDst, --pCoef]]
//
//      upto Q3 (eg 2944):
//          Dst <- Coef >> 5                        [[++pDst, --pCoef]]
//
//      upto 3 * cSubFrameSampleQuad (eg 3072):
//          Dst <- ( sin(a) * Coef ) >> 5;          [[++pDst, --pCoef]]
//
//      upto Q4 (eg 3200):
//          Dst <- ( sin(a) * Coef ) >> 5;          [[++pDst, ++pCoef]]
//
//
//
//  Values decoding the first few frames of Tough_44m_64.cmp (9/22/99 12:08PM version)
//  where pDst is relative to piOutput when c. and to piPrevOutput when p.
//
//                          iCurrSubFrame       iSubFrmOffset   pDst prior to loop:
//              FrameSample         cSubFrameSample               Q1     Q2     Q3     Q4
//
//  Decode             4096
//    inverseTransform          0       4096           0        c....0 c.1024 p....0 p.1024
//  Decode             4096
///   inverseTransform          0       2048         512        c..512 c.1024 c.1536 p....0
//    inverseTransform          1        256        1984        c.1984 p....0 p...64 p..128
//    inverseTransform          2        256        2112        p...64 p. 128 p..192 p..256
//    inverseTransform          3        256        2240        p. 192 p. 256 p..320 p..384
//    inverseTransform          4        256        2368        p..320 p..320 p..448 p..512
//    inverseTransform          5       1024        2304        p..256 p..512 p..768 p.1024
//  Decode             4096
//    inverseTransform          0       4096           0        c....0 c.1024 p....0 p.1024
//  Decode             4096
//    inverseTransform          0       4096           0        c....0 c.1024 p....0 p.1024
//  Decode             4096
//    inverseTransform          0       4096           0        c....0 c.1024 p....0 p.1024
//  Decode             4096
//    inverseTransform          0       2048         512        c..512 c.1024 c.1536 p....0
//    inverseTransform          1        512        1920        c.1920 p....0 p..128 p..256
//    inverseTransform          2        256        2240        p..192 p..256 p..320 p..384
//    inverseTransform          3        256        2368        p..320 p..384 p..448 p..512
//    inverseTransform          4        256        2496        p..448 p..512 p..576 p..640
//    inverseTransform          5        256        2624        p..576 p..640 p..704 p..768
//    inverseTransform          6        512        2688        p..640 p..768 p..896 p.1024
//
//*****************************************************************************************



void prvCalcQ1Q2(CAudioObject * pau, Bool bCurrWindow, const I16 iSizePrev, const I16 iSizeCurr,
                 I16 *piCoefRecurQ1, I16 *piCoefRecurQ2)
{
    //This function can calculate the Q1/Q2 or current window or next window but
    //so far we only it for the current window although this function support next window.
    assert(bCurrWindow);
    //if the adjacent size is bigger; just keep your own shape
    //otherwise a transition window is needed.
    if (iSizePrev >= iSizeCurr)
    {
        //just forward copy curr
        *piCoefRecurQ1 = 0;
        *piCoefRecurQ2 = iSizeCurr;
    }
    else
    {
        //long start
        *piCoefRecurQ1 = (iSizeCurr - iSizePrev) / 2;
        *piCoefRecurQ2 = (iSizeCurr + iSizePrev) / 2;
    }

    // update for MLLMUsePLLM.
//    if (bCurrWindow == WMAB_TRUE) {
    //        if (pau->m_bUnifiedPureLLMCurrFrm == WMAB_TRUE) {
    //            if (pau->m_bFirstUnifiedPureLLMFrm == WMAB_FALSE) {
    //                *piCoefRecurQ1 = (*piCoefRecurQ1 + *piCoefRecurQ2) / 2;
    //                *piCoefRecurQ2 = *piCoefRecurQ1;
    //            }
    //        }
    //    }
    //    else { // we are working on next window
    //        if (pau->m_bUnifiedPureLLMCurrFrm == WMAB_TRUE) {
    //            if (pau->m_bLastUnifiedPureLLMFrm == WMAB_FALSE) {
    //                *piCoefRecurQ1 = (*piCoefRecurQ1 + *piCoefRecurQ2) / 2;
    //                *piCoefRecurQ2 = *piCoefRecurQ1;
    //            }
    //        }
    //    }

}


void prvCalcQ3Q4(CAudioObject *pau, Bool bCurrWindow, const I16 iSizeCurr, const I16 iSizeNext,
                 const Int cSubFrameSampleHalfAdjusted,
                 I16 *piCoefRecurQ3, I16 *piCoefRecurQ4)
{
    //This function can calculate the Q1/Q2 or current window or prev window but
    //so far we only it for the current window.

    if (iSizeNext >= iSizeCurr) //window choose subframe_curr_size<=subframe_next_size
    {
        *piCoefRecurQ3 = (I16) cSubFrameSampleHalfAdjusted;//
        *piCoefRecurQ4 = (I16) cSubFrameSampleHalfAdjusted * 2;//
    }
    else     //subframe_curr_size>subframe_next_size
    {
        //just backward copy curr
        *piCoefRecurQ3 = (I16) cSubFrameSampleHalfAdjusted +
                         (iSizeCurr - iSizeNext) / 2;
        *piCoefRecurQ4 = (I16) cSubFrameSampleHalfAdjusted +
                         (iSizeCurr + iSizeNext) / 2;
    }

    // update for MLLMUsePLLM.
    //if (bCurrWindow == WMAB_TRUE) {
    //        if (pau->m_bUnifiedPureLLMCurrFrm == WMAB_TRUE) {
    //            if (pau->m_bLastUnifiedPureLLMFrm == WMAB_FALSE) {
    //                *piCoefRecurQ3 = (*piCoefRecurQ3 + *piCoefRecurQ4) / 2;
    //                *piCoefRecurQ4 = *piCoefRecurQ3;
    //            }
    //        }
    //    }
    //    else { // we are working on prev window
    //        if (pau->m_bUnifiedPureLLMCurrFrm == WMAB_TRUE) {
    //            if (pau->m_bFirstUnifiedPureLLMFrm == WMAB_FALSE) {
    //                *piCoefRecurQ3 = (*piCoefRecurQ3 + *piCoefRecurQ4) / 2;
    //                *piCoefRecurQ4 = *piCoefRecurQ3;
    //            }
    //        }
    //    }


}
#define SINSTEP_SCALE(a) a


WMARESULT prvAdaptTrigToSubframeConfig(CAudioObject *pau)
{
    WMARESULT wmaResult = WMA_OK;
    I16                 iSize2Use, iCh;

    for (iCh = 0; iCh < pau->m_cChInTile; iCh++)
    {
        I16 iChSrc = pau->m_rgiChInTile [iCh];
        PerChannelInfo* ppcinfo = pau->m_rgpcinfo + iChSrc;

        //if the adjacent size is bigger; just keep your own shape
        //otherwise a transition window is needed.
        if (ppcinfo->m_iSizePrev >= ppcinfo->m_iSizeCurr)
            iSize2Use = ppcinfo->m_iSizeCurr;  //just forward copy curr
        else
            iSize2Use = ppcinfo->m_iSizePrev;  //long start

        // Use lookup-tables if we can
        if (iSize2Use >= 64 && iSize2Use <= 2048)
        {
            const SinCosTable* pSinCosTable = rgSinCosTables[iSize2Use>>7];

            // START = sin( PI/(4*cSB) ) and cos( PI/(4*cSB) )
            ppcinfo->m_fiSinRampUpStart =  BP2_FROM_BP1(pSinCosTable->sin_PIby4cSB);    //(Int) (sin (0.5 * PI / iSizeCurr / 2) * 0x3FFFFFFF);
            ppcinfo->m_fiCosRampUpStart =  BP2_FROM_BP1(pSinCosTable->cos_PIby4cSB);    //(Int) (cos (0.5 * PI / iSizeCurr / 2) * 0x3FFFFFFF);

            // PRIOR should be sin(PI/(4*cSB) - PI/(2*cSB) ) = sin( -PI/(4*cSB) )
            ppcinfo->m_fiSinRampUpPrior = -BP2_FROM_BP1(pSinCosTable->sin_PIby4cSB);
            ppcinfo->m_fiCosRampUpPrior =  BP2_FROM_BP1(pSinCosTable->cos_PIby4cSB);
            ppcinfo->m_fiSinRampUpStep  =  SINSTEP_SCALE(pSinCosTable->sin_PIby2cSB);   // STEP = 2 * sin (PI / 2 / iSizeCurr) * 0x3FFFFFFF;
        }
        else
        {
//            DEBUG("SinCosTable is exceed!\n");
//            ppcinfo->m_fiSinRampUpStart =  (BP2Type) (sin (0.5f * PI / iSize2Use / 2) * NF2BP2);
            //            ppcinfo->m_fiCosRampUpStart =  (BP2Type) (cos (0.5f * PI / iSize2Use / 2) * NF2BP2);
            //            ppcinfo->m_fiSinRampUpPrior =  (BP2Type) -(sin(0.5f * PI / iSize2Use / 2) * NF2BP2);
            //            ppcinfo->m_fiCosRampUpPrior =  (BP2Type) (cos (0.5f * PI / iSize2Use / 2) * NF2BP2);
            //            ppcinfo->m_fiSinRampUpStep  =  (BP2Type) (sin (PI / 2 / iSize2Use) * NF2BP1);

        }

        ppcinfo++;
    }
    return wmaResult;
}
//#endif  // defined(BUILD_INTEGER)



//*****************************************************************************************
//
// auUpdateMaxEsc
//
//*****************************************************************************************
Void    auUpdateMaxEsc(CAudioObject* pau, Int iQuantStepSize)
{
    // HongCho: Adjust the max Tunstall level according to the quantization step...
    //          Matching if's in the decoder...
    //          Too many?  Maybe...
    // HongCho: Note...  For 22.05kHz, even with all 15bits, the bells don't ring...
    //if (pau->m_iVersion == 3)
    //{
    //    pau->m_iMaxEscSize = 31;
    //    pau->m_iMaxEscLevel = 0x7fffffff; // (1<<31) - 1 seems risky
    //}
    //else
    {
        if (iQuantStepSize < 5)       pau->m_iMaxEscSize = 13;
        else if (iQuantStepSize < 15) pau->m_iMaxEscSize = 13;
        else if (iQuantStepSize < 32) pau->m_iMaxEscSize = 12;
        else if (iQuantStepSize < 40) pau->m_iMaxEscSize = 11;
        else if (iQuantStepSize < 45) pau->m_iMaxEscSize = 10;
        else if (iQuantStepSize < 55) pau->m_iMaxEscSize =  9;
        else                         pau->m_iMaxEscSize =  9;
        pau->m_iMaxEscLevel = (1 << pau->m_iMaxEscSize) - 1;
    }
}


//void auLowPass(CAudioObject *pau, CoefType *rgCoef, Int iNumCoefs,
//               Int iPassThruCoefs)
//{
//    CoefType    *pEnd = rgCoef + iNumCoefs * 2; // Re and Im coefs (so times 2)
//    CoefType    *pCurr;
//    //Int     iPassThruCoefs;
//
//    // Figure out how many coefficients will pass through untouched
//    //iPassThruCoefs = (pau->m_iInterpolDstBlkSize * iNumCoefs) /
//    //    pau->m_iInterpolSrcBlkSize;
//    pCurr = rgCoef + (2 * iPassThruCoefs);   // Re and Im coefs (so times 2)
//    iNumCoefs -= iPassThruCoefs;
//
//    while (iNumCoefs > 0)
//    {
//        *pCurr++ = 0;       // Re coef
//        *pCurr++ = 0;       // Im coef
//        iNumCoefs -= 1;
//    }
//
//    assert(pCurr == pEnd);


//*****************************************************************************************
//
// qstCalcQuantStep
// like auCalcQuantStep but returns a QuantStepType which is either:
//   a FastFloat (for integer builds), a Float (Decoder) or a Double(Encoder)
//
//*****************************************************************************************
QuantStepType qstCalcQuantStep(long iQSS,int*isvalid)
{
    QuantStepType qstQuantStep;
    long iFractionhxd[17] = {0x4c1cfbc9, 0x556682d8, 0x5fd22414, 0x6b834554, 0x78a19e26, 0x43acdf5d, 0x4beed3a1, 0x5532b8e6, 0x5f98086b, 0x6b421290, 0x785876ce, 0x8707aa4e, 0x97818eef, 0xa9fe1cba, 0xbebc2000, 0xd6020ead, 0xf01ef7a5};
    short iFracBitshxd[17] = {0x6, 0x6, 0x6, 0x6, 0x6, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5};//STATIC TABLE(In ARM Cortex M3)
    if (iQSS < 18)
    {   // This happens when iPower==0, not an important case, but return 10^(1/20) with 28 FractBits
        // It can also happen with a small NoisePower (-13 has been seen)
        if (iQSS < 0)  //-19<=iQss<0
        {   // negative values of iQSS are being generated in the V5 encoder (LowRate in particular)
            qstQuantStep.iFraction = 0x62089BF;//(I32)(0.382943866392*(1<<QUANTSTEP_FRACT_BITS)),      // Average Fraction
            qstQuantStep.iFracBits = 28 - ((-iQSS >> 3) + 1);            // Approximate Exponent
        }
        else//0<=iQSS<18
        {
            qstQuantStep.iFraction = 0xde939b1;//(I32)(0.869439785679*(1<<QUANTSTEP_FRACT_BITS));     // Average Fraction
            qstQuantStep.iFracBits = 28 - ((iQSS >> 3) + 1);            // Approximate Exponent
        }
    }
    else if (iQSS < (DBPOWER_TABLE_OFFSET + DBPOWER_TABLE_SIZE))
    {
        // *** normal case ***
#ifdef WMA_TABLE_ROOM_VERIFY
        qstQuantStep.iFraction = ((const DBPowerType*)p_rgDBPower10)[ iQSS - DBPOWER_TABLE_OFFSET ];
#else
        qstQuantStep.iFraction = rgDBPower10[ iQSS - DBPOWER_TABLE_OFFSET ];
#endif
        qstQuantStep.iFracBits = 28 - ((iQSS >> 3) + 4);    // implied FractBit scale for rgiDBPower table
        //NormUInt( (long*)(&qstQuantStep.iFraction), &qstQuantStep.iFracBits, 0x3FFFFFFF);
        assert(qstQuantStep.iFraction > 0);
        //if (qstQuantStep.iFraction == 0) return; // useful if asserts are disabled
        while (qstQuantStep.iFraction < (0x3FFFFFFF >> 1))
        {
            qstQuantStep.iFraction <<= 2;
            qstQuantStep.iFracBits += 2;
			if (qstQuantStep.iFraction == 0)
			{
				*isvalid = 0xFFFFFFFF;
				return qstQuantStep;
			}
        }
        if (qstQuantStep.iFraction < 0x3FFFFFFF)
        {
            qstQuantStep.iFraction <<= 1;
            qstQuantStep.iFracBits += 1;
        }
    }
    else //In the following iQSS is larger than 145,make directly to calculate 10^(x/20)
    {   // This branch can handle out-of-range cases.
        // rare - but used in some cases by encoder - e.g. Tough_16m_16, Tough_22m_22(?).
        if (iQSS <= 162)
        {
            qstQuantStep.iFraction = iFractionhxd[iQSS-146];
            qstQuantStep.iFracBits = iFracBitshxd[iQSS-146];
        }
        else
        {
            qstQuantStep.iFraction = -1;
            qstQuantStep.iFracBits = 5;
        }
        //NormUInt( (long*)(&qstQuantStep.iFraction), &qstQuantStep.iFracBits, 0x3FFFFFFF );
        assert(qstQuantStep.iFraction > 0);
        //if (qstQuantStep.iFraction == 0) return; // useful if asserts are disabled
        //while (qstQuantStep.iFraction < (0x3FFFFFFF >> 1))
		while (qstQuantStep.iFraction < (0x3FFFFFFF >> 1))
        {
            qstQuantStep.iFraction <<= 2;
            qstQuantStep.iFracBits += 2;
			if(0 == qstQuantStep.iFraction)
			{
				*isvalid = 0xFFFFFFFF;
				return qstQuantStep;
			}
        }
        if (qstQuantStep.iFraction < 0x3FFFFFFF)
        {
            qstQuantStep.iFraction <<= 1;
            qstQuantStep.iFracBits += 1;
        }

    }
    return(qstQuantStep);
}

void prvInitDiscardSilence(CAudioObject *pau, Bool fSPDIF)
{
    Bool fStartOfStream = WMAB_TRUE;
    Int  iCh;

    assert(CODEC_BEGIN == pau->m_codecStatus);

    // We do not discard any silence for lossless mode
    //if (pau->m_bPureLosslessMode)
    //        return;


    // ********** Start of file detection ***********

    // If this is the very first frame, we need to determine if we are at the
    // start-of-file, rather than seeking. If so, we need to discard the silence
    // frames. If not, we need to only discard half a subframe. NOTE that for
    // V4 encoded streams, WE WILL GUESS INCORRECTLY. Our justification for accepting
    // this is that V4 never had timestamps and so sync was never guaranteed anyway.

    //if (pau->m_iVersion <= 2)
    {
        // Due to SCRUNCH bug #32, v5 encoder forces fMaskUpdate to TRUE and
        // all channels' m_iPower to 1 in the very first frame only. If we find that
        // fMaskUpdate, m_iPower are all TRUE but in fact there is no power here,
        // then we know this is a v5-encoded file and this is start-of-file
        //wchen: subfrmconfig should be the same for all channels for V2
        if (WMAB_FALSE == pau->m_rgpcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgfMaskUpdate[0])
            fStartOfStream = WMAB_FALSE;

        for (iCh = 0; iCh < pau->m_cChannel; iCh++)
        {
            if (0 == pau->m_rgpcinfo[iCh].m_iPower)
                fStartOfStream = WMAB_FALSE;
        }

        if (fStartOfStream)
        {
            // Bitstream has forced update, claims non-zero power for all channels.
            // Verify that claim.
            for (iCh = 0; iCh < pau->m_cChannel; iCh++)
            {
                if (0 != pau->m_rgpcinfo[iCh].m_iActualPower)
                    fStartOfStream = WMAB_FALSE;
            } // for (channels)
        }
    }

    // For v3, pau->m_fLeadingSilence indicates star-of-file

    // ********* Now set pau->m_rgiDiscardSilence array for each channel **********
    pau->m_fSeekAdjustment = WMAB_FALSE;

    //if (pau->m_iVersion <= 2)
    {
        I32 iDiscardSilence = 0;

        assert(!fSPDIF);    // we don't expect fSPDIF to be true for v2

        if (fStartOfStream)
        {
            iDiscardSilence = pau->m_cFrameSampleAdjusted;

        }
        else
        {
            I16  iSizePrev, iSizeCurr, Q1, Q2;

            iSizePrev = (I16)pau->m_rgpcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameSize[-1];
            iSizeCurr = (I16)pau->m_rgpcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameSize [0];

            //if (pau->m_fHalfTransform) {
            //                iSizeCurr >>= pau->m_iAdjustSizeShiftFactor;
            //                iSizePrev >>= pau->m_iAdjustSizeShiftFactor;
            //            }else if (pau->m_fPad2XTransform) {
            //                iSizeCurr <<= pau->m_iAdjustSizeShiftFactor;
            //                iSizePrev <<= pau->m_iAdjustSizeShiftFactor;
            //            }


            prvCalcQ1Q2(pau, WMAB_TRUE, iSizePrev, iSizeCurr, &Q1, &Q2);

            //iDiscardSilence = iSizePrev / 2 + Q2 - iSizeCurr / 2;
            iDiscardSilence = pau->m_cFrameSampleHalfAdjusted / 2 + Q2 - iSizeCurr / 2;
            pau->m_fSeekAdjustment = WMAB_TRUE;
        }

        assert(iDiscardSilence >= 0);

        // Propagate iDiscardSilence to all channels
        for (iCh = 0; iCh < pau->m_cChannel; iCh++)
            pau->m_rgiDiscardSilence[iCh] = iDiscardSilence;
    }
    //else
    //    {
    //        // ====== For V3 or above ======
    //        if( pau->m_fLeadingSilence )
    //        {
    //            // ---- We are at the beginning of a file, so discard one frame ----
    //            // for SPDIF actually we don't want to discard silence,
    //            // however we disable this in msaudiodec.c by checking SPDIF flag
    //            // so we are fine here to set non-zero values for SPDIF
    //            for (iCh = 0; iCh < pau->m_cChannel; iCh++)
    //                pau->m_rgiDiscardSilence[iCh] = AU_HALF_OR_DOUBLE(pau->m_fHalfTransform, pau->m_fPad2XTransform, pau->m_u32LeadingSize);
    //        }
    //        else
    //        {
    //            // ---- We are at the first frame for seeking ----
    //            Int     iSubframeSize, iMaxSubframeSize = 0;
    //
    //            if( fSPDIF )    // in reality this won't happen
    //            {
    //                iMaxSubframeSize = -pau->m_cFrameSampleHalfAdjusted;
    //            }
    //            else
    //            {
    //                // we try to find the max first subframe size for all the channel
    //                for ( iCh = 0; iCh < pau->m_cChannel; iCh++ ) {
    //
    //                    PerChannelInfo* ppcinfo = pau->m_rgpcinfo + iCh;
    //
    //                    iSubframeSize = ppcinfo->m_rgsubfrmconfig[CONFIG_CURRFRM].m_rgiSubFrameSize[0];
    //
    //                    if (pau->m_fHalfTransform) {
    //                        iSubframeSize >>= pau->m_iAdjustSizeShiftFactor;
    //                    }else if (pau->m_fPad2XTransform) {
    //                        iSubframeSize <<= pau->m_iAdjustSizeShiftFactor;
    //                    }
    //
    //                    if( iSubframeSize > iMaxSubframeSize )
    //                        iMaxSubframeSize = iSubframeSize;
    //                }
    //            }
    //
    //            // now discard half of the max subframe size
    //            for (iCh = 0; iCh < pau->m_cChannel; iCh++)
    //                pau->m_rgiDiscardSilence[iCh] = (pau->m_cFrameSampleHalfAdjusted + iMaxSubframeSize) / 2;
    //        }
    //
    //}

    if (CODEC_BEGIN == pau->m_codecStatus)
    {
        pau->m_codecStatus = CODEC_STEADY;
    }

} // prvInitDiscardSilence


WMARESULT auInvChannelXForm(CAudioObject *pau,Bool fInvertSuperGroupXform)
{
    WMARESULT wmaResult = WMA_OK;
    I16 iChSrc, iCh;
    PerChannelInfo* ppcinfo = pau->m_rgpcinfo;
    I32 iCoef;

    CoefType* rgfltCoefDst = pau->m_rgfltCoefDst;
    Bool fAllSkip = WMAB_TRUE;


    for (iCh = 0; iCh < pau->m_cChInTile; iCh++)
    {
        iChSrc = pau->m_rgiChInTile [iCh];
        fAllSkip &= ((pau->m_rgpcinfo + iChSrc)->m_iPower == 0);
    }

    if (pau->m_iVersion <= 2 && !fInvertSuperGroupXform)
    {
        CoefType*  piCoefRecon0;
        CoefType*  piCoefRecon1;

        assert(pau->m_cChannel <= 2);
        if (ppcinfo->m_stereoMode != STEREO_LEFTRIGHT && !fAllSkip)
        {
            //faster, special case for sum and diff
            piCoefRecon0 = (CoefType*) pau->m_rgpcinfo[0].m_rgiCoefRecon;
            piCoefRecon1 = (CoefType*) pau->m_rgpcinfo[1].m_rgiCoefRecon;
            // Do channel mixing.
            assert(pau->m_cChannel == 2);
            for (iChSrc = 0; iChSrc < pau->m_cChannel; iChSrc++)
            {
                (pau->m_rgpcinfo + iChSrc)->m_iPower = 1;
            }
            for (iCoef = (I16) ppcinfo->m_cSubbandAdjusted; iCoef > 0; iCoef--)
            {
                CoefType    cfTemp0;
                CoefType    cfTemp1;

                cfTemp0 = *piCoefRecon0;
                cfTemp1 = *piCoefRecon1;
                *piCoefRecon0++ = cfTemp0 + cfTemp1;
                *piCoefRecon1++ = cfTemp0 - cfTemp1;
                //piCoefRecon0++;
                //piCoefRecon1++;
            }
        }
    }
    //else if (pau->m_iVersion == 3) {
    //        // V3 bits. Perform inverse multichannel transform
    //        //I16 cSubbandAdj = (pau->m_rgpcinfo + pau->m_rgiChInTile [0])->m_cSubFrameSampleHalf;
    //        //const U16 cLastCodedIndex = pau->m_cHighCutOff;
    //        const U16 cLastCodedIndex = pau->m_cLastCodedIndexV3;
    //        if (!fAllSkip)
    //        {
    //            Int iChGrp, iBand;
    //
    //            // Do the transform on a channel group basis
    //            for (iChGrp = 0; iChGrp < pau->m_cChannelGroup; iChGrp++) {
    //                // Position src correctly for this channel group
    //                CChannelGroupInfo* pcgi = pau->m_rgChannelGrpInfo + iChGrp;
    //                Bool *rgfChannelMask = pcgi->m_rgfChannelMask;
    //
    //                // are we supposed to process this channel group
    //                if (pcgi->m_fIsSuperGroupXform != fInvertSuperGroupXform)
    //                    continue;
    //
    //                if (pcgi->m_cChannelsInGrp == 1 ||
    //                    (pcgi->m_fIsPredefinedXform == WMAB_TRUE &&
    //                    pcgi->m_predefinedXformType == MULTICH_IDENTITY))
    //                {
    //                    // Identity transform for this channel group: do nothing
    //                }
    //                else if (pau->m_cChannel == 2 &&
    //                         pcgi->m_fIsPredefinedXform == WMAB_TRUE &&
    //                         pcgi->m_predefinedXformType == MULTICH_HADAMARD)
    //                {
    //                    // Somewhat faster processing for stereo-pairs in stereo coding: weights absorbed into
    //                    // encoder's forward matrix.
    //                    CoefType* pfltCoefSrc0 = (CoefType*)pau->m_rgpcinfo->m_rgiCoefRecon;
    //                    CoefType* pfltCoefSrc1 = (CoefType*)(pau->m_rgpcinfo + 1)->m_rgiCoefRecon;
    //
    //                    assert(pau->m_rgChannelGrpInfo[iChGrp].m_cChannelsInGrp == 2);
    //
    //                    for (iBand = 0; iBand < pau->m_cValidBarkBand; iBand++) {
    //                        if (pcgi->m_rgfXformOn[iBand] == WMAB_TRUE) {
    //                            for (iCoef = pau->m_rgiBarkIndex[iBand];
    //                                iCoef <  min(cLastCodedIndex, pau->m_rgiBarkIndex[iBand+1]);
    //                                iCoef++)
    //                            {
    //                                CoefType fltDst0 = *pfltCoefSrc0 - *pfltCoefSrc1;
    //                                CoefType fltDst1 = *pfltCoefSrc0 + *pfltCoefSrc1;
    //
    //                                *pfltCoefSrc0++ = fltDst0;
    //                                *pfltCoefSrc1++ = fltDst1;
    //                            }
    //                        } else {
    //                            //WMAFprintf(stdout, "Here\n");
    //                            // identity transform for this bark: Just do the scaling.
    //                            for (iCoef = pau->m_rgiBarkIndex[iBand];
    //                                iCoef < min(cLastCodedIndex,pau->m_rgiBarkIndex[iBand+1]);
    //                                iCoef++)
    //                            {
    //                                *pfltCoefSrc0++ = MULT_CH_SQRT2(*pfltCoefSrc0);
    //                                *pfltCoefSrc1++ = MULT_CH_SQRT2(*pfltCoefSrc1);
    //                            }
    //                        }
    //                    }
    //                }           //end of Hadamard
    //                else    //generic transform
    //                {
    //                    // Multiple channels or generalized transformation
    //                    // Other predefined transforms, if used, should have proper values set in m_rgfltMultiXInverse
    //
    //                    // Collect group members and associated matrices together: could be done outside to
    //                    // save some cycles in encoding loops: to be done.
    //                    CoefType** rgpfltCoefGrpSrc = (CoefType**)pau->m_rgpfltCoefGrpSrc;
    //                    Int    cChannelsInGrp = pcgi->m_cChannelsInGrp;
    //                    Int    iCh0=0, iCh1=0;
    //                    ChXFormType* rgfltMultiXGrpInverse = pcgi->m_rgfltMultiXInverse;
    //
    //                    // Collect channel group members together
    //                    for (iCh = 0; iCh < pau->m_cChInTile; iCh++)
    //                    {
    //                        I16 iChSrcTmp = pau->m_rgiChInTile [iCh];
    //                        PerChannelInfo* ppcinfoTmp = pau->m_rgpcinfo + iChSrcTmp;
    //                        if (rgfChannelMask[iChSrcTmp] == WMAB_TRUE)
    //                        {
    //                            rgpfltCoefGrpSrc[iCh0] = (CoefType*)ppcinfoTmp->m_rgfltCoefRecon;
    //                            iCh0++;
    //                        }
    //                    }
    //                    assert(iCh0 == cChannelsInGrp);
    //
    //                    // Code bloat for a few popular channel group sizes.
    //                    if (cChannelsInGrp == 2)
    //                    {
    //                        for (iBand = 0; iBand < pau->m_cValidBarkBand; iBand++)
    //                        {
    //                            if (pcgi->m_rgfXformOn[iBand] == WMAB_TRUE)
    //                            {
    //                                for (iCoef = pau->m_rgiBarkIndex[iBand];
    //                                    iCoef < min(cLastCodedIndex,pau->m_rgiBarkIndex[iBand+1]);
    //                                    iCoef++)
    //                                {
    //                                    CoefType fltDst0 =
    //                                        MULT_CH(rgfltMultiXGrpInverse[0], *(rgpfltCoefGrpSrc [0])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[1], *(rgpfltCoefGrpSrc [1]));
    //                                    CoefType fltDst1 =
    //                                        MULT_CH(rgfltMultiXGrpInverse[2], *(rgpfltCoefGrpSrc [0])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[3], *(rgpfltCoefGrpSrc [1]));
    //                                    *rgpfltCoefGrpSrc[0]++ = fltDst0;
    //                                    *rgpfltCoefGrpSrc[1]++ = fltDst1;
    //                                }
    //                            }
    //                            else
    //                            {
    //                                // identity transform for this bark: do not sweat on this bark band
    //                                rgpfltCoefGrpSrc [0] +=  (pau->m_rgiBarkIndex[iBand+1]- pau->m_rgiBarkIndex[iBand]);
    //                                rgpfltCoefGrpSrc [1] +=  (pau->m_rgiBarkIndex[iBand+1]- pau->m_rgiBarkIndex[iBand]);
    //                            }
    //                        }
    //                    }
    //                    else if (cChannelsInGrp == 3)
    //                    {
    //                        for (iBand = 0; iBand < pau->m_cValidBarkBand; iBand++)
    //                        {
    //                            if (pcgi->m_rgfXformOn[iBand] == WMAB_TRUE)
    //                            {
    //                                for (iCoef = pau->m_rgiBarkIndex[iBand];
    //                                    iCoef < min(cLastCodedIndex,pau->m_rgiBarkIndex[iBand+1]);
    //                                    iCoef++)
    //                                {
    //                                    CoefType fltDst0 =
    //                                        MULT_CH(rgfltMultiXGrpInverse[0], *(rgpfltCoefGrpSrc [0])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[1], *(rgpfltCoefGrpSrc [1])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[2], *(rgpfltCoefGrpSrc [2]));
    //                                    CoefType fltDst1 =
    //                                        MULT_CH(rgfltMultiXGrpInverse[3], *(rgpfltCoefGrpSrc [0])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[4], *(rgpfltCoefGrpSrc [1])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[5], *(rgpfltCoefGrpSrc [2]));
    //                                    CoefType fltDst2 =
    //                                        MULT_CH(rgfltMultiXGrpInverse[6], *(rgpfltCoefGrpSrc [0])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[7], *(rgpfltCoefGrpSrc [1])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[8], *(rgpfltCoefGrpSrc [2]));
    //                                    *rgpfltCoefGrpSrc[0]++ = fltDst0;
    //                                    *rgpfltCoefGrpSrc[1]++ = fltDst1;
    //                                    *rgpfltCoefGrpSrc[2]++ = fltDst2;
    //                                }
    //                            }
    //                            else
    //                            {
    //                                // identity transform for this bark: do not sweat on this bark band
    //                                rgpfltCoefGrpSrc [0] +=  (pau->m_rgiBarkIndex[iBand+1]- pau->m_rgiBarkIndex[iBand]);
    //                                rgpfltCoefGrpSrc [1] +=  (pau->m_rgiBarkIndex[iBand+1]- pau->m_rgiBarkIndex[iBand]);
    //                                rgpfltCoefGrpSrc [2] +=  (pau->m_rgiBarkIndex[iBand+1]- pau->m_rgiBarkIndex[iBand]);
    //                            }
    //                        }
    //                    }
    //                    else if (cChannelsInGrp == 4)
    //                    {
    //                        for (iBand = 0; iBand < pau->m_cValidBarkBand; iBand++)
    //                        {
    //                            if (pcgi->m_rgfXformOn[iBand] == WMAB_TRUE)
    //                            {
    //                                for (iCoef = pau->m_rgiBarkIndex[iBand];
    //                                    iCoef < min(cLastCodedIndex,pau->m_rgiBarkIndex[iBand+1]);
    //                                    iCoef++)
    //                                {
    //                                    CoefType fltDst0 =
    //                                        MULT_CH(rgfltMultiXGrpInverse[0], *(rgpfltCoefGrpSrc [0])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[1], *(rgpfltCoefGrpSrc [1])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[2], *(rgpfltCoefGrpSrc [2])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[3], *(rgpfltCoefGrpSrc [3]));
    //                                    CoefType fltDst1 =
    //                                        MULT_CH(rgfltMultiXGrpInverse[4], *(rgpfltCoefGrpSrc [0])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[5], *(rgpfltCoefGrpSrc [1])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[6], *(rgpfltCoefGrpSrc [2])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[7], *(rgpfltCoefGrpSrc [3]));
    //                                    CoefType fltDst2 =
    //                                        MULT_CH(rgfltMultiXGrpInverse[8], *(rgpfltCoefGrpSrc [0])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[9], *(rgpfltCoefGrpSrc [1])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[10], *(rgpfltCoefGrpSrc [2])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[11], *(rgpfltCoefGrpSrc [3]));
    //                                    CoefType fltDst3 =
    //                                        MULT_CH(rgfltMultiXGrpInverse[12], *(rgpfltCoefGrpSrc [0])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[13], *(rgpfltCoefGrpSrc [1])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[14], *(rgpfltCoefGrpSrc [2])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[15], *(rgpfltCoefGrpSrc [3]));
    //                                    *rgpfltCoefGrpSrc[0]++ = fltDst0;
    //                                    *rgpfltCoefGrpSrc[1]++ = fltDst1;
    //                                    *rgpfltCoefGrpSrc[2]++ = fltDst2;
    //                                    *rgpfltCoefGrpSrc[3]++ = fltDst3;
    //                                }
    //                            }
    //                            else
    //                            {
    //                                // identity transform for this bark: do not sweat on this bark band
    //                                rgpfltCoefGrpSrc [0] +=  (pau->m_rgiBarkIndex[iBand+1]- pau->m_rgiBarkIndex[iBand]);
    //                                rgpfltCoefGrpSrc [1] +=  (pau->m_rgiBarkIndex[iBand+1]- pau->m_rgiBarkIndex[iBand]);
    //                                rgpfltCoefGrpSrc [2] +=  (pau->m_rgiBarkIndex[iBand+1]- pau->m_rgiBarkIndex[iBand]);
    //                                rgpfltCoefGrpSrc [3] +=  (pau->m_rgiBarkIndex[iBand+1]- pau->m_rgiBarkIndex[iBand]);
    //                            }
    //                        }
    //                    }
    //                    else if (cChannelsInGrp == 5)
    //                    {
    //                        for (iBand = 0; iBand < pau->m_cValidBarkBand; iBand++)
    //                        {
    //                            if (pcgi->m_rgfXformOn[iBand] == WMAB_TRUE)
    //                            {
    //                                for (iCoef = pau->m_rgiBarkIndex[iBand];
    //                                    iCoef < min(cLastCodedIndex,pau->m_rgiBarkIndex[iBand+1]);
    //                                    iCoef++)
    //                                {
    //                                    CoefType fltDst0 =
    //                                        MULT_CH(rgfltMultiXGrpInverse[0], *(rgpfltCoefGrpSrc [0])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[1], *(rgpfltCoefGrpSrc [1])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[2], *(rgpfltCoefGrpSrc [2])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[3], *(rgpfltCoefGrpSrc [3])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[4], *(rgpfltCoefGrpSrc [4]));
    //                                    CoefType fltDst1 =
    //                                        MULT_CH(rgfltMultiXGrpInverse[5], *(rgpfltCoefGrpSrc [0])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[6], *(rgpfltCoefGrpSrc [1])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[7], *(rgpfltCoefGrpSrc [2])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[8], *(rgpfltCoefGrpSrc [3])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[9], *(rgpfltCoefGrpSrc [4]));
    //                                    CoefType fltDst2 =
    //                                        MULT_CH(rgfltMultiXGrpInverse[10], *(rgpfltCoefGrpSrc [0])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[11], *(rgpfltCoefGrpSrc [1])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[12], *(rgpfltCoefGrpSrc [2])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[13], *(rgpfltCoefGrpSrc [3])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[14], *(rgpfltCoefGrpSrc [4]));
    //                                    CoefType fltDst3 =
    //                                        MULT_CH(rgfltMultiXGrpInverse[15], *(rgpfltCoefGrpSrc [0])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[16], *(rgpfltCoefGrpSrc [1])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[17], *(rgpfltCoefGrpSrc [2])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[18], *(rgpfltCoefGrpSrc [3])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[19], *(rgpfltCoefGrpSrc [4]));
    //                                    CoefType fltDst4 =
    //                                        MULT_CH(rgfltMultiXGrpInverse[20], *(rgpfltCoefGrpSrc [0])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[21], *(rgpfltCoefGrpSrc [1])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[22], *(rgpfltCoefGrpSrc [2])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[23], *(rgpfltCoefGrpSrc [3])) +
    //                                        MULT_CH(rgfltMultiXGrpInverse[24], *(rgpfltCoefGrpSrc [4]));
    //                                    *rgpfltCoefGrpSrc[0]++ = fltDst0;
    //                                    *rgpfltCoefGrpSrc[1]++ = fltDst1;
    //                                    *rgpfltCoefGrpSrc[2]++ = fltDst2;
    //                                    *rgpfltCoefGrpSrc[3]++ = fltDst3;
    //                                    *rgpfltCoefGrpSrc[4]++ = fltDst4;
    //                                }
    //                            }
    //                            else
    //                            {
    //                                // identity transform for this bark: do not sweat on this bark band
    //                                rgpfltCoefGrpSrc [0] +=  (pau->m_rgiBarkIndex[iBand+1]- pau->m_rgiBarkIndex[iBand]);
    //                                rgpfltCoefGrpSrc [1] +=  (pau->m_rgiBarkIndex[iBand+1]- pau->m_rgiBarkIndex[iBand]);
    //                                rgpfltCoefGrpSrc [2] +=  (pau->m_rgiBarkIndex[iBand+1]- pau->m_rgiBarkIndex[iBand]);
    //                                rgpfltCoefGrpSrc [3] +=  (pau->m_rgiBarkIndex[iBand+1]- pau->m_rgiBarkIndex[iBand]);
    //                                rgpfltCoefGrpSrc [4] +=  (pau->m_rgiBarkIndex[iBand+1]- pau->m_rgiBarkIndex[iBand]);
    //                            }
    //                        }
    //                    }
    //                    else
    //                    {
    //                        // Group size is larger than 5
    //                        for (iBand = 0; iBand < pau->m_cValidBarkBand; iBand++)
    //                        {
    //                            if (pcgi->m_rgfXformOn[iBand] == WMAB_TRUE)
    //                            {
    //                                for (iCoef = pau->m_rgiBarkIndex[iBand];
    //                                    iCoef < min(cLastCodedIndex,pau->m_rgiBarkIndex[iBand+1]);
    //                                    iCoef++)
    //                                {
    //                                    for (iChDst = 0; iChDst < cChannelsInGrp; iChDst++) {
    //                                        rgfltMultiXGrpInverse = pcgi->m_rgfltMultiXInverse + iChDst * cChannelsInGrp;
    //
    //                                        ((CoefType*)rgfltCoefDst) [iChDst] = 0;
    //                                        for (iChSrc = 0; iChSrc < cChannelsInGrp; iChSrc++) {
    //                                            ((CoefType*)rgfltCoefDst) [iChDst] += MULT_CH(rgfltMultiXGrpInverse[iChSrc], *(rgpfltCoefGrpSrc [iChSrc]));
    //                                        }
    //                                    }
    //                                    // Assign back weighted sums.
    //                                    for (iChDst = 0; iChDst < cChannelsInGrp; iChDst++) {
    //                                        *rgpfltCoefGrpSrc [iChDst] = ((CoefType*)rgfltCoefDst) [iChDst];
    //                                        rgpfltCoefGrpSrc [iChDst]++;
    //                                    }
    //                                }
    //                            }
    //                            else
    //                            {
    //                                // identity transform for this bark: do not sweat on this bark band
    //                                for (iChDst = 0; iChDst < cChannelsInGrp; iChDst++)
    //                                    rgpfltCoefGrpSrc [iChDst] +=  (pau->m_rgiBarkIndex[iBand+1]- pau->m_rgiBarkIndex[iBand]);
    //                            }
    //                        }
    //                    }
    //                }
    //            }
    //        }
    //    }


    return wmaResult;

} // auInvChannelXForm
#pragma arm section code

#endif//WMAHIGHRATE) || WMAMIDRATELOWRATE


//#endif // defined (BUILD_WMASTD) || defined (BUILD_WMAPRO)

#endif
#endif
