//*@@@+++@@@@******************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//*@@@---@@@@******************************************************************
/*
 * Windows Media Audio (WMA) Decoder API (implementation)
 *
 * Copyright (c) Microsoft Corporation 1999.  All Rights Reserved.
 */
 #include "../include/audio_main.h"
 
#ifdef A_CORE_DECODE
#ifdef WMA_DEC_INCLUDE
#ifndef USE_SPDTX

#include "..\wmaInclude\wmaudio_inc.h"


#if defined(WMAHIGHRATE) || defined(WMAMIDRATELOWRATE)
#pragma arm section code = "WmaHighLowCommonCode"

/* ===========================================================================
 * WMAFileDecodeData
--------------------------------------------------------------------------- */

tWMAFileStatus WMAFileDecodeData(tHWMAFileState hstate, tWMA_U32 *pnDecodedSamples)
{
    tWMAFileStateInternal *pInt;
    static short kk = 0;//hxd
    WMARESULT wmar = WMA_OK;
    tWMAFileStatus rc;
    // FUNCTION_PROFILE(fp);

    pInt = (tWMAFileStateInternal*) hstate;

    //if(pInt == NULL || pInt->hWMA == NULL)
    //        return cWMA_BadArgument;


    //wchen: need to initialize
    if (pnDecodedSamples != NULL)
        *pnDecodedSamples = 0;

//#if (defined(WIN32) || defined(_WIN32_WINCE) )
    /*if (IsBadReadPtr(pInt, sizeof(tWMAFileStateInternal)))
        return cWMA_BadArgument;

    if (IsBadWritePtr(pnDecodedSamples, sizeof(tWMA_U32)))
        return cWMA_BadArgument;*/
//#endif

    //FUNCTION_PROFILE_START(&fp,WMA_FILE_DECODE_DATA_PROFILE);

    //if(pInt->parse_state!=csWMA_DecodeAudioData)
    /* the function is used to parse packetheader and payloadheader firstly,
       secondly and after this parse header in WMAFileGetInput()*/
    // WMAF_UpdateNewPayload(pInt);

    do
    {   //parse_state value:6 is DecodeAudioData;othervalue is to call WMAF_UpdateNewPayload(pInt) for decode packetheader/payloadheader
        switch(pInt->parse_state)
        {
        case csWMA_DecodeAudioData:
//#ifdef WMAAPI_DEMO
//            if(pInt->nSampleCount >= WMAAPI_DEMO_LIMIT*pInt->hdr_parse.nSamplesPerSec)
//            {
//                FUNCTION_PROFILE_STOP(&fp);
//                return cWMA_DemoExpired;
//            }
//#endif /* WMAAPI_DEMO */
        //audecState value:1 is audecStateInput;2 is audecStatedecode;3 is audecStateGetPCM;0 is audecStateDone
        switch (pInt->audecState)
        {
            case audecStateInput:      /* If we used up the current payload, try to get the next one.*/
                {
                    WMAFileGetInputParam tmp;
                    U8* pbIn = NULL;
                    U32 cbIn = 0;
                    Bool fNoMoreInput = 0, fNewPacket = 0;
                    //DEBUG("audecinput=%d\n",++kk);//hxd
                    memset(&tmp, 0, sizeof(WMAFileGetInputParam));
                    wmar = WMAFileGetInput(pInt, &pbIn, &cbIn, &tmp);//get 128 bytes data to store &pbIn(length is *cbIn) every time.WMAF_UpdateNewPayload function inside this function
                    //when old payload ends,we decode newpayload's packetheader and payloadheader inside this function.

                    if (wmar == WMA_E_NO_MORE_SRCDATA)
                    {
                        fNoMoreInput = 1;
                    }
                    else if (WMA_FAILED(wmar))
                    {
                        pInt->parse_state = csWMA_DecodePayloadEnd;
                        return wmar;
                    }

                    if (wmar == WMA_S_NEWPACKET)
                    {
                        fNewPacket = 1;
                    }
                    wmar = audecInput(pInt->hWMA, pbIn, cbIn, fNewPacket, fNoMoreInput,
                                      tmp.m_fTimeIsValid, tmp.m_iTime, &pInt->audecState, NULL);
                    if (WMA_FAILED(wmar))
                    {
                        pInt->parse_state = csWMA_DecodePayloadEnd;
                        return cWMA_Failed;
                    }


                    break;
                }
            case audecStateDecode:
                {
                    wmar = audecDecode(pInt->hWMA, pnDecodedSamples, &pInt->audecState, NULL);//audio decode start
                    if (WMA_FAILED(wmar))
                    {
                        pInt->parse_state = csWMA_DecodePayloadEnd;
                        return cWMA_Failed;
                    }

                    break;
                }
            case audecStateGetPCM://this state shows packets decoding finish
                assert(!"WMAFileDecodeData called when WMAFileGetPCM should have been called");
                wmar = WMA_OK;//表示已解码完要送数据;或上帧还留下足够的数据未送出,则继续送。
                break;
            case audecStateDone://this state shows that wma decoding finish
                wmar = cWMA_NoMoreFrames;
                break;
            default:
                assert(!"bad audecState");
        }//switch pint->audecstate

        if (wmar == WMA_S_NO_MORE_SRCDATA)
        {
//#ifdef LOCAL_DEBUG
//                SerialSendString("\r** WMAFileDecodeData: WMADecodeData: no more data\n");
//#endif /* LOCAL_DEBUG */
            return cWMA_NoMoreFrames;
        }
        if (pInt->audecState == audecStateDone)
            return cWMA_NoMoreFrames;//the flag represent that all audio data is dedoced
        else
        {   //if(*pnDecodedSamples!=0)//该值不等于0表示一个子帧没有解码完毕，所以送到函数wmafilegetpcm；只有解完才送出
            return cWMA_NoErr;
        }

        default:
            /* All other state operation is done in this function
             *   so that it can be done somewhere else as well
             */

            {
                rc = WMAF_UpdateNewPayload(pInt);
                if(rc != cWMA_NoErr)
                {
                    return rc;
                }
            }
            break;
		}
    } while(1);

    //FUNCTION_PROFILE_STOP(&fp);
    return cWMA_NoErr;
} // WMAFileDecodeData

/* ===========================================================================
 * WMAFileGetPCM
--------------------------------------------------------------------------- */
tWMA_U32 WMAFileGetPCM(
    tHWMAFileState hstate,
    tWMA_I16 *pi16Channel0,
    tWMA_U32 iSizeOfChannel0,
    tWMA_U32 max_nsamples,
    tWMA_I64* piTimeStamp)
{
    tWMAFileStateInternal *pInt;
    WMARESULT wmar = WMA_OK;
    WMA_U32 cSamples = 0;

    pInt = (tWMAFileStateInternal*) hstate;


    /*audecstate值来源于 程序prvdecodedata中的语句paudec->m_externalState = audecStateGetPCM;*/
    if (pInt->audecState != audecStateGetPCM)//只要audecstate值等于获取pcm数据的状态才能将pcm数据输出
        return 0;

    {

        CAudioObjectDecoder* paudec = (CAudioObjectDecoder*)pInt->hWMA;
        //U8**  rgpiRecon = NULL;
//        I16  iCh;
        CAudioObject* pau = NULL;
        U32 cbDstLengthAdjusted = iSizeOfChannel0;
        //I16 nDRCSetting;
        U16 cTransformSamples;
        U16 cSamplesReturned = 0;

        cSamplesReturned = (U16)max_nsamples;

        //nDRCSetting = pPI ? pPI->nDRCSetting : WMA_DRC_HIGH; // update

        paudec->m_externalState = audecStateDone; // changed below on success
        pau = paudec->pau;
        //rgpiRecon = paudec->m_rgpiRecon;

        //memset(rgpiRecon, 0, sizeof(U8*) * pau->m_cChannel);

        //for (iCh = 0; iCh < pau->m_cChannel; iCh++)
        {
            //PerChannelInfo* ppcinfo = pau->m_rgpcinfo + iCh;
            //rgpiRecon [iCh] = (PCMSAMPLE *) (((U8*) pi16Channel0) + (pau->m_nBytePerSample * iCh));//(U8*) prvAdvancePCMPointer((PCMSAMPLE *)pi16Channel0,
            //pau->m_nBytePerSample, iCh);

        }

        //get pcm only when we are in these states
        if (paudec->m_decsts == BEGIN_FRAME ||paudec->m_decsts == BEGIN_SUBFRAME ||paudec->m_decsts == END_SUBFRAME1)
        {
            // post processing may require more buffer than before post processing
            // i.e., we are upsampling rate or number of channels.
            // find out how much of buffer we can use to get samples.
            // adjust cbDstLength to take care of time-domain player options
            //   - changing number of channels ("downmix")
            //   - adjusting sampling rate
            //   - bitdepth is taken care of since pau->m_nBytePerSample
            //     is already adjusted to dst.
            // adjust buffer size here since auGetPCM is common function
            // and does not have decoder specific member variables available.
            //if (paudec->pau->m_iXformSamplingRate != paudec->m_iDstSamplingRate)
            //   {
            //    cbDstLengthAdjusted = min(iSizeOfChannel0,
            //     iSizeOfChannel0*paudec->m_iInterpolSrcBlkSize/paudec->m_iInterpolDstBlkSize);
            //   }

            cbDstLengthAdjusted = min(cbDstLengthAdjusted,cbDstLengthAdjusted * pau->m_cChannel / paudec->m_cDstChannel);
            //if(cSamplesReturned>2048)//hxd
            // getchar();//hxd
            auGetPCM(pau, &cSamplesReturned, pi16Channel0, cbDstLengthAdjusted);
            // these are the number of transform samples that are consumed.
            // actual number of samples output may be smaller or larger than this.
            cTransformSamples = cSamplesReturned;

            //TRACEWMA_EXIT(wmaResult, audecPostProcessPostPCM(pDecHandle, &cSamplesReturned, pbDst, cbDstLength, nDRCSetting));
        }

        //iSampleTotal += cSamplesReturned;

        //convert back to 100ns
        // timestamps are donw using iCurrPresTime, which are calculated using
        // destination sampling rate; therefore iCurrPresTime should be updated
        // using actual number of samples that are output
        //if (piTimeStamp) *piTimeStamp = paudec->m_iCurrPresTime * 10000000 / paudec->m_iDstSamplingRate;

        //paudec->m_iCurrPresTime += cSamplesReturned;

        paudec->m_cSamplesReady -= cTransformSamples;

        if (paudec->m_cSamplesReady > 0)//表示当提供的数据超过2048个时就需要分成多次送出
            paudec->m_externalState = audecStateGetPCM;
        else
            paudec->m_externalState = audecStateDecode;

        if (&cSamples) cSamples = cSamplesReturned;

        pInt->audecState = paudec->m_externalState;

    }
    //pInt->nSampleCount = cSamples;

    return cSamples;
} // WMAFileGetPCM
#pragma arm section code

#endif //WMAMIDRATELOWRATE||WMAHIGHRATE
#endif // !USE_SPDTX
#endif
#endif
