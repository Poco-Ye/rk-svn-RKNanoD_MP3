//
//
//
#include "SysConfig.h"

#ifndef _XVID_DEC_MAIN_
#define _XVID_DEC_MAIN_

#ifdef VIDEO_AVI
/*
**whether compile XVID code.
*/
#define XVID_INCLUDE
#endif 

/*
** platform.
*/
//#define VC_PLATFORM     // VC and RVDS all can use.
#define MDK_PLATFORM  // MDK platform

/*
whether to optimization the yuv2rgb565 transform,as current the yuv2rgb565 had reduce about one third computation.
after the optimization,although the speed improve a lot,but it consume more 2.5k byte memory.
*/
#define YUV_TO_RGB_OPTIMATION

/*
** introduce the YUV2RGB formula that be used by MPEG4.
*/
#define YUV_TO_RGB_MPEG4

/*
** for DMA translate,switch data.
*/

#define SWAP_DATA_FOR_DMA_TRANSPORT

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define XVID_DEC_SUCCESS        0    //decode success. 
#define XVID_DEC_ERROR          1    //decode failure.
#define XVID_DEC_END            2    //decode end normally.


/*some global macro definition.*/

//the max supporting lcd resolution. 
#if (LCD_DIRECTION == LCD_VERTICAL)
    #define     XVID_MAX_X_WIDTH            LCD_HEIGHT
    #define     XVID_MAX_Y_HIGHT            LCD_WIDTH
#else
    #define     XVID_MAX_X_WIDTH            LCD_WIDTH
    #define     XVID_MAX_Y_HIGHT            LCD_HEIGHT
#endif

//the max supporting mb resolution. 
#define XIVD_MAX_MB_WIDTH   (XVID_MAX_X_WIDTH/16)
#define XIVD_MAX_MB_HIGHT   (XVID_MAX_Y_HIGHT/16)

/* bitsteam buf size */
#define XVID_BS_BUF_SIZE    (2*1024)  //if can,you should malloc big as far as possible,as it can reduce the nubmer to read file.



//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
malloc one line yuv memory,note: the yuv data is compact placement.
it mean one line follow  by one line closely.
*/
extern char  xvid_y_slice_buf[16][XVID_MAX_X_WIDTH];
extern char  xvid_u_slice_buf[8][XVID_MAX_X_WIDTH/2];
extern char  xvid_v_slice_buf[8][XVID_MAX_X_WIDTH/2];

#ifdef VC_PLATFORM
/*
malloc a memory that can put one frame,note,the frame data is compact placement.
it mean one line follow  by one line closely. this memory be used for testing.
*/
extern char  xvid_y_frm_buf[XVID_MAX_Y_HIGHT][XVID_MAX_X_WIDTH];
extern char  xvid_u_frm_buf[XVID_MAX_Y_HIGHT/2][XVID_MAX_X_WIDTH/2];
extern char  xvid_v_frm_buf[XVID_MAX_Y_HIGHT/2][XVID_MAX_X_WIDTH/2];
#endif

/*
malloc a memory that can put one line rgb565,note,the rgb data is compact placement.
it mean one line follow  by one line closely. this memory be used for testing.
*/
extern short xvid_rgb565_slice_buf[16][XVID_MAX_X_WIDTH];

#ifdef VC_PLATFORM
/*
malloc a memory that can put one line rgb565 data,note,the rgb data is compact placement.
it mean one line follow  by one line closely. this memory be used for testing.
*/
extern short xvid_rgb565_frm_buf[XVID_MAX_Y_HIGHT][XVID_MAX_X_WIDTH];
#endif

extern int  xvid_slice_count;        
extern int  xvid_frm_decoded;       
extern int  xvid_frm_count;          
extern int  xvid_dec_end;           
extern int  xvid_dec_error;          
extern int  xvid_assert_fail;        

/*
** input buffer.
*/
extern char xvid_bs_buf[XVID_BS_BUF_SIZE+4];  

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern int xvid_dec_open(void);
extern int xvid_dec_frame(void);
extern int xvid_dec_reset(void);
extern int xvid_dec_close(void);
extern int xvid_dec_slice_hook(int stride);

//extern int xvid_yuv_rgb565WithDMA(unsigned char *pY, unsigned char *pU, unsigned char *pV, short *pRgb, const int stride);

extern int xvid_yuv_rgb565WithDMA(unsigned char *pY, unsigned char *pU, unsigned char *pV, int *pRgb, const int stride,const int lcd_width);
//extern int xvid_yuv_rgb565(char *pY, char *pU, char *pV, short *pRgb, int stride);
extern int xvid_bs_read(char *pbuf, int size, int *end_frame);
extern int xvid_bs_init(void);

extern int AviGetVideoData(char *pbuf, int size, int *end_frame);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
