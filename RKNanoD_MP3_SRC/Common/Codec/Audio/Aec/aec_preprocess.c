/* Copyright (C) 2003 Epic Games (written by Jean-Marc Valin)
   Copyright (C) 2004-2006 Epic Games 
   
   File: preprocess.c
   Preprocessor with denoising based on the algorithm by Ephraim and Malah

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


/*
   Recommended papers:
   
   Y. Ephraim and D. Malah, "Speech enhancement using minimum mean-square error
   short-time spectral amplitude estimator". IEEE Transactions on Acoustics, 
   Speech and Signal Processing, vol. ASSP-32, no. 6, pp. 1109-1121, 1984.
   
   Y. Ephraim and D. Malah, "Speech enhancement using minimum mean-square error
   log-spectral amplitude estimator". IEEE Transactions on Acoustics, Speech and 
   Signal Processing, vol. ASSP-33, no. 2, pp. 443-445, 1985.
   
   I. Cohen and B. Berdugo, "Speech enhancement for non-stationary noise environments".
   Signal Processing, vol. 81, no. 2, pp. 2403-2418, 2001.

   Stefan Gustafsson, Rainer Martin, Peter Jax, and Peter Vary. "A psychoacoustic 
   approach to combined acoustic echo cancellation and noise reduction". IEEE 
   Transactions on Speech and Audio Processing, 2002.
   
   J.-M. Valin, J. Rouat, and F. Michaud, "Microphone array post-filter for separation
   of simultaneous non-stationary sources". In Proceedings IEEE International 
   Conference on Acoustics, Speech, and Signal Processing, 2004.
*/



#include <math.h>
#include "speex_preprocess.h"

#include "arch.h"
#include "fftwrap.h"
#include "filterbank.h"
#include "math_approx.h"
#include "os_support.h"
#include "speex_echo.h"
#include "audio_main.h"

#ifdef _AEC_DECODE

#ifndef M_PI
#define M_PI 3.14159263
#endif

#define LOUDNESS_EXP 5.f
#define AMP_SCALE .001f
#define AMP_SCALE_1 1000.f
      
#define NB_BANDS 24
#define  nbands  NB_BANDS

#define SPEECH_PROB_START_DEFAULT       QCONST16(0.9f,15)
#define SPEECH_PROB_CONTINUE_DEFAULT    QCONST16(0.60f,15)
#define NOISE_SUPPRESS_DEFAULT       -15//-15
#define ECHO_SUPPRESS_DEFAULT        -40//-40
#define ECHO_SUPPRESS_ACTIVE_DEFAULT -15//-15

#define speech_prob_start  SPEECH_PROB_START_DEFAULT
#define speech_prob_continue  SPEECH_PROB_CONTINUE_DEFAULT

#ifndef NULL
#define NULL 0
#endif

#define SQR(x) ((x)*(x))
#define SQR16(x) (MULT16_16((x),(x)))
#define SQR16_Q15(x) (MULT16_16_Q15((x),(x)))

 _ATTR_AECALG_TEXT_
 spx_word16_t DIV32_16_Q8(spx_word32_t a, spx_word32_t b)
{
   if (SHR32(a,7) >= b)
   {
      return 32767;
   } else {
      if (b>=QCONST32(1,23))
      {
         a = SHR32(a,8);
         b = SHR32(b,8);
      }
      if (b>=QCONST32(1,19))
      {
         a = SHR32(a,4);
         b = SHR32(b,4);
      }
      if (b>=QCONST32(1,15))
      {
         a = SHR32(a,4);
         b = SHR32(b,4);
      }
      a = SHL32(a,8);
      return PDIV32_16(a,b);
   }
   
}
 _ATTR_AECALG_TEXT_
 spx_word16_t DIV32_16_Q15(spx_word32_t a, spx_word32_t b)
{
   if (SHR32(a,15) >= b)
   {
      return 32767;
   } else {
      if (b>=QCONST32(1,23))
      {
         a = SHR32(a,8);
         b = SHR32(b,8);
      }
      if (b>=QCONST32(1,19))
      {
         a = SHR32(a,4);
         b = SHR32(b,4);
      }
      if (b>=QCONST32(1,15))
      {
         a = SHR32(a,4);
         b = SHR32(b,4);
      }
      a = SHL32(a,15)-a;
      return DIV32_16(a,b);
   }
}
#define SNR_SCALING 256.f
#define SNR_SCALING_1 0.0039062f
#define SNR_SHIFT 8

#define FRAC_SCALING 32767.f
#define FRAC_SCALING_1 3.0518e-05
#define FRAC_SHIFT 1

#define EXPIN_SCALING 2048.f
#define EXPIN_SCALING_1 0.00048828f
#define EXPIN_SHIFT 11
#define EXPOUT_SCALING_1 1.5259e-05

#define NOISE_SHIFT 7



/** Speex pre-processor state. */

struct SpeexPreprocessState_ {
   /* Basic info */
   int    frame_size;        /**< Number of samples processed each time */
   int    ps_size;           /**< Number of points in the power spectrum */
   FilterBank *bank;   
   /* Parameters */  
   int    noise_suppress;
   int    echo_suppress;
   int    echo_suppress_active;    
 
   /* DSP-related arrays */
   spx_word16_t frame[2*Np];      /**< Processing frame (2*ps_size) */
   spx_word16_t ft[2*Np];         /**< Processing frame in freq domain (2*ps_size) */
   spx_word32_t ps[Np+NB_BANDS];         /**< Current power spectrum */ 
   spx_word16_t window[2*Np];     /**< Analysis/Synthesis window */
   spx_word32_t noise[Np+NB_BANDS];      /**< Noise estimate */  
  
   spx_word32_t old_ps[Np+NB_BANDS];     /**< Power spectrum for last frame */
   

   spx_word32_t S[Np];          /**< Smoothed power spectrum */
   spx_word32_t Smin[Np];       /**< See Cohen paper */
   spx_word32_t Stmp[Np];       /**< See Cohen paper */
   int update_prob[Np];         /**< Probability of speech presence for noise update */

   spx_word16_t zeta[Np+NB_BANDS];       /**< Smoothed a priori SNR */
   spx_word32_t echo_noise[Np+NB_BANDS];
  

   /* Misc */
   spx_word16_t inbuf[Np];      /**< Input buffer (overlapped analysis) */
   spx_word16_t outbuf[Np];     /**< Output buffer (for overlap and add) */

   /* AGC stuff, only for floating point for now */

   int    nb_adapt;          /**< Number of frames used for adaptation so far */

   int    min_count;         /**< Number of frames processed so far */
   void  *fft_lookup;        /**< Lookup table for the FFT */
   int    was_speech;
   int    frame_shift;

};
  _ATTR_AECALG_BSS_ static SpeexPreprocessState st1;


_ATTR_AECALG_TEXT_
void conj_window(spx_word16_t *w, int len)
{
   int i;
   for (i=0;i<len;i++)
   {
      spx_word16_t tmp;

      spx_word16_t x = DIV32_16(MULT16_16(32767,i),len);

      int inv=0;
      if (x<QCONST16(1.f,13))
      {
      } else if (x<QCONST16(2.f,13))
      {
         x=QCONST16(2.f,13)-x;
         inv=1;
      } else if (x<QCONST16(3.f,13))
      {
         x=x-QCONST16(2.f,13);
         inv=1;
      } else {
         x=QCONST16(2.f,13)-x+QCONST16(2.f,13); /* 4 - x */
      }
      x = MULT16_16_Q14(QCONST16(1.271903f,14), x);
      tmp = SQR16_Q15(QCONST16(.5f,15)-MULT16_16_P15(QCONST16(.5f,15),spx_cos_norm(SHL32(EXTEND32(x),2))));
      if (inv)
         tmp=SUB16(Q15_ONE,tmp);
      w[i]=spx_sqrt(SHL32(EXTEND32(tmp),15));
   }
}

      

/* This function approximates the gain function 
   y = gamma(1.25)^2 * M(-.25;1;-x) / sqrt(x)  
   which multiplied by xi/(1+xi) is the optimal gain
   in the loudness domain ( sqrt[amplitude] )
   Input in Q11 format, output in Q15
*/
_ATTR_AECALG_TEXT_
spx_word32_t hypergeom_gain(spx_word32_t xx)
{
   int ind;
   spx_word16_t frac;
   /* Q13 table */
   _ATTR_AECALG_DATA_
   static  spx_word16_t table[21] = {
       6730,  8357,  9868, 11267, 12563, 13770, 14898,
      15959, 16961, 17911, 18816, 19682, 20512, 21311,
      22082, 22827, 23549, 24250, 24931, 25594, 26241};
      ind = SHR32(xx,10);
      if (ind<0)
         return Q15_ONE;
      if (ind>19)
         return ADD32(EXTEND32(Q15_ONE),EXTEND32(DIV32_16(QCONST32(.1296,23), SHR32(xx,EXPIN_SHIFT-SNR_SHIFT))));
      frac = SHL32(xx-SHL32(ind,10),5);
      return SHL32(DIV32_16(PSHR32(MULT16_16(Q15_ONE-frac,table[ind]) + MULT16_16(frac,table[ind+1]),7),(spx_sqrt(SHL32(xx,15)+6711))),7);
}

_ATTR_AECALG_TEXT_
spx_word16_t qcurve(spx_word16_t x)
{
   x = MAX16(x, 1);
   return DIV32_16(SHL32(EXTEND32(32767),9),ADD16(512,MULT16_16_Q15(QCONST16(.60f,15),DIV32_16(32767,x))));
}

_ATTR_AECALG_TEXT_
/* Compute the gain floor based on different floors for the background noise and residual echo */
 void compute_gain_floor(int noise_suppress, int effective_echo_suppress, spx_word32_t *noise, spx_word32_t *echo, spx_word16_t *gain_floor, int len)
{
   int i;
   
   if (noise_suppress > effective_echo_suppress)
   {
      spx_word16_t noise_gain, gain_ratio;
      noise_gain = EXTRACT16(MIN32(Q15_ONE,SHR32(spx_exp(MULT16_16(QCONST16(0.11513,11),noise_suppress)),1)));
      gain_ratio = EXTRACT16(MIN32(Q15_ONE,SHR32(spx_exp(MULT16_16(QCONST16(.2302585f,11),effective_echo_suppress-noise_suppress)),1)));

      /* gain_floor = sqrt [ (noise*noise_floor + echo*echo_floor) / (noise+echo) ] */
      for (i=0;i<len;i++)
         gain_floor[i] = MULT16_16_Q15(noise_gain,
                                       spx_sqrt(SHL32(EXTEND32(DIV32_16_Q15(PSHR32(noise[i],NOISE_SHIFT) + MULT16_32_Q15(gain_ratio,echo[i]),
                                             (1+PSHR32(noise[i],NOISE_SHIFT) + echo[i]) )),15)));
   } else {
      spx_word16_t echo_gain, gain_ratio;
      echo_gain = EXTRACT16(MIN32(Q15_ONE,SHR32(spx_exp(MULT16_16(QCONST16(0.11513,11),effective_echo_suppress)),1)));
      gain_ratio = EXTRACT16(MIN32(Q15_ONE,SHR32(spx_exp(MULT16_16(QCONST16(.2302585f,11),noise_suppress-effective_echo_suppress)),1)));

      /* gain_floor = sqrt [ (noise*noise_floor + echo*echo_floor) / (noise+echo) ] */
      for (i=0;i<len;i++)
         gain_floor[i] = MULT16_16_Q15(echo_gain,
                                       spx_sqrt(SHL32(EXTEND32(DIV32_16_Q15(MULT16_32_Q15(gain_ratio,PSHR32(noise[i],NOISE_SHIFT)) + echo[i],
                                             (1+PSHR32(noise[i],NOISE_SHIFT) + echo[i]) )),15)));
   }
}

 _ATTR_AECALG_TEXT_
 SpeexPreprocessState *speex_preprocess_state_init(int frame_size, int sample_rate,void *fft_table)
{
   int i;
   int N, N3, N4, M;
  // static SpeexPreprocessState st1;
   SpeexPreprocessState *st = &st1;
   st->frame_size = frame_size;

   /* Round ps_size down to the nearest power of two */

   st->ps_size = st->frame_size;
   N = st->ps_size;
   N3 = 2*N - st->frame_size;
  // N4 = st->frame_size - N3;
   N4 = 2*st->frame_size -2*N;   
   
  
   st->noise_suppress = NOISE_SUPPRESS_DEFAULT;
   st->echo_suppress = ECHO_SUPPRESS_DEFAULT;
   st->echo_suppress_active = ECHO_SUPPRESS_ACTIVE_DEFAULT; 
   M = NB_BANDS;
   st->bank = filterbank_new(M, sampling_rate, N, 1);
   
  
   conj_window(st->window, 2*N3);
   for (i=2*N3;i<2*st->ps_size;i++)
      st->window[i]=Q15_ONE;
   
   if (N4>0)
   {
      for (i=N3-1;i>=0;i--)
      {
         st->window[i+N3+N4]=st->window[i+N3];
         st->window[i+N3]=1;
      }
   }
   for (i=0;i<N+M;i++)
   {
      st->noise[i]=QCONST32(1.f,NOISE_SHIFT);     
      st->old_ps[i]=1;  
   }

   for (i=0;i<N;i++)
      st->update_prob[i] = 1;
   for (i=0;i<N3;i++)
   {
      st->inbuf[i]=0;
      st->outbuf[i]=0;
   }


 // st->fft_lookup = spx_fft_init(2*N);
   st->fft_lookup = fft_table;
  
  
   st->nb_adapt=0;
   st->min_count=0;
   return st;
}



 _ATTR_AECALG_TEXT_
void preprocess_analysis(SpeexPreprocessState *st, short  int *x)
{
   int i;
   int N = st->ps_size;
   int N3 = 2*N - st->frame_size;
   int N4 = st->frame_size - N3;
   spx_word32_t *ps=st->ps;

   /* 'Build' input frame */
   for (i=0;i<N3;i++)
      st->frame[i]=st->inbuf[i];
   for (i=0;i<st->frame_size;i++)
      st->frame[N3+i]=x[i];
   
   /* Update inbuf */
   for (i=0;i<N3;i++)
      st->inbuf[i]=x[N4+i];

   /* Windowing */
   for (i=0;i<2*N;i++)
      st->frame[i] = MULT16_16_Q15(st->frame[i], st->window[i]);

   {
      spx_word16_t max_val=0;
      for (i=0;i<2*N;i++)
         max_val = MAX16(max_val, ABS16(st->frame[i]));
      st->frame_shift = 14-spx_ilog2(EXTEND32(max_val));
      for (i=0;i<2*N;i++)
         st->frame[i] = SHL16(st->frame[i], st->frame_shift);
   }

   
   /* Perform FFT */
   spx_fft(st->fft_lookup, st->frame, st->ft);
         
   /* Power spectrum */
   ps[0]=MULT16_16(st->ft[0],st->ft[0]);
   for (i=1;i<N;i++)
      ps[i]=MULT16_16(st->ft[2*i-1],st->ft[2*i-1]) + MULT16_16(st->ft[2*i],st->ft[2*i]);
   for (i=0;i<N;i++)
      st->ps[i] = PSHR32(st->ps[i], 2*st->frame_shift);

   filterbank_compute_bank32(st->bank, ps, ps+N);
}

 _ATTR_AECALG_TEXT_
void update_noise_prob(SpeexPreprocessState *st)
{
   int i;
   int min_range;
   int N = st->ps_size;

   for (i=1;i<N-1;i++)
      st->S[i] =  MULT16_32_Q15(QCONST16(.8f,15),st->S[i]) + MULT16_32_Q15(QCONST16(.05f,15),st->ps[i-1]) 
                      + MULT16_32_Q15(QCONST16(.1f,15),st->ps[i]) + MULT16_32_Q15(QCONST16(.05f,15),st->ps[i+1]);
   st->S[0] =  MULT16_32_Q15(QCONST16(.8f,15),st->S[0]) + MULT16_32_Q15(QCONST16(.2f,15),st->ps[0]);
   st->S[N-1] =  MULT16_32_Q15(QCONST16(.8f,15),st->S[N-1]) + MULT16_32_Q15(QCONST16(.2f,15),st->ps[N-1]);
   
   if (st->nb_adapt==1)
   {
      for (i=0;i<N;i++)
         st->Smin[i] = st->Stmp[i] = 0;
   }

   if (st->nb_adapt < 100)
      min_range = 15;
   else if (st->nb_adapt < 1000)
      min_range = 50;
   else if (st->nb_adapt < 10000)
      min_range = 150;
   else
      min_range = 300;
   if (st->min_count > min_range)
   {
      st->min_count = 0;
      for (i=0;i<N;i++)
      {
         st->Smin[i] = MIN32(st->Stmp[i], st->S[i]);
         st->Stmp[i] = st->S[i];
      }
   } else {
      for (i=0;i<N;i++)
      {
         st->Smin[i] = MIN32(st->Smin[i], st->S[i]);
         st->Stmp[i] = MIN32(st->Stmp[i], st->S[i]);      
      }
   }
   for (i=0;i<N;i++)
   {
      if (MULT16_32_Q15(QCONST16(.4f,15),st->S[i]) > st->Smin[i])
         st->update_prob[i] = 1;
      else
         st->update_prob[i] = 0;
     
   }

}

void speex_echo_get_residual(SpeexEchoState *st, spx_word32_t *Yout, int len);
  _ATTR_AECALG_TEXT_
  int speex_preprocess_run(SpeexPreprocessState *st, short *x,SpeexEchoState *echo_st)

{
   int i;
   int M;
   int N = st->ps_size;
   int N3 = 2*N - st->frame_size;
   int N4 = st->frame_size - N3;

   spx_word32_t residual_echo[Np+NB_BANDS];  
   spx_word16_t gain2[Np+NB_BANDS];      /**< Adjusted gains */
   spx_word16_t gain_floor[Np+NB_BANDS]; /**< Minimum gain allowed */
   spx_word16_t gain[Np+NB_BANDS];       /**< Ephraim Malah gain */
   spx_word16_t prior[Np+NB_BANDS];      /**< A-priori SNR */
    spx_word16_t post[Np+NB_BANDS];       /**< A-posteriori SNR */
   spx_word32_t *ps=st->ps;
   spx_word32_t Zframe;
   spx_word16_t Pframe;
   spx_word16_t beta, beta_1;
   spx_word16_t effective_echo_suppress;
   
   st->nb_adapt++;
   if (st->nb_adapt>20000)
      st->nb_adapt = 20000;
   st->min_count++;
   
   beta = MAX16(QCONST16(.03,15),DIV32_16(Q15_ONE,st->nb_adapt));
   beta_1 = Q15_ONE-beta;
   M = nbands;
   /* Deal with residual echo if provided */
 if (echo_st)
   {
      speex_echo_get_residual(echo_st, residual_echo, N);

      for (i=0;i<N;i++)
         st->echo_noise[i] = MAX32(MULT16_32_Q15(QCONST16(.6f,15),st->echo_noise[i]), residual_echo[i]);
      filterbank_compute_bank32(st->bank, st->echo_noise, st->echo_noise+N);
   } else {
      for (i=0;i<N+M;i++)
         st->echo_noise[i] = 0;
   }
  
   preprocess_analysis(st, x);
   update_noise_prob(st);  
   
   /* Update the noise estimate for the frequencies where it can be */
   for (i=0;i<N;i++)
   {
      if (!st->update_prob[i] || st->ps[i] < PSHR32(st->noise[i], NOISE_SHIFT))
         st->noise[i] = MAX32(EXTEND32(0),MULT16_32_Q15(beta_1,st->noise[i]) + MULT16_32_Q15(beta,SHL32(st->ps[i],NOISE_SHIFT)));
   }
   filterbank_compute_bank32(st->bank, st->noise, st->noise+N);

   /* Special case for first frame */
   if (st->nb_adapt==1)
      for (i=0;i<N+M;i++)
         st->old_ps[i] = ps[i];

   /* Compute a posteriori SNR */
   for (i=0;i<N+M;i++)
   {
      spx_word16_t gamma;
      
      /* Total noise estimate including residual echo and reverberation */
      spx_word32_t tot_noise =ADD32(ADD32(EXTEND32(1), PSHR32(st->noise[i],NOISE_SHIFT)) , st->echo_noise[i]);
      
      /* A posteriori SNR = ps/noise - 1*/
      post[i] = SUB16(DIV32_16_Q8(ps[i],tot_noise), QCONST16(1.f,SNR_SHIFT));
      post[i]=MIN16(post[i], QCONST16(100.f,SNR_SHIFT));
      
      /* Computing update gamma = .1 + .9*(old/(old+noise))^2 */
      gamma = QCONST16(.1f,15)+MULT16_16_Q15(QCONST16(.89f,15),SQR16_Q15(DIV32_16_Q15(st->old_ps[i],ADD32(st->old_ps[i],tot_noise))));
      
      /* A priori SNR update = gamma*max(0,post) + (1-gamma)*old/noise */
      prior[i] = EXTRACT16(PSHR32(ADD32(MULT16_16(gamma,MAX16(0,post[i])), MULT16_16(Q15_ONE-gamma,DIV32_16_Q8(st->old_ps[i],tot_noise))), 15));
      prior[i]=MIN16(prior[i], QCONST16(100.f,SNR_SHIFT));
   }

   

   /* Recursive average of the a priori SNR. A bit smoothed for the psd components */
   st->zeta[0] = PSHR32(ADD32(MULT16_16(QCONST16(.7f,15),st->zeta[0]), MULT16_16(QCONST16(.3f,15),prior[0])),15);
   for (i=1;i<N-1;i++)
      st->zeta[i] = PSHR32(ADD32(ADD32(ADD32(MULT16_16(QCONST16(.7f,15),st->zeta[i]), MULT16_16(QCONST16(.15f,15),prior[i])),
                           MULT16_16(QCONST16(.075f,15),prior[i-1])), MULT16_16(QCONST16(.075f,15),prior[i+1])),15);
   for (i=N-1;i<N+M;i++)
      st->zeta[i] = PSHR32(ADD32(MULT16_16(QCONST16(.7f,15),st->zeta[i]), MULT16_16(QCONST16(.3f,15),prior[i])),15);

   /* Speech probability of presence for the entire frame is based on the average filterbank a priori SNR */
   Zframe = 0;
   for (i=N;i<N+M;i++)
      Zframe = ADD32(Zframe, EXTEND32(st->zeta[i]));
   Pframe = QCONST16(.1f,15)+MULT16_16_Q15(QCONST16(.899f,15),qcurve(DIV32_16(Zframe,nbands)));
   
   effective_echo_suppress = EXTRACT16(PSHR32(ADD32(MULT16_16(SUB16(Q15_ONE,Pframe), st->echo_suppress), MULT16_16(Pframe, st->echo_suppress_active)),15));
  // printf("Pframe =  %d    effective_echo_suppress = %d \n",Pframe,effective_echo_suppress);
   
   compute_gain_floor(st->noise_suppress, effective_echo_suppress, st->noise+N, st->echo_noise+N, gain_floor+N, M);
         
   /* Compute Ephraim & Malah gain speech probability of presence for each critical band (Bark scale) 
      Technically this is actually wrong because the EM gaim assumes a slightly different probability 
      distribution */
   for (i=N;i<N+M;i++)
   {
      /* See EM and Cohen papers*/
      spx_word32_t theta;
      /* Gain from hypergeometric function */
      spx_word32_t MM;
      /* Weiner filter gain */
      spx_word16_t prior_ratio;
      /* a priority probability of speech presence based on Bark sub-band alone */
      spx_word16_t P1;
      /* Speech absence a priori probability (considering sub-band and frame) */
      spx_word16_t q;

      spx_word16_t tmp;

      
      prior_ratio = PDIV32_16(SHL32(EXTEND32(prior[i]), 15), ADD16(prior[i], SHL32(1,SNR_SHIFT)));
      theta = MULT16_32_P15(prior_ratio, QCONST32(1.f,EXPIN_SHIFT)+SHL32(EXTEND32(post[i]),EXPIN_SHIFT-SNR_SHIFT));

      MM = hypergeom_gain(theta);
      /* Gain with bound */
      gain[i] = EXTRACT16(MIN32(Q15_ONE, MULT16_32_Q15(prior_ratio, MM)));
      /* Save old Bark power spectrum */
      st->old_ps[i] = MULT16_32_P15(QCONST16(.2f,15),st->old_ps[i]) + MULT16_32_P15(MULT16_16_P15(QCONST16(.8f,15),SQR16_Q15(gain[i])),ps[i]);

      P1 = QCONST16(.199f,15)+MULT16_16_Q15(QCONST16(.8f,15),qcurve (st->zeta[i]));
      q = Q15_ONE-MULT16_16_Q15(Pframe,P1);

      theta = MIN32(theta, EXTEND32(32767));
/*Q8*/tmp = MULT16_16_Q15((SHL32(1,SNR_SHIFT)+prior[i]),EXTRACT16(MIN32(Q15ONE,SHR32(spx_exp(-EXTRACT16(theta)),1))));
      tmp = MIN16(QCONST16(3.,SNR_SHIFT), tmp); /* Prevent overflows in the next line*/
/*Q8*/tmp = EXTRACT16(PSHR32(MULT16_16(PDIV32_16(SHL32(EXTEND32(q),8),(Q15_ONE-q)),tmp),8));
      gain2[i]=DIV32_16(SHL32(EXTEND32(32767),SNR_SHIFT), ADD16(256,tmp));
	 

   }
   /* Convert the EM gains and speech prob to linear frequency */
   filterbank_compute_psd16(st->bank,gain2+N, gain2);
   filterbank_compute_psd16(st->bank,gain+N, gain);
   
   /* Use 1 for linear gain resolution (best) or 0 for Bark gain resolution (faster) */
   if (1)
   {
      filterbank_compute_psd16(st->bank,gain_floor+N, gain_floor);
   
      /* Compute gain according to the Ephraim-Malah algorithm -- linear frequency */
      for (i=0;i<N;i++)
      {
         spx_word32_t MM;
         spx_word32_t theta;
         spx_word16_t prior_ratio;
         spx_word16_t tmp;
         spx_word16_t p;
         spx_word16_t g;
         
         /* Wiener filter gain */
         prior_ratio = PDIV32_16(SHL32(EXTEND32(prior[i]), 15), ADD16(prior[i], SHL32(1,SNR_SHIFT)));
         theta = MULT16_32_P15(prior_ratio, QCONST32(1.f,EXPIN_SHIFT)+SHL32(EXTEND32(post[i]),EXPIN_SHIFT-SNR_SHIFT));

         /* Optimal estimator for loudness domain */
         MM = hypergeom_gain(theta);
         /* EM gain with bound */
         g = EXTRACT16(MIN32(Q15_ONE, MULT16_32_Q15(prior_ratio, MM)));
         /* Interpolated speech probability of presence */
         p = gain2[i];
                  
         /* Constrain the gain to be close to the Bark scale gain */
         if (MULT16_16_Q15(QCONST16(.333f,15),g) > gain[i])
            g = MULT16_16(3,gain[i]);
         gain[i] = g;
         
         /* Save old power spectrum */
         st->old_ps[i] = MULT16_32_P15(QCONST16(.2f,15),st->old_ps[i]) + MULT16_32_P15(MULT16_16_P15(QCONST16(.8f,15),SQR16_Q15(gain[i])),ps[i]);
         
         /* Apply gain floor */
         if (gain[i] < gain_floor[i])
            gain[i] = gain_floor[i];

         /* Exponential decay model for reverberation (unused) */
         /*st->reverb_estimate[i] = st->reverb_decay*st->reverb_estimate[i] + st->reverb_decay*st->reverb_level*st->gain[i]*st->gain[i]*st->ps[i];*/
         
         /* Take into account speech probability of presence (loudness domain MMSE estimator) */
         /* gain2 = [p*sqrt(gain)+(1-p)*sqrt(gain _floor) ]^2 */
         tmp = MULT16_16_P15(p,spx_sqrt(SHL32(EXTEND32(gain[i]),15))) + MULT16_16_P15(SUB16(Q15_ONE,p),spx_sqrt(SHL32(EXTEND32(gain_floor[i]),15)));
         gain2[i]=SQR16_Q15(tmp);

         /* Use this if you want a log-domain MMSE estimator instead */
         /*st->gain2[i] = pow(st->gain[i], p) * pow(st->gain_floor[i],1.f-p);*/
      }
   } else {
      for (i=N;i<N+M;i++)
      {
         spx_word16_t tmp;
         spx_word16_t p = gain2[i];
         gain[i] = MAX16(gain[i], gain_floor[i]);         
         tmp = MULT16_16_P15(p,spx_sqrt(SHL32(EXTEND32(gain[i]),15))) + MULT16_16_P15(SUB16(Q15_ONE,p),spx_sqrt(SHL32(EXTEND32(gain_floor[i]),15)));
         gain2[i]=SQR16_Q15(tmp);
      }
      filterbank_compute_psd16(st->bank,gain2+N, gain2);
   }
   
   /* If noise suppression is off, don't apply the gain (but then why call this in the first place!) */
      
   /* Apply computed gain */
   for (i=1;i<N;i++)
   {
      st->ft[2*i-1] = MULT16_16_P15(gain2[i],st->ft[2*i-1]);
      st->ft[2*i] = MULT16_16_P15(gain2[i],st->ft[2*i]);
   }
   st->ft[0] = MULT16_16_P15(gain2[0],st->ft[0]);
   st->ft[2*N-1] = MULT16_16_P15(gain2[N-1],st->ft[2*N-1]);
   
   /*FIXME: This *will* not work for fixed-point */


   /* Inverse FFT with 1/N scaling */
   spx_ifft(st->fft_lookup, st->ft, st->frame);
   /* Scale back to original (lower) amplitude */
   for (i=0;i<2*N;i++)
      st->frame[i] = PSHR16(st->frame[i], st->frame_shift);

   /*FIXME: This *will* not work for fixed-point */

   
   /* Synthesis window (for WOLA) */
   for (i=0;i<2*N;i++)
      st->frame[i] = MULT16_16_Q15(st->frame[i], st->window[i]);
 #if 0

   /* Perform overlap and add */
   for (i=0;i<N3;i++)
      x[i] = st->outbuf[i] + st->frame[i];
   for (i=0;i<N4;i++)
      x[N3+i] = st->frame[N3+i];
   
   /* Update outbuf */
   for (i=0;i<N3;i++)
      st->outbuf[i] = st->frame[st->frame_size+i];
#endif

#if 1
  if(Pframe > speech_prob_start  || (st->was_speech == 1 && Pframe > speech_prob_continue)){
  	st->was_speech=1;
	
   /* Perform overlap and add */
   for (i=0;i<N3;i++)
      x[i] = st->outbuf[i] + st->frame[i];
   for (i=0;i<N4;i++)
      x[N3+i] = st->frame[N3+i];
	}
   else{
   	 st->was_speech=0;
     for (i=0;i<N3;i++)
		x[i] = 1;
	 for (i=0;i<N4;i++)
		x[N3+i] = 1;     
   }
  /* Update outbuf */
   for (i=0;i<N3;i++)
      st->outbuf[i] = st->frame[st->frame_size+i];
 #endif
   
   
   return 1;
}

#endif
 
