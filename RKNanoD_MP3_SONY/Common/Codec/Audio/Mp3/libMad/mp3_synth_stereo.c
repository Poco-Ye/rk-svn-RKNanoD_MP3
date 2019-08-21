/*
********************************************************************************
*                   Copyright (c) 2008, Rockchip
*                         All rights reserved.
*
* File Name：   synth_stereo.c
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             Vincent Hsiung     2009-1-14         1.0
*    desc:    ORG.
********************************************************************************
*/

/*
 1. todo:mad_synth_mute() 将synth模块内fifo清零
 2. todo: 参数、接口需简化整合
*/

#include "../include/audio_main.h"

#ifdef A_CORE_DECODE
#ifdef MP3_DEC_INCLUDE

# include "mp3_global.h"
# include "mp3_fixed.h"
# include "mp3_frame.h"
# include "mp3_synth.h"
# include <string.h>
# include <stdio.h>

/*
 * NAME:	synth->init()
 * DESCRIPTION:	initialize synth struct
 */
_ATTR_MP3DEC_TEXT_
void mad_synth_init(struct mad_synth *synth)
{
  mad_synth_mute(synth);

  synth->phase = 0;

  synth->pcm.samplerate = 0;
  synth->pcm.channels   = 0;
  synth->pcm.length     = 0;  
}

/*
 * NAME:	synth->mute()
 * DESCRIPTION:	zero all polyphase filterbank values, resetting synthesis
 */
_ATTR_MP3DEC_TEXT_
void mad_synth_mute(struct mad_synth *synth)
{
  unsigned int ch, s, v;

  for (ch = 0; ch < 2; ++ch) {
    for (s = 0; s < 16; ++s) {
      for (v = 0; v < 8; ++v) {
//todo , by Vincent
#if 0
        synth->filter[ch][0][0][s][v] = synth->filter[ch][0][1][s][v] = 0;
	    synth->filter[ch][1][0][s][v] = synth->filter[ch][1][1][s][v] = 0;
#endif
      }
    }
  }
}

/*
 * NAME:	synth->frame()
 * DESCRIPTION:	perform PCM synthesis of frame subband samples
 */
_ATTR_MP3DEC_TEXT_
void mad_synth_frame(struct mad_synth *synth, struct mad_frame const *frame)
{
  unsigned int nch, ns;
  
  nch = MAD_NCHANNELS(&frame->header);
  ns  = MAD_NSBSAMPLES(&frame->header);

  synth->pcm.samplerate = frame->header.samplerate;
  
  synth->pcm.channels   = nch;
  synth->pcm.length     = 32 * ns;

  synth->phase = (synth->phase + ns) % 16;
}

/*
	Add by Vincent @ Oct 20 , 2008
	mad_synth_fouth_frame()
*/
//void mad_synth_fouth_frame(struct mad_frame const *frame, int whichch,int ns,int gr)
_ATTR_MP3DEC_TEXT_
void mad_synth_fouth_frame(struct mad_frame *frame, int whichch,int ns)
{
  //unsigned int nch;  
  //nch = MAD_NCHANNELS(&frame->header);

  synth_fouth_frame_hw(frame,(mad_fixed_t(*)[32])frame->sbsample,whichch, ns);

}

#endif
#endif

