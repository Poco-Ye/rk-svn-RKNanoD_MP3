/*
 * ALAC encoder and decoder common data
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#include "SysInclude.h"
#include "audio_main.h"
#ifdef A_CORE_DECODE
#ifdef HIFI_AlAC_DECODE
#pragma arm section code = "AlacHDecCode", rodata = "AlacHDecCode", rwdata = "AlacHDecData", zidata = "AlacHDecBss"

#include "alac.h"

 uint8_t ff_alac_channel_layout_offsets[ALAC_MAX_CHANNELS][ALAC_MAX_CHANNELS] = {
    { 0 },
    { 0, 1 },
    { 2, 0, 1 },
    { 2, 0, 1, 3 },
    { 2, 0, 1, 3, 4 },
    { 2, 0, 1, 4, 5, 3 },
    { 2, 0, 1, 4, 5, 6, 3 },
    { 2, 6, 7, 0, 1, 4, 5, 3 }
};
#pragma arm section code
#endif
#endif
