



//#include"Typedef.h"
//#include "audio_main.h"
//#include "speex_echo.h"
#include "speex_preprocess.h"
#include "Fftwrap.h"



   static void *fft_table ;
   static SpeexPreprocessState *pre_st;

        /*aec initial*/
int NS_init(int pre_len,unsigned int sample_rate);

         /*do aec*/
int NS_do(short *mic_buf,unsigned long buffe_len);
