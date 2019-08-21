/*****************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  - Global definitions  -
 *
 *  Copyright(C) 2002 Michael Militzer <isibaar@xvid.org>
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
 * $Id: global.h,v 1.24 2005/09/13 12:12:15 suxen_drol Exp $
 *
 ****************************************************************************/

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

//#include <assert.h>

#include "../xvid.h"
#include "portab.h"

/* --- macroblock modes --- */

#define MODE_INTER  0
#define MODE_INTER_Q 1
#define MODE_INTER4V 2
#define MODE_INTRA  3
#define MODE_INTRA_Q 4
#define MODE_NOT_CODED 16
#define MODE_NOT_CODED_GMC 17

/* --- bframe specific --- */

#define MODE_DIRECT   0
#define MODE_INTERPOLATE 1
#define MODE_BACKWARD  2
#define MODE_FORWARD  3
#define MODE_DIRECT_NONE_MV 4
#define MODE_DIRECT_NO4V 5


/*
 * vop coding types
 * intra, prediction, backward, sprite, not_coded
 */
#define I_VOP 0
#define P_VOP 1
#define B_VOP 2
#define S_VOP 3
#define N_VOP 4

/* convert mpeg-4 coding type i/p/b/s_VOP to XVID_TYPE_xxx */
 __inline int
coding2type(int coding_type)
{
    return coding_type + 1;
}

/* convert XVID_TYPE_xxx to bitstream coding type i/p/b/s_VOP */
 __inline int
type2coding(int xvid_type)
{
    return xvid_type - 1;
}


typedef struct
{
    int x;
    int y;
}

VECTOR;



typedef struct
{
    VECTOR duv[3];
}

WARPPOINTS;

/* save all warping parameters for GMC once and for all, instead of
   recalculating for every block. This is needed for encoding&decoding
   When switching to incremental calculations, this will get much shorter
*/

/* we don't include WARPPOINTS wp here, but in FRAMEINFO itself */

typedef struct
{
    int num_wp;  /* [input]: 0=none, 1=translation, 2,3 = warping */
    /* a value of -1 means: "structure not initialized!" */
    int s;     /* [input]: calc is done with 1/s pel resolution */

    int W;
    int H;

    int ss;
    int smask;
    int sigma;

    int r;
    int rho;

    int i0s;
    int j0s;
    int i1s;
    int j1s;
    int i2s;
    int j2s;

    int i1ss;
    int j1ss;
    int i2ss;
    int j2ss;

    int alpha;
    int beta;
    int Ws;
    int Hs;

    int dxF, dyF, dxG, dyG;
    int Fo, Go;
    int cFo, cGo;
} GMC_DATA;

typedef struct _NEW_GMC_DATA
{
    /*  0=none, 1=translation, 2,3 = warping
    *  a value of -1 means: "structure not initialized!" */
    int num_wp;

    /* {0,1,2,3}  =>   {1/2,1/4,1/8,1/16} pel */
    int accuracy;

    /* sprite size * 16 */
    int sW, sH;

    /* gradient, calculated from warp points */
    int dU[2], dV[2], Uo, Vo, Uco, Vco;

    void (*predict_16x16)( struct _NEW_GMC_DATA *  This,
                          uint8_t *dst,  uint8_t *src,
                          int dststride, int srcstride, int x, int y, int rounding);
    void (*predict_8x8)  ( struct _NEW_GMC_DATA *  This,
                          uint8_t *uDst,  uint8_t *uSrc,
                          uint8_t *vDst,  uint8_t *vSrc,
                          int dststride, int srcstride, int x, int y, int rounding);
    void (*get_average_mv)( struct _NEW_GMC_DATA *  Dsp, VECTOR *  mv,
                           int x, int y, int qpel);
} NEW_GMC_DATA;

typedef struct
{
    uint8_t *y;
    uint8_t *u;
    uint8_t *v;
}

IMAGE;


typedef struct
{
    uint32_t bufa;      // bufa与bufb拼成64bit, [bufa bufb]
	                    // getbits直接从这个buff取bit，一共有64bit，bufa存放前32bit,bufb存放后32bit
    uint32_t bufb;      
    uint32_t buf;       // 用于存放无效的bits值，目前发现这个成员没有什么用
    uint32_t pos;       // [bufa bufb]中有效码流的开始bit位
    uint32_t *tail;     // 当前用到的bit buf位置, 包括bufa与bufb, 也就是说(tail+2)才是bit buf实际有效的数据
    uint32_t *start;    // bit buf的起始位置
    uint32_t length;    // 表示当前bit buf中还剩下多少字节数据
    uint32_t initpos;
	uint32_t end_frame; // 表示当前stream buf里存放的数据是否包含一帧的结尾数据
}

Bitstream;


#define MBPRED_SIZE  15


typedef struct
{
    /* decoder/encoder */
//    VECTOR mvs[4];

    short int pred_values[6][MBPRED_SIZE];
    char acpred_directions[6];

    int mode;
    int quant;     /* absolute quant */

//    int field_dct;
//    int field_pred;
//    int field_for_top;
//    int field_for_bot;

//    /* encoder specific */
//
//    VECTOR pmvs[4];
//    VECTOR qmvs[4];    /* mvs in quarter pixel resolution */
//
//    int32_t sad8[4];   /* SAD values for inter4v-VECTORs */
//    int32_t sad16;    /* SAD value for inter-VECTOR */
//
//    int dquant;
//    int cbp;

//    /* bframe stuff */
//
//    VECTOR b_mvs[4];
//    VECTOR b_qmvs[4];
//
//    VECTOR amv; /* average motion vectors from GMC  */
//    int32_t mcsel;
//
//    VECTOR  mvs_avg;      //CK average of field motion vectors
//
//    /* This structure has become way to big! What to do? Split it up?   */

}

MACROBLOCK;

/* useful macros */

#define MIN(X, Y) ((X)<(Y)?(X):(Y))
#define MAX(X, Y) ((X)>(Y)?(X):(Y))
/* #define ABS(X)    (((X)>0)?(X):-(X)) */
#define SIGN(X)   (((X)>0)?1:-1)
#define CLIP(X,AMIN,AMAX)   (((X)<(AMIN)) ? (AMIN) : ((X)>(AMAX)) ? (AMAX) : (X))
#define DIV_DIV(a,b)    (((a)>0) ? ((a)+((b)>>1))/(b) : ((a)-((b)>>1))/(b))
#define SWAP(_T_,A,B)    { _T_ tmp = A; A = B; B = tmp; }

#endif       /* _GLOBAL_H_ */
