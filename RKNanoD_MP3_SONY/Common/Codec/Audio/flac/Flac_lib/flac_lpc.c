/* libFLAC - Free Lossless Audio Codec library
 * Copyright (C) 2000,2001,2002,2003,2004,2005  Josh Coalson
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * - Neither the name of the Xiph.org Foundation nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "../../include/audio_main.h"

#ifdef A_CORE_DECODE
#ifdef FLAC_DEC_INCLUDE

#include <math.h>
#include "assert.h"
#include "format_i.h"
#include "bitmath.h"
#include "lpc.h"
#if defined DEBUG || defined FLAC__OVERFLOW_DETECT || defined FLAC__OVERFLOW_DETECT_VERBOSE
#include <stdio.h>
#endif

_ATTR_FLACDEC_TEXT_
void FLAC__lpc_restore_signal(const FLAC__int32 residual[], unsigned data_len, const FLAC__int32 qlp_coeff[], unsigned order, int lp_quantization, FLAC__int32 data[])
{
    unsigned i, j;
    FLAC__int32 sum;
    const FLAC__int32 *history;

    FLAC__ASSERT(order > 0);
#ifdef FLAC_MINUS_ZI_DATA
    if (0 == gFLACchannel)
    {
        FLAC__int16* pdata = (FLAC__int16*)(&data[0]);
        FLAC__int16* presidual = (FLAC__int16*)(&residual[0]);
        FLAC__int16* history;
        for (i = 0; i < data_len; i++)
        {
            sum = 0;
            history = pdata;

            for (j = 0; j < order; j++)
            {
                sum += qlp_coeff[j] * (*(--history));

            }
            *(pdata++) = *(presidual++) + (sum >> lp_quantization);
        }
    }
    else if (1 == gFLACchannel)
    {
        for (i = 0; i < data_len; i++)
        {

            sum = 0;
            history = data;

            for (j = 0; j < order; j++)
            {
                sum += qlp_coeff[j] * (*(--history));

            }
            *(data++) = *(residual++) + (sum >> lp_quantization);
        }
    }
    else
    {
        while (1);//for debug
    }
#else
    for (i = 0; i < data_len; i++)
    {
        sum = 0;
        history = data;

        for (j = 0; j < order; j++)
        {
            sum += qlp_coeff[j] * (*(--history));
        }
        *(data++) = *(residual++) + (sum >> lp_quantization);
    }
#endif
}

_ATTR_FLACDEC_TEXT_
void FLAC__lpc_restore_signal_wide(const FLAC__int32 residual[], unsigned data_len, const FLAC__int32 qlp_coeff[], unsigned order, int lp_quantization, FLAC__int32 data[])
{
    unsigned i, j;
    FLAC__int64 sum;
    const FLAC__int32 *history;


    FLAC__ASSERT(order > 0);
#ifdef FLAC_MINUS_ZI_DATA
    if (0 == gFLACchannel)
    {
        FLAC__int16* pdata = (FLAC__int16*)(&data[0]);
        FLAC__int16* presidual = (FLAC__int16*)(&residual[0]);
        const FLAC__int16* history;

        for (i = 0; i < data_len; i++)
        {
            sum = 0;
            history = pdata;
            for (j = 0; j < order; j++)
                sum += (FLAC__int64)qlp_coeff[j] * (FLAC__int64)(*(--history));
            FLAC__ASSERT(sum < 0x7fffffff);
            FLAC__ASSERT(sum > 0x80000000);

            *(pdata++) = *(presidual++) + (FLAC__int32)(sum >> lp_quantization);
        }
    }
    else if (1 == gFLACchannel)
    {
        for (i = 0; i < data_len; i++)
        {
            sum = 0;
            history = data;
            for (j = 0; j < order; j++)
                sum += (FLAC__int64)qlp_coeff[j] * (FLAC__int64)(*(--history));
            FLAC__ASSERT(sum < 0x7fffffff);
            FLAC__ASSERT(sum > 0x80000000);

            *(data++) = *(residual++) + (FLAC__int32)(sum >> lp_quantization);
        }
    }
    else
    {
        while (1);
    }
#else
    for (i = 0; i < data_len; i++)
    {
        sum = 0;
        history = data;
        for (j = 0; j < order; j++)
            sum += (FLAC__int64)qlp_coeff[j] * (FLAC__int64)(*(--history));
        FLAC__ASSERT(sum < 0x7fffffff);
        FLAC__ASSERT(sum > 0x80000000);

        *(data++) = *(residual++) + (FLAC__int32)(sum >> lp_quantization);
    }
#endif
}

#endif
#endif
