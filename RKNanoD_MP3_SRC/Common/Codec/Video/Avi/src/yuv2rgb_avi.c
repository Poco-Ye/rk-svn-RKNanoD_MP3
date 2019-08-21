/*****************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  - Console based decoding test application  -
 *
 *  Copyright(C) 2002-2003 Christoph Lampert
 *               2002-2003 Edouard Gomez <ed.gomez@free.fr>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * $Id: xvid_decraw.c,v 1.22.2.1 2006/07/10 15:19:41 Isibaar Exp $
 *
 ****************************************************************************/

//#include <stdio.h>
//#include <stdlib.h>
#include <string.h>
//#include <math.h>

#include "xvid.h"
//#include "src/portab.h"
#include "xvid_dec_main.h"

#ifdef XVID_INCLUDE

#ifdef MDK_PLATFORM
#pragma arm section code = "XvidDecCode", rodata = "XvidDecCode", rwdata = "XvidDecData", zidata = "XvidDecBss"
#endif



#ifdef YUV_TO_RGB_OPTIMATION

#ifdef YUV_TO_RGB_MPEG4
/*
** yuv2rgb565优化需要增加3.0kbytes的空间
*/

#if 0 // remove to iram
__attribute__((section("XvidDecCode"))) short yuv2rgb_tab_1220542[256];
__attribute__((section("XvidDecCode"))) short yuv2rgb_tab_1673527[256];
__attribute__((section("XvidDecCode"))) signed char  yuv2rgb_tab_852492[256];
__attribute__((section("XvidDecCode"))) signed char  yuv2rgb_tab_409993[256];
__attribute__((section("XvidDecCode"))) short yuv2rgb_tab_2116026[256];
__attribute__((section("XvidDecCode"))) unsigned char yuv2rgb_tab_clip[1024];
#else
short yuv2rgb_tab_1220542[256];
short yuv2rgb_tab_1673527[256];
signed char  yuv2rgb_tab_852492[256];
signed char  yuv2rgb_tab_409993[256];
short yuv2rgb_tab_2116026[256];
unsigned char yuv2rgb_tab_clip[1024];
#endif

unsigned char *p_yuv2rgb_tab_clip;

int yuv2rgb_tab_init(void)
{
	int i;

	for(i = 0; i < 256; i++)
	{
		yuv2rgb_tab_1220542[i] = (short)((1220542 * (i-16)) >> 20);
		yuv2rgb_tab_1673527[i] = (short)((1673527 * (i-128)) >> 20);
		yuv2rgb_tab_852492[i]  = (signed char) ((852492 * (i-128)) >> 20);
		yuv2rgb_tab_409993[i]  = (signed char) ((409993 * (i-128)) >> 20);
		yuv2rgb_tab_2116026[i] = (short)((2116026 * (i-128)) >> 20);
 	}

	p_yuv2rgb_tab_clip = yuv2rgb_tab_clip + 384;
	for(i = -384; i < 640; i++)
	{
		if(i <= 0)
		{
			p_yuv2rgb_tab_clip[i] = 0;
		}
		else if(i > 255)
		{
			p_yuv2rgb_tab_clip[i] = 255;
		}
		else
		{
			p_yuv2rgb_tab_clip[i] = i;
		}
	}

	return 0;
}

#else //YUV_TO_RGB_MPEG4
/*
** yuv2rgb565优化需要增加2.5kbytes的空间
*/

short yuv2rgb_tab_1470103[256];
signed char  yuv2rgb_tab_360857[256];
signed char  yuv2rgb_tab_748830[256];
short yuv2rgb_tab_1858076[256];

unsigned char yuv2rgb_tab_clip[1024];
unsigned char *p_yuv2rgb_tab_clip;

int yuv2rgb_tab_init(void)
{
	int i;

	for(i = 0; i < 256; i++)
	{
		yuv2rgb_tab_1470103[i] = (short)((1470103 * (i-128)) >> 20);
		yuv2rgb_tab_360857[i]  = (signed char) ((360857 * (i-128)) >> 20);
		yuv2rgb_tab_748830[i]  = (signed char) ((748830 * (i-128)) >> 20);
		yuv2rgb_tab_1858076[i] = (short)((1858076 * (i-128)) >> 20);
 	}

	p_yuv2rgb_tab_clip = yuv2rgb_tab_clip + 512;
	for(i = -512; i < 512; i++)
	{
		if(i <= 0)
		{
			p_yuv2rgb_tab_clip[i] = 0;
		}
		else if(i > 255)
		{
			p_yuv2rgb_tab_clip[i] = 255;
		}
		else
		{
			p_yuv2rgb_tab_clip[i] = i;
		}
	}

	return 0;
}
#endif  //YUV_TO_RGB_MPEG4

#endif  //YUV_TO_RGB_OPTIMATION

#if 0
int xvid_yuv_rgb565WithDMA(char *pY, char *pU, char *pV, short *pRgb, int stride)
{
    int i, j;
    int R, G, B, RGB;
    unsigned char *pRowY, *pRowU, *pRowV;
    int Y0, Y1, U, V;
    int *pSlice;
    int lcd_width = LCD_WIDTH;

    int a;

    if (stride > XVID_MAX_X_WIDTH)
    {
        return -1;
    }

    // yuv格式是4:2:0

    for (j = 0; j < 16; j++)        // row
    {
        pSlice = (int *)(pRgb + j*lcd_width);

        pRowY = pY + j*stride;
        pRowU = pU + (j>>1)*(stride>>1);
        pRowV = pV + (j>>1)*(stride>>1);

        //for(i = 0; i < stride; i++)         // column
        for (i = lcd_width/2-1; i >= 0; i--)  // column
        {
            Y0 = *pRowY++;
            Y1 = *pRowY++;
            U  = *pRowU++;
            V  = *pRowV++;

            {
                Y0 = yuv2rgb_tab_1220542[Y0];
                Y1 = yuv2rgb_tab_1220542[Y1];

                a = yuv2rgb_tab_1673527[V];
                R = (p_yuv2rgb_tab_clip[Y1 + a]<<16) | (p_yuv2rgb_tab_clip[Y0 + a]);

                a = yuv2rgb_tab_852492[V] + yuv2rgb_tab_409993[U];
                G = (p_yuv2rgb_tab_clip[Y1 - a]<<16) | (p_yuv2rgb_tab_clip[Y0 - a]);

                a = yuv2rgb_tab_2116026[U];
                B = (p_yuv2rgb_tab_clip[Y1 + a]<<16) | (p_yuv2rgb_tab_clip[Y0 + a]);

#if 1 // swap rgb565 data, for dma
                RGB  = (B & 0x00F800F8) << 5;
                RGB |= (G & 0x00E000E0) >> 5;
                RGB |= (G & 0x001C001C) << 11;
                RGB |= (R & 0x00F800F8);
#else
                RGB  = (R & 0x00F800F8) << 8;
                RGB |= (G & 0x00FC00FC) << 3;
                RGB |= (B & 0x00F800F8) >> 3;
#endif
                *pSlice++ = RGB;
            }
        }
    }

    return 0;
}
#endif

int xvid_yuv_rgb565(char *pY, char *pU, char *pV, short *pRgb, int stride)
{
    int i, j;
    int R, G, B, RGB;
    unsigned char *pRowY, *pRowU, *pRowV;
    int Y0, Y1, U, V;
    int *pSlice;
    int lcd_width = (LCD_WIDTH > LCD_HEIGHT)?LCD_WIDTH:LCD_HEIGHT;

    int a;

    if (stride > XVID_MAX_X_WIDTH)
    {
        return -1;
    }

    // yuv格式是4:2:0

    for (j = 0; j < 16; j++)        // row
    {
        pSlice = (int *)(pRgb + j*lcd_width);

        pRowY = pY + j*stride;
        pRowU = pU + (j>>1)*(stride>>1);
        pRowV = pV + (j>>1)*(stride>>1);

        //for(i = 0; i < stride; i++)         // column
        for (i = lcd_width/2-1; i >= 0; i--)  // column
        {
            Y0 = *pRowY++;
            Y1 = *pRowY++;
            U  = *pRowU++;
            V  = *pRowV++;

            {
                Y0 = yuv2rgb_tab_1220542[Y0];
                Y1 = yuv2rgb_tab_1220542[Y1];

                a = yuv2rgb_tab_1673527[V];
                R = (p_yuv2rgb_tab_clip[Y1 + a]<<16) | (p_yuv2rgb_tab_clip[Y0 + a]);

                a = yuv2rgb_tab_852492[V] + yuv2rgb_tab_409993[U];
                G = (p_yuv2rgb_tab_clip[Y1 - a]<<16) | (p_yuv2rgb_tab_clip[Y0 - a]);

                a = yuv2rgb_tab_2116026[U];
                B = (p_yuv2rgb_tab_clip[Y1 + a]<<16) | (p_yuv2rgb_tab_clip[Y0 + a]);

#if 0 // swap rgb565 data, for dma
                RGB  = (B & 0x00F800F8) << 5;
                RGB |= (G & 0x00E000E0) >> 5;
                RGB |= (G & 0x001C001C) << 11;
                RGB |= (R & 0x00F800F8);
#else
                RGB  = (R & 0x00F800F8) << 8;
                RGB |= (G & 0x00FC00FC) << 3;
                RGB |= (B & 0x00F800F8) >> 3;
#endif
                *pSlice++ = RGB;
            }
        }
    }

    return 0;
}

#ifdef MDK_PLATFORM
#pragma arm section code
#endif

#endif //XVID_INCLUDE

