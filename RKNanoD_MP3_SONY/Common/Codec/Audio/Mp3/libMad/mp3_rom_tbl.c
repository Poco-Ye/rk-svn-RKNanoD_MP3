/*
********************************************************************************
*                   Copyright (c) 2008, Rockchip
*                         All rights reserved.
*
* File Name£º   mp3_rom_tbl.c
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             Vincent Hsiung     2009-1-14         1.0
*    desc:    ORG.
********************************************************************************
*/

#include "../include/audio_main.h"


#ifdef A_CORE_DECODE
#ifdef MP3_DEC_INCLUDE

#include "mp3_global.h"

#define _X_(x) ((x)>>16)
#define static
#define mad_fixed_t	long
#define MAD_F(x)		((mad_fixed_t) (x##L))

/*
 * This is the lookup table for computing the CRC-check word.
 * As described in section 2.4.3.1 and depicted in Figure A.9
 * of ISO/IEC 11172-3, the generator polynomial is:
 *
 * G(X) = X^16 + X^15 + X^2 + 1
 */
//0

#define CALC

#pragma arm section code = "Mp3DecDataHL", rodata = "Mp3DecDataHL", rwdata = "Mp3DecDataHL", zidata = "Mp3DecDataHL"


unsigned short  mp3_crc_table[256] = {
  0x0000, 0x8005, 0x800f, 0x000a, 0x801b, 0x001e, 0x0014, 0x8011,
  0x8033, 0x0036, 0x003c, 0x8039, 0x0028, 0x802d, 0x8027, 0x0022,
  0x8063, 0x0066, 0x006c, 0x8069, 0x0078, 0x807d, 0x8077, 0x0072,
  0x0050, 0x8055, 0x805f, 0x005a, 0x804b, 0x004e, 0x0044, 0x8041,
  0x80c3, 0x00c6, 0x00cc, 0x80c9, 0x00d8, 0x80dd, 0x80d7, 0x00d2,
  0x00f0, 0x80f5, 0x80ff, 0x00fa, 0x80eb, 0x00ee, 0x00e4, 0x80e1,
  0x00a0, 0x80a5, 0x80af, 0x00aa, 0x80bb, 0x00be, 0x00b4, 0x80b1,
  0x8093, 0x0096, 0x009c, 0x8099, 0x0088, 0x808d, 0x8087, 0x0082,

  0x8183, 0x0186, 0x018c, 0x8189, 0x0198, 0x819d, 0x8197, 0x0192,
  0x01b0, 0x81b5, 0x81bf, 0x01ba, 0x81ab, 0x01ae, 0x01a4, 0x81a1,
  0x01e0, 0x81e5, 0x81ef, 0x01ea, 0x81fb, 0x01fe, 0x01f4, 0x81f1,
  0x81d3, 0x01d6, 0x01dc, 0x81d9, 0x01c8, 0x81cd, 0x81c7, 0x01c2,
  0x0140, 0x8145, 0x814f, 0x014a, 0x815b, 0x015e, 0x0154, 0x8151,
  0x8173, 0x0176, 0x017c, 0x8179, 0x0168, 0x816d, 0x8167, 0x0162,
  0x8123, 0x0126, 0x012c, 0x8129, 0x0138, 0x813d, 0x8137, 0x0132,
  0x0110, 0x8115, 0x811f, 0x011a, 0x810b, 0x010e, 0x0104, 0x8101,

  0x8303, 0x0306, 0x030c, 0x8309, 0x0318, 0x831d, 0x8317, 0x0312,
  0x0330, 0x8335, 0x833f, 0x033a, 0x832b, 0x032e, 0x0324, 0x8321,
  0x0360, 0x8365, 0x836f, 0x036a, 0x837b, 0x037e, 0x0374, 0x8371,
  0x8353, 0x0356, 0x035c, 0x8359, 0x0348, 0x834d, 0x8347, 0x0342,
  0x03c0, 0x83c5, 0x83cf, 0x03ca, 0x83db, 0x03de, 0x03d4, 0x83d1,
  0x83f3, 0x03f6, 0x03fc, 0x83f9, 0x03e8, 0x83ed, 0x83e7, 0x03e2,
  0x83a3, 0x03a6, 0x03ac, 0x83a9, 0x03b8, 0x83bd, 0x83b7, 0x03b2,
  0x0390, 0x8395, 0x839f, 0x039a, 0x838b, 0x038e, 0x0384, 0x8381,

  0x0280, 0x8285, 0x828f, 0x028a, 0x829b, 0x029e, 0x0294, 0x8291,
  0x82b3, 0x02b6, 0x02bc, 0x82b9, 0x02a8, 0x82ad, 0x82a7, 0x02a2,
  0x82e3, 0x02e6, 0x02ec, 0x82e9, 0x02f8, 0x82fd, 0x82f7, 0x02f2,
  0x02d0, 0x82d5, 0x82df, 0x02da, 0x82cb, 0x02ce, 0x02c4, 0x82c1,
  0x8243, 0x0246, 0x024c, 0x8249, 0x0258, 0x825d, 0x8257, 0x0252,
  0x0270, 0x8275, 0x827f, 0x027a, 0x826b, 0x026e, 0x0264, 0x8261,
  0x0220, 0x8225, 0x822f, 0x022a, 0x823b, 0x023e, 0x0234, 0x8231,
  0x8213, 0x0216, 0x021c, 0x8219, 0x0208, 0x820d, 0x8207, 0x0202
};

/*
 * scalefactor bit lengths
 * derived from section 2.4.2.7 of ISO/IEC 11172-3
 */
 //512
static
struct {
  unsigned char slen1;
  unsigned char slen2;
}  sflen_table[16] = {
  { 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 },
  { 3, 0 }, { 1, 1 }, { 1, 2 }, { 1, 3 },
  { 2, 1 }, { 2, 2 }, { 2, 3 }, { 3, 1 },
  { 3, 2 }, { 3, 3 }, { 4, 2 }, { 4, 3 }
};

/*
 * number of LSF scalefactor band values
 * derived from section 2.4.3.2 of ISO/IEC 13818-3
 */
 //544
static
unsigned char  nsfb_table[6][3][4] = {
  { {  6,  5,  5, 5 },
    {  9,  9,  9, 9 },
    {  6,  9,  9, 9 } },

  { {  6,  5,  7, 3 },
    {  9,  9, 12, 6 },
    {  6,  9, 12, 6 } },

  { { 11, 10,  0, 0 },
    { 18, 18,  0, 0 },
    { 15, 18,  0, 0 } },

  { {  7,  7,  7, 0 },
    { 12, 12, 12, 0 },
    {  6, 15, 12, 0 } },

  { {  6,  6,  6, 3 },
    { 12,  9,  9, 6 },
    {  6, 12,  9, 6 } },

  { {  8,  8,  5, 0 },
    { 15, 12,  9, 0 },
    {  6, 18,  9, 0 } }
};

/*
 * MPEG-1 scalefactor band widths
 * derived from Table B.8 of ISO/IEC 11172-3
 */
 //616
static
unsigned char  sfb_48000_long[] = {
   4,  4,  4,  4,  4,  4,  6,  6,  6,   8,  10,
  12, 16, 18, 22, 28, 34, 40, 46, 54,  54, 192
};
//638
static
unsigned char  sfb_44100_long[] = {
   4,  4,  4,  4,  4,  4,  6,  6,  8,   8,  10,
  12, 16, 20, 24, 28, 34, 42, 50, 54,  76, 158
};
//660
static
unsigned char  sfb_32000_long[] = {
   4,  4,  4,  4,  4,  4,  6,  6,  8,  10,  12,
  16, 20, 24, 30, 38, 46, 56, 68, 84, 102,  26
};
//682
static
unsigned char  sfb_48000_short[] = {
   4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  6,
   6,  6,  6,  6,  6, 10, 10, 10, 12, 12, 12, 14, 14,
  14, 16, 16, 16, 20, 20, 20, 26, 26, 26, 66, 66, 66
};
//721
static
unsigned char  sfb_44100_short[] = {
   4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  6,
   6,  6,  8,  8,  8, 10, 10, 10, 12, 12, 12, 14, 14,
  14, 18, 18, 18, 22, 22, 22, 30, 30, 30, 56, 56, 56
};

static
unsigned char  sfb_32000_short[] = {
   4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  6,
   6,  6,  8,  8,  8, 12, 12, 12, 16, 16, 16, 20, 20,
  20, 26, 26, 26, 34, 34, 34, 42, 42, 42, 12, 12, 12
};

static
unsigned char  sfb_48000_mixed[] = {
  /* long */   4,  4,  4,  4,  4,  4,  6,  6,
  /* short */  4,  4,  4,  6,  6,  6,  6,  6,  6, 10,
              10, 10, 12, 12, 12, 14, 14, 14, 16, 16,
              16, 20, 20, 20, 26, 26, 26, 66, 66, 66
};

static
unsigned char  sfb_44100_mixed[] = {
  /* long */   4,  4,  4,  4,  4,  4,  6,  6,
  /* short */  4,  4,  4,  6,  6,  6,  8,  8,  8, 10,
              10, 10, 12, 12, 12, 14, 14, 14, 18, 18,
              18, 22, 22, 22, 30, 30, 30, 56, 56, 56
};

static
unsigned char  sfb_32000_mixed[] = {
  /* long */   4,  4,  4,  4,  4,  4,  6,  6,
  /* short */  4,  4,  4,  6,  6,  6,  8,  8,  8, 12,
              12, 12, 16, 16, 16, 20, 20, 20, 26, 26,
              26, 34, 34, 34, 42, 42, 42, 12, 12, 12
};

/*
 * MPEG-2 scalefactor band widths
 * derived from Table B.2 of ISO/IEC 13818-3
 */
static
unsigned char  sfb_24000_long[] = {
   6,  6,  6,  6,  6,  6,  8, 10, 12,  14,  16,
  18, 22, 26, 32, 38, 46, 54, 62, 70,  76,  36
};

static
unsigned char  sfb_22050_long[] = {
   6,  6,  6,  6,  6,  6,  8, 10, 12,  14,  16,
  20, 24, 28, 32, 38, 46, 52, 60, 68,  58,  54
};

# define sfb_16000_long  sfb_22050_long

static
unsigned char  sfb_24000_short[] = {
   4,  4,  4,  4,  4,  4,  4,  4,  4,  6,  6,  6,  8,
   8,  8, 10, 10, 10, 12, 12, 12, 14, 14, 14, 18, 18,
  18, 24, 24, 24, 32, 32, 32, 44, 44, 44, 12, 12, 12
};

static
unsigned char  sfb_22050_short[] = {
   4,  4,  4,  4,  4,  4,  4,  4,  4,  6,  6,  6,  6,
   6,  6,  8,  8,  8, 10, 10, 10, 14, 14, 14, 18, 18,
  18, 26, 26, 26, 32, 32, 32, 42, 42, 42, 18, 18, 18
};

static
unsigned char  sfb_16000_short[] = {
   4,  4,  4,  4,  4,  4,  4,  4,  4,  6,  6,  6,  8,
   8,  8, 10, 10, 10, 12, 12, 12, 14, 14, 14, 18, 18,
  18, 24, 24, 24, 30, 30, 30, 40, 40, 40, 18, 18, 18
};

static
unsigned char  sfb_24000_mixed[] = {
  /* long */   6,  6,  6,  6,  6,  6,
  /* short */  6,  6,  6,  8,  8,  8, 10, 10, 10, 12,
              12, 12, 14, 14, 14, 18, 18, 18, 24, 24,
              24, 32, 32, 32, 44, 44, 44, 12, 12, 12
};

static
unsigned char  sfb_22050_mixed[] = {
  /* long */   6,  6,  6,  6,  6,  6,
  /* short */  6,  6,  6,  6,  6,  6,  8,  8,  8, 10,
              10, 10, 14, 14, 14, 18, 18, 18, 26, 26,
              26, 32, 32, 32, 42, 42, 42, 18, 18, 18
};

static
unsigned char  sfb_16000_mixed[] = {
  /* long */   6,  6,  6,  6,  6,  6,
  /* short */  6,  6,  6,  8,  8,  8, 10, 10, 10, 12,
              12, 12, 14, 14, 14, 18, 18, 18, 24, 24,
              24, 30, 30, 30, 40, 40, 40, 18, 18, 18
};

/*
 * MPEG 2.5 scalefactor band widths
 * derived from public sources
 */
# define sfb_12000_long  sfb_16000_long
# define sfb_11025_long  sfb_12000_long

static
unsigned char  sfb_8000_long[] = {
  12, 12, 12, 12, 12, 12, 16, 20, 24,  28,  32,
  40, 48, 56, 64, 76, 90,  2,  2,  2,   2,   2
};

# define sfb_12000_short  sfb_16000_short
# define sfb_11025_short  sfb_12000_short

static
unsigned char  sfb_8000_short[] = {
   8,  8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 16,
  16, 16, 20, 20, 20, 24, 24, 24, 28, 28, 28, 36, 36,
  36,  2,  2,  2,  2,  2,  2,  2,  2,  2, 26, 26, 26
};

# define sfb_12000_mixed  sfb_16000_mixed
# define sfb_11025_mixed  sfb_12000_mixed

/* the 8000 Hz short block scalefactor bands do not break after
   the first 36 frequency lines, so this is probably wrong */
static
unsigned char  sfb_8000_mixed[] = {
  /* long */  12, 12, 12,
  /* short */  4,  4,  4,  8,  8,  8, 12, 12, 12, 16, 16, 16,
              20, 20, 20, 24, 24, 24, 28, 28, 28, 36, 36, 36,
               2,  2,  2,  2,  2,  2,  2,  2,  2, 26, 26, 26
};

static
struct {
  unsigned char  *l;
  unsigned char  *s;
  unsigned char  *m;
}  sfbwidth_table[9] = {
  { sfb_48000_long, sfb_48000_short, sfb_48000_mixed },
  { sfb_44100_long, sfb_44100_short, sfb_44100_mixed },
  { sfb_32000_long, sfb_32000_short, sfb_32000_mixed },
  { sfb_24000_long, sfb_24000_short, sfb_24000_mixed },
  { sfb_22050_long, sfb_22050_short, sfb_22050_mixed },
  { sfb_16000_long, sfb_16000_short, sfb_16000_mixed },
  { sfb_12000_long, sfb_12000_short, sfb_12000_mixed },
  { sfb_11025_long, sfb_11025_short, sfb_11025_mixed },
  {  sfb_8000_long,  sfb_8000_short,  sfb_8000_mixed }
};

/*
 * scalefactor band preemphasis (used only when preflag is set)
 * derived from Table B.6 of ISO/IEC 11172-3
 */
static
unsigned char  pretab[22] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 3, 3, 3, 2, 0
};

/*
 * table for requantization
 *
 * rq_table[x].mantissa * 2^(rq_table[x].exponent) = x^(4/3)
 */
/*

#define FLASHROM


#ifndef FLASHROM
static
struct fixedfloat {
  unsigned long mantissa  : 27;
  unsigned short exponent :  5;
} const rq_table[8207] = {
# include "mp3_rq_table.dat"
};
#else


struct fixedfloat {
  unsigned long mantissa  : 27;
  unsigned short exponent :  5;
} ;

_ATTR_MP3DEC_FLASHDATA_
struct fixedfloat   const rq_table_flash[5135] = {
# include "mp3_flashrom.dat"
};


struct fixedfloat const rq_table[3072+15] = {
# include "mp3_rom.dat"
};
#endif
*/




#ifdef CALC
//_ATTR_MP3DEC_TEXT_
static struct fixedfloat {
  unsigned long mantissa  : 27;
  unsigned short exponent :  5;
}  rq_table_6159[6159] = {
# include "rq_6159.dat"
};



#else 
struct fixedfloat {
  unsigned long mantissa  : 27;
  unsigned short exponent :  5;
} const rq_table[8207] = {
# include "mp3_rq_table.dat"
};


#endif


/*
 * fractional powers of two
 * used for requantization and joint stereo decoding
 *
 * root_table[3 + x] = 2^(x/4)
 */
static
mad_fixed_t  root_table[7] = {
  _X_(MAD_F(0x09837f05)) /* 2^(-3/4) == 0.59460355750136 */,
  _X_(MAD_F(0x0b504f33)) /* 2^(-2/4) == 0.70710678118655 */,
  _X_(MAD_F(0x0d744fcd)) /* 2^(-1/4) == 0.84089641525371 */,
  _X_(MAD_F(0x10000000)) /* 2^( 0/4) == 1.00000000000000 */,
  _X_(MAD_F(0x1306fe0a)) /* 2^(+1/4) == 1.18920711500272 */,
  _X_(MAD_F(0x16a09e66)) /* 2^(+2/4) == 1.41421356237310 */,
  _X_(MAD_F(0x1ae89f99)) /* 2^(+3/4) == 1.68179283050743 */
};

/*
 * coefficients for aliasing reduction
 * derived from Table B.9 of ISO/IEC 11172-3
 *
 *  c[]  = { -0.6, -0.535, -0.33, -0.185, -0.095, -0.041, -0.0142, -0.0037 }
 * cs[i] =    1 / sqrt(1 + c[i]^2)
 * ca[i] = c[i] / sqrt(1 + c[i]^2)
 */
static
mad_fixed_t  cs[8] = {
  +_X_(MAD_F(0x0db84a81)) /* +0.857492926 */, +_X_(MAD_F(0x0e1b9d7f)) /* +0.881741997 */,
  +_X_(MAD_F(0x0f31adcf)) /* +0.949628649 */, +_X_(MAD_F(0x0fbba815)) /* +0.983314592 */,
  +_X_(MAD_F(0x0feda417)) /* +0.995517816 */, +_X_(MAD_F(0x0ffc8fc8)) /* +0.999160558 */,
  +_X_(MAD_F(0x0fff964c)) /* +0.999899195 */, +_X_(MAD_F(0x0ffff8d3)) /* +0.999993155 */
};

static
mad_fixed_t  ca[8] = {
  -_X_(MAD_F(0x083b5fe7)) /* -0.514495755 */, -_X_(MAD_F(0x078c36d2)) /* -0.471731969 */,
  -_X_(MAD_F(0x05039814)) /* -0.313377454 */, -_X_(MAD_F(0x02e91dd1)) /* -0.181913200 */,
  -_X_(MAD_F(0x0183603a)) /* -0.094574193 */, -_X_(MAD_F(0x00a7cb87)) /* -0.040965583 */,
  -_X_(MAD_F(0x003a2847)) /* -0.014198569 */, -_X_(MAD_F(0x000f27b4)) /* -0.003699975 */
};

/*
 * IMDCT coefficients for short blocks
 * derived from section 2.4.3.4.10.2 of ISO/IEC 11172-3
 *
 * imdct_s[i/even][k] = cos((PI / 24) * (2 *       (i / 2) + 7) * (2 * k + 1))
 * imdct_s[i /odd][k] = cos((PI / 24) * (2 * (6 + (i-1)/2) + 7) * (2 * k + 1))
 */
static
mad_fixed_t  imdct_s[6][6] = {
# include "mp3_imdct_s.dat"
};

/*
 * windowing coefficients for long blocks
 * derived from section 2.4.3.4.10.3 of ISO/IEC 11172-3
 *
 * window_l[i] = sin((PI / 36) * (i + 1/2))
 */
static
mad_fixed_t  window_l[36] = {
  _X_(MAD_F(0x00b2aa3e)) /* 0.043619387 */, _X_(MAD_F(0x0216a2a2)) /* 0.130526192 */,
  _X_(MAD_F(0x03768962)) /* 0.216439614 */, _X_(MAD_F(0x04cfb0e2)) /* 0.300705800 */,
  _X_(MAD_F(0x061f78aa)) /* 0.382683432 */, _X_(MAD_F(0x07635284)) /* 0.461748613 */,
  _X_(MAD_F(0x0898c779)) /* 0.537299608 */, _X_(MAD_F(0x09bd7ca0)) /* 0.608761429 */,
  _X_(MAD_F(0x0acf37ad)) /* 0.675590208 */, _X_(MAD_F(0x0bcbe352)) /* 0.737277337 */,
  _X_(MAD_F(0x0cb19346)) /* 0.793353340 */, _X_(MAD_F(0x0d7e8807)) /* 0.843391446 */,

  _X_(MAD_F(0x0e313245)) /* 0.887010833 */, _X_(MAD_F(0x0ec835e8)) /* 0.923879533 */,
  _X_(MAD_F(0x0f426cb5)) /* 0.953716951 */, _X_(MAD_F(0x0f9ee890)) /* 0.976296007 */,
  _X_(MAD_F(0x0fdcf549)) /* 0.991444861 */, _X_(MAD_F(0x0ffc19fd)) /* 0.999048222 */,
  _X_(MAD_F(0x0ffc19fd)) /* 0.999048222 */, _X_(MAD_F(0x0fdcf549)) /* 0.991444861 */,
  _X_(MAD_F(0x0f9ee890)) /* 0.976296007 */, _X_(MAD_F(0x0f426cb5)) /* 0.953716951 */,
  _X_(MAD_F(0x0ec835e8)) /* 0.923879533 */, _X_(MAD_F(0x0e313245)) /* 0.887010833 */,

  _X_(MAD_F(0x0d7e8807)) /* 0.843391446 */, _X_(MAD_F(0x0cb19346)) /* 0.793353340 */,
  _X_(MAD_F(0x0bcbe352)) /* 0.737277337 */, _X_(MAD_F(0x0acf37ad)) /* 0.675590208 */,
  _X_(MAD_F(0x09bd7ca0)) /* 0.608761429 */, _X_(MAD_F(0x0898c779)) /* 0.537299608 */,
  _X_(MAD_F(0x07635284)) /* 0.461748613 */, _X_(MAD_F(0x061f78aa)) /* 0.382683432 */,
  _X_(MAD_F(0x04cfb0e2)) /* 0.300705800 */, _X_(MAD_F(0x03768962)) /* 0.216439614 */,
  _X_(MAD_F(0x0216a2a2)) /* 0.130526192 */, _X_(MAD_F(0x00b2aa3e)) /* 0.043619387 */,
};

/*
 * windowing coefficients for short blocks
 * derived from section 2.4.3.4.10.3 of ISO/IEC 11172-3
 *
 * window_s[i] = sin((PI / 12) * (i + 1/2))
 */
static
mad_fixed_t  window_s[12] = {
  _X_(MAD_F(0x0216a2a2)) /* 0.130526192 */, _X_(MAD_F(0x061f78aa)) /* 0.382683432 */,
  _X_(MAD_F(0x09bd7ca0)) /* 0.608761429 */, _X_(MAD_F(0x0cb19346)) /* 0.793353340 */,
  _X_(MAD_F(0x0ec835e8)) /* 0.923879533 */, _X_(MAD_F(0x0fdcf549)) /* 0.991444861 */,
  _X_(MAD_F(0x0fdcf549)) /* 0.991444861 */, _X_(MAD_F(0x0ec835e8)) /* 0.923879533 */,
  _X_(MAD_F(0x0cb19346)) /* 0.793353340 */, _X_(MAD_F(0x09bd7ca0)) /* 0.608761429 */,
  _X_(MAD_F(0x061f78aa)) /* 0.382683432 */, _X_(MAD_F(0x0216a2a2)) /* 0.130526192 */,
};

/*
 * coefficients for intensity stereo processing
 * derived from section 2.4.3.4.9.3 of ISO/IEC 11172-3
 *
 * is_ratio[i] = tan(i * (PI / 12))
 * is_table[i] = is_ratio[i] / (1 + is_ratio[i])
 */
static
mad_fixed_t  is_table[7] = {
  _X_(MAD_F(0x00000000)) /* 0.000000000 */,
  _X_(MAD_F(0x0361962f)) /* 0.211324865 */,
  _X_(MAD_F(0x05db3d74)) /* 0.366025404 */,
  _X_(MAD_F(0x08000000)) /* 0.500000000 */,
  _X_(MAD_F(0x0a24c28c)) /* 0.633974596 */,
  _X_(MAD_F(0x0c9e69d1)) /* 0.788675135 */,
  _X_(MAD_F(0x10000000)) /* 1.000000000 */
};

/*
 * coefficients for LSF intensity stereo processing
 * derived from section 2.4.3.2 of ISO/IEC 13818-3
 *
 * is_lsf_table[0][i] = (1 / sqrt(sqrt(2)))^(i + 1)
 * is_lsf_table[1][i] = (1 /      sqrt(2)) ^(i + 1)
 */
static
mad_fixed_t  is_lsf_table[2][15] = {
  {
    _X_(MAD_F(0x0d744fcd)) /* 0.840896415 */,
    _X_(MAD_F(0x0b504f33)) /* 0.707106781 */,
    _X_(MAD_F(0x09837f05)) /* 0.594603558 */,
    _X_(MAD_F(0x08000000)) /* 0.500000000 */,
    _X_(MAD_F(0x06ba27e6)) /* 0.420448208 */,
    _X_(MAD_F(0x05a8279a)) /* 0.353553391 */,
    _X_(MAD_F(0x04c1bf83)) /* 0.297301779 */,
    _X_(MAD_F(0x04000000)) /* 0.250000000 */,
    _X_(MAD_F(0x035d13f3)) /* 0.210224104 */,
    _X_(MAD_F(0x02d413cd)) /* 0.176776695 */,
    _X_(MAD_F(0x0260dfc1)) /* 0.148650889 */,
    _X_(MAD_F(0x02000000)) /* 0.125000000 */,
    _X_(MAD_F(0x01ae89fa)) /* 0.105112052 */,
    _X_(MAD_F(0x016a09e6)) /* 0.088388348 */,
    _X_(MAD_F(0x01306fe1)) /* 0.074325445 */
  }, {
    _X_(MAD_F(0x0b504f33)) /* 0.707106781 */,
    _X_(MAD_F(0x08000000)) /* 0.500000000 */,
    _X_(MAD_F(0x05a8279a)) /* 0.353553391 */,
    _X_(MAD_F(0x04000000)) /* 0.250000000 */,
    _X_(MAD_F(0x02d413cd)) /* 0.176776695 */,
    _X_(MAD_F(0x02000000)) /* 0.125000000 */,
    _X_(MAD_F(0x016a09e6)) /* 0.088388348 */,
    _X_(MAD_F(0x01000000)) /* 0.062500000 */,
    _X_(MAD_F(0x00b504f3)) /* 0.044194174 */,
    _X_(MAD_F(0x00800000)) /* 0.031250000 */,
    _X_(MAD_F(0x005a827a)) /* 0.022097087 */,
    _X_(MAD_F(0x00400000)) /* 0.015625000 */,
    _X_(MAD_F(0x002d413d)) /* 0.011048543 */,
    _X_(MAD_F(0x00200000)) /* 0.007812500 */,
    _X_(MAD_F(0x0016a09e)) /* 0.005524272 */
  }
};

//frame.c----------------
static
unsigned long  bitrate_table[5][15] = {
  /* MPEG-1 */
  { 0,  32000,  64000,  96000, 128000, 160000, 192000, 224000,  /* Layer I   */
       256000, 288000, 320000, 352000, 384000, 416000, 448000 },
  { 0,  32000,  48000,  56000,  64000,  80000,  96000, 112000,  /* Layer II  */
       128000, 160000, 192000, 224000, 256000, 320000, 384000 },
  { 0,  32000,  40000,  48000,  56000,  64000,  80000,  96000,  /* Layer III */
       112000, 128000, 160000, 192000, 224000, 256000, 320000 },

  /* MPEG-2 LSF */
  { 0,  32000,  48000,  56000,  64000,  80000,  96000, 112000,  /* Layer I   */
       128000, 144000, 160000, 176000, 192000, 224000, 256000 },
  { 0,   8000,  16000,  24000,  32000,  40000,  48000,  56000,  /* Layers    */
        64000,  80000,  96000, 112000, 128000, 144000, 160000 } /* II & III  */
};

static
unsigned int  samplerate_table[3] = { 44100, 48000, 32000 };

//--------------------------------

//layer12.c-------------------
/*
 * scalefactor table
 * used in both Layer I and Layer II decoding
 */
static
mad_fixed_t  sf_table[64] = {
# include "mp3_sf_table.dat"
};

/* --- Layer I ------------------------------------------------------------- */

/* linear scaling table */
static
mad_fixed_t  linear_table[14] = {
  _X_(MAD_F(0x15555555)),  /* 2^2  / (2^2  - 1) == 1.33333333333333 */
  _X_(MAD_F(0x12492492)),  /* 2^3  / (2^3  - 1) == 1.14285714285714 */
  _X_(MAD_F(0x11111111)),  /* 2^4  / (2^4  - 1) == 1.06666666666667 */
  _X_(MAD_F(0x10842108)),  /* 2^5  / (2^5  - 1) == 1.03225806451613 */
  _X_(MAD_F(0x10410410)),  /* 2^6  / (2^6  - 1) == 1.01587301587302 */
  _X_(MAD_F(0x10204081)),  /* 2^7  / (2^7  - 1) == 1.00787401574803 */
  _X_(MAD_F(0x10101010)),  /* 2^8  / (2^8  - 1) == 1.00392156862745 */
  _X_(MAD_F(0x10080402)),  /* 2^9  / (2^9  - 1) == 1.00195694716243 */
  _X_(MAD_F(0x10040100)),  /* 2^10 / (2^10 - 1) == 1.00097751710655 */
  _X_(MAD_F(0x10020040)),  /* 2^11 / (2^11 - 1) == 1.00048851978505 */
  _X_(MAD_F(0x10010010)),  /* 2^12 / (2^12 - 1) == 1.00024420024420 */
  _X_(MAD_F(0x10008004)),  /* 2^13 / (2^13 - 1) == 1.00012208521548 */
  _X_(MAD_F(0x10004001)),  /* 2^14 / (2^14 - 1) == 1.00006103888177 */
  _X_(MAD_F(0x10002000))   /* 2^15 / (2^15 - 1) == 1.00003051850948 */
};

/* --- Layer II ------------------------------------------------------------ */

/* possible quantization per subband table */
static
struct {
  unsigned int sblimit;
  unsigned char  offsets[30];
}  sbquant_table[5] = {
  /* ISO/IEC 11172-3 Table B.2a */
  { 27, { 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 3, 3, 3, 3, 3,	/* 0 */
	  3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0 } },
  /* ISO/IEC 11172-3 Table B.2b */
  { 30, { 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 3, 3, 3, 3, 3,	/* 1 */
	  3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0 } },
  /* ISO/IEC 11172-3 Table B.2c */
  {  8, { 5, 5, 2, 2, 2, 2, 2, 2 } },				/* 2 */
  /* ISO/IEC 11172-3 Table B.2d */
  { 12, { 5, 5, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 } },		/* 3 */
  /* ISO/IEC 13818-3 Table B.1 */
  { 30, { 4, 4, 4, 4, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1,	/* 4 */
	  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 } }
};

/* bit allocation table */
static
struct {
  unsigned short nbal;
  unsigned short offset;
}  bitalloc_table[8] = {
  { 2, 0 },  /* 0 */
  { 2, 3 },  /* 1 */
  { 3, 3 },  /* 2 */
  { 3, 1 },  /* 3 */
  { 4, 2 },  /* 4 */
  { 4, 3 },  /* 5 */
  { 4, 4 },  /* 6 */
  { 4, 5 }   /* 7 */
};

/* offsets into quantization class table */
static
unsigned char  offset_table[6][15] = {
  { 0, 1, 16                                             },  /* 0 */
  { 0, 1,  2, 3, 4, 5, 16                                },  /* 1 */
  { 0, 1,  2, 3, 4, 5,  6, 7,  8,  9, 10, 11, 12, 13, 14 },  /* 2 */
  { 0, 1,  3, 4, 5, 6,  7, 8,  9, 10, 11, 12, 13, 14, 15 },  /* 3 */
  { 0, 1,  2, 3, 4, 5,  6, 7,  8,  9, 10, 11, 12, 13, 16 },  /* 4 */
  { 0, 2,  4, 5, 6, 7,  8, 9, 10, 11, 12, 13, 14, 15, 16 }   /* 5 */
};

/* quantization class table */
static
struct quantclass {
  unsigned short nlevels;
  unsigned char group;
  unsigned char bits;
  mad_fixed_t C;
  mad_fixed_t D;
}  qc_table[17] = {
# include "mp3_qc_table.dat"
};
//--------------------------------

#include "mp3_huffman.dat"


#pragma arm section code

#ifdef CALC
_ATTR_MP3DEC_TEXT_
struct fixedfloat rq_table_2048[2048] ={
#include "rq_2048.dat"
};
#endif

#endif
#endif

