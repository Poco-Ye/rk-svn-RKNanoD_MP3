/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis 'TREMOR' CODEC SOURCE CODE.   *
 *                                                                  *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis 'TREMOR' SOURCE CODE IS (C) COPYRIGHT 1994-2003    *
 * BY THE Xiph.Org FOUNDATION http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: PCM data vector blocking, windowing and dis/reassembly

 ********************************************************************/


#include <stdlib.h> 
#include "ogg.h"
#include "ivorbiscodec.h"
#include "codec_internal.h"



#ifdef A_CORE_DECODE
#ifdef OGG_DEC_INCLUDE

#pragma arm section code = "OggDecCode", rodata = "OggDecCode", rwdata = "OggDecData", zidata = "OggDecBss"
#include "misc.h"

#include "window_lookup.h"

#include "ogg_bitwise.c"
vorbis_dsp_state dsp_st={0};
#ifdef NANOC_DECODE
ogg_int32_t work[2][1024]={0};//channel blocksize/2  最大8192/2
ogg_int32_t mdctright[2][512]={0};//channel blocksize/4   最大8192/4
#else
ogg_int32_t work[2][4096]={0};//channel blocksize/2  最大8192/2
ogg_int32_t mdctright[2][2048]={0};//channel blocksize/4   最大8192/4
#endif
ogg_int32_t *p_work[2];
ogg_int32_t *p_mdctright[2];

int vorbis_dsp_restart(vorbis_dsp_state *v){
  if(!v)return -1;
  {
    vorbis_info *vi=v->vi;
    codec_setup_info *ci;
    
    if(!vi)return -1;
    ci=vi->codec_setup;
    if(!ci)return -1;
    
    v->out_end=-1;
    v->out_begin=-1;

    v->granulepos=-1;
    v->sequence=-1;
    v->sample_count=-1;
  }
  return 0;
}


vorbis_dsp_state *vorbis_dsp_create(vorbis_info *vi)
{
  int i;

  vorbis_dsp_state *v=&dsp_st;
  codec_setup_info *ci=(codec_setup_info *)vi->codec_setup;

  v->vi=vi;

  for(i=0;i<vi->channels;i++)
  {
   p_work[i] = work[i];	
   p_mdctright[i]=mdctright[i];   
  }
  v->work =p_work;
  v->mdctright = p_mdctright;
  
  v->lW=0; /* previous window size */
  v->W=0;  /* current window size */

  vorbis_dsp_restart(v);
  return v;
}

void vorbis_dsp_destroy(vorbis_dsp_state *v){
  int i;
  if(v){
    vorbis_info *vi=v->vi; 
	
	v->work = NULL;
	v->mdctright = NULL;
	ogg_MemSet(work,0,2*1024);	
	ogg_MemSet(mdctright,0,2*512);	
   v = NULL;
   ogg_MemSet(&dsp_st,0,sizeof(vorbis_dsp_state));
  }
}

 LOOKUP_T *_vorbis_window(int left){
  switch(left){
  case 32:
    return vwin64;
  case 64:
    return vwin128;
  case 128:
    return vwin256;
  case 256:
    return vwin512;
  case 512:
    return vwin1024;
  case 1024:
    return vwin2048;
#ifndef NANOC_DECODE
  case 2048:
    return vwin4096;
#ifndef LIMIT_TO_64kHz
  case 4096:
    return vwin8192;
#endif
#endif
  default:
    return(0);
  }
}

/* pcm==0 indicates we just want the pending samples, no more */
int vorbis_dsp_pcmout(vorbis_dsp_state *v,ogg_int16_t *pcm,int samples){
  vorbis_info *vi=v->vi;
  codec_setup_info *ci=(codec_setup_info *)vi->codec_setup;
  if(v->out_begin>-1 && v->out_begin<v->out_end){
    int n=v->out_end-v->out_begin;
    if(pcm){
      int i;
      if(n>samples)n=samples;
      for(i=0;i<vi->channels;i++)
//MDCT2FFT
#if 1
	mdct_unroll_lap(ci->blocksizes[0],ci->blocksizes[1],
			v->lW,v->W,v->work[i],v->mdctright[i],
			_vorbis_window(ci->blocksizes[0]>>1),
			_vorbis_window(ci->blocksizes[1]>>1),
			pcm+i,vi->channels,
			v->out_begin,v->out_begin+n);
#else
	imdct_sort_pcmout(ci->blocksizes[0],ci->blocksizes[1],
 			v->lW,v->W,v->work[i],v->mdctright[i],
 			_vorbis_window(ci->blocksizes[0]>>1),
 			_vorbis_window(ci->blocksizes[1]>>1),
 			pcm+i,vi->channels,
 			v->out_begin,v->out_begin+n);
#endif
    }
    return(n);
  }
  return(0);
}

int vorbis_dsp_read(vorbis_dsp_state *v,int s){
  if(s && v->out_begin+s>v->out_end)return(OV_EINVAL);
  v->out_begin+=s;
  return(0);
}

long vorbis_packet_blocksize(vorbis_info *vi,ogg_packet *op){
  codec_setup_info     *ci=(codec_setup_info *)vi->codec_setup;
  oggpack_buffer       opb;
  int                  mode;
  int modebits=0;
  int v=ci->modes;
 
  oggpack_readinit(&opb,op->packet);

  /* Check the packet type */
  if(oggpack_read(&opb,1)!=0){
    /* Oops.  This is not an audio data packet */
    return(OV_ENOTAUDIO);
  }

  while(v>1){
    modebits++;
    v>>=1;
  }

  /* read our mode and pre/post windowsize */
  mode=oggpack_read(&opb,modebits);
  if(mode==-1)return(OV_EBADPACKET);
  return(ci->blocksizes[ci->mode_param[mode].blockflag]);
}


 int ilog(ogg_uint32_t v){
  int ret=0;
  if(v)--v;
  while(v){
    ret++;
    v>>=1;
  }
  return(ret);
}

int vorbis_dsp_synthesis(vorbis_dsp_state *vd,ogg_packet *op,int decodep){
  int                   mode,i;
  vorbis_info          *vi=vd->vi;
  codec_setup_info     *ci=(codec_setup_info *)vi->codec_setup;  

  oggpack_readinit(&vd->opb,op->packet);

  /* Check the packet type */
  if(oggpack_read(&vd->opb,1)!=0)
  {
    /* Oops.  This is not an audio data packet */
    return OV_ENOTAUDIO ;
  }

  /* read our mode and pre/post windowsize */
  mode=oggpack_read(&vd->opb,ilog(ci->modes));
  if(mode==-1 || mode>=ci->modes) 
  {
    return OV_EBADPACKET;
  }

  /* shift information we still need from last window */
  vd->lW=vd->W;
  vd->W=ci->mode_param[mode].blockflag;
  for(i=0;i<vi->channels;i++)
  {
//MDCT2FFT
#if 1
    mdct_shift_right(ci->blocksizes[vd->lW],vd->work[i],vd->mdctright[i]);
#else
    imdct_shift_right(ci->blocksizes[vd->lW],vd->work[i],vd->mdctright[i]);
#endif
  }

  if(vd->W){
    int temp;
    oggpack_read(&vd->opb,1);
    temp=oggpack_read(&vd->opb,1);
    if(temp==-1)
	{
	   return OV_EBADPACKET;
	}
  }
  
  /* packet decode and portions of synthesis that rely on only this block */
  if(decodep){
    mapping_inverse(vd,ci->map_param+ci->mode_param[mode].mapping);

    if(vd->out_begin==-1){
      vd->out_begin=0;
      vd->out_end=0;
    }else{
      vd->out_begin=0;
      vd->out_end=ci->blocksizes[vd->lW]/4+ci->blocksizes[vd->W]/4;
    }
  }

  /* track the frame number... This is for convenience, but also
     making sure our last packet doesn't end with added padding.
     
     This is not foolproof!  It will be confused if we begin
     decoding at the last page after a seek or hole.  In that case,
     we don't have a starting point to judge where the last frame
     is.  For this reason, vorbisfile will always try to make sure
     it reads the last two marked pages in proper sequence */
  
  /* if we're out of sequence, dump granpos tracking until we sync back up */
  if(vd->sequence==-1 || vd->sequence+1 != op->packetno-3){
    /* out of sequence; lose count */
    vd->granulepos=-1;
    vd->sample_count=-1;
  }
  
  vd->sequence=op->packetno;
  vd->sequence=vd->sequence-3;
  
  if(vd->sample_count==-1){
    vd->sample_count=0;
  }else{
    vd->sample_count+=
      ci->blocksizes[vd->lW]/4+ci->blocksizes[vd->W]/4;
  }
  
  if(vd->granulepos==-1){
    if(op->granulepos!=-1){ /* only set if we have a
			       position to set to */
      
      vd->granulepos=op->granulepos;
      
      /* is this a short page? */
      if(vd->sample_count>vd->granulepos){
	/* corner case; if this is both the first and last audio page,
	   then spec says the end is cut, not beginning */
	if(op->e_o_s){
	  /* trim the end */
	  /* no preceeding granulepos; assume we started at zero (we'd
	     have to in a short single-page stream) */
	  /* granulepos could be -1 due to a seek, but that would result
	     in a long coun t, not short count */
	  
	  vd->out_end-=vd->sample_count-vd->granulepos;
	}else{
	  /* trim the beginning */
	  vd->out_begin+=vd->sample_count-vd->granulepos;
	  if(vd->out_begin>vd->out_end)
	    vd->out_begin=vd->out_end;
	}
	
      }
      
    }
  }else{
    vd->granulepos+=
      ci->blocksizes[vd->lW]/4+ci->blocksizes[vd->W]/4;
    if(op->granulepos!=-1 && vd->granulepos!=op->granulepos){
      
      if(vd->granulepos>op->granulepos){
	long extra=vd->granulepos-op->granulepos;
	
	if(extra)
	  if(op->e_o_s){
	    /* partial last frame.  Strip the extra samples off */
	    vd->out_end-=extra;
	  } /* else {Shouldn't happen *unless* the bitstream is out of
	       spec.  Either way, believe the bitstream } */
      } /* else {Shouldn't happen *unless* the bitstream is out of
	   spec.  Either way, believe the bitstream } */
      vd->granulepos=op->granulepos;
    }
  }

  return(0);
}
#pragma arm section code

#endif
#endif

