/* Copyright (C) 2003-2008 Jean-Marc Valin

   File: mdf.c
   Echo canceller based on the MDF algorithm (see below)

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
   The echo canceller is based on the MDF algorithm described in:

   J. S. Soo, K. K. Pang Multidelay block frequency adaptive filter, 
   IEEE Trans. Acoust. Speech Signal Process., Vol. ASSP-38, No. 2, 
   February 1990.
   
   We use the Alternatively Updated MDF (AUMDF) variant. Robustness to 
   double-talk is achieved using a variable learning rate as described in:
   
   Valin, J.-M., On Adjusting the Learning Rate in Frequency Domain Echo 
   Cancellation With Double-Talk. IEEE Transactions on Audio,
   Speech and Language Processing, Vol. 15, No. 3, pp. 1030-1034, 2007.
   http://people.xiph.org/~jm/papers/valin_taslp2006.pdf
   
   There is no explicit double-talk detection, but a continuous variation
   in the learning rate based on residual echo, double-talk and background
   noise.
   
   About the fixed-point version:
   All the signals are represented with 16-bit words. The filter weights 
   are represented with 32-bit words, but only the top 16 bits are used
   in most cases. The lower 16 bits are completely unreliable (due to the
   fact that the update is done only on the top bits), but help in the
   adaptation -- probably by removing a "threshold effect" due to
   quantization (rounding going to zero) when the gradient is small.
   
   Another kludge that seems to work good: when performing the weight
   update, we only move half the way toward the "goal" this seems to
   reduce the effect of quantization noise in the update phase. This
   can be seen as applying a gradient descent on a "soft constraint"
   instead of having a hard constraint.
   
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "arch.h"
#include "speex_echo.h"
#include "fftwrap.h"
#include "pseudofloat.h"
#include "math_approx.h"
#include "audio_main.h"


#ifdef _AEC_DECODE


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


#define WEIGHT_SHIFT 11
#define NORMALIZE_SCALEDOWN 5
#define NORMALIZE_SCALEUP 3




#define N1 2*NN
#define M1 NM



#define WORD2INT(x) ((x) < -32767 ? -32768 : ((x) > 32766 ? 32767 : (x)))  


/* If enabled, the AEC will use a foreground filter and a background filter to be more robust to double-talk
   and difficult signals in general. The cost is an extra FFT and a matrix-vector multiply */
#define TWO_PATH
_ATTR_AECALG_TEXT_
static const spx_float_t MIN_LEAK = {20972, -22};

/* Constants for the two-path filter */
_ATTR_AECALG_TEXT_
static const spx_float_t VAR1_SMOOTH = {23593, -16};
_ATTR_AECALG_TEXT_
static const spx_float_t VAR2_SMOOTH = {23675, -15};
_ATTR_AECALG_TEXT_
static const spx_float_t VAR1_UPDATE = {16384, -15};
_ATTR_AECALG_TEXT_
static const spx_float_t VAR2_UPDATE = {16384, -16};
_ATTR_AECALG_TEXT_
static const spx_float_t VAR_BACKTRACK = {16384, -12};
#define TOP16(x) ((x)>>16)


#define PLAYBACK_DELAY 2

void speex_echo_get_residual(SpeexEchoState *st, spx_word32_t *Yout, int len);


/** Speex echo cancellation state. */
typedef struct SpeexEchoState_ {
   int frame_size;           /**< Number of samples processed each time */
   int window_size;
   int M;
   int cancel_count;
   int adapted;
   int saturated;
   spx_word16_t spec_average;
   spx_word16_t beta0;
   spx_word16_t beta_max;
   spx_word32_t sum_adapt;
   spx_word16_t leak_estimate;      
   
   spx_word16_t x[N1];      /* Far-end input buffer (2N) */
   spx_word16_t X[(M1+1)*N1];      /* Far-end buffer (M+1 frames) in frequency domain */  
   spx_word16_t last_y[N1];
   spx_word16_t E[N1];    
   spx_word32_t W[M1*N1];      /* (Background) filter weights */
#ifdef TWO_PATH
   spx_word16_t foreground[M1*N1]; /* Foreground filter weights */
   spx_word32_t  Davg1;  /* 1st recursive average of the residual power difference */
   spx_word32_t  Davg2;  /* 2nd recursive average of the residual power difference */
   spx_float_t   Dvar1;  /* Estimated variance of 1st estimator */
   spx_float_t   Dvar2;  /* Estimated variance of 2nd estimator */ 
#endif
   spx_word32_t power[NN+1];  /* Power of the far-end signal */
   spx_float_t  power_1[NN+1];/* Inverse power of far-end */   
   spx_float_t   Pey;
   spx_float_t   Pyy;
   spx_word16_t window[N1];
   spx_word16_t prop[M1];
   void *fft_table;
   spx_word16_t memX[1], memD[1], memE[1];  
   spx_word16_t preemph;
   spx_word16_t notch_radius;
   spx_mem_t notch_mem[2];

}SpeexEchoState;

_ATTR_AECALG_BSS_  static SpeexEchoState	EchoSt;
 _ATTR_AECALG_TEXT_
 void filter_dc_notch16(const short  int *in, spx_word16_t radius, spx_word16_t *out, int len, spx_mem_t *mem)
{
   int i;
   spx_word16_t den2;     

   den2 = MULT16_16_Q15(radius,radius) + MULT16_16_Q15(QCONST16(.7,15),MULT16_16_Q15(32767-radius,32767-radius));
   
   for (i=0;i<len;i++)
   {
      spx_word16_t vin = in[i];
      spx_word32_t vout = mem[0] + SHL32(EXTEND32(vin),15);
      mem[0] = mem[1] + SHL32(SHL32(-EXTEND32(vin),15) + MULT16_32_Q15(radius,vout),1);
      mem[1] = SHL32(EXTEND32(vin),15) - MULT16_32_Q15(den2,vout);
      out[i] = SATURATE32(PSHR32(MULT16_32_Q15(radius,vout),15),32767);
   }
}

_ATTR_AECALG_TEXT_
/* This inner product is slightly different from the codec version because of fixed-point */
spx_word32_t mdf_inner_prod(const spx_word16_t *x, const spx_word16_t *y, int len)
{
   spx_word32_t sum=0;
   len >>= 1;
   while(len--)
   {
      spx_word32_t part=0;
      part = MAC16_16(part,*x++,*y++);
      part = MAC16_16(part,*x++,*y++);  //part =x[i]y[i] +x[i+1]y[i+1]
      /* HINT: If you had a 40-bit accumulator, you could shift only at the end */
      sum = ADD32(sum,SHR32(part,6));  
   }
   return sum;
}
_ATTR_AECALG_TEXT_
/** Compute power spectrum of a half-complex (packed) vector */
void power_spectrum(const spx_word16_t *X, spx_word32_t *ps, int N)
{
   int i, j;
   ps[0]=MULT16_16(X[0],X[0]);
   for (i=1,j=1;i<N-1;i+=2,j++)
   {
      ps[j] =  MULT16_16(X[i],X[i]) + MULT16_16(X[i+1],X[i+1]);
   }
   ps[j]=MULT16_16(X[i],X[i]);
}
_ATTR_AECALG_TEXT_
/** Compute power spectrum of a half-complex (packed) vector and accumulate */
void power_spectrum_accum(const spx_word16_t *X, spx_word32_t *ps,int N)
{
   int i, j;
   ps[0]+=MULT16_16(X[0],X[0]);
   for (i=1,j=1;i<N-1;i+=2,j++)
   {
      ps[j] +=  MULT16_16(X[i],X[i]) + MULT16_16(X[i+1],X[i+1]);
   }
   ps[j]+=MULT16_16(X[i],X[i]);// ps[0]= X[0]的平方    ps[j]= 前后两个的平方和  j大于0小于160  ps[160]=X[319]的 平方
}

/** Compute cross-power spectrum of a half-complex (packed) vectors and add to acc */
_ATTR_AECALG_TEXT_
void spectral_mul_accum(const spx_word16_t *X, const spx_word32_t *Y, spx_word16_t *acc, int N, int M)
{
   int i,j;
   spx_word32_t tmp1=0,tmp2=0;
   for (j=0;j<M;j++)
   {
      tmp1 = MAC16_16(tmp1, X[j*N],TOP16(Y[j*N]));
   }
   acc[0] = PSHR32(tmp1,WEIGHT_SHIFT);
   for (i=1;i<N-1;i+=2)
   {
      tmp1 = tmp2 = 0;
      for (j=0;j<M;j++)
      {
         tmp1 = SUB32(MAC16_16(tmp1, X[j*N+i],TOP16(Y[j*N+i])), MULT16_16(X[j*N+i+1],TOP16(Y[j*N+i+1])));
         tmp2 = MAC16_16(MAC16_16(tmp2, X[j*N+i+1],TOP16(Y[j*N+i])), X[j*N+i], TOP16(Y[j*N+i+1]));
      }
      acc[i] = PSHR32(tmp1,WEIGHT_SHIFT);
      acc[i+1] = PSHR32(tmp2,WEIGHT_SHIFT);
   }
   tmp1 = tmp2 = 0;
   for (j=0;j<M;j++)
   {
      tmp1 = MAC16_16(tmp1, X[(j+1)*N-1],TOP16(Y[(j+1)*N-1]));
   }
   acc[N-1] = PSHR32(tmp1,WEIGHT_SHIFT);
}
 _ATTR_AECALG_TEXT_
 void spectral_mul_accum16(const spx_word16_t *X, const spx_word16_t *Y, spx_word16_t *acc,int N,int  M)
{
   int i,j;
   spx_word32_t tmp1=0,tmp2=0;
   for (j=0;j<M;j++)
   {
      tmp1 = MAC16_16(tmp1, X[j*N],Y[j*N]);
   }
   acc[0] = PSHR32(tmp1,WEIGHT_SHIFT);
   for (i=1;i<N-1;i+=2)
   {
      tmp1 = tmp2 = 0;
      for (j=0;j<M;j++)
      {
         tmp1 = SUB32(MAC16_16(tmp1, X[j*N+i],Y[j*N+i]), MULT16_16(X[j*N+i+1],Y[j*N+i+1]));//tmp1=tmp1+X[i]Y[i]-X[i+1]Y[i+1]
         tmp2 = MAC16_16(MAC16_16(tmp2, X[j*N+i+1],Y[j*N+i]), X[j*N+i], Y[j*N+i+1]);//tmp2 =  tmp2 +X[i+1]Y[i] + X[i]Y[i+1]
      }
      acc[i] = PSHR32(tmp1,WEIGHT_SHIFT);
      acc[i+1] = PSHR32(tmp2,WEIGHT_SHIFT);
   }
   tmp1 = tmp2 = 0;
   for (j=0;j<M;j++)
   {
      tmp1 = MAC16_16(tmp1, X[(j+1)*N-1],Y[(j+1)*N-1]);
   }
   acc[N-1] = PSHR32(tmp1,WEIGHT_SHIFT);
}


_ATTR_AECALG_TEXT_
/** Compute weighted cross-power spectrum of a half-complex (packed) vector with conjugate */
 void weighted_spectral_mul_conj(const spx_float_t *w, const spx_float_t p, const spx_word16_t *X, const spx_word16_t *Y, spx_word32_t *prod,int  N)
{
   int i, j;
   spx_float_t W;
   W = FLOAT_AMULT(p, w[0]);
   prod[0] = FLOAT_MUL32(W,MULT16_16(X[0],Y[0]));
   for (i=1,j=1;i<N-1;i+=2,j++)
   {
      W = FLOAT_AMULT(p, w[j]);
      prod[i] = FLOAT_MUL32(W,MAC16_16(MULT16_16(X[i],Y[i]), X[i+1],Y[i+1]));
      prod[i+1] = FLOAT_MUL32(W,MAC16_16(MULT16_16(-X[i+1],Y[i]), X[i],Y[i+1]));	 
   }
   W = FLOAT_AMULT(p, w[j]);
   prod[i] = FLOAT_MUL32(W,MULT16_16(X[i],Y[i]));
}

 _ATTR_AECALG_TEXT_
 void mdf_adjust_prop(const spx_word32_t *W,int N,int  M, spx_word16_t *prop)
{
   int i, j;
   spx_word16_t max_sum = 1;
   spx_word32_t prop_sum = 1;
   for (i=0;i<M;i++)
   {
      spx_word32_t tmp = 1;
     
         for (j=0;j<N;j++)
            tmp += MULT16_16(EXTRACT16(SHR32(W[ i*N+j],18)), EXTRACT16(SHR32(W[i*N+j],18)));
#ifdef FIXED_POINT
      /* Just a security in case an overflow were to occur */
      tmp = MIN32(ABS32(tmp), 536870912);
#endif
      prop[i] = spx_sqrt(tmp);
      if (prop[i] > max_sum)
         max_sum = prop[i];
   }
   for (i=0;i<M;i++)
   {
      prop[i] += MULT16_16_Q15(QCONST16(.1f,15),max_sum);
      prop_sum += EXTEND32(prop[i]);
   }
   for (i=0;i<M;i++)
   {
      prop[i] = DIV32(MULT16_16(QCONST16(.99f,15), prop[i]),prop_sum);    
   }
   
}


 _ATTR_AECALG_TEXT_
 SpeexEchoState *speex_echo_state_init_mc(int frame_size, int filter_length ,void *fft_table)
{
	
 
	int i,N,M; 
   //static SpeexEchoState  EchoSt;
   SpeexEchoState *st = &EchoSt;
   
   
   st->frame_size = frame_size;
   st->window_size = 2*frame_size;
   N = st->window_size;
   M = st->M = (filter_length+st->frame_size-1)/frame_size;
   st->cancel_count=0;
   st->sum_adapt = 0;
   st->saturated = 0;
   
   st->spec_average = DIV32_16(SHL32(EXTEND32(st->frame_size), 15), sampling_rate);

   st->beta0 = DIV32_16(SHL32(EXTEND32(st->frame_size), 16), sampling_rate);
   st->beta_max = DIV32_16(SHL32(EXTEND32(st->frame_size), 14), sampling_rate);

   st->leak_estimate = 0;

   st->fft_table = fft_table;
 

   for (i=0;i<N>>1;i++)
   {
      st->window[i] = (16383-SHL16(spx_cos(DIV32_16(MULT16_16(25736,i<<1),N)),1));
      st->window[N-i-1] = st->window[i];
   }

   for (i=0;i<=st->frame_size;i++)
      st->power_1[i] = FLOAT_ONE;
   for (i=0;i<N*M;i++)
      st->W[i] = 0;
   {
      spx_word32_t sum = 0;
      /* Ratio of ~10 between adaptation rate of first and last block */
      spx_word16_t decay = SHR32(spx_exp(NEG16(DIV32_16(QCONST16(2.4,11),M))),1);
      st->prop[0] = QCONST16(.7, 15);
      sum = EXTEND32(st->prop[0]);
      for (i=1;i<M;i++)
      {
         st->prop[i] = MULT16_16_Q15(st->prop[i-1], decay);
         sum = ADD32(sum, EXTEND32(st->prop[i]));
      }
      for (i=M-1;i>=0;i--)
      {
         st->prop[i] = DIV32(MULT16_16(QCONST16(.8f,15), st->prop[i]),sum);
      }
   }

   st->preemph = QCONST16(.9,15);
 
   st->notch_radius = QCONST16(.9, 15);
   

   st->adapted = 0;
   st->Pey = st->Pyy = FLOAT_ONE;
   
#ifdef TWO_PATH
   st->Davg1 = st->Davg2 = 0;
   st->Dvar1 = st->Dvar2 = FLOAT_ZERO;
#endif
   
   return st;
}




_ATTR_AECALG_TEXT_
/** Performs echo cancellation on a frame */
 void speex_echo_cancellation(SpeexEchoState *st, const short  int *in, const short  int *far_end, short  int *out)
{
   int i,j;
    int N,M;
   spx_word32_t Syy,See,Sxx,Sdd, Sff;
#ifdef TWO_PATH
   spx_word32_t Dbf;
   int update_foreground;
#endif
   spx_word32_t Sey;
   spx_word16_t ss, ss_1;
   spx_float_t Pey = FLOAT_ONE, Pyy=FLOAT_ONE;
   spx_float_t alpha, alpha_1;
   spx_word16_t RER;
   spx_word32_t tmp32;
   spx_word16_t input[NN];  /* near-end buffer (N) */ 
   spx_word16_t e[N1];      /* scratch */  
   spx_word16_t y[N1];      /* scratch */
   spx_word16_t Y[N1];      /* scratch */  
#ifdef TWO_PATH
    spx_word16_t wtmp[N1];   /* scratch */
#endif   
   spx_word16_t wtmp2[N1];	/* scratch */
   spx_word32_t PHI[N1];	/* scratch */
   spx_word32_t Rf[NN+1];     /* scratch */
   spx_word32_t Yf[NN+1];     /* scratch */
   spx_word32_t Xf[NN+1];     /* scratch */
   spx_word32_t Eh[NN+1];
   spx_word32_t Yh[NN+1];
   
   N = st->window_size;  
   M = st->M;   
 
   st->cancel_count++;

   ss=DIV32_16(11469,M);
   ss_1 = SUB16(32767,ss);

      /* Apply a notch filter to make sure DC doesn't end up causing problems */
      filter_dc_notch16(in, st->notch_radius, input, st->frame_size, st->notch_mem);
      /* Copy input data to buffer and apply pre-emphasis */
      /* Copy input data to buffer */
      for (i=0;i<st->frame_size;i++)  
      {
         spx_word32_t tmp32;
         /* FIXME: This core has changed a bit, need to merge properly */
         tmp32 = SUB32(EXTEND32(input[i]), EXTEND32(MULT16_16_P15(st->preemph, st->memD[0])));		 

         if (tmp32 > 32767)
         {
            tmp32 = 32767;
            if (st->saturated == 0)
               st->saturated = 1;
         }      
         if (tmp32 < -32767)
         {
            tmp32 = -32767;
            if (st->saturated == 0)
               st->saturated = 1;
         }

         st->memD[0] = input[i];
         input[i] = EXTRACT16(tmp32);
      }         

 
      for (i=0;i<st->frame_size;i++)
      {
         spx_word32_t tmp32;
         st->x[i] = st->x[i+st->frame_size];
         tmp32 = SUB32(EXTEND32(far_end[i]), EXTEND32(MULT16_16_P15(st->preemph, st->memX[0])));

         /*FIXME: If saturation occurs here, we need to freeze adaptation for M frames (not just one) */
         if (tmp32 > 32767)
         {
            tmp32 = 32767;
            st->saturated = M+1;
         }      
         if (tmp32 < -32767)
         {
            tmp32 = -32767;
            st->saturated = M+1;
         }      

         st->x[i+st->frame_size] = EXTRACT16(tmp32);
         st->memX[0] = far_end[i];
      }  
   

      /* Shift memory: this could be optimized eventually*/
      for (j=M-1;j>=0;j--)
      { 
         for (i=0;i<N;i++)  
            st->X[(j+1)*N+i] = st->X[j*N+i];
      }
      /* Convert x (echo input) to frequency domain */
       spx_fft(st->fft_table, st->x, &st->X[0]);

   
   Sxx = 0;     

 
      Sxx += mdf_inner_prod(st->x+st->frame_size, st->x+st->frame_size, st->frame_size);
      power_spectrum_accum(st->X, Xf, N);
 
   Sff = 0;  

#ifdef TWO_PATH
      /* Compute foreground filter */
      spectral_mul_accum16(st->X, st->foreground, Y, N, M);   
      spx_ifft(st->fft_table, Y,e); 
	  for (i=0;i<st->frame_size;i++)
          e[i] = SUB16(input[i], e[i+st->frame_size]);
	   
      Sff += mdf_inner_prod(e,e, st->frame_size);
#endif
 
   
   /* Adjust proportional adaption rate */
   /* FIXME: Adjust that for C, K*/
   if (st->adapted)
      mdf_adjust_prop (st->W, N, M, st->prop);
   /* Compute weight gradient */
   if (st->saturated == 0)
   {      
            for (j=M-1;j>=0;j--)
            {
               weighted_spectral_mul_conj(st->power_1, FLOAT_SHL(PSEUDOFLOAT(st->prop[j]),-15), &st->X[(j+1)*N], st->E, PHI, N);
               for (i=0;i<N;i++)
                  st->W[ j*N+ i] += PHI[i];
            }
			//power_1 = U/参考回声的平均功率谱   
			//w = w'+u*E[k]*X[k-n]/参考回声的平均功率谱 体现了归一化的最抖下降法NLMS
     
   } else {
      st->saturated--;
   }
   
   /* FIXME: MC conversion required */ 
   /* Update weight to prevent circular convolution (MDF / AUMDF) 将滤波器系数转到频域更新邋—MDF算法*/
  
         for (j=0;j<M;j++)
         {
            /* This is a variant of the Alternatively Updated MDF (AUMDF) */
            /* Remove the "if" to make this an MDF filter */
            if (j==0 || st->cancel_count%(M-1) == j-1)
            {

               for (i=0;i<N;i++)
                  wtmp2[i] = EXTRACT16(PSHR32(st->W[ j*N + i],NORMALIZE_SCALEDOWN+16));
               spx_ifft(st->fft_table, wtmp2, wtmp);
               for (i=0;i<st->frame_size;i++)
               {
                  wtmp[i]=0;  //MDF算法，只取后N点为有效的滤波过程中线性卷积的结果
               }
               for (i=st->frame_size;i<N;i++)
               {
                  wtmp[i]=SHL16(wtmp[i],NORMALIZE_SCALEUP);
               }
               spx_fft(st->fft_table, wtmp, wtmp2);
               /* The "-1" in the shift is a sort of kludge that trades less efficient update speed for decrease noise */
               for (i=0;i<N;i++)
                  st->W[ j*N + i] -= SHL32(EXTEND32(wtmp2[i]),16+NORMALIZE_SCALEDOWN-NORMALIZE_SCALEUP-1);

            }
         }
   
   
   /* So we can use power_spectrum_accum */ 
   for (i=0;i<=st->frame_size;i++)
      Rf[i] = Yf[i] =Xf[i] = 0;
      
    Dbf = 0;
    See = 0;    
#ifdef TWO_PATH
    
   /* Difference in response, this is used to estimate the variance of our residual power estimate  */

      spectral_mul_accum(st->X, st->W, Y, N, M);//对回声进行估计
      spx_ifft(st->fft_table, Y, y);
      for (i=0;i<st->frame_size;i++)
         e[i] = SUB16(e[i+st->frame_size], y[i+st->frame_size]);
      Dbf += 10+mdf_inner_prod(e,e, st->frame_size);
      for (i=0;i<st->frame_size;i++)
         e[i] = SUB16(input[i], y[i+st->frame_size]);
      See += mdf_inner_prod(e,e, st->frame_size);

#endif



#ifdef TWO_PATH
   /* Logic for updating the foreground filter */
   
   /* For two time windows, compute the mean of the energy difference, as well as the variance */
   st->Davg1 = ADD32(MULT16_32_Q15(QCONST16(.6f,15),st->Davg1), MULT16_32_Q15(QCONST16(.4f,15),SUB32(Sff,See)));
   st->Davg2 = ADD32(MULT16_32_Q15(QCONST16(.85f,15),st->Davg2), MULT16_32_Q15(QCONST16(.15f,15),SUB32(Sff,See)));
   st->Dvar1 = FLOAT_ADD(FLOAT_MULT(VAR1_SMOOTH, st->Dvar1), FLOAT_MUL32U(MULT16_32_Q15(QCONST16(.4f,15),Sff), MULT16_32_Q15(QCONST16(.4f,15),Dbf)));
   st->Dvar2 = FLOAT_ADD(FLOAT_MULT(VAR2_SMOOTH, st->Dvar2), FLOAT_MUL32U(MULT16_32_Q15(QCONST16(.15f,15),Sff), MULT16_32_Q15(QCONST16(.15f,15),Dbf)));
   
   /* Equivalent float code:
   st->Davg1 = .6*st->Davg1 + .4*(Sff-See);
   st->Davg2 = .85*st->Davg2 + .15*(Sff-See);
   st->Dvar1 = .36*st->Dvar1 + .16*Sff*Dbf;
   st->Dvar2 = .7225*st->Dvar2 + .0225*Sff*Dbf;
   */
   
   update_foreground = 0;
   /* Check if we have a statistically significant reduction in the residual echo */
   /* Note that this is *not* Gaussian, so we need to be careful about the longer tail */
   if (FLOAT_GT(FLOAT_MUL32U(SUB32(Sff,See),ABS32(SUB32(Sff,See))), FLOAT_MUL32U(Sff,Dbf)))
      update_foreground = 1;
   else if (FLOAT_GT(FLOAT_MUL32U(st->Davg1, ABS32(st->Davg1)), FLOAT_MULT(VAR1_UPDATE,(st->Dvar1))))
      update_foreground = 1;
   else if (FLOAT_GT(FLOAT_MUL32U(st->Davg2, ABS32(st->Davg2)), FLOAT_MULT(VAR2_UPDATE,(st->Dvar2))))
      update_foreground = 1;
   
   /* Do we update? */
   if (update_foreground)
   {
	   

      st->Davg1 = st->Davg2 = 0;
      st->Dvar1 = st->Dvar2 = FLOAT_ZERO;
      /* Copy background filter to foreground filter */
      for (i=0;i<N*M;i++)
         st->foreground[i] = EXTRACT16(PSHR32(st->W[i],16));
      /* Apply a smooth transition so as to not introduce blocking artifacts */
     
         for (i=0;i<st->frame_size;i++)
            e[i+st->frame_size] = MULT16_16_Q15(st->window[i+st->frame_size],e[i+st->frame_size]) + MULT16_16_Q15(st->window[i],y[i+st->frame_size]);
   } else {
      int reset_background=0;
	  
      /* Otherwise, check if the background filter is significantly worse */
      if (FLOAT_GT(FLOAT_MUL32U(NEG32(SUB32(Sff,See)),ABS32(SUB32(Sff,See))), FLOAT_MULT(VAR_BACKTRACK,FLOAT_MUL32U(Sff,Dbf))))
         reset_background = 1;
      if (FLOAT_GT(FLOAT_MUL32U(NEG32(st->Davg1), ABS32(st->Davg1)), FLOAT_MULT(VAR_BACKTRACK,st->Dvar1)))
         reset_background = 1;
      if (FLOAT_GT(FLOAT_MUL32U(NEG32(st->Davg2), ABS32(st->Davg2)), FLOAT_MULT(VAR_BACKTRACK,st->Dvar2)))
         reset_background = 1;
      if (reset_background)
      {
		 
         /* Copy foreground filter to background filter */
         for (i=0;i<N*M;i++)
            st->W[i] = SHL32(EXTEND32(st->foreground[i]),16);
         /* We also need to copy the output so as to get correct adaptation */
        
            for (i=0;i<st->frame_size;i++)
               y[i+st->frame_size] = e[i+st->frame_size];
            for (i=0;i<st->frame_size;i++)
               e[i] = SUB16(input[i], y[i+st->frame_size]);
               
         See = Sff;
         st->Davg1 = st->Davg2 = 0;
         st->Dvar1 = st->Dvar2 = FLOAT_ZERO;
      }
   }
#endif

   Sey = Syy = Sdd = 0;  

      /* Compute error signal (for the output with de-emphasis) */ 
      for (i=0;i<st->frame_size;i++)
      {
         spx_word32_t tmp_out;
#ifdef TWO_PATH
         tmp_out = SUB32(EXTEND32(input[i]), EXTEND32(e[i+st->frame_size]));
#else
         tmp_out = SUB32(EXTEND32(input[i]), EXTEND32(y[i+st->frame_size]));
#endif
         tmp_out = ADD32(tmp_out, EXTEND32(MULT16_16_P15(st->preemph, st->memE[0])));
      /* This is an arbitrary test for saturation in the microphone signal */
         if (in[i] <= -32000 || in[i] >= 32000)
         {
         if (st->saturated == 0)
            st->saturated = 1;
         }
         out[i] = WORD2INT(tmp_out);
         st->memE[0] = tmp_out;
      }

#ifdef DUMP_ECHO_CANCEL_DATA
      dump_audio(in, far_end, out, st->frame_size);
#endif
   
      /* Compute error signal (filter update version) */ 
      for (i=0;i<st->frame_size;i++)
      {
         e[i+st->frame_size] = e[i];
         e[i] = 0;
      }
      
      /* Compute a bunch of correlations */ 
      /* FIXME: bad merge */
      Sey += mdf_inner_prod(e+st->frame_size, y+st->frame_size, st->frame_size);
      Syy += mdf_inner_prod(y+st->frame_size, y+st->frame_size, st->frame_size);
      Sdd += mdf_inner_prod(input, input, st->frame_size);
      
      /* Convert error to frequency domain */
      spx_fft(st->fft_table, e,st->E);
      for (i=0;i<st->frame_size;i++)
         y[i] = 0;
      spx_fft(st->fft_table, y, Y);
   
      /* Compute power spectrum of echo (X), error (E) and filter response (Y) */
      power_spectrum_accum(st->E, Rf, N);
      power_spectrum_accum(Y, Yf, N);

   /* Add a small noise floor to make sure not to have problems when dividing */
   See = MAX32(See, SHR32(MULT16_16(N, 100),6));
     
  
      Sxx += mdf_inner_prod(st->x+st->frame_size, st->x+st->frame_size, st->frame_size);
      power_spectrum_accum(st->X, Xf, N);


   
   /* Smooth far end energy estimate over time */
   for (j=0;j<=st->frame_size;j++)
      st->power[j] = MULT16_32_Q15(ss_1,st->power[j]) + 1 + MULT16_32_Q15(ss,Xf[j]);//对功率谱求平均

#if 0
   for (j=st->frame_size;j>=0;j--)
  {
		  spx_float_t Et, Yt;
		  Et = PSEUDOFLOAT(Rf[j] - Eh[j]);
		  Yt = PSEUDOFLOAT(Yf[j] - Yh[j]);
		  Pey = FLOAT_ADD(Pey,FLOAT_MULT(Et,Yt));
		  Pyy = FLOAT_ADD(Pyy,FLOAT_MULT(Yt,Yt));

		  Eh[j] = MAC16_32_Q15(MULT16_32_Q15(SUB16(32767,st->spec_average),Eh[j]), st->spec_average, Rf[j]);
		  Yh[j] = MAC16_32_Q15(MULT16_32_Q15(SUB16(32767,st->spec_average),Yh[j]), st->spec_average, Yf[j]);

   }
   
   Pyy = FLOAT_SQRT(Pyy);
   Pey = FLOAT_DIVU(Pey,Pyy);

   /* Compute correlation update rate */
   tmp32 = MULT16_32_Q15(st->beta0,Syy);
   if (tmp32 > MULT16_32_Q15(st->beta_max,See))
      tmp32 = MULT16_32_Q15(st->beta_max,See);
   alpha = FLOAT_DIV32(tmp32, See);
   alpha_1 = FLOAT_SUB(FLOAT_ONE, alpha);
   /* Update correlations (recursive average) */
   st->Pey = FLOAT_ADD(FLOAT_MULT(alpha_1,st->Pey) , FLOAT_MULT(alpha,Pey));
   st->Pyy = FLOAT_ADD(FLOAT_MULT(alpha_1,st->Pyy) , FLOAT_MULT(alpha,Pyy));
   if (FLOAT_LT(st->Pyy, FLOAT_ONE))
      st->Pyy = FLOAT_ONE;
   /* We don't really hope to get better than 33 dB (MIN_LEAK-3dB) attenuation anyway */
   if (FLOAT_LT(st->Pey, FLOAT_MULT(MIN_LEAK,st->Pyy)))
      st->Pey = FLOAT_MULT(MIN_LEAK,st->Pyy);
   if (FLOAT_GT(st->Pey, st->Pyy))
      st->Pey = st->Pyy;
   /* leak_estimate is the linear regression result */
   st->leak_estimate = FLOAT_EXTRACT16(FLOAT_SHL(FLOAT_DIVU(st->Pey, st->Pyy),14));//回声=leak*回声估计
   /* This looks like a stupid bug, but it's right (because we convert from Q14 to Q15) */
   if (st->leak_estimate > 16383)
      st->leak_estimate = 32767;
   else
      st->leak_estimate = SHL16(st->leak_estimate,1);//频谱上的泄漏因子
#else
   st->leak_estimate  =  32767;//to adapt to big volume toto get better than 33 dB (MIN_LEAK-3dB) attenuation
   
#endif 
   /* Compute Residual to Error Ratio */

   tmp32 = MULT16_32_Q15(st->leak_estimate,Syy);//残留回声的估计值
   tmp32 = ADD32(SHR32(Sxx,13), ADD32(tmp32, SHL32(tmp32,1)));
   /* Check for y in e (lower bound on RER) */
   {
      spx_float_t bound = PSEUDOFLOAT(Sey);
      bound = FLOAT_DIVU(FLOAT_MULT(bound, bound), PSEUDOFLOAT(ADD32(1,Syy)));
      if (FLOAT_GT(bound, PSEUDOFLOAT(See)))
         tmp32 = See;
      else if (tmp32 < FLOAT_EXTRACT32(bound))
         tmp32 = FLOAT_EXTRACT32(bound);
   }
   if (tmp32 > SHR32(See,1))
      tmp32 = SHR32(See,1);
   RER = FLOAT_EXTRACT16(FLOAT_SHL(FLOAT_DIV32(tmp32,See),15));//能量上的泄露因子


   /* We consider that the filter has had minimal adaptation if the following is true*/
   if (!st->adapted && st->sum_adapt > SHL32(EXTEND32(M),15) && MULT16_32_Q15(st->leak_estimate,Syy) > MULT16_32_Q15(QCONST16(.03f,15),Syy))
   {
      st->adapted = 1;
		   	 
   }

   if (st->adapted)
   {
	   
      /* Normal learning rate calculation once we're past the minimal adaptation phase */
      for (i=0;i<=st->frame_size;i++)
      {
         spx_word32_t r, e;
         /* Compute frequency-domain adaptation mask 计算频域最优步长U=LEAK*Y^2/E^2*/
         r = MULT16_32_Q15(st->leak_estimate,SHL32(Yf[i],3));
         e = SHL32(Rf[i],3)+1;

         if (r>SHR32(e,1))
            r = SHR32(e,1);

         r = MULT16_32_Q15(QCONST16(.7,15),r) + MULT16_32_Q15(QCONST16(.3,15),(spx_word32_t)(MULT16_32_Q15(RER,e)));
         /*st->power_1[i] = adapt_rate*r/(e*(1+st->power[i]));*/
         st->power_1[i] = FLOAT_SHL(FLOAT_DIV32_FLOAT(r,FLOAT_MUL32U(e,st->power[i]+10)),WEIGHT_SHIFT+16);//r/e即为步长因子
      }
   } else {
      /* Temporary adaption rate if filter is not yet adapted enough */
      spx_word16_t adapt_rate=0;

      if (Sxx > SHR32(MULT16_16(N, 1000),6)) 
      {
         tmp32 = MULT16_32_Q15(QCONST16(.25f, 15), Sxx);

         if (tmp32 > SHR32(See,2))
            tmp32 = SHR32(See,2);

         adapt_rate = FLOAT_EXTRACT16(FLOAT_SHL(FLOAT_DIV32(tmp32, See),15));
      }
      for (i=0;i<=st->frame_size;i++)
         st->power_1[i] = FLOAT_SHL(FLOAT_DIV32(EXTEND32(adapt_rate),ADD32(st->power[i],10)),WEIGHT_SHIFT+1);


      /* How much have we adapted so far? */
      st->sum_adapt = ADD32(st->sum_adapt,adapt_rate);
 }
	 
/* FIXME: MC conversion required */ 
      for (i=0;i<st->frame_size;i++)
         st->last_y[i] = st->last_y[st->frame_size+i];
   if (st->adapted)
   {
      /* If the filter is adapted, take the filtered echo */
      for (i=0;i<st->frame_size;i++)
         st->last_y[st->frame_size+i] = in[i]-out[i];
   } else {
      /* If filter isn't adapted yet, all we can do is take the far end signal directly */
      /* moved earlier: */
	   for (i=0;i<N;i++)
          st->last_y[i] = st->x[i];
   }
 }

/* Compute spectrum of estimated echo for use in an echo post-filter */
_ATTR_AECALG_TEXT_
void speex_echo_get_residual(SpeexEchoState *st, spx_word32_t *residual_echo, int len)

{  
   spx_word16_t y[N1];      /* scratch */
   spx_word16_t Y[N1];      /* scratch */
   int i;
   short  leak2;
   int N;
   
   N = st->window_size;

   /* Apply hanning window (should pre-compute it)*/
   for (i=0;i<N;i++)
      y[i] = MULT16_16_Q15(st->window[i],st->last_y[i]);
      
   /* Compute power spectrum of the echo */
   spx_fft(st->fft_table, y, Y);
   power_spectrum(Y, residual_echo, N);	    

   if (st->leak_estimate > 16383)
      leak2 = 32767;
   else
      leak2 = SHL16(st->leak_estimate, 1);

   /* Estimate residual echo */
   for (i=0;i<=st->frame_size;i++)
      residual_echo[i] = (int)MULT16_32_Q15(leak2,residual_echo[i]);
   
}


#endif
