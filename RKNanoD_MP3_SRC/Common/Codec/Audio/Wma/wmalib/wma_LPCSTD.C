//*@@@+++@@@@******************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//*@@@---@@@@******************************************************************
/*************************************************************************

Copyright (C) Microsoft Corporation, 1996 - 1999

Module Name:

    MsAudio.cpp

Abstract:

    Implementation of public member functions for CAudioObject.

Author:

    Wei-ge Chen (wchen) 16-November-1998

Revision History:

    Sil Sanders (sils) 8-Feb-00 - combine Integer and Float versions and simplify
This file is needed only for WMA Std functionality.

*************************************************************************/
//#if defined(ENABLE_ALL_ENCOPT) && defined(ENABLE_LPC)
#include "../include/audio_main.h"
#if !defined(_WIN32_WCE) && !defined(HITACHI)
//#include <time.h>
#endif  // !_WIN32_WCE && !HITACHI
//#include <math.h>
//#include <limits.h>
#include "..\wmaInclude\msaudio.h"
//#include "stdio.h"
#include "..\wmaInclude\AutoProfile.h"
//#include "float.h"

#ifdef A_CORE_DECODE
#ifdef WMA_DEC_INCLUDE
#pragma arm section zidata = "WmaDecoderBss",rwdata = "WmaDecoderData"
//#if defined (BUILD_WMASTD)

// *** one of these may be set by platform in macros.h ***
//// in performance figures an X86 tick = 1/1193182 seconds = 838 ns (500MHz Pentium Dell 610)
//// and a SH4 Tick = 80ns (Dreamcast SH4 200Mhz)
//// X86 tested with wmadec_s Release build w/ PROFILE and wmatest_sh4 Release Build with PROFILE

// **************** warning ***************************
// every time you change one of these PLATFORM_LPC defines below, you must rebuilt all!
// ****************************************************

// PLATFORM_LPC_DIRECT using trig recurrsion
//#define PLATFORM_LPC_DIRECT
//// X86 Tough_32s_22:  5,300,299               Tough_16m_16:  1,533,753
//// SH4 Tough_32s_22: 43,303,596               Tough_16m_16: 12,927,913

// PLATFORM_LPC_FOLDED_PRUNED uses ARM's Folded Trick with otherwise
// a pruned or decomposed FFT - fast (old default)
// Fast and uses little extra memory and so is the default
//#define PLATFORM_LPC_FOLDED_PRUNED



#define MAX_LP_SPEC_SIZE 2048

//#include "..\wmaInclude\lpcConst.h"

//function prototype, used to eliminate Mac warnings
Void prvPq2lpc(CAudioObject* pau, LpType* p, LpType* q, LpType* lpc, Int order);
Void prvConvolve_odd(CAudioObject* pau, LpType* in1, Int l1, LpType* in2, Int l2, LpType* out, Int *lout);
Void prvConvolve(CAudioObject* pau, LpType *in1, Int l1, LpType *in2, Int l2, LpType *out, Int *lout);
Void prvLsp2pq(CAudioObject* pau, U8 *lsfQ, LpType *p, LpType *q, Int order);

// *****************************************************************************
//
// Conversion from LSF thru Parcor to LPC
//
// *****************************************************************************
#ifdef WMAMIDRATELOWRATE
#pragma arm section code = "WmaLowRateCode"

// Parcor to LPC conversion
Void prvPq2lpc(CAudioObject* pau, LpType* p, LpType* q, LpType* lpc, Int order)
{
    I32 i;

    for (i = 1; i <= order / 2; i++)
    {
//#   if defined(BUILD_INTEGER_LPC)
        // this variant is necessary because we found some test vectors which overflowed
        // \\sils1\Sounds\Test\lpc\test_vector1_44s_20kbps_32khz_m.wma
        // differences caused by dividing before adding had no effect of BUILD_INTEGER_LPC-BUILD_INT_FLOAT_LPC diffs
        // for a large set of difficult test vectors.
        lpc[i-1] = -DIV2LPC(p[i]) - DIV2LPC(q[i]);
        lpc[order-i] = -DIV2LPC(p[i]) + DIV2LPC(q[i]);
        assert(lpc[i-1]     == -(((I64)DIV2LPC(p[i])) + ((I64)DIV2LPC(q[i]))));
        assert(lpc[order-i] == -(((I64)DIV2LPC(p[i])) - ((I64)DIV2LPC(q[i]))));
//#   else
//        lpc[i-1]= -DIV2LPC(p[i]+q[i]);
//        lpc[order-i]= -DIV2LPC(p[i]-q[i]);
//#   endif
    }
}

/* symmetric convolution */
Void prvConvolve_odd(CAudioObject* pau, LpType* in1, Int l1, LpType* in2, Int l2, LpType* out, Int *lout)
{
    // In BUILD_INTEGER_LPC mode, in1 assumed to have FRACT_BITS_LSP and in2 to have FRACT_BITS_LP. Output will have
    // FRACT_BITS_LP.
    I32   i, j;
    LpType ret[100];

    for (i = 0; (i < l1) && (i < l2); i++)
    {
        ret[i] = 0;
        for (j = 0; j <= i; j++)
        {
            ret[i] += MULT_LSP(in1[j], in2[i-j]);
        }
    }

    for (i = l1; i < (l1 + l2) / 2; i++)
    {
        ret[i] = 0;
        for (j = 0; j < l1; j++)
        {
            ret[i] += MULT_LSP(in1[j], in2[i-j]);
        }
    }

    *lout = (l1 + l2) - 1;
    for (i = 0; i < (*lout) / 2; i++)
    {
        out[i] = ret[i];
        out[(*lout-1)-i] = ret[i];
    }
    out[*lout/2] = ret[*lout/2];
}

/* symmetric convolution */
Void prvConvolve(CAudioObject* pau, LpType *in1, Int l1, LpType *in2, Int l2, LpType *out, Int *lout)
{
    // In BUILD_INTEGER_LPC mode, in1 assumed to have FRACT_BITS_LSP and in2 to have FRACT_BITS_LP. Output will have
    // FRACT_BITS_LP.
    I32   i, j;
    LpType ret[100];

    for (i = 0; (i < l1) && (i < l2); i++)
    {
        ret[i] = 0;
        for (j = 0; j <= i; j++)
        {
            ret[i] += MULT_LSP(in1[j], in2[i-j]);
        }
    }

    for (i = l1; i < (l1 + l2) / 2; i++)
    {
        ret[i] = 0;
        for (j = 0; j < l1; j++)
        {
            ret[i] += MULT_LSP(in1[j], in2[i-j]);
        }
    }

    *lout = (l1 + l2) - 1;
    for (i = 0; i < (*lout) / 2; i++)
    {
        out[i] = ret[i];
        out[(*lout-1)-i] = ret[i];
    }
}

#ifdef WMA_TABLE_ROOM_VERIFY
#define LSF_DECODE(i,lsfQ) *((const LpType*)p_g_rgiLsfReconLevel+i*16+lsfQ[i])
#else
#define LSF_DECODE(i,lsfQ) g_rgiLsfReconLevel[i][lsfQ[i]]
#endif
//#if defined(BUILD_INTEGER_LPC)
#ifdef WMA_TABLE_ROOM_VERIFY
#define LP_DECODE(i,lsfQ) (*((const LpType*)p_g_rgiLsfReconLevel+i*16+lsfQ[i]) >> (FRACT_BITS_LSP-FRACT_BITS_LP))
#else
#define LP_DECODE(i,lsfQ) (g_rgiLsfReconLevel[i][lsfQ[i]] >> (FRACT_BITS_LSP-FRACT_BITS_LP))
#endif
//#else // BUILD_INTEGER_LPC
//#define LP_DECODE(i,lsfQ) LSF_DECODE((i),(lsfQ))
//#endif // BUILD_INTEGER_LPC

// Only updates
#define ARRAY_RANGE_FMAX(a,b,c,d,init,i) if ((init)) d = -FLT_MAX; \
for ((i)=(b);(i)<(c);(i)++) (d) = ((d) < (a)[i]) ? (a)[i] : (d);
#define ARRAY_RANGE_FMIN(a,b,c,d,init,i) if ((init)) d = FLT_MAX; \
for ((i)=(b);(i)<(c);(i)++) (d) = ((d) > (a)[i]) ? (a)[i] : (d);

// Quantized LSF to PARCOR

Void prvLsp2pq(CAudioObject* pau, U8 *lsfQ, LpType *p, LpType *q, Int order)
{
    I32     i;
    LpType  long_seq[100];
    LspType short_seq[3];
    Int     long_length, short_length;

    short_length = 3;
    long_length  = 2;

    short_seq[0] = short_seq[2] = LSP_FROM_FLOAT(1);
    long_seq[0]  = long_seq[1]  = LP_FROM_FLOAT(1);

    for (i = 0; i < order; i += 2)
    {
        short_seq[1] = LSF_DECODE(i, lsfQ); // -2.0F*(Float)cos(2.0*PI*lsp[i])
        prvConvolve(pau, short_seq, short_length, long_seq, long_length,
                    long_seq, &long_length);
    }

    for (i = 1; i <= order / 2; i++)
    {
        p[i] = long_seq[i];
    }

    long_length = 3;
    long_seq[0] = LP_FROM_FLOAT(1);
    long_seq[1] = LP_DECODE(1, lsfQ);            // -2.0F*(Float)cos(2.0*PI*lsp[1])
    long_seq[2] = LP_FROM_FLOAT(1);

    for (i = 3; i < order; i += 2)
    {
        short_seq[1] = LSF_DECODE(i, lsfQ); // -2.0F*(Float)cos(2.0*PI*lsp[i])
        prvConvolve_odd(pau, short_seq, short_length, long_seq, long_length,
                        long_seq, &long_length);
    }

    for (i = 1; i <= order / 2; i++)
    {
        q[i] = long_seq[i] - long_seq[i-1];
        INTEGER_ONLY_LPC(assert(q[i] == (((I64)long_seq[i]) - ((I64)long_seq[i-1]))));
    }
}

// Quantized LSF to LPC
Void    auLsp2lpc(CAudioObject* pau, U8 *lsfQ, LpType *lpc, Int order)
{
    LpType p[LPCORDER+2], q[LPCORDER+2];

    prvLsp2pq(pau, lsfQ, p, q, order);

    prvPq2lpc(pau, p, q, lpc, order);
}
#pragma arm section code = "WmaLowRateCode"
#endif


// ************************************************************************************
//
// InverseQuadRoot(x) = (1/x)^(1/4)
// where the 1/4 is one squareroot combined with the flattenfactor
//
// Do this three different ways:  Encoder, Integer Decoder, IntFloat Decoder
//
// ************************************************************************************

//#ifdef BUILD_INTEGER_LPC
	//delet by hl
 // U8 gLZLTable[128];

#ifdef WMAINITIALIZE
#pragma arm section code = "WmaOpenCodecCode"
 /*
void gLZLTableInit(void)
{
    int i, j;
    U8 mask;

    for (i = 0;i < 128;i++)
    {
        mask = 0x80;
        for (j = 0;j < 8;j++)
        {
            if ((2*i)&mask)
                break;
            mask >>= 1;
        }


        gLZLTable[i] = (U8)j;

        mask = 0x80;
        for (j = 0;j < 8;j++)
        {
            if ((2*i + 1)&mask)
                break;
            mask >>= 1;
        }

        gLZLTable[i] |= j << 4;
    }

}	 */

Void prvInitInverseQuadRootTable(CAudioObject* pau)
{
    SETUP_LPC_SPEC_TRIG(pau);
   // gLZLTableInit();
} // prvInitInverseQuadRootTable
#pragma arm section code

#endif

#ifdef WMAMIDRATELOWRATE
#pragma arm section code = "WmaLowRateCode"

INLINE WeightType InverseQuadRoot(LpSpecPowerType f)
{
    //// 1.0f/(1<<30)
    WeightType uiResult;
    LpSpecPowerType uFrac;
    Int iExp;// can be -ve
    UInt uiExpVal;
    UInt uiTmp;

    uFrac = f;
    // Get normalized fractional portion with PRECESSION_BITS_FOR_INVQUADROOT
    // bits and corresponding exponent value
    if (uFrac == 0)
    {
        iExp = 0;
    }
    else
    {
        // Find the most significant bit with value 1.
        // 64-bit input: Look at each 32-bit portion
        // We are assuming that PRECESSION_BITS_FOR_INVQUADROOT is < 32.
        if ((uFrac >> 32) == 0)
        {
            iExp = 32;
            uiTmp = (UInt) uFrac;
        }
        else
        {
            iExp = 0;
            uiTmp = (UInt)(uFrac >> 32);
        }
        while (!(uiTmp & 0x80000000))
        {
            uiTmp <<= 1;
            iExp++;
        };

        // Perform shifts on the input so that we have
        // PRECESSION_BITS_FOR_INVQUADROOT bits of normalized
        // fraction, and corresponding exponent values.

        uFrac = f;
        iExp = BITS_LP_SPEC_POWER - iExp - PRECESSION_BITS_FOR_INVQUADROOT;
        if (iExp < 0)
            uFrac <<= (-iExp);
        if (iExp > 0)
            uFrac >>= iExp;
    }

    // QR_*_FRAC_BITS: same as used in prvInitInverseQuadRootTable
//    MONITOR_COUNT(gMC_ScaleInverseQuadRoot,iExp);
#ifdef WMA_TABLE_ROOM_VERIFY
    uFrac = ((const UInt *)p_g_InvQuadRootFraction)[(Int)uFrac];
#else
    uFrac = g_InvQuadRootFraction[(Int)uFrac];
#endif
    //// iFrac*1.0f/(1<<23)
#ifdef WMA_TABLE_ROOM_VERIFY
    uiExpVal = ((const UInt*)p_g_InvQuadRootExponent)[iExp+PRECESSION_BITS_FOR_INVQUADROOT];
#else
    uiExpVal = g_InvQuadRootExponent[iExp+PRECESSION_BITS_FOR_INVQUADROOT];
#endif
    //// iExp*1.0f/(1<<29)
    uiResult = MULT_HI_UDWORD(((UInt)uFrac), uiExpVal); // frac_bits = 23+29-32 = 20
    //// iResult*1.0f/(1<<20)
#if ((QR_FRACTION_FRAC_BITS+QR_EXPONENT_FRAC_BITS-32) > WEIGHTFACTOR_FRACT_BITS)
    uiResult >>= (QR_FRACTION_FRAC_BITS + QR_EXPONENT_FRAC_BITS - 31) - WEIGHTFACTOR_FRACT_BITS;
#elif ((QR_FRACTION_FRAC_BITS+QR_EXPONENT_FRAC_BITS-32) < WEIGHTFACTOR_FRACT_BITS)
    uiResult <<= WEIGHTFACTOR_FRACT_BITS - (QR_FRACTION_FRAC_BITS + QR_EXPONENT_FRAC_BITS - 31);
#endif //((QR_FRACTION_FRAC_BITS+QR_EXPONENT_FRAC_BITS-32) > WEIGHTFACTOR_FRACT_BITS)
    // uiResult = (2^19/QR(iFrac*2^-19)) * 2^6 * (2^19/QR(2^iExp)) * 2^7 * 2^-32 = 2^19/QR(iFrac*2^-19*2^iExp)
    // uiResult = MULT_HI_DWORD(iFrac<<(BITS_PER_DWORD-FRACT_BITS_LP_SPEC>>1),(iExp<<((BITS_PER_DWORD-FRACT_BITS_LP_SPEC>>1)+1)));

    //WMAFprintf(stdout, "%lf\n", (double)uiResult/(1<<WEIGHTFACTOR_FRACT_BITS));
    return uiResult;
} // InverseQuadRoot

//#else   // so must be BUILD_INT_FLOAT_LPC

//Tables for inverse quad root calculation
//UInt g_InvQuadRootExponent[0x100];
//UInt g_InvQuadRootMantissa[1<<(PRECESSION_BITS_FOR_INVQUADROOT)];
//
//#if defined(_SH4_)
//#pragma warning(push)
//#pragma warning(disable:4719)
//#endif
//
////Build tables for fast inverse quad root calculation
//Void prvInitInverseQuadRootTable (CAudioObject* pau)
//{
//    //Iterate through floating point numbers with 12 bits of presition
//    //The folowing loop is equivalent to
//    //for (Float i = 1.0; i < 1.999756;i+=1.000244)
//    Int i;
//    for (i = 0; i < 1<<(PRECESSION_BITS_FOR_INVQUADROOT); i++){
//        Int fltInt = 0x3F800000 | (i << (23-PRECESSION_BITS_FOR_INVQUADROOT));//Build floating point
//        Float f = *(Float *)&fltInt;                       //number in the form of 1.i * pow(2,0);
//        Float invQuadRoot = (Float)(1/sqrt(sqrt(f)));  //Calculate inverse quad root
//        fltInt = *(Int *)&invQuadRoot;
//        g_InvQuadRootMantissa[i] = fltInt;             //Store value in table
//    }
//
//    //Iterate through floating point exponents from pow(2,-126)...pow(2,127) by powers of 2
//    for (i = 1; i < 255; i++){
//        Int fltInt = (i << 23);                             //Build floating point number in the form
//        Float f = *(Float *)&fltInt;            //of 1.0 * pow(2,(i-125))
//        f = (Float)(1/sqrt(sqrt(f)));           //calculate inverse quad root / 2
//        g_InvQuadRootExponent[i] = *(Int *)&f;  //Store in table
//    }
//    SETUP_LPC_SPEC_TRIG(pau);
//}
//
//#if defined(_SH4_)
//#pragma warning(pop)
//#endif
//
//INLINE WeightType InverseQuadRootF(Float f){
//    Float frac = (*(Float *)&(g_InvQuadRootMantissa[((*(Int *)&f) &   0x7FFFFF) >> (23-PRECESSION_BITS_FOR_INVQUADROOT)]));
//    Float exp  = (*(Float *)&(g_InvQuadRootExponent[((*(Int *)&f) & 0x7F800000) >> 23]));
//    return (frac * exp);
//}
//
//INLINE WeightType InverseQuadRootI(Int f){
//    Float frac = (*(Float *)&(g_InvQuadRootMantissa[(f &   0x7FFFFF) >> (23-PRECESSION_BITS_FOR_INVQUADROOT)]));
//    Float exp  = (*(Float *)&(g_InvQuadRootExponent[(f & 0x7F800000) >> 23]));
//    //WMAFprintf(stdout, "%.20lf\n", ((double)*(Float*)(&f)));
//    //WMAFprintf(stdout, "%.20lf\n", ((double)frac * (double)exp));
//    return (frac * exp);
//}
//
//// there appears to be no effect of calling The I suffix or the F suffix.
//// lpc_float.c called the I suffix.
//#define InverseQuadRoot(X) InverseQuadRootI(*(Int*)&X)



// *****************************************************************************
//
// Support for Integer and Float versions of LPC to Spectrum
//
// *****************************************************************************

//INLINE LpSpecPowerType square(LpSpecType x)
//{
//    /* x must be less than sqrt(2^12)*2^19 to avoid overflow */
//    if (abs(x) < 0x01000000)
//        return (LpSpecPowerType)
//               (MULT_HI_DWORD(x << ((BITS_PER_DWORD - FRACT_BITS_LP_SPEC) >> 1), x << ((BITS_PER_DWORD - FRACT_BITS_LP_SPEC + 1) >> 1)));
//    return (LpSpecPowerType)
//           (MULT_HI_DWORD(x, x) << (BITS_PER_DWORD - FRACT_BITS_LP_SPEC));
//}

INLINE LpSpecPowerType square64(LpSpecType x)
{
#ifdef TRUNC
    return (LpSpecPowerType)(((I64)x * (I64)x) >> (2 * FRACT_BITS_LP_SPEC - FRACT_BITS_LP_SPEC_POWER));
#else
    return (LpSpecPowerType)(((I64)x * (I64)x));
#endif
}



#if defined(U64SQUARE32SR)
#   define SQUARE(x) U64SQUARE32SR(x, (2 * FRACT_BITS_LP_SPEC - FRACT_BITS_LP_SPEC_POWER))
#ifdef U64SUM_SQUARES32SR
#   define SUM_SQUARES(x,y) U64SUM_SQUARES32SR(x,y, (2 * FRACT_BITS_LP_SPEC - FRACT_BITS_LP_SPEC_POWER))
#else
#   define SUM_SQUARES(x,y) (square64(x)+square64(y))
#endif
#else // U64SQUARE32SR
#   define SQUARE(x) square64(x)
#   define SUM_SQUARES(x,y) (square64(x)+square64(y))
#endif // U64SQUARE32SR
#   define MULT_BP2LPCX(x,y) MULT_BP2LPC(x,y)

//#endif  // end of _DEBUG and not _DEBUG variations



#define SQRT2_2  BP2LPC_FROM_FLOAT(0.70710678118654752440084436210485)
#define SQRT2    BP2LPC_FROM_FLOAT(1.4142135623730950488016887242097)



//INLINE
WeightType InverseQuadRootOfSumSquares(LpSpecType F1, LpSpecType F2)
{
    // LpSpecPowerType is U64.
    LpSpecPowerType fOrig = SUM_SQUARES(F1, F2);
    LpSpecPowerType f = fOrig >> (2 * FRACT_BITS_LP_SPEC - FRACT_BITS_LP_SPEC_POWER);
    Int iExpNt = 0;

    U32 uiMSF = (U32)(f >> 32);
    Int iExp = 0;
    int iMSF8;
    U32 uiFrac1;
    U8 index;
    if (uiMSF == 0)
    {
        iExp = 32;
        uiMSF = (U32)f;
    }
    if (uiMSF == 0)
        return(0xFFFFFFFF);
    // normalize the most significant fractional part
    while ((uiMSF & 0xFf000000) == 0)
    {
        iExp += 8;
        uiMSF <<= 8;
    }

    index = (U8)(uiMSF >> 24);


#ifdef WMA_TABLE_ROOM_VERIFY
    iExp += ((((U8*)p_gLZLTable)[index>>1] >> ((index & 1) << 2))) & 0xf;
#else
    iExp += ((gLZLTable[index>>1] >> ((index & 1) << 2))) & 0xf;
#endif


    // discard the most significant one bit (it's presence is built into g_InvQuadRootFraction)
    iExp++;
    //MONITOR_COUNT(gMC_ScaleInverseQuadRoot,iExp);
//    MONITOR_RANGE(gMC_ScaleInverseQuadRoot,iExp);
    // get all 32 bits from source

    iExpNt = (iExp - (2 * FRACT_BITS_LP_SPEC - FRACT_BITS_LP_SPEC_POWER));
    uiMSF = (iExpNt > 32) ? (U32)(fOrig << (iExpNt - 32)) : (U32)(fOrig >> (32 - iExpNt));

    // split into top INVQUADROOT_FRACTION_TABLE_LOG2_SIZE==8 bits for fractional lookup and bottom bits for interpolation
    iMSF8 = uiMSF >> (32 - INVQUADROOT_FRACTION_TABLE_LOG2_SIZE);
    uiMSF <<= INVQUADROOT_FRACTION_TABLE_LOG2_SIZE;
    // lookup and interpolate - tables are set up to return correct binary point for WeightType
#ifdef WMA_TABLE_ROOM_VERIFY
    uiFrac1  = ((const UInt*)p_g_InvQuadRootFraction)[iMSF8++];
    uiFrac1 -= MULT_HI_UDWORD(uiMSF, uiFrac1 - ((const UInt*)p_g_InvQuadRootFraction)[iMSF8]);
    return MULT_HI_UDWORD(uiFrac1, ((const UInt*)p_g_InvQuadRootExponent)[ iExp ]);
#else
    uiFrac1  = g_InvQuadRootFraction[iMSF8++];
    uiFrac1 -= MULT_HI_UDWORD(uiMSF, uiFrac1 - g_InvQuadRootFraction[iMSF8]);
    return MULT_HI_UDWORD(uiFrac1, g_InvQuadRootExponent[ iExp ]);
#endif
}



// ************************************************************************************
// ************************* This is the one we normally use **************************
// ************************************************************************************

//#pragma COMPILER_MESSAGE(__FILE__ "(735) : Warning - building PLATFORM_LPC_FOLDED_PRUNED LPC spectrum")

// ************************************************************************************
//
// LPC to Spectrum using a "Pruned" FFT and redundant calculation removal
//
// See Sorensen & Burrus, IEEE Trans Signal Processing V41 #3 March 93
// and the references sited there for a treatment of pruned and Transform-decomposited DFT
// Unclear how this particular implementation relates to those articles.
//
// Originally designed and implemented by Wei-ge and Marc.
// Restructured by Sil to be a single routine.
// Later restructured again to use only a small stack array (inspired by ARM's technique).
//
// The use of pointers instead of array indexes speeds up the SH4 and has no effect on the X86
//
// Cache usage: constants 14kb (note SH3 cache size is 16kb and we need 8k of WF plus constants)
//
// ************************************************************************************
I32 multbp2lpcx(I32 a, I32 b)
{
    return (I32)(((I64)a * b) >> 30);
}

void prvDoLpc4(const Int k, const LpSpecType* pTmp, WeightType* pWF, const Int iSizeBy2, const BP2LPCType S1, const BP2LPCType C1)
{
    BP2LPCType    CmS, CpS;
    LpSpecType T2, T4, T6, T7, T8, T9, TA, TB;
    LpSpecType D, E;

    BP2LPCType    C2, S2, C3, S3;


    CmS = C1 - S1;                              // cnst4[i*6+4];
    CpS = C1 + S1;                              // cnst4[i*6+5];
    assert(BP2LPC_FROM_FLOAT(1.0) <= CpS && CpS < BP2LPC_FROM_FLOAT(1.5));
    T8 = multbp2lpcx(CmS, pTmp[2]) + multbp2lpcx(CpS, pTmp[3]);   // F[2048+j]    F[3072-j]
    T6 = multbp2lpcx(CpS, pTmp[2]) - multbp2lpcx(CmS, pTmp[3]);   // F[2048+j]    F[3072-j]

    S2 = MUL2LPC(multbp2lpcx(C1, S1));                       // sin(2x)
    C2 = BP2LPC_FROM_FLOAT(1.0f) - MUL2LPC(multbp2lpcx(S1, S1)); // cos(2x)
    CmS = C2 - S2;                              // cnst4[i*6+2];
    CpS = C2 + S2;                              // cnst4[i*6+3];
    T7 = multbp2lpcx(CmS, pTmp[0]) + multbp2lpcx(CpS, pTmp[1]);   // F[1024+j]    F[2048-j]
    T4 = multbp2lpcx(CpS, pTmp[0]) - multbp2lpcx(CmS, pTmp[1]);   // F[1024+j]    F[2048-j]

    S3 = multbp2lpcx(S1, C2) + multbp2lpcx(C1, S2); // sin(3x) = sin(x+2x)
    C3 = multbp2lpcx(C1, C2) - multbp2lpcx(S1, S2); // cos(2x) = cos(x+2x)
    CmS = C3 - S3;                              // old cnst4[i*6];
    CpS = C3 + S3;                              // old cnst4[i*6+1];
    T9 = multbp2lpcx(CmS, pTmp[4]) + multbp2lpcx(CpS, pTmp[5]);   // F[3072+j]    F[4096-j]
    T2 = multbp2lpcx(CpS, pTmp[4]) - multbp2lpcx(CmS, pTmp[5]);   // F[3072+j]    F[4096-j]

    TA = pTmp[6] + pTmp[7];                                   // F[j]      +  F[1024-j];
    TB = pTmp[6] - pTmp[7];                                   // F[j]      -  F[1024-j];

    D  = DIV2LPC( + T7  + T8 + T9 + TA);
    E  = DIV2LPC( + T4  + T6 + T2 + TB);
    pWF[k]           = InverseQuadRootOfSumSquares(D, E);              // F[j]

    D  = DIV2LPC(- T7  + T6 - T2 + TA);
    E  = DIV2LPC( + T4  + T8 - T9 - TB);
    pWF[iSizeBy2-k]  = InverseQuadRootOfSumSquares(D, E);              // F[1024-j]

    D  = DIV2LPC(- T7  - T6 + T2 + TA);
    E  = DIV2LPC(- T4  + T8 - T9 + TB);
    pWF[iSizeBy2+k]  = InverseQuadRootOfSumSquares(D, E);              // F[1024+j]

    D  = DIV2LPC(- T4  + T6 + T2 - TB);
    E  = DIV2LPC( + T7  - T8 - T9 + TA);
    pWF[(iSizeBy2<<1)-k]  = InverseQuadRootOfSumSquares(D, E);         // F[2048-j]

    //INTEGER_ONLY_LPC( assert( F[j]>=0 && F[(1024>>iShrink)-j]>=0 && F[(1024>>iShrink)+j]>=0 && F[(2048>>iShrink)-j]>=0 ) );

    //** This block uses 36 adds and 20 mults plus either 4 more mults or 4 shifts

}


//#endif  // defined(PLATFORM_SPECIFIC_DOLPC4)


WMARESULT prvLpcToSpectrum(CAudioObject* pau, const LpType* rgLpcCoef, PerChannelInfo* ppcinfo)
{
    // put often used variables near the top for easier access in platforms like the SH3 and SH4
    LpSpecType original[LPCORDER];
    LpSpecType *pFb;
    Int i, j;
    LpSpecType t1pO7, t1mO7, tO1pO9, tO1mO9, tO0pO8, tO0mO8, tO4pO6, tO4mO6;
    LpSpecType t1pO7pO3, t1pO7mO3, tO1pO5pO9, tO0pO2pO4pO6pO8;
    LpSpecType tS2x;
    LpSpecType tCpS1x, tCmS1x, tCpS2x, tCmS2x;
    LpSpecType CpS1, CmS1, CpS2, CmS2;
    LpSpecType D, E, F, G, H, I, J;

    LpSpecType* rgwtLpcSpec;
    WeightType wtLpcSpecMax, wtTemp;
    Int iFreq, iF, iLoopLimit;
    Int iShrink, iStride;
    LpSpecType Tmp[32];
    WeightType* pWF = (WeightType*)INTEGER_OR_INT_FLOAT_LPC((I32 *)ppcinfo->m_rguiWeightFactor, ppcinfo->m_rgfltWeightFactor);
    Int iSize, iSizeBy2, iSizeBy4, iSizeBy8, iSizeBy16;

    BP2LPCType SLC1, CLS1, CLC1, SLS1;

    BP2LPCType CT, ST;
    const SinCosTable* pSinCosTable;
    BP2LPCType S4, C4, S4p, C4p, STEP4;
    BP2LPCType S1, C1, S1p, C1p, STEP1;

    const BP2LPCType SL16    = BP2LPC_FROM_FLOAT(0.19509032201612826784828486847702);     // sim(pi/16)  = sin(pi*iSizeBy16*(1<<iShrink)/2048)
    const BP2LPCType CL16    = BP2LPC_FROM_FLOAT(0.98078528040323044912618223613424);     // cos(pi/16)  = sin(pi*iSizeBy16*(1<<iShrink)/2048)
    const BP2LPCType SL8     = BP2LPC_FROM_FLOAT(0.38268343236508977172845998403040);     // sim(pi/8)   = sin(pi*iSizeBy8*(1<<iShrink)/2048)
    const BP2LPCType CL8     = BP2LPC_FROM_FLOAT(0.92387953251128675612818318939679);     // cos(pi/8)   = sin(pi*iSizeBy8*(1<<iShrink)/2048)
    const BP2LPCType SL3by16 = BP2LPC_FROM_FLOAT(0.55557023301960222474283081394853);     // sim(pi3/16) = sin(pi*3*iSizeBy16*(1<<iShrink)/2048)
    const BP2LPCType CL3by16 = BP2LPC_FROM_FLOAT(0.83146961230254523707878837761791);     // cos(pi3/16) = sin(pi*3*iSizeBy16*(1<<iShrink)/2048)
    const BP2LPCType SL4     = BP2LPC_FROM_FLOAT(0.70710678118654752440084436210485);     // sim(pi/4)   = sin(pi*iSizeBy4*(1<<iShrink)/2048)
    const BP2LPCType CL4     = BP2LPC_FROM_FLOAT(0.70710678118654752440084436210485);     // cos(pi/4)   = sin(pi*iSizeBy4*(1<<iShrink)/2048)



    if (pau->m_fV5Lpc)
        iSize = ppcinfo->m_cSubband;
    else
        iSize = pau->m_cFrameSampleHalf;

    iShrink = LOG2(MAX_LP_SPEC_SIZE / iSize);   // for smaller transforms, shrink or expand indexing
    iStride = MAX_LP_SPEC_SIZE / iSize;         // for smaller transforms, stride past unused (lpc_compare only)
    iSizeBy2 = iSize >> 1;
    iSizeBy4 = iSizeBy2 >> 1;
    iSizeBy8 = iSizeBy4 >> 1;
    iSizeBy16 = iSizeBy8 >> 1;
    // iSize can be 2048, 1024, 512, 256, or 128.
    //      at 32000 Hz: 2048, 1024, 512 and 256
    //      at 22050 Hz: 1024, 512, 256 and 128
    //      at 16000 Hz: 512, 256, and 128
    //      at 11025 Hz: 512, 256, and 128
    //      at  8000 Hz: 512.



    // DEBUG_ONLY( if (pau->m_iFrameNumber==32) { DEBUG_BREAK(); } );

    assert(iSize <= MAX_LP_SPEC_SIZE);

    //for (i = 0; i < LPCORDER; i++)
    //WMAFprintf(stdout, "%.20lf\n", (double) FLOAT_FROM_LP(rgLpcCoef[i]));

    for (i = 0; i < LPCORDER; i++)
        original[i] = -LP_SPEC_FROM_LP(rgLpcCoef[i]);

    // F[128]
    Tmp[14]  = (t1pO7 = LP_SPEC_FROM_FLOAT(1) + original[7]) + (tS2x = multbp2lpcx(SQRT2, original[3]));
    // F[256]
    Tmp[22]  = (t1mO7 = LP_SPEC_FROM_FLOAT(1) - original[7]) + original[3];
    Tmp[30]  = t1mO7;                   // F[384]
    Tmp[31]  = t1pO7 - tS2x;            // F[640]
    Tmp[23]  = t1mO7 - original[3];     // F[768]
    Tmp[15]  = t1mO7;                   // F[896]

    // F[1152]
    Tmp[8]  = (tO1pO9 = original[1] + original[9]) + (tS2x = multbp2lpcx(SQRT2, original[5]));
    // F[1280]
    Tmp[16]  = (tO1mO9 = original[1] - original[9]) + original[5];
    Tmp[24]  = tO1mO9;                  // F[1408]
    Tmp[25]  = tO1pO9 - tS2x;           // F[1664]
    Tmp[17]  = tO1mO9 - original[5];    // F[1792]
    Tmp[9]  = tO1mO9;                   // F[1920]

    // F[2176]
    Tmp[10]  = (tO0pO8 = original[0] + original[8]) + (tS2x = multbp2lpcx(SQRT2, original[4]));
    // F[2304]
    Tmp[18]  = (tO0mO8 = original[0] - original[8]) + original[4];
    Tmp[26]  = tO0mO8;                  // F[2432]
    Tmp[27]  = tO0pO8 - tS2x;           // F[2688]
    Tmp[19]  = tO0mO8 - original[4];    // F[2816]
    Tmp[11]  = tO0mO8;                  // F[2944]

    // F[3200]
    Tmp[12]  = original[2] + (tS2x = multbp2lpcx(SQRT2, original[6]));
    Tmp[20]  = original[2] + original[6]; // F[3328]
    Tmp[28]  = original[2];             // F[3456]
    Tmp[29]  = original[2] - tS2x;      // F[3712]
    Tmp[21]  = original[2] - original[6]; // F[3840]
    Tmp[13]  = original[2];             // F[3968]

    tO4pO6 = original[4] + original[6];
    tO4mO6 = original[4] - original[6];
    t1pO7pO3 = t1pO7 + original[3];
    t1pO7mO3 = t1pO7 - original[3];
    tO1pO5pO9 = tO1pO9 + original[5];
    tO0pO2pO4pO6pO8 = tO0pO8 + tO4pO6 + original[2];

    D  = t1pO7pO3 + tO1pO5pO9 + tO0pO2pO4pO6pO8;    // F[0]
    E  = t1pO7mO3 + (tS2x = multbp2lpcx(SQRT2_2, tO0pO8 - tO4mO6 - original[2]));     // F[512]
    F  = t1pO7pO3 - tO1pO5pO9;                      // F[1024]
    G  = t1pO7mO3 - tS2x;                           // F[1536]
    H  = -tO1pO9 + original[5] + (tS2x = multbp2lpcx(SQRT2_2, tO0pO8 - tO4pO6 + original[2])); // F[2560]
    I  =  tO0pO8 + tO4mO6 - original[2];            // F[3072]
    J  =  tO1pO9 - original[5] + tS2x;              // F[3584]

    // j==0 and i==0 below.
    pFb   = (LpSpecType*) & pWF[0];
    *pFb  =  InverseQuadRootOfSumSquares(D, 0);     // F[0]
    pFb  +=  iSizeBy4;
    *pFb  =  InverseQuadRootOfSumSquares(E, J);     // F[512]    b: F[512]    a: F[3584]
    pFb  +=  iSizeBy4;
    *pFb  =  InverseQuadRootOfSumSquares(F, I);     // F[1024]    b: F[1024]   a: F[3072]
    pFb  +=  iSizeBy4;
    *pFb  =  InverseQuadRootOfSumSquares(G, H);     // F[1536]   b: F[1536]   a: F[2560]

    // k - example:  iSize = 0x100 = 256, j==0
    //         i:  0   1   2   3
    //    kdx         10  20  30
    // 80-kdx         70  60  50
    // 80+kdx         90  a0  b0
    //100-kdx         f0  e0  d0

    //DEBUG_ONLY(assert( fabs(FLOAT_FROM_BP2LPC(SL16)-FLOAT_FROM_BP2LPC(TRIGTBL(iSizeBy16,0)))<0.0001
    //                   && fabs(FLOAT_FROM_BP2LPC(CL16)-FLOAT_FROM_BP2LPC(TRIGTBL(iSizeBy16,1)))<0.0001 ));

    prvDoLpc4(iSizeBy16, Tmp + 8, pWF, iSizeBy2, SL16, CL16);

    //DEBUG_ONLY(assert( fabs(FLOAT_FROM_BP2LPC(SL8)-FLOAT_FROM_BP2LPC(TRIGTBL(iSizeBy8,0)))<0.0001
    //                    && fabs(FLOAT_FROM_BP2LPC(CL8)-FLOAT_FROM_BP2LPC(TRIGTBL(iSizeBy8,1)))<0.0001 ));

    prvDoLpc4(iSizeBy8,  Tmp + 16, pWF, iSizeBy2, SL8,  CL8);

    //DEBUG_ONLY(assert( fabs(FLOAT_FROM_BP2LPC(SL3by16)-FLOAT_FROM_BP2LPC(TRIGTBL(iSizeBy8 + iSizeBy16,0)))<0.0001
    //                    && fabs(FLOAT_FROM_BP2LPC(CL3by16)-FLOAT_FROM_BP2LPC(TRIGTBL(iSizeBy8 + iSizeBy16,1)))<0.0001 ));

    prvDoLpc4(iSizeBy8 + iSizeBy16, Tmp + 24, pWF, iSizeBy2, SL3by16, CL3by16);

    assert(64 <= iSize && iSize <= 2048);

    pSinCosTable = rgSinCosTables[iSize>>7];
    S4  = BP2LPC_FROM_BP2(pSinCosTable->sin_4PIbycSB);
    C4  = BP2LPC_FROM_BP2(pSinCosTable->cos_4PIbycSB);
    S4p = 0;
    C4p = BP2LPC_FROM_FLOAT(1.0f);
    STEP4 = MUL2LPC(S4);

    S1  = BP2LPC_FROM_BP1(pSinCosTable->sin_PIbycSB);
    C1  = BP2LPC_FROM_BP1(pSinCosTable->cos_PIbycSB);
    S1p = 0;
    C1p = BP2LPC_FROM_FLOAT(1.0f);
    STEP1 = BP2LPC_FROM_BP1(pSinCosTable->two_sin_PIbycSB);

    for (j = 1; j < iSizeBy16; j++)
    {
        BP2LPCType S2, C2;

        BP2LPCType C8, S8;

        assert(fabs(FLOAT_FROM_BP2LPC(S4) - sin(PI*4*j / iSize)) < 0.0001
               && fabs(FLOAT_FROM_BP2LPC(C4) - cos(PI*4*j / iSize)) < 0.0001);

        CmS1 = -C4 + S4;                    // cnst3[i*(4<<iShrink)+3];
        CpS1 =  C4 + S4;                    // cnst3[i*(4<<iShinrk)+1];

        S8 = MUL2LPC(multbp2lpcx(C4, S4));                       // sin(2x)
        C8 = BP2LPC_FROM_FLOAT(1.0f) - MUL2LPC(multbp2lpcx(S4, S4)); // cos(2x)
        CmS2 = -C8 + S8;                    // cnst3[i*(4<<iShrink)+2];
        CpS2 =  C8 + S8;                    // cnst3[i*(4<<iShink)];

        // rotate angle by b = 4*pi/iSize
        // recursion: cos(a+b) = cos(a-b) + 2*sin(b)*sin(a)
        // and:       sin(a+b) = sin(a-b) - 2*sin(b)*cos(a)
        CT = C4p - multbp2lpcx(STEP4, S4);
        ST = S4p + multbp2lpcx(STEP4, C4);
        C4p = C4;
        C4 = CT;
        S4p = S4;
        S4 = ST;

        assert(BP2LPC_FROM_FLOAT(1) <= CpS2 && CpS2 <= BP2LPC_FROM_FLOAT(1.5));
        // F[j]
        Tmp[6]  = LP_SPEC_FROM_FLOAT(1) + (tCpS2x = multbp2lpcx(CpS2, original[7])) + (tCpS1x = multbp2lpcx(CpS1, original[3]));
        // F[256-j]
        Tmp[14] = LP_SPEC_FROM_FLOAT(1) + (tCmS2x = multbp2lpcx(CmS2, original[7])) + tCpS1x;
        // F[256+j]
        Tmp[22] = LP_SPEC_FROM_FLOAT(1) - tCpS2x - (tCmS1x = multbp2lpcx(CmS1, original[3]));
        Tmp[30] = LP_SPEC_FROM_FLOAT(1) - tCmS2x + tCmS1x;  // F[512-j]
        Tmp[31] = LP_SPEC_FROM_FLOAT(1) + tCpS2x - tCpS1x;  // F[512+j]
        Tmp[23] = LP_SPEC_FROM_FLOAT(1) + tCmS2x - tCpS1x;  // F[768-j]
        Tmp[15] = LP_SPEC_FROM_FLOAT(1) - tCpS2x + tCmS1x;  // F[768+j]
        Tmp[7]  = LP_SPEC_FROM_FLOAT(1) - tCmS2x - tCmS1x;  // F[1024-j]

        // F[1024+j]
        Tmp[0]  = original[1] + (tCpS2x = multbp2lpcx(CpS2, original[9])) + (tCpS1x = multbp2lpcx(CpS1, original[5]));
        // F[1280-j]
        Tmp[8]  = original[1] + (tCmS2x = multbp2lpcx(CmS2, original[9])) + tCpS1x;
        // F[1280+j]
        Tmp[16] = original[1] - tCpS2x - (tCmS1x = multbp2lpcx(CmS1, original[5]));
        Tmp[24] = original[1] - tCmS2x + tCmS1x;            // F[1536-j]
        Tmp[25] = original[1] + tCpS2x - tCpS1x;            // F[1536+j]
        Tmp[17] = original[1] + tCmS2x - tCpS1x;            // F[1792-j]
        Tmp[9]  = original[1] - tCpS2x + tCmS1x;            // F[1792+j]
        Tmp[1]  = original[1] - tCmS2x - tCmS1x;            // F[2048-j

        // F[2048+j]
        Tmp[2]  = original[0] + (tCpS2x = multbp2lpcx(CpS2, original[8])) + (tCpS1x = multbp2lpcx(CpS1, original[4]));
        // F[2304-j]
        Tmp[10] = original[0] + (tCmS2x = multbp2lpcx(CmS2, original[8])) + tCpS1x;
        // F[2304+j]
        Tmp[18] = original[0] - tCpS2x - (tCmS1x = multbp2lpcx(CmS1, original[4]));
        Tmp[26] = original[0] - tCmS2x + tCmS1x;            // F[2560-j]
        Tmp[27] = original[0] + tCpS2x - tCpS1x;            // F[2560+j]
        Tmp[19] = original[0] + tCmS2x - tCpS1x;            // F[2816-j]
        Tmp[11] = original[0] - tCpS2x + tCmS1x;            // F[2816+j]
        Tmp[3]  = original[0] - tCmS2x - tCmS1x;            // F[3072-j]

        // F[3072+j]
        // F[3328-j]
        Tmp[4]  =  Tmp[12]  = original[2] + (tCpS1x = multbp2lpcx(CpS1, original[6]));
        // F[3328+j]
        Tmp[20] = original[2] - (tCmS1x = multbp2lpcx(CmS1, original[6]));
        Tmp[28] = original[2] + tCmS1x;                   // F[3584-j]
        Tmp[29] = original[2] - tCpS1x;                   // F[3584+j]
        Tmp[21] = original[2] - tCpS1x;                   // F[3840-j]
        Tmp[13] = original[2] + tCmS1x;                   // F[3840+j]
        Tmp[5]  = original[2] - tCmS1x;                   // F[4096-j]

        //** this block uses 52 adds and 14 mults??


        // example:  iSize = 0x100 = 256.
        //            j==1         |   j==2          |  j==3
        //       i:  0   1   2   3 |   0   1   2   3 |  0
        //    k      1  1f  21  3f |   2  1e  22  3e |  3 ...
        // 80+k     81  9f  a1  bf |  82  9e  a2  be | 83 ...
        // 80-k     7f  61  5f  41 |  7e  62  5e  42 | 7e ...
        //100-k     ff  e1  df  c1 |  fe  e2  de  c2 | fd ...


        //assert( fabs(FLOAT_FROM_BP2LPC(S1)-sin(PI*j/iSize)) < 0.0001
        //     && fabs(FLOAT_FROM_BP2LPC(C1)-cos(PI*j/iSize)) < 0.0001 );

        prvDoLpc4(j, Tmp, pWF, iSizeBy2, S1, C1);

        S2 = (SLC1 = multbp2lpcx(SL8, C1)) - (CLS1 = multbp2lpcx(CL8, S1));
        C2 = (CLC1 = multbp2lpcx(CL8, C1)) + (SLS1 = multbp2lpcx(SL8, S1));
        //DEBUG_ONLY(assert( fabs(FLOAT_FROM_BP2LPC(S2)-FLOAT_FROM_BP2LPC(TRIGTBL(iSizeBy8-j,0)))<0.0001
        //     && fabs(FLOAT_FROM_BP2LPC(C2)-FLOAT_FROM_BP2LPC(TRIGTBL(iSizeBy8-j,1)))<0.0001 ));
        prvDoLpc4(iSizeBy8 - j, Tmp + 8,  pWF, iSizeBy2, S2, C2);

        S2 = SLC1 + CLS1;
        C2 = CLC1 - SLS1;
        // DEBUG_ONLY(assert( fabs(FLOAT_FROM_BP2LPC(S2)-FLOAT_FROM_BP2LPC(TRIGTBL(iSizeBy8+j,0)))<0.0001
        //      && fabs(FLOAT_FROM_BP2LPC(C2)-FLOAT_FROM_BP2LPC(TRIGTBL(iSizeBy8+j,1)))<0.0001 ));
        prvDoLpc4(iSizeBy8 + j, Tmp + 16, pWF, iSizeBy2, S2, C2);

        S2 = multbp2lpcx(SL4, C1) - multbp2lpcx(CL4, S1);
        C2 = multbp2lpcx(CL4, C1) + multbp2lpcx(SL4, S1);
        // DEBUG_ONLY(assert( fabs(FLOAT_FROM_BP2LPC(S2)-FLOAT_FROM_BP2LPC(TRIGTBL(iSizeBy4-j,0)))<0.0001
        //      && fabs(FLOAT_FROM_BP2LPC(C2)-FLOAT_FROM_BP2LPC(TRIGTBL(iSizeBy4-j,1)))<0.0001 ));
        prvDoLpc4(iSizeBy4 - j, Tmp + 24, pWF, iSizeBy2, S2, C2);

        // rotate angle by b = pi/iSize
        // recursion: cos(a+b) = cos(a-b) + 2*sin(b)*sin(a)
        // and:       sin(a+b) = sin(a-b) - 2*sin(b)*cos(a)
        CT = C1p - multbp2lpcx(STEP1, S1);
        ST = S1p + multbp2lpcx(STEP1, C1);
        C1p = C1;
        C1 = CT;
        S1p = S1;
        S1 = ST;
    }
    //** this loop uses (512>>iShrink) * (36 adds and 24 mults)

    //** total to here:
    //**   (46 adds and 14 mults) + (N/16) * (52 adds and 14 mults) + (N/4) * (36 adds and 24 mults)
    //** TA = 46 + 3.25N + 9N      = 46 + 12.250N
    //** TM = 14 + (14/16)N + 6*N  = 14 +  6.875N
    //** An FFT takes order N*log(N)
    //** so we may need to substitute an optimized FFT for this on some platforms.

    // The inverse quad root has already been computed. Copy to destination, find max etc remain here.

    rgwtLpcSpec  = INTEGER_OR_INT_FLOAT_LPC((I32 *)ppcinfo->m_rguiWeightFactor, ppcinfo->m_rgfltWeightFactor);
    wtLpcSpecMax = WEIGHT_FROM_FLOAT(0.0F);
    iLoopLimit   = ppcinfo->m_cSubband;

    for (iFreq = 0, iF = 0; iFreq < iLoopLimit; iFreq++, iF += iStride)
    {
//        LPC_COMPARE_DEBUG( pau, iFreq, iFreq, iF, pWF, rgLpcCoef );

        wtTemp = pWF[iFreq];

        if (wtTemp > wtLpcSpecMax)
            wtLpcSpecMax = wtTemp;

//        MONITOR_RANGE(gMR_rgfltWeightFactor,wtTemp);
//        MONITOR_RANGE(gMR_fltLPC_F3,pWF[iFreq]);
    }

//    WF_PRINT(pWF);



    //if (wtLpcSpecMax == WEIGHT_FROM_FLOAT(0.0F))
    //    {
    //        //FUNCTION_PROFILE_STOP(&fp);
    //        return TraceResult(WMA_E_FAIL);
    //    }


    ppcinfo->m_wtMaxWeight = FINALWEIGHT_FROM_WEIGHT(wtLpcSpecMax);

    //FUNCTION_PROFILE_STOP(&fp);
    return WMA_OK;
}
#pragma arm section code

#endif
#endif
#endif
