
#include "audio_main.h"

#ifdef A_CORE_DECODE
#ifdef APE_DEC_INCLUDE

#pragma arm section code = "ApeDecCode", rodata = "ApeDecCode", rwdata = "ApeDecData", zidata = "ApeDecBss"

#include "ape_all.h"
#include "ape_Infodec.h"
/*Mod by Wei.Hisung 2007.03.05*/
//#include "APECompress.h"
#include "ape_decompress.h"
#include "ape_decompold.h"
/*Mod by Wei.Hisung 2007.03.05*/
//#include "WAVInputSource.h"
//#include IO_HEADER_FILE
/*Mod by Wei.Hisung 2007.03.05*/
//#include "MACProgressHelper.h"
#include "ape_globalfunc.h"
/*Mod by Wei.Hisung 2007.03.05*/
//#include "MD5.h"
//#include "CharacterHelper.h"
#include "ape_globalvardeclaration.h"

#define UNMAC_DECODER_OUTPUT_NONE       0
#define UNMAC_DECODER_OUTPUT_WAV        1
#define UNMAC_DECODER_OUTPUT_APE        2



//ape_int32 DecompressCore(ape_char * pInputFilename, ape_char * pOutputFilename, ape_int32 nOutputMode, ape_int32 nCompressionLevel, ape_int32 * pPercentageDone, APE_PROGRESS_CALLBACK ProgressCallback, ape_int32 * pKillFlag);

/*****************************************************************************************
ANSI wrappers
*****************************************************************************************/
/*Mod by Wei.Hisung 2007.03.05*/
#if 0
ape_int32  ApeDecompressFile(const str_ansi * pInputFilename, const str_ansi * pOutputFilename, ape_int32 * pPercentageDone, APE_PROGRESS_CALLBACK ProgressCallback, ape_int32 * pKillFlag)
{
	/*Mod by Wei.Hisung 2007.03.06*/
	ape_char* spInputFile=GetUTF16FromANSI(pInputFilename);
    ape_char* spOutputFile=GetUTF16FromANSI(pOutputFilename);    
    return DecompressFileW(spInputFile, pOutputFilename ? spOutputFile : NULL, pPercentageDone, ProgressCallback, pKillFlag);
}
#endif
/*****************************************************************************************
Decompress file
*****************************************************************************************/
#if 0
ape_int32  ApeDecompressFileW(ape_char * pInputFilename, ape_char * pOutputFilename, ape_int32 * pPercentageDone, APE_PROGRESS_CALLBACK ProgressCallback, ape_int32 * pKillFlag)
{
	/*Mod by Wei.Hisung 2007.03.05*/
    /*
	if (pOutputFilename == NULL)
        return VerifyFileW(pInputFilename, pPercentageDone, ProgressCallback, pKillFlag);
    else
	*/
        return DecompressCore(pInputFilename, pOutputFilename, UNMAC_DECODER_OUTPUT_WAV, -1, pPercentageDone, ProgressCallback, pKillFlag);
}
#endif

//initiaze global variable
void ApeDecodeInitialize(void)
{
  Ape_gOutBufferOffset0=0;
  Ape_gOutBufferOffset1=0;
  Ape_gDualBufferIndex=0;
  //Ape_pOutBuffer=&Ape_gOutBuffer[Ape_gDualBufferIndex][0];
  //ape_char Ape_gReadBuffer[512];
  memset(Ape_gReadBuffer,0,sizeof(ape_char)*512);  
  
  //memset(Ape_gInBuffer,0,sizeof(ape_uint32)*(BLOCKS_PER_BITSTREAMREAD+16));
  
  memset(&Ape_gIApeDecompressMalloc,0,sizeof(CAPEDecompress));
  
  Ape_gBlockAlign=0;
  Ape_gCurrentFrame=0;
  Ape_gStartBlock=0;
  Ape_gFinishBlock=0;
  Ape_gCurrentBlock=0;
  Ape_gIsRanged=FALSE;
  Ape_gDecompressorInitial=FALSE;
  Ape_gBeginDecodeFrameFlag = FALSE;
  Ape_gForwardBackIndex = 0;
  Ape_gForwardBackFlag = 0;//20070428               
  Ape_gForwardBackOffset=0x5000;//20070428
  
  
  memset(&Ape_gPrepare,0,sizeof(CPrepare));
  
  memset(&Ape_gWfeInput,0,sizeof(WAVEFORMATEX));
  
  Ape_gCRC=0;
  Ape_gStoredCRC=0;
  Ape_gSpecialCodes=0;
  Ape_pApeInfo=NULL;
  Ape_pUnBitArray=NULL;
  
  memset(&Ape_gBitArrayStateX,0,sizeof(struct UNBIT_ARRAY_STATE));
  
  memset(&Ape_gBitArrayStateY,0,sizeof(struct UNBIT_ARRAY_STATE));
  
  Ape_pNewPredictorX=NULL;
  Ape_pNewPredictorY=NULL;
  Ape_gLastXValue=0;
  Ape_gErrorDecodingCurrentFrame=FALSE;
  Ape_gCurrentFrameBufferBlock=0;
  Ape_gFrameBufferFinishedBlocks=0;
  
  memset(&Ape_gPredDecomMallocX,0,sizeof(CPredictorDecompress3950toCurrent));
  
  memset(&Ape_gPredDecomMallocY,0,sizeof(CPredictorDecompress3950toCurrent));
  
  memset(Ape_gBytesToSkipMalloc,0,sizeof(ape_int16)*16);
  
  Ape_gElements=0;
  Ape_gBytesNum=0;
  Ape_gBitsNum=0;    
  Ape_gVersion=0;
  Ape_pIO=NULL;
  Ape_gCurrentBitIndex=0;
  Ape_pBitArray=NULL;
  Ape_gFlushCounter=0;
  Ape_gFinalizeCounter=0;    
  
  Ape_gRefillBitThreshold=0; 
  Ape_gElementsBase=0;
  Ape_gBytesValueBase=0;
  Ape_gBitsValueBase=0;    
  Ape_gVersionBase=0;
  Ape_pIOBase=NULL;
  Ape_gCurrentBitIndexBase=0;
  Ape_pBitArrayBase=NULL;
  
  //CUnBitArray Ape_gUnBitArrayBaseMalloc;
  memset(&Ape_gUnBitArrayBaseMalloc,0,sizeof(CUnBitArray));
  
  /////the following global variables are used in APEDecompressOld.c/////////////////////////////////
  //----------------------------------
  Ape_pBuffer=NULL;
  Ape_gBufferTail=0;
  Ape_gBlockAlignOld=0;
  Ape_gCurrentFrameOld=0;
  Ape_gStartBlockOld=0;
  Ape_gFinishBlockOld=0;
  Ape_gCurrentBlockOld=FALSE;
  Ape_gIsRangedOld=0;
  //CUnMAC Ape_gUnMAC;
  memset(&Ape_gUnMAC,0,sizeof(CUnMAC));
  Ape_pAPEInfoOld=NULL;
  Ape_gDecompressorInitialOld=FALSE;
  //----------------------------------
  
  /////the following global variables are used in UnBitArrayOld.c/////////////////////////////////
  Ape_gElementsOld=0;
  Ape_gBytesValueOld=0;
  Ape_gBitsValueOld=0;    
  Ape_gVersionOld=0;
  Ape_pIOOld=NULL;
  Ape_gCurrentBitIndexOld=0;
  Ape_pBitArrayOld=NULL;
  //----------------------------------
  Ape_gKBitArrayOld=0;
  Ape_gKSum=0;
  Ape_gRefillBitThresholdOld=0;
  //----------------------------------
  
  /////the following global variables are used in RollBuffer.c/////////////////////////////////
  memset(Ape_gPredABufferFastMalloc,0,sizeof(ape_int32)*2*(WINDOW_ELEMENTS + HISTORY_ELEMENTS));
  
  memset(Ape_gPredBBufferFastMalloc,0,sizeof(ape_int32)*2*(WINDOW_ELEMENTS + HISTORY_ELEMENTS));
  
  memset(Ape_gAdaptABufferFastMalloc,0,sizeof(ape_int32)*2*(WINDOW_ELEMENTS + HISTORY_ELEMENTS));
  
  memset(Ape_gAdaptBBufferFastMalloc,0,sizeof(ape_int32)*2*(WINDOW_ELEMENTS + HISTORY_ELEMENTS));
  
  //Mod by Vincent Hsiung, Feb 17th, 2008
  #if 0
  memset(Ape_gDeltaMCreateMalloc,0,sizeof(ape_int32)*3*2*(WINDOW_ELEMENTS+NORDER));  
  memset(Ape_gInputCreateMalloc,0,sizeof(ape_int32)*3*2*(WINDOW_ELEMENTS+NORDER));
  #else
  memset(Ape_gDeltaMCreateMalloc,0,sizeof(ape_int32)*1*2*(NN_WINDOW_ELEMENTS+NORDER));  
  memset(Ape_gInputCreateMalloc,0,sizeof(ape_int32)*1*2*(NN_WINDOW_ELEMENTS+NORDER));
  #endif
  //memset(Ape_gParyMMalloc,0,sizeof(ape_int32)*3*2*NORDER);
  //Mod by Vincent Hsiung, Feb 16th, 2008
  memset(Ape_gParyMMalloc,0,sizeof(ape_int32)*1*2*NORDER);
  
  memset(&Ape_gCNNFilterMalloc[0],0,sizeof( CNNFilter)*2);
  
  memset(&Ape_gCNNFilter1Malloc[0],0,sizeof( CNNFilter)*2);
  
  memset(&Ape_gCNNFilter2Malloc[0],0,sizeof( CNNFilter)*2);
  
  memset(&Ape_gDecompressInfoCreate,0,sizeof(CAPEInfo));
  
  memset(&Ape_gHeaderDescriptor,0,sizeof(struct APE_DESCRIPTOR));
  
  memset(Ape_gHeaderSeekByteTable,0,sizeof(ape_uint32)*SEEKBYTE_TABLE_MAX);
  
  memset(Ape_gHeaderSeekBitTable,0,sizeof(ape_uchar)*SEEKBYTE_TABLE_MAX);
  
  memset(Ape_gWavHeaderData,0,sizeof(ape_uchar)*WAVE_HEADER_MAX);
  
  memset(&Ape_gInfoIO,0,sizeof(CIO));
  
  memset(&Ape_gInfoApeTag,0,sizeof(CAPETag));
  
  Ape_pApeTagAryFields=NULL;
  
  memset(&Ape_gApeTagIO,0,sizeof(CIO));
  
  memset(&Ape_gId3Tag,0,sizeof(struct ID3_TAG));
  
  Ape_pIOOutput=NULL;
  Ape_pApeDecompress=NULL;
  
  memset(&Ape_gCIODecompressCore,0,sizeof(CIO));
}

#if 0 //commented by hxd 20070710
//decompress ape header information
ape_int16 ApeHeaderDecode(ape_char * pInputFilename,ape_char * pOutputFilename,ape_int32 nOutputMode, ape_int32 nCompressionLevel, ape_int32 * pPercentageDone,ape_int32 * pKillFlag)
{
  // variable declares
  ape_int32 nFunctionRetVal = ERROR_SUCCESS;
  
  ape_int16 nHeaderDecRetVal = ERROR_SUCCESS;
  
  //IAPEDecompress* Ape_pApeDecompress;
  ape_uchar* spTempBuffer;
  
  WAVEFORMATEX wfeInput;
  //ape_int32 nBlocksLeft ;
  	
  if (pInputFilename == NULL) 
  {
      return (nHeaderDecRetVal = ERROR_INVALID_FUNCTION_PARAMETER);
  }
	
  // create the decoder
  Ape_pApeDecompress=(IAPEDecompress *)ApeDecompressCreate(pInputFilename, &nFunctionRetVal);
  if (Ape_pApeDecompress == NULL || nFunctionRetVal != ERROR_SUCCESS)
  { 
    return nHeaderDecRetVal = (ape_int16)nFunctionRetVal;//exit(nFunctionRetVal);
  }
  
  // get the input format
  Ape_pApeDecompress->GetInfo(APE_INFO_WAVEFORMATEX, (ape_int32) &wfeInput,0);
  
  //----FOR DEBUG-------BEGIN----------
  if (((CAPETag*)(Ape_pApeDecompress->GetInfo(APE_INFO_TAG, (ape_int32) &wfeInput,0)))->GetHasAPETag(((CAPETag*)(Ape_pApeDecompress->GetInfo(APE_INFO_TAG, (ape_int32) &wfeInput,0))))==TRUE)
  {
    //CAPETag* pTmp=(CAPETag*)(Ape_pApeDecompress->GetInfo(APE_INFO_TAG, (ape_int32) &wfeInput,0));
          
    //CAPETagField* pTmp2;
    //ape_int32 i;			
    //fprintf(stderr, "APE Tag Bytes: %i\n",pTmp->GetTagBytes(pTmp));			
    //for (i=0;i<7;i++)
    //for (i=0;i<pTmp->m_nFields;i++)
    //{
    //fprintf(stderr, "Has APE Tag: %s\n",pTmp->m_aryFields[i]);
    //}
		
	}
  else
  {
  	//fprintf(stderr, "No APE Tag.\n");
  }
  if (((CAPETag*)(Ape_pApeDecompress->GetInfo(APE_INFO_TAG, (ape_int32) &wfeInput,0)))->GetHasID3Tag(((CAPETag*)(Ape_pApeDecompress->GetInfo(APE_INFO_TAG, (ape_int32) &wfeInput,0))))==TRUE)
  {
  	//CAPETag* pTmp=(CAPETag*)(Ape_pApeDecompress->GetInfo(APE_INFO_TAG, (ape_int32) &wfeInput,0));
  
  }
  else
  {
  	//fprintf(stderr, "No ID3 Tag.\n");
  }
  // initialize the output
  if (nOutputMode == UNMAC_DECODER_OUTPUT_WAV)
  {
    //Ape_pIOOutput=(CIO *)malloc(sizeof(CIO)); 		
    Ape_pIOOutput=&Ape_gCIODecompressCore;
		ApeIoInitialize(Ape_pIOOutput);
		Ape_pIOOutput->Create(Ape_pIOOutput,pOutputFilename);
    
    
    // output the header
    //THROW_ON_ERROR(ApeWriteSafe(Ape_pIOOutput, spTempBuffer, Ape_pApeDecompress->GetInfo(APE_INFO_WAV_HEADER_BYTES,0,0)));
		ApeWriteSafe(Ape_pIOOutput, spTempBuffer, Ape_pApeDecompress->GetInfo(APE_INFO_WAV_HEADER_BYTES,0,0));
  }
     
  return nHeaderDecRetVal;
          	
}
#endif

#if 0 //commented by hxd 20070710
//decompress ape blocks
ape_int16 ApeBlocksDecode(ape_char * pInputFilename, ape_char * pOutputFilename, ape_int32 nOutputMode, ape_int32 nCompressionLevel,ape_int32* pKillFlag,ape_int32* nBlocksLeft)
{
  // variable declares
  ape_int16 nBlocksDecRetVal = ERROR_SUCCESS;
    
  // allocate output space for writing wav file
  //spTempBuffer = (ape_uchar*)&Ape_gOutBuffer[0];
  
  //if (spTempBuffer == NULL)
  //	 return (nBlocksDecRetVal = ERROR_INSUFFICIENT_MEMORY);//exit(ERROR_INSUFFICIENT_MEMORY);
  
  //nBlocksLeft = Ape_pApeDecompress->GetInfo(APE_DECOMPRESS_TOTAL_BLOCKS,0,0);
    
  // main decoding loop
  //while (nBlocksLeft > 0)
  {
    // decode data
    ape_int32 nBlocksDecoded = -1;
    ape_int32 nRetVal ;
		                                                                    
    nRetVal = Ape_pApeDecompress->GetData(Ape_pApeDecompress,(ape_char *) Ape_gOutBuffer, BLOCKS_PER_DECODE, &nBlocksDecoded);
    if (nRetVal != ERROR_SUCCESS) 
    {
      return (nBlocksDecRetVal = ERROR_INVALID_CHECKSUM);//exit(ERROR_INVALID_CHECKSUM);
    }
    
    //¸ÃÖ¡½â´íÎóÂë commented by hxd 20070703
    if(Ape_gDecodeErrorFlag != ERROR_SUCCESS)
    {
      return (nBlocksDecRetVal = ERROR_DECOMPRESSING_FRAME);
    }
  
  // handle the output
  if (nOutputMode == UNMAC_DECODER_OUTPUT_WAV)
  {
      ape_uint32 nBytesToWrite = (nBlocksDecoded * Ape_pApeDecompress->GetInfo(APE_INFO_BLOCK_ALIGN,0,0));
      ape_uint32 nBytesWritten = 0;
    ape_int32 nRetVal = Ape_pIOOutput->Write(Ape_pIOOutput,(ape_uchar*)Ape_pOutBuffer, nBytesToWrite, &nBytesWritten);
      if ((nRetVal != 0) || (nBytesToWrite != nBytesWritten)) 
          return ERROR_IO_WRITE;
  }
  else
  { 
    if (nOutputMode == UNMAC_DECODER_OUTPUT_APE)
    {
       /*Mod by Wei.Hisung 2007.03.05*/
       //THROW_ON_ERROR(spAPECompress->AddData(spTempBuffer, nBlocksDecoded * Ape_pApeDecompress->GetInfo(APE_INFO_BLOCK_ALIGN)))
    }
  }
    // update amount remaining
    *nBlocksLeft -= nBlocksDecoded;
    
    // update progress and kill flag
    
  }
  
  // terminate the output
  //if (nOutputMode == UNMAC_DECODER_OUTPUT_WAV)
  //{
  //    // write any terminating WAV data
  //    if (Ape_pApeDecompress->GetInfo(APE_INFO_WAV_TERMINATING_BYTES,0,0) > 0) 
  //    {
  //        ape_uint32 nBytesToWrite;
  //        ape_uint32 nBytesWritten;
  //        ape_int32 nRetVal;
  //        
  //        spTempBuffer=(ape_uchar*)&Ape_gOutBuffer[0];
  //        if (spTempBuffer == NULL)
  //        	  nBlocksDecRetVal = ERROR_INSUFFICIENT_MEMORY;//exit(ERROR_INSUFFICIENT_MEMORY);
  //
  //        Ape_pApeDecompress->GetInfo(APE_INFO_WAV_TERMINATING_DATA, (ape_int32) spTempBuffer, Ape_pApeDecompress->GetInfo(APE_INFO_WAV_TERMINATING_BYTES,0,0));
  //
  //        nBytesToWrite = Ape_pApeDecompress->GetInfo(APE_INFO_WAV_TERMINATING_BYTES,0,0);
  //        nBytesWritten = 0;
  //        nRetVal = Ape_pIOOutput->Write(Ape_pIOOutput,spTempBuffer, nBytesToWrite, &nBytesWritten);
  //        if ((nRetVal != 0) || (nBytesToWrite != nBytesWritten)) 
  //	    return (nBlocksDecRetVal = ERROR_IO_WRITE);//exit(ERROR_IO_WRITE);
  //    }
  //}	
  
  return nBlocksDecRetVal;
	
}
#endif

/*****************************************************************************************
Decompress a file using the specified output method
*****************************************************************************************/
#if 0
ape_int32 ApeDecompressCore(ape_char * pInputFilename, ape_char * pOutputFilename, ape_int32 nOutputMode, ape_int32 nCompressionLevel, ape_int32 * pPercentageDone, APE_PROGRESS_CALLBACK ProgressCallback, ape_int32 * pKillFlag) 
{
	// variable declares
    ape_int32 nFunctionRetVal = ERROR_SUCCESS;
	/*Mod by Wei.Hisung 2007.03.06*/
    //CSmartPtr<IO_CLASS_NAME> Ape_pIOOutput;
	CIO* Ape_pIOOutput;

	/*Mod by Wei.Hisung 2007.03.05*/
    //CSmartPtr<IAPECompress> spAPECompress;
    //CSmartPtr<IAPEDecompress> Ape_pApeDecompress;
    //CSmartPtr<ape_uchar> spTempBuffer;
	IAPEDecompress* Ape_pApeDecompress;
    ape_uchar* spTempBuffer;
	/*Mod by Wei.Hisung 2007.03.05*/
    //CSmartPtr<CMACProgressHelper> spMACProgressHelper;
    WAVEFORMATEX wfeInput;
    ape_int32 nBlocksLeft ;


	// error check the function parameters
    if (pInputFilename == NULL) 
    {
        return ERROR_INVALID_FUNCTION_PARAMETER;
    }
	

    
        // create the decoder
		/*Mod by Wei.Hisung 2007.03.06*/
        //Ape_pApeDecompress.Assign(ApeDecompressCreate(pInputFilename, &nFunctionRetVal));
		Ape_pApeDecompress=(IAPEDecompress *)ApeDecompressCreate(pInputFilename, &nFunctionRetVal);
        if (Ape_pApeDecompress == NULL || nFunctionRetVal != ERROR_SUCCESS) exit(nFunctionRetVal);//throw(nFunctionRetVal);

        // get the input format
        //THROW_ON_ERROR(Ape_pApeDecompress->GetInfo(APE_INFO_WAVEFORMATEX, (int) &wfeInput,0))
		Ape_pApeDecompress->GetInfo(APE_INFO_WAVEFORMATEX, (ape_int32) &wfeInput,0);

		//----FOR DEBUG-------BEGIN----------
	    if (((CAPETag*)(Ape_pApeDecompress->GetInfo(APE_INFO_TAG, (ape_int32) &wfeInput,0)))->GetHasAPETag(((CAPETag*)(Ape_pApeDecompress->GetInfo(APE_INFO_TAG, (ape_int32) &wfeInput,0))))==TRUE)
		{
			//CAPETag* pTmp=(CAPETag*)(Ape_pApeDecompress->GetInfo(APE_INFO_TAG, (ape_int32) &wfeInput,0));
            ///*
			//CAPETagField* pTmp2;
			//ape_int32 i;			
//			fprintf(stderr, "APE Tag Bytes: %i\n",pTmp->GetTagBytes(pTmp));			
			//for (i=0;i<7;i++)
			//for (i=0;i<pTmp->m_nFields;i++)
			//{
				//pTmp2=pTmp->GetTagFieldByIndex(pTmp,i);
				//pTmp->m_aryFields[]
				//fprintf(stderr, "Has APE Tag: %s\n",pTmp2->GetFieldValue(pTmp2));
//				fprintf(stderr, "Has APE Tag: %s\n",pTmp->m_aryFields[i]);
			//}
			//*/
		}
		else
		{
//			fprintf(stderr, "No APE Tag.\n");
		}
		if (((CAPETag*)(Ape_pApeDecompress->GetInfo(APE_INFO_TAG, (ape_int32) &wfeInput,0)))->GetHasID3Tag(((CAPETag*)(Ape_pApeDecompress->GetInfo(APE_INFO_TAG, (ape_int32) &wfeInput,0))))==TRUE)
		{
			//CAPETag* pTmp=(CAPETag*)(Ape_pApeDecompress->GetInfo(APE_INFO_TAG, (ape_int32) &wfeInput,0));

		}
		else
		{
//			fprintf(stderr, "No ID3 Tag.\n");
		}
		//----FOR DEBUG-------END----------

        // allocate space for the header
		    //spTempBuffer = (ape_uchar *)malloc(sizeof(ape_uchar)*(Ape_pApeDecompress->GetInfo(APE_INFO_WAV_HEADER_BYTES,0,0)));
        //if (spTempBuffer == NULL) exit(ERROR_INSUFFICIENT_MEMORY);//throw(ERROR_INSUFFICIENT_MEMORY);

        // get the header
        //THROW_ON_ERROR(Ape_pApeDecompress->GetInfo(APE_INFO_WAV_HEADER_DATA, (int) spTempBuffer.GetPtr(), Ape_pApeDecompress->GetInfo(APE_INFO_WAV_HEADER_BYTES)));
		    //THROW_ON_ERROR(Ape_pApeDecompress->GetInfo(APE_INFO_WAV_HEADER_DATA, (int) spTempBuffer, Ape_pApeDecompress->GetInfo(APE_INFO_WAV_HEADER_BYTES,0,0)));
		    //Ape_pApeDecompress->GetInfo(APE_INFO_WAV_HEADER_DATA, (ape_int32) spTempBuffer, Ape_pApeDecompress->GetInfo(APE_INFO_WAV_HEADER_BYTES,0,0));

        // initialize the output
        if (nOutputMode == UNMAC_DECODER_OUTPUT_WAV)
        {
			    //Ape_pIOOutput=(CIO *)malloc(sizeof(CIO)); 		
			    Ape_pIOOutput=&Ape_gCIODecompressCore;
			    ApeIoInitialize(Ape_pIOOutput);
			    Ape_pIOOutput->Create(Ape_pIOOutput,pOutputFilename);
			    // output the header
          //THROW_ON_ERROR(ApeWriteSafe(Ape_pIOOutput, spTempBuffer, Ape_pApeDecompress->GetInfo(APE_INFO_WAV_HEADER_BYTES,0,0)));
			    ApeWriteSafe(Ape_pIOOutput, spTempBuffer, Ape_pApeDecompress->GetInfo(APE_INFO_WAV_HEADER_BYTES,0,0));
        }
           
      // allocate output space for writing wav file
	    spTempBuffer = (ape_uchar*)&Ape_gOutBuffer[0];
      //if (spTempBuffer == NULL) throw(ERROR_INSUFFICIENT_MEMORY);
	    if (spTempBuffer == NULL) exit(ERROR_INSUFFICIENT_MEMORY);

      nBlocksLeft = Ape_pApeDecompress->GetInfo(APE_DECOMPRESS_TOTAL_BLOCKS,0,0);
      
      // main decoding loop
      while (nBlocksLeft > 0)
      {
        // decode data
        ape_int32 nBlocksDecoded = -1;
		    ape_int32 nRetVal ;
		                                                                    
		    nRetVal = Ape_pApeDecompress->GetData(Ape_pApeDecompress,(ape_int16 *) spTempBuffer, BLOCKS_PER_DECODE, &nBlocksDecoded);
		    if (nRetVal != ERROR_SUCCESS) 
		    {
			    //throw(ERROR_INVALID_CHECKSUM);
			    exit(ERROR_INVALID_CHECKSUM);
		    }

          // handle the output
          if (nOutputMode == UNMAC_DECODER_OUTPUT_WAV)
          {
              ape_uint32 nBytesToWrite = (nBlocksDecoded * Ape_pApeDecompress->GetInfo(APE_INFO_BLOCK_ALIGN,0,0));
              ape_uint32 nBytesWritten = 0;
              ape_int32 nRetVal = Ape_pIOOutput->Write(Ape_pIOOutput,spTempBuffer, nBytesToWrite, &nBytesWritten);
              if ((nRetVal != 0) || (nBytesToWrite != nBytesWritten)) 
                  return ERROR_IO_WRITE;
          }
          else if (nOutputMode == UNMAC_DECODER_OUTPUT_APE)
          {
			/*Mod by Wei.Hisung 2007.03.05*/
              //THROW_ON_ERROR(spAPECompress->AddData(spTempBuffer, nBlocksDecoded * Ape_pApeDecompress->GetInfo(APE_INFO_BLOCK_ALIGN)))
          }

          // update amount remaining
          nBlocksLeft -= nBlocksDecoded;
      			
          // update progress and kill flag
		/*Mod by Wei.Hisung 2007.03.05*/
		/*
          spMACProgressHelper->UpdateProgress();
          if (spMACProgressHelper->ProcessKillFlag(TRUE) != 0)
              throw(ERROR_USER_STOPPED_PROCESSING);
		*/
      }

      // terminate the output
      if (nOutputMode == UNMAC_DECODER_OUTPUT_WAV)
      {
          // write any terminating WAV data
          if (Ape_pApeDecompress->GetInfo(APE_INFO_WAV_TERMINATING_BYTES,0,0) > 0) 
          {
			ape_uint32 nBytesToWrite;
              ape_uint32 nBytesWritten;
              ape_int32 nRetVal;
              
			spTempBuffer=(ape_uchar*)malloc(sizeof(ape_uchar)*(Ape_pApeDecompress->GetInfo(APE_INFO_WAV_TERMINATING_BYTES,0,0)));
              if (spTempBuffer == NULL) exit(ERROR_INSUFFICIENT_MEMORY);//throw(ERROR_INSUFFICIENT_MEMORY);
              //THROW_ON_ERROR(Ape_pApeDecompress->GetInfo(APE_INFO_WAV_TERMINATING_DATA, (int) spTempBuffer.GetPtr(), Ape_pApeDecompress->GetInfo(APE_INFO_WAV_TERMINATING_BYTES)))
			//THROW_ON_ERROR(Ape_pApeDecompress->GetInfo(APE_INFO_WAV_TERMINATING_DATA, (int) spTempBuffer, Ape_pApeDecompress->GetInfo(APE_INFO_WAV_TERMINATING_BYTES,0,0)))
			Ape_pApeDecompress->GetInfo(APE_INFO_WAV_TERMINATING_DATA, (ape_int32) spTempBuffer, Ape_pApeDecompress->GetInfo(APE_INFO_WAV_TERMINATING_BYTES,0,0));
      
              nBytesToWrite = Ape_pApeDecompress->GetInfo(APE_INFO_WAV_TERMINATING_BYTES,0,0);
              nBytesWritten = 0;
              nRetVal = Ape_pIOOutput->Write(Ape_pIOOutput,spTempBuffer, nBytesToWrite, &nBytesWritten);
			if ((nRetVal != 0) || (nBytesToWrite != nBytesWritten)) 
				exit(ERROR_IO_WRITE);
                  //throw(ERROR_IO_WRITE);
          }
      }
	  
    // return
    return nFunctionRetVal;
}
#endif

#pragma arm section code

#endif
#endif