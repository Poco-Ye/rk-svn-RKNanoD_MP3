/* Copyright (C) 2003 Epic Games
   Written by Jean-Marc Valin */
/**
 *  @file speex_preprocess.h
 *  @brief Speex preprocessor. The preprocess can do noise suppression, 
 * residual echo suppression (after using the echo canceller), automatic
 * gain control (AGC) and voice activity detection (VAD).
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

#ifndef SPEEX_PREPROCESS_H
#define SPEEX_PREPROCESS_H
/** @defgroup SpeexPreprocessState SpeexPreprocessState: The Speex preprocessor
 *  This is the Speex preprocessor. The preprocess can do noise suppression, 
 * residual echo suppression (after using the echo canceller), automatic
 * gain control (AGC) and voice activity detection (VAD).
 *  @{
 */
#include "speex_echo.h"
#include "arch.h"


#ifdef __cplusplus
extern "C" {
#endif
   
/** State of the preprocessor (one per channel). Should never be accessed directly. */
struct SpeexPreprocessState_;

/** State of the preprocessor (one per channel). Should never be accessed directly. */
typedef struct SpeexPreprocessState_ SpeexPreprocessState;




/** Creates a new preprocessing state. You MUST create one state per channel processed.
 * @param frame_size Number of samples to process at one time (should correspond to 10-20 ms). Must be
 * the same value as that used for the echo canceller for residual echo cancellation to work.
 * @param sampling_rate Sampling rate used for the input.
 * @return Newly created preprocessor state
*/
SpeexPreprocessState *speex_preprocess_state_init(int frame_size, int sample_rate,void *fft_table);


/** Preprocess a frame 
 * @param st Preprocessor state
 * @param x Audio sample vector (in and out). Must be same size as specified in speex_preprocess_state_init().
 * @return Bool value for voice activity (1 for speech, 0 for noise/silence), ONLY if VAD turned on.
*/

int speex_preprocess_run(SpeexPreprocessState *st, short  int *x,SpeexEchoState *echo_st);


/** Update preprocessor state, but do not compute the output
 * @param st Preprocessor state
 * @param x Audio sample vector (in only). Must be same size as specified in speex_preprocess_state_init().
*/



#ifdef __cplusplus
}
#endif

/** @}*/
#endif
