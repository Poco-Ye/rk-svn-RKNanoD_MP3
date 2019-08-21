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

 function: channel mapping 0 implementation

 ********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "ogg.h"
#include "os.h"
#include "ivorbiscodec.h"
#include "codec_internal.h"
#include "codebook.h"
#include "misc.h"

#ifdef A_CORE_DECODE
#ifdef OGG_DEC_INCLUDE
				 
#pragma arm section code = "OggDecCode", rodata = "OggDecCode", rwdata = "OggDecData", zidata = "OggDecBss"



void mapping_clear_info(vorbis_info_mapping *info){
  ogg_MemSet(info->chmuxlist,0,2);
  ogg_MemSet(info->submaplist,0,2);
  ogg_MemSet(info->coupling,0,2);
  ogg_MemSet(info,0,sizeof(*info));
  
}
/*
 int ilog(unsigned int v){
  int ret=0;
  if(v)--v;
  while(v){
    ret++;
    v>>=1;
  }
  return(ret);
}
*/
/* also responsible for range checking */
int mapping_info_unpack(vorbis_info_mapping *info,vorbis_info *vi,
			oggpack_buffer *opb){
  int i;
  codec_setup_info     *ci=(codec_setup_info *)vi->codec_setup;
  ogg_MemSet(info,0,sizeof(*info));

  if(oggpack_read(opb,1))
    info->submaps=oggpack_read(opb,4)+1;
  else
    info->submaps=1;

  if(oggpack_read(opb,1)){
    info->coupling_steps=oggpack_read(opb,8)+1;
   
   // info->coupling= coupling;
    for(i=0;i<info->coupling_steps;i++){
      int testM=info->coupling[i].mag=oggpack_read(opb,ilog(vi->channels));
      int testA=info->coupling[i].ang=oggpack_read(opb,ilog(vi->channels));

      if(testM<0 || 
	 testA<0 || 
	 testM==testA || 
	 testM>=vi->channels ||
	 testA>=vi->channels) goto err_out;
    }

  }

  if(oggpack_read(opb,2)>0)goto err_out; /* 2,3:reserved */
    
  if(info->submaps>1){    
   // info->chmuxlist=chmuxlist;
	for(i=0;i<vi->channels;i++){
      info->chmuxlist[i]=oggpack_read(opb,4);
      if(info->chmuxlist[i]>=info->submaps)goto err_out;
    }
  } 
 // info->submaplist= submaplist;
  for(i=0;i<info->submaps;i++){
    int temp=oggpack_read(opb,8);
    info->submaplist[i].floor=oggpack_read(opb,8);
    if(info->submaplist[i].floor>=ci->floors)goto err_out;
    info->submaplist[i].residue=oggpack_read(opb,8);
    if(info->submaplist[i].residue>=ci->residues)goto err_out;
  }
  return 0;

 err_out:
  mapping_clear_info(info);
  return -1;
}



int mapping_inverse(vorbis_dsp_state *vd,vorbis_info_mapping *info){
  vorbis_info          *vi=vd->vi;

  ogg_int32_t *pcmbundle[2]; 
  int  zerobundle[2];
  int  nonzero[2];
  ogg_int32_t *floormemo[2];
  ogg_int32_t floormemo_s[2][36];
  codec_setup_info     *ci=(codec_setup_info *)vi->codec_setup;

  int                   i,j;
  long                  n=ci->blocksizes[vd->W];

  /* recover the spectral envelope; store it in the PCM vector for now */
  for(i=0;i<vi->channels;i++){
    int submap=0;
    int floorno;
    
    if(info->submaps>1)
      submap=info->chmuxlist[i];
    floorno=info->submaplist[submap].floor;
    
    if(ci->floor_type[floorno]){
      /* floor 1 */  
      if(floor1_memosize(ci->floor_param[floorno]) >36)
	  {
	    printf("floormemo need %d \n",floor1_memosize(ci->floor_param[floorno]));
	  }
	  floormemo[i]= floormemo_s[i];
      floormemo[i]=floor1_inverse1(vd,ci->floor_param[floorno],floormemo[i]);    
    }else{
      /* floor 0 */ 
      if(floor0_memosize(ci->floor_param[floorno]) >36)
	  {
	    printf("floormemo need %d \n",floor0_memosize(ci->floor_param[floorno]));
	  }
	  floormemo[i]=floormemo_s[i];
      floormemo[i]=floor0_inverse1(vd,ci->floor_param[floorno],floormemo[i]);
   
    }
    
    if(floormemo[i])
      nonzero[i]=1;
    else
      nonzero[i]=0;      
    ogg_MemSet(vd->work[i],0,sizeof(*vd->work[i])*n/2);
  }

  /* channel coupling can 'dirty' the nonzero listing */
  for(i=0;i<info->coupling_steps;i++){
    if(nonzero[info->coupling[i].mag] ||
       nonzero[info->coupling[i].ang]){
      nonzero[info->coupling[i].mag]=1; 
      nonzero[info->coupling[i].ang]=1; 
    }
  }

  /* recover the residue into our working vectors */
  for(i=0;i<info->submaps;i++){
    int ch_in_bundle=0;
    for(j=0;j<vi->channels;j++){
      if(!info->chmuxlist || info->chmuxlist[j]==i){
	if(nonzero[j])
	  zerobundle[ch_in_bundle]=1;
	else
	  zerobundle[ch_in_bundle]=0;
	pcmbundle[ch_in_bundle++]=vd->work[j];
      }
    }
    
    res_inverse(vd,ci->residue_param+info->submaplist[i].residue,
		pcmbundle,zerobundle,ch_in_bundle);
  }

  //for(j=0;j<vi->channels;j++)
  //_analysis_output("coupled",seq+j,vb->pcm[j],-8,n/2,0,0);

  /* channel coupling */
  for(i=info->coupling_steps-1;i>=0;i--){
    ogg_int32_t *pcmM=vd->work[info->coupling[i].mag];
    ogg_int32_t *pcmA=vd->work[info->coupling[i].ang];
    
    for(j=0;j<n/2;j++){
      ogg_int32_t mag=pcmM[j];
      ogg_int32_t ang=pcmA[j];
      
      if(mag>0)
	if(ang>0){
	  pcmM[j]=mag;
	  pcmA[j]=mag-ang;
	}else{
	  pcmA[j]=mag;
	  pcmM[j]=mag+ang;
	}
      else
	if(ang>0){
	  pcmM[j]=mag;
	  pcmA[j]=mag+ang;
	}else{
	  pcmA[j]=mag;
	  pcmM[j]=mag-ang;
	}
    }
  }

  /* compute and apply spectral envelope */
  for(i=0;i<vi->channels;i++){
    ogg_int32_t *pcm=vd->work[i];
    int submap=0;
    int floorno;

    if(info->submaps>1)
      submap=info->chmuxlist[i];
    floorno=info->submaplist[submap].floor;

    if(ci->floor_type[floorno]){
      /* floor 1 */
      floor1_inverse2(vd,ci->floor_param[floorno],floormemo[i],pcm);
    }else{
      /* floor 0 */
      floor0_inverse2(vd,ci->floor_param[floorno],floormemo[i],pcm);
    }
  }

  //for(j=0;j<vi->channels;j++)
  //_analysis_output("mdct",seq+j,vb->pcm[j],-24,n/2,0,1);

  /* transform the PCM data; takes PCM vector, vb; modifies PCM vector */
  /* only MDCT right now.... */
  for(i=0;i<vi->channels;i++)
//MDCT2FFT
#if 1
    mdct_backward(n,vd->work[i]);
#else
    imdct(n, vd->work[i]);
#endif

  //for(j=0;j<vi->channels;j++)
  //_analysis_output("imdct",seq+j,vb->pcm[j],-24,n,0,0);

  /* all done! */
  //RK_ogg_alloca_exit();

   for(i=0;i<vi->channels;i++)
  {
  	floormemo[i] = NULL;
  }   
  return(0);
}

#pragma arm section code

#endif
#endif

