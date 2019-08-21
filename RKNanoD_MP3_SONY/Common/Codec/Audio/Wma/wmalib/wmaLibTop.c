#include "..\wmaInclude\predefine.h"
#include "../include/audio_main.h"
//#include <stdio.h>
//#include <string.h>
//#include <time.h>
#include "..\wmaInclude\wmaudio.h"
#include "..\wmaInclude\AutoProfile.h"
#include "..\wmaInclude\wmaudio_type.h"
//#include "..\wmaInclude\WMAGLOBALVARDef.h"
#include "..\..\Include\audio_file_access.h"
#include "..\wmaInclude\pcmfmt.h"

__attribute__((section("WmaCommonCode")))
tWMAFileStateInternal gStateInt;
__attribute__((section("WmaCommonCode")))
unsigned char g_pBuffer[WMA_MAX_DATA_REQUESTED*2];//bitstream input buffer

#ifdef A_CORE_DECODE
#ifdef WMA_DEC_INCLUDE
#pragma arm section zidata = "WmaDecoderBss",rwdata = "WmaDecoderData",rodata = "WmaCommonCode"

#if 1
#define _BOOL_DEFINED

#if defined(AUTOPC_STYLE)
#   pragma message (__FILE__ "(15) : Warning - Not 24-bit per sample ready!")
#endif

// comment out DUMP_WAV define below to output a raw .pcm file rather than a .wav file.
#define DUMP_WAV
#ifdef DUMP_WAV
//#include "..\wmaInclude\wavfileexio.h"
#endif  /* DUMP_WAV */

//Void PCMFormat2WaveFormatEx(PCMFormat* pFormat, WAVEFORMATEX* wfx);
//Void PCMFormat2WaveFormatExtensible(PCMFormat* pFormat, WAVEFORMATEXTENSIBLE* wfx);

// WMA2CMP - define in project settings to produce a .cmp output file from the .wma suitable for use with decapp



// HEAP_DEBUG_CHECK is the same as the defines in msaudio.h
// Although it is normally bad form to copy something out a .h instead of
// including the .h, in this test program, we want to make sure we do not
// inadvertently use anything from msaudio.h
#if defined(HEAP_DEBUG_TEST) && defined(_DEBUG)
void HeapDebugCheck();
#define HEAP_DEBUG_CHECK HeapDebugCheck()
#else
#define HEAP_DEBUG_CHECK
#endif


#if defined(PRINT_FROM_SAMPLE) && defined(PRINT_TO_SAMPLE)
// One of the DCT print defines must be defined in fft.c to get this
extern int bPrintDctAtFrame;    // used to print coefs before and after DCT
#endif

extern tWMAFileContDesc *g_pdesc;
/* global */

tWMAFileHdrState g_hdrstate;
tHWMAFileState g_state;
tWMAFileHeader g_hdr;
tWMAFileLicParams g_lic;

#define wma_test  1

#if wma_test
static void *g_fp = NULL;
#else
static FILE *g_fp = NULL;
#endif


const unsigned int MAX_BUFSIZE = WMA_MAX_DATA_REQUESTED;


tWMA_U32 g_cbBuffer = 0;
tWMA_U64 g_qwBufferOffset = 0;


//time_t g_ulStartFirstSec, g_ulEndSec;
//time_t g_ulFullSec = 0;
//time_t g_ulOutputSamples = 0;
unsigned int  g_SampleRate;

unsigned int g_iSeekIndex = 0;
#if 1//ndef WMAAPI_NO_DRM
extern const unsigned char g_pmid[16] ;
#else

#endif



#define MAX_SAMPLES 2048

#define TEST_INTERLEAVED_DATA


#define STRING_SIZE 2048

unsigned int g_FrameLength;
unsigned short g_flagHighRate;

static unsigned int g_DecodedSamples;
static unsigned int g_cCountGetPCM;

extern int wma_table_room_generate(void);
extern void wma_table_room_init(void);
extern unsigned long WMAFunction(unsigned long ulIoctl, unsigned long ulParam1,
                                     unsigned long ulParam2, unsigned long ulParam3);
#ifndef rk_nano_board
#ifdef WMA_TABLE_ROOM_VERIFY
#ifdef USE_SYS_FILE_ACCESS
FILE *g_fpTable;
extern unsigned long gTableBuf[];
#else
extern unsigned long const gTableBuf[];
#endif
#endif
#endif

#endif
//extern int file_pos;
//extern const unsigned long mp3data[];
//tWMAFileStateInternal gStateInt;
//unsigned char g_pBuffer[WMA_MAX_DATA_REQUESTED*2];//bitstream input buffer
#ifdef WMAINITIALIZE_HIGH_MID_ALL_USED
#pragma arm section code = "WmaCommonCode"
#if 0
static unsigned int remainSize = 0;
static unsigned char cachbuffer[WMA_MAX_DATA_REQUESTED];

unsigned int WMA_Read(void * buffer, unsigned int length ,void *fhandle)
{
#if 0
    return fread(buffer,1,length,fhandle);
#else
    unsigned int ReadSize;
    if(remainSize >= length)
    {
		memcpy(buffer,cachbuffer+WMA_MAX_DATA_REQUESTED-remainSize,length);
		remainSize -= length;
		return length;
    }
	else
	{
		unsigned int temp = length - remainSize;
		memcpy(buffer,cachbuffer + WMA_MAX_DATA_REQUESTED - remainSize,remainSize);
		(unsigned char*)buffer += remainSize;

		//remainSize = ReadSize = fread(cachbuffer,1,WMA_MAX_DATA_REQUESTED,fhandle);
		remainSize = ReadSize = RKFIO_FRead(cachbuffer, WMA_MAX_DATA_REQUESTED, fhandle);
		if(remainSize > temp)
		{
		  memcpy(buffer,cachbuffer,temp);
		  remainSize = ReadSize - temp;
		  return length;
		}
		else
		{
			return (length - temp);
		}

	}
#endif
}
int WMA_Seek(long int offset, int whence , void * stream)
{
#if 0
   return(fseek(stream, offset, whence));
#else
    switch(whence)
    {
		case SEEK_SET:
		{
			unsigned int ReadSize;
			//fseek(stream,(offset - (offset%512)),whence);
			int ret = RKFIO_FSeek((offset - (offset%512)), whence,stream);
			if(0 != ret)
				return -1;
			//ReadSize = fread(cachbuffer,1,WMA_MAX_DATA_REQUESTED,stream);
			ReadSize = RKFIO_FRead(cachbuffer,WMA_MAX_DATA_REQUESTED,stream);
			remainSize = ReadSize - (offset%512);
			break;
		}
		default:
			while(1);
			break;

    }
	return 0;
#endif
}
#endif
/* WMAFileCBGetData */
tWMA_U32 WMAFileCBGetData(tHWMAFileState state,tWMA_U64 offset,tWMA_U32 num_bytes,unsigned char **ppData)
{
    tWMA_U32 ret;

    I64 iRelativeOffset = (tWMA_I64)offset - (tWMA_I64)g_qwBufferOffset;
    U32 iBytesRead;

    // Initialize return values
    *ppData = NULL;
    ret = 0;
    assert(num_bytes <= MAX_BUFSIZE);

    // The ANSI-C function fread seems to do fine with files which are > 4GB,
    // but fseek fails miserably. Therefore, never call fseek unless requested
    // read range is within first 0x7FFFFFFF.

    // Check if start of requested range is within our buffer
    if (iRelativeOffset >= 0 && iRelativeOffset < g_cbBuffer)
    {
        if (iRelativeOffset + num_bytes < g_cbBuffer)
        {
            // Entire request is entirely within range of current buffer
            *ppData = g_pBuffer + iRelativeOffset;//g_pBuffer points to the beginning of wma file.&ppData is &g_pBuffer[irelativeoffset]
            ret = num_bytes;
            goto exit;
        }

        // Only the start of request range is in our buffer. Collapse memory.
        memmove(g_pBuffer, g_pBuffer + iRelativeOffset, (size_t)(g_cbBuffer - iRelativeOffset));
        g_cbBuffer -= (U32)iRelativeOffset;
        g_qwBufferOffset = offset;
    }
    else
    {
        // Start of requested range is outside of our buffer
        if (offset + num_bytes <= 0x7FFFFFFF)
        {
#ifdef USE_SYS_FILE_ACCESS
            if (0 != fseek(g_fp, (U32)offset, SEEK_SET))
                goto exit;
#else
		//

         int ret = RKFIO_FSeek(offset, SEEK_SET,pRawFileCache);
            if( 0 != ret)
                goto exit;

#endif

        }
        else
        {
#if 0
            // If we reach this point, we can't use fseek
            if (iRelativeOffset < 0)
            {
                // Seeking backwards to > 32-bit offset is not supported
                assert(0);
                goto exit;
            }

            // Seek forward to the desired offset.
            iRelativeOffset -= g_cbBuffer;
            while (iRelativeOffset > 0)
            {
                U32 iBytesToRead;

                if (iRelativeOffset > MAX_BUFSIZE)
                    iBytesToRead = MAX_BUFSIZE;
                else
                    iBytesToRead = (U32)iRelativeOffset;
#ifdef USE_SYS_FILE_ACCESS
                iBytesRead = fread(g_pBuffer, 1, iBytesToRead, g_fp);
#else
                iBytesRead = RKFIO_FRead(g_pBuffer, iBytesToRead, g_fp);
#endif
                if (iBytesRead < iBytesToRead)
                {
                    // Reached EOF before reaching requested offset!
                    goto exit;
                }

                iRelativeOffset -= iBytesRead;
            }
#else
            while(1); // 当文件长度大于0x7FFFFFFF时，不支持
#endif
        }

        g_qwBufferOffset = offset;
        g_cbBuffer = 0;
    }

    /* read data(length=MAX_BUFSIZE - g_cbBuffer) from filetype pointer g_ft*/
#ifdef USE_SYS_FILE_ACCESS
    iBytesRead = fread(g_pBuffer + g_cbBuffer, 1, MAX_BUFSIZE - g_cbBuffer, g_fp);
#else
  iBytesRead = RKFIO_FRead(g_pBuffer + g_cbBuffer, MAX_BUFSIZE - g_cbBuffer, g_fp);
 //  iBytesRead = readwmadata(g_pBuffer + g_cbBuffer, MAX_BUFSIZE - g_cbBuffer, g_fp);
#endif
    g_cbBuffer += iBytesRead;
    assert(g_cbBuffer <= MAX_BUFSIZE);
    assert(offset == g_qwBufferOffset);
    *ppData = g_pBuffer;     //g_pBuffer pointer points to wmafile

    ret = num_bytes;


exit:

//    fprintf(stderr, "++ WMAFileCBGetData: %lu bytes from %lu.\n", ret, offset);

    return ret;
}
#pragma arm section code
#endif
#ifdef WMAINITIALIZE
#pragma arm section code = "WmaOpenCodecCode"
tWMAFileStatus wma_decoder_init(WMA_DEC_INFO* dec_info)
{
    // tWMA_U32 msSeekTo;

    tWMAFileStatus rc;
    tWMA_U16 wMBRTotalStreams = 0;
    U16  nMBRTargetStream = 1;
    PCMFormat pcmFormat;
//    Bool bDirMode = 0;
//    Bool bForceExt = WMAB_FALSE;
//    Bool bOutputCMP = FALSE;
    Bool bDropPacket = FALSE;
    U16  nDRCSetting = 0;
    U16  nDecoderFlags = 0;
    U32  nDstChannelMask = 0;
//    Int  iError=0;
    U32  nInterpResampRate = 0;
//    Int  iFileCnt = 0;
    //Bool bLimitPTSErrors = WMAB_TRUE;
    //U16  cMaxPTSErrors = 5;

//    int cSeek = 0;
    //float fltTimeExpected = 0;
    //const char *strLic = "c:\\ti_test\\files\\drmv1pm.new";
    //tWMAFileContDesc  *pdesc = NULL;
    tWMAExtendedContentDesc *pECDesc;
    //int iRV = 1;    // assume error exit return value
    //float fltDiffMax = 0.0F;
    //FILE *pfTime = tmpfile();
    //tWMA_I64 iSampleTotal = 0;
#ifdef PUT_ONE_OUTPUT_BUFFER_IN_IRAM
    gWmaOutputPtr[0] = gWmaOutputLeft;
    gWmaOutputPtr[1] = gWmaOutputRight;
#endif
#ifdef WMA_TABLE_ROOM_VERIFY
#if defined(USE_SYS_FILE_ACCESS)&& !defined(rk_nano_board)
    int cbTable;

    g_fpTable = fopen("wma_table_room.bin", "rb");
    if (g_fpTable == NULL)
    {
        DEBUG("Can't open wma_table_room.bin!\n");
        return cWMA_Failed;
    }

    fseek(g_fpTable, 0, 2);
    cbTable = ftell(g_fpTable);
    fseek(g_fpTable, 0, 0);

    if (cbTable >= TABLE_BUF_SIZE*sizeof(long))
    {
        DEBUG("Table too big!\n");
        return cWMA_Failed;
    }


    fclose(g_fpTable);
#endif
    wma_table_room_init();
#ifndef WMAAPI_NO_DRM
    drm9_table_room_init();
#endif
#endif
    //U16  cPTSErrors = 0;
    //FILE *outpcmfile;//hxd
    //if ((outpcmfile=fopen(pWAVFileName,"wb")) == NULL) /* open pcm file */
    //{
    // fprintf(stderr, "Cannot open output file.\n");
    // return 1;
    //}
    //outpcmfile = g_fpPCM;
    // reset global values for each file call.
//    g_ulFullSec = 0;
    g_DecodedSamples = 0;
    g_FrameLength = 2048;
    g_cCountGetPCM = 0;

    g_fp = (FILE*)dec_info->fhandle;//fopen (pWMAFileName, "rb");
    //if (g_fp == NULL) {
    //_ftprintf(stderr, TEXT("** Cannot open %s.\n"), pWMAFileName);
    //   goto lexit;
    //}

    /* init struct */
    memset((void *)&g_hdrstate, 0, sizeof(g_hdrstate));
    memset((void *)&g_state, 0, sizeof(g_state));
    memset((void *)&g_hdr, 0, sizeof(g_hdr));

    /* init the decoder and allocate mem */
    rc = WMAFileDecodeCreate(&g_state); //renew element of wmafilestateinternal to 0
    if (rc != cWMA_NoErr)
    {
       // fprintf(stderr, "** Cannot create the WMA decoder.\n");
        goto lexit;
    }

    /* test the checking API */
    //rc = WMAFileSetTargetMBRAudioStream(&g_hdrstate, nMBRTargetStream);
    ((tWMAFileHdrStateInternal *)(&g_hdrstate))->wTargetAudioStreamNumber = nMBRTargetStream;

    /* judge if file is wmafile hxd*/
    //rc = WMAFileIsWMA (&g_hdrstate);
	//    rc = WMA_ParseAsfHeader((tWMAFileHdrStateInternal *)(&g_hdrstate), 0);
	  rc = cWMA_NoErr;
    if (rc != cWMA_NoErr)
    {
//        fprintf(stderr, "** The file is not a WMA file.\n");
        goto lexit;
    }

    // Check for MBR(Multiple bitrate corresponds to VBR)
    wMBRTotalStreams = ((tWMAFileHdrStateInternal *)(&g_hdrstate))->cAudioStreams;

    rc = WMAFileDecodeInitEx(g_state, nDecoderFlags, nDRCSetting, bDropPacket,
                             nDstChannelMask, nInterpResampRate, &pcmFormat,
                             nMBRTargetStream);
    if (rc != cWMA_NoErr)
    {
//        fprintf(stderr, "** Cannot initialize the WMA decoder.\n");
        goto lexit;
    }

    /* in the following we truly begin to decode wma file after checking asf header hxd*/
    /* get header information */

    rc = WMAFileDecodeInfo(g_state, &g_hdr);
    if (rc != cWMA_NoErr)
    {
//        fprintf(stderr, "** Failed to retrieve information.\n");
        goto lexit;
    }

    /* get content description */

    rc = WMAFileContentDesc(g_state, &g_pdesc);
    if (rc != cWMA_NoErr)
    {
//        fprintf(stderr, "** Failed to retrieve content description.\n");
        goto lexit;
    }

    /* display information */


    rc = WMAFileExtendedContentDesc(g_state, &pECDesc);

    if (rc != cWMA_NoErr)
    {
//        fprintf(stderr, "** Failed to retrieve extended content description.\n");
        goto lexit;
    }
    if(g_hdr.has_DRM)
    {
#ifdef WMAAPI_NO_DRM
		rc = cWMA_DRMUnsupported;
		goto lexit;
#else
        tWMADateParams currentDate;

//    	SYSTEMTIME stDate;
        if(0 == g_flagHighRate)
        {
			rc = cWMA_DRMUnsupported;
			goto lexit;
        }

        g_lic.pPMID = (unsigned char *)&g_pmid;
        g_lic.cbPMID = sizeof(g_pmid);

//    	GetSystemTime(&stDate);
//	    SysDateToLicDate(&stDate, &currentDate);

		memset((BYTE*)&currentDate, 0, sizeof(tWMADateParams));

        rc = WMAFileLicenseInit (g_state, &g_lic, WMA_SDMI_LIC, currentDate);
        if(rc != cWMA_NoErr)
        {
            //fprintf(stderr, "** WMALicenseInit failed (%u).\n", rc);
            goto lexit;
        }
#endif
    }

#ifndef rk_nano_board
    DEBUG("++ WMA bitstream version: %d\n", g_hdr.version);
    DEBUG("++         sampling rate: %ld Hz\n", g_hdr.sample_rate);
    g_SampleRate = g_hdr.sample_rate;
    DEBUG("++         # of channels: %d\n", g_hdr.num_channels);
    DEBUG("++              bit-rate: %ld bps\n", g_hdr.bitrate);
    DEBUG("++              duration: %ld ms\n", g_hdr.duration);
    DEBUG("++           DRM content: %s\n", g_hdr.has_DRM ? "Yes" : "No");
#endif

#ifdef WMA_TABLE_ROOM_GENERATE

    if (wma_table_room_generate())
        return cWMA_Failed;

#endif
    if (g_hdr.sample_rate >= 32000)
        g_FrameLength = 2048;
    else if (g_hdr.sample_rate >= 22050)
        g_FrameLength = 1024;
    else
        g_FrameLength = 512;


lexit:
	dec_info->frameLen = g_FrameLength;
	dec_info->hdr = &g_hdr;
	dec_info->pdesc = g_pdesc;
	dec_info->isHighRate = g_flagHighRate;
    return rc;
}
#pragma arm section code
#endif
#define GPIO_PB_REG_VALUE  *((volatile unsigned long*)(0x40000000 + 0x000C))
#define GPIO_PB_REG_DIRECT  *((volatile unsigned long*)(0x40000000 + 0x0010))
#if defined(WMAHIGHRATE) || defined(WMAMIDRATELOWRATE)
#pragma arm section code = "WmaHighLowCommonCode"
tWMAFileStatus wma_decoder_dec(WMA_DEC_INFO* dec_info)
{
    tWMAFileStatus rc;
    tWMA_U32 num_samples;
    tWMA_U32 cFetchedSamples = 0;
    tWMA_U32 nDecodedSamples;

    do
    {
        rc = WMAFileDecodeData(g_state, &nDecodedSamples);//enter packet decode

        if (rc != cWMA_NoErr)
        {
            //g_ulEndSec = time(NULL);
            //wchen: I don't understand why failed is normal
            if (rc == cWMA_NoMoreFrames || rc == cWMA_Failed)
            {
                return cWMA_NoMoreFrames;
            }
            else
            {
                return cWMA_Failed;
            }
        }


        //do
        {

//            short *pL = gWmaOutputPtr[0];
            //short *pR = g_pRight;

            tWMA_I64 tPresTime = 0;
            float    fltDiff = 0;


            //HEAP_DEBUG_CHECK;
            g_cCountGetPCM++;


            //assert(cFetchedSamples <= nDecodedSamples);
            /* num_samples show that a channel has pcm sample's number.if stereo present,pcm sample's numbers are 2*num_samples*/
            //num_samples = WMAFileGetPCM(g_state, gWmaOutputPtr + g_hdr.num_channels * cFetchedSamples, sizeof(gWmaOutputPtr),
            //g_FrameLength - cFetchedSamples, &tPresTime);
            //GPIO_PB_REG_VALUE = 0xff;
            //num_samples = WMAFileGetPCM(g_state, gWmaOutputPtr[g_wmaBufIndex] + 2 * cFetchedSamples, MAX_SAMPLES_OF_ONE_CHANNEL*2,
            //                            g_FrameLength - cFetchedSamples, &tPresTime);
			num_samples = WMAFileGetPCM(g_state, (short*)dec_info->curPtr + 2 * cFetchedSamples, MAX_SAMPLES_OF_ONE_CHANNEL*2,
                                        g_FrameLength - cFetchedSamples, &tPresTime);

            //GPIO_PB_REG_VALUE = 0;
            //DEBUG("num_samples=%d\n",num_samples);//hxd
            cFetchedSamples += num_samples;
            if (cFetchedSamples < g_FrameLength)
            {
                // 未解码出足够一帧的数据，继续解码...
                continue;
            }

            g_DecodedSamples += g_FrameLength;
            //nDecodedSamples -= num_samples;//余下多少已解码的pcm据未送出
            //cFetchedSamples = 0;
        } //while(cFetchedSamples <  nDecodedSamples);//do line 1653
        dec_info->decoded_samples = g_DecodedSamples;
        // 已经解出足够一帧的数据，返回
        return cWMA_NoErr;

    }
    while (1);
}
#pragma arm section code
#endif
/*************************************************************************/
#if defined(WMAHIGHRATE) || defined(WMAMIDRATELOWRATE)
#pragma arm section code = "WmaHighLowCommonCode"
tWMAFileStatus wma_decoder_seek(WMA_DEC_INFO* dec_info)
{
    tWMAFileStatus rc;
    unsigned long msActualSeekTo;

    rc = WMAFileSeek(g_state, dec_info->msSeekTo, &msActualSeekTo);
    if (rc != cWMA_NoErr)
        return cWMA_Failed;

    // 更新已解码的样点数
    dec_info->decoded_samples = g_DecodedSamples = ((msActualSeekTo / 1000) * g_hdr.sample_rate) +
                       ((msActualSeekTo % 1000) * g_hdr.sample_rate  / 1000);

    return cWMA_NoErr;
}
#pragma arm section code
#endif
#endif
#endif