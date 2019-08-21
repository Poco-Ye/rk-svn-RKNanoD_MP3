
#include "audio_main.h"

#ifdef A_CORE_DECODE
#ifdef APE_DEC_INCLUDE

/*****************************************************************************************
CAPEInfo:
    -a class to make working with APE files and getting information about them simple
*****************************************************************************************/

#pragma arm section code = "ApeDecCode", rodata = "ApeDecCode", rwdata = "ApeDecData", zidata = "ApeDecBss"

#include "ape_all.h"
#include "ape_Infodec.h"
//#include IO_HEADER_FILE
//#include "APECompress.h"
#include "ape_headerdec.h"
#include "ape_io1.h"
#include "ape_globalvardeclaration.h"

//extern void ApeIoInitialize(CIO* aI);
#if 0 //commented by hxd
/*****************************************************************************************
Construction
*****************************************************************************************/
void ApeInfoFile(CAPEInfo* aI,ape_int32 * pErrorCode, CAPETag * pTag) 
{
  //CAPETag* p_m_spAPETag;
  *pErrorCode = ERROR_SUCCESS;
  aI->CloseFile(aI);
  
  aI->m_spIO= &Ape_gInfoIO;
  ApeIoInitialize(aI->m_spIO);
    
  if (aI->m_spIO->Open(aI->m_spIO) != 0)
  {
    aI->CloseFile(aI);
    *pErrorCode = ERROR_INVALID_INPUT_FILE;
    return;
  }
    
  // get the file information
  if (aI->GetFileInformation(aI,TRUE) != 0)
  {
      aI->CloseFile(aI);
      *pErrorCode = ERROR_INVALID_INPUT_FILE;
      return;
  }
#if 0  
  // get the tag (do this second so that we don't do it on failure)
  if (pTag == NULL)
  {
    // we don't want to analyze right away for non-local files
    // since a single I/O object is shared, we can't tag and read at the same time (i.e. in multiple threads)
    ape_BOOL bAnalyzeNow = TRUE;
    //if ((wcsnicmp(pFilename, L"http://", 7) == 0) || (wcsnicmp(pFilename, L"m01p://", 7) == 0))
    //bAnalyzeNow = FALSE;
    
    aI->m_spAPETag=&Ape_gInfoApeTag;
    ApeTagIo(aI->m_spAPETag,aI->m_spIO, bAnalyzeNow);	
  }
  else
  {
    /*Mod by Wei.Hisung 2007.03.06*/
    //m_spAPETag.Assign(pTag);
    aI->m_spAPETag=pTag;
  }
#endif
}
#endif

void ApeInfoIo(CAPEInfo* aI,ape_int32 * pErrorCode, CIO * pIO, CAPETag * pTag)
{
  //CAPETag* p_m_spAPETag;
  *pErrorCode = ERROR_SUCCESS;
  aI->CloseFile(aI);
  
  /*Mod by Wei.Hisung 2007.03.06*/
  //m_spIO.Assign(pIO, FALSE, FALSE);
  aI->m_spIO=pIO;
  
  // get the file information
  if (aI->GetFileInformation(aI,TRUE) != 0)
  {
    aI->CloseFile(aI);
    *pErrorCode = ERROR_INVALID_INPUT_FILE;
    return;
  }
  
  // get the tag (do this second so that we don't do it on failure)
  if (pTag == NULL)
  {
    aI->m_spAPETag=&Ape_gInfoApeTag;
    ApeTagIo(aI->m_spAPETag,aI->m_spIO, TRUE);
    /*Mod by Wei.Hisung 2007.03.06*/
  	
  }
  else
  {
    /*Mod by Wei.Hisung 2007.03.06*/
    //m_spAPETag.Assign(pTag);
    aI->m_spAPETag=pTag;
  }
}


/*****************************************************************************************
Destruction
*****************************************************************************************/
#if 0
void ApeInfoDelete(CAPEInfo* aI) 
{
    aI->CloseFile(aI);
}
#endif

#if 0 //commented by hxd 20070710
/*****************************************************************************************
Close the file
*****************************************************************************************/
ape_int32 ApeInfoCloseFile(CAPEInfo* aI) 
{
  // re-initialize variables
  aI->m_APEFileInfo.nSeekTableElements = 0;
  aI->m_bHasFileInformationLoaded = FALSE;
  
  return ERROR_SUCCESS;
}
#endif

#if 0 //commented by hxd 20070710
/*****************************************************************************************
Get the file information about the file
*****************************************************************************************/
ape_int32 ApeInfoGetFileInfo(CAPEInfo* aI,ape_BOOL bGetTagInformation) 
{
  ape_int32 nRetVal ;
  CAPEHeader APEHeader;
  // quit if there is no simple file
  if (aI->m_spIO == NULL)
  {
    return -1;
  }
  
  // quit if the file information has already been loaded
  if (aI->m_bHasFileInformationLoaded)
  {
    return ERROR_SUCCESS;
  }
  
  // use a CAPEHeader class to help us analyze the file
  //CAPEHeader APEHeader(aI->m_spIO);
  
  APEHeader.Analyze =(ape_int32 (*)(void *,struct APE_FILE_INFO *))ApeHeaderAnalyze ;
  APEHeader.AnalyzeCurrent =(ape_int32 (*)(void *,struct APE_FILE_INFO *))ApeHeaderAnalyzeCurrent;
  APEHeader.AnalyzeOld =(ape_int32 (*)(void *,struct APE_FILE_INFO *))ApeHeaderAnalyzeOld;
  APEHeader.cCAPEHeader =(void (*)(void *,CIO *))ApeHeaderInitialize;
  //APEHeader.dCAPEHeader =(void (*)(void *))ApeHeaderDelete ;
  APEHeader.FindDescriptor =(ape_int32 (*)(void *,ape_int32))ApeHeaderFindDescriptor;
  //APEHeader.cCAPEHeader(&APEHeader,aI->m_spIO);
  ApeHeaderInitialize(&APEHeader,aI->m_spIO);
  
  //nRetVal = APEHeader.Analyze(&APEHeader,&(aI->m_APEFileInfo));
  nRetVal = ApeHeaderAnalyze(&APEHeader,&(aI->m_APEFileInfo));
  
  // update our internal state
  if (nRetVal == ERROR_SUCCESS)
  {
    aI->m_bHasFileInformationLoaded = TRUE;
  }
  
  // return
  return nRetVal;
}
#endif

/*****************************************************************************************
Primary query function
*****************************************************************************************/
ape_int32 ApeInfoGetInfo(CAPEInfo* aI,enum APE_DECOMPRESS_FIELDS Field, ape_int32 nParam1, ape_int32 nParam2)
{
  ape_int32 nRetVal = -1;
  ape_int32 nFrameBytes ;
  ape_int32 nFrameBlocks ;
  struct WAVE_HEADER WAVHeader; 
  
  switch (Field)
  {
  case APE_INFO_FILE_VERSION:
      nRetVal = aI->m_APEFileInfo.nVersion;
      break;
  case APE_INFO_COMPRESSION_LEVEL:
      nRetVal = aI->m_APEFileInfo.nCompressionLevel;
      break;
  case APE_INFO_FORMAT_FLAGS:
      nRetVal = aI->m_APEFileInfo.nFormatFlags;
      break;
  case APE_INFO_SAMPLE_RATE:
      nRetVal = aI->m_APEFileInfo.nSampleRate;
      break;
  case APE_INFO_BITS_PER_SAMPLE:
      nRetVal = aI->m_APEFileInfo.nBitsPerSample;
      break;
  case APE_INFO_BYTES_PER_SAMPLE:
      nRetVal = aI->m_APEFileInfo.nBytesPerSample;
      break;
  case APE_INFO_CHANNELS:
      nRetVal = aI->m_APEFileInfo.nChannels;
      break;
  case APE_INFO_BLOCK_ALIGN:
      nRetVal = aI->m_APEFileInfo.nBlockAlign;
      break;
  case APE_INFO_BLOCKS_PER_FRAME:
      nRetVal = aI->m_APEFileInfo.nBlocksPerFrame;
      break;
  case APE_INFO_FINAL_FRAME_BLOCKS:
      nRetVal = aI->m_APEFileInfo.nFinalFrameBlocks;
      break;
  case APE_INFO_TOTAL_FRAMES:
      nRetVal = aI->m_APEFileInfo.nTotalFrames;
      break;
  case APE_INFO_WAV_HEADER_BYTES:
      nRetVal = aI->m_APEFileInfo.nWAVHeaderBytes;
      break;
  case APE_INFO_WAV_TERMINATING_BYTES:
      nRetVal = aI->m_APEFileInfo.nWAVTerminatingBytes;
      break;
  case APE_INFO_WAV_DATA_BYTES:
      nRetVal = aI->m_APEFileInfo.nWAVDataBytes;
      break;
  case APE_INFO_WAV_TOTAL_BYTES:
      nRetVal = aI->m_APEFileInfo.nWAVTotalBytes;
      break;
  case APE_INFO_APE_TOTAL_BYTES:
      nRetVal = aI->m_APEFileInfo.nAPETotalBytes;
      break;
  case APE_INFO_TOTAL_BLOCKS:
      nRetVal = aI->m_APEFileInfo.nTotalBlocks;
      break;
  case APE_INFO_LENGTH_MS:
      nRetVal = aI->m_APEFileInfo.nLengthMS;
      break;
  case APE_INFO_AVERAGE_BITRATE:
      nRetVal = aI->m_APEFileInfo.nAverageBitrate;
      break;
  case APE_INFO_FRAME_BITRATE:
  {
      ape_int32 nFrame = nParam1;
      
      nRetVal = 0;
      
      nFrameBytes = aI->GetInfo(aI,APE_INFO_FRAME_BYTES, nFrame,0);
      nFrameBlocks = aI->GetInfo(aI,APE_INFO_FRAME_BLOCKS, nFrame,0);
      if ((nFrameBytes > 0) && (nFrameBlocks > 0) && aI->m_APEFileInfo.nSampleRate > 0)
      {
          ape_int32 nFrameMS = (nFrameBlocks * 1000) / aI->m_APEFileInfo.nSampleRate;
          if (nFrameMS != 0)
          {
              nRetVal = (nFrameBytes * 8) / nFrameMS;
          }
      }
      break;
  }
  case APE_INFO_DECOMPRESSED_BITRATE:
      nRetVal = aI->m_APEFileInfo.nDecompressedBitrate;
      break;
  case APE_INFO_PEAK_LEVEL:
      nRetVal = -1; // no longer supported
      break;
  case APE_INFO_SEEK_BIT:
  {
    ape_int32 nFrame = nParam1;
    if ((((aI)->GetInfo(aI,APE_INFO_FILE_VERSION,0,0) > 3800) ? TRUE : FALSE)) 
    {
        nRetVal = 0;
    }
    else 
    {
      if (nFrame < 0 || nFrame >= aI->m_APEFileInfo.nTotalFrames)
      {
        nRetVal = 0;
      }
      else
      {
        nRetVal = aI->m_APEFileInfo.spSeekBitTable[nFrame];
      }
    }
      break;
  }
  case APE_INFO_SEEK_BYTE:
  {
    ape_int32 nFrame = nParam1;
    if (nFrame < 0 || nFrame >= aI->m_APEFileInfo.nTotalFrames)
    {
      nRetVal = 0;
    }
    else
    {
      if(nFrame >= SEEKBYTE_TABLE_MAX)
      {
        ape_int32 val, cbRead;
        ape_int32 offset = aI->m_spIO->GetPosition((void*)aI->m_spIO);
        aI->m_spIO->Seek((void*)aI->m_spIO, Ape_gHeaderSeekByteTableOffset+4*nFrame, 0);
        aI->m_spIO->Read((void*)aI->m_spIO, &val, 4, &cbRead);
        aI->m_spIO->Seek((void*)aI->m_spIO, offset, 0);
        nRetVal = val + aI->m_APEFileInfo.nJunkHeaderBytes;
      }
	  else
	  {
        nRetVal = aI->m_APEFileInfo.spSeekByteTable[nFrame] + aI->m_APEFileInfo.nJunkHeaderBytes;
	  }
    }
    break;
  }
  case APE_INFO_WAV_HEADER_DATA:
  {
    ape_char * pBuffer = (ape_char *) nParam1;
    ape_int32 nMaxBytes = nParam2;
    
    if (aI->m_APEFileInfo.nFormatFlags & MAC_FORMAT_FLAG_CREATE_WAV_HEADER)
    {
        if (sizeof(struct WAVE_HEADER) > nMaxBytes)
        {
            nRetVal = -1;
        }
        else
        {
          WAVEFORMATEX wfeFormat; aI->GetInfo(aI,APE_INFO_WAVEFORMATEX, (ape_int32) &wfeFormat, 0);
            
          ApeFillWaveHeader(&WAVHeader, aI->m_APEFileInfo.nWAVDataBytes, &wfeFormat,
                aI->m_APEFileInfo.nWAVTerminatingBytes);
          memcpy(pBuffer, &WAVHeader, sizeof(struct WAVE_HEADER));
          nRetVal = 0;
        }
    }
    else
    {
      if (aI->m_APEFileInfo.nWAVHeaderBytes > nMaxBytes)
      {
          nRetVal = -1;
      }
      else
      {
          memcpy(pBuffer, aI->m_APEFileInfo.spWaveHeaderData, LENGTH(aI->m_APEFileInfo.nWAVHeaderBytes));
          nRetVal = 0;
      }
    }
    break;
  }
  case APE_INFO_WAV_TERMINATING_DATA:
  {
      ape_char * pBuffer = (ape_char *) nParam1;
      ape_int32 nMaxBytes = nParam2;
      
      if (aI->m_APEFileInfo.nWAVTerminatingBytes > nMaxBytes)
      {
          nRetVal = -1;
      }
      else
      {
        if (aI->m_APEFileInfo.nWAVTerminatingBytes > 0) 
        {
            // variables
            ape_int32 nOriginalFileLocation = aI->m_spIO->GetPosition(aI->m_spIO);
            ape_uint32 nBytesRead = 0;
        
            // check for a tag
            aI->m_spIO->Seek(aI->m_spIO,-(aI->m_spAPETag->GetTagBytes(aI->m_spAPETag) + aI->m_APEFileInfo.nWAVTerminatingBytes), FILE_END);
            aI->m_spIO->Read((void*)aI->m_spIO,pBuffer,(unsigned int) aI->m_APEFileInfo.nWAVTerminatingBytes,(unsigned int*) &nBytesRead);
        
            // restore the file pointer
            aI->m_spIO->Seek(aI->m_spIO,nOriginalFileLocation, FILE_BEGIN);
        }
        nRetVal = 0;
      }
      break;
  }
  case APE_INFO_WAVEFORMATEX:
  {
       WAVEFORMATEX * pWaveFormatEx = (WAVEFORMATEX *) nParam1;
       ApeFillWaveFormatEx(pWaveFormatEx, aI->m_APEFileInfo.nSampleRate, aI->m_APEFileInfo.nBitsPerSample, aI->m_APEFileInfo.nChannels);
       nRetVal = 0;
       break;
  }
  case APE_INFO_IO_SOURCE:
  /*Mod by Wei.Hisung 2007.03.06*/
     
      nRetVal = (ape_int32) aI->m_spIO;
      break;
  case APE_INFO_FRAME_BYTES:
  {
      ape_int32 nFrame = nParam1;
      
      // bound-check the frame index
      if ((nFrame < 0) || (nFrame >= aI->m_APEFileInfo.nTotalFrames)) 
      {
          nRetVal = -1;
      }
      else
      {
          if (nFrame != (aI->m_APEFileInfo.nTotalFrames - 1))
          { 
            nRetVal = aI->GetInfo(aI,APE_INFO_SEEK_BYTE, nFrame + 1,0) - aI->GetInfo(aI,APE_INFO_SEEK_BYTE, nFrame,0);
          }
          else
          { 
            nRetVal = aI->m_spIO->GetSize(aI->m_spIO) - aI->m_spAPETag->GetTagBytes(aI->m_spAPETag) - aI->m_APEFileInfo.nWAVTerminatingBytes - aI->GetInfo(aI,APE_INFO_SEEK_BYTE, nFrame,0);
          }
      }
      break;
  }
  case APE_INFO_FRAME_BLOCKS:
  {
      ape_int32 nFrame = nParam1;
      
      // bound-check the frame index
      if ((nFrame < 0) || (nFrame >= aI->m_APEFileInfo.nTotalFrames)) 
      {
          nRetVal = -1;
      }
      else
      {
          if (nFrame != (aI->m_APEFileInfo.nTotalFrames - 1)) 
          {
            nRetVal = aI->m_APEFileInfo.nBlocksPerFrame;
          }
          else
          { 
            nRetVal = aI->m_APEFileInfo.nFinalFrameBlocks;
          }
      }
      break;
  }
  case APE_INFO_TAG:
  /*Mod by Wei.Hisung 2007.03.06*/
      
      nRetVal = (ape_int32) aI->m_spAPETag;
      break;
  case APE_INTERNAL_INFO:
      nRetVal = (ape_int32) &(aI->m_APEFileInfo);
      break;
  }
  
  return nRetVal;
}

#pragma arm section code

#endif
#endif
