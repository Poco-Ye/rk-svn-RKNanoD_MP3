/*****************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  - Native API implementation  -
 *
 *  Copyright(C) 2001-2004 Peter Ross <pross@xvid.org>
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
 * $Id: xvid.c,v 1.65.2.5 2007/06/27 18:57:42 Isibaar Exp $
 *
 ****************************************************************************/

//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <time.h>

#include "../xvid.h"
#include "decoder.h"
//#include "bitstream/cbp.h"
#include "idct/idct.h"
//#include "image/interpolate8x8.h"
#include "utils/mem_transfer.h"
//#include "utils/mbfunctions.h"
#include "dequant/dequant.h"
//#include "motion/motion.h"
#include "bitstream/mbcoding.h"
//#include "image/qpel.h"

//#if defined(_DEBUG)
//unsigned int xvid_debug = 0; /* xvid debug mask */
//#endif

#ifdef XVID_INCLUDE

#ifdef MDK_PLATFORM
#pragma arm section code = "XvidDecCode", rodata = "XvidDecCode", rwdata = "XvidDecData", zidata = "XvidDecBss"
#endif

/* detect cpu flags  */
 unsigned int
detect_cpu_flags(void)
{
    /* enable native assembly optimizations by default */
    unsigned int cpu_flags = XVID_CPU_ASM;

#if defined(ARCH_IS_IA32) || defined(ARCH_IS_X86_64)


    if ((cpu_flags & XVID_CPU_SSE))
        cpu_flags &= ~XVID_CPU_SSE;

    if ((cpu_flags & XVID_CPU_SSE2))
        cpu_flags &= ~XVID_CPU_SSE2;

#endif



    return cpu_flags;
}


/*****************************************************************************
 * XviD Init Entry point
 *
 * Well this function initialize all internal function pointers according
 * to the CPU features forced by the library client or autodetected (depending
 * on the XVID_CPU_FORCE flag). It also initializes vlc coding tables and all
 * image colorspace transformation tables.
 *
 * Returned value : XVID_ERR_OK
 *                  + API_VERSION in the input XVID_INIT_PARAM structure
 *                  + core build  "   "    "       "               "
 *
 ****************************************************************************/



int xvid_gbl_init(xvid_gbl_init_t * init)
{
    //unsigned int cpu_flags;

    if (XVID_VERSION_MAJOR(init->version) != 1) /* v1.x.x */
        return XVID_ERR_VERSION;

    //cpu_flags = (init->cpu_flags & XVID_CPU_FORCE) ? init->cpu_flags : detect_cpu_flags();

    /* Initialize the function pointers */
    idct_int32_init();

    //init_vlc_tables();

    /* Fixed Point Forward/Inverse DCT transformations */

    //idct = idct_int32;



    /* Qpel stuff */
    //xvid_QP_Funcs = &xvid_QP_Funcs_C;

    //xvid_QP_Add_Funcs = &xvid_QP_Add_Funcs_C;

    //xvid_Init_QP();

    /* Quantization functions */
    //quant_h263_intra   = quant_h263_intra_c;

    //quant_h263_inter   = quant_h263_inter_c;

    //dequant_h263_intra = dequant_h263_intra_c;

    //dequant_h263_inter = dequant_h263_inter_c;

    //quant_mpeg_intra   = quant_mpeg_intra_c;

    //quant_mpeg_inter   = quant_mpeg_inter_c;

    //dequant_mpeg_intra = dequant_mpeg_intra_c;

    //dequant_mpeg_inter = dequant_mpeg_inter_c;

    /* Block transfer related functions */
    //transfer_8to16copy = transfer_8to16copy_c;

    //transfer_16to8copy = transfer_16to8copy_c;

    //transfer_8to16sub  = transfer_8to16sub_c;

    //transfer_8to16subro  = transfer_8to16subro_c;

    //transfer_8to16sub2 = transfer_8to16sub2_c;

    //transfer_8to16sub2ro = transfer_8to16sub2ro_c;

    //transfer_16to8add  = transfer_16to8add_c;

    //transfer8x8_copy   = transfer8x8_copy_c;

    //transfer8x4_copy   = transfer8x4_copy_c;

#if 0
    /* Image interpolation related functions */
    interpolate8x8_halfpel_h  = interpolate8x8_halfpel_h_c;

    interpolate8x8_halfpel_v  = interpolate8x8_halfpel_v_c;

    interpolate8x8_halfpel_hv = interpolate8x8_halfpel_hv_c;

    interpolate8x4_halfpel_h  = interpolate8x4_halfpel_h_c;

    interpolate8x4_halfpel_v  = interpolate8x4_halfpel_v_c;

    interpolate8x4_halfpel_hv = interpolate8x4_halfpel_hv_c;

    interpolate8x8_halfpel_add = interpolate8x8_halfpel_add_c;

    interpolate8x8_halfpel_h_add = interpolate8x8_halfpel_h_add_c;

    interpolate8x8_halfpel_v_add = interpolate8x8_halfpel_v_add_c;

    interpolate8x8_halfpel_hv_add = interpolate8x8_halfpel_hv_add_c;

    interpolate16x16_lowpass_h = interpolate16x16_lowpass_h_c;

    interpolate16x16_lowpass_v = interpolate16x16_lowpass_v_c;

    interpolate16x16_lowpass_hv = interpolate16x16_lowpass_hv_c;

    interpolate8x8_lowpass_h = interpolate8x8_lowpass_h_c;

    interpolate8x8_lowpass_v = interpolate8x8_lowpass_v_c;

    interpolate8x8_lowpass_hv = interpolate8x8_lowpass_hv_c;

    interpolate8x8_6tap_lowpass_h = interpolate8x8_6tap_lowpass_h_c;

    interpolate8x8_6tap_lowpass_v = interpolate8x8_6tap_lowpass_v_c;

    interpolate8x8_avg2 = interpolate8x8_avg2_c;

    interpolate8x8_avg4 = interpolate8x8_avg4_c;
#endif

    /* Functions used in motion estimation algorithms */
    //calc_cbp   = calc_cbp_c;



//#if defined(_DEBUG)
//    xvid_debug = init->debug;

//#endif

    return(0);
}



/*****************************************************************************
 * XviD Global Entry point
 *
 * Well this function initialize all internal function pointers according
 * to the CPU features forced by the library client or autodetected (depending
 * on the XVID_CPU_FORCE flag). It also initializes vlc coding tables and all
 * image colorspace transformation tables.
 *
 ****************************************************************************/


int
xvid_global(void *handle,
            int opt,
            void *param1,
            void *param2)
{
    switch (opt)
    {

    case XVID_GBL_INIT :
        return xvid_gbl_init((xvid_gbl_init_t*)param1);


    default :
        return XVID_ERR_FAIL;
    }
}

/*****************************************************************************
 * XviD Native decoder entry point
 *
 * This function is just a wrapper to all the option cases.
 *
 * Returned values : XVID_ERR_FAIL when opt is invalid
 *                   else returns the wrapped function result
 *
 ****************************************************************************/

int
xvid_decore(void *handle,
            int opt,
            void *param1,
            void *param2)
{
    switch (opt)
    {

    case XVID_DEC_CREATE:
        return decoder_create((xvid_dec_create_t *) param1);

    case XVID_DEC_DESTROY:
        return decoder_destroy((DECODER *) handle);

    case XVID_DEC_DECODE:
        return decoder_decode((DECODER *) handle, (xvid_dec_frame_t *) param1, (xvid_dec_stats_t*) param2);

    default:
        return XVID_ERR_FAIL;
    }
}

#ifdef MDK_PLATFORM
#pragma arm section code
#endif

#endif

