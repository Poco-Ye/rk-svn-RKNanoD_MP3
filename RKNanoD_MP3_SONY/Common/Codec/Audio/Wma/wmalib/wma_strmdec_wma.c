//*@@@+++@@@@******************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//*@@@---@@@@******************************************************************
/*************************************************************************

Copyright (C) 1996-1999  Microsoft Corporation

Module Name:

 strmdec_wma.c

Abstract:

 Decoder BitStream

Author:

 Craig Dowell (craigdo@microsoft.com) 10-December-1996
 Ming-Chieh Lee (mingcl@microsoft.com) 10-December-1996
 Bruce Lin (blin@microsoft.com) 10-December-1996

Revision History:
    Wei-ge Chen (wchen@microsoft.com) 20-July-1999
    Make it in C.


*************************************************************************/
#include "../include/audio_main.h"

#ifdef A_CORE_DECODE
#ifdef WMA_DEC_INCLUDE
//#include <stdio.h>
//#include <stdlib.h>
#include "..\wmaInclude\macros.h"
#include "..\wmaInclude\strmdec_wma.h"
#include "..\wmaInclude\msaudiodec.h"

#if!defined(S_SUNOS5) && !WMA_OPT_STRMDEC_ARM

/*
const UInt getMask[33] =  //STATIC TABLEffff
{
    0x00000000,
    0x00000001,
    0x00000003,
    0x00000007,
    0x0000000f,
    0x0000001f,
    0x0000003f,
    0x0000007f,
    0x000000ff,
    0x000001ff,
    0x000003ff,
    0x000007ff,
    0x00000fff,
    0x00001fff,
    0x00003fff,
    0x00007fff,
    0x0000ffff,
    0x0001ffff,
    0x0003ffff,
    0x0007ffff,
    0x000fffff,
    0x001fffff,
    0x003fffff,
    0x007fffff,
    0x00ffffff,
    0x01ffffff,
    0x03ffffff,
    0x07ffffff,
    0x0fffffff,
    0x1fffffff,
    0x3fffffff,
    0x7fffffff,
    0xffffffff
};
*/
#endif

#ifdef WMAINITIALIZE_HIGH_MID_ALL_USED
#pragma arm section code = "WmaCommonCode"

Void WMA_set_PacketLoss(CAudioObjectDecoder* hWMA, Bool fPacketLoss)
{
    hWMA->m_fPacketLoss = fPacketLoss;
}

U8 WMA_get_nHdrBits(CAudioObjectDecoder* hWMA, U32 dwHdr)
{
    const CAudioObject *pau = ((CAudioObjectDecoder *)hWMA)->pau;
    if (pau->m_iVersion <= 2)
    {
        if (pau->m_fAllowSuperFrame)
            return (U8)(pau->m_cBitPackedFrameSize + NBITS_FRM_CNT + NBITS_PACKET_CNT + 3);
        else
            return 0;
    }
//    else
    //    {
    //  //        I32 cBitLs = NBITS_PACKET_CNT + NBITS_PACKET_EMPTYNESS;
    //  /*
    //        U8 iVal = (U8) ((dwHdr << cBitLs) >> (BITS_PER_DWORD - pau->m_cBitPackedFrameSize));
    //        if (iVal == 0)
    //  iVal = 1;
    //        return (U8) (pau->m_cBitPackedFrameSize + NBITS_PACKET_CNT + NBITS_PACKET_EMPTYNESS + iVal) ;
    //  */
    //        if (pau->m_fExtraBitsInPktHeader)
    //            return (U8) (pau->m_cBitPackedFrameSize + NBITS_PACKET_CNT + NBITS_PACKET_EMPTYNESS + NBITS_FORCE_PACKETLOSS + 11) ;
    //        else
    //            return (U8) (pau->m_cBitPackedFrameSize + NBITS_PACKET_CNT + NBITS_PACKET_EMPTYNESS + NBITS_FORCE_PACKETLOSS) ;
    //    }
    return 0;

}
#pragma arm section code

#endif
#ifdef  WMAINITIALIZE
#pragma arm section code = "WmaOpenCodecCode"
Void ibstrmInit(CWMAInputBitStream* pibstrm, CAudioObjectDecoder* pOwner)
{
    //added for streaming mode
    pibstrm->m_pfnGetMoreData = NULL;
    pibstrm->m_pUser  = 0;
    pibstrm->m_dwOwner = pOwner;
    pibstrm->m_dwHeaderBuf   = 0;
    pibstrm->m_dwHeaderBufTemp   = 0;
    pibstrm->m_pBufferBegin  = NULL;
    pibstrm->m_cbBuflenBegin = 0;    // used in association of m_pBufferBegin
    //end of streaming mode

    pibstrm->m_pBuffer = NULL;
    pibstrm->m_iPrevPacketNum = (1 << NBITS_PACKET_CNT) - 1; // Keep -1 spacing w/ curr pkt num
    pibstrm->m_fAllowPackets = pOwner->pau->m_fAllowSuperFrame;
    pibstrm->m_fSuppressPacketLoss = WMAB_TRUE; // Suppress first packet from loss detection

    ibstrmReset(pibstrm);
}
#pragma arm section code
#endif
#ifdef WMAINITIALIZE_HIGH_MID_ALL_USED
#pragma arm section code = "WmaCommonCode"
Void ibstrmReset(CWMAInputBitStream *pibstrm)
{
    pibstrm->m_dwDot = 0;
    pibstrm->m_dwBitsLeft = 0;
    pibstrm->m_dwDotT = 0;
    pibstrm->m_cBitDotT = 0;
    pibstrm->m_cbBuflen = 0;

    //if (pibstrm->m_dwOwner->m_fSPDIF)
    //    {
    //        pibstrm->m_fDeferredGap = WMAB_FALSE; // cancel any currently scheduled gap
    //        pibstrm->m_fNoMoreInput = WMAB_FALSE; // not appropriate for non-SPDIF reset
    //
    //        // reset these to prevent false positives in ibstrmMoreInputAfterGap()
    //        pibstrm->m_pBuffer = NULL;
    //        pibstrm->m_cbBuflen = 0;
    //        pibstrm->m_pBufferBegin  = NULL;
    //        pibstrm->m_cbBuflenBegin = 0;
    //
    //        // This initialization is not strictly necessary.  In fact,
    //        // 0 is not even a good value for this variable.
    //        // The real initialization happens in ibstrmSetGap(),
    //        // when processing the frame header.
    //        pibstrm->m_cbLastProcessedGapLocationBackFromEndOfLastBuffer = 0;
    //    }

}

/*
Void ibstrmAttach(CWMAInputBitStream *pibstrm, U32 dwDot, U32 dwBitsLeft, U8* pbSrc, I32 cbSrc)
{
    //added for streaming mode
    pibstrm->m_dwHeaderBuf   = 0;
    pibstrm->m_dwHeaderBufTemp   = 0;
    pibstrm->m_pBufferBegin  = NULL;
    pibstrm->m_cbBuflenBegin = 0;    // used in association of m_pBufferBegin
    //end of streaming mode

    pibstrm->m_pBuffer = pbSrc;
    pibstrm->m_cbBuflen = cbSrc;
    pibstrm->m_dwDot = dwDot;
    pibstrm->m_dwBitsLeft = dwBitsLeft;
}
*/
WMARESULT ibstrmAttach(CWMAInputBitStream *pibs, U8* pbSrc, I32 cbSrc,
                       Bool fNewPacket, Bool fNoMoreInput, Bool fSPDIF, Int iVersion)
{
    I32 iFirstBit = 0;
    WMARESULT   wmaResult = WMA_OK;

    assert(pibs->m_cbBuflen == 0);

    if ((cbSrc == 0) || (fNoMoreInput))
    {
        pibs->m_fNoMoreInput = WMAB_TRUE;
    }

    if (cbSrc <= 0)
    {
        wmaResult = WMA_OK;
        goto exit;
    }

    pibs->m_pBuffer = pbSrc;
    pibs->m_cbBuflen = cbSrc;



    if (fNewPacket || fSPDIF)
    {
        pibs->m_pBufferBegin = pibs->m_pBuffer;//input buffer of wma file begins
        pibs->m_cbBuflenBegin = pibs->m_cbBuflen;//length of input buffer
    }

    //if (fSPDIF)
    //    {
    //        if (0 != WMA_get_nHdrBits(pibs->m_dwOwner, 0) % 8) // no gaps if pkt hdr is a multiple of 8 bits
    //        {   // Update the position of the last processed gap, since the position is
    //            // relative to the end of the most recent input buffer, and we have just
    //            // received a new input buffer.
    //            pibs->m_cbLastProcessedGapLocationBackFromEndOfLastBuffer += pibs->m_cbBuflen;
    //        }
    //
    //        if (pibs->m_fDeferredGap)
    //        {   // A gap was detected during a previous buffer but the gap's location was
    //            // beyond the end of that buffer.  See if it falls within the current buffer.
    //            if ((U32)cbSrc > pibs->m_cbToDeferredGap) {
    //                assert(pibs->m_pBuffer == pibs->m_pBufferBegin); // just set above
    //                assert(pibs->m_cbBuflen == pibs->m_cbBuflenBegin); // just set above
    //                assert(pibs->m_cbBuflen > pibs->m_cbToDeferredGap);
    //
    //                pibs->m_cbBuflen = pibs->m_cbToDeferredGap;
    //                pibs->m_fDeferredGap = WMAB_FALSE; // just took care of it
    //            }
    //            else
    //            { // reschedule the gap for a later buffer
    //                pibs->m_cbToDeferredGap -= cbSrc;
    //            }
    //        }
    //    }



    if (WMAB_FALSE == pibs->m_fAllowPackets)
    {
        // In non-superframe mode, provide a running packet count that wraps around (to avoid I32 vs. U32 issues)
        if (fNewPacket)
        {
            assert(((~(NONSUPER_WRAPAROUND - 1) << 1) & NONSUPER_WRAPAROUND) == 0); // assert Pwr of 2
            pibs->m_dwHeaderBuf = (pibs->m_dwHeaderBuf + 1) & (NONSUPER_WRAPAROUND - 1);
        }
    }
    else if (fNewPacket && !fSPDIF) //set up packet header in superframe mode
    {
        Int iPrevPacketNum, iNextPacketNum;
        unsigned char nHdrBits;
        const int cPacketNumBitsRS = (BITS_PER_DWORD - NBITS_PACKET_CNT);
        const int cPacketForcePacketLossRS = (BITS_PER_DWORD - NBITS_PACKET_CNT - NBITS_PACKET_EMPTYNESS - NBITS_FORCE_PACKETLOSS);

        assert(pibs->m_pBuffer != NULL);  //always get a valid one
        pibs->m_dwHeaderBuf = pibs->m_dwHeaderBufTemp; //move a new one into the queue
        pibs->m_dwHeaderBufTemp = 0;
        pibs->m_dwHeaderBufTemp = (pibs->m_pBuffer[0] << 24) | (pibs->m_pBuffer[1] << 16) | (pibs->m_pBuffer[2] << 8) | pibs->m_pBuffer[3];

        nHdrBits = WMA_get_nHdrBits(pibs->m_dwOwner, pibs->m_dwHeaderBufTemp); // this only updates per file for v2; varialb for v3
        iFirstBit = nHdrBits % 8;
        pibs->m_pBuffer += nHdrBits / 8;
        pibs->m_cbBuflen -= nHdrBits / 8;
        assert(nHdrBits <= BITS_PER_DWORD);
//        assert (pibs->m_dwBitsLeft <= 24);  //so that we have enough to save the fractional byte that would otherwise be lost

        // Now we should check that we didn't lose a packet
        iNextPacketNum = (pibs->m_dwHeaderBufTemp >> cPacketNumBitsRS);
        iPrevPacketNum = pibs->m_iPrevPacketNum;
        pibs->m_iPrevPacketNum = iNextPacketNum;
        if (WMAB_FALSE == pibs->m_fSuppressPacketLoss)
        {
            Bool bForcedPacketLoss = WMAB_FALSE;
            if (iVersion > 2 &&
                    (pibs->m_dwHeaderBufTemp >> cPacketForcePacketLossRS) & 0X1)
            {
                bForcedPacketLoss = WMAB_TRUE;
            }

            if ((WMAB_FALSE == (iNextPacketNum - iPrevPacketNum == 1 ||
//                iNextPacketNum - iPrevPacketNum == 0 ||
                                iNextPacketNum - iPrevPacketNum + (1 << NBITS_PACKET_CNT) == 1))
                    || bForcedPacketLoss)
            {
                // PACKET LOSS: Return error. Next call to DecodeInfo will cue to next frame
                pibs->m_pBuffer = pibs->m_pBufferBegin + 4;
                pibs->m_cbBuflen = pibs->m_cbBuflenBegin - 4;

                pibs->m_dwDot = pibs->m_dwHeaderBufTemp;
                assert(nHdrBits <= 32);
                pibs->m_dwBitsLeft = 32 - nHdrBits;

                WMA_set_PacketLoss(pibs->m_dwOwner, WMAB_TRUE);
                TRACEWMA_EXIT(wmaResult, WMA_S_LOSTPACKET);
                goto exit;
            }
            else
            {
                WMA_set_PacketLoss(pibs->m_dwOwner, WMAB_FALSE);
            }
        }
        else
            // Avoid checking for packet loss, eg, after a seek
            pibs->m_fSuppressPacketLoss = WMAB_FALSE;
    }

    ibstrmLoadBits(pibs, iFirstBit);

exit:
    return wmaResult;
}

#if !WMA_OPT_STRMDEC_ARM
void ibstrmLoadBits(CWMAInputBitStream *pibs, I32 iFirstBit)//get bits from input stream
{
    U8 temp;
    if (pibs->m_cBitDotT == 0 && pibs->m_dwBitsLeft + (8 - iFirstBit) <= 32)
    {
        while (pibs->m_cbBuflen > 0 && (pibs->m_dwBitsLeft + (8 - iFirstBit)) <= 32)
        {
            temp = (*pibs->m_pBuffer++);//get data(1 byte)
            pibs->m_cbBuflen--;

            //zeros out the top (not valid) bits
            temp = temp << iFirstBit;
            temp = temp >> iFirstBit;
            pibs->m_dwDot = (pibs->m_dwDot << (8 - iFirstBit)) | temp;//two data(bytes) are composed to a word

            pibs->m_dwBitsLeft += (8 - iFirstBit);
            iFirstBit = 0;
        }
    }
    else
    {
        assert(pibs->m_cBitDotT + 8 - iFirstBit <= 32);
        temp = (*pibs->m_pBuffer++);
        pibs->m_cbBuflen--;
        temp = temp << iFirstBit;
        temp = temp >> iFirstBit;
        pibs->m_dwDotT = (pibs->m_dwDotT << (8 - iFirstBit)) | temp;
        pibs->m_cBitDotT += (8 - iFirstBit);
    }

}
#endif //WMA_OPT_STRMDEC_ARM
#pragma arm section code
#endif
#define LOAD_BITS_FROM_DotT \
if (pibstrm->m_cBitDotT > 0) \
{ \
    I32 cBitMoved = min (32 - pibstrm->m_dwBitsLeft, pibstrm->m_cBitDotT); \
    pibstrm->m_dwDot <<= cBitMoved; \
    pibstrm->m_cBitDotT -= cBitMoved; \
    pibstrm->m_dwDot |= (pibstrm->m_dwDotT >> pibstrm->m_cBitDotT); \
    pibstrm->m_dwDotT &= ((I32) (1 << pibstrm->m_cBitDotT)) - 1; \
    pibstrm->m_dwBitsLeft += cBitMoved; \
}

#define LOAD_BITS_FROM_STREAM \
while (pibstrm->m_dwBitsLeft <= 24 && pibstrm->m_cbBuflen > 0) { \
    pibstrm->m_dwDot <<= 8; \
    pibstrm->m_dwDot |= *(pibstrm->m_pBuffer)++; \
    --(pibstrm->m_cbBuflen); \
    pibstrm->m_dwBitsLeft += 8; \
}

#define LOAD_BITS_INTO_DotT \
assert(pibstrm->m_dwBitsLeft > 24 || 0 == pibstrm->m_cbBuflen); \
pibstrm->m_dwDotT = 0; \
while (pibstrm->m_cbBuflen > 0) \
{ \
    pibstrm->m_dwDotT <<= 8; \
    pibstrm->m_dwDotT |= *(pibstrm->m_pBuffer)++; \
    --(pibstrm->m_cbBuflen); \
    pibstrm->m_cBitDotT += 8; \
} \
assert (pibstrm->m_cBitDotT <= 24); // we only take this branch if we have <56 bits left total

#if !WMA_OPT_STRMDEC_ARM
#ifdef WMAINITIALIZE_HIGH_MID_ALL_USED
#pragma arm section code = "WmaCommonCode"
WMARESULT ibstrmPeekBits(CWMAInputBitStream *pibstrm, register UInt dwNumBits,
                         U32 *piRetBits)
{
    I16 cBitExtra;
    WMARESULT   wmaResult = WMA_OK;
    assert(dwNumBits <= 24);  //only works for sure under this
    if (pibstrm->m_dwBitsLeft < dwNumBits)
    {
        //make sure there is enougth data in dwDot for peek
        LOAD_BITS_FROM_DotT;
        LOAD_BITS_FROM_STREAM;

        //if enough take the data and go home; else take what ever is left
        if (pibstrm->m_dwBitsLeft < dwNumBits)
        {
            TRACEWMA_EXIT(wmaResult, ibstrmGetMoreData(pibstrm, ModePeek, dwNumBits));
            if (pibstrm->m_dwBitsLeft < dwNumBits)
            {
                assert(pibstrm->m_fNoMoreInput);
                dwNumBits = pibstrm->m_dwBitsLeft;
            }
        }
    }

    cBitExtra = (I16) pibstrm->m_dwBitsLeft - (I16) dwNumBits;
    assert(NULL != piRetBits); // Avoid conditionals
    *piRetBits = (pibstrm->m_dwDot >> cBitExtra) << (32 - dwNumBits);
    //DEBUG("%d: Peek(%d)=%08X\n", pibstrm->m_cFrmBitCnt, dwNumBits, *piRetBits);

exit:
    return wmaResult;
}
#pragma arm section code
#endif
#if defined(WMAHIGHRATE) || defined(WMAMIDRATELOWRATE)
#pragma arm section code = "WmaHighLowCommonCode"
WMARESULT ibstrmLookForBits(CWMAInputBitStream *pibstrm, UInt dwNumBits)
{
    WMARESULT   wmaResult = WMA_OK;
    assert(dwNumBits <= 56); // we need to make sure that there is enough space to put the first byte of data from the new packet. by Hong
    if (dwNumBits > pibstrm->m_dwBitsLeft + pibstrm->m_cBitDotT + pibstrm->m_cbBuflen * 8)
    {
        //load up everything so that we can return and ext. buf may be overwritten
        LOAD_BITS_FROM_STREAM;
        LOAD_BITS_INTO_DotT;
        TRACEWMA_EXIT(wmaResult, ibstrmGetMoreData(pibstrm, ModeLookFor, dwNumBits));
    }

exit:
    return wmaResult;
}
#pragma arm section code
#endif
#ifdef WMAINITIALIZE_HIGH_MID_ALL_USED
#pragma arm section code = "WmaCommonCode"
WMARESULT ibstrmFlushBits(CWMAInputBitStream *pibstrm, register UInt dwNumBits)
{

    WMARESULT wmaResult = WMA_OK;
    assert(dwNumBits <= 24); // this is to make sure there is enough buffer space when a new packet comes in. Hong

    assert(dwNumBits <= 24);  //only works for sure under this
    //make sure there is enougth data in dwDot for peek
    if (pibstrm->m_dwBitsLeft < dwNumBits)
    {
        LOAD_BITS_FROM_DotT;
        LOAD_BITS_FROM_STREAM;

        if (pibstrm->m_dwBitsLeft < dwNumBits)
        {
            TRACEWMA_EXIT(wmaResult, ibstrmGetMoreData(pibstrm, ModeGetFlush, dwNumBits));
        }
    }

    //take the data and go home; or we have to pause
    pibstrm->m_dwBitsLeft -= dwNumBits;
//    DEBUG_ONLY(pibstrm->m_cFrmBitCnt += dwNumBits;)

exit:
    return wmaResult;
}
#pragma arm section code
#endif
#endif //WMA_OPT_STRMDEC_ARM

//void ibstrmProcessGap(CWMAInputBitStream *pibs)
//{
//    CAudioObjectDecoder* paudec = pibs->m_dwOwner;
//    U32 cHdrBitsMod8 = WMA_get_nHdrBits(paudec, 0) % 8;
//    U32 cbNextGap = paudec->pau->m_cBytePacketLength - WMA_get_nHdrBits(paudec, 0) / 8;
//
//    //WMAPrintf("gap %d bits of %02X %d+%d bits into frame %d\n", cHdrBitsMod8, *pibs->m_pBuffer, pibs->m_cFrmBitCnt, ibstrmBitsInDots(pibs), pibs->m_dwOwner->pau->m_iFrameNumber);
//
//    assert(0 != cHdrBitsMod8); // ibstrmSetGap() checks this before scheduling a gap
//    assert(!pibs->m_fDeferredGap); // We never set up the next gap before the current gap has been processed
//
//    // extend pibs->m_cbBuflen to the next gap or the end of the input buffer, whichever is sooner
//    assert(pibs->m_pBuffer >= pibs->m_pBufferBegin);
//    pibs->m_cbBuflen = pibs->m_cbBuflenBegin - (U32)(pibs->m_pBuffer - pibs->m_pBufferBegin);
//    assert(pibs->m_cbBuflen > 0);
//    pibs->m_cbLastProcessedGapLocationBackFromEndOfLastBuffer = pibs->m_cbBuflen;
//    if (pibs->m_cbBuflen > cbNextGap) // is the next gap in sight ?
//    { // yes - schedule the gap for the current buffer
//        pibs->m_cbBuflen = cbNextGap;
//    }
//    else
//    { // no - schedule the gap for a future buffer
//        pibs->m_fDeferredGap = WMAB_TRUE; // audecInput() pays attention to this
//        pibs->m_cbToDeferredGap = cbNextGap - pibs->m_cbBuflen; // 0 is a valid case
//    }
//
//    ibstrmLoadBits(pibs, cHdrBitsMod8);


#define NEED_MORE_DATA (cBitsNeeded > pibs->m_dwBitsLeft + ((mode == ModeLookFor) ? (pibs->m_cBitDotT + pibs->m_cbBuflen * 8) : 0))
#ifdef WMAINITIALIZE_HIGH_MID_ALL_USED
#pragma arm section code = "WmaCommonCode"
WMARESULT ibstrmGetMoreData(CWMAInputBitStream *pibs, GetMoreDataMode mode, U32 cBitsNeeded)
{
    WMARESULT   wmaResult = WMA_OK;
    WMA_Bool fCalledDueToGap = WMAB_FALSE;

    assert(0 == pibs->m_cbBuflen);
    assert(NEED_MORE_DATA); // otherwise why was ibstrmGetMoreData() called ?

    //if (pibs->m_dwOwner->m_fSPDIF)
    //    {
    //        assert(ModePktHdr != mode);
    //        assert(pibs->m_cbBuflenBegin <= 1048576 && pibs->m_cbBuflenBegin <= 1048576); // not a real restriction - assert just to catch suspicious values
    //        assert(pibs->m_pBuffer >= pibs->m_pBufferBegin);
    //        assert(pibs->m_pBuffer + pibs->m_cbBuflen <= pibs->m_pBufferBegin + pibs->m_cbBuflenBegin);
    //
    //        if (ibstrmMoreInputAfterGap(pibs)) // more left
    //        { // not really done with the buffer - cbBuflen was set short because of a gap
    //            fCalledDueToGap = WMAB_TRUE;
    //            ibstrmProcessGap(pibs);
    //
    //            if (NEED_MORE_DATA)
    //            {   // Make sure we have reached the end of the input buffer.  We could not have hit
    //                // yet another gap as long as gaps are >= 8 bytes apart, which they
    //                // should be because gaps occur only at packed boundaries and packets are >>8 bytes.
    //                assert(pibs->m_pBuffer == pibs->m_pBufferBegin + pibs->m_cbBuflenBegin);
    //            }
    //            else
    //            {
    //                goto exit;
    //            }
    //        }
    //    }


    assert(NEED_MORE_DATA);
    if (!pibs->m_fNoMoreInput)
    {
        audecInputBufferInfo buf;
        if (!pibs->m_pfnGetMoreData)
        {
            TRACEWMA_EXIT(wmaResult, WMA_E_ONHOLD);
        }

        memset(&buf, 0, sizeof(buf));
        TRACEWMA_EXIT(wmaResult, pibs->m_pfnGetMoreData(pibs->m_pUser, &buf));
        if (!buf.fNoMoreInput)
        {
            assert(buf.cbIn >= 8); // to guarantee 56 bits when we return (+ up to 7 bits for SPDIF gap)
        }

        TRACEWMA_EXIT(wmaResult, prvNewInputBuffer(pibs->m_dwOwner, &buf)); // calls ibstrmAttach()
//        if (pibs->m_dwBitsLeft < 24)
        //        {
        //            if (ibstrmMoreInputAfterGap(pibs)) {
        //                // must have hit a gap within the first few bytes of the new buffer
        //                assert(!fCalledDueToGap); // unexpected to hit another gap having just processed one above
        //                ibstrmProcessGap(pibs);
        //            }
        //        }


    }

    if (NEED_MORE_DATA)
    {
        if (pibs->m_fNoMoreInput)
        {
            if (mode == ModeLookFor)
            { // Sometimes LookForBits() is asked for more bits than the frame contains.
                // m_fAllBarksOn = TRUE in prvDecodeTransformOnOffInfo() is one such case.
                goto exit;
            }
            else if (mode == ModePeek)
            {
                assert(pibs->m_dwBitsLeft < 24);
                assert(0 == pibs->m_cBitDotT);
                assert(0 == pibs->m_cbBuflen);
                goto exit;
            }
            else
            {   // Trying to read past the end in GetBits or FlushBits
                // indicates a corrupt or truncated bitstream.
                // Known cases are truncated SPDIF frames & WMA files truncated to packet boundary.
                if (!((mode == ModePktHdr) || pibs->m_dwOwner->m_fSPDIF))
                {
                    REPORT_BITSTREAM_CORRUPTION();
                }
                TRACEWMA_EXIT(wmaResult, WMA_E_ONHOLD);
            }
        }
        else
        {
            assert(!pibs->m_pfnGetMoreData);
            TRACEWMA_EXIT(wmaResult, WMA_E_ONHOLD);
        }
    }

    assert(!NEED_MORE_DATA);

exit:
    return wmaResult;
}
#pragma arm section code
#endif
#if defined(WMAHIGHRATE) || defined(WMAMIDRATELOWRATE)
#pragma arm section code = "WmaHighLowCommonCode"
Void ibstrmResetPacket(CWMAInputBitStream *pibs)
{
    U8 nHdrBits = WMA_get_nHdrBits(pibs->m_dwOwner, pibs->m_dwHeaderBuf); // this only updates per file for V2
    I32 iFirstBit;
    U8 temp;

    assert(pibs->m_pBuffer != 0);

    pibs->m_dwDotT = 0;
    pibs->m_cBitDotT = 0;

    //skip the packet header
    iFirstBit = nHdrBits % 8;
    pibs->m_pBuffer = pibs->m_pBufferBegin;
    pibs->m_pBuffer += nHdrBits / 8;
    pibs->m_cbBuflen = pibs->m_cbBuflenBegin - nHdrBits / 8;

    temp = (*pibs->m_pBuffer++);
    pibs->m_cbBuflen--;

    //zeros out the top (not valid) bits
    temp = temp << iFirstBit;
    temp = temp >> iFirstBit;
    pibs->m_dwDot = temp;
    pibs->m_dwBitsLeft = (8 - iFirstBit);
}

//Bool ibstrmMoreInputAfterGap(const CWMAInputBitStream* pibstrm)
//{
//    //if (pibstrm->m_dwOwner->m_fSPDIF)
//    //    {
//    //        if (pibstrm->m_pBuffer + pibstrm->m_cbBuflen < pibstrm->m_pBufferBegin + pibstrm->m_cbBuflenBegin)
//    //        {
//    //            return WMAB_TRUE;
//    //        }
//    //        else
//    //        {
//    //            return WMAB_FALSE;
//    //        }
//    //    }
//    //    else
//
//    {
//        return WMAB_FALSE;
//    }
#pragma arm section code
#endif
//Bool ibstrmNoMoreInput(const CWMAInputBitStream* pibstrm)
//{
//    if (pibstrm->m_fNoMoreInput)
//    {
////        if (ibstrmMoreInputAfterGap(pibstrm))
//        //        {
//        //            return WMAB_FALSE;
//        //        }
//        //        else
//
//        {
//            return WMAB_TRUE;
//        }
//    }
//    else
//    {
//        return WMAB_FALSE;
//    }
//}

//Int ibstrmBitsInDots(const CWMAInputBitStream* pibstrm)
//{
//    Int cBitsInDots = pibstrm->m_dwBitsLeft + pibstrm->m_cBitDotT;
//    U32 cHdrBitsMod8 = WMA_get_nHdrBits(pibstrm->m_dwOwner, 0) % 8;
//    if (0 != cHdrBitsMod8) // no gaps if pkt hdr is a multiple of 8 bits
//    {
//        U32 cbThatThing = pibstrm->m_cbLastProcessedGapLocationBackFromEndOfLastBuffer;
//        if (pibstrm->m_pBuffer > pibstrm->m_pBufferBegin + pibstrm->m_cbBuflenBegin - cbThatThing)
//        {
//            WMA_U32 cbBytesEatenSinceGap = pibstrm->m_pBuffer - (pibstrm->m_pBufferBegin + pibstrm->m_cbBuflenBegin - cbThatThing);
//            if (cBitsInDots + cHdrBitsMod8 >= 8 * cbBytesEatenSinceGap)
//            {   // We have not read any of the bits from after the gap yet, so add the gap
//                // size to cBitsInDots because m_pBuffer was incremented when the gap was
//                // eaten, but the gap bits never went into cBitsInDots.
//                //
//                // This happens when a frame ends on a byte boundary and the frame size in
//                // the frame header is actual.  We don't know that the frame size is actual
//                // when we process the frame header, so we set up a gap just in case the
//                // frame size points to a gap.  Without the special logic here, audecDecode()
//                // would then get cbBytesInLastBuffer wrong by 1 byte because bits past the
//                // gap have already been loaded into m_dwDot/m_dwDotT, and m_pBuffer was
//                // incremented when we processed the gap on the assumption that we would
//                // need to read some bits after the gap.
//                assert(cBitsInDots + cHdrBitsMod8 == 8 * cbBytesEatenSinceGap);
//                return cBitsInDots + cHdrBitsMod8;
//            }
//
//        }
//        else
//        {
//            assert(0);
//        }
//    }
//    return cBitsInDots;



//void ibstrmSetGap(CWMAInputBitStream *pibs, U32 cBitsToGap)
//{
//    U32 cBitsLeft = ibstrmBitsLeft(pibs) + 8 * ibstrmBufLen(pibs);
//    assert(pibs->m_dwOwner->m_fSPDIF);
//    assert(!pibs->m_fDeferredGap); // this is called during FHDR_SIZE, we should not have a gap yet
//
//    if (0 == WMA_get_nHdrBits(pibs->m_dwOwner, 0) % 8) // no gap - pkt hdr is a multiple of 8 bits
//    {
//        return;
//    }
//
//    // Set this so that ibstrmBitsInDots() realizes that we have not yet processed a gap.
//    pibs->m_cbLastProcessedGapLocationBackFromEndOfLastBuffer = pibs->m_cbBuflenBegin;
//
//    // pibstrm->m_cBitDotT is only set on ONHOLD, and we shouldn't be
//    // on hold this early because we are just starting to process a frame
//    assert(0 == pibs->m_cBitDotT);
//    // still, in case somebody decides to torture ibstrmGetBits()...
//    cBitsLeft += pibs->m_cBitDotT;
//
//    // Gaps only occur on byte boundaries because they come from packet headers.
//    // If a gap appears to not be, it means the frame size is real - there is no gap.
//    if (cBitsLeft % 8 != cBitsToGap % 8)
//    {
//        return;
//    }
//
//    if (cBitsLeft > cBitsToGap)
//    { // set up for the gap to be taken care of during the current input buffer
//        U32 cbBytes = (cBitsLeft - cBitsToGap) / 8;
//        if (cbBytes <= (U32)pibs->m_cbBuflen)
//        {
//            pibs->m_cbBuflen -= cbBytes;
//        }
//        else
//        { // we've already sucked a few bytes at/beyond the gap into m_dwDot.  Discard them
//            cbBytes -= pibs->m_cbBuflen;
//            assert(cbBytes <= 2); // not necessarily a problem but unexpected
//            assert(pibs->m_dwBitsLeft >= 8 * cbBytes); // this would be a problem
//            assert(0 == pibs->m_cBitDotT); // we have only decoded the length from the frm hdr
//            pibs->m_pBuffer -= cbBytes;
//            pibs->m_cbBuflen = 0;
//            pibs->m_dwBitsLeft -= 8 * cbBytes;
//            pibs->m_dwDot >>= 8 * cbBytes;
//            assert(pibs->m_pBuffer >= pibs->m_pBufferBegin);
//
//        }
//    }
//    else
//    { // set up for the gap to be taken care of during a later input buffer
//        pibs->m_fDeferredGap = WMAB_TRUE; // audecInput() pays attention to this
//        pibs->m_cbToDeferredGap = (cBitsToGap - cBitsLeft) / 8;
//    }


#ifdef WMAINITIALIZE_HIGH_MID_ALL_USED
#pragma arm section code = "WmaCommonCode"
#if !WMA_OPT_STRMDEC_ARM
WMARESULT ibstrmGetBits(CWMAInputBitStream* pibstrm, register UInt dwNumBits,
                        U32 *piResult)
{
    WMARESULT   wmaResult = WMA_OK;
#ifdef WMA_TABLE_ROOM_VERIFY
    UInt *pgetMask = (UInt *)p_getMask;
#endif

    assert(dwNumBits <= 24); // this is to make sure there is enough buffer space when a new packet comes in. Hong


    if (pibstrm->m_dwBitsLeft < dwNumBits)
    {
        LOAD_BITS_FROM_DotT;
        LOAD_BITS_FROM_STREAM;

        if (pibstrm->m_dwBitsLeft < dwNumBits)
        {
            TRACEWMA_EXIT(wmaResult, ibstrmGetMoreData(pibstrm, ModeGetFlush, dwNumBits));
        }
    }
    //
    // Do the most common case first.  If this doesn't play, we have one branch
    // to get to the next most common case (usually 1/32 of the time in the case
    // of the codec doing a huffman decode).  Note that we use a mask array to
    // avoid a special case branch when the bitcount is 32 (even though this is
    // relatively unlikely) since a left shift operation where the shift count
    // is equal to or greater than the number of bits in the destination is
    // undefined.
    //
    pibstrm->m_dwBitsLeft -= dwNumBits;
#ifdef WMA_TABLE_ROOM_VERIFY
    *piResult = (pibstrm->m_dwDot >> pibstrm->m_dwBitsLeft) & pgetMask[dwNumBits];
#else
    *piResult = (pibstrm->m_dwDot >> pibstrm->m_dwBitsLeft) & getMask[dwNumBits];//getbits(dwnumbits)
#endif
    //DEBUG("%d: Get(%d)=%08X\n", pibstrm->m_cFrmBitCnt, dwNumBits, *piResult);
//    DEBUG_ONLY(pibstrm->m_cFrmBitCnt += dwNumBits;)
exit:



    return wmaResult;
}
#endif //WMA_OPT_STRMDEC_ARM
#pragma arm section code
#endif
// end of strmdec_wma.c
#endif
#endif
