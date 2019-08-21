/*
 *
 *  Bluetooth low-complexity, subband codec (SBC) library
 *
 *  Copyright (C) 2004-2008  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2004-2005  Henryk Ploetz <henryk@ploetzli.ch>
 *  Copyright (C) 2005-2008  Brad Midgley <bmidgley@xmission.com>
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#include "SysConfig.h"
#include "sbc_enc_type.h"

#ifdef _SBC_ENCODE_

#define fabs(x) ((x) < 0 ? -(x) : (x))
/* C does not provide an explicit arithmetic shift right but this will
   always be correct and every compiler *should* generate optimal code */
#define ASR(val, bits) ((-2 >> 1 == -1) ? \
		 ((int32)(val)) >> (bits) : ((int32) (val)) / (1 << (bits)))

#define SCALE_PROTO4_TBL	15
#define SCALE_ANA4_TBL		17
#define SCALE_PROTO8_TBL			(16)			/*   1八子带enc的cos系数 :16    */
#define SCALE_ANA8_TBL				(17)			/*   八子带enc的角度pi系数 :17    */
#define SCALE4_STAGE1_BITS	15		
#define SCALE4_STAGE2_BITS	16	
#define SCALE8_STAGE1_BITS			(15)			/* 2 八子带enc cos乘加后的系数 :15    */	
#define SCALE8_STAGE2_BITS			(15)			/*  八子带enc乘加后求out的系数 :15    */	
	

typedef int32 sbc_fixed_t;


#define SCALE4_STAGE1(src)  ASR(src, SCALE4_STAGE1_BITS)
#define SCALE4_STAGE2(src)  ASR(src, SCALE4_STAGE2_BITS)
#define SCALE8_STAGE1(src)  ASR(src, SCALE8_STAGE1_BITS)
#define SCALE8_STAGE2(src)  ASR(src, SCALE8_STAGE2_BITS)

#define SBC_FIXED_0(val) { val = 0; }
#define MUL(a, b)        ((a) * (b))

#define MULA(a, b, res)  ((a) * (b) + (res))

#endif

