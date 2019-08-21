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
//#include <string.h>
//#include <math.h>

#include "xvid.h"
//#include "src/portab.h"
#include "xvid_dec_main.h"

#ifdef XVID_INCLUDE

#ifdef MDK_PLATFORM
#pragma arm section code = "XvidDecCode", rodata = "XvidDecCode", rwdata = "XvidDecData", zidata = "XvidDecBss"
#endif

/*
*/
//#define INPUT_RAW_DATA

/*****************************************************************************
 *               Global vars in module and constants
 ****************************************************************************/

#ifdef VC_PLATFORM

#define USE_PNM 0
#define USE_TGA 1

 int XDIM = 0;
 int YDIM = 0;
 int gImgWidth = 0;
 int gImgHeight = 0;

 int ARG_SAVEDECOUTPUT = 0;
 int ARG_SAVEMPEGSTREAM = 0;
 char *ARG_INPUTFILE = NULL;
 char *ARG_OUTPUTFILE = NULL;
 int CSP = XVID_CSP_I420;
 int BPP = 1;
 int FORMAT = USE_PNM;

FILE *in_file, *out_file, *out_rgb_file, *out_bs_file;

#endif

#ifdef MDK_PLATFORM

#include "SysInclude.h"
#include "FsInclude.h"

HANDLE in_file;

#endif

// char filepath[256] = "./";
 void *dec_handle = NULL;

char * type2str(int type)
{
    if (type == XVID_TYPE_IVOP)
        return "I";

    if (type == XVID_TYPE_PVOP)
        return "P";

    if (type == XVID_TYPE_BVOP)
        return "B";

    return "S";
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
** some global variables definitions
*/

int  xvid_slice_count;
int  xvid_frm_decoded;
int  xvid_frm_count;
int  xvid_dec_end;
int  xvid_dec_error;
int  xvid_assert_fail;

/*
*/
char  xvid_y_slice_buf[16][XVID_MAX_X_WIDTH];
char  xvid_u_slice_buf[8][XVID_MAX_X_WIDTH/2];
char  xvid_v_slice_buf[8][XVID_MAX_X_WIDTH/2];

#ifdef VC_PLATFORM
/*
*/
char  xvid_y_frm_buf[XVID_MAX_Y_HIGHT][XVID_MAX_X_WIDTH];
char  xvid_u_frm_buf[XVID_MAX_Y_HIGHT/2][XVID_MAX_X_WIDTH/2];
char  xvid_v_frm_buf[XVID_MAX_Y_HIGHT/2][XVID_MAX_X_WIDTH/2];
#endif

/*
*/
short xvid_rgb565_slice_buf[16][XVID_MAX_X_WIDTH];

#ifdef VC_PLATFORM
/*
*/
short xvid_rgb565_frm_buf[XVID_MAX_Y_HIGHT][XVID_MAX_X_WIDTH];
#endif

/*
*/
#ifdef MDK_PLATFORM
__align(4)
#endif
char xvid_bs_buf[XVID_BS_BUF_SIZE+4];  // +4 for save other data


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* init decoder before first run */
/* 0: success; <0: fail*/
int xvid_dec_open(void)
{
    int ret;

    xvid_gbl_init_t   xvid_gbl_init;
    xvid_dec_create_t xvid_dec_create;

    /* Reset the structure with zeros */
    memset(&xvid_gbl_init, 0, sizeof(xvid_gbl_init_t));
    memset(&xvid_dec_create, 0, sizeof(xvid_dec_create_t));

    /*------------------------------------------------------------------------
     * XviD core initialization
     *----------------------------------------------------------------------*/

    /* Version */
    xvid_gbl_init.version = XVID_VERSION;

//    /* Assembly setting */
//
//    if (use_assembler)
//#ifdef ARCH_IS_IA64
//        xvid_gbl_init.cpu_flags = XVID_CPU_FORCE | XVID_CPU_IA64;
//
//#else
//        xvid_gbl_init.cpu_flags = 0;
//
//#endif
//    else
//        xvid_gbl_init.cpu_flags = XVID_CPU_FORCE;

    //xvid_gbl_init.debug = debug_level;

    xvid_global(XVID_GBL_INIT, 0, &xvid_gbl_init, NULL);

    /*------------------------------------------------------------------------
     * XviD encoder initialization
     *----------------------------------------------------------------------*/

    /* Version */
    xvid_dec_create.version = XVID_VERSION;

    /*
     * Image dimensions -- set to 0, xvidcore will resize when ever it is
     * needed
     */
    xvid_dec_create.width = 0;

    xvid_dec_create.height = 0;

    ret = xvid_decore(NULL, XVID_DEC_CREATE, &xvid_dec_create, NULL);

    dec_handle = xvid_dec_create.handle;

#ifdef YUV_TO_RGB_OPTIMATION
	yuv2rgb_tab_init();
#endif

 	xvid_frm_count = 0;
	xvid_dec_end = 0;
	xvid_dec_error = 0;
	xvid_assert_fail = 0;

    memset(xvid_rgb565_slice_buf[0], 0, sizeof(short)*16*XVID_MAX_X_WIDTH);

	/* first fill bit stream to buffer */
	//xvid_bs_init();

    return(ret);
}

int xvid_dec_main(xvid_dec_stats_t *xvid_dec_stats)
{

    int ret;

    xvid_dec_frame_t xvid_dec_frame;

    /* Reset all structures */
    memset(&xvid_dec_frame, 0, sizeof(xvid_dec_frame_t));
    memset(xvid_dec_stats, 0, sizeof(xvid_dec_stats_t));

    /* Set version */
    xvid_dec_frame.version = XVID_VERSION;
    xvid_dec_stats->version = XVID_VERSION;

    /* No general flags to set */
    xvid_dec_frame.general          = 0;

    /* Input stream */
    xvid_dec_frame.bitstream        = NULL;
    xvid_dec_frame.length           = 0;

    /* Output frame structure */
 //   xvid_dec_frame.output.plane[0]  = ostream;
 //   xvid_dec_frame.output.stride[0] = XDIM * BPP;
 //   xvid_dec_frame.output.csp = CSP;

    ret = xvid_decore(dec_handle, XVID_DEC_DECODE, &xvid_dec_frame, xvid_dec_stats);

    return(ret);
}

/* read bitstream form avi file */
/* size is wanted length, but if there is no less than size, will return real length*/
int xvid_bs_read(char *pbuf, int size, int *end_frame)
{
	int ret;

#ifdef VC_PLATFORM
#ifdef INPUT_RAW_DATA
	ret = fread(pbuf, 1, size, in_file);
#else
	ret = AviGetVideoData(pbuf, size, end_frame);
#endif
	if(ret == 0)
	{
		xvid_dec_end = 1; // data is over
	}
	return ret;
#endif
#ifdef MDK_PLATFORM
#ifdef INPUT_RAW_DATA
	//ret = fread(pbuf, 1, size, in_file);
	ret = FileRead(pbuf, size, in_file);
#else
	ret = AviGetVideoData(pbuf, size, end_frame);
#endif
	if(ret == 0)
	{
		xvid_dec_end = 1; //data is over
	}
	return ret;
#endif
}

/* decoder one frame */
int xvid_dec_frame(void)
{
	int ret = 0;
	xvid_dec_stats_t xvid_dec_stats;

	xvid_slice_count = 0;
	xvid_frm_decoded = 0;
	xvid_dec_error = 0;
	xvid_assert_fail = 0;

	ret = xvid_bs_init();
    if(ret == -1)
    {
        return XVID_DEC_END;
    }
    else if(ret == -2)
    {
        xvid_frm_count++;
        return XVID_DEC_SUCCESS;
    }

    /* decoding first frame */
    do
    {
        /* Decode frame */
        ret = xvid_dec_main(&xvid_dec_stats);
		if(ret < 0)
		{
            break;
            //return XVID_DEC_ERROR; // decode failure
		}

    }
    while ( (xvid_frm_decoded == 0) &&   //decode one frame
		    (xvid_dec_end     == 0) &&   // decode over
		    (xvid_dec_error   == 0) &&   // decode error
		    (xvid_assert_fail == 0)      // asser failure
		  );

#ifdef VC_PLATFORM
    PRINTF("Frame %5d: type = %s, length(bytes) =%7d\n",
           xvid_frm_count, type2str(xvid_dec_stats.type), 0);
#endif

	if(xvid_frm_decoded)
	{
		xvid_frm_count++;
	}

	if(xvid_dec_error)
	{
		return XVID_DEC_ERROR;
	}

	if(xvid_assert_fail)
	{
		return XVID_DEC_ERROR;
	}

	if(xvid_dec_end)
	{
		return XVID_DEC_END;
	}

	return XVID_DEC_SUCCESS;
}

/* reset decoder after seek */
int xvid_dec_reset(void)
{
    int ret = 0;

	xvid_frm_count = 0;
	xvid_dec_end = 0;
	xvid_dec_error = 0;
	xvid_assert_fail = 0;

	//xvid_bs_init();

    return(ret);
}

/* close decoder to release resources */
int xvid_dec_close(void)
{
    int ret;

    ret = xvid_decore(dec_handle, XVID_DEC_DESTROY, NULL, NULL);

    return(ret);
}

extern int VideoDecodeInit(void);
extern int VideoDecodeUninit(void);

#ifdef VC_PLATFORM

/*****************************************************************************
 *        Main program
 ****************************************************************************/

int main(int argc, char *argv[])
{
    unsigned char *mp4_buffer = NULL;
    unsigned char *mp4_ptr    = NULL;
    unsigned char *out_buffer = NULL;
//    int useful_bytes;
//    int chunk;
//    xvid_dec_stats_t xvid_dec_stats;

//    double totaldectime;

//    long totalsize;
    int status;

    int use_assembler = 0;
    int debug_level = 0;

//    int filenr;
    int i;

    PRINTF("xvid_decraw - raw mpeg4 bitstream decoder ");
    PRINTF("written by Christoph Lampert 2002-2003\n\n");

    /*****************************************************************************
     * Command line parsing
     ****************************************************************************/

    for (i = 1; i < argc; i++)
    {

        if (strncmp("-debug", argv[i], 6) == 0 && i < argc - 1 )
        {
            i++;

            if (sscanf(argv[i], "0x%x", &debug_level) != 1)
            {
                debug_level = atoi(argv[i]);
            }
        }
        else if (strncmp("-i", argv[i], 2) == 0 && i < argc - 1 )
        {
            i++;
            ARG_INPUTFILE = argv[i];
        }
        else if (strncmp("-o", argv[i], 2) == 0 && i < argc - 1)
        {
            i++;
            ARG_OUTPUTFILE = argv[i];
        }
        else if (strncmp("-w", argv[i], 2) == 0 && i < argc - 1 )
        {
            i++;
            gImgWidth = atoi(argv[i]);
        }
        else if (strncmp("-h", argv[i], 2) == 0 && i < argc - 1)
        {
            i++;
            gImgHeight = atoi(argv[i]);
        }
        else
        {
            exit(-1);
        }
    }

    if ((gImgHeight <= 0) || (gImgWidth <= 0))
    {
        fprintf(stderr, "image width and height are error!\n");
        exit(-1);
    }

    in_file = fopen(ARG_INPUTFILE, "rb");

    if (in_file == NULL)
    {
        fprintf(stderr, "Error opening input file %s\n", ARG_INPUTFILE);
        return(-1);
    }

#ifndef INPUT_RAW_DATA
	if(VideoDecodeInit() != 0)
	{
		PRINTF("Avi File Parse Error!\n");
		return(-1);
	}

#if 0
	{
		out_bs_file = fopen("out_bs.raw", "wb");
		if(out_bs_file)
		{
			int cbRead;

			while(1)
			{
				cbRead = xvid_bs_read(xvid_bs_buf, XVID_BS_BUF_SIZE);
				if(cbRead == 0)
					break;
				fwrite(xvid_bs_buf, 1, cbRead, out_bs_file);
			}

			fclose(out_bs_file);
			return(0);
		}
	}
#endif
#endif

    if (ARG_OUTPUTFILE)
    {
        out_file = fopen(ARG_OUTPUTFILE, "wb");

        if (out_file == NULL)
        {
            fprintf(stderr, "Error opening output file %s\n", ARG_OUTPUTFILE);
            return(-1);
        }

		out_rgb_file = fopen("rgb565.rgb", "wb");
        if (out_rgb_file == NULL)
        {
            fprintf(stderr, "Error opening output file %s\n", ARG_OUTPUTFILE);
            return(-1);
        }
    }

    /*****************************************************************************
     *        XviD PART  Start
     ****************************************************************************/

    status = xvid_dec_open();

    if (status)
    {
        fprintf(stderr,
                "Decore INIT problem, return value %d\n", status);
        goto release_all;
    }


    /*****************************************************************************
     *                          Main loop
     ****************************************************************************/

	xvid_frm_count = 0;
	xvid_dec_end = 0;
	xvid_dec_error = 0;

	/* first fill bit stream */
	xvid_bs_init();

	while(1)
	{
		if(xvid_dec_frame() != XVID_DEC_SUCCESS)
		{
			break;
		}
	}

    /*****************************************************************************
     *      XviD PART  Stop
     ****************************************************************************/

release_all:
    if (dec_handle)
    {
        status = xvid_dec_close();

        if (status)
            fprintf(stderr, "decore RELEASE problem return value %d\n", status);
    }

#ifdef INPUT_RAW_DATA
	if(in_file)
	{
		fclose(in_file);
	}
#endif
	if(out_file)
	{
		fclose(out_file);
	}

	if(out_rgb_file)
	{
		fclose(out_rgb_file);
	}

    return(0);
}

#endif

#ifdef MDK_PLATFORM

int avi_main()
{
    unsigned char *mp4_buffer = NULL;
    unsigned char *mp4_ptr    = NULL;
    unsigned char *out_buffer = NULL;
    int status;
    int use_assembler = 0;
    int debug_level = 0;
    int i;

#ifdef INPUT_RAW_DATA
    in_file = FileOpen("U:\\", "OPPO    RAW", "R");
    if (in_file == NOT_OPEN_FILE)
    {
        return(-1);
    }
#else
	in_file = NOT_OPEN_FILE;
#endif

#ifndef INPUT_RAW_DATA
	if(VideoDecodeInit() != 0)
	{
		return(-1);
	}
#endif

    /*****************************************************************************
     *        XviD PART  Start
     ****************************************************************************/

    status = xvid_dec_open();
    if (status)
    {
        goto release_all;
    }


    /*****************************************************************************
     *                          Main loop
     ****************************************************************************/



	while(1)
	{
		if(xvid_dec_frame() != XVID_DEC_SUCCESS)
		{
			break;
		}
	}

    /*****************************************************************************
     *      XviD PART  Stop
     ****************************************************************************/

release_all:

    xvid_dec_close();

#ifndef INPUT_RAW_DATA
	VideoDecodeUninit();
#endif

	if(in_file != NOT_OPEN_FILE)
	{
		FileClose(in_file);
	}

    return(0);
}

#endif

#ifdef MDK_PLATFORM
#pragma arm section code
#endif

#endif

