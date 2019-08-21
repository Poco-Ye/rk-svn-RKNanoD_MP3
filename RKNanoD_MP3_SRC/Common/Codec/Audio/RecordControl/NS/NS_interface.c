
//#include <stdio.h>


#include "NS_interface.h"
#include "arch.h"

//#include "Typedef.h"
#include "audio_main.h"


#pragma arm section code = "NSCode", rodata = "NSCode", rwdata = "NSData", zidata = "NSBss"
/**********************
*     pre_len
*MP3  144
*WAV  128
**********************/
int NN ; //the number of pre_process samples ���δ�������ݸ���
  
int NS_init(int pre_len,unsigned int sample_rate)
{
	  int fs = sample_rate;
    NN = pre_len;
	  fft_table = spx_fft_init(2*128);	
    if(NN > Np)
    {
      //bb_printf1("����Ӧ����Npֵ\n");
      return -1;
    }
    else if(NN < sample_rate/100)
    {
     // bb_printf1("���δ��������� 10ms����\n");
     // return -1;
    }
    pre_st = speex_preprocess_state_init(NN, fs,fft_table); //pre_len Ϊÿ�δ�������ݸ�����==NN
	return 1;

}

int NS_do(short *mic_buf,unsigned long buffe_len)
{  
	int pos =0;
    if(buffe_len % NN)
    {
       //bb_printf1("buffe_len %d ��Ϊ%d ����",buffe_len,NN);
       return 0;
    }
	if (mic_buf)
	{   
	    
		while (pos!=buffe_len)
		{		
		    speex_preprocess_run(pre_st, &mic_buf[pos]);
			pos += NN;
		}		
		return 1;
	}

	else{return 0;}

}


