/* ***** BEGIN LICENSE BLOCK *****
 * Source last modified: $Id: aacdec.c,v 1.1 2005/02/26 01:47:31 jrecker Exp $
 *
 * Portions Copyright (c) 1995-2005 RealNetworks, Inc. All Rights Reserved.
 *
 * The contents of this file, and the files included with this file,
 * are subject to the current version of the RealNetworks Public
 * Source License (the "RPSL") available at
 * http://www.helixcommunity.org/content/rpsl unless you have licensed
 * the file under the current version of the RealNetworks Community
 * Source License (the "RCSL") available at
 * http://www.helixcommunity.org/content/rcsl, in which case the RCSL
 * will apply. You may also obtain the license terms directly from
 * RealNetworks.  You may not use this file except in compliance with
 * the RPSL or, if you have a valid RCSL with RealNetworks applicable
 * to this file, the RCSL.  Please see the applicable RPSL or RCSL for
 * the rights, obligations and limitations governing use of the
 * contents of the file.
 *
 * This file is part of the Helix DNA Technology. RealNetworks is the
 * developer of the Original Code and owns the copyrights in the
 * portions it created.
 *
 * This file, and the files included with this file, is distributed
 * and made available on an 'AS IS' basis, WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, AND REALNETWORKS HEREBY DISCLAIMS
 * ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET
 * ENJOYMENT OR NON-INFRINGEMENT.
 *
 * Technology Compatibility Kit Test Suite(s) Location:
 *    http://www.helixcommunity.org/content/tck
 *
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */

/**************************************************************************************
 * Fixed-point HE-AAC decoder
 * Jon Recker (jrecker@real.com), Ken Cooke (kenc@real.com)
 * February 2005
 *
 * aacdec.c - platform-independent top level decoder API
 **************************************************************************************/

#include "aaccommon.h"
#include "coder.h"
//#define HELUN
#include "../../include/audio_main.h"
#include <stdio.h>

#include "../include/audio_globals.h"
#include "../include/audio_file_access.h"

#ifdef A_CORE_DECODE
#ifdef AAC_DEC_INCLUDE
//#define TEST

#ifdef TEST
extern int xp, yp;
extern  int num;

#define testhl \
        do{ \
            DisplayTestDecNum(xp,yp,(unsigned char)num);\
            yp+=10;\
            if(yp>=120) \
                {xp+=40;yp=10;}\
            num++;\
            }while(0)
#define showhl(x) \
        do{ \
            DisplayTestDecNum(xp,yp,(unsigned char )x);\
            yp+=10;\
            if(yp>=120) \
                {\
                yp=0;\
                xp+=40;\
                if(xp>=120)\
                    {\
                    xp=0;\
                    }\
                }\
                }while(0)

#else
#define testhl
#define showhl

#endif





#pragma arm section code ="AacROCode", rodata = "AacROData", rwdata = "AacROData", zidata = "AacROData"
//add by helun
int *twidTabOdd32;
int *cos4sin4tab64;
int *cos1sin1tab64;
AAC_TIMEINFO timeinfo;

#include "aac_gen_table/aac_table.h"
#ifdef aac_table_test
static const int *_cos1sin1tab64_aacdec = (int *)( base + _cos1sin1tab64_aacdec_offset);
static const int *_cos4sin4tab64_aacdec = (int *)( base + _cos4sin4tab64_aacdec_offset);
static const int *_twidTabOdd32_aacdec = (int *)( base + _twidTabOdd32_aacdec_offset);
#else
static const int _cos1sin1tab64[34] =
{
    0x40000000, 0x00000000, 0x43103085, 0x0323ecbe, 0x45f704f7, 0x0645e9af, 0x48b2b335, 0x09640837,
    0x4b418bbe, 0x0c7c5c1e, 0x4da1fab5, 0x0f8cfcbe, 0x4fd288dc, 0x1294062f, 0x51d1dc80, 0x158f9a76,
    0x539eba45, 0x187de2a7, 0x553805f2, 0x1b5d100a, 0x569cc31b, 0x1e2b5d38, 0x57cc15bc, 0x20e70f32,
    0x58c542c5, 0x238e7673, 0x5987b08a, 0x261feffa, 0x5a12e720, 0x2899e64a, 0x5a6690ae, 0x2afad269,
    0x5a82799a, 0x2d413ccd,
};

static const int _cos4sin4tab64_aacdec[64] =
{
    0x40c7d2bd, 0x00c90e90, 0x424ff28f, 0x3ff4e5e0, 0x43cdd89a, 0x03ecadcf, 0x454149fc, 0x3fc395f9,
    0x46aa0d6d, 0x070de172, 0x4807eb4b, 0x3f6af2e3, 0x495aada2, 0x0a2abb59, 0x4aa22036, 0x3eeb3347,
    0x4bde1089, 0x0d415013, 0x4d0e4de2, 0x3e44a5ef, 0x4e32a956, 0x104fb80e, 0x4f4af5d1, 0x3d77b192,
    0x50570819, 0x135410c3, 0x5156b6d9, 0x3c84d496, 0x5249daa2, 0x164c7ddd, 0x53304df6, 0x3b6ca4c4,
    0x5409ed4b, 0x19372a64, 0x54d69714, 0x3a2fcee8, 0x55962bc0, 0x1c1249d8, 0x56488dc5, 0x38cf1669,
    0x56eda1a0, 0x1edc1953, 0x57854ddd, 0x374b54ce, 0x580f7b19, 0x2192e09b, 0x588c1404, 0x35a5793c,
    0x58fb0568, 0x2434f332, 0x595c3e2a, 0x33de87de, 0x59afaf4c, 0x26c0b162, 0x59f54bee, 0x31f79948,
    0x5a2d0957, 0x29348937, 0x5a56deec, 0x2ff1d9c7, 0x5a72c63b, 0x2b8ef77d, 0x5a80baf6, 0x2dce88aa,
};

static const int _twidTabOdd32_aacdec[8 * 6] =
{
    0x40000000, 0x00000000, 0x40000000, 0x00000000, 0x40000000, 0x00000000, 0x539eba45, 0xe7821d59,
    0x4b418bbe, 0xf383a3e2, 0x58c542c5, 0xdc71898d, 0x5a82799a, 0xd2bec333, 0x539eba45, 0xe7821d59,
    0x539eba45, 0xc4df2862, 0x539eba45, 0xc4df2862, 0x58c542c5, 0xdc71898d, 0x3248d382, 0xc13ad060,
    0x40000000, 0xc0000000, 0x5a82799a, 0xd2bec333, 0x00000000, 0xd2bec333, 0x22a2f4f8, 0xc4df2862,
    0x58c542c5, 0xcac933ae, 0xcdb72c7e, 0xf383a3e2, 0x00000000, 0xd2bec333, 0x539eba45, 0xc4df2862,
    0xac6145bb, 0x187de2a7, 0xdd5d0b08, 0xe7821d59, 0x4b418bbe, 0xc13ad060, 0xa73abd3b, 0x3536cc52,
};
#endif

#pragma arm section code

#pragma arm section code = "AacDecCode", rodata = "AacDecCode", rwdata = "AacDecData", zidata = "AacDecBss"


int offset_by_helun = 0;


static  char aacversion[] = "Version:0.0.1 \nDate:2012.3.28 \nLib:aac_dec_lib" ;

char * AacDecVersion()
{
    return aacversion;
}



int IS_SBR_ENABLE = 0;
static int IS_SBR_NORMAL = 0;

int UnpackADTSHeaderForTime(int  *curpos, unsigned char **buf, int *bitOffset, int *bitsAvail)
{
    int bitsUsed;
    BitStreamInfo bsi;
    ADTSHeader tADTSHeader;
    ADTSHeader *fhADTS = &tADTSHeader;
    timeinfo.frames++;
    /* init bitstream reader */
    SetBitstreamPointer(&bsi, (*bitsAvail + 7) >> 3, *buf);
    GetBits(&bsi, *bitOffset);

    /* verify that first 12 bits of header are syncword */
    if (GetBits(&bsi, 12) != 0x0fff)
        return ERR_AAC_INVALID_ADTS_HEADER;

    /* fixed fields - should not change from frame to frame */
    fhADTS->id =               GetBits(&bsi, 1);
    fhADTS->layer =            GetBits(&bsi, 2);
    fhADTS->protectBit =       GetBits(&bsi, 1);
    fhADTS->profile =          GetBits(&bsi, 2);
    fhADTS->sampRateIdx =      GetBits(&bsi, 4);
    fhADTS->privateBit =       GetBits(&bsi, 1);
    fhADTS->channelConfig =    GetBits(&bsi, 3);
    fhADTS->origCopy =         GetBits(&bsi, 1);
    fhADTS->home =             GetBits(&bsi, 1);
    /* variable fields - can change from frame to frame */
    fhADTS->copyBit =          GetBits(&bsi, 1);
    fhADTS->copyStart =        GetBits(&bsi, 1);
    fhADTS->frameLength =      GetBits(&bsi, 13);
    *curpos = *curpos + fhADTS->frameLength;    //updata curpos for next ADTS head parse
    //aac_printf("frame=%d,len=%d\n",timeinfo.frames,fhADTS->frameLength);
    timeinfo.framelen += fhADTS->frameLength;
    fhADTS->bufferFull =       GetBits(&bsi, 11);
    fhADTS->numRawDataBlocks = GetBits(&bsi, 2) + 1;

    /* note - MPEG4 spec, correction 1 changes how CRC is handled when protectBit == 0 and numRawDataBlocks > 1 */
    if (fhADTS->protectBit == 0)
        fhADTS->crcCheckWord = GetBits(&bsi, 16);

    /* byte align */
    ByteAlignBitstream(&bsi);   /* should always be aligned anyway */

    /* check validity of header */
    if (fhADTS->layer != 0 || fhADTS->profile != AAC_PROFILE_LC ||
        fhADTS->sampRateIdx >= NUM_SAMPLE_RATES || fhADTS->channelConfig >= NUM_DEF_CHAN_MAPS)
        return ERR_AAC_INVALID_ADTS_HEADER;

#ifndef AAC_ENABLE_MPEG4

    if (fhADTS->id != 1)
        return ERR_AAC_MPEG4_UNSUPPORTED;

#endif
    /* update codec info */
    timeinfo.channels = channelMapTab[fhADTS->channelConfig];

    if (timeinfo.channels == 1)
    {
        // return ERR_AAC_MPEG4_UNSUPPORTED;
    }

    /*
    showhl(fhADTS->channelConfig);
    showhl(channelMapTab[fhADTS->channelConfig]);
    showhl(fhADTS->sampRateIdx);
    showhl(sampRateTab[fhADTS->sampRateIdx]);
    DelayMs(10000);
    */
//  if(timeinfo.channels>2)
//  {
//        aac_printf("\n ### channels = %d  , > 2###\n",timeinfo.channels);
//      timeinfo.channels=2;
//  }
    timeinfo.rate = sampRateTab[fhADTS->sampRateIdx];
    timeinfo.blocks = 1024;
    return ERR_AAC_NONE;
}

#define NUM 32

int  InitTimeForADTS(FILE *fp, int filelen, int offsethelun)
{
    AAC_TIMEINFO *pt = &timeinfo;
    unsigned char temp[64];
    int  ret, flag = 0, offset;
    unsigned char *p;
    int bitsAvail;
    int bitOffset = 0;
    int curpos = 0;
    int t;
    int count = 0;
#define NUMTEST 20
    aac_MemSet(pt, 0, sizeof(AAC_TIMEINFO));
    //FileSeek(0,0 ,fp);
    offset_by_helun = curpos = offsethelun;
    ret = RKFIO_FRead(temp, 64, fp);
    count++;
    //if(ret!=64)
    //  {
    //      flag=1;
    //  }
#ifdef NUMTEST

    if (count == NUMTEST)
    {
        timeinfo.totaltime = (((double)(timeinfo.blocks * timeinfo.frames)) / timeinfo.rate) * 1000;
        timeinfo.bitrate = timeinfo.framelen * 8 / (timeinfo.totaltime);
        timeinfo.framelen = timeinfo.framelen / timeinfo.frames;
        //t=timeinfo.rate;
        //timeinfo.bitrate=((long long )timeinfo.framelen*8*t/timeinfo.blocks)/1000.0;
        aac_printf("init ok \n");
        return 0;
    }

#endif
    offset = AACFindSyncWord(temp, ret);

    if (offset < 0)
    {
        timeinfo.totaltime = (((double)(timeinfo.blocks * timeinfo.frames)) / timeinfo.rate) * 1000;
        timeinfo.bitrate = timeinfo.framelen * 8 / (timeinfo.totaltime);
        timeinfo.framelen = timeinfo.framelen / timeinfo.frames;
        //t=timeinfo.rate;
        //timeinfo.bitrate=((long long )timeinfo.framelen*8*t/timeinfo.blocks)/1000.0;
        return -1;
    }

    curpos += offset;
    p = temp + offset;
    bitsAvail = (ret - offset) << 3;

    if (bitsAvail < NUM)
    {
        aac_printf("less bits may be error \n");
    }

    /*
    showhl(temp[0]);
    showhl(temp[1]);
    DelayMs(1000);
    */

    if (UnpackADTSHeaderForTime(&curpos, &p, &bitOffset, &bitsAvail) == ERR_AAC_INVALID_ADTS_HEADER)
    {
        return ERR_AAC_INVALID_ADTS_HEADER;
    }

    bitOffset = 0;
    RKFIO_FSeek(curpos, 0, fp); //here current positon had updata in UnpackADTSHeaderForTime

    while (1)
    {
        ret = RKFIO_FRead(temp, 64, fp);
        offset = AACFindSyncWord(temp, ret);

        if (offset < 0)
        {
            timeinfo.totaltime = (((double)(timeinfo.blocks * timeinfo.frames)) / timeinfo.rate) * 1000;
            timeinfo.bitrate = timeinfo.framelen * 8 / (timeinfo.totaltime);
            timeinfo.framelen = timeinfo.framelen / timeinfo.frames;
            //t=timeinfo.rate;
            //timeinfo.bitrate=((long long )timeinfo.framelen*8*t/timeinfo.blocks)/1000.0;
            return -1;
        }

        timeinfo.frames++;
#if 0
        {
            int i;
            DEBUG("%d\n", offset);

            for (i = 0; i < 8; i++)
            {
                DEBUG("0x%2x ", temp[offset + i]);
            }

            DEBUG("\n");
        }
#endif
        /* frame length */
        count = ((((int)(temp[offset + 3] & 0x00000003)) << 11)
                 | (((int)temp[offset + 4]) << 3)
                 | ((((int)temp[offset + 5]) >> 5) & 0x00000007));
        timeinfo.framelen += count;
        curpos += count;
        curpos += offset;

        //DEBUG("curpos = %x\n",curpos);

        if (curpos >= filelen)
        {
            break;
        }

        RKFIO_FSeek(curpos, 0, fp);
    }

    timeinfo.totaltime = (((double)(timeinfo.blocks * timeinfo.frames)) / timeinfo.rate) * 1000;
    timeinfo.bitrate = timeinfo.framelen * 8 / (timeinfo.totaltime);
    timeinfo.framelen = timeinfo.framelen / timeinfo.frames;
    //t=timeinfo.rate;
    //timeinfo.bitrate=((long long )timeinfo.framelen*8*t/timeinfo.blocks)/1000.0;
    return 0;
}
void InitTimeForADIF(FILE *fp)
{
    AAC_TIMEINFO *pt = &timeinfo;
    aac_MemSet(pt, 0, sizeof(AAC_TIMEINFO));
}
long GetCurTime_AAC()
{
    long time;
    double t;
    t = 1000.0 * timeinfo.blocks / timeinfo.rate;

    if (timeinfo.curframe > timeinfo.frames)
    {
        timeinfo.curframe = timeinfo.frames;
    }

    return  (long )(timeinfo.curframe * t);
}

long GetTotalTime_AAC()
{
    return timeinfo.totaltime;
}

AAC_TIMEINFO *GetInfo_AAC()
{
    return  &timeinfo;
}

//返回－1，不需要SEEK，已到文件尾，否则返回seek 后 文件位置
int SeekTime_AAC(long offsettime, HAACDecoder hAACDecoder, FILE *fp)
{
    long pos;
    unsigned char buf[2];
    int ret;
    int flag = 0;
    int newframes;
    int bak;
    unsigned char tc[4];
    int bitoffset ;
    int bitav ;
    int res ;
    unsigned char **tp = (unsigned char **)&tc;
    int si;
    int ci;

    /*  if(offsettime+4000>=timeinfo.totaltime)
            return -1;
        */
    if (offsettime > timeinfo.totaltime)
        offsettime = timeinfo.totaltime;

    newframes = offsettime / (timeinfo.blocks * 1000.0 / timeinfo.rate);

    if (newframes > timeinfo.frames)
    {
        newframes = timeinfo.frames;
    }

    pos = timeinfo.framelen * newframes + offset_by_helun;
    RKFIO_FSeek(pos, 0 , fp);
    bak = pos;
    ret = RKFIO_FRead(buf, 1, fp);

    if (ret < 1)
    {
        aac_DEBUG(" rkfile read ret < 0");
        return -1;
    }

    bak++;

    while (!RKFIO_FEof(fp))
    {
        ret = RKFIO_FRead(buf + 1, 1, fp);

        if (ret < 1)
        {
            aac_DEBUG(" rkfile read ret < 0");
            return -1;
        }

        bak++;

        if (buf[0] == 0xff && ((buf[1] >> 4) == 0xf))
        {
#if 1
            {
                RKFIO_FSeek(-2, 1, fp);

                aac_MemSet(tc, 0, sizeof(tc));
                RKFIO_FRead(tc, 4, fp);
                RKFIO_FSeek(-2, 1, fp);
                si = (tc[2] >> 2) & 0xf;
                ci =  ((tc[2] & 1) << 2) | ((tc[3] >> 6) & 3);
                //DEBUG("si = %d,ci = %d  s=%d,c=%d\n",si,ci, sampRateTab[si],channelMapTab[ci] );

                if ((channelMapTab[ci] != timeinfo.channels) || (timeinfo.rate != sampRateTab[si]))
                {
                    aac_printf("seek position is not correct  ,i will seek to next frame\n");
                    buf[0] = buf[1];
                    continue ;
                }
            }
#endif
            flag = 1;
            break;
        }
        else
            buf[0] = buf[1];
    }

    if (flag == 0)
    {
        aac_DEBUG("flag == 0");
        return -1;
    }

    RKFIO_FSeek(-2, 1, fp);
    pos = bak - 2;
    //FileSeek(pos,0, fp);
    timeinfo.curframe = pos / timeinfo.framelen + 1;
    timeinfo.curtime = timeinfo.curframe * (timeinfo.blocks * 1000.0 / timeinfo.rate);
    aac_DEBUG("curFrame = %d,curTime = %d", timeinfo.curframe, timeinfo.curtime);
    /*
    #ifndef HELUN
    AACFlushCodec(hAACDecoder);
    #endif

    */
    //重新读取文件 中数据 送到CODEC解码
    return pos;
}
/*
void InitTime(unsigned int  len,FILE *pRawFileCache)
{
    FILE *fp=NULL;
    unsigned char id[4];


    fp=pRawFileCache;



        FileRead(id, 4, fp);

    if(id[0]=='A'&&id[1]=='D'&&id[2]=='I'&&id[3]=='F')
        {



        InitTimeForADIF(fp);


        }
    else
        {
        if(id[0]==0xff&&(id[1]&0xf0)==0xf0)
            {

            InitTimeForADTS(fp, len);




            }
        else
            {

            return ;
            }
        }








}*/

void InitAACDataTable(void)
{
    /*
                       extern int *cos4sin4tab64;
                       extern int *cos1sin1tab64;
                       extern int *twidTabOdd32;
    */
    cos4sin4tab64 = (int*)_cos4sin4tab64_aacdec;
    cos1sin1tab64 = (int*)_cos1sin1tab64_aacdec;
    twidTabOdd32 = (int*)_twidTabOdd32_aacdec;
}

/**************************************************************************************
 * Function:    AACInitDecoder
 *
 * Description: allocate memory for platform-specific data
 *              clear all the user-accessible fields
 *              initialize SBR decoder if enabled
 *
 * Inputs:      none
 *
 * Outputs:     none
 *
 * Return:      handle to AAC decoder instance, 0 if malloc fails
 **************************************************************************************/
HAACDecoder AACInitDecoder(void)
{
    AACDecInfo *aacDecInfo;
    aacDecInfo = AllocateBuffers();

    if (!aacDecInfo)
        return 0;

    return (HAACDecoder)aacDecInfo;
}

/**************************************************************************************
 * Function:    AACFreeDecoder
 *
 * Description: free platform-specific data allocated by AACInitDecoder
 *              free SBR decoder if enabled
 *
 * Inputs:      valid AAC decoder instance pointer (HAACDecoder)
 *
 * Outputs:     none
 *
 * Return:      none
 **************************************************************************************/
void AACFreeDecoder(HAACDecoder hAACDecoder)
{
    AACDecInfo *aacDecInfo = (AACDecInfo *)hAACDecoder;

    if (!aacDecInfo)
        return;

    FreeBuffers(aacDecInfo);
}

/**************************************************************************************
 * Function:    AACFindSyncWord
 *
 * Description: locate the next byte-alinged sync word in the raw AAC stream
 *
 * Inputs:      buffer to search for sync word
 *              max number of bytes to search in buffer
 *
 * Outputs:     none
 *
 * Return:      offset to first sync word (bytes from start of buf)
 *              -1 if sync not found after searching nBytes
 **************************************************************************************/
int AACFindSyncWord(unsigned char *buf, int nBytes)
{
    int i;

    /* find byte-aligned syncword (12 bits = 0xFFF) */
    for (i = 0; i < nBytes - 1; i++) {
        if ( (buf[i+0] & SYNCWORDH) == SYNCWORDH && (buf[i+1] & SYNCWORDL) == SYNCWORDL )
            return i;
    }
    return -1;
}
extern int sameheader;
int AACFindSyncWord2(unsigned char *buf, int nBytes)
{
    int i;

    /* find byte-aligned syncword (12 bits = 0xFFF) */
    for (i = 0; i < nBytes - 1; i++) {
        if ((buf[i + 0] == (sameheader >> 16 &0xff)) && (buf[i + 1] == (sameheader >> 8 & 0xff))&&(buf[i + 2] == (sameheader &0xff)))
            return i;
    }

    return -1;
}

/**************************************************************************************
 * Function:    AACGetLastFrameInfo
 *
 * Description: get info about last AAC frame decoded (number of samples decoded,
 *                sample rate, bit rate, etc.)
 *
 * Inputs:      valid AAC decoder instance pointer (HAACDecoder)
 *              pointer to AACFrameInfo struct
 *
 * Outputs:     filled-in AACFrameInfo struct
 *
 * Return:      none
 *
 * Notes:       call this right after calling AACDecode()
 **************************************************************************************/
void AACGetLastFrameInfo(HAACDecoder hAACDecoder, AACFrameInfo *aacFrameInfo)
{
    AACDecInfo *aacDecInfo = (AACDecInfo *)hAACDecoder;

    if (!aacDecInfo)
    {
        aacFrameInfo->bitRate =       0;
        aacFrameInfo->nChans =        0;
        aacFrameInfo->sampRateCore =  0;
        aacFrameInfo->sampRateOut =   0;
        aacFrameInfo->bitsPerSample = 0;
        aacFrameInfo->outputSamps =   0;
        aacFrameInfo->profile =       0;
        aacFrameInfo->tnsUsed =       0;
        aacFrameInfo->pnsUsed =       0;
    }
    else
    {
        aacFrameInfo->bitRate =       aacDecInfo->bitRate;
        aacFrameInfo->nChans =        aacDecInfo->nChans;
        aacFrameInfo->sampRateCore =  aacDecInfo->sampRate;
        aacFrameInfo->sampRateOut =   aacDecInfo->sampRate * (IS_SBR_ENABLE * aacDecInfo->sbrEnabled ? 2 : 1);
        aacFrameInfo->bitsPerSample = 16;
        aacFrameInfo->outputSamps =   aacDecInfo->nChans * AAC_MAX_NSAMPS * ((IS_SBR_ENABLE * aacDecInfo->sbrEnabled) ? 2 : 1); //by Vincent Hsiung
        aacFrameInfo->profile =       aacDecInfo->profile;
        aacFrameInfo->tnsUsed =       aacDecInfo->tnsUsed;
        aacFrameInfo->pnsUsed =       aacDecInfo->pnsUsed;
    }
}

/**************************************************************************************
 * Function:    AACSetRawBlockParams
 *
 * Description: set internal state variables for decoding a stream of raw data blocks
 *
 * Inputs:      valid AAC decoder instance pointer (HAACDecoder)
 *              flag indicating source of parameters
 *              AACFrameInfo struct, with the members nChans, sampRate, and profile
 *                optionally filled-in
 *
 * Outputs:     updated codec state
 *
 * Return:      0 if successful, error code (< 0) if error
 *
 * Notes:       if copyLast == 1, then the codec sets up its internal state (for
 *                decoding raw blocks) based on previously-decoded ADTS header info
 *              if copyLast == 0, then the codec uses the values passed in
 *                aacFrameInfo to configure its internal state (useful when the
 *                source is MP4 format, for example)
 **************************************************************************************/
int AACSetRawBlockParams(HAACDecoder hAACDecoder, int copyLast, AACFrameInfo *aacFrameInfo)
{
    AACDecInfo *aacDecInfo = (AACDecInfo *)hAACDecoder;

    if (!aacDecInfo)
        return ERR_AAC_NULL_POINTER;

    if (aacDecInfo->format != AAC_FF_LATM)
        aacDecInfo->format = AAC_FF_RAW;

    if (copyLast)
        return SetRawBlockParams(aacDecInfo, 1, 0, 0, 0);
    else
        return SetRawBlockParams(aacDecInfo, 0, aacFrameInfo->nChans, aacFrameInfo->sampRateCore, aacFrameInfo->profile);
}

/**************************************************************************************
 * Function:    AACFlushCodec
 *
 * Description: flush internal codec state (after seeking, for example)
 *
 * Inputs:      valid AAC decoder instance pointer (HAACDecoder)
 *
 * Outputs:     updated state variables in aacDecInfo
 *
 * Return:      0 if successful, error code (< 0) if error
 **************************************************************************************/
int AACFlushCodec(HAACDecoder hAACDecoder)
{
    int ch;
    AACDecInfo *aacDecInfo = (AACDecInfo *)hAACDecoder;

    if (!aacDecInfo)
        return ERR_AAC_NULL_POINTER;

    /* reset common state variables which change per-frame
     * don't touch state variables which are (usually) constant for entire clip
     *   (nChans, sampRate, profile, format, sbrEnabled)
     */
    aacDecInfo->prevBlockID = AAC_ID_INVALID;
    aacDecInfo->currBlockID = AAC_ID_INVALID;
    aacDecInfo->currInstTag = -1;

    for (ch = 0; ch < MAX_NCHANS_ELEM; ch++)
        aacDecInfo->sbDeinterleaveReqd[ch] = 0;

    aacDecInfo->adtsBlocksLeft = 0;
    aacDecInfo->tnsUsed = 0;
    aacDecInfo->pnsUsed = 0;
    /* reset internal codec state (flush overlap buffers, etc.) */
    FlushCodec(aacDecInfo);
    return ERR_AAC_NONE;
}

//0:ADTS 1:ADIF 2:RAW 3:LATM
void SetAACFormat(HAACDecoder hAACDecoder, int n)
{
    AACDecInfo *aacDecInfo = (AACDecInfo *)hAACDecoder;

    switch (n)
    {
        case 0:
        {
            aacDecInfo->format = AAC_FF_ADTS;
            break;
        }

        case 1:
        {
            aacDecInfo->format = AAC_FF_ADIF;
            break;
        }

        case 2:
        {
            aacDecInfo->format = AAC_FF_RAW;
            break;
        }

        case 3:
        {
            aacDecInfo->format = AAC_FF_LATM;
            break;
        }
    }
}

static int isCMMB_SP = 0;
void RK_SetCMMB_SP(int s)
{
    isCMMB_SP = s;
}
/**************************************************************************************
 * Function:    AACDecode
 *
 * Description: decode AAC frame
 *
 * Inputs:      valid AAC decoder instance pointer (HAACDecoder)
 *              double pointer to buffer of AAC data
 *              pointer to number of valid bytes remaining in inbuf
 *              pointer to outbuf, big enough to hold one frame of decoded PCM samples
 *                (outbuf must be double-sized if SBR enabled)
 *
 * Outputs:     PCM data in outbuf, interleaved LRLRLR... if stereo
 *                number of output samples = 1024 per channel (2048 if SBR enabled)
 *              updated inbuf pointer
 *              updated bytesLeft
 *
 * Return:      0 if successful, error code (< 0) if error
 *
 * Notes:       inbuf pointer and bytesLeft are not updated until whole frame is
 *                successfully decoded, so if ERR_AAC_INDATA_UNDERFLOW is returned
 *                just call AACDecode again with more data in inbuf
 **************************************************************************************/
int extStart = 0;
#define NONELATM
extern int m4afileformflag;
int AACDecode(HAACDecoder hAACDecoder, unsigned char **inbuf, int *bytesLeft, short *outbuf)
{
    int err, offset, bitOffset, bitsAvail;
    int ch, baseChan, baseChanSBR, elementChans;
    unsigned char *inptr;
    int start = 0;                  //indicate where the bitstream should start , by Vincent
    AACFrameInfo aacFrameInfo;  //used for RAW aac or LATM aac , by Vincent
    AACDecInfo *aacDecInfo = (AACDecInfo *)hAACDecoder;
    //By Vincent Hsiung , July 29,2008
    int timeout_cnt = 0;

    if (!aacDecInfo)
    {
        return 1;
    }

    /* make local copies (see "Notes" above) */
    inptr = *inbuf;
    bitOffset = 0;
    //get all bits  by Helun
    bitsAvail = (*bytesLeft) << 3;
    aacDecInfo->format = AAC_FF_ADTS;

    /* first time through figure out what the file format is */
    if (aacDecInfo->format == AAC_FF_Unknown)
    {
        if (bitsAvail < 32)
            return 2;

        if (IS_ADIF(inptr))
        {
            /* unpack ADIF header */
            aacDecInfo->format = AAC_FF_ADIF;
            err = UnpackADIFHeader(aacDecInfo, &inptr, &bitOffset, &bitsAvail);

            if (err)
                return err;
        }
        else
        {
            /* assume ADTS by default */
            aacDecInfo->format = AAC_FF_ADTS;
        }
    }

    /* if ADTS, search for start of next frame */
    if (aacDecInfo->format == AAC_FF_ADTS) {
        /* can have 1-4 raw data blocks per ADTS frame (header only present for first one) */
        //DEBUG("aacDecInfo->adtsBlocksLeft==%d\n",aacDecInfo->adtsBlocksLeft);
        if (aacDecInfo->adtsBlocksLeft == 0) {
            if(m4afileformflag==1)
            {
                if (timeinfo.curframe ==0 )

                   offset = AACFindSyncWord(inptr, bitsAvail >> 3);
                else
                {
                   offset = AACFindSyncWord2(inptr, bitsAvail >> 3);
                }
            }
           else
               offset = AACFindSyncWord(inptr, bitsAvail >> 3);

            if (offset < 0)
            {
                return -1;
            }

            inptr += offset;
            bitsAvail -= (offset << 3);
            err = UnpackADTSHeader(aacDecInfo, &inptr, &bitOffset, &bitsAvail);

            if (err)
                return err;

            if (aacDecInfo->nChans == -1)
            {
                /* figure out implicit channel mapping if necessary */
                err = GetADTSChannelMapping(aacDecInfo, inptr, bitOffset, bitsAvail);

                if (err)
                    return err;
            }
        }

        aacDecInfo->adtsBlocksLeft--;
    }
    else if (aacDecInfo->format == AAC_FF_RAW)
    {
        err = PrepareRawBlock(aacDecInfo);

        if (err)
            return err;
    }
    else if (aacDecInfo->format == AAC_FF_LATM)
    {
//check_latm:
        //check latm
#ifndef NONELATM
        {
//          #include <stdint.h>
#include "bitstream.h"
#define MAX_ASC_BYTES 64
            typedef struct
            {
                int inited;
                int version, versionA;
                int framelen_type;
                int useSameStreamMux;
                int allStreamsSameTimeFraming;
                int numSubFrames;
                int numPrograms;
                int numLayers;
                int otherDataPresent;
                uint32_t otherDataLenBits;
                uint32_t frameLength;
                uint8_t ASC[MAX_ASC_BYTES];
                uint32_t ASCbits;
            } latm_header;
            latm_header latm;
            BitStreamInfo bsi;
            SetBitstreamPointer(&bsi, *bytesLeft, inptr);
            aac_MemSet(&latm, 0 , sizeof(latm_header));
            //DEBUG("start parse LATM start pos %d input %d\n",start,inptr);
            /* bug fix by Vincent Hsiung , @ Apr 15 , 2009 */
            latm.inited = 1;

            if ((-1) != faad_latm_frame(&latm, &bsi, &start, &aacFrameInfo))
            {
                aacFrameInfo.nChans = 2;

                if ((aacFrameInfo.sampRateCore < 8000) || (aacFrameInfo.sampRateCore > 48000))
                    aacFrameInfo.sampRateCore = 24000;

                aacFrameInfo.profile = AAC_PROFILE_LC;
                AACSetRawBlockParams(hAACDecoder, 0 , &aacFrameInfo);
            }
            else
            {
                return (-1);
            }
        }
        err = PrepareRawBlock(aacDecInfo);

        if (err)
            return err;

#endif
        return -2;
    }

    /* check for valid number of channels */
    if (aacDecInfo->nChans > AAC_MAX_NCHANS || aacDecInfo->nChans <= 0)
    {
        //DisplayTestDecNum(80,40,53);
        //DisplayTestDecNum(80,50,aacDecInfo->nChans);
        aac_delayms(10000);
        return 4;
    }

    /* will be set later if active in this frame */
    aacDecInfo->tnsUsed = 0;
    aacDecInfo->pnsUsed = 0;
    bitOffset = start;//0;  //By Vincent
    baseChan = 0;
    baseChanSBR = 0;
    //DEBUG("start decode start pos %d input %d\n",start,inptr);
    extStart = start;

    do
    {
        // for only process 3 channels
        int maxCFlag = 0;

        if (timeout_cnt++ > 0xff)
            break;

        /* parse next syntactic element */
        err = DecodeNextElement(aacDecInfo, &inptr, &bitOffset, &bitsAvail);

        if (err)
            return err;

        elementChans = elementNumChans[aacDecInfo->currBlockID];

        // for only process 3 channels
        if (baseChan + elementChans > 3)
            maxCFlag = 1; //by Vincent

        if (baseChan + elementChans > AAC_MAX_NCHANS)
            return 5;

        /* noiseless decoder and dequantizer */
        for (ch = 0; ch < elementChans; ch++)
        {
            err = DecodeNoiselessData(aacDecInfo, &inptr, &bitOffset, &bitsAvail, ch);

            if (err)
                return err;

            if (Dequantize(aacDecInfo, ch))
                return 6;
        }

        /* mid-side and intensity stereo */
        if (aacDecInfo->currBlockID == AAC_ID_CPE)
        {
            if (StereoProcess(aacDecInfo))
                return 7;
        }

// for only process 3 channels
        if (!maxCFlag)
        {
            /* PNS, TNS, inverse transform */
            for (ch = 0; ch < elementChans; ch++)
            {
                if (PNS(aacDecInfo, ch))
                    return 8;

                if (aacDecInfo->sbDeinterleaveReqd[ch])
                {
                    /* deinterleave short blocks, if required */
                    if (DeinterleaveShortBlocks(aacDecInfo, ch))
                        return 9;

                    aacDecInfo->sbDeinterleaveReqd[ch] = 0;
                }

                if (TNSFilter(aacDecInfo, ch))
                    return 10;

                if  (IMDCT(aacDecInfo, ch, baseChan + ch, outbuf))
                    return 11;
            }
        }

        baseChan += elementChans;
    }
    while (aacDecInfo->currBlockID != AAC_ID_END);

    /*
        if(aacDecInfo->nChans>=3)
            {

    int t1,t2,t;
    int k;

    for(k=1024;k<1024*3;k++)
    {
        t1=outbuf[k];
        t2=outbuf[(k-1024)%1024];
        t=(t1+t2)/2;
        outbuf[k]=(short)t;


    }
        memmove(outbuf,outbuf+1024,2048*2);
        aacDecInfo->nChans=2;


        }*/

    if (aacDecInfo->nChans >= 3)
    {
        int k;
        int t;
        int t1, t2;

        for (k = 0; k < 2048; k++)
        {
            t1 = outbuf[k];
            t2 = outbuf[(k % 1024) + 2048];
            t = (t1 + t2) / 2;
            outbuf[k] = (short)t;
        }

        aacDecInfo->nChans = 2;
    }

    //By Vincent , 10 07 2008,TEST!!
    if ((aacDecInfo->nChans == 1) && (baseChan == 1))
    {
        short *p = outbuf;
        int i;
        short tmp[AAC_MAX_NSAMPS];

        for (i = 0; i < AAC_MAX_NSAMPS; i++)
        {
            tmp[i] = outbuf[i];
        }

        for (i = 0 ; i < AAC_MAX_NSAMPS ; i++)
        {
            //tmp = *p++;
            //*p++ = tmp;
            outbuf[2 * i] = tmp[i];
            outbuf[2 * i + 1] = tmp[i];
        }
    }
    else if (aacDecInfo->nChans == 2)
    {
        if (isCMMB_SP == 1)
        {
            short *p = outbuf;
            int tmpL;
            int tmpR;
            int i;

            for (i = 0 ; i < AAC_MAX_NSAMPS * 2 ; i++)
            {
                tmpL = *p;
                tmpR = *(p + 1);
                *p++ = ((tmpL >> 1) + (tmpR >> 1));
                *p++ = ((tmpL >> 1) + (tmpR >> 1));
            }
        }
    }

    /* byte align after each raw_data_block */
    if (bitOffset)
    {
        inptr++;
        bitsAvail -= (8 - bitOffset);
        bitOffset = 0;

        if (bitsAvail < 0)
            return 12;
    }

    /* update pointers */
    aacDecInfo->frameCount++;
    *bytesLeft -= (inptr - *inbuf);
    *inbuf = inptr;
    //DEBUG("end decode pos %d\n",inptr);
    return ERR_AAC_NONE;
}

#pragma arm section code
#endif
#endif

