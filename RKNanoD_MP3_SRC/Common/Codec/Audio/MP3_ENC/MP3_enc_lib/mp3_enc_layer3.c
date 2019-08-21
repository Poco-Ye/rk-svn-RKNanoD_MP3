/* layer3.c */

#include  "SysInclude.h"
#include  "audio_main.h"

#include  "FsInclude.h"
#include  "File.h"
#include  "FDT.h"

#ifdef MP3_ENC_INCLUDE2
#include "mp3_enc_types.h"
#include  "RecordControl.h"

#pragma arm section code = "EncodeMP3Code", rodata = "EncodeMP3Code", rwdata = "EncodeMP3Data", zidata = "EncodeMP3Bss"


/* Scalefactor bands. */


static int sfBandIndex[4][3][23] =
{
  { /* MPEG-2.5 11.025 kHz */
    {0,6,12,18,24,30,36,44,54,66,80,96,116,140,168,200,238,284,336,396,464,522,576},
    /* MPEG-2.5 12 kHz */
    {0,6,12,18,24,30,36,44,54,66,80,96,116,140,168,200,238,284,336,396,464,522,576},
    /* MPEG-2.5 8 kHz */
    {0,12,24,36,48,60,72,88,108,132,160,192,232,280,336,400,476,566,568,570,572,574,576}
  },
  {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
  },
  { /* Table B.2.b: 22.05 kHz */
    {0,6,12,18,24,30,36,44,54,66,80,96,116,140,168,200,238,284,336,396,464,522,576},
    /* Table B.2.c: 24 kHz */
    {0,6,12,18,24,30,36,44,54,66,80,96,114,136,162,194,232,278,332,394,464,540,576},
    /* Table B.2.a: 16 kHz */
    {0,6,12,18,24,30,36,44,54,66,80,96,116,140,168,200,238,284,336,396,464,522,576}
  },
  { /* Table B.8.b: 44.1 kHz */
    {0,4,8,12,16,20,24,30,36,44,52,62,74,90,110,134,162,196,238,288,342,418,576},
    /* Table B.8.c: 48 kHz */
    {0,4,8,12,16,20,24,30,36,42,50,60,72,88,106,128,156,190,230,276,330,384,576},
    /* Table B.8.a: 32 kHz */
    {0,4,8,12,16,20,24,30,36,44,54,66,82,102,126,156,194,240,296,364,448,550,576}
  }
};


 int    l3_enc[2][2][samp_per_frame2] = {0};   // 576*4*4 = 9216

 long   mdct_freq[2][2][samp_per_frame2]={0};  //576*4*4 = 9216  ¹²32256

static L3_side_info_t side_info;


long   l3_sb_sample[2][3][18][SBLIMIT]={0};   //32*6*18*4 = 13824

mp3_info mp3_con_info;


//#define NO_RESERVOIR
extern int main_data_begin ;
extern  int wr, rd; /* read and write index for side fifo */
extern  int by , bi; /* byte and bit counts within main or side stores */
extern  int count;  /* byte counter within current frame */
extern  int bits;   /* bit counter used when encoding side and main data */
extern int off[2];

int *scalefac_band_long;



void Mp3EncodeHeaderInit(void)
{
  int i =0;
  main_data_begin = 0;
  wr = rd = 0 ;
  by = bi = 0;
  count = 0;
  bits = 0;
  off[0] = off[1] =0;
 // memset(l3_enc,0,sizeof(l3_enc));
 // memset(mdct_freq,0,sizeof(mdct_freq));
//  memset(l3_sb_sample,0,sizeof(l3_sb_sample));
  if(config.mpeg.type == MPEG1)
   {
     config.mpeg.granules = 2;
     config.mpeg.samples_per_frame = samp_per_frame;
     config.mpeg.resv_limit = ((1<<9)-1)<<3;
     mp3_con_info.sideinfo_len = (config.mpeg.channels == 1) ? 168 : 288;
   }
   else /* mpeg 2/2.5 */
   {
     config.mpeg.granules = 1;
     config.mpeg.samples_per_frame = samp_per_frame2;
     config.mpeg.resv_limit = ((1<<8)-1)<<3;
     mp3_con_info.sideinfo_len = (config.mpeg.channels == 1) ? 104 : 168;
   }
   scalefac_band_long = sfBandIndex[config.mpeg.type][config.mpeg.samplerate_index];

   { /* find number of whole bytes per frame and the remainder */
     long x = config.mpeg.samples_per_frame * config.mpeg.bitr * (1000/8);
     mp3_con_info.bytes_per_frame = x / config.mpeg.samplerate;
     mp3_con_info.remainder  = x % config.mpeg.samplerate;
   }
   mp3_con_info.lag = 0;
   open_bit_stream();
}

/*
 * L3_compress:
 * ------------
 */
long L3_compress(short *in_ptr, int len,unsigned char ** ppOutBuf )
{
    int           ch;
    int           i;
    int           gr;
    int           write_bytes;
    unsigned long *buffer[2];

    buffer[0] = buffer[1] = (unsigned long *)in_ptr;

    /* sort out padding */
    config.mpeg.padding = (mp3_con_info.lag += mp3_con_info.remainder) >= config.mpeg.samplerate;

    if (config.mpeg.padding)
    {
        mp3_con_info.lag -= config.mpeg.samplerate;
    }
    config.mpeg.bits_per_frame = 8*(mp3_con_info.bytes_per_frame + config.mpeg.padding);

    /* bits per channel per granule */
    mp3_con_info.mean_bits = (config.mpeg.bits_per_frame - mp3_con_info.sideinfo_len) >>
                      (config.mpeg.granules + config.mpeg.channels - 2);


    /* polyphase filtering */
    for(gr=0; gr<config.mpeg.granules; gr++)
    {
        for(ch=0; ch<config.mpeg.channels; ch++)
        {
            for(i=0;i<18;i++)
            {
                L3_window_filter_subband(&buffer[ch], &l3_sb_sample[ch][gr+1][i][0] ,ch);
            }
        }
    }

    #if 1
    L3_mdct_sub();
    L3_iteration_loop( &side_info, mp3_con_info.mean_bits);
    #else
    L3_mdct_sub(l3_sb_sample, mdct_freq);
    L3_iteration_loop(mdct_freq, &side_info, l3_enc, mp3_con_info.mean_bits);
    #endif

    /* write the frame to the bitstream */
    write_bytes = L3_format_bitstream(l3_enc, &side_info,ppOutBuf);

    return write_bytes;
}

#pragma arm section code
#endif

