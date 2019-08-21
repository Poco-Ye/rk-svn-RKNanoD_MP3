
#include <stdio.h>

#include "SysInclude.h"	  
#include "aec_interface.h"
#include "arch.h"

#include "Typedef.h"
#include "audio_main.h"


#include "audio_main.h"
#include "speex_echo.h"
#include "speex_preprocess.h"
#include "Fftwrap.h"




#ifdef _AEC_DECODE

_ATTR_AECALG_BSS_  static SpeexEchoState *echo_st;
_ATTR_AECALG_BSS_  static void *fft_table ;
_ATTR_AECALG_BSS_  static SpeexPreprocessState *pre_st;

_ATTR_AECALG_TEXT_
 int Aec_init(int aec_pre_len)
{
	int aec_filter_len = aec_pre_len *NM;//调整AEC效果的关键，若aec_pre_len 有改动，arch.h中的文件相应的也要改动
	int fs = 8000;
	fft_table = spx_fft_init(2*aec_pre_len);
	echo_st = speex_echo_state_init_mc(aec_pre_len, aec_filter_len,fft_table);	
    pre_st = speex_preprocess_state_init(aec_pre_len, fs,fft_table); 
	if (fft_table && echo_st && pre_st)
	{
		return 1;
	}
	else {return 0;}

}
_ATTR_AECALG_TEXT_
int Aec_do(short *mic_buf,short *echo_buf,short *out_buf)
{  
	int pos =0;
	int aec_buffe_len = 128;
	if (mic_buf && echo_buf)
	{
		while (pos!=aec_buffe_len)
		{		
			speex_echo_cancellation(echo_st,&mic_buf[pos],&echo_buf[pos], &out_buf[pos]);
			speex_preprocess_run(pre_st, &out_buf[pos],echo_st);			
			pos += NN;	 
		}


		return 1;
	}

	else{return 0;}


}

#endif//_AEC_DECODE