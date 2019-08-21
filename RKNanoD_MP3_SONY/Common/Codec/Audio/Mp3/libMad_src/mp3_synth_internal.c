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

#ifdef A_CORE_DECODE
#ifdef MP3_DEC_INCLUDE

//文件访问接口
#include "..\..\Include\audio_file_access.h"

# include "mp3_global.h"
# include "mp3_fixed.h"
# include "mp3_frame.h"
# include "mp3_accelerator_hal.h"
# include <stdio.h>

//internal
_ATTR_MP3DEC_BSS_
struct mad_frame * synth_hw_frame;
_ATTR_MP3DEC_BSS_
mad_fixed_t (*synth_hw_sbsample)[36][32];	//input
_ATTR_MP3DEC_BSS_
unsigned int synth_hw_ch;
_ATTR_MP3DEC_BSS_
volatile unsigned int synth_hw_ns;
_ATTR_MP3DEC_BSS_
unsigned int synth_hw_s;
_ATTR_MP3DEC_BSS_
unsigned int synth_hw_phase;
_ATTR_MP3DEC_BSS_
volatile int synth_hw_busy;
_ATTR_MP3DEC_BSS_
int synth_hw_pcmout[32];					//output
_ATTR_MP3DEC_BSS_
static volatile int dummy;
_ATTR_MP3DEC_BSS_
unsigned int synth_end_ch_number;

_ATTR_MP3DEC_BSS_
volatile int mp3_frame_cnt;

_ATTR_MP3DEC_BSS_
volatile int is_synthing;
//extern
extern long synth_out_samples_limit;
extern long synth_out_samples_limit_change_flag ;


_ATTR_MP3DEC_TEXT_
void mp3_wait_synth()
{
	long synth_e_cnt = 0;
	while (is_synthing==1)
	{
		if (synth_end_ch_number==1)
		{
			if (synth_hw_frame->output_pos[0] == synth_hw_frame->output_pos[1])
			{
				synth_e_cnt ++;
				if (synth_e_cnt > 1024)
				{
					synth_hw_frame->output_pos[0] = 0;
					synth_hw_frame->output_pos[1] = 0;

					// by vincent hsiung , 6-17
					synth_hw_frame->phase[0] = 0;
					synth_hw_frame->phase[1] = 0;

					is_synthing = 0;
				}
			}
		}
		/* for test */
		else
			{
				// mono
				synth_e_cnt ++ ;
				if ( synth_e_cnt > 20480 ) //enlager
					{
						synth_hw_frame->output_pos[0] = 0;
						synth_hw_frame->output_pos[1] = 0;

						synth_hw_frame->phase[0] = 0;
						synth_hw_frame->phase[1] = 0;

						is_synthing = 0;
					}
			}
/*
		if (synth_out_samples_limit_change_flag == 1)
		{
			synth_hw_frame->output_pos[0] = 0;
			synth_hw_frame->output_pos[1] = 0;
			is_synthing = 0;

			synth_out_samples_limit_change_flag = 0;
		}
*/
	}
}

_ATTR_MP3DEC_TEXT_
void synth_output()
{
	int i;
	short *start_pos;
    //DEBUG2("synth_output synth_output");

	if (synth_hw_ch == 0)
		start_pos = &synth_hw_frame->output_ptrL[synth_hw_frame->output_pos[synth_hw_ch]*2] + synth_hw_ch;
	else
		start_pos = &synth_hw_frame->output_ptrR[synth_hw_frame->output_pos[synth_hw_ch]*2] + synth_hw_ch;

// for ZYZ , don't mute the first several frames
/*
	if (mp3_frame_cnt < 8)
	{
		//is_synthing = 0;
		if (synth_end_ch_number==1)	//stereo
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
*/
	{
		if (synth_end_ch_number==1)	//stereo
		{
			for(i = 0; i < 32; i++)
			{
				start_pos[i*2] = (short)(synth_hw_pcmout[i]);
			}
		}
		else						//mono
		{
			for(i = 0; i < 32; i++)
			{
				start_pos[i*2] = (short)synth_hw_pcmout[i];
				start_pos[i*2+1] = (short)synth_hw_pcmout[i];
			}
		}
	}

	synth_hw_frame->output_pos[synth_hw_ch]+=32;

    /*
	  depends frame length, when output_pos is larger than this we could output the
	  decoded data of current channel.

	  output length list :

      layer 1 : 384
      layer 2 : 1152
      layer 3 : 1152
      mpeg 2.5  : 576
	*/
    //mp3_printf("synth_out_samples_limit = %d\n",synth_out_samples_limit);
	if (synth_out_samples_limit == 1152)
	{
		if (synth_hw_frame->output_pos[synth_hw_ch]>= 1152)
		{
			synth_hw_frame->output_pos[synth_hw_ch] = 0;

			if (synth_hw_ch == 1)
			{
			    //右声道
				outbufferR_swap();
			}
			else
			{
			    //左声道
				outbufferL_swap();
			}

	        //判断此帧是否能结束
			if (synth_hw_ch == synth_end_ch_number)
				final_stuff(synth_hw_ch);
		}
	}

	if (synth_out_samples_limit == 384)
	{
		if (synth_hw_frame->output_pos[synth_hw_ch]>= 384)
		{
			synth_hw_frame->output_pos[synth_hw_ch] = 0;

			if (synth_hw_ch == 1)
			{
			    //右声道
				outbufferR_swap();
			}
			else
			{
			    //左声道
				outbufferL_swap();
			}

	        //判断此帧是否能结束
			if (synth_hw_ch == synth_end_ch_number)
				final_stuff(synth_hw_ch);
		}
	}

	if (synth_out_samples_limit == 576)
	{
		if (synth_hw_frame->output_pos[synth_hw_ch]>= 576)
		{
			synth_hw_frame->output_pos[synth_hw_ch] = 0;

			if (synth_hw_ch == 1)
			{
			    //右声道
				outbufferR_swap();
			}
			else
			{
			    //左声道
				outbufferL_swap();
			}

	        //判断此帧是否能结束
			if (synth_hw_ch == synth_end_ch_number)
				final_stuff(synth_hw_ch);
		}
	}
}

_ATTR_MP3DEC_TEXT_
void start_next_synth()
{
		if (synth_hw_s < synth_hw_ns)
		{
			SYNTH_CNFG_REG = (((15 - synth_hw_phase)<<1)|synth_hw_ch);	//config the phase & channel;
			//put the input data to the in-buffer
			if (!DMA1ToSynth((unsigned long)(&(*synth_hw_sbsample)[synth_hw_s][0])))	//if dma is busy, use memcpy
			{
				//memcpy(((void *)SYNTH_BASEADDR),&(*synth_hw_sbsample)[synth_hw_s][0],32*sizeof(int));
				long *p = (long *)SYNTH_BASEADDR;
				long *q = (long *)&(*synth_hw_sbsample)[synth_hw_s][0];
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
			}
			synth_hw_phase = (synth_hw_phase + 1) % 16;
			synth_hw_s++;
		}
		else
		{
			synth_hw_frame->phase[synth_hw_ch] = synth_hw_phase;
			synth_hw_busy = 0;
		}
}

_ATTR_MP3DEC_TEXT_
void synth_handler()
{
	SYNTH_EOIT_REG = 0;

	if (!DMA1FromSynth((unsigned long)(&synth_hw_pcmout[0])))	//if dma is busy, use memcpy
	{
		//memcpy(&synth_hw_pcmout[0],((unsigned long*)SYNTH_BASEADDR) ,32*sizeof(int));
		long *p = (long *)&synth_hw_pcmout[0];
		long *q = (long *)SYNTH_BASEADDR;
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
		synth_finish = 1;
        //mp3_printf("!@!@!@!@");
		synth_output();
		//do the next synth
		start_next_synth();
	}
}

_ATTR_MP3DEC_TEXT_
void synth_fouth_frame_hw(struct mad_frame * frame, mad_fixed_t (fouth_frame_sbsample)[36][32],
		unsigned int ch, unsigned int ns)
{
	int i;

#if 0
	if (ch > 2)
		return ; // Error
	if (ns > 18)
		return ; // Error
#endif

	is_synthing = 1;
	synth_hw_busy = 1;
	synth_hw_s = 0;
	synth_hw_ch = ch;
	synth_hw_ns = ns;
	synth_hw_frame = frame;
	synth_hw_sbsample = (mad_fixed_t (*)[36][32])fouth_frame_sbsample;
	synth_hw_phase = frame->phase[ch];

	synth_end_ch_number = (frame->header.mode?1:0);

	if (synth_hw_s < synth_hw_ns)
	{
		SYNTH_CNFG_REG = (((15 - synth_hw_phase)<<1)|synth_hw_ch);	//config the phase & channel;
		//put the input data to the in-buffer
		if (!DMA1ToSynth((unsigned long)(&(*synth_hw_sbsample)[synth_hw_s][0])))
		{
			//if dma is busy, use memcpy
			//memcpy(((void *)SYNTH_BASEADDR),&(*synth_hw_sbsample)[synth_hw_s][0],32*sizeof(int));
			long *p = (long *)SYNTH_BASEADDR;
			long *q = (long *)&(*synth_hw_sbsample)[synth_hw_s][0];
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
		}
		synth_hw_phase = (synth_hw_phase + 1) % 16;

		synth_hw_s++;
	}
	else
	{
		synth_hw_frame->phase[synth_hw_ch] = synth_hw_phase;
		synth_hw_busy = 0;
	}
}


#endif
#endif

