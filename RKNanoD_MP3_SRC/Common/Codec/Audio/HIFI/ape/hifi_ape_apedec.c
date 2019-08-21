#include "SysInclude.h"
#include <stdio.h>
#include "audio_main.h"
#include "hw_memap.h"
#include "hifi.h"
#include "Hw_hifi.h"
#include "ape.h"
#include "audio_file_access.h"

#ifdef A_CORE_DECODE
#ifdef HIFI_APE_DECODE
#pragma arm section code = "ApeHDecCode", rodata = "ApeHDecCode", rwdata = "ApeHDecData", zidata = "ApeHDecBss"

extern FILE *ape_file_handle;
APEContextdec sobj;
ByteCache dataobj;
extern APEContext apeobj;
extern ByteIOContext pbobj;
/*
 * Monkey's Audio lossless audio decoder
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
#define ALT_BITSTREAM_READER_LE

#define FFABS(a) ((a) >= 0 ? (a) : (-(a)))

/**
 * @file
 * Monkey's Audio lossless audio decoder
 */




/** Total size of all predictor histories */

#define YDELAYA (50)
#define YDELAYB (42)
#define XDELAYA (34)
#define XDELAYB (26)

#define YADAPTCOEFFSA 18
#define XADAPTCOEFFSA 14
#define YADAPTCOEFFSB 10
#define XADAPTCOEFFSB 5

/**
 * Possible compression levels
 * @{
 */
enum APECompressionLevel {
    COMPRESSION_LEVEL_FAST       = 1000,
    COMPRESSION_LEVEL_NORMAL     = 2000,
    COMPRESSION_LEVEL_HIGH       = 3000,
    COMPRESSION_LEVEL_EXTRA_HIGH = 4000,
    COMPRESSION_LEVEL_INSANE     = 5000
};
/** @} */


/** Filter orders depending on compression level */
 uint16_t ape_filter_orders[5][APE_FILTER_LEVELS] = {
    {  0,   0,    0 },
    { 16,   0,    0 },
    { 64,   0,    0 },
    { 32, 256,    0 },
    { 16, 256, 1280 }
};

/** Filter fraction (小数位数,即放大 2^fracbits)bits depending on compression level */
 uint8_t ape_filter_fracbits[5][APE_FILTER_LEVELS] = {
    {  0,  0,  0 },
    { 11,  0,  0 },
    { 11,  0,  0 },
    { 10, 13,  0 },
    { 11, 13, 15 }
};









/**
 * @defgroup rangecoder APE range decoder
 * @{
 */

#define CODE_BITS    32
#define TOP_VALUE    ((unsigned int)1 << (CODE_BITS-1))
#define SHIFT_BITS   (CODE_BITS - 9)
#define EXTRA_BITS   ((CODE_BITS-2) % 8 + 1)
#define BOTTOM_VALUE (TOP_VALUE >> 8)


#define MODEL_ELEMENTS 64

/**
 * Fixed probabilities for symbols in Monkey Audio version 3.97
 */
 uint16_t counts_3970[22] = {
        0, 14824, 28224, 39348, 47855, 53994, 58171, 60926,
    62682, 63786, 64463, 64878, 65126, 65276, 65365, 65419,
    65450, 65469, 65480, 65487, 65491, 65493,
};

/**
 * Probability ranges for symbols in Monkey Audio version 3.97
 */
 uint16_t counts_diff_3970[21] = {
    14824, 13400, 11124, 8507, 6139, 4177, 2755, 1756,
    1104, 677, 415, 248, 150, 89, 54, 31,
    19, 11, 7, 4, 2,
};

/**
 * Fixed probabilities for symbols in Monkey Audio version 3.98
 */
 uint16_t counts_3980[22] = {
        0, 19578, 36160, 48417, 56323, 60899, 63265, 64435,
    64971, 65232, 65351, 65416, 65447, 65466, 65476, 65482,
    65485, 65488, 65490, 65491, 65492, 65493,
};

/**
 * Probability ranges for symbols in Monkey Audio version 3.98
 */
 uint16_t counts_diff_3980[21] = {
    19578, 16582, 12257, 7906, 4576, 2366, 1170, 536,
    261, 119, 65, 31, 19, 10, 6, 3,
    3, 2, 1, 1, 1,
};

 uint32_t K_SUM_MIN_BOUNDARY[32] = // 2^(4+n)
 {0,32,64,128,256,512,1024,2048,
4096,8192,16384,32768,65536,131072,262144,524288,
1048576,2097152,4194304,8388608,16777216,33554432,67108864,134217728,
268435456,536870912,1073741824,2147483648,0,0,0,0};
/**
 * Decode symbol
 * @param ctx decoder context
 * @param counts probability range start position
 * @param counts_diff probability range widths 
 */

/*********************熵解码****************************************/
void init_entropy_decoder(APEContextdec * ctx,ByteIOContext *pb)
{
    ctx->CRC = fread32(&dataobj);
    ctx->frameflags = 0;
    if ((ctx->fileversion > 3820) && (ctx->CRC & 0x80000000)) 
    {
        ctx->CRC &= ~0x80000000;
        ctx->frameflags = fread32(&dataobj);
    }
    ctx->blocksdecoded = 0;
    ctx->riceX.k = 10;
    ctx->riceX.ksum = 16384;//(1 << ctx->riceX.k) * 16;
    ctx->riceY.k = 10;
    ctx->riceY.ksum = 16384;//(1 << ctx->riceY.k) * 16;
	fread8(&dataobj);
    ctx->rc.buffer = fread8(&dataobj);
    ctx->rc.low    = ctx->rc.buffer >> 1;
    ctx->rc.range  = (uint32_t) 1 << 7;
}
static const int32_t initial_coeffs[4] = {
    360, 317, -109, 98
};
void range_dec_normalize(APEContextdec * ctx)
{
    while (ctx->rc.range <= (1<<23))//BOTTOM_VALUE) 
	{
        ctx->rc.buffer <<= 8;
        if(ctx->ptr < ctx->data_end)
        {
           ctx->rc.buffer += fread8(&dataobj);
        }
        ctx->ptr++;
        ctx->rc.low    = (ctx->rc.low << 8)    | ((ctx->rc.buffer >> 1) & 0xFF);
        ctx->rc.range  <<= 8;
    }
}
int range_decode_bits(APEContextdec * ctx, int n)
{
    int sym;
	range_dec_normalize(ctx);
    ctx->rc.range = ctx->rc.range >> n;
    sym = ctx->rc.low / ctx->rc.range;
	ctx->rc.low  -= ctx->rc.range * sym;
    return sym;
}
int range_get_symbol(APEContextdec * ctx,
                                   const uint16_t counts[],
                                   const uint16_t counts_diff[])
{
    int symbol, cf;

	range_dec_normalize(ctx);
    ctx->rc.range = ctx->rc.range >> 16;
    cf = ctx->rc.low / ctx->rc.range;

    if(cf > 65492)
    {
        symbol= cf - 65535 + 63;
		ctx->rc.low  -= ctx->rc.range * cf;
        if(cf > 65535)
        {
            ctx->error = 1;
        }
        return symbol;
    }
    /* figure out the symbol inefficiently; a binary search would be much better */
    for (symbol = 0; counts[symbol + 1] <= cf; symbol++);

    ctx->rc.low  -= ctx->rc.range * counts[symbol];
    ctx->rc.range = ctx->rc.range * counts_diff[symbol];
	
    return symbol;
}



int ape_decode_value(APEContextdec * ctx, APERice *rice)
{
    int x, overflow;

    if (ctx->fileversion < 3990) 
	{
        int tmpk;

        overflow = range_get_symbol(ctx, counts_3970, counts_diff_3970);

        if (overflow == (MODEL_ELEMENTS - 1))
        {
            tmpk = range_decode_bits(ctx, 5);
            overflow = 0;
        } 
        else
        {
            tmpk = (rice->k < 1) ? 0 : rice->k - 1;
        }

        if (tmpk <= 16)
        {
            x = range_decode_bits(ctx, tmpk);
        }
        else 
        {
            x = range_decode_bits(ctx, 16);
            x |= (range_decode_bits(ctx, tmpk - 16) << 16);
        }
        x += overflow << tmpk;
    } 
	else 
	{
        int base, pivot;

        pivot = rice->ksum >> 5;
        if (pivot == 0)
        {
            pivot = 1;
        }

        overflow = range_get_symbol(ctx, counts_3980, counts_diff_3980);

        if (overflow == (MODEL_ELEMENTS - 1)) 
		{
            overflow  = range_decode_bits(ctx, 16) << 16;
            overflow |= range_decode_bits(ctx, 16);
        }

        if (pivot < 0x10000) 
		{
			range_dec_normalize(ctx);
			ctx->rc.range = ctx->rc.range / pivot;
			base = ctx->rc.low / ctx->rc.range;
			ctx->rc.low  -= ctx->rc.range * base;
        } 
		else 
		{
            int base_hi = pivot, base_lo;
            int bbits = 0;

            while (base_hi & ~0xFFFF) 
            {
                base_hi >>= 1;
                bbits++;
            }
			range_dec_normalize(ctx);
			ctx->rc.range = ctx->rc.range / (base_hi + 1);
			base_hi = ctx->rc.low / ctx->rc.range;
			ctx->rc.low  -= ctx->rc.range * base_hi;
			range_dec_normalize(ctx);
			ctx->rc.range = ctx->rc.range / (1 << bbits);
			base_lo = ctx->rc.low / ctx->rc.range;
			ctx->rc.low  -= ctx->rc.range * base_lo;
			base = (base_hi << bbits) + base_lo;
        }

        x = base + overflow * pivot;
    }

	rice->ksum += ((x + 1) / 2) - ((rice->ksum + 16) >> 5);

	if (rice->ksum < K_SUM_MIN_BOUNDARY[rice->k])
    {
	    rice->k--;
    }
	else if (rice->ksum >= K_SUM_MIN_BOUNDARY[rice->k + 1])
    {
	    rice->k++;
    }

    /* Convert to signed */
    if (x & 1)
    {
        return (x >> 1) + 1;
    }
    else
    {
        return -(x >> 1);
    }

}

static void entropy_decode(APEContextdec * ctx, int blockstodecode, int stereo)
{
    int32_t *decoded0 = ctx->decoded0;
    int32_t *decoded1 = ctx->decoded1;

    ctx->blocksdecoded = blockstodecode;

    if ((ctx->frameflags & APE_FRAMECODE_STEREO_SILENCE) == APE_FRAMECODE_STEREO_SILENCE) {
        /* We are pure silence, just memset the output buffer. */
        Hifi_Ape_MemSet(decoded0, 0, blockstodecode * sizeof(int32_t));
        Hifi_Ape_MemSet(decoded1, 0, blockstodecode * sizeof(int32_t));
    }
    else 
    {
        if(stereo)
        {

            while (blockstodecode--) 
            {
                *decoded0++ = ape_decode_value(ctx, &ctx->riceY);
                *decoded1++ = ape_decode_value(ctx, &ctx->riceX);
            }
        }
        else
        {
            while (blockstodecode--) 
            {
                *decoded0++ = ape_decode_value(ctx, &ctx->riceY);            
            }
        }
    }

    if (ctx->blocksdecoded == ctx->currentframeblocks)
        range_dec_normalize(ctx);   /* normalize to use up all bytes */
}

/*********************预测滤波****************************************/


void init_predictor_decoder(APEContextdec * ctx)
{
    APEPredictor *p = &ctx->predictor;

    /* Zero the history buffers */
    Hifi_Ape_MemSet(p->historybuffer, 0, PREDICTOR_SIZE * sizeof(int32_t));
    p->buf = p->historybuffer;

    /* Initialize and zero the coefficients */
    Hifi_Ape_Memcpy(p->coeffsA[0], initial_coeffs, sizeof(initial_coeffs));
    Hifi_Ape_Memcpy(p->coeffsA[1], initial_coeffs, sizeof(initial_coeffs));
    Hifi_Ape_MemSet(p->coeffsB, 0, sizeof(p->coeffsB));

    p->filterA[0] = p->filterA[1] = 0;
    p->filterB[0] = p->filterB[1] = 0;
    p->lastA[0]   = p->lastA[1]   = 0;
}

/** Get inverse sign of integer (-1 for positive, 1 for negative and 0 for zero) */
int APESIGN(int32_t x) 
{
    return (x < 0) - (x > 0);
}

 int predictor_update_filter(APEPredictor *p, const int decoded, const int filter,int32_t *buf,const int delayA, const int delayB, const int adaptA, const int adaptB)
  {
    int32_t predictionA, predictionB, sign;

    buf[delayA]     = p->lastA[filter];
    buf[adaptA]     = APESIGN(buf[delayA]);
    buf[delayA - 1] = buf[delayA] - buf[delayA - 1];
    buf[adaptA - 1] = APESIGN(buf[delayA - 1]);

    predictionA = buf[delayA    ] * p->coeffsA[filter][0] +
                  buf[delayA - 1] * p->coeffsA[filter][1] +
                  buf[delayA - 2] * p->coeffsA[filter][2] +
                  buf[delayA - 3] * p->coeffsA[filter][3];

    /*  Apply a scaled first-order filter compression */
    buf[delayB]     = p->filterA[filter ^ 1] - ((p->filterB[filter] * 31) >> 5);
    buf[adaptB]     = APESIGN(buf[delayB]);
    buf[delayB - 1] = buf[delayB] - buf[delayB - 1];
    buf[adaptB - 1] = APESIGN(buf[delayB - 1]);
    p->filterB[filter] = p->filterA[filter ^ 1];

    predictionB = buf[delayB    ] * p->coeffsB[filter][0] +
                  buf[delayB - 1] * p->coeffsB[filter][1] +
                  buf[delayB - 2] * p->coeffsB[filter][2] +
                  buf[delayB - 3] * p->coeffsB[filter][3] +
                  buf[delayB - 4] * p->coeffsB[filter][4];

    p->lastA[filter] = decoded + ((predictionA + (predictionB >> 1)) >> 10);
    p->filterA[filter] = p->lastA[filter] + ((p->filterA[filter] * 31) >> 5);

    sign = APESIGN(decoded);
    p->coeffsA[filter][0] += buf[adaptA    ] * sign;
    p->coeffsA[filter][1] += buf[adaptA - 1] * sign;
    p->coeffsA[filter][2] += buf[adaptA - 2] * sign;
    p->coeffsA[filter][3] += buf[adaptA - 3] * sign;
    p->coeffsB[filter][0] += buf[adaptB    ] * sign;
    p->coeffsB[filter][1] += buf[adaptB - 1] * sign;
    p->coeffsB[filter][2] += buf[adaptB - 2] * sign;
    p->coeffsB[filter][3] += buf[adaptB - 3] * sign;
    p->coeffsB[filter][4] += buf[adaptB - 4] * sign;

    return p->filterA[filter];
}


void predictor_decode_stereo(APEContextdec * ctx, int count)
{
    APEPredictor *p = &ctx->predictor;
    int32_t *decoded0 = ctx->decoded0;
    int32_t *decoded1 = ctx->decoded1;
   	int i,j;
    int32_t buf[BLOCKS_PER_LOOP+PREDICTOR_SIZE+1];
    Hifi_Ape_Memcpy(buf,p->buf,sizeof(int32_t)*(PREDICTOR_SIZE+1));    
   
    for(j=0;j<count;j++)
	{
        *decoded0 = predictor_update_filter(p, *decoded0, 0,&buf[j], YDELAYA, YDELAYB, YADAPTCOEFFSA, YADAPTCOEFFSB);
        decoded0++;
        *decoded1 = predictor_update_filter(p, *decoded1, 1,&buf[j], XDELAYA, XDELAYB, XADAPTCOEFFSA, XADAPTCOEFFSB);
        decoded1++;
       
    }
    Hifi_Ape_Memcpy(p->buf,&buf[count],(PREDICTOR_SIZE+1)*4);
}

void predictor_decode_mono(APEContextdec * ctx, int count)
{
    APEPredictor *p = &ctx->predictor;
    int32_t *decoded0 = ctx->decoded0;
    int32_t predictionA, currentA, A, sign;
	int i,j;

    currentA = p->lastA[0];

	for(j= 0; j< count; j++)
	{
        p->buf[YDELAYA] = currentA;
        p->buf[YDELAYA - 1] = p->buf[YDELAYA] - p->buf[YDELAYA - 1];

        predictionA = p->buf[YDELAYA    ] * p->coeffsA[0][0] +
                      p->buf[YDELAYA - 1] * p->coeffsA[0][1] +
                      p->buf[YDELAYA - 2] * p->coeffsA[0][2] +
                      p->buf[YDELAYA - 3] * p->coeffsA[0][3];
		currentA = decoded0[j] + (predictionA >> 10);

		p->buf[YADAPTCOEFFSA]     = APESIGN(p->buf[YDELAYA    ]);
        p->buf[YADAPTCOEFFSA - 1] = APESIGN(p->buf[YDELAYA - 1]);

        sign = APESIGN(decoded0[j]);
        p->coeffsA[0][0] += p->buf[YADAPTCOEFFSA    ] * sign;
        p->coeffsA[0][1] += p->buf[YADAPTCOEFFSA - 1] * sign;
        p->coeffsA[0][2] += p->buf[YADAPTCOEFFSA - 2] * sign;
        p->coeffsA[0][3] += p->buf[YADAPTCOEFFSA - 3] * sign;

		p->filterA[0] = currentA + ((p->filterA[0] * 31) >> 5);
        decoded0[j] = p->filterA[0];

        for(i=1; i<= PREDICTOR_SIZE; i++)
        {
			p->buf[i-1] = p->buf[i];
        }
	}
	p->lastA[0] = currentA;
}
/*********************LPC****************************************/

#ifndef HIFI_ACC
void do_init_filter(APEFilter *f, int16_t * buf, int order)
{
    f->coeffs = buf;
    f->delay       = buf + order;
    f->adaptcoeffs = buf + 2*order;
    Hifi_Ape_MemSet(buf, 0, (order * 3) * sizeof(int16_t));
    f->avg = 0;
}

void init_filter(APEContextdec * ctx, APEFilter *f, int16_t * buf, int order)
{
    do_init_filter(&f[0], buf, order);
    do_init_filter(&f[1], buf + order * 3, order);//
}

int16_t av_clip_int16(int a)
{
    if ((a+0x8000) & ~0xFFFF) return (a>>31) ^ 0x7FFF;
    else                      return a;
}


void do_apply_filter(APEContextdec * ctx,/* int version,*/ APEFilter *f, int32_t *data, int count, int order, int fracbits)
{
    int res,i,j;
    int absres;
	int mul;
    int version = ctx->fileversion; 
  
	
    for(i= 0; i<count; i++)
	{   
		res = 0;
        mul = APESIGN(data[i]);
        for(j = 0; j< order; j++)
        {
            res   += f->coeffs[j] * f->delay[j];
            f->coeffs[j] += mul * f->adaptcoeffs[j];
        }
		res = (res + (1 << (fracbits - 1))) >> fracbits; 
        res += data[i];
        data[i] = res;
     
		
        for(j = 1; j< order; j++)
        {
            f->delay[j-1] = f->delay[j];
        }
        f->delay[order-1] = av_clip_int16(res);

        for(j = 1; j< order; j++)
        {
            f->adaptcoeffs[j-1] = f->adaptcoeffs[j];
        }
		if (version < 3980) 
		{
            f->adaptcoeffs[order-1]  = (res == 0) ? 0 : ((res >> 28) & 8) - 4;
            f->adaptcoeffs[order-5] >>= 1;
            f->adaptcoeffs[order-9] >>= 1;
        } else 
		{
            absres = FFABS(res);
            if (absres)
                f->adaptcoeffs[order-1] = ((res & (1<<31)) - (1<<30)) >> (25 + (absres <= f->avg*3) + (absres <= f->avg*4/3));
            else
                f->adaptcoeffs[order-1] = 0;

            f->avg += (absres - f->avg) / 16;

            f->adaptcoeffs[order-2] >>= 1;
            f->adaptcoeffs[order-3] >>= 1;
            f->adaptcoeffs[order-9] >>= 1;
        }

	}   

}

#endif 
void ape_apply_filters(APEContextdec * ctx, int32_t * decoded0,
                              int32_t * decoded1, int count)
{
    int i;
    #ifndef HIFI_ACC
    for (i = 0; i < APE_FILTER_LEVELS; i++) 
    {
        if (!ape_filter_orders[ctx->fset][i])  
        {  
             break;
        }
        do_apply_filter(ctx,&ctx->filters[i][0], decoded0, count, ape_filter_orders[ctx->fset][i], ape_filter_fracbits[ctx->fset][i]);
        if (decoded1)
        {
          do_apply_filter(ctx, &ctx->filters[i][1], decoded1, count, ape_filter_orders[ctx->fset][i], ape_filter_fracbits[ctx->fset][i]);
        }
    }
    #else
		if(ctx->compression_level >1000)
		{
   #if 1
     /*******左声道***************/
    Hifi_Set_ACC_XFER_Disable(0,count,HIfi_ACC_TYPE_APE_L);//开始传输配置数据和初始化系数(不往fifo送) 
    APE_Set_CFG(0,ctx->fileversion,ctx->compression_level);  	  			
    Hifi_Set_ACC_XFER_Start(0,count,HIfi_ACC_TYPE_APE_L);//可以开始往fifo传数据，并且可以取数据
    HIFI_DMA_TO_ACC(decoded0,(uint32*)TX_FIFO,count,(uint32*)RX_FIFO,decoded0); 
    /*******右声道***************/
    if(decoded1)
    {
        Hifi_Set_ACC_XFER_Disable(0,count,HIfi_ACC_TYPE_APE_R);//开始传输配置数据和初始化系数(不往fifo送) 
        APE_Set_CFG(0,ctx->fileversion,ctx->compression_level);  	  			
        Hifi_Set_ACC_XFER_Start(0,count,HIfi_ACC_TYPE_APE_R);//可以开始往fifo传数据，并且可以取数据
        HIFI_DMA_TO_ACC(decoded1,(uint32*)TX_FIFO,count,(uint32*)RX_FIFO,decoded1);   
    }
   #else
     /*******左声道***************/
    Hifi_Set_ACC_XFER_Disable(0,count>>1,HIfi_ACC_TYPE_APE_L);//开始传输配置数据和初始化系数(不往fifo送) 
    APE_Set_CFG(0,ctx->fileversion,ctx->compression_level);  	  			
    Hifi_Set_ACC_XFER_Start(0,count>>1,HIfi_ACC_TYPE_APE_L);//可以开始往fifo传数据，并且可以取数据
    HIFI_DMA_TO_ACC(decoded0,(uint32*)TX_FIFO,count>>1,(uint32*)RX_FIFO,decoded0); 
    /*******右声道***************/
    if(decoded1)
    {
        Hifi_Set_ACC_XFER_Disable(0,count>>1,HIfi_ACC_TYPE_APE_R);//开始传输配置数据和初始化系数(不往fifo送) 
        APE_Set_CFG(0,ctx->fileversion,ctx->compression_level);  	  			
        Hifi_Set_ACC_XFER_Start(0,count>>1,HIfi_ACC_TYPE_APE_R);//可以开始往fifo传数据，并且可以取数据
        HIFI_DMA_TO_ACC(decoded1,(uint32*)TX_FIFO,count>>1,(uint32*)RX_FIFO,decoded1);   
    }
        /*******左声道***************/
    Hifi_Set_ACC_XFER_Disable(0,count>>1,HIfi_ACC_TYPE_APE_L);//开始传输配置数据和初始化系数(不往fifo送) 
    APE_Set_CFG(0,ctx->fileversion,ctx->compression_level);  	  			
    Hifi_Set_ACC_XFER_Start(0,count>>1,HIfi_ACC_TYPE_APE_L);//可以开始往fifo传数据，并且可以取数据
    HIFI_DMA_TO_ACC(&decoded0[count>>1],(uint32*)TX_FIFO,count>>1,(uint32*)RX_FIFO,&decoded0[count>>1]); 
    /*******右声道***************/
    if(decoded1)
    {
        Hifi_Set_ACC_XFER_Disable(0,count>>1,HIfi_ACC_TYPE_APE_R);//开始传输配置数据和初始化系数(不往fifo送) 
        APE_Set_CFG(0,ctx->fileversion,ctx->compression_level);  	  			
        Hifi_Set_ACC_XFER_Start(0,count>>1,HIfi_ACC_TYPE_APE_R);//可以开始往fifo传数据，并且可以取数据
        HIFI_DMA_TO_ACC(&decoded1[count>>1],(uint32*)TX_FIFO,count>>1,(uint32*)RX_FIFO,&decoded1[count>>1]);   
    }
   #endif
}
    #endif 
}



/********************解码主体***************************************/



void ape_unpack_mono(APEContextdec * ctx, int count)
{
    int32_t left;
    int32_t *decoded0 = ctx->decoded0;
    int32_t *decoded1 = ctx->decoded1;

    if (ctx->frameflags & APE_FRAMECODE_STEREO_SILENCE)
    {
        entropy_decode(ctx, count, 0);      
        return;
    }

    entropy_decode(ctx, count, 0);
    ape_apply_filters(ctx, decoded0, NULL, count);
    predictor_decode_mono(ctx, count);

    if (ctx->channels == 1) 
    {
        while (count--) 
        {
            left = *decoded0;
           *(decoded0++) = left;
        }
    }
}

void ape_unpack_stereo(APEContextdec * ctx, int count)
{
    int32_t left, right;
    int32_t *decoded0 = ctx->decoded0;
    int32_t *decoded1 = ctx->decoded1;

    if ((ctx->frameflags & APE_FRAMECODE_STEREO_SILENCE) == APE_FRAMECODE_STEREO_SILENCE)
    {
        return;
    }

    entropy_decode(ctx, count, 1);  
    ape_apply_filters(ctx, decoded0, decoded1, count);
    predictor_decode_stereo(ctx, count);
    /* Decorrelate and scale to output depth */
 
    while (count--) 
	{
		int32_t temp;
		if (*decoded0 <0)
		{
            temp = (-*decoded0>>1);
			temp = -temp;
		}
		else
		{
			temp = (*decoded0>>1); 
		}
        left = *decoded1 - temp;
        right = left + *decoded0;

        *(decoded0++) = left;
        *(decoded1++) = right;
    }
}

/********************解码***************************************/


static void init_frame_decoder(APEContextdec * ctx, ByteIOContext *pb)
{
    int i;
    init_entropy_decoder(ctx,pb);
    init_predictor_decoder(ctx);

    #ifndef HIFI_ACC
    for (i = 0; i < APE_FILTER_LEVELS; i++) 
    {
        if (!ape_filter_orders[ctx->fset][i])
        {
            break;
        }    
        init_filter(ctx, ctx->filters[i], ctx->filterbuf[i], ape_filter_orders[ctx->fset][i]);
    }
    #else
	 if(ctx->compression_level >1000)
	 {
    Hifi_Set_ACC_XFER_Disable(0,BLOCKS_PER_LOOP,HIfi_ACC_TYPE_APE_L);//开始传输配置数据和初始化系数(不往fifo送)
    APE_Set_CFG(0,ctx->fileversion,ctx->compression_level);  
    APE_Clear(0);
    Hifi_Set_ACC_clear(0);
    Hifi_Set_ACC_Dmacr(0);
    Hifi_Set_ACC_Intcr(0);    
    Hifi_Set_ACC_XFER_Start(0,BLOCKS_PER_LOOP,HIfi_ACC_TYPE_APE_L);//可以开始往fifo传数据，并且可以取数据
	 }
    #endif 
}
    // Initialize the frame decoder 
	

u32 ape_decode_frame(APEContextdec *s,u8 *outdata)
 {
    int nblocks;
    int i, n;
    int blockstodecode;
    int ret;
    int channel = s->channels ;
    APEContext *pApe;
    pApe = &apeobj;
	if(pApe->frm_left_sample == 0)
	{
		return 0;
	}		
	nblocks = pApe->frm_left_sample;
	blockstodecode = FFMIN(BLOCKS_PER_LOOP, nblocks);
	pApe->frm_left_sample -= blockstodecode;
    s->error=0;
	if ((s->channels == 1) || (s->frameflags & APE_FRAMECODE_PSEUDO_STEREO))
    {
	    ape_unpack_mono(s, blockstodecode);//单声道解码
	    channel = 1;
    }
	else
	{
		ape_unpack_stereo(s, blockstodecode);//双声道解码
	}

	if(s->error || (s->ptr > s->data_end))
    {
	    s->samples=0;
        Hifi_Ape_Printf(" decode error!!! s->error = %d",s->error);

		return 0;//-1;
    }
 
    Blockout(s->decoded0,s->decoded1,outdata,blockstodecode,channel,pApe->bps);//结果数据整理
    pApe->TimePos += blockstodecode;
   
    return blockstodecode;
 }

int Get_Frame_Info(APEContextdec *s,ByteIOContext *pb )
{
    int skip;
    int diff_file_pos=0;
    int i;
    u32 in_size;
    APEContext *pApe;
    pApe = &apeobj;
    s->fileversion       = pApe->fileversion;
    s->compression_level = pApe->compressiontype;
    s->flags             = pApe->formatflags;
    s->channels          = pApe->channels;
    s->fset              = s->compression_level / 1000 - 1;
     if(pApe->frm_left_sample == 0)
     {
            /********获取新帧输入数据与输出数据个数*********************/
            in_size = pApe->frames[pApe->APE_Frm_NUM].size;
            dataobj.cacheindex = 0;
        	s->ptr = 0;
        	s->data_end = in_size;
            dataobj.cacheindex = 0;
            if(pApe->APE_Frm_NUM== (pApe->totalframes - 1))
            {
            	pApe->frm_left_sample = pApe->finalframeblocks;
            }
            else
            {
            	pApe->frm_left_sample = pApe->blocksperframe;
            }
            
            /********重新初始化所有需要的buffer*********************/

          	Hifi_Ape_MemSet(s->decoded0,  0, sizeof(s->decoded0));
            Hifi_Ape_MemSet(s->decoded1,  0, sizeof(s->decoded1));
           

            #ifndef HIFI_ACC
            for (i = 0; i < APE_FILTER_LEVELS; i++)
			{
    			if(!s->filterbuf[i])
    			{
    				if (!ape_filter_orders[s->fset][i])
    				{
    					break;
    				}
    				s->filterbuf[i] = (int16_t *)av_malloc((ape_filter_orders[s->fset][i] * 3) * 4);
			    }
			}
            #endif 
            if(pApe->APE_Frm_NUM >= pApe->totalframes)
            {
                return -1;
            }
            /********获取当前帧位置*********************/
            diff_file_pos = pApe->frames[pApe->APE_Frm_NUM].pos - RKFIO_FTell(ape_file_handle);
            RKFIO_FSeek(diff_file_pos, SEEK_CUR,ape_file_handle); //获取新帧位置       

            skip =pApe->frames[pApe->APE_Frm_NUM].skip;
        	while(skip--)
			{
        		fread8(&dataobj);
			} 
             /********获取部分帧信息*********************/
             
            init_frame_decoder(s,pb);
             
             /********获取下一帧索引*********************/
             
            pApe->APE_Frm_NUM++;
		}
     return  0;

}
void ape_decode(uint8_t *APE_out_Buffer,u32* OutLength)
{    
     int frm_in_size,ret;
     ByteIOContext *pb = &pbobj;
	 APEContextdec *s = &sobj;     
     ret = Get_Frame_Info(s,pb); 
     if(ret < 0)
    {
      *OutLength = 0;
    }
     else
     {        
        *OutLength = ape_decode_frame(s,APE_out_Buffer); 
     }
}

void APE_free()
{   
    int i;
    APEContext *pApe;
    pApe = &apeobj;
    av_free(pApe->frames);
    av_free(pApe->seektable);
    for (i = 0; i < APE_FILTER_LEVELS; i++)
    {
        if(!sobj.filterbuf[i])
        {
        	if (!ape_filter_orders[sobj.fset][i])
        	{
        		break;
        	}
        	av_free(sobj.filterbuf[i]);
        }
    }              
}
#pragma arm section code
#endif
#endif
