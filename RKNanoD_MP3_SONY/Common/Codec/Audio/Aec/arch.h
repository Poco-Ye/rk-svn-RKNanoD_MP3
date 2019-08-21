/* Copyright (C) 2003 Jean-Marc Valin */
/**
   @file arch.h
   @brief Various architecture definitions Speex
*/
/*
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   
   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
   
   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
   
   - Neither the name of the Xiph.org Foundation nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ARCH_H
#define ARCH_H
#include "fixed_generic.h"
#include "audio_main.h"


#define NM 2// the filter len /NN
#define NN 128 //the number of echo process samples
#define TAIL NN*NM    //the number of echo filter length
#define Np NN //the number of pre_process samples

#define sampling_rate 8000  //needed in preprocess.c


#ifndef SPEEX_VERSION
#define SPEEX_MAJOR_VERSION 1         /**< Major Speex version. */
#define SPEEX_MINOR_VERSION 1         /**< Minor Speex version. */
#define SPEEX_MICRO_VERSION 15        /**< Micro Speex version. */
#define SPEEX_EXTRA_VERSION ""        /**< Extra Speex version. */
#define SPEEX_VERSION "speex-1.2beta3"  /**< Speex version string. */
#endif




#define ABS(x) ((x) < 0 ? (-(x)) : (x))      /**< Absolute integer value. */
#define ABS16(x) ((x) < 0 ? (-(x)) : (x))    /**< Absolute 16-bit value.  */
#define MIN16(a,b) ((a) < (b) ? (a) : (b))   /**< Maximum 16-bit value.   */
#define MAX16(a,b) ((a) > (b) ? (a) : (b))   /**< Maximum 16-bit value.   */
#define ABS32(x) ((x) < 0 ? (-(x)) : (x))    /**< Absolute 32-bit value.  */
#define MIN32(a,b) ((a) < (b) ? (a) : (b))   /**< Maximum 32-bit value.   */
#define MAX32(a,b) ((a) > (b) ? (a) : (b))   /**< Maximum 32-bit value.   */



typedef short int spx_word16_t;
typedef int  spx_word32_t;
typedef spx_word32_t spx_mem_t;
typedef spx_word16_t spx_coef_t;
typedef spx_word16_t spx_lsp_t;
typedef spx_word32_t spx_sig_t;

#define Q15ONE 32767


#define Q15_ONE ((spx_word16_t)32767)




#endif
