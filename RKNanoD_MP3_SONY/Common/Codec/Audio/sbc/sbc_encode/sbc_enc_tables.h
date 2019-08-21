/*
 *
 *  Bluetooth low-complexity, subband codec (SBC) library
 *
 *  Copyright (C) 2004-2008  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2004-2005  Henryk Ploetz <henryk@ploetzli.ch>
 *  Copyright (C) 2005-2006  Brad Midgley <bmidgley@xmission.com>
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

#include "sbc_enc.h"
#include "sbc_enc_math.h"
#include "audio_main.h"

#ifdef _SBC_ENCODE_


/* A2DP specification: Appendix B, page 69 */

_ATTR_AUDIO_SBC_ENCODE_TEXT_
static int sbc_offset4[4][4] = {
	{ -1, 0, 0, 0 },
	{ -2, 0, 0, 1 },
	{ -2, 0, 0, 1 },
	{ -2, 0, 0, 1 }
};

/* A2DP specification: Appendix B, page 69 */
_ATTR_AUDIO_SBC_ENCODE_TEXT_
static int sbc_offset8[4][8] = {
	{ -2, 0, 0, 0, 0, 0, 0, 1 },
	{ -3, 0, 0, 0, 0, 0, 1, 2 },
	{ -4, 0, 0, 0, 0, 0, 1, 2 },
	{ -4, 0, 0, 0, 0, 0, 1, 2 }
};

#define SP4(val) ASR(val, SCALE_PROTO4_TBL)
#define SA4(val) ASR(val, SCALE_ANA4_TBL)
#define SP8(val) ASR(val, SCALE_PROTO8_TBL)
#define SA8(val) ASR(val, SCALE_ANA8_TBL)


_ATTR_AUDIO_SBC_ENCODE_TEXT_
static int32 _sbc_proto_4[20] = {
	SP4(0x02cb3e8c), SP4(0x22b63dc0), SP4(0x002329cc), SP4(0x053b7548),
	SP4(0x31eab940), SP4(0xec1f5e60), SP4(0xff3773a8), SP4(0x0061c5a7),
	SP4(0x07646680), SP4(0x3f239480), SP4(0xf89f23a8), SP4(0x007a4737),
	SP4(0x00b32807), SP4(0x083ddc80), SP4(0x4825e480), SP4(0x0191e578),
	SP4(0x00ff11ca), SP4(0x00fb7991), SP4(0x069fdc58), SP4(0x4b584000)
};


_ATTR_AUDIO_SBC_ENCODE_TEXT_
static int32 _anamatrix4[4] = {
	SA4(0x2d413cc0), SA4(0x3b20d780), SA4(0x40000000), SA4(0x187de2a0)
};

_ATTR_AUDIO_SBC_ENCODE_TEXT_
static int32 _sbc_proto_8[40] = {
	SP8(0x02e5cd20), SP8(0x22d0c200), SP8(0x006bfe27), SP8(0x07808930),
	SP8(0x3f1c8800), SP8(0xf8810d70), SP8(0x002cfdc6), SP8(0x055acf28),
	SP8(0x31f566c0), SP8(0xebfe57e0), SP8(0xff27c437), SP8(0x001485cc),
	SP8(0x041c6e58), SP8(0x2a7cfa80), SP8(0xe4c4a240), SP8(0xfe359e4c),
	SP8(0x0048b1f8), SP8(0x0686ce30), SP8(0x38eec5c0), SP8(0xf2a1b9f0),
	SP8(0xffe8904a), SP8(0x0095698a), SP8(0x0824a480), SP8(0x443b3c00),
	SP8(0xfd7badc8), SP8(0x00d3e2d9), SP8(0x00c183d2), SP8(0x084e1950),
	SP8(0x4810d800), SP8(0x017f43fe), SP8(0x01056dd8), SP8(0x00e9cb9f),
	SP8(0x07d7d090), SP8(0x4a708980), SP8(0x0488fae8), SP8(0x0113bd20),
	SP8(0x0107b1a8), SP8(0x069fb3c0), SP8(0x4b3db200), SP8(0x00763f48)
};

_ATTR_AUDIO_SBC_ENCODE_TEXT_
static int32 _anamatrix8[8] = {
	SA8(0x3b20d780), SA8(0x187de2a0), SA8(0x3ec52f80), SA8(0x3536cc40),
	SA8(0x238e7680), SA8(0x0c7c5c20), SA8(0x2d413cc0), SA8(0x40000000)
};

#endif
