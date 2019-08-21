/************************************************************************
 *       Smart Jpeg Decoder
 *
 * File:
 *  jpgdec.c
 *
************************************************************************/
#include "image_main.h"
#ifdef JPG_DEC_INCLUDE

#pragma arm section code = "JpgDecCode", rodata = "JpgDecCode", rwdata = "JpgDecData", zidata = "JpgDecBss"

#include "jpgdec_decompress.h"
#include "jpgdec_tables.h"
#include "jpgdec_global.h"
#include "stdio.h"
#include "jpgdec_globalvardeclare.h"
#include "jpgdec_postprocess.h"
#include "string.h"
#include "pJPG.h"
#include "jpgdec_fixed.h"
//#ifndef ARM_PLATFORM
//#include "malloc.h"
//#endif

//*******post process********************************************
//Pcoeff_buf_t dc_coeffs[JPGD_MAXCOMPONENTS];
//Pcoeff_buf_t ac_coeffs[JPGD_MAXCOMPONENTS];

//int block_y_mcu[JPGD_MAXCOMPONENTS];
IMAGE_DEC_INFO *gJpgProOutput;

uint32 ValidW = 0;
uint32 ValidH = 0;

int eob_run;

//int dc_bias, quant_shift;

//FILE* IDCTInFile;

//int debugNumBlocks = 0;
int TranverseStep;

void (*decode_block_func)(int component_id, int16 *p);
void (*JpgZoom)(int16, int16, int, uint8*, uint16*);
void (*inverseQ_idct)(int16* p, int16* pAC, int16* Pdst_ptr, int16* q);

int16    Jpg_gIdctSize;
int16    Jpg_gImageXSize;
int16    Jpg_gImageYSize;
int16    Jpg_gDstXSizeExpand;
int16    Jpg_gDstYSizeExpand;
int16    Jpg_gDstXSize;
int16    Jpg_gDstYSize;
int16    Jpg_gNumCoffs;

uint8  Jpg_pHuffNum[JPGD_MAXHUFFTABLES][17];  /* pointer to number of Huffman codes per bit size */
uint8  Jpg_pHuffVal[JPGD_MAXHUFFTABLES][256];  /* pointer to Huffman codes per bit size */

QUANT_TYPE Jpg_pQuant[JPGD_MAXQUANTTABLES][64];    /* pointer to quantization tables */
//int16* Jpg_pQuant[JPGD_MAXQUANTTABLES];
int      Jpg_gScanType;                      /* Grey, Yh1v1, Yh1v2, Yh2v1, Yh2v2,
                                             CMYK111, CMYK4114 */

int      Jpg_gComponentsInFrame;                 /* # of components in frame */
int      Jpg_gCompHoriSamp[JPGD_MAXCOMPONENTS];     /* component's horizontal sampling factor */
int      Jpg_gCompVertSamp[JPGD_MAXCOMPONENTS];     /* component's vertical sampling factor */
int      Jpg_gCompQuant[JPGD_MAXCOMPONENTS];      /* component's quantization table selector */
int      Jpg_gCompIdent[JPGD_MAXCOMPONENTS];      /* component's ID */
int      Jpg_gCompHoriBlocks[JPGD_MAXCOMPONENTS];
int      Jpg_gCompVertBlocks[JPGD_MAXCOMPONENTS];

int      Jpg_gCompInScan;                  /* # of components in scan */
int      Jpg_gCompList[JPGD_MAXCOMPSINSCAN];      /* components in this scan */
int      Jpg_gCompDCTable[JPGD_MAXCOMPONENTS];     /* component's DC Huffman coding table selector */
int      Jpg_gCompACTable[JPGD_MAXCOMPONENTS];     /* component's AC Huffman coding table selector */

int      Jpg_gSpectralStart;                 /* spectral selection start */
int      Jpg_gSpectralEnd;                   /* spectral selection end   */
int      Jpg_gSuccessiveLow;                 /* successive approximation low */
int      Jpg_gSuccessiveHigh;                /* successive approximation high */

int      Jpg_gMaxMcuXSize;                 /* MCU's max. X size in pixels */
int      Jpg_gMaxMcuYSize;                 /* MCU's max. Y size in pixels */
int      Jpg_gBlockSize;

int      Jpg_gMaxMcuXSize2;                 /* MCU's max. X size in pixels */
int      Jpg_gMaxMcuYSize2;
int      Jpg_gBlocksPerMcu;
int      Jpg_gMaxBlocksPerRow;
int      Jpg_gMcusPerRow;   //每行多少宏块
int      Jpg_gMcusPerCol;
int      Jpg_gMcuOrg[JPGD_MAXBLOCKSPERMCU];
int      McuOrgReverse[JPGD_MAXBLOCKSPERMCU];

unsigned char  gJpg_gHuffman[(256*4 + 256 + 512*4)*JPGD_MAXHUFFTABLES];
Phuff_tables_t  Jpg_gHuffman[JPGD_MAXHUFFTABLES];// = {&gJpg_gHuffman[0],&gJpg_gHuffman[1],&gJpg_gHuffman[2],&gJpg_gHuffman[3],&gJpg_gHuffman[4],&gJpg_gHuffman[5],&gJpg_gHuffman[6],&gJpg_gHuffman[7]};
//Phuff_tables_t  DC_PhuffTab[JPGD_MAXCOMPONENTS];
//Phuff_tables_t  AC_PhuffTab[JPGD_MAXCOMPONENTS];

//  Pcoeff_buf_t dc_coeffs[JPGD_MAXCOMPONENTS];
//  Pcoeff_buf_t ac_coeffs[JPGD_MAXCOMPONENTS];

//int block_y_mcu[JPGD_MAXCOMPONENTS];

//int      Jpg_gInBufLeft;
//int      Jpg_gTemFlag;
uint8    Jpg_gEofFlag;

#ifdef ARM_PLATFORM
__align(32) uint8    JpgProInBuf[JPGD_INBUFSIZE + 128 + 32];
#else
uint8    JpgProInBuf[JPGD_INBUFSIZE + 128 + 32];
#endif
uint8*   Jpg_pInBuf;
uint32   Jpg_gBitBuf;
int      Jpg_gBitsLeft;
uint8*   Jpg_pPinBufOfs;

//  uint32   bits_Debug;
//  uint32   bits_Debugs;

int      Jpg_gRestartInterval;
int      Jpg_gRestartLeft;
int      Jpg_gNextRestartNum;

int      Jpg_gMaxMcusPerRow;    //每一行有多少MCU块
int      Jpg_gMaxBlocksPerMcu;
int      Jpg_gMaxMcusPerCol;

int      Jpg_gMcusPerCol;

int32*   Jpg_pComponent[JPGD_MAXBLOCKSPERMCU];   /* points into the lastdcvals table */
int32    Jpg_gLastDCValue[JPGD_MAXCOMPONENTS];

Phuff_tables_t   Jpg_gDCHuffSeg[JPGD_MAXBLOCKSPERMCU];
Phuff_tables_t   Jpg_gACHuffSeg[JPGD_MAXBLOCKSPERMCU];
//void*    Jpg_pBlocks[JPGD_MAXBLOCKS];         /* list of all dynamically allocated blocks */

//int      Jpg_gBlockMaxZagSet[JPGD_MAXBLOCKSPERROW];

//int16   Jpg_pPsampleBuf[30*1024];
int8   Jpg_pPsampleBuf[MACROROW_DATA_LEN];


int16    Jpg_gTempBlock[80];
int16    Jpg_gTempMcuBlock[64*6];
int8*   Jpg_pPSampleY;
int16*   Jpg_pPSampleU;

int isyuvDataNotReady = 1;
static uint8 *srcY;
static uint16 *srcUV;
static int currentRow;
int currentMacroRow = 0;
unsigned short GetByteFromWordArray(unsigned short*in, unsigned long index)
{
    unsigned short tmp0;
    if (index&1)
    {
        tmp0 = (*(in + (index >> 1)) & 0xff00) >> 8;
    }
    else
    {
        tmp0 = (*(in + (index >> 1)) & 0xff);
    }

    return tmp0;
}
//uint32 Jpg_RGBOutput[320];

//完整
void JpgZoom2RGB888(int16 SrcW, int16 SrcH, int macroRowIndex, uint8* Yptr, uint16* Uptr)
{
    unsigned short row, column;

    unsigned long zoomindstxInt;
    unsigned long zoomindstyInt;

    unsigned long zoomindstxIntInv;
    unsigned long zoomindstyIntInv;

//    long  zoomindstx;
    long  zoomindsty, nextZoomindsty;

    //uint32 RR,GG,BB;
    int32 RR, GG, BB;

    int16  zoominwidth;//原图按zoomrate缩小后图片宽度
    int16  zoominheight;//原图按zoomrate缩小后图片高度

//    unsigned long offset, len;
    unsigned long Blank, leftBlank;

    unsigned long yuvOffset;

    //int16 src_length = SCR_LENGTH;
    //int16 src_height = SCR_HEIGHT;

    uint8* dstYptr = Yptr;  //y缩放输出，于输入共用
    uint16* dstUptr = Uptr;  //uv缩放输出，与输入共用

    float  zoomrate= 1.0f;// = Jpg_gZoomrate;

//    long outputW;
//    long outputH;

    //int16 YuvBlockXSize = max_mcu_x_size2*mcu_procress_one_time>>1;
    //int16 YuvBlockXSize = Jpg_gDstXSizeExpand>>1;
    int16 YuvBlockXSize = Jpg_gDstXSizeExpand;
#if 0
    if ((Jpg_gDstXSize <= SCR_LENGTH) && (Jpg_gDstYSize <= SCR_HEIGHT))
    {
        zoomrate = 1.0f;
    }
    else
    {
        zoomrate = (float)min(1.0f * SCR_LENGTH / Jpg_gDstXSize, 1.0f * SCR_HEIGHT / Jpg_gDstYSize);
    }

    gJpgProOutput->ValidW = zoominwidth = (short)(Jpg_gDstXSize * zoomrate);
    gJpgProOutput->ValidH = zoominheight = (short)(Jpg_gDstYSize * zoomrate);
#else
    zoominwidth = ValidW;
    zoominheight = ValidH;
#endif

	if(zoomrate == 1.0)
	{
		// Y
		for (row = 0;row < Jpg_gMaxMcuYSize2;row++)
		{
			memcpy(dstYptr, Yptr, zoominwidth);
			Yptr += YuvBlockXSize;
        	dstYptr += zoominwidth;	
		}

		// UV
		for (row = 0;row < Jpg_gMaxMcuYSize2;row += 2)
		{
			memcpy(dstUptr, Uptr, zoominwidth);
			Uptr += (YuvBlockXSize >> 1);
        	dstUptr += (zoominwidth >> 1);
		}
		
		return;
	}

//    zoominwidth = outputW;//(short)(SrcW * zoomrate);
//    //zoominwidth = ((zoominwidth + 3)&(~3));
//
//    zoominheight = outputH;//(short)(SrcH * zoomrate);

    leftBlank = ((SCR_LENGTH - zoominwidth) >> 1);//每行左边填0的像素个数

    Blank = leftBlank + ((SCR_HEIGHT - zoominheight) >> 1) * (unsigned long)SCR_LENGTH;//加上上面填0的个数
    //uvBlank = leftBlank + ((SCR_HEIGHT - outputH)>>2)*(unsigned long)SCR_LENGTH;

    // 目的跟源的比率，< 1
    zoomindstxInt = ((unsigned long)zoominwidth << 16) / SrcW + 1;//预先计算出dst_y_size*dst_x_size尺寸的YUV按比率缩小到屏幕尺寸的宽度比值
    zoomindstyInt = ((unsigned long)zoominheight << 16) / SrcH + 1;//预先计算出dst_y_size*dst_x_size尺寸的YUV按比率缩小到屏幕尺寸的高度比值
    // 源跟目的的比率，> 1
    zoomindstxIntInv = ((unsigned long)SrcW << 16) / zoominwidth + 1; // 与zoomindstxInt相反
    zoomindstyIntInv = ((unsigned long)SrcH << 16) / zoominheight + 1;
    /*从目的出发映射源,部分宏行y缩放*/
    for (row = 0;row < Jpg_gMaxMcuYSize2;row++)
    {
        int16 curSrcRow = row + Jpg_gMaxMcuYSize2 * macroRowIndex;

        zoomindsty = (curSrcRow * zoomindstyInt) >> 16;
        nextZoomindsty = ((curSrcRow + 1) * zoomindstyInt) >> 16;

        if ((curSrcRow >= Jpg_gImageYSize) || (zoomindsty >= SCR_HEIGHT))
        {
            break;
        }

        for (column = 0;column < zoominwidth;column++)//紧凑存放
        {
#if 1//双线性
			unsigned long coeff0,coeff1;
			unsigned long srcx_origin = ((unsigned long)column * zoomindstxIntInv);

			coeff1 = srcx_origin&0xffff;
			coeff0 = (1<<16) - coeff1; 
            yuvOffset = srcx_origin >> 16;
			
			*(dstYptr + column) = (*(Yptr + yuvOffset)*coeff0 + *(Yptr + yuvOffset + 1)*coeff1)>>16;
#else//最近邻
            yuvOffset = ((unsigned long)column * zoomindstxIntInv) >> 16;
            *(dstYptr + column) = *(Yptr + yuvOffset);
#endif
        }
        Yptr += YuvBlockXSize;
        dstYptr += zoominwidth;
    }

    /*从目的出发映射源,一宏行uv缩放*/
    for (row = 0;row < Jpg_gMaxMcuYSize2;row += 2)
    {
        int16 curSrcRow = (row + Jpg_gMaxMcuYSize2 * macroRowIndex) >> 1;

        zoomindsty = (curSrcRow * zoomindstyInt) >> 16;
        nextZoomindsty = ((curSrcRow + 1) * zoomindstyInt) >> 16;

        if (zoomindsty >= (SCR_HEIGHT >> 1))
        {
            break;
        }

        for (column = 0;column < zoominwidth / 2;column++)//紧凑存放
        {
#if 1//binacular
            unsigned long coeff0,coeff1;
			unsigned long srcx_origin = ((unsigned long)column * zoomindstxIntInv);
			uint16 uu0,vv0,vu0,uu1,vv1,vu1,dstu,dstv;

			coeff1 = srcx_origin&0xffff;
			coeff0 = (1<<16) - coeff1; 
            yuvOffset = srcx_origin >> 16;

            vu0 = *(Uptr + yuvOffset);
			vu1 = *(Uptr + yuvOffset + 1);
			
			uu0 = vu0&0xff;
			uu1 = vu1&0xff;
			dstu = (uu0*coeff0 + uu1*coeff1)>>16;
			
			vv0 = vu0>>8;
			vv1 = vu1>>8;			
			dstv = (vv0*coeff0 + vv1*coeff1)>>16;

			*(dstUptr + column) = (dstu&0xff)|(dstv<<8);
			//*(dstUptr + column) = (*(Uptr + yuvOffset)*coeff0 + *(Uptr + yuvOffset + 1)*coeff1)>>16;			
#else			
            yuvOffset = ((unsigned long)column * zoomindstxIntInv) >> 16;
            *(dstUptr + column) = *(Uptr + yuvOffset);
#endif			
        }

        Uptr += (YuvBlockXSize >> 1);
        dstUptr += (zoominwidth >> 1);
    }

    dstYptr = (uint8*)Jpg_pPSampleY;
    dstUptr = (uint16*)Jpg_pPSampleU;
    RR = GG = BB = 0;
    //一宏行的yuv420 to rgb888
#if 0    
    for (row = 0;row < Jpg_gMaxMcuYSize2;row++)
    {
        int16 curSrcRow = row + Jpg_gMaxMcuYSize2 * macroRowIndex;

        zoomindsty = (curSrcRow * zoomindstyInt) >> 16;
        nextZoomindsty = ((curSrcRow + 1) * zoomindstyInt) >> 16;

        //如果该行数据不会被下一行覆盖，搬到sdram
        if (zoomindsty != nextZoomindsty || zoomindsty == (SCR_HEIGHT - 1))
        {
            rgbtype* dst = (rgbtype*)(gJpgProOutput->ptr_output_buf) + zoomindsty * SCR_LENGTH + Blank;
            

            if(((SCR_HEIGHT - zoominheight) >> 1) + zoomindsty >= SCR_HEIGHT)
            	break;
            //一行的yuv420torgb888
            for (column = 0;column < zoominwidth;column++)//紧凑存放
            {
                unsigned short tmpdiv = (column >> 1);
                int16 YY = *(dstYptr + column) - 16;
                uint16 VU = *(dstUptr + (column >> 1));

                int16 UU = (VU & 0xFF) - 0x80;
                int16 VV = (VU >> 8) - 0x80;

                //UU = VV = 0;

                RR = ((RGB_Y * YY + R_V * VV) >> SHIFT_SCALE) + 4 ;
                GG = ((RGB_Y * YY - G_V * VV - G_U * UU) >> SHIFT_SCALE) + 4;
                BB = ((RGB_Y * YY + B_U * UU) >> SHIFT_SCALE) + 4;

#ifdef RGB16BITS
                *dst++ = (rgbtype) MK_RGB565(RR, GG, BB);
#else
                *dst++ = (rgbtype) MK_RGB888(RR, GG, BB);
#endif
            }

        }
        dstYptr += zoominwidth;
        if ((row&1))
            dstUptr += (zoominwidth >> 1);

    }
#else
#endif	
}
#if 0
//完整
void JpgZoom2YUV420(int16 SrcW, int16 SrcH, int macroRowIndex, uint8* Yptr, uint16* Uptr)
{
    unsigned short row, column;

    unsigned long zoomindstxInt;
    unsigned long zoomindstyInt;

    unsigned long zoomindstxIntInv;
    unsigned long zoomindstyIntInv;

//    long  zoomindstx;
    long  zoomindsty, nextZoomindsty;



    int16  zoominwidth;//原图按zoomrate缩小后图片宽度
    int16  zoominheight;//原图按zoomrate缩小后图片高度

    unsigned long offset, len;
    unsigned long yBlank, uvBlank, leftBlank;

    unsigned long yuvOffset;

    int16 src_length = SCR_LENGTH;
    int16 src_height = SCR_HEIGHT;

    uint8* dstYptr = Yptr;  //y缩放输出，于输入共用
    uint16* dstUptr = Uptr;  //uv缩放输出，与输入共用

    float  zoomrate;// = Jpg_gZoomrate;

//    long outputW;
//    long outputH;

    //int16 YuvBlockXSize = max_mcu_x_size2*mcu_procress_one_time>>1;
    //int16 YuvBlockXSize = Jpg_gDstXSizeExpand>>1;
    int16 YuvBlockXSize = Jpg_gDstXSizeExpand;

    if ((Jpg_gDstXSize <= SCR_LENGTH) && (Jpg_gDstYSize <= SCR_HEIGHT))
    {
        zoomrate = 1.0;
    }
    else
    {
        zoomrate = (float)min(1.0f * SCR_LENGTH / Jpg_gDstXSize, 1.0f * SCR_HEIGHT / Jpg_gDstYSize);
    }

    gJpgProOutput->ValidW = zoominwidth = (short)(Jpg_gDstXSize * zoomrate);
    gJpgProOutput->ValidH = zoominheight = (short)(Jpg_gDstYSize * zoomrate);

    leftBlank = ((SCR_LENGTH - zoominwidth) >> 1) & (~1);//每行左边填0的像素个数

    yBlank = leftBlank + (((SCR_HEIGHT - zoominheight) >> 1) & (~1)) * (unsigned long)SCR_LENGTH;//加上上面填0的个数
    uvBlank = leftBlank + ((SCR_HEIGHT - zoominheight) >> 2) * (unsigned long)SCR_LENGTH;

    // 目的跟源的比率，< 1
    zoomindstxInt = ((unsigned long)zoominwidth << 16) / SrcW + 1;//预先计算出dst_y_size*dst_x_size尺寸的YUV按比率缩小到屏幕尺寸的宽度比值
    zoomindstyInt = ((unsigned long)zoominheight << 16) / SrcH + 1;//预先计算出dst_y_size*dst_x_size尺寸的YUV按比率缩小到屏幕尺寸的高度比值
    // 源跟目的的比率，> 1
    zoomindstxIntInv = ((unsigned long)SrcW << 16) / zoominwidth + 1; // 与zoomindstxInt相反
    zoomindstyIntInv = ((unsigned long)SrcH << 16) / zoominheight + 1;
    /*从目的出发映射源,部分宏行y缩放*/
    for (row = 0;row < Jpg_gMaxMcuYSize2;row++)
    {
        int16 curSrcRow = row + Jpg_gMaxMcuYSize2 * macroRowIndex;

        zoomindsty = (curSrcRow * zoomindstyInt) >> 16;
        nextZoomindsty = ((curSrcRow + 1) * zoomindstyInt) >> 16;

        if ((curSrcRow >= Jpg_gImageYSize) || (zoomindsty >= SCR_HEIGHT))
        {
            break;
        }
#if 0
        for (column = 0;column < zoominwidth / 2;column++)//紧凑存放
        {
            //zoomindstpixels = dstYptr + column;
            unsigned short tmp0, tmp1;
            yuvOffset = ((unsigned long)column * 2 * zoomindstxIntInv) >> 16;
            //*(dstYptr + column) = *(Yptr + yuvOffset);
            tmp0 = GetByteFromWordArray((unsigned short*)Yptr , yuvOffset);
            yuvOffset = ((unsigned long)(column * 2 + 1) * zoomindstxIntInv) >> 16;
            tmp1 = GetByteFromWordArray((unsigned short*)Yptr , yuvOffset);
            *(dstYptr + column) = (tmp1 << 8) | (tmp0);
        }
#else
        for (column = 0;column < zoominwidth;column++)//紧凑存放
        {
            yuvOffset = ((unsigned long)column * zoomindstxIntInv) >> 16;
            *(dstYptr + column) = *(Yptr + yuvOffset);

        }
#endif
        //如果该行数据不会被下一行覆盖，搬到sdram
        if (zoomindsty != nextZoomindsty || zoomindsty == (SCR_HEIGHT - 1))
        {
            len = zoominwidth * sizeof(short); //byte, 紧凑存放

            /* 把y数据送到SDRAM */
            {
                offset = (SCR_LENGTH * zoomindsty + yBlank) * sizeof(short); // byte, 紧凑存放
#if 0
                DwdmaI2E((unsigned long)DSP_BASE_ADR + (unsigned long)dstYptr*2, (unsigned long)share->JpgOutputYBuf + offset, len / 4); // 32bit
                while (EQDwdmaI2EStatus == DW_DMA_BUSY);
#else
                //y
                //fseek(yuv,offset/2,0);
                //fwrite (dstYptr, 1,len/2,yuv);
                memcpy(gJpgProOutput->OutPutBuf + offset / 2, dstYptr, len / 2);
#endif
            }
        }
        Yptr += YuvBlockXSize;
    }

    /*从目的出发映射源,部分宏行uv缩放*/
    for (row = 0;row < Jpg_gMaxMcuYSize2;row += 2)
    {
        int16 curSrcRow = (row + Jpg_gMaxMcuYSize2 * macroRowIndex) >> 1;
        //int16 curSrcRow = (row + Jpg_gMaxMcuYSize2*macroRowIndex);
        //if(!(curSrcRow&1))//specific to h1v1,h1v2 when excute idct1x1
        //continue;
        //curSrcRow = (curSrcRow>>1);

        zoomindsty = (curSrcRow * zoomindstyInt) >> 16;
        nextZoomindsty = ((curSrcRow + 1) * zoomindstyInt) >> 16;

        if (zoomindsty >= (SCR_HEIGHT >> 1))
        {
            break;
        }

        for (column = 0;column < zoominwidth / 2;column++)//紧凑存放
        {
            yuvOffset = ((unsigned long)column * zoomindstxIntInv) >> 16;
            *(dstUptr + column) = *(Uptr + yuvOffset);
        }

        if (zoomindsty != nextZoomindsty || zoomindsty == ((SCR_HEIGHT >> 1) - 1))
        {
            len = zoominwidth * sizeof(short); //byte, 紧凑存放
            {
                offset = (SCR_LENGTH * zoomindsty + uvBlank) * sizeof(short); // byte, 紧凑存放
#if 0
                DwdmaI2E((unsigned long)DSP_BASE_ADR + (unsigned long)dstUptr*2, (unsigned long)(share->JpgOutputUVBuf) + offset, len / 4); // 32bit
                while (EQDwdmaI2EStatus == DW_DMA_BUSY);
#else
                //uv
                //fseek(yuv,(unsigned long)SCR_LENGTH*SCR_HEIGHT/2 + offset/2,0);
                //fwrite (dstUptr, 1,len/2,yuv);

                memcpy(gJpgProOutput->OutPutBuf + SCR_LENGTH*SCR_HEIGHT + offset / 2, dstUptr, len / 2);
#endif
            }
        }
        Uptr += (YuvBlockXSize >> 1);
    }
}
#endif
#if 0//def JPG_PROGRESSIVE

//------------------------------------------------------------------------------
// The following methods decode the various types of blocks encountered
// in progressively encoded images.

//void decode_block_dc_first(int component_id, int block_x, int block_y)
void decode_block_dc_first(int component_id, int16* p)
{
    int s, r;

    //if ((s = JpgDecHuffDecode(Jpg_gHuffman[Jpg_gCompDCTable[component_id]])) != 0)
    //if ((s = JpgDecHuffDecode(DC_PhuffTab[component_id])) != 0)
    Phuff_tables_t ph = DC_PhuffTab[component_id];
    s = JpgDecHuffDecode(ph);
    if (s != 0)
    {
        r = JpgDecGetBits_2(s);
        s = HUFF_EXTEND_TBL(r, s);
        //#define HUFF_EXTEND_TBL(r,s) ((r) < extend_test[s] ? (r) + extend_offset[s] : (r))
        //s = ((r) < extend_test[s] ? (r) + extend_offset[s] : (r));
    }

    Jpg_gLastDCValue[component_id] = (s += Jpg_gLastDCValue[component_id]);

    //Pd->last_dc_val[component_id] = (s += Pd->last_dc_val[component_id]);

    p[0] = s << Jpg_gSuccessiveLow;
    //fwrite(p,sizeof(int16),1,IDCTInFile);
}
//------------------------------------------------------------------------------
//void decode_block_dc_refine(int component_id, int block_x, int block_y)
void decode_block_dc_refine(int component_id, int16* p)
{
    if (JpgDecGetBits_2(1))
    {
        //int16 *p = coeff_buf_getp(dc_coeffs[component_id], block_x, block_y);
        //int16 *p = coeff_buf_getp(ac_coeffs[component_id], block_x, block_y);

        p[0] |= (1 << Jpg_gSuccessiveLow);
        //fwrite(p,sizeof(int16),1,IDCTInFile);
    }
}
//------------------------------------------------------------------------------
//void decode_block_ac_first(int component_id, int block_x, int block_y)
#ifndef skip_ac_scan
void decode_block_ac_first1x1(int component_id, int16 *p)
{

    if (eob_run)
    {
        eob_run--;
        return;
    }
    {
        int k, s, r;
        //Phuff_tables_t ph = Jpg_gHuffman[Jpg_gCompACTable[component_id]];AC_PhuffTab
        Phuff_tables_t ph = AC_PhuffTab[component_id];
        for (k = Jpg_gSpectralStart; k <= Jpg_gSpectralEnd; k++)
        {
            s = JpgDecHuffDecode(ph);

            r = s >> 4;
            s &= 15;

            k += r;
            if (s)
            {
                //r = JpgDecGetBits_2(s);
                JpgDecSkipBits_2(s);
                //s = HUFF_EXTEND_TBL(r, s);

            }
            else if (r != 15)
            {
                k -= r;
                eob_run = 1 << r;

                if (r)
                    eob_run += JpgDecGetBits_2(r);

                eob_run--;

                break;

            }

        }
    }
}
#endif
void decode_block_ac_first(int component_id, int16 *p)
{
    int k, s, r;
    //int16 *p= coeff_buf_getp(ac_coeffs[component_id], block_x, block_y);

    if (eob_run)
    {
        eob_run--;
        return;
    }
    {
        //Phuff_tables_t ph = Jpg_gHuffman[Jpg_gCompACTable[component_id]];
        Phuff_tables_t ph = AC_PhuffTab[component_id];
        for (k = Jpg_gSpectralStart; k <= Jpg_gSpectralEnd; k++)
        {
            //s = JpgDecHuffDecode(Jpg_gHuffman[Jpg_gCompACTable[component_id]]);
            s = JpgDecHuffDecode(ph);

            r = s >> 4;
            s &= 15;
            k += r;
            if (s)
            {
                //if ((k += r) > 63)
                //terminate(JPGD_DECODE_ERROR);

                r = JpgDecGetBits_2(s);
                s = HUFF_EXTEND_TBL(r, s);
#ifdef MINUS_ROOM
                switch (Jpg_gIdctSize)
                {
                    case ONEONE:
                        break;
                    case TWOTWO:
                        if (ZAG[k] == 1 || ZAG[k] == 8 || ZAG[k] == 9)
                            p[CoeffTable2x2[ZAG[k]]] = s << Jpg_gSuccessiveLow;
                        break;
                    case FOURFOUR:
                        {
                            int16 tmpSuffix = CoeffTable4x4[ZAG[k]];
#if 1
                            if (ZAG[k] == 1 || ZAG[k] == 2 || ZAG[k] == 3
                                    || ZAG[k] == 8 || ZAG[k] == 9 || ZAG[k] == 10 || ZAG[k] == 11
                                    || ZAG[k] == 16 || ZAG[k] == 17 || ZAG[k] == 18 || ZAG[k] == 19
                                    || ZAG[k] == 24 || ZAG[k] == 25 || ZAG[k] == 26 || ZAG[k] == 27)
#else
                            if (tmpSuffix < 16)
#endif
                                //p[CoeffTable4x4[ZAG[k]]] = s << Jpg_gSuccessiveLow;
                                p[tmpSuffix] = s << Jpg_gSuccessiveLow;
                            break;
                        }
                    case EIGHTEIGHT:
                        p[ZAG[k]] = s << Jpg_gSuccessiveLow;
                        break;
                }

#else
                p[ZAG[k]] = s << Jpg_gSuccessiveLow;
#endif
            }

            else if (r != 15)
            {
                //if (r == 15)
                {
                    //if ((k += 15) > 63)
                    //terminate(JPGD_DECODE_ERROR);
                }
                //else
                {
                    k -= r;
                    eob_run = 1 << r;

                    if (r)
                        eob_run += JpgDecGetBits_2(r);

                    eob_run--;

                    break;
                }
            }
        }
    }
}
//------------------------------------------------------------------------------
//void decode_block_ac_refine(int component_id, int block_x, int block_y)
void decode_block_ac_refine(int component_id, int16* p)
{
    int s, k, r;
    //int16 ptemp[64];
    int p1 = 1 << Jpg_gSuccessiveLow;
    int m1 = (-1) << Jpg_gSuccessiveLow;
    //int16 *p = coeff_buf_getp(ac_coeffs[component_id], block_x, block_y);

    k = Jpg_gSpectralStart;

    if (eob_run == 0)
    {
        for (; k <= Jpg_gSpectralEnd; k++)
        {
            s = JpgDecHuffDecode(Jpg_gHuffman[Jpg_gCompACTable[component_id]]);

            r = s >> 4;
            s &= 15;

            if (s)
            {
                if (s != 1)
                    return;// JPGD_DECODE_ERROR;//return(JPGD_DECODE_ERROR);

                if (JpgDecGetBits_2(1))
                    s = p1;
                else
                    s = m1;
            }
            else
            {
                if (r != 15)
                {
                    eob_run = 1 << r;

                    if (r)
                        eob_run += JpgDecGetBits_2(r);

                    break;
                }
            }

            do
            {
#if 0//def MINUS_ROOM
                int16 *this_coef;
                switch (Jpg_gIdctSize)
                {
                    case ONEONE:
                        break;

                    case TWOTWO:
                        this_coef = p + CoeffTable2x2[ZAG[k]];
                        break;

                    case FOURFOUR:
                        this_coef = p + CoeffTable4x4[ZAG[k]];
                        break;

                    case EIGHTEIGHT:
                        this_coef = p + ZAG[k];
                        break;
                }
#else
                int16 *this_coef = p + ZAG[k];
#endif

                if (*this_coef != 0)
                {
                    if (JpgDecGetBits_2(1))
                    {
#if 0//def MINUS_ROOM
                        switch (Jpg_gIdctSize)
                        {
                            case ONEONE:
                                break;

                            case TWOTWO:
                                if ((*this_coef & p1) == 0 && ZAG[k] == 1 || ZAG[k] == 8 || ZAG[k] == 9)
                                {
                                    if (*this_coef >= 0)
                                        //p[CoeffTable2x2[ZAG[k]]] = *this_coef + p1;
                                        *this_coef  = *this_coef + p1;
                                    else
                                        //p[CoeffTable2x2[ZAG[k]]] = *this_coef + m1;
                                        *this_coef = *this_coef + m1;
                                }
                                break;

                            case FOURFOUR:
                                if ((*this_coef & p1) == 0 && ZAG[k] == 1 || ZAG[k] == 2 || ZAG[k] == 3
                                        || ZAG[k] == 8 || ZAG[k] == 9 || ZAG[k] == 10 || ZAG[k] == 11
                                        || ZAG[k] == 16 || ZAG[k] == 17 || ZAG[k] == 18 || ZAG[k] == 19
                                        || ZAG[k] == 24 || ZAG[k] == 25 || ZAG[k] == 26 || ZAG[k] == 27)

                                {
                                    if (*this_coef >= 0)
                                        //p[CoeffTable4x4[ZAG[k]]] = *this_coef + p1;
                                        *this_coef  = *this_coef + p1;
                                    else
                                        //p[CoeffTable4x4[ZAG[k]]] = *this_coef + m1;
                                        *this_coef  = *this_coef + m1;
                                }
                                break;

                            case EIGHTEIGHT:
                                if ((*this_coef & p1) == 0)
                                {
                                    if (*this_coef >= 0)
                                        *this_coef += p1;
                                    else
                                        *this_coef += m1;
                                }
                                break;
                        }
#else

                        if ((*this_coef & p1) == 0)
                        {
                            if (*this_coef >= 0)
                                *this_coef += p1;
                            else
                                *this_coef += m1;
                        }
#endif
                    }
                }
                else
                {
                    if (--r < 0)
                        break;
                }

                k++;

            }
            while (k <= Jpg_gSpectralEnd);

            if ((s) && (k < 64))
            {
#if 0//def MINUS_ROOM
                switch (Jpg_gIdctSize)
                {
                    case ONEONE:
                        break;

                    case TWOTWO:
                        if (ZAG[k] == 1 || ZAG[k] == 8 || ZAG[k] == 9)
                            p[CoeffTable2x2[ZAG[k]]] = s;
                        break;

                    case FOURFOUR:
                        if (ZAG[k] == 1 || ZAG[k] == 2 || ZAG[k] == 3
                                || ZAG[k] == 8 || ZAG[k] == 9 || ZAG[k] == 10 || ZAG[k] == 11
                                || ZAG[k] == 16 || ZAG[k] == 17 || ZAG[k] == 18 || ZAG[k] == 19
                                || ZAG[k] == 24 || ZAG[k] == 25 || ZAG[k] == 26 || ZAG[k] == 27)

                            p[CoeffTable4x4[ZAG[k]]] = s;
                        break;

                    case EIGHTEIGHT:
                        p[ZAG[k]] = s;
                        break;
                }
#else
                p[ZAG[k]] = s;
#endif
            }
        }
    }

    if (eob_run > 0)
    {
        for (; k <= Jpg_gSpectralEnd; k++)
        {
#if 0//def MINUS_ROOM
            int16 *this_coef;
            switch (Jpg_gIdctSize)
            {
                case ONEONE:
                    break;

                case TWOTWO:
                    this_coef = p + CoeffTable2x2[ZAG[k]];
                    break;

                case FOURFOUR:
                    this_coef = p + CoeffTable4x4[ZAG[k]];
                    break;

                case EIGHTEIGHT:
                    this_coef = p + ZAG[k];
                    break;
            }
#else
            int16 *this_coef = p + ZAG[k];
#endif

            if (*this_coef != 0)
            {
                if (JpgDecGetBits_2(1))
                {
#if 0//def MINUS_ROOM
                    switch (Jpg_gIdctSize)
                    {
                        case ONEONE:
                            break;

                        case TWOTWO:
                            if ((*this_coef & p1) == 0 && ZAG[k] == 1 || ZAG[k] == 8 || ZAG[k] == 9)
                            {
                                if (*this_coef >= 0)
                                    *this_coef = *this_coef + p1;
                                else
                                    *this_coef = *this_coef + m1;
                            }
                            break;

                        case FOURFOUR:
                            if ((*this_coef & p1) == 0 && ZAG[k] == 1 || ZAG[k] == 2 || ZAG[k] == 3
                                    || ZAG[k] == 8 || ZAG[k] == 9 || ZAG[k] == 10 || ZAG[k] == 11
                                    || ZAG[k] == 16 || ZAG[k] == 17 || ZAG[k] == 18 || ZAG[k] == 19
                                    || ZAG[k] == 24 || ZAG[k] == 25 || ZAG[k] == 26 || ZAG[k] == 27)

                            {
                                if (*this_coef >= 0)
                                    *this_coef = *this_coef + p1;
                                else
                                    *this_coef = *this_coef + m1;
                            }
                            break;

                        case EIGHTEIGHT:
                            if ((*this_coef & p1) == 0)
                            {
                                if (*this_coef >= 0)
                                    *this_coef += p1;
                                else
                                    *this_coef += m1;
                            }
                            break;
                    }
#else
                    if ((*this_coef & p1) == 0)
                    {
                        if (*this_coef >= 0)
                            *this_coef += p1;
                        else
                            *this_coef += m1;
                    }
#endif
                }
            }
        }

        eob_run--;
    }
    return;
}
//------------------------------------------------------------------------------
int16* coeff_buf_getp(Pcoeff_buf_t cb, int block_x, int block_y)
{
    if (block_x >= cb->block_num_x)
        //return(JPGD_ASSERTION_ERROR);
        return NULL;

    if (block_y >= cb->block_num_y)
        //return(JPGD_ASSERTION_ERROR);
        return NULL;

    return (int16 *)(cb->Pdata + block_x * cb->block_size + block_y * (cb->block_size * cb->block_num_x));
}

//------------------------------------------------------------------------------
// The coeff_buf series of methods originally stored the coefficients
// into a "virtual" file which was stored in EMS, XMS, or a disk file. A cache
// was used to make this process more efficient. Now, we can store the entire
// thing in RAM.
Pcoeff_buf_t coeff_buf_open(
    int block_num_x, int block_num_y,
    int block_len_x, int block_len_y)
{
    Pcoeff_buf_t cb = (Pcoeff_buf_t)JpgDecAlloc(sizeof(coeff_buf_t));

    cb->block_num_x = block_num_x;
    cb->block_num_y = block_num_y;

    cb->block_len_x = block_len_x;
    cb->block_len_y = block_len_y;

    cb->block_size = (block_len_x * block_len_y) * sizeof(int16);
    cb->macro_row_size = cb->block_size * block_num_x;

    //cb->Pdata = (uchar *)JpgDecAlloc(cb->block_size * block_num_x * block_num_y);
    cb->Pdata = (uchar *)JpgDecAlloc(cb->macro_row_size * block_num_y);
    if (cb->Pdata == NULL)
        return NULL;

    return cb;
}
//------------------------------------------------------------------------------
// Decode a scan in a progressively encoded image.
int decode_scan_macro_row(uchar **p, int McusPerRow, int BlocksPerMcu, void (*decode_block_func)(int , int16*))
{
    //int Xoffset[JPGD_MAXCOMPONENTS];
    uchar *dp[JPGD_MAXCOMPONENTS];
    //uchar *pp;
    //int component_id;


    const int block_size = ac_coeffs[0]->block_size;
    //memset(Xoffset, 0, sizeof(Xoffset));

    dp[0] = p[0];
    dp[1] = p[1];
    dp[2] = p[2];

    //for (mcu_col = 0; mcu_col < Jpg_gMcusPerCol; mcu_col++)
    while (McusPerRow--)
    {
        int block_x_mcu_ofs = 0, block_y_mcu_ofs = 0;
        int mcu_block;
        //if ((Jpg_gRestartInterval) && (Jpg_gRestartLeft == 0))
        if (Jpg_gRestartLeft == 0)
        {
            int c = JpgDecProcessRestart();
            if (c)
                return c;
        }

        for (mcu_block = 0; mcu_block < BlocksPerMcu; mcu_block++)
        {
            int component_id = Jpg_gMcuOrg[mcu_block];

            if (Jpg_gCompVertSamp[component_id] == 1)
            {
                decode_block_func(component_id, (int16*)(dp[component_id]));
                dp[component_id] += block_size;
            }
            else
            {
                uchar* pp;
                //pp = (p[component_id] + Xoffset[component_id]);
                pp = dp[component_id];
                if (block_x_mcu_ofs)
                    pp += block_size;
                if (block_y_mcu_ofs)
                    pp += ac_coeffs[component_id]->macro_row_size;

                decode_block_func(component_id, (int16*)(pp));
                if (++block_x_mcu_ofs == Jpg_gCompHoriSamp[component_id])
                {
                    block_x_mcu_ofs = 0;

                    if (++block_y_mcu_ofs == Jpg_gCompVertSamp[component_id])
                    {
                        block_y_mcu_ofs = 0;
                        //Xoffset[component_id] += (block_size<<(Jpg_gCompHoriSamp[component_id] - 1));
                        dp[component_id] += (block_size << (Jpg_gCompHoriSamp[component_id] - 1));
                    }
                }
            }
        }

        Jpg_gRestartLeft--;
    }
    return 0;
}
void decode_scan_macro_row_no_restart(uchar **p, int McusPerRow, int BlocksPerMcu, void (*decode_block_func)(int , int16*))
{
    //int Xoffset[JPGD_MAXCOMPONENTS];
    uchar *dp[JPGD_MAXCOMPONENTS];
    //uchar *pp;
//   int* pMcuOrg;
//    int  McuOrgReverse[JPGD_MAXBLOCKSPERMCU];
    int  mcu_block;


    const int block_size = ac_coeffs[0]->block_size;
    //memset(Xoffset, 0, sizeof(Xoffset));

    dp[0] = p[0];
    dp[1] = p[1];
    dp[2] = p[2];

//    mcu_block = BlocksPerMcu;
//    pMcuOrg = Jpg_gMcuOrg;
//    do
//    {
//     McuOrgReverse[--mcu_block] = *pMcuOrg++;
//    }while(mcu_block);

    //for (mcu_col = 0; mcu_col < Jpg_gMcusPerCol; mcu_col++)
    while (McusPerRow--)
    {
        int block_x_mcu_ofs = 0, block_y_mcu_ofs = 0;

        //if ((Jpg_gRestartInterval) && (Jpg_gRestartLeft == 0))
        //JpgDecProcessRestart();

        //for (mcu_block = 0; mcu_block < BlocksPerMcu; mcu_block++)
        mcu_block = BlocksPerMcu;
        while (mcu_block--)
        {
            //int component_id = Jpg_gMcuOrg[mcu_block];
            int component_id = McuOrgReverse[mcu_block];

            if (Jpg_gCompVertSamp[component_id] == 1)
            {
                decode_block_func(component_id, (int16*)(dp[component_id]));
                dp[component_id] += block_size;
            }
            else
            {
                uchar* pp;
                pp = dp[component_id];
                if (block_x_mcu_ofs)
                    pp += block_size;
                if (block_y_mcu_ofs)
                    pp += ac_coeffs[component_id]->macro_row_size;

                decode_block_func(component_id, (int16*)(pp));
                if (++block_x_mcu_ofs == Jpg_gCompHoriSamp[component_id])
                {
                    block_x_mcu_ofs = 0;

                    if (++block_y_mcu_ofs == Jpg_gCompVertSamp[component_id])
                    {
                        block_y_mcu_ofs = 0;
                        //Xoffset[component_id] += (block_size<<(Jpg_gCompHoriSamp[component_id] - 1));
                        dp[component_id] += (block_size << (Jpg_gCompHoriSamp[component_id] - 1));
                    }
                }
            }
        }

        //Jpg_gRestartLeft--;
    }
}
int decode_scan(void (*decode_block_func)(int , int16*))
{
    int mcu_col;
    uchar *p[JPGD_MAXCOMPONENTS];
    //uchar *pp;
    //int Yoffset[JPGD_MAXCOMPONENTS];
    //const int block_size = ac_coeffs[0]->block_size;

    p[0] = ac_coeffs[0]->Pdata;
    p[1] = ac_coeffs[1]->Pdata;
    p[2] = ac_coeffs[2]->Pdata;

    for (mcu_col = 0; mcu_col < Jpg_gMcusPerCol; mcu_col++)
    {
        int component_num, component_id;
#if 0
//        int Xoffset[JPGD_MAXCOMPONENTS];
//        memset(Xoffset, 0, sizeof(Xoffset));
//
//        for (mcu_row = 0; mcu_row < Jpg_gMcusPerRow; mcu_row++)
//        {
//            int block_x_mcu_ofs = 0, block_y_mcu_ofs = 0;
//
//            if ((Jpg_gRestartInterval) && (Jpg_gRestartLeft == 0))
//                JpgDecProcessRestart();
//
//            for (mcu_block = 0; mcu_block < Jpg_gBlocksPerMcu; mcu_block++)
//            {
//                component_id = Jpg_gMcuOrg[mcu_block];
//
//                pp = (p[component_id] + Xoffset[component_id]);
//                if (block_x_mcu_ofs)
//                    pp += block_size;
//                if (block_y_mcu_ofs)
//                    pp += ac_coeffs[component_id]->macro_row_size;
//
//                decode_block_func(component_id, (int16*)(pp));
//#if 1
//                if (Jpg_gCompVertSamp[component_id] == 1)
//                {
//                    Xoffset[component_id] += block_size;
//                }
//                else
//                {
//                    if (++block_x_mcu_ofs == Jpg_gCompHoriSamp[component_id])
//                    {
//                        block_x_mcu_ofs = 0;
//
//                        if (++block_y_mcu_ofs == Jpg_gCompVertSamp[component_id])
//                        {
//                            block_y_mcu_ofs = 0;
//                            Xoffset[component_id] += (block_size << (Jpg_gCompHoriSamp[component_id] - 1));
//                        }
//                    }
//                }
//#else
//                Xoffset[component_id] += CalOffset(component_id, &block_x_mcu_ofs, &block_y_mcu_ofs, block_size);
//#endif
//
//            }
//
//            Jpg_gRestartLeft--;
//        }
#else
        if (Jpg_gRestartInterval)
        {
            int c = decode_scan_macro_row(p, Jpg_gMcusPerRow, Jpg_gBlocksPerMcu, decode_block_func);
            if (c)
                return c;
        }
        else
        {
            decode_scan_macro_row_no_restart(p, Jpg_gMcusPerRow, Jpg_gBlocksPerMcu, decode_block_func);
        }
#endif
        {
            for (component_num = 0; component_num < Jpg_gCompInScan; component_num++)
            {
                component_id = Jpg_gCompList[component_num];
                p[component_id] += (ac_coeffs[component_id]->macro_row_size << (Jpg_gCompVertSamp[component_id] - 1));
            }
        }
    }
    return 0;
}
//---------------------------------------------------------------------------
int decode_scan_one_component(void (*decode_block_func)(int , int16*))
{
    int mcu_row, mcu_col/*, mcu_block*/;
    //int block_x_mcu[JPGD_MAXCOMPONENTS], block_y_mcu[JPGD_MAXCOMPONENTS];
    //int block_x_mcu, block_y_mcu;
    const int /*component_num,*/ component_id = Jpg_gMcuOrg[0];
    uchar *pp;
    uchar *p;
    const int block_size = ac_coeffs[0]->block_size;
    //int blocks_per_mcu/*,mcus_per_row,mcus_per_col*/;

    //blocks_per_mcu = Jpg_gBlocksPerMcu;
    //mcus_per_row = Jpg_gMcusPerRow;

// memset(block_y_mcu, 0, sizeof(block_y_mcu));
//block_y_mcu = 0;
    p = ac_coeffs[component_id]->Pdata;

    //for (mcu_col = 0; mcu_col < Jpg_gMcusPerCol; mcu_col++)
    mcu_col = Jpg_gMcusPerCol;
    while (mcu_col--)
    {
        pp = p;
        //memset(block_x_mcu, 0, sizeof(block_x_mcu));
        //block_x_mcu = 0;

        //for (mcu_row = 0; mcu_row < Jpg_gMcusPerRow; mcu_row++)
        mcu_row = Jpg_gMcusPerRow;
        //component_id = Jpg_gMcuOrg[0];
        while (mcu_row--)
        {
            //int block_x_mcu_ofs = 0, block_y_mcu_ofs = 0;


            if ((Jpg_gRestartInterval) && (Jpg_gRestartLeft == 0))
            {
                int c = JpgDecProcessRestart();
                if (c)
                    return c;
            }


            //for (mcu_block = 0; mcu_block < Jpg_gBlocksPerMcu; mcu_block++)
            //for (mcu_block = 0; mcu_block < blocks_per_mcu; mcu_block++)
            {
                //component_id = Jpg_gMcuOrg[mcu_block];
                //component_id = Jpg_gMcuOrg[0];

                //decode_block_func(component_id,
                //block_x_mcu[component_id] + block_x_mcu_ofs,
                //block_y_mcu[component_id] + block_y_mcu_ofs);
                //p = coeff_buf_getp(ac_coeffs[component_id],block_x_mcu,block_y_mcu);
                //decode_block_func(component_id,block_x_mcu,block_y_mcu);
                decode_block_func(component_id, (int16*)pp);

                //if (Jpg_gCompInScan== 1)
                //block_x_mcu[component_id]++;
                //block_x_mcu++;
                pp += block_size;
#if 0
                else
                {
                    //int tmpH,tmpV;
                    //tmpH = Jpg_gCompHoriSamp[component_id];
                    //tmpV = Jpg_gCompVertSamp[component_id];

                    if (++block_x_mcu_ofs == Jpg_gCompHoriSamp[component_id])
                    {
                        block_x_mcu_ofs = 0;

                        if (++block_y_mcu_ofs == Jpg_gCompVertSamp[component_id])
                        {
                            block_y_mcu_ofs = 0;

                            block_x_mcu[component_id] += Jpg_gCompHoriSamp[component_id];
                        }
                    }
                }
#endif
            }

            Jpg_gRestartLeft--;
        }
#if 0
        if (Jpg_gCompInScan == 1)
            block_y_mcu[Jpg_gCompList[0]]++;
        else
        {
            for (component_num = 0; component_num < Jpg_gCompInScan; component_num++)
            {
                component_id = Jpg_gCompList[component_num];

                block_y_mcu[component_id] += Jpg_gCompVertSamp[component_id];
            }
        }
#endif
        //block_y_mcu++;
        p += ac_coeffs[component_id]->macro_row_size;
    }
    return 0;
}

// Decode a progressively encoded image.
int JpgDecInitProgressive(void)
{
    int i;

    if (Jpg_gComponentsInFrame == 4)
        //return(JPGD_UNSUPPORTED_COLORSPACE);
        return JPGD_UNSUPPORTED_COLORSPACE;

    // Allocate the coefficient buffers.
    for (i = 0; i < Jpg_gComponentsInFrame; i++)
    {
        //dc_coeffs[i] = coeff_buf_open(Jpg_gMaxMcusPerRow* Jpg_gCompHoriSamp[i],
        //Jpg_gMaxMcusPerCol * Jpg_gCompVertSamp[i], 1, 1);
#ifdef MINUS_ROOM
        ac_coeffs[i] = coeff_buf_open(Jpg_gMaxMcusPerRow * Jpg_gCompHoriSamp[i],
                                      Jpg_gMaxMcusPerCol * Jpg_gCompVertSamp[i], Jpg_gIdctSize, Jpg_gIdctSize);
        if (ac_coeffs[i] == NULL)
            return JPGD_NOTENOUGHMEM;
#else
        ac_coeffs[i] = coeff_buf_open(Jpg_gMaxMcusPerRow * Jpg_gCompHoriSamp[i],
                                      Jpg_gMaxMcusPerCol * Jpg_gCompVertSamp[i], 8, 8);

        if (ac_coeffs[i] == NULL)
            return JPGD_NOTENOUGHMEM;
#endif
    }

    for (; ;)
    {
        int dc_only_scan, refinement_scan;
        //Pdecode_block_func decode_block_func;
        //void (*decode_block_func)();

        if (!JpgDecInitScan())
            break;


        dc_only_scan    = (Jpg_gSpectralStart == 0);
        refinement_scan = (Jpg_gSuccessiveHigh != 0);

        if ((Jpg_gSpectralStart > Jpg_gSpectralEnd) || (Jpg_gSpectralEnd > 63))
            //return(JPGD_BAD_SOS_SPECTRAL);
            return JPGD_BAD_SOS_SPECTRAL;

        if (dc_only_scan)
        {
            if (Jpg_gSpectralEnd)
                //return(JPGD_BAD_SOS_SPECTRAL);
                return JPGD_BAD_SOS_SPECTRAL;
        }
        else if (Jpg_gCompInScan != 1) /* AC scans can only contain one component */
            //return(JPGD_BAD_SOS_SPECTRAL);
            return JPGD_BAD_SOS_SPECTRAL;

        if ((refinement_scan) && (Jpg_gSuccessiveLow != Jpg_gSuccessiveHigh - 1))
            //return(JPGD_BAD_SOS_SUCCESSIVE);
            return JPGD_BAD_SOS_SUCCESSIVE;

        if (dc_only_scan)
        {
            if (refinement_scan)
                decode_block_func = decode_block_dc_refine;
            else
                decode_block_func = decode_block_dc_first;
        }
        else
        {
#ifdef skip_ac_scan
            if (Jpg_gIdctSize == ONEONE)
            {
                if (skip_to_next_scan())
                    goto NEXT;
                else
                    break;
            }
            else
            {
                if (refinement_scan)
                    decode_block_func = decode_block_ac_refine;
                else
                {
                    decode_block_func = decode_block_ac_first;
                }
            }
#else
            if (refinement_scan)
                decode_block_func = decode_block_ac_refine;
            else
            {
                if (Jpg_gIdctSize == ONEONE)
                    decode_block_func = decode_block_ac_first1x1;
                else
                    decode_block_func = decode_block_ac_first;
            }
#endif
        }
        if ((decode_block_func != decode_block_ac_refine) || (Jpg_gIdctSize == 8))
        {
            if (Jpg_gCompInScan == 1)
            {
                int c = decode_scan_one_component(decode_block_func);
                if (c)
                    return c;
            }
            else
            {
                int c = decode_scan(decode_block_func);
                if (c)
                    return c;
            }
        }

        JpgDecGetBits_2(Jpg_gBitsLeft & 7);

NEXT:
        Jpg_gBitsLeft = 16;
        Jpg_gBitBuf = 0;

        JpgDecSkipBits_1(16);
        JpgDecSkipBits_1(16);
    }

    Jpg_gCompInScan = Jpg_gComponentsInFrame;

    for (i = 0; i < Jpg_gComponentsInFrame; i++)
        Jpg_gCompList[i] = i;

    JpgDecCalcMcuBlockOrder();
    return 0;
}
//------------------------------------------------------------------------------
// Loads and dequantizes the next row of (already decoded) coefficients.
// Progressive images only.

void inverseQ_idct8x8(int16* p, int16* pAC, int16* Pdst_ptr, int16* q)
{
    int i;
    memcpy(&p[0], &pAC[0], 64 * sizeof(int16));

    for (i = 63; i > 0; i--)//skip 0 in bottom-right corner
        if (p[ZAG[i]])
            break;


    for (; i >= 0; i--) //inverse quantize to non-zero point
        if (p[ZAG[i]])
            p[ZAG[i]] *= q[i];

    JpgDecIdct(p, Pdst_ptr);
}

void inverseQ_idct4x4(int16* p, int16* pAC, int16* Pdst_ptr, int16* q)
{
    int i, j;
    for (i = 0;i < 4;i++)
    {
        for (j = 0;j < 4;j++)
        {
            p[j] = *pAC++;
        }
        p += 8;
    }
    p -= 32;

    for (i = 24; i > 0; i--)//skip 0 in bottom-right corner
        if (p[ZAG[i]])
            break;


    for (; i >= 0; i--) //inverse quantize to non-zero point
        if (p[ZAG[i]])
            p[ZAG[i]] *= q[i];

    JpgDecIdct4x4(/*block_seg[row_block]*/p, Pdst_ptr);
}
void inverseQ_idct2x2(int16* p, int16* pAC, int16* Pdst_ptr, int16* q)
{
    int i;
    p[0] = *pAC++;
    p[1] = *pAC++;
    p[8] = *pAC++;
    p[9] = *pAC++;

    for (i = 4; i > 0; i--)//skip 0 in bottom-right corner
        if (p[ZAG[i]])
            break;


    for (; i >= 0; i--) //inverse quantize to non-zero point
        if (p[ZAG[i]])
            p[ZAG[i]] *= q[i];

    JpgDecIdct2x2(p, Pdst_ptr);
}
void inverseQ_idct1x1(int16* p, int16* pAC, int16* Pdst_ptr, int16* q)
{
    p[0] = pAC[0] * q[0];
    JpgDecIdct1x1(p, Pdst_ptr);
}
void load_next_row(uchar **py)//output to block_seg
{
    int16 *p, *q;
    int mcu_row, mcu_block;
    //int component_num, component_id;
    //int block_x_mcu[JPGD_MAXCOMPONENTS];
    //int16 *Pdst_ptr,*pAC/*,*pDC*/;
    //volatile int debugNubMCUs = 0;
    int16 temp_block[64];
    uchar *dp[JPGD_MAXCOMPONENTS];
    const int block_size = ac_coeffs[0]->block_size;

    //memset(block_x_mcu, 0, JPGD_MAXCOMPONENTS * sizeof(int));
    memset(temp_block, 0, 64*sizeof(int16));

    dp[0] = py[0];
    dp[1] = py[1];
    dp[2] = py[2];

    for (mcu_row = 0; mcu_row < Jpg_gMcusPerRow; mcu_row++)
    {
        int16* Pdst_ptr = Jpg_gTempMcuBlock;
        int block_x_mcu_ofs = 0;
        int block_y_mcu_ofs = 0;

        for (mcu_block = 0; mcu_block < Jpg_gBlocksPerMcu; mcu_block++)
        {
            int component_id = Jpg_gMcuOrg[mcu_block];

            //memset(block_seg[row_block],0,64*sizeof(int16));
            //p = block_seg[row_block];

            int16* pAC = (int16*)(dp[component_id]);
            p = temp_block;
            q = Jpg_pQuant[Jpg_gCompQuant[component_id]];


            //-------------------------------------
            //move forward
            //if (Jpg_gCompInScan != 1)
            if (Jpg_gCompVertSamp[component_id] == 1)
            {
                inverseQ_idct(p, pAC, Pdst_ptr, q);
                dp[component_id] += block_size;
            }
            else
            {
                if (block_x_mcu_ofs)
                    pAC += (block_size >> 1);
                if (block_y_mcu_ofs)
                    pAC += (ac_coeffs[component_id]->macro_row_size >> 1);

                inverseQ_idct(p, pAC, Pdst_ptr, q);
                if (++block_x_mcu_ofs == Jpg_gCompHoriSamp[component_id])
                {
                    block_x_mcu_ofs = 0;

                    if (++block_y_mcu_ofs == Jpg_gCompVertSamp[component_id])
                    {
                        block_y_mcu_ofs = 0;

                        //block_x_mcu[component_id] += Jpg_gCompHoriSamp[component_id];
                        dp[component_id] += (block_size << (Jpg_gCompHoriSamp[component_id] - 1));
                    }
                }
            }
            Pdst_ptr += 64;//advance the output pointer to next block
//#if 0
//            else
//            {
//                //block_x_mcu[component_id]++;
//                dp[component_id] += block_size;
//            }
//#endif
        }
        JpgDecOutputOneMcu2();
    }
//#if 0
//    if (Jpg_gCompInScan == 1)
//        block_y_mcu[Jpg_gMcuOrg[0]]++;
//    else
//    {
//        for (component_num = 0; component_num < Jpg_gCompInScan; component_num++)
//        {
//            component_id = Jpg_gCompList[component_num];
//
//            block_y_mcu[component_id] += Jpg_gCompVertSamp[component_id];
//        }
//    }
//#endif
}
void load_next_row_one_com(uchar **py)//output to block_seg
{
    int16 *p, *q;
    int mcu_row, mcu_block;
    //int component_num, component_id;
    //int block_x_mcu[JPGD_MAXCOMPONENTS];
    //int16 *Pdst_ptr,*pAC/*,*pDC*/;
    //volatile int debugNubMCUs = 0;
    int16 temp_block[64];
    uchar *dp[JPGD_MAXCOMPONENTS];
    const int block_size = ac_coeffs[0]->block_size;

    //memset(block_x_mcu, 0, JPGD_MAXCOMPONENTS * sizeof(int));
    memset(temp_block, 0, 64*sizeof(int16));

    dp[0] = py[0];
    dp[1] = py[1];
    dp[2] = py[2];

    for (mcu_row = 0; mcu_row < Jpg_gMcusPerRow; mcu_row++)
    {
        int16* Pdst_ptr = Jpg_gTempMcuBlock;
        int block_x_mcu_ofs = 0;
        int block_y_mcu_ofs = 0;

        for (mcu_block = 0; mcu_block < Jpg_gBlocksPerMcu; mcu_block++)
        {
            int component_id = Jpg_gMcuOrg[mcu_block];

            //memset(block_seg[row_block],0,64*sizeof(int16));
            //p = block_seg[row_block];


            int16* pAC = (int16*)(dp[component_id]);

            p = temp_block;
            q = Jpg_pQuant[Jpg_gCompQuant[component_id]];


            inverseQ_idct(p, pAC, Pdst_ptr, q);

            Pdst_ptr += 64;//advance the output pointer to next block
            //-------------------------------------
            //move forward
#if 0
            if (Jpg_gCompInScan != 1)
            {
                if (++block_x_mcu_ofs == Jpg_gCompHoriSamp[component_id])
                {
                    block_x_mcu_ofs = 0;

                    if (++block_y_mcu_ofs == Jpg_gCompVertSamp[component_id])
                    {
                        block_y_mcu_ofs = 0;

                        //block_x_mcu[component_id] += Jpg_gCompHoriSamp[component_id];
                        dp[component_id] += (block_size << (Jpg_gCompHoriSamp[component_id] - 1));
                    }
                }
            }
            else
#endif
            {
                //block_x_mcu[component_id]++;
                dp[component_id] += block_size;
            }
        }
        JpgDecOutputOneMcu2();
    }
}

#endif

int JpgDecCheckQuantTables(void)
{
    int i;

    for (i = 0; i < Jpg_gCompInScan; i++)
    {
        if (Jpg_pQuant[Jpg_gCompQuant[Jpg_gCompList[i]]] == NULL)
        {
            return(JPGD_UNDEFINED_QUANT_TABLE);
        }
    }
    return 0;
}

//------------------------------------------------------------------------------
// This method handles all errors.
// It could easily be changed to use C++ exceptions.
//void return(int status)
//{
//
//    JpgDecFreeAllBlocks();//释放所有alloc动态分配的空间
//    exit(status);
//
//}

//------------------------------------------------------------------------------
// Clear buffer to word values.
void JpgDecWordClear(void *p, ushort c, uint n)
{
    uint16 *ps = (uint16 *)p;
    while (n)
    {
        *ps++ = c;
        n--;
    }
}

//------------------------------------------------------------------------------
void JpgDecPrepInBuffer(void)
{
    int bytes_read;
    Jpg_pPinBufOfs = Jpg_pInBuf;

    bytes_read = JpgDecRead(Jpg_pInBuf, JPGD_INBUFSIZE);//JpgDecRead(Jpg_pInBuf + Jpg_gInBufLeft, JPGD_INBUFSIZE - Jpg_gInBufLeft, &Jpg_gEofFlag);

    if (bytes_read < JPGD_INBUFSIZE)//当数据不够JPGD_INBUFSIZE时，多填充64个结束标志
    {
        JpgDecWordClear(Jpg_pInBuf + bytes_read, 0xD9FF, 64);
    }
}

// Unconditionally frees all allocated blocks.
void JpgDecFreeAllBlocks(void)
{
    //if (Pstream)
    //{
    //  Pstream->detach();
    //  Pstream = NULL;
    //}

#if 0
    int  i;
    for (i = 0; i < JPGD_MAXBLOCKS; i++)
    {
		if(Jpg_pBlocks[i])
		{
           free(Jpg_pBlocks[i]);
           Jpg_pBlocks[i] = NULL;
		}
    }
#endif
}
#if 0
void* JpgDecAlloc(int n)
{
    int  i;
    void*  q;
    // Find a free slot. The number of allocated slots will
    // always be very low, so a linear search is good enough.
    for (i = 0; i < JPGD_MAXBLOCKS; i++)
    {
        if (Jpg_pBlocks[i] == NULL)
            break;
    }

    if (i == JPGD_MAXBLOCKS)
    {
        //return(JPGD_TOO_MANY_BLOCKS);
        //while(1);
        return NULL;
    }
    //if (gJpgProHuffCoeffBuf == NULL)
    {
        q = (void *)(malloc(n + 8));
    }
    //else
    {
        //q = gJpgProHuffCoeffBuf + gJpgProBufAllocOffset;
    }
    //gJpgProBufAllocOffset += (n + 8);

    if (q == NULL)
    {
        //return(JPGD_NOTENOUGHMEM);
        return NULL;
    }

    memset(q, 0, n + 8);

    Jpg_pBlocks[i] = q;

    // Round to qword boundry, to avoid misaligned accesses with MMX code
    return ((void *)(((uint)q + 7) & ~7));
}
#endif

//------------------------------------------------------------------------------
// This method throws back into the stream any bytes that where read
// into the bit buffer during initial marker scanning.
void JpgDecFixInBuffer(void)
{
    /* In case any 0xFF's where pulled into the buffer during marker scanning */

    if (Jpg_gBitsLeft == 16)
        JpgDecStuffChar((uint8)((Jpg_gBitBuf >> 16) & 0xFF));

    if (Jpg_gBitsLeft >= 8)
        JpgDecStuffChar((uint8)((Jpg_gBitBuf >> 24) & 0xFF));

    JpgDecStuffChar((uint8)(Jpg_gBitBuf & 0xFF));

    JpgDecStuffChar((uint8)((Jpg_gBitBuf >> 8) & 0xFF));

    {
        Jpg_gBitsLeft = 16;
        Jpg_gBitBuf = 0;//suspicious
        JpgDecGetBits_2(16);
        JpgDecGetBits_2(16);
    }
}
//------------------------------------------------------------------------------
// output mcu row to frame buffer
//#define UV_COM_MID
void JpgDecOutputOneMcu2(void)
{
    //long j,m,n;
    int16 j, m, n;
    int16* pSrc, *pDst;
    int16  *pSrcBlk;
    int8* pDstY, *pDstBlkY;
    int16 xEnd, yEnd, xEnd1;
    short Jpg_gDstXSizeExpand_word, Jpg_gDstXSizeExpand_word1;
//    unsigned long offset, len;
//    long dst_x_size_expand_word;

    pSrcBlk = Jpg_gTempMcuBlock;
    xEnd = (Jpg_gMaxMcuXSize2 >> 1);
    yEnd = (Jpg_gMaxMcuYSize2 >> 1);

    xEnd1= (Jpg_gMaxMcuXSize2 >> 2);
    
    Jpg_gDstXSizeExpand_word = (Jpg_gDstXSizeExpand >> 1);
    Jpg_gDstXSizeExpand_word1 = (Jpg_gDstXSizeExpand >> 2);
    //Jpg_gDstXSizeExpand_word = ((Jpg_gMaxMcuXSize2*mcu_procress_one_time)>>1);//word

    switch (Jpg_gScanType)
    {
        case JPGD_YH2V2:
            {
                //------------------y-------------------------
                //top
                pDstBlkY = Jpg_pPSampleY;
                for (m = 0; m < 2; m++)
                {
                    pSrc = pSrcBlk;
                    pDstY = pDstBlkY;
                    for (j = 0; j < yEnd; j++)
                    {
                        //for (n = 0; n < xEnd;n += 2)
                        for (n = 0; n < xEnd;n += 1)
                        {
                            //int16* srcTemp =  pSrc + n;
                            //int16 VarTmp = (*srcTemp) | ((*(srcTemp + 1))<<8);

                            //*(pDst + (n>>1)) = VarTmp;
                            *(pDstY + n) = (char)(*(pSrc + n));
                        }
                        pSrc += 8;
                        //pDstY += Jpg_gDstXSizeExpand_word;
                        pDstY += Jpg_gDstXSizeExpand;
                    }
                    pSrcBlk += 64;
                    pDstBlkY += (Jpg_gMaxMcuXSize2 >> 1);
                }

                //bottom
                pDstBlkY = Jpg_pPSampleY + (Jpg_gMaxMcuYSize2 * Jpg_gDstXSizeExpand >> 1);
                for (m = 0; m < 2; m++)
                {
                    pSrc = pSrcBlk;
                    pDstY = pDstBlkY;
                    for (j = 0; j < yEnd; j++)
                    {
                        //for (n = 0; n < xEnd;n += 2)
                        for (n = 0; n < xEnd;n += 1)
                        {
                            //int16* srcTemp =  pSrc + n;
                            //int16 VarTmp = (*srcTemp) | ((*(srcTemp + 1))<<8);

                            //*(pDst + (n>>1)) = VarTmp;
                            *(pDstY + n) = (char)(*(pSrc + n));
                        }
                        pSrc += 8;
                        //pDst += Jpg_gDstXSizeExpand_word;
                        pDstY += Jpg_gDstXSizeExpand;
                    }
                    pSrcBlk += 64;
                    pDstBlkY += (Jpg_gMaxMcuXSize2 >> 1);
                }

                //-----------------uv-------------------
                pDst = Jpg_pPSampleU;
                pSrc = pSrcBlk;
                for (j = 0; j < yEnd; j++)
                {
                    for (n = 0; n < xEnd; n++)
                    {
                        int16* srcTemp =  pSrc + n;
                        int16 VarTmp = (*srcTemp) | ((*(srcTemp + 64)) << 8);
                        *(pDst + n) = VarTmp;
                    }
                    pSrc += 8;
                    pDst += Jpg_gDstXSizeExpand_word;
                }
                pSrcBlk += 128;


                //Jpg_pPSampleY += xEnd;
                Jpg_pPSampleY += Jpg_gMaxMcuXSize2;
                Jpg_pPSampleU += xEnd;
                break;
            }
        case   JPGD_YH2V1:  //Y两块，UV各一块
            {
                pDstBlkY = Jpg_pPSampleY;
                //两个Y块
                for (m = 0; m < 2; m++)
                {
                    pSrc = pSrcBlk;
                    pDstY = pDstBlkY;
                    for (j = 0; j < (Jpg_gMaxMcuYSize2); j++)
                    {
                        //for (n = 0; n < xEnd; n += 2)
                        for (n = 0; n < xEnd; n += 1)
                        {
                            //int16* srcTemp =  pSrc + n;
                            //int16 VarTmp = (*srcTemp) | ((*(srcTemp + 1))<<8);

                            //*(pDst + (n>>1)) = VarTmp;
                            *(pDstY + n) = (char)(*(pSrc + n));
                        }
                        pSrc += 8;
                        //pDst += Jpg_gDstXSizeExpand_word;
                        pDstY += Jpg_gDstXSizeExpand;
                    }
//    pDstBlk += Jpg_gMaxMcuXSize2/4;
                    pDstBlkY += Jpg_gMaxMcuXSize2 / 2;

                    pSrcBlk += 64;
                }

                pDst = Jpg_pPSampleU;
                pSrc = pSrcBlk;
                //一个UV块
                for (j = 0; j < (Jpg_gMaxMcuYSize2); j += 2)
                    // j+=2 : 隔行取UV值 , 这样便丢掉了一行的UV数据, 变成了 YUV 420
                {
                    for (n = 0; n < xEnd; n++)
                    {
                        int16* srcTemp =  pSrc + n;
						int16  VarTmp;
#ifdef UV_COM_MID
						int16 vTemp;
						int16 uTemp;

						vTemp = (*(srcTemp + 64) + *(srcTemp + 64 + 8) )>>1;
						uTemp = (*(srcTemp) + *(srcTemp + 8) )>>1;
						VarTmp =  (uTemp) | (vTemp<<8);

#else
                         VarTmp  = (*srcTemp) | ((*(srcTemp + 64)) << 8);                        
#endif
                        *(pDst + n) = VarTmp;
                    }
                    pSrc += 16;//跳过一行，所以是加16而不是加8
                    pDst += Jpg_gDstXSizeExpand_word;
                }

                pSrcBlk += 128;

                //Jpg_pPSampleY += xEnd;
                Jpg_pPSampleY += Jpg_gMaxMcuXSize2;
                Jpg_pPSampleU += xEnd;
                break;
            }


        case JPGD_YH1V2:
            {
                pDstBlkY = Jpg_pPSampleY;
                for (m = 0; m < 2; m++)
                {
                    pDstY = pDstBlkY;
                    pSrc = pSrcBlk;
                    for (j = 0; j < yEnd; j++)
                    {
                        //pSrc = pSrcBlk + j*8;
                        //for (n = 0; n < (Jpg_gMaxMcuXSize2); n += 2)
                        for (n = 0; n < (Jpg_gMaxMcuXSize2); n += 1)
                        {
                            //int16* srcTemp =  pSrc + n;
                            //int16 VarTmp = (*srcTemp) | ((*(srcTemp + 1))<<8);

                            //*(pDst + (n>>1)) = VarTmp;
                            *(pDstY + n) = (char)(*(pSrc + n));
                        }
                        pSrc += 8;
                        //pDst += Jpg_gDstXSizeExpand_word;
                        pDstY += Jpg_gDstXSizeExpand;
                    }
                    pSrcBlk += 64;
                    //pDstBlk += yEnd * Jpg_gDstXSizeExpand_word;
                    pDstBlkY += yEnd * Jpg_gDstXSizeExpand;

                }

                //-----------------uv-------------------
                pDst = Jpg_pPSampleU;
                pSrc = pSrcBlk;
                for (j = 0; j < yEnd; j++)
                {
                    //for (n = 0; n < xEnd; n++)
                    for (n = 0; n < Jpg_gMaxMcuXSize2; n += 2)
                    {
                        int16* srcTemp =  pSrc + n;
						int16 VarTmp;
#ifdef UV_COM_MID					
						int16 vTemp;
						int16 uTemp;

                        vTemp = (*(srcTemp + 64) + *(srcTemp + 65) )>>1;
                        uTemp = (*(srcTemp) + *(srcTemp + 1) )>>1;	
						VarTmp =  (uTemp) | (vTemp<<8);
#else
                        VarTmp = (*srcTemp) | ((*(srcTemp + 64)) << 8);
#endif
                        *(pDst + (n >> 1)) = VarTmp;
                    }
                    pSrc += 8;
                    pDst += Jpg_gDstXSizeExpand_word;
                }
                pSrcBlk += 128;

                //Jpg_pPSampleY += xEnd;
                Jpg_pPSampleY += Jpg_gMaxMcuXSize2;
                //Jpg_pPSampleU += xEnd;
                if (Jpg_gIdctSize != ONEONE)
                    Jpg_pPSampleU += xEnd;
                else
                {
                    Jpg_pPSampleU += (TranverseStep);
                    TranverseStep ^= 1;
                }

                break;
            }

        case  JPGD_YH1V1:
            {
                pDstY = Jpg_pPSampleY;
                pSrc = pSrcBlk;
                for (j = 0; j < Jpg_gMaxMcuYSize2; j++)
                {
                    //for (n = 0; n < Jpg_gMaxMcuXSize2; n += 2)
                    for (n = 0; n < Jpg_gMaxMcuXSize2; n += 1)
                    {
                        //int16* srcTemp =  pSrc + n;
                        //int16 VarTmp = (*srcTemp) | ((*(srcTemp + 1))<<8);

                        //*(pDst + (n>>1)) = VarTmp;
                        *(pDstY + n) = (char)(*(pSrc + n));
                    }
                    //pDst += Jpg_gDstXSizeExpand_word;
                    pDstY += Jpg_gDstXSizeExpand;
                    pSrc += 8;
                }

                pSrcBlk += 64;

                pDst = Jpg_pPSampleU;
                pSrc = pSrcBlk;
                for (j = 0; j < Jpg_gMaxMcuYSize2; j += 2)
                {
                    for (n = 0; n < Jpg_gMaxMcuXSize2; n += 2)
                    {
						
                        int16* srcTemp =  pSrc + n;
						int16 VarTmp;
#ifdef UV_COM_MID
                        int16 vTemp;
                        int16 uTemp;
                        
                        vTemp = (*(srcTemp + 64) + *(srcTemp + 65) + *(srcTemp + 64 + 8)  + *(srcTemp + 65 + 8))>>2;
                        uTemp = (*(srcTemp) + *(srcTemp + 1) + *(srcTemp + 8)  + *(srcTemp + 1 + 8))>>2;

                        VarTmp =  (uTemp) | (vTemp<<8);
#else
                        VarTmp = (*srcTemp) | ((*(srcTemp + 64)) << 8);
#endif
                        *(pDst + (n >> 1)) = VarTmp;
                    }
                    pDst += Jpg_gDstXSizeExpand_word;
                    pSrc += 16;//跳过一行，所以是加16而不是加8
                    //pSrc += 8;//susipicious
                }

                //Jpg_pPSampleY += xEnd;
                Jpg_pPSampleY += Jpg_gMaxMcuXSize2;
                if (Jpg_gIdctSize != ONEONE)
                    Jpg_pPSampleU += xEnd;
                else
                {
                    Jpg_pPSampleU += (TranverseStep);
                    TranverseStep ^= 1;
                }
                break;
            }
            case JPGD_YH4V1:
            {
                pDstBlkY = Jpg_pPSampleY;
                //四个Y块
                for (m = 0; m < 4; m++)
                {
                    pSrc = pSrcBlk;
                    pDstY = pDstBlkY;
                    for (j = 0; j < Jpg_gMaxMcuYSize2; j++)
                    {
                        for (n = 0; n < xEnd1; n += 1)
                        {
                            *(pDstY + n) = (char)(*(pSrc + n));
                        }
                        pSrc += 8;
                        pDstY += Jpg_gDstXSizeExpand;
                    }
                    
                    pDstBlkY += Jpg_gMaxMcuXSize2 / 4;
                    pSrcBlk += 64;
                }
            
                pDst = Jpg_pPSampleU;
                pSrc = pSrcBlk;
                for (j = 0; j < Jpg_gMaxMcuYSize2; j++)
                {
                    for (n = 0; n < xEnd1; n++)
                    {
                        uint16* srcTemp =  pSrc + n;
                        uint16 VarTmp = (*srcTemp) | ((*(srcTemp + 64)) << 8);
                        *(pDst + n) = VarTmp;
                        *(pDst + n + 1) = VarTmp;
                    }
                    pSrc += 8;
                    pDst += Jpg_gDstXSizeExpand_word1;
                }
                pSrcBlk += 128;

                Jpg_pPSampleY += Jpg_gMaxMcuXSize2;
                Jpg_pPSampleU += (xEnd1 * 2);
                break;
            }
    }
}





//------------------------------------------------------------------------------*/
// Restart interval processing.
int  JpgDecProcessRestart(void)
{
    int i, c;

    // Align to a byte boundry
    // FIXME: Is this really necessary? get_bits_2() never reads in markers!
    //JpgDecGetBits_2(Jpg_gBitsLeft & 7);

    // Let's scan a little bit to find the marker, but not _too_ far.
    // 1536 is a "fudge factor" that determines how much to scan.
    for (i = 1536; i > 0; i--)
    {
        if (JpgDecGetChar() == 0xFF)
        {
            if (i == 0)
                return(JPGD_BAD_RESTART_MARKER);
                
            break;
        }
    }

    //if (i == 0)
    //{
    //    return(JPGD_BAD_RESTART_MARKER);
    //}

    for (; i > 0; i--)
        if ((c = JpgDecGetChar()) != 0xFF)
            break;

    if (i == 0)
    {
        return(JPGD_BAD_RESTART_MARKER);
    }

    // Is it the expected marker? If not, something bad happened.
    if (c != (Jpg_gNextRestartNum + M_RST0))
    {
        return(JPGD_BAD_RESTART_MARKER);
    }

    // Reset each component's DC prediction values.
    memset(&Jpg_gLastDCValue, 0, Jpg_gComponentsInFrame * sizeof(uint));


    eob_run = 0;

    //DEBUG(" \n  %d \n", c);

    Jpg_gRestartLeft = Jpg_gRestartInterval;

    Jpg_gNextRestartNum = (Jpg_gNextRestartNum + 1) & 7;

    // Get the bit buffer going again...

    {
        Jpg_gBitsLeft = 16;
        JpgDecGetBits_2(16);
        JpgDecGetBits_2(16);
    }
    return 0;
}


//------------------------------------------------------------------------------
// Decodes and dequantizes the next row of coefficients.
#if 0
int JpgDecodeNext_8x8(void)
{
    int row_block = 0;
    int mcu_row;
    int mcu_block;
    int component_id;
    int r, s;
    int k;
    int16 *p;
    int16 *q;
    int16 *Pdst_ptr;
    int prev_num_set;
    Phuff_tables_t Ph;

    for (mcu_row = 0; mcu_row < Jpg_gMcusPerRow; mcu_row++)
    {
        Pdst_ptr = Jpg_gTempMcuBlock;
        if ((Jpg_gRestartInterval) && (Jpg_gRestartLeft == 0))
            JpgDecProcessRestart();

        for (mcu_block = 0; mcu_block < Jpg_gBlocksPerMcu; mcu_block++)
        {
            component_id = Jpg_gMcuOrg[mcu_block];
            memset(Jpg_gTempBlock, 0, 64*sizeof(int16));


            p = Jpg_gTempBlock;
            q = Jpg_pQuant[Jpg_gCompQuant[component_id]];


            if ((s = JpgDecHuffDecode(Jpg_gHuffman[Jpg_gCompDCTable[component_id]])) != 0)
            {
                r = JpgDecGetBits_2(s);
                s = HUFF_EXTEND_TBL(r, s);
            }

            Jpg_gLastDCValue[component_id] = (s += Jpg_gLastDCValue[component_id]);

            p[0] = s * q[0];

            prev_num_set = Jpg_gBlockMaxZagSet[row_block];

            Ph = Jpg_gHuffman[Jpg_gCompACTable[component_id]];

            for (k = 1; k < 64; k++)
            {
                s = JpgDecHuffDecode(Ph);

                r = s >> 4;
                s &= 15;

                if (s)
                {
                    if (r)
                    {
                        //if ((k + r) > 63)
                        // r = 63 - r;
                        // terminate(JPGD_DECODE_ERROR);

                        if (k < prev_num_set)
                        {
                            int n = min(r, prev_num_set - k);
                            int kt = k;
                            while (n--)
                                p[ZAG[kt++]] = 0;
                        }

                        k += r;
                    }

                    r = JpgDecGetBits_2(s);
                    s = HUFF_EXTEND_TBL(r, s);

                    //assert(k < 64);

                    p[ZAG[k]] = s * q[k];
                    // p[ZAG[k]] = s ;
                }
                else
                {
                    if (r == 15)
                    {
                        if ((k + 15) > 63)
                        {
                            return(JPGD_DECODE_ERROR);
                        }

                        if (k < prev_num_set)
                        {
                            int n = min(16, prev_num_set - k);  //bugfix Dec. 19, 2001 - was 15!
                            int kt = k;
                            while (n--)
                                p[ZAG[kt++]] = 0;
                        }

                        k += 15;
                    }
                    else
                    {
                        //while (k < 64)
                        //  p[ZAG[k++]] = 0;

                        break;
                    }
                }
            }

            if (k < prev_num_set)
            {
                int kt = k;
                while (kt < prev_num_set)
                    p[ZAG[kt++]] = 0;
            }
// fwrite(Jpg_gTempBlock,sizeof(short),64,test);
// fflush(test);
            // Jpg_gBlockMaxZagSet[row_block] = k;
            row_block++;
            JpgDecIdct(Jpg_gTempBlock, Pdst_ptr);
            Pdst_ptr += 64;
        }
        JpgDecOutputOneMcu2();
        Jpg_gRestartLeft--;
    }
    return 0;
}
#else
int JpgDecodeNext_8x8(void)
{
	int row_block = 0;
	int mcu_row;
	int mcu_block;
	int component_id;
	int r, s;
	int k;
	int16 *p;
	int16 *q;
	int16 *Pdst_ptr;
    int i;
    int32 *temp;
    
	Phuff_tables_t Ph;

	for (mcu_row = 0; mcu_row < Jpg_gMcusPerRow; mcu_row++)
	{
		Pdst_ptr = Jpg_gTempMcuBlock;
		if ((Jpg_gRestartInterval) && (Jpg_gRestartLeft == 0))
			JpgDecProcessRestart();

		for (mcu_block = 0; mcu_block < Jpg_gBlocksPerMcu; mcu_block++)
		{
			component_id = Jpg_gMcuOrg[mcu_block];
			
			//memset(Jpg_gTempBlock, 0, 64*sizeof(int16));  //占用 800 clk
			#if 1
			temp = (int*)Jpg_gTempBlock;	  //占用 31 clk  
			for(i = 0; i < 4; i++)  //占用 63 or 67 clk
			{
			    temp[i*8] = 0;
			    temp[i*8 + 1] = 0;
			    temp[i*8 + 2] = 0;
			    temp[i*8 + 3] = 0;
			    temp[i*8 + 4] = 0;
			    temp[i*8 + 5] = 0;
			    temp[i*8 + 6] = 0;
			    temp[i*8 + 7] = 0;
			}
			#else
	        //rk_count_clk_start();
			ImageMemSet((int*)Jpg_gTempBlock, 0, (64*sizeof(int16))/(sizeof(int) * 8)); //占用 100 clk
    	    //rk_count_clk_end();
			#endif
			    
			p = Jpg_gTempBlock;
			q = Jpg_pQuant[Jpg_gCompQuant[component_id]];


			if ((s = JpgDecHuffDecode(Jpg_gHuffman[Jpg_gCompDCTable[component_id]])) != 0)
			{
				r = JpgDecGetBits_2(s);
				s = HUFF_EXTEND_TBL(r, s);
			}

			Jpg_gLastDCValue[component_id] = (s += Jpg_gLastDCValue[component_id]);

			p[0] = s * q[0];


			Ph = Jpg_gHuffman[Jpg_gCompACTable[component_id]];

			for (k = 1; k < Jpg_gNumCoffs; k++)
			{
				s = JpgDecHuffDecode(Ph);

				r = s >> 4;
				s &= 15;

				if (s)
				{
					if ((k += r) > 63)
						return JPGD_DECODE_ERROR;

					r = JpgDecGetBits_2(s);
					s = HUFF_EXTEND_TBL(r, s);


					p[ZAG[k]] = s * q[k];
				}
				else
				{
					if (r == 15)
					{
						if ((k += 15) > 63)
							return JPGD_DECODE_ERROR;
					}
					else
					{
						goto HUFF_DECODE_END;
					}
				}
			}

			for (; k < 64; k++)
			{
				s = JpgDecHuffDecode(Ph);

				r = s >> 4;  //0的个数
				s &= 15; //bit长度

				if (s)
				{
					if ((k += r) > 63)	//增加这个可以去掉存储AC系数
						return JPGD_DECODE_ERROR;

					JpgDecSkipBits_2(s);  //增加这个可以去掉存储AC系数
				}
				else
				{
					if (r == 15)
					{
						if ((k += 15) > 63)  //增加这个可以去掉存储AC系数
							return JPGD_DECODE_ERROR;
					}
					else
					{
						break;
					}
				}
			}
HUFF_DECODE_END:

			row_block++;
			switch (Jpg_gIdctSize)
			{
				case EIGHTEIGHT:
					{
						JpgDecIdct(Jpg_gTempBlock, Pdst_ptr);
						break;
					}

				case FOURFOUR:
					{
						JpgDecIdct4x4(Jpg_gTempBlock, Pdst_ptr);
						break;
					}

				case TWOTWO:
					{
						JpgDecIdct2x2(Jpg_gTempBlock, Pdst_ptr);
						break;
					}

				case ONEONE:
					{
						JpgDecIdct1x1(Jpg_gTempBlock, Pdst_ptr);
						break;
					}
				default :
					break;
			}

			Pdst_ptr += 64;
		}
		JpgDecOutputOneMcu2();
		Jpg_gRestartLeft--;
	}
	return 0;
}

#endif


#if 0
int JpgDecodeNext_4x4(void)
{
    int row_block = 0;
    int mcu_row;
    int mcu_block;
    int component_id;
    int r, s;
    int k;
    int16 *p;
    int16 *q;
    int16 *Pdst_ptr;
    int prev_num_set;
    Phuff_tables_t Ph;
    //int i,j,temp;


    // Clearing the entire row block buffer can take a lot of time!
    // Instead of clearing the entire buffer each row, keep track
    // of the number of nonzero entries written to each block and do
    // selective clears.
    //memset(block_seg[0], 0, Jpg_gMcusPerRow * Jpg_gBlocksPerMcu * 64 * sizeof(BLOCK_TYPE));

    for (mcu_row = 0; mcu_row < Jpg_gMcusPerRow; mcu_row++)
    {
        Pdst_ptr = Jpg_gTempMcuBlock;
        if ((Jpg_gRestartInterval) && (Jpg_gRestartLeft == 0))
            JpgDecProcessRestart();

        for (mcu_block = 0; mcu_block < Jpg_gBlocksPerMcu; mcu_block++)
        {
            component_id = Jpg_gMcuOrg[mcu_block];
            memset(Jpg_gTempBlock, 0, 64*sizeof(int16));


            p = Jpg_gTempBlock;
            q = Jpg_pQuant[Jpg_gCompQuant[component_id]];


            if ((s = JpgDecHuffDecode(Jpg_gHuffman[Jpg_gCompDCTable[component_id]])) != 0)
            {
                r = JpgDecGetBits_2(s);
                s = HUFF_EXTEND_TBL(r, s);
            }

            Jpg_gLastDCValue[component_id] = (s += Jpg_gLastDCValue[component_id]);

            p[0] = s * q[0];

            prev_num_set = Jpg_gBlockMaxZagSet[row_block];

            Ph = Jpg_gHuffman[Jpg_gCompACTable[component_id]];

            for (k = 1; k < 64; k++)
            {
                s = JpgDecHuffDecode(Ph);

                r = s >> 4;
                s &= 15;

                if (s)
                {
                    if (r)
                    {
                        if ((k + r) > 63)
                        {
                            return(JPGD_DECODE_ERROR);
                        }

                        if (k < prev_num_set)
                        {
                            int n = min(r, prev_num_set - k);
                            int kt = k;
                            while (n--)
                                p[ZAG[kt++]] = 0;
                        }

                        k += r;
                    }

                    r = JpgDecGetBits_2(s);
                    s = HUFF_EXTEND_TBL(r, s);

                    //assert(k < 64);

                    p[ZAG[k]] = s * q[k];
                }
                else
                {
                    if (r == 15)
                    {
                        if ((k + 15) > 63)
                        {
                            return(JPGD_DECODE_ERROR);
                        }

                        if (k < prev_num_set)
                        {
                            int n = min(16, prev_num_set - k);  //bugfix Dec. 19, 2001 - was 15!
                            int kt = k;
                            while (n--)
                                p[ZAG[kt++]] = 0;
                        }

                        k += 15;
                    }
                    else
                    {
                        //while (k < 64)
                        //  p[ZAG[k++]] = 0;

                        break;
                    }
                }
            }

            if (k < prev_num_set)
            {
                int kt = k;
                while (kt < prev_num_set)
                    p[ZAG[kt++]] = 0;
            }

            Jpg_gBlockMaxZagSet[row_block] = k;
            row_block++;

            JpgDecIdct4x4(Jpg_gTempBlock, Pdst_ptr);
            Pdst_ptr += 64;
        }
        JpgDecOutputOneMcu2();
        Jpg_gRestartLeft--;
    }
    return 0;
}


int JpgDecodeNext_2x2(void)
{
    int row_block = 0;
    int mcu_row;
    int mcu_block;
    int component_id;
    int r, s;
    int k;
    int16 *p;
    int16 *q;
    int16 *Pdst_ptr;
    int prev_num_set;
    Phuff_tables_t Ph;


    for (mcu_row = 0; mcu_row < Jpg_gMcusPerRow; mcu_row++)
    {

        Pdst_ptr = Jpg_gTempMcuBlock;
        if ((Jpg_gRestartInterval) && (Jpg_gRestartLeft == 0))
            JpgDecProcessRestart();

        for (mcu_block = 0; mcu_block < Jpg_gBlocksPerMcu; mcu_block++)
        {
            component_id = Jpg_gMcuOrg[mcu_block];
            memset(Jpg_gTempBlock, 0, 64*sizeof(int16));


            p = Jpg_gTempBlock;
            q = Jpg_pQuant[Jpg_gCompQuant[component_id]];


            if ((s = JpgDecHuffDecode(Jpg_gHuffman[Jpg_gCompDCTable[component_id]])) != 0)
            {
                r = JpgDecGetBits_2(s);
                s = HUFF_EXTEND_TBL(r, s);
            }

            Jpg_gLastDCValue[component_id] = (s += Jpg_gLastDCValue[component_id]);

            p[0] = s * q[0];

            prev_num_set = Jpg_gBlockMaxZagSet[row_block];

            Ph = Jpg_gHuffman[Jpg_gCompACTable[component_id]];

            for (k = 1; k < 64; k++)
            {
                s = JpgDecHuffDecode(Ph);

                r = s >> 4;
                s &= 15;

                if (s)
                {
                    if (r)
                    {
                        if ((k + r) > 63)
                        {
                            return(JPGD_DECODE_ERROR);
                        }

                        if (k < prev_num_set)
                        {
                            int n = min(r, prev_num_set - k);
                            int kt = k;
                            while (n--)
                                p[ZAG[kt++]] = 0;
                        }

                        k += r;
                    }

                    r = JpgDecGetBits_2(s);
                    s = HUFF_EXTEND_TBL(r, s);

                    //assert(k < 64);

                    p[ZAG[k]] = s * q[k];
                }
                else
                {
                    if (r == 15)
                    {
                        if ((k + 15) > 63)
                        {
                            return(JPGD_DECODE_ERROR);
                        }

                        if (k < prev_num_set)
                        {
                            int n = min(16, prev_num_set - k);  //bugfix Dec. 19, 2001 - was 15!
                            int kt = k;
                            while (n--)
                                p[ZAG[kt++]] = 0;
                        }

                        k += 15;
                    }
                    else
                    {
                        //while (k < 64)
                        //  p[ZAG[k++]] = 0;

                        break;
                    }
                }
            }

            if (k < prev_num_set)
            {
                int kt = k;
                while (kt < prev_num_set)
                    p[ZAG[kt++]] = 0;
            }

            Jpg_gBlockMaxZagSet[row_block] = k;
            row_block++;
            JpgDecIdct2x2(Jpg_gTempBlock, Pdst_ptr);
            Pdst_ptr += 64;
        }
        JpgDecOutputOneMcu2();
        Jpg_gRestartLeft--;
    }
    return 0;
}


int JpgDecodeNext_1x1(void)
{
    int row_block = 0;
    int mcu_row;
    int mcu_block;
    int component_id;
    int r, s;
    int k;
    int16 *p;
    int16 *q;
    int16 *Pdst_ptr = (int16*)Jpg_pPsampleBuf;
//    int prev_num_set;
    Phuff_tables_t Ph;



    // Clearing the entire row block buffer can take a lot of time!
    // Instead of clearing the entire buffer each row, keep track
    // of the number of nonzero entries written to each block and do
    // selective clears.
    //memset(block_seg[0], 0, Jpg_gMcusPerRow * Jpg_gBlocksPerMcu * 64 * sizeof(BLOCK_TYPE));

    for (mcu_row = 0; mcu_row < Jpg_gMcusPerRow; mcu_row++)
    {
        Pdst_ptr = Jpg_gTempMcuBlock;
        if ((Jpg_gRestartInterval) && (Jpg_gRestartLeft == 0))
            JpgDecProcessRestart();

        for (mcu_block = 0; mcu_block < Jpg_gBlocksPerMcu; mcu_block++)
        {
            component_id = Jpg_gMcuOrg[mcu_block];
            memset(Jpg_gTempBlock, 0, 64*sizeof(int16));


            p = Jpg_gTempBlock;
            q = Jpg_pQuant[Jpg_gCompQuant[component_id]];


            if ((s = JpgDecHuffDecode(Jpg_gHuffman[Jpg_gCompDCTable[component_id]])) != 0)
            {
                r = JpgDecGetBits_2(s);
                s = HUFF_EXTEND_TBL(r, s);
            }

            Jpg_gLastDCValue[component_id] = (s += Jpg_gLastDCValue[component_id]);

            p[0] = s * q[0];

//            prev_num_set = Jpg_gBlockMaxZagSet[row_block];

            Ph = Jpg_gHuffman[Jpg_gCompACTable[component_id]];

            for (k = 1; k < 64; k++)
            {
                s = JpgDecHuffDecode(Ph);

                r = s >> 4;
                s &= 15;

                if (s)
                {
//                    if (r)
                    {
                        if ((k += r) > 63)
                        {
                            return(JPGD_DECODE_ERROR);
                        }

//                        if (k < prev_num_set)
//                        {
//                            int n = min(r, prev_num_set - k);
//                            int kt = k;
//                            while (n--)
//                                p[ZAG[kt++]] = 0;
//                        }

//                        k += r;
                    }

//                    r = JpgDecGetBits_2(s);
//                    s = HUFF_EXTEND_TBL(r, s);
//
//                    p[ZAG[k]] = s * q[k];
                    JpgDecSkipBits_2(s);
                }
                else
                {
                    if (r == 15)
                    {
                        if ((k += 15) > 63)
                        {
                            return(JPGD_DECODE_ERROR);
                        }

//                        if (k < prev_num_set)
//                        {
//                            int n = min(16, prev_num_set - k);  //bugfix Dec. 19, 2001 - was 15!
//                            int kt = k;
//                            while (n--)
//                                p[ZAG[kt++]] = 0;
//                        }
//
//                        k += 15;
                    }
                    else
                    {
                        break;
                    }
                }
            }

//            if (k < prev_num_set)
//            {
//                int kt = k;
//                while (kt < prev_num_set)
//                    p[ZAG[kt++]] = 0;
//            }

//            Jpg_gBlockMaxZagSet[row_block] = k;
            row_block++;
            JpgDecIdct1x1(Jpg_gTempBlock, Pdst_ptr);
            Pdst_ptr += 64;
        }
        JpgDecOutputOneMcu2();
        Jpg_gRestartLeft--;
    }
    return 0;
}
#endif
//--------------------------------------------------------------------

// Returns the next scan line.
// Returns JPGD_DONE if all scan lines have been returned.
// Returns JPGD_OKAY if a scan line has been returned.
// Returns JPGD_FAILED if an error occured.
int  baselineJpgDecodeMacroRow()
{
    //int jpeg_rows;

    //for (jpeg_rows = 0;jpeg_rows < Jpg_gMcusPerCol;jpeg_rows++)
    {
#if 0		
        switch (Jpg_gIdctSize)
        {

            case EIGHTEIGHT:
                {
                    JpgDecodeNext_8x8();
                    break;
                }

            case FOURFOUR:
                {
                    JpgDecodeNext_4x4();
                    break;
                }

            case TWOTWO:
                {
                    JpgDecodeNext_2x2();
                    break;
                }

            case ONEONE:
                {
                    JpgDecodeNext_1x1();
                    break;
                }
        }
#else
         JpgDecodeNext_8x8();
#endif
        // 重新初始化宏块行的YUV Buffer， 因为在JpgDecodeNext中已经被修改
#ifdef Y_WORD
        Jpg_pPSampleY = Jpg_pPsampleBuf;//一宏块行解码后置于运算缓冲中Y分量的位置
#else
        Jpg_pPSampleY = (int8*)Jpg_pPsampleBuf;
#endif
        Jpg_pPSampleU = (int16*)(Jpg_pPsampleBuf + (Jpg_gMaxBlocksPerMcu - 2) * Jpg_gBlockSize * Jpg_gMaxMcusPerRow);//一宏块行解码后置于运算缓冲中U分量的位置
//        Jpg_pPSampleV = Jpg_pPSampleU + Jpg_gBlockSize * Jpg_gMaxMcusPerRow ;//一宏块行解码后置于运算缓冲中V量的位置


        if (Jpg_gComponentsInFrame == 1)//只有一个颜色分量Y时,U、V置0x80值
        {
            int16 index;
            int16* PSampleUtemp = Jpg_pPSampleU;
//            int16* PSampleVtemp = Jpg_pPSampleV;
            for (index = 0; index < Jpg_gBlockSize * Jpg_gMaxMcusPerRow;index++)
            {
                *PSampleUtemp++ = (short)(0x8080);
                //*PSampleVtemp++ = 0x0080;
            }
        }


        JpgZoom2RGB888(Jpg_gDstXSize, Jpg_gDstYSize, currentMacroRow, (uint8*)Jpg_pPSampleY, (uint16*)Jpg_pPSampleU);
    }
    return 1;
}

static int getOneRowRGB(rgbtype* dst,uint8* srcY,uint16* srcUV,const int zoomwidth,const int zoomheight,int curRow,int curMacroRow)
{
	int column,zoomindsty,nextZoomindsty;
	int32 RR,GG,BB;
    int16 curSrcRow = curRow + Jpg_gMaxMcuYSize2 * curMacroRow;
	unsigned long zoomindstyInt = ((unsigned long)zoomheight << 16) / Jpg_gDstYSize + 1;

    zoomindsty = (curSrcRow * zoomindstyInt) >> 16;
    nextZoomindsty = ((curSrcRow + 1) * zoomindstyInt) >> 16;

    dst += ((ValidW - zoomwidth) >> 1);
    //如果该行数据不会被下一行覆盖，搬到sdram
    if (zoomindsty != nextZoomindsty || zoomindsty == (ValidH - 1))
    {
        //rgbtype* dst = (rgbtype*)(gJpgProOutput->ptr_output_buf) + zoomindsty * SCR_LENGTH + Blank;
        

        //if(((SCR_HEIGHT - zoomheight) >> 1) + zoomindsty >= SCR_HEIGHT)
        	//return GET_ONE_ROW_FAIL;
        //一行的yuv420torgb888
        for (column = 0;column < zoomwidth;column++)//紧凑存放
        {
            unsigned short tmpdiv = (column >> 1);
            int16 YY = *(srcY + column) - 16;
            uint16 VU = *(srcUV + (column >> 1));

            int16 UU = (VU & 0xFF) - 0x80;
            int16 VV = (VU >> 8) - 0x80;

            //UU = VV = 0;

            RR = ((RGB_Y * YY + R_V * VV) >> SHIFT_SCALE) + 4 ;
            GG = ((RGB_Y * YY - G_V * VV - G_U * UU) >> SHIFT_SCALE) + 4;
            BB = ((RGB_Y * YY + B_U * UU) >> SHIFT_SCALE) + 4;

#ifdef RGB16BITS
            *dst++ = (rgbtype) MK_RGB565(RR, GG, BB);
#else
            *dst++ = (rgbtype) MK_RGB888(RR, GG, BB);
#endif
        }
		return GET_ONE_ROW_OK;
    }
	return GET_ONE_ROW_CONTINUE;
}
int JpgDecInternal(IMAGE_DEC_INFO* pdec_info)
{
	int ret;
	int sta;
	rgbtype* dst = (rgbtype*)(pdec_info->ptr_output_buf);

	do
	{
		if(isyuvDataNotReady)
		{
		  	ret = baselineJpgDecodeMacroRow();
			isyuvDataNotReady = 0;
		    srcY = (uint8*)Jpg_pPSampleY;
			srcUV = (uint16*)Jpg_pPSampleU; 
			currentRow = 0;			
			ret = getOneRowRGB(dst,srcY,srcUV,ValidW,ValidH,currentRow,currentMacroRow);
			currentMacroRow++;
		}
	    else
	    {
		    ret = getOneRowRGB(dst,srcY,srcUV,ValidW,ValidH,currentRow,currentMacroRow);
	    }
		
		srcY += ValidW;
	    if ((currentRow&1))
	        srcUV += (ValidW >> 1);
		
		currentRow++;
		if(Jpg_gMaxMcuYSize2 == currentRow)
			isyuvDataNotReady = 1;

		if(currentMacroRow == Jpg_gMcusPerCol&&currentRow == Jpg_gMaxMcuYSize2)
		{			
		    sta = 0;
			goto exit;
	    }
	}while(GET_ONE_ROW_CONTINUE == ret);
	
	if(ret == GET_ONE_ROW_OK)
	  sta = 1;
	else
	  sta = 0;

exit:
	return sta;
}

#define MAX_BUFFER_LINE 4

// 取四行buffer用来缩放
static char RgbRowBuf[1024 * MAX_BUFFER_LINE * 4];
static int rowExist[MAX_BUFFER_LINE];
static int RgbRowIdx = 0;
static int FirstDec = 0;

int JpgDecode(IMAGE_DEC_INFO* pdec_info)
{
    uint32 overflow = 0;

	int sta;
	int k;

	int i = 0, j = 0;
	int bUpLeftLocation;

	float fX, fY;

	int iStepSrcImg = Jpg_gDstXSize;
	int iStepDstImg = pdec_info->OutputW;

	float fRateW, fRateH, fRate;
	int nWidth, nHeight;

	int iX, iY;

	int w, widthTag = 0;
	int xIndex, xIndex1, yIndex, yIndex1;
	int pSrcImgIndex00, pSrcImgIndex01, pSrcImgIndex10, pSrcImgIndex11;
	
	unsigned char bUpLeft, bUpRight, bDownLeft, bDownRight;
	unsigned char gUpLeft, gUpRight, gDownLeft, gDownRight;
	unsigned char rUpLeft, rUpRight, rDownLeft, rDownRight;

	unsigned char b, g, r;
	unsigned char *pSrcImg;
	unsigned short *pDstImg;

	fRateW = (float)pdec_info->OutputW/ Jpg_gDstXSize;	
	fRateH = (float)pdec_info->OutputH/ Jpg_gDstYSize;

	fRate = (float)min(fRateW, fRateH);
	fRateW = fRateH = fRate;
	nWidth = Jpg_gDstXSize;
	nHeight = Jpg_gDstYSize;

	if (FirstDec == 0) {
		int k;
		// 第一次将输入buffer读进来
		for (k = 0; k < 4; k++) {
			JpgDecInternal(pdec_info);
			memcpy(RgbRowBuf + Jpg_gDstXSize * k * 4, (char *)gJpgProOutput->ptr_output_buf, Jpg_gDstXSize << 2);
			rowExist[k] = k;
		}
		FirstDec = 1;
	}

	pSrcImg = RgbRowBuf;
	pDstImg = pdec_info->ScreenBuffer + ((SCR_LENGTH - gJpgProOutput->ValidW) >> 1);

	i = pdec_info->CurrentDecLine;
    w = nWidth * fRateW;
    
	fY = ((float)i) / fRateH;
	iY = (int)fY;
	bUpLeftLocation = (iY + 1);
	
    yIndex = iY * iStepSrcImg << 2;
    yIndex1 = (iY + 1) * iStepSrcImg << 2;

	for (j = 0; j < w; j++)
	{
		fX = ((float)j) / fRateW;		
		iX = (int)fX;

        xIndex = iX << 2;
        xIndex1 = (iX + 1) << 2;
        
        pSrcImgIndex00 = yIndex + xIndex; 
        pSrcImgIndex01 = yIndex + xIndex1; 
        pSrcImgIndex10 = yIndex1 + xIndex; 
        pSrcImgIndex11 = yIndex1 + xIndex1;

        if (iX + 1 > iStepSrcImg - 1)
            widthTag = 1;
        else
            widthTag = 0;
        
		if (bUpLeftLocation > rowExist[MAX_BUFFER_LINE - 1])
		{
			int k;
			int RemainBufferLine = 0;
			int SkipLine = 0;
			int DecStartLine = 0, DecEndLine = 0;

			DecEndLine = bUpLeftLocation;
			if (DecEndLine - rowExist[MAX_BUFFER_LINE - 1] > MAX_BUFFER_LINE - 1) {
				DecStartLine = DecEndLine - (MAX_BUFFER_LINE - 1);
				RemainBufferLine = 0;
			} else {
				DecStartLine = rowExist[MAX_BUFFER_LINE - 1] + 1;
				RemainBufferLine = MAX_BUFFER_LINE - (DecEndLine - rowExist[MAX_BUFFER_LINE - 1]);
			}
			SkipLine = DecStartLine - rowExist[MAX_BUFFER_LINE - 1] - 1;

			// 缓存buffer前，先跳过无用的行
			for (k = 0; k < SkipLine; k++) {
				if (rowExist[MAX_BUFFER_LINE - 1] + k <= ValidH - 1)
					JpgDecInternal(pdec_info);
			}

			// 将可以用的行往前拷贝
			for (k = 0; k < RemainBufferLine; k++)
			{
				memcpy(RgbRowBuf + Jpg_gDstXSize * k * 4, RgbRowBuf + Jpg_gDstXSize * (k + MAX_BUFFER_LINE - RemainBufferLine) * 4, Jpg_gDstXSize << 2);
				rowExist[k] = DecStartLine - RemainBufferLine + k;
			}

			// 解码余下的行
			for (k = 0; k < (DecEndLine - DecStartLine + 1); k++)
			{
				if (DecStartLine + k <= ValidH - 1) {
				JpgDecInternal(pdec_info);
				memcpy(RgbRowBuf + Jpg_gDstXSize * (RemainBufferLine + k) * 4, (char*)gJpgProOutput->ptr_output_buf, Jpg_gDstXSize << 2);
				} else if (RemainBufferLine > 0 || k > 0) {
					int PreviousLine = RemainBufferLine + k - 1;

					// 如果请求解码的行超出范围，则直接拷贝上一行的有效数据作为当前行的数据
					memcpy(RgbRowBuf + Jpg_gDstXSize * (RemainBufferLine + k) * 4, RgbRowBuf + Jpg_gDstXSize * (PreviousLine) * 4, Jpg_gDstXSize << 2);
				} else {
					goto dec_line_end;
				}

				rowExist[RemainBufferLine + k] = DecStartLine + k;
			}
		}
		
		pSrcImg = RgbRowBuf - rowExist[0] * nWidth * 4;
			
		bUpLeft = pSrcImg[pSrcImgIndex00 + 3];
		bUpRight = pSrcImg[pSrcImgIndex01 + 3];
		if (widthTag)
			bUpRight = bUpLeft;

		bDownLeft = pSrcImg[pSrcImgIndex10 + 3];
		bDownRight = pSrcImg[pSrcImgIndex11 + 3];
		if (widthTag)
			bDownRight = bDownLeft;

		gUpLeft = pSrcImg[pSrcImgIndex00 + 2];
		gUpRight = pSrcImg[pSrcImgIndex01 + 2];
		if (widthTag)
			gUpRight = gUpLeft;

		gDownLeft = pSrcImg[pSrcImgIndex10 + 2];
		gDownRight = pSrcImg[pSrcImgIndex11 + 2];
		if (widthTag)
			gDownRight = gDownLeft;

		rUpLeft = pSrcImg[pSrcImgIndex00 + 1];
		rUpRight = pSrcImg[pSrcImgIndex01 + 1];
		if (widthTag)
			rUpRight = rUpLeft;

		rDownLeft = pSrcImg[pSrcImgIndex10 + 1];
		rDownRight = pSrcImg[pSrcImgIndex11 + 1];
		if (widthTag)
			rDownRight = rDownLeft;

#if 0
		b = bUpLeft * (iX + 1 - fX) * (iY + 1 - fY) + bUpRight * (fX - iX) * (iY + 1 - fY)
			+ bDownLeft * (iX + 1 - fX) * (fY - iY) + bDownRight * (fX - iX) * (fY - iY);

		g = gUpLeft * (iX + 1 - fX) * (iY + 1 - fY) + gUpRight * (fX - iX) * (iY + 1 - fY)
			+ gDownLeft * (iX + 1 - fX) * (fY - iY) + gDownRight * (fX - iX) * (fY - iY);

		r = rUpLeft * (iX + 1 - fX) * (iY + 1 - fY) + rUpRight * (fX - iX) * (iY + 1 - fY)
			+ rDownLeft * (iX + 1 - fX) * (fY - iY) + rDownRight * (fX - iX) * (fY - iY);
#else
        b = float_from_fixed(FixedMul(fixed_from_int((int)bUpLeft), FixedMul(fixed_from_int(iX) + fixed_1 - fixed_from_float(fX), fixed_from_int(iY) + fixed_1 - fixed_from_float(fY))) +
                FixedMul(fixed_from_int((int)bUpRight), FixedMul(fixed_from_float(fX) - fixed_from_int(iX), fixed_from_int(iY) + fixed_1 - fixed_from_float(fY))) +
                FixedMul(fixed_from_int((int)bDownLeft), FixedMul(fixed_from_int(iX) + fixed_1 - fixed_from_float(fX), fixed_from_float(fY) - fixed_from_int(iY))) +
                FixedMul(fixed_from_int((int)bDownRight), FixedMul(fixed_from_float(fX) - fixed_from_int(iX), fixed_from_float(fY) - fixed_from_int(iY)))
                );
                
        g = float_from_fixed(FixedMul(fixed_from_int((int)gUpLeft), FixedMul(fixed_from_int(iX) + fixed_1 - fixed_from_float(fX), fixed_from_int(iY) + fixed_1 - fixed_from_float(fY))) +
                FixedMul(fixed_from_int((int)gUpRight), FixedMul(fixed_from_float(fX) - fixed_from_int(iX), fixed_from_int(iY) + fixed_1 - fixed_from_float(fY))) +
                FixedMul(fixed_from_int((int)gDownLeft), FixedMul(fixed_from_int(iX) + fixed_1 - fixed_from_float(fX), fixed_from_float(fY) - fixed_from_int(iY))) +
                FixedMul(fixed_from_int((int)gDownRight), FixedMul(fixed_from_float(fX) - fixed_from_int(iX), fixed_from_float(fY) - fixed_from_int(iY)))
                );
                
        r = float_from_fixed(FixedMul(fixed_from_int((int)rUpLeft), FixedMul(fixed_from_int(iX) + fixed_1 - fixed_from_float(fX), fixed_from_int(iY) + fixed_1 - fixed_from_float(fY))) +
                FixedMul(fixed_from_int((int)rUpRight), FixedMul(fixed_from_float(fX) - fixed_from_int(iX), fixed_from_int(iY) + fixed_1 - fixed_from_float(fY))) +
                FixedMul(fixed_from_int((int)rDownLeft), FixedMul(fixed_from_int(iX) + fixed_1 - fixed_from_float(fX), fixed_from_float(fY) - fixed_from_int(iY))) +
                FixedMul(fixed_from_int((int)rDownRight), FixedMul(fixed_from_float(fX) - fixed_from_int(iX), fixed_from_float(fY) - fixed_from_int(iY)))
                );        
#endif
		if (iY >= 0 && iY <= nHeight * 2 && iX >= 0 && iX <= nWidth * 2)
		{
		    pDstImg[j] = MK_RGB565(r, g, b);
		}
	}

dec_line_end:

	pdec_info->CurrentDecLine++;
	if (pdec_info->CurrentDecLine == pdec_info->OutputH) {
	    FirstDec = 0;
		return 0;
	}

	return 1;
}

// Logical rotate left operation.

uint JpgDecRol(uint32 i, int j)
{
    return ((i << j) | (i >> (32 - j)));
}
//------------------------------------------------------------------------------
// Retrieve one character from the input stream.
uint JpgDecGetChar(void)
{
    uint c;
    if (Jpg_pPinBufOfs == Jpg_pInBuf + JPGD_INBUFSIZE)//Jpg_pInBuf
    {
        // Try to get more bytes.
        JpgDecPrepInBuffer();
    }

    c = *Jpg_pPinBufOfs++;

    return (c);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Inserts a previously retrieved character back into the input buffer.
void JpgDecStuffChar(uint8 q)
{
    *(--Jpg_pPinBufOfs) = q;
//    Jpg_gInBufLeft++;
}


//------------------------------------------------------------------------------
// Retrieves one character from the input stream, but does
// not read past markers. Will continue to return 0xFF when a
// marker is encountered.
// FIXME: Bad name?
uint8 JpgDecGetOctet(void)
{
    uint8 c = JpgDecGetChar();

    if (c == 0xFF)
    {
        c = JpgDecGetChar();

        if (c == 0x00)
            return (0xFF);
        else
        {
            //JpgDecStuffChar(c);
            //JpgDecStuffChar(0xFF);
            *(--Jpg_pPinBufOfs) = c;
            *(--Jpg_pPinBufOfs) = 0xFF;
//            Jpg_gInBufLeft+=2;
            return (0xFF);
        }
    }

    return (c);
}


//------------------------------------------------------------------------------
// Retrieves a variable number of bits from the input stream.
// Does not recognize markers.
void JpgDecSkipBits_1(int numbits)
{
    uint c1;
    uint32 bit_buf = Jpg_gBitBuf;
    register bitsleft = Jpg_gBitsLeft;

    if ((bitsleft -= numbits) <= 0)
    {
        numbits += bitsleft;
        bit_buf = ((bit_buf << numbits) | (bit_buf >> (32 - numbits)));
        bit_buf &= 0xFFFF;
        c1 = JpgDecGetChar();
        bit_buf |= (c1 << 24);
        c1 = JpgDecGetChar();
        bit_buf |= (c1 << 16);

        bit_buf = ((bit_buf << (-bitsleft)) | (bit_buf >> (32 + bitsleft)));
        bitsleft += 16;
    }
    else
        bit_buf = ((bit_buf << numbits) | (bit_buf >> (32 - numbits)));


    Jpg_gBitBuf = bit_buf;
    Jpg_gBitsLeft = bitsleft;
}
uint JpgDecGetBits_1(int numbits)
{
    uint32 bit_buf = Jpg_gBitBuf;
    register bitsleft = Jpg_gBitsLeft;
    uint i = (bit_buf >> (16 - numbits)) & ((1 << numbits) - 1);
    if ((bitsleft -= numbits) <= 0)
    {
        uint c1;
        numbits += bitsleft;
        bit_buf = ((bit_buf << numbits) | (bit_buf >> (32 - numbits)));
        bit_buf &= 0xFFFF;
        c1 = JpgDecGetChar();
        bit_buf |= (c1 << 24);
        c1 = JpgDecGetChar();
        bit_buf |= (c1 << 16);
        bit_buf = ((bit_buf << (-bitsleft)) | (bit_buf >> (32 + bitsleft)));

        bitsleft += 16;
    }
    else
        bit_buf = ((bit_buf << numbits) | (bit_buf >> (32 - numbits)));

    Jpg_gBitBuf = bit_buf;
    Jpg_gBitsLeft = bitsleft;

    return i;
}


//------------------------------------------------------------------------------
// Retrieves a variable number of bits from the input stream.
// Markers will not be read into the input bit buffer. Instead,
// an infinite number of all 1's will be returned when a marker
// is encountered.
// FIXME: Is it better to return all 0's instead, like the older implementation?
uint JpgDecGetBits_2(int numbits)
{
    uint32 bit_buf = Jpg_gBitBuf;
    register bitsleft = Jpg_gBitsLeft;
    uint i = (bit_buf >> (16 - numbits)) & ((1 << numbits) - 1);
    if ((bitsleft -= numbits) <= 0)
    {
        uint c1;
        numbits += bitsleft;
        bit_buf = ((bit_buf << numbits) | (bit_buf >> (32 - numbits)));
        bit_buf &= 0xFFFF;
        c1 = JpgDecGetOctet();
        bit_buf |= (c1 << 24);
        c1 = JpgDecGetOctet();
        bit_buf |= (c1 << 16);
        bit_buf = ((bit_buf << (-bitsleft)) | (bit_buf >> (32 + bitsleft)));

        bitsleft += 16;
    }
    else
        bit_buf = ((bit_buf << numbits) | (bit_buf >> (32 - numbits)));

    Jpg_gBitBuf = bit_buf;
    Jpg_gBitsLeft = bitsleft;

    return i;
}
#ifndef USE_ASSEMBLY_OPT

void JpgDecSkipBits_2(int numbits)
{
    uint c1;

    uint32 bit_buf = Jpg_gBitBuf;
    register bitsleft = Jpg_gBitsLeft;
    if ((bitsleft -= numbits) <= 0)
    {
        numbits += bitsleft;
        bit_buf = ((bit_buf << numbits) | (bit_buf >> (32 - numbits)));
        bit_buf &= 0xFFFF;
        c1 = JpgDecGetOctet();
        bit_buf |= (c1 << 24);
        c1 = JpgDecGetOctet();
        bit_buf |= (c1 << 16);
		{
        //bit_buf = ((bit_buf << (-bitsleft)) | (bit_buf >> (32 + bitsleft)));
        int temp = 32 + bitsleft;
		bit_buf = ((bit_buf << (32-temp)) | (bit_buf >> temp));
        }
        bitsleft += 16;
    }
    else
        bit_buf = ((bit_buf << numbits) | (bit_buf >> (32 - numbits)));

    Jpg_gBitBuf = bit_buf;
    Jpg_gBitsLeft = bitsleft;
}

#endif

//------------------------------------------------------------------------------
// Decodes a Huffman encoded symbol.
int JpgDecHuffDecode(Phuff_tables_t Ph)
{
    int symbol;
    uint32 suffix = ((Jpg_gBitBuf >> 8) & 0xFF);


    symbol = Ph->look_up[suffix];
    // Check first 8-bits: do we have a complete symbol?
    //if ((symbol = Ph->look_up[(Jpg_gBitBuf >> 8) & 0xFF]) < 0)
    //if ((symbol = Ph->look_up[suffix]) < 0)
    if (symbol < 0)
    {
        // Decode more bits, use a tree traversal to find symbol.
        JpgDecSkipBits_2(8);

        do
        {
        	  uint temp = 1 - JpgDecGetBits_2(1);
        	  temp += ~symbol;
        	  symbol = Ph->tree[temp];
            //symbol = Ph->tree[~symbol + (1 - JpgDecGetBits_2(1))];
        }
        while (symbol < 0);
    }
    else
    {
        //JpgDecGetBits_2(symbol);
        JpgDecSkipBits_2(symbol);
        symbol = Ph->code_size[suffix];        
    }

    return symbol;
}


//------------------------------------------------------------------------------
// Clamps a value between 0-255.
uint8 clamp(int i)
{
    if (i & 0xFFFFFF00)
        i = (((~i) >> 31) & 0xFF);

    return (i);
}

#pragma arm section code
#endif


