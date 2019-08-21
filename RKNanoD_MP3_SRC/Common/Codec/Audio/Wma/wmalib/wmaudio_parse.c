//*@@@+++@@@@******************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//*@@@---@@@@******************************************************************
/*
 * Windows Media Audio (WMA) Decoder API (parsering)
 *
 * Copyright (c) Microsoft Corporation 1999.  All Rights Reserved.
 */
//#if defined _WIN32
//#include <windows.h>
//#define _BOOL_DEFINED
//#endif

//#define STDCALL __stdcall // For drmpd_ext.h

#ifdef __arm
#define FAR
#define TRUE  1
#define FALSE 0
#endif //__arm

//#include <stdio.h>
//#include <stddef.h>
//#ifndef __arm
//#include "malloc.h"
//#endif
#include "../include/audio_main.h"
#include "..\wmaInclude\macros.h"
#include "..\wmaInclude\wmaudio.h"
#include "..\wmaInclude\wmaudio_type.h"
#include "..\wmaInclude\loadstuff.h"
#include "..\wmaInclude\msaudiofmt.h"
#include "..\wmaInclude\audio_table_room.h"
#define INITGUID
#include "..\wmaInclude\wmaguids.h"
#include "..\wmaInclude\drmpd.h"

#ifdef A_CORE_DECODE
#ifdef WMA_DEC_INCLUDE
//#pragma arm section zidata = "WmaDecoderBss",rwdata = "WmaDecoderData"
#pragma arm section zidata = "WmaDecoderBss",rwdata = "WmaDecoderData",rodate = "WmaCommonCode"


//#ifndef MIN
//#define MIN(x, y)  ((x) < (y) ? (x) : (y))
//#endif /* MIN */

//#define MIN_WANTED 64

//
//  Routines for byte-swapping the members of various structs
//

tWMAFileContDesc *g_pdesc = NULL;
tWMAFileContDesc gDesc;
#define MAX_LIC_DATA_LEN 256
_ATTR_WMA_OPEN_CODEC_BSS_
static unsigned char gm_pLicData[MAX_LIC_DATA_LEN];
//unsigned char g_Buffer[1024];
//#define MAX_BUFSIZE 512 //128 20080724
#ifdef WMAINITIALIZE
#pragma arm section code = "WmaOpenCodecCode"

/****************************************************************************/
/****************************************************************************/
static DWORD HnsQWORDtoMsDWORD(QWORD qw)
{
    DWORD msLo, msHi;

    msLo = qw.dwLo / 10000;
    msHi = (DWORD)((float)qw.dwHi * 429496.7296);

    return msLo + msHi;
}
_ATTR_WMA_OPEN_CODEC_BSS_
static BYTE gWMAMetatdateStrBuf[WMA_MAX_METADATA_LEN + 2];
_ATTR_WMA_OPEN_CODEC_BSS_
static int gWMAMetatdateStrBufOffset;
WMAERR AllocateAndLoadMetadataStr(tHWMAFileState *pInt, U64 *piCurrFileOffset,const U64 iMaxFileOffset, WORD *pcbTotalBytesWanted,BYTE **ppBuffer)
{
    BYTE *pData = NULL;
    WMAERR wmaerr = WMAERR_OK;
    DWORD cbBytesToRead;
    DWORD cbBytesRead = 0;
    BYTE *pBuffer = NULL;

    // WMAFileCBGetData uses MAX_BUFSIZE, but this function uses WMA_MAX_DATA_REQUESTED.
    // I don't know why we need two constants here, but just make sure our assumptions hold.
    assert(WMA_MAX_DATA_REQUESTED <= MAX_BUFSIZE);

    // Check if reading the requested number of bytes would push us past end of object
    if (*piCurrFileOffset + *pcbTotalBytesWanted > iMaxFileOffset)
    {
        wmaerr = TraceResult(WMAERR_CORRUPTDATA);

        // Return directly, to avoid advancing *piCurrFileOffset into hyperspace
        return wmaerr;
    }

    // Limit us to a reasonable size, esp for embedded devices. No 64kB artist names!
    assert(WMA_MAX_METADATA_LEN < ((U32)1 << (8*sizeof(*pcbTotalBytesWanted)))); // 16-bit length MAX
    cbBytesToRead = *pcbTotalBytesWanted;
    //if (cbBytesToRead > WMA_MAX_METADATA_LEN)
    //cbBytesToRead = WMA_MAX_METADATA_LEN;

    if (cbBytesToRead + gWMAMetatdateStrBufOffset > WMA_MAX_METADATA_LEN)
        cbBytesToRead = WMA_MAX_METADATA_LEN - gWMAMetatdateStrBufOffset;

    if (cbBytesToRead < 0)
    {
        //while (1);//for debug
        return WMAERR_OK;//return ok to let decoder work even when something wrong with id3 parse
    }
    pBuffer = gWMAMetatdateStrBuf + gWMAMetatdateStrBufOffset;//malloc(cbBytesToRead + sizeof(U16)); // +1 for null-term
    if (NULL == pBuffer)
    {
        wmaerr = TraceResult(WMAERR_OUTOFMEMORY);
        goto exit;
    }
    memset(pBuffer, 0, cbBytesToRead + sizeof(U16)); // +1 for null-term

    //for (cbBytesRead = 0; cbBytesRead < cbBytesToRead;)
    {
        DWORD cbWanted;
        DWORD cbActual;

        cbWanted = cbBytesToRead;//MIN(WMA_MAX_DATA_REQUESTED, (cbBytesToRead - cbBytesRead));
        //cbActual = WMAFileCBGetData(pInt, *piCurrFileOffset + cbBytesRead, cbWanted, &pData);
        cbActual = WMAFileCBGetData(pInt, *piCurrFileOffset, cbWanted, &pData);
        if (cbActual != cbWanted)
        {
            //TraceInfo2("AllocateAndLoadMetadataStr requested %d from WMAFileCBGetData, got %d!",
            //    cbWanted, cbActual);
            wmaerr = TraceResult(WMAERR_BUFFERTOOSMALL);
            goto exit;
        }

        memcpy(pBuffer, pData, cbActual);
        cbBytesRead += cbActual;
        gWMAMetatdateStrBufOffset += cbActual;
    }

    //if (cbBytesRead != cbBytesToRead)
    {
       // wmaerr = TraceResult(WMAERR_BUFFERTOOSMALL);
        //goto exit;
    }

exit:
    // Deal with errors in a "transactional" fashion: either succeed completely,
    // or fail completely. In other words, clean up if we encountered any errors.
    if (WMAERR_OK != wmaerr)
    {
        cbBytesRead = 0;
#if 0
        if (pBuffer)
        {
            free(pBuffer);
            pBuffer = NULL;
        }
#endif
    }

    // Return our results
    //assert(cbBytesRead < ((U32)1 << (8*sizeof(*pcbTotalBytesWanted)))); // 16-bit length MAX
    *piCurrFileOffset += *pcbTotalBytesWanted; // Increment by requested amt even if we truncated
    *pcbTotalBytesWanted = (WORD)cbBytesRead; // Report actual number of bytes read
    *ppBuffer = pBuffer;

    return wmaerr;
}

WMAERR
WMA_LoadObjectHeader(tWMAFileHdrStateInternal *pInt,
                     GUID *pObjectId,
                     QWORD *pqwSize)
{
    BYTE *pData = NULL;
    DWORD cbActual = 0;
    DWORD cbWanted = 0;

//    if(pInt == NULL)
    //    {
    //        return WMAERR_INVALIDARG;
    //    }


    cbWanted = MIN_OBJECT_SIZE;//objectid(16bytes)+objectsize(8bytes)
    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                pInt->currPacketOffset, cbWanted, &pData);
    if (cbActual != cbWanted)
    {
        return WMAERR_BUFFERTOOSMALL;
    }

    pInt->currPacketOffset += cbActual;

    LoadGUID(*pObjectId, pData);//extended content description objectid
    LoadQWORD(*pqwSize, pData);//extended content description objectsize

    if (pqwSize->dwLo  < MIN_OBJECT_SIZE)
        return WMAERR_INVALIDHEADER;


    return WMAERR_OK;
}
/****************************************************************************/
WMAERR WMA_LoadVirtualObjectHeader(tWMAFileHdrStateInternal *pInt,
                                   GUID *pObjectId,
                                   QWORD *pqwSize,
                                   DWORD Offset)
{
    BYTE *pData = NULL;
    DWORD cbActual = 0;
    DWORD cbWanted = 0;

    cbWanted = MIN_OBJECT_SIZE;
    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                pInt->currPacketOffset + Offset, cbWanted, &pData);
    if (cbActual != cbWanted)
        return WMAERR_BUFFERTOOSMALL;


    LoadGUID((*pObjectId), pData);
    LoadQWORD((*pqwSize), pData);

    if (pqwSize->dwLo  < MIN_OBJECT_SIZE)
        return WMAERR_INVALIDHEADER;

    return WMAERR_OK;
}

/*****************************asf header object*****************************************/

WMAERR
WMA_LoadHeaderObject(tWMAFileHdrStateInternal *pInt,
                     int isFull)
{
    GUID objectId;//16bytes
    QWORD qwSize; //8bytes
    DWORD cHeaders = 0;//4bytes
    BYTE align = 0;  //1bytes
    BYTE arch = 0;   //1bytes

    BYTE *pData = NULL;
    DWORD cbActual = 0;
    DWORD cbWanted = 0;

    if (pInt == NULL)
    {
        return WMAERR_INVALIDARG;
    }

    cbWanted = MIN_OBJECT_SIZE + sizeof(DWORD) + 2 * sizeof(BYTE);//objectid+qwsize+cheaders+align+arch
    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                pInt->currPacketOffset, cbWanted, &pData);
    if (cbActual != cbWanted)
    {
        return WMAERR_BUFFERTOOSMALL;
    }


    pInt->currPacketOffset += cbActual;

    //header object(object id,object size,number of header objects,alignment,architecture)'s data
    LoadGUID(objectId, pData);
    LoadQWORD(qwSize, pData);
    LoadDWORD(cHeaders, pData);
    LoadBYTE(align, pData);
    LoadBYTE(arch, pData);

#ifdef WMA_TABLE_ROOM_VERIFY
    if (!WMA_IsEqualGUID(p_CLSID_CAsfHeaderObjectV0, &objectId)
#else
    if (!WMA_IsEqualGUID(&CLSID_CAsfHeaderObjectV0, &objectId)
#endif
            || align != 1
            || arch != 2)
    {
        return WMAERR_INVALIDHEADER;
    }

    /* use all */
    pInt->cbHeader = qwSize.dwLo;//pint->cbheader=objectsize .e.t qwSize low32bits

    return WMAERR_OK;
}

WMAERR
WMA_LoadPropertiesObject(tWMAFileHdrStateInternal *pInt,
                         DWORD cbSize,
                         int isFull)
{
    GUID mmsId;
    QWORD qwTotalSize;
    QWORD qwCreateTime;
    QWORD qwPackets;
    QWORD qwPlayDuration;
    QWORD qwSendDuration;
    QWORD qwPreroll;
    DWORD dwFlags = 0;
    DWORD dwMinPacketSize = 0;
    DWORD dwMaxPacketSize = 0;
    DWORD dwMaxBitrate = 0;

    BYTE *pData = NULL;
    DWORD cbActual = 0;
    DWORD cbWanted = 0;

    if (pInt == NULL)
    {
        return WMAERR_INVALIDARG;
    }
    cbSize -= MIN_OBJECT_SIZE;


    cbWanted = sizeof(GUID) + 6 * sizeof(QWORD) + 4 * sizeof(DWORD);
    if (cbWanted > cbSize)
    {
        return WMAERR_BUFFERTOOSMALL;
    }
    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                pInt->currPacketOffset, cbWanted, &pData);
    if (cbActual != cbWanted)
    {
        return WMAERR_BUFFERTOOSMALL;
    }


    LoadGUID(mmsId, pData);            //MMS ID(multimedia-stream id) in file properties object
    LoadQWORD(qwTotalSize, pData);     //total_size in file properties object
    LoadQWORD(qwCreateTime, pData);    //created_time in file properties object
    LoadQWORD(qwPackets, pData);       //num_interleave_packets in file properties object
    LoadQWORD(qwPlayDuration, pData);  //play_duration in file properties object
    LoadQWORD(qwSendDuration, pData);  //send_duration in file properties object
    LoadQWORD(qwPreroll, pData);       //preroll in file properties object
    LoadDWORD(dwFlags, pData);         //flags in file properties object
    LoadDWORD(dwMinPacketSize, pData); //min_interleave_packet_size in file properties object
    LoadDWORD(dwMaxPacketSize, pData); //max_interleave_packet_size in file properties object
    LoadDWORD(dwMaxBitrate, pData);    //maximum_bit_rate in file properties object

    if (dwMinPacketSize != dwMaxPacketSize|| (qwPackets.dwLo == 0 && qwPackets.dwHi == 0))
    {
        return WMAERR_FAIL;
    }

    pInt->cbPacketSize = dwMaxPacketSize;
    pInt->cPackets     = qwPackets.dwLo;
    pInt->msDuration   = HnsQWORDtoMsDWORD(qwPlayDuration);
    //qwSendDuration.dwHi = qwSendDuration.dwHi - (qwPreroll.dwHi*10000);
    qwSendDuration.dwLo = qwPlayDuration.dwLo - (qwPreroll.dwLo*10000);
    pInt->msDuration   = HnsQWORDtoMsDWORD(qwSendDuration);
    pInt->msPreroll    = qwPreroll.dwLo;
    pInt->dwFilePropertiesFlags = dwFlags;

    /* use all */
    pInt->currPacketOffset += cbSize;

    return WMAERR_OK;
}
//#endif

#if !defined (COPY_KSDATAFORMAT_SUBTYPE_PCM)
#define COPY_KSDATAFORMAT_SUBTYPE_PCM(guid)\
     (guid)->Data1       = 0x01;\
     (guid)->Data2       = 0x00;\
     (guid)->Data3       = 0x10;\
     (guid)->Data4[0] = 0x80;\
     (guid)->Data4[1] = 0x00;\
     (guid)->Data4[2] = 0x00;\
     (guid)->Data4[3] = 0xaa;\
     (guid)->Data4[4] = 0x00;\
     (guid)->Data4[5] = 0x38;\
     (guid)->Data4[6] = 0x9b;\
     (guid)->Data4[7] = 0x71;
#endif // COPY_KSDATAFORMAT_SUBTYPE_PCM

/****************************************************************************/

WMAERR
WMA_LoadAudioObject(tWMAFileHdrStateInternal *pInt,
                    DWORD cbSize,
                    int isFull)
{
    GUID streamType;
    GUID ecStrategy;
    QWORD qwOffset;
    DWORD cbTypeSpecific = 0;
    DWORD cbErrConcealment = 0;
    WORD wStreamNum = 0;
    DWORD dwJunk = 0;
    DWORD nBlocksPerObject = 0;
    AsfXAcmAudioErrorMaskingData *pScramblingData = NULL;
    // WAVEFORMATEX *pFmt;

    BYTE *pData = NULL;
    DWORD cbActual = 0;
    DWORD cbWanted = 0;

    WORD  tw;
    DWORD tdw;
    const BYTE *tp;


    DWORD cbObjectOffset = 0;



    if (pInt == NULL)
    {
        return WMAERR_INVALIDARG;
    }

    cbSize -= MIN_OBJECT_SIZE;


    cbWanted = 2 * sizeof(GUID) + sizeof(QWORD) + 3 * sizeof(DWORD) + sizeof(WORD);
    if (cbObjectOffset + cbWanted > cbSize)
    {
        return WMAERR_BUFFERTOOSMALL;
    }
    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                pInt->currPacketOffset + cbObjectOffset, cbWanted, &pData);
    if (cbActual != cbWanted)
    {
        return WMAERR_BUFFERTOOSMALL;
    }


    cbObjectOffset += cbActual;

    LoadGUID(streamType, pData);
    LoadGUID(ecStrategy, pData);
    LoadQWORD(qwOffset, pData);
    LoadDWORD(cbTypeSpecific, pData);
    LoadDWORD(cbErrConcealment, pData);
    LoadWORD(wStreamNum, pData);
    LoadDWORD(dwJunk, pData);

    wStreamNum &= 0x7F;
     if( WMA_IsEqualGUID( &CLSID_AsfXStreamTypeIcmVideo, &streamType ) )
        {
          wma_DEBUG("WMV");
          return WMAERR_FAIL;
        }
#ifdef WMA_TABLE_ROOM_VERIFY
    if (!WMA_IsEqualGUID(p_CLSID_AsfXStreamTypeAcmAudio, &streamType))
#else
    if (!WMA_IsEqualGUID(&CLSID_AsfXStreamTypeAcmAudio, &streamType))
#endif
    {

        /* Skip over the rest */
        pInt->currPacketOffset += cbSize;
        return WMAERR_OK;
    }

    pInt->cAudioStreams++;

    if (pInt->cAudioStreams != pInt->wTargetAudioStreamNumber)
    {
        // We dont want to decode this stream
        /* Skip over the rest */
        pInt->currPacketOffset += cbSize;
        return WMAERR_OK;
    }

    /* Type specific */
    pInt->wAudioStreamId = wStreamNum; //Amit

    if (cbTypeSpecific > 0)
    {
        WORD cbSizeWavHeader = 0;

        cbWanted = cbTypeSpecific;
        if (cbObjectOffset + cbWanted > cbSize)
        {
            return WMAERR_BUFFERTOOSMALL;
        }
        cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                    pInt->currPacketOffset + cbObjectOffset, cbWanted, &pData);
        if (cbActual != cbWanted)
        {
            return WMAERR_BUFFERTOOSMALL;
        }

        cbObjectOffset += cbActual;

        tp = pData;
        LoadWORD(tw , tp);
        pInt->wFormatTag = tw;

        switch (tw)
        {
            case WAVE_FORMAT_WMAUDIO3: // fall into
            case WAVE_FORMAT_WMAUDIO_LOSSLESS:
                if (cbTypeSpecific < 36)
                {
                    return WMAERR_FAIL;
                }

                pInt->nVersion         = 3;
                tp = pData +  4;
                LoadDWORD(tdw, tp);
                pInt->nSamplesPerSec   = tdw;
                tp = pData +  8;
                LoadDWORD(tdw, tp);
                pInt->nAvgBytesPerSec  = tdw;
                tp = pData + 12;
                LoadWORD(tw , tp);
                pInt->nBlockAlign      = tw;
                tp = pData +  2;
                LoadWORD(tw , tp);
                pInt->nChannels        = tw;

                // New information in V9
                pInt->wPCMFormatTag = WAVE_FORMAT_EXTENSIBLE;
                tp = pData + 14;
                LoadWORD(tw, tp);
                pInt->wValidBitsPerSample   = tw; // Container size

                // Round up the container size
                pInt->wBitsPerSample = 8 * ((pInt->wValidBitsPerSample + 7) / 8);

                // Digest additional information
                tp = pData + 16;
                LoadWORD(tw, tp);
                cbSizeWavHeader = tw;
                assert(cbSizeWavHeader == WMAUDIO3_WFX_EXTRA_BYTES);

                // Has V3 specific info
                // pData+18 would be reserved.
                tp = pData + 20;
                LoadDWORD(tdw, tp);
                pInt->dwChannelMask = tdw;

                // pData+24, pData+28 are reseved DWORD
                tp = pData + 32;
                LoadWORD(tw , tp);
                pInt->nEncodeOpt        = tw;
                // pData+34, is a reseved DWORD

                // Patch a GUID for file write: Should be done elsewhere
                // GUID information:
                COPY_KSDATAFORMAT_SUBTYPE_PCM(&(pInt->SubFormat));
                break;

            case WAVE_FORMAT_WMAUDIO2:

                if (cbTypeSpecific < 28 /*sizeof(WMAUDIO2WAVEFORMAT)*/)
                {
                    return WMAERR_FAIL;
                }

                pInt->nVersion         = 2;
                tp = pData +  4;
                LoadDWORD(tdw, tp);
                pInt->nSamplesPerSec   = tdw;
                tp = pData +  8;
                LoadDWORD(tdw, tp);
                pInt->nAvgBytesPerSec  = tdw;
                tp = pData + 12;
                LoadWORD(tw , tp);
                pInt->nBlockAlign      = tw;
                tp = pData +  2;
                LoadWORD(tw , tp);
                pInt->nChannels        = tw;

                // New information in V9
                pInt->wPCMFormatTag = WAVE_FORMAT_PCM;
                tp = pData + 14;
                LoadWORD(tw, tp);
                pInt->wBitsPerSample   = tw;
                pInt->wOriginalBitDepth = pInt->wValidBitsPerSample = pInt->wBitsPerSample;

                tp = pData + 18;
                LoadDWORD(tdw, tp);
                pInt->nSamplesPerBlock = tdw;
                tp = pData + 22;
                LoadWORD(tw , tp);
                pInt->nEncodeOpt       = tw;

                switch (pInt->nChannels)
                {
                    case 1:
                        pInt->dwChannelMask = SPEAKER_FRONT_CENTER;
                        break;
                    case 2:
                        pInt->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
                        break;
                    case 6:
                        // Only to support pseudo V3 streams
                        pInt->dwChannelMask = (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT |
                                               SPEAKER_FRONT_CENTER | SPEAKER_BACK_LEFT |
                                               SPEAKER_BACK_RIGHT | SPEAKER_LOW_FREQUENCY);
                        break;
                    default:
                        // "Can not deal with % channels in this format\n"
                        return WMAERR_FAIL;
                }
                break;

            case WAVE_FORMAT_MSAUDIO1:
                if (cbTypeSpecific < 22 /*sizeof(MSAUDIO1WAVEFORMAT)*/)
                {
                    return WMAERR_FAIL;
                }

                pInt->nVersion         = 1;
                tp = pData +  4;
                LoadDWORD(tdw, tp);
                pInt->nSamplesPerSec   = tdw;
                tp = pData +  8;
                LoadDWORD(tdw, tp);
                pInt->nAvgBytesPerSec  = tdw;
                tp = pData + 12;
                LoadWORD(tw , tp);
                pInt->nBlockAlign      = tw;
                tp = pData +  2;
                LoadWORD(tw , tp);
                pInt->nChannels        = tw;
                tp = pData + 20;
                LoadWORD(tw, tp);
                pInt->nEncodeOpt       = tw;
                tp = pData + 18;
                LoadWORD(tw, tp);
                pInt->nSamplesPerBlock = tw;

                // New information in V9
                pInt->wPCMFormatTag = WAVE_FORMAT_PCM;
                tp = pData + 14;
                LoadWORD(tw, tp);
                pInt->wBitsPerSample   = tw;
                pInt->wOriginalBitDepth = pInt->wValidBitsPerSample = pInt->wBitsPerSample;

                switch (pInt->nChannels)
                {
                    case 1:
                        pInt->dwChannelMask = SPEAKER_FRONT_CENTER;
                        break;
                    case 2:
                        pInt->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
                        break;
                    default:
                        // "Can not deal with % channels in this format\n"
                        return WMAERR_FAIL;
                }

                break;
            default:
                // unknown...
                return WMAERR_FAIL;
        }
    }

    /* Error concealment - this can get as big as 400!!! */

    if (cbErrConcealment > 0)
    {
#ifdef WMA_TABLE_ROOM_VERIFY
        if (WMA_IsEqualGUID(p_CLSID_AsfXSignatureAudioErrorMaskingStrategy, &ecStrategy))
#else
        if (WMA_IsEqualGUID(&CLSID_AsfXSignatureAudioErrorMaskingStrategy, &ecStrategy))
#endif
        {
            cbWanted = 9;//sizeof(AsfXSignatureAudioErrorMaskingData); Amit it is giving 12 bytes while actual is 9 09/10/2001
        }
#ifdef WMA_TABLE_ROOM_VERIFY
        else if (WMA_IsEqualGUID(p_CLSID_AsfXAcmAudioErrorMaskingStrategy, &ecStrategy))
#else
        else if (WMA_IsEqualGUID(&CLSID_AsfXAcmAudioErrorMaskingStrategy, &ecStrategy))
#endif
        {
//            cbWanted = sizeof(AsfXAcmAudioErrorMaskingData);
            cbWanted = 8;
        }
        else
        {
            return WMAERR_FAIL;
        }

        if (cbObjectOffset + cbWanted > cbSize)
        {
            return WMAERR_BUFFERTOOSMALL;
        }
        cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                    pInt->currPacketOffset + cbObjectOffset, cbWanted, &pData);
        if (cbActual != cbWanted)
        {
            return WMAERR_BUFFERTOOSMALL;
        }

#if (defined(WIN32) || defined(_WIN32_WINCE) )
        if (IsBadReadPtr(pData, cbActual))
        {
            return WMAERR_BUFFERTOOSMALL;
        }
#endif



        cbObjectOffset += cbErrConcealment; // Skip over the rest - cbActual;

#ifdef WMA_TABLE_ROOM_VERIFY
        if (WMA_IsEqualGUID(p_CLSID_AsfXSignatureAudioErrorMaskingStrategy, &ecStrategy))
#else
        if (WMA_IsEqualGUID(&CLSID_AsfXSignatureAudioErrorMaskingStrategy, &ecStrategy))
#endif
        {
            pInt->cbAudioSize = ((AsfXSignatureAudioErrorMaskingData *)pData)->maxObjectSize;


        }
#ifdef WMA_TABLE_ROOM_VERIFY
        else if (WMA_IsEqualGUID(p_CLSID_AsfXAcmAudioErrorMaskingStrategy, &ecStrategy))
#else
        else if (WMA_IsEqualGUID(&CLSID_AsfXAcmAudioErrorMaskingStrategy, &ecStrategy))
#endif
        {
            pScramblingData = (AsfXAcmAudioErrorMaskingData *)pData;

#ifdef BIG_ENDIAN
            // byte-swap the struct
            ByteSwapAsfXAcmAudioErrorMaskingData(*pScramblingData);
#endif


//            pInt->cbAudioSize = (DWORD)(pScramblingData->virtualPacketLen*pScramblingData->span);
//            pInt->cbAudioSize = (DWORD)(((WORD)(pData+1)) * ((WORD)(*pData)));
            //pInt->cbAudioSize = (DWORD)((*(WORD*)(pData+1)) * ((WORD)(*pData)));
#ifdef WMDRM_NETWORK
            //
            // I believe the non-WMDRM-ND version of this variable is
            // calculated incorrectly. Correct it.
            //
            {
                BYTE *pTempData = pData;
                BYTE bSpan;
                WORD wPktLen;

                LoadBYTE(bSpan, pTempData);
                LoadWORD(wPktLen, pTempData);
                pInt->cbAudioSize = (DWORD)(wPktLen * (WORD) bSpan);
            }
#else
            pInt->cbAudioSize = (DWORD)(((WORD)(*(pData + 1))) * ((WORD)(*pData)));
#endif
            if (pScramblingData->span > 1)
                return WMAERR_FAIL;

//#ifdef BIG_ENDIAN
//            // byte-swap the object size
//            SWAP_DWORD(pInt->cbAudioSize);
//#endif
        }
        else
        {
            return WMAERR_FAIL;
        }
    }

    if (pInt->nBlockAlign > 0)
        nBlocksPerObject = pInt->cbAudioSize / pInt->nBlockAlign;
#ifdef WMDRM_NETWORK
    //
    // Same as above.
    //
    //    pInt->cbAudioSize = nBlocksPerObject*pInt->nSamplesPerBlock*pInt->nChannels*2;
#else
    pInt->cbAudioSize = nBlocksPerObject * pInt->nSamplesPerBlock * pInt->nChannels * 2;
#endif

    /* use all */
    pInt->currPacketOffset += cbSize;

    return WMAERR_OK;
}
/****************************************************************************/
WMAERR
WMA_LoadEncryptionObject(tWMAFileHdrStateInternal *pInt,
                         DWORD cbSize)
{
    DWORD cbBlock = 0;

    BYTE *pData = NULL;
    DWORD cbActual = 0;
    DWORD cbWanted = 0;

    DWORD cbObjectOffset = 0;

    if (pInt == NULL)
    {
        return WMAERR_INVALIDARG;
    }

    cbSize -= MIN_OBJECT_SIZE;

    pInt->cbSecretData = 0;

    /* SecretData */

    cbWanted = sizeof(DWORD);
    if (cbObjectOffset + cbWanted > cbSize)
    {
        return WMAERR_BUFFERTOOSMALL;
    }
    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                pInt->currPacketOffset + cbObjectOffset, cbWanted, &pData);
    if (cbActual != cbWanted)
    {
        return WMAERR_BUFFERTOOSMALL;
    }
#if (defined(WIN32) || defined(_WIN32_WINCE) )
    if (IsBadReadPtr(pData, cbActual))
    {
        return WMAERR_BUFFERTOOSMALL;
    }
#endif

    cbObjectOffset += cbActual;

    LoadDWORD(pInt->cbSecretData, pData);

    if (pInt->cbSecretData > 32)
        return WMAERR_BUFFERTOOSMALL;

    if (pInt->cbSecretData)
    {
        cbWanted = pInt->cbSecretData;
        if (cbObjectOffset + cbWanted > cbSize)
        {
            return WMAERR_BUFFERTOOSMALL;
        }
        cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                    pInt->currPacketOffset + cbObjectOffset, cbWanted, &pData);
        if (cbActual != cbWanted)
        {
            return WMAERR_BUFFERTOOSMALL;
        }
//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//        if (IsBadReadPtr(pData, cbActual))
//        {
//            return WMAERR_BUFFERTOOSMALL;
//        }
//        if (IsBadWritePtr(pInt->pbSecretData, cbActual))
//        {
//            return WMAERR_BUFFERTOOSMALL;
//        }
//#endif


        cbObjectOffset += cbActual;

        if (cbActual <= 32)
            //memcpy(pInt->pbSecretData, pData, (size_t)cbActual);
            memcpy(pInt->pbSecretData, pData, cbActual);
        else
            return WMAERR_BUFFERTOOSMALL;

    }

    /* Type string */

    cbWanted = sizeof(DWORD);
    if (cbObjectOffset + cbWanted > cbSize)
    {
        return WMAERR_BUFFERTOOSMALL;
    }
    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                pInt->currPacketOffset + cbObjectOffset, cbWanted, &pData);
    if (cbActual != cbWanted)
    {
        return WMAERR_BUFFERTOOSMALL;
    }
#if (defined(WIN32) || defined(_WIN32_WINCE) )
    if (IsBadReadPtr(pData, cbActual))
    {
        return WMAERR_BUFFERTOOSMALL;
    }
#endif

    cbObjectOffset += cbActual;

    LoadDWORD(cbBlock, pData);

    if (cbBlock)
    {
        cbWanted = cbBlock;
        if (cbObjectOffset + cbWanted > cbSize)
        {
            return WMAERR_BUFFERTOOSMALL;
        }
        cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                    pInt->currPacketOffset + cbObjectOffset, cbWanted, &pData);
        if (cbActual != cbWanted)
        {
            return WMAERR_BUFFERTOOSMALL;
        }
#if (defined(WIN32) || defined(_WIN32_WINCE) )
        if (IsBadReadPtr(pData, cbActual))
        {
            return WMAERR_BUFFERTOOSMALL;
        }
        if (IsBadWritePtr(pInt->pbType, cbActual))
        {
            return WMAERR_BUFFERTOOSMALL;
        }
#endif

        cbObjectOffset += cbActual;

        if (cbActual <= 16)
            memcpy(pInt->pbType, pData, cbActual);
        else
            return WMAERR_BUFFERTOOSMALL;

    }

    /* Key ID */

    cbWanted = sizeof(DWORD);
    if (cbObjectOffset + cbWanted > cbSize)
    {
        return WMAERR_BUFFERTOOSMALL;
    }
    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                pInt->currPacketOffset + cbObjectOffset, cbWanted, &pData);
    if (cbActual != cbWanted)
    {
        return WMAERR_BUFFERTOOSMALL;
    }
#if (defined(WIN32) || defined(_WIN32_WINCE) )
    if (IsBadReadPtr(pData, cbActual))
    {
        return WMAERR_BUFFERTOOSMALL;
    }
#endif

    cbObjectOffset += cbActual;

    LoadDWORD(cbBlock, pData);

    if (cbBlock)
    {
        cbWanted = cbBlock;
        if (cbObjectOffset + cbWanted > cbSize)
        {
            return WMAERR_BUFFERTOOSMALL;
        }
        cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                    pInt->currPacketOffset + cbObjectOffset, cbWanted, &pData);
        if (cbActual != cbWanted)
        {
            return WMAERR_BUFFERTOOSMALL;
        }
#if (defined(WIN32) || defined(_WIN32_WINCE) )
        if (IsBadReadPtr(pData, cbActual))
        {
            return WMAERR_BUFFERTOOSMALL;
        }
        if (IsBadWritePtr(pInt->pbKeyID, cbActual))
        {
            return WMAERR_BUFFERTOOSMALL;
        }
#endif

        cbObjectOffset += cbActual;

        if (cbActual <= 32)
            //memcpy(pInt->pbKeyID, pData, (size_t)cbActual);
            memcpy(pInt->pbKeyID, pData, cbActual);
        else
            return WMAERR_BUFFERTOOSMALL;
    }

    /* License URL */

    cbWanted = sizeof(DWORD);
    if (cbObjectOffset + cbWanted > cbSize)
    {
        return WMAERR_BUFFERTOOSMALL;
    }
    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                pInt->currPacketOffset + cbObjectOffset, cbWanted, &pData);
    if (cbActual != cbWanted)
    {
        return WMAERR_BUFFERTOOSMALL;
    }
#if (defined(WIN32) || defined(_WIN32_WINCE) )
    if (IsBadReadPtr(pData, cbActual))
    {
        return WMAERR_BUFFERTOOSMALL;
    }
#endif

    cbObjectOffset += cbActual;

    LoadDWORD(cbBlock, pData);

    if (cbBlock)
    {
        cbWanted = cbBlock;
        if (cbObjectOffset + cbWanted > cbSize)
        {
            return WMAERR_BUFFERTOOSMALL;
        }
        while (cbWanted > 0)
        {
            cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                        pInt->currPacketOffset + cbObjectOffset, cbWanted, &pData);

            cbObjectOffset += cbActual;
            cbWanted -= cbActual;

        }

        /* ignore */
    }

    /* use all */
    pInt->currPacketOffset += cbSize;

    return WMAERR_OK;
}

/****************************************************************************/
WMAERR
WMA_LoadContentDescriptionObject(tWMAFileHdrStateInternal *pInt,
                                 DWORD cbSize)
{
    BYTE *pData = NULL;
    U64 iCurrFileOffset;
    U64 iMaxFileOffset;
    tWMAFileContDesc *pDesc;
    WMAERR wmaerr = WMAERR_OK;
    DWORD cbActual;
    DWORD cbWanted;

    assert(cbSize >= MIN_OBJECT_SIZE);  // Guaranteed by WMA_LoadObjectHeader
    iCurrFileOffset = pInt->currPacketOffset;
    iMaxFileOffset = iCurrFileOffset + cbSize - MIN_OBJECT_SIZE;

/////////skip the field parse///////////////////////
    if (NULL != pInt->m_pDesc)
        goto exit;

    pDesc = pInt->m_pDesc = &gDesc;//malloc(sizeof(tWMAFileContDesc));
    if (NULL == pInt->m_pDesc)
    {
        wmaerr = TraceResult(WMAERR_OUTOFMEMORY);
        goto exit;
    }

    pDesc = pInt->m_pDesc;
    memset(pDesc, 0, sizeof(tWMAFileContDesc));

    // Ugly. The programmer of this function assumed that tWMAFileHdrStateInternal *pInt
    // can be cast to (tHWMAFileState *), which is only true because
    // tWMAFileHdrStateInternal hdr_parse is first member in tWMAFileStateInternal.
    // Codify this assumption. Even here I am limited: if there are more than one
    // tWMAFileHdrStateInternal structures in tWMAFileStateInternal, we're lost.
    assert(offsetof(tWMAFileStateInternal, hdr_parse) == 0);

    // Check if reading this next field pushes us past end of object
    cbWanted = 5*sizeof(WORD);
    if (iCurrFileOffset + cbWanted > iMaxFileOffset)
    {
        wmaerr = TraceResult(WMAERR_CORRUPTDATA);
        goto exit;
    }

    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt, iCurrFileOffset,cbWanted, &pData);
    if(cbActual != cbWanted)
    {
        wmaerr = TraceResult(WMAERR_BUFFERTOOSMALL);
        goto exit;
    }
    iCurrFileOffset += cbActual;

    LoadWORD(pDesc->title_len, pData);
    LoadWORD(pDesc->author_len, pData);
    LoadWORD(pDesc->copyright_len, pData);
    LoadWORD(pDesc->description_len, pData);
    LoadWORD(pDesc->rating_len, pData);
    gWMAMetatdateStrBufOffset = 0;//add by evan wu,2009-4-13

    wmaerr = AllocateAndLoadMetadataStr((tHWMAFileState *)pInt, &iCurrFileOffset,iMaxFileOffset, (WORD*)&pDesc->title_len, (BYTE**)&pDesc->pTitle);
    if (WMAERR_OK != wmaerr)
    {
        TraceResult(wmaerr);
        goto exit;
    }

    wmaerr = AllocateAndLoadMetadataStr((tHWMAFileState *)pInt, &iCurrFileOffset,iMaxFileOffset, (WORD*)&pDesc->author_len, (BYTE**)&pDesc->pAuthor);
    if (WMAERR_OK != wmaerr)
    {
        TraceResult(wmaerr);
        goto exit;
    }

    wmaerr = AllocateAndLoadMetadataStr((tHWMAFileState *)pInt, &iCurrFileOffset,iMaxFileOffset, (WORD*)&pDesc->copyright_len, (BYTE**)&pDesc->pCopyright);
    if (WMAERR_OK != wmaerr)
    {
        TraceResult(wmaerr);
        goto exit;
    }


    wmaerr = AllocateAndLoadMetadataStr((tHWMAFileState *)pInt, &iCurrFileOffset,iMaxFileOffset, (WORD*)&pDesc->description_len, (BYTE**)&pDesc->pDescription);
    if (WMAERR_OK != wmaerr)
    {
        TraceResult(wmaerr);
        goto exit;
    }


    wmaerr = AllocateAndLoadMetadataStr((tHWMAFileState *)pInt, &iCurrFileOffset,iMaxFileOffset, (WORD*)&pDesc->rating_len, (BYTE**)&pDesc->pRating);
    if (WMAERR_OK != wmaerr)
    {
        TraceResult(wmaerr);
        goto exit;
    }

    // Confirm that we have consumed the entire object EXACTLY
    assert(iCurrFileOffset == iMaxFileOffset);

exit:
    // Deal with errors in a "transactional" fashion: either succeed completely,
    // or fail completely. In other words, clean up if we encountered any errors.
#if 0
    if (WMAERR_OK != wmaerr)
    {
        if (pInt->m_pDesc)
        {
            pDesc = pInt->m_pDesc;

            // it's OK to pass NULL pointers to free
            free(pDesc->pTitle);
            free(pDesc->pAuthor);
            free(pDesc->pCopyright);
            free(pDesc->pDescription);
            free(pDesc->pRating);

            free(pInt->m_pDesc);
            pInt->m_pDesc = NULL;
        }
    }
#endif

    // Advance to end of object
    pInt->currPacketOffset = iMaxFileOffset;

    return wmaerr;
}

/****************************************************************************/
WMAERR
WMA_LoadExtendedContentDescObject(tWMAFileHdrStateInternal *pInt,
                                  DWORD cbSize)
{
    BYTE *pData = NULL;
    DWORD cbActual;
    DWORD cbWanted;
    tWMAExtendedContentDesc *pECDesc = NULL;
    tWMA_U16 cDescriptors = 0;
    U64 iCurrFileOffset;
    U64 iMaxFileOffset;
    WMAERR wmaerr = WMAERR_OK;
//    DWORD i;

    assert(cbSize >= MIN_OBJECT_SIZE);  // Guaranteed by WMA_LoadObjectHeader
    iCurrFileOffset = pInt->currPacketOffset;
    iMaxFileOffset = iCurrFileOffset + cbSize - MIN_OBJECT_SIZE;
    cbWanted = cbSize;
    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt, iCurrFileOffset,
                                cbWanted, &pData);

    //if(cbActual != cbWanted)
    //{
    //    wmaerr = TraceResult(WMAERR_BUFFERTOOSMALL);
    //    goto exit;
    //}
    iCurrFileOffset += cbActual;
//////////skip content descriptor parse////////////////////
//    if (pInt->m_pECDesc)
//        goto exit;
//
//    pInt->m_pECDesc = &gECDesc;//malloc(sizeof(tWMAExtendedContentDesc));
//    if( pInt->m_pECDesc == NULL)
//    {
//        wmaerr = TraceResult(WMAERR_OUTOFMEMORY);
//        goto exit;
//    }
//    pECDesc = pInt->m_pECDesc;
//    memset(pECDesc, 0, sizeof(tWMAExtendedContentDesc));
//
//    // Get descriptor count
//    cbWanted = sizeof(tWMA_U16);
//    if (iCurrFileOffset + cbWanted > iMaxFileOffset)
//    {
//        wmaerr = TraceResult(WMAERR_CORRUPTDATA);
//        goto exit;
//    }
//    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt, iCurrFileOffset,
//        cbWanted, &pData);
//    if(cbActual != cbWanted)
//    {
//        wmaerr = TraceResult(WMAERR_BUFFERTOOSMALL);
//        goto exit;
//    }
//    iCurrFileOffset += cbActual;
//    LoadWORD(cDescriptors, pData);
//    pECDesc->cDescriptors = cDescriptors;
//
//    // Allocate array to hold pointers to each descriptor record
//    pECDesc->pDescriptors = (ECD_DESCRIPTOR *)gDescript;//malloc(cDescriptors * sizeof(ECD_DESCRIPTOR));
//    if(pECDesc->pDescriptors == NULL)
//    {
//       wmaerr = TraceResult(WMAERR_OUTOFMEMORY);
//       goto exit;
//    }
//    memset( pECDesc->pDescriptors, 0, cDescriptors * sizeof(ECD_DESCRIPTOR));
//
//    // Read in each descriptor record
//    for (i = 0; i < cDescriptors; i++)
//    {
//        // Load in descriptor name length
//        cbWanted = sizeof(tWMA_U16);
//        if (iCurrFileOffset + cbWanted > iMaxFileOffset)
//        {
//            wmaerr = TraceResult(WMAERR_CORRUPTDATA);
//            goto exit;
//        }
//        cbActual = WMAFileCBGetData((tHWMAFileState *)pInt, iCurrFileOffset,
//            cbWanted, &pData);
//        if(cbActual != cbWanted)
//        {
//            wmaerr = TraceResult(WMAERR_BUFFERTOOSMALL);
//            goto exit;
//        }
//        iCurrFileOffset += cbActual;
//        LoadWORD(pECDesc->pDescriptors[i].cbName, pData);
//
//        // Load in descriptor name
//        wmaerr = AllocateAndLoadMetadataStr((tHWMAFileState *)pInt, &iCurrFileOffset,
//            iMaxFileOffset, &pECDesc->pDescriptors[i].cbName,
//            (BYTE **)&pECDesc->pDescriptors[i].pwszName);
//        if (WMAERR_OK != wmaerr)
//        {
//            TraceResult(wmaerr);
//            goto exit;
//        }
//
//#ifndef LITTLE_ENDIAN
//        SwapWstr(pECDesc->pDescriptors[i].pwszName,
//            pECDesc->pDescriptors[i].cbName / sizeof(tWMA_U16));
//#endif
//
//        // Load in descriptor value type, and descriptor value length
//        cbWanted = 2 * sizeof(tWMA_U16);
//        if (iCurrFileOffset + cbWanted > iMaxFileOffset)
//        {
//            wmaerr = TraceResult(WMAERR_CORRUPTDATA);
//            goto exit;
//        }
//        cbActual = WMAFileCBGetData((tHWMAFileState *)pInt, iCurrFileOffset,
//            cbWanted, &pData);
//        if(cbActual != cbWanted)
//        {
//            wmaerr = TraceResult(WMAERR_BUFFERTOOSMALL);
//            goto exit;
//        }
//        iCurrFileOffset += cbActual;
//        LoadWORD(pECDesc->pDescriptors[i].data_type, pData);
//        LoadWORD(pECDesc->pDescriptors[i].cbValue, pData);
//
//        // Load in descriptor value
//        if (ECD_STRING == pECDesc->pDescriptors[i].data_type)
//        {
//            // We will truncate this string if necessary
//            wmaerr = AllocateAndLoadMetadataStr((tHWMAFileState *)pInt,
//                &iCurrFileOffset, iMaxFileOffset, &pECDesc->pDescriptors[i].cbValue,
//                &pECDesc->pDescriptors[i].uValue.pbBinary);
//            if (WMAERR_OK != wmaerr)
//            {
//                TraceResult(wmaerr);
//                goto exit;
//            }
//
//#ifndef LITTLE_ENDIAN
//            SwapWstr(pECDesc->pDescriptors[i].uValue.pbBinary,
//                pECDesc->pDescriptors[i].cbValue / sizeof(tWMA_U16));
//#endif // LITTLE_ENDIAN
//        }
//        else
//        {
//            U32 cbBytesRead;
//            const U32 cbBytesToRead = pECDesc->pDescriptors[i].cbValue;
//
//            // Do not truncate
//            // Check if reading this object takes us past end of object
//            if (iCurrFileOffset + pECDesc->pDescriptors[i].cbValue > iMaxFileOffset)
//            {
//                wmaerr = TraceResult(WMAERR_CORRUPTDATA);
//                goto exit;
//            }
//
//            pECDesc->pDescriptors[i].uValue.pbBinary =
//                malloc(pECDesc->pDescriptors[i].cbValue);
//            if (NULL == pECDesc->pDescriptors[i].uValue.pbBinary)
//            {
//                wmaerr = TraceResult(WMAERR_OUTOFMEMORY);
//                goto exit;
//            }
//
//            for (cbBytesRead = 0; cbBytesRead < cbBytesToRead; )
//            {
//                cbWanted = MIN(MAX_BUFSIZE, (cbBytesToRead - cbBytesRead));
//                cbActual = WMAFileCBGetData(pInt, iCurrFileOffset + cbBytesRead,
//                    cbWanted, &pData);
//                if (cbActual != cbWanted)
//                {
//                    //TraceInfo2("WMA_LoadExtendedContentDescObject requested %d "
//                    //    "from WMAFileCBGetData, got %d!", cbWanted, cbActual);
//                    wmaerr = TraceResult(WMAERR_BUFFERTOOSMALL);
//                    goto exit;
//                }
//
//                memcpy(&pECDesc->pDescriptors[i].uValue.pbBinary[cbBytesRead],
//                    pData, cbActual);
//                cbBytesRead += cbActual;
//            }
//
//            if (cbBytesRead != cbBytesToRead)
//            {
//                wmaerr = TraceResult(WMAERR_BUFFERTOOSMALL);
//                goto exit;
//            }
//
//            iCurrFileOffset += cbBytesRead;
//
//#ifndef LITTLE_ENDIAN
//            switch (pECDesc->pDescriptors[i].data_type)
//            {
//                case ECD_BOOL:
//                case ECD_DWORD:
//                    SWAPDWORD(pECDesc->pDescriptors[i].uValue.pbBinary);
//                    break;
//
//                case ECD_QWORD:
//                    SWAPBYTES(pECDesc->pDescriptors[i].uValue.pbBinary, 8);
//                    break;
//
//                case ECD_WORD:
//                    SWAPWORD(pECDesc->pDescriptors[i].uValue.pbBinary);
//                    break;
//            } // switch
//#endif // LITTLE_ENDIAN
//
//        } // else (NOT ECD_STRING)
//
//    } // for
//
//exit:
//    // Deal with errors in a "transactional" fashion: either succeed completely,
//    // or fail completely. In other words, clean up if we encountered any errors.
//    if (WMAERR_OK != wmaerr)
//    {
//        if (pInt->m_pECDesc)
//        {
//            pECDesc = pInt->m_pECDesc;
//            if (pECDesc->pDescriptors)
//            {
//                for (i = 0; i < pECDesc->cDescriptors; i++)
//                {
//                    // It's OK to pass NULL pointers to free
//                    free(pECDesc->pDescriptors[i].uValue.pbBinary);
//                }
//
//                free(pECDesc->pDescriptors);
//                pECDesc->pDescriptors = NULL;
//            }
//
//            free(pInt->m_pECDesc);
//            pInt->m_pECDesc = NULL;
//        }
//    }


    // Skip to end of object
    pInt->currPacketOffset = iMaxFileOffset;

    return wmaerr;
}


/****************************************************************************/
#ifndef WMAAPI_NO_DRM
WMAERR
WMA_LoadLicenseStoreObject(tWMAFileHdrStateInternal *pInt,
                                 DWORD cbSize)
{
    BYTE *pData = NULL;
    DWORD cbActual =0;
    DWORD cbWanted =0;
  //  DWORD cbDone=0;
    DWORD cbOffset=0;
    DWORD cbWanted1=0;


    DWORD m_dwFlag=0;

    DWORD cbObjectOffset = 0;

    if(pInt == NULL)
    {
        return WMAERR_INVALIDARG;
    }

    cbSize -= MIN_OBJECT_SIZE;

    cbWanted = 8;
    if(cbObjectOffset + cbWanted > cbSize)
    {
        return WMAERR_BUFFERTOOSMALL;
    }
    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                pInt->currPacketOffset + cbObjectOffset, cbWanted, &pData);
    if(cbActual != cbWanted)
    {
        return WMAERR_BUFFERTOOSMALL;
    }
#if (defined(WIN32) || defined(_WIN32_WINCE) )
    if (IsBadReadPtr(pData, cbActual))
    {
        return WMAERR_BUFFERTOOSMALL;
    }
#endif

    cbObjectOffset += cbActual;

    LoadDWORD(m_dwFlag, pData);
    LoadDWORD(pInt->m_dwLicenseLen, pData);

    cbWanted = pInt->m_dwLicenseLen;
    if(cbObjectOffset + cbWanted > cbSize)
    {
        return WMAERR_NOTDONE;
    }
    if(cbWanted > MAX_LIC_DATA_LEN)
    {
        //wma_DEBUG("size of licence data is too large\n");
        return WMAERR_OUTOFMEMORY;
    }
    pInt->m_pLicData = (BYTE*)gm_pLicData;//malloc(cbWanted);
    if (NULL ==  pInt->m_pLicData)
        return WMAERR_OUTOFMEMORY;

//    cbDone = 0;

    if(cbWanted > MAX_BUFSIZE)
    {
        cbOffset=0;
        do
        {
            cbWanted1 =  cbWanted > MAX_BUFSIZE ? MAX_BUFSIZE : cbWanted;
            cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                        pInt->currPacketOffset + cbObjectOffset, cbWanted1, &pData);
            if(cbActual != cbWanted1)
            {
                return WMAERR_BUFFERTOOSMALL;
            }
#if (defined(WIN32) || defined(_WIN32_WINCE) )
            if (IsBadReadPtr(pData, cbActual))
            {
                return WMAERR_BUFFERTOOSMALL;
            }
            if (IsBadWritePtr(pInt->m_pLicData+cbOffset, cbActual))
            {
                return WMAERR_BUFFERTOOSMALL;
            }
#endif

            cbObjectOffset += cbActual;
            cbWanted -=cbActual;
            if (cbOffset+cbActual  <= (DWORD)pInt->m_dwLicenseLen)
                memcpy(pInt->m_pLicData+cbOffset, pData, cbActual);
            else
                return WMAERR_BUFFERTOOSMALL;

            cbOffset +=cbActual;
        }while(cbWanted >0);
    }
    else
    {
        cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                    pInt->currPacketOffset + cbObjectOffset, cbWanted, &pData);
        if(cbActual != cbWanted)
        {
            return WMAERR_BUFFERTOOSMALL;
        }
#if (defined(WIN32) || defined(_WIN32_WINCE) )
        if (IsBadReadPtr(pData, cbActual))
        {
            return WMAERR_BUFFERTOOSMALL;
        }
        if (IsBadWritePtr(pInt->m_pLicData, cbActual))
        {
            return WMAERR_BUFFERTOOSMALL;
        }
#endif

        cbObjectOffset += cbActual;
        memcpy(pInt->m_pLicData, pData, cbActual);
    }


/*    while (cbWanted)
    {
        cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                    pInt->currPacketOffset + cbObjectOffset, cbWanted, &pData);

        if(cbActual != cbWanted)
        {
            return WMAERR_BUFFERTOOSMALL;
        }
#if (defined(WIN32) || defined(_WIN32_WINCE) )
        if (IsBadReadPtr(pData, cbActual))
        {
            return WMAERR_BUFFERTOOSMALL;
        }
#endif

        if (cbDone+cbActual <= pInt->m_dwLicenseLen)
            memcpy(pInt->m_pLicData + cbDone, pData, cbActual);
        else
            return WMAERR_BUFFERTOOSMALL;

        cbObjectOffset += cbActual;
        cbWanted -= cbActual;
        cbDone   += cbActual;
        if(cbActual == 0)
        {
            return WMAERR_FAIL;
        }
    }
*/
//    pInt->cbCDOffset = pInt->currPacketOffset + cbObjectOffset;

    /* use all */
    pInt->currPacketOffset += cbSize;

    return WMAERR_OK;
}

#else
WMAERR
WMA_LoadLicenseStoreObject(tWMAFileHdrStateInternal *pInt,
                           DWORD cbSize)
{
    BYTE *pData = NULL;
    DWORD cbActual = 0;
    DWORD cbWanted = 0;
    //  DWORD cbDone=0;
    DWORD cbOffset = 0;
    DWORD cbWanted1 = 0;


    DWORD m_dwFlag = 0;

    DWORD cbObjectOffset = 0;

    if (pInt == NULL)
    {
        return WMAERR_INVALIDARG;
    }

    cbWanted = cbSize;

    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt, pInt->currPacketOffset + cbObjectOffset,
                                cbWanted, &pData);
    if (cbActual != cbWanted)
    {
        //wmaerr = TraceResult(WMAERR_BUFFERTOOSMALL);
        return WMAERR_FAIL;//goto exit;
    }
    cbObjectOffset += cbActual;

//    cbSize -= MIN_OBJECT_SIZE;
//
//    cbWanted = 8;
//    if(cbObjectOffset + cbWanted > cbSize)
//    {
//        return WMAERR_BUFFERTOOSMALL;
//    }
//    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
//                                pInt->currPacketOffset + cbObjectOffset, cbWanted, &pData);
//    if(cbActual != cbWanted)
//    {
//        return WMAERR_BUFFERTOOSMALL;
//    }
//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//    if (IsBadReadPtr(pData, cbActual))
//    {
//        return WMAERR_BUFFERTOOSMALL;
//    }
//#endif
//
//    cbObjectOffset += cbActual;
//
//    LoadDWORD(m_dwFlag, pData);
//    LoadDWORD(pInt->m_dwLicenseLen, pData);
//
//    cbWanted = pInt->m_dwLicenseLen;
//    if(cbObjectOffset + cbWanted > cbSize)
//    {
//        return WMAERR_NOTDONE;
//    }
//    pInt->m_pLicData = malloc(cbWanted);
//    if (NULL ==  pInt->m_pLicData)
//        return WMAERR_OUTOFMEMORY;
//
////    cbDone = 0;
//
//    if(cbWanted > MAX_BUFSIZE)
//    {
//        cbOffset=0;
//        do
//        {
//            cbWanted1 =  cbWanted > MAX_BUFSIZE ? MAX_BUFSIZE : cbWanted;
//            cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
//                                        pInt->currPacketOffset + cbObjectOffset, cbWanted1, &pData);
//            if(cbActual != cbWanted1)
//            {
//                return WMAERR_BUFFERTOOSMALL;
//            }
//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//            if (IsBadReadPtr(pData, cbActual))
//            {
//                return WMAERR_BUFFERTOOSMALL;
//            }
//            if (IsBadWritePtr(pInt->m_pLicData+cbOffset, cbActual))
//            {
//                return WMAERR_BUFFERTOOSMALL;
//            }
//#endif
//
//            cbObjectOffset += cbActual;
//            cbWanted -=cbActual;
//            if (cbOffset+cbActual  <= (DWORD)pInt->m_dwLicenseLen)
//                memcpy(pInt->m_pLicData+cbOffset, pData, cbActual);
//            else
//                return WMAERR_BUFFERTOOSMALL;
//
//            cbOffset +=cbActual;
//        }while(cbWanted >0);
//    }
//    else
//    {
//        cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
//                                    pInt->currPacketOffset + cbObjectOffset, cbWanted, &pData);
//        if(cbActual != cbWanted)
//        {
//            return WMAERR_BUFFERTOOSMALL;
//        }
//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//        if (IsBadReadPtr(pData, cbActual))
//        {
//            return WMAERR_BUFFERTOOSMALL;
//        }
//        if (IsBadWritePtr(pInt->m_pLicData, cbActual))
//        {
//            return WMAERR_BUFFERTOOSMALL;
//        }
//#endif
//
//        cbObjectOffset += cbActual;
//        memcpy(pInt->m_pLicData, pData, cbActual);
//    }
//
//
///*    while (cbWanted)
//    {
//        cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
//                                    pInt->currPacketOffset + cbObjectOffset, cbWanted, &pData);
//
//        if(cbActual != cbWanted)
//        {
//            return WMAERR_BUFFERTOOSMALL;
//        }
//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//        if (IsBadReadPtr(pData, cbActual))
//        {
//            return WMAERR_BUFFERTOOSMALL;
//        }
//#endif
//
//        if (cbDone+cbActual <= pInt->m_dwLicenseLen)
//            memcpy(pInt->m_pLicData + cbDone, pData, cbActual);
//        else
//            return WMAERR_BUFFERTOOSMALL;
//
//        cbObjectOffset += cbActual;
//        cbWanted -= cbActual;
//        cbDone   += cbActual;
//        if(cbActual == 0)
//        {
//            return WMAERR_FAIL;
//        }
//    }
//*/
////    pInt->cbCDOffset = pInt->currPacketOffset + cbObjectOffset;

    /* use all */
    pInt->currPacketOffset += cbSize;

    return WMAERR_OK;
}
#endif


/****************************************************************************/
//WMAERR  WMA_LoadMarkerObject(tWMAFileHdrStateInternal *pInt, DWORD cbSize, int iIndex)
//{
//    BYTE *pData = NULL;
//    DWORD cbActual = 0;
//    DWORD cbWanted = 0;
//    DWORD cbOffset = 0;
//    DWORD cbWanted1 = 0;
//
//    DWORD cbObjectOffset = 0;
//
////    GUID    m_gMarkerStrategy;
//    WORD    m_wAlignment = 0;
//    WORD    m_wNameLen = 0;
//    WORD    tw = 0;
//    unsigned int i = 0, j = 0;
//    BYTE *pTempData = NULL;
//
//
//    if (pInt == NULL)
//    {
//        return WMAERR_INVALIDARG;
//    }
//
//    cbWanted = cbSize;
//
//    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt, pInt->currPacketOffset + cbObjectOffset,
//                                cbWanted, &pData);
//    if (cbActual != cbWanted)
//    {
//        // wmaerr = TraceResult(WMAERR_BUFFERTOOSMALL);
//        return WMAERR_FAIL;//goto exit;
//    }
//    cbObjectOffset += cbActual;
/////////skip the field parse//////////////
//    cbSize -= MIN_OBJECT_SIZE;
//
//    cbWanted = 24;
//    if(cbObjectOffset + cbWanted > cbSize)
//    {
//        return WMAERR_BUFFERTOOSMALL;
//    }
//    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
//                                pInt->currPacketOffset + cbObjectOffset, cbWanted, &pData);
//    if(cbActual != cbWanted)
//    {
//        return WMAERR_BUFFERTOOSMALL;
//    }
//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//    if (IsBadReadPtr(pData, cbActual))
//    {
//        return WMAERR_BUFFERTOOSMALL;
//    }
//#endif
//
//
//    cbObjectOffset += cbActual;
//
//    LoadGUID (m_gMarkerStrategy, pData);
//    LoadDWORD(pInt->m_dwMarkerNum, pData);
//    LoadWORD (m_wAlignment, pData);
//    LoadWORD (m_wNameLen, pData);
//
//    //pInt->m_pMarkers = (MarkerEntry *) malloc(sizeof(MarkerEntry)*pInt->m_dwMarkerNum);
//
////NQF+
//    if( pInt->m_dwMarkerNum == 0)
//    {
//        /* use all */
//        pInt->currPacketOffset += cbSize;
//        return WMAERR_OK;
//
//    } else if ( iIndex < 0 ) {  //for query number of Markers
//
//        /* use all */
//        pInt->currPacketOffset += cbSize;
//        return WMAERR_OK;
//
//    } else if ( iIndex >= (int) pInt -> m_dwMarkerNum) {
//        /* use all */
//        pInt->currPacketOffset += cbSize;
//        return WMAERR_BUFFERTOOSMALL;  //NQF_temp
//    }
////NQF-
//
//    for (j = 0; j <= (unsigned int) iIndex; j++)
//    {
//        cbWanted = 18;
//        if(cbObjectOffset + cbWanted > cbSize)
//        {
//            return WMAERR_BUFFERTOOSMALL;
//        }
//        cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
//                                    pInt->currPacketOffset + cbObjectOffset, cbWanted, &pData);
//        if(cbActual != cbWanted)
//        {
//            return WMAERR_BUFFERTOOSMALL;
//        }
//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//        if (IsBadReadPtr(pData, cbActual))
//        {
//            return WMAERR_BUFFERTOOSMALL;
//        }
//#endif
//        cbObjectOffset += cbActual;
//
//        LoadQWORD(pInt->m_pMarker->m_qOffset, pData);
//        LoadQWORD(pInt->m_pMarker->m_qtime, pData);
//        LoadWORD (pInt->m_pMarker->m_wEntryLen, pData);
//
//        cbWanted = pInt->m_pMarker->m_wEntryLen;
//        if(cbObjectOffset + cbWanted > cbSize)
//            return WMAERR_BUFFERTOOSMALL;
//
//        if(pInt->m_pMarker->m_wEntryLen < 3*sizeof(DWORD))
//            return WMAERR_BUFFERTOOSMALL;
//
//        cbWanted = 3*sizeof(DWORD);
//
//
//        cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
//                                    pInt->currPacketOffset + cbObjectOffset, cbWanted, &pData);
//        if(cbActual != cbWanted)
//        {
//            return WMAERR_BUFFERTOOSMALL;
//        }
//        cbObjectOffset += cbActual;
//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//        if (IsBadReadPtr(pData, cbActual))
//        {
//            return WMAERR_BUFFERTOOSMALL;
//        }
//#endif
//
//        LoadDWORD(pInt->m_pMarker->m_dwSendTime, pData);
//        LoadDWORD(pInt->m_pMarker->m_dwFlags, pData);
//        LoadDWORD(pInt->m_pMarker->m_dwDescLen, pData);
//
//        if (pInt->m_pMarker->m_wEntryLen < (WORD)(3*sizeof(DWORD) + sizeof(tWMA_U16)*pInt->m_pMarker->m_dwDescLen))
//            return WMAERR_BUFFERTOOSMALL;
//
//
//
//        cbWanted = pInt->m_pMarker->m_wEntryLen - 3*sizeof(DWORD);
//
//        if (cbWanted > 0)
//        {
//            if(cbWanted > MAX_BUFSIZE)
//            {
//                cbOffset=0;
//                pTempData = malloc(cbWanted);
//                if (NULL == pTempData)
//                    return( WMAERR_OUTOFMEMORY );
//                memset(pTempData, 0, cbWanted);
//
//                do
//                {
//                    cbWanted1 =  cbWanted > MAX_BUFSIZE ? MAX_BUFSIZE : cbWanted;
//                    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
//                                                pInt->currPacketOffset + cbObjectOffset, cbWanted1, &pData);
//                    if(cbActual != cbWanted1)
//                    {
//                        free(pTempData);
//                        return WMAERR_BUFFERTOOSMALL;
//                    }
//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//                    if (IsBadReadPtr(pData, cbActual))
//                    {
//                        return WMAERR_BUFFERTOOSMALL;
//                    }
//                    if (IsBadWritePtr(pTempData+cbOffset, cbActual))
//                    {
//                        return WMAERR_BUFFERTOOSMALL;
//                    }
//#endif
//
//                    cbObjectOffset += cbActual;
//                    cbWanted -=cbActual;
//                    if (cbOffset+cbActual  <= (DWORD)(sizeof(tWMA_U16)*pInt->m_pMarker->m_dwDescLen))
//                        memcpy(pTempData+cbOffset, pData, cbActual);
//                    else
//                    {
//                        free(pTempData);
//                        return WMAERR_BUFFERTOOSMALL;
//                    }
//
//                    cbOffset +=cbActual;
//                }while(cbWanted >0);
//                for (i=0;i<pInt->m_pMarker->m_dwDescLen && i < DESC_NAME_MAX_LENGTH; i++)
//                {
//                    LoadWORD(tw, pTempData);
//                    pInt->m_pMarker->m_pwDescName[i] = tw;
//                }
//                free(pTempData);
//
//            }
//            else
//            {
//                cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
//                                            pInt->currPacketOffset + cbObjectOffset, cbWanted, &pData);
//                if(cbActual != cbWanted)
//                    return WMAERR_BUFFERTOOSMALL;
//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//                if (IsBadReadPtr(pData, cbActual))
//                {
//                    return WMAERR_BUFFERTOOSMALL;
//                }
//#endif
//
//                cbObjectOffset += cbActual;
//                for (i=0;i<pInt->m_pMarker->m_dwDescLen && i < DESC_NAME_MAX_LENGTH; i++)
//                {
//                    LoadWORD(tw, pData);
//                    pInt->m_pMarker->m_pwDescName[i] = tw;
//                }
//            }
//        }
//    }
//
//    /* use all */
//    pInt->currPacketOffset += cbSize;
//
//    return WMAERR_OK;
//}

/****************************************************************************/
//WMAERR WMA_GetMarkerObject(tWMAFileHdrStateInternal *pInt, int iIndex)
//{
//    WMAERR wmarc;
//    GUID objId;
//    QWORD qwSize;
//
//    if (pInt == NULL)
//    {
//        return WMAERR_INVALIDARG;
//    }
//
//    /* initialize the some state */
//
//    pInt->currPacketOffset = 0;
//
//    /* ASF Header Object */
//
//    wmarc = WMA_LoadHeaderObject(pInt, 0);
//    if (wmarc != WMAERR_OK)
//    {
//        return wmarc;
//    }
//    pInt->cbFirstPacketOffset = pInt->cbHeader += DATA_OBJECT_SIZE;
//
//    /* Scan Header Objects */
//
//    while (pInt->currPacketOffset < pInt->cbFirstPacketOffset)
//    {
//        wmarc = WMA_LoadObjectHeader(pInt, &objId, &qwSize);
//        if (wmarc != WMAERR_OK)
//        {
//            return wmarc;
//        }
//
//#ifdef WMA_TABLE_ROOM_VERIFY
//        if (WMA_IsEqualGUID(p_CLSID_CAsfMarkerObjectV0, &objId))
//#else
//        if (WMA_IsEqualGUID(&CLSID_CAsfMarkerObjectV0, &objId))
//#endif
//        {
//            wmarc = WMA_LoadMarkerObject(pInt, qwSize.dwLo, iIndex);
//            if (wmarc != WMAERR_OK)
//            {
//                return wmarc;
//            }
//            break;
//        }
//        else
//        {
//            /* skip over this object */
//            pInt->currPacketOffset += qwSize.dwLo - MIN_OBJECT_SIZE;
//        }
//    }
//
//    return WMAERR_OK;
//}

/****************************************************************************/
WMAERR WMA_LoadVirtualMetaDataObject(tWMAFileHdrStateInternal *pInt, DWORD cbSize, DWORD Offset)
{
    BYTE *pData = NULL;
    DWORD cbActual = 0;
    DWORD cbWanted = 0;
    U64 iCurrFileOffset;
    U64 iMaxFileOffset;
    WMAERR wmaerr = WMAERR_OK;
    WORD DescRecCount = 0;
    WORD i = 0;

    WMA_MetaDataDescRecords *tDesc = NULL;

    assert(cbSize >= MIN_OBJECT_SIZE);  // Guaranteed by WMA_LoadObjectHeader
    iCurrFileOffset = pInt->currPacketOffset + Offset + MIN_OBJECT_SIZE;
    iMaxFileOffset = iCurrFileOffset + cbSize - MIN_OBJECT_SIZE;

    cbWanted = cbSize;

    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt, iCurrFileOffset,
                                cbWanted, &pData);
    if (cbActual != cbWanted)
    {
        wmaerr = TraceResult(WMAERR_BUFFERTOOSMALL);
        return WMAERR_FAIL;//goto exit;
    }
    iCurrFileOffset += cbActual;

//    if (pInt->ptMetaDataEntry)
//        goto exit;
//
//    // Load Description Records Count
//    cbWanted = 2;
//    if (iCurrFileOffset + cbWanted > iMaxFileOffset)
//    {
//        wmaerr = TraceResult(WMAERR_CORRUPTDATA);
//        goto exit;
//    }
//    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt, iCurrFileOffset,
//        cbWanted, &pData);
//    if (cbActual != cbWanted)
//    {
//        wmaerr = TraceResult(WMAERR_BUFFERTOOSMALL);
//        goto exit;
//    }
//    iCurrFileOffset += cbActual;
//    LoadWORD(DescRecCount  , pData);
//
//    if (0 == DescRecCount)
//        goto exit;
//
//    // Allocate a description records structure, and an array to hold all description records
//    pInt->ptMetaDataEntry = malloc(sizeof(WMA_MetaDataEntry));
//    if (NULL == pInt->ptMetaDataEntry)
//    {
//        wmaerr = TraceResult(WMAERR_OUTOFMEMORY);
//        goto exit;
//    }
//    pInt->ptMetaDataEntry->m_wDescRecordsCount = DescRecCount;
//    pInt->ptMetaDataEntry->pDescRec = malloc(DescRecCount * sizeof(WMA_MetaDataDescRecords));
//    if (NULL == pInt->ptMetaDataEntry->pDescRec)
//    {
//        wmaerr = TraceResult(WMAERR_OUTOFMEMORY);
//        goto exit;
//    }
//    memset(pInt->ptMetaDataEntry->pDescRec, 0, DescRecCount * sizeof(WMA_MetaDataDescRecords));
//    tDesc = pInt->ptMetaDataEntry->pDescRec;
//
//    // Read in all description records
//    for (i = 0; i < DescRecCount; i++)
//    {
//        // Load in the fixed portion of the description record
//        cbWanted = 12;
//        if (iCurrFileOffset + cbWanted > iMaxFileOffset)
//        {
//            wmaerr = TraceResult(WMAERR_CORRUPTDATA);
//            goto exit;
//        }
//        cbActual = WMAFileCBGetData((tHWMAFileState *)pInt, iCurrFileOffset,
//            cbWanted, &pData);
//        if (cbActual != cbWanted)
//        {
//            wmaerr = TraceResult(WMAERR_BUFFERTOOSMALL);
//            goto exit;
//        }
//        iCurrFileOffset += cbActual;
//
//        LoadWORD(tDesc[i].wLangIdIndex, pData);
//        LoadWORD(tDesc[i].wStreamNumber, pData);
//        LoadWORD(tDesc[i].wNameLenth, pData);
//        LoadWORD(tDesc[i].wDataType, pData);
//        LoadDWORD(tDesc[i].dwDataLength, pData);
//
//        // Read in the name. Truncate if necessary.
//        wmaerr = AllocateAndLoadMetadataStr((tHWMAFileState *)pInt, &iCurrFileOffset,
//            iMaxFileOffset, &tDesc[i].wNameLenth, (BYTE **)&tDesc[i].pwName);
//        if (WMAERR_OK != wmaerr)
//        {
//            TraceResult(wmaerr);
//            goto exit;
//        }
//
//        // Read in the value.
//        if (ECD_STRING == tDesc[i].wDataType)
//        {
//            WORD    wDataLength;
//            DWORD   dwAdditionalSkip = 0;
//
//            if (tDesc[i].dwDataLength > 0x0000FFFF)
//            {
//                // Truncate to 16-bit size, and remember to add the remainder to file offset
//                dwAdditionalSkip = tDesc[i].dwDataLength - 0x0000FFFF;
//                tDesc[i].dwDataLength = 0x0000FFFF;
//            }
//
//            // We will truncate this string if necessary
//            wDataLength = (WORD)tDesc[i].dwDataLength;
//            wmaerr = AllocateAndLoadMetadataStr((tHWMAFileState *)pInt,
//                &iCurrFileOffset, iMaxFileOffset, &wDataLength, &tDesc[i].pData);
//            tDesc[i].dwDataLength = wDataLength;
//            if (WMAERR_OK != wmaerr)
//            {
//                TraceResult(wmaerr);
//                goto exit;
//            }
//            iCurrFileOffset += dwAdditionalSkip;
//
//#ifndef LITTLE_ENDIAN
//            SwapWstr(tDesc[i].pData, tDesc[i].dwDataLength / sizeof(tWMA_U16));
//#endif // LITTLE_ENDIAN
//        }
//        else
//        {
//            U32 cbBytesRead;
//            const U32 cbBytesToRead = tDesc[i].dwDataLength;
//
//            // Do not truncate
//            // Check if reading this object takes us past end of object
//            if (iCurrFileOffset + tDesc[i].dwDataLength > iMaxFileOffset)
//            {
//                wmaerr = TraceResult(WMAERR_CORRUPTDATA);
//                goto exit;
//            }
//
//            tDesc[i].pData = malloc(tDesc[i].dwDataLength);
//            if (NULL == tDesc[i].pData)
//            {
//                wmaerr = TraceResult(WMAERR_OUTOFMEMORY);
//                goto exit;
//            }
//
//            for (cbBytesRead = 0; cbBytesRead < cbBytesToRead; )
//            {
//                cbWanted = MIN(MAX_BUFSIZE, (cbBytesToRead - cbBytesRead));
//                cbActual = WMAFileCBGetData(pInt, iCurrFileOffset + cbBytesRead,
//                    cbWanted, &pData);
//                if (cbActual != cbWanted)
//                {
//                    //TraceInfo2("WMA_LoadVirtualMetaDataObject requested %d "
//                    //    "from WMAFileCBGetData, got %d!", cbWanted, cbActual);
//                    wmaerr = TraceResult(WMAERR_BUFFERTOOSMALL);
//                    goto exit;
//                }
//
//                memcpy(&tDesc[i].pData[cbBytesRead], pData, cbActual);
//                cbBytesRead += cbActual;
//            }
//
//            if (cbBytesRead != cbBytesToRead)
//            {
//                wmaerr = TraceResult(WMAERR_BUFFERTOOSMALL);
//                goto exit;
//            }
//
//            iCurrFileOffset += cbBytesRead;
//
//#ifndef LITTLE_ENDIAN
//            switch (tDesc[i].wDataType)
//            {
//                case ECD_BOOL:
//                case ECD_DWORD:
//                    SWAPDWORD(tDesc[i].pData);
//                    break;
//
//                case ECD_QWORD:
//                    SWAPBYTES(tDesc[i].pData, 8);
//                    break;
//
//                case ECD_WORD:
//                    SWAPWORD(tDesc[i].pData);
//                    break;
//            } // switch
//#endif // LITTLE_ENDIAN
//
//        } // else (NOT ECD_STRING)
//
//    }
//
//exit:
//    // Deal with errors in a "transactional" fashion: either succeed completely,
//    // or fail completely. In other words, clean up if we encountered any errors.
//    if (WMAERR_OK != wmaerr)
//    {
//        if (pInt->ptMetaDataEntry)
//        {
//            if (pInt->ptMetaDataEntry->pDescRec)
//            {
//                tDesc = pInt->ptMetaDataEntry->pDescRec;
//                for (i = 0; i < pInt->ptMetaDataEntry->m_wDescRecordsCount; i++)
//                {
//                    // it's OK to pass NULL pointers to free
//                    free(tDesc[i].pwName);
//                    free(tDesc[i].pData);
//                }
//                free(pInt->ptMetaDataEntry->pDescRec);
//            }
//
//            free(pInt->ptMetaDataEntry);
//            pInt->ptMetaDataEntry = NULL;
//        }
//    }


    // Note: This function is not responsible for advancing pInt->currPacketOffset

    return wmaerr;
}

// Virtual load
WMAERR  WMA_LoadVirtualExtendedStreamPropertiesObject(tWMAFileHdrStateInternal *pInt,
        DWORD cbSize,
        int isFull,
        DWORD Offset)
{
    DWORD cbObjectOffset = 0;

    BYTE *pData = NULL;
    DWORD cbActual = 0;
    DWORD cbWanted = 0;
    DWORD cbOffset = 0;
    WORD  wStreamNameCount = 0;
    WORD  wPayloadSystemExtensionCount = 0;
    WMAERR rc = WMAERR_OK;

    if (pInt == NULL)
        return WMAERR_INVALIDARG;

    cbSize -= MIN_OBJECT_SIZE;
    Offset += MIN_OBJECT_SIZE; // Step over GUID + length field of current object


    // Get to stream name count
    cbObjectOffset = 60;

    cbWanted = 4;
    if (cbObjectOffset + cbWanted > cbSize)
        return WMAERR_BUFFERTOOSMALL;

    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                pInt->currPacketOffset + Offset + cbObjectOffset, cbWanted, &pData);

    if ((cbActual != cbWanted) || (NULL == pData))
    {
        return WMAERR_BUFFERTOOSMALL;
    }
#if (defined(WIN32) || defined(_WIN32_WINCE) )
    if (IsBadReadPtr(pData, cbActual))
    {
        return WMAERR_BUFFERTOOSMALL;
    }
#endif

    cbObjectOffset += cbActual;

    LoadWORD(wStreamNameCount , pData);
    LoadWORD(wPayloadSystemExtensionCount , pData);

    // Read past the stream names
    if (0 != wStreamNameCount)
    {
        int i;
        int cStreamNames = (int) wStreamNameCount;
        WORD wStreamNameLength = 0;

        for (i = 0; i < cStreamNames; i++)
        {
            cbWanted = 2;
            // Dont care for language
            cbObjectOffset += 2;
            if (cbObjectOffset + cbWanted > cbSize)
                return WMAERR_BUFFERTOOSMALL;

            cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                        pInt->currPacketOffset + Offset + cbObjectOffset, cbWanted, &pData);

            if ((cbActual != cbWanted) || (NULL == pData))
            {
                return WMAERR_BUFFERTOOSMALL;
            }
#if (defined(WIN32) || defined(_WIN32_WINCE) )
            if (IsBadReadPtr(pData, cbActual))
            {
                return WMAERR_BUFFERTOOSMALL;
            }
#endif

            cbObjectOffset += cbActual;
            LoadWORD(wStreamNameLength , pData);
            cbObjectOffset += wStreamNameLength;
        }
    }

    // Read past the payload extension systems
    if (0 != wPayloadSystemExtensionCount)
    {
        int i;
        int cPayloadSystemExtensionCount = (int) wPayloadSystemExtensionCount;
        DWORD wPayloadSystemLength = 0;

#ifdef WMDRM_NETWORK
        if (pInt->cPEData == WMA_MAX_PAYLOAD_EXTENSION_STREAMS)
        {
            //
            // We found more streams than we have space for. For simplicity
            // of implementation we have a fixed list of streams to store info
            // about the payload extensions.
            //
            return WMAERR_FAIL;
        }

        if (cPayloadSystemExtensionCount > WMA_MAX_PAYLOAD_EXTENSIONS)
        {
            //
            // We found more extensions than we have space for.
            //
            return WMAERR_FAIL;
        }

        pInt->PEData[ pInt->cPEData ].wStreamNum = wStreamNum;
#endif

        for (i = 0; i < cPayloadSystemExtensionCount; i++)
        {
#ifdef WMDRM_NETWORK
            //
            // Scan the payload extension systems looing for the "Sample ID"
            // extension type, which we need for WMDRM-ND-encrypted files. We
            // Also need to grok the size of all other extension system data
            // so we can find the sample IDs properly.
            //
            GUID guidSystemID;

            cbWanted = sizeof(GUID) + sizeof(WORD) + sizeof(DWORD);
            if (cbObjectOffset + cbWanted > cbSize)
            {
                return WMAERR_BUFFERTOOSMALL;
            }

            cbActual = WMAFileCBGetData(
                           (tHWMAFileState *)pInt,
                           pInt->currPacketOffset + Offset + cbObjectOffset,
                           cbWanted,
                           &pData);

            if ((cbActual != cbWanted) || (NULL == pData))
            {
                return WMAERR_BUFFERTOOSMALL;
            }

            LoadGUID(guidSystemID, pData);

            pInt->PEData[ pInt->cPEData ].Systems[ i ].bIsSampleId =
                WMA_IsEqualGUID(
                    &CLSID_ASF_Payload_Extension_Encryption_SampleID,
                    &guidSystemID);

            LoadWORD(pInt->PEData[ pInt->cPEData ].Systems[ i ].cbDataSize,
                     pData);

            pInt->cPEData++;
#else
            cbWanted = 4;
            // Dont care for GUID etc
            cbObjectOffset += 18;
            if (cbObjectOffset + cbWanted > cbSize)
                return WMAERR_BUFFERTOOSMALL;

            cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                        pInt->currPacketOffset + Offset + cbObjectOffset, cbWanted, &pData);

            if ((cbActual != cbWanted) || (NULL == pData))
            {
                return WMAERR_BUFFERTOOSMALL;
            }
#if (defined(WIN32) || defined(_WIN32_WINCE) )
            if (IsBadReadPtr(pData, cbActual))
            {
                return WMAERR_BUFFERTOOSMALL;
            }
#endif

            cbObjectOffset += cbActual;
            LoadDWORD(wPayloadSystemLength , pData);
            cbObjectOffset += wPayloadSystemLength;
#endif
        }
    }
    // Check for existence of Stream Properties at the end of extended stream properties.
    if ((cbObjectOffset + MIN_OBJECT_SIZE) < cbSize)
    {
        GUID  sobjId;
        QWORD sqwSize;

        rc =  WMA_LoadVirtualObjectHeader(pInt, &sobjId, &sqwSize, Offset + cbObjectOffset);
        if (rc != WMAERR_OK)
            return rc;

        cbObjectOffset += MIN_OBJECT_SIZE;

#ifdef WMA_TABLE_ROOM_VERIFY
        if (WMA_IsEqualGUID(p_CLSID_CAsfStreamPropertiesObjectV1, &sobjId)
                || WMA_IsEqualGUID(p_CLSID_CAsfStreamPropertiesObjectV2, &sobjId))
#else
        if (WMA_IsEqualGUID(&CLSID_CAsfStreamPropertiesObjectV1, &sobjId)
                || WMA_IsEqualGUID(&CLSID_CAsfStreamPropertiesObjectV2, &sobjId))
#endif
        {
            // Try reading audio property here. Since WMA_LoadAudioObject is not virtual, do
            // some work-around here.
            U64 dwSafeCurrPacketOffset = pInt->currPacketOffset;
            pInt->currPacketOffset += Offset + cbObjectOffset;

            if (pInt->currPacketOffset + (sqwSize.dwLo - MIN_OBJECT_SIZE) > pInt->cbFirstPacketOffset)
                return WMAERR_FAIL;
            rc = WMA_LoadAudioObject(pInt, sqwSize.dwLo,
                                     isFull);
            if (rc != WMAERR_OK)
            {
                return rc;
            }
            // restore
            pInt->currPacketOffset = dwSafeCurrPacketOffset;
        }
    }

    return rc;
} // WMA_LoadVirtualExtendedStreamPropertiesObject
//#endif


/****************************************************************************/

// Acutally now called header extension object.
WMAERR  WMA_LoadClockObject(tWMAFileHdrStateInternal *pInt, DWORD cbSize,
                            int isFull)
{
    DWORD cbObjectOffset = 0;

    GUID PacketClockType;
    WORD PacketClockSize;
    DWORD dwHeaderExtDataSize = 0;
    BYTE *pData = NULL;
    DWORD cbActual = 0;
    DWORD cbWanted = 0;
    DWORD cbOffset = 0;
    WMAERR rc = WMAERR_OK;
    WORD wMetaDataObject = 0;
    WORD wAsfStreamPropertiesExObject = 0;
    WORD wExtendedStreamPropertiesObject = 0;


    if (pInt == NULL)
        return WMAERR_INVALIDARG;

    cbSize -= MIN_OBJECT_SIZE;

    cbWanted = 18;
    if (cbObjectOffset + cbWanted > cbSize)
        return WMAERR_BUFFERTOOSMALL;

    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                pInt->currPacketOffset + cbObjectOffset, cbWanted, &pData);

    if ((cbActual != cbWanted) || (NULL == pData))
    {
        return WMAERR_BUFFERTOOSMALL;
    }
//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//    if (IsBadReadPtr(pData, cbActual))
//    {
//        return WMAERR_BUFFERTOOSMALL;
//    }
//#endif


    cbObjectOffset += cbActual;

    LoadGUID(PacketClockType , pData);

#ifdef WMA_TABLE_ROOM_VERIFY
    if (WMA_IsEqualGUID(p_CLSID_CAsfPacketClock1, &PacketClockType) == 0)
#else
    if (WMA_IsEqualGUID(&CLSID_CAsfPacketClock1, &PacketClockType) == 0)
#endif
    {
        pInt->currPacketOffset += cbSize;
        return WMAERR_BUFFERTOOSMALL;
    }

    LoadWORD(PacketClockSize      , pData);

    if (PacketClockSize != 6)
    {
        pInt->currPacketOffset += cbSize;
        return WMAERR_BUFFERTOOSMALL;
    }

    cbWanted = 4;
    if (cbObjectOffset + cbWanted > cbSize)
    {
        return WMAERR_BUFFERTOOSMALL;
    }

    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                pInt->currPacketOffset + cbObjectOffset, cbWanted, &pData);

    if ((cbActual != cbWanted) || (NULL == pData))
    {
        return WMAERR_BUFFERTOOSMALL;
    }
//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//    if (IsBadReadPtr(pData, cbActual))
//    {
//        return WMAERR_BUFFERTOOSMALL;
//    }
//#endif


    cbObjectOffset += cbActual;

    LoadDWORD(dwHeaderExtDataSize    , pData);

    if (dwHeaderExtDataSize >= MIN_OBJECT_SIZE)
    {
        // There are more objects. parse them.

        GUID  sobjId;
        QWORD sqwSize;

        /* search sub-header extension object(language list object,extended stream properties object etc)
           in header extension object hxd*/
        while (cbObjectOffset < cbSize)
        {
            rc =  WMA_LoadVirtualObjectHeader(pInt, &sobjId, &sqwSize, cbObjectOffset);
            if (rc != WMAERR_OK)
                return rc;
#ifdef WMA_TABLE_ROOM_VERIFY
            if (WMA_IsEqualGUID(p_CLSID_AsfXMetaDataObject, &sobjId))
#else
            if (WMA_IsEqualGUID(&CLSID_AsfXMetaDataObject, &sobjId))
#endif
            {
                wMetaDataObject++;
                if ((cbObjectOffset + sqwSize.dwLo > cbSize) || (wMetaDataObject > 1))
                {
                    rc = WMAERR_BUFFERTOOSMALL;
                    goto sabort;
                }
                rc = WMA_LoadVirtualMetaDataObject(pInt, sqwSize.dwLo, cbObjectOffset);
                cbObjectOffset += sqwSize.dwLo;
                if (WMAERR_OK != rc)
                {
                    // Note the error, but ignore it and try to continue. It's just metadata.
                    TraceResult(rc);
                    rc = WMAERR_OK;
                }
            }
#ifdef WMA_TABLE_ROOM_VERIFY
            else if (WMA_IsEqualGUID(p_CLSID_CAsfExtendedStreamPropertiesObject, &sobjId))
#else
            else if (WMA_IsEqualGUID(&CLSID_CAsfExtendedStreamPropertiesObject, &sobjId))
#endif
            {
                wExtendedStreamPropertiesObject++;
                if (cbObjectOffset + (sqwSize.dwLo - MIN_OBJECT_SIZE) > pInt->cbFirstPacketOffset)
                    return WMAERR_FAIL;
                rc = WMA_LoadVirtualExtendedStreamPropertiesObject(pInt, sqwSize.dwLo,
                        isFull, cbObjectOffset);
                cbObjectOffset += sqwSize.dwLo;
                if (rc != WMAERR_OK)
                {
                    return rc;
                }
            }
            else
            {
                /* skip over this object */
                cbObjectOffset += sqwSize.dwLo;
            }
        }

    }

sabort:
    pInt->currPacketOffset += cbSize;
    return rc;
} // WMA_LoadClockObject

/****************************************************************************/
WMAERR
WMA_ParseAsfHeader(tWMAFileHdrStateInternal *pInt,
                   int isFull)
{
    WMAERR wmarc;
    GUID objId;
    QWORD qwSize;
    WORD wHeaderObject = 0;
    WORD wPropertiesObject = 0;
    WORD wStreamPropertiesObject = 0;
    WORD wMarkerObject = 0;
    WORD wContentDescriptionObject = 0;
    WORD wExtendedContentDescObject = 0;
    WORD wLicenceStoreObject = 0;

//    if(pInt == NULL)
    //    {
    //        return WMAERR_INVALIDARG;
    //    }


    /* initialize the some state */

    pInt->currPacketOffset = 0;


    /* ASF Header Object */

    wmarc = WMA_LoadHeaderObject(pInt, isFull);
    if (wmarc != WMAERR_OK)
    {
        return wmarc;
    }
    wHeaderObject++;

    /*offset of firstpacket to asf header=countnumbers of asf header+ data_section_introduction_object(50bytes); hxd*/
    pInt->cbFirstPacketOffset = pInt->cbHeader += DATA_OBJECT_SIZE;

    /* Scan Header Objects */

    /* if currpacketoffset(current pointer offset to asf header)<data object offset to asf header,
       then current pointer is among asf header*/
    while (pInt->currPacketOffset < pInt->cbFirstPacketOffset - DATA_OBJECT_SIZE)
    {
        wmarc = WMA_LoadObjectHeader(pInt, &objId, &qwSize);
        if (wmarc != WMAERR_OK)
        {
            return wmarc;
        }

#ifdef WMA_TABLE_ROOM_VERIFY
        if (WMA_IsEqualGUID(p_CLSID_CAsfPropertiesObjectV2, &objId))
#else
        if (WMA_IsEqualGUID(&CLSID_CAsfPropertiesObjectV2, &objId))
#endif
        {
            wPropertiesObject++;
            if (pInt->currPacketOffset + (qwSize.dwLo - MIN_OBJECT_SIZE) > pInt->cbFirstPacketOffset)
                return WMAERR_FAIL;

            wmarc = WMA_LoadPropertiesObject(pInt, qwSize.dwLo,isFull);
            if (wmarc != WMAERR_OK)
            {
                return wmarc;
            }
        }
#ifdef WMA_TABLE_ROOM_VERIFY
        else if (WMA_IsEqualGUID(p_CLSID_CAsfStreamPropertiesObjectV1, &objId)
                 || WMA_IsEqualGUID(p_CLSID_CAsfStreamPropertiesObjectV2, &objId))
#else
        else if (WMA_IsEqualGUID(&CLSID_CAsfStreamPropertiesObjectV1, &objId)
                 || WMA_IsEqualGUID(&CLSID_CAsfStreamPropertiesObjectV2, &objId))
#endif
        {
            wStreamPropertiesObject++;
            if (pInt->currPacketOffset + (qwSize.dwLo - MIN_OBJECT_SIZE) > pInt->cbFirstPacketOffset)
                return WMAERR_FAIL;
            wmarc = WMA_LoadAudioObject(pInt, qwSize.dwLo,
                                        isFull);
            if (wmarc != WMAERR_OK)
            {
                return wmarc;
            }
        }
#ifdef WMA_TABLE_ROOM_VERIFY
        else if (WMA_IsEqualGUID(p_CLSID_CAsfContentEncryptionObject, &objId))
#else
        else if (WMA_IsEqualGUID(&CLSID_CAsfContentEncryptionObject, &objId))
#endif
        {
            if (pInt->currPacketOffset + (qwSize.dwLo - MIN_OBJECT_SIZE) > pInt->cbFirstPacketOffset)
                return WMAERR_FAIL;
            wmarc = WMA_LoadEncryptionObject(pInt, qwSize.dwLo);
            if (wmarc != WMAERR_OK)
            {
                return wmarc;
            }
        }

#ifdef WMA_TABLE_ROOM_VERIFY
        else if (WMA_IsEqualGUID(p_CLSID_CAsfContentDescriptionObjectV0, &objId))
#else
        else if (WMA_IsEqualGUID(&CLSID_CAsfContentDescriptionObjectV0, &objId))
#endif
        {
            wContentDescriptionObject++;
            if (pInt->currPacketOffset + (qwSize.dwLo - MIN_OBJECT_SIZE) > pInt->cbFirstPacketOffset)
                return WMAERR_FAIL;
            wmarc = WMA_LoadContentDescriptionObject(pInt, qwSize.dwLo);
            if (WMAERR_OK != wmarc)
            {
                // Note the error, but ignore it and try to continue. It's just metadata.
                TraceResult(wmarc);
                wmarc = WMAERR_OK;
            }
        }
#ifdef WMA_TABLE_ROOM_VERIFY
        else if (WMA_IsEqualGUID(p_CLSID_CAsfExtendedContentDescObject, &objId))
#else
        else if (WMA_IsEqualGUID(&CLSID_CAsfExtendedContentDescObject, &objId))
#endif
        {
            wExtendedContentDescObject++;
            if (pInt->currPacketOffset + (qwSize.dwLo - MIN_OBJECT_SIZE) > pInt->cbFirstPacketOffset)
                return WMAERR_FAIL;
            if (isFull)
            {
                // only in this case, we parse extended content desc
                wmarc = WMA_LoadExtendedContentDescObject(pInt, qwSize.dwLo);
                if (WMAERR_OK != wmarc)
                {
                    // Note the error, but ignore it and try to continue. It's just metadata.
                    TraceResult(wmarc);
                    wmarc = WMAERR_OK;
                }
            }
            else
            {
                pInt->currPacketOffset += qwSize.dwLo - MIN_OBJECT_SIZE;
                wmarc = WMAERR_OK;
            }
        }
#ifdef WMA_TABLE_ROOM_VERIFY
        else if (WMA_IsEqualGUID(p_CLSID_CAsfLicenseStoreObject, &objId))
#else
        else if (WMA_IsEqualGUID(&CLSID_CAsfLicenseStoreObject, &objId))
#endif
        {
            wLicenceStoreObject++;
            if (pInt->currPacketOffset + (qwSize.dwLo - MIN_OBJECT_SIZE) > pInt->cbFirstPacketOffset)
                return WMAERR_FAIL;
            wmarc = WMA_LoadLicenseStoreObject(pInt, qwSize.dwLo);
            if (wmarc != WMAERR_OK)
            {
                return wmarc;
            }
        }
#ifdef WMA_TABLE_ROOM_VERIFY
        else if (WMA_IsEqualGUID(p_CLSID_CAsfClockObjectV0, &objId))
#else
        else if (WMA_IsEqualGUID(&CLSID_CAsfClockObjectV0, &objId))
#endif
        {
            if (pInt->currPacketOffset + (qwSize.dwLo - MIN_OBJECT_SIZE) > pInt->cbFirstPacketOffset)
                return WMAERR_FAIL;
            wmarc = WMA_LoadClockObject(pInt, qwSize.dwLo, isFull);
            if (wmarc != WMAERR_OK)
                return wmarc;
        }
        else
        {
            /* skip over this object */
            pInt->currPacketOffset += qwSize.dwLo - MIN_OBJECT_SIZE;
        }
    }

    if ((wHeaderObject != 1) || (wPropertiesObject != 1) || (wStreamPropertiesObject < 1))
    {
        return WMAERR_FAIL;
    }
    if (pInt->currPacketOffset != (pInt->cbFirstPacketOffset - DATA_OBJECT_SIZE))
        return WMAERR_FAIL;



    return WMAERR_OK;
}
#pragma arm section code

#endif

#if defined(WMAHIGHRATE) || defined(WMAMIDRATELOWRATE)
#pragma arm section code = "WmaHighLowCommonCode"

/****************************************************************************/
WMAERR
WMA_ParsePacketHeader(tWMAFileStateInternal *pInt)
{
    BYTE b = 0;
    PACKET_PARSE_INFO_EX *pParseInfoEx = NULL;

    BYTE *pData = NULL;
    DWORD cbActual = 0;
    DWORD cbWanted = 0;
    DWORD cbLocalOffset = 0;

    if (pInt == NULL)
    {
        return WMAERR_INVALIDARG;
    }

    pParseInfoEx = &pInt->ppex;

//    cbWanted = 24;              /* at most */

    cbWanted = 1;              /* at least */
    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                pInt->hdr_parse.currPacketOffset,
                                cbWanted, &pData);


    if (cbActual != cbWanted)
    {
        return WMAERR_BUFFERTOOSMALL;
    }
//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//    if (IsBadReadPtr(pData, cbActual))
//    {
//        return WMAERR_BUFFERTOOSMALL;
//    }
//#endif



    /* ParsePacket begins */

    pParseInfoEx->fParityPacket = FALSE;
    pParseInfoEx->cbParseOffset = 0;

//////////////////////////////////////////////////
//    b = pData[pParseInfoEx->cbParseOffset];

    b = pData[cbLocalOffset];//get first word in data object
///////////////////////////////////////////////////

    pParseInfoEx->fEccPresent = (BOOL)((b & 0x80) == 0x80);//compute the highest bit of first word
    pParseInfoEx->bECLen = 0;

    /* if the highest bit is 1,then packet is beginning from error correction data;else from payload data*/
    if (pParseInfoEx->fEccPresent)
    {
        if (b&0x10)//0x10 is opaque data present
        {
            pParseInfoEx->fParityPacket = TRUE;
            return WMAERR_OK;
        }

        /*if error correction length type is set 0x00,then this field is valid;
        if it isn'5 0x00,this field is reserved for future.*/
        if (b&0x60)
        {
            return WMAERR_FAIL;
        }

        /* error correction length(bit0-3),this field should be set 0x0010;other value is invalid.*/
        pParseInfoEx->bECLen = (b & 0x0f);
        if (pParseInfoEx->bECLen != 2)
        {
            return WMAERR_FAIL;
        }


        pParseInfoEx->cbParseOffset = (DWORD)(1 + pParseInfoEx->bECLen);
//////////////////////////////////////////////////////////////////////////////
        cbWanted = 1;              /* at least */
        cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                    pInt->hdr_parse.currPacketOffset + pParseInfoEx->cbParseOffset,
                                    cbWanted, &pData);

        if (cbActual != cbWanted)
        {
            return WMAERR_BUFFERTOOSMALL;
        }
#if (defined(WIN32) || defined(_WIN32_WINCE) )
        if (IsBadReadPtr(pData, cbActual))
        {
            return WMAERR_BUFFERTOOSMALL;
        }
#endif

//        b = pData[pParseInfoEx->cbParseOffset];
        b = pData[cbLocalOffset];

//////////////////////////////////////////////////////////////////////////////
    }

    pParseInfoEx->cbPacketLenTypeOffset = pParseInfoEx->cbParseOffset;

    //get packet length type(bits 5-6) in payload parse info;four type:00,01,10,11
    pParseInfoEx->bPacketLenType = (b & 0x60) >> 5;

    //get padding length type(bits 3-4) in payload parse info;four type:00,01,10,11
    pParseInfoEx->bPadLenType = (b & 0x18) >> 3;

    //sequence type(bits 1-2) in payload parse info;four type:00,01,10,11
    pParseInfoEx->bSequenceLenType = (b & 0x06) >> 1;

    //Multiple payloads present(bit 0) in payload parse info
    pParseInfoEx->fMultiPayloads = (BOOL)(b & 0x01);

    pParseInfoEx->cbParseOffset++;//cbparseoffset increase 1 bytes;it is used to compute offset to first packet's beginning

//////////////////////////////////////////////////////////////////////////////
    cbWanted = 1;              /* at least */
    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                pInt->hdr_parse.currPacketOffset + pParseInfoEx->cbParseOffset,
                                cbWanted, &pData);

    if (cbActual != cbWanted)
    {
        return WMAERR_BUFFERTOOSMALL;
    }
#if (defined(WIN32) || defined(_WIN32_WINCE) )
    if (IsBadReadPtr(pData, cbActual))
    {
        return WMAERR_BUFFERTOOSMALL;
    }
#endif

    //        b = pData[pParseInfoEx->cbParseOffset];
    b = pData[cbLocalOffset];

//////////////////////////////////////////////////////////////////////////////

    pParseInfoEx->bOffsetBytes = 4;
    pParseInfoEx->bOffsetLenType = 3;

    /* property flag in payload parse info must be ox5d.*/
    if (b != 0x5d)
    {
        /* stream number length type(bit 6-7) must be 0x01*/
        if ((b&0xc0) != 0x40)
        {
            return WMAERR_FAIL;
        }

        /* media object number length type(bit 4-5) must be 0x01*/
        if ((b&0x30) != 0x10)
        {
            return WMAERR_FAIL;
        }

        /* offset into media object length type(bits 2-3) must be 0x11,else the following*/
        pParseInfoEx->bOffsetLenType = (b & 0x0c) >> 2;
        if (pParseInfoEx->bOffsetLenType == 0)
        {
            return WMAERR_FAIL;
        }
        else if (pParseInfoEx->bOffsetLenType < 3)
        {
            pParseInfoEx->bOffsetBytes = pParseInfoEx->bOffsetLenType;
        }

        /* Replicated data length type(bits 0-1) must be 0x01*/
        if ((b&0x03) != 0x01)
        {
            return WMAERR_FAIL;
        }
    }

    pParseInfoEx->cbParseOffset++;//cbparseoffset increse 1

//////////////////////////////////////////////////////////////////////////////
    pParseInfoEx->cbPacketLenOffset = pParseInfoEx->cbParseOffset;
    /*packet length type(bits 5-6) in payload parse info,reset 0x00
      0x01 represent Byte;0x02 represent Word;0x03 represent Dword*/
    switch (pParseInfoEx->bPacketLenType)
    {
        case 0x01:
            {
                cbWanted = 1;
                cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                            pInt->hdr_parse.currPacketOffset + pParseInfoEx->cbParseOffset,
                                            cbWanted, &pData);
                if (cbActual != cbWanted)
                {
                    return WMAERR_BUFFERTOOSMALL;
                }
#if (defined(WIN32) || defined(_WIN32_WINCE) )
                if (IsBadReadPtr(pData, cbActual))
                {
                    return WMAERR_BUFFERTOOSMALL;
                }
#endif

                pParseInfoEx->cbExplicitPacketLength = (DWORD)(*pData); //packet length in payload parse info
                pParseInfoEx->cbParseOffset++;
                break;
            }
        case 0x02:
            {
                WORD w = 0;
                cbWanted = 2;
                cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                            pInt->hdr_parse.currPacketOffset + pParseInfoEx->cbParseOffset,
                                            cbWanted, &pData);
                if (cbActual != cbWanted)
                {
                    return WMAERR_BUFFERTOOSMALL;
                }
#if (defined(WIN32) || defined(_WIN32_WINCE) )
                if (IsBadReadPtr(pData, cbActual))
                {
                    return WMAERR_BUFFERTOOSMALL;
                }
#endif

                GetUnalignedWord(pData, w);
                pParseInfoEx->cbExplicitPacketLength = (DWORD)(w);
                pParseInfoEx->cbParseOffset += 2;

                break;

            }
        case 0x03:
            {
                DWORD dw = 0;
                cbWanted = 4;
                cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                            pInt->hdr_parse.currPacketOffset + pParseInfoEx->cbParseOffset,
                                            cbWanted, &pData);
                if (cbActual != cbWanted)
                {
                    return WMAERR_BUFFERTOOSMALL;
                }
#if (defined(WIN32) || defined(_WIN32_WINCE) )
                if (IsBadReadPtr(pData, cbActual))
                {
                    return WMAERR_BUFFERTOOSMALL;
                }
#endif
                GetUnalignedDword(pData, dw);
                pParseInfoEx->cbExplicitPacketLength =  dw;
                pParseInfoEx->cbParseOffset += 4;
                break;
            }
        default:
            cbWanted = 0;
            pParseInfoEx->cbExplicitPacketLength = 0;
    }

    /*    pParseInfoEx->cbPacketLenOffset = pParseInfoEx->cbParseOffset;
        pParseInfoEx->cbExplicitPacketLength = GetASFVarField(pParseInfoEx->bPacketLenType,
                                                              &pData[pParseInfoEx->cbParseOffset],
                                                              &pParseInfoEx->cbParseOffset);
    */

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
    pParseInfoEx->cbSequenceOffset = pParseInfoEx->cbParseOffset;

    /*sequence type(bits 1-2) in payload parse info,reset 0x00
     0x01 represent Byte;0x02 represent Word;0x03 represent Dword*/
    switch (pParseInfoEx->bSequenceLenType)
    {
        case 0x01:
            {
                cbWanted = 1;
                cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                            pInt->hdr_parse.currPacketOffset + pParseInfoEx->cbParseOffset,
                                            cbWanted, &pData);
                if (cbActual != cbWanted)
                {
                    return WMAERR_BUFFERTOOSMALL;
                }
#if (defined(WIN32) || defined(_WIN32_WINCE) )
                if (IsBadReadPtr(pData, cbActual))
                {
                    return WMAERR_BUFFERTOOSMALL;
                }
#endif
                pParseInfoEx->dwSequenceNum = (DWORD)(*pData); //sequence in payload parse info
                pParseInfoEx->cbParseOffset++;
                break;
            }
        case 0x02:
            {
                WORD w = 0;
                cbWanted = 2;
                cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                            pInt->hdr_parse.currPacketOffset + pParseInfoEx->cbParseOffset,
                                            cbWanted, &pData);
                if (cbActual != cbWanted)
                {
                    return WMAERR_BUFFERTOOSMALL;
                }
#if (defined(WIN32) || defined(_WIN32_WINCE) )
                if (IsBadReadPtr(pData, cbActual))
                {
                    return WMAERR_BUFFERTOOSMALL;
                }
#endif
                GetUnalignedWord(pData, w);
                pParseInfoEx->dwSequenceNum = (DWORD)(w);
                pParseInfoEx->cbParseOffset += 2;

                break;

            }
        case 0x03:
            {
                DWORD dw = 0;
                cbWanted = 4;
                cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                            pInt->hdr_parse.currPacketOffset + pParseInfoEx->cbParseOffset,
                                            cbWanted, &pData);
                if (cbActual != cbWanted)
                {
                    return WMAERR_BUFFERTOOSMALL;
                }
#if (defined(WIN32) || defined(_WIN32_WINCE) )
                if (IsBadReadPtr(pData, cbActual))
                {
                    return WMAERR_BUFFERTOOSMALL;
                }
#endif
                GetUnalignedDword(pData, dw);
                pParseInfoEx->dwSequenceNum =  dw;
                pParseInfoEx->cbParseOffset += 4;
                break;
            }
        default:
            cbWanted = 0;
            pParseInfoEx->dwSequenceNum = 0;
    }

    /*    pParseInfoEx->cbSequenceOffset = pParseInfoEx->cbParseOffset;
    pParseInfoEx->dwSequenceNum = GetASFVarField(pParseInfoEx->bSequenceLenType,
                                                 &pData[pParseInfoEx->cbParseOffset],
                                                 &pParseInfoEx->cbParseOffset);
    */
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
    pParseInfoEx->cbPadLenOffset = pParseInfoEx->cbParseOffset;

    /*padding length type(bits 3-4) in payload parse info,reset 0x00
      0x01 represent Byte;0x02 represent Word;0x03 represent Dword*/
    switch (pParseInfoEx->bPadLenType)
    {
        case 0x01:
            {
                cbWanted = 1;
                cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                            pInt->hdr_parse.currPacketOffset + pParseInfoEx->cbParseOffset,
                                            cbWanted, &pData);
                if (cbActual != cbWanted)
                {
                    return WMAERR_BUFFERTOOSMALL;
                }
#if (defined(WIN32) || defined(_WIN32_WINCE) )
                if (IsBadReadPtr(pData, cbActual))
                {
                    return WMAERR_BUFFERTOOSMALL;
                }
#endif
                pParseInfoEx->cbPadding = (DWORD)(*pData); //padding length in payload parse info
                pParseInfoEx->cbParseOffset++;
                break;
            }
        case 0x02:
            {
                WORD w = 0;
                cbWanted = 2;
                cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                            pInt->hdr_parse.currPacketOffset + pParseInfoEx->cbParseOffset,
                                            cbWanted, &pData);
                if (cbActual != cbWanted)
                {
                    return WMAERR_BUFFERTOOSMALL;
                }
#if (defined(WIN32) || defined(_WIN32_WINCE) )
                if (IsBadReadPtr(pData, cbActual))
                {
                    return WMAERR_BUFFERTOOSMALL;
                }
#endif
                GetUnalignedWord(pData, w);
                pParseInfoEx->cbPadding = (DWORD)(w);
                pParseInfoEx->cbParseOffset += 2;

                break;

            }
        case 0x03:
            {
                DWORD dw = 0;
                cbWanted = 4;
                cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                            pInt->hdr_parse.currPacketOffset + pParseInfoEx->cbParseOffset,
                                            cbWanted, &pData);
                if (cbActual != cbWanted)
                {
                    return WMAERR_BUFFERTOOSMALL;
                }
#if (defined(WIN32) || defined(_WIN32_WINCE) )
                if (IsBadReadPtr(pData, cbActual))
                {
                    return WMAERR_BUFFERTOOSMALL;
                }
#endif
                GetUnalignedDword(pData, dw);
                pParseInfoEx->cbPadding =  dw;
                pParseInfoEx->cbParseOffset += 4;
                break;
            }
        default:
            cbWanted = 0;
            pParseInfoEx->cbPadding = 0;
    }




    /*    pParseInfoEx->cbPadLenOffset = pParseInfoEx->cbParseOffset;
    pParseInfoEx->cbPadding = GetASFVarField(pParseInfoEx->bPadLenType,
                                             &pData[pParseInfoEx->cbParseOffset],
                                             &pParseInfoEx->cbParseOffset);
    */

/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Now read 6 bytes include send time(dword) and duration(word) in payload parse info

    cbWanted = 6;
    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                pInt->hdr_parse.currPacketOffset + pParseInfoEx->cbParseOffset,
                                cbWanted, &pData);
    if (cbActual != cbWanted)
    {
        return WMAERR_BUFFERTOOSMALL;
    }
#if (defined(WIN32) || defined(_WIN32_WINCE) )
    if (IsBadReadPtr(pData, cbActual))
    {
        return WMAERR_BUFFERTOOSMALL;
    }
#endif
////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////
    /*dwSCR=send time in payload parse info*/
    GetUnalignedDword(&pData[cbLocalOffset], pParseInfoEx->dwSCR);
//    GetUnalignedDword(&pData[pParseInfoEx->cbParseOffset], pParseInfoEx->dwSCR);

/////////////////////////////////////////////////////////////////////////////////

    pParseInfoEx->cbParseOffset += 4;


//////////////////////////////////////////////////////////////////////////////////
    /*wDuration=duration in payload parse info*/
    GetUnalignedWord(&pData[cbLocalOffset+4], pParseInfoEx->wDuration);

//    GetUnalignedWord(&pData[pParseInfoEx->cbParseOffset], pParseInfoEx->wDuration);

////////////////////////////////////////////////////////////////////////////////////

    pParseInfoEx->cbParseOffset += 2;


    /* ParsePacketEx begins */
    /* enter decoding payload data*/
    pParseInfoEx->cbPayLenTypeOffset = 0;
    pParseInfoEx->bPayLenType = 0;
    pParseInfoEx->bPayBytes = 0;
    pParseInfoEx->cPayloads = 1;

    /* firstly,please judge if fmutipayloads is 1.*/
    if (pParseInfoEx->fMultiPayloads)
    {
//////////////////////////////////////////////////////
        cbWanted = 1;
        cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                    pInt->hdr_parse.currPacketOffset + pParseInfoEx->cbParseOffset,
                                    cbWanted, &pData);
        if (cbActual != cbWanted)
        {
            return WMAERR_BUFFERTOOSMALL;
        }
#if (defined(WIN32) || defined(_WIN32_WINCE) )
        if (IsBadReadPtr(pData, cbActual))
        {
            return WMAERR_BUFFERTOOSMALL;
        }
#endif


//        b = pData[pParseInfoEx->cbParseOffset];
        b = pData[cbLocalOffset];
//////////////////////////////////////////////////////
        pParseInfoEx->cbPayLenTypeOffset = pParseInfoEx->cbParseOffset;

        pParseInfoEx->bPayLenType = b>>6;//changed   by  wjr
        if (pParseInfoEx->bPayLenType != 3
                &&pParseInfoEx->bPayLenType != 2
                && pParseInfoEx->bPayLenType != 1)
        {
            printf("\n b= 0x%x b>>6 = %d \n",b,b>>6);
            return WMAERR_FAIL;
        }

        /* payload length type(bits 6-7) in multiple payload*/
        pParseInfoEx->bPayBytes = pParseInfoEx->bPayLenType;

        pParseInfoEx->cPayloads = (DWORD)(b & 0x3f);//number of payloads(bits 0-5) in multiple payload
        if (pParseInfoEx->cPayloads == 0)
        {
            return WMAERR_FAIL;
        }

        pParseInfoEx->cbParseOffset++;
    }

    return WMAERR_OK;
}


/****************************************************************************/
WMAERR
WMA_ParsePayloadHeader(tWMAFileStateInternal *pInt)
{
    DWORD cbDummy = 0;
    DWORD cbParseOffset = 0;
    DWORD cbRepDataOffset = 0;
    DWORD dwPayloadSize = 0;
    PACKET_PARSE_INFO_EX *pParseInfoEx = NULL;
    PAYLOAD_MAP_ENTRY_EX *pPayload = NULL;
    DWORD cbLocalOffset = 0;

    BYTE *pData = NULL;
    DWORD cbActual = 0;
    DWORD cbWanted = 0;
    WORD wTotalDataBytes = 0;

    if (pInt == NULL)
    {
        return WMAERR_INVALIDARG;
    }

    pParseInfoEx = &pInt->ppex;
    pPayload = &pInt->payload;


    cbWanted = 2;              /* at least */
    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                pInt->hdr_parse.currPacketOffset + pParseInfoEx->cbParseOffset,
                                cbWanted, &pData);
    if (cbActual != cbWanted)
    {
        return WMAERR_BUFFERTOOSMALL;
    }
#if (defined(WIN32) || defined(_WIN32_WINCE) )
    if (IsBadReadPtr(pData, cbActual))
    {
        return WMAERR_BUFFERTOOSMALL;
    }
#endif


    cbParseOffset = 0;

    /* Loop in ParsePacketAndPayloads */

    pPayload->cbPacketOffset = (WORD)pParseInfoEx->cbParseOffset;

    /*stream number(bits 0-6) in payload(header)*/
    pPayload->bStreamId = (pData[cbParseOffset]) & 0x7f; // Amit to get correct Streamid

    /*Media object number(Byte) in payload(header)*/
    pPayload->bObjectId = pData[cbParseOffset + 1];

    cbDummy = 0;
    switch (pParseInfoEx->bOffsetLenType)
    {
        case 0x01:
            {
                cbWanted = 1;
                cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                            pInt->hdr_parse.currPacketOffset + pParseInfoEx->cbParseOffset + 2,
                                            cbWanted, &pData);
                if (cbActual != cbWanted)
                {
                    return WMAERR_BUFFERTOOSMALL;
                }
//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//            if (IsBadReadPtr(pData, cbActual))
//            {
//                return WMAERR_BUFFERTOOSMALL;
//            }
//#endif


                pPayload->cbObjectOffset = (DWORD)(*pData);
                break;
            }
        case 0x02:
            {
                WORD w = 0;
                cbWanted = 2;
                cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                            pInt->hdr_parse.currPacketOffset + pParseInfoEx->cbParseOffset + 2,
                                            cbWanted, &pData);
                if (cbActual != cbWanted)
                {
                    return WMAERR_BUFFERTOOSMALL;
                }
//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//            if (IsBadReadPtr(pData, cbActual))
//            {
//                return WMAERR_BUFFERTOOSMALL;
//            }
//#endif

                GetUnalignedWord(pData, w);
                pPayload->cbObjectOffset = (DWORD)(w);

                break;

            }
        case 0x03:
            {
                DWORD dw = 0;
                cbWanted = 4;
                cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                            pInt->hdr_parse.currPacketOffset + pParseInfoEx->cbParseOffset + 2,
                                            cbWanted, &pData);
                if (cbActual != cbWanted)
                {
                    return WMAERR_BUFFERTOOSMALL;
                }
//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//            if (IsBadReadPtr(pData, cbActual))
//            {
//                return WMAERR_BUFFERTOOSMALL;
//            }
//#endif

                GetUnalignedDword(pData, dw);
                /*Offset into media object(dword) in payload(header)*/
                pPayload->cbObjectOffset =  dw;
                break;
            }
        default:
            cbWanted = 0;
    }


    cbRepDataOffset = cbParseOffset + 2 + pParseInfoEx->bOffsetBytes;


    cbWanted = 1;
    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                pInt->hdr_parse.currPacketOffset + pParseInfoEx->cbParseOffset + cbRepDataOffset,
                                cbWanted, &pData);

    if (cbActual != cbWanted)
    {
        return WMAERR_BUFFERTOOSMALL;
    }
//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//    if (IsBadReadPtr(pData, cbActual))
//    {
//        return WMAERR_BUFFERTOOSMALL;
//    }
//#endif

    /*Replicated data length(byte) in payload(header)*/
    pPayload->cbRepData = pData[cbLocalOffset];

    pPayload->msObjectPres = 0xffffffff;

    if (pPayload->cbRepData == 1)//if cbrepdata is 1,then the payload is compressed payload
    {
        pPayload->msObjectPres = pPayload->cbObjectOffset;
        pPayload->cbObjectOffset = 0;
        pPayload->cbObjectSize = 0;
        pPayload->bIsCompressedPayload = 1;

        cbWanted = 1;
        cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                    pInt->hdr_parse.currPacketOffset + pParseInfoEx->cbParseOffset + cbRepDataOffset + 1,
                                    cbWanted, &pData);
        if (cbActual != cbWanted)
        {
            return WMAERR_BUFFERTOOSMALL;
        }
//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//        if (IsBadReadPtr(pData, cbActual))
//        {
//            return WMAERR_BUFFERTOOSMALL;
//        }
//#endif

        pPayload->dwDeltaTime = pData[0];

        if (pParseInfoEx->fMultiPayloads)
        {
            cbWanted = 2;
            cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                        pInt->hdr_parse.currPacketOffset + pParseInfoEx->cbParseOffset + cbRepDataOffset + 2,
                                        cbWanted, &pData);

            if (cbActual != cbWanted)
            {
                return WMAERR_BUFFERTOOSMALL;
            }
//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//            if (IsBadReadPtr(pData, cbActual))
//            {
//                return WMAERR_BUFFERTOOSMALL;
//            }
//#endif


            GetUnalignedWord(&pData[cbLocalOffset], wTotalDataBytes);
        }
        else
        {
            wTotalDataBytes = 0;
        }
    }
    else if (pPayload->cbRepData >= 8)
    {

        cbWanted = 8;
        cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                    pInt->hdr_parse.currPacketOffset + pParseInfoEx->cbParseOffset + cbRepDataOffset + 1,
                                    cbWanted, &pData);
        if (cbActual != cbWanted)
        {
            return WMAERR_BUFFERTOOSMALL;
        }
//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//        if (IsBadReadPtr(pData, cbActual))
//        {
//            return WMAERR_BUFFERTOOSMALL;
//        }
//#endif

        GetUnalignedDword(&pData[cbLocalOffset],
                          pPayload->cbObjectSize);//media object size(dword)in replicate data
        GetUnalignedDword(&pData[cbLocalOffset+ 4],
                          pPayload->msObjectPres);//pres time(dword)in replicate data

        pPayload->bIsCompressedPayload = 0;

#ifdef WMDRM_NETWORK
        if (pInt->bHasWMDRMNetworkDRM)
        {
            DWORD cbPayloadExtension;
            DWORD dwIndex, i;

            //
            // First figure out which payload extension systems apply to this
            // data.
            //
            for (dwIndex = 0;
                    dwIndex < WMA_MAX_PAYLOAD_EXTENSION_STREAMS;
                    dwIndex++)
            {
                if (pInt->hdr_parse.PEData[ dwIndex ].wStreamNum ==
                        (WORD) pPayload->bStreamId)
                {
                    break;
                }
            }
            if (WMA_MAX_PAYLOAD_EXTENSION_STREAMS == dwIndex)
            {
                return WMAERR_CORRUPTDATA;
            }

            cbPayloadExtension = pPayload->cbRepData - 8;

            cbActual = WMAFileCBGetData(
                           (tHWMAFileState *)pInt,
                           pInt->hdr_parse.currPacketOffset + pParseInfoEx->cbParseOffset + cbRepDataOffset + 1 + 8,
                           cbPayloadExtension,
                           &pData);
            if (cbActual != cbPayloadExtension)
            {
                return WMAERR_BUFFERTOOSMALL;
            }

            for (i = 0; i < WMA_MAX_PAYLOAD_EXTENSIONS; i++)
            {
                //
                // Scan through all of the extensions that apply to this
                // sample.
                //
                WMA_PayloadExtensionSystem *pPES;
                WORD cbDataSize;

                pPES = &pInt->hdr_parse.PEData[ dwIndex ].Systems[ i ];

                if (pPES->cbDataSize == 0xffff)
                {
                    //
                    // This is a variable size extension; read in the size.
                    //
                    if (cbPayloadExtension < sizeof(WORD))
                    {
                        return WMAERR_BUFFERTOOSMALL;
                    }
                    LoadWORD(cbDataSize, pData);
                    cbPayloadExtension -= sizeof(WORD);
                }
                else
                {
                    //
                    // Fixed-size extension; we know the size from the ASF
                    // header.
                    //
                    cbDataSize = pPES->cbDataSize;
                }

                if (cbPayloadExtension < cbDataSize)
                {
                    return WMAERR_BUFFERTOOSMALL;
                }

                if (pPES->bIsSampleId)
                {
                    //
                    // We found the sample ID; go ahead and read it in.
                    //
                    QWORD qwSampleID;
                    WORD j;

                    qwSampleID.dwHi = qwSampleID.dwLo = 0;

                    if (cbDataSize > 8)
                    {
                        return WMAERR_FAIL;
                    }

                    for (j = 0; j < cbDataSize; j++)
                    {
                        qwSampleID.dwHi =
                            (qwSampleID.dwHi << 8) +
                            (qwSampleID.dwLo >> 24);
                        qwSampleID.dwLo =
                            (qwSampleID.dwLo << 8) + pData[ j ];
                    }

                    pInt->qwWMDRMNetworkSampleID = qwSampleID;

                    break;
                }

                pData += cbDataSize;
            }
        }
#endif
    }

    pPayload->cbTotalSize = 1 + 1 + pParseInfoEx->bOffsetBytes + 1 + pPayload->cbRepData;

    if (pParseInfoEx->fMultiPayloads)
    {
        cbDummy = 0;

        switch (pParseInfoEx->bPayLenType)
        {
            case 0x01:
                {
                    cbWanted = 1;
                    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                                pInt->hdr_parse.currPacketOffset + pParseInfoEx->cbParseOffset + pPayload->cbTotalSize,
                                                cbWanted, &pData);
                    if (cbActual != cbWanted)
                    {
                        return WMAERR_BUFFERTOOSMALL;
                    }
//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//                if (IsBadReadPtr(pData, cbActual))
//                {
//                    return WMAERR_BUFFERTOOSMALL;
//                }
//#endif

                    dwPayloadSize = (DWORD)(*pData);
                    break;
                }
            case 0x02:
                {
                    WORD w = 0;
                    cbWanted = 2;
                    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                                pInt->hdr_parse.currPacketOffset + pParseInfoEx->cbParseOffset + pPayload->cbTotalSize,
                                                cbWanted, &pData);
                    if (cbActual != cbWanted)
                    {
                        return WMAERR_BUFFERTOOSMALL;
                    }
//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//                if (IsBadReadPtr(pData, cbActual))
//                {
//                    return WMAERR_BUFFERTOOSMALL;
//                }
//#endif

                    GetUnalignedWord(pData, w);
                    dwPayloadSize = (DWORD)(w);

                    break;

                }
            case 0x03:
                {
                    DWORD dw = 0;
                    cbWanted = 4;
                    cbActual = WMAFileCBGetData((tHWMAFileState *)pInt,
                                                pInt->hdr_parse.currPacketOffset + pParseInfoEx->cbParseOffset + pPayload->cbTotalSize,
                                                cbWanted, &pData);
                    if (cbActual != cbWanted)
                    {
                        return WMAERR_BUFFERTOOSMALL;
                    }
//#if (defined(WIN32) || defined(_WIN32_WINCE) )
//                if (IsBadReadPtr(pData, cbActual))
//                {
//                    return WMAERR_BUFFERTOOSMALL;
//                }
//#endif

                    GetUnalignedDword(pData, dw);
                    dwPayloadSize =  dw;
                    break;
                }
            default:
                cbWanted = 0;
                dwPayloadSize = 0;
        }


    }
    else if (pParseInfoEx->cbExplicitPacketLength > 0)
    {
        dwPayloadSize = pParseInfoEx->cbExplicitPacketLength
                        - pParseInfoEx->cbParseOffset
                        - pPayload->cbTotalSize
                        - pParseInfoEx->cbPadding;
    }
    else
    {
        dwPayloadSize = pInt->hdr_parse.cbPacketSize
                        - pParseInfoEx->cbParseOffset
                        - pPayload->cbTotalSize
                        - pParseInfoEx->cbPadding;
    }
    if (0 == wTotalDataBytes)
        wTotalDataBytes = (WORD) dwPayloadSize;

    pPayload->cbPayloadSize = (WORD)dwPayloadSize;

    pPayload->cbTotalSize += pParseInfoEx->bPayBytes
                             + (WORD)pPayload->cbPayloadSize;

    pPayload->wTotalDataBytes = wTotalDataBytes; // Amit

    /*    if( 1 == pPayload->cbRepData )
        {
            pPayload->cbPayloadSize--;
        }
    */
    pParseInfoEx->cbParseOffset += pPayload->cbTotalSize;

    if (pParseInfoEx->cbParseOffset > pInt->hdr_parse.cbPacketSize
            || (pParseInfoEx->cbParseOffset == pInt->hdr_parse.cbPacketSize
                && pInt->iPayload < pParseInfoEx->cPayloads - 1))
    {
        return WMAERR_CORRUPTDATA;
    }

    return WMAERR_OK;
}
#pragma arm section code

#endif
#endif
#endif
