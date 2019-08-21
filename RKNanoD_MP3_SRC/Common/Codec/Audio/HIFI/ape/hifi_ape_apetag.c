
/*
 * APE tag handling
 * Copyright (c) 2007 Benjamin Zores <ben@geexbox.org>
 *  based upon libdemac from Dave Chapman.
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
#include "ape.h"
#include "audio_file_access.h"

#ifdef A_CORE_DECODE
#ifdef HIFI_APE_DECODE
#pragma arm section code = "ApeHDecCode", rodata = "ApeHDecCode", rwdata = "ApeHDecData", zidata = "ApeHDecBss"

#define ENABLE_DEBUG 0

#define APE_TAG_VERSION               2000
#define APE_TAG_FOOTER_BYTES          32
#define APE_TAG_FLAG_CONTAINS_HEADER  (1 << 31)
#define APE_TAG_FLAG_IS_HEADER        (1 << 29)

extern FILE *ape_file_handle;;
extern int ID3_len;

int  ff_ape_parse_tag()
{

    uint32_t val, fields, tag_bytes;
    uint8_t ApeData[32];
    int i;
    unsigned char *pucBuffer = ApeData;
    RKFIO_FSeek(RKFIO_FLength(ape_file_handle)-32,0 , ape_file_handle);
    RKFIO_FRead(ApeData , 32 , ape_file_handle);
    RKFIO_FSeek( ID3_len,0 , pRawFileCache);
    if (strncmp(pucBuffer, "APETAGEX", 8))
    {

        return 0;
    }

    val =((pucBuffer[11] <<	24)	| (pucBuffer[10]<< 16) |
          (pucBuffer[9] <<	 8)	|  pucBuffer[8]);        // APE tag version
    if (val > 2000)
    {

        return 0;
    }

    tag_bytes =((pucBuffer[15] <<24)	| (pucBuffer[14]<< 16) |
                (pucBuffer[13] <<8)	|  pucBuffer[12]);  // tag size
    if (tag_bytes - 32> (1024 * 1024 * 16))
    {
        return 0;
    }

    // Return the length of	the	APE tag.
    return  tag_bytes;
}


#pragma arm section code
#endif
#endif
