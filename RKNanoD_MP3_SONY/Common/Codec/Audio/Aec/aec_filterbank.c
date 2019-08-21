/* Copyright (C) 2006 Jean-Marc Valin */
/**
   @file filterbank.c
   @brief Converting between psd and filterbank
 */
/*
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

   1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   3. The name of the author may not be used to endorse or promote products
   derived from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
   STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/


#include "filterbank.h"
#include "arch.h"
#include <math.h>
#include "math_approx.h"      
#include "audio_main.h"

#ifdef _AEC_DECODE

#define toBARK(n)   (MULT16_16(26829,spx_atan(SHR32(MULT16_16(97,n),2))) + MULT16_16(4588,spx_atan(MULT16_32_Q15(20,MULT16_16(n,n)))) + MULT16_16(3355,n))

       
#define toMEL(n)    (2595.f*log10(1.f+(n)/700.f))
_ATTR_AECALG_BSS_  static FilterBank  bank1;

_ATTR_AECALG_TEXT_
FilterBank *filterbank_new(int banks, spx_word32_t sampling, int len, int type)
{
   FilterBank *bank;
   spx_word32_t df;
   spx_word32_t max_mel, mel_interval;
   int i;
   int id1;
   int id2;
   //static FilterBank  bank1;
   df = DIV32(SHL32(sampling,15),MULT16_16(2,len));
   max_mel = toBARK(EXTRACT16(sampling/2));
   mel_interval = PDIV32(max_mel,banks-1);
   
   bank = &bank1;
   bank->nb_banks = banks;
   bank->len = len; 

   for (i=0;i<len;i++)
   {
      spx_word16_t curr_freq;
      spx_word32_t mel;
      spx_word16_t val;
      curr_freq = EXTRACT16(MULT16_32_P15(i,df));
      mel = toBARK(curr_freq);
      if (mel > max_mel)
         break;

      id1 = DIV32(mel,mel_interval);

      if (id1>banks-2)
      {
         id1 = banks-2;
         val = Q15_ONE;
      } else {
         val = DIV32_16(mel - id1*mel_interval,EXTRACT16(PSHR32(mel_interval,15)));
      }
      id2 = id1+1;
      bank->bank_left[i] = id1;
      bank->filter_left[i] = SUB16(Q15_ONE,val);
      bank->bank_right[i] = id2;
      bank->filter_right[i] = val;
   }
   
   /* Think I can safely disable normalisation for fixed-point (and probably float as well) */

   return bank;
}

_ATTR_AECALG_TEXT_
void filterbank_compute_bank32(FilterBank *bank, spx_word32_t *ps, spx_word32_t *mel)
{
   int i;
   for (i=0;i<bank->nb_banks;i++)
      mel[i] = 0;

   for (i=0;i<bank->len;i++)
   {
      int id;
      id = bank->bank_left[i];
      mel[id] += MULT16_32_P15(bank->filter_left[i],ps[i]);
      id = bank->bank_right[i];
      mel[id] += MULT16_32_P15(bank->filter_right[i],ps[i]);
   }
   /* Think I can safely disable normalisation that for fixed-point (and probably float as well) */
#ifndef FIXED_POINT
   /*for (i=0;i<bank->nb_banks;i++)
      mel[i] = MULT16_32_P15(Q15(bank->scaling[i]),mel[i]);
   */
#endif
}

_ATTR_AECALG_TEXT_
void filterbank_compute_psd16(FilterBank *bank, spx_word16_t *mel, spx_word16_t *ps)
{
   int i;
   for (i=0;i<bank->len;i++)
   {
      spx_word32_t tmp;
      int id1, id2;
      id1 = bank->bank_left[i];
      id2 = bank->bank_right[i];
      tmp = MULT16_16(mel[id1],bank->filter_left[i]);
      tmp += MULT16_16(mel[id2],bank->filter_right[i]);
      ps[i] = EXTRACT16(PSHR32(tmp,15));//�ж�  Ϊʲô
   }
}

#endif //_AEC_DECODE

