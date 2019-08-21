/*
********************************************************************************
*                   Copyright (c) 2013,Tiantian
*                         All rights reserved.
*
* File Name：   sbc_interface.c
*
* Description:  SBC解码流程控制
*
* History:      <author>          <time>        <version>
*               Tiantian        2013-3-23          1.0
*    desc:    ORG.
********************************************************************************
*/
#include "SysInclude.h"
#include "audio_main.h"

//#include <sys/stat.h>
#include "sbc_interface.h"

#include "audio_file_access.h"
#include "resample_interface.h"
#include "Src.h"
#ifdef SBC_INCLUDE


#define IN_BUF_SIZE 512
#define OUT_BUF_SIZE (4096*2)//2048//2048
#define SRC_OUT_BUF_SIZE ((3584-512)*2)//2048//2048


//#define OUT_BUF_SIZE (4096+1024)//2048//2048
//#define SRC_OUT_BUF_SIZE (3584)//2048//2048

#define BUF_RESERVED_LAST_DATA (700)
#define BUF_RESERVED_OUT       (860)
#define BUF_RESERVED_HEAD      (100)


#define BUF_OFF_SET  (BUF_RESERVED_LAST_DATA+BUF_RESERVED_LAST_DATA+BUF_RESERVED_OUT+BUF_RESERVED_HEAD)
//#define OUT_BUF_SIZE 1024
//------------------global varibles----------------------------------
_ATTR_SBCDEC_BSS_ static sbc_t gSbc;

/*Audio data formats, Set it according to the actual situation.*/
_ATTR_SBCDEC_BSS_ static int g_frequency;
_ATTR_SBCDEC_BSS_ static int g_channels;

_ATTR_SBCDEC_BSS_ static unsigned int g_sbcBufIndex;
//_ATTR_SBCDEC_BSS_ static long g_fileSize;
_ATTR_SBCDEC_BSS_ static int g_outLength;
_ATTR_SBCDEC_BSS_ static unsigned int g_frameLength;

//the unit of bit rate is kb/s.
_ATTR_SBCDEC_BSS_ static int g_bitrate;

// The length of the file in milliseconds.
_ATTR_SBCDEC_BSS_ static long g_timeLength;

// The number of samples that have been encoded/decoded.
_ATTR_SBCDEC_BSS_ static long g_timePos;

_ATTR_SBCDEC_BSS_ __align(4) static unsigned char gInbuf[IN_BUF_SIZE];
_ATTR_SBCDEC_BSS_ __align(4) static unsigned char gSbcOutputPtr [2][OUT_BUF_SIZE+BUF_OFF_SET];

_ATTR_SBCDEC_BSS_ static long g_unpacked_data_size;
//FILE *pRawFileCache;
//-------------------------------------------------------------------
#ifdef SSRC
_ATTR_SBCDEC_BSS_  static unsigned char  out_put[OUT_BUF_SIZE*2+1024+512];
//_ATTR_SBCDEC_BSS_  static unsigned char  out_put_temp[OUT_BUF_SIZE+1024+1024];
_ATTR_SBCDEC_DATA_ static int resample_pos = 0;
_ATTR_SBCDEC_DATA_ static uint8 isResampleEnable = 0;
_ATTR_SBCDEC_DATA_ static SRCState *pSRCpcb;



_ATTR_SBCDEC_TEXT_
void sbc_resample_enable(void)
{
    FREQ_EnterModule(FREQ_SSRC);
    isResampleEnable = 1;    
}

_ATTR_SBCDEC_TEXT_
void sbc_resample_disable(void)
{
    if(isResampleEnable)
    {
        isResampleEnable = 0; 
        FREQ_ExitModule(FREQ_SSRC);
    }
}
#endif
_ATTR_SBCDEC_TEXT_
int sbc_open()
{
	int framelen;
	size_t readsize = IN_BUF_SIZE;
	size_t len = 0;

	g_sbcBufIndex = 0;
	g_unpacked_data_size = 0;

    readsize = RKFIO_FRead(gInbuf, readsize, pRawFileCache);
	//readsize = fread(gInbuf, 1, readsize, pRawFileCache);

 //   DisplayTestString(0,32, "EEE");
 //   DisplayTestDecNum(40,32, readsize);
	if(readsize < 0) {
		return 0;
	}

	sbc_init(&gSbc, 0L);
	gSbc.endian = SBC_LE;

	//Decodes ONE input block into ONE output block
	framelen = sbc_decode(&gSbc, gInbuf, readsize,
		&gSbcOutputPtr[g_sbcBufIndex][BUF_OFF_SET], OUT_BUF_SIZE, &len);

    //DisplayTestString(0,48, "EEE");
    //DisplayTestDecNum(40,48, framelen);
	if (framelen < 0) {
		return 0;
	}

	g_frameLength = framelen;
	g_outLength = len;
	g_timePos += g_outLength;

	g_channels = gSbc.mode == SBC_MODE_MONO ? 1 : 2;
	switch (gSbc.frequency) {
	case SBC_FREQ_16000:
		g_frequency = 16000;
		break;

	case SBC_FREQ_32000:
		g_frequency = 32000;
		break;

	case SBC_FREQ_44100:
		g_frequency = 44100;
		break;

	case SBC_FREQ_48000:
		g_frequency = 48000;
		break;
	default:
		g_frequency = 0;
	}

    #ifdef SSRC
    if(g_frequency == 44100)
    {
	    pSRCpcb = resample_init(g_channels,g_frequency,44120);


    }
    else
    {
        resample_init(g_channels,g_frequency,44120);
    }
    sbc_resample_enable();
    #endif
	/*The input data is not decoded, left to the next decoding*/
	if (readsize - framelen > 0) {
		g_unpacked_data_size = readsize - framelen;
		memmove(gInbuf, gInbuf + framelen, g_unpacked_data_size);

	}

	g_sbcBufIndex ^= 1;

	g_bitrate = (int)sbc_get_bit_rate(&gSbc);
	if (g_bitrate > 0) {
		g_timeLength = (long long)RKFIO_FLength(pRawFileCache) * 8/g_bitrate;
		//g_timeLength = g_fileSize * 8/g_bitrate;
	} else {
		g_timeLength = 60000;
	}

//	g_fileSize -= readsize;


	return 1;
}

_ATTR_SBCDEC_TEXT_
int sbc_get_buffer(unsigned long ulParam1 , unsigned long ulParam2)
{
	*(int *)ulParam1 = (unsigned int)&gSbcOutputPtr[g_sbcBufIndex^1][BUF_OFF_SET];
	*(int *)ulParam2 = g_outLength / 4;
    
	return 1;
}


_ATTR_SBCDEC_TEXT_
int sbc_dec()
{
	size_t readsize = IN_BUF_SIZE;
	size_t i, len = 0;
	int pos, framelen, count = 0;   
	int in_len = OUT_BUF_SIZE/2;
	int out_len = OUT_BUF_SIZE*2+512;
	#ifdef SSRC
	if(resample_pos >= SRC_OUT_BUF_SIZE)
	{
		count = SRC_OUT_BUF_SIZE;
		out_len = 0;
		goto sbcoutput;
		
	}
	#endif
read_data:  
	pos = 0;
	readsize = IN_BUF_SIZE;
	if (g_unpacked_data_size > 0) {
		/*copy the last unpacked data*/
        readsize = RKFIO_FRead(gInbuf + g_unpacked_data_size, 
                                    readsize - g_unpacked_data_size, pRawFileCache);
        if(readsize <= 0) {
		return 0;
    	}
		readsize += g_unpacked_data_size;
		g_unpacked_data_size = 0;

	} else {
        readsize = RKFIO_FRead(gInbuf, readsize, pRawFileCache);

        if(readsize <= 0) {
		return 0;
	}
	}



	do {
		framelen = sbc_decode(&gSbc, gInbuf + pos, readsize - pos,
			&gSbcOutputPtr[g_sbcBufIndex][BUF_OFF_SET] + count, OUT_BUF_SIZE - count, 
			&len);

		if (framelen > 0) {
			pos += framelen;
			count += len;
			
		} 
        else 
        {
			if (framelen == -1)          //Data length is not enough to a frame.
            {   
				//if (g_frameLength > (readsize - pos))
                {
					if (readsize - pos > 0) {
						g_unpacked_data_size = readsize - pos;
						memmove(gInbuf, gInbuf + pos, g_unpacked_data_size);
					}
					goto read_data;
				} 
			} else if (framelen == -2) {    //Can't find the frame head.
				for (i = pos; i < readsize; i++) {
					if (gInbuf[i] == 0x9c) {
						g_unpacked_data_size = readsize - i;
						memmove(gInbuf, gInbuf + i, g_unpacked_data_size);
						goto read_data;
					}
				}

				g_unpacked_data_size = 0;
				goto read_data;

			} else {
				for (i = pos + 1; i < readsize; i++) {
					if (gInbuf[i] == 0x9c) {
						g_unpacked_data_size = readsize - i;
						memmove(gInbuf, gInbuf + i, g_unpacked_data_size);
						goto read_data;
					}
				}

				g_unpacked_data_size = 0;
				goto read_data;
			}
		}

		/*The inbuf data decoded, read next data*/
		if (OUT_BUF_SIZE - count > 0) {
			if (readsize - pos > 0) {
				g_unpacked_data_size = readsize - pos;
				memmove(gInbuf, gInbuf + pos, g_unpacked_data_size);
			}
			goto read_data;
		} else {
			break;
		}
	}while (1);

	/*The input data is not decoded, left to the next decoding*/
	if (readsize - pos > 0) {
		g_unpacked_data_size = readsize - pos;
		memmove(gInbuf, gInbuf + pos, g_unpacked_data_size);
	}
#ifdef SSRC //for resample
    if(isResampleEnable)
    {
        if(g_frequency == 48000)
        {
            resampler_process(NULL,(short*)&gSbcOutputPtr[g_sbcBufIndex][BUF_OFF_SET],&in_len,(short*)&out_put[resample_pos],&out_len);
        }
        else
        {

            //resampler_process(NULL,(short*)&gSbcOutputPtr[g_sbcBufIndex][BUF_OFF_SET],&in_len,(short*)&out_put_temp[BUF_OFF_SET],&out_len);
            
            resampler_process(NULL,(short*)&gSbcOutputPtr[g_sbcBufIndex][BUF_OFF_SET],&in_len,(short*)&gSbcOutputPtr[g_sbcBufIndex][BUF_RESERVED_HEAD+BUF_RESERVED_LAST_DATA],&out_len);
            resampler_process(pSRCpcb,(short*)&gSbcOutputPtr[g_sbcBufIndex][BUF_RESERVED_HEAD+BUF_RESERVED_LAST_DATA],&out_len,(short*)&out_put[resample_pos],&out_len);

        }
    }
    else
    {
        memcpy(&out_put[resample_pos], &gSbcOutputPtr[g_sbcBufIndex][BUF_OFF_SET], OUT_BUF_SIZE);
        out_len = OUT_BUF_SIZE/2;
    }
sbcoutput:

	//memcpy(&gSbcOutputPtr[g_sbcBufIndex][BUF_OFF_SET], &out_put[0], OUT_BUF_SIZE);

	//resample_pos = resample_pos+out_len*2-OUT_BUF_SIZE;
	memcpy(&gSbcOutputPtr[g_sbcBufIndex][BUF_OFF_SET], &out_put[0], SRC_OUT_BUF_SIZE);

	resample_pos = resample_pos+out_len*2-SRC_OUT_BUF_SIZE;
	if(resample_pos > 0)
	memmove(&out_put[0], &out_put[SRC_OUT_BUF_SIZE], resample_pos);

    count = SRC_OUT_BUF_SIZE;


#endif
	g_outLength = count;
	g_sbcBufIndex ^= 1;
	g_timePos += g_outLength;

	return 1;
}
_ATTR_SBCDEC_TEXT_
int sbc_get_samplerate()
{
    #ifdef SSRC
	return 44100;
	#else
    return g_frequency;
    #endif
}

_ATTR_SBCDEC_TEXT_
int sbc_get_channels()
{
	return g_channels;
}

_ATTR_SBCDEC_TEXT_
int sbc_get_bitrate()
{
	return g_bitrate;
}

_ATTR_SBCDEC_TEXT_
int sbc_get_length()
{
	return g_timeLength;
}

_ATTR_SBCDEC_TEXT_
int sbc_get_timepos()
{
	return g_timePos / 4;
}

_ATTR_SBCDEC_TEXT_
int sbc_seek(long time)
{
	long pos;
	if (time > g_timeLength) {
		time = g_timeLength;
	}

	pos = ((long long) time * g_bitrate) / 8000;
	RKFIO_FSeek(pos , 0 ,  pRawFileCache);
	//fseek(pRawFileCache, pos , 0);

	g_timePos = (long long)time * gSbc.frequency / 1000;

	memset(&gSbcOutputPtr[0][BUF_OFF_SET], 0, OUT_BUF_SIZE);
	memset(&gSbcOutputPtr[1][BUF_OFF_SET], 0, OUT_BUF_SIZE);
	g_sbcBufIndex = g_sbcBufIndex^1;

	return 1;
}


_ATTR_SBCDEC_TEXT_
int sbc_close()
{
	sbc_finish(&gSbc);
    #ifdef SSRC
    sbc_resample_disable();
    #endif
	return 1;
}

#if 0
int main(int argc, char* argv[]){
	int ren, count;
	struct stat st;
	FILE *stream;

	//pRawFileCache = fopen("a2dp_media_data_from.sbc", "rb");
	pRawFileCache = fopen("input.txt", "rb");
	if (pRawFileCache == NULL) {
		printf("Cannot open input file.\n");
		return 0;
	} 

	//stream = fopen("a2dp_media_data_from.pcm", "wb");
	stream = fopen("input.pcm", "wb");
	if (stream == NULL) {
		printf("Cannot open output file.\n");
		return 0;
	} 

	//if (stat("a2dp_media_data_from.sbc", &st) < 0) {
	if (stat("input.txt", &st) < 0) {
		printf("Can't get size of file.\n");
		return 0;
	}

	g_fileSize = st.st_size;

	ren = sbc_open();
	if (ren > 0) {
		count = fwrite(&gSbcOutputPtr[g_sbcBufIndex^1][0], 1, g_outLength, stream);
		if (count != g_outLength)
			printf("Write data error.\n"); 
	}

	while (g_fileSize > 0) {
		ren = sbc_dec();
		if (ren > 0) {
			count = fwrite(&gSbcOutputPtr[g_sbcBufIndex^1][0], 1, g_outLength, stream);
			if (count != g_outLength)
				printf("Write data error.\n");
		}
	}

	fclose(pRawFileCache);
	fclose(stream);
	return 1;
}

#endif


#endif //SBC_INCLUDE