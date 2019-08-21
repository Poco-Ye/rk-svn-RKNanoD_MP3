/*
********************************************************************************
*                   Copyright (c) 2008, Rockchip
*                         All rights reserved.
*
* File Name：   layer3.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             Vincent Hsiung     2009-1-14         1.0
*    desc:    ORG.
********************************************************************************
*/

/*
    1. frame_overlap_buff[] 数据类型修改为 unsigned long;
    2. 将本文件中静态表格移动到SoC中的TableROM中;
    3. 将III_huffdecode()更换为高效版本;
    4. 将若干函数做了零星优化;
    5. III_decode()中流程改变;
    6. 一些Buffer做了调整,并将开在Stack中的大buffer移出;

    by Vincent Hsiung.
*/

#include "../include/audio_main.h"
#include "SysInclude.h"

#ifdef A_CORE_DECODE
#ifdef MP3_DEC_INCLUDE

#include "mp3_global.h"
#include "mp3_accelerator_hal.h"

#include <stdlib.h>
#include <string.h>

#ifdef HAVE_ASSERT_H
#   include <assert.h>
#endif

#define CHAR_BIT  8

# include "mp3_fixed.h"
# include "mp3_bit.h"
# include "mp3_stream.h"
# include "mp3_frame.h"
# include "mp3_huffman.h"
# include "mp3_layer3.h"

#include <stdio.h>

#define MAXLSHIFT   31

//#define SWITCH_ARM_ACC()	*((volatile unsigned long*)0x40000008) = (0x08 << 16)  | 0;
//#define SWITCH_ARM_NOR()	*((volatile unsigned long*)0x40000008) = (0x08 << 16)  | 0x08;

#define mad_f_mul2(x, y)	((((x) + (1L << 11)) >> 12) * (y))

//#define SWITCH_ARM_ACC()
//#define SWITCH_ARM_NOR()

_ATTR_MP3DEC_BSS_
static int cycle_0,cycle_1,cycle_reload;

_ATTR_MP3DEC_TEXT_
static int GetCycleInit()
{
	cycle_0 = *(unsigned int *)0xe000e018;
	cycle_reload = *(unsigned int *)0xe000e014;
}

_ATTR_MP3DEC_TEXT_
static int GetCycleDelta()
{
	int delta;

	cycle_1 = *(unsigned int *)0xe000e018;

	if (cycle_1 < cycle_0)
	{
		delta = (cycle_0 - cycle_1);
	}
	else
	{
		delta = (cycle_0 + cycle_reload - cycle_1);
	}

	return delta;
}


//overlap用buffer
_ATTR_MP3DEC_BSS_
unsigned long frame_overlap_buff[2 * 32 * 18];

extern unsigned char const nsfb_table[6][3][4] ;

extern struct {
  unsigned char slen1;
  unsigned char slen2;
} const sflen_table[16] ;

extern unsigned char const pretab[22] ;

//#define FLASH_ROM
#define CALC
#ifdef FLASH_ROM
extern
struct fixedfloat {
  unsigned long mantissa  : 27;
  unsigned short exponent :  5;
} const rq_table[8072] ;

extern
struct fixedfloat const rq_table_flash[5135] ;
#endif

#ifdef CALC
extern
struct fixedfloat {
  unsigned long mantissa  : 27;
  unsigned short exponent :  5;
}  rq_table_6159[6159] ;
extern
struct fixedfloat  rq_table_2048[2048] ;

#else
extern
struct fixedfloat {
  unsigned long mantissa  : 27;
  unsigned short exponent :  5;
} const rq_table[8072] ;


#endif

/*
extern
struct fixedfloat {
  unsigned long mantissa  : 27;
  unsigned short exponent :  5;
} const rq_table[8207] ;
*/

extern mad_fixed_t const root_table[7] ;
extern mad_fixed_t const cs[8] ;
extern mad_fixed_t const ca[8] ;
extern mad_fixed_t const imdct_s[6][6] ;
extern mad_fixed_t const window_l[36] ;
extern mad_fixed_t const window_s[12] ;
extern mad_fixed_t const is_lsf_table[2][15] ;
extern mad_fixed_t const is_table[7] ;

extern struct {
  unsigned char const *l;
  unsigned char const *s;
  unsigned char const *m;
} const sfbwidth_table[9] ;

/* --- Layer III ----------------------------------------------------------- */
_ATTR_MP3DEC_DATA_
enum {
  count1table_select = 0x01,
  scalefac_scale     = 0x02,
  preflag        	 = 0x04,
  mixed_block_flag   = 0x08
};

_ATTR_MP3DEC_DATA_
enum {
  I_STEREO  = 0x1,
  MS_STEREO = 0x2
};

_ATTR_MP3DEC_BSS_
struct sideinfo {
  unsigned int main_data_begin;
  unsigned int private_bits;

  unsigned char scfsi[2];

  struct granule {
    struct channel {
      /* from side info */
      unsigned short part2_3_length;
      unsigned short big_values;
      unsigned short global_gain;
      unsigned short scalefac_compress;

      unsigned char flags;
      unsigned char block_type;
      unsigned char table_select[3];
      unsigned char subblock_gain[3];
      unsigned char region0_count;
      unsigned char region1_count;

      /* from main_data */
      unsigned char scalefac[39];   /* scalefac_l and/or scalefac_s */
    } ch[2];
  } gr[2];
};

//--------------------------------------------------------------------
extern volatile int imdct_finish ;      //imdct36运算及结果搬运结束标志
extern volatile int synth_hw_busy;      //硬件子带合成模块忙标志

//使用硬件模块计算imdct36,与ARM并行流水运算
_ATTR_MP3DEC_TEXT_
void imdct36_hw_pipeline(long *X, long* z)
{
    imdct36_auto_output_addr = (int)z;

	if (!DMA1ToImdct((long)X))	//dma1 is busy, use memcpy
	{
		//memcpy((void *)IMDCT_BASEADDR,X,sizeof(int)*18);
		long *p = (long *)IMDCT_BASEADDR;
		p[0] = X[0];
		p[1] = X[1];
		p[2] = X[2];
		p[3] = X[3];
		p[4] = X[4];
		p[5] = X[5];
		p[6] = X[6];
		p[7] = X[7];
		p[8] = X[8];
		p[9] = X[9];
		p[10] = X[10];
		p[11] = X[11];
		p[12] = X[12];
		p[13] = X[13];
		p[14] = X[14];
		p[15] = X[15];
		p[16] = X[16];
		p[17] = X[17];
	}
}

/*
 * NAME:    III_sideinfo()
 * DESCRIPTION: decode frame side information from a bitstream
 */
_ATTR_MP3DEC_TEXT_
static
enum mad_error III_sideinfo(struct mad_bitptr *ptr, unsigned int nch,
                int lsf, struct sideinfo *si,
                unsigned int *data_bitlen,
                unsigned int *priv_bitlen)
{
  unsigned int ngr, gr, ch, i;
  enum mad_error result = MAD_ERROR_NONE;

  *data_bitlen = 0;
  *priv_bitlen = lsf ? ((nch == 1) ? 1 : 2) : ((nch == 1) ? 5 : 3);

  si->main_data_begin = mad_bit_read(ptr, lsf ? 8 : 9);
  si->private_bits    = mad_bit_read(ptr, *priv_bitlen);

  ngr = 1;
  if (!lsf) {
    ngr = 2;

    for (ch = 0; ch < nch; ++ch)
      si->scfsi[ch] = (unsigned char)mad_bit_read(ptr, 4);
  }

  for (gr = 0; gr < ngr; ++gr) {
    struct granule *granule = &si->gr[gr];

    for (ch = 0; ch < nch; ++ch) {
      struct channel *channel = &granule->ch[ch];

      channel->part2_3_length    = (unsigned short)mad_bit_read(ptr, 12);
      channel->big_values        = (unsigned short)mad_bit_read(ptr, 9);
      channel->global_gain       = (unsigned short)mad_bit_read(ptr, 8);
      channel->scalefac_compress = (unsigned short)mad_bit_read(ptr, lsf ? 9 : 4);

      *data_bitlen += channel->part2_3_length;

      if (channel->big_values > 288 && result == 0)
    result = MAD_ERROR_BADBIGVALUES;

      channel->flags = 0;

      /* window_switching_flag */
      if (mad_bit_read(ptr, 1)) {
        channel->block_type = (unsigned char)mad_bit_read(ptr, 2);

        if (channel->block_type == 0 && result == 0)
          result = MAD_ERROR_BADBLOCKTYPE;

        if (!lsf && channel->block_type == 2 && si->scfsi[ch] && result == 0)
          result = MAD_ERROR_BADSCFSI;

        channel->region0_count = 7;
        channel->region1_count = 36;

        if (mad_bit_read(ptr, 1))
          channel->flags |= mixed_block_flag;
        else if (channel->block_type == 2)
          channel->region0_count = 8;

        for (i = 0; i < 2; ++i)
          channel->table_select[i] = (unsigned char)mad_bit_read(ptr, 5);

    # if defined(DEBUG)
        channel->table_select[2] = 4;  /* not used */
    # endif

        for (i = 0; i < 3; ++i)
          channel->subblock_gain[i] = (unsigned char)mad_bit_read(ptr, 3);
      }
      else {
        channel->block_type = 0;

        for (i = 0; i < 3; ++i)
          channel->table_select[i] = (unsigned char)mad_bit_read(ptr, 5);

        channel->region0_count = (unsigned char)mad_bit_read(ptr, 4);
        channel->region1_count = (unsigned char)mad_bit_read(ptr, 3);
      }

      /* [preflag,] scalefac_scale, count1table_select */
      channel->flags |= (unsigned char)mad_bit_read(ptr, lsf ? 2 : 3);
    }
  }

  return result;
}

/*
 * NAME:    III_scalefactors_lsf()
 * DESCRIPTION: decode channel scalefactors for LSF from a bitstream
 */
_ATTR_MP3DEC_TEXT_
static
unsigned int III_scalefactors_lsf(struct mad_bitptr *ptr,
                  struct channel *channel,
                  struct channel *gr1ch, int mode_extension)
{
  struct mad_bitptr start;
  unsigned int scalefac_compress, index, slen[4], part, n, i;
  unsigned char const *nsfb;

  start = *ptr;

  scalefac_compress = channel->scalefac_compress;
  index = (channel->block_type == 2) ?
    ((channel->flags & mixed_block_flag) ? 2 : 1) : 0;

  if (!((mode_extension & I_STEREO) && gr1ch)) {
    if (scalefac_compress < 400) {
      slen[0] = (scalefac_compress >> 4) / 5;
      slen[1] = (scalefac_compress >> 4) % 5;
      slen[2] = (scalefac_compress % 16) >> 2;
      slen[3] =  scalefac_compress %  4;

      nsfb = nsfb_table[0][index];
    }
    else if (scalefac_compress < 500) {
      scalefac_compress -= 400;

      slen[0] = (scalefac_compress >> 2) / 5;
      slen[1] = (scalefac_compress >> 2) % 5;
      slen[2] =  scalefac_compress %  4;
      slen[3] = 0;

      nsfb = nsfb_table[1][index];
    }
    else {
      scalefac_compress -= 500;

      slen[0] = scalefac_compress / 3;
      slen[1] = scalefac_compress % 3;
      slen[2] = 0;
      slen[3] = 0;

      channel->flags |= preflag;

      nsfb = nsfb_table[2][index];
    }

    n = 0;
    for (part = 0; part < 4; ++part) {
      for (i = 0; i < nsfb[part]; ++i)
    channel->scalefac[n++] = mad_bit_read(ptr, slen[part]);
    }

    while (n < 39)
      channel->scalefac[n++] = 0;
  }
  else {  /* (mode_extension & I_STEREO) && gr1ch (i.e. ch == 1) */
    scalefac_compress >>= 1;

    if (scalefac_compress < 180) {
      slen[0] =  scalefac_compress / 36;
      slen[1] = (scalefac_compress % 36) / 6;
      slen[2] = (scalefac_compress % 36) % 6;
      slen[3] = 0;

      nsfb = nsfb_table[3][index];
    }
    else if (scalefac_compress < 244) {
      scalefac_compress -= 180;

      slen[0] = (scalefac_compress % 64) >> 4;
      slen[1] = (scalefac_compress % 16) >> 2;
      slen[2] =  scalefac_compress %  4;
      slen[3] = 0;

      nsfb = nsfb_table[4][index];
    }
    else {
      scalefac_compress -= 244;

      slen[0] = scalefac_compress / 3;
      slen[1] = scalefac_compress % 3;
      slen[2] = 0;
      slen[3] = 0;

      nsfb = nsfb_table[5][index];
    }

    n = 0;
    for (part = 0; part < 4; ++part) {
      unsigned int max, is_pos;

      max = (1 << slen[part]) - 1;

      for (i = 0; i < nsfb[part]; ++i) {
    is_pos = mad_bit_read(ptr, slen[part]);

    channel->scalefac[n] = is_pos;
    gr1ch->scalefac[n++] = (is_pos == max);
      }
    }

    while (n < 39) {
      channel->scalefac[n] = 0;
      gr1ch->scalefac[n++] = 0;  /* apparently not illegal */
    }
  }

  return mad_bit_length(&start, ptr);
}

/*
 * NAME:    III_scalefactors()
 * DESCRIPTION: decode channel scalefactors of one granule from a bitstream
 */
_ATTR_MP3DEC_TEXT_
static
unsigned int III_scalefactors(struct mad_bitptr *ptr, struct channel *channel,
                  struct channel const *gr0ch, unsigned int scfsi)
{
  struct mad_bitptr start;
  unsigned int slen1, slen2, sfbi;

  start = *ptr;

  slen1 = sflen_table[channel->scalefac_compress].slen1;
  slen2 = sflen_table[channel->scalefac_compress].slen2;

  if (channel->block_type == 2) {
    unsigned int nsfb;

    sfbi = 0;

    nsfb = (channel->flags & mixed_block_flag) ? 8 + 3 * 3 : 6 * 3;
    while (nsfb--)
      channel->scalefac[sfbi++] = mad_bit_read(ptr, slen1);

    nsfb = 6 * 3;
    while (nsfb--)
      channel->scalefac[sfbi++] = mad_bit_read(ptr, slen2);

    nsfb = 1 * 3;
    while (nsfb--)
      channel->scalefac[sfbi++] = 0;
  }
  else {  /* channel->block_type != 2 */
    if (scfsi & 0x8) {
      for (sfbi = 0; sfbi < 6; ++sfbi)
    channel->scalefac[sfbi] = gr0ch->scalefac[sfbi];
    }
    else {
      for (sfbi = 0; sfbi < 6; ++sfbi)
    channel->scalefac[sfbi] = mad_bit_read(ptr, slen1);
    }

    if (scfsi & 0x4) {
      for (sfbi = 6; sfbi < 11; ++sfbi)
    channel->scalefac[sfbi] = gr0ch->scalefac[sfbi];
    }
    else {
      for (sfbi = 6; sfbi < 11; ++sfbi)
    channel->scalefac[sfbi] = mad_bit_read(ptr, slen1);
    }

    if (scfsi & 0x2) {
      for (sfbi = 11; sfbi < 16; ++sfbi)
    channel->scalefac[sfbi] = gr0ch->scalefac[sfbi];
    }
    else {
      for (sfbi = 11; sfbi < 16; ++sfbi)
    channel->scalefac[sfbi] = mad_bit_read(ptr, slen2);
    }

    if (scfsi & 0x1) {
      for (sfbi = 16; sfbi < 21; ++sfbi)
    channel->scalefac[sfbi] = gr0ch->scalefac[sfbi];
    }
    else {
      for (sfbi = 16; sfbi < 21; ++sfbi)
    channel->scalefac[sfbi] = mad_bit_read(ptr, slen2);
    }

    channel->scalefac[21] = 0;
  }

  return mad_bit_length(&start, ptr);
}

/*
 * The Layer III formula for requantization and scaling is defined by
 * section 2.4.3.4.7.1 of ISO/IEC 11172-3, as follows:
 *
 *   long blocks:
 *   xr[i] = sign(is[i]) * abs(is[i])^(4/3) *
 *           2^((1/4) * (global_gain - 210)) *
 *           2^-(scalefac_multiplier *
 *               (scalefac_l[sfb] + preflag * pretab[sfb]))
 *
 *   short blocks:
 *   xr[i] = sign(is[i]) * abs(is[i])^(4/3) *
 *           2^((1/4) * (global_gain - 210 - 8 * subblock_gain[w])) *
 *           2^-(scalefac_multiplier * scalefac_s[sfb][w])
 *
 *   where:
 *   scalefac_multiplier = (scalefac_scale + 1) / 2
 *
 * The routines III_exponents() and III_requantize() facilitate this
 * calculation.
 */

/*
 * NAME:    III_exponents()
 * DESCRIPTION: calculate scalefactor exponents
 */
_ATTR_MP3DEC_TEXT_
static
void III_exponents(struct channel const *channel,
           unsigned char const *sfbwidth, signed int exponents[39])
{
  signed int gain;
  unsigned int scalefac_multiplier, sfbi;

  gain = (signed int) channel->global_gain - 210;
  scalefac_multiplier = (channel->flags & scalefac_scale) ? 2 : 1;

  if (channel->block_type == 2) {
    unsigned int l;
    signed int gain0, gain1, gain2;

    sfbi = l = 0;

    if (channel->flags & mixed_block_flag) {
      unsigned int premask;

      premask = (channel->flags & preflag) ? ~0 : 0;

      /* long block subbands 0-1 */

      while (l < 36) {
    exponents[sfbi] = gain -
      (signed int) ((channel->scalefac[sfbi] + (pretab[sfbi] & premask)) <<
            scalefac_multiplier);

    l += sfbwidth[sfbi++];
      }
    }

    /* this is probably wrong for 8000 Hz short/mixed blocks */

    gain0 = gain - 8 * (signed int) channel->subblock_gain[0];
    gain1 = gain - 8 * (signed int) channel->subblock_gain[1];
    gain2 = gain - 8 * (signed int) channel->subblock_gain[2];

    while (l < 576) {
      exponents[sfbi + 0] = gain0 -
    (signed int) (channel->scalefac[sfbi + 0] << scalefac_multiplier);
      exponents[sfbi + 1] = gain1 -
    (signed int) (channel->scalefac[sfbi + 1] << scalefac_multiplier);
      exponents[sfbi + 2] = gain2 -
    (signed int) (channel->scalefac[sfbi + 2] << scalefac_multiplier);

      l    += 3 * sfbwidth[sfbi];
      sfbi += 3;
    }
  }
  else {  /* channel->block_type != 2 */
    if (channel->flags & preflag) {
      for (sfbi = 0; sfbi < 22; ++sfbi) {
    exponents[sfbi] = gain -
      (signed int) ((channel->scalefac[sfbi] + pretab[sfbi]) <<
            scalefac_multiplier);
      }
    }
    else {
      for (sfbi = 0; sfbi < 22; ++sfbi) {
    exponents[sfbi] = gain -
      (signed int) (channel->scalefac[sfbi] << scalefac_multiplier);
      }
    }
  }
}

/*
 * NAME:    III_requantize()
 * DESCRIPTION: requantize one (positive) value
 */
















//#define TEST1
 //#define TEST2
  extern unsigned long long  rangetest[16];
  extern unsigned long long  totalnum;
  extern unsigned long long  temp[16];
	/*
	extern unsigned long long min[16];
	extern unsigned long long max[16];
	extern unsigned long long offset[16];
	extern int lastnum;
	extern int countnum;
	*/


#ifdef FLASH_ROM


#if 0
_ATTR_MP3DEC_TEXT_
struct fixedfloat const * getpower(unsigned int value)
{
	int hl;
	int index;
	struct fixedfloat const * t;
	static  unsigned long  temp=0;
	hl=value/1024;
	if(hl<3)
		return &rq_table[value];
	else
		{

		//	t=&rq_table[value];
			value-=3072;

			index=value%1024;
			/*
			if(lastnum!=hl)
				{
				lastnum=hl;
				countnum++;
				}
			if(index <min[hl])
				{
					min[hl]=index;
				}
			 if(index>max[hl])
			 	{
					max[hl]=index;
			 	}
			 	*/
			 ReadModuleData(MODULE_ID_MP3_FLASH , (unsigned char *)&temp , value*4 , 4 );
			//DisplayTestHexNum(0, 0,32,(unsigned long long )3);
			return (struct fixedfloat const *)(&temp);

		}


	return NULL;



}
#endif

_ATTR_MP3DEC_TEXT_
struct fixedfloat  * getpower(unsigned int value)
{
	unsigned int p,q,r;
	unsigned int P,Q,I;


	struct fixedfloat tf;
	static struct fixedfloat res;

	int e =11;
	unsigned int m;

	unsigned int rl;
	int i ;
	struct bijiao {
		int min;
		int max;
		}bijiao;
	struct bijiao cankao[9]= {{128,181},{182,304},{305,511},{512,861},{862,1448},{1449,2435},{2436,4095},
		{4096,6888},{6889,8191}};

	if(value >=0 && value <3072)
		return (struct fixedfloat  *) &rq_table[value];
	if(value >=8192)
		return (struct fixedfloat  *)&rq_table[value-8192+3072];
	if(value >=128 && value <1024)
		{
		p = value/8;
		q = p+1;
		r = value%8;

		tf = rq_table[p];
		rl = 	tf.mantissa>>(28-tf.exponent);

		P = rl<<4;

		tf = rq_table[q];
		rl = 	tf.mantissa>>(28-tf.exponent);

		Q = rl<<4;
		I = (P*(8-r)+Q*r)>>3;
	//DisplayTestHexNum(0, 0,32,(unsigned long long )1);
		}
	else
		{
		p = value/64;
		q = p+1;
		r = value%64;

		tf = rq_table[p];
		rl = 	tf.mantissa>>(28-tf.exponent);

		P = rl<<8;

		tf = rq_table[q];
		rl = 	tf.mantissa>>(28-tf.exponent);

		Q = rl<<8;
		I = (P*(64-r)+Q*r)>>6;
		DisplayTestHexNum(0, 0,32,(unsigned long long )value);

		}
		//tmp = I;
		for( i= 0;i<sizeof(cankao)/sizeof(struct bijiao);i++)
			{
			if(value >= cankao[i].min && value <= cankao[i].max)
				{
				e = e+ i;
				m = I <<(28-e);
				res.exponent  =e ;
				res.mantissa = m ;


				break;
				}


			}


		return &res;


}






#endif

#ifdef CALC
_ATTR_MP3DEC_TEXT_
struct fixedfloat  * getpower(unsigned int value)
{

           if(value<6159)
		   	return &rq_table_6159[value];
	    else
			return &rq_table_2048[value-6159];


}

#endif
_ATTR_MP3DEC_TEXT_

static
mad_fixed_t III_requantize(unsigned int value, signed int exp)
{
  mad_fixed_t requantized;
  int hl=0;
  signed int frac;
  struct fixedfloat const *power;

  frac = exp % 4;  /* assumes sign(frac) == sign(exp) */
  exp /= 4;

#ifdef TEST2
		hl=value/1024;
		if(hl>=8)
			{
				hl=7;
			}
		temp[hl]++;

#endif


//MODIFY BY helun
#ifdef FLASH_ROM
	power=getpower(value);
#else
	#ifdef CALC
	power = getpower( value);
	#else
  	power = &rq_table[value];
	#endif
#endif

  requantized = power->mantissa;
  exp += power->exponent;

  if (exp < 0) {
    if (-exp >= sizeof(mad_fixed_t) * CHAR_BIT) {
      /* underflow */
      requantized = 0;
    }
    else {
      requantized += 1L << (-exp - 1);
      requantized >>= -exp;
    }
  }
  else {
    if (exp >= 5) {
      /* overflow */
# if defined(DEBUG)
     //  DBG((TXT("requantize overflow (%f * 2^%d)\n"),
       //   mad_f_todouble(requantized), exp));
# endif
      requantized = MAD_F_MAX;
    }
    else
    {
            requantized <<= exp;

    }
      //requantized <<= exp;
  }

  //return frac ? mad_f_mul(requantized, root_table[3 + frac]) : requantized;
 // return frac ? (requantized>>12)*root_table[3 + frac] : requantized;
	{
		long long temp;
		temp = (long long)requantized*root_table[3 + frac];
		temp += (1<<11);
		return 	 frac ? (temp >> 12) : requantized;
	}
}

/* we must take care that sz >= bits and sz < sizeof(long) lest bits == 0 */
# if defined(CPU_ARM)
# define MASK(cache, sz, bits) \
    ({ unsigned long res; \
       asm ("mov  %0, #1\n\t" \
            "rsb  %0, %0, %0, lsl %3\n\t" \
            "and  %0, %0, %1, lsr %2" \
           : "=&r" (res) : "r" (cache), "r" ((sz) - (bits)), "r" (bits)); \
       res; \
    })
#else
# define MASK(cache, sz, bits)  \
    (((cache) >> ((sz) - (bits))) & ((1 << (bits)) - 1))
#endif
# define MASK1BIT(cache, sz)  \
    ((cache) & (1 << ((sz) - 1)))

/*
 * NAME:    III_huffdecode()
 * DESCRIPTION: decode Huffman code words of one channel of one granule
 */
_ATTR_MP3DEC_TEXT_
static
enum mad_error III_huffdecode(struct mad_bitptr *ptr, mad_fixed_t xrarr[576],
                  struct channel *channel,
                  unsigned char const *sfbwidth,
                  unsigned int part2_length)
{
  unsigned int bits;
  signed int exponents[39], exp;
  signed int const *expptr;
  struct mad_bitptr peek;
  signed int bits_left, cachesz;
  register mad_fixed_t *xr;
  mad_fixed_t const *sfbound;
  register unsigned long bitcache;

  bits_left = (signed) channel->part2_3_length - (signed) part2_length;
  if (bits_left < 0)
    return MAD_ERROR_BADPART3LEN;

  III_exponents(channel, sfbwidth, exponents);

  peek    = *ptr;
  cachesz = 0;
  sfbound = xr = xrarr;
  mad_bit_skip(ptr, bits_left);

  /* big_values */

/*-----------------------------------------------------*/

//if ((*((volatile unsigned long*)0x40000000) & 0x00000040)  == 0)
	SWITCH_ARM_ACC();

/*-----------------------------------------------------*/

  {
    int                    region;
    struct hufftable const *entry;
    union huffpair const   *table;
    unsigned int           linbits, startbits, rcount;
    mad_fixed_t            reqcache[16];
    mad_fixed_t const      *xr_end, *xr_big_val;

    rcount     = 1;
    expptr     = &exponents[0];
    region     = -1;
    exp        = 0x3210; /* start value */
    bitcache   = 0;
    linbits    = startbits = 0;
    table      = NULL;
    xr_big_val = xr + 2 * channel->big_values;

    while(xr < xr_big_val)
    {
      sfbound += *sfbwidth++;
      xr_end = sfbound > xr_big_val ? xr_big_val : sfbound;

      /* change table if region boundary */
      if(--rcount == 0)
      {
        if(exp == 0x3210)
            rcount = channel->region0_count + 1;
        else
          if(region == 0)
            rcount = channel->region1_count + 1;
      	  else
            rcount = 0;  /* all remaining */

		//DisplayTestHexNum(0,0,32,channel->table_select[1+region]);

		entry     = &mad_huff_pair_table[channel->table_select[++region]];
		table     = entry->table;
		linbits   = entry->linbits;
		startbits = entry->startbits;

	    if(table == 0)
	      return MAD_ERROR_BADHUFFTABLE;
      }

      if(exp != *expptr)
      {
	    exp = *expptr;
	    /* clear cache */
	    //memset(reqcache, 0, sizeof(reqcache));
		reqcache[ 0 ]	=	0;
		reqcache[ 1 ]	=	0;
		reqcache[ 2 ]	=	0;
		reqcache[ 3 ]	=	0;
		reqcache[ 4 ]	=	0;
		reqcache[ 5 ]	=	0;
		reqcache[ 6 ]	=	0;
		reqcache[ 7 ]	=	0;
		reqcache[ 8 ]	=	0;
		reqcache[ 9 ]	=	0;
		reqcache[10 ]	=	0;
		reqcache[11 ]	=	0;
		reqcache[12 ]	=	0;
		reqcache[13 ]	=	0;
		reqcache[14 ]	=	0;
		reqcache[15 ]	=	0;
      }

      ++expptr;

      if(linbits)
      {
        for( ; xr<xr_end; xr+=2)
        {
          union huffpair const *pair;
          register mad_fixed_t requantized;
          unsigned int clumpsz, value;

          /* maxhuffcode(hufftab16,hufftab24)=17bit + sign(x,y)=2bit */
          if(cachesz < 19)
          {
            if(cachesz < 0)
              return MAD_ERROR_BADHUFFDATA;  /* cache underrun */

            bits     = MAXLSHIFT - cachesz;
            bitcache = (bitcache << bits) | mad_bit_read(&peek, bits);
            cachesz += bits;
          }

          /* hcod (0..19) */
          clumpsz = startbits;
          pair    = &table[MASK(bitcache, cachesz, clumpsz)];

          while(!pair->final)
          {
            cachesz -= clumpsz;
            clumpsz = pair->ptr.bits;
            pair    = &table[pair->ptr.offset + MASK(bitcache, cachesz, clumpsz)];
          }

          cachesz -= pair->value.hlen;

      /* x (0..14) */
      value = pair->value.x;
      if(value == 0)
        xr[0] = 0;
          else
          {
            if(value == 15)
            {
              /* maxlinbits=13bit + sign(x,y)=2bit */
              if(cachesz < 15)
              {
                if(cachesz < 0)
                  return MAD_ERROR_BADHUFFDATA;  /* cache underrun */

                bits     = MAXLSHIFT - cachesz;
                bitcache = (bitcache << bits) | mad_bit_read(&peek, bits);
                cachesz += bits;
              }

          requantized = III_requantize(15+MASK(bitcache, cachesz, linbits), exp);
          cachesz    -= linbits;
            }
            else
            {
          if(reqcache[value])
            requantized = reqcache[value];
          else
            requantized = reqcache[value] = III_requantize(value, exp);
            }

            xr[0] = MASK1BIT(bitcache, cachesz--) ? -requantized : requantized;
          }

      /* y (0..14) */
      value = pair->value.y;
      if(value == 0)
        xr[1] = 0;
          else
          {
            if(value == 15)
            {
              /* maxlinbits=13bit + sign(y)=1bit */
              if(cachesz < 14)
              {
                if(cachesz < 0)
                  return MAD_ERROR_BADHUFFDATA;  /* cache underrun */

                bits     = MAXLSHIFT - cachesz;
                bitcache = (bitcache << bits) | mad_bit_read(&peek, bits);
                cachesz += bits;
              }

              requantized = III_requantize(15+MASK(bitcache, cachesz, linbits), exp);
              cachesz -= linbits;
            }
            else
            {
          if(reqcache[value])
            requantized = reqcache[value];
          else
            requantized = reqcache[value] = III_requantize(value, exp);
            }
        xr[1] = MASK1BIT(bitcache, cachesz--) ? -requantized : requantized;
          }
        }
      }
      else
      {
        for( ; xr<xr_end; xr+=2)
        {
          union huffpair const *pair;
          register mad_fixed_t requantized;
          unsigned int clumpsz, value;

          /* maxlookup=4bit + sign(x,y)=2bit */
          if(cachesz < 6)
          {
            if(cachesz < 0)
              return MAD_ERROR_BADHUFFDATA;  /* cache underrun */

            bits     = MAXLSHIFT - cachesz;
            bitcache = (bitcache << bits) | mad_bit_read(&peek, bits);
            cachesz += bits;
          }

          /* hcod (0..19) */
          clumpsz = startbits;
          pair    = &table[MASK(bitcache, cachesz, clumpsz)];

          while(!pair->final)
          {
            cachesz -= clumpsz;

            /* maxlookup=4bit + sign(x,y)=2bit */
            if(cachesz < 6)
            {
              if(cachesz < 0)
                return MAD_ERROR_BADHUFFDATA;  /* cache underrun */

              bits     = MAXLSHIFT - cachesz;
              bitcache = (bitcache << bits) | mad_bit_read(&peek, bits);
              cachesz += bits;
            }

            clumpsz = pair->ptr.bits;
            pair    = &table[pair->ptr.offset + MASK(bitcache, cachesz, clumpsz)];
          }

          cachesz -= pair->value.hlen;

      /* x (0..1) */
      value = pair->value.x;
      if(value == 0)
        xr[0] = 0;
      else
          {
	        if(reqcache[value])
	          requantized = reqcache[value];
	        else
	          requantized = reqcache[value] = III_requantize(value, exp);

	        xr[0] = MASK1BIT(bitcache, cachesz--) ? -requantized : requantized;
          }

      /* y (0..1) */
      value = pair->value.y;
      if(value == 0)
        xr[1] = 0;
      else
          {
	        if(reqcache[value])
	          requantized = reqcache[value];
	        else
	          requantized = reqcache[value] = III_requantize(value, exp);

	        xr[1] = MASK1BIT(bitcache, cachesz--) ? -requantized : requantized;
          }
        }
      }
    }
  }

/*-----------------------------------------------------*/
//if ((*((volatile unsigned long*)0x40000000) & 0x00000040)  == 0)
	SWITCH_ARM_NOR();
/*-----------------------------------------------------*/

  bits_left = ptr->readbit - peek.readbit;

  if(bits_left + cachesz < 0)
  {
  	bits_left = 0; /*by Vincent Hsiung : for some comstom require @ Nov 28, 2009 */
    //return MAD_ERROR_BADHUFFDATA;  /* big_values overrun */
  }

  /* count1 */
  {
    union huffquad const *table;
    register mad_fixed_t requantized;

    table = mad_huff_quad_table[channel->flags & count1table_select];

    requantized = III_requantize(1, exp);

    while(xr <= &xrarr[572] && bits_left + cachesz > 0)
    {
      union huffquad const *quad;

      /* hcod (1..6) */
      if(cachesz < 10)
      {
        if(cachesz < 0)
          return MAD_ERROR_BADHUFFDATA;  /* cache underrun */

		bits       = MAXLSHIFT - cachesz;
        bitcache   = (bitcache << bits) | mad_bit_read(&peek, bits);
		cachesz   += bits;
        bits_left -= bits;
      }

      quad = &table[MASK(bitcache, cachesz, 4)];

      /* quad tables guaranteed to have at most one extra lookup */
      if (!quad->final) {
		cachesz -= 4;

        quad = &table[quad->ptr.offset +
              MASK(bitcache, cachesz, quad->ptr.bits)];
      }

      cachesz -= quad->value.hlen;

      if (xr == sfbound) {
		sfbound += *sfbwidth++;

	    if (exp != *expptr) {
	      exp = *expptr;
	      requantized = III_requantize(1, exp);
	    }

	    ++expptr;
      }

      /* v (0..1) */
      xr[0] = quad->value.v ?
    	(MASK1BIT(bitcache, cachesz--) ? -requantized : requantized) : 0;

      /* w (0..1) */
      xr[1] = quad->value.w ?
    	(MASK1BIT(bitcache, cachesz--) ? -requantized : requantized) : 0;

      xr += 2;

      if (xr == sfbound) {
    	sfbound += *sfbwidth++;

	    if (exp != *expptr) {
	      exp = *expptr;
	      requantized = III_requantize(1, exp);
	    }

	    ++expptr;
      }

      /* x (0..1) */
      xr[0] = quad->value.x ?
    	(MASK1BIT(bitcache, cachesz--) ? -requantized : requantized) : 0;

      /* y (0..1) */
      xr[1] = quad->value.y ?
    	(MASK1BIT(bitcache, cachesz--) ? -requantized : requantized) : 0;

      xr += 2;
    }

    if(bits_left + cachesz < 0)
    {
      /* technically the bitstream is misformatted, but apparently
     some encoders are just a bit sloppy with stuffing bits */
      xr -= 4;
    }
  }

{
  int n = (char*)&xrarr[576] - (char*)xr;

if (n != 0)
{
  /* rzero */
  //memset(xr, 0, (char*)&xrarr[576] - (char*)xr);
  memset(xr, 0, n);
}
else
{
//if (n > 576/2)
/*-----------------------------------------------------*/
//if ((*((volatile unsigned long*)0x40000000) & 0x00000040)  == 0)
	SWITCH_ARM_NOR();
/*-----------------------------------------------------*/
}
}
  /*
  {
  	int n,m,i;
	n = (((char*)&xrarr[576] - (char*)xr)>>2);
	m = n & 0x3;
	n >>= 2;
	for (i = 0; i< n; i++)
	{
		xr[0] = 0;
		xr[1] = 0;
		xr[2] = 0;
		xr[3] = 0;
		xr += 4;
	}
	for (i = 0; i< m; i++)
	{
		xr[0] = 0;
		xr ++;
	}
  }
  */

  return MAD_ERROR_NONE;
}

# undef MASK
# undef MASK1BIT

/*
 * NAME:    III_reorder()
 * DESCRIPTION: reorder frequency lines of a short block into subband order
 */
_ATTR_MP3DEC_TEXT_
static
void III_reorder(mad_fixed_t xr[576], struct channel const *channel,
         unsigned char const sfbwidth[39])
{
  mad_fixed_t tmp[32][3][6];
  unsigned int sb, l, f, w, sbw[3], sw[3];

  /* this is probably wrong for 8000 Hz mixed blocks */

  sb = 0;
  if (channel->flags & mixed_block_flag) {
    sb = 2;

    l = 0;
    while (l < 36)
      l += *sfbwidth++;
  }

  for (w = 0; w < 3; ++w) {
    sbw[w] = sb;
    sw[w]  = 0;
  }

  f = *sfbwidth++;
  w = 0;

  for (l = 18 * sb; l < 576; ++l) {
    if (f-- == 0) {
      f = *sfbwidth++ - 1;
      w = (w + 1) % 3;
    }

    tmp[sbw[w]][w][sw[w]++] = xr[l];

    if (sw[w] == 6) {
      sw[w] = 0;
      ++sbw[w];
    }
  }

  memcpy(&xr[18 * sb], &tmp[sb], (576 - 18 * sb) * sizeof(mad_fixed_t));
}

/*
 * NAME:    III_stereo()
 * DESCRIPTION: perform joint stereo processing on a granule
 */
_ATTR_MP3DEC_TEXT_
static
enum mad_error III_stereo(mad_fixed_t xr[2][576],
              struct granule const *granule,
              struct mad_header *header,
              unsigned char const *sfbwidth)
{
  short modes[39];
  unsigned int sfbi, l, n, i;

  if (granule->ch[0].block_type !=
      granule->ch[1].block_type ||
      (granule->ch[0].flags & mixed_block_flag) !=
      (granule->ch[1].flags & mixed_block_flag))
    return MAD_ERROR_BADSTEREO;

  for (i = 0; i < 39; ++i)
    modes[i] = header->mode_extension;

  /* intensity stereo */

  if (header->mode_extension & I_STEREO) {
    struct channel const *right_ch = &granule->ch[1];
    mad_fixed_t const *right_xr = xr[1];
    unsigned int is_pos;

    header->flags |= MAD_FLAG_I_STEREO;

    /* first determine which scalefactor bands are to be processed */

    if (right_ch->block_type == 2) {
      unsigned int lower, start, max, bound[3], w;

      lower = start = max = bound[0] = bound[1] = bound[2] = 0;

      sfbi = l = 0;

      if (right_ch->flags & mixed_block_flag) {
    while (l < 36) {
      n = sfbwidth[sfbi++];

      for (i = 0; i < n; ++i) {
        if (right_xr[i]) {
          lower = sfbi;
          break;
        }
      }

      right_xr += n;
      l += n;
    }

    start = sfbi;
      }

      w = 0;
      while (l < 576) {
    n = sfbwidth[sfbi++];

    for (i = 0; i < n; ++i) {
      if (right_xr[i]) {
        max = bound[w] = sfbi;
        break;
      }
    }

    right_xr += n;
    l += n;
    w = (w + 1) % 3;
      }

      if (max)
    lower = start;

      /* long blocks */

      for (i = 0; i < lower; ++i)
    modes[i] = header->mode_extension & ~I_STEREO;

      /* short blocks */

      w = 0;
      for (i = start; i < max; ++i) {
    if (i < bound[w])
      modes[i] = header->mode_extension & ~I_STEREO;

    w = (w + 1) % 3;
      }
    }
    else {  /* right_ch->block_type != 2 */
      unsigned int bound;

      bound = 0;
      for (sfbi = l = 0; l < 576; l += n) {
    n = sfbwidth[sfbi++];

    for (i = 0; i < n; ++i) {
      if (right_xr[i]) {
        bound = sfbi;
        break;
      }
    }

    right_xr += n;
      }

      for (i = 0; i < bound; ++i)
    modes[i] = header->mode_extension & ~I_STEREO;
    }

    /* now do the actual processing */

    if (header->flags & MAD_FLAG_LSF_EXT) {
      unsigned char const *illegal_pos = granule[1].ch[1].scalefac;
      mad_fixed_t const *lsf_scale;

      /* intensity_scale */
      lsf_scale = is_lsf_table[right_ch->scalefac_compress & 0x1];

      for (sfbi = l = 0; l < 576; ++sfbi, l += n) {
    n = sfbwidth[sfbi];

    if (!(modes[sfbi] & I_STEREO))
      continue;

    if (illegal_pos[sfbi]) {
      modes[sfbi] &= ~I_STEREO;
      continue;
    }

    is_pos = right_ch->scalefac[sfbi];

    for (i = 0; i < n; ++i) {
      register mad_fixed_t left;

      left = xr[0][l + i];

      if (is_pos == 0)
        xr[1][l + i] = left;
      else {
        register mad_fixed_t opposite;

        //opposite = (left>>12)*lsf_scale[(is_pos - 1) / 2];//mad_f_mul(left, lsf_scale[(is_pos - 1) / 2]);
        opposite = mad_f_mul2(left, lsf_scale[(is_pos - 1) / 2]);

        if (is_pos & 1) {
          xr[0][l + i] = opposite;
          xr[1][l + i] = left;
        }
        else
          xr[1][l + i] = opposite;
      }
    }
      }
    }
    else {  /* !(header->flags & MAD_FLAG_LSF_EXT) */
      for (sfbi = l = 0; l < 576; ++sfbi, l += n) {
    n = sfbwidth[sfbi];

    if (!(modes[sfbi] & I_STEREO))
      continue;

    is_pos = right_ch->scalefac[sfbi];

    if (is_pos >= 7) {  /* illegal intensity position */
      modes[sfbi] &= ~I_STEREO;
      continue;
    }

    for (i = 0; i < n; ++i) {
      register mad_fixed_t left;

      left = xr[0][l + i];

      //xr[0][l + i] = (left>>12)*is_table[    is_pos];//mad_f_mul(left, is_table[    is_pos]);
      //xr[1][l + i] = (left>>12)*is_table[6 - is_pos];//mad_f_mul(left, is_table[6 - is_pos]);
      xr[0][l + i] = mad_f_mul2(left, is_table[    is_pos]);
	  xr[1][l + i] = mad_f_mul2(left, is_table[6 - is_pos]);

    }
      }
    }
  }

  /* middle/side stereo */

  if (header->mode_extension & MS_STEREO) {
    register mad_fixed_t invsqrt2;

    header->flags |= MAD_FLAG_MS_STEREO;

    invsqrt2 = root_table[3 + -2];

    for (sfbi = l = 0; l < 576; ++sfbi, l += n) {
      int tmpl = l - 1;
      n = sfbwidth[sfbi];

      if (modes[sfbi] != MS_STEREO)
    continue;

      //for (i = 0; i < n; ++i) {
      for (i = n ; i != 0; i--) {
    register mad_fixed_t m, s;

    //m = (xr[0][tmpl + i]>>12);
    //s = (xr[1][tmpl + i]>>12);
    //xr[0][tmpl + i] = ((m+s))*invsqrt2;//mad_f_mul(m + s, invsqrt2);  /* l = (m + s) / sqrt(2) */
    //xr[1][tmpl + i] = ((m-s))*invsqrt2;//mad_f_mul(m - s, invsqrt2);  /* r = (m - s) / sqrt(2) */

    m = (xr[0][tmpl + i]);
    s = (xr[1][tmpl + i]);
    xr[0][tmpl + i] = mad_f_mul2(m + s, invsqrt2);  /* l = (m + s) / sqrt(2) */
	xr[1][tmpl + i] = mad_f_mul2(m - s, invsqrt2);  /* r = (m - s) / sqrt(2) */

      }
    }
  }

  return MAD_ERROR_NONE;
}

/*
 * NAME:    III_aliasreduce()
 * DESCRIPTION: perform frequency line alias reduction
 */
_ATTR_MP3DEC_TEXT_
static
void III_aliasreduce(mad_fixed_t xr[576], int lines)
{
  mad_fixed_t const *bound;
  int i;

  bound = &xr[lines];
  for (xr += 18; xr < bound; xr += 18) {
    for (i = 0; i < 8; ++i) {
      register mad_fixed_t a, b;

	   mad_fixed64lo_t lo;

	  a = xr[-1 - i];
	  b = xr[     i] ;

if (a | b) {
		  long long temp;
		  temp = (long long )a * cs[i];
		  temp -= (long long)b * ca[i];
		  temp +=  1<<11;
		  lo = temp>>12;
      xr[-1 - i] = lo;

		  temp = (long long )b * cs[i];
		  temp += (long long)a * ca[i];
		  temp +=  1<<11;
		  lo = temp>>12;
      xr[     i] = lo;
}
    }
  }
}

/*
 * NAME:    III_imdct_l()
 * DESCRIPTION: perform IMDCT and windowing for long blocks
 */
//void imdct36(mad_fixed_t const X[18], mad_fixed_t x[36]);
_ATTR_MP3DEC_TEXT_
static
void III_imdct_l(mad_fixed_t const X[18], mad_fixed_t z[36],
         unsigned int block_type)
{
  unsigned int i;

  /* IMDCT */
  //move this function out of here, for DMA & HardAcc & ARM pipeline.
  //imdct36(X, z);

  /* windowing */

  switch (block_type) {
  case 0:  /* normal window */
    /*
    for (i =  0; i < 36; ++i)
        z[i] = z[i]*window_l[i];//mad_f_mul(z[i], window_l[i]);
    */
    z[ 0] *= window_l[ 0];
    z[ 1] *= window_l[ 1];
    z[ 2] *= window_l[ 2];
    z[ 3] *= window_l[ 3];
    z[ 4] *= window_l[ 4];
    z[ 5] *= window_l[ 5];
    z[ 6] *= window_l[ 6];
    z[ 7] *= window_l[ 7];
    z[ 8] *= window_l[ 8];
    z[ 9] *= window_l[ 9];
    z[10] *= window_l[10];
    z[11] *= window_l[11];
    z[12] *= window_l[12];
    z[13] *= window_l[13];
    z[14] *= window_l[14];
    z[15] *= window_l[15];
    z[16] *= window_l[16];
    z[17] *= window_l[17];
    z[18] *= window_l[18];
    z[19] *= window_l[19];
    z[20] *= window_l[20];
    z[21] *= window_l[21];
    z[22] *= window_l[22];
    z[23] *= window_l[23];
    z[24] *= window_l[24];
    z[25] *= window_l[25];
    z[26] *= window_l[26];
    z[27] *= window_l[27];
    z[28] *= window_l[28];
    z[29] *= window_l[29];
    z[30] *= window_l[30];
    z[31] *= window_l[31];
    z[32] *= window_l[32];
    z[33] *= window_l[33];
    z[34] *= window_l[34];
    z[35] *= window_l[35];

    break;

  case 1:  /* start block */
    /*
    for (i =  0; i < 18; i += 3) {
      z[i + 0] = mad_f_mul(z[i + 0], window_l[i + 0]);
      z[i + 1] = mad_f_mul(z[i + 1], window_l[i + 1]);
      z[i + 2] = mad_f_mul(z[i + 2], window_l[i + 2]);
    }*/
    for (i =  0; i < 18; i ++) {
      z[i] = z[i]* window_l[i + 0];
    }
    /*  (i = 18; i < 24; ++i) z[i] unchanged */
    for (i = 18; i < 24; i ++) {
      z[i] = (z[i]<<12);            //important!
    }
    for (i = 24; i < 30; ++i) z[i] = z[i] * window_s[i - 18];//mad_f_mul(z[i], window_s[i - 18]);
    for (i = 30; i < 36; ++i) z[i] = 0;
    break;

  case 3:  /* stop block */
    for (i =  0; i <  6; ++i) z[i] = 0;
    for (i =  6; i < 12; ++i) z[i] = z[i] * window_s[i - 6];//mad_f_mul(z[i], window_s[i - 6]);
    /*  (i = 12; i < 18; ++i) z[i] unchanged */
    for (i = 12; i < 18; i ++) {
      z[i] = (z[i]<<12);            //important!
    }
    /*
    for (i = 18; i < 36; i += 3) {
      z[i + 0] = mad_f_mul(z[i + 0], window_l[i + 0]);
      z[i + 1] = mad_f_mul(z[i + 1], window_l[i + 1]);
      z[i + 2] = mad_f_mul(z[i + 2], window_l[i + 2]);
    }
    */
    for (i = 18; i < 36; i ++) {
      z[i] = z[i + 0] * window_l[i + 0];
    }
    break;
  }
}


/*
 * NAME:    III_imdct_s()
 * DESCRIPTION: perform IMDCT and windowing for short blocks
 */
_ATTR_MP3DEC_TEXT_
static
void III_imdct_s(mad_fixed_t const X[18], mad_fixed_t z[36])
{
  mad_fixed_t y[36], *yptr;
  mad_fixed_t const *wptr;
  int w, i;
  //register mad_fixed64hi_t hi;
  register mad_fixed64lo_t lo;

  /* IMDCT */

  yptr = &y[0];

  for (w = 0; w < 3; ++w) {
    register mad_fixed_t const (*s)[6];

    s = imdct_s;

    for (i = 0; i < 3; ++i) {
      lo = (X[0]>>12) * (*s)[0]; //MAD_F_ML0(hi, lo, X[0], (*s)[0]);
      lo +=(X[1]>>12) * (*s)[1]; //MAD_F_MLA(hi, lo, X[1], (*s)[1]);
      lo +=(X[2]>>12) * (*s)[2]; //MAD_F_MLA(hi, lo, X[2], (*s)[2]);
      lo +=(X[3]>>12) * (*s)[3]; //MAD_F_MLA(hi, lo, X[3], (*s)[3]);
      lo +=(X[4]>>12) * (*s)[4]; //MAD_F_MLA(hi, lo, X[4], (*s)[4]);
      lo +=(X[5]>>12) * (*s)[5]; //MAD_F_MLA(hi, lo, X[5], (*s)[5]);

      yptr[i + 0] = lo ;         //MAD_F_MLZ(hi, lo);
      yptr[5 - i] = -yptr[i + 0];

      ++s;

      lo = (X[0]>>12) * (*s)[0]; //MAD_F_ML0(hi, lo, X[0], (*s)[0]);
      lo +=(X[1]>>12) * (*s)[1]; //MAD_F_MLA(hi, lo, X[1], (*s)[1]);
      lo +=(X[2]>>12) * (*s)[2]; //MAD_F_MLA(hi, lo, X[2], (*s)[2]);
      lo +=(X[3]>>12) * (*s)[3]; //MAD_F_MLA(hi, lo, X[3], (*s)[3]);
      lo +=(X[4]>>12) * (*s)[4]; //MAD_F_MLA(hi, lo, X[4], (*s)[4]);
      lo +=(X[5]>>12) * (*s)[5]; //MAD_F_MLA(hi, lo, X[5], (*s)[5]);

      yptr[ i + 6] = lo ;        //MAD_F_MLZ(hi, lo);
      yptr[11 - i] = yptr[i + 6];

      ++s;
    }

    yptr += 12;
    X    += 6;
  }

  /* windowing, overlapping and concatenation */

  yptr = &y[0];
  wptr = &window_s[0];

  for (i = 0; i < 6; ++i) {
    z[i +  0] = 0;
    z[i +  6] = (yptr[ 0 + 0]>>12) * wptr[0];//mad_f_mul(yptr[ 0 + 0], wptr[0]);

    //MAD_F_ML0(hi, lo, yptr[ 0 + 6], wptr[6]);
    lo = (yptr[ 0 + 6] >> 12 ) * wptr[6];
    //MAD_F_MLA(hi, lo, yptr[12 + 0], wptr[0]);
    lo += (yptr[12 + 0] >> 12 ) * wptr[0];

    z[i + 12] = lo;//MAD_F_MLZ(hi, lo);

    //MAD_F_ML0(hi, lo, yptr[12 + 6], wptr[6]);
    lo = (yptr[12 + 6] >> 12) * wptr[6];
    //MAD_F_MLA(hi, lo, yptr[24 + 0], wptr[0]);
	/* BUG FIX : by Vincent Hsiung , @ May 20 , 2009 . may cause noise . !!! */
    lo += (yptr[24 + 0] >> 12) * wptr[0];

    z[i + 18] = lo ;//MAD_F_MLZ(hi, lo);

    z[i + 24] = (yptr[24 + 6] >> 12) * wptr[6]; //mad_f_mul(yptr[24 + 6], wptr[6]);
    z[i + 30] = 0;

    ++yptr;
    ++wptr;
  }
}

/*
 * NAME:    III_overlap()
 * DESCRIPTION: perform overlap-add of windowed IMDCT outputs
 */
_ATTR_MP3DEC_TEXT_
static void III_overlap(mad_fixed_t const output[36], mad_fixed_t overlap[18],
         mad_fixed_t sample[18][32], unsigned int sb)
{
  /*
  unsigned int i;
  for (i = 0; i < 18; ++i) {
    sample[i][sb] = output[i +  0] + overlap[i];
    overlap[i]    = output[i + 18];
  */

  sample[  0 ][sb] = output[  0 ] + overlap[  0 ];
  sample[  1 ][sb] = output[  1 ] + overlap[  1 ];
  sample[  2 ][sb] = output[  2 ] + overlap[  2 ];
  sample[  3 ][sb] = output[  3 ] + overlap[  3 ];
  sample[  4 ][sb] = output[  4 ] + overlap[  4 ];
  sample[  5 ][sb] = output[  5 ] + overlap[  5 ];
  sample[  6 ][sb] = output[  6 ] + overlap[  6 ];
  sample[  7 ][sb] = output[  7 ] + overlap[  7 ];
  sample[  8 ][sb] = output[  8 ] + overlap[  8 ];
  sample[  9 ][sb] = output[  9 ] + overlap[  9 ];
  sample[ 10 ][sb] = output[ 10 ] + overlap[ 10 ];
  sample[ 11 ][sb] = output[ 11 ] + overlap[ 11 ];
  sample[ 12 ][sb] = output[ 12 ] + overlap[ 12 ];
  sample[ 13 ][sb] = output[ 13 ] + overlap[ 13 ];
  sample[ 14 ][sb] = output[ 14 ] + overlap[ 14 ];
  sample[ 15 ][sb] = output[ 15 ] + overlap[ 15 ];
  sample[ 16 ][sb] = output[ 16 ] + overlap[ 16 ];
  sample[ 17 ][sb] = output[ 17 ] + overlap[ 17 ];
  //memcpy(&overlap[0],&output[18],18*sizeof(mad_fixed_t));
  overlap[ 0] = output[18];
  overlap[ 1] = output[19];
  overlap[ 2] = output[20];
  overlap[ 3] = output[21];
  overlap[ 4] = output[22];
  overlap[ 5] = output[23];
  overlap[ 6] = output[24];
  overlap[ 7] = output[25];
  overlap[ 8] = output[26];
  overlap[ 9] = output[27];
  overlap[10] = output[28];
  overlap[11] = output[29];
  overlap[12] = output[30];
  overlap[13] = output[31];
  overlap[14] = output[32];
  overlap[15] = output[33];
  overlap[16] = output[34];
  overlap[17] = output[35];
}

/*
 * NAME:    III_overlap_z()
 * DESCRIPTION: perform "overlap-add" of zero IMDCT outputs
 */
_ATTR_MP3DEC_TEXT_
static __inline
void III_overlap_z(mad_fixed_t overlap[18],
           mad_fixed_t sample[18][32], unsigned int sb)
{
  /*
  unsigned int i;
  for (i = 0; i < 18; ++i) {
    sample[i][sb] = overlap[i];
    overlap[i]    = 0;
  }
  */
  sample[  0 ][sb] = overlap[  0 ] ;
  sample[  1 ][sb] = overlap[  1 ] ;
  sample[  2 ][sb] = overlap[  2 ] ;
  sample[  3 ][sb] = overlap[  3 ] ;
  sample[  4 ][sb] = overlap[  4 ] ;
  sample[  5 ][sb] = overlap[  5 ] ;
  sample[  6 ][sb] = overlap[  6 ] ;
  sample[  7 ][sb] = overlap[  7 ] ;
  sample[  8 ][sb] = overlap[  8 ] ;
  sample[  9 ][sb] = overlap[  9 ] ;
  sample[ 10 ][sb] = overlap[ 10 ] ;
  sample[ 11 ][sb] = overlap[ 11 ] ;
  sample[ 12 ][sb] = overlap[ 12 ] ;
  sample[ 13 ][sb] = overlap[ 13 ] ;
  sample[ 14 ][sb] = overlap[ 14 ] ;
  sample[ 15 ][sb] = overlap[ 15 ] ;
  sample[ 16 ][sb] = overlap[ 16 ] ;
  sample[ 17 ][sb] = overlap[ 17 ] ;
  //memset(&overlap[0],0,18*sizeof(mad_fixed_t));
  overlap[ 0] = 0;
  overlap[ 1] = 0;
  overlap[ 2] = 0;
  overlap[ 3] = 0;
  overlap[ 4] = 0;
  overlap[ 5] = 0;
  overlap[ 6] = 0;
  overlap[ 7] = 0;
  overlap[ 8] = 0;
  overlap[ 9] = 0;
  overlap[10] = 0;
  overlap[11] = 0;
  overlap[12] = 0;
  overlap[13] = 0;
  overlap[14] = 0;
  overlap[15] = 0;
  overlap[16] = 0;
  overlap[17] = 0;
}

/*
 * NAME:    III_freqinver()
 * DESCRIPTION: perform subband frequency inversion for odd sample lines
 */
_ATTR_MP3DEC_TEXT_
static void III_freqinver(mad_fixed_t sample[18][32], unsigned int sb)
{
  unsigned int i;

  for (i = 1; i < 18; i += 2)
    sample[i][sb] = -sample[i][sb];
}

/*
 * NAME:    III_decode()
 * DESCRIPTION: decode frame main_data
 */
_ATTR_MP3DEC_BSS_
static mad_fixed_t xr[2][576];
_ATTR_MP3DEC_BSS_
static mad_fixed_t output[36];

_ATTR_MP3DEC_TEXT_
static
enum mad_error III_decode(struct mad_bitptr *ptr, struct mad_frame *frame,
              struct sideinfo *si, unsigned int nch)
{
  struct mad_header *header = &frame->header;
  unsigned int sfreqi, ngr, gr;

	//if ((*((volatile unsigned long*)0x40000000) & 0x00000040)  == 0)
		SWITCH_ARM_ACC();

  {
    unsigned int sfreq;

    sfreq = header->samplerate;
    if (header->flags & MAD_FLAG_MPEG_2_5_EXT)
      sfreq *= 2;

    /* 48000 => 0, 44100 => 1, 32000 => 2,
       24000 => 3, 22050 => 4, 16000 => 5 */
    sfreqi = ((sfreq >>  7) & 0x000f) +
             ((sfreq >> 15) & 0x0001) - 8;

    if (header->flags & MAD_FLAG_MPEG_2_5_EXT)
      sfreqi += 3;
  }

  /* scalefactors, Huffman decoding, requantization */

  ngr = (header->flags & MAD_FLAG_LSF_EXT) ? 1 : 2;

  for (gr = 0; gr < ngr; ++gr) {
    struct granule *granule = &si->gr[gr];
    unsigned char const *sfbwidth[2];
    unsigned int ch;
    enum mad_error error;

	GetCycleInit();

    for (ch = 0; ch < nch; ++ch) {
      struct channel *channel = &granule->ch[ch];
      unsigned int part2_length;

      sfbwidth[ch] = sfbwidth_table[sfreqi].l;
      if (channel->block_type == 2) {
        sfbwidth[ch] = (channel->flags & mixed_block_flag) ?
        sfbwidth_table[sfreqi].m : sfbwidth_table[sfreqi].s;
      }

      if (header->flags & MAD_FLAG_LSF_EXT) {
        part2_length = III_scalefactors_lsf(ptr, channel,
                        ch == 0 ? 0 : &si->gr[1].ch[1],
                        header->mode_extension);
      }
      else {
             part2_length = III_scalefactors(ptr, channel, &si->gr[0].ch[ch],
             gr == 0 ? 0 : si->scfsi[ch]);
      }

      error = III_huffdecode(ptr, xr[ch], channel, sfbwidth[ch], part2_length);


	if (error)
    	return error;
    }

    /* joint stereo processing */

    if (header->mode == MAD_MODE_JOINT_STEREO && header->mode_extension) {
      error = III_stereo(xr, granule, header, sfbwidth[0]);
      if (error)
    return error;
    }

//if (GetCycleDelta() > 40000)
//if (GetCycleDelta() > 20000)
{
	//if ((*((volatile unsigned long*)0x40000000) & 0x00000040)  == 0)
		SWITCH_ARM_ACC();

}

    /* reordering, alias reduction, IMDCT, overlap-add, frequency inversion */
    for (ch = 0; ch < nch; ++ch) {
      struct channel const *channel = &granule->ch[ch];

#if 0
      mad_fixed_t (*sample)[32] = &(*frame->sbsample)[ch][18 * gr];
#else
      mad_fixed_t (*sample)[32] = &(*frame->sbsample)[0]; //By Vincent
#endif
      unsigned int sb, l, i, sblimit;

      if (channel->block_type == 2) {
        III_reorder(xr[ch], channel, sfbwidth[ch]);

    # if !defined(OPT_STRICT)
        /*
         * According to ISO/IEC 11172-3, "Alias reduction is not applied for
         * granules with block_type == 2 (short block)." However, other
         * sources suggest alias reduction should indeed be performed on the
         * lower two subbands of mixed blocks. Most other implementations do
         * this, so by default we will too.
         */
        if (channel->flags & mixed_block_flag)
          III_aliasreduce(xr[ch], 36);
    # endif
      }
      else
        III_aliasreduce(xr[ch], 576);

//----------------------------------------------------------------
        l = 0;

        //get the value : sblimit
        i = 576;
        while (i > 36 && xr[ch][i - 1] == 0)
            --i;
        sblimit = 32 - (576 - i) / 18;

        if (channel->block_type != 2)
        {
            //all long blocks
            /* long blocks */
            for (sb = 0; sb < sblimit + 1 ; ++sb, l += 18) {
                if (sb != 0)    // when first here, we don't have any data valid
                {
                    int sb_lastv = sb - 1;
                  // *(volatile long *)0xe000ed04 = (0x1<<27);

                    while(imdct_finish == 0);   //wait for hardware finish
					 // *(volatile long *)0xe000ed04 = (0x1<<28);
                    III_imdct_l(&xr[ch][l]/*this param is dummy*/, output, channel->block_type);
					III_overlap(output, (mad_fixed_t (*))&frame_overlap_buff[ch*32*18 + sb_lastv*18], sample, sb_lastv);

                    if (sb_lastv & 1)
                        III_freqinver(sample, sb_lastv);
                }
                else
                {
                    imdct_finish = 1;	//init
                }

                if (sb != sblimit)  // last here, the data is invalid , so don't let hardware do anything
                {
					while(imdct_finish == 0);   //wait for hardware free
					imdct_finish = 0;
                    imdct36_hw_pipeline(&xr[ch][l], output);
                }
            }
        }
        else
        {
            if (channel->flags & mixed_block_flag)
            {
            //混合块
                /* subbands 0-1 */
                /* short blocks */
                for (sb = 0; sb < 2; ++sb, l += 18) {
                    III_imdct_s(&xr[ch][l], output);
					III_overlap(output, (mad_fixed_t (*))&frame_overlap_buff[ch*32*18 + sb*18], sample, sb);
                }

                /* (nonzero) subbands 2-31 */
                /* long blocks */
                for (sb = 2; sb < sblimit + 1; ++sb, l += 18) {
                    if (sb != 2)    // when first here, we don't have any data valid
                    {
                        int sb_lastv = sb - 1;
                        while(imdct_finish == 0);   //wait for hardware finish
                        III_imdct_l(&xr[ch][l], output, channel->block_type);
						III_overlap(output, (mad_fixed_t (*))&frame_overlap_buff[ch*32*18 + sb_lastv*18], sample, sb_lastv);

                        if (sb_lastv & 1)
                            III_freqinver(sample, sb_lastv);
                    }
                    else
                    {
                        imdct_finish = 1;	//init
                    }

                    if (sb != sblimit)  // last here, the data is invalid , so don't let hardware do anything
                    {
                        while(imdct_finish == 0);   //wait for hardware free
                        imdct36_hw_pipeline(&xr[ch][l], output);
                    }
                }
            }
            else
            {
            //非混合块
                //all short
                /* short blocks */
                for (sb = 0; sb < sblimit; ++sb, l += 18) {
                    III_imdct_s(&xr[ch][l], output);
					III_overlap(output, (mad_fixed_t (*))&frame_overlap_buff[ch*32*18 + sb*18], sample, sb);

                    if (sb & 1)
                        III_freqinver(sample, sb);
                }
            }

       }
//----------------------------------------------------------------
      /* remaining (zero) subbands */

      for (sb = sblimit; sb < 32; ++sb) {
		III_overlap_z((mad_fixed_t (*))&frame_overlap_buff[ch*32*18 + sb*18], sample, sb);

        if (sb & 1)
          III_freqinver(sample, sb);
      }

      //使用硬件模块做子带合成
      {
        extern void mad_synth_fouth_frame(struct mad_frame const *frame, int whichch,int ns,int gr);
        while(synth_hw_busy==1);        //wait for hardware finish
		//子带数应该总使用18,无论普通的36时或mpeg2.5的18时.
        mad_synth_fouth_frame(frame , ch , 18 /*MAD_NSBSAMPLES(&frame->header)*/ , gr);
		frame_sbsample_toggle(frame);
      }

    }


  }

//if ((*((volatile unsigned long*)0x40000000) & 0x00000040)  == 0)
  	SWITCH_ARM_NOR();

  return MAD_ERROR_NONE;
}

/*
 * NAME:    layer->III()
 * DESCRIPTION: decode a single Layer III frame
 */
_ATTR_MP3DEC_TEXT_
int mad_layer_III(struct mad_stream *stream, struct mad_frame *frame)
{
  struct mad_header *header = &frame->header;
  unsigned int nch, priv_bitlen, next_md_begin = 0;
  unsigned int si_len, data_bitlen, md_len;
  unsigned int frame_space, frame_used, frame_free;
  struct mad_bitptr ptr;
  struct sideinfo si;
  enum mad_error error;
  int result = 0;

  /* allocate Layer III dynamic structures */

  if (stream->main_data == 0) {
    stream->main_data = &MainData;
    if (stream->main_data == 0) {
      stream->error = MAD_ERROR_NOMEM;
      return -1;
    }
  }

  if (frame->overlap == 0) {
    frame->overlap = (void*)frame_overlap_buff;
    if (frame->overlap == 0) {
      stream->error = MAD_ERROR_NOMEM;
      return -1;
    }
  }

  nch = MAD_NCHANNELS(header);
  si_len = (header->flags & MAD_FLAG_LSF_EXT) ?
    (nch == 1 ? 9 : 17) : (nch == 1 ? 17 : 32);

  /* check frame sanity */
  //stream->next_frame = 200097b8
  //mad_bit_nextbyte(&stream->ptr) = 200097bc
  if (stream->next_frame - mad_bit_nextbyte(&stream->ptr) <
      (signed int) si_len) {
    stream->error = MAD_ERROR_BADFRAMELEN;
    stream->md_len = 0;
    return -1;
  }

  /* check CRC word */

  if (header->flags & MAD_FLAG_PROTECTION) {
    header->crc_check =
      mad_bit_crc(stream->ptr, si_len * CHAR_BIT, header->crc_check);

    if (header->crc_check != header->crc_target &&
    !(frame->options & MAD_OPTION_IGNORECRC)) {
      stream->error = MAD_ERROR_BADCRC;
      result = -1;
    }
  }

  /* decode frame side information */

  error = III_sideinfo(&stream->ptr, nch, header->flags & MAD_FLAG_LSF_EXT,
               &si, &data_bitlen, &priv_bitlen);
  if (error && result == 0) {
    stream->error = error;
    result = -1;
  }

  header->flags        |= priv_bitlen;
  header->private_bits |= si.private_bits;

  /* find main_data of next frame */

  {
    struct mad_bitptr peek;
    unsigned long header;

    mad_bit_init(&peek, stream->next_frame);

    header = mad_bit_read(&peek, 32);
    if ((header & 0xffe60000L) /* syncword | layer */ == 0xffe20000L) {
      if (!(header & 0x00010000L))  /* protection_bit */
    mad_bit_skip(&peek, 16);  /* crc_check */

      next_md_begin =
    mad_bit_read(&peek, (header & 0x00080000L) /* ID */ ? 9 : 8);
    }

    mad_bit_finish(&peek);
  }

  /* find main_data of this frame */

  frame_space = stream->next_frame - mad_bit_nextbyte(&stream->ptr);

  if (next_md_begin > si.main_data_begin + frame_space)
    next_md_begin = 0;

  md_len = si.main_data_begin + frame_space - next_md_begin;

  frame_used = 0;

  if (si.main_data_begin == 0) {
    ptr = stream->ptr;
    stream->md_len = 0;

    frame_used = md_len;
  }
  else {
    if (si.main_data_begin > stream->md_len) {
      if (result == 0) {
    stream->error = MAD_ERROR_BADDATAPTR;
    result = -1;
      }
    }
    else {
      mad_bit_init(&ptr,
           *stream->main_data + stream->md_len - si.main_data_begin);

      if (md_len > si.main_data_begin) {
    assert(stream->md_len + md_len -
           si.main_data_begin <= MAD_BUFFER_MDLEN);

    memcpy(*stream->main_data + stream->md_len,
           mad_bit_nextbyte(&stream->ptr),
           frame_used = md_len - si.main_data_begin);
    stream->md_len += frame_used;
      }
    }
  }

  frame_free = frame_space - frame_used;

  /* decode main_data */
  if (result == 0) {
    error = III_decode(&ptr, frame, &si, nch);
    if (error) {
      stream->error = error;
      result = -1;
    }

    /* designate ancillary bits */
    stream->anc_ptr    = ptr;
    stream->anc_bitlen = md_len * CHAR_BIT - data_bitlen;
  }

# if 0 && defined(DEBUG)
  fprintf(stderr,
      "main_data_begin:%u, md_len:%u, frame_free:%u, "
      "data_bitlen:%u, anc_bitlen: %u\n",
      si.main_data_begin, md_len, frame_free,
      data_bitlen, stream->anc_bitlen);
# endif

  /* preload main_data buffer with up to 511 bytes for next frame(s) */

  if (frame_free >= next_md_begin) {
    memcpy(*stream->main_data,
       stream->next_frame - next_md_begin, next_md_begin);
    stream->md_len = next_md_begin;
  }
  else {
    if (md_len < si.main_data_begin) {
      unsigned int extra;

      extra = si.main_data_begin - md_len;
      if (extra + frame_free > next_md_begin)
    extra = next_md_begin - frame_free;

      if (extra < stream->md_len) {
    memmove(*stream->main_data,
        *stream->main_data + stream->md_len - extra, extra);
    stream->md_len = extra;
      }
    }
    else
      stream->md_len = 0;

        memcpy(*stream->main_data + stream->md_len,
           stream->next_frame - frame_free, frame_free);
    stream->md_len += frame_free;
  }

  return result;
}

#endif
#endif
