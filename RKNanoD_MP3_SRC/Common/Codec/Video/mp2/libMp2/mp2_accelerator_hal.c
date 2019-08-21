/*
********************************************************************************
*                   Copyright (c) 2008, Rockchip
*                         All rights reserved.
*
* File Name：   mp2_accelerator_hal.c
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             Vincent Hsiung     2009-5-12         1.0
*    desc:    ORG.
********************************************************************************
*/

/*

 1. 修改函数、变量名，以免与MP3重复
 2. 不使用DMA1，供视频刷屏使用

*/

#include <string.h>

#include "../include/audio_main.h"
#include "mp2_accelerator_hal.h"
#include "mp2_global.h"

#ifdef MP2_INCLUDE

_ATTR_MP2DEC_DATA_
volatile int mp2_synth_finish = 1;

_ATTR_MP2DEC_BSS_
volatile int mp2_synth_auto_output_addr;    //表示在做完子带合成运算后，自动将结果存放到哪里

extern volatile int mp2_synth_hw_busy;      //硬件子带合成模块忙标志
//extern functions
extern void mp2_start_next_synth(void);
extern void mp2_synth_output(void);

//用于开关中断
#define SETENA0_REG	*((volatile unsigned long*)(0xE000E100))
#define CLRENA0_REG	*((volatile unsigned long*)(0xE000E180))


//init funcitons--------------------------------------------------
extern int mp2_synth_hw_ch ;
extern int mp2_synth_hw_ns ;
extern int mp2_synth_hw_s ;
extern int mp2_synth_hw_phase ;


_ATTR_MP2DEC_TEXT_
void mp2_AcceleratorHWInit(void)
{
    mp2_synth_finish = 1;

	/* Apr 25 */
	mp2_synth_hw_ch = 0;
	mp2_synth_hw_ns = 0;
	mp2_synth_hw_s = 0;
	mp2_synth_hw_phase = 0;
	mp2_synth_hw_busy = 0;

	//Clear Int flags
	SYNTH_EOIT_REG = 0;

	SYNTH_CTRL_REG = 1;                         //Enable SYNTH
}

_ATTR_MP2DEC_TEXT_
void mp2_AcceleratorHWExit(void)
{
	//Clear Int flags
	SYNTH_EOIT_REG = 0;

	SYNTH_CTRL_REG = 0;                         //Disable SYNTH

    mp2_synth_hw_busy  = 0;
}

//interupt handlers-----------------------------------------------
_ATTR_MP2DEC_BSS_
static  unsigned long int_able_tmp;

#define SETENA0_REG	*((volatile unsigned long*)(0xE000E100))
#define CLRENA0_REG	*((volatile unsigned long*)(0xE000E180))
#define CLRPEND0_REG	*((volatile unsigned long*)(0xE000E280))

//has been moved to synth_internal
/*
void synth_handler();
*/
//----------------------------------------------------------------

#endif

