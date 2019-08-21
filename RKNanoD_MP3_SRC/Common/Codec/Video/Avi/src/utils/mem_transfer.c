/*****************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  - 8bit<->16bit transfer  -
 *
 *  Copyright(C) 2001-2003 Peter Ross <pross@xvid.org>
 *
 *  This program is free software ; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation ; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY ; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program ; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * $Id: mem_transfer.c,v 1.16 2005/09/13 12:12:15 suxen_drol Exp $
 *
 ****************************************************************************/

#include "../global.h"
#include "mem_transfer.h"

#ifdef XVID_INCLUDE

#ifdef MDK_PLATFORM
#pragma arm section code = "XvidDecCode", rodata = "XvidDecCode", rwdata = "XvidDecData", zidata = "XvidDecBss"
#endif

#define USE_REFERENCE_C

/*****************************************************************************
 *
 * All these functions are used to transfer data from a 8 bit data array
 * to a 16 bit data array.
 *
 * This is typically used during motion compensation, that's why some
 * functions also do the addition/substraction of another buffer during the
 * so called  transfer.
 *
 ****************************************************************************/
#if 0
/*
 * SRC - the source buffer
 * DST - the destination buffer
 *
 * Then the function does the 8->16 bit transfer and this serie of operations :
 *
 *    SRC (8bit)  = SRC
 *    DST (16bit) = SRC
 */
void
transfer_8to16copy_c(int16_t *  dst,
                      uint8_t *  src,
                     uint32_t stride)
{
    int i, j;

    for (j = 0; j < 8; j++)
    {
        for (i = 0; i < 8; i++)
        {
            dst[j * 8 + i] = (int16_t) src[j * stride + i];
        }
    }
}
#endif

/*
 * SRC - the source buffer
 * DST - the destination buffer
 *
 * Then the function does the 8->16 bit transfer and this serie of operations :
 *
 *    SRC (16bit) = SRC
 *    DST (8bit)  = max(min(SRC, 255), 0)
 */
void
transfer_16to8copy_c(uint8_t *  dst,
                      int16_t *  src,
                     uint32_t stride)
{
    int i, j;

    for (j = 0; j < 8; j++)
    {
        for (i = 0; i < 8; i++)
        {
#ifdef USE_REFERENCE_C
            int16_t pixel = src[j * 8 + i];

            //if (pixel < 0)
            //{
            //    pixel = 0;
            //}
            //else if (pixel > 255) // pixel的取值范围为[-256, 255]
            //{
            //    pixel = 255;
            //}

            dst[j * stride + i] = (uint8_t) pixel;

#else
             int16_t pixel = src[j * 8 + i];
             uint8_t value = (uint8_t)( (pixel & ~255) ? (-pixel) >> (8 * sizeof(pixel) - 1) : pixel );
            dst[j*stride + i] = value;
#endif
        }
    }
}


#if 0

/*
 * C   - the current buffer
 * R   - the reference buffer
 * DCT - the dct coefficient buffer
 *
 * Then the function does the 8->16 bit transfer and this serie of operations :
 *
 *    R   (8bit)  = R
 *    C   (8bit)  = R
 *    DCT (16bit) = C - R
 */
void
transfer_8to16sub_c(int16_t *  dct,
                    uint8_t *  cur,
                     uint8_t * ref,
                     uint32_t stride)
{
    int i, j;

    for (j = 0; j < 8; j++)
    {
        for (i = 0; i < 8; i++)
        {
             uint8_t c = cur[j * stride + i];
             uint8_t r = ref[j * stride + i];

            cur[j * stride + i] = r;
            dct[j * 8 + i] = (int16_t) c - (int16_t) r;
        }
    }
}


void
transfer_8to16subro_c(int16_t *  dct,
                       uint8_t *  cur,
                       uint8_t * ref,
                       uint32_t stride)
{
    int i, j;

    for (j = 0; j < 8; j++)
    {
        for (i = 0; i < 8; i++)
        {
             uint8_t c = cur[j * stride + i];
             uint8_t r = ref[j * stride + i];
            dct[j * 8 + i] = (int16_t) c - (int16_t) r;
        }
    }
}



/*
 * C   - the current buffer
 * R1  - the 1st reference buffer
 * R2  - the 2nd reference buffer
 * DCT - the dct coefficient buffer
 *
 * Then the function does the 8->16 bit transfer and this serie of operations :
 *
 *    R1  (8bit) = R1
 *    R2  (8bit) = R2
 *    R   (temp) = min((R1 + R2)/2, 255)
 *    DCT (16bit)= C - R
 *    C   (8bit) = R
 */
void
transfer_8to16sub2_c(int16_t *  dct,
                     uint8_t *  cur,
                      uint8_t * ref1,
                      uint8_t * ref2,
                      uint32_t stride)
{
    uint32_t i, j;

    for (j = 0; j < 8; j++)
    {
        for (i = 0; i < 8; i++)
        {
             uint8_t c = cur[j * stride + i];
             uint8_t r = (ref1[j * stride + i] + ref2[j * stride + i] + 1) >> 1;
            cur[j * stride + i] = r;
            dct[j * 8 + i] = (int16_t) c - (int16_t) r;
        }
    }
}

void
transfer_8to16sub2ro_c(int16_t *  dct,
                        uint8_t *  cur,
                        uint8_t * ref1,
                        uint8_t * ref2,
                        uint32_t stride)
{
    uint32_t i, j;

    for (j = 0; j < 8; j++)
    {
        for (i = 0; i < 8; i++)
        {
             uint8_t c = cur[j * stride + i];
             uint8_t r = (ref1[j * stride + i] + ref2[j * stride + i] + 1) >> 1;
            dct[j * 8 + i] = (int16_t) c - (int16_t) r;
        }
    }
}


/*
 * SRC - the source buffer
 * DST - the destination buffer
 *
 * Then the function does the 16->8 bit transfer and this serie of operations :
 *
 *    SRC (16bit) = SRC
 *    DST (8bit)  = max(min(DST+SRC, 255), 0)
 */
void
transfer_16to8add_c(uint8_t *  dst,
                     int16_t *  src,
                    uint32_t stride)
{
    int i, j;

    for (j = 0; j < 8; j++)
    {
        for (i = 0; i < 8; i++)
        {
#ifdef USE_REFERENCE_C
            int16_t pixel = (int16_t) dst[j * stride + i] + src[j * 8 + i];

            if (pixel < 0)
            {
                pixel = 0;
            }
            else if (pixel > 255)
            {
                pixel = 255;
            }

            dst[j * stride + i] = (uint8_t) pixel;

#else
             int16_t pixel = (int16_t) dst[j * stride + i] + src[j * 8 + i];
             uint8_t value = (uint8_t)( (pixel & ~255) ? (-pixel) >> (8 * sizeof(pixel) - 1) : pixel );
            dst[j*stride + i] = value;
#endif

        }
    }
}

/*
 * SRC - the source buffer
 * DST - the destination buffer
 *
 * Then the function does the 8->8 bit transfer and this serie of operations :
 *
 *    SRC (8bit) = SRC
 *    DST (8bit) = SRC
 */
void
transfer8x8_copy_c(uint8_t *  dst,
                    uint8_t *  src,
                    uint32_t stride)
{
    int j, i;

    for (j = 0; j < 8; ++j)
    {
        uint8_t *d = dst + j * stride;
         uint8_t *s = src + j * stride;

        for (i = 0; i < 8; ++i)
        {
            *d++ = *s++;
        }
    }
}

/*
 * SRC - the source buffer
 * DST - the destination buffer
 *
 * Then the function does the 8->8 bit transfer and this serie of operations :
 *
 *    SRC (8bit) = SRC
 *    DST (8bit) = SRC
 */
void
transfer8x4_copy_c(uint8_t *  dst,
                    uint8_t *  src,
                    uint32_t stride)
{
    uint32_t j;

    for (j = 0; j < 4; j++)
    {
        uint32_t *d = (uint32_t*)(dst + j * stride);
         uint32_t *s = ( uint32_t*)(src + j * stride);
        *(d + 0) = *(s + 0);
        *(d + 1) = *(s + 1);
    }
}

#endif

#ifdef MDK_PLATFORM
#pragma arm section code
#endif

#endif

