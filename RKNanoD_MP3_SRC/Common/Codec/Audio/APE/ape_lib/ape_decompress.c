
#include "audio_main.h"

#ifdef A_CORE_DECODE
#ifdef APE_DEC_INCLUDE

#pragma arm section code = "ApeDecCode", rodata = "ApeDecCode", rwdata = "ApeDecData", zidata = "ApeDecBss"

#include "ape_all.h"
#include "ape_decompress.h"

#include "ape_Infodec.h"
#include "ape_prepare.h"
#include "ape_unbitarray.h"
#include "ape_newpredictor.h"
#include "ape_globalvardeclaration.h"

#define RANGE_OVERFLOW_TOTAL_WIDTH        65536
#define RANGE_OVERFLOW_SHIFT            16

#define CODE_BITS 32
#define TOP_VALUE ((ape_uint32 ) 1 << (CODE_BITS - 1))
#define SHIFT_BITS (CODE_BITS - 9)
#define EXTRA_BITS (((CODE_BITS - 2) & 7) + 1)//((CODE_BITS - 2) % 8 + 1)
#define BOTTOM_VALUE (TOP_VALUE >> 8)

#define MODEL_ELEMENTS 64

#if 0
extern ape_uint32 udiv_32by32_arm9e(ape_uint32,ape_uint32);
#else
#define udiv_32by32_arm9e(a, b) (b/a)
#endif

void ApeDecompressInitialize(ape_int32 * pErrorCode, CAPEInfo * pAPEInfo, ape_int32 nStartBlock, ape_int32 nFinishBlock)
{
  *pErrorCode = ERROR_SUCCESS;
  
  Ape_gOutBufferOffset0=0;
  Ape_gOutBufferOffset1=0;
  Ape_gPrepare.Unprepare = ApeUnprepare;
  //Ape_gPrepare.UnprepareOld = ApeUnprepareOld;
  // open / analyze the file
  Ape_pApeInfo=pAPEInfo;
  
  // version check (this implementation only works with 3.93 and later files)
  if (ApeDecompressGetInfo(APE_INFO_FILE_VERSION,0,0) < 3930)
  {
      *pErrorCode = ERROR_UNDEFINED;
      return;
  }
  
  // get format information
  ApeDecompressGetInfo(APE_INFO_WAVEFORMATEX, (ape_int32) &Ape_gWfeInput,0);
  Ape_gBlockAlign = ApeDecompressGetInfo(APE_INFO_BLOCK_ALIGN,0,0);
  
  // initialize other stuff
  Ape_gDecompressorInitial = FALSE;
  Ape_gCurrentFrame = 0;
  Ape_gCurrentBlock = 0;
  Ape_gCurrentFrameBufferBlock = 0;
  Ape_gFrameBufferFinishedBlocks = 0;
  Ape_gErrorDecodingCurrentFrame = FALSE;
  
  // set the "real" start and finish blocks
  Ape_gStartBlock = (nStartBlock < 0) ? 0 : min(nStartBlock, ApeDecompressGetInfo(APE_INFO_TOTAL_BLOCKS,0,0));
  Ape_gFinishBlock = (nFinishBlock < 0) ? 
                   ApeDecompressGetInfo(APE_INFO_TOTAL_BLOCKS,0,0) : min(nFinishBlock, ApeDecompressGetInfo(APE_INFO_TOTAL_BLOCKS,0,0));
  Ape_gIsRanged = (Ape_gStartBlock != 0) || (Ape_gFinishBlock != ApeDecompressGetInfo(APE_INFO_TOTAL_BLOCKS,0,0));
}

void ApeDecompressDelete()
{
    
}


ape_int32 ApeDecompressInitializeDecompressor(CAPEDecompress * aI)
{
  ape_int32 nInitRetVal = ERROR_SUCCESS;
  // check if we have anything to do
  if (Ape_gDecompressorInitial)
  {
    return ERROR_SUCCESS;
  }
  
  // update the initialized flag
  Ape_gDecompressorInitial = TRUE;
  
  // create a frame buffer
  
  //Ape_gCbFrameBuffer.CreateBuffer(&Ape_gCbFrameBuffer,(BLOCKS_PER_DECODE_CUSTOM_HANDLE/*ApeDecompressGetInfo(APE_INFO_BLOCKS_PER_FRAME,0,0)*/ + DECODE_BLOCK_SIZE) * Ape_gBlockAlign, Ape_gBlockAlign * 64);
  //m_cbFrameBuffer_maxTail=(BLOCKS_PER_DECODE_CUSTOM_HANDLE + DECODE_BLOCK_SIZE) * Ape_gBlockAlign+1;
    
  // create decoding components
  Ape_pUnBitArray=(CUnBitArrayBase *) ApeUnBitArrayCreate(aI, ApeDecompressGetInfo(APE_INFO_FILE_VERSION,0,0));

  if (ApeDecompressGetInfo(APE_INFO_FILE_VERSION,0,0) >= 3950)
  {
    Ape_pNewPredictorX=(IPredictorDecompress*)&Ape_gPredDecomMallocX;//malloc(sizeof(CPredictorDecompress3950toCurrent));added by hxd 20070321
    Ape_pNewPredictorX->cIPredictorDecompress =(ape_int32 (*)(void *,ape_int32,ape_int32,ape_int16))ApePredictorDecompress3950ToCurrent ;
    Ape_pNewPredictorX->DecompressValue =ApePredictorDecompressValue3950ToCurrent ;
    //Ape_pNewPredictorX->dIPredictorDecompress =ApePredictorDecompress3950ToCurrentDelete ;
    Ape_pNewPredictorX->Flush =ApePredictorFlush3950ToCurrent ;
    nInitRetVal = Ape_pNewPredictorX->cIPredictorDecompress(Ape_pNewPredictorX,ApeDecompressGetInfo(APE_INFO_COMPRESSION_LEVEL,0,0), ApeDecompressGetInfo(APE_INFO_FILE_VERSION,0,0),0);
    if(nInitRetVal != ERROR_SUCCESS)
    {
      return nInitRetVal;
    }
    
    Ape_pNewPredictorY=(IPredictorDecompress*)&Ape_gPredDecomMallocY;//added by hxd 20070321
    Ape_pNewPredictorY->cIPredictorDecompress =(ape_int32 (*)(void *,ape_int32,ape_int32,ape_int16))ApePredictorDecompress3950ToCurrent ;
    Ape_pNewPredictorY->DecompressValue =ApePredictorDecompressValue3950ToCurrent ;
    //Ape_pNewPredictorY->dIPredictorDecompress =ApePredictorDecompress3950ToCurrentDelete ;
    Ape_pNewPredictorY->Flush =ApePredictorFlush3950ToCurrent ;
    nInitRetVal = Ape_pNewPredictorY->cIPredictorDecompress(Ape_pNewPredictorY,ApeDecompressGetInfo(APE_INFO_COMPRESSION_LEVEL,0,0), ApeDecompressGetInfo(APE_INFO_FILE_VERSION,0,0),1);
    if(nInitRetVal != ERROR_SUCCESS)
    {
      return nInitRetVal;
    }
  }
  else
  {
    Ape_pNewPredictorX=(IPredictorDecompress*)&Ape_gPredDecomMallocX;//added by hxd 20070321//(GetInfo(APE_INFO_COMPRESSION_LEVEL), GetInfo(APE_INFO_FILE_VERSION));
    Ape_pNewPredictorX->cIPredictorDecompress = (ape_int32 (*)(void *,ape_int32,ape_int32,ape_int16))ApeDecompressPredictor3930To3950; 
    Ape_pNewPredictorX->DecompressValue =(ape_int32 (*)(void *,ape_int32,ape_int32))ApePredictorDecompressValue3930To3950; 
    //Ape_pNewPredictorX->dIPredictorDecompress =(void (*)(void *))ApePredictorDecompress3930To3950Delete;
    Ape_pNewPredictorX->Flush =(ape_int32 (*)(void *))ApePredictorFlush3930To3950; 
    nInitRetVal = Ape_pNewPredictorX->cIPredictorDecompress(Ape_pNewPredictorX,ApeDecompressGetInfo(APE_INFO_COMPRESSION_LEVEL,0,0), ApeDecompressGetInfo(APE_INFO_FILE_VERSION,0,0),0);
    if(nInitRetVal != ERROR_SUCCESS)
    {
      return nInitRetVal;		
    }
    
    Ape_pNewPredictorY=(IPredictorDecompress*)&Ape_gPredDecomMallocY;//added by hxd 20070321//(GetInfo(APE_INFO_COMPRESSION_LEVEL), GetInfo(APE_INFO_FILE_VERSION));
    Ape_pNewPredictorY->cIPredictorDecompress = (ape_int32 (*)(void *,ape_int32,ape_int32,ape_int16))ApeDecompressPredictor3930To3950; 
    Ape_pNewPredictorY->DecompressValue =(ape_int32 (*)(void *,ape_int32,ape_int32))ApePredictorDecompressValue3930To3950; 
    //Ape_pNewPredictorY->dIPredictorDecompress =(void (*)(void *))ApePredictorDecompress3930To3950Delete;
    Ape_pNewPredictorY->Flush =(ape_int32 (*)(void *))ApePredictorFlush3930To3950; 
    nInitRetVal = Ape_pNewPredictorY->cIPredictorDecompress(Ape_pNewPredictorY,ApeDecompressGetInfo(APE_INFO_COMPRESSION_LEVEL,0,0), ApeDecompressGetInfo(APE_INFO_FILE_VERSION,0,0),1);
    if(nInitRetVal != ERROR_SUCCESS)
    {
      return nInitRetVal;		    
    }
  }
  
  // seek to the beginning
  return ApeDecompressSeek(aI,0);
}

ape_int32 ApeDecompressGetData(CAPEDecompress* aI, ape_int32 nBlocks, ape_int32 * pBlocksRetrieved)
{
  ape_int32 nRetVal = ERROR_SUCCESS;
  
  // cap
  ape_int32 nBlocksUntilFinish = Ape_gFinishBlock - Ape_gCurrentBlock;
  ape_int32 nBlocksToRetrieve = min(nBlocks, nBlocksUntilFinish);
  ape_int32 nBlocksRetrieved ;
  
  // get the data
  ape_int32 nBlocksLeft = nBlocksToRetrieve; 
  ape_int32 nBlocksThisPass = 1;
  
  if (pBlocksRetrieved)
  {
    *pBlocksRetrieved = 0;
  }
  
  // make sure we're initialized
  RETURN_ON_ERROR(ApeDecompressInitializeDecompressor(aI))
  
  //the following is added by hxd 
  if(Ape_gBeginDecodeFrameFlag == 0)
  {
  	Ape_gBeginDecodeFrameFlag = 1;
  	Ape_gFrameBufferFinishedBlocks = ApeDecompressGetInfo(APE_INFO_FRAME_BLOCKS, 0,0);//added by hxd 20070317
  }
  
  
  while ((nBlocksLeft > 0) && (nBlocksThisPass > 0))
  {
    // fill up the frame buffer
    ape_int32 nDecodeRetVal = ApeDecompressFillFrameBuffer();
    
    // analyze how much to remove from the buffer
    
    ape_int32 nFrameBufferBlocks = Ape_gFrameBufferFinishedBlocks;
    if (nDecodeRetVal != ERROR_SUCCESS)
    {
      nRetVal = nDecodeRetVal;
    }
    
    nBlocksThisPass = min(nBlocksLeft, nFrameBufferBlocks);
    
    // remove as much as possible
    if (nBlocksThisPass > 0)
    {
        //Ape_gCbFrameBuffer.Get(pOutputBuffer, nBlocksThisPass * Ape_gBlockAlign);
        //Ape_gCbFrameBuffer.Get(&Ape_gCbFrameBuffer,(ape_uchar*)pOutputBuffer, nBlocksThisPass * Ape_gBlockAlign);
        //pOutputBuffer += LENGTH(nBlocksThisPass * Ape_gBlockAlign);
        nBlocksLeft -= nBlocksThisPass;
        Ape_gFrameBufferFinishedBlocks -= nBlocksThisPass;
    }
  }
  
  // calculate the blocks retrieved
  nBlocksRetrieved = nBlocksToRetrieve - nBlocksLeft;
  
  // update position
  Ape_gCurrentBlock += nBlocksRetrieved;
  if (pBlocksRetrieved)
  {
    *pBlocksRetrieved = nBlocksRetrieved;
  }
  
  return nRetVal;
}

ape_int32 ApeDecompressSeek(CAPEDecompress *aI,ape_int32 nBlockOffset)
{
  ape_int32 nBaseFrame;
  ape_int32 nBlocksToSkip;
  ape_int32 nBytesToSkip;
  ape_int16* spTempBuffer;
  ape_int32 nBlocksRetrieved ;
  
  
  RETURN_ON_ERROR(ApeDecompressInitializeDecompressor(aI))
  
  // use the offset
  nBlockOffset += Ape_gStartBlock;
  
  // cap (to prevent seeking too far)
  if (nBlockOffset >= Ape_gFinishBlock)
  {
    nBlockOffset = Ape_gFinishBlock - 1;
  }
  if (nBlockOffset < Ape_gStartBlock)
  {
    nBlockOffset = Ape_gStartBlock;
  }
  
  // seek to the perfect location
  nBaseFrame = nBlockOffset / ApeDecompressGetInfo(APE_INFO_BLOCKS_PER_FRAME,0,0);
  nBlocksToSkip = nBlockOffset % ApeDecompressGetInfo(APE_INFO_BLOCKS_PER_FRAME,0,0);
  nBytesToSkip = nBlocksToSkip * Ape_gBlockAlign;
      
  Ape_gCurrentBlock = nBaseFrame * ApeDecompressGetInfo(APE_INFO_BLOCKS_PER_FRAME,0,0);
  Ape_gCurrentFrameBufferBlock = nBaseFrame * ApeDecompressGetInfo(APE_INFO_BLOCKS_PER_FRAME,0,0);
  Ape_gCurrentFrame = nBaseFrame;
  Ape_gFrameBufferFinishedBlocks = 0;
  //Ape_gCbFrameBuffer.Empty();
  //Ape_gCbFrameBuffer.Empty(&Ape_gCbFrameBuffer);
  RETURN_ON_ERROR(ApeDecompressSeekToFrame(Ape_gCurrentFrame));
  
  // skip necessary blocks
  spTempBuffer = &Ape_gBytesToSkipMalloc[0];
  
  //DEBUG("nbytestoskip=%d\n",nBytesToSkip);//added by hxd 20070322
  
  if (spTempBuffer == NULL)
  {
    return ERROR_INSUFFICIENT_MEMORY;
  }
  
  nBlocksRetrieved = 0;
  ApeDecompressGetData(aI, nBlocksToSkip, &nBlocksRetrieved);
  if (nBlocksRetrieved != nBlocksToSkip)
  {	
    return ERROR_UNDEFINED;
  }
  
  return ERROR_SUCCESS;
}

/*****************************************************************************************
Decodes blocks of data
*****************************************************************************************/
ape_int32 ApeDecompressFillFrameBuffer()
{
  ape_int32 nRetVal = ERROR_SUCCESS;
  
  // determine the maximum blocks we can decode
  // note that we won't do end capping because we can't use data
  // until EndFrame(...) successfully handles the frame
  // that means we may decode a little extra in end capping cases
  // but this allows robust error handling of bad frames
  //ape_int32 nMaxBlocks = Ape_gCbFrameBuffer.MaxAdd() / Ape_gBlockAlign;
  //ape_int32 nMaxBlocks = Ape_gCbFrameBuffer.MaxAdd(&Ape_gCbFrameBuffer) / Ape_gBlockAlign;
  ape_int32 nMaxBlocks=BLOCKS_PER_DECODE;//1152;
  
  // loop and decode data
  ape_int32 nBlocksLeft = nMaxBlocks;
  //while (nBlocksLeft > 0)
  {
    ape_int32 nFrameBlocks = ApeDecompressGetInfo(APE_INFO_FRAME_BLOCKS, Ape_gCurrentFrame,0);
    
    ape_int32 nFrameOffsetBlocks = Ape_gCurrentFrameBufferBlock % ApeDecompressGetInfo(APE_INFO_BLOCKS_PER_FRAME,0,0);
    ape_int32 nFrameBlocksLeft = nFrameBlocks - nFrameOffsetBlocks;
    ape_int32 nBlocksThisPass = min(nFrameBlocksLeft, nBlocksLeft);
    ape_int32 nFrameBufferBytes;
    
    //if(Ape_gCurrentFrameBufferBlock > 66815)
    //     DEBUG("ape test bug appear.\n");
    
    if (nFrameBlocks < 0)
    {
      return nRetVal;
    }
    
    // start the frame if we need to
    if (nFrameOffsetBlocks == 0)
    {	
      ApeDecompressStartFrame();
    }
    
    // store the frame buffer bytes before we start
    nFrameBufferBytes =0;
    
    // decode data
    ApeDecompressDecodeBlocks(nBlocksThisPass);
        
    // end the frame if we need to
    if ((nFrameOffsetBlocks + nBlocksThisPass) >= nFrameBlocks)
    {
      ApeDecompressEndFrame();
      if (Ape_gErrorDecodingCurrentFrame)
      {
         ape_uint16 cSilence ;
         ape_int32 z ;
         
         // add silence
         cSilence = (ApeDecompressGetInfo(APE_INFO_BITS_PER_SAMPLE,0,0) == 8) ? 127 : 0;
         for (z = 0; z < nFrameBlocks * Ape_gBlockAlign; z++)
         {//when the frame is decoded errorly, fill mute data into output buffer;
         	/*                    
             *Ape_gCbFrameBuffer.GetDirectWritePointer(&Ape_gCbFrameBuffer) = cSilence;                    
             Ape_gCbFrameBuffer.UpdateAfterDirectWrite(&Ape_gCbFrameBuffer,LEFTSHIFT1(1));
             */
         }
         
         // seek to try to synchronize after an error
         ApeDecompressSeekToFrame(Ape_gCurrentFrame);//skip current error frame and seek to next frame
         
         // save the return value
         nRetVal = ERROR_INVALID_CHECKSUM;
      }
    }
    
    nBlocksLeft -= nBlocksThisPass;
  }//while
  
  return nRetVal;
}

void ApeDecompressDecodeBlocks(ape_int32 nBlocks)
{
  // decode the samples
  ape_int32 nBlocksProcessed = 0;
  ape_int32 llastxvalue;
  //ape_uint32 loutbufferoffset0;
  ape_int32  nchannels = Ape_gWfeInput.nChannels;
  ape_int16* lpoutbufferleft = (ape_int16*)Ape_pOutBufferLeft;
  ape_int16* lpoutbufferright = (ape_int16*)Ape_pOutBufferRight;
  llastxvalue = Ape_gLastXValue;
  //loutbufferoffset0 = 0;
  
  if(Ape_gWfeInput.wBitsPerSample != 16)
  {
    Ape_gDecodeErrorFlag = ERROR_DECOMPRESSING_FRAME;
    Ape_gCurrentFrameBufferBlock += nBlocks;
    return;		
  }
  
  {
    if (Ape_gWfeInput.nChannels == 2)
    {
        if ((Ape_gSpecialCodes & SPECIAL_FRAME_LEFT_SILENCE) && 
            (Ape_gSpecialCodes & SPECIAL_FRAME_RIGHT_SILENCE)) 
        {
          for (nBlocksProcessed = nBlocks; nBlocksProcessed != 0; nBlocksProcessed--)
          {
            Ape_gPrepare.Unprepare(0, 0,&Ape_gWfeInput,(ape_uchar*) &(Ape_pOutBufferLeft[Ape_gOutBufferOffset0]),(ape_uchar*) &(Ape_pOutBufferRight[Ape_gOutBufferOffset0]));
            Ape_gOutBufferOffset0 += 1;
            
            //比特流被破坏导致解码错误,要结束该帧解码 commented by hxd 20070703
            if(Ape_gDecodeErrorFlag == ERROR_DECOMPRESSING_FRAME)
            {
              Ape_gCurrentFrameBufferBlock += nBlocks;
              return;
            }            
          }
        }
        else if (Ape_gSpecialCodes & SPECIAL_FRAME_PSEUDO_STEREO)
        {
          for (nBlocksProcessed = nBlocks; nBlocksProcessed != 0; nBlocksProcessed--)
          {
            ape_int32 X = Ape_pNewPredictorX->DecompressValue(Ape_pNewPredictorX,Ape_pUnBitArray->DecodeValueRange(&Ape_gBitArrayStateX),0);
            Ape_gPrepare.Unprepare(X , 0, &Ape_gWfeInput,(ape_uchar*) &(Ape_pOutBufferLeft[Ape_gOutBufferOffset0]),(ape_uchar*) &(Ape_pOutBufferRight[Ape_gOutBufferOffset0]));
            Ape_gOutBufferOffset0 += 1;
            
            //比特流被破坏导致解码错误,要结束该帧解码 commented by hxd 20070703
            if(Ape_gDecodeErrorFlag == ERROR_DECOMPRESSING_FRAME)
            {
              Ape_gCurrentFrameBufferBlock += nBlocks;
              return;
            }             
          }
        }    
        else
        {
            if (Ape_pApeInfo->GetInfo(Ape_pApeInfo,APE_INFO_FILE_VERSION,0,0) >= 3950)
            {
              ape_uint32 lrangeval = Ape_gRangeValue;
              ape_uint32 lbuffervalue = Ape_gBufferValue;
              ape_uint32 lcurrentbitindex = Ape_gCurrentBitIndex;
              ape_uint32 llowvalue = Ape_gLowValue;    
              struct UNBIT_ARRAY_STATE * BitArrayStateY = &Ape_gBitArrayStateY; 
              struct UNBIT_ARRAY_STATE * BitArrayStateX = &Ape_gBitArrayStateX;
              for (nBlocksProcessed = nBlocks; nBlocksProcessed != 0; nBlocksProcessed--)
              {
                //ape_int32 nY = Ape_pUnBitArray->DecodeValueRange(&Ape_gBitArrayStateY);
                //ape_int32 nX = Ape_pUnBitArray->DecodeValueRange(&Ape_gBitArrayStateX);
              	//ape_int32 nY;
                //ape_int32 nX;

                //nX = ApeUnBitArrayDecodeValueRangeMergeXAndY(&Ape_gBitArrayStateY,&Ape_gBitArrayStateX);
                ape_int32 nValue = 0;
                ape_int32 nPivotValue;
                // get the overflow
                ape_int32 nOverflow = 0;
                ape_int32 nBase = 0;
                ape_int32 nPivotValueBits = 0;
                ape_int32 nPivotValueA ;
                ape_int32 nBaseB ;
                ape_int32 nBaseLower;
                //ape_int32 nTempK;
                ape_uint32 RangeLastValue;
                ape_uint32 RangeTempValue;
                
                // make sure there is room for the data
                // this is a little slower than ensuring a huge block to start with, but it's safer
                //if (Ape_gCurrentBitIndex > Ape_gRefillBitThreshold)
                //{
                //  ApeUnBitArrayFillBitArray();
                //}
                
                if (Ape_gVersion >= 3990)
                {
                  // figure the pivot value
                  nPivotValue = max((BitArrayStateY->nKSum >> 5), 1);//max(BitArrayStateY->nKSum / 32, 1);
                  
                  
                  {
                      //Ape流被破坏会导致Ape_gRangeValue为0,进而导致下面的while死循环 added by hxd 20070703
                      if(lrangeval == 0)
                      {
                        Ape_gDecodeErrorFlag = ERROR_DECOMPRESSING_FRAME;
                        return ;
                      }
                      
                      // decode,ApeUnBitArrayRangeDecodeFast(nShift) is expanded to the following
                      while (lrangeval <= BOTTOM_VALUE)
                      {   
                        lbuffervalue = (lbuffervalue << 8)
                                       | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
                        lcurrentbitindex += 8;
                        llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
                        lrangeval <<= 8;
                      }
                
                      // decode
                      lrangeval = lrangeval >> RANGE_OVERFLOW_SHIFT;
                      
                      // lookup the symbol (must be a faster way than this)
                      nOverflow = 0;
                      RangeLastValue = 0;
                      while (llowvalue >= (RangeTempValue = lrangeval*Ape_gtRangeTotalTwo[nOverflow + 1]))
                      {
                        RangeLastValue = RangeTempValue;
                        nOverflow++;
                      }
                      
                      // update
                      llowvalue -= RangeLastValue;
                      lrangeval = lrangeval * Ape_gtRangeWidthTwo[nOverflow];
                      
                      // get the working k
                      if (nOverflow == (MODEL_ELEMENTS - 1))
                      {
                        //nOverflow = ApeUnBitArrayRangeDecodeFastWithUpdate(16);
                        while (lrangeval <= BOTTOM_VALUE)
                        {   
                            lbuffervalue = (lbuffervalue << 8)
                                        | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
                            lcurrentbitindex += 8;
                            llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
                            lrangeval <<= 8;
                        }
                        
                        // decode
                        lrangeval = lrangeval >> 16;
                        nOverflow = udiv_32by32_arm9e(lrangeval,llowvalue);//nOverflow = llowvalue / lrangeval;
                        llowvalue -= lrangeval * nOverflow;          
                        
                        nOverflow <<= 16;
                        //nOverflow |= ApeUnBitArrayRangeDecodeFastWithUpdate(16);
                        while (lrangeval <= BOTTOM_VALUE)
                        {   
                            lbuffervalue = (lbuffervalue << 8)
                                        | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
                            lcurrentbitindex += 8;
                            llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
                            lrangeval <<= 8;
                        }
                        
                        // decode
                        lrangeval = lrangeval >> 16;
                        nOverflow |= udiv_32by32_arm9e(lrangeval,llowvalue);//nOverflow |= llowvalue / lrangeval;
                        llowvalue -= lrangeval * nOverflow;          
                                  
                      }
                  }
                
                  // get the value
                  {
                      //ape_int32 nShift = 0;
                      if (nPivotValue > 65535 /*>= ((ape_int32)1 << (ape_int32)16)*/)
                      {
                        nPivotValueBits = 0;
                        while ((nPivotValue >> nPivotValueBits) > 0)
                        {
                          nPivotValueBits++;
                        }
                
                        nPivotValueBits -= 16;
                        nPivotValueA = (nPivotValue >> (nPivotValueBits)) + 1;
                        
                        //Ape流被破坏会导致Ape_gRangeValue为0,进而导致下面的while死循环 added by hxd 20070703
                        if(lrangeval == 0)
                        {
                          Ape_gDecodeErrorFlag = ERROR_DECOMPRESSING_FRAME;
                          return;
                        }
                      
                        while (lrangeval <= BOTTOM_VALUE)
                        {   
                          lbuffervalue = (lbuffervalue << 8)
                                      | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
                          lcurrentbitindex += 8;
                          llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
                          lrangeval <<= 8;
                        }
                        
                        lrangeval = lrangeval / nPivotValueA;
                        nBase = llowvalue / lrangeval;
                        llowvalue = llowvalue % lrangeval;//llowvalue -= lrangeval * nBase;
                
                        //Ape流被破坏会导致Ape_gRangeValue为0,进而导致下面的while死循环 added by hxd 20070703
                        if(lrangeval == 0)
                        {
                          Ape_gDecodeErrorFlag = ERROR_DECOMPRESSING_FRAME;
                          return;
                        }
                                  
                        while (lrangeval <= BOTTOM_VALUE)
                        {   
                          lbuffervalue = (lbuffervalue << 8)
                                        | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
                          lcurrentbitindex += 8;
                          llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
                          lrangeval <<= 8;
                        }
                        
                        lrangeval = (lrangeval >> (nPivotValueBits));
                        nBaseB = llowvalue / lrangeval;
                        llowvalue = llowvalue % lrangeval;//llowvalue -= lrangeval * nBaseB;
                        
                        nBase = (nBase << nPivotValueBits) + nBaseB;
                      }
                      else
                      {
                        //Ape流被破坏会导致Ape_gRangeValue为0,进而导致下面的while死循环 added by hxd 20070703
                        if(lrangeval == 0)
                        {
                          Ape_gDecodeErrorFlag = ERROR_DECOMPRESSING_FRAME;
                          return;
                        }
                                
                        while (lrangeval <= BOTTOM_VALUE)
                        {   
                          lbuffervalue = (lbuffervalue << 8)
                                     | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
                          lcurrentbitindex += 8;
                          llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
                          lrangeval <<= 8;
                        }
                        
                        // decode
                        lrangeval = udiv_32by32_arm9e((ape_uint32) nPivotValue,lrangeval);//lrangeval = lrangeval / nPivotValue;
                        nBaseLower = udiv_32by32_arm9e(lrangeval,llowvalue);//nBaseLower = llowvalue / lrangeval;
                        llowvalue -= lrangeval * nBaseLower;
                        
                        nBase = nBaseLower;
                      }
                  }
                  
                  // build the value
                  nValue = nBase + (nOverflow * nPivotValue);
                }
                else//the following is ape version < 3990
                {
                  //Ape流被破坏会导致Ape_gRangeValue为0,进而导致下面的while死循环 added by hxd 20070703
                  if(lrangeval == 0)
                  {
                    Ape_gDecodeErrorFlag = ERROR_DECOMPRESSING_FRAME;
                    return;
                  }
                          
                  // decode,ApeUnBitArrayRangeDecodeFast(nShift) is expanded to the following
                  while (lrangeval <= BOTTOM_VALUE)
                  {   
                    lbuffervalue = (lbuffervalue << 8)
                                   | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
                    lcurrentbitindex += 8;
                    llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
                    lrangeval <<= 8;
                  }
                
                  // decode
                  lrangeval = lrangeval >> RANGE_OVERFLOW_SHIFT;
                  
                  // lookup the symbol (must be a faster way than this)
                  nOverflow = 0;
                  RangeLastValue = 0;
                  while (llowvalue >= (RangeTempValue = lrangeval*Ape_gtRangeTotalOne[nOverflow + 1]))
                  {
                    RangeLastValue = RangeTempValue;
                    nOverflow++;
                  }
                  
                  // update
                  llowvalue -= RangeLastValue;
                  lrangeval = lrangeval * Ape_gtRangeWidthOne[nOverflow];
                  
                  // get the working k
                  
                  if (nOverflow == (MODEL_ELEMENTS - 1))
                  {
                    //RangeTempValue = ApeUnBitArrayRangeDecodeFastWithUpdate(5);
                    while (lrangeval <= BOTTOM_VALUE)
                    {   
                        lbuffervalue = (lbuffervalue << 8)
                                    | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
                        lcurrentbitindex += 8;
                        llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
                        lrangeval <<= 8;
                    }
                    
                    // decode
                    lrangeval = lrangeval >> 5;
                    RangeTempValue = udiv_32by32_arm9e(lrangeval,llowvalue);//RangeTempValue = llowvalue / lrangeval;
                    llowvalue -= lrangeval * RangeTempValue;  
                    nOverflow = 0;
                  }
                  else
                  {
                    RangeTempValue = (BitArrayStateY->k < 1) ? (0) : (BitArrayStateY->k - 1);
                  }
                  
                  // figure the extra bits on the left and the left value
                  if (RangeTempValue <= 16 || Ape_gVersion < 3910)
                  {
                    //nValue = ApeUnBitArrayRangeDecodeFastWithUpdate(RangeTempValue);
                    while (lrangeval <= BOTTOM_VALUE)
                    {   
                        lbuffervalue = (lbuffervalue << 8)
                                    | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
                        lcurrentbitindex += 8;
                        llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
                        lrangeval <<= 8;
                    }
                    
                    // decode
                    lrangeval = lrangeval >> RangeTempValue;
                    nValue = udiv_32by32_arm9e(lrangeval,llowvalue);//nValue = llowvalue / lrangeval;
                    llowvalue -= lrangeval * nValue; 
                  }                    
                  else
                  {    
                    //ape_int32 nX1 = ApeUnBitArrayRangeDecodeFastWithUpdate(16);
                    //ape_int32 nX2 = ApeUnBitArrayRangeDecodeFastWithUpdate(RangeTempValue - 16);
                    ape_int32 nX1;
                    ape_int32 nX2;
                
                    while (lrangeval <= BOTTOM_VALUE)
                    {   
                        lbuffervalue = (lbuffervalue << 8)
                                    | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
                        lcurrentbitindex += 8;
                        llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
                        lrangeval <<= 8;
                    }
                    
                    // decode
                    lrangeval = lrangeval >> 16;
                    nX1 = udiv_32by32_arm9e(lrangeval,llowvalue);//nX1 = llowvalue / lrangeval;
                    llowvalue -= lrangeval * nX1;  
                    
                    while (lrangeval <= BOTTOM_VALUE)
                    {   
                        lbuffervalue = (lbuffervalue << 8)
                                    | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
                        lcurrentbitindex += 8;
                        llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
                        lrangeval <<= 8;
                    }
                    
                    // decode
                    lrangeval = lrangeval >> (RangeTempValue - 16);
                    nX2 = udiv_32by32_arm9e(lrangeval,llowvalue);//nX2 = llowvalue / lrangeval;
                    llowvalue -= lrangeval * nX2;    
                    
                    nValue = nX1 | (nX2 << 16);
                  }
                      
                  // build the value and output it
                  nValue += (nOverflow << RangeTempValue);
                }
                RangeLastValue = BitArrayStateY->nKSum;
                // update nKSum
                RangeLastValue += ((nValue + 1) >> 1) - ((RangeLastValue + 16) >> 5);
                RangeTempValue = BitArrayStateY->k;
                // update k
                if (RangeLastValue < Ape_gtKSumMinBoundary[RangeTempValue])
                {	 
                  RangeTempValue--;
                }
                else if (RangeLastValue >= Ape_gtKSumMinBoundary[RangeTempValue + 1]) 
                	   {
                       RangeTempValue++;
                     }
                
                BitArrayStateY->nKSum = RangeLastValue;
                BitArrayStateY->k = RangeTempValue;
                // output the value (converted to signed)
                Ape_gnYValue = (nValue & 1) ? ((nValue >> 1) + 1) :( -(nValue >> 1));
                	
//              //////////////////nX///////////////////////////////////////////
                //nValue = 0;
                //nOverflow = 0;
                //nBase = 0;
                //nPivotValueBits = 0;
                if (Ape_gVersion >= 3990)
                {
                  // figure the pivot value
                  nPivotValue = max((BitArrayStateX->nKSum >> 5), 1);//max(BitArrayStateX->nKSum / 32, 1);
                  
                  
                  {
                      //Ape流被破坏会导致Ape_gRangeValue为0,进而导致下面的while死循环 added by hxd 20070703
                      if(lrangeval == 0)
                      {
                        Ape_gDecodeErrorFlag = ERROR_DECOMPRESSING_FRAME;
                        return;
                      }
                      
                      // decode,ApeUnBitArrayRangeDecodeFast(nShift) is expanded to the following
                      while (lrangeval <= BOTTOM_VALUE)
                      {   
                        lbuffervalue = (lbuffervalue << 8)
                                       | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
                        lcurrentbitindex += 8;
                        llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
                        lrangeval <<= 8;
                      }
                
                      // decode
                      lrangeval = lrangeval >> RANGE_OVERFLOW_SHIFT;
                      
                      // lookup the symbol (must be a faster way than this)
                      nOverflow = 0;
                      RangeLastValue = 0;
                      while (llowvalue >= (RangeTempValue = lrangeval*Ape_gtRangeTotalTwo[nOverflow + 1]))
                      {
                        RangeLastValue = RangeTempValue;
                        nOverflow++;
                      }
                      
                      // update
                      llowvalue -= RangeLastValue;
                      lrangeval = lrangeval * Ape_gtRangeWidthTwo[nOverflow];
                      
                      // get the working k
                      if (nOverflow == (MODEL_ELEMENTS - 1))
                      {
                        //nOverflow = ApeUnBitArrayRangeDecodeFastWithUpdate(16);
                        while (lrangeval <= BOTTOM_VALUE)
                        {   
                            lbuffervalue = (lbuffervalue << 8)
                                        | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
                            lcurrentbitindex += 8;
                            llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
                            lrangeval <<= 8;
                        }
                        
                        // decode
                        lrangeval = lrangeval >> 16;
                        nOverflow = udiv_32by32_arm9e(lrangeval,llowvalue);//nOverflow = llowvalue / lrangeval;
                        llowvalue -= lrangeval * nOverflow;          
                        
                        nOverflow <<= 16;
                        //nOverflow |= ApeUnBitArrayRangeDecodeFastWithUpdate(16);
                        while (lrangeval <= BOTTOM_VALUE)
                        {   
                            lbuffervalue = (lbuffervalue << 8)
                                        | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
                            lcurrentbitindex += 8;
                            llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
                            lrangeval <<= 8;
                        }
                        
                        // decode
                        lrangeval = lrangeval >> 16;
                        nOverflow |= udiv_32by32_arm9e(lrangeval,llowvalue);//nOverflow |= llowvalue / lrangeval;
                        llowvalue -= lrangeval * nOverflow;          
                                  
                      }
                  }
                
                  // get the value
                  {
                      //ape_int32 nShift = 0;
                      if (nPivotValue > 65535 /*>= ((ape_int32)1 << (ape_int32)16)*/)
                      {
                        nPivotValueBits = 0;
                        while ((nPivotValue >> nPivotValueBits) > 0)
                        {
                          nPivotValueBits++;
                        }
                
                        nPivotValueBits -= 16;
                        nPivotValueA = (nPivotValue >> (nPivotValueBits)) + 1;
                        
                        //Ape流被破坏会导致Ape_gRangeValue为0,进而导致下面的while死循环 added by hxd 20070703
                        if(lrangeval == 0)
                        {
                          Ape_gDecodeErrorFlag = ERROR_DECOMPRESSING_FRAME;
                          return;
                        }
                      
                        while (lrangeval <= BOTTOM_VALUE)
                        {   
                          lbuffervalue = (lbuffervalue << 8)
                                      | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
                          lcurrentbitindex += 8;
                          llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
                          lrangeval <<= 8;
                        }
                        
                        lrangeval = lrangeval / nPivotValueA;
                        nBase = llowvalue / lrangeval;
                        llowvalue = llowvalue % lrangeval;//llowvalue -= lrangeval * nBase;
                
                        //Ape流被破坏会导致Ape_gRangeValue为0,进而导致下面的while死循环 added by hxd 20070703
                        if(lrangeval == 0)
                        {
                          Ape_gDecodeErrorFlag = ERROR_DECOMPRESSING_FRAME;
                          return;
                        }
                                  
                        while (lrangeval <= BOTTOM_VALUE)
                        {   
                          lbuffervalue = (lbuffervalue << 8)
                                        | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
                          lcurrentbitindex += 8;
                          llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
                          lrangeval <<= 8;
                        }
                        
                        lrangeval = (lrangeval >> (nPivotValueBits));
                        nBaseB = llowvalue / lrangeval;
                        llowvalue = llowvalue % lrangeval;//llowvalue -= lrangeval * nBaseB;
                        
                        nBase = (nBase << nPivotValueBits) + nBaseB;
                      }
                      else
                      {
                        //Ape流被破坏会导致Ape_gRangeValue为0,进而导致下面的while死循环 added by hxd 20070703
                        if(lrangeval == 0)
                        {
                          Ape_gDecodeErrorFlag = ERROR_DECOMPRESSING_FRAME;
                          return;
                        }
                                
                        while (lrangeval <= BOTTOM_VALUE)
                        {   
                          lbuffervalue = (lbuffervalue << 8)
                                     | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
                          lcurrentbitindex += 8;
                          llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
                          lrangeval <<= 8;
                        }
                        
                        // decode
                        lrangeval = udiv_32by32_arm9e((ape_uint32) nPivotValue,lrangeval);//lrangeval = lrangeval / nPivotValue;
                        nBaseLower = udiv_32by32_arm9e(lrangeval,llowvalue);//nBaseLower = llowvalue / lrangeval;
                        llowvalue -= lrangeval * nBaseLower;
                        
                        nBase = nBaseLower;
                      }
                  }
                  
                  // build the value
                  nValue = nBase + (nOverflow * nPivotValue);
                }
                else//the following is ape version < 3990
                {
                  //Ape流被破坏会导致Ape_gRangeValue为0,进而导致下面的while死循环 added by hxd 20070703
                  if(lrangeval == 0)
                  {
                    Ape_gDecodeErrorFlag = ERROR_DECOMPRESSING_FRAME;
                    return;
                  }
                          
                  // decode,ApeUnBitArrayRangeDecodeFast(nShift) is expanded to the following
                  while (lrangeval <= BOTTOM_VALUE)
                  {   
                    lbuffervalue = (lbuffervalue << 8)
                                   | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
                    lcurrentbitindex += 8;
                    llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
                    lrangeval <<= 8;
                  }
                
                  // decode
                  lrangeval = lrangeval >> RANGE_OVERFLOW_SHIFT;
                  
                  // lookup the symbol (must be a faster way than this)
                  nOverflow = 0;
                  RangeLastValue = 0;
                  while (llowvalue >= (RangeTempValue = lrangeval*Ape_gtRangeTotalOne[nOverflow + 1]))
                  {
                    RangeLastValue = RangeTempValue;
                    nOverflow++;
                  }
                  
                  // update
                  llowvalue -= RangeLastValue;
                  lrangeval = lrangeval * Ape_gtRangeWidthOne[nOverflow];
                  
                  // get the working k
                  
                  if (nOverflow == (MODEL_ELEMENTS - 1))
                  {
                    //RangeTempValue = ApeUnBitArrayRangeDecodeFastWithUpdate(5);
                    while (lrangeval <= BOTTOM_VALUE)
                    {   
                        lbuffervalue = (lbuffervalue << 8)
                                    | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
                        lcurrentbitindex += 8;
                        llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
                        lrangeval <<= 8;
                    }
                    
                    // decode
                    lrangeval = lrangeval >> 5;
                    RangeTempValue = udiv_32by32_arm9e(lrangeval,llowvalue);//RangeTempValue = llowvalue / lrangeval;
                    llowvalue -= lrangeval * RangeTempValue;   
                    nOverflow = 0;
                  }
                  else
                  {
                    RangeTempValue = (BitArrayStateX->k < 1) ? (0) : (BitArrayStateX->k - 1);
                  }
                  
                  // figure the extra bits on the left and the left value
                  if (RangeTempValue <= 16 || Ape_gVersion < 3910)
                  {
                    //nValue = ApeUnBitArrayRangeDecodeFastWithUpdate(RangeTempValue);
                    while (lrangeval <= BOTTOM_VALUE)
                    {   
                        lbuffervalue = (lbuffervalue << 8)
                                    | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
                        lcurrentbitindex += 8;
                        llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
                        lrangeval <<= 8;
                    }
                    
                    // decode
                    lrangeval = lrangeval >> RangeTempValue;
                    nValue = udiv_32by32_arm9e(lrangeval,llowvalue);//nValue = llowvalue / lrangeval;
                    llowvalue -= lrangeval * nValue;        
                  }                    
                  else
                  {    
                    //ape_int32 nX1 = ApeUnBitArrayRangeDecodeFastWithUpdate(16);
                    //ape_int32 nX2 = ApeUnBitArrayRangeDecodeFastWithUpdate(RangeTempValue - 16);
                    ape_int32 nX1;
                    ape_int32 nX2;
                
                    while (lrangeval <= BOTTOM_VALUE)
                    {   
                        lbuffervalue = (lbuffervalue << 8)
                                    | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
                        lcurrentbitindex += 8;
                        llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
                        lrangeval <<= 8;
                    }
                    
                    // decode
                    lrangeval = lrangeval >> 16;
                    nX1 = udiv_32by32_arm9e(lrangeval,llowvalue);//nX1 = llowvalue / lrangeval;
                    llowvalue -= lrangeval * nX1; 
                    
                    while (lrangeval <= BOTTOM_VALUE)
                    {   
                        lbuffervalue = (lbuffervalue << 8)
                                    | ((Ape_pBitArray[lcurrentbitindex >> 5] >> (24 - (lcurrentbitindex & 31))) & 0xFF);
                        lcurrentbitindex += 8;
                        llowvalue = (llowvalue << 8) | ((lbuffervalue >> 1) & 0xFF);
                        lrangeval <<= 8;
                    }
                    
                    // decode
                    lrangeval = lrangeval >> (RangeTempValue - 16);
                    nX2 = udiv_32by32_arm9e(lrangeval,llowvalue);//nX2 = llowvalue / lrangeval;
                    llowvalue -= lrangeval * nX2;          
                    
                    nValue = nX1 | (nX2 << 16);
                  }
                      
                  // build the value and output it
                  nValue += (nOverflow << RangeTempValue);
                }
                
                RangeLastValue = BitArrayStateX->nKSum;
                // update nKSum
                RangeLastValue += ((nValue + 1) >> 1) - ((RangeLastValue + 16) >> 5);
                RangeTempValue = BitArrayStateX->k;
                // update k
                if (RangeLastValue < Ape_gtKSumMinBoundary[RangeTempValue])
                {	 
                  RangeTempValue--;
                }
                else if (RangeLastValue >= Ape_gtKSumMinBoundary[RangeTempValue + 1]) 
                	   {
                       RangeTempValue++;
                     }
                     
                BitArrayStateX->nKSum = RangeLastValue;
                BitArrayStateX->k = RangeTempValue;  
                   
                
                // output the value (converted to signed)
                //*nX = (nValue & 1) ? ((nValue >> 1) + 1) :( -(nValue >> 1));
                //return  (nValue & 1) ? ((nValue >> 1) + 1) :( -(nValue >> 1));
                nOverflow = (nValue & 1) ? ((nValue >> 1) + 1) :( -(nValue >> 1));              
                ////////////////////////////////////////////////////////////////////////////////////
                {
                ape_int32 Y;
                ape_int32 X;
                //Y = Ape_pNewPredictorY->DecompressValue(Ape_pNewPredictorY,nY, llastxvalue);
                //X = Ape_pNewPredictorX->DecompressValue(Ape_pNewPredictorX,nX, Y);
                X = ApePredictorDecompressValue3950ToCurrentMergeXAndY(Ape_gnYValue,llastxvalue,nOverflow);
                llastxvalue = X;
                Y = Ape_gYValue;
                
                //Ape_gPrepare.Unprepare(X, Y,nchannels,lpoutbuffer);
                {
                  ape_int32  nRTemp,nLTemp;
     
                  if (nchannels == 2) 
                  {
                      {
                          // get the right and left values
                          ape_int32 nR = X - (Y / 2);
                          ape_int32 nL = nR + Y;
                          
                          nRTemp = nR + 0x8000;
                          nRTemp >>= 16;
                          nLTemp = nL + 0x8000;
                          nLTemp >>= 16;            
                          // error check (for overflows)
                          if((nRTemp | nLTemp) == 0)
                          {
	                          *lpoutbufferright++ = (ape_int16) nR;
                            *lpoutbufferleft++ = (ape_int16) nL;
                          }
                          else
                          {
                            Ape_gDecodeErrorFlag = ERROR_DECOMPRESSING_FRAME;
                          }
                      }
                  
                  }
                  else if (nchannels == 1) 
                  {
                      {
                          ape_int16 R = X;
                              
                          *lpoutbufferright++ = (ape_int16) R;
                  
                      }
                  }                	
                }               	
                }
                //Ape_gCbFrameBuffer.UpdateAfterDirectWrite(&Ape_gCbFrameBuffer,Ape_gBlockAlign);
                
                //比特流被破坏导致解码错误,要结束该帧解码 commented by hxd 20070703
                //if(Ape_gDecodeErrorFlag == ERROR_DECOMPRESSING_FRAME)
                //{
                //  Ape_gCurrentFrameBufferBlock += nBlocks;
                //  return;
                //} 
                                 
              }
              Ape_gRangeValue = lrangeval;
              Ape_gBufferValue = lbuffervalue;
              Ape_gCurrentBitIndex = lcurrentbitindex;
              Ape_gLowValue = llowvalue;  
              //fprintf(stderr,"A OK \n");
            }
            else
            {
               for (nBlocksProcessed = nBlocks; nBlocksProcessed != 0; nBlocksProcessed--)
               {
                 ape_int32 X = Ape_pNewPredictorX->DecompressValue(Ape_pNewPredictorX,Ape_pUnBitArray->DecodeValueRange(&Ape_gBitArrayStateX),0);
                 ape_int32 Y = Ape_pNewPredictorY->DecompressValue(Ape_pNewPredictorY,Ape_pUnBitArray->DecodeValueRange(&Ape_gBitArrayStateY),0);
                 
                 Ape_gPrepare.Unprepare(X, Y, &Ape_gWfeInput,(ape_uchar*) &(Ape_pOutBufferLeft[Ape_gOutBufferOffset0]),(ape_uchar*) &(Ape_pOutBufferRight[Ape_gOutBufferOffset0]));
                 Ape_gOutBufferOffset0 += 1;
                 //Ape_gCbFrameBuffer.UpdateAfterDirectWrite(&Ape_gCbFrameBuffer,Ape_gBlockAlign);
                 
                 //比特流被破坏导致解码错误,要结束该帧解码 commented by hxd 20070703
                 if(Ape_gDecodeErrorFlag == ERROR_DECOMPRESSING_FRAME)
                 {
                   Ape_gCurrentFrameBufferBlock += nBlocks;
                   return;
                 }                 
               }
            }
        }
    }
    else
    {
      if (Ape_gSpecialCodes & SPECIAL_FRAME_MONO_SILENCE)
      {
          for (nBlocksProcessed = nBlocks; nBlocksProcessed != 0; nBlocksProcessed--)
          {
            Ape_gPrepare.Unprepare(0, 0,&Ape_gWfeInput,(ape_uchar*) &(Ape_pOutBufferLeft[Ape_gOutBufferOffset0]),(ape_uchar*) &(Ape_pOutBufferRight[Ape_gOutBufferOffset0]));
            Ape_gOutBufferOffset0 += 1;
            //Ape_gCbFrameBuffer.UpdateAfterDirectWrite(&Ape_gCbFrameBuffer,Ape_gBlockAlign);
            
            //比特流被破坏导致解码错误,要结束该帧解码 commented by hxd 20070703
            if(Ape_gDecodeErrorFlag == ERROR_DECOMPRESSING_FRAME)
            {
              Ape_gCurrentFrameBufferBlock += nBlocks;
              return;
            }
          }
                    
      }
      else
      {
          for (nBlocksProcessed = nBlocks; nBlocksProcessed != 0; nBlocksProcessed--)
          {
            ape_int32 X = Ape_pNewPredictorX->DecompressValue(Ape_pNewPredictorX,Ape_pUnBitArray->DecodeValueRange(&Ape_gBitArrayStateX),0);
            
            Ape_gPrepare.Unprepare(X, 0,&Ape_gWfeInput,(ape_uchar*) &(Ape_pOutBufferLeft[Ape_gOutBufferOffset0]),(ape_uchar*) &(Ape_pOutBufferRight[Ape_gOutBufferOffset0]));
            Ape_gOutBufferOffset0 += 1;
            //Ape_gCbFrameBuffer.UpdateAfterDirectWrite(&Ape_gCbFrameBuffer,Ape_gBlockAlign);

            //比特流被破坏导致解码错误,要结束该帧解码 commented by hxd 20070703
            if(Ape_gDecodeErrorFlag == ERROR_DECOMPRESSING_FRAME)
            {
              Ape_gCurrentFrameBufferBlock += nBlocks;
              return;
            }
          }
          

      }
    }
  }
  
  Ape_gLastXValue = llastxvalue;  
  Ape_gOutBufferOffset0=0;
  Ape_gOutBufferOffset1=0;
  Ape_gCurrentFrameBufferBlock += nBlocks;
}

void ApeDecompressStartFrame()
{
  Ape_gCRC = 0xFFFFFFFF;
  
  //get the frame header
  Ape_gStoredCRC = Ape_pUnBitArray->DecodeValue(DECODE_VALUE_METHOD_UNSIGNED_INT,0 , 0);
  Ape_gErrorDecodingCurrentFrame = FALSE;
  
  //get any 'special' codes if the file uses them (for silence, FALSE stereo, etc.)
  Ape_gSpecialCodes = 0;
  //if (GET_USES_SPECIAL_FRAMES(Ape_pApeInfo))
  //(((APE_INFO)->GetInfo(APE_INFO_FILE_VERSION) > 3820) ? TRUE : FALSE)
  if ((((Ape_pApeInfo)->GetInfo(Ape_pApeInfo,APE_INFO_FILE_VERSION,0,0) > 3820) ? TRUE : FALSE))		
  {
    if (Ape_gStoredCRC & 0x80000000) 
    {
        Ape_gSpecialCodes = Ape_pUnBitArray->DecodeValue(DECODE_VALUE_METHOD_UNSIGNED_INT , 0 , 0);
    }
    Ape_gStoredCRC &= 0x7FFFFFFF;
  }
  
  Ape_pNewPredictorX->Flush(Ape_pNewPredictorX);
  Ape_pNewPredictorY->Flush(Ape_pNewPredictorY);
  
  Ape_pUnBitArray->FlushState(&Ape_gBitArrayStateX);
  Ape_pUnBitArray->FlushState(&Ape_gBitArrayStateY);
  
  Ape_pUnBitArray->FlushBitArray();//reset buffer,low,range value when decoding new frame
  
  Ape_gLastXValue = 0;
}

void ApeDecompressEndFrame()
{
  Ape_gFrameBufferFinishedBlocks += ApeDecompressGetInfo(APE_INFO_FRAME_BLOCKS, Ape_gCurrentFrame,0);
  Ape_gCurrentFrame++;
  
  // finalize
  Ape_pUnBitArray->Finalize();

#if 0 //commented by hxd 20070703  
  // check the CRC
  Ape_gCRC = Ape_gCRC ^ 0xFFFFFFFF;
  Ape_gCRC >>= 1;
  if (Ape_gCRC != Ape_gStoredCRC)
  {
    Ape_gErrorDecodingCurrentFrame = TRUE;
  }
#endif 

}

/*****************************************************************************************
Seek to the proper frame (if necessary) and do any alignment of the bit array
*****************************************************************************************/
ape_int32 ApeDecompressSeekToFrame(ape_int32 nFrameIndex)
{
  ape_int32 nSeekRemainder = (ApeDecompressGetInfo(APE_INFO_SEEK_BYTE, nFrameIndex,0) - ApeDecompressGetInfo(APE_INFO_SEEK_BYTE, 0,0)) % 4;
  ape_int32 tmp=ApeDecompressGetInfo(APE_INFO_SEEK_BYTE, nFrameIndex,0) - nSeekRemainder;
  ape_int32 tmp1=nSeekRemainder*8;
  return Ape_pUnBitArray->FillAndResetBitArray(tmp, tmp1);
}

/*****************************************************************************************
Get information from the decompressor
*****************************************************************************************/
//ape_int32 ApeDecompressGetInfo(APE_DECOMPRESS_FIELDS Field, ape_int32 nParam1, ape_int32 nParam2)
ape_int32 ApeDecompressGetInfo(ape_int32 Field, ape_int32 nParam1, ape_int32 nParam2)
{
    ape_int32 nRetVal = 0;
    ape_BOOL bHandled = TRUE;

    switch (Field)
    {
    case APE_DECOMPRESS_CURRENT_BLOCK:
        nRetVal = Ape_gCurrentBlock - Ape_gStartBlock;
        break;
    case APE_DECOMPRESS_CURRENT_MS:
    {
        ape_int32 nSampleRate = Ape_pApeInfo->GetInfo(Ape_pApeInfo,APE_INFO_SAMPLE_RATE, 0, 0);
        if (nSampleRate > 0)
        {
          nRetVal = (ape_int32)(((double)(Ape_gCurrentBlock) * (double)(1000)) / (double)(nSampleRate));
        }
        break;
    }
    case APE_DECOMPRESS_TOTAL_BLOCKS:
        nRetVal = Ape_gFinishBlock - Ape_gStartBlock;
        break;
    case APE_DECOMPRESS_LENGTH_MS:
    {
        ape_int32 nSampleRate = Ape_pApeInfo->GetInfo(Ape_pApeInfo,APE_INFO_SAMPLE_RATE, 0, 0);
        if (nSampleRate > 0)
        {
          nRetVal = (ape_int32)(((double)(Ape_gFinishBlock - Ape_gStartBlock) * (double)(1000)) / (double)(nSampleRate));
        }
        break;
    }
    case APE_DECOMPRESS_CURRENT_BITRATE:
        nRetVal = ApeDecompressGetInfo(APE_INFO_FRAME_BITRATE, Ape_gCurrentFrame,0);
        break;
    case APE_DECOMPRESS_AVERAGE_BITRATE:
    {
        if (Ape_gIsRanged)
        {
            // figure the frame range
            ape_int32 nBlocksPerFrame = ApeDecompressGetInfo(APE_INFO_BLOCKS_PER_FRAME,0,0);
            ape_int32 nStartFrame = Ape_gStartBlock / nBlocksPerFrame;
            ape_int32 nFinishFrame = (Ape_gFinishBlock + nBlocksPerFrame - 1) / nBlocksPerFrame;

            // get the number of bytes in the first and last frame
            ape_int32 nTotalBytes = (ApeDecompressGetInfo(APE_INFO_FRAME_BYTES, nStartFrame,0) * (Ape_gStartBlock % nBlocksPerFrame)) / nBlocksPerFrame;
			      ape_int32 nTotalFrames;
			      ape_int32 nTotalMS ;
			      ape_int32 nFrame;
            if (nFinishFrame != nStartFrame)
            {
              nTotalBytes += (ApeDecompressGetInfo(APE_INFO_FRAME_BYTES, nFinishFrame,0) * (Ape_gFinishBlock % nBlocksPerFrame)) / nBlocksPerFrame;
            }

            // get the number of bytes in between
            nTotalFrames = ApeDecompressGetInfo(APE_INFO_TOTAL_FRAMES,0,0);
            for (nFrame = nStartFrame + 1; (nFrame < nFinishFrame) && (nFrame < nTotalFrames); nFrame++)
            {
              nTotalBytes += ApeDecompressGetInfo(APE_INFO_FRAME_BYTES, nFrame,0);
            }

            // figure the bitrate
            nTotalMS = (ape_int32)(((double)(Ape_gFinishBlock - Ape_gStartBlock) * (double)(1000)) / (double)(ApeDecompressGetInfo(APE_INFO_SAMPLE_RATE,0,0)));
            if (nTotalMS != 0)
            {
              nRetVal = (nTotalBytes * 8) / nTotalMS;
            }
        }
        else
        {
            nRetVal = ApeDecompressGetInfo(APE_INFO_AVERAGE_BITRATE,0,0);
        }

        break;
    }
    default:
        bHandled = FALSE;
    }

    if (!bHandled && Ape_gIsRanged)
    {
        bHandled = TRUE;

        switch (Field)
        {
        case APE_INFO_WAV_HEADER_BYTES:
            nRetVal = sizeof(struct WAVE_HEADER);
            break;
        case APE_INFO_WAV_HEADER_DATA:
        {
            ape_char * pBuffer = (ape_char *) nParam1;
            ape_int32 nMaxBytes = nParam2;
            
            if (sizeof(struct WAVE_HEADER) > nMaxBytes)
            {
                nRetVal = -1;
            }
            else
            {
			        struct WAVE_HEADER WAVHeader;
              WAVEFORMATEX wfeFormat; 
			        ApeDecompressGetInfo(APE_INFO_WAVEFORMATEX, (ape_int32) &wfeFormat, 0);
              ApeFillWaveHeader(&WAVHeader, 
                    (Ape_gFinishBlock - Ape_gStartBlock) * ApeDecompressGetInfo(APE_INFO_BLOCK_ALIGN,0,0), 
                    &wfeFormat,    0);
              memcpy(pBuffer, &WAVHeader, sizeof(struct WAVE_HEADER));
              nRetVal = 0;
            }
            break;
        }
        case APE_INFO_WAV_TERMINATING_BYTES:
            nRetVal = 0;
            break;
        case APE_INFO_WAV_TERMINATING_DATA:
            nRetVal = 0;
            break;
        default:
            bHandled = FALSE;
        }
    }

    if (bHandled == FALSE)
        nRetVal = Ape_pApeInfo->GetInfo(Ape_pApeInfo,(enum APE_DECOMPRESS_FIELDS)Field, nParam1, nParam2);

    return nRetVal;
}

#pragma arm section code

#endif
#endif
