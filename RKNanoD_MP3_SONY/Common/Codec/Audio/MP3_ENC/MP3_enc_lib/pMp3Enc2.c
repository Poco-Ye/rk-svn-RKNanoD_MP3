/* main.c
 * Command line interface.
 *
 */

#include "audio_globals.h"

#ifdef MP3_ENC_INCLUDE2
#include "mp3_enc_types.h"
#pragma arm section code = "EncodeMP3Code", rodata = "EncodeMP3Code", rwdata = "EncodeMP3Data", zidata = "EncodeMP3Bss"


//extern void L3_compress(void);
config_t config;

/*
 * set_defaults:
 * -------------
 */



int find_samplerate_index(long freq)
{
  long sr[4][3] =  {{11025, 12000,  8000},   /* mpeg 2.5 */
                          {    0,     0,     0},   /* reserved */
                          {22050, 24000, 16000},   /* mpeg 2 */
                          {44100, 48000, 32000}};  /* mpeg 1 */
  int i, j;

  for(j=0; j<4; j++)
    for(i=0; i<3; i++)
      if((freq == sr[j][i]) && (j != 1))
      {
        config.mpeg.type = j;
        return i;
      }

  return 0;
}

/*
 * find_bitrate_index:
 * -------------------
 */
int find_bitrate_index(int bitr)
{
  long br[2][15] =
    {{0, 8,16,24,32,40,48,56, 64, 80, 96,112,128,144,160},   /* mpeg 2/2.5 */
     {0,32,40,48,56,64,80,96,112,128,160,192,224,256,320}};  /* mpeg 1 */
  int i;

  for(i=1; i<15; i++)
    if(bitr<=br[config.mpeg.type & 1][i]) return i;

  return 0;
}

int set_cutoff(void)
{
int cutoff_tab[3][2][15] =
  {
    { /* 44.1k, 22.05k, 11.025k */
      {100,104,131,157,183,209,261,313,365,418,418,418,418,418,418}, /* stereo */
      {183,209,261,313,365,418,418,418,418,418,418,418,418,418,418}  /* mono */
    },
    { /* 48k, 24k, 12k */
      {100,104,131,157,183,209,261,313,384,384,384,384,384,384,384}, /* stereo */
      {183,209,261,313,365,384,384,384,384,384,384,384,384,384,384}  /* mono */
    },
    { /* 32k, 16k, 8k */
      {100,104,131,157,183,209,261,313,365,418,522,576,576,576,576}, /* stereo */
      {183,209,261,313,365,418,522,576,576,576,576,576,576,576,576}  /* mono */
    }
  };

  return cutoff_tab[config.mpeg.samplerate_index]
                   [config.mpeg.mode == MODE_MONO]
                   [config.mpeg.bitrate_index];
}



#pragma arm section code 



/*
--------------------------------------------------------------------------------
  Function name : void WavEncodeVariableInit()
  Author        : WangBo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                 WangBo         2009-4-16          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

unsigned int Mp3EncodeVariableInit2(int samplerate,int channel,int  Bitrate)
{    
    config.mpeg.samplerate = samplerate; //sampling rate. 8000
    config.mpeg.channels = channel;//1
    config.mpeg.bitr = Bitrate; //32
     if(config.mpeg.samplerate <= 11025)
      {
         config.mpeg.type = MPEG2_5;
    	 if (config.mpeg.bitr >160)
    	 {
    		   config.mpeg.bitr = 160;
    	 }
      }
      else if(config.mpeg.samplerate <= 22050)
      {
         config.mpeg.type = MPEG2;
    	 if (config.mpeg.bitr >160)
    	 {
    		 config.mpeg.bitr = 160;
    	 }
      }
      else
      {
         config.mpeg.type = MPEG1;
    	 if (config.mpeg.bitr >320)
    	 {
    		 config.mpeg.bitr = 320;
    	 }
      }
    if (config.mpeg.channels == 1)
    {
         config.mpeg.mode = MODE_MONO; 
    }
    else
    {
        config.mpeg.mode = MODE_STEREO; 
    // config.mpeg.mode = MODE_DUAL_CHANNEL; 
    // config.mpeg.mode = MODE_MS_STEREO; 
    }
    config.mpeg.layr = LAYER_3;   
    config.mpeg.psyc = 0;
    config.mpeg.emph = 0;
    config.mpeg.crc  = 0;
    config.mpeg.ext  = 0;
    config.mpeg.mode_ext  = 0;
    config.mpeg.copyright = 0;
    config.mpeg.original  = 1;
    config.mpeg.granules = 1; //与MP3格式有关(标准)，会被重新赋值

    config.mpeg.samplerate_index = find_samplerate_index(config.mpeg.samplerate);
    config.mpeg.bitrate_index    =find_bitrate_index(config.mpeg.bitr);
    config.mpeg.cutoff = set_cutoff();

    Mp3EncodeHeaderInit(); 

    return config.mpeg.samples_per_frame;

}


#endif



