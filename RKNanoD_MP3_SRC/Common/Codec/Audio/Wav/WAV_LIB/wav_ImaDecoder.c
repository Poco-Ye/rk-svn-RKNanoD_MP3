/* Copyright (C) 2007 ROCK-CHIPS FUZHOU . All Rights Reserved. */
/*
File : \Audio\ADPCM
Desc : WAV解码。包括PCM WAV , IMA-ADPCM WAV , MS-ADPCM WAV 。

Author : FSH , Vincent Hisung
Date : 2007-08-xx
Notes :

$Log :
* vincent     2007/08/xx 建立此文件
*
*/
/****************************************************************/

#include "../include/audio_main.h"

#ifdef A_CORE_DECODE
#ifdef WAV_DEC_INCLUDE

//#include "../Buffer/buffer.h"
#include "PCM.H"

#include "../include/audio_file_access.h"

//extern FILE *pRawFileCache;
  extern  unsigned long SRC_Num_Forehead; //for src
  extern  short  *WavoutBuf[2];
// -------------------------------------------------------
//  IMA DECODE
// -------------------------------------------------------
_ATTR_WAVDEC_DATA_
static int v_index_adjust[8] = {-1,-1,-1,-1,2,4,6,8};

_ATTR_WAVDEC_DATA_
static int v_step_table[89] = {7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25,
    28, 31, 34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130, 143,
    157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658,
    724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749,
    3024, 3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487,
    12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
    };



_ATTR_WAVDEC_TEXT_
short decode_ima_adpcm_one(char code,int *index,short cur_sample)
{
    int delta;
    char sb;
    int linear=cur_sample;
    int ss;

    if ((code & 8) != 0)
        sb=1;
    else
        sb=0;

    /* e = (adpcm+0.5)*step/4 */
    //delta=(step_table[*index]*code)/4 + step_table[*index]/8;
    //if (sb==1)
    //    delta=(-delta);
    
    //by Vincent , for error case  @ May 5, 2009
    if (*index > 88)
    {
		return 0;
	}
    

    ss = v_step_table[*index];
    delta = ss >> 3;
    if (code & 0x01)
        delta += (ss >> 2);

    if (code & 0x02)
        delta += (ss >> 1);

    if (code & 0x04)
        delta += ss;

    if (code & 0x08)
        delta = (-delta);

    linear+=delta;            // 计算出当前的波形数据

    if (linear>32767) linear=32767;
    else if (linear<-32768) linear=( -32768);
    code=(code&0x7);
    (*index)+=v_index_adjust[code];
    if ((*index)<0) (*index)=0;
    if ((*index)>88) (*index)=88;
    cur_sample=linear;
    return cur_sample;
}

_ATTR_WAVDEC_TEXT_
void decode_ima_adpcm(char in[],long int in_buf_size,short out[],short outR[],int Channels,int blocksize)
{
    short cur_sample[2];
    int index[2];
    int j[2];

    int i=0;    		//读到的位置

    int code;   		//读取到的ADPCM码
    int CurChannel=0;   //当前的Channel编号
    int p;

    for (p=0;p<Channels;p++)
    {
        cur_sample[p]=0;
        index[p]=0;
        j[p]=0;
    }

    while(i<=in_buf_size)
    {
        if (i>=blocksize)	//Bug fix: Important! by Vincent , 2007 08 14
        	{
        		break;
        	}
        if (i==0)
        {
            for (CurChannel=0;CurChannel<Channels;CurChannel++)
                {
                    short *pshort=(short *)in;
                    if (CurChannel==0)
                    {
                    	cur_sample[CurChannel]=out[(j[CurChannel]++)]=pshort[i/2];
                    }
                    else
                    {
                    	cur_sample[CurChannel]=outR[(j[CurChannel]++)]=pshort[i/2];
                    }
                    i+=2;
                    index[CurChannel]=pshort[i/2];
                    i+=2;
                }
        }
        for (CurChannel=0;CurChannel<Channels;CurChannel++)
        {
            if (CurChannel==0)
            	{
		            code=((in[i]))&0x0f;
		            cur_sample[CurChannel]=out[j[CurChannel]++]=decode_ima_adpcm_one(code,&index[CurChannel],cur_sample[CurChannel]);
		            code=(in[i]>>4)&0x0f;
		            cur_sample[CurChannel]=out[j[CurChannel]++]=decode_ima_adpcm_one(code,&index[CurChannel],cur_sample[CurChannel]);
		            i++;
		            code=((in[i]))&0x0f;
		            cur_sample[CurChannel]=out[j[CurChannel]++]=decode_ima_adpcm_one(code,&index[CurChannel],cur_sample[CurChannel]);
		            code=(in[i]>>4)&0x0f;
		            cur_sample[CurChannel]=out[j[CurChannel]++]=decode_ima_adpcm_one(code,&index[CurChannel],cur_sample[CurChannel]);
		            i++;
		            code=((in[i]))&0x0f;
		            cur_sample[CurChannel]=out[j[CurChannel]++]=decode_ima_adpcm_one(code,&index[CurChannel],cur_sample[CurChannel]);
		            code=(in[i]>>4)&0x0f;
		            cur_sample[CurChannel]=out[j[CurChannel]++]=decode_ima_adpcm_one(code,&index[CurChannel],cur_sample[CurChannel]);
		            i++;
		            code=((in[i]))&0x0f;
		            cur_sample[CurChannel]=out[j[CurChannel]++]=decode_ima_adpcm_one(code,&index[CurChannel],cur_sample[CurChannel]);
		            code=(in[i]>>4)&0x0f;
		            cur_sample[CurChannel]=out[j[CurChannel]++]=decode_ima_adpcm_one(code,&index[CurChannel],cur_sample[CurChannel]);
		            i++;
				}
				else
				{
		            code=((in[i]))&0x0f;
		            cur_sample[CurChannel]=outR[j[CurChannel]++]=decode_ima_adpcm_one(code,&index[CurChannel],cur_sample[CurChannel]);
		            code=(in[i]>>4)&0x0f;
		            cur_sample[CurChannel]=outR[j[CurChannel]++]=decode_ima_adpcm_one(code,&index[CurChannel],cur_sample[CurChannel]);
		            i++;
		            code=((in[i]))&0x0f;
		            cur_sample[CurChannel]=outR[j[CurChannel]++]=decode_ima_adpcm_one(code,&index[CurChannel],cur_sample[CurChannel]);
		            code=(in[i]>>4)&0x0f;
		            cur_sample[CurChannel]=outR[j[CurChannel]++]=decode_ima_adpcm_one(code,&index[CurChannel],cur_sample[CurChannel]);
		            i++;
		            code=((in[i]))&0x0f;
		            cur_sample[CurChannel]=outR[j[CurChannel]++]=decode_ima_adpcm_one(code,&index[CurChannel],cur_sample[CurChannel]);
		            code=(in[i]>>4)&0x0f;
		            cur_sample[CurChannel]=outR[j[CurChannel]++]=decode_ima_adpcm_one(code,&index[CurChannel],cur_sample[CurChannel]);
		            i++;
		            code=((in[i]))&0x0f;
		            cur_sample[CurChannel]=outR[j[CurChannel]++]=decode_ima_adpcm_one(code,&index[CurChannel],cur_sample[CurChannel]);
		            code=(in[i]>>4)&0x0f;
		            cur_sample[CurChannel]=outR[j[CurChannel]++]=decode_ima_adpcm_one(code,&index[CurChannel],cur_sample[CurChannel]);
		            i++;
				}
        }
    }
}

//****************************************************************************
//
// IMA-ADPCM format file   Decoding 
//
//****************************************************************************
_ATTR_WAVDEC_BSS_
short *ima_tmp;	//short ima_tmp[WAV_IMAMAX_PCM_LENGTH];

extern char fdata_buf[14312];

_ATTR_WAVDEC_TEXT_
unsigned long IMAADPCM_FORMAT(tPCM *pPCM)
{
	long lLength, lBlockSize;

	long NumOfSamples;
	volatile long	generatedSampleSize;
	long	ReadDataSize;
	long	RequestDataSize;
	unsigned int Counter = 0;
	int i;

	ima_tmp = (short*)&fdata_buf[0];

	if(((pPCM->usOffset + pPCM->usBytesPerBlock) > pPCM->usValid) 
		&& (pPCM->usValid < pPCM->ulDataValid))
	{
		/* Copy the tail of the buffer to the beginning. */
		memcpy(pPCM->pucEncodedData,
			pPCM->pucEncodedData + (pPCM->usOffset  & ~3),
			pPCM->usValid - (pPCM->usOffset & ~3));

		/* Update the number of valid bytes in the buffer. */
		pPCM->usValid -= (pPCM->usOffset  & ~3);

		/* Read more data into the buffer. */
		RequestDataSize = (MSADPCM_MAX_ENCBUF_LENGTH+512 - pPCM->usValid) & ~511;

		ReadDataSize = RKFIO_FRead( pPCM->pucEncodedData + pPCM->usValid, RequestDataSize , pRawFileCache );

		if ( RequestDataSize != ReadDataSize)
		{
			pPCM->ulDataValid = 0;	 
			return 0;
		}
		else
			pPCM->ulDataValid -= ReadDataSize;

		/* Update the number of valid bytes in the buffer. */
		pPCM->usValid += ReadDataSize;


		/* Set the current offset to the beginning of the buffer. */
		pPCM->usOffset &= 3;
	}
	
	if((pPCM->usValid - pPCM->usOffset) >=	pPCM->usBytesPerBlock)
		lBlockSize = pPCM->usBytesPerBlock;
	else
		lBlockSize = pPCM->usValid - pPCM->usOffset;

	if ( lBlockSize == 0 )
		return 0;

	if( pPCM->ucChannels == 1 )
		generatedSampleSize = (lBlockSize - 4) * 8/pPCM->sPCMHeader.uBitsPerSample ;
	else
		generatedSampleSize = (lBlockSize - 8) * 4/pPCM->sPCMHeader.uBitsPerSample ;

	if(generatedSampleSize == 0)
		return 0;

	/* Decode the next block. */	

	decode_ima_adpcm((char*)pPCM->pucEncodedData + pPCM->usOffset,
	                (long int)lBlockSize*4,
	                //(short*) (&pPCM->FrameOutputLeft[0]),
	                //(short*) (&pPCM->FrameOutputRight[0]),
					(short*)(&WavoutBuf[pPCM->CurDecodeIdx][SRC_Num_Forehead]),//WAV_IMAMAX_PCM_LENGTH
					//(short*)(&WavoutBuf[pPCM->CurDecodeIdx][WAV_IMAMAX_PCM_LENGTH]),//WAV_IMAMAX_PCM_LENGTH
					(short*)(&ima_tmp[0]),
	                pPCM->sPCMHeader.ucChannels,
	                lBlockSize);

  pPCM->usOffset += (unsigned short)lBlockSize;

#if 0
	NumOfSamples = pPCM->N_remain +generatedSampleSize;

	if( lLength>NumOfSamples)
		lLength = NumOfSamples;
	
	pPCM->N_GeneratedSample = (int)(lLength - lLength%4);

	/* Increment the time based on the number of samples. */
	pPCM->ulTimePos += pPCM->N_GeneratedSample;
#endif

#if 0
{
		// three times more
		int i;
		for (i = 0 ; i < 3 ; i ++ )
		{
			decode_ima_adpcm((char*)pPCM->pucEncodedData + pPCM->usOffset,
		                (long int)lBlockSize*4,
										(short*)(&WavoutBuf[pPCM->CurDecodeIdx][generatedSampleSize * i]),//WAV_IMAMAX_PCM_LENGTH
										(short*)(&ima_tmp[generatedSampleSize * i]),
		                pPCM->sPCMHeader.ucChannels,
		                lBlockSize);
			pPCM->usOffset += (unsigned short)lBlockSize;
		}
		
		NumOfSamples = pPCM->N_remain + generatedSampleSize*4;

		//if( lLength>NumOfSamples)
		lLength = NumOfSamples;
		
		pPCM->N_GeneratedSample = (int)(lLength - lLength%4);
	
		/* Increment the time based on the number of samples. */
		pPCM->ulTimePos += pPCM->N_GeneratedSample;
}
#endif

	{
		int i;
		short *pL,*pR,*pO;

		//generatedSampleSize *=4 ;

		pO = &WavoutBuf[pPCM->CurDecodeIdx][SRC_Num_Forehead];
		pL = &WavoutBuf[pPCM->CurDecodeIdx][SRC_Num_Forehead];
		pR = &ima_tmp[0];
		
		for (i = generatedSampleSize - 1 ; i > 0 ; i--)
		{
			pL[2*i] = pL[i];
		}

		if	(pPCM->ucChannels == 1)
		{
			for (i = 0; i < generatedSampleSize ; i++)
			{
				pO[2*i + 1] = pO[2*i];
			}
		}
		else
		{
			for (i = 0; i < generatedSampleSize ; i++)
			{
				pO[2*i + 1] = pR[i];
			}
		}
	}
	
	return generatedSampleSize;
}
#endif
#endif
