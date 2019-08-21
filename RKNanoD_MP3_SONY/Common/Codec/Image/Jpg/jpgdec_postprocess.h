#ifndef JPGDEC_POSTPROCESS_H
#define JPGDEC_POSTPROCESS_H

#include "jpgdec_global.h"

#define SHIFT_SCALE 9

#define RGB_Y  0x0254
#define B_U  0x0409
#define G_U  0x00c8
#define G_V  0x01a0
#define R_V  0x0331

#define MK_RGB565(R,G,B) ((max(0,min(255, R)) << 8) & 0xf800) | \
       ((max(0,min(255, G)) << 3) & 0x07e0) | \
       ((max(0,min(255, B)) >> 3) & 0x001f)//RGB的565格式的第15位到第11位为R,第10位到第5位为G,第4位到第0位为B

#define MK_RGB555(R,G,B) ((max(0,min(255, R)) << 7) & 0x7c00) | \
       ((max(0,min(255, G)) << 2) & 0x03e0) | \
       ((max(0,min(255, B)) >> 3) & 0x001f)//RGB的555格式的最高位(第15位)为0,第14位到第10位为R,第9位到第5位为G,第4位到第0位为B


#define MK_RGB888(R,G,B) (((long)max(0,min(255, B)) << 24) & 0xff000000L) | \
                         (((long)max(0,min(255, G)) << 16) & 0xff0000L) | \
                         (((long)max(0,min(255, R)) << 8) & 0xff00L)//RGB的888格式的低8位填0?

//int16  YUVHORIZENTAL[SCR_LENGTH*6];
//int16  YUVBUF[SCR_LENGTH*3];
//uint16  output_line[SCR_LENGTH];     // initial to be 0

//void fwritepc(void);
#endif
