/*
********************************************************************************
*                   Copyright (c) 2008, Rockchip
*                         All rights reserved.
*
* File Name：   accelerator_hal.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             Vincent Hsiung     2009-1-14         1.0
*    desc:    ORG.
********************************************************************************
*/

#include <string.h>

#include "../include/audio_main.h"

#ifdef A_CORE_DECODE
#ifdef MP3_DEC_INCLUDE

#include "mp3_accelerator_hal.h"
#include "mp3_global.h"

_ATTR_MP3DEC_DATA_
volatile int dma1_busy = 0;
_ATTR_MP3DEC_DATA_
volatile int dma1_trans_type = 0;
_ATTR_MP3DEC_DATA_
volatile int imdct_finish = 1;
_ATTR_MP3DEC_DATA_
volatile int synth_finish = 1;

_ATTR_MP3DEC_BSS_
volatile int imdct36_auto_output_addr;  //表示在做完imdct36运算后，自动将结果存放到哪里
_ATTR_MP3DEC_BSS_
volatile int synth_auto_output_addr;    //表示在做完子带合成运算后，自动将结果存放到哪里

extern volatile int synth_hw_busy;      //硬件子带合成模块忙标志
//extern functions
extern void start_next_synth(void);
extern void synth_output(void);

#define UINT32 unsigned long

//用于开关中断
#define SETENA0_REG	*((volatile unsigned long*)(0xE000E100))
#define CLRENA0_REG	*((volatile unsigned long*)(0xE000E180))

//DMA short cuts
_ATTR_MP3DEC_TEXT_
int DMA1ToImdct(unsigned long pSrc)
{
	int tmp;

	//for debug
	 return false;

	tmp = SETENA0_REG;

    //关中断
	CLRENA0_REG = tmp;//0xfffffffe;
    if (dma1_busy != 0)
	{
		SETENA0_REG = tmp;//0xfffffffe;
		return false;
	}

	dma1_busy = 1;
	dma1_trans_type = 1;
	imdct_finish = 0;

	WriteReg32(DMA_ISA1, pSrc);
	WriteReg32(DMA_CTL1, DMA_SHORTCUT_TO_IMDCT);        //DMA传输会在配置的同时立刻开始

    //开中断
	SETENA0_REG = tmp; //0xfffffffe;

	return true;
}

_ATTR_MP3DEC_TEXT_
int DMA1FromImdct(unsigned long pDst)
{
	int tmp;

	//for debug
	return false;

	if (dma1_busy != 0)
		return false;

	tmp = SETENA0_REG;

    //关中断
	CLRENA0_REG = tmp;//0xfffffffe;

	dma1_busy = 1;
	dma1_trans_type = 2;

	WriteReg32(DMA_IDA1, pDst);
	WriteReg32(DMA_CTL1, DMA_SHORTCUT_FROM_IMDCT);      //DMA传输会在配置的同时立刻开始

    //开中断
	SETENA0_REG = tmp; //0xfffffffe;

	return true;
}

_ATTR_MP3DEC_TEXT_
int DMA1ToSynth(unsigned long pSrc)
{
	int tmp;

	//for debug
	 return false;

	if (dma1_busy != 0)
		return false;

	tmp = SETENA0_REG;

    //关中断
	CLRENA0_REG = tmp;//0xfffffffe;

	dma1_busy = 1;
	dma1_trans_type = 3;
	synth_finish = 0;
	WriteReg32(DMA_ISA1, pSrc);
	WriteReg32(DMA_CTL1, DMA_SHORTCUT_TO_SYNTH);        //DMA传输会在配置的同时立刻开始

    //开中断
	SETENA0_REG = tmp;//0xfffffffe;

	return true;
}

_ATTR_MP3DEC_TEXT_
int DMA1FromSynth(unsigned long pDst)
{
	int tmp;

	//for debug
	 return false;

	if (dma1_busy != 0)
		return false;

	tmp = SETENA0_REG;

    //关中断
	CLRENA0_REG = tmp;//0xfffffffe;

	dma1_busy = 1;
	dma1_trans_type = 4;
	WriteReg32(DMA_IDA1, pDst);
	WriteReg32(DMA_CTL1, DMA_SHORTCUT_FROM_SYNTH);      //DMA传输会在配置的同时立刻开始

    //开中断
	SETENA0_REG = tmp;//0xfffffffe;

	return true;
}

//init funcitons--------------------------------------------------
extern int synth_hw_ch ;
extern int synth_hw_ns ;
extern int synth_hw_s ;
extern int synth_hw_phase ;
//extern int synth_hw_busy ;

_ATTR_MP3DEC_TEXT_
void AcceleratorHWInit(void)
{
    dma1_busy = 0;
    dma1_trans_type = 0;
    imdct_finish = 1;
    synth_finish = 1;

	/* Apr 25 */
	synth_hw_ch = 0;
	synth_hw_ns = 0;
	synth_hw_s = 0;
	synth_hw_phase = 0;
	synth_hw_busy = 0;
	//dele by hl
	#if 0
	//Init DMA1
	SetRegBits32(DMA_ICON, DMA_TCIE1_BIT);      // Channel 1 Trans_complete_int_enable
    //06 27 by Vincent
	//SetRegBits32(DMA_CTL1, DMA_CHEN_BIT);       // Enable channel 1
 	   ClrRegBits32(DMA_ICON, DMA_TCIM1_BIT);      // Clear Channel 1 Trans_complete_int_mask
	#endif
	//Clear Int flags
	IMDCT_EOIT_REG = 0;
	SYNTH_EOIT_REG = 0;

	SYNTH_CTRL_REG = 1;                         //Enable SYNTH
	IMDCT_CTRL_REG = 3;                         //Enable IMDCT, right shift 12bit mode
}

_ATTR_MP3DEC_TEXT_
void AcceleratorHWExit(void)
{
	//Clear Int flags
	IMDCT_EOIT_REG = 0;
	SYNTH_EOIT_REG = 0;

	SYNTH_CTRL_REG = 0;                         //Disable SYNTH
	IMDCT_CTRL_REG = 0;                         //Disable IMDCT

    synth_hw_busy  = 0;
}

//interupt handlers-----------------------------------------------
_ATTR_MP3DEC_BSS_
static  unsigned long int_able_tmp;

#define SETENA0_REG	*((volatile unsigned long*)(0xE000E100))
#define CLRENA0_REG	*((volatile unsigned long*)(0xE000E180))
#define CLRPEND0_REG	*((volatile unsigned long*)(0xE000E280))

_ATTR_MP3DEC_TEXT_
void dma1_handler()
{
//	*((unsigned int*)(0x6204004c)) = 0x11;	//clear dma channel 1 int
    //mp3_printf("!!!!!!!!!!");
	SYNTH_INTR_REG = 0x11;// add by hl
	int_able_tmp = SETENA0_REG;
	CLRENA0_REG	 = 0xffffffff;

	dma1_busy = 0;

	switch(dma1_trans_type)
	{
		case 2:
		    {
    			imdct_finish = 1;
    			break;
    		}
		case 4:
		    {
    			synth_output();
    			//do the next synth
    			start_next_synth();
    			synth_finish = 1;
    			break;
    		}
	}
	dma1_trans_type = 5;

	SETENA0_REG = int_able_tmp ;
}

_ATTR_MP3DEC_TEXT_
void imdct36_handler()
{/*
	IMDCT_EOIT_REG = 0;		//clear the interrupt

	if (!DMA1FromImdct((unsigned long)imdct36_auto_output_addr)) //dma busy, use memcpy
	{
		//memcpy((void *)imdct36_auto_output_addr,(unsigned char *)(IMDCT_BASEADDR + 18*4),36*4);
		long *p = (long *)imdct36_auto_output_addr;
		long *q = (long *)(IMDCT_BASEADDR + 18*4);
		p[ 0] = q[ 0];
		p[ 1] = q[ 1];
		p[ 2] = q[ 2];
		p[ 3] = q[ 3];
		p[ 4] = q[ 4];
		p[ 5] = q[ 5];
		p[ 6] = q[ 6];
		p[ 7] = q[ 7];
		p[ 8] = q[ 8];
		p[ 9] = q[ 9];
		p[10] = q[10];
		p[11] = q[11];
		p[12] = q[12];
		p[13] = q[13];
		p[14] = q[14];
		p[15] = q[15];
		p[16] = q[16];
		p[17] = q[17];
		p[18] = q[18];
		p[19] = q[19];
		p[20] = q[20];
		p[21] = q[21];
		p[22] = q[22];
		p[23] = q[23];
		p[24] = q[24];
		p[25] = q[25];
		p[26] = q[26];
		p[27] = q[27];
		p[28] = q[28];
		p[29] = q[29];
		p[30] = q[30];
		p[31] = q[31];
		p[32] = q[32];
		p[33] = q[33];
		p[34] = q[34];
		p[35] = q[35];
		imdct_finish = 1;
	}*/
	;
}


//has been moved to synth_internal
/*
void synth_handler();
*/
//----------------------------------------------------------------

#endif
#endif

