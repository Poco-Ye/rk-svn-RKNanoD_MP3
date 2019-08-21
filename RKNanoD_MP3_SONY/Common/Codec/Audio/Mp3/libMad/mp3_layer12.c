/*
********************************************************************************
*                   Copyright (c) 2008, Rockchip
*                         All rights reserved.
*
* File Name：   layer12.c
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             Vincent Hsiung     2009-1-14         1.0
*    desc:    ORG.
********************************************************************************
*/

#include "../include/audio_main.h"

#ifdef A_CORE_DECODE
#ifdef MP3_DEC_INCLUDE

#include "mp3_global.h"

#define CHAR_BIT  8

#include "mp3_fixed.h"
#include "mp3_bit.h"
#include "mp3_stream.h"
#include "mp3_frame.h"
#include "mp3_layer12.h"

extern mad_fixed_t const sf_table[64] ;
extern mad_fixed_t const linear_table[14] ;

//#define SWITCH_ARM_ACC()	*((volatile unsigned long*)0x40000008) = (0x08 << 16)  | 0;
//#define SWITCH_ARM_NOR()	*((volatile unsigned long*)0x40000008) = (0x08 << 16)  | 0x08;




extern 		void SWITCH_ARM_ACC(void);
extern 		void SWITCH_ARM_NOR(void);

/*
 * NAME:	I_sample()
 * DESCRIPTION:	decode one requantized Layer I sample from a bitstream
 */
_ATTR_MP3DEC_TEXT_
static
mad_fixed_t I_sample(struct mad_bitptr *ptr, unsigned int nb)
{
  mad_fixed_t sample;

  sample = mad_bit_read(ptr, nb);

  /* invert most significant bit, extend sign, then scale to fixed format */

  sample ^= 1 << (nb - 1);
  sample |= -(sample & (1 << (nb - 1)));

  sample <<= MAD_F_FRACBITS - (nb - 1);

  /* requantize the sample */

  /* s'' = (2^nb / (2^nb - 1)) * (s''' + 2^(-nb + 1)) */

  sample += MAD_F_ONE >> (nb - 1);

  return ((sample>>12)*linear_table[nb - 2]);//mad_f_mul(sample, linear_table[nb - 2]);

  /* s' = factor * s'' */
  /* (to be performed by caller) */
}

/*
 * NAME:	layer->I()
 * DESCRIPTION:	decode a single Layer I frame
 */
_ATTR_MP3DEC_TEXT_
int mad_layer_I(struct mad_stream *stream, struct mad_frame *frame)
{
  struct mad_header *header = &frame->header;
  unsigned int nch, bound, ch, s, sb, nb;
  unsigned char allocation[2][32], scalefactor[2][32];
  
  //对于layer 1,每个声道占用12个子带
  mad_fixed_t (*sampleL)[32] = &(*frame->sbsample)[0];
  mad_fixed_t (*sampleR)[32] = &(*frame->sbsample)[12];

  nch = MAD_NCHANNELS(header);

//if ((*((volatile unsigned long*)0x40000000) & 0x00000040)  == 0)
	SWITCH_ARM_ACC();

  bound = 32;
  if (header->mode == MAD_MODE_JOINT_STEREO) {
    header->flags |= MAD_FLAG_I_STEREO;
    bound = 4 + header->mode_extension * 4;
  }

  /* check CRC word */

  if (header->flags & MAD_FLAG_PROTECTION) {
    header->crc_check =
      mad_bit_crc(stream->ptr, 4 * (bound * nch + (32 - bound)),
		  header->crc_check);

    if (header->crc_check != header->crc_target &&
	!(frame->options & MAD_OPTION_IGNORECRC)) {
      stream->error = MAD_ERROR_BADCRC;
      return -1;
    }
  }

  /* decode bit allocations */

  for (sb = 0; sb < bound; ++sb) {
    for (ch = 0; ch < nch; ++ch) {
      nb = mad_bit_read(&stream->ptr, 4);

      if (nb == 15) {
    	stream->error = MAD_ERROR_BADBITALLOC;
    	return -1;
      }

      allocation[ch][sb] = nb ? nb + 1 : 0;
    }
  }

  for (sb = bound; sb < 32; ++sb) {
    nb = mad_bit_read(&stream->ptr, 4);

    if (nb == 15) {
      stream->error = MAD_ERROR_BADBITALLOC;
      return -1;
    }

    allocation[0][sb] =
    allocation[1][sb] = nb ? nb + 1 : 0;
  }

  /* decode scalefactors */

  for (sb = 0; sb < 32; ++sb) {
    for (ch = 0; ch < nch; ++ch) {
      if (allocation[ch][sb]) {
    	scalefactor[ch][sb] = (unsigned char)mad_bit_read(&stream->ptr, 6);
    
# if defined(OPT_STRICT)
    	/*
    	 * Scalefactor index 63 does not appear in Table B.1 of
    	 * ISO/IEC 11172-3. Nonetheless, other implementations accept it,
    	 * so we only reject it if OPT_STRICT is defined.
    	 */
    	if (scalefactor[ch][sb] == 63) {
    	  stream->error = MAD_ERROR_BADSCALEFACTOR;
    	  return -1;
    	}
# endif
      }
    }
  }

  /* decode samples */

  for (s = 0; s < 12; ++s) {
    for (sb = 0; sb < bound; ++sb) {
      for (ch = 0; ch < nch; ++ch) {
        //mad_fixed_t (*sample)[32] = &(*frame->sbsample)[0];
		nb = allocation[ch][sb];

        //左右声道的样本分开存储
		/*((*frame->sbsample)[ch][s][sb])*/
		if (ch==0)
		{
			sampleL[s][sb]= nb ?
			  ( (I_sample(&stream->ptr, nb)>>12) * sf_table[scalefactor[ch][sb]]) : 0;
		}
		else
		{
			sampleR[s][sb]= nb ?
			  ( (I_sample(&stream->ptr, nb)>>12) * sf_table[scalefactor[ch][sb]]) : 0;
		}
		
      }
    }

    for (sb = bound; sb < 32; ++sb) {
      if ((nb = allocation[0][sb])) {
		mad_fixed_t sample;

		sample = I_sample(&stream->ptr, nb);

		for (ch = 0; ch < nch; ++ch) {
		    //左右声道的样本分开存储
		    /*(*frame->sbsample[ch][s][sb]) =*/
			if (ch==0)
			{
				sampleL[s][sb] = (sample>>12)*sf_table[scalefactor[ch][sb]];
			}
			else
			{
				sampleR[s][sb] = (sample>>12)*sf_table[scalefactor[ch][sb]];
			}	    
		}
      }
      else {
		for (ch = 0; ch < nch; ++ch)
		    //左右声道的样本分开存储
		    //(*frame->sbsample[ch][s][sb]) = 0;
			if(ch)
				sampleL[s][sb] = 0;
			else
				sampleR[s][sb] = 0;
      }
    }
    
  }

  {
    extern void mad_synth_fouth_frame(struct mad_frame const *frame, int whichch,int ns,int gr);
	extern volatile int synth_hw_busy;      //硬件子带合成模块忙标志

    //左声道
	while(synth_hw_busy==1);        //wait for hardware finish
    mad_synth_fouth_frame(frame , 0 , 12 , 0);
	
	//右声道
	if (nch > 1)
	{
    	while(synth_hw_busy==1);        //wait for hardware finish
    	memcpy(&sampleL[0][0],&sampleR[0][0],32*12*sizeof(mad_fixed_t));
    	mad_synth_fouth_frame(frame , 1 , 12 , 0);
	}

    frame_sbsample_toggle(frame);
  }


//if ((*((volatile unsigned long*)0x40000000) & 0x00000040)  == 0)
	SWITCH_ARM_NOR();

  return 0;
}














/* --- Layer II ------------------------------------------------------------ */












extern struct {
  unsigned int sblimit;
  unsigned char const offsets[30];
} const sbquant_table[5] ;

extern struct {
  unsigned short nbal;
  unsigned short offset;
} const bitalloc_table[8] ;

extern unsigned char const offset_table[6][15] ;

extern struct quantclass {
  unsigned short nlevels;
  unsigned char group;
  unsigned char bits;
  mad_fixed_t C;
  mad_fixed_t D;
} const qc_table[17] ;

extern struct quantclass const qc_table_2[17] ;

/*
 * NAME:	II_samples()
 * DESCRIPTION:	decode three requantized Layer II samples from a bitstream
 */
_ATTR_MP3DEC_TEXT_
static
void II_samples(struct mad_bitptr *ptr,
		struct quantclass const *quantclass,
		mad_fixed_t output[3])
{
  unsigned int nb, s, sample[3];

  if ((nb = quantclass->group)) {
    unsigned int c, nlevels;

    /* degrouping */
    c = mad_bit_read(ptr, quantclass->bits);
    nlevels = quantclass->nlevels;

    for (s = 0; s < 3; ++s) {
      sample[s] = c % nlevels;
      c /= nlevels;
    }
  }
  else {
    nb = quantclass->bits;

    for (s = 0; s < 3; ++s)
      sample[s] = mad_bit_read(ptr, nb);
  }

  for (s = 0; s < 3; ++s) {
    mad_fixed_t requantized;

    /* invert most significant bit, extend sign, then scale to fixed format */

    requantized  = sample[s] ^ (1 << (nb - 1));
    requantized |= -(requantized & (1 << (nb - 1)));

    requantized <<= MAD_F_FRACBITS - (nb - 1);

    /* requantize the sample */

    /* s'' = C * (s''' + D) */

    //output[s] = ((requantized + quantclass->D)>>12) * quantclass->C;//mad_f_mul(requantized + quantclass->D, quantclass->C);
	//output[s] = (((requantized)>>12) + quantclass->D ) * quantclass->C;//mad_f_mul(requantized + quantclass->D, quantclass->C);
	//output[s] = mad_f_mul(requantized + quantclass->D, quantclass->C);
	//for old chip
	output[s] = (((requantized)>>12) + (quantclass->D<<4) ) * quantclass->C;//mad_f_mul(requantized + quantclass->D, quantclass->C);
	//for new chip
	//output[s] = (((requantized)>>12) + (quantclass->D>>12) ) * quantclass->C;//mad_f_mul(requantized + quantclass->D, quantclass->C);

    /* s' = factor * s'' */
    /* (to be performed by caller) */
  }
}

/*
_ATTR_MP3DEC_TEXT_
static
void II_samples_2(struct mad_bitptr *ptr,
		struct quantclass const *quantclass,
		mad_fixed_t output[3])
{
  output[0] = quantclass->C;
  output[1] = quantclass->D;
}
*/

/*
 * NAME:	layer->II()
 * DESCRIPTION:	decode a single Layer II frame
 */
_ATTR_MP3DEC_TEXT_
int mad_layer_II(struct mad_stream *stream, struct mad_frame *frame)
{
  struct mad_header *header = &frame->header;
  struct mad_bitptr start;
  unsigned int index, sblimit, nbal, nch, bound, gr, ch, s, sb;
  unsigned char const *offsets;
  unsigned char allocation[2][32], scfsi[2][32], scalefactor[2][32][3];
  mad_fixed_t samples[3];

  //对于layer 2,每个声道占用18个子带(与layer 3相同,但因为layer 2子带数据是左右两个声道一起产生,
  //必须使用存储buffer暂存,由此导致frame.h中sbsample数组须开到36组子带)
  mad_fixed_t (*sampleL)[32] = &(*frame->sbsample)[0];
  mad_fixed_t (*sampleR)[32] = &(*frame->sbsample)[18];

  nch = MAD_NCHANNELS(header);

//if ((*((volatile unsigned long*)0x40000000) & 0x00000040)  == 0)
	SWITCH_ARM_ACC();

  if (header->flags & MAD_FLAG_LSF_EXT)
    index = 4;
  else if (header->flags & MAD_FLAG_FREEFORMAT)
    goto freeformat;
  else {
    unsigned long bitrate_per_channel;

    bitrate_per_channel = header->bitrate;
    if (nch == 2) {
          bitrate_per_channel /= 2;
    
    # if defined(OPT_STRICT)
          /*
           * ISO/IEC 11172-3 allows only single channel mode for 32, 48, 56, and
           * 80 kbps bitrates in Layer II, but some encoders ignore this
           * restriction. We enforce it if OPT_STRICT is defined.
           */
          if (bitrate_per_channel <= 28000 || bitrate_per_channel == 40000) {
    	stream->error = MAD_ERROR_BADMODE;
    	return -1;
          }
    # endif
        }
    else {  /* nch == 1 */
    # if defined(OPT_STRICT)
          if (bitrate_per_channel > 192000) {
        	/*
        	 * ISO/IEC 11172-3 does not allow single channel mode for 224, 256,
        	 * 320, or 384 kbps bitrates in Layer II.
        	 */
        	stream->error = MAD_ERROR_BADMODE;
        	return -1;
          }
    # endif
        }

    if (bitrate_per_channel <= 48000)
        index = (header->samplerate == 32000) ? 3 : 2;
    else if (bitrate_per_channel <= 80000)
        index = 0;
    else {
      freeformat:
        index = (header->samplerate == 48000) ? 0 : 1;
    }
  }

  sblimit = sbquant_table[index].sblimit;
  offsets = sbquant_table[index].offsets;

  bound = 32;
  if (header->mode == MAD_MODE_JOINT_STEREO) {
    header->flags |= MAD_FLAG_I_STEREO;
    bound = 4 + header->mode_extension * 4;
  }

  if (bound > sblimit)
    bound = sblimit;

  start = stream->ptr;

  /* decode bit allocations */

  for (sb = 0; sb < bound; ++sb) {
    nbal = bitalloc_table[offsets[sb]].nbal;

    for (ch = 0; ch < nch; ++ch)
      allocation[ch][sb] = mad_bit_read(&stream->ptr, nbal);
  }

  for (sb = bound; sb < sblimit; ++sb) {
    nbal = bitalloc_table[offsets[sb]].nbal;

    allocation[0][sb] =
    allocation[1][sb] = mad_bit_read(&stream->ptr, nbal);
  }

  /* decode scalefactor selection info */

  for (sb = 0; sb < sblimit; ++sb) {
    for (ch = 0; ch < nch; ++ch) {
      if (allocation[ch][sb])
	scfsi[ch][sb] = (unsigned char)mad_bit_read(&stream->ptr, 2);
    }
  }

  /* check CRC word */

  if (header->flags & MAD_FLAG_PROTECTION) {
    header->crc_check =
      mad_bit_crc(start, mad_bit_length(&start, &stream->ptr),
		  header->crc_check);

    if (header->crc_check != header->crc_target &&
	!(frame->options & MAD_OPTION_IGNORECRC)) {
      stream->error = MAD_ERROR_BADCRC;
      return -1;
    }
  }

  /* decode scalefactors */

  for (sb = 0; sb < sblimit; ++sb) {
    for (ch = 0; ch < nch; ++ch) {
      if (allocation[ch][sb]) {
	scalefactor[ch][sb][0] = (unsigned char)mad_bit_read(&stream->ptr, 6);

	switch (scfsi[ch][sb]) {
	case 2:
	  scalefactor[ch][sb][2] =
	  scalefactor[ch][sb][1] =
	  scalefactor[ch][sb][0];
	  break;

	case 0:
	  scalefactor[ch][sb][1] = (unsigned char)mad_bit_read(&stream->ptr, 6);
	  /* fall through */

	case 1:
	case 3:
	  scalefactor[ch][sb][2] = (unsigned char)mad_bit_read(&stream->ptr, 6);
	}

	if (scfsi[ch][sb] & 1)
	  scalefactor[ch][sb][1] = scalefactor[ch][sb][scfsi[ch][sb] - 1];

# if defined(OPT_STRICT)
	/*
	 * Scalefactor index 63 does not appear in Table B.1 of
	 * ISO/IEC 11172-3. Nonetheless, other implementations accept it,
	 * so we only reject it if OPT_STRICT is defined.
	 */
	if (scalefactor[ch][sb][0] == 63 ||
	    scalefactor[ch][sb][1] == 63 ||
	    scalefactor[ch][sb][2] == 63) {
	  stream->error = MAD_ERROR_BADSCALEFACTOR;
	  return -1;
	}
# endif
      }
    }
  }

  /* decode samples */
  for (gr = 0; gr < 12; ++gr) {

    //在layer 2中,每帧有12个节,每节3个子带,每18个子带做一次子带合成
	int gr1 = (gr%6);

    for (sb = 0; sb < bound; ++sb) {
      for (ch = 0; ch < nch; ++ch) {
    		if ((index = allocation[ch][sb])) {
        		  index = offset_table[bitalloc_table[offsets[sb]].offset][index - 1];
        
        		  II_samples(&stream->ptr, &qc_table[index], samples);
        
        		  for (s = 0; s < 3; ++s) {	    
                        /*
                        ((*frame->sbsample)[ch][3 * gr + s][sb]) =
                          mad_f_mul(samples[s], sf_table[scalefactor[ch][sb][gr / 4]]);
                        */
        	          //左右声道分开存
        			  if (ch == 0)
        			  {
        				  sampleL[3 * gr1 + s][sb] = (samples[s]>>12)*sf_table[scalefactor[ch][sb][gr / 4]];
        			  }
        			  else
        			  {
        				  sampleR[3 * gr1 + s][sb] = (samples[s]>>12)*sf_table[scalefactor[ch][sb][gr / 4]];
        			  }
        		  }
        		}
        		else {
        		  for (s = 0; s < 3; ++s)
        			//((*frame->sbsample)[ch][3 * gr + s][sb]) = 0;
        			//左右声道分开存
        			if (ch == 0)
        				sampleL[3 * gr1 + s][sb] = 0;
        			else
        				sampleR[3 * gr1 + s][sb] = 0;
        		}
    	    }
		}

		for (sb = bound; sb < sblimit; ++sb) {
		  if ((index = allocation[0][sb])) {
    		index = offset_table[bitalloc_table[offsets[sb]].offset][index - 1];
    
    		II_samples(&stream->ptr, &qc_table[index], samples);
    
    		for (ch = 0; ch < nch; ++ch) {
    		  for (s = 0; s < 3; ++s) {
                    /*
                    (*frame->sbsample[ch][3 * gr + s][sb]) =
                      mad_f_mul(samples[s], sf_table[scalefactor[ch][sb][gr / 4]]);
                    */
                    //左右声道分开存
                    if (ch == 0)
                    {
                        sampleL[3 * gr1 + s][sb] = (samples[s]>>12)*sf_table[scalefactor[ch][sb][gr / 4]];
                    }
                    else
                    {
                        sampleR[3 * gr1 + s][sb] = (samples[s]>>12)*sf_table[scalefactor[ch][sb][gr / 4]];
                    }
    		  }
    		}
		  }
		  else {
		for (ch = 0; ch < nch; ++ch) {
		  for (s = 0; s < 3; ++s)
		    {
    			//(*frame->sbsample[ch][3 * gr + s][sb]) = 0;
    			//左右声道分开存
    			if (ch == 0)
    				sampleL[3 * gr1 + s][sb] = 0;
    			else
    				sampleR[3 * gr1 + s][sb] = 0;
			}
		}
		  }
		}

		for (ch = 0; ch < nch; ++ch) {
		  for (s = 0; s < 3; ++s) {
    		for (sb = sblimit; sb < 32; ++sb)
    		{
    			//((*frame->sbsample)[ch][3 * gr + s][sb]) = 0;
    			//左右声道分开存
    			if (ch == 0)
    				sampleL[3 * gr1 + s][sb] = 0;
    			else
    				sampleR[3 * gr1 + s][sb] = 0;
    		}
		  }
		}

        //在layer 2中,每帧有12个节,每节3个子带,每18个子带做一次子带合成
        //因此当gr1==5时进行子带合成
		if (gr1 == 5)
		{
			{
			    extern void mad_synth_fouth_frame(struct mad_frame const *frame, int whichch,int ns,int gr);
				extern volatile int synth_hw_busy;      //硬件子带合成模块忙标志
			
			    //左声道
				while(synth_hw_busy==1);        //wait for hardware finish
			    mad_synth_fouth_frame(frame , 0 , 18 , 0);

				//右声道
				if (nch > 1)
				{
				    while(synth_hw_busy==1);        //wait for hardware finish
					memcpy(&sampleL[0][0],&sampleR[0][0],32*18*sizeof(mad_fixed_t));
				    mad_synth_fouth_frame(frame , 1 , 18 , 0);
				}
								
			    frame_sbsample_toggle(frame);

				//对于layer 2,每个声道占用18个子带(与layer 3相同,但因为layer 2子带数据是左右两个声道一起产生,
  				//必须使用存储buffer暂存,由此导致frame.h中sbsample数组须开到36组子带)
  				sampleL = &(*frame->sbsample)[0];
  				sampleR = &(*frame->sbsample)[18];
			}
		}
  }

//if ((*((volatile unsigned long*)0x40000000) & 0x00000040)  == 0)
	SWITCH_ARM_NOR();  	

  return 0;
}
#endif
#endif

