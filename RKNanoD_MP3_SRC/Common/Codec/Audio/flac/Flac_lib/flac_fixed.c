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
#include "bitmath.h"
#include "fixed.h"
#include "assert.h"

#ifndef M_LN2
/* math.h in VC++ doesn't seem to have this (how Microsoft is that?) */
#define M_LN2 0.69314718055994530942
#endif

#ifdef min
#undef min
#endif
#define min(x,y) ((x) < (y)? (x) : (y))

#ifdef local_abs
#undef local_abs
#endif
#define local_abs(x) ((unsigned)((x)<0? -(x) : (x)))
    
_ATTR_FLACDEC_TEXT_
void FLAC__fixed_restore_signal(const FLAC__int32 residual[], unsigned data_len, unsigned order, FLAC__int32 data[])
{
    int i, idata_len = (int)data_len;
#ifdef FLAC_MINUS_ZI_DATA
    if (0 == gFLACchannel)
    {
        FLAC__int16* pDataTmp = (FLAC__int16*)(&data[0]);
        FLAC__int16* pResidual = (FLAC__int16*)(&residual[0]);
        switch (order)
        {
            case 0:
                for (i = 0; i < idata_len; i++)
                {
                    pDataTmp[i] = pResidual[i];
                }
                break;
            case 1:
                for (i = 0; i < idata_len; i++)
                {
                    pDataTmp[i] = pResidual[i] + pDataTmp[i-1];
                }
                break;
            case 2:
                for (i = 0; i < idata_len; i++)
                {
                    /* == residual[i] + 2*data[i-1] - data[i-2] */
                    pDataTmp[i] = pResidual[i] + (pDataTmp[i-1] << 1) - pDataTmp[i-2];
                }
                break;
            case 3:
                for (i = 0; i < idata_len; i++)
                {
                    /* residual[i] + 3*data[i-1] - 3*data[i-2]) + data[i-3] */
                    pDataTmp[i] = pResidual[i] + (((pDataTmp[i-1] - pDataTmp[i-2]) << 1) + (pDataTmp[i-1] - pDataTmp[i-2])) + pDataTmp[i-3];
                }
                break;
            case 4:
                for (i = 0; i < idata_len; i++)
                {
                    /* == residual[i] + 4*data[i-1] - 6*data[i-2] + 4*data[i-3] - data[i-4] */
                    pDataTmp[i] = pResidual[i] + ((pDataTmp[i-1] + pDataTmp[i-3]) << 2) - ((pDataTmp[i-2] << 2) + (pDataTmp[i-2] << 1)) - pDataTmp[i-4];
                }
                break;
            default:
                FLAC__ASSERT(0);
        }
    }
    else if (1 == gFLACchannel)
    {
        switch (order)
        {
            case 0:
                for (i = 0; i < idata_len; i++)
                {
                    data[i] = residual[i];
                }
                break;
            case 1:
                for (i = 0; i < idata_len; i++)
                {
                    data[i] = residual[i] + data[i-1];
                }
                break;
            case 2:
                for (i = 0; i < idata_len; i++)
                {
                    /* == residual[i] + 2*data[i-1] - data[i-2] */
                    data[i] = residual[i] + (data[i-1] << 1) - data[i-2];
                }
                break;
            case 3:
                for (i = 0; i < idata_len; i++)
                {
                    /* residual[i] + 3*data[i-1] - 3*data[i-2]) + data[i-3] */
                    data[i] = residual[i] + (((data[i-1] - data[i-2]) << 1) + (data[i-1] - data[i-2])) + data[i-3];
                }
                break;
            case 4:
                for (i = 0; i < idata_len; i++)
                {
                    /* == residual[i] + 4*data[i-1] - 6*data[i-2] + 4*data[i-3] - data[i-4] */
                    data[i] = residual[i] + ((data[i-1] + data[i-3]) << 2) - ((data[i-2] << 2) + (data[i-2] << 1)) - data[i-4];
                }
                break;
            default:
                FLAC__ASSERT(0);
        }
    }
#else
    switch (order)
    {
        case 0:
            for (i = 0; i < idata_len; i++)
            {
                data[i] = residual[i];
            }
            break;
        case 1:
            for (i = 0; i < idata_len; i++)
            {
                data[i] = residual[i] + data[i-1];
            }
            break;
        case 2:
            for (i = 0; i < idata_len; i++)
            {
                /* == residual[i] + 2*data[i-1] - data[i-2] */
                data[i] = residual[i] + (data[i-1] << 1) - data[i-2];
            }
            break;
        case 3:
            for (i = 0; i < idata_len; i++)
            {
                /* residual[i] + 3*data[i-1] - 3*data[i-2]) + data[i-3] */
                data[i] = residual[i] + (((data[i-1] - data[i-2]) << 1) + (data[i-1] - data[i-2])) + data[i-3];
            }
            break;
        case 4:
            for (i = 0; i < idata_len; i++)
            {
                /* == residual[i] + 4*data[i-1] - 6*data[i-2] + 4*data[i-3] - data[i-4] */
                data[i] = residual[i] + ((data[i-1] + data[i-3]) << 2) - ((data[i-2] << 2) + (data[i-2] << 1)) - data[i-4];
            }
            break;
        default:
            FLAC__ASSERT(0);
    }
#endif
}

#endif
#endif
