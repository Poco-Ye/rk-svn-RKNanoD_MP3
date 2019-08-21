/*
********************************************************************************
*                   Copyright (c) 2008, Rockchip
*                         All rights reserved.
*
* File Name：   synth_internal.c
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             Vincent Hsiung     2009-1-14         1.0
*    desc:    ORG.
********************************************************************************
*/

/*
 1. 使用硬件synth模块来进行子带合成运算
 2. 在结果输出时需要注意: 声道数，每帧样本数，帧结束判断等
 3. 要注意与layer 1,layer 2的兼容性
*/


#include "../include/audio_main.h"
#ifdef MP2_INCLUDE

//文件访问接口
#include "..\..\Include\audio_file_access.h"

# include "mp2_global.h"
# include "mp2_fixed.h"
# include "mp2_frame.h"
# include "mp2_accelerator_hal.h"
# include <stdio.h>

//internal
_ATTR_MP2DEC_BSS_
struct mad_frame * mp2_synth_hw_frame;
_ATTR_MP2DEC_BSS_
mad_fixed_t (*mp2_synth_hw_sbsample)[36][32];	//input
_ATTR_MP2DEC_BSS_
unsigned int mp2_synth_hw_ch;
_ATTR_MP2DEC_BSS_
volatile unsigned int mp2_synth_hw_ns;
_ATTR_MP2DEC_BSS_
unsigned int mp2_synth_hw_s;
_ATTR_MP2DEC_BSS_
unsigned int mp2_synth_hw_phase;
_ATTR_MP2DEC_BSS_
volatile int mp2_synth_hw_busy;
_ATTR_MP2DEC_BSS_
int mp2_synth_hw_pcmout[32];					//output
_ATTR_MP2DEC_BSS_
static volatile int mp2_dummy;
_ATTR_MP2DEC_BSS_
unsigned int mp2_synth_end_ch_number;

_ATTR_MP2DEC_BSS_
volatile int mp2_frame_cnt;

_ATTR_MP2DEC_BSS_
volatile int mp2_is_synthing;
//extern
extern long mp2_synth_out_samples_limit;
extern long mp2_synth_out_samples_limit_change_flag ;

_ATTR_MP2DEC_TEXT_
void mp2_wait_synth()
{
	while (mp2_is_synthing==1)
	{
		if (mp2_synth_hw_frame->output_pos[0] == mp2_synth_hw_frame->output_pos[1])
		{
			mp2_synth_hw_frame->output_pos[0] = 0;
			mp2_synth_hw_frame->output_pos[1] = 0;
			mp2_is_synthing = 0;
		}
	}
}

_ATTR_MP2DEC_TEXT_
void mp2_synth_output()
{
	int i;
	short *start_pos;

	if (mp2_synth_hw_ch == 0)
		start_pos = &mp2_synth_hw_frame->output_ptrL[mp2_synth_hw_frame->output_pos[mp2_synth_hw_ch]*2] + mp2_synth_hw_ch;
	else
		start_pos = &mp2_synth_hw_frame->output_ptrR[mp2_synth_hw_frame->output_pos[mp2_synth_hw_ch]*2] + mp2_synth_hw_ch;

	if (mp2_frame_cnt < 5)
	{
		if (mp2_synth_end_ch_number==1)	//stereo
		{
			for(i = 0; i < 32; i++)
			{
				start_pos[i*2] = 0;
			}
		}
		else						//mono
		{
			for(i = 0; i < 32; i++)
			{			
				start_pos[i*2] = 0;
				start_pos[i*2+1] = 0;
			}
		}
	}
	else
	{
		if (mp2_synth_end_ch_number==1)	//stereo
		{
			for(i = 0; i < 32; i++)
			{
				start_pos[i*2] = (short)(mp2_synth_hw_pcmout[i]);
			}
		}
		else						//mono
		{
			for(i = 0; i < 32; i++)
			{			
				start_pos[i*2] = (short)mp2_synth_hw_pcmout[i];
				start_pos[i*2+1] = (short)mp2_synth_hw_pcmout[i];
			}
		}
	}
	
	mp2_synth_hw_frame->output_pos[mp2_synth_hw_ch]+=32;

    /*
	  depends frame length, when output_pos is larger than this we could output the 
	  decoded data of current channel.

	  output length list :

      layer 1 : 384
      layer 2 : 1152
      layer 3 : 1152
      mpeg 2.5  : 576
	*/

	if (mp2_synth_out_samples_limit == 1152)
	{
		if (mp2_synth_hw_frame->output_pos[mp2_synth_hw_ch]>= 1152)
		{
			mp2_synth_hw_frame->output_pos[mp2_synth_hw_ch] = 0;
	
			if (mp2_synth_hw_ch == 1)
			{
			    //右声道
				mp2_outbufferR_swap();
			}
			else
			{
			    //左声道
				mp2_outbufferL_swap();
			}
	
	        //判断此帧是否能结束
			if (mp2_synth_hw_ch == mp2_synth_end_ch_number)
				mp2_final_stuff(mp2_synth_hw_ch);
		}
	}

	if (mp2_synth_out_samples_limit == 384)
	{
		if (mp2_synth_hw_frame->output_pos[mp2_synth_hw_ch]>= 384)
		{
			mp2_synth_hw_frame->output_pos[mp2_synth_hw_ch] = 0;
	
			if (mp2_synth_hw_ch == 1)
			{
			    //右声道
				mp2_outbufferR_swap();
			}
			else
			{
			    //左声道
				mp2_outbufferL_swap();
			}
	
	        //判断此帧是否能结束
			if (mp2_synth_hw_ch == mp2_synth_end_ch_number)
				mp2_final_stuff(mp2_synth_hw_ch);
		}
	}

	if (mp2_synth_out_samples_limit == 576)
	{
		if (mp2_synth_hw_frame->output_pos[mp2_synth_hw_ch]>= 576)
		{
			mp2_synth_hw_frame->output_pos[mp2_synth_hw_ch] = 0;
	
			if (mp2_synth_hw_ch == 1)
			{
			    //右声道
				mp2_outbufferR_swap();
			}
			else
			{
			    //左声道
				mp2_outbufferL_swap();
			}
	
	        //判断此帧是否能结束
			if (mp2_synth_hw_ch == mp2_synth_end_ch_number)
				mp2_final_stuff(mp2_synth_hw_ch);
		}
	}
}

_ATTR_MP2DEC_TEXT_
void mp2_start_next_synth()
{
		if (mp2_synth_hw_s < mp2_synth_hw_ns)
		{
			SYNTH_CNFG_REG = (((15 - mp2_synth_hw_phase)<<1)|mp2_synth_hw_ch);	//config the phase & channel;
			//put the input data to the in-buffer
				memcpy(((void *)SYNTH_BASEADDR),&(*mp2_synth_hw_sbsample)[mp2_synth_hw_s][0],32*sizeof(int));
			
			mp2_synth_hw_phase = (mp2_synth_hw_phase + 1) % 16;
			mp2_synth_hw_s++;
		}
		else
		{
			mp2_synth_hw_frame->phase[mp2_synth_hw_ch] = mp2_synth_hw_phase;
			mp2_synth_hw_busy = 0;
		}
}

_ATTR_MP2DEC_TEXT_
void mp2_synth_handler()
{	
	SYNTH_EOIT_REG = 0;

	memcpy(&mp2_synth_hw_pcmout[0],((unsigned long*)SYNTH_BASEADDR) ,32*sizeof(int));
	mp2_synth_finish = 1;
	
	mp2_synth_output();
	//do the next synth
	mp2_start_next_synth();
}

_ATTR_MP2DEC_TEXT_
void mp2_synth_fouth_frame_hw(struct mad_frame * frame, mad_fixed_t (fouth_frame_sbsample)[36][32],
		unsigned int ch, unsigned int ns)
{
	int i;

#if 0
	if (ch > 2)
		return ; // Error
	if (ns > 18)
		return ; // Error
#endif

	mp2_is_synthing = 1;
	mp2_synth_hw_busy = 1;
	mp2_synth_hw_s = 0;
	mp2_synth_hw_ch = ch;
	mp2_synth_hw_ns = ns;
	mp2_synth_hw_frame = frame;
	mp2_synth_hw_sbsample = (mad_fixed_t (*)[36][32])fouth_frame_sbsample;
	mp2_synth_hw_phase = frame->phase[ch];

	mp2_synth_end_ch_number = (frame->header.mode?1:0);

	if (mp2_synth_hw_s < mp2_synth_hw_ns)
	{
		SYNTH_CNFG_REG = ( ((15 - mp2_synth_hw_phase)<<1) | mp2_synth_hw_ch );	//config the phase & channel;
		//put the input data to the in-buffer
		{
			memcpy(((void *)SYNTH_BASEADDR),&(*mp2_synth_hw_sbsample)[mp2_synth_hw_s][0],32*sizeof(int));
		}
		mp2_synth_hw_phase = (mp2_synth_hw_phase + 1) % 16;

		mp2_synth_hw_s++;
	}
	else
	{
		mp2_synth_hw_frame->phase[mp2_synth_hw_ch] = mp2_synth_hw_phase;
		mp2_synth_hw_busy = 0;
	}
}

#endif

