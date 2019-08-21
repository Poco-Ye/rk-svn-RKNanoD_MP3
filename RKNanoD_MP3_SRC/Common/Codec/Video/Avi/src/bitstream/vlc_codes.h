/*****************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  - Variable Length Code header  -
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
 * $Id: vlc_codes.h,v 1.18 2004/03/22 22:36:23 edgomez Exp $
 *
 ****************************************************************************/
#ifndef _VLC_CODES_H_
#define _VLC_CODES_H_

#include "../portab.h"

#define VLC_ERROR (-1)

#define ESCAPE  3
#define ESCAPE1 6
#define ESCAPE2 14
#define ESCAPE3 15

typedef struct
{
    //uint32_t code;
	uint8_t code;
    uint8_t len;
}

VLC;

typedef struct
{
    uint8_t last;
    uint8_t run;
    int8_t level;
}

EVENT;

typedef struct
{
    uint8_t len;
    uint8_t last;
    uint8_t run;
    int8_t level;
}

REVERSE_EVENT;

typedef struct
{
    VLC vlc;
    EVENT event;
}

VLC_TABLE;


/******************************************************************
 * common tables between encoder/decoder                          *
 ******************************************************************/

extern VLC dc_lum_tab[];
extern short dc_threshold[];
extern VLC_TABLE coeff_tab[2][102];
extern uint8_t max_level/*[2]*/[2][64];
extern uint8_t max_run/*[2]*/[2][64];
extern VLC sprite_trajectory_code[32768];
extern VLC sprite_trajectory_len[15];
extern VLC mcbpc_intra_tab[15];
extern VLC mcbpc_inter_tab[29];
extern VLC xvid_cbpy_tab[16];
extern VLC dcy_tab[511];
extern VLC dcc_tab[511];
extern VLC mb_motion_table[65];
extern VLC mcbpc_intra_table[64];
extern VLC mcbpc_inter_table[257];
extern VLC cbpy_table[64];
extern VLC TMNMVtab0[];
extern VLC TMNMVtab1[];
extern VLC TMNMVtab2[];

//extern REVERSE_EVENT XvidIntraHuffmanTbl[528]; // (8, 4)
extern REVERSE_EVENT XvidIntraHuffmanTbl[1064];	 // (10, 2)
//extern REVERSE_EVENT XvidIntraHuffmanTbl[4096];	 // (12, 0)

#endif /* _VLC_CODES_H */
