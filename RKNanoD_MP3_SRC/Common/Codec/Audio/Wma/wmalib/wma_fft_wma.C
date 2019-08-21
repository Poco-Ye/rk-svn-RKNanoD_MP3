//*@@@+++@@@@******************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//*@@@---@@@@******************************************************************
//+-------------------------------------------------------------------------
//
//  Microsoft Windows Media
//
//  Copyright (C) Microsoft Corporation, 1999 - 1999
//
//  File:       fft.c
//
//  The FFT used for the DctIV and DctIV
//
//--------------------------------------------------------------------------

//#include <math.h>
//#include <assert_wma.h>
//#include <stdio.h>
//#include <limits.h>
#include "../include/audio_main.h"
#include "..\wmaInclude\msaudio.h"
#include "..\wmaInclude\macros.h"
#include "..\wmaInclude\AutoProfile.h"
//#include "math.h"
#include "..\wmaInclude\predefine.h"
//#define PI  3.1415926535897932384626433832795
//static const double dPI = PI;

/*时间抽选基2FFT及IFFT算法C语言实现*/
/*Author :Junyi Sun*/
/*Copyright 2004-2005*/
/*Mail:ccnusjy@yahoo.com.cn*/
/*定义复数类型*/
//typedef struct
//{
//    double real;
//    double img;
//}complex;

//complex *W; /*输入序列,变换核*/

//#ifdef RECALC
//#define RECALC_FFT
//#endif

// Define to get Split Radix FFt algorithm - which is expected to be 30% faster than Radix 2
//#define SPLIT_RADIX_FFT


//***************************************************************************
// Intel Optimized Library Prolog
//***************************************************************************
//static const double dPI = PI;       // 3.1415926535897932384626433832795;

// the following defines do most of the computational work of the FFT, so they should be done efficently
// note need ur and ui to be defined unless assembly routines which should not need them
// note pointers will be incremented as a side effect of these macros
// define these in assembly for any processor whose compiler needs further optimization

#ifdef A_CORE_DECODE
#ifdef WMA_DEC_INCLUDE

#if 0//ndef WMA_TARGET_X86
// the following tables save floating point conversions and trig function calls
// compiler is unwilling to evaluate a constant expression of the form cos(PI/4) as a constant
const BP1Type icosPIbynp[16] =
{
    BP1_FROM_FLOAT(-0.999999999999996),   // cos(dPI/1)
    BP1_FROM_FLOAT(-0.000000043711390),   // cos(dPI/2)
    BP1_FROM_FLOAT(0.707106765732237),    // cos(dPI/4)
    BP1_FROM_FLOAT(0.923879528329380),    // cos(dPI/8)
    BP1_FROM_FLOAT(0.980785279337272),    // cos(dPI/16)
    BP1_FROM_FLOAT(0.995184726404418),    // cos(dPI/32)
    BP1_FROM_FLOAT(0.998795456138147),    // cos(dPI/64)
    BP1_FROM_FLOAT(0.999698818679443),    // cos(dPI/128)
    BP1_FROM_FLOAT(0.999924701834954),    // cos(dPI/256)
    BP1_FROM_FLOAT(0.999981175281554),    // cos(dPI/512)
    BP1_FROM_FLOAT(0.999995293809314),    // cos(dPI/1024)
    BP1_FROM_FLOAT(0.999998823451636),    // cos(dPI/2048)
    BP1_FROM_FLOAT(0.999999705862866),    // cos(dPI/4096)
    BP1_FROM_FLOAT(0.999999926465714),    // cos(dPI/8192)
    BP1_FROM_FLOAT(0.999999981616428),    // cos(dPI/16384)
    BP1_FROM_FLOAT(0.999999995404107)
};  // cos(dPI/32768)
const BP1Type isinPIbynp[16] =
{
    BP1_FROM_FLOAT(0.000000087422780),    // sin(-dPI/1)
    BP1_FROM_FLOAT(-0.999999999999999),   // sin(-dPI/2)
    BP1_FROM_FLOAT(-0.707106796640858),   // sin(-dPI/4)
    BP1_FROM_FLOAT(-0.382683442461104),   // sin(-dPI/8)
    BP1_FROM_FLOAT(-0.195090327375064),   // sin(-dPI/16)
    BP1_FROM_FLOAT(-0.098017143048367),   // sin(-dPI/32)
    BP1_FROM_FLOAT(-0.049067675691754),   // sin(-dPI/64)
    BP1_FROM_FLOAT(-0.024541229205697),   // sin(-dPI/128)
    BP1_FROM_FLOAT(-0.012271538627189),   // sin(-dPI/256)
    BP1_FROM_FLOAT(-0.006135884819899),   // sin(-dPI/512)
    BP1_FROM_FLOAT(-0.003067956848339),   // sin(-dPI/1024)
    BP1_FROM_FLOAT(-0.001533980228972),   // sin(-dPI/2048)
    BP1_FROM_FLOAT(-0.000766990340086),   // sin(-dPI/4096)
    BP1_FROM_FLOAT(-0.000383495198243),   // sin(-dPI/8192)
    BP1_FROM_FLOAT(-0.000191747602647),   // sin(-dPI/16384)
    BP1_FROM_FLOAT(-0.000095873801764)
}; // sin(-dPI/32768)
#endif // WMA_TARGET_X86
//****************************************************************************************************************
//
// A Split Radix FFT for the DCT -
// See WMAConcepts.xls - Sheet SRFFT32 for how this works.
// See also Sorensen & Heldeman, IEEE Trans ASSP, Vol ASSP-34, #1, 2/86, pp152-156.
// And also G. M. Blair, Electronics & Comm Engr Journal, August 1995, pp169-177.
//
//****************************************************************************************************************
#define MULT_CBP3(a,b) (I32)((((I64)(a))*((I64)(b)))>>32)

#define BP2Const6(a,b,c,d,e,f) BP2_FROM_FLOAT(a),BP2_FROM_FLOAT(b),BP2_FROM_FLOAT(c), \
                               BP2_FROM_FLOAT(d),BP2_FROM_FLOAT(e),BP2_FROM_FLOAT(f)

_ATTR_WMA_COMMON_CODE_  const BP2Type rgcbp2SrFFTTrig[90] =  //STATIC TABLE
{
//        STEP1             STEP3
//        2sin(2pi/2^k) Cos(2pi/2^k)  Sin(2pi/2^k)  2sin(6pi/2^k)  Cos(6pi/2^k)  Sin(6pi/2^k)
    BP2Const6(1.41421356237, 0.70710678119, 0.70710678119, 1.41421356237, -0.70710678119, 0.70710678119),    // K=3
    BP2Const6(0.76536686473, 0.92387953251, 0.38268343237, 1.84775906502, 0.38268343237, 0.92387953251),    // K=4
    BP2Const6(0.39018064403, 0.98078528040, 0.19509032202, 1.11114046604, 0.83146961230, 0.55557023302),    // K=5
    BP2Const6(0.19603428066, 0.99518472667, 0.09801714033, 0.58056935451, 0.95694033573, 0.29028467725),    // K=6
    BP2Const6(0.09813534865, 0.99879545621, 0.04906767433, 0.29346094891, 0.98917650996, 0.14673047446),    // k=7
    BP2Const6(0.04908245705, 0.99969881870, 0.02454122852, 0.14712912720, 0.99729045668, 0.07356456360),    // k=8
    BP2Const6(0.02454307657, 0.99992470184, 0.01227153829, 0.07361444588, 0.99932238459, 0.03680722294),    // k=9
    BP2Const6(0.01227176930, 0.99998117528, 0.00613588465, 0.03681345981, 0.99983058180, 0.01840672991),    // k=10
    BP2Const6(0.00613591353, 0.99999529381, 0.00306795676, 0.01840750956, 0.99995764455, 0.00920375478),    // k=11
    BP2Const6(0.00306796037, 0.99999882345, 0.00153398019, 0.00920385224, 0.99998941108, 0.00460192612),    // k=12
    BP2Const6(0.00153398064, 0.99999970586, 0.00076699032, 0.00460193830, 0.99999735277, 0.00230096915),    // k=13
    BP2Const6(0.00076699038, 0.99999992647, 0.00038349519, 0.00230097067, 0.99999933819, 0.00115048534),    // k=14
    BP2Const6(0.00038349519, 0.99999998162, 0.00019174760, 0.00115048553, 0.99999983455, 0.00057524276),    // k=15
    BP2Const6(0.00019174760, 0.99999999540, 0.00009587380, 0.00057524279, 0.99999995864, 0.00028762139),    // k=16
    0, 0, 0, 0, 0, 0
};


//#ifdef BUILD_INTEGER
#   define INTDIV2(a) ((a)>>1)
//#else
//#   define INTDIV2(a) (a)
//#endif
#if defined(WMAHIGHRATE) || defined(WMAMIDRATELOWRATE)
#pragma arm section code = "WmaHighLowCommonCode"

#if (!WMA_OPT_FFT_ARM)
void prvFFT4DCT(CoefType data[], const Int nLog2np)
{
    Int np = (1 << nLog2np);
    Int np2 = np << 1;
    Int np2m7 = np2 - 7;
    CoefType *pxk, *pxi, *px0, *px1, *px2, *px3;
    CoefType *px = data;
    I32 i, j, k, iOffset, iStride, iStridem1;
    Int n2k, n2km1, n2km2, n2kp2, idx;
    CoefType tmp, ur0, ui0, ur1, ui1;
    BP2Type bp2Step1, bp2Cos1, bp2Sin1, bp2Step3, bp2Cos3, bp2Sin3;
    BP2Type bp2Cos1p, bp2Sin1p, bp2Cos3p, bp2Sin3p;
    BP2Type bp2Cos1T, bp2Sin1T, bp2Cos3T, bp2Sin3T;
    CoefType ur2, ui2, ur3, ui3, urp, uip, urm, uim;
    const BP2Type* pbp2Trig = rgcbp2SrFFTTrig;
    Int n2kp1;

    // bit reverse (same code as Radix 2)
    if (np > 2)
    {
        I32 n2, n21;

        n2 = np / 2;                        /// n2: 512
        n21 = np + 1;                       /// n21: 1025
        j = 0;
        for (i = 0; i < np; i += 4)         /// i,j: 0,0; 4,1024; 8,512; 12,1536; ... 2044,??? [255x]
        {

            if (i < j)
            {
                // swap 4 pairs of values (2 complex pairs with 2 others)
                // px[i] <-> px[j]; px[i+1] <-> px[j+1]
                // px[i+1+n21] <-> px[j+1+n21];  px[i+1+n21+1] <-> px[j+1+n21+1]
                pxi = &px[i];
                pxk = &px[j];
                tmp = *pxi;
                *pxi++ = *pxk;
                *pxk++ = tmp;
                tmp = *pxi;
                *pxi = *pxk;
                *pxk = tmp;
                pxi  += n21;
                pxk  += n21;
                tmp = *pxi;
                *pxi++ = *pxk;
                *pxk++ = tmp;
                tmp = *pxi;
                *pxi = *pxk;
                *pxk = tmp;
            }

            // swap 2 pairs of values (1 complex pair with another)
            // px[i+2] <-> px[j+np];  px[i+3] <-> px[j+np+1]
            pxi = &px[i+2];
            pxk = &px[j+np];
            tmp = *pxi;
            *pxi++ = *pxk;
            *pxk++ = tmp;
            tmp = *pxi;
            *pxi = *pxk;
            *pxk = tmp;

            k = n2;
            while (k <= j)                  // k: {1024} {1024,512} {1024} {1024,512,256} ...
            {
                j -= k;
                //k = k / 2;
                k = (k >> 1);
            }
            j += k;                         /// j: {1024} {512} {1536} {256} ...
        }
    }

    // Length 2 butterflies
    for (iOffset = 0, iStride = 8;
            iOffset < np2;
            iOffset = (iStride << 1) - 4, iStride <<= 2)
    {
        iStridem1 = iStride - 1;
        for (idx = iOffset, px1 = (px0 = px + iOffset) + 2;
                idx < np2;
                idx += iStride, px0 += iStridem1, px1 += iStridem1)
        {
            ur0 = *px0;
            ur1 = *px1;
            *px0++ = ur0 + ur1;
            *px1++ = ur0 - ur1;
            ui0 = *px0;
            ui1 = *px1;
            *px0 = ui0 + ui1;
            *px1 = ui0 - ui1;
        }

    }

    // Radix 4 like Butterflies - either with ot without multiplies
    n2k = 2;

    k = nLog2np-2;
    do
    {
#if 0		
        n2k <<= 1;
        n2km1 = n2k >> 1;
        n2km2 = n2k >> 2;
        n2kp2 = n2k << 2;

        n2kp1 = n2k << 1;
#else        
        n2km1 = n2k;
        n2k <<= 1;
		n2km2 = n2k >> 2;
		n2kp2 = n2km2 << 4;
		n2kp1 = n2kp2 >> 1;
#endif
        // we must scale all the px's by 2 that will not be accessed in the remainder of this main loop
        for (iOffset = n2kp1, iStride = n2k << 3;
                iOffset < np2;
                iOffset = (iStride << 1) - n2kp1, iStride <<= 2)
        {
            for (idx = iOffset; idx < np2; idx += iStride)
            {
                for (i = 0, px0 = px + idx; i < n2kp1; i++)
                    *px0++ >>= 1;
            }
        }

        // Trivial Butterflies - j==0 - no multiplies since Cos==1 and Sin==0
        for (iOffset = 0, iStride = n2kp2;
                iOffset < np2;
                iOffset = (iStride - n2k) << 1, iStride <<= 2)
        {
            px3 = (px2 = (px1 = (px0 = px + iOffset) + n2km1) + n2km1) + n2km1;
            iStridem1 = iStride - 1;
            for (idx = iOffset;
                    idx < np2m7;
                    idx += iStride, px0 += iStridem1, px1 += iStridem1, px2 += iStridem1, px3 += iStridem1)
            {
                urp = (ur0 = INTDIV2(*px2++)) + (ur1 = INTDIV2(*px3++));
                urm = ur0 - ur1;
                uip = (ui0 = INTDIV2(*px2--)) + (ui1 = INTDIV2(*px3--));
                uim = ui0 - ui1;
                *px2++  = (ur0 = INTDIV2(*px0)) - urp;
                *px0++  = ur0 + urp;
                *px3++  = (ur1 = INTDIV2(*px1)) - uim;
                *px1++  = ur1 + uim;
                *px2    = (ui0 = INTDIV2(*px0)) - uip;
                *px0    = ui0 + uip;
                *px3    = (ui1 = INTDIV2(*px1)) + urm;
                *px1    = ui1 - urm;
            }
        }
        // Now the non-trivial butterflies
        if (n2km2 > 1)
        {
            if (*pbp2Trig != 0)
            {
                // normal case with k <= 16
                bp2Step1 = *pbp2Trig++;     // 2*sin(2*pi/2^k)
                bp2Cos1  = *pbp2Trig++;     // cos(2*pi/2^k)
                bp2Sin1  = *pbp2Trig++;     // sin(2*pi/2^k)
                bp2Step3 = *pbp2Trig++;     // 2*sin(6*pi/2^k)
                bp2Cos3  = *pbp2Trig++;     // cos(6*pi/2^k)
                bp2Sin3  = *pbp2Trig++;     // sin(6*pi/2^k)
            }
            else
            {   // k > 16 is not normal - but cleaniness before smallness n2k=65536
                bp2Step1 = BP2_FROM_FLOAT(0.0/*2*sin(2*PI/n2k)*/);
                bp2Cos1  = BP2_FROM_FLOAT(1.0/*cos(2*PI/n2k)*/);
                bp2Sin1  = BP2_FROM_FLOAT(0.0/*sin(2*PI/n2k)*/);
                bp2Step3 = BP2_FROM_FLOAT(0.0/*2*sin(6*PI/n2k)*/);
                bp2Cos3  = BP2_FROM_FLOAT(1.0/*cos(6*PI/n2k)*/);
                bp2Sin3  = BP2_FROM_FLOAT(0.0/*sin(6*PI/n2k)*/);
            }
            bp2Cos1p = bp2Cos3p = BP2_FROM_FLOAT(1.0);
            bp2Sin1p = bp2Sin3p = BP2_FROM_FLOAT(0.0);
            for (j = 1; j < n2km2;  j++)
            {
                //assert( fabs(FLOAT_FROM_BP2(bp2Sin1) - sin((2*PI*j)/n2k)) < 0.0001 );
                //assert( fabs(FLOAT_FROM_BP2(bp2Sin3) - sin((6*PI*j)/n2k)) < 0.0001 );
                for (iOffset = j << 1, iStride = n2kp2;
                        iOffset < np2;
                        iOffset = (iStride - n2k + j) << 1, iStride <<= 2)
                {
                    px3 = (px2 = (px1 = (px0 = px + iOffset) + n2km1) + n2km1) + n2km1;
                    iStridem1 = iStride - 1;
                    for (idx = iOffset;
                            idx < np2m7;
                            idx += iStride, px0 += iStridem1, px1 += iStridem1, px2 += iStridem1, px3 += iStridem1)
                    {
						#if 0
                        // The pentium prefers this way, but still does not generate wonderful code
                        ur0 = INTDIV2(*px2++);
                        ui0 = INTDIV2(*px2--);
                        ur2 = MULT_CBP2(bp2Cos1, ur0) + MULT_CBP2(bp2Sin1, ui0);
                        ui2 = MULT_CBP2(bp2Cos1, ui0) - MULT_CBP2(bp2Sin1, ur0);
                        ur1 = INTDIV2(*px3++);
                        ui1 = INTDIV2(*px3--);
                        ur3 = MULT_CBP2(bp2Cos3, ur1) + MULT_CBP2(bp2Sin3, ui1);
                        ui3 = MULT_CBP2(bp2Cos3, ui1) - MULT_CBP2(bp2Sin3, ur1);
                        urp = ur2 + ur3;
                        ur0 = INTDIV2(*px0);
                        *px2++  = ur0 - urp;
                        *px0++  = ur0 + urp;
						

                        uim     = ui2 - ui3;
                        ur1 = INTDIV2(*px1);
                        *px3++  = ur1 - uim;
                        *px1++  = ur1 + uim;

                        uip     = ui2 + ui3;
                        ui0 = INTDIV2(*px0);
                        *px2    = ui0 - uip;
                        *px0    = ui0 + uip;

                        urm     = ur2 - ur3;
                        ui1 = INTDIV2(*px1);
                        *px3    = ui1 + urm;
                        *px1    = ui1 - urm;
                        #else
						ur0 = (*px2++);
                        ui0 = (*px2--);
                        ur2 = (MULT_CBP3((bp2Cos1*2), ur0) + MULT_CBP3((bp2Sin1*2), ui0));
                        ui2 = (MULT_CBP3((bp2Cos1*2), ui0) - MULT_CBP3((bp2Sin1*2), ur0));
						
                        ur1 = (*px3++);
                        ui1 = (*px3--);
                        ur3 = (MULT_CBP3((bp2Cos3*2), ur1) + MULT_CBP3((bp2Sin3*2), ui1));
                        ui3 = (MULT_CBP3((bp2Cos3*2), ui1) - MULT_CBP3((bp2Sin3*2), ur1));
                        urp = ur2 + ur3;
						
                        ur0 = INTDIV2(*px0);
                        *px2++  = ur0 - urp;
                        *px0++  = ur0 + urp;
						

                        uim     = ui2 - ui3;
                        ur1 = INTDIV2(*px1);
                        *px3++  = ur1 - uim;
                        *px1++  = ur1 + uim;

                        uip     = ui2 + ui3;
                        ui0 = INTDIV2(*px0);
                        *px2    = ui0 - uip;
                        *px0    = ui0 + uip;

                        urm     = ur2 - ur3;
                        ui1 = INTDIV2(*px1);
                        *px3    = ui1 + urm;
                        *px1    = ui1 - urm;
						#endif

                        //idx -= iStride;px0 += iStridem1; px1 += iStridem1; px2 += iStridem1; px3 += iStridem1;
                    }
                }
                if ((j + 1) < n2km2)
                {   // Trig Recurrsion for both 2*pi/2^k and 6*pi/2^k
                    // sin(a+b) = sin(a-b) + 2*sin(b)*cos(a)
                    // cos(a+b) = cos(a-b) - 2*sin(b)*sin(a)
                    // Lay these out like this as ahint to optimizer to overlap operations
                    bp2Sin1T = bp2Sin1p + MULT_BP2(bp2Step1, bp2Cos1);
                    bp2Cos1T = bp2Cos1p - MULT_BP2(bp2Step1, bp2Sin1);
                    bp2Sin3T = bp2Sin3p + MULT_BP2(bp2Step3, bp2Cos3);
                    bp2Cos3T = bp2Cos3p - MULT_BP2(bp2Step3, bp2Sin3);
                    bp2Sin1p = bp2Sin1;
                    bp2Sin1 = bp2Sin1T;
                    bp2Cos1p = bp2Cos1;
                    bp2Cos1 = bp2Cos1T;
                    bp2Sin3p = bp2Sin3;
                    bp2Sin3 = bp2Sin3T;
                    bp2Cos3p = bp2Cos3;
                    bp2Cos3 = bp2Cos3T;
                }
            }
        }

    }while(k--);
}
#endif


/*变址计算，将x(n)码位倒置*/
//void change(complex* x, long size_x)
//{
//    complex temp;
//    unsigned short i = 0, j = 0, k = 0;
//    double t;
//    for (i = 0;i < size_x;i++)
//    {
//        k = i;
//        j = 0;
//        t = (log(size_x) / log(2));
//        while ((t--) > 0)
//        {
//            j = j << 1;
//            j |= (k & 1);
//            k = k >> 1;
//        }
//        if (j > i)
//        {
//            temp = x[i];
//            x[i] = x[j];
//            x[j] = temp;
//        }
//    }
//}
#if 0
void add(complex a, complex b, complex *c)
{
    c->real = a.real + b.real;
    c->img = a.img + b.img;
}

void mul(complex a, complex b, complex *c)
{
    c->real = a.real * b.real - a.img * b.img;
    c->img = a.real * b.img + a.img * b.real;
}
void sub(complex a, complex b, complex *c)
{
    c->real = a.real - b.real;
    c->img = a.img - b.img;
}
void divi(complex a, complex b, complex *c)
{
    c->real = (a.real * b.real + a.img * b.img) / (b.real * b.real + b.img * b.img);
    c->img = (a.img * b.real - a.real * b.img) / (b.real * b.real + b.img * b.img);
}
#endif
///*快速傅里叶变换*/
//void fft(complex* x, long size_x)
//{
//    int i = 0, j = 0, k = 0, l = 0;
//    complex up, down, product;
//    change(x, size_x);
//    for (i = 0;i < log(size_x) / log(2) ;i++)    /*一级蝶形运算*/
//    {
//        l = 1 << i;
//        for (j = 0;j < size_x;j += 2 * l)       /*一组蝶形运算*/
//        {
//            for (k = 0;k < l;k++)    /*一个蝶形运算*/
//            {
//                mul(x[j+k+l], W[size_x*k/2/l], &product);
//                add(x[j+k], product, &up);
//                sub(x[j+k], product, &down);
//                x[j+k] = up;
//                x[j+k+l] = down;
//            }
//        }
//    }
//}



/*初始化变换核*/
//void initW(long size_x)
//{
//    int i;
//    W = (complex *)malloc(sizeof(complex) * size_x);
//    for (i = 0;i < size_x;i++)
//    {
//        W[i].real = cos(2 * PI / size_x * i);
//        W[i].img = -1 * sin(2 * PI / size_x * i);
//    }
//}
#if WMA_OPT_FFT_ARM
/* x[ch][k]=scale_factor*(SgM(0,N-1)(spec_coef[ch][k])*cos(pi/N*(n+(N+1)/2)+(k+0.5)))*/
WMARESULT auDctIV(long* rgiCoef, long fltAfterScaleFactor,//Float fltAfterScaleFactor,
                  const Int cSubbandAdjusted, short nFacExponent)//rgiCoef is Q31
{
    long *piCoefTop, *piCoefBottom, *piCoefBottomOut;
    long iTr, iTi, iBr, iBi;
    long CR, CI, UR, UI, STEP, CR1, CI1, CR2, CI2;
    long iFFTSize, i, cSB, nLog2SB;
    long  iFac;
    const SinCosTable* pSinCosTable;

    U32 iMagnitude = 0;



    // m_cSubbandAdjusted below deals with the need to scale transform results to compensate the fact
    // that we're inv transforming coefficients from a transform that was twice or half our size

    cSB = cSubbandAdjusted;
    iFFTSize = cSB / 2;
    nLog2SB = LOG2(cSB);



    // initialize sin/cos recursion
    // note this style of recurrsion is more accurate than Numerical Recipies 5.5.6
    if (64 <= cSB && cSB <= 2048)  //64<=N(cSB)<=2048
    {
        pSinCosTable = rgSinCosTables[cSB>>7];
//#       if defined(BUILD_INTEGER)
        iFac = fltAfterScaleFactor;//hxd iFac = (I32)ROUNDD( fac );
//#       else  // must be BUILD_INT_FLOAT
//            iFac = (BP1Type)fac;
//#       endif // BUILD_INTEGER or BUILD_INT_FLOAT
        // initial cosine/sine values
        CR = (((long)(((__int64)(iFac) * (__int64)(pSinCosTable->cos_PIby4cSB)) >> 32)) << 1);//Q31 MULT_BP1(iFac,pSinCosTable->cos_PIby4cSB);        // CR = (I32)(fac*cos(-PI/(4*m_cSubband)) * NF2BP1)
        CI = -(((long)(((__int64)(iFac) * (__int64)(pSinCosTable->sin_PIby4cSB)) >> 32)) << 1);//Q31 -MULT_BP1(iFac,pSinCosTable->sin_PIby4cSB);        // CI = (I32)(fac*sin(-PI/(4*m_cSubband)) * NF2BP1)
        // prior cosine/sine values to init Pre-FFT recurrsion trig( -PI/(4*M) - (-PI/M ) = trig( 3*PI/(4*M) )
        CR1 =  MULT_BP1(iFac, pSinCosTable->cos_3PIby4cSB); // CR = (I32)(fac*cos(+3*PI/(4*m_cSubband)) * NF2BP1)
        CI1 =  MULT_BP1(iFac, pSinCosTable->sin_3PIby4cSB); // CI = (I32)(fac*sin(+3*PI/(4*m_cSubband)) * NF2BP1)
        // rotation step for both recurrsions
        STEP = -pSinCosTable->two_sin_PIbycSB;    // STEP = 2*sin(-PI/m_cSubband)
        // prior cosine/sine values to init Post-FFT recurrsion
        CR2 =  pSinCosTable->cos_PIbycSB;     // CR = (I32)(cos( PI/m_cSubband) * NF2BP1)
        CI2 =  pSinCosTable->sin_PIbycSB;     // CI = (I32)(sin( PI/m_cSubband) * NF2BP1)
    }

    prvDctIV_ARM(rgiCoef, nLog2SB, CR, CI, CR1, CI1, STEP, CR2);


    if (nFacExponent > 0)
    { // This scaling needed in v1 bit-streams
        piCoefTop    = rgiCoef;
        iMagnitude <<= nFacExponent;
        for (i = cSB; i > 0; i--)
        {
            *piCoefTop++ <<= nFacExponent;
        }
    }

    return WMA_OK;
}
#else
WMARESULT auDctIV(long* rgiCoef, long iFac,//Float fltAfterScaleFactor,
                  const Int cSB, short nFacExponent)//rgiCoef is Q31
{
    long *piCoefTop, *piCoefBottom, *piCoefBottomOut;
    long iTr, iTi, iBr, iBi;
    long CR, CI, UR, UI, STEP, CR1, CI1, CR2, CI2;
    const Int iHalfFFTSize = cSB / 4,nLog2SB = LOG2(cSB);
	long i;
//    long  iFac;
    //const SinCosTable* pSinCosTable;

    //U32 iMagnitude = 0;


    // m_cSubbandAdjusted below deals with the need to scale transform results to compensate the fact
    // that we're inv transforming coefficients from a transform that was twice or half our size

    //cSB = cSubbandAdjusted;//cSB is N value
    //iHalfFFTSize = cSB / 4;
    //nLog2SB = LOG2(cSB);

    piCoefTop          = rgiCoef;
    piCoefBottomOut = piCoefBottom = rgiCoef + cSB - 1;
    //piCoefBottomOut    = rgiCoef + cSB - 1;     // separate pointer now obsolete but still used


    // initialize sin/cos recursion
    // note this style of recurrsion is more accurate than Numerical Recipies 5.5.6
    if (64 <= cSB && cSB <= 2048)  //64<=N(cSB)<=2048
    {
        const SinCosTable*pSinCosTable = rgSinCosTables[cSB>>7];//choose &g_sct2048=rgsincostables[2048>>7]

        //iFac = fltAfterScaleFactor;//hxd iFac = (I32)ROUNDD( fac );

        // initial cosine/sine values
        CR = MULT_BP1(iFac,pSinCosTable->cos_PIby4cSB);        // CR = (I32)(fac*cos(-PI/(4*m_cSubband)) * NF2BP1)
        CI = -MULT_BP1(iFac,pSinCosTable->sin_PIby4cSB);        // CI = (I32)(fac*sin(-PI/(4*m_cSubband)) * NF2BP1)
        // prior cosine/sine values to init Pre-FFT recurrsion trig( -PI/(4*M) - (-PI/M ) = trig( 3*PI/(4*M) )
        CR1 =  MULT_BP1(iFac, pSinCosTable->cos_3PIby4cSB); // CR = (I32)(fac*cos(+3*PI/(4*m_cSubband)) * NF2BP1)
        CI1 =  MULT_BP1(iFac, pSinCosTable->sin_3PIby4cSB); // CI = (I32)(fac*sin(+3*PI/(4*m_cSubband)) * NF2BP1)
        // rotation step for both recurrsions
        STEP = -pSinCosTable->two_sin_PIbycSB;              // STEP = 2*sin(-PI/m_cSubband)
        // prior cosine/sine values to init Post-FFT recurrsion
        CR2 =  pSinCosTable->cos_PIbycSB;                   // CR = (I32)(cos( PI/m_cSubband) * NF2BP1)
        CI2 =  pSinCosTable->sin_PIbycSB;                   // CI = (I32)(sin( PI/m_cSubband) * NF2BP1)
    }

    for (i = iHalfFFTSize; i > 0; i--)
    {

        iBi = piCoefBottom[0];
        //piCoefBottomOut[0] = piCoefTop[1];
        piCoefBottom[0] = piCoefTop[1];

        iTr = piCoefTop[0];
        piCoefTop[0] =  MULT_CBP1(CR,iTr) - MULT_CBP1(CI,iBi);
        piCoefTop[1] =  MULT_CBP1(CR,iBi) + MULT_CBP1(CI,iTr);

        // rotate angle by -b = -pi/cSubband
        // recursion: cos(a-b) = cos(a+b) - 2*sin(b)*sin(a)
        // and:       sin(a-b) = sin(a+b) + 2*sin(b)*cos(a)
        UR = CR1 - MULT_BP1(STEP,CI);
        UI = CI1 + MULT_BP1(STEP,CR);
        CR1 = CR;
        CR = UR;
        CI1 = CI;
        CI = UI;

        piCoefTop += 2;
        piCoefBottom -= 2;
        //piCoefBottomOut -= 2;
    }

    for (i = iHalfFFTSize; i > 0; i--)
    {
        iTr = piCoefTop[0];
        iTi = piCoefTop[1];
        piCoefTop[0] =  MULT_CBP1(CR,iTr) - MULT_CBP1(CI,iTi);
        piCoefTop[1] =  MULT_CBP1(CR,iTi) + MULT_CBP1(CI,iTr);

        // rotate angle by b = -pi/cSubband
        UR = CR1 - MULT_BP1(STEP,CI);
        UI = CI1 + MULT_BP1(STEP,CR);
        CR1 = CR;
        CR = UR;
        CI1 = CI;
        CI = UI;

        piCoefTop += 2;
    }


    prvFFT4DCT(rgiCoef, nLog2SB - 1);//nLog2SB = LOG2( cSB ); level



    piCoefTop      = rgiCoef;           //reuse this pointer; start from head;
    piCoefBottom   = rgiCoef + cSB - 2; //reuse this pointer; start from tail;
    CR = BP1_FROM_FLOAT(1);             //one
    CI = 0;                             //zero

    for (i = iHalfFFTSize; i > 0; i--)
    {
        iTr = piCoefTop[0];
        iTi = piCoefTop[1];
        iBr = piCoefBottom[0];
        iBi = piCoefBottom[1];

        piCoefTop[0] =  MULT_CBP1(CR,iTr) -  MULT_CBP1(CI,iTi);
        piCoefBottom[1] = MULT_CBP1(-CI,iTr) - MULT_CBP1(CR,iTi);

        // rotate angle by -b = -pi/cSubband
        // recursion: cos(a-b) = cos(a+b) - 2*sin(b)*sin(a)
        // and:       sin(a-b) = sin(a+b) + 2*sin(b)*cos(a)
        UR = CR2 - MULT_BP1(STEP,CI);
        UI = CI2 + MULT_BP1(STEP,CR);
        CR2 = CR;
        CR = UR;
        CI2 = CI;
        CI = UI;

        // note that cos(-(cSubband/2 - i)*pi/cSubband ) = -sin( -i*pi/cSubband )
        piCoefTop[1] = MULT_CBP1(CR,iBr) + MULT_CBP1(CI,iBi);
        piCoefBottom[0] = MULT_CBP1(-CI,iBr) +  MULT_CBP1(CR,iBi);

        piCoefTop += 2;
        piCoefBottom -= 2;
    }

    if (nFacExponent > 0)
    {   // This scaling needed in v1 bit-streams
        piCoefTop      = rgiCoef;
        //iMagnitude <<= nFacExponent;
        for (i = cSB; i > 0; i--)
        {
            *piCoefTop++ <<= nFacExponent;
        }
    }

    return WMA_OK;
}

#endif
#pragma arm section code

#endif//defined(WMAHIGHRATE) || defined(WMAMIDRATELOWRATE)


#endif
#endif





































